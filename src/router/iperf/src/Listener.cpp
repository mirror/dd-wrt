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
 *
 * Listener.cpp
 * by Mark Gates <mgates@nlanr.net> 
 * &  Ajay Tirumala <tirumala@ncsa.uiuc.edu> 
 * ------------------------------------------------------------------- 
 * Listener sets up a socket listening on the server host. For each 
 * connected socket that accept() returns, this creates a Server 
 * socket and spawns a thread for it. 
 * 
 * Changes to the latest version. Listener will run as a daemon 
 * Multicast Server is now Multi-threaded 
 * ------------------------------------------------------------------- 
 * headers 
 * uses 
 *   <stdlib.h> 
 *   <stdio.h> 
 *   <string.h> 
 *   <errno.h> 
 * 
 *   <sys/types.h> 
 *   <unistd.h> 
 * 
 *   <netdb.h> 
 *   <netinet/in.h> 
 *   <sys/socket.h> 
 * ------------------------------------------------------------------- */ 


#define HEADERS() 

#include "headers.h" 
#include "Listener.hpp" 
#include "Server.hpp" 
#include "Locale.hpp" 
#include "Audience.hpp"
#include "List.h"
#include "Speaker.hpp"
#include "Client.hpp"
#include "util.h" 


/* ------------------------------------------------------------------- 
 * Stores local hostname and socket info. 
 * ------------------------------------------------------------------- */ 

Listener::Listener( ext_Settings *inSettings ) 
: PerfSocket( inSettings ), 
Thread() {

    mCount = ( inSettings->mThreads != 0 );
    mClients = inSettings->mThreads;

    // open listening socket 
    Listen( inSettings->mLocalhost, mSettings->mDomain ); 
    ReportServerSettings( inSettings->mLocalhost ); 

} // end Listener 

/* ------------------------------------------------------------------- 
 * Delete memory (hostname string). 
 * ------------------------------------------------------------------- */ 
Listener::~Listener() {
    DELETE_ARRAY( mSettings->mHost      );
    DELETE_ARRAY( mSettings->mLocalhost );
    DELETE_ARRAY( mSettings->mFileName  );
    DELETE_ARRAY( mSettings->mOutputFileName );
    DELETE_PTR( mSettings );
} // end ~Listener 

/* ------------------------------------------------------------------- 
 * Listens for connections and starts Servers to handle data. 
 * For TCP, each accepted connection spawns a Server thread. 
 * For UDP, handle all data in this thread for Win32 Only, otherwise
 *          spawn a new Server thread. 
 * ------------------------------------------------------------------- */ 
void Listener::Run( void ) {
    extern Mutex clients_mutex;
    extern Iperf_ListEntry *clients;

    Audience *theAudience=NULL;
    SocketAddr     *client = NULL;
    if ( mSettings->mHost != NULL ) {
        client = new SocketAddr( mSettings->mHost, mSettings->mPort, mSettings->mDomain );
    }
    ext_Settings *tempSettings = NULL;
    iperf_sockaddr peer;
    Iperf_ListEntry *exist, *listtemp;

    if ( mUDP ) {
        struct UDP_datagram* mBuf_UDP  = (struct UDP_datagram*) mBuf;
        client_hdr* hdr = (client_hdr*) (mBuf_UDP + 1);
        // UDP uses listening socket 
        // The server will now run as a multi-threaded server 

        // Accept each packet, 
        // If there is no existing client, then start  
        // a new thread to service the new client 
        // The main server runs in a single thread 
        // Thread per client model is followed 
        do {
            peer = Accept_UDP(); 
            if ( client != NULL ) {
                if ( !SocketAddr::Hostare_Equal( client->get_sockaddr(), 
                                                 (sockaddr*) &peer ) ) {
                    continue;
                }
            }
#ifdef HAVE_THREAD
            clients_mutex.Lock(); 
            exist = Iperf_present( &peer, clients); 
            clients_mutex.Unlock(); 
            int32_t datagramID = ntohl( mBuf_UDP->id ); 
            if ( exist == NULL && datagramID >= 0 ) {
                int rc = connect( mSock, (struct sockaddr*) &peer,
                                  // Some OSes do not like the size of sockaddr_storage so we
                                  // get more exact here..
#ifndef IPV6
                                  sizeof(sockaddr_in));
#else
                                  (((struct sockaddr*)&peer)->sa_family == AF_INET ? 
                                   sizeof(sockaddr_in) : sizeof(sockaddr_in6)));
#endif // IPV6
                FAIL_errno( rc == SOCKET_ERROR, "connect UDP" );       
#ifndef WIN32
                listtemp = new Iperf_ListEntry;
                memcpy(listtemp, &peer, sizeof(peer));
                listtemp->next = NULL;
                clients_mutex.Lock(); 
                exist = Iperf_hostpresent( &peer, clients); 

                if ( exist != NULL ) {
                    listtemp->holder = exist->holder;
                    exist->holder->AddSocket(mSock);
                } else {
                    clients_mutex.Unlock();
                    tempSettings = NULL;
                    if ( !mSettings->mCompat ) {
                        Settings::GenerateSpeakerSettings( mSettings, &tempSettings, 
                                                           hdr, (sockaddr*) &peer );
                    }

                    theAudience = new Audience( mSettings, mSock ); 

                    if ( tempSettings != NULL ) {
                        Speaker *theSpeaker = new Speaker( tempSettings );
                        theSpeaker->OwnSettings();
                        theSpeaker->DeleteSelfAfterRun();
                        if ( tempSettings->mMode == kTest_DualTest ) {
                            theSpeaker->Start();
                        } else {
                            theAudience->StartWhenDone( theSpeaker );
                        }
                    }

                    listtemp->holder = theAudience;

                    // startup the server thread, then forget about it 
                    theAudience->DeleteSelfAfterRun(); 
                    theAudience->Start(); 
                    theAudience = NULL; 
                    clients_mutex.Lock(); 
                }
                Iperf_pushback( listtemp, &clients ); 
                clients_mutex.Unlock(); 
#else /* WIN32 */ 
                tempSettings = NULL;
                if ( !mSettings->mCompat ) {
                    Settings::GenerateSpeakerSettings( mSettings, &tempSettings, 
                                                       hdr, (sockaddr*) &peer );
                }

                bool startLate = false;
                Speaker *theSpeaker = NULL;
                if ( tempSettings != NULL ) {
                    theSpeaker = new Speaker( tempSettings );
                    theSpeaker->OwnSettings();
                    theSpeaker->DeleteSelfAfterRun();
                    if ( tempSettings->mMode == kTest_DualTest ) {
                        theSpeaker->Start();
                    } else {
                        startLate = true;
                    }
                }
                // WIN 32 Does not handle multiple UDP stream hosts.
                Server *theServer=NULL;
                theServer = new Server(mSettings, mSock); 
                theServer->Run(); 
                DELETE_PTR( theServer );
                if ( startLate && theSpeaker != NULL ) {
                    theSpeaker->Start();
                    theSpeaker = NULL;
                }

#endif /* WIN32 */
#else // HAVE_THREAD
            {
                tempSettings = NULL;
                int rc = connect( mSock, (struct sockaddr*) &peer,
                                  // Some OSes do not like the size of sockaddr_storage so we
                                  // get more exact here..
#ifndef IPV6
                                  sizeof(sockaddr_in));
#else
                                  (((struct sockaddr*)&peer)->sa_family == AF_INET ? 
                                   sizeof(sockaddr_in) : sizeof(sockaddr_in6)));
#endif // IPV6
                FAIL_errno( rc == SOCKET_ERROR, "connect UDP" );       
                if ( !mSettings->mCompat ) {
                    Settings::GenerateSpeakerSettings( mSettings, &tempSettings, 
                                                        hdr, (sockaddr*) &peer );
                }
                Server *theServer=NULL;
                
                theServer = new Server(mSettings, mSock);
                theServer->DeleteSelfAfterRun();
                theServer->Start();
    
                if ( tempSettings != NULL ) {
                    Client *theClient = NULL;
                    theClient = new Client( tempSettings );
                    theClient->DeleteSelfAfterRun();
                    theClient->Start();
                    theClient = NULL;
                    DELETE_PTR( tempSettings );
                }
#endif // HAVE_THREAD
                // create a new socket 
                mSock = -1; 
                Listen( mSettings->mLocalhost, mSettings->mDomain );
                mClients--; 
            }
        } while ( !mCount || ( mCount && mClients > 0 ) ); 
    } else {
        // TCP uses sockets returned from Accept 
        client_hdr buf;
        int connected_sock; 
        do {
            connected_sock = Accept(); 
            if ( connected_sock >= 0 ) {
                Socklen_t temp = sizeof( peer );
                getpeername( connected_sock, (sockaddr*)&peer, &temp );
                if ( client != NULL ) {
                    if ( !SocketAddr::Hostare_Equal( client->get_sockaddr(), 
                                                     (sockaddr*) &peer ) ) {
                        close( connected_sock );
                        continue;
                    }
                }
                tempSettings = NULL;
#ifdef HAVE_THREAD
                clients_mutex.Lock(); 
                exist = Iperf_hostpresent( &peer, clients); 
                listtemp = new Iperf_ListEntry;
                memcpy(listtemp, &peer, sizeof(peer));
                listtemp->next = NULL;

                if ( exist != NULL ) {
                    listtemp->holder = exist->holder;
                    exist->holder->AddSocket(connected_sock);
                } else {
                    clients_mutex.Unlock(); 
                    if ( !mSettings->mCompat ) {
                        if ( recv( connected_sock, (char*)&buf, sizeof(buf), 0) > 0 ) {
                            Settings::GenerateSpeakerSettings( mSettings, &tempSettings, 
                                                               &buf, (sockaddr*) &peer );
                        }
                    }

                    theAudience = new Audience( mSettings, connected_sock ); 

                    if ( tempSettings != NULL ) {
                        Speaker *theSpeaker = new Speaker( tempSettings );
                        theSpeaker->OwnSettings();
                        theSpeaker->DeleteSelfAfterRun();
                        if ( tempSettings->mMode == kTest_DualTest ) {
                            theSpeaker->Start();
                        } else {
                            theAudience->StartWhenDone( theSpeaker );
                        }
                    }

                    listtemp->holder = theAudience;

                    // startup the server thread, then forget about it 
                    theAudience->DeleteSelfAfterRun(); 
                    theAudience->Start(); 
                    theAudience = NULL; 
                    clients_mutex.Lock(); 
                }
                Iperf_pushback( listtemp, &clients ); 
                clients_mutex.Unlock();
#else
                if ( !mSettings->mCompat ) {
                    if ( recv( connected_sock, (char*)&buf, sizeof(buf), 0 ) > 0 ) {
                        Settings::GenerateSpeakerSettings( mSettings, &tempSettings, 
                                                           &buf, (sockaddr*) &peer );
                    }
                }

                Server* theServer = new Server( mSettings, connected_sock );
                theServer->DeleteSelfAfterRun();
                theServer->Start();
                
                if ( tempSettings != NULL ) {
                    Client *theClient = new Client( tempSettings );
                    theClient->DeleteSelfAfterRun();
                    theClient->Start();
                    DELETE_PTR( tempSettings );
                }
#endif
                mClients--; 
            }
        } while ( !mCount || ( mCount && mClients > 0 ) ); 
    }
} // end Run 


/**-------------------------------------------------------------------- 
 * Run the server as a daemon  
 * --------------------------------------------------------------------*/ 

void Listener::runAsDaemon(const char *pname, int facility) {
#ifndef WIN32 
    pid_t pid; 

    /* Create a child process & if successful, exit from the parent process */ 
    if ( (pid = fork()) == -1 ) {
        printf("error in first child create\n");     
        exit(0); 
    } else if ( pid != 0 ) {
        exit(0); 
    }

    /* Try becoming the session leader, once the parent exits */
    if ( setsid() == -1 ) {           /* Become the session leader */ 
        fputs("Cannot change the session group leader\n",stdout); 
    } else {
    } 
    signal(SIGHUP,SIG_IGN); 


    /* Now fork() and get released from the terminal */  
    if ( (pid = fork()) == -1 ) {
        printf("error\n");   
        exit(0); 
    } else if ( pid != 0 ) {
        exit(0); 
    }

    chdir("."); 
    printf("Running Iperf Server as a daemon\n"); 
    printf("The Iperf daemon process ID : %d\n",((int)getpid())); 
    fflush(stdout);  
    /*umask(0);*/ 

    fclose(stdout); 
    fclose(stdin); 
    openlog(pname,LOG_CONS,facility); 
#else 
    printf("Use the precompiled windows version for service (daemon) option\n"); 
#endif  

} 
