/*********************************************
CAUTION: this file using the following funtcions:
htons and htonl
so must include relative .h file according to OS before include this file
**********************************************/
#pragma once
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// tlv type 100 - 127 is system reserved and no need to resolve but must reply the same tlv
#define TLV_RES_JSON 100
#define TLV_RES_SYNHEAD 102
#define _TLV_MALLOC_PIECE 1024
//#define TLV_Swap16(_s_) ((((_s_) & 0xff) << 8) | (((_s_) >> 8) & 0xff))

struct _tlv {
  unsigned char type;
  unsigned char res;
  unsigned short len; //16位长度，要考虑大小序，必须转成网络序
};

//define msg subtype, if type is char, odd; if type is number, even
class StarTlv { //why define using class? for the enum type no coflict
public:
  char *_out;
  int _out_len;//这个是固定一次性分配的1kb长度
  //char _needfree; // needfree _out
  int total;//这个是实际使用的内存长度

  //StarTlv() { //_needfree = 0;
  //}; //do nothing
  StarTlv(unsigned char type) {
    //_out = buf;
    //_needfree = 0;
    //if (_out == 0) {
      _out = new char[_TLV_MALLOC_PIECE];
      //_needfree = 1;
    //}
    struct _tlv* tlv = (struct _tlv*)_out;
    tlv->type = type;
    total = sizeof(struct _tlv);
    _out_len = _TLV_MALLOC_PIECE;
  }

  ~StarTlv() {
    if (_out != 0) {
      delete[] _out;
    }
  }

  inline void pack_atom(unsigned char type, int len, char* value) {
  //type是tlv类型，len是value的长度，value是要组的数据内容
    if (total + len > _out_len) {//如果实际使用的长度+len超过了我一个tlv申请的固定的长度
      _out_len += (_TLV_MALLOC_PIECE > len) ? _TLV_MALLOC_PIECE : len;
      char* newout = new char[_out_len];
      memcpy(newout, _out, total);
      /*
      void* memcpy(void* destin, void* source, unsigned n)；
      函数的功能是从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中，即从源source中拷贝n个字节到目标destin中。
      */
      delete[] _out;
      _out = newout;
    }

    struct _tlv* tlv = (struct _tlv*)(_out + total);
    // may cannot swap value according to len and type
    if (type % 2 == 0) {
      memcpy((char*)(tlv + 1), value, len);
      *((char*)(tlv + 1) + len) = 0;
      len += 1;
    } else { 
      /*for (int i = 0; i < len; i++) {
        char *swapInt = (char*)(tlv + 1);
        swapInt[i] = value[len - i - 1];
      }*/
      if (len == 2) {
        *(unsigned short*)(tlv + 1) = htons(*(unsigned short*)value);
      } else if (len == 4) {
        *(unsigned int*)(tlv + 1) = htonl(*(unsigned int*)value);
      }
    }
    
    tlv->type = type;
    tlv->len = htons(len);
    total += sizeof(struct _tlv) + len;
  }

/*
  inline void insert_syshead(int len, char* value) {
    char mainType = *_out;
    _out -= sizeof(struct _tlv) + len + 1;

    struct _tlv* tlv = (struct _tlv*)_out;
    tlv->type = mainType;

    tlv += 1;
    memcpy((char*)(tlv + 1), value, len);
    *((char*)(tlv + 1) + len) = 0; // process like string;
    len += 1;

    tlv->type = TLV_RES_SYNHEAD;
    tlv->len = TLV_Swap16(len);
    total += sizeof(struct _tlv) + len;
  }
  */
  inline char* get_final() {
    int len = total - sizeof(struct _tlv);
    *(short*)(_out + 2) = htons(len);
    return _out;
  }

  char *in;
  int offset;
  StarTlv(char* data, bool needcopy=false) {
    total = htons(((struct _tlv*)data)->len);
    offset = 0;

    if (needcopy == false) {
      _out = 0;
      in = data + sizeof(struct _tlv);
    } else {
      _out = new char[total + sizeof(struct _tlv)];
      memcpy(_out, data, total + sizeof(struct _tlv));
      in = _out + sizeof(struct _tlv);
    }
    return;
  }

  // suggest get tlv according to fifo, because may have more then one tlv same type
  char* get_tlv(unsigned char type) {
    while (offset < total) {
      struct _tlv* tlv = (struct _tlv*)(in + offset);
      offset += htons(tlv->len) + sizeof(struct _tlv);
      if (tlv->type == type) {
          return (char*)(tlv + 1);
      }
    }
    return 0;
  }

  inline int get_len(char* value) {
    struct _tlv* tlv = (struct _tlv*)value;
    tlv -= 1;
    if (tlv->type % 2 == 0) {
      return htons(tlv->len) - 1;
    }
    return htons(tlv->len);
  }

public:
  enum MSGTYPE { //msg must be pair of requst and reply which with correct odd/even 
    MIN = 0x80,
    //CRTSVCCHNL = 0x80, // because this type may be captured in kernel, so suggest is a clear value
    TRANSNATADDR = 0x80,
    RTRANSNATADDR = 0x81, //subtlv same as transnataddr;
    DBQUERY = 0x82,
    RDBQUERY = 0x83,
    ROUTING = 0x84, //because this message similar as segment-routing
    RROUTING = 0x85,
    QUERYSVCADDR = 0x86, //get service address
    RQUERYSVCADDR = 0x87, //
    INNERSHELLMSG = 0x88,
    COOKIECONFIRM = 0x89, //废弃
    RCOOKIECONFIRM = 0x8a, //废弃
    LOGIN, //login
    RLOGIN, //login reply
    REGISTER, // client register, creating control channel
    RREGISTER, // reply
    ADVERTISERELAY, //advertise relay ability;
    REGHOST,
    NOTIFYNATSVCADDR,
    GETCTRLINFO,
    RGETCTRLINFO,
    NOTIFYRELAYCHANGE,
    GETDATACHNLADDR,//消息类型
    RGETDATACHNLADDR,
    REQUESTPUBLICADDR,
    RREQUESTPUBLICADDR,
    RELEASEPUBLICADDR,
    GETSELFADDR,
    DRV_CONNECTION,
    DRV_DATA,
    INNERNETTYPETEST,
    INNERREGCLIENT,
    INNERSETAUTOPOWER,
    INNERREGHOST,
    //INNERNOTIFYHOSTSTATUS,
    //INNERRLOGIN,
    INNERIPCHANGE, //废弃
    INNER_SOCKDOWN,
    INNERSVCWORKING,
    INNERSVCISUSING,
    INNERREGSVC,
    INNER_FULLCONE_INFO,
    INNER_FULLCONE_GETADDR,
    INNER_FULLCONE_RELEASEADDR,
    INNER_SERIALIZE,
    INNERLOGIN,
    INNERLOGINRTN,
    INNERRSHELLMSG,
    INNER_OBJDUMP,
    INNER_TIMER,
    MAX,

    CLIENTCONNECTSERVER_NOTICE_PC,


  };

  enum DBQUERY_SUBYPE { //
    DBQUERY_TABLE = 0,
    DBQUERY_USER_NAME = 2,
    DBQUERY_USER_PWD = 4,
    DBQUERY_HWID = 6,
  };

  enum RDBQUERY_SUBYPE { //
    RDBQUERY_RESULT = 3,
    RDBQUERY_SHARE = 5,
    RDBQUERY_DOMAIN_NUMS = 9,
    RDBQUERY_DOMAIN = 10,
  };

  enum LOGIN_SUBYPE { // for using PACK_ATOM, subtype must not equal MSGTYPE
    LOGIN_VERSION = 0,
//    LOGIN_NAME = 2, // if pwdis mac-address
 //   LOGIN_PWD = 4, 
    //LOGIN_RANDOM = 0x06,
    //LOGIN_HWID = 8, 
  //  LOGIN_NETTYPE = 9,
  //  LOGIN_NEEDRELAY = 10,
  };

  enum RLOGIN_SUBYPE { //
    RLOGIN_RANDOM = 2,
    /* replace by json
    RLOGIN_RESULT = 3,
    RLOGIN_HOST_NUMS = 5,
    RLOGIN_HOSTNAME = 6,
    RLOGIN_SHARE = 7,
    RLOGIN_DEFAULTHOST = 8, */
  };

  enum REGISTER_SUBYPE { //
    REGISTER_TOKEN = 2,
    REGISTER_NETTYPE = 5,
    REGISTER_NETATTR = 7,
  };

  enum ADVERTISERELAY_SUBTYPE {
    ADVERTISERELAY_ABILITY = 0, // 1 -enable, 0 - disable
    ADVERTISERELAY_TOKEN = 2, 
    ADVERTISERELAY_ADDR = 4
  };

  enum RREGISTER_SUBYPE { //
    RREGISTER_RESULT = 0x01,
  };

  enum REGHOST_SUBYPE { //
    REGHOST_TYPE = 0,
    REGHOST_HOSTNAME = 2,
  };

  enum NETTYPETEST_SUBTYPE {
    NETTYPETEST_TYPE = 1, // 1 - request, 2 - assign
    NETTYPETEST_TOKEN = 2,
    NETTYPETEST_ADDR = 4,
  };

  enum RNETTYPETEST_SUBTYPE {
    RNETTYPETEST_INTERADDR = 2,
  };

  enum NOTIFYNATSVCADDR_SUBTYPE {
    //NOTIFYNATSVCADDR_TLP = 1,
    NOTIFYNATSVCADDR_INTRAPORT = 2,
    NOTIFYNATSVCADDR_IP = 4,
    NOTIFYNATSVCADDR_PORT = 6,
  };

  enum GETCTRLINFO_SUBTYPE {
    GETCTRLINFO_VERSION = 0,
    GETCTRLINFO_TOKEN = 2,
    GETCTRLINFO_DOMAIN = 4,
    GETCTRLINFO_NETTYPE = 5,
    GETCTRLINFO_CTRLIP = 6, //cur ctrl ip
  };

  enum RGETCTRLINFO_SUBTYPE {
    RGETCTRLINFO_HOSTNAME = 0,
    RGETCTRLINFO_RESULT = 1,
    RGETCTRLINFO_ADDR = 2,
    RGETCTRLINFO_TOKEN = 4,
  };

#if 0
  enum QUERYSVCADDR_SUBTYPE {
    //synheader
    //QUERYSVCADDR_AUTHTOKEN = 0,
    QUERYSVCADDR_SVCDOMAIN = 2,
    QUERYSVCADDR_SVCPORT = 3,
    QUERYSVCADDR_DEVICENAME = 4,
  };

  enum RQUERYSVCADDR_SUBTYPE {
    //RQUERYSVCADDR_RESULT = 1,
    RQUERYSVCADDR_IP = 2,
    RQUERYSVCADDR_PORT = 4,
  };
#endif

  enum NOTIFYREALIZERELAY_SUBTYPE {
    NOTIFYREALIZERELAY_HASHINDEX = 1,
    NOTIFYREALIZERELAY_IP = 2,
    NOTIFYREALIZERELAY_PORT = 4,
    NOTIFYREALIZERELAY_TOKEN = 6,
  };

  enum NOTIFYRELAYCHANGE_SUBTYPE {
    //NOTIFYCHANGERELAY_HASHINDEX = 1,
    NOTIFYRELAYCHANGE_IP = 2,
    NOTIFYRELAYCHANGE_PORT = 4,
    NOTIFYRELAYCHANGE_TOKEN = 6,
    //NOTIFYRELAYCHANGE_HOSTNAME = 8,
  };

  enum TRANSNATADDR_SUBTYPE {
    TRANSNATADDR_LOCALPORT = 2,
    TRANSNATADDR_INTERIP = 4,
    TRANSNATADDR_INTERPORT = 6,
  };

  enum GETDATACHNLADDR_SUBTYPE {
    //synheader,
  };

  enum RGETDATACHNLADDR_SUBTYPE {//这个具体是干啥的？
    //synheader
    RGETDATACHNLADDR_ADDR = 0,
  };

  enum REQUESTPUBLICADDR_SUBTYPE {
    //synheader
    REQUESTPUBLICADDR_INTRAADDR = 2,
    REQUESTPUBLICADDR_CTRLIP = 4,
  };

  enum RREQUESTPUBLICADDR_SUBTYPE {
    //synheader
    RREQUESTPUBLICADDR_INTRAADDR = 0,
    RREQUESTPUBLICADDR_INTERADDR = 2,
    RREQUESTPUBLICADDR_RELAYINTRAPORT = 4, //if fullcone working as relay, the relayport != interaddr.sin_addr
  };

  enum RELEASEPUBLICADDR_SUBTYPE {
    RELEASEPUBLICADDR_INTERADDR = 0,
  };
  
  enum POSTSVCUSERS_SUBTYPE {
    POSTSVCUSERS_NUM = 1,
    POSTSVCUSERS_DOMAIN = 2,
    POSTSVCUSERS_NAME = 4,
  };

  enum ROUTING_SUBTYPE {
    //syn
    ROUTING_DHOST = 2, //dest hostname
    ROUTING_PAYLOAD = 4, //payload is json
  };

  enum RROUTING_SUBTYPE {
    //syn
    RROUTING_PAYLOAD = 2, //payload is json
  };

  enum DRV_CONNECTION_SUBTYPE {
    DRV_CONNECTION_TYPE = 0, // 0 - connection down; 1 - new connection; 2 - new connection reply
    DRV_CONNECTION_RMTFWDID = 2,
    DRV_CONNECTION_LOCALFWDID = 4,
    DRV_CONNECTION_ADDR = 6,
  };

  enum DRV_DATA_SUBTYPE {
    DRV_DATA_FWDID = 0,
    DRV_DATA_PAYLOAD = 2,
  };

  enum INNERNETTYPETEST_SUBTYPE {
    INNERNETTYPETEST_ADDR = 2,
  };

  enum INNERNOTIFYNETTYPE_SUBTYPE {
    INNERNOTIFYNETTYPE_TYPE = 2,
    INNERNOTIFYNETTYPE_ATTR = 4,
    INNERNOTIFYNETTYPE_MAINIP = 6,
  };

  enum INNERNOTIFYSTARINFO_SUBTYPE {
    INNERNOTIFYSTARINFO_TYPE = 0,
    INNERNOTIFYSTARINFO_STATUS = 2,
    INNERNOTIFYSTARINFO_ARP = 4,
  };

  enum INNERREGCLIENT_SUBTYPE {
    INNERREGCLIENT_NAME = 2,
    INNERREGCLIENT_AUTOPOWER = 4,
    INNERREGCLIENT_MAC = 6,
  };

  enum INNERLOGINSUCCESS_SUBTYPE {
    INNERLOGINSUCCESS_NETTYPE = 0,
    INNERLOGINSUCCESS_NETATTR = 2,
  };

  enum INNERSETAUTOPOWER_SUBTYPE {
    INNERSETAUTOPOWER_CLTIP,
    INNERSETAUTOPOWER_VALUE,
  };

  enum INNERREGHOST_SUBTYPE {
    INNERREGHOST_TYPE = 0, //register or unregister
    INNERREGHOST_HOSTNAME = 2,
    INNERREGHOST_HOSTIP = 4,
    //INNERREGHOST_HOSTMAC = 4,
    //INNERREGHOST_RELAYADDR = 6,
    //INNERREGHOST_ISMAIN = 8,
    //INNERREGHOST_RELAYTOKEN = 10,
    //INNERREGHOST_TOKEN = 12, //only for shell to save the token
  };

  enum INNERNOTIFYHOSTSTATUS_SUBTYPE {
    INNERNOTIFYHOSTSTATUS_HOSTNAME = 2,
    INNERNOTIFYHOSTSTATUS_STATUS = 4,
  };

  enum INNER_SOCKDOWN_SUBTYPE {
    INNER_SOCKDOWN_SOCK = 2,
    INNER_SOCKDOWN_SOCK_TIMESTAMP = 4,
  };

  enum INNERSVCWORKING_SUBTYPE {
    INNERSVCWORKING_ACTION = 0,
    INNERSVCWORKING_ADDR = 2,
  };

  enum INNERSVCISUSING_SUBTYPE {
    INNERSVCISUSING_ADDR = 2,
  };

  enum INNERREGSVC_SUBTYPE {
    //INNERREGSVC_CLTMAC = 0,
    INNERREGSVC_CLTIP = 0,
    INNERREGSVC_PROTOCOL = 2,
    INNERREGSVC_PORT = 4,
    INNERREGSVC_SVCNAME = 6,
    INNERREGSVC_TYPE = 8,
  };

  enum INNER_FULLCONE_INFO_SUBTYPE {
    INNER_FULLCONE_INFO_SRCIP = 2,
    INNER_FULLCONE_INFO_DSTIP = 4,
    INNER_FULLCONE_INFO_DSTPORT = 6,
  };

  enum INNER_FULLCONE_GETADDR_SUBTYPE {
    //synheader
    INNER_FULLCONE_GETADDR_LOCALPORT = 2,
    INNER_FULLCONE_GETADDR_INTERIP = 4,
    INNER_FULLCONE_GETADDR_INTERPORT = 6,
  };

  enum INNER_FULLCONE_RELEASEADDR_SUBTYPE {
    INNER_FULLCONE_RELEASEADDR_LOCALPORT = 2,
  };

  enum INNERLOGIN_SUBTYPE {
    INNERLOGIN_CLTMAC = 0,
    INNERLOGIN_USER = 2, //废弃
    INNERLOGIN_PWD = 4, // 废弃
  };

  enum INNERLOGINRTN_SUBTYPE {
    INNERLOGINRTN_CLTMAC = 0,
  };

  enum GETSELFADDR_SUBTYPE {
    //GETSELFADDR_RNDCHARS = 0,
    GETSELFADDR_INTERADDR = 2,
    GETSELFADDR_KNOWNPORT_NUM = 3,
    GETSELFADDR_KNOWNPORT = 4,
  };

  enum INNER_OBJDUMP_SUBTYPE {
    INNER_OBJDUMP_DESC = 0,
    INNER_OBJDUMP_TYPE = 1,
    INNER_OBJDUMP_KEY = 2,
  };
};

#ifdef __cplusplus
}
#endif //__cplusplus