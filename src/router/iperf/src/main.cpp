/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002,2003                              
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 * main.cpp
 * by Mark Gates <mgates@nlanr.net>
 * &  Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * -------------------------------------------------------------------
 * main does initialization and creates the various objects that will
 * actually run the iperf program, then waits in the Joinall().
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <string.h>
 *
 *   <signal.h>
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"

#include "Client.hpp"
#include "Settings.hpp"
#include "Listener.hpp"
#include "Speaker.hpp"
#include "Locale.hpp"
#include "Condition.hpp"
#include "List.h"
#include "util.h"

#ifdef WIN32
    #include "service.h"
#endif 

/* -------------------------------------------------------------------
 * prototypes
 * ------------------------------------------------------------------- */

void waitUntilQuit( void );

void sig_quit( int inSigno );

void cleanup( void );

/* -------------------------------------------------------------------
 * global variables
 * ------------------------------------------------------------------- */
#define GLOBAL()

Condition gQuit_cond;

/* -------------------------------------------------------------------
 * sets up signal handlers
 * parses settings from environment and command line
 * starts up server or client thread
 * waits for all threads to complete
 * ------------------------------------------------------------------- */

int main( int argc, char **argv ) {

#ifdef WIN32
    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main},
        { NULL, NULL}
    };
#endif

    Listener *theListener = NULL;
    Speaker  *theSpeaker  = NULL;

    // signal handlers quietly exit on ^C and kill
    // these are usually remapped later by the client or server
    my_signal( SIGTERM, sig_exit );
    my_signal( SIGINT,  sig_exit );

#ifndef WIN32
    signal(SIGPIPE,SIG_IGN);
#else
    WSADATA wsaData;
    int rc = WSAStartup( 0x202, &wsaData );
    FAIL_errno( rc == SOCKET_ERROR, "WSAStartup" );

    SetConsoleCtrlHandler( sig_dispatcher, true );
#endif

    // perform any cleanup when quitting Iperf
    atexit( cleanup );

    ext_Settings* ext_gSettings = new ext_Settings;
    Settings* gSettings = NULL;

    // read settings from environment variables and command-line interface
    gSettings = new Settings( ext_gSettings );
    gSettings->ParseEnvironment();
    gSettings->ParseCommandLine( argc, argv );

    // start up client or server (listener)
    if ( gSettings->GetServerMode() == kMode_Server ) {
        // start up a listener
        theListener = new Listener( ext_gSettings );
        theListener->DeleteSelfAfterRun();

        // Start the server as a daemon
        if ( gSettings->GetDaemonMode() == true ) {
#ifdef WIN32
            CmdInstallService(argc, argv);
            DELETE_PTR( theListener );
            DELETE_PTR( gSettings );

            return 0;
#else
            theListener->runAsDaemon(argv[0],LOG_DAEMON);
#endif
        }
#ifdef WIN32
          else {
            if ( gSettings->GetRemoveService() == true ) {
                // remove the service and continue to run as a normal process
                if ( CmdRemoveService() ) {
                    printf("IPerf Service is removed.\n");

                    DELETE_PTR( theListener );
                    DELETE_PTR( gSettings );

                    return 0;
                }
            } else {
                // try to start the service
                if ( CmdStartService(argc, argv) ) {
                    printf("IPerf Service already exists.\n");

                    DELETE_PTR( theListener );
                    DELETE_PTR( gSettings );

                    return 0;
                }
            }
        }
#endif

        theListener->Start();

        if ( ext_gSettings->mThreads == 0 ) {
            theListener->SetDaemon();

            // the listener keeps going; we terminate on user input only
            waitUntilQuit();
#ifdef HAVE_THREAD
            if ( Thread::NumUserThreads() > 0 ) {
                printf( wait_server_threads );
                fflush( 0 );
            }
#endif
        }
    } else if ( gSettings->GetServerMode() == kMode_Client ) {
#ifdef HAVE_THREAD
        theSpeaker = new Speaker(ext_gSettings);
        theSpeaker->OwnSettings();
        theSpeaker->DeleteSelfAfterRun();
        theSpeaker->Start();
#else
        // If we need to start up a listener do it now
        ext_Settings *temp = NULL;
        Settings::GenerateListenerSettings( ext_gSettings, &temp );
        if ( temp != NULL ) {
            theListener = new Listener( temp );
            theListener->DeleteSelfAfterRun();
        }
        Client* theClient = new Client( ext_gSettings );
        theClient->InitiateServer();
        theClient->Start();
        if ( theListener != NULL ) {
            theListener->Start();
        }
#endif
    } else {
        // neither server nor client mode was specified
        // print usage and exit

#ifdef WIN32

        // starting the service by SCM, there is no arguments will be passed in.
        // the arguments will pass into Service_Main entry.

        if ( !StartServiceCtrlDispatcher(dispatchTable) )
            printf(usage_short, argv[0], argv[0]);
#else    
        printf( usage_short, argv[0], argv[0] );
#endif
    }

    // wait for other (client, server) threads to complete
    Thread::Joinall();
    DELETE_PTR( gSettings );  // modified by qfeng
    // all done!
    return 0;
} // end main

/* -------------------------------------------------------------------
 * Blocks the thread until a quit thread signal is sent
 * ------------------------------------------------------------------- */

void waitUntilQuit( void ) {
#ifdef HAVE_THREAD

    // signal handlers send quit signal on ^C and kill
    gQuit_cond.Lock();
    my_signal( SIGTERM, sig_quit );
    my_signal( SIGINT,  sig_quit );

#ifdef HAVE_USLEEP

    // this sleep is a hack to get around an apparent bug? in IRIX
    // where pthread_cancel doesn't work unless the thread
    // starts up before the gQuit_cand.Wait() call below.
    // A better solution is to just use sigwait here, but
    // then I have to emulate that for Windows...
    usleep( 10 );
#endif

    // wait for quit signal
    gQuit_cond.Wait();
    gQuit_cond.Unlock();
#endif
} // end waitUntilQuit

/* -------------------------------------------------------------------
 * Sends a quit thread signal to let the main thread quit nicely.
 * ------------------------------------------------------------------- */

void sig_quit( int inSigno ) {
#ifdef HAVE_THREAD

    // if we get a second signal after 1/10 second, exit
    // some implementations send the signal to all threads, so the 1/10 sec
    // allows us to ignore multiple receipts of the same signal
    static Timestamp* first = NULL;
    if ( first != NULL ) {
        Timestamp now;
        if ( now.subSec( *first ) > 0.1 ) {
            sig_exit( inSigno );
        }
    } else {
        first = new Timestamp();
    }

    // with threads, send a quit signal
    gQuit_cond.Signal();

#else

    // without threads, just exit quietly, same as sig_exit()
    sig_exit( inSigno );

#endif
} // end sig_quit

/* -------------------------------------------------------------------
 * Any necesary cleanup before Iperf quits. Called at program exit,
 * either by exit() or terminating main().
 * ------------------------------------------------------------------- */

void cleanup( void ) {
#ifdef WIN32
    WSACleanup();
#endif
    extern Iperf_ListEntry *clients;
    Iperf_destroy ( &clients );

} // end cleanup

#ifdef WIN32
/*--------------------------------------------------------------------
 * ServiceStart
 *
 * each time starting the service, this is the entry point of the service.
 * Start the service, certainly it is on server-mode
 * 
 *
 *-------------------------------------------------------------------- */
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv) {
    Listener *theListener = NULL;

    // report the status to the service control manager.
    //
    if ( !ReportStatusToSCMgr(
                             SERVICE_START_PENDING, // service state
                             NO_ERROR,              // exit code
                             3000) )                 // wait hint
        goto clean;

    ext_Settings* ext_gSettings = new ext_Settings;
    Settings *gSettings;

    // read settings from passing by StartService
    gSettings = new Settings( ext_gSettings );
    gSettings->ParseEnvironment();
    gSettings->ParseCommandLine( dwArgc, lpszArgv );

    // report the status to the service control manager.
    //
    if ( !ReportStatusToSCMgr(
                             SERVICE_START_PENDING, // service state
                             NO_ERROR,              // exit code
                             3000) )                 // wait hint
        goto clean;

    // if needed, redirect the output into a specified file
    if ( gSettings->GetFileOutput() ) {
        redirect(gSettings->GetOutputFileName());
    }

    // report the status to the service control manager.
    //
    if ( !ReportStatusToSCMgr(
                             SERVICE_START_PENDING, // service state
                             NO_ERROR,              // exit code
                             3000) )                 // wait hint
        goto clean;

    theListener = new Listener( ext_gSettings );

    theListener->Start();
    theListener->SetDaemon();

    // report the status to the service control manager.
    //
    if ( !ReportStatusToSCMgr(
                             SERVICE_RUNNING,       // service state
                             NO_ERROR,              // exit code
                             0) )                    // wait hint
        goto clean;

    // the listener keeps going; we terminate on user input only
    waitUntilQuit();
#ifdef HAVE_THREAD
    if ( Thread::NumUserThreads() > 0 ) {
        printf( wait_server_threads );
        fflush( 0 );
    }
#endif


    clean:
    // wait for other (client, server) threads to complete
    Thread::Joinall();
    DELETE_PTR( theListener );
    DELETE_PTR( gSettings );  // modified by qfeng
    DELETE_PTR( ext_gSettings );  // modified by qfeng
}


//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop() {
#ifdef HAVE_THREAD
    gQuit_cond.Signal();
#else
    sig_exit(1);
#endif
}

#endif





