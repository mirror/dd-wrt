/*===========================================================================
FILE:
   Socket.cpp

DESCRIPTION:
   Implementation of cSocket class

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
#include "StdAfx.h"
#include "Socket.h"
#include "ProtocolServer.h"
#include <sys/socket.h>
#include <sys/un.h>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
// Thread commands
#define START_READ_CMD  0
#define STOP_READ_CMD   1
#define EXIT_CMD        2

// Size of the QMUXD command payload
// GET_CLIENT_ID and RELEASE_CLIENT_ID must pass in a buffer of this size
#define PAYLOAD_SIZE 664

/*=========================================================================*/
// struct sQMUXDHeader
/*=========================================================================*/
#pragma pack( push, 1 )

struct sQMUXDHeader
{
   /* Total size of header and following buffer */
   int mTotalSize;

   /* QMUXD client ID */
   int mQMUXDClientID;

   /* Message type */
   eQMUXDMessageTypes mQMUXDMsgID;
   
   /* Duplicate of mQMUXDClientID */
   int mQMUXDClientIDDuplicate;
   
   /* Transaction ID */
   unsigned long mTxID;

   /* System error code */
   int mSysErrCode;

   /* QMI error code (duplicate of TLV 0x02) */
   int mQmiErrCode;

   /* SMD channel.  0 = SMD_DATA_5 */
   int mQMUXDConectionType;
   
   /* QMI service ID */
   int mQMUXServiceID;

   /* QMI client ID */
   unsigned char mQMUXClientID;

   /* QMI flags */
   unsigned char mRxFlags;

   /* In QMUXD this struct is not packed, so the compiler appends
      these two bytes */
   unsigned short int mMissing2Bytes;
};

#pragma pack( pop )


/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   RxSocketThread (Free Method)

DESCRIPTION:
   Thread for simulating asynchronous reads to a socket

PARAMETERS:
   pData      [ I ]   cSocket pointer

RETURN VALUE:
   void * - thread exit value (always 0)
===========================================================================*/
void * RxSocketThread( void * pData )
{
   cSocket * pSocket = (cSocket*)pData;
   if (pSocket == NULL || pSocket->IsValid() == false)
   {
      return 0;
   }

   fd_set inputSet, outputSet;
   FD_ZERO( &inputSet );
   FD_SET( pSocket->mCommandPipe[READING], &inputSet );
   int largestFD = pSocket->mCommandPipe[READING];

   int status = 0;
   while (true)
   {
      // No FD_COPY() available
      memcpy( &outputSet, &inputSet, sizeof( fd_set ) );

      // Wait until we recieve a command or data is available
      status = select( largestFD + 1, &outputSet, NULL, NULL, NULL );
      if (status <= 0)
      {
         TRACE( "error %d in select, errno %d\n", status, errno );
         break;
      }

      if (FD_ISSET( pSocket->mCommandPipe[READING], &outputSet ) == true)
      {
         // Read command value from the pipe
         BYTE cmd;
         status = read( pSocket->mCommandPipe[READING], &cmd, 1 );
         if (status != 1)
         {
            TRACE( "cmd error %d\n", status );
            break;
         }

         if (cmd == START_READ_CMD)
         {
            FD_SET( pSocket->mSocket, &inputSet );
            largestFD = std::max( pSocket->mSocket, 
                                  pSocket->mCommandPipe[READING] );
         }
         else if (cmd == STOP_READ_CMD)
         {
            FD_CLR( pSocket->mSocket, &inputSet );
            largestFD = pSocket->mCommandPipe[READING];
         }
         else
         {
            // EXIT_CMD or anything else
            pSocket->mpRxCallback = 0;
            break;
         }
      }
      else if (FD_ISSET( pSocket->mSocket, &outputSet ) == true)
      {
         // Stop watching for read data
         FD_CLR( pSocket->mSocket, &inputSet );
         largestFD = pSocket->mCommandPipe[READING];

         // Perform a recv for the header
         sQMUXDHeader recvHdr;
         status = recv( pSocket->mSocket,
                        &recvHdr,
                        sizeof( recvHdr ),
                        0 );         
         if (status != sizeof( recvHdr ))
         {
            TRACE( "recv error, bad size %d\n", status );
            break;
         }            

         // Calculate and read the remaining data
         int remainder = recvHdr.mTotalSize - sizeof( recvHdr );
         if (remainder > pSocket->mBuffSz)
         {
            TRACE( "read too large for buffer\n" );
            break;
         }

         status = recv( pSocket->mSocket,
                        pSocket->mpBuffer,
                        remainder,
                        0 );

         // Is this one of our IOCTLS or a standard message?
         if (recvHdr.mQMUXDMsgID == eQMUXD_MSG_WRITE_QMI_SDU)
         {
            cIOCallback * pCallback = pSocket->mpRxCallback;
            pSocket->mpRxCallback = 0;

            if (pCallback == (cIOCallback *)1)
            {
               // We wanted to read, but not to be notified
            }   
            else if (status >= 0)
            {
               pCallback->IOComplete( 0, status );
            }
            else
            {
               pCallback->IOComplete( status, 0 );
            }
         }
         else
         {
            pSocket->mpRxCallback = 0;
            // Notify SendCtl() that control message completed

            if (recvHdr.mQMUXDMsgID == eQMUXD_MSG_ALLOC_QMI_CLIENT_ID)
            {
               DWORD clientID;
               memcpy( &clientID, &pSocket->mpBuffer[0], 4 );

               pSocket->mCtrlMsgComplete.Set( clientID );
            }
            else
            {
               // Just set the event
               pSocket->mCtrlMsgComplete.Set( 0 );
            }
         }
      }
   }

   return 0;
};

/*=========================================================================*/
// cSocket Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cSocket (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cSocket::cSocket()
   :  mSocket( INVALID_HANDLE_VALUE ),
      mbCancelWrite( false ),
      mpBuffer( 0 ),
      mBuffSz( 0 ),
      mRxThreadID( 0 ),
      mCtrlMsgComplete(),
      mQMUXDClientID( 0 ),
      mQMUXClientID( 0 ),
      mQMUXServiceID( 0 ),
      mChannelID( -1 ),
      mQMUXDTxID( 0 )
{
   mCommandPipe[READING] = INVALID_HANDLE_VALUE;
   mCommandPipe[WRITING] = INVALID_HANDLE_VALUE;
}

/*===========================================================================
METHOD:
   ~cSocket (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cSocket::~cSocket()
{
   // Disconnect from current port
   Disconnect();

   mCommandPipe[READING] = INVALID_HANDLE_VALUE;
   mCommandPipe[WRITING] = INVALID_HANDLE_VALUE;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?

RETURN VALUE:
   Bool
===========================================================================*/
bool cSocket::IsValid()
{
   // Nothing to do
   return true;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified port

PARAMETERS:
   pChannel       [ I ] - Channel number (IE: "0" = SMD_DATA_5 )

RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::Connect( LPCSTR pChannel )
{
   if (IsValid() == false || pChannel == 0 || pChannel[0] == 0)
   {
      return false;
   }

   if (mSocket != INVALID_HANDLE_VALUE)
   {
      Disconnect();
   }

   // Initialize command pipe for read thread
   int nRet = pipe( mCommandPipe );
   if (nRet != 0)
   {
      TRACE( "cSocket:Connect() pipe creation failed %d\n", nRet );
      return false;
   }

   // Start the read thread
   nRet = pthread_create( &mRxThreadID,
                          0,
                          RxSocketThread,
                          this );
   if (nRet != 0)
   {
      TRACE( "cSocket::Connect() pthread_create = %d\n",  nRet );

      Disconnect();
      return false;
   }

   // Create a socket
   mSocket = socket( AF_UNIX, SOCK_STREAM, 0 );
   if (mSocket == INVALID_HANDLE_VALUE)
   {
      TRACE( "unable to create socket %d\n", errno );

      Disconnect();
      return false;
   }

   struct sockaddr_un clientSockAddr;
   memset( &clientSockAddr, 0, sizeof( clientSockAddr ) );
   clientSockAddr.sun_family = AF_UNIX;

   // Format the client path
   snprintf( &clientSockAddr.sun_path[0], 
             sizeof( clientSockAddr.sun_path ),
             "/var/qmux_client_socket%7lu",
             (unsigned long)getpid() );

   // Delete if it exists already
   unlink( clientSockAddr.sun_path );

   // Bind to a client address
   nRet = bind( mSocket, 
                (struct sockaddr *)&clientSockAddr,
                sizeof( sockaddr_un ) );
   if (nRet == -1)
   {
      TRACE( "bad bind %d\n", errno );

      Disconnect();
      return false;
   }

   // Format the connection path
   struct sockaddr_un connectSockAddr;
   memset( &connectSockAddr, 0, sizeof( connectSockAddr ) );
   connectSockAddr.sun_family = AF_UNIX;

   snprintf( &connectSockAddr.sun_path[0], 
             sizeof( connectSockAddr.sun_path ),
             "/var/qmux_connect_socket" );

   // Connect to server address
   nRet = connect( mSocket,
                   (struct sockaddr *)&connectSockAddr,
                   sizeof( sockaddr_un ) );
   if (nRet < 0)
   {
      TRACE( "bad connect %d\n", errno );

      Disconnect();
      return false;
   }

   int clientID;
   nRet = recv( mSocket, &clientID, sizeof( clientID ), 0 );
   if (nRet != sizeof( clientID ))
   {
      printf( "bad client ID %d\n", errno );
      
      Disconnect();
      return false;
   }

   // Save QMUXD Client ID
   mQMUXDClientID = clientID;

   // Save SMD channel
   mChannelID = strtol( pChannel, 0, 10 );
   if (mChannelID == -1)
   {
      Disconnect();
      return false;
   }

   // Success!
   return true;
}

/*===========================================================================
METHOD:
   SendCtl (Public Method)

DESCRIPTION:
   Send a control message to the lower layer

PARAMETERS:
   msgType  [ I ] - eQMUXDMessageType
   pData    [I/O] - input or output specific to ioctl request value

RETURN VALUE:
   int - control message return value (0 for success)
===========================================================================*/
int cSocket::SendCtl(
   UINT     msgType,
   void *   pData )
{
   if (mSocket == INVALID_HANDLE_VALUE)
   {
      TRACE( "Invalid file handle\n" );
      return -EBADFD;
   }

   BYTE msg[sizeof( sQMUXDHeader ) + PAYLOAD_SIZE];
   memset( &msg[0], 0, sizeof( msg ) );

   // The important QMUXD header values
   sQMUXDHeader * pHdr = (sQMUXDHeader *)&msg[0];
   pHdr->mTotalSize = sizeof( msg );
   pHdr->mQMUXDClientID = mQMUXDClientID;
   pHdr->mQMUXDMsgID = (eQMUXDMessageTypes)msgType;
   pHdr->mQMUXDClientIDDuplicate = mQMUXDClientID;
   
   // mQMUXDTxID could go to INT_MAX, but rather than dealing with possible
   // overflow in qmuxd or one of the lower layers, we'll stop early
   mQMUXDTxID++;
   if (mQMUXDTxID > 100000)
   {
      mQMUXDTxID = 1;
   }
   pHdr->mTxID = ++mQMUXDTxID;

   // The Payload
   BYTE * pPayload = &msg[sizeof( sQMUXDHeader )];
   if (msgType == (int)eQMUXD_MSG_ALLOC_QMI_CLIENT_ID)
   {
      memcpy( &mQMUXServiceID, pData, 4 );
      memcpy( &pPayload[0], &mQMUXServiceID, 4 );
   }
   else if (msgType == (int)eQMUXD_MSG_RELEASE_QMI_CLIENT_ID)
   {
      memcpy( &pPayload[0], &mQMUXServiceID, 4 );
      memcpy( &pPayload[4], &mQMUXClientID, 4 );
   }

   // Send the message
   int rc = send( mSocket, &msg[0], sizeof( msg ), 0 );
   if (rc != sizeof( msg ))
   {
      TRACE( "bad write %d\n", rc );
      return rc;
   }

   if (mpRxCallback == 0)
   {
      // No one is currently reading, need to trigger a read
      // so our data can be recieved
      RxData( &msg[0], sizeof( msg ), 0 );
   }

   // Wait for the response (10s timeout)
   DWORD val;
   rc = mCtrlMsgComplete.Wait( 10000, val );
   if (rc != 0)
   {
      TRACE( "bad SendCtl() wait %d\n", rc );
      return rc;
   }

   if (msgType == (int)eQMUXD_MSG_ALLOC_QMI_CLIENT_ID)
   {
      // Grab the client ID
      mQMUXClientID = val;
   }
   
   return 0;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Disconnect from the current port
  
RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::Disconnect()
{
   // Assume success
   bool bRC = true;

   if (mCommandPipe[WRITING] != INVALID_HANDLE_VALUE)
   {
      if (mRxThreadID != 0)
      {
         // Notify the thread to exit
         BYTE byte = EXIT_CMD;
         write( mCommandPipe[WRITING], &byte, 1 );

         // And wait for it
         int nRC = pthread_join( mRxThreadID, 0 );
         if (nRC != 0)
         {
            TRACE( "failed to join thread %d\n", nRC );
            bRC = false;
         }

         mRxThreadID = 0;
      }

      close( mCommandPipe[WRITING] );
      close( mCommandPipe[READING] );
      mCommandPipe[READING] = INVALID_HANDLE_VALUE;
      mCommandPipe[WRITING] = INVALID_HANDLE_VALUE;
   }

   if (mSocket != INVALID_HANDLE_VALUE)
   {
      close( mSocket );
      mSocket = INVALID_HANDLE_VALUE;
   }

   // Double check
   mpRxCallback = 0;

   mCtrlMsgComplete.Clear();
   mQMUXDClientID = 0;
   mQMUXClientID = 0;
   mQMUXServiceID = 0;
   mQMUXDTxID = 0;
   return bRC;
}

/*===========================================================================
METHOD:
   CancelIO (Public Method)

DESCRIPTION:
   Cancel any in-progress I/O

PARAMETERS:

RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::CancelIO()
{
   if (mSocket == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   bool bRxCancel = CancelRx();
   bool bTxCancel = CancelTx();

   return (bRxCancel && bTxCancel);
}

/*===========================================================================
METHOD:
   CancelRx (Public Method)

DESCRIPTION:
   Cancel any in-progress receive operation
  
RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::CancelRx()
{
   if (mSocket == INVALID_HANDLE_VALUE
   ||  mCommandPipe[WRITING] == INVALID_HANDLE_VALUE
   ||  mpRxCallback == 0
   ||  mRxThreadID == 0)
   {
      TRACE( "cannot cancel, thread not active\n" );
      mpRxCallback = 0;
      return false;
   }

   // Notify the thread to stop reading
   BYTE byte = STOP_READ_CMD;
   int nRC = write( mCommandPipe[WRITING], &byte, 1 );
   if (nRC != 1)
   {
      TRACE( "error %d canceling read\n", nRC );
      return false;
   }
   
   // Remove the old callback
   mpRxCallback = 0;

   return true;
}

/*===========================================================================
METHOD:
   CancelTx (Public Method)

DESCRIPTION:
   Cancel any in-progress transmit operation
  
RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::CancelTx()
{
   if (mSocket == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   mbCancelWrite = true;

   return true;
}

/*===========================================================================
METHOD:
   RxData (Public Method)

DESCRIPTION:
   Receive data

PARAMETERS:
   pBuf        [ I ] - Buffer to contain received data
   bufSz       [ I ] - Amount of data to be received
   pCallback   [ I ] - Callback object to be exercised when the
                       operation completes

RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::RxData(
   BYTE *                     pBuf, 
   ULONG                      bufSz,
   cIOCallback *              pCallback )
{
   if (IsValid() == false || mpRxCallback != 0)
   {
      return false;
   }

   if (pCallback == 0)
   {
      // Not interested in being notified, but we still need a value
      // for this so that only one outstanding I/O operation is active
      // at any given point in time
      mpRxCallback = (cIOCallback * )1;
   }
   else
   {
      mpRxCallback = pCallback;
   }

   mpBuffer = pBuf;
   mBuffSz = bufSz;

   // Notify the thread to start reading
   BYTE byte = START_READ_CMD;
   int nRC = write( mCommandPipe[WRITING], &byte, 1 );
   if (nRC != 1)
   {
      TRACE( "error %d starting read\n", nRC );
      return false;
   }

   return true;
}

/*===========================================================================
METHOD:
   TxData (Public Method)

DESCRIPTION:
   Transmit data

PARAMETERS:
   pBuf        [ I ] - Data to be transmitted
   bufSz       [ I ] - Amount of data to be transmitted

RETURN VALUE:
   bool
===========================================================================*/
bool cSocket::TxData(
   const BYTE *               pBuf, 
   ULONG                      bufSz )
{
   if (IsValid() == false)
   {
      return false;
   }
   
#ifdef DEBUG
   ULONGLONG nStart = GetTickCount();
#endif

   // Allow ourselves to be interupted
   mbCancelWrite = false;

   // Format the header
   int totalSz = sizeof( sQMUXDHeader ) + bufSz;
   BYTE * pMsg = new BYTE[totalSz];
   if (pMsg == 0)
   {
      TRACE( "unable to allocate buffer\n" );
      return false;
   }
   memset( pMsg, 0, totalSz );

   // The important QMUXD header values
   sQMUXDHeader * pHdr = (sQMUXDHeader *)pMsg;
   pHdr->mTotalSize = totalSz;
   pHdr->mQMUXDClientID = mQMUXDClientID;
   pHdr->mQMUXDMsgID = eQMUXD_MSG_WRITE_QMI_SDU;
   pHdr->mQMUXDClientIDDuplicate = mQMUXDClientID;

   // mQMUXDTxID could go to INT_MAX, but rather than dealing with possible
   // overflow in qmuxd or one of the lower layers, we'll stop early
   mQMUXDTxID++;
   if (mQMUXDTxID > 100000)
   {
      mQMUXDTxID = 1;
   }
   pHdr->mTxID = ++mQMUXDTxID;

   pHdr->mQMUXServiceID = mQMUXServiceID;
   pHdr->mQMUXClientID = mQMUXClientID;

   // The data payload
   memcpy( &pMsg[sizeof( sQMUXDHeader )], pBuf, bufSz );

   // Send the message
   int nRet = send( mSocket, pMsg, totalSz, 0 );
   delete [] pMsg;
   if (nRet != totalSz)
   {
      TRACE( "cSocket::TxData() write returned %d instead of %d\n",
             nRet,
             totalSz );
      return false;
   }

#ifdef DEBUG
   TRACE( "Write of %d bytes took %llu miliseconds\n", 
          totalSz, 
          GetTickCount() - nStart );
#endif

   return true;
}
