#pragma once

#include "base.h"
#include "unidef.h"
#include<map>


#define databufFERLEN 2048
//extern UCHAR databuf[];

enum NBSDRV_PORTTYPE {
  DRIVERI_NORMAL = 0,
  DRIVERI_TUNNEL,
  DRIVERI_SVCSTART,
  DRIVERI_SVCTRANSIT,
  DRIVERI_SVCEND,
  DRIVERI_MAX
};

enum NBSDRV_FWDMODE {
  DRIVERI_FWDKERNEL = 0,
  DRIVERI_FWDUSER_SHARE,
};

struct DataChnlAttr {
    UINT32 localip;
    std::vector<UINT16> assignedports;

    DataChnlAttr(SOCKET s);
    virtual ~DataChnlAttr();
};

struct _fwdid_ {
    SOCKET s;
    time_t t;

    _fwdid_(SOCKET sock, time_t tm) {
        s = sock;
        t = tm;
    }
};

#define _FWDID_MAX_LEN 16
struct _fwdidinfo {
    char peerFwdId[_FWDID_MAX_LEN];
    char fwdidlen;
    SockEx* localEsock;
};

//static char* savetlv;//用来暂存服务器发给目标主机的tlv的

struct driveri {
#ifdef __cplusplus
  /*
  type=DRIVERI_TRANSIT：如果addr->port == 0, 说明要分配transitport；
  */    
    static UINT16 updateFwdInfo(SOCKADDR* addr, UINT32 type = DRIVERI_NORMAL, SOCKADDR* rmtAddr = nullptr, SockEx* tunnel = nullptr, SOCKADDR_IN* upAddr = nullptr);
    driveri();
    virtual ~driveri();
    static UINT32 GetGateWayAndIfIP(char*, UINT32*);
    static void getIfMac(char*, char*);
    static UINT32 getIfIP(char*);
    static void sendtcp6(sockaddr_in6* src, sockaddr_in6* dst);

    static int getPublicAddrr(UINT16 port, SockEx* ctrlchnl, SOCKADDR_IN* intraaddr, SOCKADDR_IN* publicaddr, NBSDRV_FWDMODE mode= DRIVERI_FWDUSER_SHARE);
    static UINT16 getAvailablePort(NBSDRV_PORTTYPE type, UINT32 peerip, UINT16 peerport, SockEx* tnl = nullptr);
#endif // __CPLUSPLUS
};

class selfServiceConnect :public SockExTCP {
public:
    //ChnlTCP(SOCKADDR_IN* peeraddr, char* token);
    //virtual ~ChnlTCP();
    virtual int onConnect(bool bConnect);
    //bool notUsing();
    virtual int onRcv(int len);
    //static void FwdtoDataChnl(SockEx* rcvsock, SOCKET datachnl, char peerFwdId, char fwdidlen, char* rcvbuf);
    UCHAR databuf[databufFERLEN];
    char peerFwdId[_FWDID_MAX_LEN];//
    char fwdidlen;
    int RcvEx(UCHAR* rcvbuf);

};

class SoftFwder {
public:
    SoftFwder();
    //static char* savetlv;
    
    int updatefwdinfo(SOCKADDR* xaddr, UINT32 type, SOCKADDR* downAddr, SockEx* xtunnel, SOCKADDR_IN* upAddr);
   
    static int proDrvConnect(SockEx* esock, StarTlv& tlvs);
    static int proDrvData(SockEx* esock, StarTlv& tlvs);
   
    static map<SOCKET,selfServiceConnect*>mapserandpc;
   
    static int onConnect(bool bConnect);
};
extern SoftFwder* fwder;
//map<SOCKET, char*> SoftFwder::mapserandpc;






