// star.cpp : 此文件包含 "main" 函数。程序执行将在此处开始
//
#include <iostream>
#include <exception>
#include "json-c/json.h"
#include "base.h"
#include "star.h"
#include "cchnl.h"
#include "driveri.h"

#ifdef _ROUTER_
#include "relay.h"
#endif
using namespace std;
StarAttr g_attr;

void SerializeTlv(StarTlv& tlv) {
  NBS_CREATESOCKADDR(sockdownaddr, htonl(0x7F000001), htons(NBS_SERVER_PORT));
  StarTlv msg(StarTlv::INNER_SERIALIZE);
  msg.pack_atom(TLV_RES_JSON, tlv.total, tlv.get_final());
  sendto(g_attr.shellsock->sock, msg.get_final(), msg.total, MSG_NOSIGNAL, (SOCKADDR*)&sockdownaddr, sizeof(sockdownaddr));
}

void sig_alrm(int signo) {
  StarTlv tlv(StarTlv::INNER_TIMER);
  SerializeTlv(tlv);
  return;
}

void sendToTsinghua() {
    if (IN6_IS_ADDR_UNSPECIFIED(&g_attr.bestip6)) {
        sockthread::wait(0);

        struct hostent* ht = gethostbyname("ipv6.tsinghua.edu.cn");
        if (ht != nullptr) {
            SOCKET sock = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, 0);
            sockaddr_in6 addr6;
            socklen_t len = sizeof(addr6);
            addr6.sin6_family = AF_INET6;
            memcpy(&addr6.sin6_addr, (in6_addr*)ht->h_addr, sizeof(in6_addr));
            addr6.sin6_port = htons(80);
            if (connect(sock, (sockaddr*)&addr6, sizeof(sockaddr_in6)) == 0) {
                getsockname(sock, (sockaddr*)&addr6, &len);
                memcpy(&g_attr.bestip6, &addr6.sin6_addr, sizeof(in6_addr));//将获取到的ip6地址复制到这里
                DBG("get localipv6 ok: ");
            }
            closesocket(sock);
            char addrstr[INET6_ADDRSTRLEN];
            DBG("my ipv6:%s, errno:%s", inet_ntop(AF_INET6, &addr6.sin6_addr, addrstr, INET6_ADDRSTRLEN), strerror(errno));

        }
    }
}


int main(int argc, char* argv[]) {
  //daemon(1, 0);
  openlog((char*)"star", LOG_CONS, 0);
  LOG("START VER:%s", VERSION_STR);
  sendToTsinghua();
  //signal(SIGALRM, sig_alrm);
  /* TODO: 修改mac地址，从入参中获得mac地址
  memcpy(g_attr.mac, */
  //for (int i = 1; i <2 ; i++) {
    //  cout << argv[i] << endl;
  //}

  cout << argv[1] << endl;
  
  sscanf(argv[1], "%02X%02X%02X%02X%02X%02X", &g_attr.mac[0], &g_attr.mac[1], &g_attr.mac[2], &g_attr.mac[3], &g_attr.mac[4], &g_attr.mac[5]);

  sockthread mainthread(true);
  new driveri(); //must init driveri first, because CtrlChnl depend driveri
  new CtrlChnl();
  
  NBS_CREATESOCKADDR(server_addr, INADDR_ANY, NBS_SERVER_PORT);
  g_attr.shellsock = new ShellUdpSock(&server_addr);
//  g_attr.addrpool = new AddrPool();
  StarAttr::parsecfg();

  testnettype();
  //alarm(1); //启动定时器

  sockRoutine(&mainthread);
  return 0;
}

void IntraAddrtoPublic(SOCKADDR_IN* xAddr) {
  return;
}

int testnettype() {
  g_attr.netAttribute = NBS_NETATTR_NONE;
  list_head* entry;
  list_for_each(entry, &g_attr.clients) {
    Client* clt = container_of(entry, Client, entry);
    clt->loginRtn = NBS_NETWORK_ERROR;
    json_object_put(clt->cfginsrv);
    clt->cfginsrv = nullptr;
  }

#if defined(_ROUTER_) || defined(_WINSOCKAPI_)
  g_attr.netType = NBS_NETTYPE::INVALID;
#else
  g_attr.netType = NBS_NETTYPE::SYMMETRIC;
#endif

  StarTlv tlvs(StarTlv::INNERNETTYPETEST);
  SerializeTlv(tlvs);
  return 0;
}

int proInnerNetTypeTest(SockEx* esock, StarTlv& tlvs) {
  DBG("proInnerNetTypeTest, type:%d", g_attr.netType);
  if (g_attr.netType == NBS_NETTYPE::INVALID) {
    NBS_NETTYPE lasttype = g_attr.netType;

    SOCKADDR_IN selfAddr;
    driveri::GetGateWayAndIfIP(nullptr, (UINT32*)&selfAddr.sin_addr.s_addr);
    UINT16 knownports[] = { htons(80) };
    //当前是在服务器上做端口测试，过于耗时，很可能导致服务器线程用光，不再用这种方法测试了；可以考虑客户端通过驱动发包来测试
    UINT32 testknownportnum = 0; // sizeof(knownports) / sizeof(UINT16); 

    StarTlv msg(StarTlv::GETSELFADDR);
    sockthread::pack_synheader(&msg);
    msg.pack_atom(StarTlv::GETSELFADDR_INTERADDR, sizeof(selfAddr), (char*)&selfAddr);
    msg.pack_atom(StarTlv::GETSELFADDR_KNOWNPORT_NUM, sizeof(testknownportnum), (char*)&testknownportnum);
    for (int i = 0; i < testknownportnum; i++) {
      msg.pack_atom(StarTlv::GETSELFADDR_KNOWNPORT, sizeof(UINT16), (char*)&knownports[i]);
    }

    struct hostent *ht = gethostbyname(NBS_SERVER_NAME);
    if (ht == nullptr) {
      return testnettype();
    }
    struct in_addr* srvaddr = (struct in_addr*)ht->h_addr;
    NBS_CREATESOCKADDR(rmtaddr, srvaddr->s_addr, NBS_SERVER_PORT);
    sendto(g_attr.shellsock->sock, msg.get_final(), msg.total, MSG_NOSIGNAL, (SOCKADDR*)&rmtaddr, sizeof(rmtaddr));
    char* buf = sockthread::wait(2 + 3 * testknownportnum);
    DBG("GET GETSELFADDR:%p", buf);
    if (buf == nullptr) {
      return testnettype();
    }

    StarTlv tlvs(buf);
    SOCKADDR_IN* interAddr = (SOCKADDR_IN*)tlvs.get_tlv(StarTlv::GETSELFADDR_INTERADDR);
    if (interAddr->sin_addr.s_addr == selfAddr.sin_addr.s_addr) {
      g_attr.netType = NBS_NETTYPE::DYNGLOBAL;
      testknownportnum = ntohl(*(UINT32*)tlvs.get_tlv(StarTlv::GETSELFADDR_KNOWNPORT_NUM));
      for (int i = 0; i < testknownportnum; i++) {
        UINT16 knownport = ntohs(*(UINT16*)tlvs.get_tlv(StarTlv::GETSELFADDR_KNOWNPORT));
        if (knownport == 80) {
          g_attr.netAttribute |= NBS_NETATTR_HTTP_VALID;
          g_attr.bannedport[80] = 0;
          g_attr.bannedport[443] = 0;
          g_attr.bannedport[8080] = 0;
        } else {
          g_attr.bannedport[80] = 1;
          g_attr.bannedport[443] = 1;
          g_attr.bannedport[8080] = 1;
        }
      }
    } else {
      g_attr.netType = NBS_NETTYPE::SYMMETRIC;
      SOCKET testsock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
      if (testsock != -1) {
        StarTlv tlvs(StarTlv::RTRANSNATADDR);
        sendto(testsock, tlvs.get_final(), tlvs.total, MSG_NOSIGNAL, (SOCKADDR*)interAddr, sizeof(SOCKADDR_IN));

        char* msg = sockthread::wait(2);
        if (msg != nullptr && msg[0] == StarTlv::RTRANSNATADDR) {
          g_attr.netAttribute |= NBS_NETATTR_FAKESERVERIP;
        }
      }
      closesocket(testsock);
    }

#ifdef _ROUTER_ //may cause exception
    if (lasttype != g_attr.netType) {
      LOG("+++nettype change");
      delete (relayi*)g_attr.relay;
      g_attr.relay = new relayi(g_attr.netType);
      LOG("---nettype change ok");
    }
#endif // _ROUTER_
  }

#if 0
  StarTlv shellmsg(StarTlv::INNERSHELLMSG);
  Client* clt = Client::getbymac((char*)g_attr.mac);
  json_object* commitcfg = json_tokener_parse(json_object_to_json_string(clt->cfg));

  struct in_addr ifaddr;
  UINT32 gwip = 0;
  driveri::GetGateWayAndIfIP(nullptr, (UINT32*)&ifaddr.s_addr);
  
  json_object_object_add(commitcfg, "msgtype", json_object_new_string("commitcfg"));
  json_object_object_add(commitcfg, "cltip", json_object_new_string(inet_ntoa(ifaddr)));
  json_object_object_add(commitcfg, "mac", json_object_new_string((char*)clt->mac));

  json_object* user = json_object_new_object();
  json_object_object_add(user, "name", json_object_new_string((char*)clt->mac));
  json_object_object_add(commitcfg, "user", user);

  char* jsostr = (char*)json_object_to_json_string(commitcfg);
  shellmsg.pack_atom(TLV_RES_JSON, strlen(jsostr), jsostr);
  sockthread::pack_synheader(&shellmsg);

  NBS_CREATESOCKADDR(gwaddr, gwip, NBS_SERVER_PORT);
  sendto(g_attr.shellsock->sock, shellmsg.get_final(), shellmsg.total, MSG_NOSIGNAL, (SOCKADDR*)&gwaddr, sizeof(gwaddr));
  json_object_put(commitcfg);
  char* buf = sockthread::wait();
  if (buf != nullptr) {
    return 0;
  }
#endif

  StarTlv lgnmsg(StarTlv::INNERLOGIN);
  lgnmsg.pack_atom(StarTlv::INNERLOGIN_CLTMAC, IFHWADDRLEN, (char*)g_attr.mac);
  SerializeTlv(lgnmsg);

  return 0;
}

int ShellUdpSock::proInnerSerialize(SockEx* esock, StarTlv& tlvs) {
  char* msg = tlvs.get_tlv(TLV_RES_JSON);
  unsigned char type = (unsigned char)msg[0];
  StarTlv truetlvs(msg);
  return tlvcbs[type](esock, truetlvs);
}

int ShellUdpSock::procTlvMsg(char* data, int len) {
  UCHAR type = (UCHAR)data[0];
  switch (type) {
  case StarTlv::INNER_SERIALIZE:
  case StarTlv::INNERLOGIN:
    if (peer->sin_addr.s_addr != htonl(0x7F000001)) {
      break;
    }
  default:
    StarTlv tlvs(data);
    if (type == StarTlv::RGETCTRLINFO) {
      TlvSynReplyCb(this, tlvs);
    }else if (tlvcbs[type] != nullptr) {
      tlvcbs[type](this, tlvs);
    }
  }

  return 0;
}

#if 0
Client::Client(UINT32 cltip, char autopower = 0) {
  list_add_tail(&entry, &g_attr.clients);
  cfginsrv = nullptr;
  cfg = nullptr;
  loginRtn = NBS_NETWORK_ERROR;

  //NBS_CREATESOCKADDR(addr, cltip, 0);
  //driveri::getArp(cltip, mac);
  //getnameinfo((SOCKADDR*)&addr, sizeof(addr), name, sizeof(name), nullptr, 0, NI_NUMERICHOST);
  bAutoPower = autopower;
  hostip = cltip;
  memset(mac, 0, sizeof(mac));
 // name[0] = 0;

}
#endif
Client::Client(json_object* cltcfg) {
  list_add_tail(&entry, &g_attr.clients);
  
  json_object* item;
  if (json_object_object_get_ex(cltcfg, "mac", &item)) {
    //https://blog.csdn.net/hs_guanqi/article/details/2258558
    sscanf(json_object_get_string(item), "%02X%02X%02X%02X%02X%02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  } else {
        memcpy(mac, g_attr.mac, sizeof(g_attr.mac));
  }

  bAutoPower = 1;
  if (json_object_object_get_ex(cltcfg, "autopower", &item)) {
    bAutoPower = json_object_get_int(item);
  }
  cfg = cltcfg;
  cfginsrv = nullptr;
  loginRtn = NBS_NETWORK_ERROR;
  initActiveip();

  //json_object_object_get_ex(cltcfg, "name", &item);
  //strcpy(name, (char*)json_object_get_string(item));
  INIT_LIST_HEAD(&allsvc);
  if (json_object_object_get_ex(cfg, "services", &item)) {
    char hoststr[NBS_HOSTNAME_SIZE];
    char* hostdefault = getdefaulthost(hoststr);
    int num = json_object_array_length(item);
    for (int i = 0; i < num; i++) {
      json_object* svc = json_object_array_get_idx(item, i);
      json_object* port;
      json_object* proto;
      json_object* type;
      json_object* cname;
      char* host = hostdefault;
      json_object_object_get_ex(svc, "port", &port);
      json_object_object_get_ex(svc, "type", &type);
      json_object_object_get_ex(svc, "protocol", &proto);
      if (json_object_object_get_ex(svc, "cname", &cname)) {
        host = (char*)json_object_get_string(cname);
      }
      CtrlChnl::regSvc(this, host, (char*)json_object_get_string(type), htons(json_object_get_int(port)), json_object_get_int(proto));
    }

    if (memcmp(mac, g_attr.mac, IFHWADDRLEN) == 0) {
      new appsrv(this, hostdefault, NBS_SERVER_PORT);
    }
  }
}

Client::~Client() {
  list_del(&entry);

  list_for_each_del(&allsvc) {
    service* svc = container_of_del(service, entry);
    delete svc;
  }

  json_object_put(cfginsrv);
  //json_object_put(cfg);

  //StarAttr::savecfg();
}

#if 0
char* Client::getmacbyname(char* devname) {
  list_head* entry;
  list_for_each(entry, &g_attr.clients) {
    Client* clt = container_of(entry, Client, entry);
    if (strcmp(clt->name, devname) == 0) {
      return clt->mac;
    }
  }

  return nullptr;
}
#endif

Client* Client::getbymac(char* mac) {
  list_head* entry;
  list_for_each(entry, &g_attr.clients) {
    Client* clt = container_of(entry, Client, entry);
    if (memcmp(clt->mac, mac, IFHWADDRLEN) == 0) {
      return clt;
    }
  }

  return nullptr;
}

void Client::autopower() {
  SOCKET sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (sock == INVALID_SOCKET) {
    DBG("autopower sock:%d, err:%s", sock, strerror(errno));
    return;
  }

  int intragwip = driveri::getIfIP(g_attr.getintragwif());
  //must bind to intragwip, oresle, the target pc maynot rcv the magic packet; maybe the packet send to internet
  NBS_CREATESOCKADDR(localaddr, intragwip, 0);
  int rtn = ::bind(sock, (SOCKADDR*)&localaddr, sizeof(localaddr));
  
  //broadcast, boptval must be int, not bool;
  //bool bOptVal = TRUE;
  int bOptVal = true;
  setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&bOptVal, sizeof(bOptVal));
  SOCKADDR_IN to;
  to.sin_family = AF_INET;
  to.sin_port = 80; //MUST set any port not 0
  to.sin_addr.s_addr = INADDR_BROADCAST;

  unsigned char magicpacket[128];
  memset(magicpacket, 0xff, 6);
  unsigned char* lastpos = magicpacket + 6;
  for (int i = 0; i < 16; i++) {
    memcpy(lastpos, mac, IFHWADDRLEN);
    lastpos += IFHWADDRLEN;
  }

  rtn = sendto(sock, (const char*)magicpacket, lastpos - magicpacket, MSG_NOSIGNAL, (SOCKADDR*)&to, sizeof(to));
  closesocket(sock);
}

int Client::login(char* name, char* pwd) {
  json_object* jso = json_object_new_object();
  int rtn = NBS_NETWORK_ERROR;
  StarTlv loginrtn(StarTlv::INNERLOGINRTN);
  loginrtn.pack_atom(StarTlv::INNERLOGINRTN_CLTMAC, IFHWADDRLEN, (char*)mac);

  if (g_cchnl->chnl != nullptr || memcmp(mac, g_attr.mac, IFHWADDRLEN) == 0) {
    do {
      if (name == nullptr) { //only device, using router to access the client
        rtn = NBS_OK;
        break;
      }
      LOG("name:%s login, pwd:%p", name, pwd);
      struct hostent *ht = gethostbyname(NBS_SERVER_NAME);
      if (ht == nullptr) {
        break;
      }
      struct in_addr* srvaddr = (struct in_addr*)ht->h_addr;

      StarTlv msg(StarTlv::LOGIN);
      sockthread::pack_synheader(&msg);
      msg.pack_atom(StarTlv::LOGIN_VERSION, sizeof(VERSION_STR) - 1, (char*)VERSION_STR);
      json_object_object_add(jso, "name", json_object_new_string(name));
      if (pwd == nullptr) {
        json_object_object_add(jso, "nettype", json_object_new_int(g_attr.netType));
        //json_object_object_add(jso, "needrelay", json_object_new_int(1));
      } else {
        json_object_object_add(jso, "pwd", json_object_new_string(pwd));
      }
      msg.pack_atom(TLV_RES_JSON, strlen(json_object_to_json_string(jso)), (char*)json_object_to_json_string(jso));
      
      NBS_CREATESOCKADDR(rmtaddr, srvaddr->s_addr, NBS_SERVER_PORT);
      sendto(g_attr.shellsock->sock, msg.get_final(), msg.total, MSG_NOSIGNAL, (SOCKADDR*)&rmtaddr, sizeof(rmtaddr));

      char* buf = sockthread::wait();
      DBG("buf:%p", buf);
      if (buf == nullptr) {
        break;
      }
      json_object_put(jso);

      StarTlv tlvs(buf);
      jso = json_tokener_parse(tlvs.get_tlv(TLV_RES_JSON));
      DBG("jso:%p", jso);
      if (jso == nullptr) {
        break;
      }

      json_object* item;
      json_object_object_get_ex(jso, "result", &item);
      rtn = json_object_get_int(item);
      DBG("rnt:%p", rtn);
      if (rtn != NBS_OK) {
        break;
      }
      
      json_object* sharecfg;
      json_object_object_get_ex(jso, "cfg", &item);
      if (json_object_object_get_ex(item, "share", &sharecfg) && json_object_get_int(sharecfg)) {
        g_attr.netAttribute |= NBS_NETATTR_SHARE;
      }

      if (g_cchnl->chnl == nullptr) {
        json_object* relayinfo;
        json_object_object_get_ex(jso, "relayinfo", &relayinfo);
        // gettoken from relayinfo then send udp to server get truerelay
        json_object_object_get_ex(relayinfo, "token", &item);
        char* token = (char*)json_object_get_string(item);
        StarTlv ctrltlv(StarTlv::GETCTRLINFO);
        sockthread::pack_synheader(&ctrltlv);
        ctrltlv.pack_atom(StarTlv::GETCTRLINFO_TOKEN, strlen(token), token);
        ctrltlv.pack_atom(StarTlv::GETCTRLINFO_NETTYPE, sizeof(UINT32), (char*)&g_attr.netType);
        sendto(g_attr.shellsock->sock, ctrltlv.get_final(), ctrltlv.total, MSG_NOSIGNAL, (SOCKADDR*)&rmtaddr, sizeof(rmtaddr));
        char* xbuf = sockthread::wait();
        DBG("xbuf:%p", xbuf);
        if (xbuf == nullptr) {
          json_object_put(jso);
          return testnettype();
        }

        StarTlv rctrltlv(xbuf);
        sockaddr_in* relayaddr = (sockaddr_in*)rctrltlv.get_tlv(StarTlv::RGETCTRLINFO_ADDR);
        token = rctrltlv.get_tlv(StarTlv::RGETCTRLINFO_TOKEN);
        if (CtrlChnl::createUplink(relayaddr, token) != NBS_OK) {
          json_object_put(jso);
          return testnettype();
        }
      }
    } while (0);
  }

  json_object_object_add(jso, "result", json_object_new_int(rtn));
  const char* rlgnstr = json_object_to_json_string(jso);
  loginrtn.pack_atom(TLV_RES_JSON, strlen(rlgnstr), (char*)rlgnstr);
  json_object_put(jso);

  SerializeTlv(loginrtn);
  LOG("+++login result:%d", rtn);
  return 0;
}

int proInnerLogin(SockEx* esock, StarTlv& tlvs) {
  char* mac = (char*)tlvs.get_tlv(StarTlv::INNERLOGIN_CLTMAC);
  Client* clt = Client::getbymac(mac);
  if (clt == nullptr) {
    //clt = new Client(cltip, 0);
    throw std::logic_error("ERR:clt null");
  }
  /* 如果路由器下挂第三方设备（如智能摄像头），则可能使用mac地址作用户名进行登陆
  可以考虑换些设计思路，后面再具体设计吧，基本逻辑暂时保留
  思路一：比如第三方设备检测到有路由器后，通过路由器建立代理登陆？
  思路二：第三方设备不直接作为路由器的客户端，登陆流程是独立的，而是在选择中继时进行特别处理，如果路由器有中继能力，则直接使用路由器作为中继设备
  感觉2是优选思路
  char* user = tlvs.get_tlv(StarTlv::INNERLOGIN_USER);
  char* pwd = tlvs.get_tlv(StarTlv::INNERLOGIN_PWD);
  if (user != nullptr) { // login by web, check if the ip assigned to another client
    clt->loginRtn = NBS_USRPWD_ERROR;
    //json_object();
  }*/
  char user[NBS_USER_SIZE];
  clt->login(clt->getUser(user), clt->getPwd());
  return 0;
}

/*
客户端分为三类：
1、自身，以本机mac地址作为用户名登陆
2、下挂的PC，没有用户名
3、下挂的第三方设备，此时客户端有用户名（实际上仍然会使用mac地址作为用户名）
*/
char* Client::getUser(char* user) {
  json_object* item;
  if (json_object_object_get_ex(cfg, "user", &item)) {
    json_object* name;
    if (json_object_object_get_ex(item, "name", &name)) {
      return (char*)json_object_get_string(name);
    }
  }
  
  if (memcmp(mac, g_attr.mac, sizeof(mac)) == 0) {
    sprintf_s(user, NBS_USER_SIZE, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return user;
  }

  return nullptr;
}

char* Client::getPwd() {
  json_object* item;
  if (json_object_object_get_ex(cfg, "user", &item)) {
    json_object* pwd;
    if (json_object_object_get_ex(item, "pwd", &pwd)) {
      return (char*)json_object_get_string(pwd);
    }  
  }
  
  return nullptr;
}

int Client::proInnerLoginRtn(SockEx* esock, StarTlv& tlvs) {
  Client* clt = getbymac(tlvs.get_tlv(StarTlv::INNERLOGINRTN_CLTMAC));
  DBG("proInnerLoginRtn, clt:%p", clt);
  if (clt == nullptr) {
    return 0;
  }

  json_object* jso = json_tokener_parse(tlvs.get_tlv(TLV_RES_JSON));
  json_object* item;
  json_object_object_get_ex(jso, "result", &item);
  clt->loginRtn = json_object_get_int(item);
  DBG("proInnerLoginRtn, result:%d", clt->loginRtn);

  clt->cfginsrv = nullptr;
  if (json_object_object_get_ex(jso, "cfg", &item)) {
    clt->cfginsrv = json_object_get(item);
  }
  json_object_put(jso);

  switch (clt->loginRtn) {
  case NBS_OK:
    char user[NBS_USER_SIZE];
    //client which using name must register an default host which is the first in srv
    clt->regHost(1);
    //break;  because also need login others
  case NBS_USRPWD_ERROR:
    list_head* entry;
    list_for_each(entry, &g_attr.clients) {
      Client* nextclt = container_of(entry, Client, entry);
      if (nextclt != clt && nextclt->loginRtn != NBS_OK && clt->loginRtn != NBS_USRPWD_ERROR) {
        StarTlv lgnmsg(StarTlv::INNERLOGIN);
        lgnmsg.pack_atom(StarTlv::INNERLOGIN_CLTMAC, IFHWADDRLEN, (char*)&nextclt->mac);
        SerializeTlv(lgnmsg);
        break;
      }
    }
    break;
  case NBS_VER_INCOMPATIBLE:
    system("./starshell newest");
    LOG("INFO:restart because update version");
    exit(0);
  default:
    StarTlv lgnmsg(StarTlv::INNERLOGIN);
    lgnmsg.pack_atom(StarTlv::INNERLOGIN_CLTMAC, IFHWADDRLEN, (char*)&clt->mac);
    sockthread::wait(3);
    SerializeTlv(lgnmsg);
    break;
  }
  
  return 0;
}

#if 0
int Client::proInnerSetAutoPower(SockEx* esock, StarTlv& tlvs) {
  Client* clt = getbyip(*(UINT32*)tlvs.get_tlv(StarTlv::INNERSETAUTOPOWER_CLTIP));
  if (clt == nullptr) {
    return 0;
  }

  clt->bAutoPower = *tlvs.get_tlv(StarTlv::INNERSETAUTOPOWER_VALUE);
  return 0;
}

int Client::proInnerRegHost(SockEx* esock, StarTlv& tlvs) {
  char type = *tlvs.get_tlv(StarTlv::INNERREGHOST_TYPE);
  char* hostname = tlvs.get_tlv(StarTlv::INNERREGHOST_HOSTNAME);
  Client* clt = Client::getbyip(*(UINT32*)tlvs.get_tlv(StarTlv::INNERREGHOST_HOSTIP));
  if (clt == nullptr) {
    return 0;
  }

  if (type) {
    
    return CtrlChnl::regHost(clt, hostname);
  }

  return CtrlChnl::unregHost(clt, hostname);
}
#endif

int Client::proInnerRegSvc(SockEx* esock, StarTlv& tlvs) {
  LOG("proInnerRegSvc");
  return 0;
#if 0  
  UINT32 cltip = *(UINT32*)tlvs.get_tlv(StarTlv::INNERREGSVC_CLTIP);
  char proto = *tlvs.get_tlv(StarTlv::INNERREGSVC_PROTOCOL);
  UINT16 port = *(UINT16*)tlvs.get_tlv(StarTlv::INNERREGSVC_PORT);
  char type = *tlvs.get_tlv(StarTlv::INNERREGSVC_TYPE);
  Client* clt = Client::getbyip(cltip);
  if (clt == nullptr) {
    return 0;
  }
  
  if (type) {
    return CtrlChnl::regSvc(clt, port, proto);
  }

  return CtrlChnl::unregSvc(clt, port, proto);
#endif
}

int Client::proInnerShellMsg(SockEx* esock, StarTlv& tlvs) {
  char* jsostr = tlvs.get_tlv(TLV_RES_JSON);
  char* synheader = tlvs.get_tlv(TLV_RES_SYNHEAD);
  SOCKADDR_IN* udppeer = nullptr;
  json_object* item;
  json_object* jso = json_tokener_parse(jsostr);
  json_object* reply = json_object_new_object();
  char* replystr = nullptr;
  DBG("proInnerShellMsg:%s", jsostr);

  if (synheader == nullptr) {
    udppeer = ((ShellUdpSock*)esock)->peer;
  }

  json_object_object_get_ex(jso, "msgtype", &item);
  const char* type = json_object_get_string(item);
  if (strcmp(type, "getcfg") == 0) {
    replystr = StarAttr::procGetcfg(reply);
  } else if (strcmp(type, "commitcfg") == 0) {
    replystr = StarAttr::procCommitcfg(jso, reply);
  } else if (strcmp(type, "getcookie") == 0) { 
    //replystr = g_attr.gettoken();
  } else if (strcmp(type, "QUERYSVCADDR") == 0) {
    replystr = CtrlChnl::proQuerySvcAddr(jso, reply);
  }
    
  DBG("proc rely:%s", replystr);
  if (synheader == nullptr) { // from browser, using udp and no 
    int n = sendto(esock->sock, replystr, strlen(replystr), MSG_NOSIGNAL, (SOCKADDR*)udppeer, sizeof(SOCKADDR_IN));
    DBG("sendto proc rely:%d, %s", n, strerror(errno));
  } else {
    StarTlv replymsg(StarTlv::INNERRSHELLMSG);
    replymsg.pack_atom(TLV_RES_SYNHEAD, tlvs.get_len(synheader), synheader);
    replymsg.pack_atom(TLV_RES_JSON, strlen(replystr), replystr);
    send(esock->sock, replymsg.get_final(), replymsg.total, MSG_NOSIGNAL);
  }
    
  json_object_put(jso);
  json_object_put(reply);
  return 0;
}

int Client::initActiveip() {
  activeip.s_addr = 0;
  activeip6 = IN6ADDR_ANY_INIT;

#ifdef _ROUTER_  
  map<NeighEntryKey, NeighEntry*>::iterator iter;
  for (iter = g_attr.addrpool->addrcache.begin(); iter != g_attr.addrpool->addrcache.end(); iter++) {
    NeighEntry* entry = iter->second;
    if (entry->state == NUD_REACHABLE && memcmp(entry->mac, mac, IFHWADDRLEN) == 0) {
      if (IN6_IS_ADDR_V4COMPAT(&entry->xaddr.xaddr)) {
        activeip.s_addr = entry->xaddr.xaddr.s6_addr32[3];
      } else {
        memcpy(&activeip6, &entry->xaddr.xaddr, sizeof(in6_addr));
      }
      break;
    }
  }
#endif

  return 0;
}

#ifdef _ROUTER_
/*ref:
http://www.voidcn.com/article/p-fazrgbwg-bqy.html
http://man7.org/linux/man-pages/man7/rtnetlink.7.html */
int Client::proNeighChange(int family, void* xaddr, int type) {
  char addrstr[INET6_ADDRSTRLEN];
  DBG("Client::proNeighChange, addr:%s, type:%d", inet_ntop(family, xaddr, addrstr, INET6_ADDRSTRLEN), type);
  
  if (family == AF_INET6) {
    if (IN6_IS_ADDR_LINKLOCAL(xaddr) || IN6_IS_ADDR_SITELOCAL_NEW(xaddr)) { //must be global addr
      LOG("proNeighChange AF_INET6 invalid");
      return 0;
    }

    if (type == RTM_DELNEIGH) {
      if (!IN6_ARE_ADDR_EQUAL(&activeip6, xaddr)) {
        return 0;
      }

      activeip6 = IN6ADDR_ANY_INIT;
      //getActiveip(family);
    } else { //NEWNEIGH or GETNEIGH
      if (!IN6_IS_ADDR_UNSPECIFIED(&activeip6) || IN6_ARE_ADDR_EQUAL(&activeip6, xaddr)) {
        return 0;
      }
      memcpy(&activeip6, xaddr, sizeof(activeip6));
    }
  } else { //AF_INET
    in_addr* addr = (in_addr*)xaddr;
    if (type == RTM_DELNEIGH) {
      if (addr->s_addr == INADDR_ANY || addr->s_addr == INADDR_BROADCAST) {
        LOG("proNeighChange AF_INET invalid");
        return 0;
      }

      if (activeip.s_addr != addr->s_addr) {
        return 0;
      }

      activeip.s_addr = 0;
      //getActiveip(family);
    } else { //below is NEWNEHGH or GETNEIGH      
      if (activeip.s_addr != 0 || activeip.s_addr == addr->s_addr) {
        return 0;
      }
      memcpy(&activeip, xaddr, sizeof(activeip));
    }
  }

  list_head* entry;
  list_for_each(entry, &allsvc) {
    service* svc = container_of(entry, service, entry);
    //DBG("Client::proNeighChange, updateTransitInfo:%d", family);
    svc->updateTransitInfo(family);
  }

  return 0;
}
#endif

char* Client::getdefaulthost(char* hoststr) {
  hoststr[0] = 0;

  if (memcmp(g_attr.mac, mac, IFHWADDRLEN) == 0) {
    sprintf(hoststr, "%02x%02x%02x%02x%02x%02x.%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], DEFAULT_HOST_SUFFIX);
  } else {
    sprintf(hoststr, "%02x%02x%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x.%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
      g_attr.mac[0], g_attr.mac[1], g_attr.mac[2], g_attr.mac[3], g_attr.mac[4], g_attr.mac[5], DEFAULT_HOST_SUFFIX);
  }

  return hoststr;
}

// refrence from dnsmasq lease.c
char* Client::getnamefromdhcp(char* name) {
#ifdef _ROUTER_
  FILE* leasestream = fopen("/tmp/dhcp.leases", "r");
  if (leasestream == nullptr) {
    DBG("NO DHCP.LEASES, %s", strerror(errno));
    return "UNKNOWN";
  }
  
  char* line = nullptr;
  size_t len = 0;
  while (getline(&line, &len, leasestream) != -1) {
    char macstr[24];
    char dhcp_buff[256];
    if (sscanf(line, "%255s %255s %64s %255s %764s", dhcp_buff, macstr, dhcp_buff, name, dhcp_buff) == 5) {
      char cltmacstr[24];
      sprintf(cltmacstr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      DBG("cltmac:%s, mac:%s, name:%s", cltmacstr, macstr, name);
      if (strcmp(cltmacstr, macstr) == 0) {
        fclose(leasestream);
        free(line);
        return name;
      }
    }

    len = 0;
    free(line);
    line = nullptr;
  }
  
  //while (fscanf(leasestream, "%255s %255s %64s %255s %764s", dhcp_buff, macstr, dhcp_buff, name, dhcp_buff) == 5) {}

  fclose(leasestream);
#endif
  return (char*)"UNKNOWN";
}

int Client::regHost(int reg) {
  json_object* item;
  char user[NBS_USER_SIZE];
  if (cfginsrv == nullptr) {
    return -1;
  }

  if (!json_object_object_get_ex(cfginsrv, "hosts", &item)) {
    return -1;
  }

  int num = json_object_array_length(item);
  for (int i = 0; i < num; i++) {
    json_object* host = json_object_array_get_idx(item, i);
    StarTlv msg(StarTlv::REGHOST);
    msg.pack_atom(StarTlv::REGHOST_TYPE, sizeof(int), (char*)&reg);
    msg.pack_atom(StarTlv::REGHOST_HOSTNAME, json_object_get_string_len(host), (char*)json_object_get_string(host));
    send(g_cchnl->chnl->sock, msg.get_final(), msg.total, MSG_NOSIGNAL);
  }
 
  return 0;
}

char* StarAttr::getintragwif() {
  json_object* item;
  if (json_object_object_get_ex(cfg, "intragwif", &item)) {
    return (char*)json_object_get_string(item);
  }

  return (char*)"br-lan";
}

#if 0
char* StarAttr::gettoken() {
  json_object* item;
  char token[IFHWADDRLEN * 2 + NBS_TOKEN_LEN + 1];
  if (!json_object_object_get_ex(cfg, "token", &item)) {
    driveri::getIfMac(g_attr.getintragwif(), token);
    srand(time(0));
    for (int i = IFHWADDRLEN * 2; i < IFHWADDRLEN * 2 + NBS_TOKEN_LEN + 1; i++) { //only lowcase 
      token[i] = rand() % 26 + 0x61;
    }
    token[IFHWADDRLEN * 2 + NBS_TOKEN_LEN] = 0;
    item = json_object_new_string(token);
    json_object_object_add(cfg, "token", item);
    savecfg();
  }
    
  return (char*)json_object_get_string(item);
}
#endif

void StarAttr::savecfg() {
  json_object* clients = json_object_new_array();
  list_head* entry, *svcentry;
  list_for_each(entry, &g_attr.clients) {
    Client* clt = container_of(entry, Client, entry);
    char hoststr[NBS_HOSTNAME_SIZE];
    json_object* svcs = json_object_new_array();
    list_for_each(svcentry, &clt->allsvc) {
      service* svc = container_of(svcentry, service, entry);
      if (strcmp(svc->type, "appsrv") != 0) {
        json_object* jsosvc = json_object_new_object();
        json_object_object_add(jsosvc, "protocol", json_object_new_int(svc->protocol));
        json_object_object_add(jsosvc, "port", json_object_new_int(ntohs(svc->intraport)));
        json_object_object_add(jsosvc, "type", json_object_new_string(svc->type));
        if (strcmp(svc->host, clt->getdefaulthost(hoststr)) != 0) {
          json_object_object_add(jsosvc, "cname", json_object_new_string(svc->host));
        }

        json_object_array_add(svcs, jsosvc);
      }
    }
    json_object_object_add(clt->cfg, "services", svcs);
    json_object_array_add(clients, json_tokener_parse(json_object_to_json_string(clt->cfg)));
  }
  json_object_object_add(g_attr.cfg, "clients", clients);
  
  json_object_to_file("config.txt", g_attr.cfg);
  //json_object_object_del(g_attr.cfg, "clients");
  //json_object_put(clients);
}

void writehosts(UINT32 nip) {
#ifdef _ROUTER_
  FILE* fp = fopen("/www/nbs/hosts", "w");
  char hosts[256];
  struct in_addr inaddr;
  inaddr.s_addr = nip;
  int n = sprintf(hosts, "%s cfg.netbridge.club i.netbridge.club\n", inet_ntoa(inaddr));
  DBG("WRITEHOST:%s", hosts);
  fwrite(hosts, sizeof(char), n, fp);
  fclose(fp);

  system("/etc/init.d/dnsmasq restart");
#endif // _ROUTER_
}

int proIpChange(int ifindex, int changedip, int type) { 
  SOCKET toserver = INVALID_SOCKET;
  if (g_cchnl->chnl != nullptr) {
    toserver = g_cchnl->chnl->sock;
  }

  LOG("ip 0x%08x change, action:%u", changedip, type);
  if (type == RTM_DELADDR) { //DELETE
    if (changedip == g_cchnl->getbestip()) {
      exit(0);
    }
#if 0
    //这里是考虑到多播的情况，如果有多个公网IP，一个IP故障后尽量避免整体重启影响其他IP；当前简化成直接重启整个程序
    if (changedip == g_cchnl->getbestip()) {
      //the chnltcp must down, so the host no need process
      delete g_cchnl->chnl;
      g_cchnl->chnl = nullptr;
      g_attr.bestip = 0;
      g_attr.bestip6 = IN6ADDR_ANY_INIT;
    }

    // this situation is dynglobal ip
    list_head* pi, *pk;
    list_for_each(pi, &g_attr.clients) {
      Client* clt = container_of(pi, Client, entry);
      list_for_each(pk, &clt->allsvc) {
        service* svc = container_of(pk, service, entry);
        if (svc->interip == changedip) {
          svc->notinuse();
        }
      }
    }

#ifdef _ROUTER_        
    relayi::changeRelayIP(changedip, relayi::IPOP::DELRELAYIP);
#endif
#endif
  } else { //ADD
    /* cannot get bestip, because here the route not exist
    if (g_attr.bestIP == 0) {
      driveri::GetGateWayAndIfIP(&g_attr.bestIP);
    }*/
    /*
    json_object_object_get_ex(jso, "ifindex", &item);
    char ifName[IF_NAMESIZE];
    if_indextoname(json_object_get_int(item), ifName);
    DBG("ip change ifname:%s", ifName);
    if (strcmp(g_attr.getintragwif(), ifName) == 0) {
      g_attr.intragwip = changedip;
      writehosts(changedip);
    }

#ifdef _ROUTER_  
     这里是为了pppoe多拨预留的设计；代码有bug，如果变化的不是公网地址也会进行处理，所以暂时注释掉
    if (changedip != g_cchnl->getbestip() || (g_attr.netAttribute & NBS_NETATTR_SHARE) == NBS_NETATTR_SHARE) {
      relayi::changeRelayIP(changedip, relayi::IPOP::ADDRELAYIP);
    }
#endif
*/

    /*
    如果修改的是br-lan接口的ipv4地址，需要修改这条配置；主要是因为relay上，从tunnel出来的报文需要转发到源设备，此时源地址是公网ip+端口
    如果没有这条配置，报文会被修改掉port号
    uci show firewall | grep masq_src= > /tmp/starfwderrules
     system("uci set firewall.@zone[1].masq_src='192.168.9.0/24';uci commit firewall;service firewall restart");
    */
  }

  return 0;
}

StarAttr::StarAttr() {
  tlvcbs[StarTlv::INNERNETTYPETEST] = proInnerNetTypeTest;
  tlvcbs[StarTlv::RTRANSNATADDR] = TlvSynReplyCb;
  tlvcbs[StarTlv::GETSELFADDR] = TlvSynReplyCb;
  tlvcbs[StarTlv::RLOGIN] = TlvSynReplyCb;
  tlvcbs[StarTlv::INNERLOGIN] = proInnerLogin;
  tlvcbs[StarTlv::INNERLOGINRTN] = Client::proInnerLoginRtn;
  //tlvcbs[StarTlv::INNERREGHOST] = Client::proInnerRegHost;
  tlvcbs[StarTlv::INNERREGSVC] = Client::proInnerRegSvc;
  tlvcbs[StarTlv::INNERSHELLMSG] = Client::proInnerShellMsg;
  tlvcbs[StarTlv::INNERRSHELLMSG] = TlvSynReplyCb;
  //tlvcbs[StarTlv::INNERIPCHANGE] = proInnerIpChange;
  //tlvcbs[StarTlv::COOKIECONFIRM] = proCookieConfirm;
  tlvcbs[StarTlv::INNER_TIMER] = Timer::proInnerTimer;
  //tlvcbs[StarTlv::RGETCTRLINFO] = TlvSynReplyCb; 因为relayi里面也有这个处理，为了避免冲突，这个TLV直接放到ShellUdpSock::procSock函数中特殊处理

  tlvcbs[StarTlv::INNER_SERIALIZE] = ShellUdpSock::proInnerSerialize;

  


  bestip = 0;
  bestip6 = IN6ADDR_ANY_INIT;
  //driveri::getIfMac(g_attr.getintragwif(), (char*)mac);
  DBG("mac:%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  INIT_LIST_HEAD(&clients);
  //readconfig from config.txt
  cfg = json_object_from_file("config.txt");
  if (cfg == nullptr) {
    throw std::logic_error("ERR:no valid config file");
  }

#ifdef _ROUTER_
  netType = NBS_NETTYPE::INVALID;
  //writehosts(intragwip);
#else
  netType = NBS_NETTYPE::SYMMETRIC;
#endif
  netAttribute = NBS_NETATTR_NONE;
  memset(bannedport, 0, sizeof(bannedport));
  bannedport[80] = 1;
  bannedport[443] = 1;
  bannedport[8080] = 1;
  relay = nullptr;  
}

void StarAttr::parsecfg() {
  json_object* cltscfg;
  if (json_object_object_get_ex(g_attr.cfg, "clients", &cltscfg)) {
    int num = json_object_array_length(cltscfg);
    for (int i = 0; i < num; i++) {
      json_object* item;
      json_object* cltcfg = json_object_array_get_idx(cltscfg, i);
      new Client(json_object_get(cltcfg));
    }
    //json_object_object_del(cfg, "clients");
    //json_object_put(cltscfg);
  }
}

char* StarAttr::procGetcfg(json_object* reply) {
  json_object* clts = json_object_new_array();
  list_head* xentry;
  list_for_each(xentry, &g_attr.clients) {
    Client* clt = container_of(xentry, Client, entry);
    json_object* jsoclt = json_tokener_parse(json_object_to_json_string(clt->cfg));

    char tmp[NBS_HOSTNAME_SIZE];
    json_object_object_add(jsoclt, "defaulthost", json_object_new_string(clt->getdefaulthost(tmp)));

    char* cltname = (char*)"-";
    if (memcmp(clt->mac, g_attr.mac, IFHWADDRLEN) != 0) {
      cltname = clt->getnamefromdhcp(tmp);
    }
    json_object_object_add(jsoclt, "clientname", json_object_new_string(cltname));

    json_object_array_add(clts, jsoclt);
  }
  json_object_object_add(reply, "clients", clts);
  json_object_object_add(reply, "dhcpclients", getdhcpclients());

  return (char*)json_object_to_json_string(reply);
}

char* StarAttr::procCommitcfg(json_object* jso, json_object* reply) {
  json_object* item = nullptr;
  json_object* svccfg;
  json_object* commitcfg;
  if (json_object_object_get_ex(jso, "clients", &commitcfg)) {
    int cltnum = json_object_array_length(commitcfg);
    for (int i = 0; i < cltnum; i++) {
      json_object* jsoclt = json_object_array_get_idx(commitcfg, i);
      if (!json_object_object_get_ex(jsoclt, "mac", &item)) {
        continue;
      }

      Client* clt;
      char* macstr = (char*)json_object_get_string(item);
      if (strcmp(macstr, "000000000000") != 0) {
        char mac[IFHWADDRLEN];
        sscanf(macstr, "%02X%02X%02X%02X%02X%02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        clt = Client::getbymac(mac);
      } else {
        clt = Client::getbymac((char*)g_attr.mac);
      }
      if (clt == nullptr) {
        json_object* cltcfg = json_object_new_object();
        json_object_object_add(cltcfg, "mac", json_object_new_string(macstr));
        clt = new Client(cltcfg);
        if (clt == nullptr) {
          LOG("ERR: procCommitcfg clt null");
          return procGetcfg(reply);
        }
      }

      json_object_object_get_ex(jsoclt, "services", &svccfg);
      int svcnum = json_object_array_length(svccfg);
      for (int j = 0; j < svcnum; j++) {
        json_object* jsosvc = json_object_array_get_idx(svccfg, j);
        json_object_object_get_ex(jsosvc, "svckey", &item);
        int keyport = htons(json_object_get_int(item));
        list_head* entry;
        list_for_each(entry, &clt->allsvc) {
          service* svc = container_of(entry, service, entry);
          if (svc->intraport == keyport) {
            delete svc;
            break;
          }
        }

        json_object_object_get_ex(jsosvc, "servicetype", &item);
        char* svctype = (char*)json_object_get_string(item);
        json_object_object_get_ex(jsosvc, "port", &item);
        UINT16 nport = htons(json_object_get_int(item));
        if (json_object_object_get_ex(jsosvc, "enable", &item)) {
          if (json_object_object_get_ex(jsosvc, "cname", &item)) {
            new service(clt, (char*)json_object_get_string(item), svctype, nport, 6);
          } else {
            char hoststr[NBS_HOSTNAME_SIZE];
            new service(clt, clt->getdefaulthost(hoststr), svctype, nport, 6);
          }
        }
      }

      if (list_empty(&clt->allsvc)) {
        delete clt;
      }
    }
  }

  StarAttr::savecfg();
  return procGetcfg(reply);
}

#ifndef _WINSOCKAPI_
AddrPool::AddrPool(SOCKET s) :SockEx(s) {
  if (s == INVALID_SOCKET) {    
    struct sockaddr_nl addr;
    SOCKET fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_NEIGH;
    bind(fd, (struct sockaddr*) & addr, sizeof(addr));

    new (this)AddrPool(fd);
  } else {
    DBG("AddrPool getActiveip");
    int nlsock = -1;
    struct nlmsghdr* rcvmsg = nullptr;
    struct ndmsg* ndp = nullptr;

    try {
      char msgBuf[128] = { 0 };
      struct nlmsghdr* nlMsg = (struct nlmsghdr*)msgBuf;
      int len = 0;
      nlsock = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_ROUTE);
      if (-1 == nlsock) {
        throw std::logic_error("netlink not create");
      }

      nlMsg = (struct nlmsghdr*)msgBuf;
      nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
      nlMsg->nlmsg_type = RTM_GETNEIGH;
      nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
      ndp = (struct ndmsg*)NLMSG_DATA(nlMsg);
      if (send(nlsock, nlMsg, nlMsg->nlmsg_len, MSG_NOSIGNAL) < 0) {
        throw std::logic_error("cannot send");
      }

      while (true) {
        if ((len = recv(nlsock, msgBuf, 0, MSG_PEEK | MSG_TRUNC)) < 0) {
          throw std::logic_error("recv err");
        }
        rcvmsg = (struct nlmsghdr*)malloc(len);
        if ((len = recv(nlsock, rcvmsg, len, 0)) < 0) {
          throw std::logic_error("recv err");
        }
        if (rcvmsg->nlmsg_type == NLMSG_DONE || rcvmsg->nlmsg_type == NLMSG_ERROR) {
          break;
        }

        nlMsg = rcvmsg;
        for (; NLMSG_OK(nlMsg, (unsigned int)len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
          ndp = (struct ndmsg*) NLMSG_DATA(nlMsg);
          if (ndp->ndm_state == NUD_REACHABLE) {
            struct rtattr* ndap = RTM_RTA(ndp);
            int ndl = RTM_PAYLOAD(nlMsg);

            char* mac = nullptr;
            void* addr = nullptr;
            for (; RTA_OK(ndap, ndl); ndap = RTA_NEXT(ndap, ndl)) {
              if (ndap->rta_type == NDA_DST) {
                addr = RTA_DATA(ndap);
              } else if (ndap->rta_type == NDA_LLADDR) {
                mac = (char*)RTA_DATA(ndap);
              }

              if (addr != nullptr && mac != nullptr) {
                in6_addr v4compat;
                if (ndp->ndm_family == AF_INET) {
                  memset(&v4compat, 0, sizeof(in6_addr));
                  v4compat.s6_addr32[3] = ((in_addr*)addr)->s_addr;
                  addr = &v4compat;
                }
                NeighEntry* entry = new NeighEntry((in6_addr*)addr, mac);
                addrcache.insert(std::map<NeighEntryKey, NeighEntry*>::value_type(entry->xaddr, entry));
                
                break;
              }
            }
          }
        }

        free(rcvmsg);
        rcvmsg = nullptr;
      }
    } catch (exception & e) {
      LOG("ERR:getActiveip %s", e.what());
    }

    if (nlsock != INVALID_SOCKET) {
      closesocket(nlsock);
    }
    if (rcvmsg != nullptr) {
      free(rcvmsg);
    }
  }
  return;
}

//https://blog.csdn.net/hansel/article/details/38088645
int AddrPool::procSock(__uint32_t events) {
  struct nlmsghdr* nh = nullptr;
  void* rcvbuf;
  int retval = recv(sock, nh, 0, MSG_PEEK | MSG_TRUNC);
  if (retval < 0) {
    LOG("addrchange, peek < 0");
    return 0;
  }
  rcvbuf = malloc(retval);
  retval = recv(sock, rcvbuf, retval, 0);
  if (retval < 0) {
    LOG("addrchange, rcv < 0");
    free(rcvbuf);
    return 0;
  }

  for (nh = (struct nlmsghdr*)rcvbuf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval)) {
    if (nh->nlmsg_type == NLMSG_DONE || nh->nlmsg_type == NLMSG_ERROR) {
      break;
    }
      
    if (nh->nlmsg_type == RTM_NEWADDR || nh->nlmsg_type == RTM_DELADDR) {
      struct ifaddrmsg* ifa = (struct ifaddrmsg*)NLMSG_DATA(nh);
      struct rtattr* rth = IFA_RTA(ifa);
      int rtl = IFA_PAYLOAD(nh);
      while (rtl && RTA_OK(rth, rtl)) {
        if (rth->rta_type == IFA_LOCAL) {
          uint32_t ipaddr = *((uint32_t*)RTA_DATA(rth));
          proIpChange(ifa->ifa_index, ipaddr, nh->nlmsg_type);
        }
        rth = RTA_NEXT(rth, rtl);
      }        
    } else if (nh->nlmsg_type == RTM_NEWNEIGH || nh->nlmsg_type == RTM_DELNEIGH) {
      struct ndmsg* ndp = (struct ndmsg*) NLMSG_DATA(nh);
      //if (ndp->ndm_state == NUD_REACHABLE) {
        struct rtattr* ndap = RTM_RTA(ndp);
        int ndl = RTM_PAYLOAD(nh);

        //Client* clt = nullptr;
        char* mac = nullptr;
        void* addr = nullptr;
        for (; RTA_OK(ndap, ndl); ndap = RTA_NEXT(ndap, ndl)) {
          if (ndap->rta_type == NDA_DST) {
            addr = RTA_DATA(ndap);
            if ((ndp->ndm_family == AF_INET6 && (IN6_IS_ADDR_LINKLOCAL(addr) || IN6_IS_ADDR_SITELOCAL_NEW(addr) || IN6_IS_ADDR_UNSPECIFIED(addr)))
              || (ndp->ndm_family == AF_INET && (((in_addr*)addr)->s_addr == INADDR_ANY || ((in_addr*)addr)->s_addr == INADDR_BROADCAST))) {
              break;
            }
            //char addrstr[INET6_ADDRSTRLEN];
            //DBG("ntlink notify, ip:%s, state:%d", inet_ntop(ndp->ndm_family, addr, addrstr, INET6_ADDRSTRLEN), ndp->ndm_state);
          } else if (ndap->rta_type == NDA_LLADDR) {
            mac = (char*)RTA_DATA(ndap);
          /*  clt = Client::getbymac((char*)RTA_DATA(ndap));
            if (clt == nullptr) {
              break;
            }*/
          }

          if (addr != nullptr && mac != nullptr) {
            in6_addr v4compat;
            if (ndp->ndm_family == AF_INET) {
              memset(&v4compat, 0, sizeof(in6_addr));
              v4compat.s6_addr32[3] = ((in_addr*)addr)->s_addr;
              addr = &v4compat;
            }

            auto it = g_attr.addrpool->addrcache.find(*(NeighEntryKey*)addr);
            if (nh->nlmsg_type == RTM_NEWNEIGH) {
              NeighEntry* entry = it->second;
              if (it == g_attr.addrpool->addrcache.end()) {
                if (ndp->ndm_state == NUD_REACHABLE) {
                  entry = new NeighEntry((in6_addr*)addr, mac);
                  g_attr.addrpool->addrcache.insert(std::map<NeighEntryKey, NeighEntry*>::value_type(entry->xaddr, entry));
                }
              } else {
                memcpy(entry->mac, mac, IFHWADDRLEN);
                entry->lastchangetime = time(0);
                entry->state = ndp->ndm_state;
              }
            } else {
              if (it != g_attr.addrpool->addrcache.end()) {
                g_attr.addrpool->addrcache.erase(it);
                delete it->second;
              }
            }

            break;
          }

          /*
          if (clt != nullptr && addr != nullptr) {
            //char addrstr[INET6_ADDRSTRLEN];
            //DBG("ntlink notify, ip:%s, state:%d", inet_ntop(ndp->ndm_family, addr, addrstr, INET6_ADDRSTRLEN), ndp->ndm_state);

            clt->proNeighChange(ndp->ndm_family, addr, nh->nlmsg_type);
            break;
          }*/
        }
      //}
    }
  }

  free(rcvbuf);
  return 0;
}
#endif

int Timer::proInnerTimer(SockEx* esock, StarTlv& tlvs) {
#ifndef _WINSOCKAPI_
  //https://en.cppreference.com/w/cpp/container/map/erase
  for (auto it = g_attr.addrpool->addrcache.begin(); it != g_attr.addrpool->addrcache.end();) {
    NeighEntry* addr = it->second;
    if (addr->state == NUD_FAILED) {
      it = g_attr.addrpool->addrcache.erase(it);
      delete addr;
      continue;
    }

    /* 从实测情况看，邻居可能很长时间都停留在NUD_STALE状态，所以必须发包检测
    可优化点：把这个函数转移到AddrPool/NeighEntry里面；然后确定是clt正在使用的IP地址再发包检测
    */
    time_t now = time(0);
    if (addr->state != NUD_REACHABLE && (now - addr->lastchangetime) > 2 * 60) {
      sockaddr_in6 addr6;
      sockaddr_in addr4;
      sockaddr* xaddr;
      socklen_t len;
      if (IN6_IS_ADDR_V4COMPAT(addr)) {
        addr4.sin_family = AF_INET;
        addr4.sin_port = 0xFFFF;
        addr4.sin_addr.s_addr = addr->xaddr.xaddr.s6_addr32[3];
        xaddr = (sockaddr*)&addr4;
        len = sizeof(addr4);
      } else {
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = 0xFFFF;
        memcpy(&addr6.sin6_addr, &addr->xaddr.xaddr, sizeof(in6_addr));
        xaddr = (sockaddr*)&addr6;
        len = sizeof(addr6);
      }
      SOCKET sock = socket(xaddr->sa_family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
      len = sendto(sock, "", 0, MSG_NOSIGNAL, xaddr, len);
      closesocket(sock);

      //char addrstr[INET6_ADDRSTRLEN];
      //DBG("send ageing test:%s, state:%d, send len:%d, err:%s", inet_ntop(xaddr->sa_family, &addr->xaddr.xaddr, addrstr, INET6_ADDRSTRLEN), 
      //  addr->state, len, strerror(errno));

      addr->state = NUD_FAILED; //FAILED状态，下一个周期就会老化掉
    }

    ++it;
  }

  //必须放到最后面，因为可能导致多线程
  if (IN6_IS_ADDR_UNSPECIFIED(&g_attr.bestip6)) {
    sockthread::wait(0);

    struct hostent* ht = gethostbyname2("ipv6.tsinghua.edu.cn", AF_INET6);
    if (ht != nullptr) {
      SOCKET sock = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, 0);
      sockaddr_in6 addr6;
      socklen_t len = sizeof(addr6);
      addr6.sin6_family = AF_INET6;
      memcpy(&addr6.sin6_addr, (in6_addr*)ht->h_addr, sizeof(in6_addr));
      addr6.sin6_port = htons(80);
      if (connect(sock, (sockaddr*)&addr6, sizeof(sockaddr_in6)) == 0) {
        getsockname(sock, (sockaddr*)&addr6, &len);
        memcpy(&g_attr.bestip6, &addr6.sin6_addr, sizeof(in6_addr));
        DBG("get localipv6 ok: ");
      }
      closesocket(sock);

      char addrstr[INET6_ADDRSTRLEN];
      DBG("my ipv6:%s, errno:%s", inet_ntop(AF_INET6, &addr6.sin6_addr, addrstr, INET6_ADDRSTRLEN), strerror(errno));
      
    }
  }

  alarm(60);
#endif

  return 0;
}

#ifndef _WINSOCKAPI_
NeighEntry::NeighEntry(in6_addr* addr, char* mac) {
  memcpy(&xaddr.xaddr, addr, sizeof(in6_addr));
  memcpy(this->mac, mac, IFHWADDRLEN);
  this->lastchangetime = time(0);
  this->state = NUD_REACHABLE;

  Client* clt = Client::getbymac(mac);
  if (clt != nullptr) {
    int family = AF_INET6;
    void* addr = &xaddr;
    if (IN6_IS_ADDR_V4COMPAT(&xaddr)) {
      family = AF_INET;
      addr = &xaddr.xaddr.s6_addr32[3];
    }
    clt->proNeighChange(family, addr, RTM_NEWNEIGH);
  }

  return;
}

NeighEntry::~NeighEntry() {
  Client* clt = Client::getbymac(mac);
  if (clt != nullptr) {
    int family = AF_INET6;
    void* addr = &xaddr;
    //int check = memcmp(addr, &clt->activeip6, sizeof(in6_addr));
    
    if (IN6_IS_ADDR_V4COMPAT(&xaddr)) {
      family = AF_INET;
      addr = &xaddr.xaddr.s6_addr32[3];
      //check = memcmp(addr, &clt->activeip, sizeof(in_addr));
    }

    clt->proNeighChange(family, addr, RTM_DELNEIGH);
    /* reselect an active addr in pool then call proNeighChange
    if (check == 0) {
     
    }*/
  }

  return;
}
#endif

json_object* getdhcpclients() {
  json_object* clts = json_object_new_array();
#ifdef _ROUTER_
  FILE* leasestream = fopen("/tmp/dhcp.leases", "r");
  if (leasestream != nullptr) {
    char* line = nullptr;
    size_t len = 0;
    while (getline(&line, &len, leasestream) != -1) {
      char name[NBS_CLIENTNAME_SIZE];
      char macstr[24];
      char dhcp_buff[256];
      if (sscanf(line, "%255s %255s %64s %255s %764s", dhcp_buff, macstr, dhcp_buff, name, dhcp_buff) == 5) {
        json_object* clt = json_object_new_object();
        json_object_object_add(clt, "mac", json_object_new_string(macstr));
        json_object_object_add(clt, "name", json_object_new_string(name));
        json_object_array_add(clts, clt);
      }

      len = 0;
      free(line);
      line = nullptr;
    }

    fclose(leasestream);
  }
#endif
  return clts;
}
