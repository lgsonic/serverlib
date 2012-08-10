#include "udpclient.h"


bool CUdpClient::Start(unsigned int IP, unsigned short Port)
{
	m_IP = IP;
	m_Port = Port;
	
	m_Socket = socket(PF_INET, SOCK_DGRAM, 0);
	
	fcntl(m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK);
	
	m_Io.set(m_Loop);
	m_Io.set<CUdpClient, &CUdpClient::__ReadCallback>(this);
	m_Io.start(m_Socket, ev::READ);

	m_Loop.loop();
	
	return true;
}

bool CUdpClient::Stop()
{
	if (m_Socket > 0)
	{
		close(m_Socket);
		m_Socket = 0;
	}

	m_Io.stop();
	m_Loop.unloop();
	
	return true;
}

void CUdpClient::Send(const char * pData, int nDataLen)
{
	if (m_Socket > 0)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(m_Port);
		addr.sin_addr.s_addr = htonl(m_IP);
		sendto(m_Socket, pData, nDataLen, 0, (sockaddr*)&addr, sizeof(struct sockaddr_in));
	}
}

void CUdpClient::__ReadCallback(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) 
	{
		return;
	}

	char szBuffer[1500];
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	ssize_t nRead = recvfrom(watcher.fd, szBuffer, sizeof(szBuffer), 0, (sockaddr*)&addr, &addrlen);

	if (nRead > 0)
	{
		m_pDataHandle->OnDataReceived(ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port ), szBuffer, nRead);
	}
}


