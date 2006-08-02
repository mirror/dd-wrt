#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <socketcc.h>

#include "Kaid.h"
#include "KaiDaemon.h"
#include "KaiEngine.h"
#include "ConfigFile.h"
#include "PersistFile.h"

bool        Daemon::bKillFlag = false;
bool        Daemon::bRestartFlag = false;
Condition   Daemon::condSignal;

void Daemon::SignalHandler(int iSig)
{
    condSignal.LockMutEx();
    switch (iSig)
    {
        case SIGINT:
        case SIGTERM:   bKillFlag = true; break;
        case SIGHUP:    bRestartFlag = true; break;
    }
    condSignal.UnlockMutEx();
    condSignal.Signal();
}

void Daemon::Start()
{
    if(file_exists(m_sConfig.c_str())) {
    } else {
		debuglog("KAID", "Config file '" + m_sConfig + "' not found...");
		exit(1);		
	}
	Conf->Load();
	Persist=new CPersistFile(Conf->CacheFile);
	try
	{
		debuglog("KAID", "Kai Engine for "PLATFORM" is starting...");;
		Engine = new CKaiEngine(Conf);
		debuglog("KAID", "Kai Engine for "PLATFORM" has started...");;
    }
	catch (SocketException &excep)
	{
		string err = "ERROR: Socket Exception (";
			err += ((const char *) excep);
			err += "): KaiDaemon";
       	debuglog("KAID", err);
       	throw errAbnormalTermination;
	}
   	catch (ThreadException &excep)
	{
		string err = "Thread Exception (";
			err += ((const char *) excep);
			err += "): KaiDaemon";
       	debuglog("KAID", err);
       	throw errAbnormalTermination;
   	}
   	Engine->Persist=Persist;
}

void Daemon::Stop()
{
	debuglog("KAID", "Kai Engine for "PLATFORM" is stopping...");
	delete Engine;
	delete Persist;
	debuglog("KAID", "Kai Engine for "PLATFORM" has stopped...");
}

Daemon::Daemon(string pcConfigFile)
{
    m_sConfig = pcConfigFile;
    Conf = new CConfigFile(m_sConfig);

    signal(SIGINT, &SignalHandler);
    signal(SIGTERM, &SignalHandler);
    signal(SIGHUP, &SignalHandler);
}

Daemon::~Daemon()
{
    delete Conf;
}

void Daemon::RunDaemon()
{
    Start();

    condSignal.LockMutEx();
    while (!bKillFlag)
    {
        condSignal.Wait();

        if (bRestartFlag)
        {
            Stop();
            Start();
            bRestartFlag = false;
        }
    }
    condSignal.UnlockMutEx();
    Stop();
}
