#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <socketcc.h>
#include <pthreadcc.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

#include "Kaid.h"
#include "KaiEngine.h"
#include "OrbStreamThread.h"
#include "KaiItem.h"
#include "ConfigFile.h"
#include "PersistFile.h"
#include "StrUtils.h"

using namespace std;

COrbStreamThread::COrbStreamThread (CKaiEngine * cParent, CConfigFile * cConf)
{
	m_bTerminate = true;
	m_bConnected = false;
	m_bAuthenticated = false;
	m_cParent = cParent;
	m_cParent->verbositylog(2,"THREAD","Orbital stream thread started...");
	m_cConf = cConf;
    m_sBuffer = "";
    m_cOrbClient = NULL;
}

COrbStreamThread::~COrbStreamThread ()
{
	m_cParent->verbositylog(2,"THREAD","Orbital stream thread stopped...");
	m_bTerminate = true;
	m_bConnected = false;
	if (m_cOrbClient)
		delete m_cOrbClient;
	// WaitThread ();
}

void
COrbStreamThread::SendOrbAuth ()
{
	try
	{
		string sHash=m_cParent->Persist->GetValue(m_cConf->Username,"HASH");
		if(sHash.size()==0) sHash=";";
		string sAuth ("KAI_AUTHENTICATION;" + m_cConf->Username +
			      ";" + m_cConf->Password + ";" + sHash);
		sAuth += (char) 1;
		m_cOrbClient->SendASCII (sAuth.c_str ());
        m_cParent->m_iUpOrbCount += sAuth.size();
	}
	catch (...)
	{
	}
}

void
COrbStreamThread::Start (IPAddress & OrbAddr)
{
	try
	{
		m_cOrbClient =
			new TCPClientSocket (OrbAddr, m_cConf->OrbPort);
		
		long one=1;
		m_cOrbClient->SetSockOpt(SOL_SOCKET,SO_KEEPALIVE,&one,sizeof(one));
		
		m_cParent->verbositylog(1,"ORBSTREAM","Orbital stream established...");
		
		m_bConnected = true;
		m_bTerminate = false;
		SendOrbAuth ();
		LaunchThread ();
	}
	catch (...)
	{
	}
}

void
COrbStreamThread::Stop ()
{
	if (m_cOrbClient)
		delete m_cOrbClient;
}

void *
COrbStreamThread::Execute ()
{
	int iBytesTransferred, retval;
	char pcBuffer[2048];
	vector < string > vsMsgs;
	SocketSet mySet;
	struct timeval timeout;

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
				mySet += (SocketBase *) m_cOrbClient;
				retval = select ((int) mySet + 1, mySet, NULL, NULL, &timeout);
				if (retval == -1)
					throw errClientError;
			} while((retval==0) && (!m_bTerminate));

			iBytesTransferred =
				m_cOrbClient->RecvData (pcBuffer,
							sizeof (pcBuffer) -
							1);
			if (m_bTerminate)
				throw errServerDown;
			if (iBytesTransferred > 0)
			{
				pcBuffer[iBytesTransferred] = 0;
				m_sRequest = pcBuffer;
                
                m_sBuffer += m_sRequest;
                while(m_sBuffer.find((char)1) != string::npos)
                {
                    string chunk = headc(m_sBuffer, 0x01);
                    m_sBuffer = m_sBuffer.substr(m_sBuffer.find((char)1) + 1);
					HandleOrbMsg (chunk);
					if (m_sAnswer != "")
						SendStream (m_sAnswer);
				}
				vsMsgs.clear ();
			}
		}
		catch (SocketException & excep)
		{
			m_cParent->verbositylog(1,"ORBSTREAM","Orbital stream lost...");
			if (excep == SocketException::errNotConnected)
			{
				m_bConnected = false;
				m_bTerminate = true;			
				raise(SIGTERM);
				return NULL;
			}		
			else
			{
			}
			if(m_cParent->m_bAttached)
					m_cParent->SendClientResponse("KAI_CLIENT_DETACH;");
			return NULL;
		}

		catch (ThreadExceptions & excep)
		{
			m_cParent->verbositylog(1,"ORBSTREAM","Orbital stream lost...");
			m_bTerminate = true;
			return NULL;
		}
	}
	return NULL;
}

void
COrbStreamThread::SendStream (string msg)
{
	m_cParent->verbositylog(3,"ORBSTREAM","Sending Orb command: "+msg);
	msg += (char) 1;
	
	m_cOrbClient->SendData (msg.c_str (), msg.size ());

    if(msg.substr(0, 9) == "KAI_CHAT;")
	{
        m_cParent->m_iUpChatCount += msg.size();
    } 
	else 
	{
        m_cParent->m_iUpOrbCount += msg.size();
    }
}

void
COrbStreamThread::SendClientResponse (string resp)
{
	if (m_cParent)
		m_cParent->SendClientResponse (resp);
}

void
COrbStreamThread::HandleOrbMsg (string sRequest)
{
	m_cParent->verbositylog(3,"ORBSTREAM","Sending Orb response: "+sRequest);
	// Too small?
	if (sRequest.size () < 2)
		return;
	// Verify 2nd char is ";" etc.
	char Opcode = sRequest.at (0);
	if (sRequest.at (1) != ';')
		return;

	vector < string > vsSegments;
	Tokenize (sRequest, vsSegments, ";");
    if(Opcode == 'A') {
        m_cParent->m_iDownChatCount += sRequest.size();
    } else {
        m_cParent->m_iDownOrbCount += sRequest.size();
    }
    
    // Switch on Opcode
	switch (Opcode)
	{
	case 'A':
		// A chat packet - simple
		SendClientResponse ("KAI_CLIENT_CHAT;" + vsSegments[1] + ";" +
				    vsSegments[2] + ";" + vsSegments[3] +
				    ";");
		break;
	case 'B':
		// Someone has joined the room
		SendClientResponse ("KAI_CLIENT_JOINS_CHAT;" + vsSegments[1] +
				    ";" + vsSegments[2] + ";");

		// Add him to our list?
		m_cParent->AddChatUser (vsSegments[1], vsSegments[2]);
		break;

	case 'C':
		// Someone has left the room
		SendClientResponse ("KAI_CLIENT_LEAVES_CHAT;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";");

		// Remove him from our list?
		m_cParent->RemoveChatUser (vsSegments[1], vsSegments[2]);
		break;
	case '~':
	{
		string sContacts="";
		
		if(vsSegments.size()<=1)
			Tokenize("~;"+m_cParent->Persist->GetValue(m_cConf->Username,"CONTACTS"),vsSegments,";");
		// empty contact list!!
		if (vsSegments.size () > 1)
		{
			// Add all the messenger contacts
			for (unsigned int i = 1; i < vsSegments.size (); i++)
			{
				CMessengerItem *pcContact;
				pcContact = new CMessengerItem ();
				pcContact->m_sContactName = vsSegments[i];
				m_cParent->m_cMessengerContacts.
					push_back (pcContact);

				SendClientResponse ("KAI_CLIENT_ADD_CONTACT;"
						    +
						    pcContact->
						    m_sContactName + ";");

				Deliver (pcContact->m_sContactName, '6',
					 m_cConf->Username,
					 m_cParent->MyIP ().
					 GetAddressString (),
					 m_cParent->MyPort (),
					 (m_cConf->UseDeep?m_cParent->MyDeepIP().GetAddressString():""),
				 	 (m_cConf->UseDeep?m_cParent->MyDeepPort():0));
				 
				 sContacts+=vsSegments[i]+";";
			}
		}
		m_cParent->Persist->SetValue(m_cConf->Username,"CONTACTS",sContacts);

		
		SendClientResponse ("KAI_CLIENT_CONNECTED_MESSENGER;");
		// Tell the client its now online (status)
		m_cParent->SendClientStatus ("XLink Kai is Online..");
	}
		break;
	case '=':
		// Simply a profile - send to client
		SendClientResponse ("KAI_CLIENT_USER_PROFILE;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";" + vsSegments[3] + ";" +
				    vsSegments[4] + ";" + vsSegments[5] +
				    ";" + vsSegments[6] + ";" +
				    vsSegments[7] + ";" + vsSegments[8] +
				    ";");
		break;

	case '#':
		// Set the case corrected username
		m_bAuthenticated = true;
		m_cParent->LoggedIn (vsSegments[1]);
		m_cParent->Persist->SetValue(m_cConf->Username,"HASH",vsSegments[2]);
		break;

	case '?':
		// Orb bad
		m_bAuthenticated = false;
		m_cParent->SendClientStatus ("Authentication Failed..");
		SendClientResponse ("KAI_CLIENT_AUTHENTICATION_FAILED;");
		break;

	case '+':
		// Subvector users update - straight to UI
		SendClientResponse ("KAI_CLIENT_SUB_VECTOR_UPDATE;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";" + vsSegments[3] + ";");
		break;

		// TODO: Join next case on single case with iif
	case ')':
		// Bad password for arena - simply pass to UI
		SendClientResponse ("KAI_CLIENT_VECTOR_DISALLOWED;" +
				    vsSegments[1] +
				    ";The supplied password was incorrect.;");
		break;

	case '(':
		// arena full - simply pass to UI
		SendClientResponse ("KAI_CLIENT_VECTOR_DISALLOWED;" +
				    vsSegments[1] + ";The Arena is full.;");
		break;

	case ']':
		// Specific response - simple 4ward
		SendClientResponse ("KAI_CLIENT_APP_SPECIFIC;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";");
		break;

	case 'D':
	{
		// If it's not our vector - bail
		string b, sAux;

		b = vsSegments[1].substr (0, vsSegments[1].rfind ("/"));
		if (b != m_cParent->m_sCurrentArena)
			return;
		// Here is a possible sub vector - tell the client
		m_cParent->AddVector (vsSegments[1], vsSegments[2]);
		
		sAux = vsSegments[1] + ";" + vsSegments[3] + ";" +
			vsSegments[4] + ";" + vsSegments[5] + ";" +
			vsSegments[6] + ";";
		
		if (vsSegments[2] == "1")
		{
			m_cParent->
				SendClientResponse
				("KAI_CLIENT_USER_SUB_VECTOR;" + sAux +  vsSegments[7] + ";");
		}
		else
		{
			m_cParent->
				SendClientResponse ("KAI_CLIENT_SUB_VECTOR;" +
						    sAux);
		}
	}
		break;
	case 'K':
		// Simply send to client
	{
		string relay = "KAI_CLIENT_SPECIFIC_COUNT;" + vsSegments[1] +
			";" + vsSegments[2] + ";" + vsSegments[3] + ";";
		SendClientResponse (relay);
	}
		break;

	case 'E':
		// Not us
		if (vsSegments[1] == m_cConf->Username)
			return;

		// Someone has joined the vector
		SendClientResponse ("KAI_CLIENT_JOINS_VECTOR;" +
				    vsSegments[2] + ";");
		// If it's not our vector - bail
		if (vsSegments[1] != m_cParent->m_sCurrentArena)
			return;
		// Add the guy
		m_cParent->AddArenaContact (vsSegments[2], vsSegments[3],
					    atoi (vsSegments[4].c_str ()),
					    vsSegments[5],
					    atoi (vsSegments[6].c_str ()));
		break;

	case 'F':
		// If it's not our vector - bail
		if (vsSegments[1] != m_cParent->m_sCurrentArena)
			return;
		// Someone has left the vector
		SendClientResponse ("KAI_CLIENT_LEAVES_VECTOR;" +
				    vsSegments[2] + ";");
		// Remove the guy
		m_cParent->RemoveArenaContact (vsSegments[2]);
		break;

	case 'G':
	{
		// Attached to vector sucessfully
		m_cParent->m_cArenaContacts.clear ();	// Empty users          
		m_cParent->m_sVectors.clear ();	// Empty subs
		gettimeofday (&m_cParent->m_tvLastVectorChange, NULL);	// Record switch
		m_cParent->m_sCurrentArena = vsSegments[1];
		m_cParent->m_iRoutingMode = atoi(vsSegments[3].c_str());
		m_cParent->m_iBusyLevel = atol(vsSegments[4].c_str());
		// Echo to client
		string CanCreate = (vsSegments[2].empty() ? "0" : vsSegments[2]);
		m_cParent->m_sLastVectorChange = "KAI_CLIENT_VECTOR;" + vsSegments[1] + ";" + CanCreate + ";";
		SendClientResponse (m_cParent->m_sLastVectorChange);
		
		if (vsSegments[1].length()>m_cConf->Username.length()+5 &&
		    vsSegments[1].substr( vsSegments[1].length()-(m_cConf->Username.length()+1) ) == "/"+m_cConf->Username)
		{
			// It's our room..
			m_cParent->m_iArenaStatus=2;
		}
		else
		{
			// It's not our room..
			m_cParent->m_iArenaStatus=1;
		}
	
		SendClientResponse ("KAI_CLIENT_ARENA_STATUS;" + Str(m_cParent->m_iArenaStatus) + ";" +
		Str(m_cParent->m_iArenaMyPlayers) + ";");
		
		break;
	}	
	case 'H':
		// Echo to client
		SendClientResponse ("KAI_CLIENT_VECTOR_DISALLOWED;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";");
		break;

	case '8':
		if (m_cParent->FindUser (vsSegments[1]))
			m_cParent->SetOfflineContact (vsSegments[1]);
		break;

	case '9':
	{
		// A user vector supposedly in this arena is dead - if so, remove it from here and from the UI
		string b (vsSegments[1].
			  substr (0, vsSegments[1].rfind ("/")));
		if (m_cParent->m_sCurrentArena == b)
		{
			// Its our bit - lets maime it up :D
			m_cParent->RemoveVector (vsSegments[1]);
			SendClientResponse ("KAI_CLIENT_REMOVE_SUB_VECTOR;" +
					    vsSegments[1] + ";");
		}
		break;
	}
	case 'l':
		// An update of the privileges on an arena - simple store
		// and forward to the client.
		SendClientResponse ("KAI_CLIENT_MODERATOR_PRIVILEGES;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";");
		m_cParent->m_sStoredPrivs="KAI_CLIENT_MODERATOR_PRIVILEGES;" +
				    vsSegments[1] + ";" + vsSegments[2] +
				    ";";
		break;

	case 'r':
		// Admin list update
		m_cParent->m_sStoredAdmins = vsSegments[1];
		SendClientResponse ("KAI_CLIENT_ADMIN_PRIVILEGES;" +
				    vsSegments[1] + ";");
		break;

	case '/':
	{
		// Successful contact add
		CMessengerItem *newGuy;
		newGuy = new CMessengerItem ();
		newGuy->m_sContactName = vsSegments[1];
		SendClientResponse ("KAI_CLIENT_ADD_CONTACT;" +
				    vsSegments[1] + ";");
		m_cParent->m_cMessengerContacts.push_back (newGuy);
		Deliver (vsSegments[1], '6', m_cConf->Username,
			 m_cParent->MyIP ().GetAddressString (),
			 m_cParent->MyPort (),
			 (m_cConf->UseDeep?m_cParent->MyDeepIP().GetAddressString():""),
			 (m_cConf->UseDeep?m_cParent->MyDeepPort():0));
		break;
	}
	case '|':
	{
		// A contact remove happened - tell the client he's gone, then remove
		Deliver (vsSegments[1], '8', m_cConf->Username,
			 m_cParent->MyIP ().GetAddressString (),
			 m_cParent->MyPort (), "", 0);
		m_cParent->DeleteContact (vsSegments[1]);
		break;
	}
	case '6':
	{
		/* Directive 6 - someone has sent and "Are you here?" request
		 * Well, we obviously are, so lets tell them that then,
		 * after making sure theyre in our list */

		// Check if they're one of our boys - bail if not
		if (!m_cParent->FindUser (vsSegments[1]))
			break;

		// OK - he's good.. send him msg7
		string output;
		char myport[5];
		sprintf (myport, "%d", m_cParent->MyPort ());
		output = "KAI_DELIVER;" + vsSegments[1] + ";7;" +
			m_cConf->Username + ";" +
			m_cParent->MyIP ().GetAddressString () + ";" +
			myport + ";";
		if(m_cConf->UseDeep)
			output=output+m_cParent->MyDeepIP().GetAddressString() + ";" + Str(m_cParent->MyDeepPort()) + ";";
		else
			output+=";;";
		SendStream (output);

		// We can also add him to our contacts - coz he's obviously online now
		int port, deepport;
		port = (vsSegments[3] ==
			"") ? 0 : atoi (vsSegments[3].c_str ());
		deepport =
			(vsSegments[5] ==
			 "") ? 0 : atoi (vsSegments[5].c_str ());
		m_cParent->SetOnlineContact (vsSegments[1], vsSegments[2],
					     port, vsSegments[4], deepport);
		break;
	}

	case '7':
	{
		/* One of the guys we asked is here - so lets check
		 * him then add him */

		int port;
		port = (vsSegments[3] ==
			"") ? 0 : atoi (vsSegments[3].c_str ());
		// deepport = (vsSegments[5] == "") ? 0 : atoi(vsSegments[5].c_str());

		m_cParent->SetOnlineContact (vsSegments[1], vsSegments[2],
					     port, "", 0);
		break;
	}
	case '&':
		// Its an avatar for our client 
		m_cParent->SendClientResponse ("KAI_CLIENT_AVATAR;" +
					       vsSegments[1] + ";" +
					       vsSegments[2] + ";");
		break;
	case 'o':
		m_cParent->m_sOurOrbName = vsSegments[1];
		break;
	}
}

void
COrbStreamThread::Deliver (string contact, char opcode, string username,
			   string myip, int myport, string deepip,
			   int deepport)
{
	string output;
	output = "KAI_DELIVER;" + contact + ";" + opcode + ";" + username +
		";" + myip + ";" + Str (myport) + ";" + deepip + ";" +
		Str (deepport) + ";";
	SendStream (output);
}
