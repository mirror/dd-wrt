/*===========================================================================
FILE:
   GobiCMCallback.cpp

DESCRIPTION:
   Contains the implementation of each Gobi CM callback function.
   
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
#include "SampleCM.h"
#include "GobiCMCallback.h"
#include <sstream>

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   WDSEventReportCallback (Free Method)

DESCRIPTION:
   Function called by WDS event report callback

PARAMETERS:
   svcID       [ I ] - QMI service ID
   msgID       [ I ] - QMI message ID
   handle      [ I ] - Handle to Gobi API connection
   outLen      [ I ] - Length of indication buffer
   pOut        [ I ] - Indication buffer

RETURN VALUE:
   None
===========================================================================*/
void WDSEventReportCallback(
   ULONG                         svcID,
   ULONG                         msgID,
   GOBIHANDLE                    /* handle */,
   ULONG                         outLen,
   const BYTE *                  pOut )
{
   if (gpCM == 0 || svcID != 1 || msgID != 1)
   {
      return;
   }

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &pOut[0], outLen );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( 0x17 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      if (pTmp->mLength >= sizeof (sWDSEventReportIndication_DataBearerTechnology))
      {
         pTmp++;
         const sWDSEventReportIndication_DataBearerTechnology * pDBT =
            (const sWDSEventReportIndication_DataBearerTechnology *)pTmp;

         gpCM->OnDataBearerCBNotification( pDBT->mDataBearerTechnology );
      }
   }

   ULONGLONG txTotalBytes = ULLONG_MAX;
   ULONGLONG rxTotalBytes = ULLONG_MAX;

   pIter = tlvs.find( 0x19 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      if (pTmp->mLength >= sizeof (sWDSEventReportIndication_TXBytes))
      {
         pTmp++;
         const sWDSEventReportIndication_TXBytes * pTX =
            (const sWDSEventReportIndication_TXBytes *)pTmp;

         txTotalBytes = pTX->mTXByteTotal;
      }
   }

   pIter = tlvs.find( 0x1A );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      if (pTmp->mLength >= sizeof (sWDSEventReportIndication_RXBytes))
      {
         pTmp++;
         const sWDSEventReportIndication_RXBytes * pRX =
            (const sWDSEventReportIndication_RXBytes *)pTmp;

         rxTotalBytes = pRX->mRXByteTotal;
      }
   }

   if (txTotalBytes != ULLONG_MAX || rxTotalBytes != ULLONG_MAX)
   {
      gpCM->OnByteTotalsNotification( rxTotalBytes, txTotalBytes );
   }
}

/*===========================================================================
METHOD:
   WDSSessionStateCallback (Free Method)

DESCRIPTION:
   Function called by WDS packet service status callback

PARAMETERS:
   svcID       [ I ] - QMI service ID
   msgID       [ I ] - QMI message ID
   handle      [ I ] - Handle to Gobi API connection
   outLen      [ I ] - Length of indication buffer
   pOut        [ I ] - Indication buffer

RETURN VALUE:
   None
===========================================================================*/
void WDSSessionStateCallback(
   ULONG                         svcID,
   ULONG                         msgID,
   GOBIHANDLE                    /* handle */,
   ULONG                         outLen,
   const BYTE *                  pOut )
{
   if (gpCM == 0 || svcID != 1 || msgID != 34)
   {
      return;
   }

   ULONG state = ULONG_MAX;

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &pOut[0], outLen );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( 0x01 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      if (pTmp->mLength >= sizeof (sWDSPacketServiceStatusReportIndication_Status))
      {
         pTmp++;
         const sWDSPacketServiceStatusReportIndication_Status * pState =
            (const sWDSPacketServiceStatusReportIndication_Status *)pTmp;

         state = pState->mConnectionStatus;
      }
   }

   if (state != ULONG_MAX)
   {
      gpCM->OnSessionStateCBNotification( state );
   }
}

/*===========================================================================
METHOD:
   NASEventReportCallback (Free Method)

DESCRIPTION:
   Function called by NAS event report callback

PARAMETERS:
   svcID       [ I ] - QMI service ID
   msgID       [ I ] - QMI message ID
   handle      [ I ] - Handle to Gobi API connection
   outLen      [ I ] - Length of indication buffer
   pOut        [ I ] - Indication buffer

RETURN VALUE:
   None
===========================================================================*/
void NASEventReportCallback(
   ULONG                         svcID,
   ULONG                         msgID,
   GOBIHANDLE                    /* handle */,
   ULONG                         outLen,
   const BYTE *                  pOut )
{
   if (gpCM == 0 || svcID != 3 || msgID != 2)
   {
      return;
   }

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &pOut[0], outLen );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( 0x10 );
   if (pIter == tlvs.end())
   {
      return;
   }

   const sQMIRawContentHeader * pTmp = pIter->second;
   if (pTmp->mLength >= sizeof (sNASEventReportIndication_SignalStrength))
   {
      pTmp++;
      const sNASEventReportIndication_SignalStrength * pSS =
         (const sNASEventReportIndication_SignalStrength *)pTmp;

      gpCM->OnSignalStrengthCBNotificaion( pSS->mSignalStrengthdBm, 
                                           pSS->mRadioInterface );
   }
}

/*===========================================================================
METHOD:
   NASServingSystemCallback (Free Method)

DESCRIPTION:
   Function called by NAS serving system callback

PARAMETERS:
   svcID       [ I ] - QMI service ID
   msgID       [ I ] - QMI message ID
   handle      [ I ] - Handle to Gobi API connection
   outLen      [ I ] - Length of indication buffer
   pOut        [ I ] - Indication buffer

RETURN VALUE:
   None
===========================================================================*/
void NASServingSystemCallback(
   ULONG                         svcID,
   ULONG                         msgID,
   GOBIHANDLE                    /* handle */,
   ULONG                         outLen,
   const BYTE *                  pOut )
{
   if (gpCM == 0 || svcID != 3 || msgID != 36)
   {
      return;
   }

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &pOut[0], outLen );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( 0x10 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      if (pTmp->mLength >= sizeof (sNASServingSystemIndication_RoamingIndicator))
      {
         pTmp++;
         const sNASServingSystemIndication_RoamingIndicator * pRI =
            (const sNASServingSystemIndication_RoamingIndicator *)pTmp;

         BYTE roam = pRI->mRoamingIndicator;
         if (roam == 0xFF)
         {
            gpCM->SetRoam( "Unknown" );
         }
         else
         {
            std::ostringstream roamStr;
            roamStr << roam;
            gpCM->SetRoam( roamStr.str() );
         }
      }
   }

   pIter = tlvs.find( 0x11 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      ULONG tlvLen = (ULONG)pTmp->mLength;
      ULONG dsLen = (ULONG)sizeof( sNASServingSystemIndication_DataServices ); 
      if (tlvLen < dsLen)
      {
         return;
      }

      pTmp++;
      const sNASServingSystemIndication_DataServices * pDS =
         (const sNASServingSystemIndication_DataServices *)pTmp;

      ULONG dcCount = (ULONG)pDS->mNumberOfDataCapabilities;
      ULONG dcSz = (ULONG)sizeof( eQMINASDataServiceCapabilities2 );
      dsLen += dcCount * dcSz;
      if (tlvLen < dsLen)
      {
         return;
      }

      pDS++;
      gpCM->OnDataCapsNotification( dcCount,
                                    (eQMINASDataServiceCapabilities2 *)pDS );
   }
}
