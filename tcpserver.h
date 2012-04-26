#ifndef _D_TCPSERVER_H_
#define _D_TCPSERVER_H_


#include "iserver.h"
#include <ev++.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <pthread.h>
#include <resolv.h>
#include <errno.h>
#include <queue>
#include <map>
#include <tr1/memory>
#include <tr1/functional>


#define SOCKET_CHANNEL_NUM 		256
#define TCP_CONNECTION_TIMEOUT	600


class CTcpServer;


class COutputBuffer 
{
public:
	typedef std::tr1::shared_ptr<COutputBuffer> Pointer;
public:
	COutputBuffer(const char * szBytes, ssize_t nBytes) 
	{
		m_nPos = 0;
		m_nLen = nBytes;
		m_szData = new char[nBytes];
		memcpy(m_szData, szBytes, nBytes);
	}

	~COutputBuffer() 
	{
		delete [] m_szData;
	}

	char * GetDataPos()
	{
		return m_szData + m_nPos;
	}

	ssize_t GetBytes()
	{
		return m_nLen - m_nPos;
	}
public:
	char * m_szData;
	ssize_t m_nLen;
	ssize_t m_nPos;
};


class CTcpConnection
{
public:
	typedef std::tr1::shared_ptr<CTcpConnection> Pointer;
public:
	CTcpConnection(CTcpServer * pTcpServer, const SocketClientData_t & sClient);
	virtual ~CTcpConnection();
public:
	void Send(COutputBuffer::Pointer pOutputBuffer);
	void Close();
private:
	void __IoCallback(ev::io &watcher, int revents);
	void __TimerCallback(ev::timer &watcher, int revents);
	void __ErrorCallback();
private:
	SocketClientData_t m_SocketClient;
	int m_Socket;
	CTcpServer * m_pTcpServer;
	ev::io m_Io;
	ev::timer m_Timer;
	std::queue<COutputBuffer::Pointer> m_WriteQueue;
	int m_nLasttime;
};


class CSocketInfoChannel
{
	typedef CTcpConnection::Pointer ConnectionPtr_t;
	typedef std::map<SocketClientData_t, ConnectionPtr_t> ConnectionPtrMap_t;
public:
	CSocketInfoChannel() 
	{
		pthread_spin_init(&m_Spinlock, 0);
	}
	~CSocketInfoChannel()
	{
		pthread_spin_destroy(&m_Spinlock);
	}
public:
	void Add(const SocketClientData_t & sClient, const ConnectionPtr_t pConn)
	{
		pthread_spin_lock(&m_Spinlock);

		m_Connections[sClient] = pConn;
		
		pthread_spin_unlock(&m_Spinlock);
	}
	ConnectionPtr_t Get(const SocketClientData_t & sClient)
	{
		pthread_spin_lock(&m_Spinlock);
		
		ConnectionPtr_t pConn;
		ConnectionPtrMap_t::const_iterator it_find = m_Connections.find(sClient);
		if (it_find != m_Connections.end())
		{
			pConn = it_find->second;
		}
		
		pthread_spin_unlock(&m_Spinlock);
		
		return pConn;
	}
	void Remove(const SocketClientData_t & sClient)
	{
		pthread_spin_lock(&m_Spinlock);

		m_Connections.erase(sClient);
		
		pthread_spin_unlock(&m_Spinlock);
	}
private:
	ConnectionPtrMap_t m_Connections;
	pthread_spinlock_t m_Spinlock;
};


class CSocketInfoManager
{
	typedef CTcpConnection::Pointer ConnectionPtr_t;
public:
	CSocketInfoManager(){}
	~CSocketInfoManager(){}
public:
	void Add(const SocketClientData_t & sClient, const ConnectionPtr_t pConn)
	{
		m_Channels[__Hash(sClient)].Add(sClient, pConn);
	}
	ConnectionPtr_t Get(const SocketClientData_t & sClient)
	{
		return m_Channels[__Hash(sClient)].Get(sClient);
	}
	void Remove(const SocketClientData_t & sClient)
	{
		m_Channels[__Hash(sClient)].Remove(sClient);
	}
private:
	int __Hash(const SocketClientData_t & sClient) const
	{
		return (sClient.Socket%SOCKET_CHANNEL_NUM);
	}
private:
	CSocketInfoChannel m_Channels[SOCKET_CHANNEL_NUM];
};


class CTcpServer: public ITcpServer
{
	typedef std::tr1::function<void()> Function_t;
	typedef std::vector<Function_t> FunctionList_t;
public:
	CTcpServer(): m_nTimeout(TCP_CONNECTION_TIMEOUT)
	{
		pthread_spin_init(&m_Spinlock, 0);
	}
	virtual ~CTcpServer()
	{
		pthread_spin_destroy(&m_Spinlock);
	}
public:
	void SetDataHandle(ITcpServerDataHandle * pDataHandle) { m_pDataHandle = pDataHandle; }
	bool Start(unsigned int IP, unsigned short Port);
	bool Stop();
	bool Send(SocketClientData_t sClient, const char *pData, int nDataLen);
	bool CloseClient(SocketClientData_t sClient);
public:
	void SetConnectionTimeout(int nTimeout) { m_nTimeout = nTimeout; }
	int GetConnectionTimeout() const { return m_nTimeout; }
public:
	void  OnClientDisconnected(SocketClientData_t sClient, int nErrorCode);
	void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nLen);
	void  OnClientRecvError(SocketClientData_t sClient, int nErrorCode);
	void  OnClientSendError(SocketClientData_t sClient, int nErrorCode);
	void  OnClientTimeout(SocketClientData_t sClient);
	ev::dynamic_loop & GetLoop() { return m_Loop; }
private:
	bool __Send(SocketClientData_t sClient, COutputBuffer::Pointer pOutputBuffer);
	void __Accept(ev::io &watcher, int revents);
	void __AsyncCallback(ev::async &watcher, int revents);
	static void __SigCallback(ev::sig &signal, int revents);
private:
	CSocketInfoManager m_SocketInfoManager;
	ITcpServerDataHandle * m_pDataHandle;
	int m_nTimeout;
	int m_Socket;
	ev::dynamic_loop m_Loop;
	ev::io m_ListenIo;
	ev::async m_Async;
	static ev::sig  m_Sig;	// sig can only attach one loop
	FunctionList_t m_Functions;
	pthread_spinlock_t m_Spinlock;
};


#endif
