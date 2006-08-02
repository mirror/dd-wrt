#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <vector>
#include <map>

using namespace std;

typedef pair <string, struct ConsoleID *> ConsoleIDPair;
struct ConsoleID {
	string	Manufacturer;
	string	ConsoleName;
};

class CConfigFile {
private:
	vector<string>  m_vsFields;
	vector<string>  m_vsValues;
	map<string,struct ConsoleID *> m_mConsoles;
    string          m_sFilename;
    void            GetOrbList();
    void            ParseBindings();
    string		ParseServers(string,string,bool);
    void			ParseConsoles(string);
public:
                    CConfigFile(string configFile);
    string		GetConsoleID(string);
    void            Load();
    
    int             OrbPort;
	int			OrbDeepPort;
	int				Verbosity;
	
	#ifdef PLATFORM_freebsd
	    string	    User;
	#endif
    
    string			UIBind;
    string			UIIP;
    string			UIDev;
    int             UIPort;
    
    string          EngineBind;
    string          EngineIP;
    string          EngineDev;
    int             EnginePort;
	string		DeepBind;
	string		DeepIP;
	string		DeepDev;
	int			DeepPort;
	bool			UseDeep;
	int				EnginePAT;
	unsigned int	LocalDevices;
	
    string          SniffDevice;
	bool			XBoxHomebrew;
    string          ConfigURL;
    string          OrbList;
    string		DeepResolutionServerList;
	string          Username;
    string          Password;
    string		CacheFile;
    string		ConfigCache;
    int		    	AutoLogin;
};

#endif

