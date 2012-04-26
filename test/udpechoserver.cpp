#include "../iserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

IUdpServer * g_pUdpServer;
IUdpServer * g_pUdpServer2;

class CUdpServerDataHandle: public IUdpServerDataHandle
{
public:
	virtual ~CUdpServerDataHandle(){}
public: 
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nLen)
	{
		g_pUdpServer->Send(IP, Port, pData, nLen);
	}
};

class CUdpServerDataHandle2: public IUdpServerDataHandle
{
public:
	virtual ~CUdpServerDataHandle2(){}
public: 
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nLen)
	{
		g_pUdpServer2->Send(IP, Port, pData, nLen);
	}
};

CUdpServerDataHandle g_UdpServerDataHandle;
CUdpServerDataHandle2 g_UdpServerDataHandle2;

int port = 6666;
int port2 = 8888;

static void* thread_run(void *)
{
	g_pUdpServer->Start(0, port);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1)
		port = atoi(argv[1]);

	if (argc > 2)
		port2 = atoi(argv[2]);

	g_pUdpServer = CreateUdpServerInstance();
	g_pUdpServer->SetDataHandle(&g_UdpServerDataHandle);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_t thread_id;
	pthread_create(&thread_id, &thread_attr, &thread_run, NULL);

	g_pUdpServer2 = CreateUdpServerInstance();
	g_pUdpServer2->SetDataHandle(&g_UdpServerDataHandle2);
	g_pUdpServer2->Start(0, port2);

	return 0;
}
