#ifndef DEAMON_H
#define DEAMON_H

#include <pthreadcc.h>

class CKaiEngine;
class CConfigFile;
class CPersistFile;

class Daemon
{
    private:
        static bool			bKillFlag;
        static bool			bRestartFlag;
        static Condition	condSignal;
        static void 		SignalHandler(int iSig);
		string				m_sConfig;
		CKaiEngine			*Engine;
        CConfigFile     	*Conf;
        CPersistFile	*Persist;
        void        		Start();
        void        		Stop();

    public:
        enum DaemonError 	{ errAbnormalTermination };
                    		Daemon(string pcConfigFile);
                    		~Daemon();
        void        		RunDaemon();
};

#endif
