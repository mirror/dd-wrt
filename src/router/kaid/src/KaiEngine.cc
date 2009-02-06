// 

#include <iostream>
#include <string>
#include <string.h>
#include <iterator>
#include <vector>

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h>
#include <pwd.h>
#ifdef PLATFORM_freebsd
#    include <login_cap.h>
#    include <netinet/in.h>
#endif

#ifdef PLATFORM_bsd
#    include <netinet/in_systm.h>
#    include <netinet/if_ether.h>
#    include <netinet/ip.h>
#else
#    include <netinet/ether.h>
#endif

#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#include "libpcap/pcap.h"
#include "libpcap/pcap-bpf.h"
#include "pthreadcc.h"
#include "socketcc.h"

#include "Kaid.h"
#include "KaiItem.h"
#ifdef PLATFORM_macosx_jaguar
	#include "OSXTimerThread.h"
#else
	#include "TimerThread.h"
#endif
#include "KaiEngine.h"
#include "OrbStreamThread.h"
#include "OrbDgramThread.h"
#include "PktSnifferThread.h"
#include "ConfigFile.h"
#include "PersistFile.h"
#include "Api.h"
#include "StrUtils.h"

#define	MESSENGER_PING_TIMER	1
#define MESSENGER_CLEAN_TIMER 	2
#define	ARENA_BUSY_TIMER		3
#define	ARENA_PING_TIMER		4
#define TRAFFIC_TIMER           5
#define PRESTEERING_TIMER		6
#define ARENA_IDLE_TIMER		7
#define ORB_REQUERY_TIMER		8

using namespace std;

MutualExclusion
	CKaiEngine::mutexProtect;
Condition
	CKaiEngine::condSignal;

int
CKaiEngine::MyPort ()
{
	return m_cUOrb->MyPort ();
}

IPAddress & CKaiEngine::MyIP ()
{
	return m_cUOrb->MyIP ();
}
IPAddress & CKaiEngine::MyDeepIP ()
{
	return m_cUOrb->MyDeepIP ();
}
int CKaiEngine::MyDeepPort()
{
	return m_cUOrb->MyDeepPort();
}

void
CKaiEngine::StartServer ()
{
	m_bTerminate = false;
	m_cSniffer = new CPktSnifferThread (this, m_cConf);
	
	// After we have the networking set up, we no longer need
	// the privileges that we are started with - ON FREEBSD ONLY!! (?)
#if defined(PLATFORM_freebsd)
	
	if(! m_cConf->User.empty()) {
		const struct passwd *pwd = getpwnam(m_cConf->User.c_str());
		if(pwd == 0) {
			string err = "Unknown switch-to User: ";
			err += m_cConf->User;
			debuglog("KAID",err);
			throw ThreadException(ThreadException::errUnknown);
		}

		login_cap_t *lc = login_getpwclass(pwd);
		if(lc == 0 ||
                   setusercontext(lc, pwd, pwd->pw_uid, LOGIN_SETALL) != 0) {
			string err = "Unable to switch to User: ";
			err += m_cConf->User + "...";
			err += ": ";
			err += strerror(errno);
			debuglog("KAID",err);
			throw ThreadException(ThreadException::errUnknown);
		}
	}
	
#endif

	m_cTOrb = new COrbStreamThread (this, m_cConf);
	m_cUOrb = new COrbDgramThread (this, m_cConf);
	
	LaunchThread ();
}

CKaiEngine::CKaiEngine (CConfigFile * cConf)
{
	m_cConf = cConf;
	verbositylog(2,"THREAD","Engine thread started...");
	CTimerThread *newTimer;
	
	m_iMessengerCurrentPing=0;
	m_sStoredPrivs="";
	m_sOurOrbName = "";
	m_sReachable = "Not yet";
	m_bAttached = false;
	m_bShuttingDown = false;
	m_iBusyCounter = 0;
	m_bBusy = false;
	m_bLockedPcap = false;
	m_sAdapterName = m_cConf->SniffDevice;
	m_iArenaStatus = 1;
	m_iArenaMyPlayers = 1;
	m_sLastVectorChange = "";
	m_iSpeexReq = 0;
	m_sUICaps = "";
	m_iRoutingMode = 0;
	m_iBusyLevel = 3000;
	m_iUIPort = 0;
	m_bSeenAllDevices = false;
	m_bFoundDevice = false;
    m_iUpBroadcast = 0;
	m_iUpBroadcastCount = 0;
    m_iDownBroadcast = 0;
	m_iDownBroadcastCount = 0;
    m_iUpDirect = 0;
	m_iUpDirectCount = 0;
    m_iDownDirect = 0;
	m_iDownDirectCount = 0;
	
	bzero (&m_tvLastDiscover, sizeof (m_tvLastDiscover));
	m_tvLastUserInteraction=Now();
	
	// Create UI socket
	try
	{
		if( m_cConf->UIIP.empty())
		{
			m_cListeningSocket = new UDPServerSocket (m_cConf->UIPort);
		}
		else
		{
			IPAddress cBind;
			int opt = 1;
			cBind = m_cConf->UIIP.c_str();
			
			m_cListeningSocket = new UDPServerSocket (cBind, m_cConf->UIPort);
			m_cListeningSocket->SetSockOpt(SOL_SOCKET, SO_BROADCAST, &opt, sizeof(int));
		}
	}
	catch(...)
	{
		debuglog("KAID","Failed to create UI socket...");
		exit(1);
	}
			
	// Create necessary timers
	newTimer = new CTimerThread (this, 1000, MESSENGER_PING_TIMER);
	m_cTimers.push_back (newTimer);
	newTimer = new CTimerThread (this, 15000, MESSENGER_CLEAN_TIMER);
	m_cTimers.push_back (newTimer);	
	newTimer = new CTimerThread (this, 800, ARENA_PING_TIMER);
	m_cTimers.push_back (newTimer);	
	newTimer = new CTimerThread (this, 5000, ARENA_BUSY_TIMER);
	m_cTimers.push_back (newTimer);	
	newTimer = new CTimerThread (this, 1000, TRAFFIC_TIMER);
	m_cTimers.push_back (newTimer);
	newTimer = new CTimerThread (this, 15000, PRESTEERING_TIMER);
	m_cTimers.push_back (newTimer);
	newTimer = new CTimerThread (this, 15000, ARENA_IDLE_TIMER);
	m_cTimers.push_back (newTimer);
	newTimer = new CTimerThread (this, 15000, ORB_REQUERY_TIMER);
	m_cTimers.push_back (newTimer);

	// Setup DHCP responses
	m_ucaDHCPOffer[0]=0x00;	m_ucaDHCPOffer[1]=0x00;	m_ucaDHCPOffer[2]=0x00;	m_ucaDHCPOffer[3]=0x00;
	m_ucaDHCPOffer[4]=0x00;	m_ucaDHCPOffer[5]=0x00;	m_ucaDHCPOffer[6]=0x00;	m_ucaDHCPOffer[7]=0x00;
	m_ucaDHCPOffer[8]=0xff;	m_ucaDHCPOffer[9]=0xff;	m_ucaDHCPOffer[10]=0xff;m_ucaDHCPOffer[11]=0xff;
	m_ucaDHCPOffer[12]=0xff;m_ucaDHCPOffer[13]=0xff;m_ucaDHCPOffer[14]=0x08;m_ucaDHCPOffer[15]=0x00;
	m_ucaDHCPOffer[16]=0x45;m_ucaDHCPOffer[17]=0x00;m_ucaDHCPOffer[18]=0x01;m_ucaDHCPOffer[19]=0x4a;
	m_ucaDHCPOffer[20]=0xb4;m_ucaDHCPOffer[21]=0xce;m_ucaDHCPOffer[22]=0x00;m_ucaDHCPOffer[23]=0x00;
	m_ucaDHCPOffer[24]=0x80;m_ucaDHCPOffer[25]=0x11;m_ucaDHCPOffer[26]=0xc4;m_ucaDHCPOffer[27]=0x2b;
	m_ucaDHCPOffer[28]=0xc0;m_ucaDHCPOffer[29]=0xa8;m_ucaDHCPOffer[30]=0x00;m_ucaDHCPOffer[31]=0x01;
	m_ucaDHCPOffer[32]=0xff;m_ucaDHCPOffer[33]=0xff;m_ucaDHCPOffer[34]=0xff;m_ucaDHCPOffer[35]=0xff;
	m_ucaDHCPOffer[36]=0x00;m_ucaDHCPOffer[37]=0x43;m_ucaDHCPOffer[38]=0x00;m_ucaDHCPOffer[39]=0x44;
	m_ucaDHCPOffer[40]=0x01;m_ucaDHCPOffer[41]=0x36;m_ucaDHCPOffer[42]=0x00;m_ucaDHCPOffer[43]=0x00;
	m_ucaDHCPOffer[44]=0x02;m_ucaDHCPOffer[45]=0x01;m_ucaDHCPOffer[46]=0x06;m_ucaDHCPOffer[47]=0x00;
	m_ucaDHCPOffer[48]=0x00;m_ucaDHCPOffer[49]=0x00;m_ucaDHCPOffer[50]=0x00;m_ucaDHCPOffer[51]=0x00;
	m_ucaDHCPOffer[52]=0x00;m_ucaDHCPOffer[53]=0x00;m_ucaDHCPOffer[54]=0x00;m_ucaDHCPOffer[55]=0x00;
	m_ucaDHCPOffer[56]=0x00;m_ucaDHCPOffer[57]=0x00;m_ucaDHCPOffer[58]=0x00;m_ucaDHCPOffer[59]=0x00;
	m_ucaDHCPOffer[60]=0xa9;m_ucaDHCPOffer[61]=0xfe;m_ucaDHCPOffer[62]=0x00;m_ucaDHCPOffer[63]=0x00;
	m_ucaDHCPOffer[64]=0x00;m_ucaDHCPOffer[65]=0x00;m_ucaDHCPOffer[66]=0x00;m_ucaDHCPOffer[67]=0x00;
	m_ucaDHCPOffer[68]=0x00;m_ucaDHCPOffer[69]=0x00;m_ucaDHCPOffer[70]=0x00;m_ucaDHCPOffer[71]=0x00;
	m_ucaDHCPOffer[72]=0x00;m_ucaDHCPOffer[73]=0x00;m_ucaDHCPOffer[74]=0x00;m_ucaDHCPOffer[75]=0x00;
	m_ucaDHCPOffer[76]=0x00;m_ucaDHCPOffer[77]=0x00;m_ucaDHCPOffer[78]=0x00;m_ucaDHCPOffer[79]=0x00;
	m_ucaDHCPOffer[80]=0x00;m_ucaDHCPOffer[81]=0x00;m_ucaDHCPOffer[82]=0x00;m_ucaDHCPOffer[83]=0x00;
	m_ucaDHCPOffer[84]=0x00;m_ucaDHCPOffer[85]=0x00;m_ucaDHCPOffer[86]=0x00;m_ucaDHCPOffer[87]=0x00;
	m_ucaDHCPOffer[88]=0x00;m_ucaDHCPOffer[89]=0x00;m_ucaDHCPOffer[90]=0x00;m_ucaDHCPOffer[91]=0x00;
	m_ucaDHCPOffer[92]=0x00;m_ucaDHCPOffer[93]=0x00;m_ucaDHCPOffer[94]=0x00;m_ucaDHCPOffer[95]=0x00;
	m_ucaDHCPOffer[96]=0x00;m_ucaDHCPOffer[97]=0x00;m_ucaDHCPOffer[98]=0x00;m_ucaDHCPOffer[99]=0x00;
	m_ucaDHCPOffer[100]=0x00;m_ucaDHCPOffer[101]=0x00;m_ucaDHCPOffer[102]=0x00;m_ucaDHCPOffer[103]=0x00;
	m_ucaDHCPOffer[104]=0x00;m_ucaDHCPOffer[105]=0x00;m_ucaDHCPOffer[106]=0x00;m_ucaDHCPOffer[107]=0x00;
	m_ucaDHCPOffer[108]=0x00;m_ucaDHCPOffer[109]=0x00;m_ucaDHCPOffer[110]=0x00;m_ucaDHCPOffer[111]=0x00;
	m_ucaDHCPOffer[112]=0x00;m_ucaDHCPOffer[113]=0x00;m_ucaDHCPOffer[114]=0x00;m_ucaDHCPOffer[115]=0x00;
	m_ucaDHCPOffer[116]=0x00;m_ucaDHCPOffer[117]=0x00;m_ucaDHCPOffer[118]=0x00;m_ucaDHCPOffer[119]=0x00;
	m_ucaDHCPOffer[120]=0x00;m_ucaDHCPOffer[121]=0x00;m_ucaDHCPOffer[122]=0x00;m_ucaDHCPOffer[123]=0x00;
	m_ucaDHCPOffer[124]=0x00;m_ucaDHCPOffer[125]=0x00;m_ucaDHCPOffer[126]=0x00;m_ucaDHCPOffer[127]=0x00;
	m_ucaDHCPOffer[128]=0x00;m_ucaDHCPOffer[129]=0x00;m_ucaDHCPOffer[130]=0x00;m_ucaDHCPOffer[131]=0x00;
	m_ucaDHCPOffer[132]=0x00;m_ucaDHCPOffer[133]=0x00;m_ucaDHCPOffer[134]=0x00;m_ucaDHCPOffer[135]=0x00;
	m_ucaDHCPOffer[136]=0x00;m_ucaDHCPOffer[137]=0x00;m_ucaDHCPOffer[138]=0x00;m_ucaDHCPOffer[139]=0x00;
	m_ucaDHCPOffer[140]=0x00;m_ucaDHCPOffer[141]=0x00;m_ucaDHCPOffer[142]=0x00;m_ucaDHCPOffer[143]=0x00;
	m_ucaDHCPOffer[144]=0x00;m_ucaDHCPOffer[145]=0x00;m_ucaDHCPOffer[146]=0x00;m_ucaDHCPOffer[147]=0x00;
	m_ucaDHCPOffer[148]=0x00;m_ucaDHCPOffer[149]=0x00;m_ucaDHCPOffer[150]=0x00;m_ucaDHCPOffer[151]=0x00;
	m_ucaDHCPOffer[152]=0x00;m_ucaDHCPOffer[153]=0x00;m_ucaDHCPOffer[154]=0x00;m_ucaDHCPOffer[155]=0x00;
	m_ucaDHCPOffer[156]=0x00;m_ucaDHCPOffer[157]=0x00;m_ucaDHCPOffer[158]=0x00;m_ucaDHCPOffer[159]=0x00;
	m_ucaDHCPOffer[160]=0x00;m_ucaDHCPOffer[161]=0x00;m_ucaDHCPOffer[162]=0x00;m_ucaDHCPOffer[163]=0x00;
	m_ucaDHCPOffer[164]=0x00;m_ucaDHCPOffer[165]=0x00;m_ucaDHCPOffer[166]=0x00;m_ucaDHCPOffer[167]=0x00;
	m_ucaDHCPOffer[168]=0x00;m_ucaDHCPOffer[169]=0x00;m_ucaDHCPOffer[170]=0x00;m_ucaDHCPOffer[171]=0x00;
	m_ucaDHCPOffer[172]=0x00;m_ucaDHCPOffer[173]=0x00;m_ucaDHCPOffer[174]=0x00;m_ucaDHCPOffer[175]=0x00;
	m_ucaDHCPOffer[176]=0x00;m_ucaDHCPOffer[177]=0x00;m_ucaDHCPOffer[178]=0x00;m_ucaDHCPOffer[179]=0x00;
	m_ucaDHCPOffer[170]=0x00;m_ucaDHCPOffer[181]=0x00;m_ucaDHCPOffer[182]=0x00;m_ucaDHCPOffer[183]=0x00;
	m_ucaDHCPOffer[184]=0x00;m_ucaDHCPOffer[185]=0x00;m_ucaDHCPOffer[186]=0x00;m_ucaDHCPOffer[187]=0x00;
	m_ucaDHCPOffer[188]=0x00;m_ucaDHCPOffer[189]=0x00;m_ucaDHCPOffer[190]=0x00;m_ucaDHCPOffer[191]=0x00;
	m_ucaDHCPOffer[192]=0x00;m_ucaDHCPOffer[193]=0x00;m_ucaDHCPOffer[194]=0x00;m_ucaDHCPOffer[195]=0x00;
	m_ucaDHCPOffer[196]=0x00;m_ucaDHCPOffer[197]=0x00;m_ucaDHCPOffer[198]=0x00;m_ucaDHCPOffer[199]=0x00;
	m_ucaDHCPOffer[200]=0x00;m_ucaDHCPOffer[201]=0x00;m_ucaDHCPOffer[202]=0x00;m_ucaDHCPOffer[203]=0x00;
	m_ucaDHCPOffer[204]=0x00;m_ucaDHCPOffer[205]=0x00;m_ucaDHCPOffer[206]=0x00;m_ucaDHCPOffer[207]=0x00;
	m_ucaDHCPOffer[208]=0x00;m_ucaDHCPOffer[209]=0x00;m_ucaDHCPOffer[210]=0x00;m_ucaDHCPOffer[211]=0x00;
	m_ucaDHCPOffer[212]=0x00;m_ucaDHCPOffer[213]=0x00;m_ucaDHCPOffer[214]=0x00;m_ucaDHCPOffer[215]=0x00;
	m_ucaDHCPOffer[216]=0x00;m_ucaDHCPOffer[217]=0x00;m_ucaDHCPOffer[218]=0x00;m_ucaDHCPOffer[219]=0x00;
	m_ucaDHCPOffer[220]=0x00;m_ucaDHCPOffer[221]=0x00;m_ucaDHCPOffer[222]=0x00;m_ucaDHCPOffer[223]=0x00;
	m_ucaDHCPOffer[224]=0x00;m_ucaDHCPOffer[225]=0x00;m_ucaDHCPOffer[226]=0x00;m_ucaDHCPOffer[227]=0x00;
	m_ucaDHCPOffer[228]=0x00;m_ucaDHCPOffer[229]=0x00;m_ucaDHCPOffer[230]=0x00;m_ucaDHCPOffer[231]=0x00;
	m_ucaDHCPOffer[232]=0x00;m_ucaDHCPOffer[233]=0x00;m_ucaDHCPOffer[234]=0x00;m_ucaDHCPOffer[235]=0x00;
	m_ucaDHCPOffer[236]=0x00;m_ucaDHCPOffer[237]=0x00;m_ucaDHCPOffer[238]=0x00;m_ucaDHCPOffer[239]=0x00;
	m_ucaDHCPOffer[240]=0x00;m_ucaDHCPOffer[241]=0x00;m_ucaDHCPOffer[242]=0x00;m_ucaDHCPOffer[243]=0x00;
	m_ucaDHCPOffer[244]=0x00;m_ucaDHCPOffer[245]=0x00;m_ucaDHCPOffer[246]=0x00;m_ucaDHCPOffer[247]=0x00;
	m_ucaDHCPOffer[248]=0x00;m_ucaDHCPOffer[249]=0x00;m_ucaDHCPOffer[250]=0x00;m_ucaDHCPOffer[251]=0x00;
	m_ucaDHCPOffer[252]=0x00;m_ucaDHCPOffer[253]=0x00;m_ucaDHCPOffer[254]=0x00;m_ucaDHCPOffer[255]=0x00;
	m_ucaDHCPOffer[256]=0x00;m_ucaDHCPOffer[257]=0x00;m_ucaDHCPOffer[258]=0x00;m_ucaDHCPOffer[259]=0x00;
	m_ucaDHCPOffer[260]=0x00;m_ucaDHCPOffer[261]=0x00;m_ucaDHCPOffer[262]=0x00;m_ucaDHCPOffer[263]=0x00;
	m_ucaDHCPOffer[264]=0x00;m_ucaDHCPOffer[265]=0x00;m_ucaDHCPOffer[266]=0x00;m_ucaDHCPOffer[267]=0x00;
	m_ucaDHCPOffer[268]=0x00;m_ucaDHCPOffer[269]=0x00;m_ucaDHCPOffer[270]=0x00;m_ucaDHCPOffer[271]=0x00;
	m_ucaDHCPOffer[272]=0x00;m_ucaDHCPOffer[273]=0x00;m_ucaDHCPOffer[274]=0x00;m_ucaDHCPOffer[275]=0x00;
	m_ucaDHCPOffer[276]=0x00;m_ucaDHCPOffer[277]=0x00;m_ucaDHCPOffer[278]=0x00;m_ucaDHCPOffer[279]=0x00;
	m_ucaDHCPOffer[280]=0x63;m_ucaDHCPOffer[281]=0x82;m_ucaDHCPOffer[282]=0x53;m_ucaDHCPOffer[283]=0x63;
	m_ucaDHCPOffer[284]=0x35;m_ucaDHCPOffer[285]=0x01;m_ucaDHCPOffer[286]=0x02;m_ucaDHCPOffer[287]=0x36;
	m_ucaDHCPOffer[288]=0x04;m_ucaDHCPOffer[299]=0xc0;m_ucaDHCPOffer[290]=0xa8;m_ucaDHCPOffer[291]=0x00;
	m_ucaDHCPOffer[292]=0x01;m_ucaDHCPOffer[293]=0x01;m_ucaDHCPOffer[294]=0x04;m_ucaDHCPOffer[295]=0xff;
	m_ucaDHCPOffer[296]=0xff;m_ucaDHCPOffer[297]=0x00;m_ucaDHCPOffer[298]=0x00;m_ucaDHCPOffer[299]=0x03;
	m_ucaDHCPOffer[300]=0x04;m_ucaDHCPOffer[301]=0xc0;m_ucaDHCPOffer[302]=0xa8;m_ucaDHCPOffer[303]=0x00;
	m_ucaDHCPOffer[304]=0x01;m_ucaDHCPOffer[305]=0x06;m_ucaDHCPOffer[306]=0x04;m_ucaDHCPOffer[307]=0xc0;
	m_ucaDHCPOffer[308]=0xa8;m_ucaDHCPOffer[309]=0x00;m_ucaDHCPOffer[310]=0x01;m_ucaDHCPOffer[311]=0x3a;
	m_ucaDHCPOffer[312]=0x04;m_ucaDHCPOffer[313]=0xff;m_ucaDHCPOffer[314]=0xff;m_ucaDHCPOffer[315]=0xff;
	m_ucaDHCPOffer[316]=0xff;m_ucaDHCPOffer[317]=0x3b;m_ucaDHCPOffer[318]=0x04;m_ucaDHCPOffer[319]=0xff;
	m_ucaDHCPOffer[320]=0xff;m_ucaDHCPOffer[321]=0xff;m_ucaDHCPOffer[322]=0xff;m_ucaDHCPOffer[323]=0x33;
	m_ucaDHCPOffer[324]=0x04;m_ucaDHCPOffer[325]=0xff;m_ucaDHCPOffer[326]=0xff;m_ucaDHCPOffer[327]=0xff;
	m_ucaDHCPOffer[328]=0xff;m_ucaDHCPOffer[329]=0x2e;m_ucaDHCPOffer[330]=0x01;m_ucaDHCPOffer[331]=0x04;
	m_ucaDHCPOffer[332]=0x0f;m_ucaDHCPOffer[333]=0x0b;m_ucaDHCPOffer[334]=0x78;m_ucaDHCPOffer[335]=0x2d;
	m_ucaDHCPOffer[336]=0x6c;m_ucaDHCPOffer[337]=0x69;m_ucaDHCPOffer[338]=0x6e;m_ucaDHCPOffer[339]=0x6b;
	m_ucaDHCPOffer[340]=0x2d;m_ucaDHCPOffer[341]=0x6b;m_ucaDHCPOffer[342]=0x61;m_ucaDHCPOffer[343]=0x69;
	m_ucaDHCPOffer[344]=0x00;m_ucaDHCPOffer[345]=0xff;
	
	m_ucaDHCPAck[0]=0x00;m_ucaDHCPAck[1]=0x00;m_ucaDHCPAck[2]=0x00;	m_ucaDHCPAck[3]=0x00;
	m_ucaDHCPAck[4]=0x00;m_ucaDHCPAck[5]=0x00;m_ucaDHCPAck[6]=0x00;	m_ucaDHCPAck[7]=0x00;
	m_ucaDHCPAck[8]=0xff;m_ucaDHCPAck[9]=0xff;m_ucaDHCPAck[10]=0xff;m_ucaDHCPAck[11]=0xff;
	m_ucaDHCPAck[12]=0xff;m_ucaDHCPAck[13]=0xff;m_ucaDHCPAck[14]=0x08;m_ucaDHCPAck[15]=0x00;
	m_ucaDHCPAck[16]=0x45;m_ucaDHCPAck[17]=0x00;m_ucaDHCPAck[18]=0x01;m_ucaDHCPAck[19]=0x4a;
	m_ucaDHCPAck[20]=0xb4;m_ucaDHCPAck[21]=0xce;m_ucaDHCPAck[22]=0x00;m_ucaDHCPAck[23]=0x00;
	m_ucaDHCPAck[24]=0x80;m_ucaDHCPAck[25]=0x11;m_ucaDHCPAck[26]=0xc4;m_ucaDHCPAck[27]=0x2b;
	m_ucaDHCPAck[28]=0xc0;m_ucaDHCPAck[29]=0xa8;m_ucaDHCPAck[30]=0x00;m_ucaDHCPAck[31]=0x01;
	m_ucaDHCPAck[32]=0xff;m_ucaDHCPAck[33]=0xff;m_ucaDHCPAck[34]=0xff;m_ucaDHCPAck[35]=0xff;
	m_ucaDHCPAck[36]=0x00;m_ucaDHCPAck[37]=0x43;m_ucaDHCPAck[38]=0x00;m_ucaDHCPAck[39]=0x44;
	m_ucaDHCPAck[40]=0x01;m_ucaDHCPAck[41]=0x36;m_ucaDHCPAck[42]=0x00;m_ucaDHCPAck[43]=0x00;
	m_ucaDHCPAck[44]=0x02;m_ucaDHCPAck[45]=0x01;m_ucaDHCPAck[46]=0x06;m_ucaDHCPAck[47]=0x00;
	m_ucaDHCPAck[48]=0x00;m_ucaDHCPAck[49]=0x00;m_ucaDHCPAck[50]=0x00;m_ucaDHCPAck[51]=0x00;
	m_ucaDHCPAck[52]=0x00;m_ucaDHCPAck[53]=0x00;m_ucaDHCPAck[54]=0x00;m_ucaDHCPAck[55]=0x00;
	m_ucaDHCPAck[56]=0x00;m_ucaDHCPAck[57]=0x00;m_ucaDHCPAck[58]=0x00;m_ucaDHCPAck[59]=0x00;
	m_ucaDHCPAck[60]=0xa9;m_ucaDHCPAck[61]=0xfe;m_ucaDHCPAck[62]=0x00;m_ucaDHCPAck[63]=0x00;
	m_ucaDHCPAck[64]=0x00;m_ucaDHCPAck[65]=0x00;m_ucaDHCPAck[66]=0x00;m_ucaDHCPAck[67]=0x00;
	m_ucaDHCPAck[68]=0x00;m_ucaDHCPAck[69]=0x00;m_ucaDHCPAck[70]=0x00;m_ucaDHCPAck[71]=0x00;
	m_ucaDHCPAck[72]=0x00;m_ucaDHCPAck[73]=0x00;m_ucaDHCPAck[74]=0x00;m_ucaDHCPAck[75]=0x00;
	m_ucaDHCPAck[76]=0x00;m_ucaDHCPAck[77]=0x00;m_ucaDHCPAck[78]=0x00;m_ucaDHCPAck[79]=0x00;
	m_ucaDHCPAck[80]=0x00;m_ucaDHCPAck[81]=0x00;m_ucaDHCPAck[82]=0x00;m_ucaDHCPAck[83]=0x00;
	m_ucaDHCPAck[84]=0x00;m_ucaDHCPAck[85]=0x00;m_ucaDHCPAck[86]=0x00;m_ucaDHCPAck[87]=0x00;
	m_ucaDHCPAck[88]=0x00;m_ucaDHCPAck[89]=0x00;m_ucaDHCPAck[90]=0x00;m_ucaDHCPAck[91]=0x00;
	m_ucaDHCPAck[92]=0x00;m_ucaDHCPAck[93]=0x00;m_ucaDHCPAck[94]=0x00;m_ucaDHCPAck[95]=0x00;
	m_ucaDHCPAck[96]=0x00;m_ucaDHCPAck[97]=0x00;m_ucaDHCPAck[98]=0x00;m_ucaDHCPAck[99]=0x00;
	m_ucaDHCPAck[100]=0x00;m_ucaDHCPAck[101]=0x00;m_ucaDHCPAck[102]=0x00;m_ucaDHCPAck[103]=0x00;
	m_ucaDHCPAck[104]=0x00;m_ucaDHCPAck[105]=0x00;m_ucaDHCPAck[106]=0x00;m_ucaDHCPAck[107]=0x00;
	m_ucaDHCPAck[108]=0x00;m_ucaDHCPAck[109]=0x00;m_ucaDHCPAck[110]=0x00;m_ucaDHCPAck[111]=0x00;
	m_ucaDHCPAck[112]=0x00;m_ucaDHCPAck[113]=0x00;m_ucaDHCPAck[114]=0x00;m_ucaDHCPAck[115]=0x00;
	m_ucaDHCPAck[116]=0x00;m_ucaDHCPAck[117]=0x00;m_ucaDHCPAck[118]=0x00;m_ucaDHCPAck[119]=0x00;
	m_ucaDHCPAck[120]=0x00;m_ucaDHCPAck[121]=0x00;m_ucaDHCPAck[122]=0x00;m_ucaDHCPAck[123]=0x00;
	m_ucaDHCPAck[124]=0x00;m_ucaDHCPAck[125]=0x00;m_ucaDHCPAck[126]=0x00;m_ucaDHCPAck[127]=0x00;
	m_ucaDHCPAck[128]=0x00;m_ucaDHCPAck[129]=0x00;m_ucaDHCPAck[130]=0x00;m_ucaDHCPAck[131]=0x00;
	m_ucaDHCPAck[132]=0x00;m_ucaDHCPAck[133]=0x00;m_ucaDHCPAck[134]=0x00;m_ucaDHCPAck[135]=0x00;
	m_ucaDHCPAck[136]=0x00;m_ucaDHCPAck[137]=0x00;m_ucaDHCPAck[138]=0x00;m_ucaDHCPAck[139]=0x00;
	m_ucaDHCPAck[140]=0x00;m_ucaDHCPAck[141]=0x00;m_ucaDHCPAck[142]=0x00;m_ucaDHCPAck[143]=0x00;
	m_ucaDHCPAck[144]=0x00;m_ucaDHCPAck[145]=0x00;m_ucaDHCPAck[146]=0x00;m_ucaDHCPAck[147]=0x00;
	m_ucaDHCPAck[148]=0x00;m_ucaDHCPAck[149]=0x00;m_ucaDHCPAck[150]=0x00;m_ucaDHCPAck[151]=0x00;
	m_ucaDHCPAck[152]=0x00;m_ucaDHCPAck[153]=0x00;m_ucaDHCPAck[154]=0x00;m_ucaDHCPAck[155]=0x00;
	m_ucaDHCPAck[156]=0x00;m_ucaDHCPAck[157]=0x00;m_ucaDHCPAck[158]=0x00;m_ucaDHCPAck[159]=0x00;
	m_ucaDHCPAck[160]=0x00;m_ucaDHCPAck[161]=0x00;m_ucaDHCPAck[162]=0x00;m_ucaDHCPAck[163]=0x00;
	m_ucaDHCPAck[164]=0x00;m_ucaDHCPAck[165]=0x00;m_ucaDHCPAck[166]=0x00;m_ucaDHCPAck[167]=0x00;
	m_ucaDHCPAck[168]=0x00;m_ucaDHCPAck[169]=0x00;m_ucaDHCPAck[170]=0x00;m_ucaDHCPAck[171]=0x00;
	m_ucaDHCPAck[172]=0x00;m_ucaDHCPAck[173]=0x00;m_ucaDHCPAck[174]=0x00;m_ucaDHCPAck[175]=0x00;
	m_ucaDHCPAck[176]=0x00;m_ucaDHCPAck[177]=0x00;m_ucaDHCPAck[178]=0x00;m_ucaDHCPAck[179]=0x00;
	m_ucaDHCPAck[170]=0x00;m_ucaDHCPAck[181]=0x00;m_ucaDHCPAck[182]=0x00;m_ucaDHCPAck[183]=0x00;
	m_ucaDHCPAck[184]=0x00;m_ucaDHCPAck[185]=0x00;m_ucaDHCPAck[186]=0x00;m_ucaDHCPAck[187]=0x00;
	m_ucaDHCPAck[188]=0x00;m_ucaDHCPAck[189]=0x00;m_ucaDHCPAck[190]=0x00;m_ucaDHCPAck[191]=0x00;
	m_ucaDHCPAck[192]=0x00;m_ucaDHCPAck[193]=0x00;m_ucaDHCPAck[194]=0x00;m_ucaDHCPAck[195]=0x00;
	m_ucaDHCPAck[196]=0x00;m_ucaDHCPAck[197]=0x00;m_ucaDHCPAck[198]=0x00;m_ucaDHCPAck[199]=0x00;
	m_ucaDHCPAck[200]=0x00;m_ucaDHCPAck[201]=0x00;m_ucaDHCPAck[202]=0x00;m_ucaDHCPAck[203]=0x00;
	m_ucaDHCPAck[204]=0x00;m_ucaDHCPAck[205]=0x00;m_ucaDHCPAck[206]=0x00;m_ucaDHCPAck[207]=0x00;
	m_ucaDHCPAck[208]=0x00;m_ucaDHCPAck[209]=0x00;m_ucaDHCPAck[210]=0x00;m_ucaDHCPAck[211]=0x00;
	m_ucaDHCPAck[212]=0x00;m_ucaDHCPAck[213]=0x00;m_ucaDHCPAck[214]=0x00;m_ucaDHCPAck[215]=0x00;
	m_ucaDHCPAck[216]=0x00;m_ucaDHCPAck[217]=0x00;m_ucaDHCPAck[218]=0x00;m_ucaDHCPAck[219]=0x00;
	m_ucaDHCPAck[220]=0x00;m_ucaDHCPAck[221]=0x00;m_ucaDHCPAck[222]=0x00;m_ucaDHCPAck[223]=0x00;
	m_ucaDHCPAck[224]=0x00;m_ucaDHCPAck[225]=0x00;m_ucaDHCPAck[226]=0x00;m_ucaDHCPAck[227]=0x00;
	m_ucaDHCPAck[228]=0x00;m_ucaDHCPAck[229]=0x00;m_ucaDHCPAck[230]=0x00;m_ucaDHCPAck[231]=0x00;
	m_ucaDHCPAck[232]=0x00;m_ucaDHCPAck[233]=0x00;m_ucaDHCPAck[234]=0x00;m_ucaDHCPAck[235]=0x00;
	m_ucaDHCPAck[236]=0x00;m_ucaDHCPAck[237]=0x00;m_ucaDHCPAck[238]=0x00;m_ucaDHCPAck[239]=0x00;
	m_ucaDHCPAck[240]=0x00;m_ucaDHCPAck[241]=0x00;m_ucaDHCPAck[242]=0x00;m_ucaDHCPAck[243]=0x00;
	m_ucaDHCPAck[244]=0x00;m_ucaDHCPAck[245]=0x00;m_ucaDHCPAck[246]=0x00;m_ucaDHCPAck[247]=0x00;
	m_ucaDHCPAck[248]=0x00;m_ucaDHCPAck[249]=0x00;m_ucaDHCPAck[250]=0x00;m_ucaDHCPAck[251]=0x00;
	m_ucaDHCPAck[252]=0x00;m_ucaDHCPAck[253]=0x00;m_ucaDHCPAck[254]=0x00;m_ucaDHCPAck[255]=0x00;
	m_ucaDHCPAck[256]=0x00;m_ucaDHCPAck[257]=0x00;m_ucaDHCPAck[258]=0x00;m_ucaDHCPAck[259]=0x00;
	m_ucaDHCPAck[260]=0x00;m_ucaDHCPAck[261]=0x00;m_ucaDHCPAck[262]=0x00;m_ucaDHCPAck[263]=0x00;
	m_ucaDHCPAck[264]=0x00;m_ucaDHCPAck[265]=0x00;m_ucaDHCPAck[266]=0x00;m_ucaDHCPAck[267]=0x00;
	m_ucaDHCPAck[268]=0x00;m_ucaDHCPAck[269]=0x00;m_ucaDHCPAck[270]=0x00;m_ucaDHCPAck[271]=0x00;
	m_ucaDHCPAck[272]=0x00;m_ucaDHCPAck[273]=0x00;m_ucaDHCPAck[274]=0x00;m_ucaDHCPAck[275]=0x00;
	m_ucaDHCPAck[276]=0x00;m_ucaDHCPAck[277]=0x00;m_ucaDHCPAck[278]=0x00;m_ucaDHCPAck[279]=0x00;
	m_ucaDHCPAck[280]=0x63;m_ucaDHCPAck[281]=0x82;m_ucaDHCPAck[282]=0x53;m_ucaDHCPAck[283]=0x63;
	m_ucaDHCPAck[284]=0x35;m_ucaDHCPAck[285]=0x01;m_ucaDHCPAck[286]=0x05;m_ucaDHCPAck[287]=0x36;
	m_ucaDHCPAck[288]=0x04;m_ucaDHCPAck[299]=0xc0;m_ucaDHCPAck[290]=0xa8;m_ucaDHCPAck[291]=0x00;
	m_ucaDHCPAck[292]=0x01;m_ucaDHCPAck[293]=0x01;m_ucaDHCPAck[294]=0x04;m_ucaDHCPAck[295]=0xff;
	m_ucaDHCPAck[296]=0xff;m_ucaDHCPAck[297]=0x00;m_ucaDHCPAck[298]=0x00;m_ucaDHCPAck[299]=0x03;
	m_ucaDHCPAck[300]=0x04;m_ucaDHCPAck[301]=0xc0;m_ucaDHCPAck[302]=0xa8;m_ucaDHCPAck[303]=0x00;
	m_ucaDHCPAck[304]=0x01;m_ucaDHCPAck[305]=0x06;m_ucaDHCPAck[306]=0x04;m_ucaDHCPAck[307]=0xc0;
	m_ucaDHCPAck[308]=0xa8;m_ucaDHCPAck[309]=0x00;m_ucaDHCPAck[310]=0x01;m_ucaDHCPAck[311]=0x3a;
	m_ucaDHCPAck[312]=0x04;m_ucaDHCPAck[313]=0xff;m_ucaDHCPAck[314]=0xff;m_ucaDHCPAck[315]=0xff;
	m_ucaDHCPAck[316]=0xff;m_ucaDHCPAck[317]=0x3b;m_ucaDHCPAck[318]=0x04;m_ucaDHCPAck[319]=0xff;
	m_ucaDHCPAck[320]=0xff;m_ucaDHCPAck[321]=0xff;m_ucaDHCPAck[322]=0xff;m_ucaDHCPAck[323]=0x33;
	m_ucaDHCPAck[324]=0x04;m_ucaDHCPAck[325]=0xff;m_ucaDHCPAck[326]=0xff;m_ucaDHCPAck[327]=0xff;
	m_ucaDHCPAck[328]=0xff;m_ucaDHCPAck[329]=0x2e;m_ucaDHCPAck[330]=0x01;m_ucaDHCPAck[331]=0x04;
	m_ucaDHCPAck[332]=0x0f;m_ucaDHCPAck[333]=0x0b;m_ucaDHCPAck[334]=0x78;m_ucaDHCPAck[335]=0x2d;
	m_ucaDHCPAck[336]=0x6c;m_ucaDHCPAck[337]=0x69;m_ucaDHCPAck[338]=0x6e;m_ucaDHCPAck[339]=0x6b;
	m_ucaDHCPAck[340]=0x2d;m_ucaDHCPAck[341]=0x6b;m_ucaDHCPAck[342]=0x61;m_ucaDHCPAck[343]=0x69;
	m_ucaDHCPAck[344]=0x00;m_ucaDHCPAck[345]=0xff;
	
	// Begin server
	StartServer ();
}

CKaiEngine::~CKaiEngine ()
{
	SendClientResponse("KAI_CLIENT_DETACH;");
	
	Shutdown();
	// Inform my buddies
	LoggedOut();
		
	delete m_cListeningSocket;
	for(unsigned int i = 0; i < m_cTimers.size(); i++)
	{
		m_cTimers[i]->Terminate();
		delete (CTimerThread*)m_cTimers[i];
	}

	delete m_cTOrb;
    delete m_cUOrb;
    delete m_cSniffer;
	
	// WaitThread ();
	verbositylog(2,"THREAD","Engine thread stopped...");
}

void CKaiEngine::HandleSpeexData(void *speex_data, int speex_size, bool incoming, string sWho)
{
	if(incoming)
	{
		string prefix = "KAI_CLIENT_SPEEX;" + sWho + ";";
		int psize = prefix.size() + speex_size - 2;
		u_char newbuf[psize];
		memcpy (newbuf, prefix.c_str(), prefix.size());
		memcpy (newbuf + prefix.size(), (u_char*)speex_data + 2, speex_size - 2);
		m_cListeningSocket->SendDatagram(newbuf, psize, m_cUIAddr, m_iUIPort);
	}
	else
	{	
		int psize = speex_size - 15;
		u_char newbuf[psize];
		memcpy (newbuf + 2, (u_char*)speex_data + 17, speex_size - 17);
		newbuf[0] = '!';
		newbuf[1] = ';';
		
		for (unsigned int i=0; i < m_cMessengerContacts.size(); i++)
		{
			if (m_cMessengerContacts[i]->m_iState == 1 && m_cMessengerContacts[i]->m_bSpeexConnected)
			{
				// TODO: Deep stuff
				m_cUOrb->SendSpeex(newbuf, psize,  m_cMessengerContacts[i]->m_sIPAddress,
					m_cMessengerContacts[i]->m_iPort);
			}
		}
	}
}

void
CKaiEngine::HandleUIRequest ()
{
	// Log
	verbositylog(3,"UI","Parsing UI command: "+m_sRequest);
	
	// Default to no answer! Execute() will skip it
	m_sAnswer = "";

	string Client = m_cIPSource.GetAddressString();
	Client += ":" + Str(m_iSourcePort);

	// TODO: Check UIAllow from .conf (subnets, specific IPs, special strings = ALL, WORLD, HITMEHARD)
	// If not allowed, bail
	// if(!ClientAllowed(m_cIPSource.GetAddressString()))
	// {
	//		return;
	//	}
	
	// Safety! If some UI is attached, and we got a broadcast from some other IP not asking to takeover, drop it
	if(m_bAttached && (strcmp(m_cIPSource.GetAddressString(), m_cUIAddr.GetAddressString()) != 0) )
	{
		if(m_sRequest != "KAI_CLIENT_ATTACH;" && m_sRequest != "KAI_CLIENT_TAKEOVER;" &&
			m_sRequest != "KAI_CLIENT_DISCOVER;")
		{
			return;
		}
	}
		
    if(m_sRequest.size() < 2)
    {
        return;
    }

	KAI_UI_OPCODE Opcode;
	vector < string > vsSegments;

	Tokenize (m_sRequest, vsSegments, ";");
	Opcode = MsgToUIOpcode(vsSegments[0]);
	
    int n = SegDiff(Opcode, vsSegments.size());
    if( n != 0)
	{
        return;
    }
	
	// Filter out a SPEEX send - no ending ;
	switch(Opcode) {
		case KAI_CLIENT_DISCOVER:
		{			
			// If last discover received is not 2 secs old, bail
			if (TimeDelta(m_tvLastDiscover) > 2000)
			{
				m_tvLastDiscover = Now();
				string sEngineHere = "KAI_CLIENT_ENGINE_HERE;";
				m_cListeningSocket->SendDatagram(sEngineHere.c_str(), sEngineHere.size(), m_cIPSource, m_iSourcePort);
			}
			break;
		}
		case KAI_CLIENT_TAKEOVER:
		{
			// Tell old UI he's detached - but only if he's different to this one here
			if (m_cUIAddr!=m_cIPSource ||
			    m_iUIPort!=m_iSourcePort) 
					SendClientResponse("KAI_CLIENT_DETACH;");
					
			m_bAttached = true;
			m_cUIAddr = m_cIPSource;
			m_iUIPort = m_iSourcePort;	
			m_sUICaps = "0";
			SendLocalConsoleData();

			// Tell new UI he's attached
			m_sAnswer = "KAI_CLIENT_ATTACH;";
			verbositylog(1,"UI","UI Attached...");
			break;
		} 
		case KAI_CLIENT_ATTACH:
		{
			if(m_bAttached)
			{
				string sInUse = "KAI_CLIENT_ENGINE_IN_USE;";
				m_cListeningSocket->SendDatagram(sInUse.c_str(), sInUse.size(), m_cIPSource, m_iSourcePort);
				string x = m_cIPSource.GetAddressString();
				x += ":" + Str(m_iSourcePort);	
			}
			else
			{
				m_bAttached = true;
				m_cUIAddr = m_cIPSource;
				m_iUIPort = m_iSourcePort;	
				m_sUICaps = "0";
				SendLocalConsoleData();
				m_sAnswer = "KAI_CLIENT_ATTACH;";
				verbositylog(1,"UI","UI Attached...");
			}
			break;
		}
		case KAI_CLIENT_GETSTATE:
		{
			if (m_cTOrb->m_bConnected && m_cTOrb->m_bAuthenticated)
			{
				ReAttach ();
			}
			else
			{
				// Get user and pass from conf
				m_sAnswer =
					"KAI_CLIENT_NOT_LOGGED_IN;" +
					m_cConf->Username + ";" + m_cConf->Password +
					";" + Str (m_cConf->AutoLogin) + ";";
			}
			break;
		}
		case KAI_CLIENT_LOGIN:
		{
			m_cConf->Username = vsSegments[1];
			m_cConf->Password = vsSegments[2];
	
			if (m_cTOrb->m_bConnected)
			{
				if (!m_cTOrb->m_bAuthenticated)
					m_cTOrb->SendOrbAuth ();
				else
					SendClientResponse
						("KAI_CLIENT_ALREADY_LOGGED_IN;");
			}
			else
			{
				m_cUOrb->RemakeSocket();

				if(m_cConf->DeepPort!=0)
				{
					SendClientStatus ("Scanning deep resolution servers...");
					m_cUOrb->ScanDeepResolutionServers();
				}
				
				SendClientStatus ("Querying orbital mesh...");
				StartTimer(ORB_REQUERY_TIMER);
				IPAddress cNearestOrb;
				
				// First attempt
				cNearestOrb = m_cUOrb->FindNearestOrb();
				
				while (!m_cUOrb->m_bOrbFound)
				{
					sleep(5);
					cNearestOrb = m_cUOrb->FindNearestOrb();
				}
				
				StopTimer(ORB_REQUERY_TIMER);
				
				m_cTOrb->Start (cNearestOrb);
				m_cUOrb->Start ();
				m_cSniffer->Start ();
			}
			break;
		}
		case KAI_CLIENT_PM:
		{
			// The UI has asked to send a PM.. echo back, then send over
			// UDP to contact
			CMessengerItem *theContact;
			theContact = FindUser (vsSegments[1]);
			if (theContact)
			{
				if(theContact->m_bUseDeep)
				{
					m_cUOrb->RelayDatagram ("P;" + m_cConf->Username +
								";" + vsSegments[2] + ";",
								theContact->m_sDeepIP,
								theContact->m_iDeepPort,
								true);
				}
				else
				{
					m_cUOrb->RelayDatagram ("P;" + m_cConf->Username +
								";" + vsSegments[2] + ";",
								theContact->m_sIPAddress,
								theContact->m_iPort,
								false);
				}
			}			
			break;
		}
		case KAI_CLIENT_DETACH:
		{
			m_bAttached = false;
			m_sUICaps = "";
			m_sAnswer = m_sRequest;
			verbositylog(1,"UI","UI Detached...");
			break;
		}
		case KAI_CLIENT_ADD_CONTACT:
		{
			m_cTOrb->SendStream ("KAI_ADD_CONTACT;" + vsSegments[1] +
						 ";");
			break;
		}
		case KAI_CLIENT_REMOVE_CONTACT:
		{
			m_cTOrb->SendStream ("KAI_REMOVE_CONTACT;" + vsSegments[1] +
						 ";");
			break;
		}
		case KAI_CLIENT_CHATMODE:
		{
			// Set the mode
			m_sChatMode = vsSegments[1];

			// Null the chat users
			m_svChatUsers.clear();
			
			SendClientResponse ("KAI_CLIENT_CHATMODE;" + vsSegments[1] +
						";");
			// Init
			if (m_cTOrb)
				m_cTOrb->SendStream ("KAI_CHATMODE;" + vsSegments[1] +
							 ";");
			break;
		}
		case KAI_CLIENT_CHAT:
		{
			// Update user interaction
			m_tvLastUserInteraction=Now();
			
			// Take out file:: stuff
			string temp = vsSegments[1];
			if (temp.find ("file:") != string::npos)
				return;	// Dump syslog bad string
			// The UI has sent a chat request.. handle it
			string s = case_lower(temp.substr (0, temp.find(" ")));
			if (s == "/orb")
			{
				// Make sure there's something after /orb before passing it along - MF
				if(temp.size()>5)
					
				// The ui has send ao orb request - process it
					if (m_cTOrb)
						m_cTOrb->SendStream ("KAI_ORB_COMMAND;" +
								 	temp.substr (5) + ";");
				return;
			}
			else if (s == "/engine")
			{
				string check;
				check = case_lower(trim(temp.substr(temp.find(" ") + 1)));
				if (check == "status")
				{
					// Status
					string strOut = "";
					strOut = "I've got " + Str(m_cMessengerContacts.size()) + " messenger contacts, "
						"and " + Str(m_cArenaContacts.size()) + " arena contacts. "
						"The MAC cache contains " + Str(m_cDevices.size()) + " entries "
						"and I have also cached " + Str(m_svChatUsers.size()) + " chat users.";
					SendClientResponse("KAI_CLIENT_CHAT;" + m_sCurrentArena + ";Local Kai Engine;" + strOut + ";");
					return;
				}
			}

			// Passon 
			if (m_cTOrb)
				m_cTOrb->SendStream ("KAI_CHAT;" + m_sChatMode + ";" +
							 vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_AVATAR:
		{
			// The UI has asked for an avatar - tell teh orb
			if (m_cTOrb)
				m_cTOrb->SendStream ("KAI_GET_AVATAR;" +
							 vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_GET_METRICS:
		{
			string strOutput;
			string strLocked = "1";
			string strPort = Str (MyPort ());
			string strEngineUp = Str(m_iUpEngine);
			string strEngineDown = Str(m_iDownEngine);
			string strChatUp = Str(m_iUpChat);
			string strChatDown = Str(m_iDownChat);
			string strOrbUp = Str(m_iUpOrb);
			string strOrbDown = Str(m_iDownOrb);
			string strBroadcastUp = Str(m_iUpBroadcast);
			string strBroadcastDown = Str(m_iDownBroadcast);
			string strDirectUp = Str(m_iUpDirect);
			string strDirectDown = Str(m_iDownDirect);
			
			strOutput = "KAI_CLIENT_METRICS;" + m_sOurOrbName +
				";" + m_sReachable + ";" +
				MyIP ().GetAddressString () + ";" + strPort +
				";" VERSION ";" + PLATFORM + ";" + AUTHOR + ";" +
				m_sAdapterName + ";" + strLocked + ";" + strEngineUp +
				";" + strEngineDown + ";" + strChatUp + ";" +
				strChatDown + ";" + strOrbUp + ";" + strOrbDown +
				";libpcap;n/a;"+strBroadcastDown+";"+strBroadcastUp+";"+strDirectDown+";"+strDirectUp+";";
			m_sAnswer = strOutput;
			break;
		}
		case KAI_CLIENT_GET_PROFILE:
		{
			if (m_cTOrb)
				m_cTOrb->SendStream ("KAI_GET_PROFILE;" +
							 vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_VECTOR:
		{
			// The UI has set a mode - send up to orbital and store UI interaction
			m_tvLastUserInteraction=Now();
			m_cTOrb->SendStream ("KAI_VECTOR;" + vsSegments[1] + ";" +
						 ((vsSegments.size () <
						   3) ? "" : vsSegments[2]) + ";");
			break;
		}
		case KAI_CLIENT_GET_VECTORS:
		{			// The UI has set a mode - send up to orbital
			// m_sCurrentArena = vsSegments[1];
			m_cTOrb->SendStream ("KAI_GET_VECTORS;" + vsSegments[1] +
						 ";");
			break;
		}
		case KAI_CLIENT_SPECIFIC_COUNT:
		{
			// Relay to orb 
			m_cTOrb->SendStream ("KAI_SPECIFIC_COUNT;" + vsSegments[1] +
						 ";");
			break;
		}
		case KAI_CLIENT_APP_SPECIFIC:
		{
			m_cTOrb->SendStream ("KAI_SPECIFIC;" + vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_CREATE_VECTOR:
		{
			string outputdata;
			outputdata =
				"KAI_CREATE_VECTOR;" + vsSegments[1] + ";" +
				vsSegments[2] + ";" + vsSegments[3] + ";";
			m_cTOrb->SendStream (outputdata);
			break;
		}
		case KAI_CLIENT_ARENA_KICK:
		{
			// Ask to kick a user
			m_cTOrb->SendStream ("KAI_ARENA_KICK;" + vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_ARENA_BAN:
		{
			// Ask to kick a user
			m_cTOrb->SendStream ("KAI_ARENA_BAN;" + vsSegments[1] + ";");
			break;
		}
		case KAI_CLIENT_ARENA_STATUS:
		{
			// Simply store the data - then return this one
			m_iArenaStatus = atoi (vsSegments[1].c_str ());
			m_iArenaMyPlayers = atoi (vsSegments[2].c_str ());
			SendClientResponse (m_sRequest);
			break;
		}
		case KAI_CLIENT_CAPS:
		{
			string cap = trim(vsSegments[1]);
			if(cap == "")
				cap = "0";
			m_sUICaps = cap;
			break;
		}
		case KAI_CLIENT_ARENA_PM:
		{
			// The UI has asked to send a PM.. echo back, then send over UDP
			// stream to contact
			CArenaItem* theGuy = FindArenaUser(vsSegments[1]);
			if (!theGuy) {
				break;
			}
			string msg = "$;" + m_cConf->Username + ";" + vsSegments[2] + ";";
			if(theGuy->m_bUseDeep)
				m_cUOrb->RelayDatagram(msg, theGuy->m_sDeepIP, theGuy->m_iDeepPort, true);
			else
				m_cUOrb->RelayDatagram(msg, theGuy->m_sIPAddress, theGuy->m_iPort, false);
			break;
		}
		case KAI_CLIENT_SPEEX_ON:
		{
			// Set the players "requesting flag" to on
			CMessengerItem* theGuy = FindUser(vsSegments[1]);
			if (!theGuy) break;
			
			theGuy->m_iSpeexMode = 1;
			// Done - tell UI
			SendClientResponse("KAI_CLIENT_SPEEX_ON;" + theGuy->m_sContactName + ";");
			CheckIfSpeexNeeded();
			break;
		}
		case KAI_CLIENT_SPEEX_OFF:
		{
			// Set the players "requesting flag" to off
			CMessengerItem* theGuy = FindUser(vsSegments[1]);
			if (!theGuy) break;
			
			theGuy->m_iSpeexMode = 0;
			theGuy->m_bSpeexConnected = false;
			// Done - tell UI
			SendClientResponse("KAI_CLIENT_SPEEX_OFF;" + theGuy->m_sContactName + ";");
			SendClientResponse("KAI_CLIENT_SPEEX_DISCONNECTED;" + theGuy->m_sContactName + ";");
			CheckIfSpeexNeeded();
			break;
		}

		case KAI_CLIENT_INVITE:
		{
			CMessengerItem* theGuy = FindUser(vsSegments[1]);
			if(!theGuy) {
				break;
			} else {
				if(theGuy->m_bUseDeep)
					m_cUOrb->RelayDatagram("};" + m_cConf->Username + ";" + vsSegments[2] + ";",
						theGuy->m_sDeepIP, theGuy->m_iDeepPort, true);
				else			
					m_cUOrb->RelayDatagram("};" + m_cConf->Username + ";" + vsSegments[2] + ";",
						theGuy->m_sIPAddress, theGuy->m_iPort, false);
			}
			break;
		}
	}
}

void
CKaiEngine::SendClientResponse (string resp)
{
	
	if(m_bAttached || (resp == "KAI_CLIENT_ENGINE_HERE;") )
	{
		verbositylog(3,"UI","Sending UI response: "+resp);
		m_cListeningSocket->SendDatagram (resp.c_str (), resp.length (),
					  m_cUIAddr, m_iUIPort);
	}
	else
	{
	}
}

void *
CKaiEngine::Execute ()
{
	int iBytesTransferred, retval;
	char pcBuffer[2048];
	SocketSet mySet;
	struct timeval timeout;
	
	syslog (LOG_INFO, "Now waiting for UI commands.");

	
	while (!m_bTerminate)
	{
		try
		{
			// Changed to 1-second timeout to allow clean shutdown - MF
			do
			{
				timeout.tv_sec=1;
				timeout.tv_usec=0;
				mySet.Clear();
				mySet += (SocketBase *) m_cListeningSocket;
				retval = select ((int) mySet + 1, mySet, NULL, NULL, &timeout);
				if (retval == -1)
					throw errClientError;
			} while((retval==0) && (!m_bTerminate));

			iBytesTransferred =
				m_cListeningSocket->ReceiveDatagram (pcBuffer,
								     sizeof
								     (pcBuffer)
								     - 1,
								     m_cIPSource,
								     m_iSourcePort);
			if (m_bTerminate)
				throw errServerDown;
			if (iBytesTransferred > 0)
			{
				pcBuffer[iBytesTransferred] = 0;
				m_sRequest = pcBuffer;
				if(m_sRequest.substr(0, 17) == "KAI_CLIENT_SPEEX;")
					HandleSpeexData(&pcBuffer, iBytesTransferred);
				else
				{				
					HandleUIRequest ();
					if (m_sAnswer != "")
						SendClientResponse (m_sAnswer);
				}
			}
		}
		catch (...)
		{
			if(!m_bTerminate)
			{
				// If the engine throws an error, shut down cleanly - MF
				verbositylog(1,"ENGINE","An unrecoverable error occurred in the engine process.  kaid is shutting down.");
				raise(SIGTERM);
			}
			Shutdown();
			return NULL;
		}
	}
	return NULL;
}

void
CKaiEngine::SendClientStatus (string stat)
{
	if (stat != m_strStatusCache)
	{
		SendClientResponse ("KAI_CLIENT_STATUS;" + stat + ";");
		m_strStatusCache = stat;
	}
}

void
CKaiEngine::GetContacts ()
{
	// Get contacts from XServer
	SendClientStatus ("Downloading Contacts...");
	if (m_cTOrb)
		m_cTOrb->SendStream ("KAI_GET_CONTACTS;");
}

void
CKaiEngine::AddVector (string vector, string type)
{
	m_sVectors.push_back (type + ";" + vector);
}

void
CKaiEngine::LoggedIn (string usr)
{
	m_cConf->Username = usr;
	SendClientResponse ("KAI_CLIENT_USER_DATA;" + m_cConf->Username +
			    ";");
	GetContacts ();
		
	StartTimer(MESSENGER_PING_TIMER); 	// Ping timer
	StartTimer(MESSENGER_CLEAN_TIMER); 	// Stalled contacts go offline	
	StartTimer(ARENA_PING_TIMER);
	StartTimer(ARENA_BUSY_TIMER);
    StartTimer(TRAFFIC_TIMER);
	StartTimer(PRESTEERING_TIMER);
	StartTimer(ARENA_IDLE_TIMER);
}

void
CKaiEngine::LoggedOut()
{
	vector <CMessengerItem*>::iterator i;
	for (i = m_cMessengerContacts.begin (); i != m_cMessengerContacts.end (); i++)
	{
		CMessengerItem* theGuy = (*i);
		if(theGuy->m_iState == 1)
		{
			if(m_cTOrb)
			{
				m_cTOrb->Deliver (theGuy->m_sContactName, '8',
					 m_cConf->Username,
					 MyIP ().
					 GetAddressString (),
					 MyPort (), "", 0);
			}
		}
	}
}

void
CKaiEngine::AddChatUser (string strRoom, string strUser)
{
	if (strRoom == m_sChatMode)
		m_svChatUsers.push_back (strUser);
}

void
CKaiEngine::RemoveChatUser (string strRoom, string strUser)
{
	vector < string >::iterator i;
	for (i = m_svChatUsers.begin (); i != m_svChatUsers.end (); i++)
	{
		if (*i == strUser)
		{
			m_svChatUsers.erase (i);
			return;
		}
	}
}

void
CKaiEngine::RemoveVector (string strVector)
{
	vector < string >::iterator i;
	for (i = m_sVectors.begin (); i != m_sVectors.end (); i++)
	{
		if (*i == strVector)
		{
			m_sVectors.erase (i);
			return;
		}
	}
}

CMessengerItem *
CKaiEngine::FindUser (const string & user)
{
	for (unsigned int i = 0; i < m_cMessengerContacts.size (); i++)
	{
		if (m_cMessengerContacts[i]->m_sContactName == user)
		{
			return m_cMessengerContacts[i];
		}
	}
	return NULL;
}

CMessengerItem *
CKaiEngine::FindUser (string sIP, int iPort)
{
	vector <CMessengerItem*>::iterator i;
	for (i = m_cMessengerContacts.begin (); i != m_cMessengerContacts.end (); i++)
		{
		if (((*i)->m_sIPAddress == sIP) && ((*i)->m_iPort == iPort))
			return *i;
		if(((*i)->m_sDeepIP == sIP) && ((*i)->m_iDeepPort == iPort))
			return *i;
	}
	return NULL;
}

CArenaItem *
CKaiEngine::FindArenaUser (const string & user)
{
	for (unsigned int i = 0; i < m_cArenaContacts.size (); i++)
	{
		if (m_cArenaContacts[i]->m_sContactName == user)
		{
			return m_cArenaContacts[i];
		}
	}
	return NULL;
}

CArenaItem *
CKaiEngine::FindArenaUser (string sIP, int iPort)
{
	vector <CArenaItem*>::iterator i;	
	for (i = m_cArenaContacts.begin (); i != m_cArenaContacts.end (); i++)
	{
		if (((*i)->m_sIPAddress == sIP) && ((*i)->m_iPort == iPort))
			return *i;
		if(((*i)->m_sDeepIP == sIP) && ((*i)->m_iDeepPort == iPort))
			return *i;		
	}
	return NULL;
}

void
CKaiEngine::SetOfflineContact (const string & user)
{
	CMessengerItem *theGuy = FindUser (user);
	if (!theGuy)
		return;

	// Inform UI
	SendClientResponse ("KAI_CLIENT_CONTACT_OFFLINE;" +
			    theGuy->m_sContactName + ";");

	// Null it
	bzero (&theGuy->m_tvLastConnectionQuery,
	       sizeof (theGuy->m_tvLastConnectionQuery));
	bzero (&theGuy->m_tvLastQueryResponse,
	       sizeof (theGuy->m_tvLastQueryResponse));
	theGuy->m_iState = 0;
	theGuy->m_sIPAddress = "";
	theGuy->m_iPort = 0;
	theGuy->m_sDeepIP = "";
	theGuy->m_iDeepPort = 0;
	theGuy->m_iRecast = 0;
	theGuy->m_iSpeexMode = 0;
//	if(theGuy->m_bSpeexConnected)
//			SendClientResponse("KAI_CLIENT_SPEEX_DISCONNECTED;");
	theGuy->m_bSpeexConnected = false;
	theGuy->m_bUseDeep=false;
	theGuy->m_bDeepUnknown=false;

	CheckIfSpeexNeeded();
}

void
CKaiEngine::DeleteContact (string contact)
{
	vector <CMessengerItem*>::iterator i;
	// OK.. a contact has been killed..
	mutexProtect.Lock ();
	for (i = m_cMessengerContacts.begin ();
	     i != m_cMessengerContacts.end (); i++)
	{
		if ((*i)->m_sContactName == contact)
		{
			// Ok... the swine is here... unleash ;)
			m_cMessengerContacts.erase (i);
			break;
		}
	}
	mutexProtect.Unlock ();
	// Tell UI about removal
	SendClientResponse ("KAI_CLIENT_REMOVE_CONTACT;" + contact + ";");
}

void
CKaiEngine::RemoveFailedDevice (string device)
{
	vector <CKaiDevice*>::iterator i;

	for (i = m_cFailedDevices.begin (); i != m_cFailedDevices.end (); i++)
	{
		if ((*i)->m_sMacAddress == device)
		{
			// Ok... the swine is here... unleash ;)
			m_cFailedDevices.erase (i);
			break;
		}
	}
}

void
CKaiEngine::RemoveLocalDevice (string device)
{
	vector <CKaiDevice*>::iterator i;

	for (i = m_cDevices.begin (); i != m_cDevices.end (); i++)
	{
		if ((*i)->m_sMacAddress == device)
		{
			// Ok... the swine is here... unleash ;)
			verbositylog(1,"CONSOLE","Console Removed...");
			m_cDevices.erase (i);
			break;
		}
	}
}

void
CKaiEngine::RemoveArenaContact (string contact)
{
	vector <CArenaItem*>::iterator i;
	// OK.. an arena dude has been killed..
	mutexProtect.Lock ();
	for (i = m_cArenaContacts.begin (); i != m_cArenaContacts.end (); i++)
	{
		if ((*i)->m_sContactName == contact)
		{
			// Ok... the swine is here... unleash ;)
			m_cArenaContacts.erase (i);
			break;
		}
	}
	mutexProtect.Unlock ();
}

void
CKaiEngine::AddFailedDevice(string device)
{
	CKaiDevice *d = new CKaiDevice(device, true);
	m_cFailedDevices.push_back(d);
	
	SendClientResponse("KAI_CLIENT_DHCP_FAILURE;"+case_upper(crop(device, ':'))+";");
}

void
CKaiEngine::AddArenaContact (string name, string ip, int port, string deepip,
			     int deepport)
{
	CArenaItem* newGuy = new CArenaItem ();
	newGuy->m_sContactName = name;
	newGuy->m_sIPAddress = ip;
	newGuy->m_iPort = port;
	newGuy->m_sDeepIP = deepip;
	newGuy->m_iDeepPort = deepport;
	newGuy->m_bDeepUnknown=((m_cConf->UseDeep) && (deepip.size()>0) && (deepport!=0));
	m_cArenaContacts.push_back (newGuy);
}


void
CKaiEngine::SetOnlineContact (string contact, string ipaddress,
			      int port, string deepip, int deepport)
{
	CMessengerItem *theGuy = FindUser (contact);
	if (!theGuy)
		return;
	SendClientResponse ("KAI_CLIENT_CONTACT_ONLINE;" +
			    theGuy->m_sContactName + ";");
	theGuy->m_tvLastCompletePing = Now();
	theGuy->m_sIPAddress = ipaddress;
	theGuy->m_iPort = port;
	theGuy->m_iState = 1;
	theGuy->m_sDeepIP=deepip;
	theGuy->m_iDeepPort=deepport;
	theGuy->m_bDeepUnknown=((m_cConf->UseDeep) && (deepip.size()>0) && (deepport!=0));
}

void
CKaiEngine::RequeryMessengerConnections ()
{
	CMessengerItem *theGuy,*firstGuy;
	if (m_cMessengerContacts.size () < 1)
		return;

	mutexProtect.Lock ();
	
	firstGuy = m_cMessengerContacts[m_iMessengerCurrentPing];
	
	m_iMessengerCurrentPing++;
	if (m_iMessengerCurrentPing >= m_cMessengerContacts.size ())
	m_iMessengerCurrentPing = 0;
		
	theGuy = m_cMessengerContacts[m_iMessengerCurrentPing];
	
	// Need to see if this guy is online - if not, hell, dont try to ping him :D
	while(theGuy->m_iState!=1 && theGuy!=firstGuy)
	{
		m_iMessengerCurrentPing++;
		if (m_iMessengerCurrentPing >= m_cMessengerContacts.size ())
		m_iMessengerCurrentPing = 0;
		theGuy = m_cMessengerContacts[m_iMessengerCurrentPing];
	}
	
	// Bail if not online
	if (theGuy->m_iState == 1)
	{
		// Set the time params
		theGuy->Reset();
		theGuy->m_tvLastConnectionQuery = Now();

		if(theGuy->m_bDeepUnknown || theGuy->m_bUseDeep)			
		m_cUOrb->
			RelayDatagram ((theGuy->m_iSpeexMode !=
					0) ? "1;1;" : "1;0;",
					       theGuy->m_sDeepIP, theGuy->m_iDeepPort, true);
		if(theGuy->m_bDeepUnknown || !theGuy->m_bUseDeep)
			m_cUOrb->
				RelayDatagram ((theGuy->m_iSpeexMode !=
						0) ? "1;1;" : "1;0;",
					       theGuy->m_sIPAddress, theGuy->m_iPort, false);
			
	}
	mutexProtect.Unlock ();
}

void
CKaiEngine::RequeryArenaConnections ()
{
	CArenaItem *theGuy;
	if (m_cArenaContacts.size () < 1)
		return;

	mutexProtect.Lock();
	m_iArenaCurrentPing++;
	if (m_iArenaCurrentPing >= m_cArenaContacts.size ())
		m_iArenaCurrentPing = 0;

	theGuy = m_cArenaContacts[m_iArenaCurrentPing];
	// Set the time params
    	theGuy->Reset();
	theGuy->m_tvLastConnectionQuery = Now();

	// Tell the streamer to send it - to any non null ip
	if(theGuy->m_bDeepUnknown || theGuy->m_bUseDeep)
		m_cUOrb->RelayDatagram ("3;", theGuy->m_sDeepIP, theGuy->m_iDeepPort, true);
	if(theGuy->m_bDeepUnknown || !theGuy->m_bUseDeep)
		m_cUOrb->RelayDatagram ("3;", theGuy->m_sIPAddress, theGuy->m_iPort, false);
	mutexProtect.Unlock();
}

void
CKaiEngine::CleanMessengerContacts ()
{
	CMessengerItem *c;
	mutexProtect.Lock();
	for (unsigned int i = 0; i < m_cMessengerContacts.size (); i++)
	{
		c = m_cMessengerContacts[i];
		if (c->m_iState == 1)
		{
			if (c->LastPingAge() > 30000)
			{
				SetOfflineContact (c->m_sContactName);
				m_cTOrb->Deliver (c->m_sContactName, '6',
						  m_cConf->Username,
						  MyIP ().GetAddressString (),
						  MyPort (), 
						  (m_cConf->UseDeep?MyDeepIP().GetAddressString():""),
						  (m_cConf->UseDeep?MyDeepPort():0));
			}
		}
	}
	mutexProtect.Unlock();
}

void
CKaiEngine::OnTimer (unsigned int id)
{	
	switch (id) {
		case MESSENGER_PING_TIMER: {
			// if (m_bAttached)
				RequeryMessengerConnections ();
			break;
		}
		case MESSENGER_CLEAN_TIMER: {
			CleanMessengerContacts ();
			break;
		}
		case ARENA_BUSY_TIMER: {
			CheckBusy ();
			break;
		}
		case ARENA_PING_TIMER: {
			// Bail if not on arena...
			if(m_sCurrentArena.empty())
				return;

			int iNext = 800, i = m_cArenaContacts.size();
			if (i > 33) iNext = 600;
			if (i > 66) iNext = 400;
			if (i > 100) iNext = 200;
			TimerCtl(ARENA_PING_TIMER, ChangeTiming, iNext);
			RequeryArenaConnections();
			break;
		}
        case TRAFFIC_TIMER: {
            CalcTraffic();
            break;
        }
		case PRESTEERING_TIMER: {
			SendPresteering();
			break;
		}
		case ARENA_IDLE_TIMER: {
			CheckArenaIdle();
			break;
		}
		case ORB_REQUERY_TIMER: {
			SendClientStatus("Requerying orbital mesh...");
			m_cUOrb->RemakeSocket();
			break;
		}
	}
}

void
CKaiEngine::CheckArenaIdle()
{
	if ( TimeDelta(m_tvLastUserInteraction)>1200000 &&
		 m_sCurrentArena!="" &&
		 m_cTOrb &&
		 m_iArenaStatus<3
		 )
		 {
			// Close chat
			m_cTOrb->SendStream ("KAI_CHATMODE;;");
			// Ask orb for messenger move
			m_cTOrb->SendStream ("KAI_VECTOR;;;");
			
			// Reset clock
			m_tvLastUserInteraction=Now();
		 }
}

void 
CKaiEngine::SendPresteering()
{	
	// Send presteering data if we're hosting in mode 5
	if (m_iRoutingMode==5 &&
		m_iArenaStatus>1)
		{
			// Debug
		
			// Send presteering data to all active clients
			CArenaItem* a;
			CArenaItem* b;
			
			mutexProtect.Lock();
			for (unsigned int i = 0; i < m_cArenaContacts.size(); i++)
			{
				a = m_cArenaContacts[i];
				
				if (TimeDelta(a->m_tvLastPacketTime) < 1000)
				{
					// This guy needs to be sent presteering data
					for (unsigned int j = 0; j < m_cArenaContacts.size(); j++)
					{
						b = m_cArenaContacts[j];
						
						// As long as a isnt b, make the msg, and send it now
						if (TimeDelta(b->m_tvLastPacketTime) < 1000 && a!=b)
						{
							// Send it
							if(a->m_bUseDeep)
								m_cUOrb->RelayDatagram ("^;"+b->m_sContactName+";", a->m_sDeepIP, a->m_iDeepPort, true);
							else
								m_cUOrb->RelayDatagram ("^;"+b->m_sContactName+";", a->m_sIPAddress, a->m_iPort, false);
						}
					}
				}
			}
			mutexProtect.Unlock();
		}
}

CArenaItem *CKaiEngine::WhoHasDevice (string device)
{
	if (m_sCurrentArena.empty ())
	{	
		// Messenger Mode
		for (unsigned int i = 0; i < m_cMessengerContacts.size ();
		     i++)
			if (m_cMessengerContacts[i]->FindDevice(device) >= 0)
				return m_cMessengerContacts[i];
	}
	else
	{	
		// Arena mode
		// In the contact list - or not?
		for (unsigned int i = 0; i < m_cArenaContacts.size (); i++)
			if (m_cArenaContacts[i]->FindDevice (device) >= 0)
				return m_cArenaContacts[i];
	}
	// Otherwise, never seen - return NULL
	return NULL;
}

void
CKaiEngine::HandleEthernetFrame (const u_char * packet, int packet_size,
				 bool incoming, string sender_ip,
				 int sender_port)
{
	// Lead in time check..
	if (TimeDelta(m_tvLastVectorChange) < 5000) return;
    
	// Strip eth parts
	char aux[18];
	string sConsoleID;
	string srcmac, dstmac, srcip, dstip;
	int srcport, dstport;
	struct ether_header *ep;
	struct ether_addr *srcaddy, *dstaddy;
	struct ip *my_ip;
	struct udphdr *my_packetheader;
	ep = (struct ether_header *) packet;
	srcaddy = (struct ether_addr *) ep->ether_shost;
	dstaddy = (struct ether_addr *) ep->ether_dhost;

	#if defined(PLATFORM_bsd) && \
	    !(defined(PLATFORM_macosx_1) || defined(PLATFORM_macosx_jaguar))
	#define ether_addr_octet octet
	#endif

	// Get source MAC
	sprintf (aux, "%02X%02X%02X%02X%02X%02X",
		 srcaddy->ether_addr_octet[0], srcaddy->ether_addr_octet[1],
		 srcaddy->ether_addr_octet[2], srcaddy->ether_addr_octet[3],
		 srcaddy->ether_addr_octet[4], srcaddy->ether_addr_octet[5]);
	srcmac = aux;

	// Get dest MAC
	sprintf (aux, "%02X%02X%02X%02X%02X%02X",
		 dstaddy->ether_addr_octet[0], dstaddy->ether_addr_octet[1],
		 dstaddy->ether_addr_octet[2], dstaddy->ether_addr_octet[3],
		 dstaddy->ether_addr_octet[4], dstaddy->ether_addr_octet[5]);
	dstmac = aux;
	
	// Get IP struct, and then src and dest port.
	my_ip = (struct ip *) (packet + sizeof (struct ether_header));
	
	// Use UDP header for everything - we only need ports anyway.
	my_packetheader = (struct udphdr *) (packet +
					    sizeof (struct ether_header) +
					    ((my_ip->ip_hl) << 2));
	// Platform depend
	#ifdef PLATFORM_bsd
		srcport = ntohs (my_packetheader->uh_sport);
		dstport = ntohs (my_packetheader->uh_dport);
	#else
		srcport = ntohs (my_packetheader->source);
		dstport = ntohs (my_packetheader->dest);
	#endif

	// Identify console type
	string console = srcmac.substr(0, 6);
	
	// Is this an incoming frame? 
	if(incoming)
	{
		// Filter any remote DHCP requests - just to be safe
		if (srcport==68) return;
		
		// Remote device frame
		CArenaItem *theGuy;
		string sMsg;
		
		if (m_sCurrentArena.empty())
		{
			theGuy = FindUser(sender_ip, sender_port);
			sMsg = "KAI_CLIENT_REMOTE_DEVICE;";
		}
		else
		{
			theGuy = FindArenaUser(sender_ip, sender_port);		
			sMsg = "KAI_CLIENT_REMOTE_ARENA_DEVICE;";
		}
		if(theGuy)
		{
			if(theGuy->AddDevice(srcmac))
				SendClientResponse (sMsg + theGuy->m_sContactName + ";" + srcmac + ";");
		} else {
			return;
		}
	}
	else
	{
		// Make sure only known patterns are allowed to go
		// through. Every other mac pattern is dropped.
		bool IsXbox = false, IsPS2 = false, IsGC = false, IsPSP = false;
		sConsoleID=m_cConf->GetConsoleID(console);
		if(sConsoleID=="Xbox")
			IsXbox=true;
		else if(sConsoleID=="Playstation 2")
			IsPS2=true;
		else if(sConsoleID=="Gamecube")
			IsGC=true;
		else if(sConsoleID=="PSP")
			IsPSP=true;
		
		// Never let them through if they arent a console
		if(!IsXbox && !IsPS2 && !IsGC && !IsPSP) return;

		// If this is Xbox, never let them through if they arent sending *to* a console
		string destconsole=dstmac.substr(0,6);
		sConsoleID=m_cConf->GetConsoleID(destconsole);
		if(IsXbox && sConsoleID=="")
			return;
			
		// Do we need to DHCP this?
		if (srcport==68)
		{
			if (IsXbox && !m_cConf->XBoxHomebrew) return;
			HandleDHCPQuery(console, srcmac, packet, packet_size, incoming, sender_ip, sender_port);
			return;
		}
		
		// If this is an ARP packet, we need some special processing - because if they
		// sneak through they can screw up LAN's on the other end - causing IP
		// conflicts and shit.
		if (packet[12]==(unsigned char)8 && packet[13]==(unsigned char)6)
		{
			// All depends on console
			if (IsGC)
			{	
				// Cube processing
				if (packet[28]!=(unsigned char)0)
					if (packet[31]!=packet[11] || packet[30]!=packet[10] || packet[29]!=(unsigned char)254 || packet[28]!=(unsigned char)10 )
					{
						// Bail out - this device hasn't been configured right..
						FailedDevice(srcmac);
						return;
					}	
			}
			
			if (IsPS2)
			{
				// PS2 Processing
				if (packet[28]!=(unsigned char)0)
					if (packet[31]!=packet[11] || packet[30]!=packet[10] || packet[29]!=(unsigned char)253 || packet[28]!=(unsigned char)10 )
					{
						// Bail out - this device hasn't been configured right..
						FailedDevice(srcmac);
						return;
					}	
			}
			
			if (IsXbox)
			{
				// Xbox Processing
				if (packet[28]!=(unsigned char)0)
					if (packet[31]!=packet[11] || packet[30]!=packet[10] || packet[29]!=(unsigned char)252 || packet[28]!=(unsigned char)10 )
					{
						// Bail out - this device hasn't been configured right..
						FailedDevice(srcmac);
						return;
					}	

			}
		}
		else
		{
			// It wasnt ARP - so we need to verify the IP address is one assigned by us, then it's
			// all good for processing.
		
			// For xboxes, we dont check this if its from 0.0.0.1
			if (IsXbox &&  packet[26]==(unsigned char)0)
			{
				// A retail xbox - cool - we just let these bad boys through..
			}
			else
			if (!IsPSP) // We never DHCP PSP's
			{
				unsigned char cSegment=(unsigned char)254; // GCN
				if (IsPS2) cSegment=(unsigned char)253;
				if (IsXbox) cSegment=(unsigned char)252; // PS2 or XBox
				
				if ( packet[29]!=packet[11] || packet[28]!=packet[10] ||
					packet[27]!=cSegment || packet[26]!=(unsigned char)10 )
				{
					FailedDevice(srcmac);
					return;
				}
			}
		}		
	}

	// If we got here, pass to general Engine Worker
	HandleFrame(console, dstmac, srcmac, packet, packet_size, incoming, sender_ip, sender_port);
}

int
CKaiEngine::FindLocalDevice(string device)
{
	for (unsigned int i=0; i < m_cDevices.size(); i++) {
		if(m_cDevices[i]->m_sMacAddress == device)
			return 1;
	}
	return -1;
}

int
CKaiEngine::FindFailedDevice(string device)
{
	for (unsigned int i=0; i < m_cFailedDevices.size(); i++) {
		if(m_cFailedDevices[i]->m_sMacAddress == device)
			return 1;
	}
	return -1;
}

void
CKaiEngine::FailedDevice(string device)
{
	// Remove from local devices, if there
	if (FindLocalDevice(device)>-1)
	{
		RemoveLocalDevice(device);
	}
	
	// Add to failed, if not there
	if (FindFailedDevice(device)==-1)
	{
		verbositylog(2,"DHCP","Device failed to configure: "+device);
		AddFailedDevice(device);
	}
}

void
CKaiEngine::HandleDHCPQuery (string console, string srcmac,
							 const u_char * packet, int packet_size,
							 bool incoming, string sender_ip, int sender_port)
{

	// Can it really be :)?
	if (packet_size<285) return;
	
	// Identify IP segment from device class
	unsigned int uiIPSegment;
	string sConsoleID;
	
	sConsoleID=m_cConf->GetConsoleID(console);
	if(sConsoleID=="Xbox")
		uiIPSegment=252;
	else if(sConsoleID=="Playstation 2")
		uiIPSegment=253;
	else if(sConsoleID=="Gamecube")
		uiIPSegment=254;
	else
		return;
	
	// Get transaction ID from frame
	u_char trans_id[4];
	trans_id[0]=packet[46];
	trans_id[1]=packet[47];
	trans_id[2]=packet[48];
	trans_id[3]=packet[49];
	
	// Establish DHCP call type
	if ( packet[284]==(unsigned char)1 )
	{
		verbositylog(2,"DHCP","Attempting to configure (Offer): "+srcmac);
		
		// Discover sent - reply with offer
		m_ucaDHCPOffer[2]=packet[6];
		m_ucaDHCPOffer[3]=packet[7];
		m_ucaDHCPOffer[4]=packet[8];
		m_ucaDHCPOffer[5]=packet[9];
		m_ucaDHCPOffer[6]=packet[10];
		m_ucaDHCPOffer[7]=packet[11];
		m_ucaDHCPOffer[72]=packet[6];
		m_ucaDHCPOffer[73]=packet[7];
		m_ucaDHCPOffer[74]=packet[8];
		m_ucaDHCPOffer[75]=packet[9];
		m_ucaDHCPOffer[76]=packet[10];
		m_ucaDHCPOffer[77]=packet[11];
		
		// Set assigned IP
		m_ucaDHCPOffer[60]=(unsigned int)10;
		m_ucaDHCPOffer[61]=uiIPSegment;
		m_ucaDHCPOffer[62]=packet[10];
		m_ucaDHCPOffer[63]=packet[11];

		// Set new transid
		m_ucaDHCPOffer[48]=trans_id[0];
		m_ucaDHCPOffer[49]=trans_id[1];
		m_ucaDHCPOffer[50]=trans_id[2];
		m_ucaDHCPOffer[51]=trans_id[3];
		
		// Send
		InjectPacket(m_ucaDHCPOffer+2,344);
	}
	else
	{
		verbositylog(2,"DHCP","Attempting to configure (Ack): "+srcmac);				
		
		// Request sent - reply with ack
		m_ucaDHCPAck[2]=packet[6];
		m_ucaDHCPAck[3]=packet[7];
		m_ucaDHCPAck[4]=packet[8];
		m_ucaDHCPAck[5]=packet[9];
		m_ucaDHCPAck[6]=packet[10];
		m_ucaDHCPAck[7]=packet[11];
		m_ucaDHCPAck[72]=packet[6];
		m_ucaDHCPAck[73]=packet[7];
		m_ucaDHCPAck[74]=packet[8];
		m_ucaDHCPAck[75]=packet[9];
		m_ucaDHCPAck[76]=packet[10];
		m_ucaDHCPAck[77]=packet[11];
		
		// Set assigned IP
		m_ucaDHCPAck[60]=(unsigned int)10;
		m_ucaDHCPAck[61]=uiIPSegment;
		m_ucaDHCPAck[62]=packet[10];
		m_ucaDHCPAck[63]=packet[11];

		// Set new transid
		m_ucaDHCPAck[48]=trans_id[0];
		m_ucaDHCPAck[49]=trans_id[1];
		m_ucaDHCPAck[50]=trans_id[2];
		m_ucaDHCPAck[51]=trans_id[3];

		// Send
		InjectPacket(m_ucaDHCPAck+2,344);	
		}
					
}

void
CKaiEngine::verbositylog(int level, string sSection, string sMessage)
{
	if (level<=m_cConf->Verbosity)
		debuglog(sSection,sMessage);
}

void
CKaiEngine::HandleFrame (string console, string dstmac, string srcmac,
			     const u_char * packet, int packet_size,
			     bool incoming, string sender_ip, int sender_port)
{
	// Get opcode
    int psize = packet_size + 2;
    u_char newbuf[psize];
    char Opcode;
    string sConsoleID;
	
	sConsoleID=m_cConf->GetConsoleID(console);
	// Set byte 0 of outgoing based on console
	if(sConsoleID=="Xbox")
		Opcode = m_sCurrentArena.empty () ? 'X' : 'R'; // Box
	else if (sConsoleID=="Gamecube") 
		Opcode = m_sCurrentArena.empty () ? 'Y' : 'S'; // Cube
	else if ((sConsoleID=="Playstation 2") || (sConsoleID=="PSP"))
		Opcode = m_sCurrentArena.empty () ? 'p' : 'q'; // PS2 / PSP    
	else return;
	
	memcpy (newbuf + 2, packet, packet_size);
    newbuf[0] = Opcode;
    newbuf[1] = ';';

	// If data came from local network - make sure its not a remote packet that just got injected
	// - but only we're using LearningModes Auto, FirstRun or FirstFound.			
	if(!incoming)
	{
		// Is it destined for a local device?
		if (FindLocalDevice(dstmac)>-1)
		{
			return;
		}
		
		// Check if ether src is remote - if it is, we just bail - it's a loopback
		if(WhoHasDevice(srcmac)!=NULL) 
		{
			return;
		}
			
		// So, it's local, destined for a remote - what if it's not registered local yet?
		if(FindLocalDevice(srcmac) < 0)
		{
			
			// OK.. we need to add this local device in and tell the UI.
			string aux = "";
			
			// Build MAC address and add
			for(unsigned int i = 0; i < srcmac.size(); i++)
			{
				aux += srcmac[i]; i++;
				aux += srcmac[i];
				if (i<(srcmac.size()-1)) aux += ":";
			}
			
			// Add it
			//aux += ";";
			//string eth_address = aux;

			// Add the local device
			verbositylog(1,"CONSOLE","Console Added...");
			AddLocalDevice(srcmac);
			
			// Also, if it's failed before, it's not failed now
			if (FindFailedDevice(srcmac)!=-1)
				RemoveFailedDevice(srcmac);
			
			// Tell the UI
			SendClientResponse("KAI_CLIENT_LOCAL_DEVICE;"+case_upper(crop(srcmac, ':'))+";");
		}
	}			
	
	// OK - it's handleable - let's go..
	if (m_sCurrentArena.empty ())
	{
		if (incoming)
		{
			if (dstmac=="FFFFFFFFFFFF") m_iDownBroadcastCount++; else m_iDownDirectCount++;
			InjectPacket (packet, packet_size);
		}
		else
		{
			// Send it out 
			if (dstmac=="FFFFFFFFFFFF") m_iUpBroadcastCount++; else m_iUpDirectCount++;
			
			// Local data
			m_tvLastUserInteraction=Now();
			
			CMessengerItem *c = (CMessengerItem*)WhoHasDevice(dstmac);			

			// Broadcast or normal?
			if (dstmac == "FFFFFFFFFFFF" || !c)
			{
				// Broadcast packet to all greens
				for (unsigned int i = 0;
				     i < m_cMessengerContacts.size (); i++)
				{
					c = m_cMessengerContacts[i];
					if (c->m_iState == 1)
					{
						if(c->m_bUseDeep)
							m_cUOrb->RelayPacket(newbuf, psize, c->m_sDeepIP, c->m_iDeepPort, true);
						else
							m_cUOrb->RelayPacket(newbuf, psize, c->m_sIPAddress, c->m_iPort, false);
					}
				}
			}
			else
			{
				// Identified target
				if(c->m_bUseDeep)
					m_cUOrb->RelayPacket (newbuf, psize,c->m_sDeepIP,c->m_iDeepPort,true);
				else			
					m_cUOrb->RelayPacket (newbuf, psize,c->m_sIPAddress,c->m_iPort,false);
			}
		}
	}
	else
	{	
		// Must be arena
		if (incoming) 
		{
			// Data came from a remote source - handle and inject
			CArenaItem *theGuy = FindArenaUser (sender_ip, sender_port);
			if(theGuy)
			{
				if(dstmac == "FFFFFFFFFFFF")
				{
					m_iDownBroadcastCount++;
					theGuy->m_tvLastBroadcastData = Now();
				}
				else
				{
					m_iDownDirectCount++;
					theGuy->m_tvLastPacketTime = Now();
				}
            }
			InjectPacket(packet, packet_size);
		 }
		 else
		 {
			// Send it out 
			if (dstmac=="FFFFFFFFFFFF") m_iUpBroadcastCount++; else m_iUpDirectCount++;

			// User interaction
			m_tvLastUserInteraction=Now();
			 
			// This now based on routing settings ... lets check
			bool bProcess = false;
			
			CArenaItem *a = WhoHasDevice(dstmac);
             
			if (dstmac == "FFFFFFFFFFFF" || !a )
			{
                // Broadcast packet to all greens
				for (unsigned int i = 0; i < m_cArenaContacts.size(); i++)
                {
					bProcess = false;
					a = m_cArenaContacts[i];
					// Switch routing
					switch(m_iRoutingMode)
					{
						case 0:
						{
							// Standard kai routing - this goes to everyone
							// who isnt busy right now.
							bProcess = (a->m_iLastStatus != 1);
							break;
						}
						case 1:
						{
							// Open Kai routing - this goes to *everyone*
							// regardless of our/their status
							bProcess = true;
							break;
						}
						case 2:
						{
							// Strict Kai routing - broadcasts go to hosts
							// only - unless we're host, then all
							if (a->m_iLastStatus > 1 ||
							    m_iArenaStatus>1)
								bProcess=true;
							break;
						}
						case 3:
						{
							// Strict Kai routing - broadcasts go to hosts
							// only - unless we're host, then all
							if (m_iArenaStatus<2)
							{
								if (a->m_iLastStatus > 1 ||
								    TimeDelta(a->m_tvLastBroadcastData) < 1000)
									bProcess=true;
							}
							else
							{
								if (a->m_iLastStatus < 2 &&
								    TimeDelta(a->m_tvLastBroadcastData) < 1000)
									bProcess=true;
							}
							break;
						}
						case 4:
						{
						    // Same as 3, but takes into account direct data also
							if (m_iArenaStatus<2)
							{
								if (a->m_iLastStatus > 1 ||
								    (TimeDelta(a->m_tvLastBroadcastData) < 1000) ||
									(TimeDelta(a->m_tvLastPacketTime) < 1000))
									bProcess=true;
							}
							else
							{
								if (a->m_iLastStatus < 2 &&
								    (TimeDelta(a->m_tvLastBroadcastData) < 1000) ||
									(TimeDelta(a->m_tvLastPacketTime) < 1000))
									bProcess=true;
							}
							break;
						}
						case 5:
						{
							// Host managed mode 5 - pretty similar to 4, apart from a clever check if its really FFFFFFFFFFFF
							// or not - and inclusion of the presteering from the host.
							if (m_iArenaStatus>1)
							{
							    // We're a host - send it out :)
								if (a->m_iLastStatus < 2 &&
								    (TimeDelta(a->m_tvLastBroadcastData) < 1000) ||
									(TimeDelta(a->m_tvLastPacketTime) < 1000 || Future(a->m_tvLastPacketTime) ))
									bProcess=true;
							}
							else
							{
								if (m_bBusy || dstmac!="FFFFFFFFFFFF")
								{
									// We're busy.. send to all non-hosts we are active with, or have been sent through presteering.
									// Also force this path if it's not a real broadcast.
									if (a->m_iLastStatus < 2 &&
								    (TimeDelta(a->m_tvLastBroadcastData) < 1000) ||
									(TimeDelta(a->m_tvLastPacketTime) < 1000 || Future(a->m_tvLastPacketTime)))
									bProcess=true;
								}
								else
								{
								    // We're not busy.. send to all hosts, and all active contacts - including presteering.
									if (a->m_iLastStatus > 1 ||
								    (TimeDelta(a->m_tvLastBroadcastData) < 1000) ||
									(TimeDelta(a->m_tvLastPacketTime) < 1000 || Future(a->m_tvLastPacketTime)))
									bProcess=true;
								}
							}
							break;
						}
						case 6:
						{
							// EXT_CAP routing - not implemented as yet in *nix core.
							break;
						}
					}
							 
					if ( bProcess )
					{						
                        // Send RAW over internet
                        		if(a->m_bUseDeep)
                        			m_cUOrb->RelayArenaPacket(newbuf,psize,a->m_sDeepIP,
                        								 a->m_iDeepPort, true);
                        		else
                            		m_cUOrb->RelayArenaPacket(newbuf, psize, a->m_sIPAddress,
                             							 a->m_iPort, false);
                    }
                }
                        }
                        else
                        {
						if(a->m_bUseDeep)
                        			m_cUOrb->RelayArenaPacket(newbuf,psize,a->m_sDeepIP,
                        								 a->m_iDeepPort, true);
						else
							m_cUOrb->RelayArenaPacket (newbuf, psize, a->m_sIPAddress,
                                     					  a->m_iPort, a->m_bUseDeep);
	
			}
		}
    }
}

int
CKaiEngine::InjectPacket (const u_char * packet, int packet_size)
{
	int x = m_cSniffer->Inject(packet, packet_size);
	if(x == packet_size)
		{
		}
	
	return x;
}

void
CKaiEngine::SendLocalConsoleData ()
{
    vector<CKaiDevice*>::iterator i;
    for (i = m_cDevices.begin(); i != m_cDevices.end(); i++)
    {
        if ((*i)->m_bDetected)
        {
			SendClientResponse("KAI_CLIENT_LOCAL_DEVICE;" + case_upper(crop((*i)->m_sMacAddress, ':')) + ";");
        }
    }

	for (i = m_cFailedDevices.begin(); i != m_cFailedDevices.end(); i++)
    {
			SendClientResponse("KAI_CLIENT_DHCP_FAILURE;" + case_upper(crop((*i)->m_sMacAddress, ':')) + ";");
    }

}

void
CKaiEngine::Shutdown ()
{
	m_bShuttingDown = true;
	m_bTerminate = true;
	m_bAttached = false;
}

void
CKaiEngine::ReAttach ()
{
    unsigned int i;
	// send logged in msg - the client will just wait for info
	SendClientResponse ("KAI_CLIENT_LOGGED_IN;");
	SendClientResponse ("KAI_CLIENT_STATUS;XLink Kai is Online..;");
	SendClientResponse ("KAI_CLIENT_USER_DATA;" + m_cConf->Username + ";");
	SendClientResponse ("KAI_CLIENT_ARENA_STATUS;" + Str(m_iArenaStatus) + ";" +
		Str(m_iArenaMyPlayers) + ";");
		
	// send arena mode
	if (!m_sCurrentArena.empty ())
	{
		SendClientResponse (m_sLastVectorChange);
		SendClientResponse ("KAI_CLIENT_CONNECTED_ARENA;");
	}
	else
	{
		SendClientResponse ("KAI_CLIENT_CONNECTED_MESSENGER;");
	}

	// send chat room
	SendClientResponse ("KAI_CLIENT_CHATMODE;" + m_sChatMode + ";");

	// send messenger contacts and state	
	for (i = 0; i < m_cMessengerContacts.size (); i++)
	{		
		SendClientResponse ("KAI_CLIENT_ADD_CONTACT;" +
				    m_cMessengerContacts[i]->m_sContactName + ";");
		if (m_cMessengerContacts[i]->m_iState)
		{
			// He's Online, inform UI
			SendClientResponse ("KAI_CLIENT_CONTACT_ONLINE;" +
					    m_cMessengerContacts[i]->m_sContactName + ";");
		}
        	if (m_cMessengerContacts[i]->m_iSpeexMode == 1)
        	{
            		SendClientResponse("KAI_CLIENT_SPEEX_ON;" +
                               m_cMessengerContacts[i]->m_sContactName + ";");
            		if (m_cMessengerContacts[i]->m_bSpeexConnected)
                		SendClientResponse("KAI_CLIENT_SPEEX_CONNECTED;" +
                                   m_cMessengerContacts[i]->m_sContactName+";");
        	}        
	}
	// Now send invites
	for (i = 0; i < m_sInvites.size(); i++)
	{
		SendClientResponse(m_sInvites[i]);
	}
	m_sInvites.clear();

    if(!m_sCurrentArena.empty())
    {
        for(i = 0; i < m_cArenaContacts.size(); i++)
        {
            SendClientResponse("KAI_CLIENT_JOINS_VECTOR;" +
                               m_cArenaContacts[i]->m_sContactName + ";");
            if(!(m_cArenaContacts[i]->m_tvLastCompletePing.tv_sec == 0 &&
               m_cArenaContacts[i]->m_tvLastCompletePing.tv_usec == 0))
            {
                SendClientResponse("KAI_CLIENT_ARENA_PING;" +
                    m_cArenaContacts[i]->m_sContactName + ";" +
                    Str(m_cArenaContacts[i]->m_iLastPing) + ";" + 
                    Str(m_cArenaContacts[i]->m_iLastStatus) + ";" +
                    Str(m_cArenaContacts[i]->m_iLastPlayers) +";");
            }
        }
    }
    
	SendLocalConsoleData();
	
    // send privs (admins first)
    SendClientResponse("KAI_CLIENT_ADMIN_PRIVILEGES;" + m_sStoredAdmins + ";");
    SendClientResponse(m_sStoredPrivs);
	
    // send chat ppl
    for (i=0; i < m_svChatUsers.size(); i++)
    {
        SendClientResponse("KAI_CLIENT_JOINS_CHAT;" + m_sChatMode + ";" +
                           m_svChatUsers[i] + ";");
    }
}

void CKaiEngine::StartTimer(const unsigned int TimerID)
{
	TimerCtl(TimerID, Start);
}

void CKaiEngine::StopTimer(const unsigned int TimerID)
{
	TimerCtl(TimerID, Stop);
}

void CKaiEngine::TimerCtl(const unsigned int TimerID, TimerControl Operation, int data)
{
	CTimerThread* thisTimer;
	for(unsigned int i = 0; i < m_cTimers.size(); i++) {
		thisTimer = m_cTimers[i];
		if (thisTimer->ID() == TimerID) {
			switch(Operation) {
				case Start: {
					thisTimer->Start();
					break;
				}
				case Stop: {
					thisTimer->Stop();
					break;
				}
				case ChangeTiming: {
					thisTimer->SetTimer(data);
				}
			}
			return;
		}
	}
	return;
}

void CKaiEngine::CheckBusy()
{
	m_bBusy = (m_iBusyCounter > (m_iBusyLevel * 5));
	m_iBusyCounter = 0;
}

void CKaiEngine::CalcTraffic()
{
    // Engine <-> Engine
    m_iUpEngine = m_iUpEngineCount;
    m_iUpEngineCount = 0;
    m_iDownEngine = m_iDownEngineCount;
    m_iDownEngineCount = 0;
    
    // Orb
    m_iUpChat = m_iUpChatCount;
    m_iUpChatCount = 0;
    m_iDownChat = m_iDownChatCount;
    m_iDownChatCount = 0;
    
    // Chat
    m_iUpOrb = m_iUpOrbCount;
    m_iUpOrbCount = 0;
    m_iDownOrb = m_iDownOrbCount;
    m_iDownOrbCount = 0;

	// Broadcast Frames
    m_iUpBroadcast = m_iUpBroadcastCount;
    m_iUpBroadcastCount = 0;
    m_iDownBroadcast = m_iDownBroadcastCount;
    m_iDownBroadcastCount = 0;

	// Direct Frames
    m_iUpDirect = m_iUpDirectCount;
    m_iUpDirectCount = 0;
    m_iDownDirect = m_iDownDirectCount;
    m_iDownDirectCount = 0;

}

void CKaiEngine::CheckIfSpeexNeeded()
{
	unsigned int i;
    // If speex off, do we need it?	
    if (m_iSpeexReq == 0)
    {
        for (i = 0; i < m_cMessengerContacts.size(); i++)
        {
            if (m_cMessengerContacts[i]->m_bSpeexConnected)
            {
                // Yep - we do
                m_iSpeexReq = 1;
                SendClientResponse("KAI_CLIENT_SPEEX_START;");
                return;
            }
        }
    }
    else
    {
        // Its on, but do we need it?
        for (i = 0; i < m_cMessengerContacts.size(); i++)
        {
			if (m_cMessengerContacts[i]->m_bSpeexConnected)
				return; // Bail
        }
		// Not needed
		m_iSpeexReq = 0;
		SendClientResponse("KAI_CLIENT_SPEEX_STOP;");
    }
}

void CKaiEngine::CleanOldInvitation(string sFrom)
{
        vector < string >::iterator i;
        for (i = m_sInvites.end (); i != m_sInvites.begin (); i--)
		{
			vector<string> sInvite;
	        Tokenize(*i, sInvite, ";");
            if ( sInvite[1] == sFrom )
			{
				m_sInvites.erase(i);
				return;
			}
		}
}

void CKaiEngine::AddLocalDevice(string mac)
{
	CKaiDevice *d = new CKaiDevice(mac, true);
	m_cDevices.push_back(d);
	
	// Have we hit the lock limit?
	if (!m_bLockedPcap && m_cConf->LocalDevices>0 && m_cDevices.size()==m_cConf->LocalDevices)
	{
		m_bLockedPcap = true;
		
		// Build lock string, then call sniffer function to lock
		string sLock="";
		for (unsigned int i=0; i<m_cConf->LocalDevices; i++)
		{	
			string sTemp=m_cDevices[i]->m_sMacAddress;
			sTemp=sTemp.substr(0,2)+":"+sTemp.substr(2,2)+":"+
				  sTemp.substr(4,2)+":"+sTemp.substr(6,2)+":"+
				  sTemp.substr(8,2)+":"+sTemp.substr(10,2);
			sLock+=sTemp+";";
		}
		
		// Log and execute
		verbositylog(1,"KAID","Locking consoles...");
		m_cSniffer->LockConsoles(sLock);
	}
}

void CKaiEngine::DetectedDevice(string mac)
{
	bool IsAnyDeviceMissing = false;
	vector <CKaiDevice*>::iterator i;
    for (i = m_cDevices.begin(); i != m_cDevices.end(); i++)
    {
    	if((*i)->m_sMacAddress != mac)
		{
			if(!(*i)->m_bDetected)
				IsAnyDeviceMissing = true;
        }
		else
		{
			(*i)->m_bDetected = true;
		}
	}
	m_bSeenAllDevices = !IsAnyDeviceMissing;
}

