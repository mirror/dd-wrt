/*===========================================================================
FILE:
   Gobi3000TranslationNAS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (NAS Service)

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

// Maximum length for a scanned network description
const ULONG MAX_SNI_DESCRIPTION_LEN = 255;

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sEVDOCustomSCPConfig
//    Struct to represent CDMA 1xEV-DO custom SCP config
/*=========================================================================*/
struct sEVDOCustomSCPConfig
{
   public:
      BYTE mbActive;
      ULONG mProtocolMask;
      ULONG mBroadcastMask;
      ULONG mApplicationMask;
};

/*=========================================================================*/
// Struct sScannedNetworkInfo
//    Struct to represent scanned network information
/*=========================================================================*/
struct sScannedNetworkInfo
{
   public:
      USHORT mMCC;
      USHORT mMNC;
      ULONG mInUse;
      ULONG mRoaming;
      ULONG mForbidden;
      ULONG mPreferred;
      CHAR mDescription[MAX_SNI_DESCRIPTION_LEN];
};

/*=========================================================================*/
// Struct sScannedNetworkRATInfo
//    Struct to represent scanned network RAT information
/*=========================================================================*/
struct sScannedNetworkRATInfo
{
   public:
      USHORT mMCC;
      USHORT mMNC;
      ULONG mRAT;
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

/*===========================================================================
METHOD:
   ParseGetANAAAAuthenticationStatus

DESCRIPTION:
   This function gets the AN-AAA authentication status

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pStatus     [ O ] - AN-AAA authentication status

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetANAAAAuthenticationStatus(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pStatus )
{
   // Validate arguments
   if (pIn == 0 || pStatus == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sNASGetANAAAAuthenticationStatusResponse_Status * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sNASGetANAAAAuthenticationStatusResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pStatus = pTLVx01->mANAAAAuthenticationStatus;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSignalStrength

DESCRIPTION:
   This function gets the current signal strength (in dBm) as measured by
   the device, the signal strength returned will be one of the currently
   available technologies with preference CDMA 1xEV-DO, CDMA, AMPS,
   WCDMA, GSM

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pSignalStrength   [ O ] - Received signal strength (dBm)
   pRadioInterface   [ O ] - Radio interface technology

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSignalStrength(
   ULONG             inLen,
   const BYTE *      pIn,
   INT8 *            pSignalStrength,
   ULONG *           pRadioInterface )
{
   // Validate arguments
   if (pSignalStrength == 0 || pRadioInterface == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG sigSz = 12;
   INT8 sigs[12];
   ULONG radios[12];
   ULONG qcErr = ParseGetSignalStrengths( inLen,
                                          pIn,
                                          &sigSz,
                                          &sigs[0],
                                          &radios[0] );
   if (qcErr != eGOBI_ERR_NONE)
   {
      return qcErr;
   }

   std::map <ULONG, INT8> sigMap;
   for (ULONG s = 0; s < sigSz; s++)
   {
      sigMap[radios[s]] = sigs[s];
   }

   std::map <ULONG, INT8>::const_iterator pIter;

   // HDR?
   pIter = sigMap.find( 2 );
   if (pIter != sigMap.end())
   {
      *pSignalStrength = pIter->second;
      *pRadioInterface = pIter->first;

      return eGOBI_ERR_NONE;
   }

   // CDMA?
   pIter = sigMap.find( 1 );
   if (pIter != sigMap.end())
   {
      *pSignalStrength = pIter->second;
      *pRadioInterface = pIter->first;

      return eGOBI_ERR_NONE;
   }

   // AMPS?
   pIter = sigMap.find( 3 );
   if (pIter != sigMap.end())
   {
      *pSignalStrength = pIter->second;
      *pRadioInterface = pIter->first;

      return eGOBI_ERR_NONE;
   }

   // WCDMA?
   pIter = sigMap.find( 5 );
   if (pIter != sigMap.end())
   {
      *pSignalStrength = pIter->second;
      *pRadioInterface = pIter->first;

      return eGOBI_ERR_NONE;
   }

   // GSM?
   pIter = sigMap.find( 4 );
   if (pIter != sigMap.end())
   {
      *pSignalStrength = pIter->second;
      *pRadioInterface = pIter->first;

      return eGOBI_ERR_NONE;
   }

   // Error values
   *pSignalStrength = -128;
   *pRadioInterface = 0;

   return eGOBI_ERR_NO_SIGNAL;
}

/*===========================================================================
METHOD:
   ParseGetSignalStrengths

DESCRIPTION:
   This function gets the current available signal strengths (in dBm)
   as measured by the device

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pArraySizes       [I/O] - Upon input the maximum number of elements
                             that each array can contain can contain.
                             Upon successful output the actual number
                             of elements in each array
   pSignalStrengths  [ O ] - Received signal strength array (dBm)
   pRadioInterfaces  [ O ] - Radio interface technology array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSignalStrengths(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pArraySizes,
   INT8 *            pSignalStrengths,
   ULONG *           pRadioInterfaces )
{
   // Validate arguments
   if (pIn == 0
   ||  pArraySizes == 0
   ||  *pArraySizes == 0
   ||  pSignalStrengths == 0
   ||  pRadioInterfaces == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSignals = (ULONG)*pArraySizes;

   // Assume failure
   *pArraySizes = 0;

   // Find the first signal strength value
   const sNASGetSignalStrengthResponse_SignalStrength * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sNASGetSignalStrengthResponse_SignalStrength ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Weed out bogus values
   std::map <ULONG, INT8> sigMap;

   INT8 sigVal = pTLVx01->mSignalStrengthdBm;
   ULONG radioVal = pTLVx01->mRadioInterface;
   if (sigVal <= -30 && sigVal > -125 && radioVal != 0)
   {
      sigMap[radioVal] = sigVal;
   }

   // Handle list, if present
   const sNASGetSignalStrengthResponse_SignalStrengthList * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sNASGetSignalStrengthResponse_SignalStrengthList ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG auxSigs = pTLVx10->mNumberOfInfoInstances;
      if (auxSigs > maxSignals)
      {
         auxSigs = maxSignals;
      }

      const sNASGetSignalStrengthResponse_SignalStrengthList::sInfo * pInfo;

      // Verify there is room for the array in the TLV
      if (outLenx10 < sizeof( sNASGetSignalStrengthResponse_SignalStrengthList )
                    + sizeof( sNASGetSignalStrengthResponse_SignalStrengthList::sInfo ) 
                      * auxSigs)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Align to the first array element
      pInfo = (const sNASGetSignalStrengthResponse_SignalStrengthList::sInfo *)
              ((const BYTE *)pTLVx10 
              + sizeof( sNASGetSignalStrengthResponse_SignalStrengthList ));

      for (ULONG s = 0; s < auxSigs; s++)
      {
         sigVal = pInfo->mSignalStrengthdBm;
         radioVal = pInfo->mRadioInterface;
         if (sigVal <= -30 && sigVal > -125 && radioVal != 0)
         {
            sigMap[radioVal] = sigVal;
         }

         // Move pInfo forward one element
         pInfo++;
      }
   }

   ULONG sigCount = 0;
   std::map <ULONG, INT8>::const_iterator pIter;
   for (pIter = sigMap.begin(); pIter != sigMap.end(); pIter++, sigCount++)
   {
      if (sigCount < maxSignals)
      {
         pSignalStrengths[sigCount] = pIter->second;
         pRadioInterfaces[sigCount] = pIter->first;
         *pArraySizes = sigCount + 1;
      }
   }

   // No valid signals?
   if (sigCount == 0)
   {
      return eGOBI_ERR_NO_SIGNAL;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetRFInfo

DESCRIPTION:
   This function gets the current RF information

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the
                          RF info instance array can contain.  Upon success
                          the actual number of elements in the RF info
                          instance array
   pInstances     [ O ] - The RF info instance array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetRFInfo(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances )
{
   // Validate arguments
   if (pIn == 0
   ||  pInstanceSize == 0
   ||  *pInstanceSize == 0
   ||  pInstances == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   BYTE maxInstances = *pInstanceSize;
   *pInstanceSize = 0;

   // Find the TLV
   const sNASGetRFInfoResponse_RFInfo * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sNASGetRFInfoResponse_RFInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   BYTE ifaceCount = pTLVx01->mNumberOfInstances;
   if (ifaceCount > maxInstances)
   {
      ifaceCount = maxInstances;
   }

   const sNASGetRFInfoResponse_RFInfo::sInstance * pInstance;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sNASGetRFInfoResponse_RFInfo )
                  + sizeof( sNASGetRFInfoResponse_RFInfo::sInstance ) 
                    * ifaceCount)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pInstance = (const sNASGetRFInfoResponse_RFInfo::sInstance *)
               ((const BYTE *)pTLVx01 
               + sizeof( sNASGetRFInfoResponse_RFInfo ));

   ULONG * pOutput = (ULONG *)pInstances;
   for (BYTE i = 0; i < ifaceCount; i++)
   {
      *pOutput++ = pInstance->mRadioInterface;
      *pOutput++ = pInstance->mActiveBandClass;
      *pOutput++ = pInstance->mActiveChannel;

      // Move pInstance forward one element
      pInstance++;
   }

   *pInstanceSize = ifaceCount;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParsePerformNetworkScan

DESCRIPTION:
   This function performs a scan for available networks

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the
                          network info instance array can contain.  Upon
                          success the actual number of elements in the
                          network info instance array
   pInstances     [ O ] - The network info instance array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParsePerformNetworkScan(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances )
{
   // Validate arguments
   if (pIn == 0
   ||  pInstanceSize == 0
   ||  *pInstanceSize == 0
   ||  pInstances == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxInstances = *pInstanceSize;

   // Assume failure
   *pInstanceSize = 0;

   // Find the TLV
   const sNASPerformNetworkScanResponse_NetworkInfo * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sNASPerformNetworkScanResponse_NetworkInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   UINT16 netCount = pTLVx10->mNumberOfInfoInstances;
   if (netCount > maxInstances)
   {
      netCount = maxInstances;
   }

   const sNASPerformNetworkScanResponse_NetworkInfo::sNetworkInfo * pNetInfo;

   // Align to the first array element
   pNetInfo = (const sNASPerformNetworkScanResponse_NetworkInfo::sNetworkInfo *)
              ((const BYTE *)pTLVx10 
              + sizeof( sNASPerformNetworkScanResponse_NetworkInfo ));
   ULONG offset = sizeof( sNASPerformNetworkScanResponse_NetworkInfo );

   sScannedNetworkInfo * pNet = (sScannedNetworkInfo *)pInstances;
   for (BYTE i = 0; i < netCount; i++)
   {
      // Check TLV size
      if (offset > outLenx10)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      pNet->mMCC       = pNetInfo->mMobileCountryCode;
      pNet->mMNC       = pNetInfo->mMobileNetworkCode;
      pNet->mInUse     = pNetInfo->mInUseStatus;
      pNet->mRoaming   = pNetInfo->mRoamingStatus;
      pNet->mForbidden = pNetInfo->mForbiddenStatus;
      pNet->mPreferred = pNetInfo->mPreferredStatus;

      memset( &pNet->mDescription[0], 0, MAX_SNI_DESCRIPTION_LEN );

      BYTE descLen = pNetInfo->mDescriptionLength;
      if (descLen > 0)
      {
         // Move pNetInfo forward
         pNetInfo++;
         offset += sizeof( sNASPerformNetworkScanResponse_NetworkInfo::sNetworkInfo );

         // Check TLV size
         if (offset > outLenx10)
         {
            return eGOBI_ERR_MALFORMED_RSP;
         }

         std::string netDesc( (LPCSTR)pNetInfo );

         ULONG actualLen = (ULONG)netDesc.size();
         if (actualLen >= MAX_SNI_DESCRIPTION_LEN)
         {
            actualLen = MAX_SNI_DESCRIPTION_LEN - 1;
         }

         LPCSTR pNetDesc = netDesc.c_str();
         memcpy( &pNet->mDescription[0], pNetDesc, actualLen );

         // Move pNetInfo past string
         pNetInfo = (const sNASPerformNetworkScanResponse_NetworkInfo::sNetworkInfo *)
                    ((const BYTE *)pNetInfo + descLen);
         offset += descLen;
      }

      pNet++;
   }

   *pInstanceSize = (BYTE)netCount;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParsePerformNetworkRATScan

DESCRIPTION:
   This function performs a scan for available networks (includes RAT)

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the
                          network info instance array can contain.  Upon
                          success the actual number of elements in the
                          network info instance array
   pInstances     [ O ] - The network info instance array
   pRATSize       [I/O] - Upon input the maximum number of elements that the
                          RAT info instance array can contain.  Upon success
                          the actual number of elements in the RAT info
                          instance array
   pRATInstances  [ O ] - The RAT info instance array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParsePerformNetworkRATScan(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances,
   BYTE *            pRATSize,
   BYTE *            pRATInstances )
{
   // Validate arguments
   if (pIn == 0
   ||  pInstanceSize == 0
   ||  *pInstanceSize == 0
   ||  pInstances == 0
   ||  pRATSize == 0
   ||  *pRATSize == 0
   ||  pRATInstances == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxRATInstances = *pRATSize;

   // Assume failure
   *pInstanceSize = 0;
   *pRATSize = 0;

   // First, generate the instances using ParsePerformNetworkScan
   ULONG rc = ParsePerformNetworkScan( inLen, pIn, pInstanceSize, pInstances );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Now find the RAT info too

   // Find the TLV
   const sNASPerformNetworkScanResponse_NetworkRAT * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx11 < sizeof( sNASPerformNetworkScanResponse_NetworkRAT ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   UINT16 ratCount = pTLVx11->mNumberOfInfoInstances;
   if (ratCount > maxRATInstances)
   {
      ratCount = maxRATInstances;
   }

   const sNASPerformNetworkScanResponse_NetworkRAT::sInfo * pRatInfo;

   // Verify there is room for the array in the TLV
   if (outLenx11 < sizeof( sNASPerformNetworkScanResponse_NetworkRAT )
                  + sizeof( sNASPerformNetworkScanResponse_NetworkRAT::sInfo ) 
                    * ratCount)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pRatInfo = (const sNASPerformNetworkScanResponse_NetworkRAT::sInfo *)
              ((const BYTE *)pTLVx11 
              + sizeof( sNASPerformNetworkScanResponse_NetworkRAT ));

   sScannedNetworkRATInfo * pRAT = (sScannedNetworkRATInfo *)pRATInstances;
   for (BYTE r = 0; r < ratCount; r++)
   {
      pRAT->mMCC = pRatInfo->mMobileCountryCode;
      pRAT->mMNC = pRatInfo->mMobileNetworkCode;
      pRAT->mRAT = pRatInfo->mRadioAccessTechnology;

      pRAT++;
      pRatInfo++;
   }

   *pRATSize = (BYTE)ratCount;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackInitiateNetworkRegistration

DESCRIPTION:
   This function initiates a network registration

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   regType     [ I ] - Registration type
   mcc         [ I ] - Mobile country code (ignored for auto registration)
   mnc         [ I ] - Mobile network code (ignored for auto registration)
   rat         [ I ] - Radio access type (ignored for auto registration)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackInitiateNetworkRegistration(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      regType,
   WORD                       mcc,
   WORD                       mnc,
   ULONG                      rat )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Set the action

   // Check size
   WORD tlvx01Sz = sizeof( sNASInitiateNetworkRegisterRequest_Action );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sNASInitiateNetworkRegisterRequest_Action * pTLVx01;
   pTLVx01 = (sNASInitiateNetworkRegisterRequest_Action*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mRegisterAction = (eQMINASRegisterActions)regType;

   offset += tlvx01Sz;

   // Set the info

   // Check size
   WORD tlvx10Sz = sizeof( sNASInitiateNetworkRegisterRequest_ManualInfo );
   if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   pHeader = (sQMIRawContentHeader*)(pOut + offset);
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   offset = sizeof( sQMIRawContentHeader );

   sNASInitiateNetworkRegisterRequest_ManualInfo * pTLVx10;
   pTLVx10 = (sNASInitiateNetworkRegisterRequest_ManualInfo*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mMobileCountryCode = mcc;
   pTLVx10->mMobileNetworkCode = mnc;
   pTLVx10->mRadioAccessTechnology = (eQMINASRadioAccessTechnologies)rat;

   offset += tlvx10Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackInitiateDomainAttach

DESCRIPTION:
   This function initiates a domain attach (or detach)

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   action      [ I ] - PS attach action (attach or detach)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackInitiateDomainAttach(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      action )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx10Sz = sizeof( sNASInitiateAttachRequest_Action );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sNASInitiateAttachRequest_Action * pTLVx10;
   pTLVx10 = (sNASInitiateAttachRequest_Action*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mPSAttachAction = (eQMINASPSAttachActions)action;

   offset += tlvx10Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetServingNetwork

DESCRIPTION:
   Gets information regarding the system that currently provides service
   to the device

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pRegistrationState   [ O ] - Registration state
   pCSDomain            [ O ] - Circuit switch domain status
   pPSDomain            [ O ] - Packet switch domain status
   pRAN                 [ O ] - Radio access network
   pRadioIfacesSize     [I/O] - Upon input the maximum number of elements
                                that the radio interfaces can contain.  Upon
                                successful output the actual number of elements
                                in the radio interface array
   pRadioIfaces         [ O ] - The radio interface array
   pRoaming             [ O ] - Roaming indicator (0xFFFFFFFF - Unknown)
   pMCC                 [ O ] - Mobile country code (0xFFFF - Unknown)
   pMNC                 [ O ] - Mobile network code (0xFFFF - Unknown)
   nameSize             [ I ] - The maximum number of characters (including
                                NULL terminator) that the network name array
                                can contain
   pName                [ O ] - The network name or description represented
                                as a NULL terminated string (empty string
                                returned when unknown)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetServingNetwork(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pRegistrationState,
   ULONG *           pCSDomain,
   ULONG *           pPSDomain,
   ULONG *           pRAN,
   BYTE *            pRadioIfacesSize,
   BYTE *            pRadioIfaces,
   ULONG *           pRoaming,
   WORD *            pMCC,
   WORD *            pMNC,
   BYTE              nameSize,
   CHAR *            pName )
{
   // Validate arguments
   if (pIn == 0
   ||  pRegistrationState == 0
   ||  pCSDomain == 0
   ||  pPSDomain == 0
   ||  pRAN == 0
   ||  pRadioIfacesSize == 0
   ||  *pRadioIfacesSize == 0
   ||  pRadioIfaces == 0
   ||  pRoaming == 0
   ||  pMCC == 0
   ||  pMNC == 0
   ||  nameSize == 0
   ||  pName == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxRadioIfaces = *pRadioIfacesSize;

   // Assume failure
   *pRadioIfacesSize = 0;
   *pRoaming = 0xffffffff;
   *pMCC = 0xffff;
   *pMNC = 0xffff;
   *pName = 0;

   // Parse the serving system (mandatory)

   // Find the TLV
   const sNASGetServingSystemResponse_ServingSystem * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sNASGetServingSystemResponse_ServingSystem ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Populate the variables
   *pRegistrationState = pTLVx01->mRegistrationState;
   *pCSDomain = pTLVx01->mCSAttachState;
   *pPSDomain = pTLVx01->mPSAttachState;
   *pRAN = pTLVx01->mRegisteredNetwork;

   BYTE activeRadioIfaces = pTLVx01->mNumberOfRadioInterfacesInUse;
   if (activeRadioIfaces > maxRadioIfaces)
   {
      activeRadioIfaces = maxRadioIfaces;
   }

   const eQMINASRadioInterfaces * pRadioInfo;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sNASGetServingSystemResponse_ServingSystem )
                  + sizeof( eQMINASRadioInterfaces ) * activeRadioIfaces)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pRadioInfo = (const eQMINASRadioInterfaces *)
                ((const BYTE *)pTLVx01 
                + sizeof( sNASGetServingSystemResponse_ServingSystem ));

   ULONG * pOutRadioIfaces = (ULONG *)pRadioIfaces;
   for (ULONG r = 0; r < activeRadioIfaces; r++)
   {
      *pOutRadioIfaces = *pRadioInfo;
      pOutRadioIfaces++;
      pRadioInfo++;
   }

   *pRadioIfacesSize = activeRadioIfaces;

   // Find the roaming indicator (optional)
   const sNASGetServingSystemResponse_RoamingIndicator * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sNASGetServingSystemResponse_RoamingIndicator ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Get the values
      *pRoaming = (eQMINASRoamingIndicators)pTLVx10->mRoamingIndicator;
   }

   // Find the PLMN (optional)
   const sNASGetServingSystemResponse_CurrentPLMN * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx12 < sizeof( sNASGetServingSystemResponse_CurrentPLMN ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pMCC = pTLVx12->mMobileCountryCode;
      *pMNC = pTLVx12->mMobileNetworkCode;

      ULONG descLen = pTLVx12->mDescriptionLength;
      const CHAR * pDesc;

      // Verify there is room for the array in the TLV
      if (outLenx12 < sizeof( sNASGetServingSystemResponse_CurrentPLMN )
                     + sizeof( CHAR ) * descLen)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Space to perform the copy?
      if (nameSize < descLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      // Align to the first array element
      pDesc = (const CHAR *)((const BYTE *)pTLVx12 
            + sizeof( sNASGetServingSystemResponse_CurrentPLMN ));

      memcpy( pName, pDesc, descLen );
      pName[descLen] = 0;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetServingNetworkCapabilities

DESCRIPTION:
   Gets information regarding the data capabilities of the system that
   currently provides service to the device

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pDataCapsSize  [I/O] - Upon input the maximum number of elements that the
                          data capabilities array can contain.  Upon success
                          the actual number of elements in the data
                          capabilities array
   pDataCaps      [ O ] - The data capabilities array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetServingNetworkCapabilities(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pDataCapsSize,
   BYTE *            pDataCaps )
{
   // Validate arguments
   if (pIn == 0
   ||  pDataCapsSize == 0
   ||  *pDataCapsSize == 0
   ||  pDataCaps == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxDataCaps = *pDataCapsSize;

   // Assume failure
   *pDataCapsSize = 0;

   // Find the TLV
   const sNASGetServingSystemResponse_DataServices * pTLVx11;
   ULONG outLenx11;
   ULONG rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx11 < sizeof( sNASGetServingSystemResponse_DataServices ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   BYTE activeDataCaps = pTLVx11->mNumberOfDataCapabilities;
   if (activeDataCaps > maxDataCaps)
   {
      activeDataCaps = maxDataCaps;
   }

   const eQMINASDataServiceCapabilities2 * pInDataCaps;

   // Verify there is room for the array in the TLV
   if (outLenx11 < sizeof( sNASGetServingSystemResponse_DataServices )
                 + sizeof( eQMINASDataServiceCapabilities2 ) * activeDataCaps)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pInDataCaps = (const eQMINASDataServiceCapabilities2 *)
                 ((const BYTE *)pTLVx11 
                 + sizeof( sNASGetServingSystemResponse_DataServices ));

   ULONG * pOutDataCaps = (ULONG *)pDataCaps;
   for (ULONG d = 0; d < activeDataCaps; d++)
   {
      *pOutDataCaps = *pInDataCaps;
      pOutDataCaps++;
      pInDataCaps++;
   }

   *pDataCapsSize = activeDataCaps;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetHomeNetwork

DESCRIPTION:
   This function retrieves information about the home network of the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pMCC        [ O ] - Mobile country code
   pMNC        [ O ] - Mobile network code
   nameSize    [ I ] - The maximum number of characters (including NULL
                       terminator) that the network name array can contain
   pName       [ O ] - The network name or description represented as a NULL
                       terminated string (empty string returned when unknown)
   pSID        [ O ] - Home network system ID (0xFFFF - Unknown)
   pNID        [ O ] - Home network ID (0xFFFF - Unknown)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetHomeNetwork(
   ULONG             inLen,
   const BYTE *      pIn,
   WORD *            pMCC,
   WORD *            pMNC,
   BYTE              nameSize,
   CHAR *            pName,
   WORD *            pSID,
   WORD *            pNID )
{
   // Validate arguments
   if (pIn == 0
   ||  pMCC == 0
   ||  pMNC == 0
   ||  nameSize == 0
   ||  pName == 0
   ||  pSID == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pName = 0;
   *pSID = 0xffff;
   *pNID = 0xffff;

   // Find the name (mandatory)
   const sNASGetHomeNetworkResponse_HomeNetwork * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sNASGetHomeNetworkResponse_HomeNetwork ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Populate the variables
   *pMCC = pTLVx01->mMobileCountryCode;
   *pMNC = pTLVx01->mMobileNetworkCode;

   ULONG descLen = pTLVx01->mDescriptionLength;
   const CHAR * pDesc;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sNASGetHomeNetworkResponse_HomeNetwork )
                  + sizeof( CHAR ) * descLen)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Space to perform the copy?
   if (nameSize < descLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   // Align to the first array element
   pDesc = (const CHAR *)((const BYTE *)pTLVx01 
         + sizeof( sNASGetHomeNetworkResponse_HomeNetwork ));

   memcpy( pName, pDesc, descLen );
   pName[descLen] = 0;


   // Find the SID/NID (optional)
   const sNASGetHomeNetworkResponse_HomeIDs * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sNASGetHomeNetworkResponse_HomeIDs ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pSID = pTLVx10->mSystemID;
      *pNID = pTLVx10->mNetworkID;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetNetworkPreference

DESCRIPTION:
   This function sets the network registration preference

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   technologyPref [ I ] - Technology preference bitmap
   duration       [ I ] - Duration of active preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetNetworkPreference(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      technologyPref,
   ULONG                      duration )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sNASSetTechnologyPreferenceRequest_Preference );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sNASSetTechnologyPreferenceRequest_Preference * pTLVx01;
   pTLVx01 = (sNASSetTechnologyPreferenceRequest_Preference*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Copy technology preference WORD as-is
   memcpy( &pTLVx01->mValOfTechnology, &technologyPref, 2 );

   pTLVx01->mDuration = (eQMINASTechPrefDurations)duration;

   offset += tlvx01Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetNetworkPreference

DESCRIPTION:
   This function returns the network registration preference

PARAMETERS:
   inLen                      [ I ] - Length of input buffer
   pIn                        [ I ] - Input buffer
   pTechnologyPref            [ O ] - Technology preference bitmap
   pDuration                  [ O ] - Duration of active preference
   pPersistentTechnologyPref  [ O ] - Persistent technology preference bitmap

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetNetworkPreference(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pTechnologyPref,
   ULONG *           pDuration,
   ULONG *           pPersistentTechnologyPref )
{
   // Validate arguments
   if (pIn == 0
   ||  pTechnologyPref == 0
   ||  pDuration == 0
   ||  pPersistentTechnologyPref == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the preference (mandatory)
   const sNASGetTechnologyPreferenceResponse_ActivePreference * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sNASGetTechnologyPreferenceResponse_ActivePreference ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Copy technology preference WORD as-is
   *pTechnologyPref = 0;
   memcpy( pTechnologyPref, &pTLVx01->mValOfTechnology, 2 );

   *pDuration = pTLVx01->mDuration;


   // Until we know any better the persistent setting is the current setting
   *pPersistentTechnologyPref = *pTechnologyPref;

   // Find the persistant technology preference (optional)
   const sNASGetTechnologyPreferenceResponse_PersistentPreference * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sNASGetTechnologyPreferenceResponse_PersistentPreference ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Copy technology preference WORD as-is
      *pTechnologyPref = 0;
      memcpy( pPersistentTechnologyPref, &pTLVx10->mValOfTechnology, 2 );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetCDMANetworkParameters

DESCRIPTION:
   This function sets the desired CDMA network parameters

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   pSPC           [ I ] - Six digit service programming code
   pForceRev0     [ I ] - (Optional) Force CDMA 1x-EV-DO Rev. 0 mode?
   pCustomSCP     [ I ] - (Optional) Use a custom config for CDMA 1x-EV-DO SCP?
   pProtocol      [ I ] - (Optional) Protocol mask for custom SCP config
   pBroadcast     [ I ] - (Optional) Broadcast mask for custom SCP config
   pApplication   [ I ] - (Optional) Application mask for custom SCP config
   pRoaming       [ I ] - (Optional) Roaming preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetCDMANetworkParameters(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE *            pForceRev0,
   BYTE *            pCustomSCP,
   ULONG *           pProtocol,
   ULONG *           pBroadcast,
   ULONG *           pApplication,
   ULONG *           pRoaming )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // If you specify one of the custom SCP config fields then you must
   // specify them all
   ULONG scpCount = 0;
   if (pCustomSCP != 0)
   {
      scpCount++;
   }

   if (pProtocol != 0)
   {
      scpCount++;
   }

   if (pBroadcast != 0)
   {
      scpCount++;
   }

   if (pApplication != 0)
   {
      scpCount++;
   }

   if (scpCount != 0 && scpCount != 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Rev. 0 and SCP custom config are mutually exclusive
   if (pForceRev0 != 0 && scpCount == 4)
   {
      if (*pForceRev0 != 0 && *pCustomSCP != 0)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }

   sQMIRawContentHeader * pHeader;
   ULONG offset = 0;

   // Need to start with SPC?
   if (pForceRev0 != 0 || scpCount == 4)
   {
      // Validate arguments
      if (pSPC == 0 || pSPC[0] == 0)
      {
         return eGOBI_ERR_INVALID_ARG;
      }

      std::string spc( pSPC );
      if (spc.size() > 6)
      {
         return eGOBI_ERR_INVALID_ARG;
      }

      if (spc.find_first_not_of( "0123456789" ) != std::string::npos )
      {
         return eGOBI_ERR_INVALID_ARG;
      }

      // Check size
      WORD tlvx10Sz = sizeof( sNASSetNetworkParametersRequest_SPC );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sNASSetNetworkParametersRequest_SPC * pTLVx10;
      pTLVx10 = (sNASSetNetworkParametersRequest_SPC*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the values
      memcpy( &pTLVx10->mSPC[0], spc.c_str(), spc.size() );

      offset += tlvx10Sz;
   }

   // Force Rev. 0?
   if (pForceRev0 != 0)
   {
      // Check size
      WORD tlvx14Sz = sizeof( sNASSetNetworkParametersRequest_CDMA1xEVDORevision );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx14Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x14;
      pHeader->mLength = tlvx14Sz;

      offset += sizeof( sQMIRawContentHeader );

      sNASSetNetworkParametersRequest_CDMA1xEVDORevision * pTLVx14;
      pTLVx14 = (sNASSetNetworkParametersRequest_CDMA1xEVDORevision*)(pOut + offset);
      memset( pTLVx14, 0, tlvx14Sz );

      // Set the value
      pTLVx14->mForceCDMA1xEVDORev0 = (*pForceRev0 == 0 ? 0 : 1);

      offset += tlvx14Sz;
   }

   if (scpCount == 4)
   {
      // Check size
      WORD tlvx15Sz = sizeof( sNASSetNetworkParametersRequest_CDMA1xEVDOSCPCustom );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx15Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x15;
      pHeader->mLength = tlvx15Sz;

      offset += sizeof( sQMIRawContentHeader );

      sNASSetNetworkParametersRequest_CDMA1xEVDOSCPCustom * pTLVx15;
      pTLVx15 = (sNASSetNetworkParametersRequest_CDMA1xEVDOSCPCustom*)(pOut + offset);
      memset( pTLVx15, 0, tlvx15Sz );

      // Set the values
      pTLVx15->mCDMA1xEVDOSCPCustomConfig = (*pCustomSCP == 0 ? 0 : 1);

      // The pProtocol bitmask
      pTLVx15->mSubtype2PhysicalLayer = (*pProtocol & 0x00000001 ? 1 : 0);
      pTLVx15->mEnhancedCCMAC = (*pProtocol & 0x00000002 ? 1 : 0);
      pTLVx15->mEnhancedACMAC = (*pProtocol & 0x00000004 ? 1 : 0);
      pTLVx15->mEnhancedFTCMAC = (*pProtocol & 0x00000008 ? 1 : 0);
      pTLVx15->mSubtype3RTCMAC = (*pProtocol & 0x00000010 ? 1 : 0);
      pTLVx15->mSubtype1RTCMAC = (*pProtocol & 0x00000020 ? 1 : 0);
      pTLVx15->mEnhancedIdle = (*pProtocol & 0x00000040 ? 1 : 0);
      pTLVx15->mGenericMultimodeCapableDiscPort
         = (*pProtocol & 0x00000080 ? 1 : 0);

      pTLVx15->mGenericBroadcast = (*pBroadcast & 0x00000001 ? 1 : 0);

      pTLVx15->mSNMultiflowPacketApplication
         = (*pApplication & 0x00000001 ? 1 : 0);

      pTLVx15->mSNEnhancedMultiflowPacketApplication
         = (*pApplication & 0x00000002 ? 1 : 0);

      offset += tlvx15Sz;
   }

   if (pRoaming != 0)
   {
      // Check size
      WORD tlvx16Sz = sizeof( sNASSetNetworkParametersRequest_Roaming );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx16Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x16;
      pHeader->mLength = tlvx16Sz;

      offset += sizeof( sQMIRawContentHeader );

      sNASSetNetworkParametersRequest_Roaming * pTLVx16;
      pTLVx16 = (sNASSetNetworkParametersRequest_Roaming*)(pOut + offset);
      memset( pTLVx16, 0, tlvx16Sz );

      // Set the values
      pTLVx16->mRoamPreference = (eQMINASRoamingPreferences)*pRoaming;

      offset += tlvx16Sz;
   }

   // At least one of the optional parameters must have been set
   if (offset == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetCDMANetworkParameters

DESCRIPTION:
   This function gets the current CDMA network parameters

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pSCI           [ O ] - Slot cycle index
   pSCM           [ O ] - Station class mark
   pRegHomeSID    [ O ] - Register on home SID?
   pRegForeignSID [ O ] - Register on foreign SID?
   pRegForeignNID [ O ] - Register on foreign NID?
   pForceRev0     [ O ] - Force CDMA 1x-EV-DO Rev. 0 mode?
   pCustomSCP     [ O ] - Use a custom config for CDMA 1x-EV-DO SCP?
   pProtocol      [ O ] - Protocol mask for custom SCP config
   pBroadcast     [ O ] - Broadcast mask for custom SCP config
   pApplication   [ O ] - Application mask for custom SCP config
   pRoaming       [ O ] - Roaming preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetCDMANetworkParameters(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pSCI,
   BYTE *            pSCM,
   BYTE *            pRegHomeSID,
   BYTE *            pRegForeignSID,
   BYTE *            pRegForeignNID,
   BYTE *            pForceRev0,
   BYTE *            pCustomSCP,
   ULONG *           pProtocol,
   ULONG *           pBroadcast,
   ULONG *           pApplication,
   ULONG *           pRoaming )
{
   // Validate arguments
   if (pIn == 0
   ||  pSCI == 0
   ||  pSCM == 0
   ||  pRegHomeSID == 0
   ||  pRegForeignSID == 0
   ||  pRegForeignNID == 0
   ||  pForceRev0 == 0
   ||  pCustomSCP == 0
   ||  pProtocol == 0
   ||  pBroadcast == 0
   ||  pApplication == 0
   ||  pRoaming == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSCI = 0xff;
   *pSCM = 0xff;
   *pRegHomeSID = 0xff;
   *pRegForeignSID = 0xff;
   *pRegForeignNID = 0xff;
   *pForceRev0 = 0xff;
   *pCustomSCP = 0xff;
   *pProtocol = 0xffffffff;
   *pBroadcast = 0xffffffff;
   *pApplication = 0xffffffff;
   *pRoaming = 0xff;

   // Find the SCI
   const sNASGetNetworkParametersResponse_SCI * pTLVx11;
   ULONG outLenx11;
   ULONG rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sNASGetNetworkParametersResponse_SCI ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pSCI = pTLVx11->mSlotCycleIndex;
   }

   // Find the SCM
   const sNASGetNetworkParametersResponse_SCM * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx12 < sizeof( sNASGetNetworkParametersResponse_SCM ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pSCM = pTLVx12->mStationClassMark;
   }

   // Find the Registration
   const sNASGetNetworkParametersResponse_Registration * pTLVx13;
   ULONG outLenx13;
   rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx13 < sizeof( sNASGetNetworkParametersResponse_Registration ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRegHomeSID = pTLVx13->mRegisterOnHomeSystem;
      *pRegForeignSID = pTLVx13->mRegisterOnForeignSystem;
      *pRegForeignNID = pTLVx13->mRegisterOnForeignNetwork;
   }

   // Rev. 0?
   const sNASGetNetworkParametersResponse_CDMA1xEVDORevision * pTLVx14;
   ULONG outLenx14;
   rc = GetTLV( inLen, pIn, 0x14, &outLenx14, (const BYTE **)&pTLVx14 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx14 < sizeof( sNASGetNetworkParametersResponse_CDMA1xEVDORevision ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRegHomeSID = pTLVx14->mForceCDMA1xEVDORev0;
   }

   // We're lazy, so we'll just typecast all the bitmask members from
   // sNASGetNetworkParametersResponse_CDMA1xEVDOSCPCustom into their
   // respective container parameters
   const sEVDOCustomSCPConfig * pTLVx15;
   ULONG outLenx15;
   rc = GetTLV( inLen, pIn, 0x15, &outLenx15, (const BYTE **)&pTLVx15 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx15 < sizeof( sEVDOCustomSCPConfig ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pCustomSCP   = pTLVx15->mbActive;
      *pProtocol    = pTLVx15->mProtocolMask;
      *pBroadcast   = pTLVx15->mBroadcastMask;
      *pApplication = pTLVx15->mApplicationMask;
   }

   // Roaming?
   const sNASGetNetworkParametersResponse_Roaming * pTLVx16;
   ULONG outLenx16;
   rc = GetTLV( inLen, pIn, 0x16, &outLenx16, (const BYTE **)&pTLVx16 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx16 < sizeof( sNASGetNetworkParametersResponse_Roaming ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRoaming = (eQMINASRoamingPreferences)pTLVx16->mRoamPreference;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetACCOLC

DESCRIPTION:
   This function returns the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pACCOLC        [ O ] - The ACCOLC

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetACCOLC(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pACCOLC )
{
   // Validate arguments
   if (pIn == 0 || pACCOLC == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the ACCOLC (mandatory)
   const sNASGetACCOLCResponse_ACCOLC * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sNASGetACCOLCResponse_ACCOLC ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pACCOLC = pTLVx01->mACCOLC;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetACCOLC

DESCRIPTION:
   This function sets the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   pSPC        [ I ] - NULL terminated string representing the six digit
                       service programming code
   accolc      [ I ] - The ACCOLC

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetACCOLC(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE              accolc )
{
   // Validate arguments
   if (pOut == 0 || pSPC == 0 || pSPC[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string spc( pSPC );
   if (spc.size() > 6)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (spc.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sNASSetACCOLCRequest_ACCOLC );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sNASSetACCOLCRequest_ACCOLC * pTLVx01;
   pTLVx01 = (sNASSetACCOLCRequest_ACCOLC*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );
   pTLVx01->mACCOLC = accolc;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPLMNMode

DESCRIPTION:
   This function returns the PLMN mode from the CSP

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pMode       [ O ] - PLMN mode

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPLMNMode(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMode )
{
   // Validate arguments
   if (pIn == 0 || pMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the mode (mandatory)
   const sNASGetCSPPLMNModeResponse_Mode * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx10 < sizeof( sNASGetCSPPLMNModeResponse_Mode ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pMode = pTLVx10->mRestrictManualPLMNSelection;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetPLMNName

DESCRIPTION:
   This function returns PLMN name information for the given MCC/MNC

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   mcc         [ I ] - Mobile country code
   mnc         [ I ] - Mobile network code

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetPLMNName(
   ULONG *           pOutLen,
   BYTE *            pOut,
   USHORT            mcc,
   USHORT            mnc )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sNASGetPLMNNameRequest_PLMN );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sNASGetPLMNNameRequest_PLMN * pTLVx01;
   pTLVx01 = (sNASGetPLMNNameRequest_PLMN*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mMobileCountryCode = mcc;
   pTLVx01->mMobileNetworkCode = mnc;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPLMNName

DESCRIPTION:
   This function returns PLMN name information for the given MCC/MNC

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pNamesSize  [I/O] - Upon input the size in BYTEs of the name structure
                       array.  Upon success the actual number of BYTEs
                       copied to the name structure array
   pNames      [ O ] - The name structure array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPLMNName(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pNamesSize,
   BYTE *            pNames )
{
   // Validate arguments
   if (pIn == 0 || *pNamesSize == 0 || pNames == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   const BYTE * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // The output format just happens to be the same as
   // sNASGetPLMNNameResponse_Name.  Copy the full TLV to pNames
   if (outLenx10 > *pNamesSize)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pNames, pTLVx10, outLenx10 );
   *pNamesSize = outLenx10;

   return eGOBI_ERR_NONE;
}
