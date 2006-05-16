#ifndef OSXTIMERTHREAD_H
#define OSXTIMERTHREAD_H

#include <string>

using namespace std;

class CKaiEngine;


void* testTimer(void *param);

class CTimerThread
{
	private:
		pthread_t		tidInet;
		pthread_attr_t  attrInet;
	public:
				        CTimerThread(CKaiEngine* cParent, unsigned int msec, unsigned int id);
						~CTimerThread();
		bool			m_bActive;
		unsigned int	m_iInterval;
		unsigned int	m_iID; // this timer ID (unique or not)
		CKaiEngine*		m_iParent;
		void	        Start() { m_bActive = true; }
		void	        Stop() { m_bActive = false; }
		void			SetTimer(unsigned int msec) { if(m_iInterval != msec) m_iInterval = msec; }
		void			Terminate();
        unsigned int	ID() { return m_iID; }
};

#endif
