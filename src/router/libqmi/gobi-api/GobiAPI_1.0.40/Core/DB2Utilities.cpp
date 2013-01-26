/*===========================================================================
FILE:
   DB2Utilities.cpp

DESCRIPTION:
   Utility functions for packing/parsing protocol entities using the
   database

PUBLIC ENUMERATIONS AND METHODS:
   sProtocolEntityKey
   sDB2PackingInput
   sDB2NavInput

   MapQMIEntityTypeToProtocolType
   MapQMIEntityTypeToQMIServiceType
   MapQMIProtocolTypeToEntityType
   DB2GetMaxBufferSize
   DB2BuildQMIBuffer
   DB2PackQMIBuffer
   DB2ReduceQMIBuffer

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
#include "DB2Utilities.h"

#include "QMIBuffers.h"

#include "DataPacker.h"
#include "ProtocolEntityFieldEnumerator.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Free Methods
//---------------------------------------------------------------------------

/*===========================================================================
METHOD:
   MapQMIEntityTypeToQMIServiceType (Public Free Method)

DESCRIPTION:
   Map a DB protocol entity type (for QMI) to a QMI service type

PARAMETERS:
   et          [ I ] - Protocol entity type
  
RETURN VALUE:
   eQMIService
===========================================================================*/
eQMIService MapQMIEntityTypeToQMIServiceType( eDB2EntityType et )
{
   eQMIService st = eQMI_SVC_ENUM_BEGIN;
   switch (et)
   {
      case eDB2_ET_QMI_CTL_REQ:
      case eDB2_ET_QMI_CTL_RSP:
      case eDB2_ET_QMI_CTL_IND:
         st = eQMI_SVC_CONTROL;
         break;

      case eDB2_ET_QMI_WDS_REQ:
      case eDB2_ET_QMI_WDS_RSP:
      case eDB2_ET_QMI_WDS_IND:
         st = eQMI_SVC_WDS;
         break;

      case eDB2_ET_QMI_DMS_REQ:
      case eDB2_ET_QMI_DMS_RSP:
      case eDB2_ET_QMI_DMS_IND:
         st = eQMI_SVC_DMS;
         break;

      case eDB2_ET_QMI_NAS_REQ:
      case eDB2_ET_QMI_NAS_RSP:
      case eDB2_ET_QMI_NAS_IND:
         st = eQMI_SVC_NAS;
         break;

      case eDB2_ET_QMI_QOS_REQ:
      case eDB2_ET_QMI_QOS_RSP:
      case eDB2_ET_QMI_QOS_IND:
         st = eQMI_SVC_QOS;
         break;

      case eDB2_ET_QMI_WMS_REQ:
      case eDB2_ET_QMI_WMS_RSP:
      case eDB2_ET_QMI_WMS_IND:
         st = eQMI_SVC_WMS;
         break;

      case eDB2_ET_QMI_PDS_REQ:
      case eDB2_ET_QMI_PDS_RSP:
      case eDB2_ET_QMI_PDS_IND:
         st = eQMI_SVC_PDS;
         break;
      
      case eDB2_ET_QMI_AUTH_REQ:
      case eDB2_ET_QMI_AUTH_RSP:
      case eDB2_ET_QMI_AUTH_IND:
         st = eQMI_SVC_AUTH;
         break;

      case eDB2_ET_QMI_VOICE_REQ:
      case eDB2_ET_QMI_VOICE_RSP:
      case eDB2_ET_QMI_VOICE_IND:
         st = eQMI_SVC_VOICE;
         break;

      case eDB2_ET_QMI_CAT_REQ:
      case eDB2_ET_QMI_CAT_RSP:
      case eDB2_ET_QMI_CAT_IND:
         st = eQMI_SVC_CAT;
         break;

      case eDB2_ET_QMI_RMS_REQ:
      case eDB2_ET_QMI_RMS_RSP:
      case eDB2_ET_QMI_RMS_IND:
         st = eQMI_SVC_RMS;
         break;

      case eDB2_ET_QMI_OMA_REQ:
      case eDB2_ET_QMI_OMA_RSP:
      case eDB2_ET_QMI_OMA_IND:
         st = eQMI_SVC_OMA;
         break;
   }

   return st;
}

/*===========================================================================
METHOD:
   MapQMIEntityTypeToProtocolType (Public Free Method)

DESCRIPTION:
   Map a DB protocol entity type (for QMI) to a buffer protocol type

PARAMETERS:
   et          [ I ] - Protocol entity type
  
RETURN VALUE:
   eProtocolType
===========================================================================*/
eProtocolType MapQMIEntityTypeToProtocolType( eDB2EntityType et )
{
   eProtocolType pt = ePROTOCOL_ENUM_BEGIN;
   switch (et)
   {
      case eDB2_ET_QMI_WDS_REQ:
         pt = ePROTOCOL_QMI_WDS_TX;
         break;

      case eDB2_ET_QMI_WDS_RSP:
      case eDB2_ET_QMI_WDS_IND:
         pt = ePROTOCOL_QMI_WDS_RX;
         break;

      case eDB2_ET_QMI_DMS_REQ:
         pt = ePROTOCOL_QMI_DMS_TX;
         break;

      case eDB2_ET_QMI_DMS_RSP:
      case eDB2_ET_QMI_DMS_IND:
         pt = ePROTOCOL_QMI_DMS_RX;
         break;

      case eDB2_ET_QMI_NAS_REQ:
         pt = ePROTOCOL_QMI_NAS_TX;
         break;

      case eDB2_ET_QMI_NAS_RSP:
      case eDB2_ET_QMI_NAS_IND:
         pt = ePROTOCOL_QMI_NAS_RX;
         break;

      case eDB2_ET_QMI_QOS_REQ:
         pt = ePROTOCOL_QMI_QOS_TX;
         break;

      case eDB2_ET_QMI_QOS_RSP:
      case eDB2_ET_QMI_QOS_IND:
         pt = ePROTOCOL_QMI_QOS_RX;
         break;

      case eDB2_ET_QMI_WMS_REQ:
         pt = ePROTOCOL_QMI_WMS_TX;
         break;

      case eDB2_ET_QMI_WMS_RSP:
      case eDB2_ET_QMI_WMS_IND:
         pt = ePROTOCOL_QMI_WMS_RX;
         break;

      case eDB2_ET_QMI_PDS_REQ:
         pt = ePROTOCOL_QMI_PDS_TX;
         break;

      case eDB2_ET_QMI_PDS_RSP:
      case eDB2_ET_QMI_PDS_IND:
         pt = ePROTOCOL_QMI_PDS_RX;
         break;

      case eDB2_ET_QMI_AUTH_REQ:
         pt = ePROTOCOL_QMI_AUTH_TX;
         break;

      case eDB2_ET_QMI_AUTH_RSP:
      case eDB2_ET_QMI_AUTH_IND:
         pt = ePROTOCOL_QMI_AUTH_RX;
         break;

      case eDB2_ET_QMI_VOICE_REQ:
         pt = ePROTOCOL_QMI_VOICE_TX;
         break;

      case eDB2_ET_QMI_VOICE_RSP:
      case eDB2_ET_QMI_VOICE_IND:
         pt = ePROTOCOL_QMI_VOICE_RX;
         break;

      case eDB2_ET_QMI_CAT_REQ:
         pt = ePROTOCOL_QMI_CAT_TX;
         break;

      case eDB2_ET_QMI_CAT_RSP:
      case eDB2_ET_QMI_CAT_IND:
         pt = ePROTOCOL_QMI_CAT_RX;
         break;

      case eDB2_ET_QMI_RMS_REQ:
         pt = ePROTOCOL_QMI_RMS_TX;
         break;

      case eDB2_ET_QMI_RMS_RSP:
      case eDB2_ET_QMI_RMS_IND:
         pt = ePROTOCOL_QMI_RMS_RX;
         break;

      case eDB2_ET_QMI_OMA_REQ:
         pt = ePROTOCOL_QMI_OMA_TX;
         break;

      case eDB2_ET_QMI_OMA_RSP:
      case eDB2_ET_QMI_OMA_IND:
         pt = ePROTOCOL_QMI_OMA_RX;
         break;

      case eDB2_ET_QMI_CTL_REQ:
         pt = ePROTOCOL_QMI_CTL_TX;
         break;

      case eDB2_ET_QMI_CTL_RSP:
      case eDB2_ET_QMI_CTL_IND:
         pt = ePROTOCOL_QMI_CTL_RX;
         break;
   }

   return pt;
}

/*===========================================================================
METHOD:
   MapQMIProtocolTypeToEntityType (Public Free Method)

DESCRIPTION:
   Map a buffer protocol type to a DB protocol entity type

PARAMETERS:
   pt          [ I ] - Protocol type
   bIndication [ I ] - Is this for an indication?
  
RETURN VALUE:
   eDB2EntityType
===========================================================================*/
eDB2EntityType MapQMIProtocolTypeToEntityType( 
   eProtocolType              pt,
   bool                       bIndication )
{
   eDB2EntityType et = eDB2_ET_ENUM_BEGIN;
   switch (pt)
   {
      case ePROTOCOL_QMI_WDS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_WDS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_WDS_RSP;
         }
         break;

      case ePROTOCOL_QMI_WDS_TX:
         et = eDB2_ET_QMI_WDS_REQ;
         break;

      case ePROTOCOL_QMI_DMS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_DMS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_DMS_RSP;
         }
         break;

      case ePROTOCOL_QMI_DMS_TX:
         et = eDB2_ET_QMI_DMS_REQ;
         break;

      case ePROTOCOL_QMI_NAS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_NAS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_NAS_RSP;
         }
         break;

      case ePROTOCOL_QMI_NAS_TX:
         et = eDB2_ET_QMI_NAS_REQ;
         break;

      case ePROTOCOL_QMI_QOS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_QOS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_QOS_RSP;
         }
         break;

      case ePROTOCOL_QMI_QOS_TX:
         et = eDB2_ET_QMI_QOS_REQ;
         break;

      case ePROTOCOL_QMI_WMS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_WMS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_WMS_RSP;
         }
         break;

      case ePROTOCOL_QMI_WMS_TX:
         et = eDB2_ET_QMI_WMS_REQ;
         break;

      case ePROTOCOL_QMI_PDS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_PDS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_PDS_RSP;
         }
         break;

      case ePROTOCOL_QMI_PDS_TX:
         et = eDB2_ET_QMI_PDS_REQ;
         break;

               case ePROTOCOL_QMI_AUTH_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_AUTH_IND;
         }
         else
         {
            et = eDB2_ET_QMI_AUTH_RSP;
         }
         break;

      case ePROTOCOL_QMI_AUTH_TX:
         et = eDB2_ET_QMI_AUTH_REQ;
         break;

      case ePROTOCOL_QMI_VOICE_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_VOICE_IND;
         }
         else
         {
            et = eDB2_ET_QMI_VOICE_RSP;
         }
         break;

      case ePROTOCOL_QMI_VOICE_TX:
         et = eDB2_ET_QMI_VOICE_REQ;
         break;

      case ePROTOCOL_QMI_CAT_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_CAT_IND;
         }
         else
         {
            et = eDB2_ET_QMI_CAT_RSP;
         }
         break;

      case ePROTOCOL_QMI_CAT_TX:
         et = eDB2_ET_QMI_CAT_REQ;
         break;

      case ePROTOCOL_QMI_RMS_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_RMS_IND;
         }
         else
         {
            et = eDB2_ET_QMI_RMS_RSP;
         }
         break;

      case ePROTOCOL_QMI_RMS_TX:
         et = eDB2_ET_QMI_RMS_REQ;
         break;

      case ePROTOCOL_QMI_OMA_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_OMA_IND;
         }
         else
         {
            et = eDB2_ET_QMI_OMA_RSP;
         }
         break;

      case ePROTOCOL_QMI_OMA_TX:
         et = eDB2_ET_QMI_OMA_REQ;
         break;

      case ePROTOCOL_QMI_CTL_RX:
         if (bIndication == true)
         {
            et = eDB2_ET_QMI_CTL_IND;
         }
         else
         {
            et = eDB2_ET_QMI_CTL_RSP;
         }
         break;

      case ePROTOCOL_QMI_CTL_TX:
         et = eDB2_ET_QMI_CTL_REQ;
         break;
   }

   return et;
}

/*===========================================================================
METHOD:
   DB2GetMaxBufferSize (Public Free Method)

DESCRIPTION:
   Return the maximum size of a payload buffer for given type of 
   protocol entity

PARAMETERS:
   et          [ I ] - Protocol entity type
  
RETURN VALUE:
   ULONG - Maximum 
===========================================================================*/
ULONG DB2GetMaxBufferSize( eDB2EntityType et )
{
   ULONG maxSzInBytes = MAX_SHARED_BUFFER_SIZE;

   if (IsQMIEntityType( et ) == true)
   {
      // QMI items are further constrained in size
      maxSzInBytes = QMI_MAX_BUFFER_SIZE;
   }

   return maxSzInBytes;
}

/*===========================================================================
METHOD:
   DB2BuildQMIBuffer (Internal Method)

DESCRIPTION:
   Build and return an allocated shared buffer for the QMI protocol using 
   the specified DB keys and payloads (one per TLV content)

PARAMETERS:
   input       [ I ] - Protocol entity key and payload
  
RETURN VALUE:
   sSharedBuffer * (0 upon failure)
===========================================================================*/
sSharedBuffer * DB2BuildQMIBuffer(
   const std::vector <sDB2NavInput> &  input )
{
   // Assume failure
   sSharedBuffer * pRef = 0;

   const ULONG szTransHdr = (ULONG)sizeof(sQMIServiceRawTransactionHeader);
   const ULONG szMsgHdr   = (ULONG)sizeof(sQMIRawMessageHeader);
   const ULONG szTLVHdr   = (ULONG)sizeof(sQMIRawContentHeader);

   // Need something to build (but not too much)
   ULONG tlvs = (ULONG)input.size();
   if (tlvs == 0 || tlvs > (ULONG)UCHAR_MAX)
   {
      return pRef;
   }

   // The protocol entity keys need to be consistent
   const sDB2NavInput & tlvInput = input[0];
   if (tlvInput.IsValid() == false || tlvInput.mKey.size() < 3)
   {
      return pRef;
   }

   eDB2EntityType et = (eDB2EntityType)tlvInput.mKey[0];
   if (IsQMIEntityType( et ) == false)
   {
      return pRef;
   }

   ULONG t = 0;
   for (t = 0; t < tlvs; t++)
   {
      const sDB2NavInput & tlvInput = input[t];
      if (tlvInput.mPayloadLen > QMI_MAX_BUFFER_SIZE)
      {
         return pRef;
      }
   }

   ULONG szReq = szTransHdr + szMsgHdr + (tlvs * szTLVHdr);
   szReq += tlvInput.mPayloadLen;

   for (t = 1; t < tlvs; t++)
   {
      const sDB2NavInput & tlv2Input = input[t];
      if (tlv2Input.IsValid() == false || tlv2Input.mKey.size() < 3)
      {
         return pRef;
      }

      if ( (tlvInput.mKey[0] != tlv2Input.mKey[0])
      ||   (tlvInput.mKey[1] != tlv2Input.mKey[1]) )
      {
         return pRef;
      }

      szReq += tlv2Input.mPayloadLen;
   }

   // What we are building cannot be too large
   if (szReq > QMI_MAX_BUFFER_SIZE)
   {
      return pRef;
   }

   BYTE buf[QMI_MAX_BUFFER_SIZE];

   sQMIRawContentHeader * pTLV = 0;
   pTLV = (sQMIRawContentHeader *)&buf[0];

   for (t = 0; t < tlvs; t++)
   {
      const sDB2NavInput & tlv2Input = input[t];

      pTLV->mTypeID = (BYTE)tlv2Input.mKey[2];
      pTLV->mLength = (WORD)tlv2Input.mPayloadLen;
      pTLV++;

      const BYTE * pPayload = (const BYTE *)pTLV;
      memcpy( (LPVOID)pPayload, 
              (LPCVOID)tlv2Input.mpPayload, 
              (SIZE_T)tlv2Input.mPayloadLen );

      pPayload += tlv2Input.mPayloadLen;
      pTLV = (sQMIRawContentHeader *)pPayload;
   }

   ULONG contentLen = szReq - szTransHdr - szMsgHdr;
   eQMIService st = MapQMIEntityTypeToQMIServiceType( et );

   pRef = sQMIServiceBuffer::BuildBuffer( st,
                                          (WORD)tlvInput.mKey[1],
                                          IsQMIEntityResponseType( et ),
                                          IsQMIEntityIndicationType( et ),
                                          &buf[0],
                                          contentLen );

   return pRef;
}

/*===========================================================================
METHOD:
   DB2PackQMIBuffer (Internal Method)

DESCRIPTION:
   Build an allocated shared buffer for the QMI protocol

PARAMETERS:
   db          [ I ] - Database to use for packing
   input       [ I ] - Protocol entity key and field values
  
RETURN VALUE:
   sSharedBuffer * (0 upon failure)
===========================================================================*/
sSharedBuffer * DB2PackQMIBuffer(
   const cCoreDatabase &                  db,
   const std::vector <sDB2PackingInput> & input )
{
   // Assume failure
   sSharedBuffer * pRef = 0;

   // Need something to build (but not too much)
   ULONG tlvs = (ULONG)input.size();
   if (tlvs == 0 || tlvs > (ULONG)UCHAR_MAX)
   {
      return pRef;
   }

   // The protocol entity keys need to be consistent
   const sDB2PackingInput & tlvInput = input[0];
   if (tlvInput.IsValid() == false || tlvInput.mKey.size() < 3)
   {
      return pRef;
   }

   eDB2EntityType et = (eDB2EntityType)tlvInput.mKey[0];
   if (IsQMIEntityType( et ) == false)
   {
      return pRef;
   }

   ULONG t = 0;
   for (t = 0; t < tlvs; t++)
   {
      const sDB2PackingInput & tlvInput = input[t];
      if (tlvInput.mDataLen > QMI_MAX_BUFFER_SIZE)
      {
         return pRef;
      }
   }

   for (t = 1; t < tlvs; t++)
   {
      const sDB2PackingInput & tlv2Input = input[t];
      if (tlv2Input.IsValid() == false || tlv2Input.mKey.size() < 3)
      {
         return pRef;
      }

      if ( (tlvInput.mKey[0] != tlv2Input.mKey[0])
      ||   (tlvInput.mKey[1] != tlv2Input.mKey[1]) )
      {
         return pRef;
      }
   }

   BYTE buf[QMI_MAX_BUFFER_SIZE];
   ULONG bufLen = 0;

   sQMIRawContentHeader * pTLV = 0;
   pTLV = (sQMIRawContentHeader *)&buf[0];

   bool bOK = true;
   for (t = 0; t < tlvs; t++)
   {
      ULONG packedLen = 0;
      const BYTE * pPackedData = 0;

      const sDB2PackingInput & tlv2Input = input[t];

      if (tlv2Input.mbString == true)
      {
         if (tlv2Input.mValues.empty() == false)
         {
            // Convert field string to input fields
            std::list <sUnpackedField> fields 
               = cDataPacker::LoadValues( tlv2Input.mValues );

            // Now pack
            cDataPacker dp( db, tlv2Input.mKey, fields );
            bOK = dp.Pack();
            if (bOK == false)
            {
               break;
            }

            pPackedData = dp.GetBuffer( packedLen );
            if (pPackedData == 0)
            {
               bOK = false;
               break;
            }
         }
      }
      else
      {
         packedLen = tlv2Input.mDataLen;
         pPackedData = tlv2Input.mpData;
      }

      // Check if we need to adjust buffer
      cProtocolEntityFieldEnumerator pefe( db, tlv2Input.mKey );
      bool bEnum = pefe.Enumerate();
      if (bEnum == true)
      {
         const std::vector <ULONG> & fieldIDs = pefe.GetFields();
         ULONG fieldCount = (ULONG)fieldIDs.size();
         if (fieldCount == 1)
         {
            const tDB2FieldMap & dbFields = db.GetProtocolFields();

            tDB2FieldMap::const_iterator pField = dbFields.find( fieldIDs[0] );
            if (pField != dbFields.end())
            {
               const sDB2Field & theField = pField->second;
               if ( (theField.mType == eDB2_FIELD_STD)
               &&   (theField.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_ANT) )
               {
                  // For QMI we need to strip out the trailing NULL 
                  // string terminator when the TLV consists solely
                  // of a string since the length contained in the 
                  // TLV structure itself renders the trailing NULL 
                  // redundant
                  if (packedLen > 2)
                  {
                     packedLen--;
                  }
                  else 
                  {
                     // This is the only way to specify an empty string in QMI
                     // when the TLV consists solely of a string
                     if (packedLen == 1)
                     {
                        packedLen--;
                     }
                  }
               }
            }
         }
      }

      bufLen += (ULONG)sizeof(sQMIRawContentHeader);
      bufLen += packedLen;

      // What we are building cannot be too large
      if (bufLen > QMI_MAX_BUFFER_SIZE)
      {
         bOK = false;
         break;
      }

      pTLV->mTypeID = (BYTE)tlv2Input.mKey[2];
      pTLV->mLength = (WORD)packedLen;
      pTLV++;

      const BYTE * pPayload = (const BYTE *)pTLV;
      memcpy( (LPVOID)pPayload, 
              (LPCVOID)pPackedData, 
              (SIZE_T)packedLen );

      pPayload += packedLen;
      pTLV = (sQMIRawContentHeader *)pPayload;
   }

   if (bOK == false)
   {
      return pRef;
   }

   eQMIService st = MapQMIEntityTypeToQMIServiceType( et );
   pRef = sQMIServiceBuffer::BuildBuffer( st,
                                          (WORD)tlvInput.mKey[1],
                                          IsQMIEntityResponseType( et ),
                                          IsQMIEntityIndicationType( et ),
                                          &buf[0],
                                          bufLen );

   return pRef;
}

/*===========================================================================
METHOD:
   DB2ReduceQMIBuffer (Public Free Method)

DESCRIPTION:
   Reduce a DIAG buffer to a DB key and payload

PARAMETERS:
   buf         [ I ] - Protocol buffer being reduced
  
RETURN VALUE:
   sDB2NavInput (invalid upon failure)
===========================================================================*/ 
std::vector <sDB2NavInput> DB2ReduceQMIBuffer( const sProtocolBuffer & buf )
{
   std::vector <sDB2NavInput> retInput;

   // We must have a valid protocol buffer
   if (buf.IsValid() == false)
   {
      return retInput;
   }

   eProtocolType pt = (eProtocolType)buf.GetType();
   if (IsQMIProtocol( pt ) != true)
   {
      return retInput;
   }

   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );
   std::map <ULONG, const sQMIRawContentHeader *> tlvs = qmiBuf.GetContents();

   bool bErr = false;
   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   for (pIter = tlvs.begin(); pIter != tlvs.end(); pIter++)
   {
      const sQMIRawContentHeader * pHdr = pIter->second;
      if (pHdr == 0)
      {
         bErr = true;
         break;
      }

      bool bIndication = qmiBuf.IsIndication();
      eProtocolType pt = (eProtocolType)qmiBuf.GetType();
      eDB2EntityType et = MapQMIProtocolTypeToEntityType( pt, bIndication );

      sDB2NavInput tmp;
      tmp.mKey.push_back( (ULONG)et );
      tmp.mKey.push_back( qmiBuf.GetMessageID() );
      tmp.mKey.push_back( (ULONG)pHdr->mTypeID );
      
      tmp.mPayloadLen = pHdr->mLength;
      pHdr++;

      tmp.mpPayload = (const BYTE *)pHdr;
      if (tmp.IsValid() == true)
      {
         retInput.push_back( tmp );
      }
      else
      {
         // Ignore empty TLVs
         if (tmp.mPayloadLen != 0)
         {
            bErr = true;
            break;
         }
      }
   }

   if (bErr == true)
   {
      retInput.clear();
   }

   return retInput;
}
