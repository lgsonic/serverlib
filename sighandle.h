#ifndef _D_SIGHANDLE_H_
#define _D_SIGHANDLE_H_


#include <ev++.h>


class CSigHandle
{
public:
	static CSigHandle & GetInstance(ev::dynamic_loop & mainloop)
	{
		static CSigHandle _s(mainloop);
		return _s;
	}
protected:
	CSigHandle(ev::dynamic_loop & mainloop)
	{
		m_Sig.set(mainloop);
		m_Sig.set<&CSigHandle::__SigCallback>();
		m_Sig.start(SIGPIPE);
	}
	~CSigHandle() 
	{
		m_Sig.stop();
	}
private:
	static void __SigCallback(ev::sig &signal, int revents) {}
	ev::sig  m_Sig;
};


#endif
