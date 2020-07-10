#include "star.h"
#include "service.h"
#include "cchnl.h"
#include "driveri.h"
#include "tlv.h"

service::service(Client* clt, char* host, char* name, UINT16 nport, UCHAR proto, intermode omode) {
  this->clt = clt;
  strcpy_s(this->host, sizeof(this->host), host);
  strcpy_s(type, sizeof(type), name);
  intraport = nport;
  mode = omode;
  fwdChnl = nullptr;
  list_add_tail(&entry, &clt->allsvc);
  //interport = 0;
  this->protocol = proto;

	// 所有不是本机的service都分配中转port
	//transitport = 0;
	//if (memcmp(clt->mac, g_attr.mac, IFHWADDRLEN) != 0) {
  transitport = driveri::getAvailablePort(DRIVERI_SVCTRANSIT, htonl(INADDR_LOOPBACK), intraport);
	//}
}

service::~service() {
	list_del(&entry);
	service::notinuse();

	if (transitport != 0) {
		NBS_CREATESOCKADDR(addr, 0, transitport);
		driveri::updateFwdInfo((SOCKADDR*)&addr, DRIVERI_NORMAL);
	}
	/*
	  if (fwdChnl != nullptr) { // the service using the fwdchnl
		StarTlv msg(StarTlv::RELEASEPUBLICADDR);
		NBS_CREATESOCKADDR(interaddr, interip, interport);
		msg.pack_atom(StarTlv::RELEASEPUBLICADDR_INTERADDR, sizeof(SOCKADDR_IN), (char*)&interaddr);
		send(fwdChnl->sock, msg.get_final(), msg.total, MSG_NOSIGNAL);

		if (!((ChnlTCP*)fwdChnl)->notUsing()) {
		  return;
		}

		delete fwdChnl;
	  }*/
}
/*
int service::RequestPublicAddr(void* host) {
 
  StarTlv request(StarTlv::REQUESTPUBLICADDR);
  sockthread::pack_synheader(&request);
  
  NBS_CREATESOCKADDR(intraAddr, g_attr.bestip, (transitport == 0) ? intraport : transitport);
  request.pack_atom(StarTlv::REQUESTPUBLICADDR_INTRAADDR, sizeof(SOCKADDR_IN), (char*)&intraAddr);

	SOCKADDR_IN ctrladdr;
	socklen_t len = sizeof(SOCKADDR_IN);
	getpeername(g_cchnl->chnl->sock, (SOCKADDR*)&ctrladdr, &len);
	request.pack_atom(StarTlv::REQUESTPUBLICADDR_CTRLIP, sizeof(UINT32), (char*)&ctrladdr.sin_addr.s_addr);

	//  interAddr.sin_addr.s_addr = htonl(0x7F000001); //avoid request public addr multi-time, because wait will new a thread, if the new thread receive the same request 
	send(g_cchnl->chnl->sock, request.get_final(), request.total, MSG_NOSIGNAL);
	char* msg = sockthread::wait(4);
	if (msg == nullptr) {
		DBG("rrequest nil");
		return -1;
	}

	StarTlv rrtlv(msg);
	// confirm local service is exist, because may other threads delete the service
	//SOCKADDR_IN* cltaddr = (SOCKADDR_IN*)rrtlv.get_tlv(StarTlv::RREQUESTPUBLICADDR_INTRAADDR);
	//if (g_cchnl == nullptr || g_cchnl->getService(cltaddr->sin_addr.s_addr, cltaddr->sin_port) != this) {
	  //dbgOut("gchnel or service change:chnl:%p", g_cchnl);
	  //return -1;
	//}
	//20190415: if no multi-thread protection may cause exception; but consider the service delete is not frequence, to simplify the process, we just suppose ok
	//and if exception happend, just restart the service
	

	SOCKADDR_IN* addr = (SOCKADDR_IN*)rrtlv.get_tlv(StarTlv::RREQUESTPUBLICADDR_INTERADDR);
	if (addr == nullptr) {
		DBG("rrpublicaddr nil");
		return -1;
	}

	fwdChnl = g_cchnl->chnl;
	interip = addr->sin_addr.s_addr;
	interport = addr->sin_port;

	DBG("INTERADDR:0x%08x:%d, transitport:%d", interip, interport, transitport);

	if (transitport == 0) {
		driveri::updateFwdInfo((SOCKADDR*)&intraAddr, DRIVERI_SVCEND, (SOCKADDR*)addr, fwdChnl);
	}
	else {
		updateTransitInfo();
	}

	return 0;
}
*/



int service::getPublicAddr(void* host, SOCKADDR_IN* addr) {
	/* TODO: 如果不是公网地址场景，调用driveri::getPublicAddr获取地址 */
	int rtn = 0;
	if (g_attr.netType == NBS_NETTYPE::DYNGLOBAL) {
		addr->sin_addr.s_addr = g_cchnl->getbestip();//获取的公网ip

		addr->sin_port = transitport;
		/*
		if (transitport != 0) {
			interport = transitport;
			updateTransitInfo(AF_INET);
		}
		else {
			interport = intraport;
			xaddr.sin_port = interport;
			//addr->sin_addr.s_addr = interip;
			//addr->sin_port = interport;
			driveri::updateFwdInfo((SOCKADDR*)&xaddr, DRIVERI_SVCEND);
		}
		*/
	}
	else { //symmetric
//rtn = RequestPublicAddr(host);
		NBS_CREATESOCKADDR(intraAddr, g_attr.bestip, intraport);
		driveri::getPublicAddrr(transitport, g_cchnl->chnl, &intraAddr, addr);
	}


	//addr->sin_addr.s_addr = interip;
	//addr->sin_port = interport;
	return rtn;
}



int service::updateTransitInfo(int family) {
	DBG("service::updateTransitInfo, family:%d", family);

	if (transitport == 0) {
		DBG("service::updateTransitInfo, no transitsock");
		return 0;
	}

	if (clt->activeip.s_addr == 0 && family != AF_INET6) {
		//clt->getActiveip(AF_INET);
		DBG("service::updateTransitInfo, getActiveip AF_INET");
	}
	if (family != AF_INET && IN6_IS_ADDR_UNSPECIFIED(&clt->activeip6)) {
		//clt->getActiveip(AF_INET6);
		DBG("service::updateTransitInfo, AF_INET6, no addr");
	}

	if (family != AF_INET6 && g_attr.bestip != 0) {
		NBS_CREATESOCKADDR(addr, g_attr.bestip, transitport);
		NBS_CREATESOCKADDR(downaddr, clt->activeip.s_addr, intraport);
		NBS_CREATESOCKADDR(upaddr, interip, transitport);
		if (clt->activeip.s_addr == 0) {
			driveri::updateFwdInfo((SOCKADDR*)&addr, DRIVERI_NORMAL);
		}
		else {
			driveri::updateFwdInfo((SOCKADDR*)&addr, DRIVERI_SVCTRANSIT, (SOCKADDR*)&downaddr, fwdChnl, &upaddr);
		}
	}

	if (family != AF_INET && !IN6_IS_ADDR_UNSPECIFIED(&g_attr.bestip6)) {
		sockaddr_in6 addr;
		sockaddr_in6 rmtaddr;
		addr.sin6_family = AF_INET6;
		rmtaddr.sin6_family = AF_INET6;
		memcpy(&addr.sin6_addr, &g_attr.bestip6, sizeof(in6_addr));
		memcpy(&rmtaddr.sin6_addr, &clt->activeip6, sizeof(in6_addr));
		addr.sin6_port = transitport;
		rmtaddr.sin6_port = intraport;

		if (IN6_IS_ADDR_UNSPECIFIED(&clt->activeip6)) {
			driveri::updateFwdInfo((SOCKADDR*)&addr, DRIVERI_NORMAL);
		}
		else {
			driveri::updateFwdInfo((SOCKADDR*)&addr, DRIVERI_SVCTRANSIT, (SOCKADDR*)&rmtaddr);
		}
	}

	return 0;
}


int service::notinuse() {
  
  NBS_CREATESOCKADDR(interAddr, 0,0);
  if (fwdChnl != nullptr) {
    StarTlv msg(StarTlv::RELEASEPUBLICADDR);
    msg.pack_atom(StarTlv::RELEASEPUBLICADDR_INTERADDR, sizeof(interAddr), (char*)&interAddr);
    send(fwdChnl->sock, msg.get_final(), msg.total, MSG_NOSIGNAL);

		ChnlTCP* usingChnl = (ChnlTCP*)fwdChnl;
		fwdChnl = nullptr;
		if (usingChnl->notUsing()) {
			delete usingChnl;
		}
	}

  if (transitport != 0) {
    NBS_CREATESOCKADDR(intraAddr, 0, transitport);
    driveri::updateFwdInfo((SOCKADDR*)&intraAddr, DRIVERI_NORMAL);
  } else {
    driveri::updateFwdInfo((SOCKADDR*)&interAddr, DRIVERI_NORMAL);
  }
  
  return 0;
}

service* service::get(char* domain, char* type) {
	list_head* entry;
	list_for_each(entry, &g_attr.clients) {
		Client* clt = container_of(entry, Client, entry);
		list_head* svcentry;
		list_for_each(svcentry, &clt->allsvc) {
			service* svc = container_of(svcentry, service, entry);
			if (strcmp(type, svc->type) == 0 && strcmp(domain, svc->host) == 0) {
				return svc;
			}
		}
	}

	return nullptr;
}

appsrv::appsrv(Client* clt, char* host, UINT16 nport) :service(clt, host, (char*)"appsrv", nport, IPPROTO_TCP) {
	SOCKET s = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
	sockaddr_in6 addr6;
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_addr = IN6ADDR_ANY_INIT;
	addr6.sin6_port = nport;

#ifdef _WINSOCKAPI_
  //srv = new SockExTCP(s, (SOCKADDR*)&addr6, sizeof(addr6), true);
#else
	srv = new SockExListen<AppSrvEsock>(s, (SOCKADDR*)&addr6, sizeof(addr6));
#endif
};

appsrv::~appsrv() {
	delete srv;
}