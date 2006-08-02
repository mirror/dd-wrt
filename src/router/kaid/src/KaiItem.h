#ifndef KAIITEMS_H
#define KAIITEMS_H

#include <string>
#include <vector>

using namespace std;

class CArenaItem
{
    public:
		int 			m_iLastStatus, m_iCheckCounter, m_iRecast;
		int 			m_iPort, m_iDeepPort, m_iLastPing, m_iLastPlayers;
		unsigned char	m_ucGCNCheck[2];
		
		char 			m_gcnLastCheck[2]; // GCN? Gamecube stuff?

		string 			m_sDeepIP, m_sIPAddress, m_sContactName;
		bool			m_bUseDeep;
		bool			m_bDeepUnknown;
		vector<string>  m_vsDevices;

		struct timeval	m_tvLastCompletePing, m_tvLastQueryResponse;
		struct timeval	m_tvLastConnectionQuery, m_tvLastBroadcastData;
		struct timeval	m_tvLastPacketTime;
						CArenaItem();
        unsigned int    Ping();
		int				FindDevice(string device);
		bool			AddDevice(string device);
		void			Reset();
		unsigned int	LastPingAge();
};

class CMessengerItem : public CArenaItem
{
    public:
		bool    m_bSpeexConnected;
		int		m_iSpeexMode, m_iState;
				CMessengerItem();
};

#endif
