/*===========================================================================
FILE:
   ProtocolRequest.cpp

DESCRIPTION:
   Generic protocol request/command related structures and 
   affliated methods, these structures are used by clients of
   the protocol server to specify outgoing requests

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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"

#include "ProtocolRequest.h"
#include "ProtocolNotification.h"
#include "ProtocolServer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Default protocol request timeout 
const ULONG DEFAULT_REQ_TIMEOUT = 1000;

// Minimum and maximum allowable timeout values (in milliseconds)
const ULONG MIN_REQ_TIMEOUT = 100;
const ULONG MAX_REQ_TIMEOUT = 300000;

// Minimum number of attempts a request can be scheduled for
const ULONG MIN_REQ_ATTEMPTS = 1;

// Value to indicate that a request is to be sent out indefinately
const ULONG INFINITE_REQS = 0xFFFFFFFF;

// Minimum/default amount of time between repeated requests (in milliseconds)
const ULONG MIN_REQ_FREQUENCY = 10;
const ULONG DEFAULT_REQ_FREQUENCY = 100;

/*=========================================================================*/
// sProtocolRequest Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sProtocolRequest

DESCRIPTION:
   Parameterized constructor
  
PARAMETERS:
   pBuffer     [ I ] - Shareable buffer representing the request (must be
                       valid)

   schedule    [ I ] - When (from now, in milliseconds) to send the first
                       request, this isn't a hard value as the request is
                       only guaranteed to go out after this time elapses

   timeout     [ I ] - Milliseconds to wait for a response to an individual
                       request before declaring a timeout.  Regardless of 
                       what is passed in the timeout value used will be 
                       between MIN/MAX_REQ_TIMEOUT

   requests    [ I ] - Number of request attempts to make, this isn't a 
                       retry count rather this value is used to specify
                       repeating requests.  Regardless of what is passed in 
                       the requests value used will be at least 
                       MIN_REQ_ATTEMPTS

   frequency   [ I ] - If the 'requests' value is greater than the
                       MIN_REQ_ATTEMPTS than this represents the amount of 
                       time to wait between requests (from the completion of 
                       the last request attempt, in milliseconds), again this 
                       isn't a hard value.  Regardless of what is passed in 
                       the frequency value used will be at least 
                       MIN_REQ_FREQUENCY

   pNotifier   [ I ] - Status notification mechanism (may be 0)


RETURN VALUE:
   None
===========================================================================*/
sProtocolRequest::sProtocolRequest( 
   sSharedBuffer *            pBuffer,
   ULONG                      schedule,
   ULONG                      timeout,
   ULONG                      requests,
   ULONG                      frequency,
   cProtocolNotification *    pNotifier )
   :  sProtocolBuffer( pBuffer ),
      mSchedule( schedule ),
      mTimeout( DEFAULT_REQ_TIMEOUT ),
      mRequests( MIN_REQ_ATTEMPTS ),
      mFrequency( DEFAULT_REQ_FREQUENCY ),
      mpNotifier( 0 ),
      mpAuxData( 0 ),
      mAuxDataSize( 0 ),
      mbTXOnly( false )
{
   // Constrain requested timeout to allowable range
   if (timeout < MIN_REQ_TIMEOUT)
   {
      timeout = MIN_REQ_TIMEOUT;
   }

   if (timeout > MAX_REQ_TIMEOUT)
   {
      timeout = MAX_REQ_TIMEOUT;
   }

   mTimeout = timeout;

   // Constrain request attempts
   if (requests >= MIN_REQ_ATTEMPTS)
   {
      mRequests = requests;
   }

   // Constrain frequency
   if (frequency >= MIN_REQ_FREQUENCY)
   {
      mFrequency = frequency;
   }

   // Clone notifier?
   if (pNotifier != 0)
   {
      mpNotifier = pNotifier->Clone();
   }
}

/*===========================================================================
METHOD:
   sProtocolRequest

DESCRIPTION:
   Parameterized constructor (notification with defaults)
  
PARAMETERS:
   pBuffer     [ I ] - Shareable buffer representing the request (must be
                       valid)

   pNotifier   [ I ] - Status notification mechanism (may be 0)


RETURN VALUE:
   None
===========================================================================*/
sProtocolRequest::sProtocolRequest( 
   sSharedBuffer *            pBuffer,
   cProtocolNotification *    pNotifier )
   :  sProtocolBuffer( pBuffer ),
      mSchedule( 0 ),
      mTimeout( DEFAULT_REQ_TIMEOUT ),
      mRequests( MIN_REQ_ATTEMPTS ),
      mFrequency( DEFAULT_REQ_FREQUENCY ),
      mpNotifier( pNotifier ),
      mpAuxData( 0 ),
      mAuxDataSize( 0 ),
      mbTXOnly( false )
{
   // Clone notifier?
   if (pNotifier != 0)
   {
      mpNotifier = pNotifier->Clone();
   }

   Validate();
}

/*===========================================================================
METHOD:
   sProtocolRequest

DESCRIPTION:
   Copy constructor
  
PARAMETERS:
   req         [ I ] - Request to copy

RETURN VALUE:
   None
===========================================================================*/
sProtocolRequest::sProtocolRequest( const sProtocolRequest & req )
   :  sProtocolBuffer( req ),
      mSchedule( req.mSchedule ),
      mTimeout( req.mTimeout ),
      mRequests( req.mRequests ),
      mFrequency( req.mFrequency ),
      mpNotifier( 0 ),
      mpAuxData( req.mpAuxData ),
      mAuxDataSize( req.mAuxDataSize ),
      mbTXOnly( req.mbTXOnly )
{
   // Clone notifier?
   if (req.mpNotifier != 0)
   {
      mpNotifier = req.mpNotifier->Clone();
   }

   Validate();
}

/*===========================================================================
METHOD:
   ~sProtocolRequest

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sProtocolRequest::~sProtocolRequest() 
{
   // Delete cloned notifier?
   if (mpNotifier != 0)
   {
      delete mpNotifier;
      mpNotifier = 0;
   }
}
