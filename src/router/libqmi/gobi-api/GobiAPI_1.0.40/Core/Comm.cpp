/*===========================================================================
FILE:
   Comm.cpp

DESCRIPTION:
   Implementation of cComm class

PUBLIC CLASSES AND METHODS:
   cComm
      This class wraps low level port communications

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
#include "Comm.h"
#include "ProtocolServer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
// Thread commands
#define START_READ_CMD  0
#define STOP_READ_CMD   1
#define EXIT_CMD        2

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   RxThread (Free Method)

DESCRIPTION:
   Thread for simulating asynchronous reads

PARAMETERS:
   pData      [ I ]   Asynchronous read object

RETURN VALUE:
   void * - thread exit value (always 0)
===========================================================================*/
void * RxThread( void * pData )
{
   cComm * pComm = (cComm*)pData;
   if (pComm == NULL || pComm->IsValid() == false)
   {
      return 0;
   }

   fd_set inputSet, outputSet;
   FD_ZERO( &inputSet );
   FD_SET( pComm->mCommandPipe[READING], &inputSet );
   int largestFD = pComm->mCommandPipe[READING];

   int status = 0;
   while (true)
   {
      // No FD_COPY() available
      memcpy( &outputSet, &inputSet, sizeof( fd_set ) );

      status = select( largestFD + 1, &outputSet, NULL, NULL, NULL );
      if (status <= 0)
      {
         TRACE( "error %d in select, errno %d\n", status, errno );
         break;
      }

      if (FD_ISSET( pComm->mCommandPipe[READING], &outputSet ) == true)
      {
         // Read from the pipe
         BYTE cmd;
         status = read( pComm->mCommandPipe[READING], &cmd, 1 );
         if (status != 1)
         {
            TRACE( "cmd error %d\n", status );
            break;
         }

         if (cmd == START_READ_CMD)
         {
            FD_SET( pComm->mPort, &inputSet );
            largestFD = std::max( pComm->mPort, 
                                  pComm->mCommandPipe[READING] );
         }
         else if (cmd == STOP_READ_CMD)
         {
            FD_CLR( pComm->mPort, &inputSet );
            largestFD = pComm->mCommandPipe[READING];
         }
         else
         {
            // EXIT_CMD or anything else
            break;
         }
      }
      else if (FD_ISSET( pComm->mPort, &outputSet ) == true)
      {
         // Stop watching for read data
         FD_CLR( pComm->mPort, &inputSet );
         largestFD = pComm->mCommandPipe[READING];

         // Perform a read
         status = read( pComm->mPort,
                        pComm->mpBuffer,
                        pComm->mBuffSz );

         cIOCallback * pCallback = pComm->mpRxCallback;
         pComm->mpRxCallback = 0;

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
   }

   return 0;
};

/*=========================================================================*/
// cComm Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cComm (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cComm::cComm()
   :  mPortName( "" ),
      mPort( INVALID_HANDLE_VALUE ),
      mpRxCallback( 0 ),
      mbCancelWrite( false ),
      mpBuffer( 0 ),
      mBuffSz( 0 ),
      mRxThreadID( 0 )
{
   mCommandPipe[READING] = INVALID_HANDLE_VALUE;
   mCommandPipe[WRITING] = INVALID_HANDLE_VALUE;
}

/*===========================================================================
METHOD:
   ~cComm (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cComm::~cComm()
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
bool cComm::IsValid()
{
   // Nothing to do, dependant on extended class functionality
   return true;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified port

PARAMETERS:
   pPort       [ I ] - Name of port to open (IE: /dev/qcqmi0)

RETURN VALUE:
   bool
===========================================================================*/
bool cComm::Connect( LPCSTR pPort )
{
   if (IsValid() == false || pPort == 0 || pPort[0] == 0)
   {
      return false;
   }

   if (mPort != INVALID_HANDLE_VALUE)
   {
      Disconnect();
   }

   // Initialize command pipe for read thread
   int nRet = pipe( mCommandPipe );
   if (nRet != 0)
   {
      TRACE( "cComm:Connect() pipe creation failed %d\n", nRet );
      return false;
   }

   // Start the read thread
   nRet = pthread_create( &mRxThreadID,
                          0,
                          RxThread,
                          this );
   if (nRet != 0)
   {
      TRACE( "cComm::Connect() pthread_create = %d\n",  nRet );

      Disconnect();
      return false;
   }

   // Opening the com port
   mPort = open( pPort, O_RDWR );
   if (mPort == INVALID_HANDLE_VALUE) 
   {
      Disconnect();
      return false;
   }

   // Save port name
   mPortName = pPort;

   // Success!
   return true;
}

/*===========================================================================
METHOD:
   RunIOCTL (Public Method)

DESCRIPTION:
   Run an IOCTL on the open file handle

PARAMETERS:
   ioctlReq [ I ] - ioctl request value
   pData    [I/O] - input or output specific to ioctl request value

RETURN VALUE:
   int - ioctl return value (0 for success)
===========================================================================*/
int cComm::RunIOCTL(
   UINT     ioctlReq,
   void *   pData )
{
   if (mPort == INVALID_HANDLE_VALUE)
   {
      TRACE( "Invalid file handle\n" );
      return -EBADFD;
   }
   
   return ioctl( mPort, ioctlReq, pData );
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Disconnect from the current port
  
RETURN VALUE:
   bool
===========================================================================*/
bool cComm::Disconnect()
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
         TRACE( "cComm::Disconnnect() joining thread\n" );
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

   if (mPort != INVALID_HANDLE_VALUE)
   {
      close( mPort );
      mPort = INVALID_HANDLE_VALUE;
   }

   mPortName.clear();
   return bRC;
}

/*===========================================================================
METHOD:
   ConfigureSettings (Public Method)

DESCRIPTION:
   Configure the port with the passed in parameters

PARAMETERS:
   pSettings   [ I ] - Desired port settings

RETURN VALUE:
   bool
===========================================================================*/
bool cComm::ConfigureSettings( termios * pSettings )
{
   if (mPort == INVALID_HANDLE_VALUE || pSettings == 0)
   {
      return false;
   }

   tcflush( mPort, TCIOFLUSH );
   int nRC = tcsetattr( mPort, TCSANOW, pSettings );
   if (nRC == -1)
   {
      return false;
   }

   // Success!
   return true;
}

/*===========================================================================
METHOD:
   GetSettings (Public Method)

DESCRIPTION:
   Return the current port settings

PARAMETERS:
   pSettings   [ I ] - Current port settings

RETURN VALUE:
   bool
===========================================================================*/
bool cComm::GetSettings( termios * pSettings )
{
   if (mPort == INVALID_HANDLE_VALUE || pSettings == 0)
   {
      return false;
   }

   // Get the COM port settings
   int nRC = tcgetattr( mPort, pSettings );
   if (nRC == -1)
   {
      return false;
   }

   // Success!
   return true;
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
bool cComm::CancelIO()
{
   if (mPort == INVALID_HANDLE_VALUE)
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
bool cComm::CancelRx()
{
   if (mPort == INVALID_HANDLE_VALUE
   ||  mCommandPipe[WRITING] == INVALID_HANDLE_VALUE
   ||  mpRxCallback == 0
   ||  mRxThreadID == 0)
   {
      TRACE( "cannot cancel, thread not active\n" );
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
bool cComm::CancelTx()
{
   if (mPort == INVALID_HANDLE_VALUE)
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
bool cComm::RxData(
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

   // Notify the thread to stop reading
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
bool cComm::TxData(
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

   // This seems a bit pointless, but we're still going verify
   // the device is ready for writing, and give it up to
   // (1000 + num bytes) MS to be ready (in 100 MS chunks)
   
   struct timeval TimeOut;
   fd_set set;
   
   int nReady = 0;
   int nCount = 0;
   
   while ( nReady == 0 )
   {
      if (mbCancelWrite == true)
      {
         TRACE( "cComm::TxData() write canceled before device was ready\n" );
         return false;
      }
      
      if (nCount >= (1000 + bufSz) / 100)   
      {
         // Timeout is expired
         break;
      }
      
      FD_ZERO( &set );
      FD_SET( mPort, &set );
      TimeOut.tv_sec = 0;
      TimeOut.tv_usec = 100000;
      nReady = select( mPort + 1, NULL, &set, NULL, &TimeOut );
      
      nCount++;
   }
      
   if (nReady <= 0)
   {
      TRACE( "cComm::TxData() Unable to get device ready for"
             " Write, error %d: %s\n",
             nReady,
             strerror( nReady) );
      return false;
   }
   
   int nRet = write( mPort, pBuf, bufSz );
   if (nRet != bufSz)
   {
      TRACE( "cComm::TxData() write returned %d instead of %lu\n",
             nRet,
             bufSz );
      return false;
   }

#ifdef DEBUG
   TRACE( "Write of %lu bytes took %llu miliseconds\n", bufSz, GetTickCount() - nStart );
#endif

   return true;
}
