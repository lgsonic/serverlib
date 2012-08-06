#ifndef _D_TCPCLIENT_H_
#define _D_TCPCLIENT_H_


#include "iclient.h"
#include "outputbuffer.h"
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


class CTcpClient: public ITcpClient
{
	typedef std::tr1::function<void()> Function_t;
	typedef std::vector<Function_t> FunctionList_t;
public:
	CTcpClient(): m_Socket(0)
	{
		pthread_spin_init(&m_Spinlock, 0);
	}
	virtual ~CTcpClient()
	{
		pthread_spin_destroy(&m_Spinlock);
	}
public:
	void SetDataHandle(ITcpClientDataHandle * pDataHandle) { m_pDataHandle = pDataHandle; }
	bool Start(unsigned int IP, unsigned short Port);
	bool Stop();
	bool Send(const char *pData, int nDataLen);
private:
	bool __Connect();
	bool __Send(COutputBuffer::Pointer pOutputBuffer);
	void __IoCallback(ev::io &watcher, int revents);
	void __AsyncCallback(ev::async &watcher, int revents);
	void __ErrorCallback();
private:
	ITcpClientDataHandle * m_pDataHandle;
	int m_Socket;
	unsigned int m_IP;
	unsigned short m_Port;
	SocketClientData_t m_SocketClient;
	ev::dynamic_loop m_Loop;
	ev::io m_Io;
	ev::async m_Async;
	FunctionList_t m_Functions;
	pthread_spinlock_t m_Spinlock;
	std::queue<COutputBuffer::Pointer> m_WriteQueue;
};


#endif
