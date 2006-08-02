#ifndef _ORBDGRAM_H_
#define _ORBDGRAM_H_

#include <socketcc.h>
#include <pthreadcc.h>

using namespace std;

class CKaiEngine;
class CConfigFile;

class COrbDgramThread : public ThreadBase
{
	private:
		string		        m_sRequest, m_sAnswer;
		bool		        m_bTerminate;
		CKaiEngine*	        m_cParent;
        CConfigFile*        m_cConf;
        UDPServerSocket*   	m_cListeningSocket;
		UDPServerSocket*	m_cDeepListeningSocket;
		IPAddress			m_cNearestOrb, m_cMyIP, m_cMyDeepIP, m_cIPSource;
		int                 m_iMyPort, m_iMyDeepPort, m_iSourcePort;
		static MutualExclusion mutexProtect;
		void				*game_packet;
	public:
					        COrbDgramThread(CKaiEngine *cParent, CConfigFile* cConf);
					        ~COrbDgramThread();
       	void		        Start();
        void                Stop();
		void				RemakeSocket();
		void				HandleOrbDatagram(string dgram, IPAddress& srchost,int srcport,bool bDeep);
		void                RelayDatagram(string msg, string sAddr, int iPort, bool bDeep);
        void				SendSpeex(void *data, int size, string sAddr, int iPort);
        void				HandleSpeex(void *data, int size, string host, int port);
		void				RelayPacket(void *packet, int packet_size,string sAddr, int iPort, bool bDeep);
		void				RelayArenaPacket(void *packet, int packet_size, 
								string sAddr, int iPort, bool bDeep);
		IPAddress&	        FindNearestOrb();
		void				ScanDeepResolutionServers();
        int                 MyPort() { return m_iMyPort; }
		bool				m_bOrbFound;
        IPAddress&          MyIP() { return m_cMyIP; }
		int				MyDeepPort() { return m_iMyDeepPort; }
		IPAddress&		MyDeepIP() { return m_cMyDeepIP; }
	protected:
		enum		        OrbDgramExceptions { errGetOrbList };
		virtual void *	    Execute();
};

#endif
