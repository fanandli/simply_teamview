#ifndef __SOCKEX_H__
#define __SOCKEX_H__
#define _WINSOCKAPI_
#include <atomic>
#include <condition_variable>
#include "base.h"
//#include "Mswsock.h"
using namespace std;

typedef DWORD pthread_t;
class sockthread;
//extern atomic<sockthread*> freethreads;
extern sockthread* allthreads[];
class SockEx;
class StarTlv;

#define THREADPOOL_THREAD_MAXNUM 50
class sockthread {
public:
  enum threadtype {
    LISTENING,
    WORKING,
    FREE,
  };
public:
  sockthread* next; //MUST CHECK, need to define atomic type?
  pthread_t tid;
  threadtype type;
  condition_variable sig;
  atomic_int synseq; //must be same with the synheader, orelse cannot wakeup the thread
  char* msg;
public:
  sockthread(bool hasthread);
  void withdraw();
  void  wakeup(char* buf = nullptr, int buflen = 0);

  static sockthread* getfree();
  static INT32 getselfid();
  static char* wait(int timeout = 8);
  static void pack_synheader(StarTlv* msg);
};

//#define SOCKEX_CONESOCK_KAINTERVEL 32
//extern list_head g_activeConeSockSock[];

typedef int(*tlvprocesscallback)(SockEx*, StarTlv&);
extern tlvprocesscallback tlvcbs[];
extern HANDLE completePort;
extern int TlvSynReplyCb(SockEx* sockex, StarTlv& packet);

#define SOCKEX_BUF_LEN 1460 //mtu - ipheader - tcpheader
class SockEx {
public:
  SOCKET sock;
public:
  SockEx(bool isdummy) {};
  SockEx(SOCKET);
  virtual ~SockEx() {};
  virtual int procTlvMsg(char* data, int len);
  UINT16 getPort(int family = AF_INET) {
      if (family == AF_INET) {
          SOCKADDR_IN addr;
          int len = sizeof(SOCKADDR_IN);
          int err = getsockname(sock, (SOCKADDR*)&addr, &len);
          return addr.sin_port;
      }
      SOCKADDR_IN6 addr6;
      int len = sizeof(SOCKADDR_IN6);
      int err = getsockname(sock, (SOCKADDR*)&addr6, &len);
      return addr6.sin6_port;
  }
};

class SockExTCP:public SockEx { // complete io thread process, because need overlapped
public:
  SockExTCP* srv;
  UCHAR buf[64];
public:
  /* 默认是同步, 连接不成功时，调用处会出现超时
  如果入参是异步，连接不成功时回掉 onconnect */
  int ConnectEx(sockaddr *name, char* buffer = NULL, int len = 0);
  int RcvEx(UCHAR*, int n = 4, DWORD flags = MSG_PEEK); //4 = sizeof(_tlv)
  //int close();
  virtual int onConnect(bool bConnect);
  virtual int onRcv(int n);
  //virtual int onClose() { return 0; };
  SockExTCP(SOCKET s = INVALID_SOCKET, SOCKADDR* saddr=NULL, int len = 0, bool server=false);
  virtual ~SockExTCP();

  static int AcceptEx(SockExTCP*);
  virtual SockExTCP* newAcceptSock(SockExTCP* srv); 
};

class SockExUDP :public SockEx {
public:
  SOCKADDR_IN* peer;
  char buf[4]; //sizeof(_tlv)

  SockExUDP(SOCKADDR_IN* saddr, SOCKET s = INVALID_SOCKET);
  //virtual int procTlvMsg(char* data, int len);
  int onRcv(int n);
};

class SockExOL { //sockex overlapped
public:
  enum OP_TYPE {
    ACCEPT,
    RCV,
    CONNECT,
    RCVFROM,
    INVALID,
  };
public:
  WSAOVERLAPPED overlapped;
  OP_TYPE op;
  WSABUF wsabuffer;
  SockEx* sockex;

public:
  SockExOL(SockEx* sockex, OP_TYPE op, char* rcvbuf = nullptr, int rcvmax = 4) { //4 = sizeof(_tlv)
    this->sockex = sockex;
    this->op = op;
    ZeroMemory(&overlapped, sizeof(overlapped));

    wsabuffer.buf = rcvbuf;
    wsabuffer.len = rcvmax;
  };
};

DWORD WINAPI sockRoutine(_In_ LPVOID lpParameter);
#endif