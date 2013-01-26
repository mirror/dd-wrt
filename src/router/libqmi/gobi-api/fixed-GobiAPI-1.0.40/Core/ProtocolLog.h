/*===========================================================================
FILE:
   ProtocolLog.h

DESCRIPTION:
   Simple protocol 'log' class declaration
   
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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "ProtocolBuffer.h"
#include "SyncQueue.h"

#include <climits>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
const ULONG INVALID_LOG_INDEX = ULONG_MAX;

/*=========================================================================*/
// Class cProtocolLog
/*=========================================================================*/
class cProtocolLog
{
   public:
      // Constructor
      cProtocolLog( ULONG maxBuffers );

      // Destructor
      virtual ~cProtocolLog();

      // Add an protocol buffer to the end of the log
      virtual ULONG AddBuffer( sProtocolBuffer & buf );

      // Return the protocol buffer at the given index from the log
      virtual sProtocolBuffer GetBuffer( ULONG idx ) const;

      // Return the underlying signal event
      virtual cEvent & GetSignalEvent() const;

      // Return the total number of buffers added to the log
      virtual ULONG GetCount() const;

      // Clear the log
      virtual void Clear();

   protected:
      /* The underlying 'log' */
      cSyncQueue <sProtocolBuffer> mLog;
};
