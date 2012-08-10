#ifndef _D_UDPCLIENT_H_
#define _D_UDPCLIENT_H_


#include "iclient.h"
#include <ev++.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>


class CUdpClient: public IUdpClient
{
public:
	CUdpClient(){}
	virtual ~CUdpClient(){}
public:
	void SetDataHandle(IUdpClientDataHandle * pDataHandle) { m_pDataHandle = pDataHandle; }
	bool Start(unsigned int IP, unsigned short Port);
	bool Stop();
	void Send(const char * pData, int nDataLen);
private:
	void __ReadCallback(ev::io &watcher, int revents);
private:
	IUdpClientDataHandle * m_pDataHandle;
	unsigned int m_IP;
	unsigned short m_Port;
	int m_Socket;
	ev::dynamic_loop m_Loop;
	ev::io m_Io;
};


#endif
