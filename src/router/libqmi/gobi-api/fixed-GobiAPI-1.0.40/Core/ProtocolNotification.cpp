/*===========================================================================
FILE: 
   ProtocolNotification.cpp

DESCRIPTION:
   Implementation of cProtocolNotification base class and derivations

PUBLIC CLASSES AND METHODS:
   sProtocolNotificationEvent
      Generic protocol event notification structure

   cProtocolNotification
      This abstract base class provides notification of protocol server  
      events sent from the protocol server to protocol server clients

   cProtocolQueueNotification
      This class provides notification via a cSyncQueue object
      populated with sProtocolNotificationEvent objects

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
#include "ProtocolNotification.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cProtocolQueueNotification Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cProtocolQueueNotification (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pSQ         [ I ] - Sync queue to utilize
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolQueueNotification::cProtocolQueueNotification( 
   cSyncQueue <sProtocolNotificationEvent> * pSQ )
   :  mpSQ( pSQ )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   cProtocolQueueNotification (Public Method)

DESCRIPTION:
   Copy constructor

PARAMETERS:
   notifier    [ I ] - Notifier to base the new one on
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolQueueNotification::cProtocolQueueNotification( 
   const cProtocolQueueNotification &  notifier )
   :  mpSQ( notifier.mpSQ )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cProtocolQueueNotification (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolQueueNotification::~cProtocolQueueNotification()
{
   mpSQ = 0;
}

/*===========================================================================
METHOD:
   Clone (Public Method)

DESCRIPTION:
   Return an allocated copy of this object downcasted to our base class
  
RETURN VALUE:
   cProtocolNotification * : Cloned object (0 on error)
===========================================================================*/
cProtocolNotification * cProtocolQueueNotification::Clone() const
{
   cProtocolQueueNotification * pCopy = 0;

   try
   {
      pCopy = new cProtocolQueueNotification( *this );
   }
   catch (...)
   {
      // Simply return 0
   }

   return ((cProtocolNotification *)pCopy);
}

/*===========================================================================
METHOD:
   Notify (Public Method)

DESCRIPTION:
   Notify view of a protocol event by adding notification structure to 
   the underlying sync queue (which will provide the notification
   by signalling an event)

PARAMETERS:
   eventType   [ I ] - Protocol event type
   param1      [ I ] - Event type specific argument (see header description)
   param2      [ I ] - Event type specific argument (see header description)
  
RETURN VALUE:
   None
===========================================================================*/
void cProtocolQueueNotification::Notify(
   eProtocolEventType         eventType,
   DWORD                      param1,
   DWORD                      param2 ) const
{
   sProtocolNotificationEvent evt( eventType, param1, param2 );
   if (evt.IsValid() == true && mpSQ != 0 && mpSQ->IsValid() == true)
   {
      sProtocolNotificationEvent elem( eventType, param1, param2 );
      mpSQ->AddElement( elem );
   }
}
