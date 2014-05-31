// zfc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "zfc.h"
#include "recvCmd.h"
#include "sendCmd.h"

//#include <windows.h>

string teamName="";
string serverIp="";
ushort serverPort=0;
string myRole=""; //"POL" OR "THI"
ushort localPort=0;
Game game;


//void Wchar_tToString(std::string& szDst, _TCHAR* wchar)
//{
//	wchar_t * wText = wchar;
//	DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,wText,-1,NULL,0,NULL,FALSE);// WideCharToMultiByte的运用
//	char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量
//	psText = new char[dwNum];
//	WideCharToMultiByte (CP_OEMCP,NULL,wText,-1,psText,dwNum,NULL,FALSE);// WideCharToMultiByte的再次运用
//	szDst = psText;// std::string赋值
//	delete []psText;// psText的清除
//}


int checkPara(int argc, _TCHAR* argv[])
{
	//string teamName2, serverIp2, serverPort2, myRole2, localPort2;

    if(argc!=6)
    {
        log("错误：参数个数错误(%d)！", argc);
        log("命令格式： zfc 队名 服务器IP 服务器端口 角色（注：POL或THI） 本地接收端口（注：两种角色使用不同端口）");
        goto checkParaErr;
    }

	//Wchar_tToString(teamName2, argv[1]);
	//Wchar_tToString(serverIp2, argv[2]);
	//Wchar_tToString(serverPort2, argv[3]);
	//Wchar_tToString(myRole2, argv[4]);
	//Wchar_tToString(localPort2, argv[5]);

	//teamName=teamName2;
	//serverIp=serverIp2;
	//serverPort = stoi(serverPort2);
	//myRole = myRole2;
	//localPort = stoi(localPort2.c_str());


	teamName=argv[1];
	serverIp=argv[2];
	serverPort = atoi(argv[3]);
	myRole = argv[4];
	localPort = atoi(argv[5]);

    log("getPara: teamName[%s] serverIp[%s] serverPort[%u] myRole[%s] localPort[%u]", 
        teamName.c_str(), serverIp.c_str(), serverPort, myRole.c_str(), localPort);
    if(myRole!="POL" && myRole!="THI")
    {
        log("错误：角色参数错误（%s）！请使用POL或THI。",myRole.c_str());
        goto checkParaErr;
    }
	game.setRole(myRole);
    return OK;

checkParaErr:
    return ERROR;
}

int _tmain(int argc, _TCHAR* argv[])
{
    //检查参数
    int rt=checkPara(argc, argv);
    if(rt<0) goto end;

    //初始化UDP，挂接接收处理函数recvCmd处理接收指令
    if (ERROR == InitClient((char*)serverIp.c_str(),serverPort,localPort,recvCmd))
    {
        log("InitClient 失败");
        return ERROR;
    }

    //发送自己角色的信息给服务器：POL 或 THI指令
    sendMyInfoToServer();

    waitThreadEnd();//循环等待线程退出，可以在接收到END指令后直接用exit(0);退出程序。
end:
    getchar();
    return 0;
}

