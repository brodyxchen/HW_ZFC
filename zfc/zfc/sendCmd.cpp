#include "stdafx.h"
#include "zfc.h"
#include "sendCmd.h"

//发送指令到UDP层
int sendCmd(char *str)
{
    log("[SEND]: %s", str);
    UDP_SendMsgToServer(str);
    return 0;
}

//发送POL指令
int sendPolice(string name, ushort port)
{
    char buf[BUFLEN];
    sprintf_s(buf, BUFLEN, "POL(%s,%u)", name.c_str(), port);
    sendCmd(buf);
    return 0;
}

//发送THI指令
int sendThief(string name, ushort port)
{
    char buf[BUFLEN];
    sprintf_s(buf, BUFLEN, "THI(%s,%u)", name.c_str(), port);
    sendCmd(buf);
    return 0;
}

//发送自己的角色信息到服务器
int sendMyInfoToServer()
{
    if(myRole=="POL")
    {
        sendPolice(teamName, localPort);
    }
    else if(myRole=="THI")
    {
        sendThief(teamName, localPort);
    }

    return 0;
}

int sendMoveToServer(char* str)
{
	sendCmd(str);
	return 0;
}
