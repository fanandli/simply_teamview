#pragma once
//NBS: netbridge system

enum NBS_NETTYPE {
  SERVER, 
  DYNGLOBAL, //has global ip
  FULLCONE, //full-cone NAT
  SYMMETRIC, //port-strict cone / address-strict cone / symmetric
  INVALID,
};

#define NBS_LOGINRAND_LEN 16
#define NBS_USER_SIZE 32
#define NBS_PWD_LEN 64
#define NBS_HOSTNAME_SIZE 64
#define NBS_TOKEN_LEN 31

#define NBS_NETATTR_NONE 0
#define NBS_NETATTR_FAKESERVERIP 1
#define NBS_NETATTR_SHARE 2
#define NBS_NETATTR_HTTP_VALID 4
#define NBS_NETATTR_FWD_TCPX 8
#define NBS_NETATTR_INVALID 0xFFFFFFFF

#define NBS_CREATESOCKADDR(_sockaddr_, _ip_, _port_) SOCKADDR_IN _sockaddr_; _sockaddr_.sin_addr.s_addr = _ip_; _sockaddr_.sin_port = _port_; _sockaddr_.sin_family = AF_INET;
#define NBS_SERVER_NAME "a.netbridge.vip"
#define NBS_SERVER_PORT 0x9999

#define NBS_CLIENTNAME_SIZE 32

enum NBS_ERRCODE {
  NBS_OK = 0,
  NBS_NETWORK_ERROR,
  NBS_USRPWD_ERROR,
  NBS_TOKEN_EXPIRED,
  NBS_NOHOST,
  NBS_VER_INCOMPATIBLE,
  NBS_NORELAY,
  NBS_MAX_ERROR,
};
