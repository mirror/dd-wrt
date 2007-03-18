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
 * Queue.cpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * Queue handles storing a FIFO queue of pointers to arbitrary objects
 * -------------------------------------------------------------------
 * headers
 * uses
 *   <stdlib.h>
 *   <assert.h>
 * ------------------------------------------------------------------- */


#define HEADERS()

#include "headers.h"

#include "Queue.hpp"

#include "util.h"

/* -------------------------------------------------------------------
 * allocate memory for inSize elements (pointers and lengths)
 * and initialize queue to empty
 * ------------------------------------------------------------------- */

Queue::Queue( int inSize ) {
    Lock();

    // initialize data queue
    mSize = inSize;
    mQ    = new void*[ mSize ];
    mHead = 0;
    mTail = 0;

    Unlock();
} // end Queue

/* -------------------------------------------------------------------
 * delete associated memory
 * ------------------------------------------------------------------- */

Queue::~Queue() {
    DELETE_ARRAY( mQ );
} // end ~Queue

/* -------------------------------------------------------------------
 * Place the given item at the end of the queue. 
 * Returns true if successful, false otherwise
 * ------------------------------------------------------------------- */
bool Queue::Enqueue( void* inItem ) {
    bool result = false;

    if ( ! IsFull() ) {
        // store at end of queue
        mQ[ mTail ] = inItem;
        mTail++;

        // wrap around if needed
        if ( mTail == mSize ) {
            mTail = 0;
        }
        result = true;
    }

    return result;
} // end Enqueue

/* -------------------------------------------------------------------
 * Remove and return the buffer at the beginning of the queue.
 * Returns NULL if the queue is empty
 * ------------------------------------------------------------------- */
void* Queue::Dequeue( void ) {
    void* item = NULL;

    if ( ! IsEmpty() ) {
        // fetch front of queue
        item = mQ[ mHead ];
        mHead++;

        // wrap around if needed
        if ( mHead == mSize ) {
            mHead = 0;
        }
    }

    return item;
} // end Dequeue

/* -------------------------------------------------------------------
 * return the buffer at the beginning of the queue.
 * Returns NULL if the queue is empty
 * This does NOT LOCK the queue.
 * ------------------------------------------------------------------- */
void* Queue::Front( void ) {
    void* item = NULL;

    if ( ! IsEmpty() ) {
        // fetch front of queue
        item = mQ[ mHead ];
    }

    return item;
} // end Front

/* -------------------------------------------------------------------
 * Sleep thread until the some change happens in the queue,
 * or our timeout expires. If timeout is zero, sleep indefinitely.
 * ------------------------------------------------------------------- */

void Queue::BlockUntilSignal( time_t inAbsTimeout ) {
    if ( inAbsTimeout > 0 ) {
        mQ_cond.TimedWait( inAbsTimeout );
    } else {
        mQ_cond.Wait();
    }
} // end BlockUntilNonempty

/* -------------------------------------------------------------------
 * return true if the queue is full
 * ------------------------------------------------------------------- */

bool Queue::IsFull( void ) {
    int nexttail = mTail+1;
    if ( nexttail == mSize ) {
        nexttail = 0;
    }
    return(nexttail == mHead);
} // end IsFull
