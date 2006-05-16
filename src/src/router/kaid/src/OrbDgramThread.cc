#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <socketcc.h>
#include <pthreadcc.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/time.h>

#include "Kaid.h"
#include "KaiEngine.h"
#include "OrbDgramThread.h"
#include "PktSnifferThread.h"
#include "KaiItem.h"
#include "ConfigFile.h"

using namespace std;

MutualExclusion	COrbDgramThread::mutexProtect;
	
COrbDgramThread::COrbDgramThread(CKaiEngine *cParent, CConfigFile* cConf)
{
	m_bTerminate = true;
	m_cParent = cParent;
	m_cParent->verbositylog(2,"THREAD","Datagram server thread started...");
    m_cConf = cConf;
	m_cListeningSocket = NULL;
	m_iMyPort = 0;
	m_iSourcePort = 0;
	m_bOrbFound = false;
}

COrbDgramThread::~COrbDgramThread()
{
	m_cParent->verbositylog(2,"THREAD","Datagram server thread stopped...");
	m_bTerminate = true;
	if(m_cListeningSocket)
		delete m_cListeningSocket;
}

void COrbDgramThread::RemakeSocket()
{	
	// Destroy if exists
	if (m_cListeningSocket) delete m_cListeningSocket;
	m_cListeningSocket=NULL;
	
	int	iPort = m_cConf->EnginePort;

	// Create Orb listener/worker
	try
	{
		if( m_cConf->EngineIP.empty())
		{
			m_cListeningSocket = new UDPServerSocket(iPort);
		}
		else
		{
			IPAddress cBind;
			cBind = m_cConf->EngineIP.c_str();
			m_cListeningSocket = new UDPServerSocket (cBind, iPort);
		}
	}
	catch(...)
	{
		debuglog("KAID","Failed to bind Engine socket...");
		exit(1);
	}

}
void COrbDgramThread::Start()
{
	m_bTerminate = false;
	LaunchThread();
}

void COrbDgramThread::Stop()
{
    m_bTerminate = true;
}


void *COrbDgramThread::Execute()
{
    int     	iBytesTransferred, retval;
    char    	pcBuffer[2048];
	bool		GamePacket = false;
	SocketSet   mySet;
	
    mySet += (struct SocketBase *)m_cListeningSocket;
	while( !m_bTerminate && !m_cParent->m_bShuttingDown)
    {
        try
        {
		 	retval = select((int)mySet + 1, mySet, NULL, NULL, NULL);
			if(retval == -1)
				continue;

            iBytesTransferred = m_cListeningSocket->ReceiveDatagram(pcBuffer,
                sizeof(pcBuffer) - 1, m_cIPSource, m_iSourcePort);
            
            if(m_bTerminate)
                return NULL;
            
            if(iBytesTransferred > 0) {
				m_cParent->m_iDownEngineCount += iBytesTransferred;
                pcBuffer[iBytesTransferred] = 0;
                m_sRequest = pcBuffer;
                string Opcode = m_sRequest.substr(0, 2);
                if(Opcode == "!;")
                {
                	HandleSpeex(&pcBuffer, iBytesTransferred, m_cIPSource.GetAddressString(), m_iSourcePort);
                } else {
					GamePacket = ( Opcode == "X;" || Opcode == "R;" || Opcode == "p;" || Opcode == "q;" || Opcode == "Y;" || Opcode == "S;");
					if( GamePacket ) {
						m_cParent->HandleEthernetFrame((u_char *)pcBuffer + 2,
							iBytesTransferred - 2, true, m_cIPSource.GetAddressString(), m_iSourcePort);
					} else {
						if(m_sRequest != "K;") {
							HandleOrbDatagram(m_sRequest, m_cIPSource, m_iSourcePort);
						}
					}
					if( m_sAnswer != "" ) {
						// Empty ?
					}
				}
            }
		}
		catch (...)
		{
			m_bTerminate = true;
            return NULL;
		}
    }
	return NULL;
}

IPAddress& COrbDgramThread::FindNearestOrb()
{
	const size_t BUFFER_SIZE = 65535;
	
	m_bOrbFound=false;
	if (!m_cListeningSocket) return m_cNearestOrb;
	
	unsigned int i;
	int iBytesTransferred, iSrcPort;
	char *pcBuffer = new char[BUFFER_SIZE];
	IPAddress pcAddr;
    vector<string> vsOrbs;
    vector<string> vsSegments;
    string sAux;
	int	iPort = m_cConf->EnginePort;
	
    try
    {
        Tokenize(m_cConf->OrbList, vsOrbs, ";");
        for(i = 0; i < vsOrbs.size(); i++) 
		{
            pcAddr = vsOrbs[i].c_str();
			
			if (m_cConf->EnginePAT==1)
			{
				sAux = "KAI_ORBITAL_REGISTER;" + m_cConf->Username + ";0;;0;";
			}
			else
			{
				sAux = "KAI_ORBITAL_REGISTER;" + m_cConf->Username + ";" + Str(iPort) + ";;0;";
			}
            
            string a = pcAddr.GetAddressString(); 
            m_cListeningSocket->SendDatagram(sAux.c_str(), sAux.size(), pcAddr,
                m_cConf->OrbPort);
        }

        iBytesTransferred = m_cListeningSocket->ReceiveDatagram(pcBuffer, BUFFER_SIZE - 1,
              m_cNearestOrb, iSrcPort);
			 		
        pcBuffer[iBytesTransferred] = 0;
        sAux = pcBuffer;
        Tokenize(sAux, vsSegments, ";");
        m_cMyIP = vsSegments[1].c_str();
        m_iMyPort = atoi((vsSegments[2].c_str()));

		if(m_cConf->EnginePort == 0)
			m_cConf->EnginePort = m_iMyPort;

        string a = m_cNearestOrb.GetAddressString();
		m_bOrbFound=true;
		
    }
    catch (SocketException &excep)
    {
    }
    catch (...)
    {
    }
	
	delete pcBuffer;
	return m_cNearestOrb;
}

void COrbDgramThread::HandleOrbDatagram(string dgram, IPAddress& srchost,
    int srcport)
{
    // Too small?
    if (dgram.size()<2) return;
    // Verify 2nd char is ";" etc.
    char Opcode = dgram.at(0);
    if (dgram.at(1) != ';') return;

    vector<string> vsSegments;
    Tokenize(dgram, vsSegments, ";");

    switch(Opcode) {
        case 'P':
			{
            	// A client has sent a PM to us.. just 4ward it to the UI
            	m_cParent->SendClientResponse("KAI_CLIENT_PM;" + vsSegments[1] +
                	";" + vsSegments[2]+ ";");
            	break;
			}
        case '$': {
            // A client has sent an arena PM to us.. just 4ward it to the UI (think?)
            m_cParent->SendClientResponse("KAI_CLIENT_ARENA_PM;" +
                vsSegments[1] + ";" + vsSegments[2] + ";");
            break;
        }

		case '^':
		{
			// Do we ned to process this? 
			if (m_cParent->m_iRoutingMode!=5 ||
			    m_cParent->m_iArenaStatus>1) return;
				
			// Presteering uses security too :)
			CArenaItem* theGuy = m_cParent->FindArenaUser(m_cIPSource.GetAddressString(), m_iSourcePort); // AJS MOD 29/1/05
			if(!theGuy)
				break;
			
			// A host is asking us to presteer broadcast / untargeted broadcast frames to this guy.. fair enough - update the guys records..
			if (vsSegments.size()<2) return;
		
			// OK - find the guy, set the last data thing
			CArenaItem *a = m_cParent->FindArenaUser(vsSegments[1]); // AJS MODE 29/1/05
			
			if (a)
			{
				// Set
				a->m_tvLastPacketTime=Now();
				a->m_tvLastPacketTime.tv_sec+=10; // Eww?
			}
			else
			{
			   // Weird..
			}
			
			break;
		}

        case '1':
		{
			m_cParent->m_sReachable = "Yes";
			CMessengerItem* theGuy = m_cParent->FindUser(m_cIPSource.GetAddressString(), m_iSourcePort);
			if(!theGuy)
				break;
			
			string Name = theGuy->m_sContactName;
			RelayDatagram("2;" + m_cParent->m_sCurrentArena + ";" + m_cParent->m_sUICaps + ";",
				srchost.GetAddressString(), srcport);

			if(vsSegments[1] == "1")
			{
				if(theGuy->m_iSpeexMode == 1)
				{
					if(!theGuy->m_bSpeexConnected)
					{
						theGuy->m_bSpeexConnected = true;
						m_cParent->SendClientResponse("KAI_CLIENT_SPEEX_CONNECTED;" + Name + ";");
					}
				}
				else
				{
					if(!theGuy->m_bSpeexConnected)
						m_cParent->SendClientResponse("KAI_CLIENT_SPEEX_RING;" + Name + ";");
				}
			}
			
			if (vsSegments[1] == "0")
			{
				// This users speex has not allowed us.. so we're not connected whatever
				if (theGuy->m_bSpeexConnected)
				{
					// We were talking.. turn it off
					theGuy->m_bSpeexConnected = true;
					m_cParent->SendClientResponse("KAI_CLIENT_SPEEX_DISCONNECTED;" + Name + ";");
				}
				
			}
			m_cParent->CheckIfSpeexNeeded();
			break;
		}
        case '2':
			{
				// A client has responded to our messenger mode ping request
				CMessengerItem* theGuy = m_cParent->FindUser(srchost.GetAddressString(), srcport);
				string		sPingMsg;
				
				if(!theGuy) {
					return;
				}
				// OK - this guy has responsed - so we can maybe null one of the things
				// TODO: Deep stuff, if is deep, clear normal ip and port
				// If normal, clear deep fields
	
				// OK - this dude is for real.. send the shit back to the client
				theGuy->m_iLastPing = theGuy->Ping();
	
				// Protect us from old engine msgs
				if (vsSegments.size() < 3)
					vsSegments.push_back("");
				sPingMsg = "KAI_CLIENT_CONTACT_PING;" + theGuy->m_sContactName +
					";" + vsSegments[1]  + ";" + Str(theGuy->m_iLastPing) + ";" +
					vsSegments[2] + ";";
				m_cParent->SendClientResponse(sPingMsg);
	
				// Reset the lastquery and response
				theGuy->Reset();
				break;
			}
		case '3':
			{
				// A client is requesting arena mode ping - just respond
				m_cParent->m_sReachable = "Yes";
				string temp; int output;
				if (m_cParent->m_iArenaStatus == 1) {
					output = (m_cParent->m_bBusy ? 1 : 0);
				} else {
					output = m_cParent->m_iArenaStatus;
				}
				temp = "4;" + Str(output) + ";" + Str(m_cParent->m_iArenaMyPlayers) + ";" +
						m_cParent->m_sUICaps + ";";
				RelayDatagram(temp, srchost.GetAddressString(), srcport);
				break;
			}
		case '4':
		{
        	// A client has responded to our messenger mode ping request
            CArenaItem* theGuy = m_cParent->FindArenaUser(srchost.GetAddressString(), srcport);
            string      sPingMsg;

            if(!theGuy) {
                return;
			}
            // OK - this guy has responsed - so we can maybe null one of the things
            // TODO: Deep stuff, if is deep, clear normal ip and port
            // If normal, clear deep fields

            // OK - this dude is for real.. send the shit back to the client
			theGuy->m_iLastStatus = atoi(vsSegments[1].c_str());
            theGuy->m_iLastPing = theGuy->Ping();
            theGuy->m_iLastPlayers = atoi(vsSegments[2].c_str());
            
			if ( vsSegments.size() < 4 )
				vsSegments.push_back("0");
            sPingMsg = "KAI_CLIENT_ARENA_PING;" + theGuy->m_sContactName +
                    ";" + Str(theGuy->m_iLastPing) + ";" + vsSegments[1]  +
                    ";" + vsSegments[2] + ";" + vsSegments[3] + ";";
            m_cParent->SendClientResponse(sPingMsg);

			// Reset the lastquery and response
			theGuy->Reset();
				
			break;
		}
        case '}':
		{
			// A client has sent us an invite - send to UI if we have one. If not, queue
			string sInvite = "KAI_CLIENT_INVITE;" + vsSegments[1] + ";" +
					vsSegments[2] + ";" + tstamp() + ";";
			
			if (m_cParent->m_bAttached)
			{
				// We have a UI - send straight down
				m_cParent->SendClientResponse(sInvite);
			}
			else
			{
				// We dont have a UI - remember for next attach
				m_cParent->CleanOldInvitation(vsSegments[1]);
				m_cParent->m_sInvites.push_back(sInvite);
			}
			break;
		}					
	}
}

void COrbDgramThread::RelayDatagram(string msg, string sAddr, int iPort)
{
    if(!m_cListeningSocket) return;
   	m_cParent->m_iUpEngineCount += msg.size();
    IPAddress   cDest;
    cDest = sAddr.c_str();
    m_cListeningSocket->SendDatagram(msg.c_str(), msg.size(), cDest, iPort);
}

void COrbDgramThread::RelayPacket(void *packet, int packet_size, string sAddr,
	int iPort)
{
    if(!m_cListeningSocket) return; 
    IPAddress   cDest;
    cDest = sAddr.c_str();
    m_cParent->m_iUpEngineCount += packet_size; 
    string a = cDest.GetAddressString();
	m_cListeningSocket->SendDatagram(packet, packet_size, cDest, iPort);
}

void COrbDgramThread::RelayArenaPacket(void *packet, int packet_size,
	string sAddr, int iPort)
{
	m_cParent->m_iBusyCounter += packet_size;
	RelayPacket(packet, packet_size, sAddr, iPort);
}

void COrbDgramThread::SendSpeex(void *data, int size, string sAddr, int iPort)
{
    if(!m_cListeningSocket) return;
   	m_cParent->m_iUpEngineCount += size;
    IPAddress cDest;
    cDest = sAddr.c_str();
    m_cListeningSocket->SendDatagram(data, size, cDest, iPort);
}

void COrbDgramThread::HandleSpeex(void *data, int size, string host, int port)
{
	if (m_cParent->m_iSpeexReq == 0) return;
	CMessengerItem *theGuy = m_cParent->FindUser(host, port);
	if(!theGuy) {
		return;
	} else
		m_cParent->HandleSpeexData(data, size, true, theGuy->m_sContactName);
}
