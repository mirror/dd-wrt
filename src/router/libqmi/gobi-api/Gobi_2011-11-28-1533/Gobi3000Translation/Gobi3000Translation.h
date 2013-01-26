/*===========================================================================
FILE:
   Gobi3000Translation.h

DESCRIPTION:
   QUALCOMM Tanslation for Gobi 3000

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

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

#ifndef GOBI_TYPEDEFS
#define GOBI_TYPEDEFS

// Type Definitions
typedef unsigned long      ULONG;
typedef unsigned long *    ULONG_PTR;
typedef unsigned long long ULONGLONG;
typedef signed char        INT8;
typedef unsigned char      UINT8;
typedef signed short       INT16;
typedef unsigned short     UINT16;
typedef signed int         INT32;
typedef unsigned int       UINT32;
typedef unsigned char      BYTE;
typedef char               CHAR;
typedef unsigned short     WORD;
typedef unsigned short     USHORT;
typedef const char *       LPCSTR;

#ifdef WINDOWS
   typedef signed __int64     INT64;
   typedef unsigned __int64   UINT64;
#else
   typedef signed long long   INT64;
   typedef unsigned long long UINT64;
#endif

#endif

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include <map>
#include <string.h>
#include <string>
#include "GobiConnectionMgmtAPIStructs.h"

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// Get a TLV
ULONG GetTLV(
   ULONG          inLen,
   const BYTE *   pIn,
   BYTE           typeID,
   ULONG *        pOutLen,
   const BYTE **  ppOut );

// WDS

ULONG ParseGetSessionState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState );

ULONG ParseGetSessionDuration(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONGLONG *       pDuration );

ULONG ParseGetDormancyState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState );

ULONG ParseGetEnhancedAutoconnect(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pSetting,
   ULONG *           pRoamSetting );

ULONG PackSetEnhancedAutoconnect(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             setting,
   ULONG *           pRoamSetting );

ULONG PackSetDefaultProfile(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             profileType,
   ULONG *           pPDPType,
   ULONG *           pIPAddress,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS,
   ULONG *           pAuthentication,
   CHAR *            pName,
   CHAR *            pAPNName,
   CHAR *            pUsername,
   CHAR *            pPassword );

ULONG PackGetDefaultProfile(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             profileType );

ULONG ParseGetDefaultProfile(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPDPType,
   ULONG *           pIPAddress,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS,
   ULONG *           pAuthentication,
   BYTE              nameSize,
   CHAR *            pName,
   BYTE              apnSize,
   CHAR *            pAPNName,
   BYTE              userSize,
   CHAR *            pUsername );

ULONG PackStartDataSession(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG *           pTechnology,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS,
   ULONG *           pPrimaryNBNS,
   ULONG *           pSecondaryNBNS,
   CHAR *            pAPNName,
   ULONG *           pIPAddress,
   ULONG *           pAuthentication,
   CHAR *            pUsername,
   CHAR *            pPassword );

ULONG ParseStartDataSession(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pSessionId,
   ULONG *           pFailureReason );

ULONG PackStopDataSession(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             sessionId );

ULONG PackGetIPAddress(
   ULONG *           pOutLen,
   BYTE *            pOut );

ULONG ParseGetIPAddress(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pIPAddress );

ULONG ParseGetConnectionRate(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pCurrentChannelTXRate,
   ULONG *           pCurrentChannelRXRate,
   ULONG *           pMaxChannelTXRate,
   ULONG *           pMaxChannelRXRate );

ULONG PackGetPacketStatus(
   ULONG *           pOutLen,
   BYTE *            pOut );

ULONG ParseGetPacketStatus(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pTXPacketSuccesses,
   ULONG *           pRXPacketSuccesses,
   ULONG *           pTXPacketErrors,
   ULONG *           pRXPacketErrors,
   ULONG *           pTXPacketOverflows,
   ULONG *           pRXPacketOverflows );

ULONG PackGetByteTotals(
   ULONG *           pOutLen,
   BYTE *            pOut );

ULONG ParseGetByteTotals(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONGLONG *       pTXTotalBytes,
   ULONGLONG *       pRXTotalBytes );

ULONG PackSetMobileIP(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             mode );

ULONG ParseGetMobileIP(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMode );

ULONG PackSetActiveMobileIPProfile(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE              index );

ULONG ParseGetActiveMobileIPProfile(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pIndex );

ULONG PackSetMobileIPProfile(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE              index,
   BYTE *            pEnabled,
   ULONG *           pAddress,
   ULONG *           pPrimaryHA,
   ULONG *           pSecondaryHA,
   BYTE *            pRevTunneling,
   CHAR *            pNAI,
   ULONG *           pHASPI,
   ULONG *           pAAASPI,
   CHAR *            pMNHA,
   CHAR *            pMNAAA );

ULONG PackGetMobileIPProfile(
   ULONG *           pOutLen,
   BYTE *            pOut,
   BYTE              index );

ULONG ParseGetMobileIPProfile(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pEnabled,
   ULONG *           pAddress,
   ULONG *           pPrimaryHA,
   ULONG *           pSecondaryHA,
   BYTE *            pRevTunneling,
   BYTE              naiSize,
   CHAR *            pNAI,
   ULONG *           pHASPI,
   ULONG *           pAAASPI,
   ULONG *           pHAState,
   ULONG *           pAAAState );

ULONG PackSetMobileIPParameters(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   ULONG *           pMode,
   BYTE *            pRetryLimit,
   BYTE *            pRetryInterval,
   BYTE *            pReRegPeriod,
   BYTE *            pReRegTraffic,
   BYTE *            pHAAuthenticator,
   BYTE *            pHA2002bis );

ULONG ParseGetMobileIPParameters(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMode,
   BYTE *            pRetryLimit,
   BYTE *            pRetryInterval,
   BYTE *            pReRegPeriod,
   BYTE *            pReRegTraffic,
   BYTE *            pHAAuthenticator,
   BYTE *            pHA2002bis );

ULONG ParseGetLastMobileIPError(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pError );

ULONG PackSetDNSSettings(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS );

ULONG ParseGetDNSSettings(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS );

ULONG ParseGetDataBearerTechnology(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pDataBearer );

// NAS

ULONG ParseGetANAAAAuthenticationStatus(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pStatus );

ULONG ParseGetSignalStrength(
   ULONG             inLen,
   const BYTE *      pIn,
   INT8 *            pSignalStrength,
   ULONG *           pRadioInterface );

ULONG ParseGetSignalStrengths(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pArraySizes,
   INT8 *            pSignalStrengths,
   ULONG *           pRadioInterfaces );

ULONG ParseGetRFInfo(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances );

ULONG ParsePerformNetworkScan(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances );

ULONG ParsePerformNetworkRATScan(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pInstanceSize,
   BYTE *            pInstances,
   BYTE *            pRATSize,
   BYTE *            pRATInstances );

ULONG PackInitiateNetworkRegistration(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             regType,
   WORD              mcc,
   WORD              mnc,
   ULONG             rat );

ULONG PackInitiateDomainAttach(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             action );

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
   CHAR *            pName );

ULONG ParseGetServingNetworkCapabilities(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pDataCapsSize,
   BYTE *            pDataCaps );

ULONG ParseGetHomeNetwork(
   ULONG             inLen,
   const BYTE *      pIn,
   WORD *            pMCC,
   WORD *            pMNC,
   BYTE              nameSize,
   CHAR *            pName,
   WORD *            pSID,
   WORD *            pNID );

ULONG PackSetNetworkPreference(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             technologyPref,
   ULONG             duration );

ULONG ParseGetNetworkPreference(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pTechnologyPref,
   ULONG *           pDuration,
   ULONG *           pPersistentTechnologyPref );

ULONG PackSetCDMANetworkParameters(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE *            pForceRev0,
   BYTE *            pCustomSCP,
   ULONG *           pProtocol,
   ULONG *           pBroadcast,
   ULONG *           pApplication,
   ULONG *           pRoaming );

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
   ULONG *           pRoaming );

ULONG ParseGetACCOLC(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pACCOLC );

ULONG PackSetACCOLC(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC,
   BYTE              accolc );

ULONG ParseGetPLMNMode(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMode );

ULONG PackGetPLMNName(
   ULONG *           pOutLen,
   BYTE *            pOut,
   USHORT            mcc,
   USHORT            mnc );

ULONG ParseGetPLMNName(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pNamesSize,
   BYTE *            pNames );

// DMS

ULONG ParseGetDeviceCapabilities(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMaxTXChannelRate,
   ULONG *           pMaxRXChannelRate,
   ULONG *           pDataServiceCapability,
   ULONG *           pSimCapability,
   ULONG *           pRadioIfacesSize,
   BYTE *            pRadioIfaces );

ULONG ParseGetManufacturer(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString );

ULONG ParseGetModelID(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString );

ULONG ParseGetFirmwareRevision(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString );

ULONG ParseGetFirmwareRevisions(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              amssSize,
   CHAR *            pAMSSString,
   BYTE              bootSize,
   CHAR *            pBootString,
   BYTE              priSize,
   CHAR *            pPRIString );

ULONG ParseGetVoiceNumber(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              voiceNumberSize,
   CHAR *            pVoiceNumber,
   BYTE              minSize,
   CHAR *            pMIN );

ULONG ParseGetIMSI(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString );

ULONG ParseGetSerialNumbers(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              esnSize,
   CHAR *            pESNString,
   BYTE              imeiSize,
   CHAR *            pIMEIString,
   BYTE              meidSize,
   CHAR *            pMEIDString );

ULONG PackSetLock(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             state,
   CHAR *            pCurrentPIN );

ULONG ParseQueryLock(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState );

ULONG PackChangeLockPIN(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pCurrentPIN,
   CHAR *            pDesiredPIN );

ULONG ParseGetHardwareRevision(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString );

ULONG ParseGetPRLVersion(
   ULONG             inLen,
   const BYTE *      pIn,
   WORD *            pPRLVersion );

ULONG ParseGetERIFile(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pFileSize,
   BYTE *            pFile );

ULONG PackActivateAutomatic(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pActivationCode );

ULONG PackResetToFactoryDefaults(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC );

ULONG ParseGetActivationState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pActivationState );

ULONG PackSetPower(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             powerMode );

ULONG ParseGetPower(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPowerMode );

ULONG ParseGetOfflineReason(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pReasonMask,
   ULONG *           pbPlatform );

ULONG ParseGetNetworkTime(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONGLONG *       pTimeCount,
   ULONG *           pTimeSource );

ULONG PackValidateSPC(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC );

// SMS

ULONG PackDeleteSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG *           pMessageIndex,
   ULONG *           pMessageTag );

ULONG PackGetSMSList(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG *           pRequestedTag );

ULONG ParseGetSMSList(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageListSize,
   BYTE *            pMessageList );

ULONG PackGetSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageIndex );

ULONG ParseGetSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageTag,
   ULONG *           pMessageFormat,
   ULONG *           pMessageSize,
   BYTE *            pMessage );

ULONG PackModifySMSStatus(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageIndex,
   ULONG             messageTag );

ULONG PackSaveSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             storageType,
   ULONG             messageFormat,
   ULONG             messageSize,
   BYTE *            pMessage );

ULONG ParseSaveSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageIndex );

ULONG PackSendSMS(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             messageFormat,
   ULONG             messageSize,
   BYTE *            pMessage );

ULONG ParseSendSMS(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMessageFailureCode );

ULONG ParseGetSMSCAddress(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              addressSize,
   CHAR *            pSMSCAddress,
   BYTE              typeSize,
   CHAR *            pSMSCType );

ULONG PackSetSMSCAddress(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSMSCAddress,
   CHAR *            pSMSCType );

ULONG ParseGetSMSRoutes(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pRouteSize,
   BYTE *            pRoutes );

ULONG PackSetSMSRoutes(
   ULONG *           pOutLen,
   BYTE *            pOut,
   BYTE *            pRouteSize,
   BYTE *            pRoutes );

// DMS UIM

ULONG PackUIMUnblockControlKey( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id, 
   CHAR *            pValue );

ULONG ParseUIMUnblockControlKey( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMSetControlKeyProtection( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id, 
   ULONG             status,
   CHAR *            pValue );

ULONG ParseUIMSetControlKeyProtection( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pVerifyRetriesLeft );

ULONG PackUIMGetControlKeyBlockingStatus( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id );

ULONG ParseUIMGetControlKeyBlockingStatus( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pStatus,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft,
   ULONG *           pbBlocking );

ULONG ParseUIMGetControlKeyStatus( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pStatus,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMGetControlKeyStatus( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id );

ULONG ParseUIMGetICCID( 
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize, 
   CHAR *            pString );

ULONG ParseUIMGetPINStatus( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG             id,
   ULONG *           pStatus,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMChangePIN( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id,
   CHAR *            pOldValue,
   CHAR *            pNewValue );

ULONG ParseUIMChangePIN( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMUnblockPIN( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id,
   CHAR *            pOldValue,
   CHAR *            pNewValue );

ULONG ParseUIMUnblockPIN( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMVerifyPIN( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id,
   CHAR *            pValue );

ULONG ParseUIMVerifyPIN( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

ULONG PackUIMSetPINProtection( 
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             id,
   ULONG             bEnable,
   CHAR *            pValue );

ULONG ParseUIMSetPINProtection( 
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pVerifyRetriesLeft,
   ULONG *           pUnblockRetriesLeft );

// PDS

ULONG ParseGetPDSState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pEnabled,
   ULONG *           pTracking );

ULONG PackSetPDSState(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             enable );

ULONG PackPDSInjectTimeReference(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONGLONG         systemTime,
   USHORT            systemDiscontinuities );

ULONG ParseGetPDSDefaults(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pOperation,
   BYTE *            pTimeout,
   ULONG *           pInterval,
   ULONG *           pAccuracy );

ULONG PackSetPDSDefaults(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             operation,
   BYTE              timeout,
   ULONG             interval,
   ULONG             accuracy );

ULONG ParseGetXTRAAutomaticDownload(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pbEnabled,
   USHORT *          pInterval );

ULONG PackSetXTRAAutomaticDownload(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bEnabled,
   USHORT            interval );

ULONG ParseGetXTRANetwork(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPreference );

ULONG PackSetXTRANetwork(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             preference );

ULONG ParseGetXTRAValidity(
   ULONG             inLen,
   const BYTE *      pIn,
   USHORT *          pGPSWeek,
   USHORT *          pGPSWeekOffset,
   USHORT *          pDuration );

ULONG ParseGetXTRADataState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState );

ULONG PackSetXTRADataState(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             state );

ULONG ParseGetXTRATimeState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState );

ULONG PackSetXTRATimeState(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             state );

ULONG ParseGetAGPSConfig(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pServerAddress,
   ULONG *           pServerPort );

ULONG PackSetAGPSConfig(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             serverAddress,
   ULONG             serverPort );

ULONG ParseGetServiceAutomaticTracking(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pbAuto );

ULONG PackSetServiceAutomaticTracking(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bAuto );

ULONG ParseGetPortAutomaticTracking(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pbAuto );

ULONG PackSetPortAutomaticTracking(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bAuto );

ULONG PackResetPDSData(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG *           pGPSDataMask,
   ULONG *           pCellDataMask );

// CAT

ULONG PackCATSendTerminalResponse(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             refID,
   ULONG             dataLen,
   BYTE *            pData );

ULONG PackCATSendEnvelopeCommand(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             cmdID,
   ULONG             dataLen,
   BYTE *            pData );

// RMS

ULONG ParseGetSMSWake(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pbEnabled,
   ULONG *           pWakeMask );

ULONG PackSetSMSWake(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bEnable,
   ULONG             wakeMask );

// OMADM

ULONG PackOMADMStartSession(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             sessionType );

ULONG ParseOMADMGetSessionInfo(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pSessionState,
   ULONG *           pSessionType,
   ULONG *           pFailureReason,
   BYTE *            pRetryCount,
   WORD *            pSessionPause,
   WORD *            pTimeRemaining );

ULONG ParseOMADMGetPendingNIA(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pSessionType,
   USHORT *          pSessionID );

ULONG PackOMADMSendSelection(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             selection,
   USHORT            sessionID );

ULONG ParseOMADMGetFeatureSettings(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pbProvisioning,
   ULONG *           pbPRLUpdate );

ULONG PackOMADMSetProvisioningFeature(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bProvisioning );

ULONG PackOMADMSetPRLUpdateFeature(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             bPRLUpdate );

// Voice

ULONG PackOriginateUSSD(
   ULONG *           pOutLen,
   BYTE *            pOut,
   BYTE *            pInfo );

ULONG PackAnswerUSSD(
   ULONG *           pOutLen,
   BYTE *            pOut,
   BYTE *            pInfo );

