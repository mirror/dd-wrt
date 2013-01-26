/*===========================================================================
FILE:
   SyncQueue.h

DESCRIPTION:
   Declaration/Implementation of cSyncQueue class
   
PUBLIC CLASSES AND METHODS:
   cSyncQueue
      Synchronized shareable (across multiple threads) queue of
      structures with event notifications

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include <deque>
#include "Event.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cSyncQueue
/*=========================================================================*/
template <class tElementType> class cSyncQueue
{
   public:
      // (Inline) Constructor
      cSyncQueue(
         ULONG                      maxElements,
         bool                       bSignalEvent = false )
         :  mSignature( (ULONG)eSYNC_QUEUE_SIG ),
            mSignalEvent(),
            mbSignalEvent( bSignalEvent ),
            mMaxElements( maxElements ),
            mTotalElements( 0 )
      {
         // Create sync CS
         int nRet = pthread_mutex_init( &mSyncSection, NULL );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to init sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return;
         }
      };

      // (Inline) Destructor
      ~cSyncQueue()
      {
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Double deletion detected in ~cSyncQueue" == 0 );
         }
         else
         {
            EmptyQueue();

            mSignature = 0;
            int nRet = pthread_mutex_destroy( &mSyncSection );
            if (nRet != 0)
            {
               TRACE( "SyncQueue: Unable to destroy sync mutex."
                      " Error %d: %s\n",
                      nRet,
                      strerror( nRet ) );
               return;
            }
         }

      };

      // (Inline) Add an element to the queue
      bool AddElement( const tElementType & elem )
      {
         // Assume failure
         bool bRC = false;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return bRC;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         // Are we out of space?
         if ((ULONG)mElementDeque.size() >= mMaxElements)
         {
            // Yes, drop oldest element
            mElementDeque.pop_front();
         }

         // Add new item to the queue
         mElementDeque.push_back( elem );
         mTotalElements++;

         // Set event?
         if (mbSignalEvent == true)
         {
            // Signal index of event
            nRet = mSignalEvent.Set( mTotalElements - 1 );
            if (nRet != 0)
            {
               TRACE( "SyncQueue: Unable to signal. Error %d: %s\n",
                      nRet,
                      strerror( nRet ) );
               return false;
            }
         }

         // Success!
         bRC = true;

         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         return bRC;
      };

      // (Inline) Add an element to the queue returning the index of
      // the element
      bool AddElement( 
         const tElementType &       elem,
         ULONG &                    idx )
      {
         // Assume failure
         bool bRC = false;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return bRC;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         // Are we out of space?
         if ((ULONG)mElementDeque.size() >= mMaxElements)
         {
            mElementDeque.pop_front();
         }

         // Add new item to the queue
         mElementDeque.push_back( elem );
         idx = mTotalElements++;

         // Set event?
         if (mbSignalEvent == true)
         {
            // Signal index of event
            nRet = mSignalEvent.Set( mTotalElements - 1 );
            if (nRet != 0)
            {
               TRACE( "SyncQueue: Unable to signal. Error %d: %s\n",
                      nRet,
                      strerror( nRet ) );
               return false;
            }
         }

         // Success!
         bRC = true;

         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         return bRC;
      };

      // (Inline) Return given element in the queue
      bool GetElement(
         ULONG                      idx,
         tElementType &             elem ) const
      {
         // Assume failure
         bool bRC = false;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return bRC;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         // Is this a current element index?
         ULONG expiredIndices = mTotalElements - (ULONG)mElementDeque.size();
         if (idx >= expiredIndices)
         {
            // Yes, grab it from the deque
            idx -= expiredIndices;
            if (idx < (ULONG)mElementDeque.size())
            {
               elem = mElementDeque[idx];
               bRC = true;
            }
         }   
     
         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         return bRC;
      };

      // (Inline) Empty element queue
      bool EmptyQueue()
      {
         // Assume failure
         bool bRC = false;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return bRC;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }


         mElementDeque.clear();
         mTotalElements = 0;
         
         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return false;
         }

         bRC = true;
         return bRC;
      };

      // (Inline) Return the number of queued elements
      ULONG GetQueueCount() const
      {
         ULONG elems = 0;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return elems;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return 0;
         }
         
         elems = (ULONG)mElementDeque.size();
         
         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return 0;
         }

         return elems;
      };

      // (Inline) Return the total number of elements added to queue
      ULONG GetTotalCount() const
      {
         ULONG elems = 0;
         if (IsValid() == false)
         {
            ASSERT( (PVOID)"Bad cSyncQueue object detected" == 0 );
            return elems;
         }

         int nRet = pthread_mutex_lock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to lock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return 0;
         }
         
         elems = mTotalElements;
         
         nRet = pthread_mutex_unlock( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SyncQueue: Unable to unlock sync mutex. Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return 0;
         }

         return elems;
      };

      // (Inline) Return the signal event
      cEvent & GetSignalEvent() const
      {
         return mSignalEvent;
      };

      // (Inline) Is this sync queue valid?
      bool IsValid() const
      {
         return (mSignature == (ULONG)eSYNC_QUEUE_SIG);
      };

   protected:
      // Object signature
      enum eClassConstants
      {
         eSYNC_QUEUE_SIG = 0x1799A2BC
      };

      /* Object signature */
      ULONG mSignature;

      /* Multithreaded mutex type */
      mutable pthread_mutex_t mSyncSection;

      /* Signal event, set everytime an element is added (if configured) */
      mutable cEvent mSignalEvent;
      
      /* Use above signal event? */
      bool mbSignalEvent;

      /* Maximum number of elements to add to the deque */
      ULONG mMaxElements;

      /* Total number of elements added to the deque */
      ULONG mTotalElements;

     /* Element queue */
      std::deque <tElementType> mElementDeque;
};
