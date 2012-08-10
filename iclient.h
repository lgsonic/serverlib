#ifndef _D_ICLIENT_H_
#define _D_ICLIENT_H_

#include "isocket.h"

class ITcpClientDataHandle
{
public:
	virtual ~ITcpClientDataHandle(){}
public: 
	virtual void  OnConnected(SocketClientData_t sClient) = 0;
	virtual void  OnDisconnected(SocketClientData_t sClient, int nErrorCode) = 0;
	virtual void  OnDataReceived(SocketClientData_t sClient, const char * pData, int nLen) = 0;
	virtual void  OnSendError(SocketClientData_t sClient, int nErrorCode) = 0;
	virtual void  OnRecvError(SocketClientData_t sClient, int nErrorCode) = 0;
};


class IUdpClientDataHandle
{
public:
	virtual ~IUdpClientDataHandle(){}
public:
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nLen) = 0;
};


class ITcpClient
{
public:
	virtual ~ITcpClient(){}
public:
	virtual void SetDataHandle(ITcpClientDataHandle * pDataHandle) = 0;
	virtual bool Start(unsigned int IP, unsigned short Port) = 0;
	virtual bool Stop() = 0;
	virtual bool Send(const char *pData, int nDataLen) = 0;
};


class IUdpClient
{
public:
	virtual ~IUdpClient(){}
public:
	virtual void SetDataHandle(IUdpClientDataHandle * pDataHandle) = 0;
	virtual bool Start(unsigned int IP, unsigned short Port) = 0;
	virtual bool Stop() = 0;
	virtual void Send(const char * pData, int nDataLen) = 0;
};


ITcpClient * CreateTcpClientInstance();
IUdpClient * CreateUdpClientInstance();


#endif
