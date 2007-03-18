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
 * Condition.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * An abstract class for waiting on a condition variable. If
 * threads are not available, this does nothing.
 * ------------------------------------------------------------------- */

#ifndef CONDITION_H
#define CONDITION_H

#include "headers.h"

#include "Mutex.hpp"

#include "util.h"

/* ------------------------------------------------------------------- */
class Condition : public Mutex
{
  public:
    // initialize condition
    Condition( void )
      : Mutex()
    {
#if   defined( HAVE_POSIX_THREAD )
      pthread_cond_init( &mCondition, NULL );
#elif defined( HAVE_WIN32_THREAD )
      // set all conditions to be broadcast
      // unfortunately in Win32 you have to know at creation
      // whether the signal is broadcast or not.
      mCondition = CreateEvent( NULL, true, false, NULL );
#endif
    }

    // make sure condition is unlocked TODO
    ~Condition()
    {
#if   defined( HAVE_POSIX_THREAD )
      pthread_cond_destroy( &mCondition );
#elif defined( HAVE_WIN32_THREAD )
      CloseHandle( mCondition );
#endif
    }

    // sleep this thread, waiting for condition signal
    void Wait( void )
    {
#if   defined( HAVE_POSIX_THREAD )
      pthread_cond_wait( &mCondition, &mMutex );
#elif defined( HAVE_WIN32_THREAD )
      // atomically release mutex and wait on condition,
      // then re-acquire the mutex
      SignalObjectAndWait( mMutex, mCondition, INFINITE, false );
      WaitForSingleObject( mMutex, INFINITE );
#endif
    }

    // sleep this thread, waiting for condition signal,
    // but bound sleep time by the relative time inSeconds.
    void TimedWait( time_t inSeconds )
    {
#if   defined( HAVE_POSIX_THREAD )
      struct timespec absTimeout;
      absTimeout.tv_sec  = time( NULL ) + inSeconds;
      absTimeout.tv_nsec = 0;

      pthread_cond_timedwait( &mCondition, &mMutex, &absTimeout );
#elif defined( HAVE_WIN32_THREAD )
      // atomically release mutex and wait on condition,
      // then re-acquire the mutex
      SignalObjectAndWait( mMutex, mCondition, inSeconds*1000, false );
      WaitForSingleObject( mMutex, INFINITE );
#else
      // avoid some compiler warnings about unused args
      int secs;
      secs = inSeconds;
#endif
    }

    // send a condition signal to wake one thread waiting on condition
    // in Win32, this actually wakes up all threads, same as Broadcast
    // use PulseEvent to auto-reset the signal after waking all threads
    void Signal( void )
    {
#if   defined( HAVE_POSIX_THREAD )
      pthread_cond_signal( &mCondition );
#elif defined( HAVE_WIN32_THREAD )
      PulseEvent( mCondition );
#endif
    }

    // send a condition signal to wake all threads waiting on condition
    void Broadcast( void )
    {
#if   defined( HAVE_POSIX_THREAD )
      pthread_cond_broadcast( &mCondition );
#elif defined( HAVE_WIN32_THREAD )
      PulseEvent( mCondition );
#endif
    }

  protected:
#if   defined( HAVE_POSIX_THREAD )
    pthread_cond_t mCondition;
#elif defined( HAVE_WIN32_THREAD )
    HANDLE mCondition;
#endif

}; // end class Condition

#endif // CONDITION_H
