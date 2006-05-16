#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <vector>

using namespace std;

class CConfigFile {
private:
	vector<string>  m_vsFields;
	vector<string>  m_vsValues;
    string          m_sFilename;
    void            GetOrbList();
    void            ParseBindings();
public:
                    CConfigFile(string configFile);
    void            Load();
    
    int             OrbPort;
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
	int				EnginePAT;
	unsigned int	LocalDevices;
	
    string          SniffDevice;
	bool			XBoxHomebrew;
    string          ConfigURL;
    string          OrbList;
    string          Username;
    string          Password;
    int		    	AutoLogin;
};

#endif

