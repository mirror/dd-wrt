/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002                              
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
 * Notify.cpp
 * by Kevin Gibbs <kgibbs@ncsa.uiuc.edu>
 * ---------------------------------------------------------------- */

#include "headers.h"
#include "Notify.hpp"
#include "Locale.hpp"
#include "Settings.hpp"

/*
 * Create a Notify instance which is responsible
 * for summing of threads.
 */
Notify::Notify( int numThreads ) {
    per_Reporting = true;
    per_Reported = 0;
    mThreads = numThreads;
    mNum_Threads = numThreads;
    mZero.set( 0, 0 );
    mStart.set( -1, -1 );
    mStop.set( -1, -1 );
    totalBytes = 0;
    mper_Start.set( -1, -1 );
    mper_Stop.set( -1, -1 );
    totalperBytes = 0;
    mOwnSettings = false;
}

/*
 * Destroy the settings if we Own them
 */
Notify::~Notify() {
    if ( mOwnSettings ) {
        DELETE_ARRAY( mSettings->mHost      );
        DELETE_ARRAY( mSettings->mLocalhost );
        DELETE_ARRAY( mSettings->mFileName  );
        DELETE_ARRAY( mSettings->mOutputFileName );
        DELETE_PTR( mSettings );
    }
}

/*
 * Barrier to wait until all the Threads
 * are running.
 */
void Notify::WaitThreadsRunning() {
    mcond.Lock();
    ThreadStarted();
    while ( !AllThreadsRunning() ) {
        mcond.Wait();
    }
    mcond.Broadcast();
    mcond.Unlock();
}

/*
 * Simple function determining if all the
 * threads are running. Assumes mcond.Lock()
 * has been called externally
 */
bool Notify::AllThreadsRunning() {
    return mNum_Threads == 0;
}

/*
 * Simple function determining if all the
 * threads are finished. Assumes mcond.Lock()
 * has been called externally
 */
bool Notify::AllThreadsDone() {
    return mNum_Threads == mThreads;
}

/*
 * Adds a thread to the number of running
 * threads. Assumes mcond.Lock() has
 * been called externally
 */
void Notify::ThreadStarted() {
    mNum_Threads--;
}

/*
 * Adds a thread to the number of finished
 * threads. If this is the last running
 * thread then it prints out the sum total.
 */
void Notify::ThreadFinished( Timestamp inStop, max_size_t inBytes ) {
    mcond.Lock();
    per_Reporting = false;
    totalBytes += inBytes;
    if ( inStop.after( mStop ) ) {
        mStop = inStop;
    }
    mNum_Threads++;
    if ( AllThreadsDone() && mThreads > 1 ) {
        PerfSocket::sReporting.Lock();

        // print a field header every 20 lines
        if ( --PerfSocket::sReportCount <= 0 ) {
            printf( report_bw_header );
            PerfSocket::sReportCount = 20;
        }

        double secs = mStop.subSec( mStart );

        char bytes[ 32 ];
        char speed[ 32 ];

        byte_snprintf( bytes, sizeof(bytes), (double)totalBytes,
                       toupper( mSettings->mFormat ));
        byte_snprintf( speed, sizeof(speed),
                       totalBytes / secs, mSettings->mFormat );

        printf( report_sum_bw_format,
                0.0, secs, bytes, speed );
        fflush( stdout );

        PerfSocket::sReporting.Unlock();
    }
    mcond.Broadcast();
    mcond.Unlock();
}

/*
 * Sets the Start time for a thread. The
 * lowest value is retained for reporting
 */
double Notify::StartTime( Timestamp inStart ) {
    if ( inStart.before( mStart ) || 
          mStart.before( mZero  ) ) {
        mStart = inStart;
    }
    return mStart.get();
}

/*
 * Periodically print the sum if the settings
 * call for interval printing.
 */
void Notify::PeriodicUpdate( Timestamp start, Timestamp stop,
                             max_size_t inbytes ) {
    per_Reported++;
    if ( start.before( mper_Start ) || 
         mper_Start.before( mZero ) ) {
        mper_Start = start;
    }
    if ( stop.after( mper_Stop ) ) {
        mper_Stop = stop;
    }
    totalperBytes += inbytes;

    if ( mThreads > 1 && mThreads == per_Reported ) {
        // print a field header every 20 lines
        if ( --PerfSocket::sReportCount <= 0 ) {
            printf( report_bw_header );
            PerfSocket::sReportCount = 20;
        }

        char bytes[ 32 ];
        char speed[ 32 ];

        byte_snprintf( bytes, sizeof(bytes), (double)totalperBytes,
                       toupper( mSettings->mFormat ));
        byte_snprintf( speed, sizeof(speed),
                       totalperBytes / mper_Stop.subSec( mper_Start ), 
                       mSettings->mFormat);

        printf( report_sum_bw_format,
                mper_Start.get(), mper_Stop.get(), 
                bytes, speed );
        fflush( stdout );
        mper_Start.set( -1, -1 );
        mper_Stop.set( -1, -1 );
        totalperBytes = 0;
        per_Reported = 0;
    }
}

/*
 * Tells the Notify instance that it is
 * responsible for freeing the settings
 * that were passed to it.
 */
void Notify::OwnSettings() {
    mOwnSettings = true;
}
