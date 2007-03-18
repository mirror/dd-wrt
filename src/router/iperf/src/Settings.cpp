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
 * Settings.cpp
 * by Mark Gates <mgates@nlanr.net>
 * & Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * -------------------------------------------------------------------
 * Stores and parses the initial values for all the global variables.
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <stdio.h>
 *   <string.h>
 *
 *   <unistd.h>
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"

#include "Settings.hpp"
#include "Locale.hpp"

#include "util.h"

#include "gnu_getopt.h"

/* -------------------------------------------------------------------
 * command line options
 *
 * The option struct essentially maps a long option name (--foobar)
 * or environment variable ($FOOBAR) to its short option char (f).
 * ------------------------------------------------------------------- */
#define LONG_OPTIONS()

const struct option long_options[] =
{
{"bandwidth",  required_argument, NULL, 'b'},
{"client",     required_argument, NULL, 'c'},
{"dualtest",         no_argument, NULL, 'd'},
{"format",     required_argument, NULL, 'f'},
{"help",             no_argument, NULL, 'h'},
{"interval",   required_argument, NULL, 'i'},
{"len",        required_argument, NULL, 'l'},
{"print_mss",        no_argument, NULL, 'm'},
{"num",        required_argument, NULL, 'n'},
{"output",     required_argument, NULL, 'o'},
{"port",       required_argument, NULL, 'p'},
{"tradeoff",         no_argument, NULL, 'r'},
{"server",           no_argument, NULL, 's'},
{"time",       required_argument, NULL, 't'},
{"udp",              no_argument, NULL, 'u'},
{"version",          no_argument, NULL, 'v'},
{"window",     required_argument, NULL, 'w'},

// more esoteric options
{"bind",       required_argument, NULL, 'B'},
{"compatibility",    no_argument, NULL, 'C'},
{"daemon",           no_argument, NULL, 'D'},
{"file_input", required_argument, NULL, 'F'},
{"stdin_input",      no_argument, NULL, 'I'},
{"mss",        required_argument, NULL, 'M'},
{"nodelay",          no_argument, NULL, 'N'},
{"listenport", required_argument, NULL, 'L'},
{"parallel",   required_argument, NULL, 'P'},
{"remove",        no_argument, NULL, 'R'},
{"tos",        required_argument, NULL, 'S'},
{"ttl",        required_argument, NULL, 'T'},
{"ipv6_domian",       no_argument, NULL, 'V'},
{"suggest_win_size",  no_argument, NULL, 'W'},
{0, 0, 0, 0}
};

#define ENV_OPTIONS()

const struct option env_options[] =
{
{"IPERF_BANDWIDTH",  required_argument, NULL, 'b'},
{"IPERF_CLIENT",     required_argument, NULL, 'c'},
{"IPERF_DUALTEST",         no_argument, NULL, 'd'},
{"IPERF_FORMAT",     required_argument, NULL, 'f'},
// skip help
{"IPERF_INTERVAL",   required_argument, NULL, 'i'},
{"IPERF_LEN",        required_argument, NULL, 'l'},
{"IPERF_PRINT_MSS",        no_argument, NULL, 'm'},
{"IPERF_NUM",        required_argument, NULL, 'n'},
{"IPERF_PORT",       required_argument, NULL, 'p'},
{"IPERF_TRADEOFF",         no_argument, NULL, 'r'},
{"IPERF_SERVER",           no_argument, NULL, 's'},
{"IPERF_TIME",       required_argument, NULL, 't'},
{"IPERF_UDP",              no_argument, NULL, 'u'},
// skip version
{"TCP_WINDOW_SIZE",  required_argument, NULL, 'w'},

// more esoteric options
{"IPERF_BIND",       required_argument, NULL, 'B'},
{"IPERF_COMPAT",       no_argument, NULL, 'C'},
{"IPERF_DAEMON",       no_argument, NULL, 'D'},
{"IPERF_FILE_INPUT", required_argument, NULL, 'F'},
{"IPERF_STDIN_INPUT",      no_argument, NULL, 'I'},
{"IPERF_MSS",        required_argument, NULL, 'M'},
{"IPERF_NODELAY",          no_argument, NULL, 'N'},
{"IPERF_LISTENPORT", required_argument, NULL, 'L'},
{"IPERF_PARALLEL",   required_argument, NULL, 'P'},
{"IPERF_TOS",        required_argument, NULL, 'S'},
{"IPERF_TTL",        required_argument, NULL, 'T'},
{"IPERF_IPV6_DOMAIN",      no_argument, NULL, 'V'},
{"IPERF_SUGGEST_WIN_SIZE", required_argument, NULL, 'W'},
{0, 0, 0, 0}
};

#define SHORT_OPTIONS()

const char short_options[] = "b:c:df:hi:l:mn:o:p:rst:uvw:B:CDF:IL:M:NP:RS:T:VW";

/* -------------------------------------------------------------------
 * defaults
 * ------------------------------------------------------------------- */
#define DEFAULTS()

const long kDefault_UDPRate = 1024 * 1024; // -u  if set, 1 Mbit/sec
const int  kDefault_UDPBufLen = 1470;      // -u  if set, read/write 1470 bytes
// 1470 bytes is small enough to be sending one packet per datagram on ethernet

// 1450 bytes is small enough to be sending one packet per datagram on ethernet
//  **** with IPv6 ****

/* -------------------------------------------------------------------
 * Initialize all settings to defaults.
 * ------------------------------------------------------------------- */

Settings::Settings( ext_Settings *main ) {

    mExtSettings = main;

    // option, default
    mExtSettings->mUDPRate    = 0;             // -b,  ie. TCP mode
    mExtSettings->mHost       = NULL;          // -c,  none, required for client
    mExtSettings->mMode       = kTest_Normal;  // -d,  mMode == kTest_DualTest
    mExtSettings->mFormat     = 'a';           // -f,  adaptive bits
    // skip help                               // -h,
    mExtSettings->mBufLenSet  = false;         // -l,	
    mExtSettings->mBufLen     = 8 * 1024;      // -l,  8 Kbyte
    mExtSettings->mInterval   = 0;             // -i,  ie. no periodic bw reports
    mExtSettings->mPrintMSS   = false;         // -m,  don't print MSS
    // mAmount is time also                    // -n,  N/A
    mExtSettings->mOutputFileName = NULL;      // -o,  filename
    mExtSettings->mPort       = 5001;          // -p,  ttcp port
    // mMode    = kTest_Normal;                // -r,  mMode == kTest_TradeOff
    mExtSettings->mServerMode = kMode_Unknown; // -s,  or -c, none
    mExtSettings->mAmount     = -1000;           // -t,  10 seconds
    // mUDPRate > 0 means UDP                  // -u,  N/A, see kDefault_UDPRate
    // skip version                            // -v,
    mExtSettings->mTCPWin     = 0;             // -w,  ie. don't set window

    // more esoteric options
    mExtSettings->mLocalhost  = NULL;          // -B,  none
    mExtSettings->mCompat     = false;         // -C,  run in Compatibility mode
    mExtSettings->mDaemon     = false;         // -D,  run as a daemon
    mExtSettings->mFileInput  = false;         // -F,
    mExtSettings->mFileName   = NULL;          // -F,  filename 
    mExtSettings->mStdin      = false;         // -I,  default not stdin
    mExtSettings->mListenPort = 0;             // -L,  listen port
    mExtSettings->mMSS        = 0;             // -M,  ie. don't set MSS
    mExtSettings->mNodelay    = false;         // -N,  don't set nodelay
    mExtSettings->mThreads    = 0;             // -P,
    mExtSettings->mRemoveService = false;      // -R,
    mExtSettings->mTOS        = 0;             // -S,  ie. don't set type of service
    mExtSettings->mTTL        = 1;             // -T,  link-local TTL
    mExtSettings->mDomain     = kMode_IPv4;    // -V,
    mExtSettings->mSuggestWin = false;         // -W,  Suggest the window size.

    mExtSettings->mStdout = true;              // default stdout


} // end Settings

/* -------------------------------------------------------------------
 * Delete memory (hostname string).
 * ------------------------------------------------------------------- */

Settings::~Settings() {
} // end ~Settings

/* -------------------------------------------------------------------
 * Parses settings from user's environment variables.
 * ------------------------------------------------------------------- */
void Settings::ParseEnvironment( void ) {
    char *theVariable;

    int i = 0;
    while ( env_options[i].name != NULL ) {
        theVariable = getenv( env_options[i].name );
        if ( theVariable != NULL ) {
            Interpret( env_options[i].val, theVariable );
        }
        i++;
    }
} // end ParseEnvironment

/* -------------------------------------------------------------------
 * Parse settings from app's command line.
 * ------------------------------------------------------------------- */

void Settings::ParseCommandLine( int argc, char **argv ) {
    int option;
    while ( (option =
             gnu_getopt_long( argc, argv, short_options,
                              long_options, NULL )) != EOF ) {
        Interpret( option, gnu_optarg );
    }

    for ( int i = gnu_optind; i < argc; i++ ) {
        fprintf( stderr, "%s: ignoring extra argument -- %s\n", argv[0], argv[i] );
    }
} // end ParseCommandLine

/* -------------------------------------------------------------------
 * Interpret individual options, either from the command line
 * or from environment variables.
 * ------------------------------------------------------------------- */

void Settings::Interpret( char option, const char *optarg ) {
    char outarg[100];

    switch ( option ) {
        case 'b': // UDP bandwidth
            if ( mExtSettings->mUDPRate == 0 ) {
                printf( warn_implied_udp, option );
            }

            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }

            GetLowerCaseArg(optarg,outarg);
            mExtSettings->mUDPRate = byte_atoi(outarg);

            // if -l has already been processed, mBufLenSet is true
            // so don't overwrite that value.
            if ( ! mExtSettings->mBufLenSet ) {
                mExtSettings->mBufLen = kDefault_UDPBufLen;
            }
            break;

        case 'c': // client mode w/ server host to connect to
            if ( mExtSettings->mServerMode == kMode_Unknown ) {
                mExtSettings->mServerMode = kMode_Client;
                mExtSettings->mThreads = 1;
            }

            mExtSettings->mHost = new char[ strlen( optarg ) + 1 ];
            strcpy( mExtSettings->mHost, optarg );
            break;

        case 'd': // Dual-test Mode
            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }
            if ( mExtSettings->mCompat ) {
                printf( warn_invalid_compatibility_option, option );
            }
#ifdef HAVE_THREAD
            mExtSettings->mMode = kTest_DualTest;
#else
            printf( warn_invalid_single_threaded, option );
            mExtSettings->mMode = kTest_TradeOff;
#endif
            break;

        case 'f': // format to print in
            mExtSettings->mFormat = (*optarg);
            break;

        case 'h': // print help and exit
#ifndef WIN32
            fprintf( stderr, usage_long );
#else
            fprintf(stderr, usage_long1);
            fprintf(stderr, usage_long2);
#endif
            exit(1);
            break;

        case 'i': // specify interval between periodic bw reports
            mExtSettings->mInterval = atof( optarg );
            if ( mExtSettings->mInterval < 0.5 ) {
                printf (report_interval_small, mExtSettings->mInterval);
                mExtSettings->mInterval = 0.5;
            }
            break;

        case 'l': // length of each buffer
            GetUpperCaseArg(optarg,outarg);
            mExtSettings->mBufLen = byte_atoi( outarg );
            mExtSettings->mBufLenSet = true;
            if ( mExtSettings->mUDPRate == 0 &&
                 mExtSettings->mBufLen < (int) sizeof( client_hdr ) &&
                 !mExtSettings->mCompat ) {
                mExtSettings->mCompat = true;
                printf( warn_implied_compatibility, option );
            } else if ( mExtSettings->mUDPRate > 0 ) {
                if ( mExtSettings->mBufLen < (int) sizeof( UDP_datagram ) ) {
                    mExtSettings->mBufLen = sizeof( UDP_datagram );
                    printf( warn_buffer_too_small, mExtSettings->mBufLen );
                } else if ( !mExtSettings->mCompat &&
                            mExtSettings->mBufLen < (int) ( sizeof( UDP_datagram )
                            + sizeof( client_hdr ) ) ) {
                    mExtSettings->mCompat = true;
                    printf( warn_implied_compatibility, option );
                }
            }

            break;

        case 'm': // print TCP MSS
            mExtSettings->mPrintMSS = true;
            break;

        case 'n': // bytes of data
            // positive indicates amount mode (instead of time mode)
            GetUpperCaseArg(optarg,outarg);
            mExtSettings->mAmount = +byte_atoi( outarg );
            break;

        case 'o' : // output the report and other messages into the file
            mExtSettings->mStdout = false;
            mExtSettings->mOutputFileName = new char[strlen(optarg)+1];
            strcpy( mExtSettings->mOutputFileName, optarg);
            break;

        case 'p': // server port
            mExtSettings->mPort = atoi( optarg );
            break;

        case 'r': // test mode tradeoff
            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }
            if ( mExtSettings->mCompat ) {
                printf( warn_invalid_compatibility_option, option );
            }

            mExtSettings->mMode = kTest_TradeOff;
            break;

        case 's': // server mode
            if ( mExtSettings->mServerMode != kMode_Unknown ) {
                printf( warn_invalid_client_option, option );
                break;
            }

            mExtSettings->mServerMode = kMode_Server;
            break;

        case 't': // seconds to write for
            // negative indicates time mode (instead of amount mode)
            mExtSettings->mAmount = (int) (-atof( optarg ) * 100);
            break;

        case 'u': // UDP instead of TCP
            // if -b has already been processed, UDP rate will
            // already be non-zero, so don't overwrite that value
            if ( mExtSettings->mUDPRate == 0 ) {
                mExtSettings->mUDPRate = kDefault_UDPRate;
            }

            // if -l has already been processed, mBufLenSet is true
            // so don't overwrite that value.
            if ( ! mExtSettings->mBufLenSet ) {
                mExtSettings->mBufLen = kDefault_UDPBufLen;
            } else if ( mExtSettings->mBufLen < (int) ( sizeof( UDP_datagram ) 
                        + sizeof( client_hdr ) ) &&
                        !mExtSettings->mCompat ) {
                mExtSettings->mCompat = true;
                printf( warn_implied_compatibility, option );
            }
            break;

        case 'v': // print version and exit
            fprintf( stderr, version );
            exit(1);
            break;

        case 'w': // TCP window size (socket buffer size)
            GetUpperCaseArg(optarg,outarg);
            mExtSettings->mTCPWin = byte_atoi(outarg);

            if ( mExtSettings->mTCPWin < 2048 ) {
                printf( warn_window_small, mExtSettings->mTCPWin );
            }
            break;

            // more esoteric options
        case 'B': // specify bind address
            mExtSettings->mLocalhost = new char[ strlen( optarg ) + 1 ];
            strcpy( mExtSettings->mLocalhost, optarg );
            break;

        case 'C': // Run in Compatibility Mode
            mExtSettings->mCompat = true;
            if ( mExtSettings->mMode != kTest_Normal ) {
                printf( warn_invalid_compatibility_option,
                        ( mExtSettings->mMode == kTest_DualTest ?
                          'd' : 'r' ) );
                mExtSettings->mMode = kTest_Normal;
            }
            break;

        case 'D': // Run as a daemon
            mExtSettings->mDaemon = true;
            break;

        case 'F' : // Get the input for the data stream from a file
            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }

            mExtSettings->mFileInput = true;
            mExtSettings->mFileName = new char[strlen(optarg)+1];
            strcpy( mExtSettings->mFileName, optarg);
            break;

        case 'I' : // Set the stdin as the input source
            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }

            mExtSettings->mFileInput = true;
            mExtSettings->mStdin     = true;
            mExtSettings->mFileName = new char[strlen("<stdin>")+1];
            strcpy( mExtSettings->mFileName,"<stdin>");
            break;

        case 'L': // Listen Port (bidirectional testing client-side)
            if ( mExtSettings->mServerMode != kMode_Client ) {
                printf( warn_invalid_server_option, option );
                break;
            }

            mExtSettings->mListenPort = atoi( optarg );
            break;

        case 'M': // specify TCP MSS (maximum segment size)
            GetUpperCaseArg(optarg,outarg);

            mExtSettings->mMSS = byte_atoi( outarg );
            break;

        case 'N': // specify TCP nodelay option (disable Jacobson's Algorithm)
            mExtSettings->mNodelay = true;
            break;

        case 'P': // number of client threads
#ifdef HAVE_THREAD
            mExtSettings->mThreads = atoi( optarg );
#else
            if ( mExtSettings->mServerMode != kMode_Server ) {
                printf( warn_invalid_single_threaded, option );
            } else {
                mExtSettings->mThreads = atoi( optarg );
            }
#endif
            break;

        case 'R':
            mExtSettings->mRemoveService = true;
            break;

        case 'S': // IP type-of-service
            // TODO use a function that understands base-2
            // the zero base here allows the user to specify
            // "0x#" hex, "0#" octal, and "#" decimal numbers
            mExtSettings->mTOS = strtol( optarg, NULL, 0 );
            break;

        case 'T': // time-to-live for multicast
            mExtSettings->mTTL = atoi( optarg );
            break;

        case 'V': // IPv6 Domain
            mExtSettings->mDomain = kMode_IPv6;
            break;

        case 'W' :
            mExtSettings->mSuggestWin = false;
            printf("The -W option is not available in this release\n");
            break;

        default: // ignore unknown
            break;
    }
} // end Interpret

const void Settings::GetUpperCaseArg(const char *inarg, char *outarg) {

    int len = strlen(inarg);
    strcpy(outarg,inarg);

    if ( (len > 0) && (inarg[len-1] >='a') 
         && (inarg[len-1] <= 'z') )
        outarg[len-1]= outarg[len-1]+'A'-'a';
}

const void Settings::GetLowerCaseArg(const char *inarg, char *outarg) {

    int len = strlen(inarg);
    strcpy(outarg,inarg);

    if ( (len > 0) && (inarg[len-1] >='A') 
         && (inarg[len-1] <= 'Z') )
        outarg[len-1]= outarg[len-1]-'A'+'a';
}

/*
 * Settings::GenerateListenerSettings
 * Called to generate the settings to be passed to the Listener
 * instance that will handle dual testings from the client side
 * this should only return an instance if it was called on 
 * the ext_Settings instance generated from the command line 
 * for client side execution 
 */
void Settings::GenerateListenerSettings( ext_Settings *old, ext_Settings **listener ) {

    if ( !old->mCompat && 
         (old->mMode == kTest_DualTest || old->mMode == kTest_TradeOff) ) {
        *listener = new ext_Settings;
        memcpy(*listener, old, sizeof( ext_Settings ));
        (*listener)->mCompat     = true;
        (*listener)->mDaemon     = false;
        if ( old->mListenPort != 0 ) {
            (*listener)->mPort   = old->mListenPort;
        } else {
            (*listener)->mPort   = old->mPort;
        }
        (*listener)->mFileName   = NULL;
        (*listener)->mHost       = NULL;
        (*listener)->mLocalhost  = NULL;
        (*listener)->mOutputFileName = NULL;
        (*listener)->mMode       = kTest_Normal;
        (*listener)->mServerMode = kMode_Server;
        if ( old->mHost != NULL ) {
            (*listener)->mHost = new char[strlen( old->mHost ) + 1];
            strcpy( (*listener)->mHost, old->mHost );
        }
        if ( old->mLocalhost != NULL ) {
            (*listener)->mLocalhost = new char[strlen( old->mLocalhost ) + 1];
            strcpy( (*listener)->mLocalhost, old->mLocalhost );
        }
    } else {
        *listener = NULL;
    }
}

/*
 * Settings::GenerateSpeakerSettings
 * Called to generate the settings to be passed to the Speaker
 * instance that will handle dual testings from the server side
 * this should only return an instance if it was called on 
 * the ext_Settings instance generated from the command line 
 * for server side execution. This should be an inverse operation
 * of GenerateClientHdr. 
 */
void Settings::GenerateSpeakerSettings( ext_Settings *old, ext_Settings **speaker, 
                                        client_hdr *hdr, sockaddr* peer ) {
    int flags = ntohl(hdr->flags);
    if ( (flags & HEADER_VERSION1) != 0 ) {
        *speaker = new ext_Settings;
        memcpy(*speaker, old, sizeof( ext_Settings ));
        (*speaker)->mCompat     = true;
        (*speaker)->mPort       = (unsigned short) ntohl(hdr->mPort);
        (*speaker)->mThreads    = ntohl(hdr->numThreads);
        if ( hdr->bufferlen != 0 ) {
            (*speaker)->mBufLen = ntohl(hdr->bufferlen);
        }
        if ( hdr->mWinBand != 0 ) {
            if ( old->mUDPRate == 0 ) {
                (*speaker)->mTCPWin = ntohl(hdr->mWinBand);
            } else {
                (*speaker)->mUDPRate = ntohl(hdr->mWinBand);
            }
        }
        (*speaker)->mAmount     = ntohl(hdr->mAmount);
        (*speaker)->mFileName   = NULL;
        (*speaker)->mHost       = NULL;
        (*speaker)->mLocalhost  = NULL;
        (*speaker)->mOutputFileName = NULL;
        (*speaker)->mMode       = ((flags & RUN_NOW) == 0 ?
                                   kTest_TradeOff : kTest_DualTest);
        (*speaker)->mServerMode = kMode_Client;
        if ( old->mLocalhost != NULL ) {
            (*speaker)->mLocalhost = new char[strlen( old->mLocalhost ) + 1];
            strcpy( (*speaker)->mLocalhost, old->mLocalhost );
        }
        (*speaker)->mHost = new char[REPORT_ADDRLEN];
        if ( peer->sa_family == AF_INET ) {
            inet_ntop( AF_INET, &((sockaddr_in*)peer)->sin_addr, 
                       (*speaker)->mHost, REPORT_ADDRLEN);
        }
#ifdef IPV6
          else {
            inet_ntop( AF_INET6, &((sockaddr_in6*)peer)->sin6_addr, 
                       (*speaker)->mHost, REPORT_ADDRLEN);
        }
#endif
    } else {
        *speaker = NULL;
    }
}

/*
 * Settings::GenerateClientHdr
 * Called to generate the client header to be passed to the
 * server that will handle dual testings from the server side
 * This should be an inverse operation of GenerateSpeakerSettings
 */
void Settings::GenerateClientHdr( ext_Settings *old, client_hdr *hdr ) {
    if ( old->mMode != kTest_Normal ) {
        hdr->flags  = htonl(HEADER_VERSION1);
    } else {
        hdr->flags  = 0;
    }
    if ( old->mBufLenSet ) {
        hdr->bufferlen = htonl(old->mBufLen);
    } else {
        hdr->bufferlen = 0;
    }
    if ( old->mUDPRate == 0 ) {
        hdr->mWinBand  = htonl(old->mTCPWin);
    } else {
        hdr->mWinBand  = htonl(old->mUDPRate);
    }
    if ( old->mListenPort != 0 ) {
        hdr->mPort  = htonl(old->mListenPort);
    } else {
        hdr->mPort  = htonl(old->mPort);
    }
    hdr->numThreads = htonl(old->mThreads);
    hdr->mAmount    = htonl(old->mAmount);
    if ( old->mMode == kTest_DualTest ) {
        hdr->flags |= htonl(RUN_NOW);
    }
}

