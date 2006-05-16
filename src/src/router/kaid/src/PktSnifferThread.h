#ifndef PCAPSNIFF_H
#define PCAPSNIFF_H

#include <string>
#include <pthreadcc.h>
#include <pcap.h>
#ifdef PLATFORM_macosx_jaguar
#include <sys/socket.h>
#endif

using namespace std;

class CKaiEngine;
class CConfigFile;

class CPktSnifferThread : public ThreadBase
{
    private:
		Condition           condWakeThread;
        string				m_sFilter, m_sDevice;
		bpf_u_int32			net, mask;
		bool                m_bTerminate;
		pcap_t*				m_ptHandle;
		#ifdef PLATFORM_macosx_jaguar
		struct sockaddr		mySocket;
		int					injectFD;
		int					openRawSocket(struct sockaddr *newSocket);
		int					sendRawPacket(const void *mPacket, int datasize);
		#endif			
        CKaiEngine*         m_cParent;
        CConfigFile*        m_cConf;
		void				FindFirstAvailDev();
	protected:
		static MutualExclusion      mutexProtect, mutexAccept;
		enum                PCapExceptions { errRootRequired, errInvalidFilter,
                                errInvalidDevice, errFatalError };
		virtual void *      Execute();
    public:
							CPktSnifferThread(CKaiEngine *cParent, CConfigFile* cConf);
        virtual             ~CPktSnifferThread();

        void                Start();
		void				Stop();
		void				LockConsoles(string MacAddressList);
		int					Inject(const void *data, int data_size);
};

#endif
