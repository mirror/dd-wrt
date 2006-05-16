#ifndef _ORBSTREAM_H_
#define _ORBSTREAM_H_

#include <socketcc.h>
#include <pthreadcc.h>

#define ORB_DEFAULT_PORT 34525

using namespace std;

class CKaiEngine;
class CConfigFile;

class COrbStreamThread : public ThreadBase
{
	private:
		string				m_sRequest, m_sAnswer, m_sBuffer;
		bool				m_bTerminate;
		CKaiEngine*         m_cParent;
        CConfigFile*        m_cConf;
		TCPClientSocket*    m_cOrbClient;		
		void				SendClientResponse(string resp);
	public:
							COrbStreamThread(CKaiEngine *cParent, CConfigFile* cConf);
							~COrbStreamThread();
       	void               	Start(IPAddress& OrbAddr);
        void                Stop();
		void				HandleOrbMsg(string sRequest);
        void                Deliver(string contact, char opcode, string username,
                                string myip, int myport, string deepip, int deepport);
		void				SendStream(string msg);
		bool				m_bAuthenticated, m_bConnected;
		void				SendOrbAuth();
	protected:
		enum                ThreadExceptions { errFatalError, errClientError, errServerDown };
		virtual void *     	Execute();
};

#endif
