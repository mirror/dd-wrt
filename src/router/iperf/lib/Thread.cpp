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
 * Thread.cpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * An abstract class for starting and running pthreads. Define the
 * Run() function as the main loop for the thread. If threads are
 * not available (HAVE_THREAD is undefined), Start calls Run in a
 * single thread, and returns only after Run returns.
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <stdio.h>
 *   <assert.h>
 *   <errno.h>
 * Thread.hpp may include <pthread.h>
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"

#include "Thread.hpp"

#include "util.h"

/* -------------------------------------------------------------------
 * define static variables.
 * ------------------------------------------------------------------- */

int Thread::sNum = 0;
Condition Thread::sNum_cond;

/* -------------------------------------------------------------------
 * Initialize thread object. Do not start the object's thread execution.
 * ------------------------------------------------------------------- */

Thread::Thread( void ) {
    mTID = ZeroID();
    mDeleteSelf = false;
} // end Thread

/* -------------------------------------------------------------------
 * Stop this object's thread execution (if any) immediately.
 * ------------------------------------------------------------------- */

Thread::~Thread() {
    Stop();
#if defined( HAVE_WIN32_THREAD )
    CloseHandle( mHandle );
#endif
} // end ~Thread

/* -------------------------------------------------------------------
 * Start the object's thread execution. Increments thread
 * count, spawns new thread, and stores thread ID.
 * ------------------------------------------------------------------- */

void Thread::Start( void ) {
    if ( EqualID( mTID, ZeroID() ) ) {

        // increment thread count
        sNum_cond.Lock();
        sNum++;
        sNum_cond.Unlock();

        Thread* ptr = this;

#if   defined( HAVE_POSIX_THREAD )

        // pthreads -- spawn new thread
        int err = pthread_create( &mTID, NULL, Run_Wrapper, ptr );
        FAIL( err != 0, "pthread_create" );
#elif defined( HAVE_WIN32_THREAD )

        // Win32 threads -- spawn new thread
        // Win32 has a thread handle in addition to the thread ID
        mHandle = CreateThread( NULL, 0, Run_Wrapper, ptr, 0, &mTID );
        FAIL_errno( mHandle == NULL, "CreateThread" );
#else

        // single-threaded -- call Run_Wrapper in this thread
        Run_Wrapper( ptr );
#endif
    }
} // end Start

/* -------------------------------------------------------------------
 * Stop the thread immediately. Decrements thread count and
 * resets the thread ID.
 * ------------------------------------------------------------------- */

void Thread::Stop( void ) {
    if ( ! EqualID( mTID, ZeroID() ) ) {
        // decrement thread count
        sNum_cond.Lock();
        sNum--;
        sNum_cond.Signal();
        sNum_cond.Unlock();

#ifdef HAVE_THREAD
        nthread_t oldTID = mTID;
        mTID = ZeroID();

        // exit thread
#if   defined( HAVE_POSIX_THREAD )

        // use exit()   if called from within this thread
        // use cancel() if called from a different thread
        if ( EqualID( pthread_self(), oldTID ) ) {
            pthread_exit( NULL );
        } else {
            // Cray J90 doesn't have pthread_cancel; Iperf works okay without
#ifdef HAVE_PTHREAD_CANCEL
            pthread_cancel( oldTID );
#endif
        }
#elif defined( HAVE_WIN32_THREAD )
        if ( EqualID( GetID(), oldTID ) ) {
            ExitThread( 0 );
        } else {
            // this is a somewhat dangerous function; it's not
            // suggested to Stop() threads a lot.
            TerminateThread( mHandle, 0 );
        }
#endif
#endif
    }
} // end Stop

/* -------------------------------------------------------------------
 * Low level function which starts a new thread, called by
 * Start(). The argument should be a pointer to a Thread object.
 * Calls the virtual Run() function for that object.
 * Upon completing, decrements thread count and resets thread ID.
 * If the object is deallocated immediately after calling Start(),
 * such as an object created on the stack that has since gone
 * out-of-scope, this will obviously fail.
 * [static]
 * ------------------------------------------------------------------- */

#if   defined( HAVE_WIN32_THREAD )
DWORD WINAPI
#else
void*
#endif
Thread::Run_Wrapper( void* paramPtr ) {
    assert( paramPtr != NULL );

    Thread* objectPtr = (Thread*) paramPtr;

    // run (pure virtual function)
    objectPtr->Run();

#ifdef HAVE_POSIX_THREAD
    // detach Thread. If someone already joined it will not do anything
    // If noone has then it will free resources upon return from this
    // function (Run_Wrapper)
    pthread_detach(objectPtr->mTID);
#endif

    // set TID to zero, then delete it
    // the zero TID causes Stop() in the destructor not to do anything
    objectPtr->mTID = ZeroID();

    if ( objectPtr->mDeleteSelf ) {
        DELETE_PTR( objectPtr );
    }

    // decrement thread count and send condition signal
    // do this after the object is destroyed, otherwise NT complains
    sNum_cond.Lock();
    sNum--;
    sNum_cond.Signal();
    sNum_cond.Unlock();

    return NULL;
} // end run_wrapper

/* -------------------------------------------------------------------
 * Wait for this object's thread execution (if any) to complete.
 * ------------------------------------------------------------------- */

void Thread::Join( void ) {
    if ( ! EqualID( mTID, ZeroID()) ) {
#if   defined( HAVE_POSIX_THREAD )
        pthread_join( mTID, NULL );
#elif defined( HAVE_WIN32_THREAD )
        WaitForSingleObject( mHandle, INFINITE );
#endif
    }
} // end Join

/* -------------------------------------------------------------------
 * Wait for all thread object's execution to complete. Depends on the
 * thread count being accurate and the threads sending a condition
 * signal when they terminate.
 * [static]
 * ------------------------------------------------------------------- */

void Thread::Joinall( void ) {
    sNum_cond.Lock();
    while ( sNum > 0 ) {
        sNum_cond.Wait();
    }
    sNum_cond.Unlock();
} // end Joinall


/* -------------------------------------------------------------------
 * Return the current thread's ID.
 * [static]
 * ------------------------------------------------------------------- */

nthread_t Thread::GetID( void ) {
#if   defined( HAVE_POSIX_THREAD )
    return pthread_self();
#elif defined( HAVE_WIN32_THREAD )
    return GetCurrentThreadId();
#else
    return 0;
#endif
}

/* -------------------------------------------------------------------
 * Compare the thread ID's (inLeft == inRight); return true if they
 * are equal. On some OS's nthread_t is a struct so == will not work.
 * TODO use pthread_equal. Any Win32 equivalent??
 * [static]
 * ------------------------------------------------------------------- */

bool Thread::EqualID( nthread_t inLeft, nthread_t inRight ) {
    return(memcmp( &inLeft, &inRight, sizeof(inLeft)) == 0);
}

/* -------------------------------------------------------------------
 * Return a zero'd out thread ID. On some OS's nthread_t is a struct
 * so == 0 will not work.
 * [static]
 * ------------------------------------------------------------------- */

nthread_t Thread::ZeroID( void ) {
    nthread_t a;
    memset( &a, 0, sizeof(a));
    return a;
}

/* -------------------------------------------------------------------
 * set a thread to be daemon, so joinall won't wait on it
 * this simply decrements the thread count that joinall uses,
 * which is not a thorough solution, but works for the moment
 * ------------------------------------------------------------------- */

void Thread::SetDaemon( void ) {
    sNum_cond.Lock();
    sNum--;
    sNum_cond.Signal();
    sNum_cond.Unlock();
}
