/*===========================================================================
FILE:
   QMIBuffers.cpp

DESCRIPTION:
   QMI service protocol related structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sQMIControlRawTransactionHeader
   sQMIServiceRawTransactionHeader
   sQMIRawMessageHeader
   sQMIRawContentHeader

   sQMIServiceBuffer

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
#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// sQMIServiceBuffer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sQMIServiceBuffer (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the DIAG data
  
RETURN VALUE:
   None
===========================================================================*/
sQMIServiceBuffer::sQMIServiceBuffer( sSharedBuffer * pBuffer )
   :  sProtocolBuffer( pBuffer )
{
   sQMIServiceBuffer::Validate();
}

/*===========================================================================
METHOD:
   ~sQMIServiceBuffer (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sQMIServiceBuffer::~sQMIServiceBuffer() 
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   GetResult (Public Method)

DESCRIPTION:
   Return contents of mandatory result content
  
PARAMETERS:
   returnCode  [ I ] - The return code (should be eQMIResultCode)
   errorCode   [ I ] - The error code (should be eQMIErrorCode)

RETURN VALUE:
   bool
===========================================================================*/
bool sQMIServiceBuffer::GetResult( 
   ULONG &                    returnCode,
   ULONG &                    errorCode )
{
   if (IsResponse() == false)
   {
      return false;
   }

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = mContents.find( QMI_TLV_ID_RESULT );
   if (pIter == mContents.end())
   {
      return false;
   }

   const sQMIRawContentHeader * pContent = pIter->second;
   if (pContent == 0)
   {
      ASSERT( 0 );
      return false;
   }

   if (pContent->mLength != 4)
   {
      return false;
   }
   
   const WORD * pData = (const WORD *)(++pContent);

   returnCode = (ULONG)*pData++;
   errorCode = (ULONG)*pData;

   return true;
}

/*===========================================================================
METHOD:
   BuildBuffer (Static Public Method)

DESCRIPTION:
   Build a QMI request
  
PARAMETERS:
   serviceType [ I ] - QMI service type
   msgID       [ I ] - The QMI message request ID
   bResponse   [ I ] - Build a response?
   bIndication [ I ] - Build an indication?
   pPayload    [ I ] - Payload
   payloadLen  [ I ] - Size of above payload

RETURN VALUE:
   sSharedBuffer * : The request in an allocated buffer (0 on error)
===========================================================================*/
sSharedBuffer * sQMIServiceBuffer::BuildBuffer( 
   eQMIService                serviceType,
   WORD                       msgID,
   bool                       bResponse,
   bool                       bIndication,
   const BYTE *               pPayload,
   ULONG                      payloadLen )
{
   const ULONG szTransHdr = (ULONG)sizeof(sQMIServiceRawTransactionHeader);
   const ULONG szMsgHdr   = (ULONG)sizeof(sQMIRawMessageHeader);
   const ULONG totalHdrSz = szTransHdr + szMsgHdr;

   // Truncate payload?
   if (payloadLen > (QMI_MAX_BUFFER_SIZE - totalHdrSz))
   {
      payloadLen = QMI_MAX_BUFFER_SIZE - totalHdrSz;
   }

   // Make sure length agrees with pointer
   if (pPayload == 0)
   {
      payloadLen = 0;
   }

   // Allocate buffer
   PBYTE pBuffer = new BYTE[payloadLen + totalHdrSz];
   if (pBuffer == 0)
   {
      return 0;
   }

   // Format header
   sQMIServiceRawTransactionHeader * pHdr = 0;
   pHdr = (sQMIServiceRawTransactionHeader *)&pBuffer[0];
   pHdr->mCompound      = 0;
   pHdr->mResponse      = 0;
   pHdr->mIndication    = 0;
   pHdr->mReserved      = 0;
   pHdr->mTransactionID = 1;
   
   bool bTX = true;
   if (bResponse == true)
   {
      pHdr->mResponse = 1;
      bTX = false;
   }
   else if (bIndication == true)
   {
      pHdr->mIndication = 1;
      bTX = false;
   }

   pHdr++;

   // Format message header
   sQMIRawMessageHeader * pMsg = 0;
   pMsg = (sQMIRawMessageHeader *)pHdr;
   pMsg->mMessageID = msgID;
   pMsg->mLength    = (WORD)payloadLen;
   
   // Copy in payload?
   if (payloadLen > 0 && pPayload != 0)
   {
      memcpy( (LPVOID)&pBuffer[totalHdrSz], 
              (LPCVOID)&pPayload[0], 
              (SIZE_T)payloadLen );
   }   

   // Compute total size
   ULONG sz = payloadLen + totalHdrSz;

   // Build and return the shared buffer
   eProtocolType pt = MapQMIServiceToProtocol( serviceType, bTX );
   sSharedBuffer * pBuf = new sSharedBuffer( sz, pBuffer, pt );
   return pBuf;
}

/*===========================================================================
METHOD:
   Validate (Internal Method)

DESCRIPTION:
   Is this open unframed request/response packet valid?
  
RETURN VALUE:
   bool
===========================================================================*/ 
bool sQMIServiceBuffer::Validate()
{
   // Assume failure
   bool bRC = false;

   // Sanity check protocol type
   eProtocolType pt = GetType();
   if (IsQMIProtocol( pt ) == false)
   {
      mbValid = bRC;
      return bRC;
   }

   const ULONG szTransHdr   = (ULONG)sizeof(sQMIServiceRawTransactionHeader);
   const ULONG szMsgHdr     = (ULONG)sizeof(sQMIRawMessageHeader);
   const ULONG szContentHdr = (ULONG)sizeof(sQMIRawContentHeader);

   // Must be enough space for both headers
   ULONG sz = GetSize();
   if (sz < szTransHdr + szMsgHdr)
   {
      mbValid = bRC;
      return bRC;
   }

   const BYTE * pBuffer = GetBuffer();

   // Obtain transaction header
   const sQMIServiceRawTransactionHeader * pTransHdr = 0;
   pTransHdr = (const sQMIServiceRawTransactionHeader *)pBuffer;
   pBuffer += szTransHdr;

   // This is required to be 0
   if (pTransHdr->mCompound != 0)
   {
      mbValid = bRC;
      return bRC;
   }

   // These are mutually exclusive
   if (pTransHdr->mIndication == 1 && pTransHdr->mResponse == 1)
   {
      mbValid = bRC;
      return bRC;
   }

   // Requests/responses required valid transaction IDs 
   if ( (pTransHdr->mIndication == 0) 
   &&   (pTransHdr->mTransactionID == (WORD)INVALID_QMI_TRANSACTION_ID) )
   {
      mbValid = bRC;
      return bRC;
   }

   if ( (pTransHdr->mResponse == 1 || pTransHdr->mIndication == 1)
   &&   (IsQMIProtocolRX( pt ) == false) )
   {
      mbValid = bRC;
      return bRC;
   }

   if ( (pTransHdr->mResponse == 0 && pTransHdr->mIndication == 0)
   &&   (IsQMIProtocolTX( pt ) == false) )
   {
      mbValid = bRC;
      return bRC;
   }

   // Obtain message header
   const sQMIRawMessageHeader * pMsgHdr = 0;
   pMsgHdr = (const sQMIRawMessageHeader *)pBuffer;
   pBuffer += szMsgHdr;

   // Validate reported length
   if (sz != ((ULONG)pMsgHdr->mLength + szTransHdr + szMsgHdr))
   {
      mbValid = bRC;
      return bRC;
   }

   // Extract content TLV structures
   ULONG contentProcessed = 0;
   ULONG contentSz = (ULONG)pMsgHdr->mLength;
   while (contentProcessed < contentSz)
   {
      const sQMIRawContentHeader * pContent = 0;
      pContent = (const sQMIRawContentHeader *)pBuffer;

      ULONG tlvLen = szContentHdr + pContent->mLength; 
      
      contentProcessed += tlvLen;
      if (contentProcessed <= contentSz)
      {
         mContents[(ULONG)pContent->mTypeID] = pContent;
      }
      else
      {
         mContents.clear();

         mbValid = bRC;
         return bRC;
      }

      pBuffer += tlvLen;
   }

   // Validate TLV reported lengths
   if (contentProcessed != contentSz)
   {
      mbValid = bRC;
      return bRC;
   }

   // Success!
   bRC = true;

   mbValid = bRC;
   return mbValid;
}

