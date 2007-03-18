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
 * Settings.hpp
 * by Mark Gates <mgates@nlanr.net>
 * &  Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * -------------------------------------------------------------------
 * Stores and parses the initial values for all the global variables.
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <assert.h>
 * ------------------------------------------------------------------- */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "headers.h"

#include "Thread.hpp"

/* -------------------------------------------------------------------
 * constants
 * ------------------------------------------------------------------- */

// server/client mode
enum ServerMode {
    kMode_Server,
    kMode_Client,
    kMode_Unknown
};

// test mode
enum TestMode {
    kTest_Normal,
    kTest_DualTest,
    kTest_TradeOff,
    kTest_Unknown
};

// protocol mode
const bool kMode_UDP    = false;
const bool kMode_TCP    = true;

// termination mode
const bool kMode_Amount = false;  // transmit fixed amount
const bool kMode_Time   = true;   // transmit fixed time

// domain mode 
const bool kMode_IPv4 = false; 
const bool kMode_IPv6 = true;


class Settings;

/*
 * The ext_Settings is a structure that holds all
 * options for a given execution of either a client
 * or server. By using this structure rather than
 * a global structure or class we can have multiple
 * clients or servers running with different settings.
 */
struct ext_Settings {
    char*  mFileName;               // -F
    char*  mHost;                   // -c
    char*  mLocalhost;              // -B
    char*  mOutputFileName;         // -o
    double mInterval;               // -i
    int mAmount;                    // -n or -t
    int mBufLen;                    // -l
    int mMSS;                       // -M
    int mTCPWin;                    // -w
    int mThreads;                   // -P
    int mTOS;                       // -S
    int mUDPRate;                   // -b or -u
    ServerMode mServerMode;         // -s or -c
    TestMode mMode;                 // -r or -d
    unsigned short mListenPort;     // -L
    unsigned short mPort;           // -p
    char   mFormat;                 // -f
    u_char mTTL;                    // -T
    bool   mBufLenSet;              // -l
    bool   mCompat;                 // -C
    bool   mDaemon;                 // -D
    bool   mDomain;                 // -V
    bool   mFileInput;              // -F or -I
    bool   mNodelay;                // -N
    bool   mPrintMSS;               // -m
    bool   mRemoveService;          // -R
    bool   mStdin;                  // -I
    bool   mStdout;                 // -o
    bool   mSuggestWin;             // -W
};

#define HEADER_VERSION1 0x80000000
#define RUN_NOW         0x00000001

// used to reference the 4 byte ID number we place in UDP datagrams
// use int32_t if possible, otherwise a 32 bit bitfield (e.g. on J90) 
struct UDP_datagram {
#ifdef HAVE_INT32_T
    int32_t id;
    u_int32_t tv_sec;
    u_int32_t tv_usec;
#else
    signed   int id      : 32;
    unsigned int tv_sec  : 32;
    unsigned int tv_usec : 32;
#endif
};

/*
 * The client_hdr structure is sent from clients
 * to servers to alert them of things that need
 * to happen. Order must be perserved in all 
 * future releases for backward compatibility.
 * 1.7 has flags, numThreads, mPort, and bufferlen
 */
struct client_hdr {

#ifdef HAVE_INT32_T

    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no 
     * information bits are set then the header is ignored.
     * The lowest order diferentiates between dualtest and
     * tradeoff modes, wheither the speaker needs to start 
     * immediately or after the audience finishes.
     */
    int32_t flags;
    int32_t numThreads;
    int32_t mPort;
    int32_t bufferlen;
    int32_t mWinBand;
    int32_t mAmount;
#else
    signed int flags      : 32;
    signed int numThreads : 32;
    signed int mPort      : 32;
    signed int bufferlen  : 32;
    signed int mWinBand   : 32;
    signed int mAmount    : 32;
#endif
};

/*
 * The server_hdr structure facilitates the server
 * report of jitter and loss on the client side.
 * It piggy_backs on the existing clear to close
 * packet.
 */
struct server_hdr {

#ifdef HAVE_INT32_T

    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no 
     * information bits are set then the header is ignored.
     */
    int32_t flags;
    int32_t total_len1;
    int32_t total_len2;
    int32_t stop_sec;
    int32_t stop_usec;
    int32_t error_cnt;
    int32_t outorder_cnt;
    int32_t datagrams;
    int32_t jitter1;
    int32_t jitter2;
#else
    signed int flags        : 32;
    signed int total_len1   : 32;
    signed int total_len2   : 32;
    signed int stop_sec     : 32;
    signed int stop_usec    : 32;
    signed int error_cnt    : 32;
    signed int outorder_cnt : 32;
    signed int datagrams    : 32;
    signed int jitter1      : 32;
    signed int jitter2      : 32;
#endif

};

/* ------------------------------------------------------------------- */
class Settings {
public:
    // set to defaults
    Settings( ext_Settings* main );

    // free associated memory
    ~Settings();

    // parse settings from user's environment variables
    void ParseEnvironment( void );

    // parse settings from app's command line
    void ParseCommandLine( int argc, char **argv );

    // convert to lower case for [KMG]bits/sec
    const void GetLowerCaseArg(const char *,char *);

    // conver to upper case for [KMG]bytes/sec
    const void GetUpperCaseArg(const char *,char *);

    // generate settings for listener instance
    static void GenerateListenerSettings( ext_Settings *old, ext_Settings **listener);

    // generate settings for speaker instance
    static void GenerateSpeakerSettings( ext_Settings *old, ext_Settings **speaker,
                                         client_hdr *hdr, sockaddr* peer );

    // generate client header for server
    static void GenerateClientHdr( ext_Settings *old, client_hdr *hdr );

    // ---- access settings

    // -b #
    // valid in UDP mode
    double GetUDPRate( void ) const {
        assert( GetProtocolMode() == kMode_UDP );
        return mExtSettings->mUDPRate;
    }

    // -c <host>
    // valid in client mode
    const char* GetHost( void ) const {
        assert( mExtSettings->mServerMode == kMode_Client );
        return mExtSettings->mHost;
    }

    // -f [kmKM]
    char GetFormat( void ) const {
        return mExtSettings->mFormat;
    }

    // -l #
    int GetBufferLen( void ) const {
        return mExtSettings->mBufLen;
    }

    // -m
    // valid in TCP mode
    bool GetPrintMSS( void ) const {
        assert( GetProtocolMode() == kMode_TCP );
        return mExtSettings->mPrintMSS;
    }

    // -n #
    // valid in Amount mode (transmit fixed amount of data)
    max_size_t GetAmount( void ) const {
        assert( mExtSettings->mAmount > 0 );
        assert( GetTerminationMode() == kMode_Amount );
        return (max_size_t) mExtSettings->mAmount;
    }

    // -p #
    unsigned short GetPort( void ) const {
        return mExtSettings->mPort;
    }

    // -L #
    unsigned short GetListenPort( void ) const {
        return mExtSettings->mListenPort;
    }

    // -s or -c
    ServerMode GetServerMode( void ) const {
        return mExtSettings->mServerMode;
    }

    // -d or -r
    TestMode GetTestingMode( void ) const {
        return mExtSettings->mMode;
    }

    // -t #
    // valid in Time mode (transmit for fixed time)
    double GetTime( void ) const {
        assert( mExtSettings->mAmount < 0 );
        return (-mExtSettings->mAmount) / 100.0;
    }

    // -w #
    int GetTCPWindowSize( void ) const {
        return mExtSettings->mTCPWin;
    }

    // ---- more esoteric options

    // -B <host>
    const char* GetLocalhost( void ) const {
        return mExtSettings->mLocalhost;
    }

    // -i #
    double GetInterval( void ) const {
        return mExtSettings->mInterval;
    }

    // -M #
    // valid in TCP mode
    int GetTCP_MSS( void ) const {
        assert( GetProtocolMode() == kMode_TCP );
        return mExtSettings->mMSS;
    }

    // -N
    // valid in TCP mode
    bool GetTCP_Nodelay( void ) const {
        assert( GetProtocolMode() == kMode_TCP );
        return mExtSettings->mNodelay;
    }

    // -P #
    int GetClientThreads( void ) const {
        return mExtSettings->mThreads;
    }

    // -S #
    int GetTOS( void ) const {
        return mExtSettings->mTOS;
    }

    // -T #
    // valid in UDP mode
    u_char GetMcastTTL( void ) const {
        return mExtSettings->mTTL;
    }

    // ---- modes

    // differentiate Time and Amount; -t # and -n #
    // note these optimize to just the comparison with zero
    bool GetTerminationMode( void ) const {
        return ( mExtSettings->mAmount < 0  ?  kMode_Time  :  kMode_Amount );
    }

    // differentiate UDP and TCP; -u or -b # gives UDP
    bool GetProtocolMode( void ) const {
        return ( mExtSettings->mUDPRate == 0  ?  kMode_TCP  :  kMode_UDP );
    }

    // whether the server runs as a daemon or not
    bool GetDaemonMode(void) const {
        return mExtSettings->mDaemon;
    }

    // whether running in compatibility mode or not
    bool GetCompatMode(void) const {
        return mExtSettings->mCompat;
    }

    // Whether the client must suggest the window size or not
    bool GetSuggestWin(void) const {
        return mExtSettings->mSuggestWin;
    }

    // Set the interval time
    void SetInterval(double interval) {
        mExtSettings->mInterval = interval;
    }

    // Set the file input status
    void SetFileInput(bool status) {
        mExtSettings->mFileInput = status;
    }


    // Whether the input is the default stream or a
    // file stream
    bool GetFileInput(void) const {
        return mExtSettings->mFileInput;
    }

    // return the file name if the input is through a 
    // file stream
    char *GetFileName(void) const {
        if ( mExtSettings->mFileInput )
            return mExtSettings->mFileName;
        return NULL;
    }

    // return whether the input is from stdin
    bool GetStdin(void) const {
        return mExtSettings->mStdin;
    }

    // file stream
    bool GetFileOutput(void) const {
        if ( mExtSettings->mStdout )
            return false;
        return true;
    }

    // return the file name if the output is through a 
    // file stream
    char *GetOutputFileName(void) const {
        if ( !mExtSettings->mStdout )
            return mExtSettings->mOutputFileName;
        return NULL;
    }

    // whether win32 service should be removed or not
    bool GetRemoveService(void) const {
        return mExtSettings->mRemoveService;
    }

    // IPv4 or IPv6 domain
    bool GetDomain(void) const {
        return mExtSettings->mDomain;
    }


protected:
    void Interpret( char option, const char *optarg );

    ext_Settings* mExtSettings;

}; // end class Settings

#endif // SETTINGS_H
