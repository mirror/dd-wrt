/*===========================================================================
FILE:
   ProtocolRequest.h

DESCRIPTION:
   Generic protocol request/command related structures and 
   affliated methods, these structures are used by clients of
   the protocol server to specify outgoing protocol requests

PUBLIC CLASSES AND METHODS:
   sProtocolRequest

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
#include "ProtocolBuffer.h"

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------
class cProtocolNotification;

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Default protocol request timeout
extern const ULONG DEFAULT_REQ_TIMEOUT;

// Minimum and maximum allowable timeout values (in milliseconds)
extern const ULONG MIN_REQ_TIMEOUT;
extern const ULONG MAX_REQ_TIMEOUT;

// Minimum number of attempts a request can be scheduled for
extern const ULONG MIN_REQ_ATTEMPTS;

// Value to indicate that a request is to be sent out indefinately
extern const ULONG INFINITE_REQS;

// Minimum/default amount of time between repeated requests (in milliseconds)
extern const ULONG MIN_REQ_FREQUENCY;
extern const ULONG DEFAULT_REQ_FREQUENCY;

/*=========================================================================*/
// Struct sProtocolRequest
//
//    Structure to represent a generic request packet, including all the
//    information needed to schedule the request, send the request, and 
//    (optionally) reschedule the request for another TX/RX attempt
//
//    The default parameters schedule an immediate request (indicated by
//    passing in '0' for the schedule parameter) to be sent once with
//    the default timeout value
/*=========================================================================*/
struct sProtocolRequest : public sProtocolBuffer
{
   public: 
      // Parameterized constructor
      sProtocolRequest( 
         sSharedBuffer *            pBuffer,
         ULONG                      schedule = 0,
         ULONG                      timeout = DEFAULT_REQ_TIMEOUT,
         ULONG                      requests = MIN_REQ_ATTEMPTS,
         ULONG                      frequency = DEFAULT_REQ_FREQUENCY,
         cProtocolNotification *    pNotifier = 0 );

      // Parameterized constructor (notification with defaults)
      sProtocolRequest( 
         sSharedBuffer *            pBuffer,
         cProtocolNotification *    pNotifier );

      // Copy constructor
      sProtocolRequest( const sProtocolRequest & req );

      // Destructor
      virtual ~sProtocolRequest();

      // (Inline) Get schedule value (value is in milliseconds)
      ULONG GetSchedule() const 
      {
         return mSchedule;
      };

      // (Inline) Get timeout value
      ULONG GetTimeout() const
      {
         return mTimeout;
      };

      // (Inline) Get requests value
      ULONG GetRequests() const 
      {
         return mRequests;
      };

      // (Inline) Get frequency value (value is in milliseconds)
      ULONG GetFrequency() const 
      {
         return mFrequency;
      };

      const cProtocolNotification * GetNotifier() const
      {
         return mpNotifier;
      };

      // (Inline) Set auxiliary data
      void SetAuxiliaryData(
         const BYTE *               pData,
         ULONG                      dataSz )
      {
         mpAuxData = pData;
         mAuxDataSize = dataSz;
      };

      // (Inline) Get auxiliary data
      const BYTE * GetAuxiliaryData( ULONG & dataSz ) const
      {
         dataSz = mAuxDataSize;
         return mpAuxData;
      };

      // (Inline) Set TX only flag
      void SetTXOnly()
      {
         mbTXOnly = true;
      };

      // (Inline) Get TX only flag
      bool IsTXOnly() const
      {
         return mbTXOnly;
      };

   protected:
      /* Schedule (approximately when to send the initial request) */
      ULONG mSchedule;

      /* Timeout value for receiving a response */
      ULONG mTimeout;

      /* Number of requests to schedule (must be at least one) */
      ULONG mRequests;

      /* Frequency (approximately how long to wait before next request) */
      ULONG mFrequency;

      /* Notification object */
      cProtocolNotification * mpNotifier;

      /* Auxiliary data */
      const BYTE * mpAuxData;

      /* Auxilary data size */
      ULONG mAuxDataSize;

      /* TX only (i.e. do not wait for a response) ? */
      bool mbTXOnly;
};

