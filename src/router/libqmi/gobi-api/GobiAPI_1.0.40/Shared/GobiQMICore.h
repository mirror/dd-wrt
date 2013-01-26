/*===========================================================================
FILE: 
   GobiQMICore.h

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core

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

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "CoreDatabase.h"
#include "ProtocolBuffer.h"
#include "QMIProtocolServer.h"
#include "DataParser.h"
#include "DataPacker.h"
#include "DB2Utilities.h"
#include "SyncQueue.h"
#include "GobiError.h"
#include "GobiMBNMgmt.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Default timeout for Gobi QMI requests
extern const ULONG DEFAULT_GOBI_QMI_TIMEOUT;

/*=========================================================================*/
// Prototypes 
/*=========================================================================*/

// Find the given TLV
sDB2NavInput FindTLV( 
   const std::vector <sDB2NavInput> &  tlvs, 
   const sProtocolEntityKey &          tlvKey );

// Parse the given TLV to fields
cDataParser::tParsedFields ParseTLV( 
   const cCoreDatabase &               db,
   const sProtocolBuffer &             qmiBuf,
   const std::vector <sDB2NavInput> &  tlvs, 
   const sProtocolEntityKey &          tlvKey,
   bool                                bFieldStrings = false );

/*=========================================================================*/
// Class cGobiQMICore
/*=========================================================================*/
class cGobiQMICore
{
   public:
      // Constructor
      cGobiQMICore();

      // Destructor
      virtual ~cGobiQMICore();

      // Initialize the object
      virtual bool Initialize();

      // Cleanup the object
      virtual bool Cleanup();

      // (Inline) Return the QMI database
      const cCoreDatabase & GetDatabase()
      {
         return mDB;
      };

      // (Inline) Return the server as determined by the service type
      cQMIProtocolServer * GetServer( eQMIService svc )
      {
         cQMIProtocolServer * pSvr = 0;

         std::map <eQMIService, cQMIProtocolServer *>::const_iterator pIter;
         pIter = mServers.find( svc );

         if (pIter != mServers.end())
         {
            pSvr = pIter->second;
         }

         return pSvr;
      };

      // (Inline) Clear last error recorded
      void ClearLastError()
      {
         mLastError = eGOBI_ERR_NONE;
      };

      // (Inline) Get last error recorded
      eGobiError GetLastError()
      {
         return mLastError;
      };

      // (Inline) Return the last recorded error (if this happens to indicate 
      // that no error occurred then return eGOBI_ERR_INTERNAL)
      eGobiError GetCorrectedLastError()
      {
         eGobiError ec = GetLastError();
         if (ec == eGOBI_ERR_NONE)
         {
            ec = eGOBI_ERR_INTERNAL;
         }

         return ec;
      };

      // (Inline) Return the correct QMI error (if this happens to indicate 
      // that no error occurred then return the mapped eQMI_ERR_INTERNAL 
      // value)
      eGobiError GetCorrectedQMIError( ULONG qmiErrorCode )
      {
         ULONG ec = (ULONG)eQMI_ERR_INTERNAL + (ULONG)eGOBI_ERR_QMI_OFFSET;
         if (qmiErrorCode != (ULONG)eQMI_ERR_NONE)
         {
            ec = qmiErrorCode + (ULONG)eGOBI_ERR_QMI_OFFSET;
         }

         return (eGobiError)ec;
      };

      // Return the set of available Gobi devices
      typedef std::pair <std::string, std::string> tDeviceID;
      virtual std::vector <tDeviceID> GetAvailableDevices();

      // Connect to the specified (or first detected) Gobi device
      virtual bool Connect(
         LPCSTR                     pDeviceNode = 0,
         LPCSTR                     pDeviceKey = 0 );

      // Disconnect from the currently connected Gobi device
      virtual bool Disconnect();

      // Get the device ID of the currently connected Gobi device
      virtual bool GetConnectedDeviceID(
         std::string &                 devNode,
         std::string &                 devKey );

      // Send a request using the specified QMI protocol server and wait 
      // for (and then return) the response
      sProtocolBuffer Send(
         eQMIService                svc,
         sSharedBuffer *            pRequest,
         ULONG                      to = DEFAULT_GOBI_QMI_TIMEOUT );

      // Send a request using the specified QMI protocol server and wait 
      // for (and then validate) the response
      eGobiError SendAndCheckReturn(
         eQMIService                svc,
         sSharedBuffer *            pRequest,
         ULONG                      to = DEFAULT_GOBI_QMI_TIMEOUT );

      // Generate/send a request using the specified QMI protocol server 
      // and wait for (and then return) the response
      sProtocolBuffer SendSimple(
         eQMIService                svc,
         WORD                       msgID,
         ULONG                      to = DEFAULT_GOBI_QMI_TIMEOUT );

      // Cancel the most recent in-progress Send() based operation
      eGobiError CancelSend();

#ifdef WDS_SUPPORT
      // Return the state of the current packet data session
      eGobiError GetSessionState( ULONG * pState );

      // Return the duration of the current packet data session
      eGobiError GetSessionDuration( ULONGLONG * pDuration );

      // Return the active/total durations of the current packet data session
      eGobiError GetSessionDurations( 
         ULONGLONG *                pActiveDuration,
         ULONGLONG *                pTotalDuration );

      // Return the dormancy state of the current packet session
      eGobiError GetDormancyState( ULONG * pState );

      // Return the current autoconnect data session setting
      eGobiError GetEnhancedAutoconnect( 
         ULONG *                    pSetting,
         ULONG *                    pRoamSetting );

      // Set the autoconnect data session setting
      eGobiError SetEnhancedAutoconnect( 
         ULONG                      setting,
         ULONG *                    pRoamSetting );

      // Write the default profile settings to the device
      eGobiError SetDefaultProfile( 
         ULONG                      profileType,
         ULONG *                    pPDPType, 
         ULONG *                    pIPAddress, 
         ULONG *                    pPrimaryDNS, 
         ULONG *                    pSecondaryDNS, 
         ULONG *                    pAuthentication, 
         CHAR *                     pName, 
         CHAR *                     pAPNName, 
         CHAR *                     pUsername,
         CHAR *                     pPassword );

      // Read the default profile settings from the device
      eGobiError GetDefaultProfile( 
         ULONG                      profileType,
         ULONG *                    pPDPType, 
         ULONG *                    pIPAddress, 
         ULONG *                    pPrimaryDNS, 
         ULONG *                    pSecondaryDNS, 
         ULONG *                    pAuthentication, 
         BYTE                       nameSize,
         CHAR *                     pName, 
         BYTE                       apnSize,
         CHAR *                     pAPNName, 
         BYTE                       userSize,
         CHAR *                     pUsername );

      // Activate a packet data session
      eGobiError StartDataSession( 
         ULONG *                    pTechnology, 
         ULONG *                    pPrimaryDNS, 
         ULONG *                    pSecondaryDNS, 
         ULONG *                    pPrimaryNBNS, 
         ULONG *                    pSecondaryNBNS, 
         CHAR *                     pAPNName, 
         ULONG *                    pIPAddress, 
         ULONG *                    pAuthentication, 
         CHAR *                     pUsername, 
         CHAR *                     pPassword,
         ULONG *                    pSessionId,
         ULONG *                    pFailureReason );

      // Cancel an in-progress packet data session activation
      eGobiError CancelDataSession();

      // Stop the current data session
      eGobiError StopDataSession( ULONG sessionId );

      // Return the current packet data session IP address
      eGobiError GetIPAddress( ULONG * pIPAddress );

      // Return connection rate information for the packet data connection
      eGobiError GetConnectionRate(
         ULONG *                    pCurrentChannelTXRate,
         ULONG *                    pCurrentChannelRXRate,
         ULONG *                    pMaxChannelTXRate,
         ULONG *                    pMaxChannelRXRate );

      // Return the packet data transfer statistics
      eGobiError GetPacketStatus(  
         ULONG *                    pTXPacketSuccesses, 
         ULONG *                    pRXPacketSuccesses, 
         ULONG *                    pTXPacketErrors, 
         ULONG *                    pRXPacketErrors, 
         ULONG *                    pTXPacketOverflows, 
         ULONG *                    pRXPacketOverflows );

      // Returns the RX/TX byte counts
      eGobiError GetByteTotals(   
         ULONGLONG *                pTXTotalBytes, 
         ULONGLONG *                pRXTotalBytes );

      // Set the current mobile IP setting
      eGobiError SetMobileIP( ULONG mode );

      // Get the current mobile IP setting
      eGobiError GetMobileIP( ULONG * pMode );

      // Set the active mobile IP profile index
      eGobiError SetActiveMobileIPProfile( 
         CHAR *                     pSPC,
         BYTE                       index );

      // Get the active mobile IP profile index
      eGobiError GetActiveMobileIPProfile( BYTE * pIndex );

      // Set the specified mobile IP profile settings
      eGobiError SetMobileIPProfile( 
         CHAR *                     pSPC,
         BYTE                       index,
         BYTE *                     pEnabled,
         ULONG *                    pAddress,
         ULONG *                    pPrimaryHA,
         ULONG *                    pSecondaryHA,
         BYTE *                     pRevTunneling,
         CHAR *                     pNAI,
         ULONG *                    pHASPI,
         ULONG *                    pAAASPI,
         CHAR *                     pMNHA,
         CHAR *                     pMNAAA );

      // Get the specified mobile IP profile settings
      eGobiError GetMobileIPProfile( 
         BYTE                       index,
         BYTE *                     pEnabled,
         ULONG *                    pAddress,
         ULONG *                    pPrimaryHA,
         ULONG *                    pSecondaryHA,
         BYTE *                     pRevTunneling,
         BYTE                       naiSize,
         CHAR *                     pNAI,
         ULONG *                    pHASPI,
         ULONG *                    pAAASPI,
         ULONG *                    pHAState,
         ULONG *                    pAAAState );

      // Set the mobile IP parameters
      eGobiError SetMobileIPParameters( 
         CHAR *                     pSPC,
         ULONG *                    pMode,
         BYTE *                     pRetryLimit,
         BYTE *                     pRetryInterval,
         BYTE *                     pReRegPeriod,
         BYTE *                     pReRegTraffic,
         BYTE *                     pHAAuthenticator,
         BYTE *                     pHA2002bis );

      // Get the mobile IP parameters
      eGobiError GetMobileIPParameters( 
         ULONG *                    pMode,
         BYTE *                     pRetryLimit,
         BYTE *                     pRetryInterval,
         BYTE *                     pReRegPeriod,
         BYTE *                     pReRegTraffic,
         BYTE *                     pHAAuthenticator,
         BYTE *                     pHA2002bis );

      // Get the last mobile IP error
      eGobiError GetLastMobileIPError( ULONG * pError );

      // Set the DNS settings for the device
      eGobiError SetDNSSettings( 
         ULONG *                    pPrimaryDNS, 
         ULONG *                    pSecondaryDNS ); 

      // Get the DNS settings for the device
      eGobiError GetDNSSettings( 
         ULONG *                    pPrimaryDNS, 
         ULONG *                    pSecondaryDNS ); 
#endif

#ifdef NAS_SUPPORT
      // Get the AN-AAA authentication status
      eGobiError GetANAAAAuthenticationStatus( ULONG * pStatus );

      // Get the current available signal strengths (in dBm)
      eGobiError GetSignalStrengths( 
         ULONG *                    pArraySizes, 
         INT8 *                     pSignalStrengths, 
         ULONG *                    pRadioInterfaces );

      // Get the current RF information
      eGobiError GetRFInfo( 
         BYTE *                     pInstanceSize, 
         BYTE *                     pInstances );

      // Perform a scan for available networks
      eGobiError PerformNetworkScan( 
         BYTE *                     pInstanceSize, 
         BYTE *                     pInstances );

      // Perform a scan for available networks (includes RAT)
      eGobiError PerformNetworkRATScan( 
         BYTE *                     pInstanceSize, 
         BYTE *                     pInstances,
         BYTE *                     pRATSize, 
         BYTE *                     pRATInstances );

      // Initiate a network registration
      eGobiError InitiateNetworkRegistration( 
         ULONG                      regType,
         WORD                       mcc, 
         WORD                       mnc, 
         ULONG                      rat );

      // Initiate a domain attach (or detach)
      eGobiError InitiateDomainAttach( ULONG action );

      // Get information regarding the system that currently provides service
      eGobiError GetServingNetwork( 
         ULONG *                    pRegistrationState, 
         ULONG *                    pCSDomain, 
         ULONG *                    pPSDomain, 
         ULONG *                    pRAN, 
         BYTE *                     pRadioIfacesSize, 
         BYTE *                     pRadioIfaces, 
         ULONG *                    pRoaming, 
         WORD *                     pMCC, 
         WORD *                     pMNC, 
         BYTE                       nameSize, 
         CHAR *                     pName );

      // Get data capabilities of serving network system
      eGobiError GetServingNetworkCapabilities( 
         BYTE *                     pDataCapssSize, 
         BYTE *                     pDataCaps );

      // Retrieves the current data bearer technology
      eGobiError GetDataBearerTechnology( ULONG * pDataBearer );

      // Retrieve information about the home network of the device
      eGobiError GetHomeNetwork( 
         WORD *                     pMCC, 
         WORD *                     pMNC, 
         BYTE                       nameSize, 
         CHAR *                     pName, 
         WORD *                     pSID, 
         WORD *                     pNID );

      // Sets the network registration preference
      eGobiError SetNetworkPreference( 
         ULONG                      technologyPref, 
         ULONG                      duration );

      // Return the network registration preference
      eGobiError GetNetworkPreference( 
         ULONG *                    pTechnologyPref, 
         ULONG *                    pDuration, 
         ULONG *                    pPersistentTechnologyPref );

      // Set the current CDMA network parameters
      eGobiError SetCDMANetworkParameters( 
         CHAR *                     pSPC,
         BYTE *                     pForceRev0,
         BYTE *                     pCustomSCP,
         ULONG *                    pProtocol,
         ULONG *                    pBroadcast,
         ULONG *                    pApplication,
         ULONG *                    pRoaming );

      // Return the current CDMA network parameters
      eGobiError GetCDMANetworkParameters( 
         BYTE *                     pSCI,
         BYTE *                     pSCM,
         BYTE *                     pRegHomeSID,
         BYTE *                     pRegForeignSID,
         BYTE *                     pRegForeignNID,
         BYTE *                     pForceRev0,
         BYTE *                     pCustomSCP,
         ULONG *                    pProtocol,
         ULONG *                    pBroadcast,
         ULONG *                    pApplication,
         ULONG *                    pRoaming );

      // Return the Access Overload Class (ACCOLC) of the device
      eGobiError GetACCOLC( BYTE * pACCOLC );

      // Set the Access Overload Class (ACCOLC) of the device
      eGobiError SetACCOLC( 
         CHAR *                     pSPC,
         BYTE                       accolc );

      // Return the PLMN mode from the CSP
      eGobiError GetPLMNMode( ULONG * pMode );

      // Return PLMN name information for the given MCC/MNC
      eGobiError GetPLMNName(
         USHORT                     mcc,
         USHORT                     mnc,
         ULONG *                    pNamesSize, 
         BYTE *                     pNames );
#endif

#ifdef DMS_SUPPORT
      // Get device capabilities
      eGobiError GetDeviceCapabilities( 
         ULONG *                    pMaxTXChannelRate, 
         ULONG *                    pMaxRXChannelRate, 
         ULONG *                    pDataServiceCapability, 
         ULONG *                    pSimCapability, 
         ULONG *                    pRadioIfacesSize, 
         BYTE *                     pRadioIfaces );

      // Return the device manufacturer name
      eGobiError GetManufacturer( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return the device model ID
      eGobiError GetModelID( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return the device firmware revision
      eGobiError GetFirmwareRevision( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return the device firmware revisions
      eGobiError GetFirmwareRevisions( 
         BYTE                       amssSize, 
         CHAR *                     pAMSSString,
         BYTE                       bootSize, 
         CHAR *                     pBootString,
         BYTE                       priSize, 
         CHAR *                     pPRIString );

      // Return the voice number in use by the device
      eGobiError GetVoiceNumber( 
         BYTE                       voiceNumberSize, 
         CHAR *                     pVoiceNumber,
         BYTE                       minSize, 
         CHAR *                     pMIN );

      // Return the device IMSI
      eGobiError GetIMSI( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return all serial numbers assigned to the device
      eGobiError GetSerialNumbers( 
         BYTE                       esnSize, 
         CHAR *                     pESNString, 
         BYTE                       imeiSize, 
         CHAR *                     pIMEIString, 
         BYTE                       meidSize, 
         CHAR *                     pMEIDString );

      // Set the user lock state maintained by the device
      eGobiError SetLock( 
         ULONG                      state, 
         CHAR *                     pCurrentPIN );

      // Set the user lock state maintained by the device
      eGobiError QueryLock( ULONG * pState );

      // Set the user lock code maintained by the device
      eGobiError ChangeLockPIN( 
         CHAR *                     pCurrentPIN, 
         CHAR *                     pDesiredPIN );

      // Return the device hardware revision
      eGobiError GetHardwareRevision( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return the version of the active Preferred Roaming List (PRL)
      eGobiError GetPRLVersion( WORD * pPRLVersion );

      // Return the ERI file that is stored in EFS on the device
      eGobiError GetERIFile( 
         ULONG *                    pFileSize, 
         BYTE *                     pFile );

      // Request the device to perform automatic service activation
      eGobiError ActivateAutomatic( CHAR * pActivationCode );

      // Request the device perform manual service activation
      eGobiError ActivateManual( 
         CHAR *                     pSPC,
         WORD                       sid, 
         CHAR *                     pMDN,
         CHAR *                     pMIN, 
         ULONG                      prlSize, 
         BYTE *                     pPRL, 
         CHAR *                     pMNHA,
         CHAR *                     pMNAAA );

      // Requests the device reset configuration to factory defaults
      eGobiError ResetToFactoryDefaults( CHAR * pSPC );

      // Return the device activation state
      eGobiError GetActivationState( ULONG * pActivationState );

      // Set the operating mode of the device`
      eGobiError SetPower( ULONG powerMode );

      // Return the operating mode of the device
      eGobiError GetPower( ULONG * pPowerMode );

      // Return operating mode info from the device
      eGobiError GetPowerInfo( 
         ULONG *                    pPowerMode,
         ULONG *                    pReasonMask,
         ULONG *                    pbPlatform );

      // Return the reason why the device is currently offline
      eGobiError GetOfflineReason( 
         ULONG *                    pReasonMask,
         ULONG *                    pbPlatform );

      // Return the current time of the device
      eGobiError GetNetworkTime( 
         ULONGLONG *                pTimeCount,  
         ULONG *                    pTimeSource );

      // Validates the service programming code
      eGobiError ValidateSPC( CHAR * pSPC );
#endif

#ifdef UIM_SUPPORT
      // Enable or disable protection of UIM contents by a given PIN
      eGobiError UIMSetPINProtection( 
         ULONG                      id, 
         ULONG                      bEnable,
         CHAR *                     pValue,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft );

      // Verify the PIN before accessing the UIM contents
      eGobiError UIMVerifyPIN( 
         ULONG                      id, 
         CHAR *                     pValue,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft );

      // Unblock a blocked PIN
      eGobiError UIMUnblockPIN( 
         ULONG                      id, 
         CHAR *                     pPUKValue,
         CHAR *                     pNewValue,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft );

      // Change the PIN value
      eGobiError UIMChangePIN( 
         ULONG                      id, 
         CHAR *                     pOldValue,
         CHAR *                     pNewValue,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft );

      // Return the status of the pin
      eGobiError UIMGetPINStatus( 
         ULONG                      id, 
         ULONG *                    pStatus,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft );

      // Return the UIM ICCID
      eGobiError UIMGetICCID( 
         BYTE                       stringSize, 
         CHAR *                     pString );

      // Return the blocking status of the specified facility control key
      eGobiError UIMGetControlKeyBlockingStatus( 
         ULONG                      id, 
         ULONG *                    pStatus,
         ULONG *                    pVerifyRetriesLeft,
         ULONG *                    pUnblockRetriesLeft,
         ULONG *                    pbBlocking );

      // Change the specified facility control key
      eGobiError UIMSetControlKeyProtection( 
         ULONG                      id, 
         ULONG                      status,
         CHAR *                     pValue,
         ULONG *                    pVerifyRetriesLeft );

      // Unblock the specified facility control key
      eGobiError UIMUnblockControlKey( 
         ULONG                      id, 
         CHAR *                     pValue,
         ULONG *                    pUnblockRetriesLeft );
#endif

#ifdef WMS_SUPPORT
      // Delete one or more SMS messages from device memory
      eGobiError DeleteSMS( 
         ULONG                      storageType, 
         ULONG *                    pMessageIndex, 
         ULONG *                    pMessageTag );

      // Return the list of SMS messages stored on the device
      eGobiError GetSMSList( 
         ULONG                      storageType, 
         ULONG *                    pRequestedTag,
         ULONG *                    pMessageListSize, 
         BYTE *                     pMessageList );

      // Return an SMS message from device memory
      eGobiError GetSMS( 
         ULONG                      storageType, 
         ULONG                      messageIndex, 
         ULONG *                    pMessageTag,
         ULONG *                    pMessageFormat,
         ULONG *                    pMessageSize, 
         BYTE *                     pMessage );

      // Modify the status of an SMS message
      eGobiError ModifySMSStatus( 
         ULONG                      storageType, 
         ULONG                      messageIndex, 
         ULONG                      messageTag );

      // Save an SMS message to device memory
      eGobiError SaveSMS(    
         ULONG                      storageType, 
         ULONG                      messageFormat, 
         ULONG                      messageSize, 
         BYTE *                     pMessage, 
         ULONG *                    pMessageIndex );

      // Send an SMS message for immediate over the air transmission
      eGobiError SendSMS(    
         ULONG                      messageFormat, 
         ULONG                      messageSize, 
         BYTE *                     pMessage, 
         ULONG *                    pMessageFailureCode );

      // Return the SMS center address
      eGobiError GetSMSCAddress( 
         BYTE                       addressSize,
         CHAR *                     pSMSCAddress,
         BYTE                       typeSize,
         CHAR *                     pSMSCType );

      // Set the SMS center address
      eGobiError SetSMSCAddress( 
         CHAR *                     pSMSCAddress, 
         CHAR *                     pSMSCType );

      // Get the current incoming SMS routing information
      eGobiError GetSMSRoutes( 
         BYTE *                     pRouteSize, 
         BYTE *                     pRoutes );

      // Set the desired incoming SMS routing information
      eGobiError SetSMSRoutes( 
         BYTE *                     pRouteSize, 
         BYTE *                     pRoutes );
#endif

#ifdef PDS_SUPPORT
      // Return the current PDS state
      eGobiError GetPDSState( 
         ULONG *                    pEnabled,
         ULONG *                    pTracking );

      // Set the PDS state
      eGobiError SetPDSState( ULONG enable );

      // Inject a system time into the PDS engine
      eGobiError PDSInjectTimeReference( 
         ULONGLONG                  systemTime,
         USHORT                     systemDiscontinuities );

      // Return the default tracking session configuration
      eGobiError GetPDSDefaults( 
         ULONG *                    pOperation,
         BYTE *                     pTimeout,
         ULONG *                    pInterval,
         ULONG *                    pAccuracy );

      // Set the default tracking session configuration
      eGobiError SetPDSDefaults( 
         ULONG                      operation,
         BYTE                       timeout,
         ULONG                      interval,
         ULONG                      accuracy );

      // Return the XTRA automatic download configuration
      eGobiError GetXTRAAutomaticDownload( 
         ULONG *                    pbEnabled,
         USHORT *                   pInterval );

      // Set the XTRA automatic download configuration
      eGobiError SetXTRAAutomaticDownload( 
         ULONG                      bEnabled,
         USHORT                     interval );

      // Return the XTRA WWAN network preference
      eGobiError GetXTRANetwork( ULONG * pPreference );

      // Set the XTRA WWAN network preference
      eGobiError SetXTRANetwork( ULONG preference );

      // Return the XTRA database validity period
      eGobiError GetXTRAValidity(
         USHORT *                   pGPSWeek,
         USHORT *                   pGPSWeekOffset, 
         USHORT *                   pDuration );

      // Force the XTRA database to be downloaded to the device
      eGobiError ForceXTRADownload();

      // Return the XTRA data positioning state
      eGobiError GetXTRADataState( ULONG * pState );

      // Set the XTRA data positioning state
      eGobiError SetXTRADataState( ULONG state );

      // Return the XTRA time positioning state
      eGobiError GetXTRATimeState( ULONG * pState );

      // Set the XTRA time positioning state
      eGobiError SetXTRATimeState( ULONG state );

      // Return the PDS AGPS configuration
      eGobiError GetAGPSConfig( 
         ULONG *                    pServerAddress,
         ULONG *                    pServerPort );

      // Set the PDS AGPS configuration
      eGobiError SetAGPSConfig( 
         ULONG                      serverAddress,
         ULONG                      serverPort );

      // Return the automatic tracking state for the service
      eGobiError GetServiceAutomaticTracking( ULONG * pbAuto );

      // Set the automatic tracking state for the service
      eGobiError SetServiceAutomaticTracking( ULONG bAuto );

      // Return the automatic tracking config for the NMEA COM port
      eGobiError GetPortAutomaticTracking( ULONG * pbAuto );

      // Set the automatic tracking config for the NMEA COM port
      eGobiError SetPortAutomaticTracking( ULONG bAuto );

      // Reset the specified PDS data
      eGobiError ResetPDSData( 
         ULONG *                    pGPSDataMask,
         ULONG *                    pCellDataMask );
#endif

#ifdef CAT_SUPPORT
      // Send the terminal response to the device
      eGobiError CATSendTerminalResponse( 
         ULONG                      refID,
         ULONG                      dataLen,
         BYTE *                     pData );

      // Send the envelope command to the device
      eGobiError CATSendEnvelopeCommand( 
         ULONG                      cmdID,
         ULONG                      dataLen,
         BYTE *                     pData );
#endif

#ifdef RMS_SUPPORT
      // Query the state of the SMS wake functionality
      eGobiError GetSMSWake( 
         ULONG *                    pbEnabled,
         ULONG *                    pWakeMask );

      // Enable/disable the SMS wake functionality
      eGobiError SetSMSWake( 
         ULONG                      bEnable,
         ULONG                      wakeMask );
#endif

#ifdef OMA_SUPPORT
      // Start an OMA-DM session
      eGobiError OMADMStartSession( ULONG sessionType );

      // Cancel an ongoing OMA-DM session
      eGobiError OMADMCancelSession();

      // Return info related to the current (or previous) OMA-DM session
      eGobiError OMADMGetSessionInfo( 
         ULONG *                    pSessionState,
         ULONG *                    pSessionType,
         ULONG *                    pFailureReason,
         BYTE *                     pRetryCount,
         WORD *                     pSessionPause,
         WORD *                     pTimeRemaining );

      // Return information about the pending network initiated alert
      eGobiError OMADMGetPendingNIA( 
         ULONG *                    pSessionType,
         USHORT *                   pSessionID );

      // Send the specified OMA-DM selection for the current network 
      // initiated session
      eGobiError OMADMSendSelection( 
         ULONG                      selection,
         USHORT                     sessionID );

      // Return the OMA-DM feature settings
      eGobiError OMADMGetFeatureSettings( 
         ULONG *                    pbProvisioning,
         ULONG *                    pbPRLUpdate );

      // Set the OMA-DM device provisioning service update feature setting
      eGobiError OMADMSetProvisioningFeature( ULONG bProvisioning );

      // Set the OMA-DM PRL service update feature setting
      eGobiError OMADMSetPRLUpdateFeature( ULONG bPRLUpdate );
#endif

#ifdef VOICE_SUPPORT
      // Initiates a USSD operation
      eGobiError OriginateUSSD( BYTE * pInfo );

      // Respond to a USSD request from the network
      eGobiError AnswerUSSD( BYTE * pInfo );

      // Cancels an in-progress USSD operation
      eGobiError CancelUSSD();
#endif

#ifdef IMG_SUPPORT
      // Get the current image preference list
      eGobiError GetImagesPreference( 
         ULONG *                    pImageListSize, 
         BYTE *                     pImageList );

      // Set the current image preference list
      eGobiError SetImagesPreference( 
         ULONG                      imageListSize, 
         BYTE *                     pImageList,
         ULONG                      bForceDownload,
         BYTE                       modemIndex,
         ULONG *                    pImageTypesSize, 
         BYTE *                     pImageTypes );

      // Return the boot and recovery image download mode
      eGobiError GetBARMode( ULONG * pBARMode );

      // Request the device enter boot and recovery image download mode
      eGobiError SetBARMode();

      // Get the list of images stored on the device
      eGobiError GetStoredImages( 
         ULONG *                    pImageListSize, 
         BYTE *                     pImageList );

      // Return info about the specified image from the device
      eGobiError GetStoredImageInfo( 
         ULONG                      imageInfoSize, 
         BYTE *                     pImageInfo,
         ULONG *                    pMajorVersion, 
         ULONG *                    pMinorVersion,
         ULONG *                    pVersionID,
         CHAR *                     pInfo,
         ULONG *                    pLockID );

      // Delete the specified image from the device
      eGobiError DeleteStoredImage( 
         ULONG                      imageInfoSize, 
         BYTE *                     pImageInfo );
#endif

#ifdef IMG2K_SUPPORT
      eGobiError GetFirmwareInfo( 
         ULONG *                    pFirmwareID,
         ULONG *                    pTechnology,
         ULONG *                    pCarrier,
         ULONG *                    pRegion,
         ULONG *                    pGPSCapability );

      // Upgrade firmware
      eGobiError UpgradeFirmware( CHAR * pDestinationPath );

      // Return image information
      eGobiError GetImageInfo( 
         CHAR *                     pPath, 
         ULONG *                    pFirmwareID,
         ULONG *                    pTechnology,
         ULONG *                    pCarrier,
         ULONG *                    pRegion,
         ULONG *                    pGPSCapability );

      // Return the image store folder
      eGobiError GetImageStore( 
         WORD                       pathSize,
         CHAR *                     pImageStorePath );
#endif

   protected:
      /* Database used for packing/parsing QMI protocol entities */
      cCoreDatabase mDB;

      /* Service type/service is required for object operation */
      typedef std::pair <eQMIService, bool> tServerConfig;

      /* Servers object is configured to support */
      std::set <tServerConfig> mServerConfig;

      /* QMI protocol servers */
      std::map <eQMIService, cQMIProtocolServer *> mServers;

      /* Fail connect attempts when multiple devices are present? */
      bool mbFailOnMultipleDevices;

      /* Device node that this object is currently connected to */
      std::string mDeviceNode;

      /* Device key string of connected device (may be empty) */
      std::string mDeviceKey;

      /* Last error recorded */
      eGobiError mLastError;

      /* Outstanding requests */
      typedef std::pair <eQMIService, ULONG> tServiceRequest;
      cSyncQueue <tServiceRequest> mRequests;

      /* Last recorded QMI_WDS_START_NET transaction ID */
      WORD mLastNetStartID;
};
