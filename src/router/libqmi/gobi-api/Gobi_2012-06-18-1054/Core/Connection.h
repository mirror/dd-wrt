/*===========================================================================
FILE:
   Connection.h

DESCRIPTION:
   Declaration of cConnection class

PUBLIC CLASSES AND METHODS:
   cComm
      This class defines a prototype for low level communications

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

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
#include "Event.h"

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// Class cIOCallback
/*=========================================================================*/
class cIOCallback
{
   public:
      // (Inline) Constructor
      cIOCallback() { };

      // (Inline) Destructor
      virtual ~cIOCallback() { };

      // The I/O has been completed, process the results
      virtual void IOComplete(
         DWORD                      status,
         DWORD                      bytesTransferred ) = 0;
};

/*=========================================================================*/
// Class cConnection
/*=========================================================================*/
class cConnection
{
   public:
      // Constructor
      cConnection()
         : mpRxCallback( 0 )
      { };

      // Is this object valid?
      virtual bool IsValid()
      {
         return false;
      };

      // Connect to the specified interface
      virtual bool Connect( LPCSTR pPort )
      {
         return false;
      };
      
      // Send a control message
      virtual int SendCtl(
         UINT     type,
         void *   pData )
      {
         return -1;
      };

      // Disconnect from the current port
      virtual bool Disconnect()
      {
         return false;
      };

      // Cancel any in-progress I/O
      virtual bool CancelIO()
      {
         return false;
      };

      // Cancel any in-progress receive operation
      virtual bool CancelRx()
      {
         return false;
      };

      // Cancel any in-progress transmit operation
      virtual bool CancelTx()
      {
         return false;
      };

      // Receive data
      virtual bool RxData(
         BYTE *                     pBuf, 
         ULONG                      bufSz,
         cIOCallback *              pCallback )
      {
         return false;
      };
      
      // Transmit data
      virtual bool TxData(
         const BYTE *               pBuf, 
         ULONG                      bufSz )
      {
         return false;
      };

      // Are we currently connected to a port?
      virtual bool IsConnected()
      {
         return false;
      };

   protected:
      /* Read callbacks */
      cIOCallback * mpRxCallback;
};
