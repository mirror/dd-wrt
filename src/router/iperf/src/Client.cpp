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
 * Client.cpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A client thread initiates a connect to the server and handles
 * sending and receiving data, then closes the socket.
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <string.h>
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"
#include "Client.hpp"
#include "Settings.hpp"
#include "util.h"

/* -------------------------------------------------------------------
 * Store server hostname, optionally local hostname, and socket info.
 * ------------------------------------------------------------------- */

Client::Client( ext_Settings *inSettings, bool inPrintSettings,
                Notify* toNotify )
: PerfSocket( inSettings, toNotify ),
Thread() {
    mSettings = inSettings;

    // connect
    Connect( mSettings->mHost, mSettings->mLocalhost );

    if ( inPrintSettings ) {
        ReportClientSettings( mSettings->mHost, mSettings->mLocalhost );
    }

} // end Client

/* -------------------------------------------------------------------
 * Delete memory (hostname strings).
 * ------------------------------------------------------------------- */

Client::~Client() {
} // end ~Client

/* -------------------------------------------------------------------
 * Connect to the server and send data.
 * ------------------------------------------------------------------- */

void Client::Run( void ) {
#ifdef HAVE_THREAD

    // Barrier
    // wait until the number of anticipated threads have reached this point
    if ( ptr_parent ) {
        ptr_parent->WaitThreadsRunning();
    }
#endif

    // send data
    if ( mUDP ) {
        Send_UDP();
    } else {
        Send_TCP();
    }

    if ( ptr_parent != NULL ) {
        ptr_parent->ThreadFinished(mEndTime, mTotalLen);
    }
} // end Run

void Client::InitiateServer() {
    if ( !mSettings->mCompat ) {
        int currLen;
        client_hdr* temp_hdr;
        if ( mUDP ) {
            UDP_datagram *UDPhdr = (UDP_datagram *)mBuf;
            temp_hdr = (client_hdr*)(UDPhdr + 1);
        } else {
            temp_hdr = (client_hdr*)mBuf;
        }
        Settings::GenerateClientHdr( mSettings, temp_hdr );
        if ( !mUDP ) {
            currLen = send( mSock, mBuf, sizeof(client_hdr), 0 );
            if ( currLen < 0 ) {
                WARN_errno( currLen < 0, "write" );
            }
        }
    }
}
