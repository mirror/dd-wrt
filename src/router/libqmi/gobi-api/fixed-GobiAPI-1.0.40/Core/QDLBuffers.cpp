/*===========================================================================
FILE:
   QDLBuffers.cpp

DESCRIPTION:
   QDL protocol related structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sQDLRawHelloReq
   sQDLRawHelloRsp
   sQDLRawErrorRsp
   sQDLRawOpenUnframedReq
   sQDLRawOpenUnframedRsp
   sQDLRawWriteUnframedReq
   sQDLRawWriteUnframedRsp
   sQDLRawDoneRsp
   sQDLRawGetImagePrefRsp
   sQDLRawImageID

   sQDLHello
   sQDLError
   sQDLOpenUnframed
   sQDLWriteUnframed
   sQDLDone
   sQDLGetImagePref

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
#include "QDLBuffers.h"
#include "CRC.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// sQDLHello Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLHello (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLHello::sQDLHello( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLHello::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLHello (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLHello::~sQDLHello() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this hello request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLHello::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX && pt != ePROTOCOL_QDL_TX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szReq = (ULONG)sizeof(sQDLRawHelloReq);
   const ULONG szRsp = (ULONG)sizeof(sQDLRawHelloRsp);

   if (pt == ePROTOCOL_QDL_TX && sz == szReq)
   {
      const sQDLRawHelloReq * pReq = (const sQDLRawHelloReq *)GetBuffer();
      if (pReq->mCommandCode != (BYTE)eQDL_CMD_HELLO_REQ)
      {
         return bRC;
      }

      int notEqual = memcmp( (const void *)&pReq->mMagicNumber[0],
                             (const void *)&QDL_HELLO_MAGIC_REQ[0],
                             sizeof( QDL_HELLO_MAGIC_REQ ) );

      if (notEqual != 0)
      {
         return bRC;
      }

      bRC = true;
   }
   else if (pt == ePROTOCOL_QDL_RX && sz == szRsp)
   {
      const sQDLRawHelloRsp * pRsp = (const sQDLRawHelloRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_HELLO_RSP)
      {
         return bRC;
      }

      int notEqual = memcmp( (const void *)&pRsp->mMagicNumber[0],
                             (const void *)&QDL_HELLO_MAGIC_RSP[0],
                             sizeof( QDL_HELLO_MAGIC_RSP ) );

      if (notEqual != 0)
      {
         return bRC;
      }

      if ( (pRsp->mMaxVersion != QDL_MIN_VERSION) 
      ||   (pRsp->mMinVersion != QDL_MAX_VERSION) )
      {
         return bRC;
      }

      if ( ((pRsp->mFeatures & QDL_FEATURE_GENERIC_UNFRAMED) == 0)
      ||   ((pRsp->mFeatures & QDL_FEATURE_QDL_UNFRAMED) == 0) )
      {
         return bRC;
      }

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*===========================================================================
METHOD:
   GetBootVersionInfo (Internal Method)

DESCRIPTION:
   Extract boot downloader version info from the response
  
PARAMETERS:
   major       [ O ] - Major version
   minor       [ O ] - Minor version

RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLHello::GetBootVersionInfo(
   ULONG &                    major,
   ULONG &                    minor ) const
{
   // Assume failure
   bool bRC = false;

   major = 0;
   minor = 0;

   const sQDLRawHelloRsp * pRsp = GetResponse();
   if (pRsp == 0)
   {
      return bRC;
   }

   major = (ULONG)pRsp->mBootMajorVersion;
   minor = (ULONG)pRsp->mBootMinorVersion;

   bRC = true;
   return bRC;
}

/*===========================================================================
METHOD:
   BuildHelloReq (Static Public Method)

DESCRIPTION:
   Build a hello request

PARAMETERS:
   bBARMode    [ O ] - Request boot and recovery mode feature

RETURN VALUE:
   sSharedBuffer * : The request in an allocated buffer (0 on error)
===========================================================================*/ 
sSharedBuffer * sQDLHello::BuildHelloReq( bool bBARMode )
{
   const ULONG sz = (ULONG)sizeof(sQDLRawHelloReq);
   BYTE req[sz];

   sQDLRawHelloReq * pReq = (sQDLRawHelloReq *)&req[0];

   pReq->mCommandCode = (BYTE)eQDL_CMD_HELLO_REQ;
   pReq->mMaxVersion  = QDL_MIN_VERSION;
   pReq->mMinVersion  = QDL_MAX_VERSION;
   pReq->mFeatures    = QDL_FEATURE_GENERIC_UNFRAMED | QDL_FEATURE_QDL_UNFRAMED;

   if (bBARMode == true)
   {
      pReq->mFeatures |= QDL_FEATURE_BAR_MODE;
   }

   memcpy( (PVOID)&pReq->mMagicNumber[0],
           (const VOID *)&QDL_HELLO_MAGIC_REQ[0],
           (SIZE_T)32 );

   eProtocolType pt = ePROTOCOL_QDL_TX;
   sSharedBuffer * pRetBuf = new sSharedBuffer( (const BYTE *)req, sz, pt );
   return pRetBuf;
}

/*=========================================================================*/
// sQDLError Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLError (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLError::sQDLError( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLError::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLError (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLError::~sQDLError() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this session done request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLError::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szRsp = (ULONG)sizeof( sQDLRawErrorRsp );

   if (sz >= szRsp)
   {
      const sQDLRawErrorRsp * pRsp = 0;
      pRsp = (const sQDLRawErrorRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_ERROR)
      {
         return bRC;
      }

      // Error code needs to be valid
      if (::IsValid( (eQDLError)pRsp->mErrorCode ) == false)
      {
         return bRC;
      }

      // Error text needs to be NULL terminated
      const BYTE * pTmp = GetBuffer();
      if (pTmp[sz - 1] != 0)
      {
         return bRC;
      }

      // What there is of the error text needs to be printable
      pTmp = &pRsp->mErrorText;
      while (*pTmp != 0)
      {
         int val = (int)*pTmp++;
         if (isprint( (int)val ) == 0)
         {
            return bRC;
         }
      }

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*=========================================================================*/
// sQDLOpenUnframed Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLOpenUnframed (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLOpenUnframed::sQDLOpenUnframed( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLOpenUnframed::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLOpenUnframed (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLOpenUnframed::~sQDLOpenUnframed() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this open unframed request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLOpenUnframed::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX && pt != ePROTOCOL_QDL_TX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szReq = (ULONG)sizeof(sQDLRawOpenUnframedReq);
   const ULONG szRsp = (ULONG)sizeof(sQDLRawOpenUnframedRsp);

   if (pt == ePROTOCOL_QDL_TX && sz == szReq)
   {
      const sQDLRawOpenUnframedReq * pReq = 0;
      pReq = (const sQDLRawOpenUnframedReq *)GetBuffer();
      if (pReq->mCommandCode != (BYTE)eQDL_CMD_OPEN_UNFRAMED_REQ)
      {
         return bRC;
      }

      if (::IsValid( (eQDLImageType)pReq->mImageType ) == false)
      {
         return bRC;
      }

      if (pReq->mWindowSize != 1)
      {
         return bRC;
      }

      bRC = true;
   }
   else if (pt == ePROTOCOL_QDL_RX && sz == szRsp)
   {
      const sQDLRawOpenUnframedRsp * pRsp = 0;
      pRsp = (const sQDLRawOpenUnframedRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_OPEN_UNFRAMED_RSP)
      {
         return bRC;
      }

      if (pRsp->mWindowSize != 1)
      {
         return bRC;
      }

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*===========================================================================
METHOD:
   GetChunkSize (Internal Method)

DESCRIPTION:
   Extract chunk size info from the response
  
PARAMETERS:
   chunkSize   [ O ] - Target supported chunk size
   
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLOpenUnframed::GetChunkSize( ULONG & chunkSize ) const
{
   // Assume failure
   bool bRC = false;

   chunkSize = 0;

   const sQDLRawOpenUnframedRsp * pRsp = GetResponse();
   if (pRsp == 0)
   {
      return bRC;
   }

   chunkSize = (ULONG)pRsp->mUnframedChunkSize;

   bRC = true;
   return bRC;
}

/*===========================================================================
METHOD:
   BuildOpenUnframedReq (Static Public Method)

DESCRIPTION:
   Build an open image for unframed write request

PARAMETERS:
   imageType   [ I ] - Type of image about to be written
   imageSize   [ I ] - Size of image about to be written
   chunkSize   [ I ] - Desired size of chunk for each write 

RETURN VALUE:
   sSharedBuffer * : The request in an allocated buffer (0 on error)
===========================================================================*/ 
sSharedBuffer * sQDLOpenUnframed::BuildOpenUnframedReq(
   eQDLImageType              imageType,
   ULONG                      imageSize,
   ULONG                      chunkSize )
{
   sSharedBuffer * pRetBuf = 0;   
   if (::IsValid( imageType ) == false)
   {
      return pRetBuf;
   }

   // We can not write out chunks larger than the maximum
   if (chunkSize > QDL_MAX_CHUNK_SIZE)
   {
      return pRetBuf;
   }

   const ULONG sz = (ULONG)sizeof(sQDLRawOpenUnframedReq);
   BYTE req[sz];

   memset( (LPVOID)&req[0], 0, (SIZE_T)sz );

   sQDLRawOpenUnframedReq * pReq = (sQDLRawOpenUnframedReq *)&req[0];

   pReq->mCommandCode       = (BYTE)eQDL_CMD_OPEN_UNFRAMED_REQ;
   pReq->mImageType         = (BYTE)imageType;
   pReq->mImageLength       = (DWORD)imageSize;
   pReq->mWindowSize        = 1;
   pReq->mUnframedChunkSize = chunkSize;

   eProtocolType pt = ePROTOCOL_QDL_TX;
   pRetBuf = new sSharedBuffer( (const BYTE *)req, sz, pt );
   return pRetBuf;
}

/*=========================================================================*/
// sQDLWriteUnframed Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLWriteUnframed (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLWriteUnframed::sQDLWriteUnframed( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLWriteUnframed::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLWriteUnframed (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLWriteUnframed::~sQDLWriteUnframed() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this unframed write request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLWriteUnframed::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX && pt != ePROTOCOL_QDL_TX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szReq = (ULONG)sizeof( sQDLRawWriteUnframedReq );
   const ULONG szRsp = (ULONG)sizeof( sQDLRawWriteUnframedRsp );

   if (pt == ePROTOCOL_QDL_TX && sz == szReq)
   {
      const sQDLRawWriteUnframedReq * pReq = 0;
      pReq = (const sQDLRawWriteUnframedReq *)GetBuffer();
      if (pReq->mCommandCode != (BYTE)eQDL_CMD_WRITE_UNFRAMED_REQ)
      {
         return bRC;
      }

      bRC = CheckCRC( GetBuffer(), szReq - sizeof( USHORT ) ); 
   }
   else if (pt == ePROTOCOL_QDL_RX && sz == szRsp)
   {
      const sQDLRawWriteUnframedRsp * pRsp = 0;
      pRsp = (const sQDLRawWriteUnframedRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_WRITE_UNFRAMED_RSP)
      {
         return bRC;
      }

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*===========================================================================
METHOD:
   GetSequenceNumber (Internal Method)

DESCRIPTION:
   Extract sequence number from the response
  
PARAMETERS:
   sequenceNumber [ O ] - Target reported sequence number
   
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLWriteUnframed::GetSequenceNumber( ULONG & sequenceNumber ) const
{
   // Assume failure
   bool bRC = false;

   sequenceNumber = 0;

   const sQDLRawWriteUnframedRsp * pRsp = GetResponse();
   if (pRsp == 0)
   {
      return bRC;
   }

   sequenceNumber = (ULONG)pRsp->mSequenceNumber;

   bRC = true;
   return bRC;
}

/*===========================================================================
METHOD:
   BuildWriteUnframedReq (Static Public Method)

DESCRIPTION:
   Build an unframed write request

PARAMETERS:
   sequenceNumber [ I ] - Type of image about to be written
   chunkSize      [ I ] - Size of chunk being written 

RETURN VALUE:
   sSharedBuffer * : The request in an allocated buffer (0 on error)
===========================================================================*/ 
sSharedBuffer * sQDLWriteUnframed::BuildWriteUnframedReq(
   USHORT                     sequenceNumber,
   ULONG                      chunkSize )
{
   sSharedBuffer * pRetBuf = 0;   

   // We can not write out chunks larger than the maximum
   if (chunkSize == 0 || chunkSize > (ULONG)QDL_MAX_CHUNK_SIZE)
   {
      return pRetBuf;
   }

   const ULONG sz = (ULONG)sizeof(sQDLRawWriteUnframedReq);
   BYTE req[sz];

   memset( (LPVOID)&req[0], 0, (SIZE_T)sz );

   sQDLRawWriteUnframedReq * pReq = (sQDLRawWriteUnframedReq *)&req[0];

   pReq->mCommandCode       = (BYTE)eQDL_CMD_WRITE_UNFRAMED_REQ;
   pReq->mSequenceNumber    = (WORD)sequenceNumber;
   pReq->mUnframedChunkSize = (DWORD)chunkSize;
   SetCRC( req, sz - sizeof( USHORT ) ); 

   eProtocolType pt = ePROTOCOL_QDL_TX;
   pRetBuf = new sSharedBuffer( (const BYTE *)req, sz, pt );
   return pRetBuf;
}

/*===========================================================================
METHOD:
   BuildWriteUnframedReqs (Static Public Method)

DESCRIPTION:
   Build list of unframed write requests from the given parameters

PARAMETERS:
   chunkSize      [ I ] - Size to write in each request
   dataLen        [ I ] - Total number of bytes to write

RETURN VALUE:
  std::list <sSharedBuffer *> : The requests in allocated buffers (the
  list is empty on error)
===========================================================================*/
std::list <sSharedBuffer *> sQDLWriteUnframed::BuildWriteUnframedReqs( 
   ULONG                      chunkSize,
   ULONG                      totalSize )
{ 
   std::list <sSharedBuffer *> retList;

   // Check length (in bytes) is acceptable
   if (chunkSize == 0 || chunkSize > QDL_MAX_CHUNK_SIZE)
   {
      return retList;
   }   

   ULONG writes = 1;
   ULONG rem = totalSize;

   // Will we need more than one write request?
   if (totalSize > chunkSize)
   {
      writes = totalSize / chunkSize;
      rem = totalSize % chunkSize;
      
      // Total size is a multiple of chunk size?
      if (rem == 0)
      {   
         // Yes, the remainder will be the block size
         rem = chunkSize;
      }
      else
      {
         // No, we need an extra write for the remainder
         writes++;
      }
   }

   ULONG blockSz = chunkSize;
   if (writes == 1)
   {
      blockSz = rem;
   }

   // Generate first request
   USHORT seqNum = 0;
   sSharedBuffer * pReq = 0;
   pReq = sQDLWriteUnframed::BuildWriteUnframedReq( seqNum++, blockSz );
   if (pReq != 0)
   {
      retList.push_back( pReq );
   }

   // Generate remaining requests
   for (UINT b = 1; b < writes; b++)
   {
      blockSz = chunkSize;
      if (b == writes - 1)
      {
         blockSz = rem;
      }

      pReq = sQDLWriteUnframed::BuildWriteUnframedReq( seqNum++, blockSz );     
      if (pReq != 0)
      {
         retList.push_back( pReq );
      }
   }

   // Errors?
   if (retList.size() != writes)
   {
      // Free up all our hard work
      std::list <sSharedBuffer *>::const_iterator pIter = retList.begin();
      while (pIter != retList.end())
      {
         delete [] *pIter;
         pIter++;
      }

      retList.clear();
   }

   return retList;
}

/*=========================================================================*/
// sQDLDone Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLDone (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLDone::sQDLDone( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLDone::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLDone (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLDone::~sQDLDone() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this session done request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLDone::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX && pt != ePROTOCOL_QDL_TX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szReq = (ULONG)sizeof( BYTE );
   const ULONG szRsp = (ULONG)sizeof( sQDLRawDoneRsp );

   if (pt == ePROTOCOL_QDL_TX && sz == szReq)
   {
      const BYTE * pReq = GetBuffer();
      if (*pReq != (BYTE)eQDL_CMD_SESSION_DONE_REQ)
      {
         return bRC;
      }

      bRC = true;
   }
   else if (pt == ePROTOCOL_QDL_RX && sz >= szRsp)
   {
      const sQDLRawDoneRsp * pRsp = 0;
      pRsp = (const sQDLRawDoneRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_SESSION_DONE_RSP)
      {
         return bRC;
      }

      // Status needs to be valid
      if (::IsValid( (eQDLDoneStatus)pRsp->mStatus ) == false)
      {
         return bRC;
      }

      // For success the error text should be NULL
      if ( (pRsp->mStatus == (WORD)eQDL_DONE_STATUS_SUCCESS) 
      &&   (sz != szRsp || pRsp->mErrorText != 0) )
      {
         return bRC;
      }

      if (pRsp->mStatus != (WORD)eQDL_DONE_STATUS_SUCCESS)
      {
         // Error text needs to be NULL terminated
         const BYTE * pTmp = GetBuffer();
         if (pTmp[sz - 1] != 0)
         {
            return bRC;
         }

         // What there is of the error text needs to be printable
         pTmp = &pRsp->mErrorText;
         while (*pTmp != 0)
         {
            int val = (int)*pTmp++;
            if (isprint( (int)val ) == 0)
            {
               return bRC;
            }
         }
      }

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*=========================================================================*/
// sQDLGetImagePref Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQDLGetImagePref (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the QDL data
  
RETURN VALUE:
   None
===========================================================================*/
sQDLGetImagePref::sQDLGetImagePref( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQDLGetImagePref::Validate();
}

/*===========================================================================
METHOD:
   ~sQDLGetImagePref (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQDLGetImagePref::~sQDLGetImagePref() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this get image preference request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQDLGetImagePref::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (pt != ePROTOCOL_QDL_RX && pt != ePROTOCOL_QDL_TX)
   {
      mbValid = bRC;
      return bRC;
   }

   ULONG sz = GetSize();
   const ULONG szReq = (ULONG)sizeof( BYTE );
   const ULONG szRsp = (ULONG)sizeof( sQDLRawGetImagePrefRsp );

   if (pt == ePROTOCOL_QDL_TX && sz == szReq)
   {
      const BYTE * pReq = GetBuffer();
      if (*pReq != (BYTE)eQDL_CMD_GET_IMAGE_PREF_REQ)
      {
         return bRC;
      }

      bRC = true; 
   }
   else if (pt == ePROTOCOL_QDL_RX && sz >= szRsp)
   {
      const sQDLRawGetImagePrefRsp * pRsp = 0;
      pRsp = (const sQDLRawGetImagePrefRsp *)GetBuffer();
      if (pRsp->mCommandCode != (BYTE)eQDL_CMD_GET_IMAGE_PREF_RSP)
      {
         return bRC;
      }

      BYTE entries = pRsp->mEntries;
      ULONG needSz = szRsp + (ULONG)entries * (ULONG)sizeof( sQDLRawImageID );
      if (sz != needSz)
      {
         return bRC;
      }

      // Skip response header
      pRsp++;

      // Validate image IDs
      const sQDLRawImageID * pID = (const sQDLRawImageID *)pRsp;
      for (BYTE e = 0; e < entries; e++)
      {
         sQDLRawImageID imagePref = *pID++;
         if (::IsValid( (eQDLImageType)imagePref.mImageType) == false)
         {
            return bRC;
         }
      }  

      bRC = true;
   }

   mbValid = bRC;
   return mbValid;
}

/*===========================================================================
METHOD:
   GetImageIDs (Public Method)

DESCRIPTION:
   Return image IDs
  
RETURN VALUE:
   std::list <sQDLRawImageID>
===========================================================================*/ 
std::list <sQDLRawImageID> sQDLGetImagePref::GetImageIDs() const
{
   // Assume failure
   std::list <sQDLRawImageID> retIDs;

   const sQDLRawGetImagePrefRsp * pRsp = GetResponse();
   if (pRsp == 0)
   {
      return retIDs;
   }
    
   BYTE entries = pRsp->mEntries;
   if (entries == 0)
   {
      return retIDs;
   }

   // Skip response header
   pRsp++;

   const sQDLRawImageID * pID = (const sQDLRawImageID *)pRsp;
   for (BYTE e = 0; e < entries; e++)
   {
      retIDs.push_back( *pID++ );
   }  

   return retIDs;
}
