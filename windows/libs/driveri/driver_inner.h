#pragma once

#include "driveri.h"
#define _NBS_KACHNL_NETLINK 21
#define _DRIVERI_SENDTCP_PORT NBS_SERVER_PORT
/*
enum _nbsdrv_inner_porttype {
  DRIVERI_TCPX = DRIVERI_MAX,
  DRIVERI_SVCPROXY,
};*/

struct _nbsdrv_fwdinfo {
  unsigned int svctype;
  struct {
    unsigned int tnllocalip;
    unsigned short tnllocalport;
  };
  struct {
    unsigned int tnlip; //updatefwdinfo cmd
    unsigned short tnlport;
  };
  struct {
    unsigned int ip;
    unsigned short port;
  };
  unsigned short mss;
};

enum _nbsdrv_nlmsg_cmd {
  REGISTER,
  UNREGISTER,
  UPDATEFWDINFO,
};

struct _nbsdrv_nlmsg { //netlink msg
  enum _nbsdrv_nlmsg_cmd cmd;
  struct {
    unsigned short port;
  }fwdkey;
  struct _nbsdrv_fwdinfo fwdinfo;
};

struct _psdheader {
  unsigned int saddr; //源IP地址
  unsigned int daddr; //目的IP地址
  char mbz; // mbz = must be zero, 用于填充对齐
  char protocol;    //8位协议号
  unsigned short tcpl; // TCP包长度
};

//ref to rfc: https://tools.ietf.org/html/rfc2460#section-8.1
struct _psdheader6 {
  struct in6_addr saddr;
  struct in6_addr daddr;
  unsigned int tcpl;
  char mbz[3];
  char nextheader; //similar to protocol
};

//is same as csum_partial, but i don't know how to use csum_partial
unsigned short inline GetCheckSum(unsigned short initCksum, unsigned short* buffer, int size) {
  unsigned int cksum = (unsigned int)initCksum;
  while (size > 1) {
    cksum += (*buffer);
    buffer++;
    size -= sizeof(unsigned short);
  }
  if (size) {
    char left_over[2] = { 0 };
    left_over[0] = *(unsigned char*)buffer;
    cksum += *(unsigned short*)left_over;
  }

  while (cksum > 0xffff) {
    cksum = (cksum >> 16) + (cksum & 0xffff);
  }

  return cksum;
}
