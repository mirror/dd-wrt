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
 * Speaker.cpp
 * by Kevin Gibbs <kgibbs@ncsa.uiuc.edu>
 * -------------------------------------------------------------------
 * Speaker is simply a wrapper for the client class like listener is
 * a wrapper of the server class. This is useful in the bidirectional
 * testing model as a server will need to make out going connections and
 * a client will need to open ports to accept connections from remote
 * servers.
 * ------------------------------------------------------------------- */

#include "Speaker.hpp"
#include "Client.hpp"
#include "Listener.hpp"

extern void sig_quit( int inSigno );

/* Constructor */ 
Speaker::Speaker( ext_Settings *inSettings ) 
: Notify(inSettings->mThreads) {
    mSettings = inSettings;
}

/* DeConstructor */
Speaker::~Speaker() {
}

void  Speaker::Run() {
    // If we need to start up a listener do it now
    ext_Settings *temp = NULL;
    Settings::GenerateListenerSettings( mSettings, &temp );
    if ( temp != NULL ) {
        Listener* theListener = new Listener( temp );
        theListener->DeleteSelfAfterRun();
        theListener->Start();
    }

    // Create clients
    Client* theClient =
    new Client( mSettings,
                true,
                (Notify*) this);
    theClient->InitiateServer();
    theClient->DeleteSelfAfterRun();
    theClient->Start();


    for ( int i = 1; i < mThreads; i++ ) {
        // start up a client
        Client* theClient =
        new Client( mSettings,
                    false,
                    (Notify*) this);
        theClient->InitiateServer();
        theClient->DeleteSelfAfterRun();
        theClient->Start();
    }

    // Stick around until all the Threads are done
    mcond.Lock();
    while ( !AllThreadsRunning() ) {
        mcond.Wait();
    }
    while ( !AllThreadsDone() ) {
        mcond.Wait();
    }
    mcond.Unlock();

}

