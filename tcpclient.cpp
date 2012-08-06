#include "tcpclient.h"


#define WRITE_QUEUE_MAX_SIZE	1000000

static void __SetNonblock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

static void __SetReuseaddr(int fd)
{
	int reuse = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}

static void __SetNodelay(int fd)
{
	int nodelay = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
}

bool CTcpClient::Start(unsigned int IP, unsigned short Port)
{
	m_IP = IP;
	m_Port = Port;

	m_Io.set(m_Loop);
	m_Async.set(m_Loop);

	m_Io.set<CTcpClient, &CTcpClient::__IoCallback>(this);

	m_Async.set<CTcpClient, &CTcpClient::__AsyncCallback>(this);
	m_Async.start();
	m_Async.send();

	if (!__Connect())
	{
		return false;
	}

	m_Loop.loop();

	return true;
}

bool CTcpClient::Stop()
{
	shutdown(m_Socket, SHUT_RDWR);
	close(m_Socket);

	m_Io.stop();
	m_Async.stop();
	m_Loop.unloop();

	return true;
}

bool CTcpClient::Send(const char *pData, int nDataLen)
{
	pthread_spin_lock(&m_Spinlock);

	COutputBuffer::Pointer pOutputBuffer(new COutputBuffer(pData, nDataLen));
	m_Functions.push_back(std::tr1::bind(&CTcpClient::__Send, this, pOutputBuffer));
	m_Async.send();

	pthread_spin_unlock(&m_Spinlock);

	return true;
}

bool CTcpClient::__Connect()
{
	if (m_Socket > 0)
	{
		close(m_Socket);
		m_Socket = 0;
	}
	
	m_Socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_Socket <= 0)
	{
		return false;
	}

	m_SocketClient.IP = m_IP;
	m_SocketClient.Port = m_Port;
	m_SocketClient.Socket = m_Socket;

	__SetNonblock(m_Socket);
	__SetReuseaddr(m_Socket);
	__SetNodelay(m_Socket);

	fcntl(m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK);
	int reuse = 1;
	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
	struct linger slinger = {1, 0};   
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&slinger, sizeof(slinger)); 

	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port   = htons(m_Port);
	sock_addr.sin_addr.s_addr = htonl(m_IP);
	connect(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));

	m_Io.stop();
	m_Io.start(m_Socket, ev::READ|ev::WRITE);

	return true;
}

bool CTcpClient::__Send(COutputBuffer::Pointer pOutputBuffer)
{
	if (m_WriteQueue.size() > WRITE_QUEUE_MAX_SIZE)
	{
		m_pDataHandle->OnSendError(m_SocketClient, 0);
		return false;
	}
	
	m_WriteQueue.push(pOutputBuffer);
	
	m_Io.set(ev::READ|ev::WRITE);

	return true;
}

void CTcpClient::__AsyncCallback(ev::async &watcher, int revents)
{
	FunctionList_t functions;
	
	{
		pthread_spin_lock(&m_Spinlock);

		functions.swap(m_Functions);

		pthread_spin_unlock(&m_Spinlock);
	}  
	
	for (size_t i = 0; i < functions.size(); ++i)  
	{    
		functions[i]();  
	}
}

void CTcpClient::__IoCallback(ev::io &watcher, int revents) 
{
	if (EV_ERROR & revents) 
	{
		__ErrorCallback();
		return;
	}

	if (revents & EV_READ)
	{
		char szBuffer[4096];

		ssize_t nRead = recv(watcher.fd, szBuffer, sizeof(szBuffer), 0);

		if (nRead == 0) 
		{
			m_pDataHandle->OnDisconnected(m_SocketClient, 0);
			sleep(1);
			__Connect();
			return;
		} 
		else if (nRead < 0) 
		{
			if (errno == EAGAIN)
			{
				// ignore	
			}
			else
			{
				m_pDataHandle->OnRecvError(m_SocketClient, errno);
				sleep(1);
				__Connect();
				return;
			}
		}
		else
		{
			m_pDataHandle->OnDataReceived(m_SocketClient, szBuffer, nRead);
		}
	}

	if (revents & EV_WRITE)
	{
		if (m_WriteQueue.empty()) 
		{
			m_Io.set(ev::READ);
			return;
		}

		COutputBuffer::Pointer pBuffer = m_WriteQueue.front();

		ssize_t nWritten = write(watcher.fd, pBuffer->GetDataPos(), pBuffer->GetBytes());
		if (nWritten < 0) 
		{
			if (errno == EAGAIN)
			{
				// ignore	
				return;
			}
			else
			{
				m_pDataHandle->OnSendError(m_SocketClient, errno);
				sleep(1);
				__Connect();
				return;
			}
		}

		pBuffer->m_nPos += nWritten;
		if (pBuffer->GetBytes() == 0) 
		{
			m_WriteQueue.pop();
		}

		if (m_WriteQueue.empty()) 
		{
			m_Io.set(ev::READ);
		}
	}
}

void CTcpClient::__ErrorCallback()
{
	m_pDataHandle->OnDisconnected(m_SocketClient, errno);
	sleep(1);
	__Connect();
}


