#ifndef _D_UDPSERVER_H_
#define _D_UDPSERVER_H_


#include "iserver.h"
#include <ev++.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>


class CUdpServer: public IUdpServer
{
public:
	CUdpServer(){}
	virtual ~CUdpServer(){}
public:
	void SetDataHandle(IUdpServerDataHandle * pDataHandle) { m_pDataHandle = pDataHandle; }
	bool Start(unsigned long IP, unsigned short Port);
	bool Stop();
	void Send(unsigned long IP, unsigned short Port, const char * pData, int nDataLen);
private:
	void __ReadCallback(ev::io &watcher, int revents);
private:
	IUdpServerDataHandle * m_pDataHandle;
	int m_Socket;
	ev::dynamic_loop m_Loop;
	ev::io m_Io;
};


#endif
