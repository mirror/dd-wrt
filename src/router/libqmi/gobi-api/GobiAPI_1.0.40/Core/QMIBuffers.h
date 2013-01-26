/*===========================================================================
FILE:
   QMIBuffers.h

DESCRIPTION:
   QMI service protocol related structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sQMUXHeader
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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "ProtocolBuffer.h"
#include "QMIEnum.h"

#include <map>
#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// QMI maximum buffer size (cannot be larger than MAX_SHARED_BUFFER_SIZE) 
const ULONG QMI_MAX_BUFFER_SIZE = MAX_SHARED_BUFFER_SIZE;

// Content ID for mandatory result TLV
const ULONG QMI_TLV_ID_RESULT = 2;

/*===========================================================================
METHOD:
   MapQMIServiceToProtocol (Inline Method)

DESCRIPTION:
   Map QMI service type (eQMIService) and direction to a protocol type
   (eProtocolType)

PARAMETERS:
   serviceType    [ I ] - Enum value being mapped
   bTransmission  [ I ] - IS this a transmission (TX vs. RX)?

RETURN VALUE:
   bool
===========================================================================*/
inline eProtocolType MapQMIServiceToProtocol( 
   eQMIService                serviceType,
   bool                       bTransmission = true )
{
   eProtocolType pt = ePROTOCOL_ENUM_BEGIN;
   switch (serviceType)
   {
      case eQMI_SVC_WDS:
         pt = ePROTOCOL_QMI_WDS_RX;
         break;

      case eQMI_SVC_DMS:
         pt = ePROTOCOL_QMI_DMS_RX;
         break;

      case eQMI_SVC_NAS:
         pt = ePROTOCOL_QMI_NAS_RX;
         break;

      case eQMI_SVC_QOS:
         pt = ePROTOCOL_QMI_QOS_RX;
         break;

      case eQMI_SVC_WMS:
         pt = ePROTOCOL_QMI_WMS_RX;
         break;

      case eQMI_SVC_PDS:
         pt = ePROTOCOL_QMI_PDS_RX;
         break;

      case eQMI_SVC_AUTH:
         pt = ePROTOCOL_QMI_AUTH_RX;
         break;

      case eQMI_SVC_VOICE:
         pt = ePROTOCOL_QMI_VOICE_RX;
         break;

      case eQMI_SVC_CAT:
         pt = ePROTOCOL_QMI_CAT_RX;
         break;

      case eQMI_SVC_RMS:
         pt = ePROTOCOL_QMI_RMS_RX;
         break;

      case eQMI_SVC_OMA:
         pt = ePROTOCOL_QMI_OMA_RX;
         break;

      case eQMI_SVC_CONTROL:
         pt = ePROTOCOL_QMI_CTL_RX;
         break;

   }

   if (pt != ePROTOCOL_ENUM_BEGIN && bTransmission == true)
   {
      // This relies on the fact the the TX variant is always the next
      // enumerated value after the RX variant (so don't do something
      // to change that)
      pt  = (eProtocolType)((ULONG)pt + 1);
   }

   return pt;
};

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sQMUXHeader
//    Struct to represent a QMUX transaction header (raw)
/*=========================================================================*/
struct sQMUXHeader
{
   public:
      WORD mLength;   
      BYTE mFlags;
      BYTE mServiceType;
      BYTE mClientID; 
};

/*=========================================================================*/
// Struct sQMIControlRawTransactionHeader
//    Struct to represent a QMI control transaction header (raw)
/*=========================================================================*/
struct sQMIControlRawTransactionHeader
{
   public:
      BYTE mResponse   : 1;   // Is this a response transaction?
      BYTE mIndication : 1;   // Is this an indication transaction?
      BYTE mReserved   : 6; 

      BYTE mTransactionID;    // Transaction ID
};

/*=========================================================================*/
// Struct sQMIServiceRawTransactionHeader
//    Struct to represent a QMI service transaction header (raw)
/*=========================================================================*/
struct sQMIServiceRawTransactionHeader
{
   public:
      BYTE mCompound   : 1;   // Is this a compound transaction?
      BYTE mResponse   : 1;   // Is this a response transaction?
      BYTE mIndication : 1;   // Is this an indication transaction?
      BYTE mReserved   : 5; 

      WORD mTransactionID;    // Transaction ID
};

/*=========================================================================*/
// Struct sQMIRawMessageHeader
//    Struct to represent a QMI (control/service) message header (raw)
/*=========================================================================*/
struct sQMIRawMessageHeader
{
   public:
      WORD mMessageID;        // Message ID
      WORD mLength;           // Length of message (not including this header)
};

/*=========================================================================*/
// Struct sQMIRawContentHeader
//    Struct to represent a QMI (control/service) content 
//    (i.e Type/Length/Value, TLV) header (raw)
/*=========================================================================*/
struct sQMIRawContentHeader
{
   public:
      BYTE mTypeID;           // Content type ID
      WORD mLength;           // Content length (not including this header)
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )


/*=========================================================================*/
// Struct sQMIServiceBuffer
//    Struct to represent a QMI service channel request/response/indication 
//    (shared buffer)
/*=========================================================================*/
struct sQMIServiceBuffer : public sProtocolBuffer
{
   public:
      // Constructor
      sQMIServiceBuffer( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQMIServiceBuffer();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;

         const sQMIServiceRawTransactionHeader * pHdr = GetHeader();
         if (pHdr != 0)
         {
            bRequest = (pHdr->mResponse == 0 && pHdr->mIndication == 0);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;

         const sQMIServiceRawTransactionHeader * pHdr = GetHeader();
         if (pHdr != 0)
         {
            bResponse = (pHdr->mResponse == 1);
         }

         return bResponse;
      };

      // (Inline) Is this an indication?
      bool IsIndication() const
      {
         bool bInd = false;

         const sQMIServiceRawTransactionHeader * pHdr = GetHeader();
         if (pHdr != 0)
         {
            bInd = (pHdr->mIndication == 1);
         }

         return bInd;
      };

      // (Inline) Return raw header
      const sQMIServiceRawTransactionHeader * GetHeader() const
      {
         const sQMIServiceRawTransactionHeader * pHdr = 0;
         if (IsValid() == true)
         {
            pHdr = (const sQMIServiceRawTransactionHeader *)GetBuffer();
         }

         return pHdr;
      };

      // (Inline) Return the message ID
      ULONG GetMessageID() const
      {
         ULONG id = (ULONG)ULONG_MAX;

         const sQMIServiceRawTransactionHeader * pHdr = GetHeader();
         if (pHdr != 0)
         {
            pHdr++;
            const sQMIRawMessageHeader * pMsgHdr = 0;
            pMsgHdr = (sQMIRawMessageHeader *)pHdr;

            id = pMsgHdr->mMessageID;
         }

         return id;
      };

      // (Inline) Return the transaction ID
      WORD GetTransactionID() const
      {
         WORD id = (WORD)INVALID_QMI_TRANSACTION_ID;

         const sQMIServiceRawTransactionHeader * pHdr = GetHeader();
         if (pHdr != 0)
         {
            id = pHdr->mTransactionID;
         }

         return id;
      };

      // (Inline) Return content structures
      std::map <ULONG, const sQMIRawContentHeader *> GetContents() const
      {
         return mContents;
      };

      // Return contents of mandatory result content
      bool GetResult( 
         ULONG &                    returnCode,
         ULONG &                    errorCode );

      // Build a QMI request/response/indication
      static sSharedBuffer * BuildBuffer( 
         eQMIService                serviceType,
         WORD                       msgID,
         bool                       bResponse = false,
         bool                       bIndication = false,
         const BYTE *               pData = 0,
         ULONG                      dataLen = 0 );

   protected:
      // QMI protocol server has to be able to set the transaction ID
      friend class cQMIProtocolServer;

      // Set the transaction ID
      void SetTransactionID( WORD tid ) const
      {
         if (tid == (WORD)INVALID_QMI_TRANSACTION_ID || IsValid() == false)
         {
            return;
         }

         sQMIServiceRawTransactionHeader * pHdr = 0;
         pHdr = (sQMIServiceRawTransactionHeader *)GetHeader();
         if (pHdr != 0)
         {
            pHdr->mTransactionID = tid;
         }
      };

      // Is this QMI request/response/indication packet valid?
      virtual bool Validate();

      /* Content TLV structures (indexed by type ID) */
      std::map <ULONG, const sQMIRawContentHeader *> mContents;

   private:
      // Prevent 'upcopying'
      sQMIServiceBuffer( const sProtocolBuffer & );
      sQMIServiceBuffer & operator = ( const sProtocolBuffer & );
};

