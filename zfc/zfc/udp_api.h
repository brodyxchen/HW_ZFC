#ifndef INTERFACE_API_H 
#define INTERFACE_API_H

typedef int (*PFN_RECV)(char *str);

//初始化客户端
extern int InitClient(char* pServerIpAddr, unsigned short usServerPort,
               unsigned short usClientPort, PFN_RECV pFn);


extern int UDP_SendMsgToServer(const char* pMsg);
extern void waitThreadEnd();

#endif