#include "iclient.h"
#include "tcpclient.h"


ITcpClient * CreateTcpClientInstance()
{
	return new CTcpClient();
}


