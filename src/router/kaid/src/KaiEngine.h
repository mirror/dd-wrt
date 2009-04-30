#ifndef KAIENGINE_H
#define KAIENGINE_H

#include <iostream>
#include <vector>
#include "pcap.h"
#include <pthreadcc.h>
#include <socketcc.h>
#include <sys/time.h>

#ifdef PLATFORM_macosx_jaguar
	#include "OSXTimerThread.h"
#else
	#include "TimerThread.h"
#endif

using namespace std;

class CMessengerItem;
class CArenaItem;
class CPktSnifferThread;
class COrbStreamThread;
class COrbDgramThread;
class CConfigFile;
class CPersistFile;

class CKaiDevice
{
	public:
		CKaiDevice(string sMac, bool AlreadyDetected = false) { m_sMacAddress = sMac; m_bDetected = AlreadyDetected; }
		string	m_sMacAddress;
		bool	m_bDetected;
};

class CKaiEngine:public ThreadBase
{
	private:
		// Objects
		UDPServerSocket * m_cListeningSocket;
		COrbDgramThread *m_cUOrb;
		COrbStreamThread *m_cTOrb;
		CPktSnifferThread *m_cSniffer;
		CConfigFile *m_cConf;
		vector <CTimerThread*> m_cTimers;
		// Others
		u_char m_ucaDHCPOffer[400];
		u_char m_ucaDHCPAck[400];
		static MutualExclusion mutexProtect;
		string m_sRequest, m_sAnswer;
		IPAddress m_cIPSource, m_cUIAddr;
		int m_iSourcePort, m_iUIPort;
		unsigned int m_iMessengerCurrentPing, m_iArenaCurrentPing;
		unsigned int m_iOutgoingPktCnt, m_iIncomingPktCnt;
		unsigned int m_iCapturedPktCnt, m_iReceivedPktCnt;
		bool m_bTerminate, m_bUseIPv6, m_bFoundDevice,m_bLockedPcap;
		int m_iUpBroadcast;
		int m_iUpBroadcastCount;
		int m_iDownBroadcast;
		int m_iDownBroadcastCount;
		int m_iUpDirect;
		int m_iUpDirectCount;
		int m_iDownDirect;
		int m_iDownDirectCount;
		static Condition condSignal;
	protected:
		enum ThreadExceptions	{ errFatalError, errTooManyThreads, errClientError, errServerDown };
		enum TimerControl { Start, Stop, ChangeTiming };
		virtual void *Execute ();
		void TimerCtl(const unsigned int TimerID, TimerControl Operation, int data = 0);
	public:
		CKaiEngine (CConfigFile * cConf);
		string m_sStoredPrivs;
		virtual ~ CKaiEngine ();

		CPersistFile *Persist;
		void verbositylog(int level, string sSection, string sMessage);
		string m_strStatusCache, m_sCurrentArena, m_sLastVectorChange;
		string m_sStoredAdmins, m_sChatMode, m_sReachable;
		string m_sAdapterName, m_sOurOrbName, m_sUICaps;
		bool m_bAttached, m_bShuttingDown;
		
		unsigned int m_iUpEngine, m_iDownEngine, m_iDownEngineCount,
			m_iUpEngineCount;
		unsigned int m_iUpChat, m_iDownChat, m_iUpChatCount, m_iDownChatCount;
		unsigned int m_iUpOrb, m_iDownOrb, m_iUpOrbCount, m_iDownOrbCount;
		unsigned int m_iBusyCounter;
		bool m_bBusy;
		unsigned int m_iArenaStatus, m_iArenaMyPlayers, m_iActiveConnection;
		unsigned int m_iSpeexReq;
		unsigned int m_iRoutingMode, m_iBusyLevel;
		
		vector <CMessengerItem*> m_cMessengerContacts;
		vector <CArenaItem*> m_cArenaContacts;
		vector <CKaiDevice*> m_cDevices;
		vector <CKaiDevice*> m_cFailedDevices;
		
		bool	m_bSeenAllDevices;
		vector <string> m_sVectors, m_svChatUsers, m_sInvites;
		struct timeval m_tvLastVectorChange, m_tvLastDiscover, m_tvLastUserInteraction;
		
		void HandleDHCPQuery (string console, string srcmac, const u_char * packet, int packet_size, bool incoming, string sender_ip, int sender_port);
		void FailedDevice(string device);
		void CheckArenaIdle();
		void SendPresteering();
		void StartServer ();
		void RemoveFailedDevice (string device);
		void RemoveLocalDevice (string device);		
		void HandleUIRequest ();
		void SendClientResponse (string resp);
		void LoggedIn (string usr);
		void LoggedOut ();
		void AddChatUser (string strRoom, string strUser);
		void RemoveChatUser (string strRoom, string strUser);
		void SendClientStatus (string stat);
		void GetContacts ();
		void AddVector (string vector, string type);
		void AddArenaContact (string name, string ip, int port,
						string deepip, int deepport);
		void RemoveArenaContact (string contact);
		void RemoveVector (string strVector);
		CMessengerItem *FindUser (const string & user);
		CMessengerItem *FindUser (string sIP, int iPort);
		CArenaItem *FindArenaUser (const string & user);
		CArenaItem *FindArenaUser (string sIP, int iPort);
		void SetOfflineContact (const string & user);
		void SetOnlineContact (string contact, string ipaddress,
						int port, string deepip, int deepport);
		void DeleteContact (string contact);
		void AddLocalDevice(string mac);
		void OnTimer (unsigned int id);
		int MyPort ();
		IPAddress & MyIP ();
		int MyDeepPort ();
		IPAddress & MyDeepIP ();
		void RequeryMessengerConnections ();
		void CleanMessengerContacts ();
		CArenaItem* WhoHasDevice (string device);
		void HandleEthernetFrame (const u_char * packet,
						int packet_size, bool incoming =
						false, string sender_ip =
						"", int sender_port = 0);
		void HandleFrame (string console, string dstmac, string srcmac,
						const u_char * packet, int packet_size,
						bool incoming = false, string sender_ip =
						"", int sender_port = 0);
		int InjectPacket (const u_char * packet, int packet_size);
		void SendLocalConsoleData ();
		void Shutdown ();
		void ReAttach ();
		void RequeryArenaConnections ();
		void StartTimer(const unsigned int TimerID);
		void StopTimer(const unsigned int TimerID);
		void CheckBusy();
		void AddFailedDevice(string device);
		void CalcTraffic();
		void CheckIfSpeexNeeded();
		void CleanOldInvitation(string sFrom);
		void DetectedDevice(string);		
		void HandleSpeexData(void *speex_data, int speex_size, bool incoming = false, string sWho = "");
		int	 FindLocalDevice(string device);
		int	 FindFailedDevice(string device);
};
#endif
