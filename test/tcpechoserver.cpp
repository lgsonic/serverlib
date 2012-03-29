#include "../iserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

ITcpServer * g_pTcpServer;
ITcpServer * g_pTcpServer2;

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

class CTcpServerDataHandle2: public ITcpServerDataHandle
{
public:
	virtual ~CTcpServerDataHandle2(){}
public: 
	virtual void  OnClientConnected(SocketClientData_t sClient)
	{
	}

	virtual void  OnClientDisconnected(SocketClientData_t sClient, int nErrorCode)
	{
		
	}

	virtual void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nLen)
	{
		g_pTcpServer2->Send(sClient, pData, nLen);
	}
};

CTcpServerDataHandle g_TcpServerDataHandle;
CTcpServerDataHandle2 g_TcpServerDataHandle2;

int port = 6666;
int port2 = 8888;

static void* thread_run(void *)
{
	g_pTcpServer->Start(0, port);
}

int main(int argc, char **argv)
{
	if (argc > 1)
		port = atoi(argv[1]);

	if (argc > 2)
		port2 = atoi(argv[2]);

	g_pTcpServer = CreateTcpServerInstance();
	g_pTcpServer->SetDataHandle(&g_TcpServerDataHandle);
	
	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_t thread_id;
	pthread_create(&thread_id, &thread_attr, &thread_run, NULL);

	g_pTcpServer2 = CreateTcpServerInstance();
	g_pTcpServer2->SetDataHandle(&g_TcpServerDataHandle2);
	g_pTcpServer2->Start(0, port2);

	return 0;
}
