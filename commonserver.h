#ifndef _D_COMMONSERVER_H_
#define _D_COMMONSERVER_H_


#include "iserver.h"
#include "tcpserver.h"


class CPktHandleThread
{
	typedef std::pair<SocketClientData_t, COutputBuffer::Pointer> PktData_t;
public:
	CPktHandleThread(): m_bThreadRun(false) {}
	~CPktHandleThread()
	{
		Stop();
	}
public:
	void Init(ITcpServerDataHandle * pDataHandle);
	void Stop();
	void AddPkt(const PktData_t & PktData);
	void HandlePkt();
private:
	ITcpServerDataHandle * m_pDataHandle;
	bool m_bThreadRun;
	pthread_t m_ThreadId;
	pthread_mutex_t m_ThreadMutex;
	pthread_cond_t  m_ThreadCond;
	std::queue<PktData_t> m_PktQueue;
};


class CCommonServer: public CTcpServer, public ICommonServer
{
	typedef std::map<SocketClientData_t, std::string> ContentMap_t;
public:
	CCommonServer(): m_nThreadNum(0), m_nCount(0) {}
	virtual ~CCommonServer();
public:
	virtual void SetDataHandle(ITcpServerDataHandle * pDataHandle);
	virtual bool Start(unsigned int IP, unsigned short Port);
	virtual bool Stop();
	virtual bool Send(SocketClientData_t sClient, const char *pData, int nDataLen);
	virtual bool CloseClient(SocketClientData_t sClient);
	virtual void SetConnectionTimeout(int nTimeout);
	virtual void SetThreadNum(int nThreadNum);
public:
	virtual void  OnClientDisconnected(SocketClientData_t sClient, int nErrorCode);
	virtual void  OnClientDataReceived(SocketClientData_t sClient, const char * pData, int nDataLen);
	virtual void  OnClientRecvError(SocketClientData_t sClient, int nErrorCode);
	virtual void  OnClientSendError(SocketClientData_t sClient, int nErrorCode);
	virtual void  OnClientTimeout(SocketClientData_t sClient);
private:
	void __AddContent(const SocketClientData_t & sClient, const char * pData, int nDataLen);
	void __RemoveContent(const SocketClientData_t & sClient);
	void __HandlePkt(const SocketClientData_t & sClient, const char * pData, int nDataLen);
private:	
	int m_nThreadNum;
	long m_nCount;
	CPktHandleThread * m_Threads;
	ContentMap_t m_Contents;
};


#endif
