#ifndef _D_OUTPUTBUFFER_H_
#define _D_OUTPUTBUFFER_H_


#include <string.h>
#include <tr1/memory>


class COutputBuffer 
{
public:
	typedef std::tr1::shared_ptr<COutputBuffer> Pointer;
public:
	COutputBuffer(const char * szBytes, ssize_t nBytes) 
	{
		m_nPos = 0;
		m_nLen = nBytes;
		m_szData = new char[nBytes];
		memcpy(m_szData, szBytes, nBytes);
	}

	~COutputBuffer() 
	{
		delete [] m_szData;
	}

	char * GetDataPos()
	{
		return m_szData + m_nPos;
	}

	ssize_t GetBytes()
	{
		return m_nLen - m_nPos;
	}
public:
	char * m_szData;
	ssize_t m_nLen;
	ssize_t m_nPos;
};


#endif
