/*===========================================================================
FILE:
   ProtocolServer.cpp

DESCRIPTION:
   Generic protocol packet server
   
PUBLIC CLASSES AND METHODS:
   cProtocolServer
      Abstract base class for protocol servers

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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"

#include "ProtocolServer.h"
#include "ProtocolNotification.h"

#include <climits>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Invalid request ID
const ULONG INVALID_REQUEST_ID = 0;

// Default activity timeout value
const ULONG DEFAULT_WAIT = 100;

// MTU (Maximum Transmission Unit) for auxiliary data (QC USB imposed)
const ULONG MAX_AUX_MTU_SIZE = 1024 * 256;

// USB's MaxPacketSize
const ULONG MAX_PACKET_SIZE = 512;

// Maximum amount of time to wait on external access synchronization object
#ifdef DEBUG
   // For the sake of debugging do not be so quick to assume failure
   const ULONG DEADLOCK_TIME = 180000;
#else
   const ULONG DEADLOCK_TIME = 10000;
#endif

// Maximum amount of time to wait for the protocol server to process a command
const ULONG COMMAND_TIME = DEADLOCK_TIME;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   ScheduleThread (Free Method)
   
DESCRIPTION:
   Watch schedule for event to process or timeout

PARAMETERS:
   pArg        [ I ] - The protocol server object

RETURN VALUE:
   void * - thread exit value (always NULL)
===========================================================================*/
void * ScheduleThread( PVOID pArg )
{  
   // Do we have a server?
   cProtocolServer * pServer = (cProtocolServer *)pArg;
   if (pServer == 0)
   {
      TRACE( "ScheduleThread started with empty pArg."
             "  Unable to locate cProtocolServer\n" );
      
      ASSERT( 0 );
      return NULL;
   }
   
   TRACE( "Schedule thread [%lu] started\n", 
          pthread_self() );
   
   // Default wait event
   timespec toTime = TimeIn( DEFAULT_WAIT );

   // Return value checking
   int nRet;
   
   while (pServer->mbExiting == false)
   {
      DWORD nTemp; 
      nRet = pServer->mThreadScheduleEvent.Wait( TimeFromNow( toTime ), nTemp );
      if (nRet != 0 && nRet != ETIME)
      {
         // Error condition
         TRACE( "ScheduleThread [%lu] ScheduleThread wait error %d, %s\n",
                pthread_self(),
                nRet,
                strerror( nRet ) );
         break;
      }

      // Time to exit?
      if (pServer->mbExiting == true)
      {
         break;
      }

      // Get Schedule Mutex (non-blocking)
      nRet = pthread_mutex_trylock( &pServer->mScheduleMutex );
      if (nRet == EBUSY)
      {
         // Not an error, we're just too slow
         // Someone else got to the ScheduleMutex before us
         // We'll wait for the signal again
         toTime = TimeIn( DEFAULT_WAIT );
         TRACE( "ScheduleThread [%lu] unable to lock ScheduleMutex\n", 
                pthread_self() );
         continue;
      }
      else if (nRet != 0)
      {
         // Error condition
         TRACE( "ScheduleThread [%lu] mScheduleMutex error %d, %s\n",
                pthread_self(),
                nRet,
                strerror( nRet ) );
         break;
      }

      // Verify time.  In the rare event it does move backward
      // it would simply place all our schedule items as due now
      pServer->CheckSystemTime();
      
      // Default next wait period
      toTime = TimeIn( DEFAULT_WAIT );

      timespec curTime = TimeIn( 0 );

      if (pServer->mpActiveRequest != 0)
      {
         if (pServer->mpActiveRequest->mbWaitingForResponse == true)
         {
            // Waiting on a response, this takes priority over the next
            //    scheduled event
            
            // Has timeout expired?
            if (pServer->mActiveRequestTimeout <= curTime)
            {
               // Response timeout
   
               // Note: This may clear mpActiveRequest
               pServer->RxTimeout();
            }
            else
            {
               // Active response timer is not yet due to expire
               // Default timeout again, or this response's timeout?
               if (pServer->mActiveRequestTimeout <= toTime)
               {
                  toTime = pServer->mActiveRequestTimeout;
               }
            }
         }
         else
         {
            // This should never happen

            TRACE( "ScheduleThread() Sequencing error: "
                   "Active request %lu is not waiting for response ???\n",
                   pServer->mpActiveRequest->mID );

            break;
         }
      }

      if (pServer->mpActiveRequest == 0 
          && pServer->mRequestSchedule.size() > 0)
      {
         // No response timer active, ready to start the next 
         //    scheduled item if due
         
         timespec scheduledItem = pServer->GetNextRequestTime();
         
         // Is item due to be scheduled?
         if (scheduledItem <= curTime)
         {
            // Process scheduled item
            pServer->ProcessRequest();
         }
         else
         {
            // Scheduled item is not yet due to be processed
            // Default timeout again, or this item's start time?
            if (scheduledItem <= toTime)
            {
               toTime = scheduledItem;
            }
         }
      }
      
      /*TRACE( "Updated timer at %llu waiting %lu\n", 
             GetTickCount(), 
             TimeFromNow( toTime ) );  */
      
      // Unlock schedule mutex        
      nRet = pthread_mutex_unlock( &pServer->mScheduleMutex );
      if (nRet != 0)
      {
         TRACE( "ScheduleThread Unable to unlock schedule mutex."
                " Error %d: %s\n",
                nRet,
                strerror( nRet ) );
         return false;
      }
   }

   TRACE( "Schedule thread [%lu] exited\n", 
          pthread_self() );

   return NULL;
}

/*===========================================================================
METHOD:
   TimeIn (Free Method)
   
DESCRIPTION:
   Fill timespec with the time it will be in specified milliseconds
   Relative time to Absolute time

PARAMETERS:
   millis   [ I ] - Milliseconds from current time

RETURN VALUE:
   timespec - resulting time (from epoc)
     NOTE: tv_sec of 0 is an error
===========================================================================*/
timespec TimeIn( ULONG millis )
{
   timespec outTime;

   int nRC = clock_gettime( CLOCK_REALTIME, &outTime );
   if (nRC == 0)
   {
      // Add avoiding an overflow on (long)nsec
      outTime.tv_sec += millis / 1000l;
      outTime.tv_nsec += ( millis % 1000l ) * 1000000l;

      // Check if we need to carry
      if (outTime.tv_nsec >= 1000000000l)
      {
         outTime.tv_sec += outTime.tv_nsec / 1000000000l;
         outTime.tv_nsec = outTime.tv_nsec % 1000000000l;
      }
   }
   else
   {
      outTime.tv_sec = 0;
      outTime.tv_nsec = 0;
   }

   return outTime;
}

/*===========================================================================
METHOD:
   TimeFromNow (Free Method)
   
DESCRIPTION:
   Find the milliseconds from current time this timespec will occur
   Absolute time to Relative time

PARAMETERS:
   time   [ I ] - Absolute time

RETURN VALUE:
   Milliseconds in which absolute time will occur
     0 if time has passed or error has occured
===========================================================================*/
ULONG TimeFromNow( timespec time )
{
   // Assume failure
   ULONG nOutTime = 0;

   timespec now;
   int nRC = clock_gettime( CLOCK_REALTIME, &now );
   if (nRC == -1)
   {
      TRACE( "Error %d with gettime, %s\n", errno, strerror( errno ) );
      return nOutTime;
   }

   if (time <= now)
   {
      return nOutTime;
   }
   
   nOutTime = (time.tv_sec - now.tv_sec) * 1000l;
   nOutTime += (time.tv_nsec - now.tv_nsec) / 1000000l;

   return nOutTime;
}

/*===========================================================================
METHOD:
   GetTickCount (Free Method)
   
DESCRIPTION:
   Provide a number for sequencing reference, similar to the windows
   ::GetTickCount().  
   
   NOTE: This number is based on the time since epoc, not 
   uptime.

PARAMETERS:

RETURN VALUE:
   ULONGLONG - Number of milliseconds system has been up
===========================================================================*/
ULONGLONG GetTickCount()
{
   timespec curtime = TimeIn( 0 );

   ULONGLONG outtime = curtime.tv_sec * 1000LL;
   outtime += curtime.tv_nsec / 1000000LL;
   
   return outtime;
}

/*=========================================================================*/
// cProtocolServerRxCallback Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   IOComplete (Free Method)
   
DESCRIPTION:
   The I/O has been completed, process the results

PARAMETERS:
   status         [ I ] - Status of operation
   bytesReceived  [ I ] - Bytes received during operation

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServerRxCallback::IOComplete(
   DWORD                      status,
   DWORD                      bytesReceived )
{
   if (mpServer != 0)
   {
      mpServer->RxComplete( status, bytesReceived );
   }
}

/*=========================================================================*/
// cProtocolServer::sProtocolReqRsp Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cProtocolServer::sProtocolReqRsp (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   requestInfo [ I ] - Underlying request object
   requestID   [ I ] - Request ID
   auxDataMTU  [ I ] - MTU (Maximum Transmission Unit) for auxiliary data

RETURN VALUE:
   None
===========================================================================*/
cProtocolServer::sProtocolReqRsp::sProtocolReqRsp(
   const sProtocolRequest &   requestInfo,
   ULONG                      requestID,
   ULONG                      auxDataMTU )
   :  mRequest( requestInfo ),
      mID( requestID ),
      mAttempts( 0 ),
      mEncodedSize( requestInfo.GetSize() ),
      mRequiredAuxTxs( 0 ),
      mCurrentAuxTx( 0 ),
      mbWaitingForResponse( false )
{
   ULONG auxDataSz = 0;
   const BYTE * pAuxData = requestInfo.GetAuxiliaryData( auxDataSz );

   // Compute the number of required auxiliary data transmissions?
   if (auxDataMTU > 0 && pAuxData != 0 && auxDataSz > 0)
   {
      mRequiredAuxTxs = 1;
      if (auxDataSz > auxDataMTU)
      {
         mRequiredAuxTxs = auxDataSz / auxDataMTU;
         if ((auxDataSz % auxDataMTU) != 0)
         {   
            mRequiredAuxTxs++;
         }
      }
   }
}

/*===========================================================================
METHOD:
   cProtocolServer::sProtocolReqRsp (Public Method)

DESCRIPTION:
   Coop constructor

PARAMETERS:
   reqRsp      [ I ] - Object being copied

RETURN VALUE:
   None
===========================================================================*/
cProtocolServer::sProtocolReqRsp::sProtocolReqRsp( 
   const sProtocolReqRsp &    reqRsp )
   :  mRequest( reqRsp.mRequest ),
      mID( reqRsp.mID ),
      mAttempts( reqRsp.mAttempts ),
      mEncodedSize( reqRsp.mEncodedSize ),
      mRequiredAuxTxs( reqRsp.mRequiredAuxTxs ),
      mCurrentAuxTx( reqRsp.mCurrentAuxTx ),
      mbWaitingForResponse( reqRsp.mbWaitingForResponse )
{
   // Nothing to do
};

/*=========================================================================*/
// cProtocolServer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cProtocolServer (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   rxType      [ I ] - Protocol type to assign to incoming data
   txType      [ I ] - Protocol type to verify for outgoing data
   bufferSzRx  [ I ] - Size of data buffer for incoming data
   logSz       [ I ] - Size of log (number of buffers)

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cProtocolServer::cProtocolServer( 
   eProtocolType              rxType,
   eProtocolType              txType,
   ULONG                      bufferSzRx,
   ULONG                      logSz )
   :  mComm(),
      mRxCallback(),
      mScheduleThreadID( 0 ),
      mThreadScheduleEvent(),
      mbExiting( false ),
      mpServerControl( 0 ),
      mLastRequestID( 1 ),
      mpActiveRequest( 0 ),
      mpRxBuffer( 0 ),
      mRxBufferSize( bufferSzRx ),
      mRxType( rxType ),
      mTxType( txType ),
      mLog( logSz )
{
   mLastTime = TimeIn( 0 );

   // Allocate receive buffer?
   if (mRxBufferSize > 0 && mComm.IsValid() == true)
   {
      mpRxBuffer = new BYTE[mRxBufferSize];
   }

   // Before continuing verify receive buffer was allocated
   if (mpRxBuffer != 0)
   {
      // Schedule mutex
      int nRet = pthread_mutex_init( &mScheduleMutex, NULL );
      if (nRet != 0)
      {
         TRACE( "Unable to init schedule mutex. Error %d: %s\n",
                nRet,
                strerror( nRet ) );
         return;
      }
   }
}

/*===========================================================================
METHOD:
   ~cProtocolServer (Public Method)

DESCRIPTION:
   Destructor

SEQUENCING:
   None (destroys sequencing objects)
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolServer::~cProtocolServer() 
{
   // This should have already been called, but ...
   Exit();

   // Schedule mutex
   int nRet = pthread_mutex_destroy( &mScheduleMutex );
   if (nRet != 0)
   {
      TRACE( "Unable to destroy schedule mutex. Error %d: %s\n",
             nRet,
             strerror( nRet ) );
   }
   
   // Free receive buffer
   if (mpRxBuffer != 0)
   {
      delete [] mpRxBuffer;
      mpRxBuffer = 0;
   }
}

/*===========================================================================
METHOD:
   HandleRemoveRequest (Public Method)

DESCRIPTION:
   Remove a previously added protocol request 
   
   Note: if a request is being processed, it cannot be inturrupted

PARAMETERS:
   reqID       [ I ] - Server assigned request ID

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::HandleRemoveRequest( ULONG reqID )
{
   // Assume failure
   bool bRC = false;

   // Find and erase request from request map
   std::map <ULONG, sProtocolReqRsp *>::iterator pReqIter;
   pReqIter = mRequestMap.find( reqID );

   if (pReqIter != mRequestMap.end())
   {
      sProtocolReqRsp * pReqRsp = pReqIter->second;
      if (pReqRsp != 0)
      {
         delete pReqRsp;
      }

      mRequestMap.erase( pReqIter );

      // Success!
      bRC = true;

      // Find and erase request from schedule
      bool bFound = false;
      int entryIndex = -1;

      std::set <tSchedule>::iterator pScheduleIter;
      pScheduleIter = mRequestSchedule.begin();
      
      while (pScheduleIter != mRequestSchedule.end())
      {
         entryIndex++;

         tSchedule entry = *pScheduleIter;
         if (entry.second == reqID)
         {
            bFound = true;
            mRequestSchedule.erase( pScheduleIter );
            break;
         }
         else
         {
            pScheduleIter++;
         }
      }

      // Note: schedule will be updated when mutex is unlocked/signaled
   }
   else if (mpActiveRequest != 0 && mpActiveRequest->mID == reqID)
   {
      const sProtocolRequest & req = mpActiveRequest->mRequest;
      const cProtocolNotification * pNotifier = req.GetNotifier();

      // Cancel the response timer (when active)
      if (mpActiveRequest->mbWaitingForResponse == true)
      {
         // Schedule will be updated when mutex is unlocked

         // Failure to receive response, notify client
         if (pNotifier != 0)
         {
            pNotifier->Notify( ePROTOCOL_EVT_RSP_ERR, 
                               (DWORD)reqID, 
                               ECANCELED );
         }
      }
      else
      {
         // This is the active request, cancel the underlying transmit
         // Note: Because ProcessRequest and RemoveRequest are both muxed
         //    with ScheduleMutex, it is impossible to for the write
         //    to actually be in progress when this code is reached.
         mComm.CancelTx();

         // Failure to send request, notify client
         if (pNotifier != 0)
         {
            pNotifier->Notify( ePROTOCOL_EVT_REQ_ERR, 
                               (DWORD)reqID, 
                               ECANCELED );
         }
      }

      // Now delete the request
      delete mpActiveRequest;
      mpActiveRequest = 0;

      // Success!
      bRC = true;
   }
   else
   {
      TRACE( "cProtocolServer::RemoveRequest( %lu ),"
             " invalid request ID\n",
             reqID );
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ScheduleRequest (Internal Method)

DESCRIPTION:
   Schedule a request for transmission

PARAMETERS:
   reqID       [ I ] - ID of the request being scheduled this ID must exist
                       in the internal request/schedule maps

   schedule    [ I ] - Value in milliseconds that indicates the approximate
                       time from now that the request is to be sent out, the
                       actual time that the request is sent will be greater 
                       than or equal to this value dependant on requests 
                       scheduled before the request in question and
                       standard server processing time

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::ScheduleRequest(
   ULONG                      reqID,
   ULONG                      schedule )
{
   // Assume failure
   bool bRC = false;
   
   // Schedule adjust is in milliseconds
   timespec schTimer = TimeIn( schedule );

   // Create the schedule entry
   tSchedule newEntry( schTimer, reqID );

   // Fit this request into the schedule (ordered by scheduled time)
   mRequestSchedule.insert( newEntry );

   // Note: timer will be updated when mScheduleMutex is unlocked
   
   return bRC;      
}

/*===========================================================================
METHOD:
   RescheduleActiveRequest (Internal Method)

DESCRIPTION:
   Reschedule (or cleanup) the active request

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServer::RescheduleActiveRequest()
{
   // Are there more attempts to be made?
   if (mpActiveRequest->mAttempts < mpActiveRequest->mRequest.GetRequests())
   {
      // Yes, first reset the request 
      mpActiveRequest->Reset();

      // Now add it back to the request map
      mRequestMap[mpActiveRequest->mID] = mpActiveRequest;

      TRACE( "RescheduleActiveRequest(): req %lu rescheduled\n", mpActiveRequest->mID );                       
      
      // Lastly reschedule the request
      ScheduleRequest( mpActiveRequest->mID, 
                       mpActiveRequest->mRequest.GetFrequency() );

   }
   else
   {
      TRACE( "RescheduleActiveRequest(): req %lu removed\n", mpActiveRequest->mID );

      // No, we are through with this request
      delete mpActiveRequest;
   }

   // There is no longer an active request
   mpActiveRequest = 0;
  
}

/*===========================================================================
METHOD:
   ProcessRequest (Internal Method)

DESCRIPTION:
   Process a single outgoing protocol request, this consists of removing
   the request ID from the head of the schedule, looking up the internal 
   request object in the request map, sending out the request, and setting
   up the response timer (if a response is required)

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
===========================================================================*/
void cProtocolServer::ProcessRequest()
{
   // Is there already an active request?
   if (mpActiveRequest != 0)
   {
      return;
   }
   
   // Grab request ID from the schedule
   std::set <tSchedule>::iterator pScheduleIter;
   pScheduleIter = mRequestSchedule.begin();

   // Did we find the request?
   if (pScheduleIter == mRequestSchedule.end())
   {
      // No
      return;
   }

   // Yes, grab the request ID
   ULONG reqID = pScheduleIter->second;

   // Remove from schedule
   mRequestSchedule.erase( pScheduleIter );

   // Look up the internal request object
   std::map <ULONG, sProtocolReqRsp *>::iterator pReqIter;
   pReqIter = mRequestMap.find( reqID );

   // Request not found around?
   if (pReqIter == mRequestMap.end() || pReqIter->second == 0)
   {
      // No
      return;
   }

   // Set this request as the active request
   mpActiveRequest = pReqIter->second;
   
   TRACE( "ProcessRequest(): req %lu started\n", mpActiveRequest->mID );

   // Remove request from pending request map
   mRequestMap.erase( pReqIter );

   // Extract the underlying request
   const sProtocolRequest & req = mpActiveRequest->mRequest;

   // Increment attempt count?
   if (req.GetRequests() != INFINITE_REQS)
   {
      // This request isn't an indefinite one, so keep track of each attempt
      mpActiveRequest->mAttempts++;
   }

   bool bTxSuccess = false;

   // Encode data for transmission?
   bool bEncoded = false;
   sSharedBuffer * pEncoded = 0;
   pEncoded = EncodeTxData( req.GetSharedBuffer(), bEncoded );
   if (bEncoded == false)
   {
      // Note: no longer asynchronus
      // Send the request data
      bTxSuccess = mComm.TxData( req.GetBuffer(), 
                                 req.GetSize() );
   }
   else if (bEncoded == true)
   {
      if (pEncoded != 0 && pEncoded->IsValid() == true)
      {
         // Note: no longer asynchronus
         // Send the request data
         mpActiveRequest->mEncodedSize = pEncoded->GetSize();
         bTxSuccess = mComm.TxData( pEncoded->GetBuffer(), 
                                    pEncoded->GetSize() );
      }
   }

   if (bTxSuccess == true)
   {
      TRACE( "ProcessRequest(): req %lu finished\n", mpActiveRequest->mID );
      TxComplete();
   }
   else
   {
      TxError();
      TRACE( "ProcessRequest(): req finished with a TxError\n" );
   }

   return;
}

/*===========================================================================
METHOD:
   CheckSystemTime (Internal Method)

DESCRIPTION:
   Check that system time hasn't moved backwards.  Since we use the system
   time for scheduling requests we need to periodically check that the
   user (or system itself) hasn't reset system time backwards, if it has
   then we reschedule everything to the current system time.  This disrupts
   the schedule but avoids stranding requests

   Updates mLastTime

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   bool: System time moved backwards?
===========================================================================*/
bool cProtocolServer::CheckSystemTime()
{
   // Assume that time is still marching forward
   bool bAdjust = false;   

   timespec curTime = TimeIn( 0 );

   if (curTime < mLastTime)
   {
      // Looks like the system clock has been adjusted to an earlier
      // value, go through the current schedule and adjust each timer
      // to reflect the adjustment.  This isn't an exact approach but
      // it prevents requests from being stranded which is our goal

      // Note: set iterators are constant.  This means we need to
      //  create a set with the new data, we can't modify this one
      
      std::set < tSchedule, std::less <tSchedule> > tempSchedule;
      
      std::set <tSchedule>::iterator pScheduleIter;
      pScheduleIter = mRequestSchedule.begin();

      while (pScheduleIter != mRequestSchedule.end())
      {
         tSchedule entry = *pScheduleIter;
         entry.first.tv_sec = curTime.tv_sec;
         entry.first.tv_nsec = curTime.tv_nsec;
         tempSchedule.insert( entry );

         pScheduleIter++;
      }
      
      mRequestSchedule = tempSchedule;
      
      // Update mActiveRequestTimeout
      if ( (mpActiveRequest != 0)
      && (mpActiveRequest->mbWaitingForResponse == true) )
      {
         // Restart active request's timeout
         ULONG mTimeout = mpActiveRequest->mRequest.GetTimeout();
         mActiveRequestTimeout = TimeIn( mTimeout );
      }         

      TRACE( "Time has moved backwards, schedule updated\n" );

      // Indicate the change
      bAdjust = true;
   }

   mLastTime.tv_sec = curTime.tv_sec;
   mLastTime.tv_nsec = curTime.tv_nsec;

   return bAdjust;
}

/*===========================================================================
METHOD:
   RxComplete (Internal Method)

DESCRIPTION:
   Handle completion of receive data operation

PARAMETERS:
   status         [ I ] - Status of operation
   bytesReceived  [ I ] - Number of bytes received

SEQUENCING:
   This method is sequenced according to the schedule mutex
   i.e. any other thread that needs to modify the schedule 
   will block until this method completes

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServer::RxComplete(
   DWORD                      status,
   DWORD                      bytesReceived )
{
   if (status != NO_ERROR)
   {
      TRACE( "cProtocolServer::RxComplete() = %lu\n", status );
   }

   // Error with the read
   if (status != NO_ERROR || bytesReceived == 0)
   {
      // Setup the next read
      mComm.RxData( mpRxBuffer, 
                    (ULONG)mRxBufferSize, 
                    (cIOCallback *)&mRxCallback );

      return;
   }

   // Get Schedule Mutex
   if (GetScheduleMutex() == false)
   {
      TRACE( "RxComplete(), unable to get schedule Mutex\n" );
      return;
   }

   TRACE( "RxComplete() - Entry at %llu\n", GetTickCount() );

   // Decode data
   bool bAbortTx = false;
   ULONG rspIdx = INVALID_LOG_INDEX;
   bool bRsp = DecodeRxData( bytesReceived, rspIdx, bAbortTx );

   // Is there an active request that needs to be aborted
   if (mpActiveRequest != 0 && bAbortTx == true)
   {
      // Yes, terminate the transmission and handle the error
      mComm.CancelTx();
      TxError();
   }
   // Is there an active request and a valid response?
   else if (mpActiveRequest != 0 && bRsp == true)
   {
      const sProtocolRequest & req = mpActiveRequest->mRequest;
      const cProtocolNotification * pNotifier = req.GetNotifier();

      // Notify client that response was received
      if (pNotifier != 0)
      {
         pNotifier->Notify( ePROTOCOL_EVT_RSP_RECV, 
                            (DWORD)mpActiveRequest->mID, 
                            (DWORD)rspIdx );
      }

      // Reschedule request as needed
      RescheduleActiveRequest();
   }
   
   // Setup the next read
   mComm.RxData( mpRxBuffer, 
                 (ULONG)mRxBufferSize, 
                 (cIOCallback *)&mRxCallback );

   TRACE( "RxComplete() - Exit at %llu\n", GetTickCount() );

   // Unlock schedule mutex        
   if (ReleaseScheduleMutex() == false)
   {
      // This should never happen
      return;
   }

   return;
}

/*===========================================================================
METHOD:
   RxTimeout (Internal Method)

DESCRIPTION:
   Handle the response timer expiring

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServer::RxTimeout()
{
   // No active request?
   if (mpActiveRequest == 0)
   {
      TRACE( "RxTimeout() with no active request\n" );
      ASSERT( 0 );
   }
   
   TRACE( "RxTimeout() for req %lu\n", mpActiveRequest->mID );

   const sProtocolRequest & req = mpActiveRequest->mRequest;
   const cProtocolNotification * pNotifier = req.GetNotifier();

   // Failure to receive response, notify client
   if (pNotifier != 0)
   {
      pNotifier->Notify( ePROTOCOL_EVT_RSP_ERR, 
                         (DWORD)mpActiveRequest->mID, 
                         (DWORD)0 );
   }

   // Reschedule request as needed
   RescheduleActiveRequest();
}

/*===========================================================================
METHOD:
   TxComplete (Internal Method)

DESCRIPTION:
   Handle completion of transmit data operation

PARAMETERS:

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServer::TxComplete()
{
   // No active request?
   if (mpActiveRequest == 0)
   {
      TRACE( "TxComplete() called with no active request\n" );
      ASSERT( 0 );
   }
   
   TRACE( "TxComplete() req %lu started\n", mpActiveRequest->mID );

   ULONG reqID = mpActiveRequest->mID;
   const sProtocolRequest & req = mpActiveRequest->mRequest;
   const cProtocolNotification * pNotifier = req.GetNotifier();

   // Notify client of auxiliary data being sent?
   if (mpActiveRequest->mRequiredAuxTxs && mpActiveRequest->mCurrentAuxTx)
   {
      pNotifier->Notify( ePROTOCOL_EVT_AUX_TU_SENT, 
                         (DWORD)reqID, 
                         (DWORD)mpActiveRequest->mEncodedSize );
   }

   // Check for more auxiliary data to transmit
   if (mpActiveRequest->mCurrentAuxTx < mpActiveRequest->mRequiredAuxTxs)
   {
      ULONG auxDataSz = 0;
      const BYTE * pAuxData = req.GetAuxiliaryData( auxDataSz );
      if (auxDataSz > 0 && pAuxData != 0)
      {
         bool bRC = false;

         // Adjust for current MTU
         pAuxData += (mpActiveRequest->mCurrentAuxTx * MAX_AUX_MTU_SIZE);
         mpActiveRequest->mCurrentAuxTx++;

         // Last MTU?
         if (mpActiveRequest->mCurrentAuxTx == mpActiveRequest->mRequiredAuxTxs)
         {
            // More than one MTU?
            if (mpActiveRequest->mRequiredAuxTxs > 1)
            {
               auxDataSz = (auxDataSz % MAX_AUX_MTU_SIZE);
               if (auxDataSz == 0)
               {
                  auxDataSz = MAX_AUX_MTU_SIZE;
               }
            }
            
            if (auxDataSz % MAX_PACKET_SIZE == 0)
            {
               // If last write of unframed write request is divisible
               //    by 512, break off last byte and send seperatly.
               TRACE( "TxComplete() Special case, break off last byte\n" );

               bRC = mComm.TxData( pAuxData, 
                                   auxDataSz - 1  );

               if (bRC == true)
               {
                  bRC = mComm.TxData( pAuxData + auxDataSz -1,
                                      1 );
               }
            }
            else
            {
               bRC = mComm.TxData( pAuxData, 
                                   auxDataSz );
            }
         }
         else if (mpActiveRequest->mRequiredAuxTxs > 1)
         {
            auxDataSz = MAX_AUX_MTU_SIZE;

            bRC = mComm.TxData( pAuxData, 
                                auxDataSz );
         }

         if (bRC == true)
         {
            mpActiveRequest->mEncodedSize = auxDataSz;
            TxComplete();
         }
         else
         {
            TxError();
         }

         return;
      }
   }
   
   // Another successful transmission, add the buffer to the log
   ULONG reqIdx = INVALID_LOG_INDEX;

   sProtocolBuffer pb( req.GetSharedBuffer() );
   reqIdx = mLog.AddBuffer( pb );

   // Notify client?
   if (pNotifier != 0)
   {
      pNotifier->Notify( ePROTOCOL_EVT_REQ_SENT, (DWORD)reqID, (DWORD)reqIdx );
   }

   // Wait for a response?
   if (mpActiveRequest->mRequest.IsTXOnly() == false)
   {
      // We now await the response
      mpActiveRequest->mbWaitingForResponse = true;
      mActiveRequestTimeout = TimeIn( mpActiveRequest->mRequest.GetTimeout() );
   }
   else
   {
      // Reschedule request as needed
      RescheduleActiveRequest();
   }
}

/*===========================================================================
METHOD:
   TxError (Internal Method)

DESCRIPTION:
   Handle transmit data operation error be either rescheduling the
   request or cleaning it up

SEQUENCING:
   Calling process must have lock on mScheduleMutex

RETURN VALUE:
   None
===========================================================================*/
void cProtocolServer::TxError()
{
   // No active request?
   if (mpActiveRequest == 0)
   {
      return;
   }

   ULONG reqID = mpActiveRequest->mID;
   const sProtocolRequest & req = mpActiveRequest->mRequest;
   const cProtocolNotification * pNotifier = req.GetNotifier();

   // Failure to send request, notify client
   if (pNotifier != 0)
   {
      pNotifier->Notify( ePROTOCOL_EVT_REQ_ERR, (DWORD)reqID, (DWORD)0 );
   }

   // Reschedule request as needed
   RescheduleActiveRequest();
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Initialize the protocol server by starting up the schedule thread

SEQUENCING:
   This method is sequenced according to the schedule mutex, i.e. any
   other thread that needs to modify the schedule will block until 
   this method completes

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::Initialize()
{
   // Assume failure
   bool bRC = false;

   mbExiting = false;

   // Get mScheduleMutex
   if (GetScheduleMutex() == true)
   {
      if (mScheduleThreadID == 0)
      {
         // Yes, start thread
         int nRet = pthread_create( &mScheduleThreadID,
                                    NULL,
                                    ScheduleThread,
                                    this );
         if (nRet == 0)
         {
            // Success!
            bRC = true;
         }
      }
   }
   else
   {
      TRACE( "cProtocolServer::Initialize(), unable to aquire ScheduleMutex\n" );
      return false;
   }

   // Unlock schedule mutex        
   if (ReleaseScheduleMutex() == false)
   {
      // This should never happen
      return false;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   Exit (Public Method)

DESCRIPTION:
   Exit the protocol server by exiting the schedule thread (if necessary)

SEQUENCING:
   This method is sequenced according to the schedule mutex, i.e. any
   other thread that needs to modify the schedule will block until 
   this method completes

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::Exit()
{
   // Assume failure
   bool bRC = false;

   if (mScheduleThreadID != 0)
   {
      if (GetScheduleMutex() == false)
      {
         // This should never happen
         return false;
      }

      // Set exit event
      mbExiting = true;
      
      // Signal a schedule update
      if (mThreadScheduleEvent.Set( 1 ) != 0)
      {
         // This should never happen
         return false;
      }
      
      TRACE( "Joining ScheduleThread\n" );
      
      // Allow process to continue until it finishes
      int nRet = pthread_join( mScheduleThreadID, NULL );
      if (nRet == ESRCH)
      {
         TRACE( "ScheduleThread has exited already\n" );
      }
      else if (nRet != 0)
      {
         TRACE( "Unable to join ScheduleThread. Error %d: %s\n",
                nRet,
                strerror( nRet ) );
         return false;
      }
      
      TRACE( "cProtocolServer::Exit(), completed thread %lu\n",
             (ULONG)mScheduleThreadID );

      bRC = true;
      
      // Release "handle"
      mScheduleThreadID = 0;
      
      // Release mutex lock, don't signal ScheduleThread
      if (ReleaseScheduleMutex( false ) == false)
      {
         // This should never happen
         return false;
      }   
   }
   else
   {
      // No ScheduleThread
      bRC = true;
   }

   // Free any allocated requests
   std::map <ULONG, sProtocolReqRsp *>::iterator pReqIter;
   pReqIter = mRequestMap.begin();

   while (pReqIter != mRequestMap.end())
   {
      sProtocolReqRsp * pReqRsp = pReqIter->second;
      if (pReqRsp != 0)
      {
         delete pReqRsp;
      }

      pReqIter++;
   }

   mRequestMap.clear();

   // Free log
   mLog.Clear();

   return bRC;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the given communications port

PARAMETERS:
   pPort       [ I ] - String pointer representing the device node to
                       connect to (IE: /dev/qcqmi0)

SEQUENCING:
   None

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::Connect( LPCSTR pPort )
{
   // Assume failure
   bool bRC = false;
   if (pPort == 0 || pPort[0] == 0)
   {
      return bRC;
   }

   // Connect to device

   // Set callback
   mRxCallback.SetServer( this );

   // Override to initialize port with protocol specific options
   bRC = mComm.Connect( pPort );
   if (bRC == true)
   {
      bRC = InitializeComm();
      if (bRC == true)
      {
         // Setup the initial read
         mComm.RxData( mpRxBuffer, 
                       (ULONG)mRxBufferSize, 
                       (cIOCallback *)&mRxCallback );
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Disconnect from the current communications port

SEQUENCING:
   None

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::Disconnect()
{
   // Disconnect

   // Cancel any outstanding I/O
   mComm.CancelIO();

   // Empty callback
   mRxCallback.SetServer( 0 );

   // Cleanup COM port
   CleanupComm();

   // Now disconnect
   return mComm.Disconnect();
}

/*===========================================================================
METHOD:
   IsConnected (Public Method)

DESCRIPTION:
   Are we currently connected to a port?

SEQUENCING:
   None

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::IsConnected()
{
   return mComm.IsConnected();
}

/*===========================================================================
METHOD:
   AddRequest (Public Method)

DESCRIPTION:
   Add an outgoing protocol request to the protocol server request queue

PARAMETERS:
   req        [ I ] - Request being added

SEQUENCING:
   This method is sequenced according to the schedule mutex, i.e. any
   other thread that needs to modify the schedule will block until 
   this method completes

RETURN VALUE:
   ULONG - ID of scheduled request (INVALID_REQUEST_ID upon error)
===========================================================================*/
ULONG cProtocolServer::AddRequest( const sProtocolRequest & req )
{
   // Assume failure
   ULONG reqID = INVALID_REQUEST_ID;

   // Server not configured for sending requests?
   if (IsValid( mTxType ) == false)
   {
      return reqID;
   }

   // Request type not valid for server?
   if (req.GetType() != mTxType)
   {
      return reqID;
   }

   // Invalide request?
   if (ValidateRequest( req ) == false)
   {
      return reqID;
   }

   // Get mScheduleMutex
   if (GetScheduleMutex() == true)
   {
      TRACE( "AddRequest() - Entry at %llu\n", GetTickCount() );

      // Grab next available request ID
      if (++mLastRequestID == 0)
      {
         mLastRequestID++;
      }
      
      reqID = mLastRequestID;
      while (mRequestMap.find( reqID ) != mRequestMap.end())
      {
         reqID++;
      }

      // Wrap in our internal structure
      sProtocolReqRsp * pReqRsp = 0;
      pReqRsp = new sProtocolReqRsp( req, reqID, MAX_AUX_MTU_SIZE );

      if (pReqRsp != 0)
      {
         // Add to request map
         mRequestMap[reqID] = pReqRsp;
         
         // ... and schedule
         ScheduleRequest( reqID, req.GetSchedule() );
      }
      
      TRACE( "AddRequest() - Exit at %llu\n", GetTickCount() );

      // Unlock schedule mutex        
      if (ReleaseScheduleMutex() == false)
      {
         // This should never happen
         return INVALID_REQUEST_ID;
      }
   }
   else
   {
      TRACE( "cProtocolServer::AddRequest(), unable to get schedule Mutex\n" );
   }

   return reqID;
}

/*===========================================================================
METHOD:
   RemoveRequest (Public Method)

DESCRIPTION:
   Remove a previously added protocol request

SEQUENCING:
   This method is sequenced according to the schedule mutex, i.e. any
   other thread that needs to modify the schedule will block until 
   this method completes
   
   Note: If a request is being written, it cannot be inturrupted as
      both ProcessRequest and RemoveRequest depend on the ScheduleMutex
      and the write is synchronus.  If the request has been written but
      the read has not been triggered it can be removed.

PARAMETERS:
   reqID    [ I ] - ID of request being removed

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::RemoveRequest( ULONG reqID )
{
   // Assume failure
   bool bRC = false;
   
   // Get Schedule Mutex
   if (GetScheduleMutex() == true)
   {
      TRACE( "RemoveRequest() - Entry at %llu\n", GetTickCount() );
      
      bRC = HandleRemoveRequest( reqID );
      
      TRACE( "RemoveRequest() - Exit at %llu\n", GetTickCount() );

      // Unlock schedule mutex        
      if (ReleaseScheduleMutex() == false)
      {
         // This should never happen
         return false;
      }
   }
   else
   {
      TRACE( "cProtocolServer::RemoveRequest(), unable to get mScheduleMutex\n" );
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetScheduleMutex (Internal Method)

DESCRIPTION:
   Get the schedule mutex.  Additionally a check is applied to verify the 
   DEADLOCK_TIME was not exceeded

SEQUENCING:
   This function will block until the mScheduleMutex is aquired

PARAMETERS:

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::GetScheduleMutex()
{
   ULONGLONG nStart = GetTickCount();
   
   //TRACE( "Locking Schedule mutex\n" );
   int nRet = pthread_mutex_lock( &mScheduleMutex );
   if (nRet != 0)
   {
      TRACE( "Unable to lock schedule mutex. Error %d: %s\n",
             nRet,
             strerror( nRet ) );
      return false;
   }
   
   ULONGLONG nEnd = GetTickCount();
   if (nEnd - nStart > DEADLOCK_TIME)
   {
      TRACE( "Deadlock time exceeded: took %llu ms\n", nEnd - nStart );
      ReleaseScheduleMutex( true );
      return false;
   }

   //TRACE( "Locked ScheduleMutex\n" );
   return true;
}

/*===========================================================================
METHOD:
   ReleaseScheduleMutex (Internal Method)

DESCRIPTION:
   Release lock on the schedule mutex

SEQUENCING:
   Calling process must have lock

PARAMETERS:
   bSignalThread  [ I ] -  Signal Schedule thread as well?

RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolServer::ReleaseScheduleMutex( bool bSignalThread )
{
   if (bSignalThread == true)
   {
      if (mThreadScheduleEvent.Set( 1 ) != 0)
      {
         return false;
      }
   }
   
   int nRet = pthread_mutex_unlock( &mScheduleMutex );
   if (nRet != 0)
   {
      TRACE( "Unable to unlock schedule mutex. Error %d: %s\n",
             nRet,
             strerror( nRet ) );
      return false;
   }

   return true;
}
