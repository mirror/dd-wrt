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
 * Audience.cpp
 * by Kevin Gibbs <kgibbs@ncsa.uiuc.edu>
 * ---------------------------------------------------------------- */

#include "headers.h"
#include "Audience.hpp"
#include "Server.hpp"

/*
 * Create an Audience of for multiple Servers.
 * Start with the first Server socket and then
 * use AddSocket to add more later.
 */
Audience::Audience( ext_Settings *inSettings, int insocket ) : 
Notify( 0 ), Thread() {
    Server* theServer = new Server( inSettings, insocket, (Notify*)this );
    theServer->DeleteSelfAfterRun();
    theServer->Start();
    mThreads++;
    mSettings = inSettings;
    mStartWD = false;
}

/*
 * If we need to start a Speaker instance when we are
 * done then do it.
 */
Audience::~Audience() {
    if ( mStartWD ) {
        mThreadWD->Start();
    }
}

/*
 * Remain in scope as long as there are Servers reporting
 * to you.
 */
void Audience::Run() {

    mcond.Lock();
    while ( !AllThreadsRunning() ) {
        mcond.Wait();
    }
    while ( !AllThreadsDone() ) {
        mcond.Wait();
    }
    mcond.Unlock();

}

/*
 * Add another Server to your Audience
 */
void Audience::AddSocket(int insocket) {
    Server* theServer = new Server( mSettings, insocket, (Notify*)this);
    theServer->DeleteSelfAfterRun();
    theServer->Start();
    mThreads++;
}

/*
 * Specify a Thread that we need to Start when
 * we are finished.
 */
void Audience::StartWhenDone( Thread *thread ) {
    mStartWD = true;
    mThreadWD = thread;
}
