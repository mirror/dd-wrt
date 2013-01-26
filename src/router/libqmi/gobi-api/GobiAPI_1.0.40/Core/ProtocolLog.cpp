/*===========================================================================
FILE:
   ProtocolLog.h

DESCRIPTION:
   Simple protocol 'log' class definition
   
PUBLIC CLASSES AND METHODS:
   cProtocolLog
      This class stores protocol buffers in to a flat array (actually a
      double-ended queue) so that they can be accessed by other objects 
      during the flow of normal processing.  Note that the storage is
      in-memory and therefore finite

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
#include "ProtocolLog.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// The maximum number of in-memory buffers we allow
const ULONG MAX_PROTOCOL_BUFFERS = 1024 * 16;

/*=========================================================================*/
// cProtocolLog Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cProtocolLog (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   maxBuffers  [ I ] - Maximum number of buffers to store in the log
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolLog::cProtocolLog( ULONG maxBuffers )
   :  mLog( maxBuffers > MAX_PROTOCOL_BUFFERS ? MAX_PROTOCOL_BUFFERS : maxBuffers,
            true )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cProtocolLog (Public Method)

DESCRIPTION:
   Destructor

RETURN VALUE:
   None
===========================================================================*/
cProtocolLog::~cProtocolLog()
{
   // Empty out the log
   Clear();
}

/*===========================================================================
METHOD:
   AddBuffer (Public Method)

DESCRIPTION:
   Add an protocol buffer to the end of the log

PARAMETERS:
   buff        [ I ] - Protocol buffer to add

RETURN VALUE:
   ULONG - Index of newly added buffer (INVALID_LOG_INDEX upon failure)
===========================================================================*/
ULONG cProtocolLog::AddBuffer( sProtocolBuffer & buf )
{
   ULONG idx = INVALID_LOG_INDEX;
   if (buf.IsValid() == false)
   {
      return idx;
   }

   bool bRC = mLog.AddElement( buf, idx );
   if (bRC == false)
   {
      idx = INVALID_LOG_INDEX;
   }
    
   return idx;
}

/*===========================================================================
METHOD:
   GetBuffer (Public Method)

DESCRIPTION:
   Return the protocol buffer at the given index from the log

PARAMETERS:
   idx         [ I ] - Index of protocol buffer to obtain

RETURN VALUE:
   sProtocolBuffer - Protocol buffer
===========================================================================*/
sProtocolBuffer cProtocolLog::GetBuffer( ULONG idx ) const
{
   sProtocolBuffer buf;
   mLog.GetElement( idx, buf );
   return buf;
}

/*===========================================================================
METHOD:
   GetSignalEvent (Public Method)

DESCRIPTION:
   Return the underlying signal event, which will be set when
   the log is updated.

RETURN VALUE:
   cEvent - Signal event
===========================================================================*/
cEvent & cProtocolLog::GetSignalEvent() const
{
   return mLog.GetSignalEvent();
}

/*===========================================================================
METHOD:
   GetCount (Public Method)

DESCRIPTION:
   Return the total number of buffers added to the log

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cProtocolLog::GetCount() const
{
   return mLog.GetTotalCount();
}

/*===========================================================================
METHOD:
   Clear (Public Method)

DESCRIPTION:
   Clear the log

RETURN VALUE:
   None
===========================================================================*/
void cProtocolLog::Clear()
{
   mLog.EmptyQueue();
}
