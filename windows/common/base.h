#ifndef __BASE_H__
#define __BASE_H__
//https://blog.csdn.net/sukhoi27smk/article/details/55000279, orelse compiler error
#define _WINSOCKAPI_
#include "windows.h"
#include "in6addr.h"
#include "ws2tcpip.h"
#include <atltime.h>
#include <iostream>
#include "precompiler.h"

extern void openlog(char* filename, int level, int unknown);
extern int WriteLog(char* str);

/*
# define LOG(_format_, ...) {\
    char _output_[256];\
    sprintf_s(_output_, _format_, __VA_ARGS__);WriteLog(_output_);\
  }*/

#define LOG(_format_, ...) printf(_format_"\r\n", ##__VA_ARGS__)
#define DBG(_format_, ...) printf(_format_"\r\n", ##__VA_ARGS__)

#define MSG_NOSIGNAL 0
#define SOCK_CLOEXEC 0
#define LOG_CONS 0

#define IFHWADDRLEN 6
#define RTM_DELADDR 29
#define IF_NAMESIZE 16
typedef int socklen_t;
typedef UINT32 __uint32_t;
typedef int ssize_t;

#define DEFAULT_HOST_SUFFIX "pcs.netbridge.vip"

#endif