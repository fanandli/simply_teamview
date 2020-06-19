#pragma once
#include <stdio.h>
#include <map>
#include "json-c/json.h"
#include "list.h"
#include "base.h"
#include "unidef.h"
#include "sockex.h"
#include "tlv.h"

struct NeighEntryKey {
  in6_addr xaddr;
  bool operator < (const NeighEntryKey& _A) const {
    if (memcmp(&xaddr, &_A.xaddr, sizeof(xaddr)) < 0) {
      return true;
    }
    return false;
  }
};

class NeighEntry {
public:
  NeighEntryKey xaddr;
  char mac[IFHWADDRLEN];
  time_t lastchangetime;
  int state;

  NeighEntry(in6_addr*, char* mac);
  ~NeighEntry();
};

class AddrPool :public SockEx {
public:
  std::map<NeighEntryKey, NeighEntry*> addrcache;

  AddrPool(SOCKET s = INVALID_SOCKET);
  virtual int procSock(__uint32_t events);
};

extern int testnettype();
class ShellUdpSock :public SockExUDP {
public:
  virtual int procTlvMsg(char* data, int len);
  ShellUdpSock(sockaddr_in* server_address):SockExUDP(server_address) {};
  virtual ~ShellUdpSock() {};
  static int proInnerSerialize(SockEx*, StarTlv& tlvs);
};

struct StarAttr {
  //UINT32 bestIP;
  //STAGE stage;
  //Timer tm;
  NBS_NETTYPE netType;
  UINT32 netAttribute;
  UCHAR bannedport[8192]; // suppose 443,80,8080 often banned by carrier
  //SockEx *uiserver;
  //list_head shells;
  UINT16 multiDialNum;
  ShellUdpSock* shellsock;
  void* relay;
  list_head clients;
  json_object* cfg;
  //char intragwif[IFNAMSIZ];
  //UINT32 intragwip;
  unsigned char mac[IFHWADDRLEN];
  UINT32 bestip;
  in6_addr bestip6;
  AddrPool* addrpool;
  
  StarAttr();
  static void parsecfg();
  static void savecfg();
  static char* procGetcfg(json_object*);
  static char* procCommitcfg(json_object*, json_object*);
  char* getintragwif();
  char* gettoken();
};
extern StarAttr g_attr;

//Client是静态配置部分，CtrlChnl是动态信息部分
class Client {
public:
  list_head entry;
  list_head allsvc;
  unsigned char mac[IFHWADDRLEN];
  char bAutoPower;

  //ShellSockex* esock;
  int loginRtn;
  json_object* cfginsrv;
  json_object* cfg;
  in_addr activeip;
  in6_addr activeip6;

  //Client(UINT32 cltip, char autopower);
  Client(json_object* cfg);
  ~Client();
  static char* getmacbyname(char* devname);
  static Client* getbymac(char* mac);
  char* getUser(char*);
  char* getPwd();
  void autopower();
  int login(char*name, char* pwd = nullptr);
  int initActiveip();
  int proNeighChange(int family, void* addr, int type);
  char* getdefaulthost(char*);
  char* getnamefromdhcp(char*);
  int regHost(int reg);
  
  static int proInnerSetAutoPower(SockEx*, StarTlv&);
  static int proInnerLoginRtn(SockEx*, StarTlv&);
  static int proInnerRegHost(SockEx* esock, StarTlv& tlv);
  static int proInnerRegSvc(SockEx*, StarTlv&);
  static int proInnerShellMsg(SockEx*, StarTlv&);
};

class Timer {
public:
  static int proInnerTimer(SockEx*, StarTlv&);
};

extern json_object* getdhcpclients();
