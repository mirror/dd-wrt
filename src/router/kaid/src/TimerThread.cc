#ifndef PLATFORM_macosx_jaguar
#include <string>

#include <unistd.h>
#include <pthreadcc.h>
#include "TimerThread.h"
#include "KaiEngine.h"
#include "Kaid.h"


CTimerThread::CTimerThread(CKaiEngine* cParent, unsigned int msec, unsigned int id)
{
	m_bActive = false;
	m_bTerminate = false;
	m_iInterval	= msec;
	m_iID = id;
	m_cParent = cParent;
	LaunchThread();
}

CTimerThread::~CTimerThread()
{
	m_bActive = false;
	// WaitThread();
}

void* CTimerThread::Execute()
{
	try
	{
		while(!m_bTerminate) {
			if(m_iInterval > 1)
			{
				m_iTick = 0;
				while( (m_iTick < m_iInterval) && !m_bTerminate)
				{
					usleep(1000000);
					m_iTick += 1000;
				}
			}
			else
			{
				usleep(m_iInterval * 1000);
			}
			if(m_bActive)
				m_cParent->OnTimer(m_iID);
		}
	}
	catch(...)
	{
		return NULL;
	}
	return NULL;
}
#endif
