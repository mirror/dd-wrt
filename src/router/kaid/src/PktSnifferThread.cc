#include <string>
#include <iostream>

#include <unistd.h>
#ifdef PLATFORM_freebsd
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#endif
#ifdef PLATFORM_macosx_jaguar
#include <sys/errno.h>
#endif

#include "pcap.h"
#include "pcap-bpf.h"
#include "PktSnifferThread.h"
#include "Kaid.h"
#include "KaiEngine.h"
#include "ConfigFile.h"
#include "StrUtils.h"

using namespace std;
// home Xbox: 00:50:f2:d4:f2:03

#define ETHLENGTH 		14
#define IP_MIN_LENGTH 	20
#define CATCHSIZE		65536

#ifdef PLATFORM_macosx_jaguar

int CPktSnifferThread::openRawSocket(struct sockaddr *newSocket)
{
	int newFD;
	
	newFD = socket(AF_NDRV, SOCK_RAW, 0);
	if (newFD != -1)
	{
		newSocket->sa_len = sizeof(struct sockaddr);
		newSocket->sa_family = AF_NDRV;
		strcpy(newSocket->sa_data, m_cConf->SniffDevice.c_str());
		if( bind(newFD, newSocket, sizeof(*newSocket)) == -1)
			return 0;
		else
			return (newFD);
	}
	else
		return 0;
}

int CPktSnifferThread::sendRawPacket(const void *mPacket, int datasize)
{
    int		nleft = 0;
    int		nwritten = 0;
    const void	*ptr;
	
	ptr = mPacket;
	nleft = datasize;

	while (nleft > 0)
	{
		nwritten = sendto(injectFD, ptr, nleft, 0, &mySocket, sizeof(mySocket));
		if (nwritten <= 0)
		{
			if (errno == EINTR)
				nwritten = 0;	// try calling write once more
			else
			{
				fprintf (stderr, "Error writing data to socket, errno: %d\n", errno);
				return nwritten;
			}
		}
		nleft -= nwritten;
		(u_char *)ptr += nwritten;
    }
    return nwritten;
}

#endif // PLATFORM_macosx_jaguar

void CPktSnifferThread::FindFirstAvailDev()
{
	char *dev, errbuf[PCAP_ERRBUF_SIZE];
	dev = pcap_lookupdev(errbuf);
    string d = dev;
}

CPktSnifferThread::~CPktSnifferThread()
{
	m_cParent->verbositylog(2,"THREAD","Packet sniffing thread stopped...");
	
	m_bTerminate = true;
	if(m_ptHandle)
	{
		pcap_breakloop(m_ptHandle);
		pcap_close(m_ptHandle);
	}
	WaitThread();
}

CPktSnifferThread::CPktSnifferThread(CKaiEngine* cParent, CConfigFile* cConf)
{
	char ErrBuf[PCAP_ERRBUF_SIZE];
	uid_t user;
		
	m_cParent = cParent;
	m_cConf = cConf;
	
	m_cParent->verbositylog(2,"THREAD","Packet sniffing thread started...");
	
	m_sDevice = m_cConf->SniffDevice;
	m_sFilter = "";

	try
	{
		user = geteuid();
		if(user != 0)
			throw errRootRequired;

		m_ptHandle = pcap_open_live(m_sDevice.c_str(), CATCHSIZE, 1, 1, ErrBuf);
		
		if(!m_ptHandle)
			throw errInvalidDevice;
		
		#ifdef PLATFORM_freebsd
			int one = 1;
			if(ioctl(pcap_fileno(m_ptHandle), BIOCSHDRCMPLT, &one) == -1) {
			pcap_close(m_ptHandle);
			throw errFatalError;
			}
		#endif

		#ifdef PLATFORM_macosx_jaguar
			injectFD = openRawSocket(&mySocket);
		#endif
		
		pcap_lookupnet(m_sDevice.c_str(), &net, &mask, ErrBuf);
	}
	catch (PCapExceptions &excep)
	{
		switch(excep)
		{
			case errRootRequired: {
				break;
			}
			case errInvalidFilter: {
				break;
			}
			case errInvalidDevice: {
				FindFirstAvailDev();
				break;
			}
			case errFatalError: {
				break;
			}
		}
	}
}

void *CPktSnifferThread::Execute()
{
	struct pcap_pkthdr pkthdr;	
	u_char* packet;
	struct timeval timeout;
	try
	{
		
		while(!m_bTerminate) {
			#ifndef PLATFORM_macosx_jaguar
			fd_set rfds;
			int ret;
			
			// Changed to 1-second timeout to allow clean shutdown - MF
			do
			{
				timeout.tv_sec=1;
				timeout.tv_usec=0;
				FD_ZERO(&rfds);
				FD_SET(pcap_fileno(m_ptHandle), &rfds);
				ret = select(pcap_fileno(m_ptHandle) + 1, &rfds, NULL, NULL, &timeout);
			} while((ret==0) && (!m_bTerminate));
			if(m_bTerminate)
				return NULL;
			if( ret == -1)
			{
				
			}
			else
			{
				if( ret )
				{
			#endif
					/* if there is no 'next' packet in time, continue loop */
					if ((packet = (u_char *)pcap_next(m_ptHandle, &pkthdr))==NULL) continue;
					/* if the packet is to small, continue loop */
					if (pkthdr.len <= (ETHLENGTH + IP_MIN_LENGTH)) continue; 
					/* Now process this packet and inject with poison! */
					m_cParent->HandleEthernetFrame(packet, pkthdr.len);
			#ifndef PLATFORM_macosx_jaguar
				}
			}
			#endif
		}
	}
	catch (...)
	{
		
		return NULL;
	}
	return NULL;
}

void CPktSnifferThread::Start()
{
	
	m_bTerminate = false;
	LaunchThread();
}

void CPktSnifferThread::LockConsoles(string MacAddressList)
{
	// Pause pcap
	if (ThreadRunning()) pcap_breakloop(m_ptHandle);
	
	int check;
	struct bpf_program filter;
	char pcapfilt[1024];
	vector<string> consoles;
	Tokenize(MacAddressList, consoles, ";");

	// Build pcap filter
	for(unsigned int i = 0; i < consoles.size(); i++)
	{
		if(m_sFilter != "") m_sFilter += " or ";
		m_sFilter += "ether src " + case_upper(consoles[i]);
	}

	// Apply pcap filter
	if (m_sFilter != "") {
		sprintf(pcapfilt, "%s", m_sFilter.c_str());
		m_cParent->verbositylog(2,"PCAP","Applying libpcap filter ("+m_sFilter+")...");
		check = pcap_compile(m_ptHandle, &filter, pcapfilt, 0, net);
		if (check == -1)
			return;
			//throw errInvalidFilter;
	
		pcap_freecode(&filter);
	}
        
}

int CPktSnifferThread::Inject(const void *data, int data_size)
{
#ifdef PLATFORM_macosx_jaguar
	return sendRawPacket(data, data_size);
#else
	return write(pcap_fileno(m_ptHandle), data, data_size);
#endif
}
