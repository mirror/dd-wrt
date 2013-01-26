/*===========================================================================
FILE:
   ProtocolServer.h

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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "Comm.h"
#include "ProtocolRequest.h"
#include "ProtocolLog.h"
#include "Event.h"

#include <map>
#include <set>

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------
class cProtocolServer;
struct sServerControl;

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Invalid request ID
extern const ULONG INVALID_REQUEST_ID;

// Fill timespec with the time it will be in specified milliseconds
//   Relative time to Absolute time
timespec TimeIn( ULONG millis );

// Find the milliseconds from current time this timespec will occur
//   Absolute time to Relative time
ULONG TimeFromNow( timespec time );

// Provide a number for sequencing reference, similar to the windows function
ULONGLONG GetTickCount();

// timespec < comparison method
inline bool operator< (const timespec & first, const timespec & second)
{
   return ( (first.tv_sec < second.tv_sec)
          ||( (first.tv_sec == second.tv_sec)
            &&(first.tv_nsec < second.tv_nsec) ) );
}

// timespec <= comparison method
inline bool operator<= (const timespec & first, const timespec & second)
{
   return ( (first.tv_sec < second.tv_sec)
          ||( (first.tv_sec == second.tv_sec)
            &&(first.tv_nsec <= second.tv_nsec) ) );
}

/*=========================================================================*/
// Class cProtocolServerRxCallback
/*=========================================================================*/
class cProtocolServerRxCallback
{
   public:
      // (Inline) Constructor
      cProtocolServerRxCallback()
         :  mpServer( 0 )
      { };

      // (Inline) Destructor
      virtual ~cProtocolServerRxCallback() { };

      // (Inline) Set server object to pass results to
      void SetServer( cProtocolServer * pServer )
      {
         mpServer = pServer;
      };

      // The I/O has been completed, process the results
      virtual void IOComplete(
         DWORD                      status,
         DWORD                      bytesReceived );

   protected:
      /* Protocol server to interact with */
      cProtocolServer * mpServer;
};

/*=========================================================================*/
// Class cProtocolServer
/*=========================================================================*/
class cProtocolServer
{
   public:
      // Constructor
      cProtocolServer( 
         eProtocolType              rxType,
         eProtocolType              txType,
         ULONG                      bufferSzRx,
         ULONG                      logSz );

      // Destructor
      virtual ~cProtocolServer();

      // Initialize the protocol server
      bool Initialize();

      // Exit the protocol server
      bool Exit();

      // Connect to the given communications port
      bool Connect( LPCSTR pPort );

      // Disconnect from target
      bool Disconnect();

      // Are we currently connected to a port?
      bool IsConnected();

      // Add an outgoing protocol request to the protocol server request queue
      ULONG AddRequest( const sProtocolRequest & req );
 
      // Remove a previously added protocol request 
      bool RemoveRequest( ULONG reqID );

      // (Inline) Return the protocol log
      const cProtocolLog & GetLog()
      {
         return mLog;
      };

   protected:
      // Internal protocol server request/response structure, used to track
      // info related to sending out a request
      struct sProtocolReqRsp
      {
         public:
            // Constructor
            sProtocolReqRsp(
               const sProtocolRequest &   requestInfo,
               ULONG                      requestID,
               ULONG                      auxDataMTU );

            // Copy constructor
            sProtocolReqRsp( const sProtocolReqRsp & reqRsp );

            // (Inline) Reset for next transmission attempt
            void Reset()
            {
               mEncodedSize = mRequest.GetSize();

               mCurrentAuxTx = 0;
               mbWaitingForResponse = 0;
            };

            /* Request ID */
            ULONG mID;

            /* Number of times this request has been attempted */
            ULONG mAttempts;

            /* Size of encoded data being transmitted */
            ULONG mEncodedSize;

            /* Number of required auxiliary data transmissions */
            ULONG mRequiredAuxTxs;

            /* Current auxiliary data transmission */
            ULONG mCurrentAuxTx;

            /* Are we currently waiting for a response? */
            bool mbWaitingForResponse;

            /* Underlying protocol request */
            sProtocolRequest mRequest;
      };

      // Handle the remove request
      bool HandleRemoveRequest( ULONG reqID );

      // Schedule a request for transmission
      bool ScheduleRequest(
         ULONG                      reqID,
         ULONG                      schedule );

      // (Inline) Get next request's time from mRequestSchedule
      timespec GetNextRequestTime()
      {
         timespec outTime;
      
         std::set <tSchedule>::iterator pScheduleIter;
         pScheduleIter = mRequestSchedule.begin();
         tSchedule entry = *pScheduleIter;

         outTime = entry.first;
         return outTime;
      }
   
      // (Inline) Validate a request that is about to be scheduled
      virtual bool ValidateRequest( const sProtocolRequest & req )
      {
         return req.IsValid();
      };

      // Reschedule (or cleanup) the active request
      void RescheduleActiveRequest();

      // Process a single outgoing protocol request
      void ProcessRequest();

      // Check that system time hasn't moved backwards
      bool CheckSystemTime();

      // Perform protocol specific communications port initialization
      virtual bool InitializeComm() = 0;

      // Perform protocol specific communications port cleanup
      virtual bool CleanupComm() = 0;

      // Encode data for transmission
      virtual sSharedBuffer * EncodeTxData( 
         sSharedBuffer *            pBuffer,
         bool &                     bEncoded ) = 0;

      // Decode incoming data into packets returning the last response
      virtual bool DecodeRxData( 
         ULONG                      bytesReceived,
         ULONG &                    rspIdx,
         bool &                     bAbortTx ) = 0;

      // Handle completion of receive data operation
      void RxComplete(
         DWORD                      status,
         DWORD                      bytesReceived );

      // Handle the response timer expiring
      void RxTimeout();
      
      // Handle completion of transmit data operation
      virtual void TxComplete();

      // Handle a transmission error
      void TxError();

      /* Underlying communications object */
      cComm mComm;

      /* Rx callback */
      cProtocolServerRxCallback mRxCallback;

      /* ID of Schedule thread */
      pthread_t mScheduleThreadID;

      // ScheduleThread signal event
      cEvent mThreadScheduleEvent;

      // Schedule mutex
      // Ensures exclusive access to mRequestSchedule
      pthread_mutex_t mScheduleMutex;
      
      // Is the thread in the process of exiting?
      //  (no new commands will be accepted)
      bool mbExiting;
      
      /* Client/server thread control object */
      sSharedBuffer * mpServerControl;

      /* Protocol request schedule (scheduled time/request ID) */
      typedef std::pair <timespec, ULONG> tSchedule;
      std::set < tSchedule, std::less <tSchedule> > mRequestSchedule;

      /* Last system time value (used to check for time changes) */
      timespec mLastTime;

      /* Protocol request map (request ID mapped to internal req/rsp struct) */
      std::map <ULONG, sProtocolReqRsp *> mRequestMap;

      /* Last assigned request ID */
      ULONG mLastRequestID;

      /* Current request being processed */
      sProtocolReqRsp * mpActiveRequest;
      
      /* Absolute timeout for mpActiveRequest
         based on when write was completed */
      timespec mActiveRequestTimeout;

      /* Data buffer for incoming data */
      BYTE * mpRxBuffer;

      /* Size of above buffer (i.e. how much data to read in at once) */
      ULONG mRxBufferSize;

      /* Protocol type for incoming/outgoing data*/
      eProtocolType mRxType;
      eProtocolType mTxType;

      /* Protocol log */
      cProtocolLog mLog;

      // Get a lock on ScheduleMutex
      bool GetScheduleMutex();
      
      // Release lock on ScheduleMutex
      // Signal ScheduleThread if desired
      bool ReleaseScheduleMutex( bool bSignalThread = true );

      // Schedule Thread gets full access
      friend void * ScheduleThread( PVOID pArg );

      // Callback objects get full access
      friend class cProtocolServerRxCallback;
};

