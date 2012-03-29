#include "iserver.h"
#include "tcpserver.h"
#include "udpserver.h"


ITcpServer * CreateTcpServerInstance()
{
	return new CTcpServer();
}


IUdpServer * CreateUdpServerInstance()
{
	return new CUdpServer();
}


