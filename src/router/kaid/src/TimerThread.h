#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#include <string>
#include <pthreadcc.h>

using namespace std;

class CKaiEngine;

class CTimerThread: ThreadBase
{
	private:
		unsigned int	m_iInterval, m_iTick;
		bool			m_bActive, m_bTerminate;
		unsigned int	m_iID; // this timer ID (unique or not)
		CKaiEngine*		m_cParent;
	public:
				        CTimerThread(CKaiEngine* cParent, unsigned int msec, unsigned int id);
				        ~CTimerThread();
		void*	        Execute();
		void	        Start() { m_bActive = true; }
		void	        Stop() { m_bActive = false; }
		void			SetTimer(unsigned int msec) { if(m_iInterval != msec) m_iInterval = msec; }
		void			Terminate() { m_bTerminate = true; m_bActive = false;}
        unsigned int    ID() { return m_iID; }
};

#endif
