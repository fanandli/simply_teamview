#include <memory.h>
#include <vector>
#include <map>
#include "base.h"
#include "json-c/json.h"
#include "unidef.h"
#include "sockex.h"
#include "driver_inner.h"
#include "tlv.h"
#include "driveri.h"
#include "mswsock.h"

struct IpPort {//里面存申请到的远端的ip+port
	struct in_addr ip;
	UINT16 port = 0;
};

struct IpPort IpPortt[65540];//这里的下标是本地port

map<SOCKET, selfServiceConnect*>SoftFwder::mapserandpc;

//全局变量在同一个文件中定义，按照先后顺序初始化；全局变量在运行前初始化；所以driveri部分全部定义成全局变量是可行的
driveri::driveri() {
	/* 转发面和控制面线程分离，实现转发面的completeio和转发面线程实现 */
	tlvcbs[StarTlv::RGETDATACHNLADDR] = TlvSynReplyCb;
	tlvcbs[StarTlv::DRV_CONNECTION] = SoftFwder::proDrvConnect;
	tlvcbs[StarTlv::DRV_DATA] = SoftFwder::proDrvData;
}

/* TODO: 暂时可能不需要实现 */
UINT16 driveri::updateFwdInfo(SOCKADDR* addr, UINT32 type, SOCKADDR* downAddr, SockEx* xtunnel, SOCKADDR_IN* upAddr) {
	return 0;
}

driveri::~driveri() {

}

/*
https://www.cnblogs.com/lidabo/p/5344899.html
get the default gateway, then get the ip address on the interface
*/
/* TODO: 暂时不需要实现 */
UINT32 driveri::GetGateWayAndIfIP(char* ifname, UINT32* sip) {
	return 0;
}
/* TODO: 暂时不需要实现 */
void driveri::getIfMac(char* ifName, char* mac) {
	
	/*mac[0] = 0xB4;
	mac[1] = 0x2E;
	mac[2] = 0x99;
	mac[3] = 0xE2;
	mac[4] = 0x7B;
	mac[5] = 0x46;
    */

}

/* TODO: 暂时不需要实现 */
UINT32 driveri::getIfIP(char* ifname) {
	return 0;
}

/* TODO: 调试ipv6时实现 */
//只是为了构建一个五元组，给运营商
//src是客户端的ip+port
//dst是自己的ip+port
extern LPFN_CONNECTEX lpfnConnectEx;

//此函数主要是用来向运营商发一个syn，为了就是让运营商有我们的五元组（客户端+目标主机的五元组）
void driveri::sendtcp6(sockaddr_in6* src, sockaddr_in6* dst) {
	SockExTCP* s = new SockExTCP();
	int bindres = ::bind(s->sock, (const sockaddr*)src, sizeof(sockaddr_in6));

	dst->sin6_family = AF_INET6;
	SockExOL* ov = new SockExOL(s, SockExOL::CONNECT);
	lpfnConnectEx(s->sock, (sockaddr*)dst, sizeof(SOCKADDR_IN), NULL, 0, 0, &ov->overlapped);
}

/* TODO: 根据intraport或transitport获取对应的公网地址
port:transitport或intraport
ctrlchnl:控制通道
intraddr:入参，intraaddr
publicaddr:出参，公网地址
*/
//shared_ptr<SockExTCP> dataptr = NULL;
SockExTCP* dataptr = NULL;

int driveri::getPublicAddrr(UINT16 port, SockEx* ctrlchnl, SOCKADDR_IN* intraaddr, SOCKADDR_IN* publicaddr, NBSDRV_FWDMODE mode) {
	//根据port查找是否已经申请公网地址
	//如果已经申请，则直接返回
	//如果没有申请，看是否有数据通道；如果不存在数据通道，则从控制通道发送数据通道申请消息
	//通过数据通道发送公网地址申请消息
	//存储port/内网地址/公网地址对应关系
	StarTlv request(StarTlv::GETDATACHNLADDR);//初始化一个tlv
	StarTlv requesttwo(StarTlv::REQUESTPUBLICADDR);
	sockthread::pack_synheader(&request);

	SOCKADDR_IN ctrladdr;
	socklen_t len = sizeof(SOCKADDR_IN);

	if (IpPortt[port].port) {//直接返回公网地址
		publicaddr->sin_addr = IpPortt[port].ip;
		publicaddr->sin_port = IpPortt[port].port;
	}
	else {//向服务器申请公网ip
		if (dataptr) {//如果数据通道存在，就从数据通道申请
			request.pack_atom(StarTlv::REQUESTPUBLICADDR_INTRAADDR, sizeof(SOCKADDR_IN), (char*)intraaddr);
			request.pack_atom(StarTlv::REQUESTPUBLICADDR_CTRLIP, sizeof(UINT32), (char*)&ctrladdr.sin_addr.s_addr);
			send(dataptr->sock, requesttwo.get_final(), requesttwo.total, MSG_NOSIGNAL);//通过数据通道发送申请
		}
		else {//数据通道不存在，就走控制通道申请一个数据通道，然后再从数据通道发送申请
			//shared_ptr<SockExTCP> SET(new SockExTCP());
			SockExTCP* SET = new SockExTCP();//
		   //request.pack_atom(StarTlv::REQUESTPUBLICADDR_CTRLIP, sizeof(UINT32), (char*)&ctrladdr.sin_addr.s_addr);//将ctrladdr加到request中
			send(ctrlchnl->sock, request.get_final(), request.total, MSG_NOSIGNAL);//发送这个
			char* msg = sockthread::wait(4);
			if (msg == nullptr) {
				DBG("rrequest nil");
				return -1;
			}
			StarTlv rrtlv(msg);
			SOCKADDR_IN* addr = (SOCKADDR_IN*)rrtlv.get_tlv(StarTlv::RGETDATACHNLADDR_ADDR);//get_tlv函数是用来干嘛的？
			addr->sin_family = AF_INET;
			sockthread::pack_synheader(&requesttwo);
			requesttwo.pack_atom(StarTlv::REQUESTPUBLICADDR_INTRAADDR, sizeof(SOCKADDR_IN), (char*)intraaddr);
			requesttwo.pack_atom(StarTlv::REQUESTPUBLICADDR_CTRLIP, sizeof(UINT32), (char*)&ctrladdr.sin_addr.s_addr);
			NBS_CREATESOCKADDR(localAddr,0, 0);
			bind(SET->sock, (SOCKADDR*)&localAddr, sizeof(SOCKADDR_IN));//
			SET->ConnectEx((struct sockaddr*)addr, requesttwo.get_final(), requesttwo.total);//建立数据通道，
			//cout << addr;
			char* msgtwo = sockthread::wait(4);
			if (msgtwo == nullptr) {
				DBG("rrequest nil");
				return -1;
			}
			StarTlv rrrtlv(msgtwo);

			SOCKADDR_IN* addrtwo = (SOCKADDR_IN*)rrrtlv.get_tlv(StarTlv::RREQUESTPUBLICADDR_INTERADDR);
			
			publicaddr->sin_addr = addrtwo->sin_addr;
			publicaddr->sin_port = addrtwo->sin_port;

			//存储ip + port
			//port是本地的
			//sin_port是远端的
			//port = addrtwo->sin_port;
			//IpPortt[i].ip = addrtwo->sin_addr;
			IpPortt[port].ip = addrtwo->sin_addr;
			IpPortt[port].port = addrtwo->sin_port;
			dataptr = SET;
		}
	}
	return 0;
}

class TransChnl;

class ClitoTargethostIp6 :public SockExTCP {
public:
	//ChnlTCP(SOCKADDR_IN* peeraddr, char* token);
	TransChnl* ToTransChnlBuf;
	ClitoTargethostIp6(TransChnl* a) {
		ToTransChnlBuf = a;
	}
	~ClitoTargethostIp6();

	virtual int onConnect(bool bConnect);
	//bool notUsing();
	virtual int onRcv(int len);
	//static void FwdtoDataChnl(SockEx* rcvsock, SOCKET datachnl, char peerFwdId, char fwdidlen, char* rcvbuf);
	UCHAR databuf[databufFERLEN];//存的是3389发过来的
	//int RcvEx(UCHAR* rcvbuf, int n);
	// static map<SOCKET, SOCKET>mapTransAndSer;
};



struct TransInfo_s {
	SockExTCP* lsn;
	UINT32 ip;
	UINT16 port;
}TransInfo[65535];

//map<SOCKET, SOCKET>mapTransAndSer;
//SockExTCP* cltChnlPtr = NULL;
class TransChnl:public SockExTCP {//用来处理客户端发过来的连接（与客户端发数据的连接）
public:
	 //map<SOCKET,SOCKET>mapTransAndSer;
	 UCHAR databuf[databufFERLEN];//存的是客户端要发给3389的内容
	//SockEx* Transsock = NULL;
	 ClitoTargethostIp6* clitoPtr = NULL;

	SockExTCP* newAcceptSock(SockExTCP* srv) {//?
		TransChnl* esock = new TransChnl();
		cout << "TransChnl sock: " << esock->sock << endl;
		esock->srv = srv;
		return esock;
	}

	~TransChnl() {
		DBG("~TransChnl, sock:%d", sock);
		if (clitoPtr) {
			clitoPtr->ToTransChnlBuf = nullptr;
			delete clitoPtr;
		}
	}

	int onConnect(bool bConn) {//这个函数
		//处理客户端与我建立的连接
		//通过transInfo获取本地的端口和IP并建立连接
		//建立socket之间的映射关系
		//这里的onConnect主要是与3389建立连接
		//setsockopt(sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&srv->sock, sizeof(srv->sock)); //MUST set this option, orelse cannot use SHUTDOWN, getpeername ...
		UINT16 port = srv->getPort(AF_INET6);//这个获取到的是transitport
		//shared_ptr<ClitoTargethostIp6> transChnlPtr (new ClitoTargethostIp6(this));
		ClitoTargethostIp6* transChnlPtr = NULL;
		transChnlPtr = new ClitoTargethostIp6(this);//这里的this是指向TransChnl的实例的指针
		cout << "TransChnl onConnect transChnlPtr " << transChnlPtr->sock << endl;
		//Transsock = transChnlPtr;
		 
		sockaddr_in sockLocal;
		sockLocal.sin_port = TransInfo[port].port;//获取到要访问的服务的ip+port
		sockLocal.sin_addr.s_addr = TransInfo[port].ip;  //这个类型转换？
		cout << "-----" << port << " " << TransInfo[port].ip << " " << TransInfo[port].port << endl;
		sockLocal.sin_family = AF_INET;

		NBS_CREATESOCKADDR(localAddr, 0,0);
		bind(transChnlPtr->sock, (SOCKADDR*)&localAddr, sizeof(SOCKADDR_IN));

		transChnlPtr->ConnectEx((struct sockaddr*)&sockLocal, NULL, 0);

		//然后将这个sock与port建立对应关系 --->这个对应关系要传给谁吗？
		//mapserandpc[SET->sock] = SET;
		//mapTransAndSer[transChnlPtr->sock] = sock;//下标是与3389连接的sock，sock是客户端发的sock
		//RcvEx(databuf);
		clitoPtr = transChnlPtr;
		cout << "TransChnl sock: " << sock << " " << this << endl;

		return 0;
	}
	int onRcv(int n) {//这个是处理客户端发来的数据,发给目标主机中的目的端口
		//发到本地sockex
		//转发
		//send(sock, payload, tlvs.get_len(payload), MSG_NOSIGNAL);//发送给服务port
		 // sizeof(_tlv)*3 + 1 subtlv end with 0 + fwdidlen
		//_tlv* msg = (_tlv*)databuf;
		//msg->type = StarTlv::DRV_DATA;
		//UINT16 port = getPort();
		cout << "TransChnl onRcv sock: " << sock <<" "<< this << endl;
		cout << "TransChnl::onRcv send : " << clitoPtr << endl;
		int sendreturn = send(clitoPtr->sock, (const char*)databuf, n, MSG_NOSIGNAL);//将databufsend出去
		
		wprintf(L"send failed with error: %d\n", WSAGetLastError());
		if (sendreturn == -1) {
			exit(0);
		}
		RcvEx(databuf, sizeof(databuf), 0);//用来告诉操作系统将收到的数据写到databuf里去
		return 0;
	}
};

ClitoTargethostIp6::~ClitoTargethostIp6() {
	if (ToTransChnlBuf) {
		ToTransChnlBuf->clitoPtr = nullptr;
		delete ToTransChnlBuf;
	}
}

int ClitoTargethostIp6::onRcv(int n) {//向客户端发送数据
	//cout << "ClitoTargethostIp6::onRcv: " << this << endl;
	send(ToTransChnlBuf->sock, (const char*)databuf, n, MSG_NOSIGNAL);
	RcvEx(databuf, sizeof(databuf), 0);
	return 0;
}

int ClitoTargethostIp6::onConnect(bool bConn) {
	cout << "ClitoTargethostIp6::onConnect: " << this << endl;
	//cout << bConn << endl;
	RcvEx(databuf,sizeof(databuf), 0);//将3389发出来的告诉系统写到这里
	ToTransChnlBuf->RcvEx(ToTransChnlBuf->databuf,sizeof(ToTransChnlBuf->databuf), 0);
	//RcvEx();
	return 0;
}


//int ClitoTargethostIp6::onRcv(int n){
	//int sendreturn = send(mapTransAndSer[sock], (const char*)databuf, n, MSG_NOSIGNAL);

//}

//分配transitport
UINT16 driveri::getAvailablePort(NBSDRV_PORTTYPE type, UINT32 peerip, UINT16 peerport, SockEx* tnl) {
	SOCKET s = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
	sockaddr_in6 addr6;
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;

	TransChnl* acceptsock = new TransChnl();//这个是如果来了socket，就会调用这个socket]
	cout << "driveri::getAvailablePort accept sock:" << acceptsock->sock << endl;
	acceptsock->srv = new SockExTCP(s, (SOCKADDR*)&addr6, sizeof(addr6), true);//生成一个listen 的socket
	//这个里面的bind就会随机分一个port，作为transitport
	SockExTCP::AcceptEx(acceptsock);//侦听
	UINT16 port = acceptsock->srv->getPort(AF_INET6);//这里拿的是transitport

	if (type == DRIVERI_SVCTRANSIT) {
		TransInfo[port].ip = peerip;//这里存的是service的port
		TransInfo[port].port = peerport;//这个结构体里的port是要访问的port，下标port是我们的transitport
		TransInfo[port].lsn = acceptsock->srv;//是指向socket的指针
	}

	DBG("transitport:%d", ntohs(port));
	return port;
}

/* TODO: 实现一些转发函数
收到远端发来的新连接建立消息
收到远端发来的连接中断消息
收到本端&远端的数据报文消息
*/

//服务器收到客户端的消息后，服务器会向目标主机发送“连接建立消息”
//我们自己目标主机要接收这个消息，从中拿出客户端向服务器发出的socket的id
//然后去建立一个连接，连到（比如）3389服务端口，然后将这个连接的socketid与上面那个socketid
//建立对应关系，然后再将这个对应关系，返回给服务器。

//服务器如果收到了客户端发送的“终端连接消息”的话
//向目标主机发送消息，目标主机停止相应的与服务的连接

//收到数据报文，通过查询建立的对应关系，就转发给对应的socket（服务端口）

int SoftFwder::proDrvConnect(SockEx* esock, StarTlv& tlvs) {
	//接收到的消息
	char type = *tlvs.get_tlv(StarTlv::DRV_CONNECTION_TYPE);//

	//char* savetlv = tlvs.get_tlv(StarTlv::DRV_CONNECTION_LOCALFWDID);//存一下发给我tlv信息,这个类型只有1的时候才有
	//远端的forwdid

	//SockExTCP* SET = 0;

	//建立与服务的连接
	//SockExTCP* SET = new selfserviceconnect();
	
	if (type == 1) {//新建一个socket
		char* savetlv = tlvs.get_tlv(StarTlv::DRV_CONNECTION_LOCALFWDID);//这个是服务器发给我的（远端）forwdid
		int tlvlen = tlvs.get_len(savetlv);
		
		selfServiceConnect* SET = new selfServiceConnect();//SET里有本地连接的sockid
		SOCKADDR_IN* addr = (SOCKADDR_IN*)tlvs.get_tlv(StarTlv::DRV_CONNECTION_ADDR);//get_tlv函数是用来干嘛的？
		addr->sin_family = AF_INET;
		NBS_CREATESOCKADDR(localAddr, 0, 0);
		bind(SET->sock, (SOCKADDR*)&localAddr, sizeof(SOCKADDR_IN));
		SET->ConnectEx((struct sockaddr*)addr, NULL, NULL);//建立与port连接

		SET->fwdidlen = tlvlen;
		mapserandpc[SET->sock] = SET;
		memcpy(SET->peerFwdId, savetlv, tlvlen);//peerFwdId里是远端的socketid
		

	}else if(type == 0) {//0,说明服务端的连接断掉了，我们本地的相应的socket也要断掉
		cout << "-----prodrvconnect else if type == 0-----" << endl;
		//SET->ConnectEx((struct sockaddr*)addr, NULL, NULL);//建立数据通道，
		//存储socket id的对应关系
		//SoftFwder::mapserandpc[SET->sock] = savetlv;
		//mapserandpc[SET->sock] = savetlv;
		//delete SET;
		int sock = *(SOCKET*)tlvs.get_tlv(StarTlv::DRV_CONNECTION_RMTFWDID);
		//char* savetlv = tlvs.get_tlv(StarTlv::DRV_CONNECTION_RMTFWDID);//
		//closesocket(localfwdid->s); //触发本sock收到len=0报文用于删除sockex；不在这边删除sockex是为了避免多线程同时删除可能导致的异常
		delete mapserandpc[sock];//m
	}
	//将收到的消息加上socketid的对应关系，然后发送给服务器，通过数据通道？->在onconnect逻辑里
	return 0;
}
int selfServiceConnect::RcvEx(UCHAR* rcvbuf) {
	DWORD flags = 0; //must initial equal 0, orelse, no message rcv by workthread
	SockExOL* ov = new SockExOL(this, SockExOL::RCV, (char*)rcvbuf+ 12 + 1 + fwdidlen, databufFERLEN - 12 - 1 - fwdidlen);
	int rtn = WSARecv(sock, &ov->wsabuffer, 1, NULL, &flags, &ov->overlapped, NULL);
	//DBG("selfserviceconnect recv: sock: %d, %d, %d", sock, rtn, WSAGetLastError());
	return 0;
}

int selfServiceConnect::onConnect(bool bConnect) {
	int type = 0;
	if (!bConnect) {//如果与比如3389建立连接不成功
		//this->SockExTCP::onConnect(bConnect);
		StarTlv request(StarTlv::GETDATACHNLADDR);
		request.pack_atom(StarTlv::DRV_CONNECTION_TYPE, sizeof(int), (char*)&type);
		request.pack_atom(StarTlv::DRV_CONNECTION_RMTFWDID, fwdidlen, peerFwdId);
		send(dataptr->sock,request.get_final(),request.total, MSG_NOSIGNAL);
		delete this;
		
	}else {//连接成功
		StarTlv request(StarTlv::DRV_CONNECTION);
		int two = 2;
		request.pack_atom(StarTlv::DRV_CONNECTION_TYPE, sizeof(two),(char*)&two);
		request.pack_atom(StarTlv::DRV_CONNECTION_RMTFWDID, fwdidlen, peerFwdId);
		request.pack_atom(StarTlv::DRV_CONNECTION_LOCALFWDID, sizeof(sock), (char*)&sock);
		send(dataptr->sock, request.get_final(), request.total, MSG_NOSIGNAL);
	}

	RcvEx(databuf);//收比如3389发送过来的数据
	return 0;
}


int SoftFwder::proDrvData(SockEx* esock, StarTlv& tlvs) {//这个是用来处理从数据通道收到的消息
	//_fwdid_* localfwdid = (_fwdid_*)tlvs.get_tlv(StarTlv::DRV_DATA_FWDID);//拿到id
	int sock = *(SOCKET*)tlvs.get_tlv(StarTlv::DRV_DATA_FWDID);
	char* payload = tlvs.get_tlv(StarTlv::DRV_DATA_PAYLOAD);//拿到data
	send(sock, payload, tlvs.get_len(payload), MSG_NOSIGNAL);//发送给服务port

	return 0;
}

int selfServiceConnect::onRcv(int len) {//接收port发来的
	//StarTlv request(StarTlv::DRV_DATA);
	//将tlv组上去
	//将payload组上去
	//发送给对应的端口？从哪里获取这个？用recv函数接收获取吗？
	//request.pack_atom(StarTlv::DRV_DATA_FWDID, sizeof(peerFwdId), (char*)&(peerFwdId));//组tlv
	//request.pack_atom(StarTlv::DRV_DATA_FWDID, sizeof(mapserandpc), (char*)&(mapserandpc));//组tlv
	//request.pack_atom(StarTlv::DRV_DATA_PAYLOAD, n, (char*)databuf);//组payload,payload在rcvbuf中

	int headlen = 12 + 1 + fwdidlen; // sizeof(_tlv)*3 + 1 subtlv end with 0 + fwdidlen
	_tlv* msg = (_tlv*)databuf;
	msg->type = StarTlv::DRV_DATA;

	//subtlv fwdid
	_tlv* subtlv = msg + 1;
	subtlv->type = StarTlv::DRV_DATA_FWDID;
	subtlv->len = htons(fwdidlen + 1);
	subtlv += 1;
	memcpy(subtlv, peerFwdId, fwdidlen);
	
	//subtlv payload
	subtlv = (_tlv*)((char*)subtlv + fwdidlen + 1);
	subtlv->type = StarTlv::DRV_DATA_PAYLOAD;
	subtlv->len = htons(len + 1);

	msg->len = htons(headlen - sizeof(_tlv) + len);//更新msg len

	int sendreturn = send(dataptr->sock, (char*)msg, len+headlen, MSG_NOSIGNAL);//发给数据通道
	//cout << "selfServiceConnect::onRcv send return: " << sendreturn << endl;
	RcvEx(databuf);//
	//procTlvMsg(rcvbuf, msglen);
	return 0;

	//update
	//直接将port接收来的数据，写入databuf中（空出一段内存头）然后
	//
}

