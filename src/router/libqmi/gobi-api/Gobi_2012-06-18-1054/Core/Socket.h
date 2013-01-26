/*===========================================================================
FILE:
   Socket.h

DESCRIPTION:
   Declaration of cSocket class

PUBLIC CLASSES AND METHODS:
   cSocket
      This class wraps low level communication to qmuxd

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
#include "Connection.h"

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// Enum eQMUXDMessageTypes
//   Types to be passed into SendCtl() for cSocket
/*=========================================================================*/
enum eQMUXDMessageTypes
{
   eQMUXD_MSG_WRITE_QMI_SDU = 0,
   eQMUXD_MSG_ALLOC_QMI_CLIENT_ID = 1,
   eQMUXD_MSG_RELEASE_QMI_CLIENT_ID = 2,
};

/*=========================================================================*/
// Class cSocket
/*=========================================================================*/
class cSocket : public cConnection
{
   public:
      // Constructor
      cSocket();

      // Destructor
      ~cSocket();

      // Is this object valid?
      bool IsValid();

      // Connect to the specified channel
      bool Connect( LPCSTR pChannel );
      
      // Run an IOCTL on the open file handle
      int SendCtl(
         UINT     ioctlReq,
         void *   pData );

      // Disconnect from the current port
      bool Disconnect();

      // Configure the port with the passed in parameters
      bool ConfigureSettings( termios * pSettings );

      // Return the current port settings
      bool GetSettings( termios * pSettings );

      // Cancel any in-progress I/O
      bool CancelIO();

      // Cancel any in-progress receive operation
      bool CancelRx();

      // Cancel any in-progress transmit operation
      bool CancelTx();

      // Receive data
      bool RxData(
         BYTE *                     pBuf, 
         ULONG                      bufSz,
         cIOCallback *              pCallback );
      
      // Transmit data
      bool TxData(
         const BYTE *               pBuf, 
         ULONG                      bufSz );

      // (Inline) Return current channel ID
      int GetChannelID() const 
      {
         return mChannelID; 
      };

      // Are we currently connected to a port?
      bool IsConnected()
      {
         return (mSocket != INVALID_HANDLE_VALUE);
      };

   protected:

      /* Handle to socket */
      int mSocket;
      
      // Cancel the write request?
      bool mbCancelWrite;

      /* Buffer */
      BYTE * mpBuffer;

      /* Buffer size */
      ULONG mBuffSz;

      /* Pipe for comunication with thread */
      int mCommandPipe[2];

      /* Thread ID of Rx Thread. */
      pthread_t mRxThreadID;

      /* Control message completion event */
      cEvent mCtrlMsgComplete;

      /* QMUXD client ID */
      int mQMUXDClientID;

      /* QMUX client and service IDs */
      int mQMUXClientID;
      int mQMUXServiceID;

      /* SMD Channel ID. 0 = SMD_DATA_5 */
      int mChannelID;

      /* The SMD transaction ID */
      int mQMUXDTxID;

      // Rx thread is allowed complete access
      friend void * RxSocketThread( void * pData );
};
