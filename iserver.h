#ifndef _D_ISERVER_H_
#define _D_ISERVER_H_

#include "isocket.h"

class ITcpServerDataHandle
{
public:
	virtual ~ITcpServerDataHandle(){}
public: 
	virtual void  OnClientConnected(SocketClientData_t sClient) = 0;
	virtual void  OnClientDisconnected(SocketClientData_t sClient, int nErrorCode) = 0;
	virtual void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nLen) = 0;
};


class IUdpServerDataHandle
{
public:
	virtual ~IUdpServerDataHandle(){}
public:
	virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nLen) = 0;
};


class ITcpServer
{
public:
	virtual ~ITcpServer(){}
public:
	virtual void SetDataHandle(ITcpServerDataHandle * pDataHandle) = 0;
	virtual bool Start(unsigned int IP, unsigned short Port) = 0;
	virtual bool Stop() = 0;
	virtual bool Send(SocketClientData_t sClient, const char *pData, int nDataLen) = 0;
	virtual bool CloseClient(SocketClientData_t sClient) = 0;
	virtual void SetConnectionTimeout(int nTimeout) = 0;
};


class IUdpServer
{
public:
	virtual ~IUdpServer(){}
public:
	virtual void SetDataHandle(IUdpServerDataHandle * pDataHandle) = 0;
	virtual bool Start(unsigned long IP, unsigned short Port) = 0;
	virtual bool Stop() = 0;
	virtual void Send(unsigned long IP, unsigned short Port, const char * pData, int nDataLen) = 0;
};


class ICommonServer: public ITcpServer
{
public:
	virtual ~ICommonServer(){}
public:
	virtual void SetThreadNum(int nThreadNum) = 0;
};


ITcpServer * CreateTcpServerInstance();
IUdpServer * CreateUdpServerInstance();
ICommonServer * CreateCommonServerInstance();


#endif
