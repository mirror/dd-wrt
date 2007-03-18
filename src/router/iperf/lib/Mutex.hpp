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
 * Mutex.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * An abstract class for locking a mutex (mutual exclusion). If
 * threads are not available, this does nothing.
 * ------------------------------------------------------------------- */
#ifndef MUTEX_H
#define MUTEX_H

#include "headers.h"

/* ------------------------------------------------------------------- */
class Mutex {
public:
    // initialize mutex
    Mutex( void ) {
#if   defined( HAVE_POSIX_THREAD )
        pthread_mutex_init( &mMutex, NULL );
#elif defined( HAVE_WIN32_THREAD )
        mMutex = CreateMutex( NULL, false, NULL );
#endif
    }

    // destroy, making sure mutex is unlocked
    ~Mutex() {
#if   defined( HAVE_POSIX_THREAD )
        int rc = pthread_mutex_destroy( &mMutex );
        if ( rc == EBUSY ) {
            Unlock();
            pthread_mutex_destroy( &mMutex );
        }
#elif defined( HAVE_WIN32_THREAD )
        CloseHandle( mMutex );
#endif
    }

    // lock the mutex variable
    void Lock( void ) {
#if   defined( HAVE_POSIX_THREAD )
        pthread_mutex_lock( &mMutex );
#elif defined( HAVE_WIN32_THREAD )
        WaitForSingleObject( mMutex, INFINITE );
#endif
    }

    // unlock the mutex variable
    void Unlock( void ) {
#if   defined( HAVE_POSIX_THREAD )
        pthread_mutex_unlock( &mMutex );
#elif defined( HAVE_WIN32_THREAD )
        ReleaseMutex( mMutex );
#endif
    }

protected:
#if   defined( HAVE_POSIX_THREAD )
    pthread_mutex_t mMutex;
#elif defined( HAVE_WIN32_THREAD )
    HANDLE mMutex;
#endif

}; // end class Mutex

#endif // MUTEX_H
