/*===========================================================================
FILE:
   GobiCMDLL.cpp

DESCRIPTION:
   Simple class to load and interface to the Gobi CM DLL
   
PUBLIC CLASSES AND METHODS:
   cGobiCMDLL
      This class loads the Gobi CM DLL and then interfaces to it

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
#include "GobiCMDLL.h"
#include "GobiConnectionMgmtAPIStructs.h"
#include "Gobi3000Translation.h"
#include <string.h>

/*=========================================================================*/
// cGobiCMDLL Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetString (Internal Method)

DESCRIPTION:
   Call a Gobi CM API function that returns a string

PARAMETERS:
   mpFnString  [ I ] - Gobi CM API function pointer
   tlvID       [ I ] - ID of response TLV that contains the string
   strSz       [ I ] - Max string size (including NULL terminator)
   pStr        [ O ] - Buffer to hold the string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetString(
   tFNGobiInputOutput         mpFnString,
   BYTE                       tlvID,
   BYTE                       strSz,
   CHAR *                     pStr )
{
   // Assume failure
   if (strSz > 0 && pStr != 0)
   {
      pStr[0] = 0;
   }

   // Query for string?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mpFnString == 0 || mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = mpFnString( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &rsp[0], lo );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( tlvID );
   if (pIter == tlvs.end())
   {
      return eGOBI_ERR_GENERAL;
   }

   const sQMIRawContentHeader * pTmp = pIter->second;
   ULONG strLen = (ULONG)pTmp->mLength;
   pTmp++;

   if (strLen != 0 && strSz > 0 && pStr != 0)
   {
      ULONG needLen = strLen;
      if (needLen + 1 > strSz)
      {
         needLen = strSz - 1;
      }

      memcpy( pStr, pTmp, needLen );
      pStr[needLen] = 0;
   }

   return status;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Calls GobiConnect

PARAMETERS:
   pInterface  [ I ] - Interace to connect to

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::Connect( LPCSTR pInterface )
{
   // Connect to WDS, DMS, and NAS services
   ULONG svc[3] = { 1, 2, 3 };
   ULONG svcCount = 3;
   GOBIHANDLE handle = 0;
   ULONG status = GobiConnect( pInterface, &svcCount, &svc[0], &handle );
   if (status == 0)
   {
      if (svcCount == 3)
      {
         mhGobi = handle;
      }
      else
      {
         // We require WDS, DMS, and NAS services
         Disconnect();
         status = eGOBI_ERR_GENERAL;
      }
   }

   return status;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Calls GobiDisconnect

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::Disconnect()
{
   if (mhGobi == 0)
   {
      return eGOBI_ERR_GENERAL;
   }

   return GobiDisconnect( mhGobi );
}

/*===========================================================================
ETHOD:
   StartDataSession (Public Method)

DESCRIPTION:
   Calls WDSStartNetworkInterface

PARAMETERS:
   pAPN           [ I ] - Access point name
   pUser          [ I ] - Username
   pPwd           [ I ] - Password
   pSessionID     [ O ] - Session ID
   pFailureCode   [ O ] - Failure code (if present)

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::StartDataSession(
   LPCSTR                     pAPN,
   LPCSTR                     pUser,
   LPCSTR                     pPwd,
   ULONG *                    pSessionID,
   ULONG *                    pFailureCode )
{
   // Assume failure
   if (pSessionID != 0)
   {
      *pSessionID = 0xFFFFFFFF;
   }

   // Start a data session?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   UINT8 req[1024] = { 0 };
   UINT8 * pData = (UINT8 *)&req[0];

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)pData;
   pTLV->mTypeID = 0x16;
   pTLV->mLength 
      = (UINT16)sizeof( sWDSStartNetworkInterfaceRequest_Authentication );     
   pData += sizeof( sQMIRawContentHeader );

   sWDSStartNetworkInterfaceRequest_Authentication * pAuth =
      (sWDSStartNetworkInterfaceRequest_Authentication *)pData;
   pAuth->mEnablePAP = 1;
   pAuth->mEnableCHAP = 1;
   pData += sizeof( sWDSStartNetworkInterfaceRequest_Authentication );

   if (pAPN != 0 && pAPN[0] != 0)
   {
      size_t len = strnlen( pAPN, 256 );

      pTLV = (sQMIRawContentHeader *)pData;
      pTLV->mTypeID = 0x14;
      pTLV->mLength = (UINT16)len;
      pData += sizeof( sQMIRawContentHeader );

      memcpy( pData, pAPN, len );
      pData += len;
   }

   if (pUser != 0 && pUser[0] != 0)
   {
      size_t len = strnlen( pUser, 256 );

      pTLV = (sQMIRawContentHeader *)pData;
      pTLV->mTypeID = 0x17;
      pTLV->mLength = (UINT16)len;
      pData += sizeof( sQMIRawContentHeader );

      memcpy( pData, pUser, len );
      pData += len;
   }

   if (pPwd != 0 && pPwd[0] != 0)
   {
      size_t len = strnlen( pPwd, 256 );

      pTLV = (sQMIRawContentHeader *)pData;
      pTLV->mTypeID = 0x18;
      pTLV->mLength = (UINT16)len;
      pData += sizeof( sQMIRawContentHeader );

      memcpy( pData, pPwd, len );
      pData += len;
   }
   
   ULONG li = (ULONG)pData - (ULONG)&req[0];
   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = WDSStartNetworkInterface( mhGobi, 300000, li, &req[0], &lo, &rsp[0] );

   // On success pSessionID is valid, on failure pFailureCode is valid
   ULONG status2 = ParseStartDataSession( lo, &rsp[0], pSessionID, pFailureCode );

   if (status == eGOBI_ERR_NONE)
   {
      return status2;
   }

   return status;
}

/*===========================================================================
METHOD:
   CancelDataSession (Public Method)

DESCRIPTION:
   Calls GobiCancel/WDSAbort

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::CancelDataSession()
{
   // Cancel outstanding API request?
   if (mhGobi == 0)
   {
      return eGOBI_ERR_GENERAL;
   }

   // Cancel the request with the API
   ULONG svcID = 1;
   ULONG txID = 0xFFFFFFFF;
   ULONG status = GobiCancel( mhGobi, svcID, &txID );
   if (status != 0 || txID == 0xFFFFFFFF)
   {
      return eGOBI_ERR_GENERAL;
   }

   UINT8 req[256] = { 0 };
   UINT8 * pData = (UINT8 *)&req[0];

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)pData;
   pTLV->mTypeID = 0x01;
   pTLV->mLength = (UINT16)sizeof( sWDSAbortRequest_TransactionID );     
   pData += sizeof( sQMIRawContentHeader );

   sWDSAbortRequest_TransactionID * pID =
      (sWDSAbortRequest_TransactionID *)pData;
   pID->mTransactionID = (UINT16)txID;
   pData += sizeof( sWDSAbortRequest_TransactionID );

   // Cancel the request with the device
   ULONG li = (ULONG)pData - (ULONG)&req[0];
   status = WDSAbort( mhGobi, 2000, li, &req[0], 0, 0 );
   return status;
}

/*===========================================================================
METHOD:
   StopDataSession (Public Method)

DESCRIPTION:
   Calls WDSStopNetworkInterface

PARAMETERS:
   sessionID   [ I ] - Session ID

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::StopDataSession( ULONG sessionID )
{
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   UINT8 req[256] = { 0 };
   ULONG li = 256;

   status = PackStopDataSession( &li, &req[0], sessionID );
   if (status != 0)
   {
      return status;
   }

   // Stop data session
   status = WDSStopNetworkInterface( mhGobi, 2000, li, &req[0], 0, 0 );
   return status;
}

/*===========================================================================
METHOD:
   GetSessionState (Public Method)

DESCRIPTION:
   Calls WDSGetPacketServiceStatus

PARAMETERS:
   pSessionState  [ O ] - Current session state

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetSessionState( ULONG * pSessionState )
{
   // Assume failure
   if (pSessionState != 0)
   {
      *pSessionState = 0xFFFFFFFF;
   }

   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = WDSGetPacketServiceStatus( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetSessionState( lo, &rsp[0], pSessionState );
   return status;
}

/*===========================================================================
METHOD:
   GetSessionDuration (Public Method)

DESCRIPTION:
   Calls WDSGetDataSessionDuration

PARAMETERS:
   pSessionDuration  [ O ] - Session duration

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetSessionDuration( ULONGLONG * pSessionDuration )
{
   // Assume failure
   if (pSessionDuration != 0)
   {
      *pSessionDuration = 0xFFFFFFFF;
   }

   // Query for session duration
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = WDSGetDataSessionDuration( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetSessionDuration( lo, &rsp[0], pSessionDuration );
   return status;
}

/*===========================================================================
METHOD:
   GetDataBearerTechnology (Public Method)

DESCRIPTION:
   Calls WDSGetDataBearerTechnology

PARAMETERS:
   pDataBearerTech   [ O ] - Data bearer technology

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetDataBearerTechnology( ULONG * pDataBearerTech )
{
   // Assume failure
   if (pDataBearerTech != 0)
   {
      *pDataBearerTech = 0xFFFFFFFF;
   }

   // Query for data bearer duration?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = WDSGetDataBearerTechnology( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetDataBearerTechnology( lo, &rsp[0], pDataBearerTech );
   return status;
}

/*===========================================================================
METHOD:
   GetConnectionRate (Public Method)

DESCRIPTION:
   Calls WDSGetChannelRates

PARAMETERS:
   pCurTX      [ O ] - Current TX rate
   pCurRX      [ O ] - Current RX rate
   pMaxTX      [ O ] - Max TX rate
   pMaxRX      [ O ] - Max RX rate

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetConnectionRate(
   ULONG *                     pCurTX,
   ULONG *                     pCurRX,
   ULONG *                     pMaxTX,
   ULONG *                     pMaxRX )
{
   // Assume failure
   pCurTX != 0 ? *pCurTX = 0xFFFFFFFF : 0;
   pCurRX != 0 ? *pCurRX = 0xFFFFFFFF : 0;
   pMaxTX != 0 ? *pMaxTX = 0xFFFFFFFF : 0;
   pMaxRX != 0 ? *pMaxRX = 0xFFFFFFFF : 0;

   // Query for rates?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = WDSGetChannelRates( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetConnectionRate( lo, 
                                    &rsp[0], 
                                    pCurTX, 
                                    pCurRX, 
                                    pMaxTX, 
                                    pMaxRX );
   return status;
}

/*===========================================================================
METHOD:
   GetFirmwareRevision (Public Method)

DESCRIPTION:
   Calls DMSGetDeviceRevision

PARAMETERS:
   strSz      [ I ] - Maximum number of characters
   pStr       [ O ] - Firmware revision string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetFirmwareRevision( 
   BYTE                       strSz,
   CHAR *                     pStr )
{
   return GetString( DMSGetDeviceRevision, 0x01, strSz, pStr );
}

/*===========================================================================
METHOD:
   GetManufacturer (Public Method)

DESCRIPTION:
   Calls DMSGetDeviceManfacturer

PARAMETERS:
   strSz       [ I ] - Maximum string size
   pStr        [ O ] - Manufacturer string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetManufacturer(
   BYTE                       strSz,
   CHAR *                     pStr )
{
   return GetString( DMSGetDeviceManfacturer, 0x01, strSz, pStr );
}

/*===========================================================================
METHOD:
   GetModelID (Public Method)

DESCRIPTION:
   Calls GetModelID

PARAMETERS:
   strSz       [ I ] - Max string size
   pStr        [ O ] - Model ID string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetModelID(
   BYTE                       strSz,
   CHAR *                     pStr )
{
   return GetString( DMSGetDeviceModel, 0x01, strSz, pStr );
}

/*===========================================================================
METHOD:
   GetHardwareRevision (Public Method)

DESCRIPTION:
   Calls DMSGetHardwareRevision

PARAMETERS:
   strSz       [ I ] - Max size of string
   pStr        [ O ] - Hardware revision string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetHardwareRevision(
   BYTE                       strSz,
   CHAR *                     pStr )
{
   return GetString( DMSGetHardwareRevision, 0x01, strSz, pStr );
}

/*===========================================================================
METHOD:
   GetVoiceNumber (Public Method)

DESCRIPTION:
   Calls GetVoiceNumber

PARAMETERS:
   voiceSz     [ I ] - Max characters in voice string
   pVoiceStr   [ O ] - Voice number string
   minSz       [ I ] - Max characters in MIN string
   pMINStr     [ O ] - MIN string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetVoiceNumber(
   BYTE                       voiceSz,
   CHAR *                     pVoiceStr,
   BYTE                       minSz,
   CHAR *                     pMINStr )
{
   // Assume failure
   if (voiceSz > 0 && pVoiceStr != 0)
   {
      pVoiceStr[0] = 0;
   }

   if (minSz > 0 && pMINStr != 0)
   {
      pMINStr[0] = 0;
   }

   // Query for voice numbers?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = DMSGetDeviceVoiceNumber( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetVoiceNumber( lo, 
                                 &rsp[0], 
                                 voiceSz, 
                                 pVoiceStr, 
                                 minSz, 
                                 pMINStr );
   return status;
}

/*===========================================================================
METHOD:
   GetSerialNumbers (Public Method)

DESCRIPTION:
   Calls DMSGetDeviceSerialNumbers

PARAMETERS:
   esnSz       [ I ] - ESN size
   pESNStr     [ O ] - ESN string
   imeiSz      [ I ] - IMEI size
   pIMEIStr    [ O ] - IMSI string
   meidSz      [ I ] - MEID size
   pMEIDStr    [ O ] - MEID string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetSerialNumbers(
   BYTE                       esnSz,
   CHAR *                     pESNStr,
   BYTE                       imeiSz,
   CHAR *                     pIMEIStr,
   BYTE                       meidSz,
   CHAR *                     pMEIDStr )
{
   // Assume failure
   if (esnSz > 0 && pESNStr != 0)
   {
      pESNStr[0] = 0;
   }

   if (imeiSz > 0 && pIMEIStr != 0)
   {
      pIMEIStr[0] = 0;
   }

   if (meidSz > 0 && pMEIDStr != 0)
   {
      pMEIDStr[0] = 0;
   }

   // Query for serial numbers?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = DMSGetDeviceSerialNumbers( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetSerialNumbers( lo, 
                                   &rsp[0], 
                                   esnSz, 
                                   pESNStr,
                                   imeiSz,
                                   pIMEIStr,
                                   meidSz,
                                   pMEIDStr );
   return status;
}

/*===========================================================================
METHOD:
   GetIMSI (Public Method)

DESCRIPTION:
   Get IMSI

PARAMETERS:
   imsiSz      [ I ] - IMSI size
   pIMSIStr    [ O ] - IMSI string

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetIMSI(
   BYTE                       imsiSz,
   CHAR *                     pIMSIStr )
{
   return GetString( DMSGetDeviceVoiceNumber, 0x11, imsiSz, pIMSIStr );
}

/*===========================================================================
METHOD:
   GetSignalStrengths (Public Method)

DESCRIPTION:
   Calls NASGetSignalStrength

PARAMETERS:
   pSigStrengths     [ O ] - Received signal strength
   pRadioInterfaces  [ O ] - Radio interface technology

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetSignalStrengths( 
   INT8 *                     pSigStrengths,
   ULONG *                    pRadioInterfaces )
{
   // Assume failure
   for (ULONG s = 0; s < MAX_SIGNALS; s++)
   {
      pSigStrengths[s] = 0;
      pRadioInterfaces[s] = 0xFFFFFFFF;
   }

   // Query for signal strengths?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 1024;
   BYTE rsp[1024] = { 0 };
   status = NASGetSignalStrength( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }
   
   status = ParseGetSignalStrength( lo, 
                                    &rsp[0], 
                                    pSigStrengths,
                                    pRadioInterfaces );
   return status;
}

/*===========================================================================
METHOD:
   GetServingNetwork (Public Method)

DESCRIPTION:
   Calls NASGetServingSystem

PARAMETERS:
   pDataCapabilities [ O ] - Data capabilities
   pMCC              [ O ] - Mobile country code
   pMNC              [ O ] - Mobile network code
   nameSize          [ I ] - Network name max size
   pName             [ O ] - Network name
   pSID              [ O ] - System ID
   pNID              [ O ] - Network ID
   pRoam             [ O ] - Roaming indicator
   
RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetServingNetwork(
   ULONG *                    pDataCapabilities,
   WORD *                     pMCC,
   WORD *                     pMNC,
   BYTE                       nameSize,
   CHAR *                     pName,
   WORD *                     pSID,
   WORD *                     pNID,
   ULONG *                    pRoam )
{
   // Assume failure
   for (ULONG d = 0; d < MAX_DATA_CAPABILITIES; d++)
   {
      pDataCapabilities[d] = 0xFFFFFFFF;
   }

   if (nameSize > 0 && pName != 0)
   {
      pName[0] = 0;
   }

   pMCC != 0 ? *pMCC = 0xFFFF : 0;
   pMNC != 0 ? *pMNC = 0xFFFF : 0;
   pRoam != 0 ? *pRoam = 0xFFFFFFFF : 0;
   pSID != 0 ? *pSID = 0xFFFF : 0;
   pNID != 0 ? *pNID = 0xFFFF : 0;

   // Query for serving system?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 8096;
   BYTE rsp[8096] = { 0 };
   status = NASGetServingSystem( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   std::map <UINT8, const sQMIRawContentHeader *> tlvs = GetTLVs( &rsp[0], lo );
   std::map <UINT8, const sQMIRawContentHeader *>::const_iterator pIter = tlvs.find( 0x11 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      ULONG tlvLen = (ULONG)pTmp->mLength;
      ULONG dsLen = (ULONG)sizeof( sNASGetServingSystemResponse_DataServices ); 
      if (tlvLen < dsLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pTmp++;
      const sNASGetServingSystemResponse_DataServices * pDS =
         (const sNASGetServingSystemResponse_DataServices *)pTmp;

      ULONG dcCount = (ULONG)pDS->mNumberOfDataCapabilities;
      ULONG dcSz = (ULONG)sizeof( eQMINASDataServiceCapabilities2 );
      dsLen += dcCount * dcSz;
      if (tlvLen < dsLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pDS++;
      eQMINASDataServiceCapabilities2 * pCap = (eQMINASDataServiceCapabilities2 *)pDS;
      if (dcCount > MAX_DATA_CAPABILITIES)
      {
         dcCount = MAX_DATA_CAPABILITIES;
      }

      for (ULONG i = 0; i < dcCount; i++)
      {
        pDataCapabilities[i] = (ULONG)*pCap++;
      }
   }

   pIter = tlvs.find( 0x12 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      ULONG tlvLen = (ULONG)pTmp->mLength;
      ULONG plmnLen = (ULONG)sizeof( sNASGetServingSystemResponse_CurrentPLMN ); 
      if (tlvLen < plmnLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pTmp++;
      const sNASGetServingSystemResponse_CurrentPLMN * pPLMN =
         (const sNASGetServingSystemResponse_CurrentPLMN *)pTmp;

      ULONG strLen = (ULONG)pPLMN->mDescriptionLength;
      plmnLen += strLen;
      if (tlvLen < plmnLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pMCC != 0 ? *pMCC = (ULONG)pPLMN->mMobileCountryCode : 0;
      pMNC != 0 ? *pMNC = (ULONG)pPLMN->mMobileNetworkCode : 0;
      pPLMN++;

      if (strLen != 0 && nameSize > 0 && pName != 0)
      {
         ULONG needLen = strLen;
         if (needLen + 1 > nameSize)
         {
            needLen = nameSize - 1;
         }

         memcpy( pName, pPLMN, needLen );
         pName[needLen] = 0;
      }
   }

   pIter = tlvs.find( 0x13 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      ULONG tlvLen = (ULONG)pTmp->mLength;
      ULONG sysLen = (ULONG)sizeof( sNASGetServingSystemResponse_SystemID ); 
      if (tlvLen < sysLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pTmp++;
      const sNASGetServingSystemResponse_SystemID * pSys =
         (const sNASGetServingSystemResponse_SystemID *)pTmp;

      pSID != 0 ? *pSID = (ULONG)pSys->mSystemID : 0;
      pNID != 0 ? *pNID = (ULONG)pSys->mNetworkID : 0;
   }

   pIter = tlvs.find( 0x16 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pTmp = pIter->second;
      ULONG tlvLen = (ULONG)pTmp->mLength;
      ULONG roamLen = (ULONG)sizeof( sNASGetServingSystemResponse_DefaultRoaming ); 
      if (tlvLen < roamLen)
      {
         return eGOBI_ERR_GENERAL;
      }

      pTmp++;
      const sNASGetServingSystemResponse_DefaultRoaming * pDR =
         (const sNASGetServingSystemResponse_DefaultRoaming *)pTmp;

      pRoam != 0 ? *pRoam = (ULONG)pDR->mRoamingIndicator : 0;
   }

   return status;
}

/*===========================================================================
METHOD:
   GetHomeNetwork (Public Method)

DESCRIPTION:
   Calls NASGetHomeNetwork

PARAMETERS:
   pHomeMCC       [ O ] - Mobile country code
   pHomeMNC       [ O ] - Mobile network code
   homeNameSize   [ I ] - Max name size
   pHomeName      [ O ] - Home network name
   pSID           [ O ] - System ID
   pNID           [ O ] - Network ID

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::GetHomeNetwork(
   WORD *                     pHomeMCC,
   WORD *                     pHomeMNC,
   BYTE                       homeNameSize,
   CHAR *                     pHomeName,
   WORD *                     pSID,
   WORD *                     pNID )
{
   // Assume failure
   if (homeNameSize > 0 && pHomeName != 0)
   {
      pHomeName[0] = 0;
   }

   pHomeMCC != 0 ? *pHomeMCC = 0xFFFF : 0;
   pHomeMNC != 0 ? *pHomeMNC = 0xFFFF : 0;
   pSID != 0 ? *pSID = 0xFFFF : 0;
   pNID != 0 ? *pNID = 0xFFFF : 0;

   // Query for home system?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   ULONG lo = 8096;
   BYTE rsp[8096] = { 0 };
   status = NASGetHomeNetwork( mhGobi, 2000, 0, 0, &lo, &rsp[0] );
   if (status != 0)
   {
      return status;
   }

   status = ParseGetHomeNetwork( lo, 
                                 &rsp[0], 
                                 pHomeMCC,
                                 pHomeMNC,
                                 homeNameSize,
                                 pHomeName,
                                 pSID,
                                 pNID );
   return status;
}

/*===========================================================================
METHOD:
   SetWDSEventReportCB (Public Method)

DESCRIPTION:
   Calls WDSSetEventReport/SetGenericCallback

PARAMETERS:
   pCallback   [ I ] - Callback function pointer
   interval    [ I ] - Interval (in seconds) for transfer statistics

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::SetWDSEventReportCB( 
   tFNGenericCallback         pCallback,
   BYTE                       interval )
{
   // Set WDS event callback?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   // Configure the QMI service
   UINT8 req[1024] = { 0 };
   UINT8 * pData = (UINT8 *)&req[0];

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)pData;
   pTLV->mTypeID = 0x11;
   pTLV->mLength = (UINT16)sizeof( sWDSSetEventReportRequest_TransferStatisticsIndicator );     
   pData += sizeof( sQMIRawContentHeader );

   sWDSSetEventReportRequest_TransferStatisticsIndicator * pTS =
      (sWDSSetEventReportRequest_TransferStatisticsIndicator *)pData;
   pTS->mTransferStatisticsIntervalSeconds = interval;
   pTS->mReportTXPacketSuccesses = 0;
   pTS->mReportRXPacketSuccesses = 0;
   pTS->mReportTXPacketErrors = 0;
   pTS->mReportRXPacketErrors = 0;
   pTS->mReportTXOverflows = 0;
   pTS->mReportRXOverflows = 0;
   pTS->mTXByteTotal = 1;
   pTS->mRXByteTotal = 1;
   pData += sizeof( sWDSSetEventReportRequest_TransferStatisticsIndicator );

   pTLV = (sQMIRawContentHeader *)pData;
   pTLV->mTypeID = 0x12;
   pTLV->mLength = (UINT16)sizeof( sWDSSetEventReportRequest_DataBearerTechnologyIndicator );     
   pData += sizeof( sQMIRawContentHeader );

   sWDSSetEventReportRequest_DataBearerTechnologyIndicator * pTI =
      (sWDSSetEventReportRequest_DataBearerTechnologyIndicator *)pData;
   pTI->mReportDataBearerTechnology = 1;
   pData += sizeof( sWDSSetEventReportRequest_DataBearerTechnologyIndicator );

   ULONG li = (ULONG)pData - (ULONG)&req[0];
   status = WDSSetEventReport( mhGobi, 2000, li, &req[0], 0, 0 );
   if (status != 0)
   {
      return status;
   }

   // Configure the callback with the API
   status = SetGenericCallback( mhGobi, 1, 1, pCallback );
   return status;
}

/*===========================================================================
METHOD:
   SetWDSSessionStateCB (Public Method)

DESCRIPTION:
   Calls SetGenericCallback

PARAMETERS:
   pCallback   [ I ] - Callback function pointer

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::SetWDSSessionStateCB( tFNGenericCallback pCallback )
{
   // Set WDS packet service status callback?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   // Configure the callback with the API
   status = SetGenericCallback( mhGobi, 1, 34, pCallback );
   return status;
}

/*===========================================================================
METHOD:
   SetNASEventReportCB (Public Method)

DESCRIPTION:
   Calls NASSetEventReport/SetGenericCallback

PARAMETERS:
   pCallback      [ I ] - Callback function pointer
   thresholdsSize [ I ] - Threshold size
   pThresholds    [ I ] - Array of thresholds

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::SetNASEventReportCB(
   tFNGenericCallback         pCallback,
   BYTE                       thresholdsSize,
   INT8 *                     pThresholds )
{
   // Set NAS event report callback request?
   ULONG status = eGOBI_ERR_GENERAL;
   if ( (mhGobi == 0)
   ||   (thresholdsSize > 0 && pThresholds == 0) )
   {
      return status;
   }

   // Configure the QMI service
   UINT8 req[1024] = { 0 };
   UINT8 * pData = (UINT8 *)&req[0];

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)pData;
   pTLV->mTypeID = 0x10;
   pTLV->mLength = (UINT16)sizeof( sNASSetEventReportRequest_SignalIndicator );     
   pTLV->mLength += (UINT16)thresholdsSize;
   pData += sizeof( sQMIRawContentHeader );

   sNASSetEventReportRequest_SignalIndicator * pSI =
      (sNASSetEventReportRequest_SignalIndicator *)pData;
   pSI->mReportSignalStrength = 1;
   pSI->mNumberOfThresholds = thresholdsSize;
   pData += sizeof( sNASSetEventReportRequest_SignalIndicator );

   for (UINT8 i = 0; i < thresholdsSize; i++)
   {
      INT8 * pThresh = (INT8 *)pData;
      *pThresh = pThresholds[i];
      pData++;
   }

   ULONG li = (ULONG)pData - (ULONG)&req[0];
   status = NASSetEventReport( mhGobi, 2000, li, &req[0], 0, 0 );
   if (status != 0)
   {
      return status;
   }

   // Configure the callback with the API
   status = SetGenericCallback( mhGobi, 3, 2, pCallback );
   return status;
}

/*===========================================================================
METHOD:
   SetNASServingSystemCB (Public Method)

DESCRIPTION:
   Calls SetGenericCallback

PARAMETERS:
   pCallback      [ I ] - Callback function pointer

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cGobiCMDLL::SetNASServingSystemCB( tFNGenericCallback pCallback )
{
   // Set NAS serving system request?
   ULONG status = eGOBI_ERR_GENERAL;
   if (mhGobi == 0)
   {
      return status;
   }

   // Configure the callback with the API
   status = SetGenericCallback( mhGobi, 3, 36, pCallback );
   return status;
}
