#ifndef UDP_H 
#define UDP_H

#include <WinSock2.h>
#include <Windows.h>

#define MSG_MAX 8192

//udp初始化
int UDP_InitSocket(char* pServerIpAddr, unsigned short usServerPort,
                   unsigned short usClientPort);

//清空sock相关资源
int UDP_CloseSocket();

//线程处理函数
DWORD WINAPI ThreadFunc(LPVOID lp);

//记录日志函数
//extern void log(const char *fmt, ...);

//清空资源
void ClearRes();

#ifndef UDP_LOG
#define UDP_LOG printf
#endif

#endif