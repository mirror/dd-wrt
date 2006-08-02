#include <iostream>
#include <fstream>
#ifdef OPENWRT
#include <utility>
#else
#ifdef PLATFORM_freebsd
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <socketcc.h>
#include "ConfigFile.h"
#include "Kaid.h"
#include "StrUtils.h"
#include "ifaddrlist.h"

#ifndef OPENWRT
void doEnumeration()
{
	struct ifaddrlist *al;
	char err[132];
	int n, i;
	in_addr addr;

	n = ifaddrlist(&al, err);
	if(n < 0)
	{
    } else {
		for (i = n; i > 0; --i, ++al)
		{
			addr.s_addr = al->addr;
			printf("Device: %s -> %s\n", al->device, inet_ntoa(addr));
		}
	}
}
#endif

using namespace std;

CConfigFile::CConfigFile (string configFile)
{
	// Lets go
	m_sFilename = configFile;
	
	// Set defaults
	OrbPort = 34525;
	OrbDeepPort = 34523;
	
	#ifdef PLATFORM_freebsd
		User = "";
	#endif

	UIBind = "";
	UIIP = "";
	UIDev = "";
	UIPort = 34522;
	
	EngineBind = "";
	EngineIP = "";
	EngineDev = "";
	EnginePort = 0;
	EnginePAT = 0;
	DeepBind="";
	DeepIP="";
	DeepDev="";
	DeepPort=0;
	UseDeep=false;
	Verbosity=0;
	LocalDevices=0;
	CacheFile="/tmp/kaiEnginePersist.txt";
	ConfigCache="/tmp/kaiSystemConfig.txt";
	
	#ifdef PLATFORM_macosx_jaguar
	SniffDevice = "en0";
	#elif defined(PLATFORM_openwrt)
		SniffDevice = "br0";
	#else
	SniffDevice = "eth0";
	#endif	

	XBoxHomebrew = false;
	ConfigURL = "";
	OrbList = "";
	Username = "";
	Password = "";
	AutoLogin = 0;
}

void
CConfigFile::Load ()
{
	ifstream file (m_sFilename.c_str ());
	
	string line;
	string name;
	string value;
	int posEqual;
	
	while (std::getline (file, line))
	{
		if (!line.length ())
			continue;	
		if (line[0] == '#')
			continue;
		if (line[0] == ';')
			continue;

		do
		{
			posEqual = line.find('\r');
			if(posEqual!=string::npos)
				line.erase(posEqual,1);
		} while (posEqual!=string::npos);
		do
		{
			posEqual = line.find('\n');
			if(posEqual!=string::npos)
				line.erase(posEqual,1);
		} while (posEqual!=string::npos);
			
		posEqual = line.find ('=');
		name = trim (line.substr (0, posEqual));
		value = trim (line.substr (posEqual + 1));
		m_vsFields.push_back (name);
		m_vsValues.push_back (value);
	}
	
	for (unsigned int i = 0; i < m_vsFields.size (); i++)
	{
		string fld = case_lower (m_vsFields[i]);
		string val = case_lower (m_vsValues[i]);
		if (fld == "uibind")
		{
			UIBind = m_vsValues[i];
		}
		else if (fld == "orbport")
		{
			OrbPort = atoi (m_vsValues[i].c_str ());
		}
		else if (fld == "orbdeepport")
		{
			OrbDeepPort = atoi(m_vsValues[i].c_str());
		}
		else if (fld == "enginebind")
		{
			EngineBind = m_vsValues[i];
		}
		else if (fld == "enginedeepbind")
		{
			DeepBind = m_vsValues[i];	
		}
		else if (fld == "sniffdevice")
		{
			SniffDevice = m_vsValues[i];
		}
		else if (fld == "configurl")
		{
			ConfigURL = m_vsValues[i];
		}
		else if (fld == "username")
		{
			Username = m_vsValues[i];
		}
		else if (fld == "password")
		{
			Password = m_vsValues[i];
		}
		else if (fld == "autologin")
		{
			AutoLogin = atoi (m_vsValues[i].c_str ());
		} 
		else if (fld == "localdevices")
		{
			LocalDevices = atoi (m_vsValues[i].c_str ());
		} 
		else if (fld == "verbosity")
		{
			Verbosity = atoi (m_vsValues[i].c_str ());
		} 
		else if (fld == "enginepat")
		{
			EnginePAT = atoi (m_vsValues[i].c_str ());
		} 
		else if (fld == "xboxhomebrew")
		{
			XBoxHomebrew = ((val == "1") || (val == "on") || (val == "true") ||
			(val == "yes") || (val == "yay") || (val == "foshizzle"));
		}
		else if (fld == "user")
		{
			#ifdef PLATFORM_freebsd
				User = m_vsValues[i];
			#endif
 		}
 		else if (fld == "cachefile")
 		{
 			CacheFile = m_vsValues[i];
 		}
 		else if (fld == "configcache")
 		{
 			ConfigCache = m_vsValues[i];
 		}
		else
		{
		}
	}
	ParseBindings ();
	GetOrbList ();
}

void
CConfigFile::ParseBindings ()
{
	if (EngineBind.find (":") < 0)
    {
      	if (EngineBind.find (".") < 0)
			EngineDev = EngineBind;
      	else
			EngineIP = EngineBind;
    }
  	else
    {
    	string sAux = EngineBind.substr (0, EngineBind.find (":"));
    	if( sAux.find(".") < 0)
    		EngineDev = sAux;
    	else
    		EngineIP = sAux;
      	EnginePort = atoi (EngineBind.substr (EngineBind.find (":") + 1).c_str ());
    }
	// Parse deep configuration - MF
	if (DeepBind.find (":") < 0)
	{
	 	if (DeepBind.find (".") < 0)
			DeepDev = DeepBind;
	 	else
			DeepIP = DeepBind;		
	}
	else
	{
		string sAux = DeepBind.substr (0, DeepBind.find (":"));
		if( sAux.find(".") < 0)
			DeepDev = sAux;
		else
			DeepIP = sAux;
	 	DeepPort = atoi (DeepBind.substr (DeepBind.find (":") + 1).c_str ());
	}

	if (UIBind.find (":") < 0)
    {
      	if (UIBind.find (".") < 0)
			UIDev = UIBind;
      	else
			UIIP = UIBind;
    }
  	else
    {
    	string sAux = UIBind.substr (0, UIBind.find (":"));
    	if( sAux.find(".") < 0)
    		UIDev = sAux;
    	else
    		UIIP = sAux;
      	UIPort = atoi (UIBind.substr (UIBind.find (":") + 1).c_str ());
    }
}

void
CConfigFile::GetOrbList ()
{
  string sOrbList, sAux, sAux2;
  if (ConfigURL.empty ())
    return;
  try
  {
    debuglog("KAID", "Kai Engine for "PLATFORM" is initialising...");
    string sURL = ConfigURL;
    string sHostSep = "/";
    int iProtoSeps = sURL.find ("://");
    if (iProtoSeps > 0)
      sURL = sURL.substr (iProtoSeps + 3, sURL.size () - iProtoSeps + 3);
    int iHostSepPos = sURL.find (sHostSep);
    string sHost (sURL.substr (0, iHostSepPos));
    string sHTTPReq (sURL.substr (iHostSepPos));
    IPAddress ipOrbListServer;
    ipOrbListServer = sHost.c_str ();

    TCPClientSocket pcHTTPClient (ipOrbListServer, 80);
    string sRequest ("GET " + sHTTPReq + " HTTP/1.1\r\nHost: " +
		     sHost + "\r\nAccept: text/xml\r\n\r\n");
    pcHTTPClient.SendASCII (sRequest.c_str ());

    while (1)
    {
		char pcRecvBuffer[8192];
		int iBytesTransferred;
		iBytesTransferred = pcHTTPClient.RecvData (pcRecvBuffer,
						   sizeof (pcRecvBuffer) - 1);
		if (iBytesTransferred < 1)
	  		throw "Error getting Orb list!";
		pcRecvBuffer[iBytesTransferred] = 0;
		sAux += pcRecvBuffer;
	}
  }
  catch (SocketException & excep)
  {
	string sTemp;
	int iFrom,iTo;
	iFrom=sAux.find("HTTP");
	if(iFrom>=0)
		iTo=sAux.find("\n",iFrom);
	if(iFrom>=0 && iTo>=0)
		sTemp=sAux.substr(iFrom,iTo-iFrom);
	
	if ((sTemp.find("200") == (uint)-1) || (excep != SocketException::errNotConnected))
	{
		string err = (const char *) excep;
		debuglog("KAID","Failed to download orbital list...");
		ifstream fReader(ConfigCache.c_str());
		if(!fReader.is_open())
		{
			debuglog("KAID","Unable to open system configuration cache file (" + ConfigCache + ")");
			throw "No orbitals found!";
		}
		debuglog("KAID","Using cached system configuration");
		while(getline(fReader,sTemp))
			sAux+=sTemp+"\n";
		fReader.close();
	}
		OrbList = ParseServers(sAux,"KaiOrbitalServer",true);
		DeepResolutionServerList = ParseServers(sAux,"KaiDeepResolutionServer",false);
		ParseConsoles(sAux);
	
	ofstream fWriter(ConfigCache.c_str());
	if(!fWriter.is_open())
	{
		debuglog("WARNING","Unable to open system configuration cache file for output - check permissions (" + ConfigCache + ")");
	}
    else
    {
		int iFromChar,iToChar;
		
		iFromChar=sAux.find("KAI_XSERVER_CONFIG;");
		if(iFromChar>=0)
		{
			iToChar=sAux.rfind(";");
			if(iToChar>=0)
				fWriter << sAux.substr(iFromChar,iToChar-iFromChar+1) + "\n\n";
		}
		fWriter.close();
	}
  }
}

string CConfigFile::ParseServers(string sAux, string Token, bool ThrowError)
{
	int iFromChar, iToChar;
	unsigned int i;
	string sList,sAux2;
	vector < string > vsServers;
	
	iFromChar=sAux.find(Token+"=");
	if(iFromChar<0)
	{
		if(ThrowError)
		{
			debuglog("KAID","No " + Token + "s are available...");
			throw "No " + Token + "s found!";
		}
		else
			return "";
	}
	iToChar = sAux.rfind(Token+"=");
	sAux2=sAux.substr(iToChar);
	iToChar+=sAux2.find(";")+1;
	sAux = sAux.substr (iFromChar, (iToChar - iFromChar));
	Tokenize (sAux, vsServers, ";");
	for (i = 0; i < vsServers.size (); i++)
  		sList.append (vsServers[i].substr (vsServers[i].find ("=") + 1) + ";");
	return(sList);
}

void CConfigFile::ParseConsoles(string sAux)
{
	int iFromChar, iToChar;
	unsigned int i;
	string sAux2;
	vector <string> vsConsoles;
	struct ConsoleID *thisConsole;
	
	iFromChar=sAux.find("KaiConsole=");
	if(iFromChar<0)
	{
		debuglog("KAID","No console specifications found in client configuration...");
		throw "No console specifications found!";
	}
	
	do
	{
		iToChar=sAux.find("\n",iFromChar+11);
		sAux2=sAux.substr(iFromChar+11,iToChar-iFromChar-12);
		Tokenize(sAux.substr(iFromChar+11,iToChar-iFromChar-12),vsConsoles,";");

		thisConsole=new struct ConsoleID;
		thisConsole->Manufacturer=vsConsoles[0];
		thisConsole->ConsoleName=vsConsoles[1];

		for(i=2;i<vsConsoles.size();i++)
			m_mConsoles.insert(ConsoleIDPair(vsConsoles[i],thisConsole));
		iFromChar=sAux.find("KaiConsole=",iToChar);
	} while(iFromChar>=0);
	thisConsole=new struct ConsoleID;
	thisConsole->Manufacturer="Broadcast";
	thisConsole->ConsoleName="Broadcast";
	m_mConsoles.insert(ConsoleIDPair("FFFFFF",thisConsole));
}

string CConfigFile::GetConsoleID(string sMACPrefix)
{
	map <string,struct ConsoleID *> :: iterator theConsole;
		
	theConsole=m_mConsoles.find(sMACPrefix);
	
	if(theConsole==m_mConsoles.end())
		return "";
	else
		return theConsole->second->ConsoleName;
}
