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

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   RxCompletionRoutine (Free Method)

DESCRIPTION:
   Completion routine for receive operation, exercises current receive 
   callback object

PARAMETERS:
   returnSignal [ I/O ]   Asynchronus signal event 
                          (contains pointer to cCOMM object)

RETURN VALUE:
   None
===========================================================================*/
VOID RxCompletionRoutine( sigval returnSignal )
{
   cComm * pComm = (cComm *)returnSignal.sival_ptr;
      
   if (pComm == NULL || pComm->IsValid() == false)
   {
      return;
   }

   cIOCallback * pCallback = pComm->mpRxCallback;
   if (pCallback == 0)
   {
      // aio_cancel is broken if file pointer is bad
      // Notify cComm::CancelIO() manually
      TRACE( "%s manual notification %d\n", __func__, pComm->mPort );
      pComm->mReadCanceled.Set( 1 );
      
      return;
   }

   pComm->mpRxCallback = 0;
   if (pCallback != (cIOCallback *)1)
   {
      int nEC = aio_error( &pComm->mReadIO );
      int nBytesTransfered = aio_return( &pComm->mReadIO );

      pCallback->IOComplete( nEC, nBytesTransfered );
   }
}

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
      mReadCanceled()
{
   memset( &mReadIO, 0, sizeof( aiocb) );

   mReadIO.aio_sigevent.sigev_value.sival_ptr = this;
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

   mReadIO.aio_sigevent.sigev_value.sival_ptr = NULL;
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
   return (mReadIO.aio_sigevent.sigev_value.sival_ptr == this);
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

   // Opening the com port
   mPort = open( pPort, O_RDWR );
   if (mPort == INVALID_HANDLE_VALUE) 
   {
      return false;
   }

   // Clear any contents
   tcdrain( mPort );

   mReadCanceled.Clear();
 
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

   if (mPort != INVALID_HANDLE_VALUE)
   {
      int nClose = close( mPort );
      if (nClose == -1)
      {
         bRC = false;
      }

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
   if (mPort == INVALID_HANDLE_VALUE || mpRxCallback == 0)
   {
      return false;
   }

   int nReadRC = aio_cancel( mPort, &mReadIO );
   mpRxCallback = 0;

   if (nReadRC == -1 && errno == EBADF)
   {
      // aio_cancel is broken if file pointer is bad
      // wait for completion
      TRACE( "cComm::CancelRx manual wait %d\n", mPort );

      DWORD nTemp; 
      if (mReadCanceled.Wait( INFINITE, nTemp ) == 0)
      {
         return true;
      }

      // Timeout or some other failure
      return false;
   }

   return (nReadRC == AIO_CANCELED);
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

   mReadIO.aio_fildes = mPort;
   mReadIO.aio_buf = pBuf;
   mReadIO.aio_nbytes = bufSz;
   mReadIO.aio_sigevent.sigev_notify = SIGEV_THREAD;
   mReadIO.aio_sigevent.sigev_notify_function = RxCompletionRoutine;
   mReadIO.aio_sigevent.sigev_value.sival_ptr = this;
   mReadIO.aio_offset = 0;

   int nRet = aio_read( &mReadIO );
   if (nRet != 0)
   {
      TRACE( "cComm::RxData() = %d, %s\n", nRet, strerror( errno ) );
      errno = 0;
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
