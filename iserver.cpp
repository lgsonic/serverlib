#include "iserver.h"
#include "tcpserver.h"
#include "udpserver.h"
#include "commonserver.h"


ITcpServer * CreateTcpServerInstance()
{
	return new CTcpServer();
}


IUdpServer * CreateUdpServerInstance()
{
	return new CUdpServer();
}


ICommonServer * CreateCommonServerInstance()
{
	return new CCommonServer();
}

