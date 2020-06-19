#pragma once
#include <mutex>
#include <vector>
#include "unidef.h"
#include "json-c/json.h"
#include "service.h"
#include "sockex.h"
#include "tlv.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

class ChnlTCP :public SockExTCP {
public:
  ChnlTCP(SOCKADDR_IN* peeraddr, char* token);
  virtual ~ChnlTCP();
  virtual int onConnect(bool bConnect);
  bool notUsing();
};

/*
class ClientHost {
public:
  list_head entry;
  char domainName[NBS_HOSTNAME_SIZE];
  char mac[IFHWADDRLEN];
  UCHAR local;
public:
  ClientHost(char* hostname, char* mac, int justlocal = 0);
  virtual ~ClientHost();
  int regHost(int bReg = 1);
  inline void notifyHostStatus(char*, int status);
};

class ChnlPubInfo {
public:
  SOCKET resSock;
  UINT32 bestIP;
  UINT16 chnlport;
  ChnlTCP* chnl; //ctrlchnl, maybe forward data; fwdchnl is the true forward channel for per service

  ChnlPubInfo();
  virtual ~ChnlPubInfo();
};
*/

class CtrlChnl {
public:
  /*CAUTION: because innerreghost will fork new thread, and after the thread, we may delete the host;
  if all hosts in one list, may cause exception
  */
  //list_head hosts;
  //ChnlPubInfo *pubinfo;
  ChnlTCP* chnl; //ctrlchnl, maybe forward data; fwdchnl is the true forward channel for per service

public:
  CtrlChnl();
  virtual ~CtrlChnl();
  void clear();
  int login(SOCKADDR_IN*, char*, int*);
  ChnlTCP* nettest(SOCKADDR_IN *, char*);
  int regallsrv();
  char* getselfdomain(char*);
  //ClientHost* getHost(char*);
  //inline ClientHost* getDefaultHost();
  
  UINT32 getbestip();
  void setUpChnl(ChnlTCP*);
  static int createUplink(SOCKADDR_IN* addr, char* token);
  static int regSvc(Client*, char*, char*, UINT16 nport, char proto);
  static int unregSvc(Client*, char*, char*, UINT16 nport = 0, char proto =0);

  static int proNotifyRelayChange(SockEx* esock, StarTlv& msg);
  static char* proQuerySvcAddr(json_object* msg, json_object* reply);
//  static int proInnerIpChange(SockEx* esock, StarTlv& tlv);
  static int proInnerSvcIsUsing(SockEx* esock, StarTlv& tlvs);
  static int proRouting(SockEx*, StarTlv&);  
};
extern CtrlChnl* g_cchnl;

#ifdef __cplusplus
}
#endif //__cplusplus