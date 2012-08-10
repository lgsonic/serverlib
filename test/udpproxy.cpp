#include "../iserver.h"
#include "../iclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static const int serverport = 9999;
static std::string targetip = "127.0.0.1";
static int targetport = 8888;
static int concurrency = 4;

class IUdpProxyClientPool
{
public:
	virtual void Init() = 0;
	virtual void Send(unsigned int IP, unsigned short Port, const char *pData, int nDataLen) = 0;
};

IUdpProxyClientPool * g_pUdpProxyClientPool;
IUdpServer * g_pUdpServer;
IUdpServerDataHandle *g_UdpServerDataHandle;


class CUdpClientDataHandle: public IUdpClientDataHandle
{
public:
	virtual ~CUdpClientDataHandle(){}
public: 
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nDataLen)
	{
		if(nDataLen <= 10)
		{
			return;
		}

		std::string strSend;
		int nPktLen = nDataLen - 10;
		strSend.insert(strSend.end(), (char*)&nPktLen, (char*)&nPktLen+4);
		strSend.insert(strSend.end(), pData + 10, pData + nDataLen);

		IP = *(unsigned int*)&pData[4];
		Port = *(unsigned short*)&pData[8];

		g_pUdpServer->Send(IP, Port, strSend.c_str(), strSend.size());
	}
};

class CUdpServerDataHandle: public IUdpServerDataHandle
{
public:
	virtual ~CUdpServerDataHandle(){}
public: 
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nDataLen)
	{
		if(nDataLen <= 4)
		{
			return;
		}
		
		std::string strSend;
		int nPktLen = nDataLen-4+6;
		strSend.insert(strSend.end(), (char*)&nPktLen, (char*)&nPktLen+4);
		strSend.insert(strSend.end(), (char*)&IP, (char*)&IP + sizeof(IP));
		strSend.insert(strSend.end(), (char*)&Port, (char*)&Port + sizeof(Port));
		strSend.insert(strSend.end(), pData+4, pData + nDataLen);
		
		g_pUdpProxyClientPool->Send(IP, Port, strSend.c_str(), strSend.size());
	}
};

static void* run_thread_client(void * param)
{
	IUdpClient * pUdpClient = (IUdpClient *)param;
	pUdpClient->Start(ntohl(inet_addr(targetip.c_str())), targetport);
	return 0;
}

class CUdpProxyClient
{
public:
	void Init()
	{
		m_pUdpClient = CreateUdpClientInstance();
		m_pUdpClient->SetDataHandle(&m_UdpClientDataHandle);
		pthread_attr_t thread_attr_client;
		pthread_attr_init(&thread_attr_client);
		pthread_t thread_id_client;
		pthread_create(&thread_id_client, &thread_attr_client, &run_thread_client, m_pUdpClient);
	}
	void Send(const char *pData, int nDataLen)
	{
		m_pUdpClient->Send(pData, nDataLen);
	}
private:
	IUdpClient * m_pUdpClient;
	CUdpClientDataHandle m_UdpClientDataHandle;
};

class CUdpProxyClientPool: public IUdpProxyClientPool
{
public:
	void Init()
	{
		m_UdpProxyClients = new CUdpProxyClient[concurrency];
		for(int i = 0; i < concurrency; ++i)
		{
			m_UdpProxyClients[i].Init();
		}
	}
	void Send(unsigned int IP, unsigned short Port, const char *pData, int nDataLen)
	{
		m_UdpProxyClients[__Hash(IP, Port)].Send(pData, nDataLen);
	}
private:
	int __Hash(unsigned int IP, unsigned short Port) const
	{
		return ((IP+Port)%concurrency);
	}
	
	CUdpProxyClient * m_UdpProxyClients;
};

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("usage: -c concurrency ip:port\n");
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'c':
				concurrency = atoi(argv[i+1]);
				break;
			default:
				break;
			}
			i++;
		}
		else
		{
			std::string url = argv[i];
			int pos = url.find(':');
			targetip = url.substr(0, pos);
			targetport = atoi(url.c_str()+pos+1);
		}
	}
	
	g_pUdpProxyClientPool = new CUdpProxyClientPool();
	g_pUdpProxyClientPool->Init();
	
	g_pUdpServer = CreateUdpServerInstance();
	g_UdpServerDataHandle = new CUdpServerDataHandle();
	g_pUdpServer->SetDataHandle(g_UdpServerDataHandle);
	g_pUdpServer->Start(0, serverport);

	return 0;
}

