/*===========================================================================
FILE:
   Gobi3000TranslationWMS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (WMS Service)

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
==========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "Gobi3000Translation.h"

/*===========================================================================
METHOD:
   PackDeleteSMS

DESCRIPTION:
   This function deletes one or more SMS messages from device memory

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   storageType    [ I ] - SMS message storage type
   pMessageIndex  [ I ] - (Optional) message index
   pMessageTag    [ I ] - (Optional) message tag

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackDeleteSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG *           pMessageIndex,
   ULONG *           pMessageTag )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSDeleteRequest_MemoryStorage );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The SPC
   sWMSDeleteRequest_MemoryStorage * pTLVx01;
   pTLVx01 = (sWMSDeleteRequest_MemoryStorage*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mStorageType = (eQMIWMSStorageTypes)storageType;

   offset += tlvx01Sz;

   // Add the Message index, if specified
   if (pMessageIndex != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWMSDeleteRequest_MessageIndex );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      // The SPC
      sWMSDeleteRequest_MessageIndex * pTLVx10;
      pTLVx10 = (sWMSDeleteRequest_MessageIndex*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the values
      pTLVx10->mStorageIndex = *pMessageIndex;

      offset += tlvx10Sz;
   }

   // Add the Message tag, if specified
   if (pMessageTag != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWMSDeleteRequest_MessageTag );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      // The SPC
      sWMSDeleteRequest_MessageTag * pTLVx11;
      pTLVx11 = (sWMSDeleteRequest_MessageTag*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      // Set the values
      pTLVx11->mMessageTag = (eQMIWMSMessageTags)*pMessageTag;

      offset += tlvx11Sz;
   }

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetSMSList

DESCRIPTION:
   This function returns the list of SMS messages stored on the device

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
   storageType       [ I ] - SMS message storage type
   pRequestedTag     [ I ] - Message index

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetSMSList(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG *           pRequestedTag )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSListMessagesRequest_MemoryStorage );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The storage type
   sWMSListMessagesRequest_MemoryStorage * pTLVx01;
   pTLVx01 = (sWMSListMessagesRequest_MemoryStorage*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mStorageType = (eQMIWMSStorageTypes)storageType;

   offset += tlvx01Sz;

   // Add the Message tag, if specified
   if (pRequestedTag != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWMSListMessagesRequest_MessageTag );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      // The SPC
      sWMSListMessagesRequest_MessageTag * pTLVx10;
      pTLVx10 = (sWMSListMessagesRequest_MessageTag*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the values
      pTLVx10->mMessageTag = (eQMIWMSMessageTags)*pRequestedTag;

      offset += tlvx10Sz;
   }

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSMSList

DESCRIPTION:
   This function returns the list of SMS messages stored on the device

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pMessageListSize  [I/O] - Upon input the maximum number of elements that the
                             message list array can contain.  Upon successful
                             output the actual number of elements in the message
                             list array
   pMessageList      [ O ] - The message list array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSMSList(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageListSize,
   BYTE *            pMessageList )
{
   // Validate arguments
   if (pIn == 0 || pMessageListSize == 0
   || *pMessageListSize == 0 || pMessageList == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxMessageListSz = *pMessageListSize;

   // Assume failure
   *pMessageListSize = 0;

   // Find the messages
   const sWMSListMessagesResponse_MessageList * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sWMSListMessagesResponse_MessageList ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   ULONG messageListSz = pTLVx01->mNumberOfMessages;
   if (messageListSz == 0)
   {
      // No stored messages, but not necessarily a failure
      return eGOBI_ERR_NONE;
   }

   if (maxMessageListSz < messageListSz)
   {
      messageListSz = maxMessageListSz;
   }

   const sWMSListMessagesResponse_MessageList::sMessage * pMessages;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sWMSListMessagesResponse_MessageList )
                  + sizeof( sWMSListMessagesResponse_MessageList::sMessage ) 
                    * messageListSz)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pMessages = (const sWMSListMessagesResponse_MessageList::sMessage *)
               ((const BYTE *)pTLVx01 
               + sizeof( sWMSListMessagesResponse_MessageList ));

   ULONG * pData = (ULONG *)pMessageList;
   for (ULONG m = 0; m < messageListSz; m++)
   {
      *pData++ = pMessages->mStorageIndex;
      *pData++ = pMessages->mMessageTag;
      pMessages++;
   }

   *pMessageListSize = messageListSz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetSMS

DESCRIPTION:
   This function returns an SMS message from device memory

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   storageType    [ I ] - SMS message storage type
   messageIndex   [ I ] - Message index

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageIndex )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSRawReadRequest_MessageIndex );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The index
   sWMSRawReadRequest_MessageIndex * pTLVx01;
   pTLVx01 = (sWMSRawReadRequest_MessageIndex*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mStorageType = (eQMIWMSStorageTypes)storageType;
   pTLVx01->mStorageIndex = messageIndex;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSMS

DESCRIPTION:
   This function returns an SMS message from device memory

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pMessageTag    [ O ] - Message tag
   pMessageFormat [ O ] - Message format
   pMessageSize   [I/O] - Upon input the maximum number of bytes that can be
                          written to the message array.  Upon successful
                          output the actual number of bytes written to the
                          message array
   pMessage       [ O ] - The message contents array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageTag,
   ULONG *           pMessageFormat,
   ULONG *           pMessageSize,
   BYTE *            pMessage )
{
   // Validate arguments
   if (pIn == 0
   ||  pMessageTag == 0
   ||  pMessageFormat == 0
   ||  pMessageSize == 0
   ||  *pMessageSize == 0
   ||  pMessage == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxMessageSz = *pMessageSize;

   // Assume failure
   *pMessageSize = 0;

   // Find the messages
   const sWMSRawReadResponse_MessageData * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sWMSRawReadResponse_MessageData ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pMessageTag = pTLVx01->mMessageTag;
   *pMessageFormat = pTLVx01->mMessageFormat;

   ULONG messageSz = pTLVx01->mRawMessageLength;
   if (messageSz == 0)
   {
      // No stored messages, but not necessarily a failure
      return eGOBI_ERR_NONE;
   }

   if (messageSz > maxMessageSz)
   {
      messageSz = maxMessageSz;
   }

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sWMSRawReadResponse_MessageData ) + messageSz)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   memcpy( pMessage,
           pTLVx01 + sizeof( sWMSRawReadResponse_MessageData ),
           messageSz );

   *pMessageSize = messageSz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackModifySMSStatus

DESCRIPTION:
   This function modifies the status of an SMS message saved in storage on
   the device

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   storageType    [ I ] - SMS message storage type
   messageIndex   [ I ] - Message index
   messageTag     [ I ] - Message tag

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackModifySMSStatus(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageIndex,
   ULONG             messageTag )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSModifyTagRequest_MessageTag );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The index
   sWMSModifyTagRequest_MessageTag * pTLVx01;
   pTLVx01 = (sWMSModifyTagRequest_MessageTag*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mStorageType = (eQMIWMSStorageTypes)storageType;
   pTLVx01->mStorageIndex = messageIndex;
   pTLVx01->mMessageTag = (eQMIWMSMessageTags)messageTag;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSaveSMS

DESCRIPTION:
   This function saves an SMS message to device memory

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   storageType    [ I ] - SMS message storage type
   messageFormat  [ I ] - Message format
   messageSize    [ I ] - The length of the message contents in bytes
   pMessage       [ I ] - The message contents

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSaveSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageFormat,
   ULONG             messageSize,
   BYTE *            pMessage )
{
   // Validate arguments
   if (pOut == 0 || messageSize == 0 || pMessage == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSRawWriteRequest_MessageData )
                 + (WORD)messageSize;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The index
   sWMSRawWriteRequest_MessageData * pTLVx01;
   pTLVx01 = (sWMSRawWriteRequest_MessageData*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mStorageType = (eQMIWMSStorageTypes)storageType;
   pTLVx01->mMessageFormat = (eQMIWMSMessageFormats)messageFormat;
   pTLVx01->mRawMessageLength = (UINT16)messageSize;

   offset += sizeof( sWMSRawWriteRequest_MessageData );

   // Add the message
   memcpy( (pOut + offset), pMessage, messageSize );

   offset += messageSize;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseSaveSMS

DESCRIPTION:
   This function saves an SMS message to device memory

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pMessageIndex  [ O ] - The message index assigned by the device

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseSaveSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageIndex )
{
   // Validate arguments
   if (pIn == 0 || pMessageIndex == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the messages
   const sWMSRawWriteResponse_MessageIndex * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sWMSRawWriteResponse_MessageIndex ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pMessageIndex = pTLVx01->mStorageIndex;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSendSMS

DESCRIPTION:
   This function sends an SMS message for immediate over the air transmission

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   messageFormat        [ I ] - Message format
   messageSize          [ I ] - The length of the message contents in bytes
   pMessage             [ I ] - The message contents

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSendSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             messageFormat,
   ULONG             messageSize,
   BYTE *            pMessage )
{
   // Validate arguments
   if (pOut == 0 || messageSize == 0 || pMessage == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWMSRawSendRequest_MessageData )
                 + (WORD)messageSize;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The index
   sWMSRawSendRequest_MessageData * pTLVx01;
   pTLVx01 = (sWMSRawSendRequest_MessageData*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mMessageFormat = (eQMIWMSMessageFormats)messageFormat;
   pTLVx01->mRawMessageLength = (UINT16)messageSize;

   offset += sizeof( sWMSRawSendRequest_MessageData );

   // Add the message
   memcpy( (pOut + offset), pMessage, messageSize );

   offset += messageSize;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseSendSMS

DESCRIPTION:
   This function sends an SMS message for immediate over the air transmission

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pMessageFailureCode  [ O ] - When the function fails due to an error sending
                                the message this parameter may contain the
                                message failure cause code (see 3GPP2 N.S0005
                                Section 6.5.2.125).  If the cause code is not
                                provided then the value will be 0xFFFFFFFF

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseSendSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageFailureCode )
{
   // Validate arguments
   if (pIn == 0 || pMessageFailureCode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume we have no message failure cause code
   *pMessageFailureCode = 0xffffffff;

   // Check mandatory response
   const sResultCode * pTLVx02;
   ULONG outLenx02;
   ULONG rc = GetTLV( inLen, pIn, 0x02, &outLenx02, (const BYTE **)&pTLVx02 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx02 < sizeof( sResultCode ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   if (pTLVx02->mQMIResult != eQMIResults_Success)
   {
      rc = pTLVx02->mQMIError + eGOBI_ERR_QMI_OFFSET;
   }

   if (rc != eGOBI_ERR_NONE)
   {
      // Check for the failure code (optional)
      const sWMSRawSendResponse_CauseCode * pTLVx10;
      ULONG outLenx10;
      ULONG rc2 = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
      if (rc2 == eGOBI_ERR_NONE)
      {
         if (outLenx10 < sizeof( sWMSRawSendResponse_CauseCode ))
         {
            return eGOBI_ERR_MALFORMED_RSP;
         }

         *pMessageFailureCode = pTLVx10->mCauseCode;
      }
   }

   return rc;
}

/*===========================================================================
METHOD:
   ParseGetSMSCAddress

DESCRIPTION:
   This function returns the SMS center address

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   addressSize    [ I ] - The maximum number of characters (including NULL
                          terminator) that the SMS center address array
                          can contain
   pSMSCAddress   [ O ] - The SMS center address represented as a NULL
                          terminated string
   typeSize       [ I ] - The maximum number of characters (including NULL
                          terminator) that the SMS center address type array
                          can contain
   pSMSCType      [ O ] - The SMS center address type represented as a NULL
                          terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSMSCAddress(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              addressSize,
   CHAR *            pSMSCAddress,
   BYTE              typeSize,
   CHAR *            pSMSCType )
{
   // Validate arguments
   if (pIn == 0
   || addressSize == 0 || pSMSCAddress == 0
   || typeSize == 0 || pSMSCType == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume empty
   pSMSCAddress[0] = 0;
   pSMSCType[0] = 0;

   // Get the address (mandatory)
   const sWMSGetSMSCAddressResponse_Address * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sWMSRawSendResponse_CauseCode ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Handle the type as a string (maximum 3 chars)
   std::string smscType( &pTLVx01->mSMSCAddressType[0], 3 );

   // Is the SMSC type present? (optional)
   ULONG smscTypeLen = (ULONG)smscType.size();
   if (smscTypeLen > 0)
   {
      // Space to perform copy?
      if (typeSize < smscTypeLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pSMSCType, &pTLVx01->mSMSCAddressType[0], smscTypeLen );
      pSMSCType[smscTypeLen] = 0;
   }

   // Treat the address as a null terminated string
   std::string smscAddr( (const CHAR *)pTLVx01 
                         + sizeof( sWMSGetSMSCAddressResponse_Address ),
                         pTLVx01->mSMSCAddressLength );

   ULONG smscAddrLen = (ULONG)smscAddr.size();
   if (addressSize < smscAddrLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pSMSCAddress, smscAddr.c_str(), addressSize );
   pSMSCAddress[addressSize] = 0;

   return rc;
}

/*===========================================================================
METHOD:
   PackSetSMSCAddress

DESCRIPTION:
   This function sets the SMS center address

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   pSMSCAddress   [ I ] - The SMS center address represented as a NULL
                          terminated string
   pSMSCType      [ I ] - The SMS center address type represented as a NULL
                          terminated string (optional)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetSMSCAddress(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSMSCAddress,
   CHAR *            pSMSCType )
{
   // Validate arguments
   if (pOut == 0 || pSMSCAddress == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // The TLV contains only the address
   std::string smscAddr( pSMSCAddress );

   // Check size
   WORD tlvx01Sz = (WORD)smscAddr.size();
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   memcpy( (pOut + offset), smscAddr.c_str(), tlvx01Sz );
   offset += tlvx01Sz;

   // smscType is optional
   if (pSMSCType != 0)
   {
      // The TLV contains only the type
      std::string smscType( pSMSCType );

      if (smscType.size() != 0)
      {
         // Check size
         WORD tlvx10Sz = (WORD)smscType.size();
         if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
         {
            return eGOBI_ERR_BUFFER_SZ;
         }

         sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
         pHeader->mTypeID = 0x10;
         pHeader->mLength = tlvx10Sz;

         ULONG offset = sizeof( sQMIRawContentHeader );

         memcpy( (pOut + offset), smscType.c_str(), tlvx10Sz );
         offset += tlvx10Sz;
      }
   }


   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSMSRoutes

DESCRIPTION:
   This function gets the current incoming SMS routing information

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pRouteSize  [I/O] - Upon input the maximum number of elements that the
                       SMS route array can contain.  Upon succes the actual
                       number of elements in the SMS route array
   pRoutes     [ O ] - The SMS route array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSMSRoutes(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pRouteSize,
   BYTE *            pRoutes )
{
   // Validate arguments
   if (pIn == 0 || pRouteSize == 0 || *pRouteSize == 0 || pRoutes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   BYTE maxRoutes = *pRouteSize;
   *pRouteSize = 0;

   // Get the route list
   const sWMSGetRoutesResponse_RouteList * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sWMSGetRoutesResponse_RouteList ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   ULONG routeCount = pTLVx01->mNumberOfRoutes;
   if (routeCount > (ULONG)maxRoutes)
   {
      routeCount = (ULONG)maxRoutes;
   }

   const sWMSGetRoutesResponse_RouteList::sRoute * pInRoute;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sWMSGetRoutesResponse_RouteList )
                  + sizeof( sWMSGetRoutesResponse_RouteList::sRoute ) 
                    * routeCount)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pInRoute = (const sWMSGetRoutesResponse_RouteList::sRoute *)
              ((const BYTE *)pTLVx01 
              + sizeof( sWMSGetRoutesResponse_RouteList ));

   ULONG * pRouteArray = (ULONG *)pRoutes;
   for (ULONG r = 0; r < routeCount; r++)
   {
      *pRouteArray++ = pInRoute->mMessageType;
      *pRouteArray++ = pInRoute->mMessageClass;
      *pRouteArray++ = pInRoute->mStorageType;
      *pRouteArray++ = pInRoute->mRouteValue;
      pInRoute++;
   }

   *pRouteSize = (BYTE)routeCount;
   return rc;
}

/*===========================================================================
METHOD:
   PackSetSMSRoutes

DESCRIPTION:
   This function sets the desired incoming SMS routing information

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied to pOut
   pOut        [ O ] - Output buffer
   pRouteSize  [ I ] - The number of elements in the SMS route array
   pRoutes     [ I ] - The SMS route array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetSMSRoutes(
   ULONG *           pOutLen,
   BYTE *            pOut,
   BYTE *            pRouteSize,
   BYTE *            pRoutes )
{
   // Validate arguments
   if (pOut == 0 || pRouteSize == 0 || *pRouteSize == 0 || pRoutes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   UINT16 routeCount = (ULONG)*pRouteSize;

   // Check size
   WORD tlvx01Sz = sizeof( sWMSSetRoutesRequest_RouteList )
      + sizeof( sWMSSetRoutesRequest_RouteList::sRoute ) * routeCount;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // Add route count
   sWMSSetRoutesRequest_RouteList * pTLVx01;
   pTLVx01 = (sWMSSetRoutesRequest_RouteList*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mNumberOfRoutes = routeCount;
   offset += sizeof( sWMSSetRoutesRequest_RouteList );

   sWMSSetRoutesRequest_RouteList::sRoute * pOutRoute;

   // Align to the first array element
   pOutRoute = (sWMSSetRoutesRequest_RouteList::sRoute *)(pOut + offset);

   // Add the routes
   ULONG * pRouteArray = (ULONG *)pRoutes;
   for (ULONG r = 0; r < routeCount; r++)
   {
      pOutRoute->mMessageType = (eQMIWMSMessageTypes)*pRouteArray++;
      pOutRoute->mMessageClass = (eQMIWMSMessageClasses)*pRouteArray++;
      pOutRoute->mStorageType = (eQMIWMSStorageTypes)*pRouteArray++;
      pOutRoute->mReceiptAction = (eQMIWMSReceiptActions)*pRouteArray++;
      pOutRoute++;
      offset += sizeof( sWMSSetRoutesRequest_RouteList::sRoute );
   }

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}
