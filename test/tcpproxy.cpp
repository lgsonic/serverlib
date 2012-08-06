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

class ITcpProxyClientPool
{
public:
	virtual void Init() = 0;
	virtual void Send(const SocketClientData_t & sClient, const char *pData, int nDataLen) = 0;
};

ITcpProxyClientPool * g_pTcpProxyClientPool;
ITcpServer * g_pTcpServer;
ITcpServerDataHandle *g_TcpServerDataHandle;

class CServerContentManager
{
private:
	typedef std::map<SocketClientData_t, std::string> userRequestMap_t;
public:
	void Add(const SocketClientData_t & sClient ,const char * pData ,int nDataLen)
	{
		std::string & strData = m_userRequests[sClient];
		strData.insert(strData.end(), pData, pData+nDataLen);

		while(strData.size() > 4)
		{
			int nPktLen = *(int*)&strData[0];
			if((int)strData.size() >= (nPktLen + 4))
			{
				HandlePkt(sClient, &strData[4], nPktLen);
				strData = strData.substr(nPktLen+4);
			}
			else
			{
				break;
			}
		}

	}
	void Remove(const SocketClientData_t & sClient)
	{
		m_userRequests.erase(sClient);
	}
	virtual void HandlePkt(const SocketClientData_t & sClient ,const char * pData ,int nDataLen)
	{
		std::string strSend;
		int nPktLen = nDataLen + sizeof(SocketClientData_t);
		strSend.insert(strSend.end(), (char*)&nPktLen, (char*)&nPktLen+4);
		strSend.insert(strSend.end(), (char*)&sClient, (char*)&sClient + sizeof(SocketClientData_t));
		strSend.insert(strSend.end(), pData, pData + nDataLen);
		
		g_pTcpProxyClientPool->Send(sClient, strSend.c_str(), strSend.size());
	}
private:
	userRequestMap_t m_userRequests;
};

class CClientContentManager: public CServerContentManager
{
public:
	virtual void HandlePkt(const SocketClientData_t & sClient ,const char * pData ,int nDataLen)
	{
		if(nDataLen <= (int)sizeof(SocketClientData_t))
		{
			return;
		}

		SocketClientData_t sTarget = *(SocketClientData_t*)pData;
		std::string strSend;
		int nPktLen = nDataLen - sizeof(SocketClientData_t);
		strSend.insert(strSend.end(), (char*)&nPktLen, (char*)&nPktLen+4);
		strSend.insert(strSend.end(), pData + sizeof(SocketClientData_t), pData + sizeof(SocketClientData_t) + nPktLen);
		
		g_pTcpServer->Send(sTarget, strSend.c_str(), strSend.size());
	}
};

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
		m_ContentManager.Remove(sClient);
	}

	virtual void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nLen)
	{
		m_ContentManager.Add(sClient, pData, nLen);
	}
private:
	CServerContentManager m_ContentManager;
};

class CTcpClientDataHandle: public ITcpClientDataHandle
{
public:
	virtual ~CTcpClientDataHandle(){}
public: 
	virtual void  OnConnected(SocketClientData_t sClient)
	{
	}

	virtual void  OnDisconnected(SocketClientData_t sClient, int nErrorCode)
	{
		m_ContentManager.Remove(sClient);
	}
	virtual void  OnSendError(SocketClientData_t sClient, int nErrorCode)
	{
		m_ContentManager.Remove(sClient);
	}
	virtual void  OnRecvError(SocketClientData_t sClient, int nErrorCode)
	{
		m_ContentManager.Remove(sClient);
	}
	virtual void  OnDataReceived(SocketClientData_t sClient, const char * pData, int nLen)
	{
		m_ContentManager.Add(sClient, pData, nLen);
	}
private:
	CClientContentManager m_ContentManager;
};

static void* run_thread_client(void * param)
{
	ITcpClient * pTcpClient = (ITcpClient *)param;
	pTcpClient->Start(ntohl(inet_addr(targetip.c_str())), targetport);
	return 0;
}

class CTcpProxyClient
{
public:
	void Init()
	{
		m_pTcpClient = CreateTcpClientInstance();
		m_pTcpClient->SetDataHandle(&m_TcpClientDataHandle);
		pthread_attr_t thread_attr_client;
		pthread_attr_init(&thread_attr_client);
		pthread_t thread_id_client;
		pthread_create(&thread_id_client, &thread_attr_client, &run_thread_client, m_pTcpClient);
	}
	void Send(const char *pData, int nDataLen)
	{
		m_pTcpClient->Send(pData, nDataLen);
	}
private:
	ITcpClient * m_pTcpClient;
	CTcpClientDataHandle m_TcpClientDataHandle;
};

class CTcpProxyClientPool: public ITcpProxyClientPool
{
public:
	void Init()
	{
		m_TcpProxyClients = new CTcpProxyClient[concurrency];
		for(int i = 0; i < concurrency; ++i)
		{
			m_TcpProxyClients[i].Init();
		}
	}
	void Send(const SocketClientData_t & sClient, const char *pData, int nDataLen)
	{
		m_TcpProxyClients[__Hash(sClient)].Send(pData, nDataLen);
	}
private:
	int __Hash(const SocketClientData_t & sClient) const
	{
		return (sClient.Socket%concurrency);
	}
	
	CTcpProxyClient * m_TcpProxyClients;
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
	
	g_pTcpProxyClientPool = new CTcpProxyClientPool();
	g_pTcpProxyClientPool->Init();
	
	g_pTcpServer = CreateTcpServerInstance();
	g_TcpServerDataHandle = new CTcpServerDataHandle();
	g_pTcpServer->SetDataHandle(g_TcpServerDataHandle);
	g_pTcpServer->Start(0, serverport);

	return 0;
}

