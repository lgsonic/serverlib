#include "../iserver.h"
#include <stdio.h>
#include <stdlib.h>


ICommonServer * g_pTcpServer;

class CTcpServerDataHandle: public ITcpServerDataHandle
{
public:
	virtual ~CTcpServerDataHandle(){}
public: 
	virtual void  OnClientConnected(SocketClientData_t sClient)
	{
	}

	virtual void  OnClientDisconnected(SocketClientData_t sClient, int nErrorCode)
	{
		
	}

	virtual void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nLen)
	{
		g_pTcpServer->Send(sClient, pData, nLen);
	}
};


CTcpServerDataHandle g_TcpServerDataHandle;

int port = 8888;
int threadnum = 4;

int main(int argc, char **argv)
{
	g_pTcpServer = CreateCommonServerInstance();
	g_pTcpServer->SetDataHandle(&g_TcpServerDataHandle);
	g_pTcpServer->SetThreadNum(threadnum);
	g_pTcpServer->Start(0, port);

	return 0;
}
