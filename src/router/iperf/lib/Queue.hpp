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
 * Queue.hpp
 * by Mark Gates <mgates@nlanr.net>
 * Alex Warshavsky <alexw@nlanr.net>
 * -------------------------------------------------------------------
 * Queue handles storing a FIFO queue of pointers to arbitrary objects
 * ------------------------------------------------------------------- */

#ifndef QUEUE_H
#define QUEUE_H

#include "Thread.hpp"
#include "Condition.hpp"

/* ------------------------------------------------------------------- */
class Queue {
public:
    // create a queue with space for inSize pointers
    Queue( int inSize );
    ~Queue();

    // store to and retreive buffers from the FIFO queue
    // true if operation was successful
    bool  Enqueue( void* inItem );
    void* Dequeue( void );
    void* Front( void );

    // number of actual items in the queue
    int Size( void ) {
        return((mTail - mHead) % mSize);
    } // end Size

    // true if queue is empty
    bool IsEmpty( void ) {
        return(mHead == mTail);
    }

    // true if queue is full
    bool IsFull( void );

    // sleeps thread until something changes in the queue
    // you -must- lock the queue before this, and unlock afterwards
    void BlockUntilSignal( time_t inAbsTimeout = 0 );

    // provide locking for thread safety
    void Lock( void ) {
        mQ_cond.Lock();
    }

    // provide locking for thread safety
    // also sends signal to wake BlockUntilSignal function
    void Unlock( void ) {
        mQ_cond.Signal();
        mQ_cond.Unlock();
    }

protected:

    // ---------- member data
    Condition mQ_cond;

    void **mQ;
    int  mHead;
    int  mTail;
    int  mSize;

}; // end class Queue

#endif // QUEUE_H
