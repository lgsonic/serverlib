#include "../commonserver.h"
#include <sys/time.h>


#define MAX_THREAD_NUM	64


static void * __Thread_Run(void *arg);

void CPktHandleThread::Init(ITcpServerDataHandle * pDataHandle)
{
	m_pDataHandle = pDataHandle;

	m_bThreadRun = true;
	pthread_mutex_init(&m_ThreadMutex, NULL);
	pthread_cond_init(&m_ThreadCond, NULL);
	pthread_create(&m_ThreadId, NULL, __Thread_Run, this);
}

void CPktHandleThread::Stop()
{
	if (m_bThreadRun)
	{
		m_bThreadRun = false;
		pthread_join(m_ThreadId, NULL);
	}
}

void CPktHandleThread::AddPkt(const PktData_t & PktData)
{
	pthread_mutex_lock(&m_ThreadMutex);
	m_PktQueue.push(PktData);
	pthread_cond_signal(&m_ThreadCond);
	pthread_mutex_unlock(&m_ThreadMutex);
}

void CPktHandleThread::HandlePkt()
{
	while (m_bThreadRun)
	{
		int nSize = 0;
		{
			pthread_mutex_lock(&m_ThreadMutex);
			nSize = m_PktQueue.size();
			pthread_mutex_unlock(&m_ThreadMutex);
			
			while ((nSize == 0) && m_bThreadRun)
			{
				pthread_mutex_lock(&m_ThreadMutex);
				struct timeval nowtime; 
				gettimeofday(&nowtime, NULL); 
				struct timespec abstime;
				abstime.tv_nsec = nowtime.tv_usec * 1000; 
				abstime.tv_sec = nowtime.tv_sec + 1;
				pthread_cond_timedwait(&m_ThreadCond, &m_ThreadMutex, &abstime);
				nSize = m_PktQueue.size();
				pthread_mutex_unlock(&m_ThreadMutex);
			}
		}

		while (nSize-- > 0)
		{
			pthread_mutex_lock(&m_ThreadMutex);
			PktData_t PktData = m_PktQueue.front();
			m_PktQueue.pop();
			pthread_mutex_unlock(&m_ThreadMutex);

			m_pDataHandle->OnClientDataReceived(PktData.first, PktData.second->GetDataPos(), PktData.second->GetBytes());
		}		
	}

	pthread_exit(NULL);
}

void * __Thread_Run(void *arg)
{
	CPktHandleThread * pPktHandleThread = (CPktHandleThread *)arg;
	pPktHandleThread->HandlePkt();
	return 0;
}


CCommonServer::~CCommonServer()
{
	if (m_nThreadNum > 0)
	{
		delete [] m_Threads;
	}
}

void CCommonServer::SetDataHandle(ITcpServerDataHandle * pDataHandle)
{
	CTcpServer::SetDataHandle(pDataHandle);
}

bool CCommonServer::Start(unsigned int IP, unsigned short Port)
{
	return CTcpServer::Start(IP, Port);
}

bool CCommonServer::Stop()
{
	return CTcpServer::Stop();
}

bool CCommonServer::Send(SocketClientData_t sClient, const char *pData, int nDataLen)
{
	return CTcpServer::Send(sClient, pData, nDataLen);
}

bool CCommonServer::CloseClient(SocketClientData_t sClient)
{
	return CTcpServer::CloseClient(sClient);
}

void CCommonServer::SetConnectionTimeout(int nTimeout)
{
	CTcpServer::SetConnectionTimeout(nTimeout);
}

void CCommonServer::SetThreadNum(int nThreadNum)
{
	if ((nThreadNum <= 0) || (nThreadNum > MAX_THREAD_NUM))
	{
		return;
	}
	
	m_nThreadNum = nThreadNum;
	m_Threads = new CPktHandleThread[m_nThreadNum]();
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_Threads[i].Init(m_pDataHandle);
	}
}

void CCommonServer::OnClientDisconnected(SocketClientData_t sClient, int nErrorCode)
{
	CTcpServer::OnClientDisconnected(sClient, nErrorCode);
	
	__RemoveContent(sClient);
}

void CCommonServer::OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nDataLen)
{
	__AddContent(sClient, pData, nDataLen);
}

void CCommonServer::OnClientRecvError(SocketClientData_t sClient, int nErrorCode)
{
	CTcpServer::OnClientRecvError(sClient, nErrorCode);

	__RemoveContent(sClient);
}

void CCommonServer::OnClientSendError(SocketClientData_t sClient, int nErrorCode)
{
	CTcpServer::OnClientSendError(sClient, nErrorCode);

	__RemoveContent(sClient);
}

void CCommonServer::OnClientTimeout(SocketClientData_t sClient)
{
	CTcpServer::OnClientTimeout(sClient);

	__RemoveContent(sClient);
}

void CCommonServer::__AddContent(const SocketClientData_t & sClient, const char * pData, int nDataLen)
{
	std::string & strData = m_Contents[sClient];
	strData.insert(strData.end(), pData, pData+nDataLen);

	while (strData.size() > 4)
	{
		int nPktLen = *(int*)&strData[0];
		if(nPktLen <= 0)	//abnormal
		{
			strData.clear();
			break;
		}
		if ((int)strData.size() >= (nPktLen + 4))
		{
			__HandlePkt(sClient, strData.c_str(), nPktLen + 4);
			
			strData = strData.substr(nPktLen+4);
		}
		else
		{
			break;
		}
	}
}

void CCommonServer::__RemoveContent(const SocketClientData_t & sClient)
{
	m_Contents.erase(sClient);
}

void CCommonServer::__HandlePkt(const SocketClientData_t & sClient, const char * pData, int nDataLen)
{
	if (m_nThreadNum == 0)
	{
		m_pDataHandle->OnClientDataReceived(sClient, pData, nDataLen);
	}
	else
	{
		COutputBuffer::Pointer pOutputBuffer(new COutputBuffer(pData, nDataLen));
		m_Threads[((unsigned int)__sync_fetch_and_add(&m_nCount, 1))%m_nThreadNum].AddPkt(std::make_pair(sClient, pOutputBuffer));
	}
}


