#include <iostream>
#include <sys/time.h>
#include <string.h>
#include "KaiItem.h"
#include "Kaid.h"

using namespace std;

CArenaItem::CArenaItem()
{
	m_iRecast = m_iDeepPort = m_iCheckCounter = m_iPort = m_iLastStatus = 0;
    m_iLastPing = m_iLastPlayers = 0;
	m_sContactName = m_sDeepIP = m_sIPAddress = "";
	m_bUseDeep = m_bDeepUnknown = false;
	m_vsDevices.clear();
	m_ucGCNCheck[0]=0;
	m_ucGCNCheck[1]=0;
	bzero(&m_tvLastConnectionQuery, sizeof(m_tvLastConnectionQuery));
	bzero(&m_tvLastCompletePing, sizeof(m_tvLastCompletePing));
	bzero(&m_tvLastQueryResponse, sizeof(m_tvLastQueryResponse));
	bzero(&m_tvLastBroadcastData, sizeof(m_tvLastBroadcastData));
	bzero(&m_tvLastPacketTime, sizeof(m_tvLastPacketTime));
}

unsigned int CArenaItem::Ping()
{
	unsigned int daPing;
	m_tvLastQueryResponse = Now();
    m_tvLastCompletePing = m_tvLastQueryResponse;
    daPing = TimeDelta(m_tvLastConnectionQuery);
  // AhMan  20051112  Bug fix  daPing can never be a negative number as it's unsigned
  // if the ping is over 1000 sec, it may as well be bad
  //	return (daPing < 0 ? 0 : daPing);
  return (daPing > 1000000 ? 0 : daPing);
}

int CArenaItem::FindDevice(string device)
{	
	for (unsigned int i=0; i < m_vsDevices.size(); i++) {
		if(m_vsDevices[i] == device)
			return i;
	}
	return -1;
}

bool CArenaItem::AddDevice(string device)
{
	int iPos = FindDevice(device);
	if( iPos < 0)
	{
		m_vsDevices.push_back(device);
		return true;
	}
	else
		return false;
}

void CArenaItem::Reset()
{
	bzero(&m_tvLastConnectionQuery, sizeof(m_tvLastConnectionQuery));
	bzero(&m_tvLastQueryResponse, sizeof(m_tvLastQueryResponse));
}

unsigned int CArenaItem::LastPingAge()
{
	struct timeval now;
	int delta_usec, delta_sec;	
	gettimeofday (&now, NULL);
	delta_sec = now.tv_sec - m_tvLastCompletePing.tv_sec;
	delta_usec = now.tv_usec - m_tvLastCompletePing.tv_usec;
	return ((delta_sec * 1000) + (delta_usec / 1000));
}

CMessengerItem::CMessengerItem()
{
	m_bSpeexConnected = false;
	m_iSpeexMode = m_iState = 0;
}
