#ifdef PLATFORM_macosx_jaguar
#include <string>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "OSXTimerThread.h"
#include "KaiEngine.h"
#include "Kaid.h"

CTimerThread::CTimerThread(CKaiEngine* cParent, unsigned int msec, unsigned int id)
		{
			m_bActive = false;
			m_iInterval	= msec;
			m_iID = id;
			m_iParent = cParent;
			m_iParent->verbositylog(2,"THREAD","Timer thread started...");
			pthread_attr_init(&attrInet);
			pthread_create( &tidInet, &attrInet, testTimer, this);
			pthread_detach(tidInet);
		}

CTimerThread::~CTimerThread()
		{
			m_iParent->verbositylog(2,"THREAD","Timer thread stopped...");
		}

void CTimerThread::Terminate()
		{
			m_bActive = false;
			pthread_kill(tidInet, SIGINT);
		}

void* testTimer(void *param)
		{
			CTimerThread *m_Timer;
			m_Timer = (CTimerThread*)param;
			
			while(1)
			{
				if(m_Timer->m_iInterval > 1)
				{
					unsigned int m_iTick = 0;
					while(m_iTick < m_Timer->m_iInterval)
					{
						usleep(1000000);
						m_iTick += 1000;
					}
				}
				else
				{
					usleep(m_Timer->m_iInterval * 1000);
				}
				if(m_Timer->m_bActive)
					m_Timer->m_iParent->OnTimer(m_Timer->m_iID);
			}
		}

#endif