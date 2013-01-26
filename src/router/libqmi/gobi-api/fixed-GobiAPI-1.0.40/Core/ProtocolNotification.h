/*===========================================================================
FILE: 
   ProtocolNotification.h

DESCRIPTION:
   Declaration of cProtocolNotification base class and derivations

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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "SyncQueue.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
enum eProtocolEventType
{
   ePROTOCOL_EVT_BEGIN = -1,

   ePROTOCOL_EVT_REQ_ERR,     // There was an error sending the request
   ePROTOCOL_EVT_REQ_SENT,    // The request has been sent

   ePROTOCOL_EVT_RSP_ERR,     // There was an error receiving the response
   ePROTOCOL_EVT_RSP_RECV,    // The response has been received

   ePROTOCOL_EVT_AUX_TU_SENT, // Auxiliary data transmission unit sent      

   ePROTOCOL_EVT_END
};

// NOTE: The arguments for each event are currently as follows:
//
//    ePROTOCOL_EVT_REQ_ERR
//       param1:  Request ID
//       param2:  Error code
//
//    ePROTOCOL_EVT_REQ_SENT
//       param1:  Request ID
//       param2:  Index of request buffer in associated protocol log

//    ePROTOCOL_EVT_RSP_ERR
//       param1:  Request ID
//       param2:  Error code
//
//    ePROTOCOL_EVT_RSP_RECV
//       param1:  Request ID
//       param2:  Index of response buffer in associated protocol log
//
//    ePROTOCOL_EVT_AUX_TU_SENT
//       param1:  Request ID
//       param2:  Size of transmission unit

// NOTE: To handle protoocl events using the Windows notifier add the following
// prototype to your Window class header file:
//
//    afx_msg LRESULT OnProtocolEvent( 
//       WPARAM                     wParam, 
//       LPARAM                     lParam );
//
// Then add an entry to the message map in your Window class source file:
//
// BEGIN_MESSAGE_MAP( CView, CChildView )
//    ON_MESSAGE( PROTOCOL_WM_BASE + (ULONG)ePROTOCOL_EVT_XXX, OnProtocolEvent )
// END_MESSAGE_MAP()
//
// Finally write the handler itself:
//
// LRESULT CView::OnProtocolEvent( 
//    WPARAM                     wParam, 
//    LPARAM                     lParam )
// {
//    Do something
//    return 0;
// }

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eProtocolEventType validity check

PARAMETERS:
   evtType     [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eProtocolEventType evtType )
{
   bool bRC = false;
   if (evtType > ePROTOCOL_EVT_BEGIN && evtType < ePROTOCOL_EVT_END)
   {
      bRC = true;
   }

   return bRC;
};

/*=========================================================================*/
// Struct sProtocolNotificationEvent
/*=========================================================================*/
struct sProtocolNotificationEvent
{
   public:
      // (Inline) Default constructor (results in invalid object)
      sProtocolNotificationEvent()
         :  mEventType( ePROTOCOL_EVT_BEGIN ),
            mParam1( 0 ),
            mParam2( 0 )
      {
         // Nothing to do
      };

      // (Inline) Parameter constructor
      sProtocolNotificationEvent(
         eProtocolEventType         eventType,
         DWORD                      param1,
         DWORD                      param2 )
         :  mEventType( eventType ),
            mParam1( param1 ),
            mParam2( param2 )
      {
         // Nothing to do
      };

      // (Inline) Is this object valid?
      bool IsValid()
      {
         return ::IsValid( mEventType );
      }

      /* Event type */
      eProtocolEventType mEventType;

      /* First parameter (see above) */
      DWORD mParam1;

      /* Second parameter (see above) */
      DWORD mParam2;
};

/*=========================================================================*/
// Class cProtocolNotification
//
//    This abstract base class provides notification of protocol server  
//    events sent from the protocol server to protocol server clients
/*=========================================================================*/
class cProtocolNotification
{
   public:
      // Return an allocated copy of this object
      virtual cProtocolNotification * Clone() const = 0;

      // Notify view of a protocol event
      virtual void Notify(
         eProtocolEventType         eventType,
         DWORD                      param1,
         DWORD                      param2 ) const = 0;
};

/*=========================================================================*/
// Class cProtocolQueueNotification
//
//    This class provides notification via a cSyncQueue object
//    populated with sProtocolNotificationEvent objects
/*=========================================================================*/
class cProtocolQueueNotification : public cProtocolNotification
{
   public:
      // Constructor
      cProtocolQueueNotification( cSyncQueue <sProtocolNotificationEvent> * pSQ );

      // Copy constructor
      cProtocolQueueNotification( const cProtocolQueueNotification & notifier );

      // Destructor
      virtual ~cProtocolQueueNotification();

      // Return a copy of this object
      virtual cProtocolNotification * Clone() const;

      // Notify view of a MIS event
      virtual void Notify(
         eProtocolEventType         eventType,
         DWORD                      param1,
         DWORD                      param2 ) const;

   protected:
      /* Event notification queue */
      mutable cSyncQueue <sProtocolNotificationEvent> * mpSQ;
};
