#include "stdafx.h"
#include "zfc.h"
#include "recvCmd.h"


int recvIni(char *str)
{
    //接收初始化指令处理
	game.receiveINI(str);

    return 0;
}

int recvInf(char *str)
{
    //接收INF指令   
	game.receiveINF(str);
	game.sendMOV();

    return 0;
}

//接收到END指令，退出进程
int recvEnd(char *str)
{
    log("recv END!");
    exit(0);//退出进程
}

int recvCmd(char *str)
{
    log("[RECV]: %s",str);
    if(strstr(str,"INF"))
    {
        recvInf(str);
        //接收到INF之后的处理
    }
    else if(strstr(str,"INI"))
    {
        recvIni(str);
    }
    else if(strstr(str,"END"))
    {
        recvEnd(str);
    }
    else
    {
        log("\r\n ERROR RECV CMD[%s]!", str);
    }

    return 0;
}

