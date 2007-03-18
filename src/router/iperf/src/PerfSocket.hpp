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
 * PerfSocket.hpp
 * by Mark Gates <mgates@nlanr.net>
 *  &  Ajay Tirumala <tirumala@ncsa.uiuc.edu>   
 * -------------------------------------------------------------------
 * Changes in version 1.6
 *   Incorporates class declarations for fetching data from files 
 * ------------------------------------------------------------------- */


#ifndef PERFSOCKET_H
#define PERFSOCKET_H

#include "Socket.hpp"
#include "Timestamp.hpp"
#include "Mutex.hpp"
#include "Extractor.hpp"
#include "Notify.hpp"
#include "Settings.hpp"


class Notify;
/* ------------------------------------------------------------------- */
class PerfSocket : public Socket {
public:
    // stores port, UDP/TCP mode, and UDP rate
    PerfSocket( ext_Settings *inSettings, Notify* toNotify = NULL );

    // destroy the iperf socket object
    virtual ~PerfSocket();

    iperf_sockaddr Accept_UDP( void );

    // lock while doing reporting; printf often isn't thread safe.
    static Mutex sReporting;
    static int sReportCount;


protected:
    // UDP, in PerfSocket_UDP.cpp
    void Send_UDP( void );
    void Recv_UDP( void );

    void write_UDP_FIN( );
    void write_UDP_AckFIN( max_size_t mTotalLen, Timestamp mEndTime,
                           Timestamp mStartTime, int errorCnt,
                           int outofOrder, int32_t datagramID );

    // TCP, in PerfSocket_TCP.cpp
    void Send_TCP( void );
    void Recv_TCP( void );

    // Used for automatic determining of Window size
    void Client_Recv_TCP(void);
    void Server_Send_TCP(void);

    void Multicast_remove_client( iperf_sockaddr );

    virtual void SetSocketOptions( void );
    virtual int set_tcp_windowsize( int, int ) = 0;
    virtual int get_tcp_windowsize( int ) = 0;

    // General, in PerfSocket.cpp
    void InitTransfer( void );

    void ReportPeriodicBW( void );
    void ReportBW( max_size_t inBytes, double inStart, double inStop );

    void ReportPeriodicBW_Jitter_Loss( int32_t errorCnt,
                                       int32_t outofOrder,
                                       int32_t datagramID );

    void ReportBW_Jitter_Loss( max_size_t inBytes,
                               double inStart, double inStop,
                               int32_t inErrorcnt,
                               int32_t inOutofOrder,
                               int32_t inDatagrams );

    void ReportPeer( int inSock );
    void ReportMSS( int MSS );
    void ReportWindowSize( void );

    void ReportClientSettings( const char* inHost,
                               const char* inLocalhost );
    void ReportServerSettings( const char* inLocalhost );

    // handle interupts
    static void Sig_Interupt( int inSigno );

    static bool sInterupted;

    // Extra Settings
    ext_Settings *mSettings;

    // buffer to do reads/writes
    char *mBuf;

    // individual and cummulative bytes written
    max_size_t mTotalLen;

    // termination variables
    bool mMode_time;
    max_size_t mAmount;

    // UDP jitter and loss calculations
    double mJitter;

    Timestamp mPacketTime;
    Timestamp mStartTime;
    Timestamp mEndTime;

    // periodic reporting bandwidth, loss, jitter
    Timestamp mPLastTime;
    Timestamp mPNextTime;
    Timestamp mPInterval;

    bool mPReporting;
    int32_t mPLastErrorcnt;
    int32_t mPLastOutofOrder;
    int32_t mPLastDatagramID;
    max_size_t mPLastTotalLen;

    Extractor *extractor;
    Notify    *ptr_parent;


}; // end class PerfSocket

#endif // PERFSOCKET_H
