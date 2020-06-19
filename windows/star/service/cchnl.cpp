#include <string>
#include "json-c/json.h"
#include "base.h"
#include "star.h"
#include "cchnl.h"
#include "time.h"
#include "driveri.h"
#ifdef _ROUTER_
#include "relay.h"
#endif

CtrlChnl* g_cchnl;
#define CHNL_KEEPALIVE  1

ChnlTCP::ChnlTCP(SOCKADDR_IN* peerAddr, char* token):SockExTCP() {
  StarTlv msg(StarTlv::REGISTER);
  sockthread::pack_synheader(&msg);
  msg.pack_atom(StarTlv::REGISTER_TOKEN, NBS_TOKEN_LEN, token);
  UINT32 nettype = g_attr.netType;
  msg.pack_atom(StarTlv::REGISTER_NETTYPE, sizeof(nettype), (char*)&nettype);
  msg.pack_atom(StarTlv::REGISTER_NETATTR, sizeof(UINT32), (char*)&g_attr.netAttribute);
  
  //sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  NBS_CREATESOCKADDR(localAddr, g_attr.bestip, 0);
  /*int reuseaddr = true;
  int rtn = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));*/
  ::bind(sock, (SOCKADDR*)&localAddr, sizeof(SOCKADDR_IN));
  peerAddr->sin_family = AF_INET;
  ConnectEx((SOCKADDR*)peerAddr, msg.get_final(), msg.total);
  /* 20190415: this wait may cause exception; but we just simplify the process and let this exception exist in the app
  beceuse this exception happend hardly and effect little
  */

  char* replymsg = sockthread::wait(); 
  if (replymsg == nullptr) {
    throw std::logic_error("CHNLERR");
  }
  
  StarTlv rrtlv(replymsg);
  if (*(int*)rrtlv.get_tlv(StarTlv::RREGISTER_RESULT) != 0) {
    throw std::logic_error("CHNL RESULT ERR");
  }

  socklen_t len = sizeof(localAddr);
  getsockname(sock, (SOCKADDR*)&localAddr, &len);
  if (g_attr.bestip == 0) {
    g_attr.bestip = localAddr.sin_addr.s_addr;
  }

  driveri::updateFwdInfo((SOCKADDR*)&localAddr, DRIVERI_TUNNEL);
}

ChnlTCP::~ChnlTCP() {
  service* svc = nullptr;
  list_head* entry;
  list_for_each(entry, &g_attr.clients) {
    Client* clt = container_of(entry, Client, entry);
    list_for_each_del(&clt->allsvc) {
      svc = container_of_del(service, entry);
      if (svc->fwdChnl == this) {
        svc->fwdChnl = nullptr;
        svc->notinuse();
      }
    }
  }

  if (g_cchnl->chnl == this) {
    g_cchnl->setUpChnl(nullptr);
    testnettype();
  }
}

bool ChnlTCP::notUsing() {
  if (g_cchnl->chnl == this) {
    return true;
  }

  list_head* entry;
  list_head* tmp;
  list_for_each(tmp, &g_attr.clients) {
    Client* clt = container_of(tmp, Client, entry);
    list_for_each(entry, &clt->allsvc) {
      service* svc = container_of(entry, service, entry);
      if (svc->fwdChnl == this) {
        return false;
      }
    }
  }

  return true;
}

int ChnlTCP::onConnect(bool bConnect) {
  if (!bConnect) {
    return 0;
  }

  this->SockExTCP::onConnect(bConnect);
  DWORD opt = true;
  int rtn = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, sizeof(opt)); //modify keepalive
  
  int interval = CHNL_KEEPALIVE * 5;
  //setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, &interval, sizeof(int)); 

  /* Send next probes after the specified interval. Note that we set the
   * delay as interval / 3, as we send three probes before detecting
   * an error (see the next setsockopt call). */
  interval = CHNL_KEEPALIVE;
  //setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
  DBG("setsockopt TCP_KEEPINTVL: %d", errno);
   
/* //windows
  struct tcp_keepalive tcpka = { tcpka.onoff = 1, tcpka.keepalivetime = 5000, tcpka.keepaliveinterval = 1000 }; //according to msdn, tcp will send 10 packet using interval, so will 8s to detect tcp error
  rtn = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &tcpka, sizeof(tcpka), NULL, 0, &opt, NULL, NULL); 
*/

  return 0;
}

/*
int ClientHost::regHost(int reg) {
  DBG("regHost:%s, %p, local:%d", domainName, g_cchnl->chnl, local);
  if (g_cchnl->chnl != nullptr && (!local) ) {
    // first message is register the client, include token, username, clientid,
    StarTlv msg(StarTlv::REGHOST);
    msg.pack_atom(StarTlv::REGHOST_TYPE, sizeof(int), (char*)&reg);
    msg.pack_atom(StarTlv::REGHOST_HOSTNAME, strlen(domainName), domainName);
    send(g_cchnl->chnl->sock, msg.get_final(), msg.total, MSG_NOSIGNAL);
  }

  return 0;
}
*/
UINT32 CtrlChnl::getbestip() {
  return g_attr.bestip;
}

int CtrlChnl::createUplink(SOCKADDR_IN* addr, char* token) {
  //local shell using macaddr login, must create uplink
  try {
    g_cchnl->chnl = new ChnlTCP(addr, token);

#ifdef _ROUTER_
    relayi::setUplink(g_cchnl->chnl->sock);
    if ((g_attr.netAttribute & NBS_NETATTR_SHARE) == NBS_NETATTR_SHARE) {
      relayi::changeRelayIP(g_attr.bestip, relayi::IPOP::ADDRELAYIP);
    }
#endif
  } catch (exception &e) {
    LOG("NEW CHNL ERR:%s", e.what());
    g_cchnl->chnl = nullptr;

    return NBS_MAX_ERROR;
  }

  return NBS_OK;
}

void CtrlChnl::setUpChnl(ChnlTCP* chnl) {
  g_cchnl->chnl = chnl;

  SOCKET uplink = INVALID_SOCKET;
  if (chnl != nullptr) {
    uplink = chnl->sock;
  }

#ifdef _ROUTER_
  relayi::setUplink(uplink);
#endif // _ROUTER_ 
}

CtrlChnl::CtrlChnl() {
  g_cchnl = this;
  
  chnl = nullptr;
  /*autopowers = json_object_from_file(FL_AUTOPOWER);
  if (autopowers == nullptr) {
    autopowers = json_object_new_object();
  }

  char port[6] = { 0 };
  int n = sprintf(port, "%d", uiserver->getPort());

  FILE* fp = fopen("starport.txt", "w");
  fwrite(port, sizeof(char), n, fp);
  fclose(fp);*/

  //tlvcbs[StarTlv::INNERNOTIFYNETTYPE] = CtrlChnl::proInnerNotifyNetType;
  tlvcbs[StarTlv::RREGISTER] = TlvSynReplyCb;
  tlvcbs[StarTlv::NOTIFYRELAYCHANGE] = CtrlChnl::proNotifyRelayChange;
  //tlvcbs[StarTlv::QUERYSVCADDR] = CtrlChnl::proQuerySvcAddr;
  tlvcbs[StarTlv::RQUERYSVCADDR] = TlvSynReplyCb;
  tlvcbs[StarTlv::ROUTING] = CtrlChnl::proRouting;
  tlvcbs[StarTlv::RROUTING] = TlvSynReplyCb;
  tlvcbs[StarTlv::RREQUESTPUBLICADDR] = TlvSynReplyCb;
  //tlvcbs[StarTlv::INNERIPCHANGE] = CtrlChnl::proInnerIpChange;
  tlvcbs[StarTlv::INNERSVCISUSING] = CtrlChnl::proInnerSvcIsUsing;
  tlvcbs[StarTlv::INNER_FULLCONE_GETADDR] = TlvSynReplyCb;
}

// g_cchnl exists alltime
CtrlChnl::~CtrlChnl() {
  /*list_for_each_del(&hosts) {
    ClientHost* host = container_of_del(ClientHost, entry);
    delete host;
  } must confirm delete all the chnl
  */
    
  g_attr.netType = NBS_NETTYPE::INVALID;
  g_cchnl = NULL;
  delete chnl;

/*  json_object_put(autopowers);

  remove("starport.txt");
  delete uiserver; */
}

/*
void CtrlChnl::clear() {
  g_attr.netType = NBS_NETTYPE::INVALID;

  list_for_each_del(&allsvc) {
    service* svc = container_of_del(service, entry);
    delete svc;
  }
  
  list_for_each_del(&hosts) {
    ClientHost* host = container_of_del(ClientHost, entry);
    ChnlTCP* chnl = host->chnl;
    delete host;
    if (chnl != nullptr) {
      if (chnl->notUsing()) {
        delete chnl;
      }
    }
  }
}*/

int CtrlChnl::proNotifyRelayChange(SockEx* esock, StarTlv& tlvs) {
  ChnlTCP* chnl = (ChnlTCP*)esock;
  UINT32* relayip = (UINT32*)tlvs.get_tlv(StarTlv::NOTIFYRELAYCHANGE_SUBTYPE::NOTIFYRELAYCHANGE_IP);

  if (relayip != nullptr) {
    NBS_CREATESOCKADDR(addr, *relayip, *(UINT16*)tlvs.get_tlv(StarTlv::NOTIFYRELAYCHANGE_SUBTYPE::NOTIFYRELAYCHANGE_PORT));
    char* token = tlvs.get_tlv(StarTlv::NOTIFYRELAYCHANGE_SUBTYPE::NOTIFYRELAYCHANGE_TOKEN);
    //char* hostname = tlvs.get_tlv(StarTlv::NOTIFYRELAYCHANGE_SUBTYPE::NOTIFYRELAYCHANGE_HOSTNAME);
    try {
      LOG("++EX:sock %d rcv relay change to 0x%08x:%d", chnl->sock, addr.sin_addr.s_addr, addr.sin_port);
      ChnlTCP* newChnl = new ChnlTCP(&addr, token);
      g_cchnl->setUpChnl(newChnl);

      list_head* entry;
      list_for_each(entry, &g_attr.clients) {
        Client* clt = container_of(entry, Client, entry);
        if (clt->regHost(1) == 0) {
          LOG("host(%s) register on chnl(sock:%d) because relay change", clt->getdefaulthost(nullptr), newChnl->sock);
        }
      }
    } catch (std::exception &e) {
      //if the remote socket down, may cause duplicate free ChnlTCP
      LOG("ERR: cannot connect to new relay because:%s", e.what());
    }
  }

  if (chnl->notUsing()) {
    LOG("chnl(sock:%d) delete because not using", chnl->sock);
    delete chnl;
  }

  LOG("--EX:relay change");
  return 0;
}

int CtrlChnl::proRouting(SockEx* esock, StarTlv& tlvs) {
  char* synheader = tlvs.get_tlv(TLV_RES_SYNHEAD);
  char* dest = tlvs.get_tlv(StarTlv::ROUTING_DHOST);
  char* jsostr = tlvs.get_tlv(StarTlv::ROUTING_PAYLOAD);

  DBG("CtrlChnl::proRouting, :%s, jsostr:%s", dest, jsostr);
  StarTlv rrouting(StarTlv::RROUTING);
  rrouting.pack_atom(TLV_RES_SYNHEAD, tlvs.get_len(synheader), synheader);
  
  if (jsostr != nullptr) {
    list_head* entry;
    list_for_each(entry, &g_attr.clients) {
      Client* clt = container_of(entry, Client, entry);
      char host[NBS_HOSTNAME_SIZE];
      if (strcmp(clt->getdefaulthost(host), dest) == 0) {
        json_object* jso = json_tokener_parse(jsostr);
        json_object* reply = json_object_new_object();
        json_object* item = nullptr;
        if (json_object_object_get_ex(jso, "msgtype", &item)) {
          if (strcmp(json_object_get_string(item), "QUERYSVCADDR") == 0) {
            jsostr = proQuerySvcAddr(jso, reply);
          } //else if () {}
        }

        rrouting.pack_atom(StarTlv::RROUTING_PAYLOAD, strlen(jsostr), jsostr);

        json_object_put(jso);
        json_object_put(reply);
      }
    }
  }
  
  send(esock->sock, rrouting.get_final(), rrouting.total, MSG_NOSIGNAL);
  return 0;
}

char* CtrlChnl::proQuerySvcAddr(json_object* msg, json_object* reply) {
  json_object* item;
  json_object_object_get_ex(msg, "domain", &item);
  char* domain = (char*)json_object_get_string(item);
  json_object_object_get_ex(msg, "svctype", &item);
  char* svctype = (char*)json_object_get_string(item);
  
  DBG("proQuerySvcAddr:%s, %s", domain, svctype);
  service* svc = service::get(domain, svctype);
  if (svc != nullptr) {
    Client* clt = svc->clt;
    if (clt->bAutoPower) {
      clt->autopower();
    }

    if (svc != nullptr) {
      char addrstr[INET6_ADDRSTRLEN];

      //LOG("++EX:get svc %s:%d on host %s", device, ntohs(nport), domain);
      LOG("++EX:get svc %s on host %s", svctype, domain);
      SOCKADDR_IN interAddr;
      if (svc->getPublicAddr(nullptr, &interAddr) == NBS_OK) {
        inet_ntop(AF_INET, &interAddr.sin_addr, addrstr, INET6_ADDRSTRLEN);
        DBG("CTRLIP, %s:%d", addrstr, interAddr.sin_port);
        json_object_object_add(reply, "ip", json_object_new_string(addrstr));
        json_object_object_add(reply, "port", json_object_new_int(ntohs(interAddr.sin_port)));
      }
      LOG("--EX:get svc");
    }
  }

  return (char*)json_object_to_json_string(reply);
}

int CtrlChnl::regSvc(Client* clt, char* host, char* name, UINT16 nport, char proto) {
  list_head* entry;
  list_for_each(entry, &clt->allsvc) {
    service* svc = container_of(entry, service, entry);
    if (strcmp(name, svc->type) == 0 && strcmp(host, svc->host) == 0) {
      return 0;
    }
  }
    
  new service(clt, host, name, nport, proto);
  return 0;
}

int CtrlChnl::unregSvc(Client* clt, char* host, char* name, UINT16 nport, char proto) {
  list_head* entry;
  list_for_each(entry, &clt->allsvc) {
    service* svc = container_of(entry, service, entry);
    if (strcmp(name, svc->type) == 0 && strcmp(host, svc->host) == 0) {
      delete svc;
    }
  }
    
  return 0;
}

int CtrlChnl::proInnerSvcIsUsing(SockEx* esock, StarTlv& tlvs) {
/*  SOCKADDR_IN* addr = (SOCKADDR_IN*)tlvs.get_tlv(StarTlv::INNERSVCISUSING_ADDR);
  service* svc = g_cchnl->getService(addr->sin_addr.s_addr, addr->sin_port);
  if (svc == nullptr) {
    return 0;
  }

#ifdef _NODRIVER_
  if (svc->allFwdSocks.empty()) {
    delete svc;
  }
#else
  //may need check the fwdpkt, if no packet fwd in 15minutes, we delete this svc
#endif // !_NODRIVER_
*/
  return 0;
}

