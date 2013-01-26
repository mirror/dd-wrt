/*===========================================================================
FILE: 
   GobiQMICoreSMS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (SMS Service)

PUBLIC CLASSES AND FUNCTIONS:
   cGobiQMICore

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
#include "StdAfx.h"
#include "GobiQMICore.h"

#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   DeleteSMS (Public Method)

DESCRIPTION:
   This function deletes one or more SMS messages from device memory

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   pMessageIndex  [ I ] - (Optional) message index
   pMessageTag    [ I ] - (Optional) message tag
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::DeleteSMS( 
   ULONG                      storageType, 
   ULONG *                    pMessageIndex, 
   ULONG *                    pMessageTag )
{
   WORD msgID = (WORD)eQMI_WMS_DELETE;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)storageType;

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   if (pMessageIndex != 0)
   {
      std::ostringstream tmp2;
      tmp2 << (UINT)*pMessageIndex;

      sProtocolEntityKey pek1( eDB2_ET_QMI_WMS_REQ, msgID, 16 );
      sDB2PackingInput pi1( pek1, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi1 );
   }

   if (pMessageTag != 0)
   {
      std::ostringstream tmp2;
      tmp2 << (UINT)*pMessageTag;

      sProtocolEntityKey pek1( eDB2_ET_QMI_WMS_REQ, msgID, 17 );
      sDB2PackingInput pi1( pek1, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi1 );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WMS, pRequest, 10000 );
}

/*===========================================================================
METHOD:
   GetSMSList (Public Method)

DESCRIPTION:
   This function returns the list of SMS messages stored on the device

PARAMETERS:
   storageType       [ I ] - SMS message storage type
   pRequestedTag     [ I ] - Message index
   pMessageListSize  [I/O] - Upon input the maximum number of elements that the 
                             message list array can contain.  Upon successful 
                             output the actual number of elements in the message 
                             list array
   pMessageList      [ O ] - The message list array
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSMSList( 
   ULONG                      storageType, 
   ULONG *                    pRequestedTag,
   ULONG *                    pMessageListSize, 
   BYTE *                     pMessageList )
{
   // Validate arguments
   if (pMessageListSize == 0 || *pMessageListSize == 0 || pMessageList == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxMessageListSz = *pMessageListSize;

   // Assume failure
   *pMessageListSize = 0;

   WORD msgID = (WORD)eQMI_WMS_GET_MSG_LIST;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)storageType;

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   if (pRequestedTag != 0)
   {
      std::ostringstream tmp2;
      tmp2 << (UINT)*pRequestedTag;

      sProtocolEntityKey pek1( eDB2_ET_QMI_WMS_REQ, msgID, 16 );
      sDB2PackingInput pi1( pek1, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi1 );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WMS, pRequest, 5000 );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG messageListSz = pf[0].mValue.mU32;
   if (messageListSz == 0)
   {
      // No stored messages, but not necessarily a failure
      return eGOBI_ERR_NONE;
   }

   if (pf.size() < (1 + (messageListSz * 2)) )
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (maxMessageListSz < messageListSz)
   {
      messageListSz = maxMessageListSz;
   }

   ULONG m = 0;
   ULONG mf = 1;
   ULONG * pData = (ULONG *)pMessageList;   
   for (m = 0; m < messageListSz; m++)
   {
      *pData++ = pf[mf++].mValue.mU32;
      *pData++ = pf[mf++].mValue.mU32;
   }
   
   *pMessageListSize = messageListSz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSMS (Public Method)

DESCRIPTION:
   This function returns an SMS message from device memory

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   messageIndex   [ I ] - Message index
   pMessageTag    [ O ] - Message tag
   pMessageFormat [ O ] - Message format
   pMessageSize   [I/O] - Upon input the maximum number of bytes that can be 
                          written to the message array.  Upon successful 
                          output the actual number of bytes written to the 
                          message array
   pMessage       [ I ] - The message contents array
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSMS( 
   ULONG                      storageType, 
   ULONG                      messageIndex, 
   ULONG *                    pMessageTag,
   ULONG *                    pMessageFormat,
   ULONG *                    pMessageSize, 
   BYTE *                     pMessage )
{
   // Validate arguments
   if ( (pMessageTag == 0) 
   ||   (pMessageFormat == 0)
   ||   (pMessageSize == 0) 
   ||   (*pMessageSize == 0) 
   ||   (pMessage == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxMessageSz = *pMessageSize;

   // Assume failure
   *pMessageSize = 0;

   WORD msgID = (WORD)eQMI_WMS_RAW_READ;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)storageType << " " << (UINT)messageIndex;

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WMS, pRequest, 5000 );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pMessageTag = pf[0].mValue.mU32;
   *pMessageFormat = pf[1].mValue.mU32;

   ULONG messageSz = (ULONG)pf[2].mValue.mU16;
   if (messageSz == 0)
   {
      // There has to be message data
      return eGOBI_ERR_INVALID_RSP;
   }

   if (pf.size() < 3 + messageSz)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (maxMessageSz < messageSz)
   {
      // We have to be able to copy the whole message
      return eGOBI_ERR_BUFFER_SZ;
   }

   // Copy message data
   for (ULONG b = 0; b < messageSz; b++)
   {
      pMessage[b] = pf[3 + b].mValue.mU8;
   }
   
   *pMessageSize = messageSz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ModifySMSStatus (Public Method)

DESCRIPTION:
   This function modifies the status of an SMS message saved in storage on 
   the device

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   messageIndex   [ I ] - Message index
   messageTag     [ I ] - Message tag
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ModifySMSStatus( 
   ULONG                      storageType, 
   ULONG                      messageIndex, 
   ULONG                      messageTag )
{
   WORD msgID = (WORD)eQMI_WMS_MODIFY_TAG;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)storageType << " " << (UINT)messageIndex << " " 
       << (UINT)messageTag;

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WMS, pRequest, 5000 );
}

/*===========================================================================
METHOD:
   SaveSMS (Public Method)

DESCRIPTION:
   This function saves an SMS message to device memory

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   messageFormat  [ I ] - Message format   
   messageSize    [ I ] - The length of the message contents in bytes
   pMessage       [ I ] - The message contents
   pMessageIndex  [ O ] - The message index assigned by the device
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SaveSMS(    
   ULONG                      storageType, 
   ULONG                      messageFormat, 
   ULONG                      messageSize, 
   BYTE *                     pMessage, 
   ULONG *                    pMessageIndex )
{
   // Validate arguments
   if (messageSize == 0 || pMessage == 0 || pMessageIndex == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WMS_RAW_WRITE;
   std::vector <sDB2PackingInput> piv;

   // "%u %u %u"
   std::ostringstream tmp;
   tmp << (UINT)storageType << " " << (UINT)messageFormat
       << " " << (UINT)messageSize;

   for (ULONG b = 0; b < messageSize; b++)
   {
      tmp << " " << (UINT)pMessage[b];
   }

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WMS, pRequest, 10000 );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pMessageIndex = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SendSMS (Public Method)

DESCRIPTION:
   This function sends an SMS message for immediate over the air transmission

PARAMETERS:
   messageFormat        [ I ] - Message format   
   messageSize          [ I ] - The length of the message contents in bytes
   pMessage             [ I ] - The message contents
   pMessageFailureCode  [ O ] - When the function fails due to an error sending
                                the message this parameter may contain the 
                                message failure cause code (see 3GPP2 N.S0005 
                                Section 6.5.2.125).  If the cause code is not
                                provided then the value will be 0xFFFFFFFF
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SendSMS(    
   ULONG                      messageFormat, 
   ULONG                      messageSize, 
   BYTE *                     pMessage, 
   ULONG *                    pMessageFailureCode )
{
   // Validate arguments
   if (messageSize == 0 || pMessage == 0 || pMessageFailureCode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume we have no message failure cause code
   *pMessageFailureCode = ULONG_MAX;

   WORD msgID = (WORD)eQMI_WMS_RAW_SEND;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)messageFormat << " " << (UINT)messageSize;

   for (ULONG b = 0; b < messageSize; b++)
   {
      tmp << " " << (UINT)pMessage[b];
   }

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WMS, pRequest, 300000 );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 1) 
      {
         *pMessageFailureCode = (ULONG)pf[0].mValue.mU16;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSMSCAddress (Public Method)

DESCRIPTION:
   Return the SMS center address

PARAMETERS:
   addressSize    [ I ] - The maximum number of characters (including NULL 
                          terminator) that the SMS center address array 
                          can contain
   pSMSCAddress   [ 0 ] - The SMS center address represented as a NULL 
                          terminated string
   typeSize       [ I ] - The maximum number of characters (including NULL 
                          terminator) that the SMS center address type array
                          can contain
   pSMSCType      [ 0 ] - The SMS center address type represented as a NULL 
                          terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSMSCAddress( 
   BYTE                       addressSize,
   CHAR *                     pSMSCAddress,
   BYTE                       typeSize,
   CHAR *                     pSMSCType )
{
   // Validate arguments
   if (addressSize == 0 || pSMSCAddress == 0 || typeSize == 0 || pSMSCType == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   pSMSCAddress[0] = 0;
   pSMSCType[0] = 0;
   
   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WMS_GET_SMSC_ADDR;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WMS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   LONG strLen = pf[0].mValueString.size();
   if (strLen > 0)
   {
      // Space to perform the copy?
      if (typeSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pSMSCType, (LPCSTR)pf[0].mValueString.c_str(), strLen );
      pSMSCType[strLen] = 0;
   }

   strLen = pf[2].mValueString.size();
   if (strLen > 0)
   {
      // Space to perform the copy?
      if (addressSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pSMSCAddress, (LPCSTR)pf[2].mValueString.c_str(), strLen );
      pSMSCAddress[strLen] = 0;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetSMSCAddress (Public Method)

DESCRIPTION:
   Set the SMS center address

PARAMETERS:
   pSMSCAddress   [ I ] - The SMS center address represented as a NULL 
                          terminated string (maximum of 21 characters,
                          including NULL)
   pSMSCType      [ I ] - The SMS center address type represented as a NULL 
                          terminated string (optional)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetSMSCAddress( 
   CHAR *                     pSMSCAddress, 
   CHAR *                     pSMSCType )
{
   // Validate arguments
   if (pSMSCAddress == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WMS_SET_SMSC_ADDR;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream addr;
   if (pSMSCAddress[0] != 0)
   {
      addr << "\"" << pSMSCAddress << "\"";
   }

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)addr.str().c_str() );
   piv.push_back( pi );

   if (pSMSCType != 0)
   {
      std::ostringstream addrType;
      if (pSMSCType[0] != 0)
      {
         addrType << "\"" << pSMSCType << "\"";
      }

      pek = sProtocolEntityKey( eDB2_ET_QMI_WMS_REQ, msgID, 16 );
      pi = sDB2PackingInput( pek, (LPCSTR)addrType.str().c_str() );
      piv.push_back( pi );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WMS, pRequest, 5000 );
}

/*===========================================================================
METHOD:
   GetSMSRoutes (Public Method)

DESCRIPTION:
   Get the current incoming SMS routing information

PARAMETERS:
   pRouteSize  [I/O] - Upon input the maximum number of elements that the 
                       SMS route array can contain.  Upon succes the actual 
                       number of elements in the SMS route array
   pRoutes     [ O ] - The SMS route array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSMSRoutes( 
   BYTE *                     pRouteSize, 
   BYTE *                     pRoutes )
{
   // Validate arguments
   if (pRouteSize == 0 || *pRouteSize == 0 || pRoutes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   BYTE maxRoutes = *pRouteSize;
   *pRouteSize = 0;
   
   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WMS_GET_ROUTES;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WMS, msgID, 5000 );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG fi = 0;
   ULONG routeCount = (ULONG)pf[fi++].mValue.mU16;
   if ((ULONG)pf.size() < 1 + 4 * routeCount)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (routeCount > (ULONG)maxRoutes)
   {
      routeCount = (ULONG)maxRoutes;
   }

   ULONG * pRouteArray = (ULONG *)pRoutes;
   for (ULONG r = 0; r < routeCount; r++)
   {
      // Message type
      *pRouteArray++ = pf[fi++].mValue.mU32;

      // Message class
      *pRouteArray++ = pf[fi++].mValue.mU32;

      // Storage type
      *pRouteArray++ = pf[fi++].mValue.mU32;

      // Receipt action
      *pRouteArray++ = pf[fi++].mValue.mU32;
   }

   *pRouteSize = (BYTE)routeCount;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetSMSRoutes (Public Method)

DESCRIPTION:
   Set the desired incoming SMS routing information

PARAMETERS:
   pRouteSize  [ I ] - The number of elements in the SMS route array
   pRoutes     [ I ] - The SMS route array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetSMSRoutes( 
   BYTE *                     pRouteSize, 
   BYTE *                     pRoutes )
{
   // Validate arguments
   if (pRouteSize == 0 || *pRouteSize == 0 || pRoutes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Format up the request
   ULONG routeCount = (ULONG)*pRouteSize;

   // %u
   std::ostringstream tmp;
   tmp << routeCount;

   ULONG * pRouteArray = (ULONG *)pRoutes;
   for (ULONG r = 0; r < routeCount; r++)
   {
      // Message type, class, storage type, receipt action
      for (ULONG f = 0; f < 4; f++)
      {
         // tmp += " %u"
         tmp << " " << *pRouteArray++;
      }
   }

   WORD msgID = (WORD)eQMI_WMS_SET_ROUTES;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WMS, pRequest, 5000 );
}
