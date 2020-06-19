#include "sockex.h"
#include <ws2tcpip.h>
#include "Mswsock.h"
#include <exception>
#include <list>
#include <time.h>
#include "base.h"
#include "tlv.h"
using namespace std;

#define CONSTRUCT_ERROR  \
  closesocket(sock);\
  sock = INVALID_SOCKET;\
  throw std::exception("SockEx error");

HANDLE completePort;
//sockthread** threads = NULL; //这些线程是默认需要的线程，为了简单只创建，不析构；程序结束后按说应该自动析构了
//int g_threadsNum = 1; //MUST check

//PSLIST_HEADER tcpExlist;
LPFN_ACCEPTEX lpfnAcceptEx;
LPFN_DISCONNECTEX lpfnDisconnectEx;
LPFN_CONNECTEX lpfnConnectEx;
//list_head g_activeConeSock[SOCKEX_CONESOCK_KAINTERVEL];
//HANDLE coneSockTimer;
//int coneSockTimerPos = 0;
//socktype: considered three types: tcpserver, tcprelay, tcpclient, udpclient
// for supporting cocurrent tcp connections, tcprelay/tcpserver type using pools to store sockets;

// refrence to the articles: https://blog.csdn.net/piggyxp/article/details/6922277 and https://blog.csdn.net/crasyangel/article/details/40458123
SockEx::SockEx(SOCKET s) {
  if (s == INVALID_SOCKET) {
    return;
  }  
  this->sock = s;

  if (completePort == NULL) {
    //SYSTEM_INFO si;
    //GetSystemInfo(&si);
    //g_threadsNum = 2 * si.dwNumberOfProcessors;

    //threads = (sockthread**)malloc(sizeof(sockthread*) * g_threadsNum);

    completePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long)0, 0);
    if (completePort == NULL) {
      throw std::exception("pool init error");
    }

#if 0
    for (int i = 0; i < SOCKEX_CONESOCK_KAINTERVEL; i++) {
      INIT_LIST_HEAD(&g_activeConeSock[i]);
    }
    CreateTimerQueueTimer(&coneSockTimer, NULL, ConeSock::_KeepAlive, this, 1000, 1000, WT_EXECUTEDEFAULT);
#endif

    GUID GuidSockEx = WSAID_ACCEPTEX;
    DWORD dwBytes;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidSockEx, sizeof(GuidSockEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL) == SOCKET_ERROR) {
      CONSTRUCT_ERROR;
    }
    GuidSockEx = WSAID_CONNECTEX;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidSockEx, sizeof(GuidSockEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL) == SOCKET_ERROR) {
      CONSTRUCT_ERROR;
    }
    GuidSockEx = WSAID_DISCONNECTEX;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidSockEx, sizeof(GuidSockEx), &lpfnDisconnectEx, sizeof(lpfnDisconnectEx), &dwBytes, NULL, NULL) == SOCKET_ERROR) {
      CONSTRUCT_ERROR;
    }
  }

  // completePort = CreateIoCompletionPort((HANDLE)sock, completePort, (u_long)0, 0);
  CreateIoCompletionPort((HANDLE)sock, completePort, (u_long)0, 0); //if the sock is reuse, this function will return 0, errcode is 87; so only bind but not set return value to completePort
}

int SockExTCP::RcvEx(UCHAR* rcvbuf, int rcvmax) {
  DWORD flags = MSG_PEEK; //must initial equal 0, orelse, no message rcv by workthread
  SockExOL* ov = new SockExOL(this, SockExOL::RCV, (char*)rcvbuf, rcvmax);
  int rtn = WSARecv(sock, &ov->wsabuffer, 1, NULL, &flags, &ov->overlapped, NULL);
  DBG("wsarcvPEEK:sock:%d, %d, %d", sock, rtn, WSAGetLastError());
  return 0;
}

SockExTCP::SockExTCP(SOCKET s, SOCKADDR *saddr, int len, bool server) :SockEx(s){
  srv = NULL;

  if (s == INVALID_SOCKET) {
    sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    new ((SockExTCP*)this)SockExTCP(sock, saddr, len); //must use SockExTCP to renew, orelse call virtual deconstruct function, ~SockExTCP cannot be triggered
    return;
  }

  DWORD opt = TRUE;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

  if (saddr != NULL) {
    //if also include std namespace, strong recommend using ::bind, orelse the FUNCTION bind is not the winsock bind
    ::bind(sock, (sockaddr*)(saddr), len); // http://www.cnblogs.com/MRRAOBX/articles/3082997.html, must bind, orelse connectex occur 10022 error;

    if (server) {
      ::listen(sock, SOMAXCONN);
      SockExTCP::AcceptEx(this);
    }
  }
};

/* 调用connectex肯定是异步场景;
如果需要同步编程,可以用wait结合connect使用 */
int SockExTCP::ConnectEx(sockaddr *name, char* buffer, int len) {
  SockExOL* ov = new SockExOL(this, SockExOL::CONNECT);
  lpfnConnectEx(sock, name, sizeof(SOCKADDR_IN), buffer, len, 0, &ov->overlapped);
  return 0;
}

/*
如果是服务端sockextcp，估计要按照下面的步骤操作
SockExTCP* listen = new SockExTCP(INVALID_SOCKET, addr);
listen->acceptex(listen);
*/
int SockExTCP::AcceptEx(SockExTCP* lsn) {
  SockExTCP* tcp = new SockExTCP();
  tcp->srv = lsn;
  SockExOL* ov = new SockExOL(tcp, SockExOL::ACCEPT);
  extern LPFN_ACCEPTEX lpfnAcceptEx;
  int rtn = lpfnAcceptEx(lsn->sock, tcp->sock, &tcp->buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, &ov->overlapped); //suggest rcvdatalen equals 0, orelse may cause accecptex not return?
  return 0;
}

int SockExTCP::onConnect(bool bConnect) {
  if (!bConnect) {
    delete this;
    return 0;
  }

  if (srv == nullptr) {
    DWORD opt = true;
    setsockopt(sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, (char*)&opt, sizeof(opt)); //MUST set this option, orelse cannot use getpeername ....    
  } else {
    setsockopt(sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&srv->sock, sizeof(srv->sock)); //MUST set this option, orelse cannot use SHUTDOWN, getpeername ...
  }

  RcvEx(buf);
  return 0;
}

int SockExTCP::onRcv(int n) {
  cout << "sockextcp::onrcv " << n <<endl;
  if (n < sizeof(_tlv)) {
    RcvEx(buf);
    cout << "sockextcp::onrcv _tlv null" << endl;
    return 0;
  }

  int msglen = ntohs(((struct _tlv*)buf)->len) + sizeof(_tlv);
  cout << "sockextcp::msglen " << msglen << endl;
  char* rcvbuf = new char[msglen];
  if (recv(sock, rcvbuf, msglen, MSG_WAITALL) < 0) {
    DBG("wsarcvPEEK:sock:%d, %d", sock, WSAGetLastError());
    goto end;
  }

  RcvEx(buf);
  procTlvMsg(rcvbuf, msglen);
end:  
  delete[] rcvbuf;
  return 0;
};

int SockEx::procTlvMsg(char* data, int len) { //only reserved for starService.cpp
  struct _tlv* tlv = (struct _tlv*)data;

  cout << "sockex::protlvmsg type:" << (int)tlv->type;
  cout << ", sockex::protlvmsg len:" << ntohs(tlv->len)<<endl;
  while (len >= sizeof(struct _tlv)) {
    if (tlv->type < StarTlv::MIN || tlv->type > StarTlv::MAX) {
      return -1;
    }

    int tlvlen = ntohs(tlv->len);
    if (len < tlvlen + sizeof(struct _tlv)) {
      break;
    }

    if (tlvcbs[tlv->type] != 0) {
      StarTlv tlvs((char*)tlv);
      tlvcbs[tlv->type](this, tlvs);
    }

    len -= tlvlen + sizeof(struct _tlv);
    tlv = (struct _tlv*)((char*)tlv + tlvlen + sizeof(struct _tlv));
  }

  return len;
}

SockExTCP::~SockExTCP() {
  //onClose();
  //CancelIoEx((HANDLE)sock, nullptr);
  cout << "--------------sockextcp::~sockextcp-------------" << endl;
  closesocket(sock);
  return;
}

SockExUDP::SockExUDP(SOCKADDR_IN* saddr, SOCKET s) :SockEx(s) {
  if (s == INVALID_SOCKET) {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    new ((SockExUDP*)this)SockExUDP(saddr, sock); //must use SockExTCP to renew, orelse call virtual deconstruct function, ~SockExTCP cannot be triggered
    return;
  }

  DWORD opt = TRUE;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
  ::bind(sock, (sockaddr*)(saddr), sizeof(sockaddr_in));

  DWORD flags = MSG_PEEK;
  SockExOL* ov = new SockExOL(this, SockExOL::RCVFROM, buf, sizeof(buf));
  int n = WSARecv(sock, &ov->wsabuffer, 1, NULL, &flags, &ov->overlapped, NULL);
  LOG("wsarecv:sock:%d, %d, %d", sock, n, WSAGetLastError());
}

int SockExUDP::onRcv(int n) {
#if 0 //MSG_WAITALL不支持；初步认为udp包就是整体到达，后续有问题再说
  if (n < sizeof(buf) && recv(sock, (char*)&buf[n], sizeof(_tlv) - n, MSG_WAITALL) < 0) {
    return 0;
  } 
#endif
  int msglen = ntohs(((struct _tlv*)buf)->len) + sizeof(_tlv);
  char* rcvbuf = new char[msglen];
  
  SOCKADDR_IN peeraddr;
  socklen_t len = sizeof(struct sockaddr_in); //must init with size, orelse, addr get is incorrect; getpeername no effect
  int _s = recvfrom(sock, rcvbuf, msglen, 0, (struct sockaddr*)&peeraddr, &len); //because login is udp, one read can get whole info
  
  /* 1、必须在收完之后，否则可能导致报文内容重复  
  2、proTlvMsg中如果还需要使用本socket收消息，就没法接收；*/
  DWORD flags = MSG_PEEK; //must initial equal 0, orelse, no message rcv by workthread
  SockExOL* ov = new SockExOL(this, SockExOL::RCVFROM, buf, sizeof(buf));
  int rtn = WSARecv(sock, &ov->wsabuffer, 1, NULL, &flags, &ov->overlapped, NULL);
  LOG("wsarecvxx:sock:%d, %d, %d", sock, n, WSAGetLastError());

  if (_s > 0) {
    peer = &peeraddr;
    procTlvMsg(rcvbuf, msglen);
  }

  delete[] rcvbuf;
  return 0;
}

DWORD WINAPI sockRoutine(_In_ LPVOID lpParameter) {
  ULONG_PTR *lpContext = NULL;
  OVERLAPPED        *pOverlapped = NULL;
  DWORD            dwBytesTransfered = 0;

  while (TRUE) {
    BOOL bRet = GetQueuedCompletionStatus(completePort, &dwBytesTransfered, (PULONG_PTR)&lpContext, &pOverlapped, INFINITE);
    if (pOverlapped == NULL) {
      LOG("ERR:GetQueue err:%d", GetLastError());
      continue;
    }

    SockExOL* ol = CONTAINING_RECORD(pOverlapped, SockExOL, overlapped);
    SockEx *sockex = ol->sockex;
    SockExOL::OP_TYPE op = ol->op;
    delete ol;

    LOG("ERR:GetQueue OK:%d", op);
    //rcv len = 0 indicate remote close socket; and happened in 2 way:
//1, remote socket shutdown 2, local tcp enable keepalive option, and keepalive detect the tcp not working

    if (!bRet || op == SockExOL::INVALID) {
      DWORD errCode = GetLastError();
      if (errCode != ERROR_MORE_DATA) {
        LOG("ERR:errcode:%d, sockex:%p, op:%d, dwBytesTransfered=%d", errCode, sockex, op, dwBytesTransfered);

        if (op == SockExOL::CONNECT) { //ConnectEx error
          ((SockExTCP*)sockex)->onConnect(false);
        }
        continue;
      }
            
#if 0
      ERROR_ALREADY_EXISTS  183 (0xB7) Cannot create a file when that file already exists.
      ERROR_SEM_TIMEOUT 121 (0x79) The semaphore timeout period has expired // 
      ERROR_SERVER_SHUTDOWN_IN_PROGRESS 1255 (0x4E7) The server machine is shutting down. //nettest will cause this error
      ERROR_OPERATION_ABORTED(995->0x3e3) // cancelioex may cause 995 error
      ERROR_SEM_TIMEOUT(0x79) //if server refuse the connection or timeout, errcode=0x79, we should wakeup the thread; but for simplify coding, the thread will self-wakeup after 8s
      ERROR_NETNAME_DELETED  64(0x40) The specified network name is no longer available.
      ERROR_NOT_FOUND 1168(0x490) Element not found.
          //now just server and client; listen not access here
          //((SockExTCP*)sockex)->close(); ---> 20181221, closesocket at connectex
#endif
    }

    /* event and overlapped: https://docs.microsoft.com/zh-cn/windows/desktop/Sync/synchronization-and-overlapped-input-and-output
    All of the members of the OVERLAPPED structure must be initialized to zero unless an event will be used to signal completion of an I/O operation. If an event is used, the hEvent member of the OVERLAPPED structure specifies a handle to the allocated event object. The system sets the state of the event object to nonsignaled when a call to the I/O function returns before the operation has been completed */
    if (op == SockExOL::CONNECT) {
      /*if connectex indicate the connection is ok, we should not process the tcp->data;
      //and when debugging, we find an interesting thing, in this situation, the dwBytesTransfered = sended len, 
      //but the overlapped receive buffer has the reply msg */
      //((sockthread*)lpParameter)->withdraw(); //MUST check, why need this?
      
      ((SockExTCP*)sockex)->onConnect(true);
    } else if (op == SockExOL::ACCEPT) { //acceptex rcv len = 0, so no need consider process rcv data while acceptex
      //SO_UPDATE_ACCEPT_CONTEXT, opt must be listenSock and not BOOLEAN, msdn SOL_SOCKET's relative refrence is error
      ((SockExTCP*)sockex)->AcceptEx(((SockExTCP*)sockex)->srv);
      ((SockExTCP*)sockex)->onConnect(true);
    } else if (op == SockExOL::RCV){
      if (dwBytesTransfered == 0 || ((SockExTCP*)sockex)->onRcv(dwBytesTransfered) < 0) {
        delete (SockExTCP*)sockex;
      }
    } else if (op == SockExOL::RCVFROM) {
      ((SockExUDP*)sockex)->onRcv(dwBytesTransfered);
    }
    
    ((sockthread*)lpParameter)->withdraw(); //check if should suspend thread;
  }

  WSACleanup();
  return 0;
}

tlvprocesscallback tlvcbs[256] = {0};