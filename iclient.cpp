#include "iclient.h"
#include "tcpclient.h"
#include "udpclient.h"


ITcpClient * CreateTcpClientInstance()
{
	return new CTcpClient();
}


IUdpClient * CreateUdpClientInstance()
{
	return new CUdpClient();
}


