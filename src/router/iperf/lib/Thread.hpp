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
 * Thread.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * An abstract class for starting and running pthreads. Define the
 * Run() function as the main loop for the thread. If threads are
 * not available (HAVE_THREAD is undefined), Start calls Run in a
 * single thread, and returns only after Run returns.
 * ------------------------------------------------------------------- */

#ifndef THREAD_H
#define THREAD_H



#if   defined( HAVE_POSIX_THREAD )

/* Definitions for Posix Threads (pthreads) */
    #include <pthread.h>

typedef pthread_t nthread_t;

typedef void* Thread_func( void* );
typedef Thread_func *Thread_func_ptr;

    #define HAVE_THREAD 1

#elif defined( HAVE_WIN32_THREAD )

/* Definitions for Win32 NT Threads */
typedef DWORD nthread_t;
typedef LPTHREAD_START_ROUTINE Thread_func_ptr;

    #define HAVE_THREAD 1

#else

/* Definitions for no threads */
typedef int nthread_t;
typedef void* Thread_func( void* );
typedef Thread_func *Thread_func_ptr;

    #undef HAVE_THREAD

#endif

#include "Mutex.hpp"
#include "Condition.hpp"

/* ------------------------------------------------------------------- */
class Thread {
public:
    Thread( void );
    virtual ~Thread();

    // start or stop a thread executing
    void Start( void );
    void Stop( void );

    // run is the main loop for this thread
    // usually this is called by Start(), but may be called
    // directly for single-threaded applications.
    virtual void Run( void ) = 0;

    // wait for this or all threads to complete
    void Join( void );
    static void Joinall( void );

    void DeleteSelfAfterRun( void ) {
        mDeleteSelf = true;
    }

    // set a thread to be daemon, so joinall won't wait on it
    void SetDaemon( void );

    // returns the number of user (i.e. not daemon) threads
    static int NumUserThreads( void ) {
        return sNum;
    }

    static nthread_t GetID( void );

    static bool EqualID( nthread_t inLeft, nthread_t inRight );

    static nthread_t ZeroID( void );

protected:
    nthread_t mTID;
    bool mDeleteSelf;

#if defined( HAVE_WIN32_THREAD )
    HANDLE mHandle;
#endif

    // count of threads; used in joinall
    static int sNum;
    static Condition sNum_cond;

private:
    // low level function which calls Run() for the object
    // this must be static in order to work with pthread_create
#if   defined( HAVE_WIN32_THREAD )
    static DWORD WINAPI Run_Wrapper( void* paramPtr );
#else
    static void*        Run_Wrapper( void* paramPtr );
#endif

}; // end class Thread

#endif // THREAD_H
