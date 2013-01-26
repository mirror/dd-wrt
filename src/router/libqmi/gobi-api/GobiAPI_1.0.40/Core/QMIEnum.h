/*===========================================================================
FILE:
   QMIEnum.h

DESCRIPTION:
   QMI protocol enumerations and related methods

PUBLIC ENUMERATIONS AND METHODS:
   eQMIService
   eQMIMessageCTL
   eQMIMessageWDS
   eQMIMessageDMS
   eQMIMessageNAS
   eQMIMessageWMS
   eQMIMessagePDS
   eQMIMessageAUTH
   eQMIMessageCAT
   eQMIMessageRMS
   eQMIMessageOMA
   eQMIResultCode
   eQMIErrorCode
   eQMICallEndReason

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

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Invalid QMI transaction ID
const ULONG INVALID_QMI_TRANSACTION_ID = 0;

// QMI DMS PRL size constants
const ULONG QMI_DMS_MAX_PRL_SIZE = 16384;
const ULONG QMI_DMS_MAX_PRL_BLOCK = 256;

/*=========================================================================*/
// eQMIService Enumeration
//    QMI Service Type Enumeration
/*=========================================================================*/
enum eQMIService
{
   eQMI_SVC_ENUM_BEGIN = -1, 

   eQMI_SVC_CONTROL,          // 000 Control service
   eQMI_SVC_WDS,              // 001 Wireless data service
   eQMI_SVC_DMS,              // 002 Device management service
   eQMI_SVC_NAS,              // 003 Network access service
   eQMI_SVC_QOS,              // 004 Quality of service, err, service 
   eQMI_SVC_WMS,              // 005 Wireless messaging service
   eQMI_SVC_PDS,              // 006 Position determination service
   eQMI_SVC_AUTH,             // 007 Authentication service

   eQMI_SVC_VOICE = 9,        // 009 Voice service

   eQMI_SVC_CAT = 224,        // 224 Card application toolkit service
   eQMI_SVC_RMS,              // 225 Remote management service
   eQMI_SVC_OMA,              // 226 Open mobile alliance dev mgmt service

   eQMI_SVC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIService validity check

PARAMETERS:
   svc         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIService svc )
{
   bool retVal = false;
   if ( (svc > eQMI_SVC_ENUM_BEGIN && svc <= eQMI_SVC_AUTH)
   ||   (svc == eQMI_SVC_VOICE)
   ||   (svc >= eQMI_SVC_CAT && svc < eQMI_SVC_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageCTL Enumeration
//    QMI Control Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageCTL
{
   eQMI_CTL_ENUM_BEGIN = -1, 

   eQMI_CTL_SET_INSTANCE_ID = 32,   // 32 Set the unique link instance ID
   eQMI_CTL_GET_VERSION_INFO,       // 33 Get supported service version info
   eQMI_CTL_GET_CLIENT_ID,          // 34 Get a unique client ID 
   eQMI_CTL_RELEASE_CLIENT_ID,      // 35 Release the unique client ID 
   eQMI_CTL_REVOKE_CLIENT_ID_IND,   // 36 Indication of client ID revocation
   eQMI_CTL_INVALID_CLIENT_ID,      // 37 Indication of invalid client ID
   eQMI_CTL_SET_DATA_FORMAT,        // 38 Set host driver data format 
   eQMI_CTL_SYNC,                   // 39 Synchronize client/server
   eQMI_CTL_SYNC_IND = 39,          // 39 Synchronize indication
   eQMI_CTL_SET_EVENT,              // 40 Set event report conditions
   eQMI_CTL_EVENT_IND = 40,         // 40 Event report indication
   eQMI_CTL_SET_POWER_SAVE_CFG,     // 41 Set power save config
   eQMI_CTL_SET_POWER_SAVE_MODE,    // 42 Set power save mode
   eQMI_CTL_GET_POWER_SAVE_MODE,    // 43 Get power save mode

   eQMI_CTL_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageCTL validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageCTL msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_CTL_SET_INSTANCE_ID && msgID < eQMI_CTL_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageWDS Enumeration
//    QMI WDS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageWDS
{
   eQMI_WDS_ENUM_BEGIN = -1, 

   eQMI_WDS_RESET,                // 00 Reset WDS service state variables
   eQMI_WDS_SET_EVENT,            // 01 Set connection state report conditions
   eQMI_WDS_EVENT_IND = 1,        // 01 Connection state report indication
   eQMI_WDS_ABORT,                // 02 Abort previously issued WDS command

   eQMI_WDS_START_NET = 32,       // 32 Start WDS network interface
   eQMI_WDS_STOP_NET,             // 33 Stop WDS network interface
   eQMI_WDS_GET_PKT_STATUS,       // 34 Get packet data connection status
   eQMI_WDS_PKT_STATUS_IND = 34,  // 34 Packet data connection status indication
   eQMI_WDS_GET_RATES,            // 35 Get current bit rates of the connection
   eQMI_WDS_GET_STATISTICS,       // 36 Get the packet data transfer statistics
   eQMI_WDS_G0_DORMANT,           // 37 Go dormant
   eQMI_WDS_G0_ACTIVE,            // 38 Go active
   eQMI_WDS_CREATE_PROFILE,       // 39 Create profile with specified settings
   eQMI_WDS_MODIFY_PROFILE,       // 40 Modify profile with specified settings
   eQMI_WDS_DELETE_PROFILE,       // 41 Delete the specified profile 
   eQMI_WDS_GET_PROFILE_LIST,     // 42 Get all profiles
   eQMI_WDS_GET_PROFILE,          // 43 Get the specified profile
   eQMI_WDS_GET_DEFAULTS,         // 44 Get the default data session settings 
   eQMI_WDS_GET_SETTINGS,         // 45 Get the runtime data session settings 
   eQMI_WDS_SET_MIP,              // 46 Get the mobile IP setting 
   eQMI_WDS_GET_MIP,              // 47 Set the mobile IP setting 
   eQMI_WDS_GET_DORMANCY,         // 48 Get the dormancy status

   eQMI_WDS_GET_AUTOCONNECT = 52, // 52 Get the NDIS autoconnect setting
   eQMI_WDS_GET_DURATION,         // 53 Get the duration of data session
   eQMI_WDS_GET_MODEM_STATUS,     // 54 Get the modem status
   eQMI_WDS_MODEM_IND = 54,       // 54 Modem status indication
   eQMI_WDS_GET_DATA_BEARER,      // 55 Get the data bearer type
   eQMI_WDS_GET_MODEM_INFO,       // 56 Get the modem info
   eQMI_WDS_MODEM_INFO_IND = 56,  // 56 Modem info indication

   eQMI_WDS_GET_ACTIVE_MIP = 60,  // 60 Get the active mobile IP profile
   eQMI_WDS_SET_ACTIVE_MIP,       // 61 Set the active mobile IP profile
   eQMI_WDS_GET_MIP_PROFILE,      // 62 Get mobile IP profile settings
   eQMI_WDS_SET_MIP_PROFILE,      // 63 Set mobile IP profile settings
   eQMI_WDS_GET_MIP_PARAMS,       // 64 Get mobile IP parameters
   eQMI_WDS_SET_MIP_PARAMS,       // 65 Set mobile IP parameters
   eQMI_WDS_GET_LAST_MIP_STATUS,  // 66 Get last mobile IP status
   eQMI_WDS_GET_AAA_AUTH_STATUS,  // 67 Get AN-AAA authentication status
   eQMI_WDS_GET_CUR_DATA_BEARER,  // 68 Get current data bearer
   eQMI_WDS_GET_CALL_LIST,        // 69 Get the call history list
   eQMI_WDS_GET_CALL_ENTRY,       // 70 Get an entry from the call history list
   eQMI_WDS_CLEAR_CALL_LIST,      // 71 Clear the call history list
   eQMI_WDS_GET_CALL_LIST_MAX,    // 72 Get maximum size of call history list

   eQMI_WDS_SET_IP_FAMILY = 77,   // 77 Set the client IP family preference

   eQMI_WDS_SET_AUTOCONNECT = 81, // 81 Set the NDIS autoconnect setting
   eQMI_WDS_GET_DNS,              // 82 Get the DNS setting
   eQMI_WDS_SET_DNS,              // 83 Set the DNS setting

   eQMI_WDS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageWDS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageWDS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_WDS_ENUM_BEGIN && msgID <= eQMI_WDS_ABORT)
   ||   (msgID >= eQMI_WDS_START_NET && msgID <= eQMI_WDS_GET_DORMANCY)
   ||   (msgID >= eQMI_WDS_GET_AUTOCONNECT && msgID <= eQMI_WDS_MODEM_INFO_IND)
   ||   (msgID >= eQMI_WDS_GET_ACTIVE_MIP && msgID <= eQMI_WDS_GET_CALL_LIST_MAX)
   ||   (msgID == eQMI_WDS_SET_IP_FAMILY)
   ||   (msgID >= eQMI_WDS_SET_AUTOCONNECT && msgID < eQMI_WDS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageDMS Enumeration
//    QMI DMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageDMS
{
   eQMI_DMS_ENUM_BEGIN = -1, 

   eQMI_DMS_RESET,               // 00 Reset DMS service state variables
   eQMI_DMS_SET_EVENT,           // 01 Set connection state report conditions
   eQMI_DMS_EVENT_IND = 1,       // 01 Connection state report indication

   eQMI_DMS_GET_CAPS = 32,       // 32 Get the device capabilities
   eQMI_DMS_GET_MANUFACTURER,    // 33 Get the device manfacturer
   eQMI_DMS_GET_MODEL_ID,        // 34 Get the device model ID
   eQMI_DMS_GET_REV_ID,          // 35 Get the device revision ID
   eQMI_DMS_GET_NUMBER,          // 36 Get the assigned voice number
   eQMI_DMS_GET_IDS,             // 37 Get the ESN/IMEI/MEID
   eQMI_DMS_GET_POWER_STATE,     // 38 Get the get power state
   eQMI_DMS_UIM_SET_PIN_PROT,    // 39 UIM - Set PIN protection
   eQMI_DMS_UIM_PIN_VERIFY,      // 40 UIM - Verify PIN 
   eQMI_DMS_UIM_PIN_UNBLOCK,     // 41 UIM - Unblock PIN
   eQMI_DMS_UIM_PIN_CHANGE,      // 42 UIM - Change PIN
   eQMI_DMS_UIM_GET_PIN_STATUS,  // 43 UIM - Get PIN status
   eQMI_DMS_GET_MSM_ID = 44,     // 44 Get MSM ID
   eQMI_DMS_GET_OPERTAING_MODE,  // 45 Get the operating mode
   eQMI_DMS_SET_OPERATING_MODE,  // 46 Set the operating mode
   eQMI_DMS_GET_TIME,            // 47 Get timestamp from the device
   eQMI_DMS_GET_PRL_VERSION,     // 48 Get the PRL version
   eQMI_DMS_GET_ACTIVATED_STATE, // 49 Get the activation state 
   eQMI_DMS_ACTIVATE_AUTOMATIC,  // 50 Perform an automatic activation
   eQMI_DMS_ACTIVATE_MANUAL,     // 51 Perform a manual activation
   eQMI_DMS_GET_USER_LOCK_STATE, // 52 Get the lock state
   eQMI_DMS_SET_USER_LOCK_STATE, // 53 Set the lock state
   eQMI_DMS_SET_USER_LOCK_CODE,  // 54 Set the lock PIN
   eQMI_DMS_READ_USER_DATA,      // 55 Read user data
   eQMI_DMS_WRITE_USER_DATA,     // 56 Write user data
   eQMI_DMS_READ_ERI_FILE,       // 57 Read the enhanced roaming indicator file
   eQMI_DMS_FACTORY_DEFAULTS,    // 58 Reset to factory defaults
   eQMI_DMS_VALIDATE_SPC,        // 59 Validate service programming code
   eQMI_DMS_UIM_GET_ICCID,       // 60 Get UIM ICCID
   eQMI_DMS_GET_FIRWARE_ID,      // 61 Get firmware ID
   eQMI_DMS_SET_FIRMWARE_ID,     // 62 Set firmware ID
   eQMI_DMS_GET_HOST_LOCK_ID,    // 63 Get host lock ID
   eQMI_DMS_UIM_GET_CK_STATUS,   // 64 UIM - Get control key status
   eQMI_DMS_UIM_SET_CK_PROT,     // 65 UIM - Set control key protection
   eQMI_DMS_UIM_UNBLOCK_CK,      // 66 UIM - Unblock facility control key
   eQMI_DMS_GET_IMSI,            // 67 Get the IMSI
   eQMI_DMS_UIM_GET_STATE,       // 68 UIM - Get the UIM state
   eQMI_DMS_GET_BAND_CAPS,       // 69 Get the device band capabilities
   eQMI_DMS_GET_FACTORY_ID,      // 70 Get the device factory ID
   eQMI_DMS_GET_FIRMWARE_PREF,   // 71 Get firmware preference 
   eQMI_DMS_SET_FIRMWARE_PREF,   // 72 Set firmware preference 
   eQMI_DMS_LIST_FIRMWARE,       // 73 List all stored firmware
   eQMI_DMS_DELETE_FIRMWARE,     // 74 Delete specified stored firmware
   eQMI_DMS_SET_TIME,            // 75 Set device time
   eQMI_DMS_GET_FIRMWARE_INFO,   // 76 Get stored firmware info
   eQMI_DMS_GET_ALT_NET_CFG,     // 77 Get alternate network config
   eQMI_DMS_SET_ALT_NET_CFG,     // 78 Set alternate network config
   eQMI_DMS_GET_IMG_DLOAD_MODE,  // 79 Get next image download mode
   eQMI_DMS_SET_IMG_DLOAD_MODE,  // 80 Set next image download mod

   eQMI_DMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageDMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageDMS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_DMS_ENUM_BEGIN && msgID <= eQMI_DMS_EVENT_IND)
   ||   (msgID >= eQMI_DMS_GET_CAPS && msgID < eQMI_DMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageNAS Enumeration
//    QMI NAS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageNAS
{
   eQMI_NAS_ENUM_BEGIN = -1, 

   eQMI_NAS_RESET,               // 00 Reset NAS service state variables
   eQMI_NAS_ABORT,               // 01 Abort previously issued NAS command
   eQMI_NAS_SET_EVENT,           // 02 Set NAS state report conditions
   eQMI_NAS_EVENT_IND = 2,       // 02 Connection state report indication
   eQMI_NAS_SET_REG_EVENT,       // 03 Set NAS registration report conditions

   eQMI_NAS_GET_RSSI = 32,       // 32 Get the signal strength
   eQMI_NAS_SCAN_NETS,           // 33 Scan for visible network
   eQMI_NAS_REGISTER_NET,        // 34 Initiate a network registration
   eQMI_NAS_ATTACH_DETACH,       // 35 Initiate an attach or detach action
   eQMI_NAS_GET_SS_INFO,         // 36 Get info about current serving system
   eQMI_NAS_SS_INFO_IND = 36,    // 36 Current serving system info indication
   eQMI_NAS_GET_HOME_INFO,       // 37 Get info about home network
   eQMI_NAS_GET_NET_PREF_LIST,   // 38 Get the list of preferred networks
   eQMI_NAS_SET_NET_PREF_LIST,   // 39 Set the list of preferred networks
   eQMI_NAS_GET_NET_BAN_LIST,    // 40 Get the list of forbidden networks
   eQMI_NAS_SET_NET_BAN_LIST,    // 41 Set the list of forbidden networks
   eQMI_NAS_SET_TECH_PREF,       // 42 Set the technology preference
   eQMI_NAS_GET_TECH_PREF,       // 43 Get the technology preference
   eQMI_NAS_GET_ACCOLC,          // 44 Get the Access Overload Class
   eQMI_NAS_SET_ACCOLC,          // 45 Set the Access Overload Class 
   eQMI_NAS_GET_SYSPREF,         // 46 Get the CDMA system preference 
   eQMI_NAS_GET_NET_PARAMS,      // 47 Get various network parameters 
   eQMI_NAS_SET_NET_PARAMS,      // 48 Set various network parameters 
   eQMI_NAS_GET_RF_INFO,         // 49 Get the SS radio/band channel info
   eQMI_NAS_GET_AAA_AUTH_STATUS, // 50 Get AN-AAA authentication status
   eQMI_NAS_SET_SYS_SELECT_PREF, // 51 Set system selection preference
   eQMI_NAS_GET_SYS_SELECT_PREF, // 52 Get system selection preference
   eQMI_NAS_SYS_SELECT_IND = 52, // 52 System selection pref indication

   eQMI_NAS_SET_DDTM_PREF = 55,  // 55 Set DDTM preference
   eQMI_NAS_GET_DDTM_PREF,       // 56 Get DDTM preference
   eQMI_NAS_DDTM_IND = 56,       // 56 DDTM preference indication

   eQMI_NAS_GET_PLMN_MODE = 59,  // 59 Get PLMN mode bit from CSP
   eQMI_NAS_PLMN_MODE_IND,       // 60 CSP PLMN mode bit indication

   eQMI_NAS_GET_PLMN_NAME = 68,  // 68 Get operator name for specified network

   eQMI_NAS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageNAS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageNAS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_NAS_ENUM_BEGIN && msgID <= eQMI_NAS_SET_REG_EVENT)
   ||   (msgID >= eQMI_NAS_GET_RSSI && msgID <= eQMI_NAS_SYS_SELECT_IND)
   ||   (msgID >= eQMI_NAS_SET_DDTM_PREF && msgID <= eQMI_NAS_DDTM_IND)
   ||   (msgID >= eQMI_NAS_GET_PLMN_MODE && msgID <= eQMI_NAS_PLMN_MODE_IND)
   ||   (msgID >= eQMI_NAS_GET_PLMN_NAME && msgID < eQMI_NAS_ENUM_END) )
   {
      retVal = true;
   }
   return retVal;
};

/*=========================================================================*/
// eQMIMessageWMS Enumeration
//    QMI WMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageWMS
{
   eQMI_WMS_ENUM_BEGIN = -1, 

   eQMI_WMS_RESET,                  // 00 Reset WMS service state variables
   eQMI_WMS_SET_EVENT,              // 01 Set new message report conditions
   eQMI_WMS_EVENT_IND = 1,          // 01 New message report indication

   eQMI_WMS_RAW_SEND = 32,          // 32 Send a raw message
   eQMI_WMS_RAW_WRITE,              // 33 Write a raw message to the device
   eQMI_WMS_RAW_READ,               // 34 Read a raw message from the device
   eQMI_WMS_MODIFY_TAG,             // 35 Modify message tag on the device
   eQMI_WMS_DELETE,                 // 36 Delete message by index/tag/memory

   eQMI_WMS_GET_MSG_PROTOCOL = 48,  // 48 Get the current message protocol
   eQMI_WMS_GET_MSG_LIST,           // 49 Get list of messages from the device
   eQMI_WMS_SET_ROUTES,             // 50 Set routes for message memory storage
   eQMI_WMS_GET_ROUTES,             // 51 Get routes for message memory storage
   eQMI_WMS_GET_SMSC_ADDR,          // 52 Get SMSC address
   eQMI_WMS_SET_SMSC_ADDR,          // 53 Set SMSC address
   eQMI_WMS_GET_MSG_LIST_MAX,       // 54 Get maximum size of SMS storage
   eQMI_WMS_SEND_ACK,               // 55 Send ACK
   eQMI_WMS_SET_RETRY_PERIOD,       // 56 Set retry period
   eQMI_WMS_SET_RETRY_INTERVAL,     // 57 Set retry interval
   eQMI_WMS_SET_DC_DISCO_TIMER,     // 58 Set DC auto-disconnect timer
   eQMI_WMS_SET_MEMORY_STATUS,      // 59 Set memory storage status
   eQMI_WMS_SET_BC_ACTIVATION,      // 60 Set broadcast activation
   eQMI_WMS_SET_BC_CONFIG,          // 61 Set broadcast config
   eQMI_WMS_GET_BC_CONFIG,          // 62 Get broadcast config
   eQMI_WMS_MEMORY_FULL_IND,        // 63 Memory full indication
   eQMI_WMS_GET_DOMAIN_PREF,        // 64 Get domain preference
   eQMI_WMS_SET_DOMAIN_PREF,        // 65 Set domain preference
   eQMI_WMS_MEMORY_SEND,            // 66 Send message from memory store
   eQMI_WMS_GET_MSG_WAITING,        // 67 Get message waiting info
   eQMI_WMS_MSG_WAITING_IND,        // 68 Message waiting indication
   eQMI_WMS_SET_PRIMARY_CLIENT,     // 69 Set client as primary client
   eQMI_WMS_SMSC_ADDR_IND,          // 70 SMSC address indication

   eQMI_WMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageWMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageWMS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_WMS_ENUM_BEGIN && msgID <= eQMI_WMS_EVENT_IND)
   ||   (msgID >= eQMI_WMS_RAW_SEND && msgID <= eQMI_WMS_DELETE)
   ||   (msgID >= eQMI_WMS_GET_MSG_PROTOCOL && msgID < eQMI_WMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessagePDS Enumeration
//    QMI PDS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessagePDS
{
   eQMI_PDS_ENUM_BEGIN = -1, 

   eQMI_PDS_RESET,                // 00 Reset PDS service state variables
   eQMI_PDS_SET_EVENT,            // 01 Set PDS report conditions
   eQMI_PDS_EVENT_IND = 1,        // 01 PDS report indication

   eQMI_PDS_GET_STATE = 32,       // 32 Return PDS service state
   eQMI_PDS_STATE_IND = 32,       // 32 PDS service state indication
   eQMI_PDS_SET_STATE,            // 33 Set PDS service state
   eQMI_PDS_START_SESSION,        // 34 Start a PDS tracking session
   eQMI_PDS_GET_SESSION_INFO,     // 35 Get PDS tracking session info
   eQMI_PDS_FIX_POSITION,         // 36 Manual tracking session position
   eQMI_PDS_END_SESSION,          // 37 End a PDS tracking session
   eQMI_PDS_GET_NMEA_CFG,         // 38 Get NMEA sentence config
   eQMI_PDS_SET_NMEA_CFG,         // 39 Set NMEA sentence config
   eQMI_PDS_INJECT_TIME,          // 40 Inject a time reference
   eQMI_PDS_GET_DEFAULTS,         // 41 Get default tracking session config
   eQMI_PDS_SET_DEFAULTS,         // 42 Set default tracking session config
   eQMI_PDS_GET_XTRA_PARAMS,      // 43 Get the GPS XTRA parameters 
   eQMI_PDS_SET_XTRA_PARAMS,      // 44 Set the GPS XTRA parameters 
   eQMI_PDS_FORCE_XTRA_DL,        // 45 Force a GPS XTRA database download
   eQMI_PDS_GET_AGPS_CONFIG,      // 46 Get the AGPS mode configuration
   eQMI_PDS_SET_AGPS_CONFIG,      // 47 Set the AGPS mode configuration

   eQMI_PDS_GET_SVC_AUTOTRACK,    // 48 Get the service auto-tracking state
   eQMI_PDS_SET_SVC_AUTOTRACK,    // 49 Set the service auto-tracking state
   eQMI_PDS_GET_COM_AUTOTRACK,    // 50 Get COM port auto-tracking config
   eQMI_PDS_SET_COM_AUTOTRACK,    // 51 Set COM port auto-tracking config
   eQMI_PDS_RESET_DATA,           // 52 Reset PDS service data
   eQMI_PDS_SINGLE_FIX,           // 53 Request single position fix
   eQMI_PDS_GET_VERSION,          // 54 Get PDS service version
   eQMI_PDS_INJECT_XTRA,          // 55 Inject XTRA data
   eQMI_PDS_INJECT_POSITION,      // 56 Inject position data
   eQMI_PDS_INJECT_WIFI,          // 57 Inject Wi-Fi obtained data
   eQMI_PDS_GET_SBAS_CONFIG,      // 58 Get SBAS config
   eQMI_PDS_SET_SBAS_CONFIG,      // 59 Set SBAS config
   eQMI_PDS_SEND_NI_RESPONSE,     // 60 Send network initiated response
   eQMI_PDS_INJECT_ABS_TIME,      // 61 Inject absolute time
   eQMI_PDS_INJECT_EFS,           // 62 Inject EFS data
   eQMI_PDS_GET_DPO_CONFIG,       // 63 Get DPO config
   eQMI_PDS_SET_DPO_CONFIG,       // 64 Set DPO config
   eQMI_PDS_GET_ODP_CONFIG,       // 65 Get ODP config
   eQMI_PDS_SET_ODP_CONFIG,       // 66 Set ODP config
   eQMI_PDS_CANCEL_SINGLE_FIX,    // 67 Cancel single position fix
   eQMI_PDS_GET_GPS_STATE,        // 68 Get GPS state

   eQMI_PDS_GET_METHODS = 80,     // 80 Get GPS position methods state
   eQMI_PDS_SET_METHODS,          // 81 Set GPS position methods state

   eQMI_PDS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessagePDS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessagePDS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_PDS_ENUM_BEGIN && msgID <= eQMI_PDS_EVENT_IND)
   ||   (msgID >= eQMI_PDS_GET_STATE && msgID <= eQMI_PDS_GET_GPS_STATE)
   ||   (msgID >= eQMI_PDS_GET_METHODS && msgID < eQMI_PDS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageAUTH Enumeration
//    QMI Authentication Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageAUTH
{
   eQMI_AUTH_ENUM_BEGIN = -1, 

   eQMI_AUTH_START_EAP = 32,        // 32 Start the EAP session
   eQMI_AUTH_SEND_EAP,              // 33 Send and receive EAP packets
   eQMI_AUTH_EAP_RESULT_IND,        // 34 EAP session result indication
   eQMI_AUTH_GET_EAP_KEYS,          // 35 Get the EAP session keys
   eQMI_AUTH_END_EAP,               // 36 End the EAP session

   eQMI_AUTH_ENUM_END
};

/*=========================================================================*/
// eQMIMessageVoice Enumeration
//    QMI Voice Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageVoice
{
   eQMI_VOICE_ENUM_BEGIN = -1, 

   eQMI_VOICE_INDICATION_REG = 3,    // 03 Set indication registration state

   eQMI_VOICE_CALL_ORIGINATE = 32,   // 32 Originate a voice call
   eQMI_VOICE_CALL_END,              // 33 End a voice call
   eQMI_VOICE_CALL_ANSWER,           // 34 Answer incoming voice call

   eQMI_VOICE_GET_CALL_INFO = 36,    // 36 Get call information
   eQMI_VOICE_OTASP_STATUS_IND,      // 37 OTASP/OTAPA event indication
   eQMI_VOICE_INFO_REC_IND,          // 38 New info record indication
   eQMI_VOICE_SEND_FLASH,            // 39 Send a simple flash
   eQMI_VOICE_BURST_DTMF,            // 40 Send a burst DTMF
   eQMI_VOICE_START_CONT_DTMF,       // 41 Starts a continuous DTMF
   eQMI_VOICE_STOP_CONT_DTMF,        // 42 Stops a continuous DTMF
   eQMI_VOICE_DTMF_IND,              // 43 DTMF event indication
   eQMI_VOICE_SET_PRIVACY_PREF,      // 44 Set privacy preference
   eQMI_VOICE_PRIVACY_IND,           // 45 Privacy change indication
   eQMI_VOICE_ALL_STATUS_IND,        // 46 Voice all call status indication
   eQMI_VOICE_GET_ALL_STATUS,        // 47 Get voice all call status

   eQMI_VOICE_MANAGE_CALLS = 49,     // 49 Manage calls
   eQMI_VOICE_SUPS_NOTIFICATION_IND, // 50 Supplementary service notifications
   eQMI_VOICE_SET_SUPS_SERVICE,      // 51 Manage supplementary service
   eQMI_VOICE_GET_CALL_WAITING,      // 52 Query sup service call waiting
   eQMI_VOICE_GET_CALL_BARRING,      // 53 Query sup service call barring
   eQMI_VOICE_GET_CLIP,              // 54 Query sup service CLIP
   eQMI_VOICE_GET_CLIR,              // 55 Query sup service CLIR
   eQMI_VOICE_GET_CALL_FWDING,       // 56 Query sup service call forwarding
   eQMI_VOICE_SET_CALL_BARRING_PWD,  // 57 Set call barring password
   eQMI_VOICE_ORIG_USSD,             // 58 Initiate USSD operation
   eQMI_VOICE_ANSWER_USSD,           // 59 Answer USSD request
   eQMI_VOICE_CANCEL_USSD,           // 60 Cancel USSD operation
   eQMI_VOICE_USSD_RELEASE_IND,      // 61 USSD release indication
   eQMI_VOICE_USSD_IND,              // 62 USSD request/notification indication
   eQMI_VOICE_UUS_IND,               // 63 UUS information indication
   eQMI_VOICE_SET_CONFIG,            // 64 Set config
   eQMI_VOICE_GET_CONFIG,            // 65 Get config
   eQMI_VOICE_SUPS_IND,              // 66 Sup service request indication
   eQMI_VOICE_ASYNC_ORIG_USSD,       // 67 Initiate USSD operation
   eQMI_VOICE_ASYNC_USSD_IND = 67,   // 67 USSD request/notification indication

   eQMI_VOICE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageVoice validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageVoice msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_VOICE_INDICATION_REG)
   ||   (msgID >= eQMI_VOICE_CALL_ORIGINATE && msgID <= eQMI_VOICE_CALL_ANSWER)
   ||   (msgID >= eQMI_VOICE_GET_CALL_INFO && msgID <= eQMI_VOICE_GET_ALL_STATUS)
   ||   (msgID >= eQMI_VOICE_MANAGE_CALLS && msgID < eQMI_VOICE_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageAUTH validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageAUTH msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_AUTH_START_EAP && msgID < eQMI_AUTH_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageCAT Enumeration
//    QMI CAT Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageCAT
{
   eQMI_CAT_ENUM_BEGIN = -1, 

   eQMI_CAT_RESET,                  // 00 Reset CAT service state variables
   eQMI_CAT_SET_EVENT,              // 01 Set new message report conditions
   eQMI_CAT_EVENT_IND = 1,          // 01 New message report indication

   eQMI_CAT_GET_STATE = 32,         // 32 Get service state information
   eQMI_CAT_SEND_TERMINAL,          // 33 Send a terminal response
   eQMI_CAT_SEND_ENVELOPE,          // 34 Send an envelope command

   eQMI_CAT_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageCAT validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageCAT msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_CAT_ENUM_BEGIN && msgID <= eQMI_CAT_EVENT_IND)
   ||   (msgID >= eQMI_CAT_GET_STATE && msgID < eQMI_CAT_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageRMS Enumeration
//    QMI RMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageRMS
{
   eQMI_RMS_ENUM_BEGIN = -1, 

   eQMI_RMS_RESET,                  // 00 Reset RMS service state variables

   eQMI_RMS_GET_SMS_WAKE = 32,      // 32 Get SMS wake settings
   eQMI_RMS_SET_SMS_WAKE,           // 33 Set SMS wake settings

   eQMI_RMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageRMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageRMS msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_RMS_RESET)
   ||   (msgID >= eQMI_RMS_GET_SMS_WAKE && msgID < eQMI_RMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageOMA Enumeration
//    QMI OMA-DM Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageOMA
{
   eQMI_OMA_ENUM_BEGIN = -1, 

   eQMI_OMA_RESET,                  // 00 Reset OMA service state variables
   eQMI_OMA_SET_EVENT,              // 01 Set OMA report conditions
   eQMI_OMA_EVENT_IND = 1,          // 01 OMA report indication

   eQMI_OMA_START_SESSION = 32,     // 32 Start client inititated session
   eQMI_OMA_CANCEL_SESSION,         // 33 Cancel session
   eQMI_OMA_GET_SESSION_INFO,       // 34 Get session information
   eQMI_OMA_SEND_SELECTION,         // 35 Send selection for net inititated msg
   eQMI_OMA_GET_FEATURES,           // 36 Get feature settings
   eQMI_OMA_SET_FEATURES,           // 37 Set feature settings

   eQMI_OMA_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageOMA validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageOMA msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_OMA_ENUM_BEGIN && msgID <= eQMI_OMA_EVENT_IND)
   ||   (msgID >= eQMI_OMA_START_SESSION && msgID < eQMI_OMA_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIResultCode Enumeration
//    QMI Result Code Enumeration
/*=========================================================================*/
enum eQMIResultCode
{
   eQMI_RC_ENUM_BEGIN = -1, 

   eQMI_RC_SUCCESS,           // 00 Success
   eQMI_RC_ERROR,             // 01 Error

   eQMI_RC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIResultCode validity check

PARAMETERS:
   rc          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIResultCode rc )
{
   bool retVal = false;
   if (rc > eQMI_RC_ENUM_BEGIN && rc < eQMI_RC_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIErrorCode Enumeration
//    QMI Error Code Enumeration
/*=========================================================================*/
enum eQMIErrorCode
{
   eQMI_ERR_ENUM_BEGIN = -1, 

   eQMI_ERR_NONE,                            // 00
   eQMI_ERR_MALFORMED_MSG,                   // 01
   eQMI_ERR_NO_MEMORY,                       // 02
   eQMI_ERR_INTERNAL,                        // 03    
   eQMI_ERR_ABORTED,                         // 04
   eQMI_ERR_CLIENT_IDS_EXHAUSTED,            // 05
   eQMI_ERR_UNABORTABLE_TRANSACTION,         // 06
   eQMI_ERR_INVALID_CLIENT_ID,               // 07
   eQMI_ERR_NO_THRESHOLDS,                   // 08
   eQMI_ERR_INVALID_HANDLE,                  // 09
   eQMI_ERR_INVALID_PROFILE,                 // 10
   eQMI_ERR_INVALID_PIN_ID,                  // 11
   eQMI_ERR_INCORRECT_PIN,                   // 12
   eQMI_ERR_NO_NETWORK_FOUND,                // 13
   eQMI_ERR_CALL_FAILED,                     // 14
   eQMI_ERR_OUT_OF_CALL,                     // 15
   eQMI_ERR_NOT_PROVISIONED,                 // 16
   eQMI_ERR_MISSING_ARG,                     // 17
   eQMI_ERR_18,                              // 18
   eQMI_ERR_ARG_TOO_LONG,                    // 19
   eQMI_ERR_20,                              // 20
   eQMI_ERR_21,                              // 21
   eQMI_ERR_INVALID_TX_ID,                   // 22
   eQMI_ERR_DEVICE_IN_USE,                   // 23
   eQMI_ERR_OP_NETWORK_UNSUPPORTED,          // 24
   eQMI_ERR_OP_DEVICE_UNSUPPORTED,           // 25
   eQMI_ERR_NO_EFFECT,                       // 26
   eQMI_ERR_NO_FREE_PROFILE,                 // 27
   eQMI_ERR_INVALID_PDP_TYPE,                // 28
   eQMI_ERR_INVALID_TECH_PREF,               // 29
   eQMI_ERR_INVALID_PROFILE_TYPE,            // 30
   eQMI_ERR_INVALID_SERVICE_TYPE,            // 31
   eQMI_ERR_INVALID_REGISTER_ACTION,         // 32
   eQMI_ERR_INVALID_PS_ATTACH_ACTION,        // 33
   eQMI_ERR_AUTHENTICATION_FAILED,           // 34
   eQMI_ERR_PIN_BLOCKED,                     // 35
   eQMI_ERR_PIN_ALWAYS_BLOCKED,              // 36
   eQMI_ERR_UIM_UNINITIALIZED,               // 37
   eQMI_ERR_MAX_QOS_REQUESTS_IN_USE,         // 38
   eQMI_ERR_INCORRECT_FLOW_FILTER,           // 39
   eQMI_ERR_NETWORK_QOS_UNAWARE,             // 40
   eQMI_ERR_INVALID_QOS_ID,                  // 41
   eQMI_ERR_REQUESTED_NUM_UNSUPPORTED,       // 42
   eQMI_ERR_INTERFACE_NOT_FOUND,             // 43
   eQMI_ERR_FLOW_SUSPENDED,                  // 44
   eQMI_ERR_INVALID_DATA_FORMAT,             // 45
   eQMI_ERR_GENERAL,                         // 46
   eQMI_ERR_UNKNOWN,                         // 47
   eQMI_ERR_INVALID_ARG,                     // 48
   eQMI_ERR_INVALID_INDEX,                   // 49
   eQMI_ERR_NO_ENTRY,                        // 50
   eQMI_ERR_DEVICE_STORAGE_FULL,             // 51
   eQMI_ERR_DEVICE_NOT_READY,                // 52
   eQMI_ERR_NETWORK_NOT_READY,               // 53
   eQMI_ERR_WMS_CAUSE_CODE,                  // 54
   eQMI_ERR_WMS_MESSAGE_NOT_SENT,            // 55
   eQMI_ERR_WMS_MESSAGE_DELIVERY_FAILURE,    // 56
   eQMI_ERR_WMS_INVALID_MESSAGE_ID,          // 57
   eQMI_ERR_WMS_ENCODING,                    // 58
   eQMI_ERR_AUTHENTICATION_LOCK,             // 59
   eQMI_ERR_INVALID_TRANSITION,              // 60
   eQMI_ERR_61,                              // 61
   eQMI_ERR_62,                              // 62
   eQMI_ERR_63,                              // 63
   eQMI_ERR_64,                              // 64
   eQMI_ERR_SESSION_INACTIVE,                // 65
   eQMI_ERR_SESSION_INVALID,                 // 66
   eQMI_ERR_SESSION_OWNERSHIP,               // 67
   eQMI_ERR_INSUFFICIENT_RESOURCES,          // 68
   eQMI_ERR_DISABLED,                        // 69
   eQMI_ERR_INVALID_OPERATION,               // 70
   eQMI_ERR_INVALID_QMI_CMD,                 // 71
   eQMI_ERR_WMS_TPDU_TYPE,                   // 72
   eQMI_ERR_WMS_SMSC_ADDR,                   // 73
   eQMI_ERR_INFO_UNAVAILABLE,                // 74
   eQMI_ERR_SEGMENT_TOO_LONG,                // 75
   eQMI_ERR_SEGMENT_ORDER,                   // 76
   eQMI_ERR_BUNDLING_NOT_SUPPORTED,          // 77
   eQMI_ERR_78,                              // 78
   eQMI_ERR_POLICY_MISMATCH,                 // 79
   eQMI_ERR_SIM_FILE_NOT_FOUND,              // 80
   eQMI_ERR_EXTENDED_EXTERNAL,               // 81
   eQMI_ERR_ACCESS_DENIED,                   // 82
   eQMI_ERR_HARDWARE_RESTRICTED,             // 83
   eQMI_ERR_ACK_NOT_SENT,                    // 84

   eQMI_ERR_INCOMPATIBLE_STATE = 90,         // 90
   eQMI_ERR_FDN_RESTRICT,                    // 91
   eQMI_ERR_SUPS_FAILURE_CAUSE,              // 92
   eQMI_ERR_NO_RADIO,                        // 93
   eQMI_ERR_NOT_SUPPORTED,                   // 94

   eQMI_ERR_CARD_CALL_CONTROL_FAILED = 96,   // 96
   eQMI_ERR_NETWORK_ABORTED,                 // 97

   eQMI_ERR_CAT_EVT_REG_FAILED,              // 61441
   eQMI_ERR_CAT_INVALID_TR,                  // 61442
   eQMI_ERR_CAT_INVALID_ENV_CMD,             // 61443
   eQMI_ERR_CAT_ENV_CMD_BUSY,                // 61444
   eQMI_ERR_CAT_ENV_CMD_FAIL,                // 61445

   eQMI_ERR_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIErrorCode validity check

PARAMETERS:
   ec          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIErrorCode ec )
{
   bool retVal = false;
   if ( (ec > eQMI_ERR_ENUM_BEGIN && ec <= eQMI_ERR_ACK_NOT_SENT)
   ||   (ec >= eQMI_ERR_INCOMPATIBLE_STATE && ec <= eQMI_ERR_NOT_SUPPORTED)
   ||   (ec == eQMI_ERR_CARD_CALL_CONTROL_FAILED)
   ||   (ec == eQMI_ERR_NETWORK_ABORTED)
   ||   (ec >= eQMI_ERR_CAT_EVT_REG_FAILED && ec < eQMI_ERR_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMICallEndReason Enumeration
//    QMI Call End Reason Enumeration
/*=========================================================================*/
enum eQMICallEndReason
{
    eQMI_CALL_END_REASON_BEGIN = -1,

   // General 
   eQMI_CALL_END_REASON_UNSPECIFIED = 1,           // 1
   eQMI_CALL_END_REASON_CLIENT_END,                // 2
   eQMI_CALL_END_REASON_NO_SRV,                    // 3
   eQMI_CALL_END_REASON_FADE,                      // 4
   eQMI_CALL_END_REASON_REL_NORMAL,                // 5
   eQMI_CALL_END_REASON_ACC_IN_PROG,               // 6
   eQMI_CALL_END_REASON_ACC_FAIL,                  // 7
   eQMI_CALL_END_REASON_REDIR_OR_HANDOFF,          // 8
   eQMI_CALL_END_REASON_CLOSE_IN_PROGRESS,         // 9
   eQMI_CALL_END_REASON_AUTH_FAILED,               // 10
   eQMI_CALL_END_REASON_INTERNAL,                  // 11

   // CDMA
   eQMI_CALL_END_REASON_CDMA_LOCK = 500,           // 500
   eQMI_CALL_END_REASON_INTERCEPT,                 // 501
   eQMI_CALL_END_REASON_REORDER,                   // 502
   eQMI_CALL_END_REASON_REL_SO_REJ,                // 503
   eQMI_CALL_END_REASON_INCOM_CALL,                // 504
   eQMI_CALL_END_REASON_ALERT_STOP,                // 505
   eQMI_CALL_END_REASON_ACTIVATION,                // 506
   eQMI_CALL_END_REASON_MAX_ACCESS_PROBE,          // 507
   eQMI_CALL_END_REASON_CCS_NOT_SUPPORTED_BY_BS,   // 508
   eQMI_CALL_END_REASON_NO_RESPONSE_FROM_BS,       // 509
   eQMI_CALL_END_REASON_REJECTED_BY_BS,            // 510
   eQMI_CALL_END_REASON_INCOMPATIBLE,              // 511
   eQMI_CALL_END_REASON_ALREADY_IN_TC,             // 512
   eQMI_CALL_END_REASON_USER_CALL_ORIG_DURING_GPS, // 513
   eQMI_CALL_END_REASON_USER_CALL_ORIG_DURING_SMS, // 514
   eQMI_CALL_END_REASON_NO_CDMA_SRV,               // 515

   // GSM/WCDMA
   eQMI_CALL_END_REASON_CONF_FAILED = 1000,        // 1000
   eQMI_CALL_END_REASON_INCOM_REJ,                 // 1001
   eQMI_CALL_END_REASON_NO_GW_SRV,                 // 1002
   eQMI_CALL_END_REASON_NETWORK_END,               // 1003

   eQMI_CALL_END_REASON_LLC_SNDCP_FAILURE,         // 1004
   eQMI_CALL_END_REASON_INSUFFICIENT_RESOURCES,    // 1005
   eQMI_CALL_END_REASON_OPTION_TEMP_OOO,           // 1006
   eQMI_CALL_END_REASON_NSAPI_ALREADY_USED,        // 1007
   eQMI_CALL_END_REASON_REGULAR_DEACTIVATION,      // 1008
   eQMI_CALL_END_REASON_NETWORK_FAILURE,           // 1009
   eQMI_CALL_END_REASON_UMTS_REATTACH_REQ,         // 1010
   eQMI_CALL_END_REASON_UMTS_PROTOCOL_ERROR,       // 1011
   eQMI_CALL_END_REASON_OPERATOR_BARRING,          // 1012
   eQMI_CALL_END_REASON_UNKNOWN_APN,               // 1013
   eQMI_CALL_END_REASON_UNKNOWN_PDP,               // 1014
   eQMI_CALL_END_REASON_GGSN_REJECT,               // 1015
   eQMI_CALL_END_REASON_ACTIVATION_REJECT,         // 1016
   eQMI_CALL_END_REASON_OPTION_NOT_SUPPORTED,      // 1017
   eQMI_CALL_END_REASON_OPTION_UNSUBSCRIBED,       // 1018
   eQMI_CALL_END_REASON_QOS_NOT_ACCEPTED,          // 1019
   eQMI_CALL_END_REASON_TFT_SEMANTIC_ERROR,        // 1020
   eQMI_CALL_END_REASON_TFT_SYNTAX_ERROR,          // 1021
   eQMI_CALL_END_REASON_UNKNOWN_PDP_CONTEXT,       // 1022
   eQMI_CALL_END_REASON_FILTER_SEMANTIC_ERROR,     // 1023
   eQMI_CALL_END_REASON_FILTER_SYNTAX_ERROR,       // 1024
   eQMI_CALL_END_REASON_PDP_WITHOUT_ACTIVE_TFT,    // 1025
   eQMI_CALL_END_REASON_INVALID_TRANSACTION_ID,    // 1026
   eQMI_CALL_END_REASON_MESSAGE_SEMANTIC_ERROR,    // 1027
   eQMI_CALL_END_REASON_INVALID_MANDATORY_INFO,    // 1028
   eQMI_CALL_END_REASON_TYPE_UNSUPPORTED,          // 1029
   eQMI_CALL_END_REASON_MSG_TYPE_WRONG_FOR_STATE,  // 1030
   eQMI_CALL_END_REASON_UNKNOWN_INFO_ELEMENT,      // 1031
   eQMI_CALL_END_REASON_CONDITIONAL_IE_ERROR,      // 1032
   eQMI_CALL_END_REASON_MSG_WRONG_FOR_PROTOCOL,    // 1033
   eQMI_CALL_END_REASON_APN_TYPE_CONFLICT,         // 1034
   eQMI_CALL_END_REASON_NO_GPRS_CONTEXT,           // 1035
   eQMI_CALL_END_REASON_FEATURE_NOT_SUPPORTED,     // 1036

   // CDMA 1xEV-DO (HDR)
   eQMI_CALL_END_REASON_CD_GEN_OR_BUSY = 1500,     // 1500
   eQMI_CALL_END_REASON_CD_BILL_OR_AUTH,           // 1501
   eQMI_CALL_END_REASON_CHG_HDR,                   // 1502
   eQMI_CALL_END_REASON_EXIT_HDR,                  // 1503
   eQMI_CALL_END_REASON_HDR_NO_SESSION ,           // 1504
   eQMI_CALL_END_REASON_HDR_ORIG_DURING_GPS_FIX,   // 1505
   eQMI_CALL_END_REASON_HDR_CS_TIMEOUT ,           // 1506
   eQMI_CALL_END_REASON_HDR_RELEASED_BY_CM,        // 1507

   eQMI_CALL_END_REASON_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMICallEndReason validity check

PARAMETERS:
   err         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMICallEndReason err )
{
   if ( (err >= eQMI_CALL_END_REASON_UNSPECIFIED) 
   &&   (err <= eQMI_CALL_END_REASON_INTERNAL) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CDMA_LOCK)
   &&   (err <= eQMI_CALL_END_REASON_NO_CDMA_SRV) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CONF_FAILED)
   &&   (err <= eQMI_CALL_END_REASON_FEATURE_NOT_SUPPORTED) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CD_GEN_OR_BUSY)
   &&   (err <= eQMI_CALL_END_REASON_HDR_RELEASED_BY_CM) )
   {
      return true;
   }

   return false;
};
