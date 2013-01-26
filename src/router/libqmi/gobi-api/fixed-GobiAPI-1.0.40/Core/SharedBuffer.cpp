/*===========================================================================
FILE:
   SharedBuffer.cpp

DESCRIPTION:
   Shareable protocol structures and affliated methods
   
PUBLIC CLASSES AND METHODS:

   sSharedBuffer
      Simple struct to represent a reference counted shareable (no copy)
      buffer, as the basis for all buffer related classes

   sDiagBuffer
      Simple struct to represent a DIAG buffer using a reference counted
      (shared) buffer, this allows us to use in in several places without
      copying it once in each place.  A few base services are provided
      but the main purpose is to provide a class to inherit off of for
      DIAG command code specific DIAG buffers

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
#include "SharedBuffer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Synchronization object
struct sSharedBufferSync
{
   public:
      // Constructor
      sSharedBufferSync()
         :  mbInitialized( false )
      {
         int nRet = pthread_mutex_init( &mSyncSection, NULL );
         if (nRet != 0)
         {
            TRACE( "SharedBuffer: Unable to init sync mutex."
                   " Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
            return;
         }         

         mbInitialized = true;
      };

      // Destructor
      ~sSharedBufferSync()
      {
         mbInitialized = false;
         int nRet = pthread_mutex_destroy( &mSyncSection );
         if (nRet != 0)
         {
            TRACE( "SharedBuffer: Unable to destroy sync mutex."
                   " Error %d: %s\n",
                   nRet,
                   strerror( nRet ) );
         }         

      };

      // Lock sync object
      void Lock()
      {
         if (mbInitialized == true)
         {
            int nRet = pthread_mutex_lock( &mSyncSection );
            if (nRet != 0)
            {
               TRACE( "SharedBuffer: Unable to lock sync mutex."
                      " Error %d: %s\n",
                      nRet,
                      strerror( nRet ) );
               return;
            }            

         }
      };

      // Unlock sync object
      void Unlock()
      {
         if (mbInitialized == true)
         {
            int nRet = pthread_mutex_unlock( &mSyncSection );
            if (nRet != 0)
            {
               TRACE( "SharedBuffer: Unable to unlock sync mutex."
                      " Error %d: %s\n",
                      nRet,
                      strerror( nRet ) );
               return;
            }            

         }
      };
   
   protected:
      /* DIAG buffer critical section */
      pthread_mutex_t mSyncSection;

      /* Has this object been initialized? */
      bool mbInitialized;
};

// Global (across all shared buffers) reference count guard
sSharedBufferSync gRefCount;

/*=========================================================================*/
// sSharedBuffer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sSharedBuffer (Public Method)

DESCRIPTION:
   Constructor (copy passed in buffer)

PARAMETERS:
   pDataToCopy [ I ] - The data buffer to copy (should be non-zero)
   dataLen     [ I ] - The length of the above buffer (should be > 1)
   dataType    [ I ] - Type of data (not used internal to class)
  
RETURN VALUE:
   None
===========================================================================*/
sSharedBuffer::sSharedBuffer( 
   const BYTE *               pDataToCopy,
   ULONG                      dataLen,
   ULONG                      dataType )
   :  mpData( 0 ),
      mSize( 0 ),
      mType( dataType ),
      mRefCount( 0 )
{
   // Length not too small/not too big?
   if (IsValidSize( dataLen ) == true)
   {
      // Yes, data actually exists?
      if (pDataToCopy != 0)
      {
         // Yes, try to allocate memory
         mpData = new BYTE[dataLen];
         if (mpData != 0)
         {
            // Now copy into our allocation
            memcpy( (PVOID)mpData, 
                          (LPCVOID)pDataToCopy, 
                          (SIZE_T)dataLen );

            // Now set the size, we do this last so that our double 
            // deletion logic is only applied if we had an allocation 
            // in the first place
            mSize = dataLen;
         }
      }
   }
}

/*===========================================================================
METHOD:
   sSharedBuffer (Public Method)

DESCRIPTION:
   Constructor (assume ownership of passed in buffer)

PARAMETERS:
   dataLen     [ I ] - The length of the above buffer (should be > 1)
   pDataToOwn  [ I ] - The data buffer to assume ownership of (should 
                       be non-zero)

   dataType    [ I ] - Type of data (not used internal to class)

   NOTE: The order is intentionally reversed from the previous constructor
   to avoid any cases of mistaken identity (copy versus assume ownership)
  
RETURN VALUE:
   None
===========================================================================*/
sSharedBuffer::sSharedBuffer( 
   ULONG                      dataLen,
   PBYTE                      pDataToOwn,
   ULONG                      dataType )
   :  mpData( 0 ),
      mSize( 0 ),
      mType( dataType ),
      mRefCount( 0 )
{
   // Data actually exists?
   if (pDataToOwn != 0)
   {
      // Yes, length not too small/not too big?
      if (IsValidSize( dataLen ) == true)
      {
         // Yes, assume ownership of the passed in buffer
         mpData = pDataToOwn;
         mSize = dataLen;        
      }
      else
      {
         // This data buffer is not acceptable to us, but we have assumed
         // ownership of the memory which we will now free
         delete [] pDataToOwn;
      }
   }
}

/*===========================================================================
METHOD:
   ~sSharedBuffer (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sSharedBuffer::~sSharedBuffer()
{
   ASSERT( mRefCount == 0 );

   // Buffer data to free?
   if (mpData != 0)
   {
      // Yes, zero first byte for caution and then delete it
      mpData[0] = 0;
      delete [] mpData;

      // Even more caution, zero out pointer
      mpData = 0;
   }
   else if (mSize != 0)
   {
      ASSERT( (PVOID)("Double deletion detected in ~sSharedBuffer") == 0 );
   }
}

/*===========================================================================
METHOD:
   operator == (Public Method)

DESCRIPTION:
   Equality operator
  
RETURN VALUE:
   bool
===========================================================================*/
bool sSharedBuffer::operator == ( const sSharedBuffer & refBuf ) const
{
   // Assume they are not equal
   bool bEq = false;

   // The buffers must be the same
   if (mpData == refBuf.mpData)
   {
      if (mSize == refBuf.mSize)
      {
         if (mRefCount == refBuf.mRefCount)
         {
            if (mType == refBuf.mType)
            {
               // The shared buffers are the same
               bEq = true;
            }
         }
         else
         {
            // Very odd - the buffers are the same, but not the ref count?!?
            ASSERT( 0 );
         }
      }
      else
      {
         // Very odd - the buffers are the same, but not the size?!?
         ASSERT( 0 );
      }
   }

   return bEq;
}

/*===========================================================================
METHOD:
   operator != (Public Method)

DESCRIPTION:
   Inequality operator
  
RETURN VALUE:
   bool
===========================================================================*/
bool sSharedBuffer::operator != ( const sSharedBuffer & refBuf ) const
{
   if (*this == refBuf)
   {
      return false;
   }

   return true;
}

/*===========================================================================
METHOD:
   AddRef (Internal Method)

DESCRIPTION:
   Increment reference count
  
RETURN VALUE:
   None
===========================================================================*/
void sSharedBuffer::AddRef()
{
   gRefCount.Lock();
   mRefCount++;     
   gRefCount.Unlock();
}

/*===========================================================================
METHOD:
   Release (Internal Method)

DESCRIPTION:
   Release reference, delete if reference count zero
  
RETURN VALUE:
   None
===========================================================================*/
void sSharedBuffer::Release()
{
   gRefCount.Lock();

   ASSERT( mRefCount != 0 );

   // Decrement reference count
   if (mRefCount > 0)
   {
      mRefCount--;
   }

   // ... and delete if reference count now 0
   if (mRefCount == 0)
   {
      delete this;
   }

   gRefCount.Unlock();
}
