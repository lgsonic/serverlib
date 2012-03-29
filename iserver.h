#ifndef _D_ISERVER_H_
#define _D_ISERVER_H_


struct SocketClientData_t
{
public:
	unsigned int IP;
	unsigned short Port;
	int Socket;
public:
	bool operator<(const SocketClientData_t & info) const
	{
		if(IP < info.IP)
		{
			return true;
		}
		else if(IP == info.IP)
		{
			if(Port < info.Port)
			{
				return true;
			}
			else if(Port == info.Port)
			{
				return (Socket < info.Socket);
			}
		}
		return false;
	}
	bool operator==(const SocketClientData_t & info) const
	{
		return (IP == info.IP)&&(Port == info.Port)&&(Socket == info.Socket);
	}
};


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


ITcpServer * CreateTcpServerInstance();
IUdpServer * CreateUdpServerInstance();


#endif
