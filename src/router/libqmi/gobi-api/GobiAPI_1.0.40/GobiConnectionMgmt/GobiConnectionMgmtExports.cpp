/*===========================================================================
FILE: 
   GobiConnectionMgmtExports.cpp

DESCRIPTION:
   QUALCOMM Gobi 3000 Connection Management API exports

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
#include "GobiConnectionMgmt.h"
#include "GobiConnectionMgmtAPI.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Maximum length for adapter device path
const ULONG MAX_AI_DEVICE_PATH = 256;

// Maximum length for adapter key
const ULONG MAX_AI_KEY = 16;

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sAdapterInfo
//    Struct to represent Gobi adapter info
/*=========================================================================*/  
struct sAdapterInfo
{
   public:
      CHAR mPath[MAX_AI_DEVICE_PATH];
      CHAR mKey[MAX_AI_KEY];
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

/*=========================================================================*/
// Exported Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   QCWWANEnumerateDevices

DESCRIPTION:
   This function enumerates the Gobi devices currently attached to the
   system

PARAMETERS:
   pDevicesSize   [I/O] - Upon input the maximum number of elements that the 
                          device array can contain.  Upon successful output 
                          the actual number of elements in the device array
   pDevices       [ O ] - The device array 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QCWWANEnumerateDevices( 
   BYTE *                     pDevicesSize, 
   BYTE *                     pDevices )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   // Validate arguments
   if (pDevicesSize == 0 || pDevices == 0) 
   {
      return (ULONG)eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   BYTE maxInstances = *pDevicesSize;
   *pDevicesSize = 0;

   // Obtain adapter info
   std::vector <cGobiConnectionMgmt::tDeviceID> adapters;
   adapters = pAPI->GetAvailableDevices();

   ULONG sz = (ULONG)adapters.size();
   if (sz > (ULONG)maxInstances)
   {
      sz = (ULONG)maxInstances;
   }

   sAdapterInfo * pOutput = (sAdapterInfo *)pDevices;
   for (ULONG a = 0; a < sz; a++)
   {
      const cGobiConnectionMgmt::tDeviceID & id = adapters[a];

      memset( &pOutput->mPath[0], 0, (SIZE_T)MAX_AI_DEVICE_PATH );
      memset( &pOutput->mKey[0], 0, (SIZE_T)MAX_AI_KEY );

      ULONG len = id.first.size();
      if (len > 0)
      {
         if (len >= MAX_AI_DEVICE_PATH)
         {
            len = MAX_AI_DEVICE_PATH - 1;
         }

         memcpy( (LPVOID)&pOutput->mPath[0], 
                 (LPVOID)id.first.c_str(), 
                 (SIZE_T)len );
      }

      len = id.second.size();
      if (len > 0)
      {
         if (len >= MAX_AI_KEY)
         {
            len = MAX_AI_KEY - 1;
         }

         memcpy( (LPVOID)&pOutput->mKey[0], 
                 (LPCVOID)id.second.c_str(), 
                 (SIZE_T)len );
      }

      pOutput++;
   }

   *pDevicesSize = (BYTE)sz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   QCWWANConnect

DESCRIPTION:
   This function connects the CM API library to the specified Gobi 
   device

   Both device node and key are case sensitive

PARAMETERS:
   pDeviceNode   [ I ] - The device node
   pDeviceKey    [ I ] - The device key (unique, stored on-device)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QCWWANConnect(
   CHAR *                     pDeviceNode,
   CHAR *                     pDeviceKey )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   // Validate arguments
   if (pDeviceNode == 0 && pDeviceKey != 0)
   {
      // If you specify a device key then you have to specify a device ID
      return (ULONG)eGOBI_ERR_INVALID_ARG;
   }

   bool bConnect = pAPI->Connect( (LPCSTR)pDeviceNode, 
                                  (LPCSTR)pDeviceKey );
   if (bConnect == false)
   {
      return (ULONG)pAPI->GetCorrectedLastError();
   }

   return (ULONG)eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   QCWWANCancel

DESCRIPTION:
   This function cancels the most recent outstanding request
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QCWWANCancel()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->CancelSend();
}

/*===========================================================================
METHOD:
   QCWWANDisconnect

DESCRIPTION:
   This function disconnects the CM API library from the currently 
   connected Gobi device

PARAMETERS:
   pState      [ O ] - State of the current packet session
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QCWWANDisconnect()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   bool bDisco = pAPI->Disconnect();
   if (bDisco == false)
   {
      return (ULONG)pAPI->GetCorrectedLastError();
   }

   return (ULONG)eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   QCWWANGetConnectedDeviceID

DESCRIPTION:
   This function returns the Node/key of the device the Gobi CM API library 
   is currently connected to

PARAMETERS:
   deviceNodeSize [ I ] - The maximum number of characters (including NULL 
                          terminator) that the device Node array can contain
   pDeviceNode    [ O ] - Device Node (NULL terminated string)
   deviceKeySize  [ I ] - The maximum number of characters (including NULL 
                          terminator) that the device key array can contain
   pDeviceKey     [ O ] - Device key (NULL terminated string)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QCWWANGetConnectedDeviceID( 
   ULONG                      deviceNodeSize, 
   CHAR *                     pDeviceNode,
   ULONG                      deviceKeySize, 
   CHAR *                     pDeviceKey )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   // Validate arguments
   if ( (deviceNodeSize == 0) 
   ||   (pDeviceNode == 0) 
   ||   (deviceKeySize == 0)
   ||   (pDeviceKey == 0) )
   {
      return (ULONG)eGOBI_ERR_INVALID_ARG;
   }

   *pDeviceNode = 0;
   *pDeviceKey = 0;

   std::string devNode = "";
   std::string devKey = "";   
   bool bOK = pAPI->GetConnectedDeviceID( devNode, devKey );
   if (bOK == false)
   {
      return (ULONG)pAPI->GetCorrectedLastError();
   }

   ULONG lenNode = (ULONG)devNode.size();

   // Space to perform the copy?
   if (deviceNodeSize < lenNode + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pDeviceNode, (LPVOID)devNode.c_str(), lenNode );
   
   // Enforce null termination
   pDeviceNode[lenNode] = 0;

   ULONG lenKey = (ULONG)devKey.size();

   // Space to perform the copy?
   if (deviceKeySize < lenKey + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pDeviceKey, (LPVOID)devKey.c_str(), lenKey );
   
   // Enforce null termination
   pDeviceKey[lenKey] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSessionState

DESCRIPTION:
   This function returns the state of the current packet data session

PARAMETERS:
   pState      [ O ] - State of the current packet session
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSessionState( ULONG * pState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSessionState( pState );
}

/*===========================================================================
METHOD:
   GetSessionDuration

DESCRIPTION:
   This function returns the duration of the current packet data session

PARAMETERS:
   pDuration   [ O ] - Duration of the current packet session
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSessionDuration( ULONGLONG * pDuration )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }
   
   return (ULONG)pAPI->GetSessionDuration( pDuration );
}

/*===========================================================================
METHOD:
   GetDormancyState

DESCRIPTION:
   This function returns the dormancy state of the current packet 
   data session (when connected)

PARAMETERS:
   pState      [ O ] - Dormancy state of the current packet session
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetDormancyState( ULONG * pState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetDormancyState( pState );
}

/*===========================================================================
METHOD:
   GetAutoconnect (Deprecated)

DESCRIPTION:
   This function returns the current autoconnect data session setting

PARAMETERS:
   pSetting    [ O ] - NDIS autoconnect setting
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetAutoconnect( ULONG * pSetting )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG dummy;
   return (ULONG)pAPI->GetEnhancedAutoconnect( pSetting, &dummy );
}

/*===========================================================================
METHOD:
   SetAutoconnect (Deprecated)

DESCRIPTION:
   This function sets the autoconnect data session setting

PARAMETERS:
   setting    [ I ] - NDIS autoconnect disabled (0) or enabled (non-zero)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetAutoconnect( ULONG setting )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetEnhancedAutoconnect( setting, 0 );
}

/*===========================================================================
METHOD:
   GetEnhancedAutoconnect

DESCRIPTION:
   This function returns the current autoconnect data session setting

PARAMETERS:
   pSetting       [ O ] - NDIS autoconnect setting
   pRoamSetting   [ O ] - NDIS autoconnect roam setting 
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetEnhancedAutoconnect( 
   ULONG *                    pSetting,
   ULONG *                    pRoamSetting )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetEnhancedAutoconnect( pSetting, pRoamSetting );
}

/*===========================================================================
METHOD:
   SetEnhancedAutoconnect

DESCRIPTION:
   This function sets the autoconnect data session setting

PARAMETERS:
   setting        [ I ] - NDIS autoconnect setting
   pRoamSetting   [ I ] - (Optional) NDIS autoconnect roam setting 

 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetEnhancedAutoconnect( 
   ULONG                      setting,
   ULONG *                    pRoamSetting )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetEnhancedAutoconnect( setting, pRoamSetting );
}


/*===========================================================================
METHOD:
   SetDefaultProfile

DESCRIPTION:
   This function writes the default profile settings to the device, the 
   default profile is used during autoconnect

PARAMETERS:
   profileType       [ I ] - Profile type being written
   pPDPType          [ I ] - (Optional) PDP type
   pIPAddress        [ I ] - (Optional) Preferred assigned IPv4 address
   pPrimaryDNS       [ I ] - (Optional) Primary DNS IPv4 address 
   pSecondaryDNS     [ I ] - (Optional) Secondary DNS IPv4 address 
   pAuthentication   [ I ] - (Optional) Authentication algorithm bitmap
   pName             [ I ] - (Optional) The profile name or description 
   pAPNName          [ I ] - (Optional) Access point name 
   pUsername         [ I ] - (Optional) Username used during authentication
   pPassword         [ I ] - (Optional) Password used during authentication

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDefaultProfile( 
   ULONG                      profileType,
   ULONG *                    pPDPType, 
   ULONG *                    pIPAddress, 
   ULONG *                    pPrimaryDNS, 
   ULONG *                    pSecondaryDNS, 
   ULONG *                    pAuthentication, 
   CHAR *                     pName, 
   CHAR *                     pAPNName, 
   CHAR *                     pUsername,
   CHAR *                     pPassword )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetDefaultProfile( profileType, 
                                             pPDPType, 
                                             pIPAddress,
                                             pPrimaryDNS,
                                             pSecondaryDNS, 
                                             pAuthentication, 
                                             pName,
                                             pAPNName,
                                             pUsername,
                                             pPassword );
}

/*===========================================================================
METHOD:
   GetDefaultProfile

DESCRIPTION:
   This function reads the default profile settings from the device, the 
   default profile is used during autoconnect

PARAMETERS:
   profileType       [ I ] - Profile type being read
   pPDPType          [ O ] - PDP type
   pIPAddress        [ O ] - Preferred assigned IPv4 address
   pPrimaryDNS       [ O ] - Primary DNS IPv4 address 
   pSecondaryDNS     [ O ] - Secondary DNS IPv4 address 
   pAuthentication   [ O ] - Authentication algorithm bitmap
   nameSize          [ I ] - The maximum number of characters (including 
                             NULL terminator) that the profile name array 
                             can contain
   pName             [ O ] - The profile name or description 
   apnSize           [ I ] - The maximum number of characters (including 
                             NULL terminator) that the APN name array 
                             can contain
   pAPNName          [ O ] - Access point name represented as a NULL 
                             terminated string (empty string returned when 
                             unknown)
   userSize          [ I ] - The maximum number of characters (including 
                             NULL terminator) that the username array 
                             can contain
   pUsername         [ O ] - Username used during authentication

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetDefaultProfile( 
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
   CHAR *                     pUsername )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetDefaultProfile( profileType, 
                                             pPDPType, 
                                             pIPAddress,
                                             pPrimaryDNS,
                                             pSecondaryDNS, 
                                             pAuthentication, 
                                             nameSize,
                                             pName,
                                             apnSize,
                                             pAPNName,
                                             userSize,
                                             pUsername );
}

/*===========================================================================
METHOD:
   StartDataSession

DESCRIPTION:
   This function activates a packet data session

PARAMETERS:
   pTechnology       [ I ] - (Optional) Technology bitmap
   pPrimaryDNS       [ I ] - (Optional) Primary DNS IPv4 address 
   pSecondaryDNS     [ I ] - (Optional) Secondary DNS IPv4 address 
   pPrimaryNBNS      [ I ] - (Optional) Primary NetBIOS NS IPv4 address
   pSecondaryNBNS    [ I ] - (Optional) Secondary NetBIOS NS IPv4 address
   pAPNName          [ I ] - (Optional) Access point name 
   pIPAddress        [ I ] - (Optional) Preferred assigned IPv4 address
   pAuthentication   [ I ] - (Optional) Authentication algorithm bitmap
   pUsername         [ I ] - (Optional) Username used during authentication
   pPassword         [ I ] - (Optional) Password used during authentication
   pSessionId        [ O ] - The assigned session ID
   pFailureReason    [ O ] - Upon call failure the failure reason provided

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG StartDataSession( 
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
   ULONG *                    pFailureReason )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->StartDataSession( pTechnology, 
                                            pPrimaryDNS, 
                                            pSecondaryDNS, 
                                            pPrimaryNBNS, 
                                            pSecondaryNBNS, 
                                            pAPNName, 
                                            pIPAddress, 
                                            pAuthentication, 
                                            pUsername, 
                                            pPassword,
                                            pSessionId,
                                            pFailureReason );
}

/*===========================================================================
METHOD:
   CancelDataSession

DESCRIPTION:
   This function cancels an in-progress packet data session activation

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CancelDataSession()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->CancelDataSession();
}

/*===========================================================================
METHOD:
   StopDataSession

DESCRIPTION:
   This function stops the current data session

PARAMETERS:
   sessionId   [ I ] - The ID of the session to terminate
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG StopDataSession( ULONG sessionId )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->StopDataSession( sessionId );
}

/*===========================================================================
METHOD:
   GetIPAddress

DESCRIPTION:
   This function returns the current packet data session IP address

PARAMETERS:
   pIPAddress        [ O ] - Assigned IPv4 address
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetIPAddress( ULONG * pIPAddress )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->GetIPAddress( pIPAddress );
}

/*===========================================================================
METHOD:
   GetConnectionRate

DESCRIPTION:
   This function returns connection rate information for the packet data 
   connection

PARAMETERS:
   pCurrentChannelTXRate   [ O ] - Current channel TX rate (bps)
   pCurrentChannelRXRate   [ O ] - Current channel RX rate (bps)
   pMaxChannelTXRate       [ O ] - Maximum channel TX rate (bps)
   pMaxChannelRXRate       [ O ] - Maximum channel RX rate (bps)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetConnectionRate(
   ULONG *                    pCurrentChannelTXRate,
   ULONG *                    pCurrentChannelRXRate,
   ULONG *                    pMaxChannelTXRate,
   ULONG *                    pMaxChannelRXRate )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetConnectionRate( pCurrentChannelTXRate,
                                             pCurrentChannelRXRate,
                                             pMaxChannelTXRate,
                                             pMaxChannelRXRate );
}

/*===========================================================================
METHOD:
   GetPacketStatus

DESCRIPTION:
   This function returns the packet data transfer statistics since the start 
   of the current packet data session

PARAMETERS:
   pTXPacketSuccesses   [ O ] - Packets transmitted without error
   pRXPacketSuccesses   [ O ] - Packets received without error
   pTXPacketErrors      [ O ] - Outgoing packets with framing errors
   pRXPacketErrors      [ O ] - Incoming packets with framing errors
   pTXPacketOverflows   [ O ] - Packets dropped because TX buffer overflowed 
   pRXPacketOverflows   [ O ] - Packets dropped because RX buffer overflowed
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPacketStatus(  
   ULONG *                    pTXPacketSuccesses, 
   ULONG *                    pRXPacketSuccesses, 
   ULONG *                    pTXPacketErrors, 
   ULONG *                    pRXPacketErrors, 
   ULONG *                    pTXPacketOverflows, 
   ULONG *                    pRXPacketOverflows )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPacketStatus( pTXPacketSuccesses,
                                           pRXPacketSuccesses,
                                           pTXPacketErrors,
                                           pRXPacketErrors,
                                           pTXPacketOverflows,
                                           pRXPacketOverflows );
}

/*===========================================================================
METHOD:
   GetByteTotals

DESCRIPTION:
   This function returns the RX/TX byte counts since the start of the 
   current packet data session

PARAMETERS:
   pTXTotalBytes  [ O ] - Bytes transmitted without error
   pRXTotalBytes  [ O ] - Bytes received without error
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetByteTotals(   
   ULONGLONG *                pTXTotalBytes, 
   ULONGLONG *                pRXTotalBytes )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetByteTotals( pTXTotalBytes, pRXTotalBytes );
}

/*===========================================================================
METHOD:
   SetMobileIP

DESCRIPTION:
   This function sets the current mobile IP setting

PARAMETERS:
   mode        [ I ] - Desired mobile IP setting
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetMobileIP( ULONG mode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetMobileIP( mode );
}

/*===========================================================================
METHOD:
   GetMobileIP

DESCRIPTION:
   This function gets the current mobile IP setting

PARAMETERS:
   pMode       [ O ] - Current mobile IP setting
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetMobileIP( ULONG * pMode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetMobileIP( pMode );
}

/*===========================================================================
METHOD:
   SetActiveMobileIPProfile

DESCRIPTION:
   This function sets the active mobile IP profile index

PARAMETERS:
   pSPC        [ I ] - Six digit service programming code
   index       [ I ] - Desired mobile IP profile index
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetActiveMobileIPProfile( 
   CHAR *                     pSPC,
   BYTE                       index )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetActiveMobileIPProfile( pSPC, index );
}

/*===========================================================================
METHOD:
   GetActiveMobileIPProfile

DESCRIPTION:
   This function gets the the active mobile IP profile index

PARAMETERS:
   pIndex      [ O ] - Active mobile IP profile index
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetActiveMobileIPProfile( BYTE * pIndex )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetActiveMobileIPProfile( pIndex );
}

/*===========================================================================
METHOD:
   SetMobileIPProfile

DESCRIPTION:
   This function sets the specified mobile IP profile settings

PARAMETERS:
   pSPC           [ I ] - Six digit service programming code
   index          [ I ] - Mobile IP profile ID
   pEnabled       [ I ] - (Optional) Enable MIP profile?
   pAddress       [ I ] - (Optional) Home IPv4 address
   pPrimaryHA     [ I ] - (Optional) Primary home agent IPv4 address
   pSecondaryHA   [ I ] - (Optional) Secondary home agent IPv4 address
   bRevTunneling  [ I ] - (Optional) Enable reverse tunneling?
   pNAI           [ I ] - (Optional) Network access identifier string
   pHASPI         [ I ] - (Optional) HA security parameter index
   pAAASPI        [ I ] - (Optional) AAA security parameter index
   pMNHA          [ I ] - (Optional) MN-HA string
   pMNAAA         [ I ] - (Optional) MN-AAA string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetMobileIPProfile( 
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
   CHAR *                     pMNAAA )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetMobileIPProfile( pSPC,
                                              index,
                                              pEnabled,
                                              pAddress,
                                              pPrimaryHA,
                                              pSecondaryHA,
                                              pRevTunneling,
                                              pNAI,
                                              pHASPI,
                                              pAAASPI,
                                              pMNHA,
                                              pMNAAA );
}

/*===========================================================================
METHOD:
   GetMobileIPProfile

DESCRIPTION:
   This function gets the specified mobile IP profile settings

PARAMETERS:
   index          [ I ] - Mobile IP profile ID
   pEnabled       [ O ] - Mobile IP profile enabled?
   pAddress       [ O ] - Home IPv4 address
   pPrimaryHA     [ O ] - Primary home agent IPv4 address
   pSecondaryHA   [ O ] - Secondary home agent IPv4 address
   pRevTunneling  [ O ] - Reverse tunneling enabled?
   naiSize        [ I ] - The maximum number of characters (including NULL 
                          terminator) that the NAI array can contain
   pNAI           [ O ] - Network access identifier string
   pHASPI         [ O ] - HA security parameter index
   pAAASPI        [ O ] - AAA security parameter index
   pHAState       [ O ] - HA key state
   pAAAState      [ O ] - AAA key state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetMobileIPProfile( 
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
   ULONG *                    pAAAState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetMobileIPProfile( index,
                                              pEnabled,
                                              pAddress,
                                              pPrimaryHA,
                                              pSecondaryHA,
                                              pRevTunneling,
                                              naiSize,
                                              pNAI,
                                              pHASPI,
                                              pAAASPI,
                                              pHAState,
                                              pAAAState );
}

/*===========================================================================
METHOD:
   SetMobileIPParameters

DESCRIPTION:
   This function sets the specified mobile IP parameters

PARAMETERS:
   pSPC              [ I ] - Six digit service programming code
   pMode             [ I ] - (Optional) Desired mobile IP setting
   pRetryLimit       [ I ] - (Optional) Retry attempt limit
   pRetryInterval    [ I ] - (Optional) Retry attempt interval
   pReRegPeriod      [ I ] - (Optional) Re-registration period
   pReRegTraffic     [ I ] - (Optional) Re-registration only with traffic?
   pHAAuthenticator  [ I ] - (Optional) MH-HA authenticator calculator?
   pHA2002bis        [ I ] - (Optional) MH-HA RFC 2002bis authentication?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetMobileIPParameters( 
   CHAR *                     pSPC,
   ULONG *                    pMode,
   BYTE *                     pRetryLimit,
   BYTE *                     pRetryInterval,
   BYTE *                     pReRegPeriod,
   BYTE *                     pReRegTraffic,
   BYTE *                     pHAAuthenticator,
   BYTE *                     pHA2002bis )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetMobileIPParameters( pSPC,
                                                 pMode,
                                                 pRetryLimit,
                                                 pRetryInterval,
                                                 pReRegPeriod,
                                                 pReRegTraffic,
                                                 pHAAuthenticator,
                                                 pHA2002bis );
}

/*===========================================================================
METHOD:
   GetMobileIPParameters

DESCRIPTION:
   This function gets the mobile IP parameters

PARAMETERS:
   pMode             [ O ] - Current mobile IP setting
   pRetryLimit       [ O ] - Retry attempt limit
   pRetryInterval    [ O ] - Retry attempt interval
   pReRegPeriod      [ O ] - Re-registration period
   pReRegTraffic     [ O ] - Re-registration only with traffic?
   pHAAuthenticator  [ O ] - MH-HA authenticator calculator?
   pHA2002bis        [ O ] - MH-HA RFC 2002bis authentication?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetMobileIPParameters( 
   ULONG *                    pMode,
   BYTE *                     pRetryLimit,
   BYTE *                     pRetryInterval,
   BYTE *                     pReRegPeriod,
   BYTE *                     pReRegTraffic,
   BYTE *                     pHAAuthenticator,
   BYTE *                     pHA2002bis )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetMobileIPParameters( pMode,
                                                 pRetryLimit,
                                                 pRetryInterval,
                                                 pReRegPeriod,
                                                 pReRegTraffic,
                                                 pHAAuthenticator,
                                                 pHA2002bis );
}

/*===========================================================================
METHOD:
   GetLastMobileIPError

DESCRIPTION:
   This function gets the last mobile IP error

PARAMETERS:
   pError      [ O ] - Last mobile IP error

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetLastMobileIPError( ULONG * pError )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetLastMobileIPError( pError );
}

/*===========================================================================
METHOD:
   SetDNSSettings

DESCRIPTION:
   This function sets the DNS settings for the device

PARAMETERS:
   pPrimaryDNS       [ I ] - (Optional) Primary DNS IPv4 address 
   pSecondaryDNS     [ I ] - (Optional) Secondary DNS IPv4 address 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDNSSettings( 
   ULONG *                    pPrimaryDNS, 
   ULONG *                    pSecondaryDNS )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetDNSSettings( pPrimaryDNS, pSecondaryDNS );
}

/*===========================================================================
METHOD:
   GetDNSSettings

DESCRIPTION:
   This function gets the DNS settings for the device

PARAMETERS:
   pPrimaryDNS       [ O ] - Primary DNS IPv4 address 
   pSecondaryDNS     [ O ] - Secondary DNS IPv4 address 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetDNSSettings( 
   ULONG *                    pPrimaryDNS, 
   ULONG *                    pSecondaryDNS )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetDNSSettings( pPrimaryDNS, pSecondaryDNS );
}

/*===========================================================================
METHOD:
   GetANAAAAuthenticationStatus

DESCRIPTION:
   This function gets the AN-AAA authentication status

PARAMETERS:
   pStatus     [ O ] - AN-AAA authentication status

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetANAAAAuthenticationStatus( ULONG * pStatus )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetANAAAAuthenticationStatus( pStatus );
}

/*===========================================================================
METHOD:
   GetSignalStrengths

DESCRIPTION:
   This function gets the current available signal strengths (in dBm) 
   as measured by the device

PARAMETERS:
   pArraySizes       [I/O] - Upon input the maximum number of elements 
                             that each array can contain can contain.  
                             Upon successful output the actual number 
                             of elements in each array
   pSignalStrengths  [ O ] - Received signal strength array (dBm)
   pRadioInterfaces  [ O ] - Radio interface technology array 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSignalStrengths( 
   ULONG *                    pArraySizes, 
   INT8 *                     pSignalStrengths, 
   ULONG *                    pRadioInterfaces )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSignalStrengths( pArraySizes, 
                                              pSignalStrengths, 
                                              pRadioInterfaces );
}

/*===========================================================================
METHOD:
   GetRFInfo

DESCRIPTION:
   This function gets the current RF information

PARAMETERS:
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the 
                          RF info instance array can contain.  Upon success
                          the actual number of elements in the RF info 
                          instance array
   pInstances     [ O ] - The RF info instance array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetRFInfo( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetRFInfo( pInstanceSize, pInstances ); 
}



/*===========================================================================
METHOD:
   PerformNetworkScan

DESCRIPTION:
   This function performs a scan for available networks

PARAMETERS:
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the 
                          network info instance array can contain.  Upon 
                          success the actual number of elements in the
                          network info instance array
   pInstances     [ O ] - The network info instance array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PerformNetworkScan( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->PerformNetworkScan( pInstanceSize, pInstances ); 
}

/*===========================================================================
METHOD:
   PerformNetworkRATScan

DESCRIPTION:
   This function performs a scan for available networks (includes RAT)

PARAMETERS:
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
ULONG PerformNetworkRATScan( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances,
   BYTE *                     pRATSize, 
   BYTE *                     pRATInstances )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->PerformNetworkRATScan( pInstanceSize, 
                                              pInstances,
                                              pRATSize,
                                              pRATInstances ); 
}

/*===========================================================================
METHOD:
   InitiateNetworkRegistration

DESCRIPTION:
   This function initiates a network registration

PARAMETERS:
   regType     [ I ] - Registration type 
   mcc         [ I ] - Mobile country code (ignored for auto registration)
   mnc         [ I ] - Mobile network code (ignored for auto registration)
   rat         [ I ] - Radio access type (ignored for auto registration)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG InitiateNetworkRegistration( 
   ULONG                      regType,
   WORD                       mcc, 
   WORD                       mnc, 
   ULONG                      rat )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->InitiateNetworkRegistration( regType, 
                                                       mcc, 
                                                       mnc, 
                                                       rat ); 
}

/*===========================================================================
METHOD:
   InitiateDomainAttach

DESCRIPTION:
   This function initiates a domain attach (or detach)

PARAMETERS:
   action      [ I ] - PS attach action (attach or detach)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG InitiateDomainAttach( ULONG action )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->InitiateDomainAttach( action ); 
}

/*===========================================================================
METHOD:
   GetServingNetwork

DESCRIPTION:
   Gets information regarding the system that currently provides service 
   to the device

PARAMETERS:
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
ULONG GetServingNetwork( 
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
   CHAR *                     pName )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetServingNetwork( pRegistrationState,
                                             pCSDomain, 
                                             pPSDomain, 
                                             pRAN, 
                                             pRadioIfacesSize, 
                                             pRadioIfaces, 
                                             pRoaming, 
                                             pMCC, 
                                             pMNC, 
                                             nameSize, 
                                             pName );
}

/*===========================================================================
METHOD:
   GetServingNetworkCapabilities

DESCRIPTION:
   Gets information regarding the data capabilities of the system that 
   currently provides service to the device

PARAMETERS:
   pDataCapsSize  [I/O] - Upon input the maximum number of elements that the 
                          data capabilities array can contain.  Upon success
                          the actual number of elements in the data 
                          capabilities array
   pDataCaps      [ O ] - The data capabilities array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetServingNetworkCapabilities( 
   BYTE *                     pDataCapsSize, 
   BYTE *                     pDataCaps )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetServingNetworkCapabilities( pDataCapsSize,
                                                         pDataCaps ); 
}

/*===========================================================================
METHOD:
   GetDataBearerTechnology

DESCRIPTION:
   This function retrieves the current data bearer technology (only
   valid when connected)

PARAMETERS:
   pDataCaps      [ O ] - The data bearer technology
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetDataBearerTechnology( ULONG * pDataBearer )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetDataBearerTechnology( pDataBearer );
}

/*===========================================================================
METHOD:
   GetHomeNetwork

DESCRIPTION:
   This function retrieves information about the home network of the device

PARAMETERS:
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
ULONG GetHomeNetwork( 
   WORD *                     pMCC, 
   WORD *                     pMNC, 
   BYTE                       nameSize, 
   CHAR *                     pName, 
   WORD *                     pSID, 
   WORD *                     pNID )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetHomeNetwork( pMCC,
                                          pMNC,
                                          nameSize,
                                          pName,
                                          pSID,
                                          pNID );
}

/*===========================================================================
METHOD:
   SetNetworkPreference

DESCRIPTION:
   This function sets the network registration preference

PARAMETERS:
   technologyPref [ I ] - Technology preference bitmap
   duration       [ I ] - Duration of active preference
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetNetworkPreference( 
   ULONG                      technologyPref, 
   ULONG                      duration )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetNetworkPreference( technologyPref, duration );
}

/*===========================================================================
METHOD:
   GetNetworkPreference

DESCRIPTION:
   This function returns the network registration preference

PARAMETERS:
   pTechnologyPref            [ O ] - Technology preference bitmap
   pDuration                  [ O ] - Duration of active preference
   pPersistentTechnologyPref  [ O ] - Persistent technology preference bitmap 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetNetworkPreference( 
   ULONG *                    pTechnologyPref, 
   ULONG *                    pDuration, 
   ULONG *                    pPersistentTechnologyPref )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetNetworkPreference( pTechnologyPref,
                                                pDuration,
                                                pPersistentTechnologyPref );
}

/*===========================================================================
METHOD:
   SetCDMANetworkParameters

DESCRIPTION:
   This function sets the desired CDMA network parameters

PARAMETERS:
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
ULONG SetCDMANetworkParameters( 
   CHAR *                     pSPC,
   BYTE *                     pForceRev0,
   BYTE *                     pCustomSCP,
   ULONG *                    pProtocol,
   ULONG *                    pBroadcast,
   ULONG *                    pApplication,
   ULONG *                    pRoaming )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetCDMANetworkParameters( pSPC,
                                             pForceRev0,
                                             pCustomSCP,
                                             pProtocol,
                                             pBroadcast,
                                             pApplication,
                                             pRoaming );
}

/*===========================================================================
METHOD:
   GetEVDONetworkParameters

DESCRIPTION:
   This function gets the current CDMA network parameters

PARAMETERS:
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
ULONG GetCDMANetworkParameters( 
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
   ULONG *                    pRoaming )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->GetCDMANetworkParameters( pSCI,
                                             pSCM,
                                             pRegHomeSID,
                                             pRegForeignSID,
                                             pRegForeignNID,
                                             pForceRev0,
                                             pCustomSCP,
                                             pProtocol,
                                             pBroadcast,
                                             pApplication,
                                             pRoaming );
}

/*===========================================================================
METHOD:
   GetACCOLC

DESCRIPTION:
   This function returns the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   pACCOLC     [ O ] - The ACCOLC
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetACCOLC( BYTE * pACCOLC )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetACCOLC( pACCOLC );
}

/*===========================================================================
METHOD:
   SetACCOLC

DESCRIPTION:
   This function sets the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   pSPC        [ I ] - NULL terminated string representing the six digit 
                       service programming code
   accolc      [ I ] - The ACCOLC
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetACCOLC( 
   CHAR *                     pSPC,
   BYTE                       accolc )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetACCOLC( pSPC, accolc );
}

/*===========================================================================
METHOD:
   GetPLMNMode

DESCRIPTION:
   This function returns the PLMN mode from the CSP

PARAMETERS:
   pMode       [ O ] - PLMN mode
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPLMNMode( ULONG * pMode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->GetPLMNMode( pMode );
}

/*===========================================================================
METHOD:
   GetPLMNName

DESCRIPTION:
   This function returns PLMN name information for the given MCC/MNC

PARAMETERS:
   mcc         [ I ] - Mobile country code
   mnc         [ I ] - Mobile network code
   pNamesSize  [I/O] - Upon input the size in BYTEs of the name structure 
                       array.  Upon success the actual number of BYTEs 
                       copied to the name structure array
   pNames      [ O ] - The name structure array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPLMNName(
   USHORT                     mcc,
   USHORT                     mnc,
   ULONG *                    pNamesSize, 
   BYTE *                     pNames )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->GetPLMNName( mcc, mnc, pNamesSize, pNames );
}

/*===========================================================================
METHOD:
   GetDeviceCapabilities

DESCRIPTION:
   This function gets device capabilities

PARAMETERS:
   pMaxTXChannelRate       [ O ] - Maximum transmission rate (bps) 
   pMaxRXChannelRate       [ O ] - Maximum reception rate (bps)
   pDataServiceCapability  [ O ] - CS/PS data service capability
   pSimCapability          [ O ] - Device SIM support
   pRadioIfacesSize        [I/O] - Upon input the maximum number of elements 
                                   that the radio interfaces can contain.  
                                   Upon successful output the actual number 
                                   of elements in the radio interface array
   pRadioIfaces            [ O ] - The radio interface array
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetDeviceCapabilities( 
   ULONG *                    pMaxTXChannelRate, 
   ULONG *                    pMaxRXChannelRate, 
   ULONG *                    pDataServiceCapability, 
   ULONG *                    pSimCapability, 
   ULONG *                    pRadioIfacesSize, 
   BYTE *                     pRadioIfaces )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetDeviceCapabilities( pMaxTXChannelRate,
                                                 pMaxRXChannelRate,
                                                 pDataServiceCapability,
                                                 pSimCapability,
                                                 pRadioIfacesSize,
                                                 pRadioIfaces );
}

/*===========================================================================
METHOD:
   GetManufacturer

DESCRIPTION:
   This function returns the device manufacturer name

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetManufacturer( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetManufacturer( stringSize, pString );
}

/*===========================================================================
METHOD:
   GetModelID

DESCRIPTION:
   This function returns the device model ID

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetModelID( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetModelID( stringSize, pString );
}

/*===========================================================================
METHOD:
   GetFirmwareRevision

DESCRIPTION:
   This function returns the device firmware revision

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetFirmwareRevision( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetFirmwareRevision( stringSize, pString );
}

/*===========================================================================
METHOD:
   GetFirmwareRevisions

DESCRIPTION:
   This function returns the device firmware (AMSS, boot, and PRI)
   revisions

PARAMETERS:
   amssSize    [ I ] - The maximum number of characters (including NULL 
                       terminator) that the AMSS string array can contain
   pAMSSString [ O ] - NULL terminated AMSS revision string
   bootSize    [ I ] - The maximum number of characters (including NULL 
                       terminator) that the boot string array can contain
   pBootString [ O ] - NULL terminated boot code revision string  
   priSize     [ I ] - The maximum number of characters (including NULL 
                       terminator) that the PRI string array can contain
   pPRIString  [ O ] - NULL terminated PRI revision string  

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetFirmwareRevisions( 
   BYTE                       amssSize, 
   CHAR *                     pAMSSString,
   BYTE                       bootSize, 
   CHAR *                     pBootString,
   BYTE                       priSize, 
   CHAR *                     pPRIString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetFirmwareRevisions( amssSize, 
                                                pAMSSString,
                                                bootSize,
                                                pBootString,
                                                priSize,
                                                pPRIString );
}

/*===========================================================================
METHOD:
   GetFirmwareInfo

DESCRIPTION:
   Returns image information obtained from the current device firmware

PARAMETERS:
   pFirmwareID    [ O ] - Firmware ID obtained from the firmware image
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetFirmwareInfo( 
   ULONG *                    pFirmwareID,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetFirmwareInfo( pFirmwareID,
                                           pTechnology,
                                           pCarrier,
                                           pRegion,
                                           pGPSCapability ); 
}

/*===========================================================================
METHOD:
   GetVoiceNumber

DESCRIPTION:
   This function returns the voice number in use by the device

PARAMETERS:
   voiceNumberSize   [ I ] - The maximum number of characters (including NULL 
                             terminator) that the voice number array can 
                             contain
   pVoiceNumber      [ O ] - Voice number (MDN or ISDN) string
   minSize           [ I ] - The maximum number of characters (including NULL 
                             terminator) that the MIN array can contain
   pMIN              [ O ] - MIN string (empty string returned when MIN is
                             not supported/programmed)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetVoiceNumber( 
   BYTE                       voiceNumberSize, 
   CHAR *                     pVoiceNumber,
   BYTE                       minSize, 
   CHAR *                     pMIN )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetVoiceNumber( voiceNumberSize,
                                          pVoiceNumber,
                                          minSize,
                                          pMIN );
}

/*===========================================================================
METHOD:
   GetIMSI

DESCRIPTION:
   This function returns the device IMSI

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetIMSI( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetIMSI( stringSize, pString );
}

/*===========================================================================
METHOD:
   GetSerialNumbers

DESCRIPTION:
   This command returns all serial numbers assigned to the device

PARAMETERS:
   esnSize     [ I ] - The maximum number of characters (including NULL 
                       terminator) that the ESN array can contain
   pESNString  [ O ] - ESN string (empty string returned when ESN is
                       not supported/programmed)
   imeiSize    [ I ] - The maximum number of characters (including NULL 
                       terminator) that the IMEI array can contain
   pIMEIString [ O ] - IMEI string (empty string returned when IMEI is
                       not supported/programmed)
   meidSize    [ I ] - The maximum number of characters (including NULL 
                       terminator) that the MEID array can contain
   pMEIDString [ O ] - MEID string (empty string returned when MEID is
                       not supported/programmed)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSerialNumbers( 
   BYTE                       esnSize, 
   CHAR *                     pESNString, 
   BYTE                       imeiSize, 
   CHAR *                     pIMEIString, 
   BYTE                       meidSize, 
   CHAR *                     pMEIDString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSerialNumbers( esnSize,
                                            pESNString,
                                            imeiSize,
                                            pIMEIString,
                                            meidSize,
                                            pMEIDString );
}

/*===========================================================================
METHOD:
   SetLock

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   state       [ I ] - Desired lock state
   pCurrentPIN [ I ] - Current four digit PIN string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetLock( 
   ULONG                      state, 
   CHAR *                     pCurrentPIN )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetLock( state, pCurrentPIN );
}

/*===========================================================================
METHOD:
   QueryLock

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   pState      [ O ] - Current lock state
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG QueryLock( ULONG * pState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->QueryLock( pState );
}

/*===========================================================================
METHOD:
   ChangeLockPIN

DESCRIPTION:
   This command sets the user lock code maintained by the device

PARAMETERS:
   pCurrentPIN [ O ] - Current four digit PIN string
   pDesiredPIN [ O ] - New four digit PIN string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ChangeLockPIN( 
   CHAR *                     pCurrentPIN, 
   CHAR *                     pDesiredPIN )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->ChangeLockPIN( pCurrentPIN, pDesiredPIN );
}

/*===========================================================================
METHOD:
   GetHardwareRevision

DESCRIPTION:
   This function returns the device hardware revision

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetHardwareRevision( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetHardwareRevision( stringSize, pString );
}

/*===========================================================================
METHOD:
   GetPRLVersion

DESCRIPTION:
   This function returns the version of the active Preferred Roaming List 
   (PRL) in use by the device

PARAMETERS:
   pPRLVersion [ O ] - The PRL version number
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPRLVersion( WORD * pPRLVersion )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPRLVersion( pPRLVersion );
}

/*===========================================================================
METHOD:
   GetERIFile

DESCRIPTION:
   This command returns the ERI file that is stored in EFS on the device

PARAMETERS:
   pFileSize   [I/O] - Upon input the maximum number of bytes that the file 
                       contents array can contain.  Upon successful output 
                       the actual number of bytes written to the file contents 
                       array
   pFile       [ O ] - The file contents
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetERIFile( 
   ULONG *                    pFileSize, 
   BYTE *                     pFile )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetERIFile( pFileSize, pFile );
}

/*===========================================================================
METHOD:
   ActivateAutomatic

DESCRIPTION:
   This function requests the device to perform automatic service activation

PARAMETERS:
   pActivationCode   [ I ] - Activation code (maximum string length of 12) 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ActivateAutomatic( CHAR * pActivationCode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->ActivateAutomatic( pActivationCode );
}

/*===========================================================================
METHOD:
   ActivateManual

DESCRIPTION:
   This function requests the device perform manual service activation

PARAMETERS:
   pSPC        [ I ] - NULL terminated string representing the six digit 
                       service programming code
   sid         [ I ] - System identification number
   pMDN        [ I ] - Mobile Directory Number string
   pMIN        [ I ] - Mobile Identification Number string
   prlSize     [ I ] - (Optional) Size of PRL file array
   pPRL        [ I ] - (Optional) The PRL file contents
   pMNHA       [ I ] - (Optional) MN-HA string
   pMNAAA      [ I ] - (Optional) MN-AAA string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ActivateManual( 
   CHAR *                     pSPC,
   WORD                       sid, 
   CHAR *                     pMDN,
   CHAR *                     pMIN, 
   ULONG                      prlSize, 
   BYTE *                     pPRL, 
   CHAR *                     pMNHA,
   CHAR *                     pMNAAA )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->ActivateManual( pSPC,
                                   sid,
                                   pMDN,
                                   pMIN,
                                   prlSize,
                                   pPRL,
                                   pMNHA,
                                   pMNAAA );
}

/*===========================================================================
METHOD:
   ResetToFactoryDefaults

DESCRIPTION:
   This function requests the device reset configuration to factory defaults

PARAMETERS:
   pSPC        [ I ] - NULL terminated string representing the six digit 
                       service programming code

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ResetToFactoryDefaults( CHAR * pSPC )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->ResetToFactoryDefaults( pSPC );
}

/*===========================================================================
METHOD:
   GetActivationState

DESCRIPTION:
   This function returns the device activation state

PARAMETERS:
   pActivationState  [ O ] - Service activation state
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetActivationState( ULONG * pActivationState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetActivationState( pActivationState );
}

/*===========================================================================
METHOD:
   SetPower

DESCRIPTION:
   This function sets the operating mode of the device

PARAMETERS:
   powerMode   [ I ] - Selected operating mode
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPower( ULONG powerMode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->SetPower( powerMode );
}

/*===========================================================================
METHOD:
   GetPower

DESCRIPTION:
   This function returns the operating mode of the device

PARAMETERS:
   pPowerMode  [ O ] - Current operating mode
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPower( ULONG * pPowerMode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPower( pPowerMode );
}

/*===========================================================================
METHOD:
   GetOfflineReason

DESCRIPTION:
   This function returns the reason why the operating mode of the device
   is currently offline

PARAMETERS:
   pReasonMask [ O ] - Bitmask of offline reasons
   pbPlatform  [ O ] - Offline due to being platform retricted?
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetOfflineReason( 
   ULONG *                    pReasonMask,
   ULONG *                    pbPlatform )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetOfflineReason( pReasonMask, pbPlatform );
}

/*===========================================================================
METHOD:
   GetNetworkTime

DESCRIPTION:
   This function returns the current time of the device

PARAMETERS:
   pTimeStamp  [ O ] - Count of 1.25ms that have elapsed from the start 
                       of GPS time (Jan 6, 1980)
   pTimeSource [ O ] - Source of the timestamp

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetNetworkTime( 
   ULONGLONG *                pTimeCount,  
   ULONG *                    pTimeSource )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetNetworkTime( pTimeCount, pTimeSource );
}

/*===========================================================================
METHOD:
   ValidateSPC

DESCRIPTION:
   This function validates the service programming code

PARAMETERS:
   pSPC        [ I ] - Six digit service programming code

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ValidateSPC( CHAR * pSPC )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->ValidateSPC( pSPC );
}

/*===========================================================================
METHOD:
   DeleteSMS

DESCRIPTION:
   This function deletes one or more SMS messages from device memory

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   pMessageIndex  [ I ] - (Optional) message index
   pMessageTag    [ I ] - (Optional) message tag
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DeleteSMS( 
   ULONG                      storageType, 
   ULONG *                    pMessageIndex, 
   ULONG *                    pMessageTag )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->DeleteSMS( storageType, pMessageIndex, pMessageTag );
}

/*===========================================================================
METHOD:
   GetSMSList

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
   ULONG - Return code
===========================================================================*/
ULONG GetSMSList( 
   ULONG                      storageType, 
   ULONG *                    pRequestedTag,
   ULONG *                    pMessageListSize, 
   BYTE *                     pMessageList )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSMSList( storageType,
                                      pRequestedTag,
                                      pMessageListSize,
                                      pMessageList );
}

/*===========================================================================
METHOD:
   GetSMS

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
   ULONG - Return code
===========================================================================*/
ULONG GetSMS( 
   ULONG                      storageType, 
   ULONG                      messageIndex, 
   ULONG *                    pMessageTag,
   ULONG *                    pMessageFormat,
   ULONG *                    pMessageSize, 
   BYTE *                     pMessage )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSMS( storageType,
                                  messageIndex,
                                  pMessageTag,
                                  pMessageFormat,
                                  pMessageSize,
                                  pMessage );
}

/*===========================================================================
METHOD:
   ModifySMSStatus

DESCRIPTION:
   This function modifies the status of an SMS message saved in storage on 
   the device

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   messageIndex   [ I ] - Message index
   messageTag     [ I ] - Message tag
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ModifySMSStatus( 
   ULONG                      storageType, 
   ULONG                      messageIndex, 
   ULONG                      messageTag )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return pAPI->ModifySMSStatus( storageType, messageIndex, messageTag );
}

/*===========================================================================
METHOD:
   SaveSMS

DESCRIPTION:
   This function saves an SMS message to device memory

PARAMETERS:
   storageType    [ I ] - SMS message storage type
   messageFormat  [ I ] - Message format   
   messageSize    [ I ] - The length of the message contents in bytes
   pMessage       [ I ] - The message contents
   pMessageIndex  [ O ] - The message index assigned by the device
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SaveSMS(    
   ULONG                      storageType, 
   ULONG                      messageFormat, 
   ULONG                      messageSize, 
   BYTE *                     pMessage, 
   ULONG *                    pMessageIndex )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SaveSMS( storageType,
                                   messageFormat,
                                   messageSize,
                                   pMessage,
                                   pMessageIndex );
}

/*===========================================================================
METHOD:
   SendSMS

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
   ULONG - Return code
===========================================================================*/
ULONG SendSMS(    
   ULONG                      messageFormat, 
   ULONG                      messageSize, 
   BYTE *                     pMessage, 
   ULONG *                    pMessageFailureCode )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SendSMS( messageFormat,
                                   messageSize,
                                   pMessage,
                                   pMessageFailureCode );
}

/*===========================================================================
METHOD:
   GetSMSCAddress

DESCRIPTION:
   This function returns the SMS center address

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
   ULONG - Return code
===========================================================================*/
ULONG GetSMSCAddress( 
   BYTE                       addressSize,
   CHAR *                     pSMSCAddress,
   BYTE                       typeSize,
   CHAR *                     pSMSCType )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSMSCAddress( addressSize,
                                          pSMSCAddress,
                                          typeSize,
                                          pSMSCType );
}

/*===========================================================================
METHOD:
   SetSMSCAddress

DESCRIPTION:
   This function sets the SMS center address

PARAMETERS:
   pSMSCAddress   [ 0 ] - The SMS center address represented as a NULL 
                          terminated string
   pSMSCType      [ 0 ] - The SMS center address type represented as a NULL 
                          terminated string (optional)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetSMSCAddress( 
   CHAR *                     pSMSCAddress, 
   CHAR *                     pSMSCType )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetSMSCAddress( pSMSCAddress, pSMSCType );
}

/*===========================================================================
METHOD:
   GetSMSRoutes

DESCRIPTION:
   This function gets the current incoming SMS routing information

PARAMETERS:
   pRouteSize  [I/O] - Upon input the maximum number of elements that the 
                       SMS route array can contain.  Upon succes the actual 
                       number of elements in the SMS route array
   pRoutes     [ O ] - The SMS route array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSMSRoutes( 
   BYTE *                     pRouteSize, 
   BYTE *                     pRoutes )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSMSRoutes( pRouteSize, pRoutes );
}

/*===========================================================================
METHOD:
   SetSMSRoutes

DESCRIPTION:
   This function sets the desired incoming SMS routing information

PARAMETERS:
   pRouteSize  [ I ] - The number of elements in the SMS route array
   pRoutes     [ I ] - The SMS route array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetSMSRoutes( 
   BYTE *                     pRouteSize, 
   BYTE *                     pRoutes )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetSMSRoutes( pRouteSize, pRoutes );
}

/*===========================================================================
METHOD:
   UIMSetPINProtection

DESCRIPTION:
   This function enables or disables protection of UIM contents by a 
   given PIN

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   bEnable              [ I ] - Enable/disable PIN protection (0 = disable)?
   pValue               [ I ] - PIN value of the PIN to be enabled/disabled
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate 
                                the number of retries left, after which the 
                                PIN will be blocked (0xFFFFFFFF = unknown)
   pUnblockRetriesLeft  [ O ] - Upon operational failure this will indicate 
                                the number of unblock retries left, after 
                                which the PIN will be permanently blocked, 
                                i.e. UIM is unusable (0xFFFFFFFF = unknown)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSetPINProtection( 
   ULONG                      id, 
   ULONG                      bEnable,
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMSetPINProtection( id,
                                               bEnable,
                                               pValue,
                                               pVerifyRetriesLeft,
                                               pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMVerifyPIN

DESCRIPTION:
   This function verifies the PIN before accessing the UIM contents

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   pValue               [ I ] - PIN value of the PIN to verify
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate 
                                the number of retries left, after which the 
                                PIN will be blocked (0xFFFFFFFF = unknown)
   pUnblockRetriesLeft  [ O ] - Upon operational failure this will indicate 
                                the number of unblock retries left, after 
                                which the PIN will be permanently blocked, 
                                i.e. UIM is unusable (0xFFFFFFFF = unknown)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMVerifyPIN( 
   ULONG                      id, 
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMVerifyPIN( id,
                                        pValue,
                                        pVerifyRetriesLeft,
                                        pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMUnblockPIN

DESCRIPTION:
   This function unblocks a blocked PIN

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   pPUKValue            [ I ] - PUK value of the PIN to unblock
   pNewValue            [ I ] - New PIN value of the PIN to unblock
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate 
                                the number of retries left, after which the 
                                PIN will be blocked (0xFFFFFFFF = unknown)
   pUnblockRetriesLeft  [ O ] - Upon operational failure this will indicate 
                                the number of unblock retries left, after 
                                which the PIN will be permanently blocked, 
                                i.e. UIM is unusable (0xFFFFFFFF = unknown)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMUnblockPIN( 
   ULONG                      id, 
   CHAR *                     pPUKValue,
   CHAR *                     pNewValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMUnblockPIN( id,
                                         pPUKValue,
                                         pNewValue,
                                         pVerifyRetriesLeft,
                                         pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMChangePIN

DESCRIPTION:
   This function change the PIN value

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   pOldValue            [ I ] - Old PIN value of the PIN to change
   pNewValue            [ I ] - New PIN value of the PIN to change
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate 
                                the number of retries left, after which the 
                                PIN will be blocked (0xFFFFFFFF = unknown)
   pUnblockRetriesLeft  [ O ] - Upon operational failure this will indicate 
                                the number of unblock retries left, after 
                                which the PIN will be permanently blocked, 
                                i.e. UIM is unusable (0xFFFFFFFF = unknown)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMChangePIN( 
   ULONG                      id, 
   CHAR *                     pOldValue,
   CHAR *                     pNewValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMChangePIN( id,
                                        pOldValue,
                                        pNewValue,
                                        pVerifyRetriesLeft,
                                        pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMGetPINStatus

DESCRIPTION:
   This function returns the status of the pin

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   pStatus              [ O ] - PIN status (0xFFFFFFFF = unknown)
   pVerifyRetriesLeft   [ O ] - The number of retries left, after which the 
                                PIN will be blocked (0xFFFFFFFF = unknown)
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the PIN will be permanently blocked, 
                                i.e. UIM is unusable (0xFFFFFFFF = unknown)
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetPINStatus( 
   ULONG                      id, 
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMGetPINStatus( id,
                                           pStatus,
                                           pVerifyRetriesLeft,
                                           pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMGetICCID

DESCRIPTION:
   This function returns the UIM ICCID

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetICCID( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMGetICCID( stringSize, pString );
}

/*===========================================================================
METHOD:
   UIMGetControlKeyStatus

DESCRIPTION:
   This function returns the status of the specified facility control key

PARAMETERS:
   id                   [ I ] - Facility ID
   pStatus              [ O ] - Control key status
   pVerifyRetriesLeft   [ O ] - The number of retries left, after which the 
                                control key will be blocked 
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked 
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetControlKeyStatus( 
   ULONG                      id, 
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMGetControlKeyBlockingStatus( id, 
                                                          pStatus, 
                                                          pVerifyRetriesLeft, 
                                                          pUnblockRetriesLeft,
                                                          0 );
}

/*===========================================================================
METHOD:
   UIMGetControlKeyBlockingStatus

DESCRIPTION:
   This function returns the blocking status of the specified facility 
   control key

PARAMETERS:
   id                   [ I ] - Facility ID
   pStatus              [ O ] - Control key status
   pVerifyRetriesLeft   [ O ] - The number of retries left, after which the 
                                control key will be blocked 
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked 
   pbBlocking           [ O ] - (Optional) Is the facility blocking? 
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetControlKeyBlockingStatus( 
   ULONG                      id, 
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft,
   ULONG *                    pbBlocking )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMGetControlKeyBlockingStatus( id, 
                                                          pStatus, 
                                                          pVerifyRetriesLeft, 
                                                          pUnblockRetriesLeft,
                                                          pbBlocking);
}

/*===========================================================================
METHOD:
   UIMSetControlKeyProtection

DESCRIPTION:
   This function changes the specified facility control key

PARAMETERS:
   id                   [ I ] - Facility ID
   status               [ I ] - Control key status
   pValue               [ I ] - Control key de-personalization string
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate 
                                the number of retries left, after which the 
                                control key will be blocked 
                                (0xFFFFFFFF = unknown)
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSetControlKeyProtection( 
   ULONG                      id, 
   ULONG                      status,
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMSetControlKeyProtection( id, 
                                                      status, 
                                                      pValue,
                                                      pVerifyRetriesLeft );
}

/*===========================================================================
METHOD:
   UIMUnblockControlKey

DESCRIPTION:
   This function unblocks the specified facility control key

PARAMETERS:
   id                   [ I ] - Facility ID
   pValue               [ I ] - Control key de-personalization string
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked 
                                (0xFFFFFFFF = unknown)
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMUnblockControlKey( 
   ULONG                      id, 
   CHAR *                     pValue,
   ULONG *                    pUnblockRetriesLeft )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UIMUnblockControlKey( id, 
                                                pValue,
                                                pUnblockRetriesLeft );
}

/*===========================================================================
METHOD:
   GetPDSState

DESCRIPTION:
   This function returns the current PDS state

PARAMETERS:
   pEnabled    [ O ] - Current PDS state (0 = disabled)
   pTracking   [ O ] - Current PDS tracking session state


RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPDSState( 
   ULONG *                    pEnabled,
   ULONG *                    pTracking )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPDSState( pEnabled, pTracking );
}

/*===========================================================================
METHOD:
   SetPDSState

DESCRIPTION:
   This function sets the PDS state

PARAMETERS:
   enable      [ I ] - Desired PDS state (0 = disable)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPDSState( ULONG enable )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPDSState( enable );
}

/*===========================================================================
METHOD:
   PDSInjectTimeReference

DESCRIPTION:
   This function injects a system time into the PDS engine

PARAMETERS:
   sysTime              [ I ] - System time
   sysDiscontinuities   [ I ] - Number of system time discontinuities
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectTimeReference( 
   ULONGLONG                  systemTime,
   USHORT                     systemDiscontinuities )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->PDSInjectTimeReference( systemTime, 
                                                  systemDiscontinuities );
}

/*===========================================================================
METHOD:
   GetPDSDefaults

DESCRIPTION:
   This function returns the default tracking session configuration

PARAMETERS:
   pOperation  [ O ] - Current session operating mode 
   pTimeout    [ O ] - Maximum amount of time (seconds) to work on each fix
   pInterval   [ O ] - Interval (milliseconds) between fix requests
   pAccuracy   [ O ] - Current accuracy threshold (meters)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPDSDefaults( 
   ULONG *                    pOperation,
   BYTE *                     pTimeout,
   ULONG *                    pInterval,
   ULONG *                    pAccuracy )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPDSDefaults( pOperation, 
                                          pTimeout, 
                                          pInterval, 
                                          pAccuracy );
}

/*===========================================================================
METHOD:
   SetPDSDefaults

DESCRIPTION:
   This function sets the default tracking session configuration

PARAMETERS:
   operation   [ I ] - Desired session operating mode 
   timeout     [ I ] - Maximum amount of time (seconds) to work on each fix
   interval    [ I ] - Interval (milliseconds) between fix requests
   accuracy    [ I ] - Desired accuracy threshold (meters)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPDSDefaults( 
   ULONG                      operation,
   BYTE                       timeout,
   ULONG                      interval,
   ULONG                      accuracy )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPDSDefaults( operation, 
                                          timeout, 
                                          interval, 
                                          accuracy );
}

/*===========================================================================
METHOD:
   GetXTRAAutomaticDownload

DESCRIPTION:
   This function returns the XTRA automatic download configuration

PARAMETERS:
   pbEnabled   [ O ] - Automatic download enabled?
   pInterval   [ O ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetXTRAAutomaticDownload( 
   ULONG *                    pbEnabled,
   USHORT *                   pInterval )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetXTRAAutomaticDownload( pbEnabled, pInterval );
}

/*===========================================================================
METHOD:
   SetXTRAAutomaticDownload

DESCRIPTION:
   This function sets the XTRA automatic download configuration

PARAMETERS:
   bEnabled    [ I ] - Automatic download enabled?
   interval    [ I ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetXTRAAutomaticDownload( 
   ULONG                      bEnabled,
   USHORT                     interval )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetXTRAAutomaticDownload( bEnabled, interval );
}

/*===========================================================================
METHOD:
   GetXTRANetwork

DESCRIPTION:
   This function returns the XTRA WWAN network preference

PARAMETERS:
   pPreference [ O ] - XTRA WWAN network preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetXTRANetwork( ULONG * pPreference )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetXTRANetwork( pPreference );
}

/*===========================================================================
METHOD:
   SetXTRANetwork

DESCRIPTION:
   This function sets the XTRA WWAN network preference

PARAMETERS:
   preference  [ I ] - XTRA WWAN network preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetXTRANetwork( ULONG preference )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetXTRANetwork( preference );
}

/*===========================================================================
METHOD:
   GetXTRAValidity

DESCRIPTION:
   This function returns the XTRA database validity period

PARAMETERS:
   pGPSWeek       [ O ] - Starting GPS week of validity period
   pGPSWeekOffset [ O ] - Starting GPS week offset (minutes) of validity period
   pDuration      [ O ] - Length of validity period (hours)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetXTRAValidity(
   USHORT *                   pGPSWeek,
   USHORT *                   pGPSWeekOffset, 
   USHORT *                   pDuration )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetXTRAValidity( pGPSWeek, 
                                           pGPSWeekOffset, 
                                           pDuration );
}

/*===========================================================================
METHOD:
   ForceXTRADownload

DESCRIPTION:
   This function forces the XTRA database to be downloaded to the device

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ForceXTRADownload()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->ForceXTRADownload();
}

/*===========================================================================
METHOD:
   GetXTRADataState

DESCRIPTION:
   This function returns the XTRA data positioning state

PARAMETERS:
   pState      [ O ] - XTRA data positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetXTRADataState( ULONG * pState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetXTRADataState( pState );
}

/*===========================================================================
METHOD:
   SetXTRADataState

DESCRIPTION:
   This function sets the XTRA data positioning state

PARAMETERS:
   state       [ I ] - XTRA data positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetXTRADataState( ULONG state )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetXTRADataState( state );
}

/*===========================================================================
METHOD:
   GetXTRATimeState

DESCRIPTION:
   This function returns the XTRA time positioning state

PARAMETERS:
   pState      [ O ] - XTRA time positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetXTRATimeState( ULONG * pState )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetXTRATimeState( pState );
}

/*===========================================================================
METHOD:
   SetXTRATimeState

DESCRIPTION:
   This function sets the XTRA time positioning state

PARAMETERS:
   state       [ I ] - XTRA time positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetXTRATimeState( ULONG state )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetXTRATimeState( state );
}

/*===========================================================================
METHOD:
   GetAGPSConfig

DESCRIPTION:
   This function returns the PDS AGPS configuration

PARAMETERS:
   pServerAddress [ O ] - IPv4 address of AGPS server
   pServerPort    [ O ] - Port number of AGPS server

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetAGPSConfig( 
   ULONG *                    pServerAddress,
   ULONG *                    pServerPort )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetAGPSConfig( pServerAddress, pServerPort );
}

/*===========================================================================
METHOD:
   SetAGPSConfig

DESCRIPTION:
   This function sets the PDS AGPS configuration

PARAMETERS:
   serverAddress [ I ] - IPv4 address of AGPS server
   serverPort    [ I ] - Port number of AGPS server

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetAGPSConfig( 
   ULONG                      serverAddress,
   ULONG                      serverPort )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetAGPSConfig( serverAddress, serverPort );
}

/*===========================================================================
METHOD:
   GetServiceAutomaticTracking

DESCRIPTION:
   This function returns the automatic tracking state for the service

PARAMETERS:
   pbAuto      [ O ] - Automatic tracking session started for service?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetServiceAutomaticTracking( ULONG * pbAuto )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetServiceAutomaticTracking( pbAuto );
}

/*===========================================================================
METHOD:
   SetServiceAutomaticTracking

DESCRIPTION:
   This function sets the automatic tracking state for the service

PARAMETERS:
   pbAuto      [ I ] - Start automatic tracking session for service?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetServiceAutomaticTracking( ULONG bAuto )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetServiceAutomaticTracking( bAuto );
}

/*===========================================================================
METHOD:
   GetPortAutomaticTracking

DESCRIPTION:
   This function returns the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   pbAuto      [ O ] - Automatic tracking enabled for NMEA COM port?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetPortAutomaticTracking( ULONG * pbAuto )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetPortAutomaticTracking( pbAuto );
}

/*===========================================================================
METHOD:
   SetPortAutomaticTracking

DESCRIPTION:
   This function sets the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   pbAuto      [ I ] - Enable automatic tracking for NMEA COM port?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPortAutomaticTracking( ULONG bAuto )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPortAutomaticTracking( bAuto );
}

/*===========================================================================
METHOD:
   ResetPDSData

DESCRIPTION:
   This function resets the specified PDS data

PARAMETERS:
   pGPSDataMask   [ I ] - Bitmask of GPS data to clear (optional)
   pCellDataMask  [ I ] - Bitmask of cell data to clear (optional)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ResetPDSData( 
   ULONG *                    pGPSDataMask,
   ULONG *                    pCellDataMask )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->ResetPDSData( pGPSDataMask, pCellDataMask );
}

/*===========================================================================
METHOD:
   CATSendTerminalResponse

DESCRIPTION:
   This function sends the terminal response to the device

PARAMETERS:
   refID       [ I ] - UIM reference ID (from CAT event)
   dataLen     [ I ] - Terminal response data length
   pData       [ I ] - Terminal response data

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSendTerminalResponse( 
   ULONG                      refID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->CATSendTerminalResponse( refID, dataLen, pData );
}

/*===========================================================================
METHOD:
   CATSendEnvelopeCommand

DESCRIPTION:
   This function sends the envelope command to the device

PARAMETERS:
   cmdID       [ I ] - Envelope command ID
   dataLen     [ I ] - Envelope command data length
   pData       [ I ] - Envelope command data

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSendEnvelopeCommand( 
   ULONG                      cmdID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->CATSendEnvelopeCommand( cmdID, dataLen, pData );
}

/*===========================================================================
METHOD:
   GetSMSWake

DESCRIPTION:
   This function queries the state of the SMS wake functionality

PARAMETERS:
   pbEnabled   [ O ] - SMS wake functionality enabled?
   pWakeMask   [ O ] - SMS wake mask (only relevant when enabled)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetSMSWake( 
   ULONG *                    pbEnabled,
   ULONG *                    pWakeMask )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetSMSWake( pbEnabled, pWakeMask );
}

/*===========================================================================
METHOD:
   SetSMSWake

DESCRIPTION:
   This function enables/disables the SMS wake functionality

PARAMETERS:
   bEnable     [ I ] - Enable SMS wake functionality?
   wakeMask    [ I ] - SMS wake mask (only relevant when enabling)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetSMSWake( 
   ULONG                      bEnable,
   ULONG                      wakeMask )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetSMSWake( bEnable, wakeMask );
}

/*===========================================================================
METHOD:
   OMADMStartSession

DESCRIPTION:
   This function starts an OMA-DM session

PARAMETERS:
   sessionType [ I ] - Type of session to initiate

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMStartSession( ULONG sessionType )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMStartSession( sessionType );
}

/*===========================================================================
METHOD:
   OMADMCancelSession

DESCRIPTION:
   This function cancels an ongoing OMA-DM session

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMCancelSession()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMCancelSession();
}

/*===========================================================================
METHOD:
   OMADMGetSessionInfo

DESCRIPTION:
   This function returns information related to the current (or previous
   if no session is active) OMA-DM session

PARAMETERS:
   pSessionState  [ O ] - State of session
   pSessionType   [ O ] - Type of session
   pFailureReason [ O ] - Session failure reason (when state indicates failure)
   pRetryCount    [ O ] - Session retry count (when state indicates retrying)
   pSessionPause  [ O ] - Session pause timer (when state indicates retrying)
   pTimeRemaining [ O ] - Pause time remaining (when state indicates retrying)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMGetSessionInfo( 
   ULONG *                    pSessionState,
   ULONG *                    pSessionType,
   ULONG *                    pFailureReason,
   BYTE *                     pRetryCount,
   WORD *                     pSessionPause,
   WORD *                     pTimeRemaining )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMGetSessionInfo( pSessionState,
                                               pSessionType,
                                               pFailureReason,
                                               pRetryCount,
                                               pSessionPause,
                                               pTimeRemaining );
}

/*===========================================================================
METHOD:
   OMADMGetPendingNIA

DESCRIPTION:
   This function returns information about the pending network initiated
   alert

PARAMETERS:
   pSessionType   [ O ] - Type of session
   pSessionID     [ O ] - Unique session ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMGetPendingNIA( 
   ULONG *                    pSessionType,
   USHORT *                   pSessionID )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMGetPendingNIA( pSessionType, pSessionID );
}

/*===========================================================================
METHOD:
   OMADMSendSelection

DESCRIPTION:
   This function sends the specified OMA-DM selection for the current 
   network initiated session

PARAMETERS:
   selection   [ I ] - Selection
   sessionID   [ I ] - Unique session ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMSendSelection( 
   ULONG                      selection,
   USHORT                     sessionID )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMSendSelection( selection, sessionID );
}

/*===========================================================================
METHOD:
   OMADMGetFeatureSettings

DESCRIPTION:
   This function returns the OMA-DM feature settings

PARAMETERS:
   pbProvisioning [ O ] - Device provisioning service update enabled
   pbPRLUpdate    [ O ] - PRL service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMGetFeatureSettings( 
   ULONG *                    pbProvisioning,
   ULONG *                    pbPRLUpdate )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMGetFeatureSettings( pbProvisioning,
                                                   pbPRLUpdate );
}

/*===========================================================================
METHOD:
   OMADMSetProvisioningFeature

DESCRIPTION:
   This function sets the OMA-DM device provisioning service 
   update feature setting

PARAMETERS:
   bProvisioning  [ I ] - Device provisioning service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMSetProvisioningFeature( 
   ULONG                      bProvisioning )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMSetProvisioningFeature( bProvisioning );
}

/*===========================================================================
METHOD:
   OMADMSetPRLUpdateFeature

DESCRIPTION:
   This function sets the OMA-DM PRL service update feature setting

PARAMETERS:
   bPRLUpdate     [ I ] - PRL service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMADMSetPRLUpdateFeature( 
   ULONG                      bPRLUpdate )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OMADMSetPRLUpdateFeature( bPRLUpdate );
}

/*===========================================================================
METHOD:
   OriginateUSSD

DESCRIPTION:
   This function initiates a USSD operation

PARAMETERS:
   pInfo          [ I ] - USSD information

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OriginateUSSD( BYTE * pInfo )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->OriginateUSSD( pInfo );
}

/*===========================================================================
METHOD:
   AnswerUSSD

DESCRIPTION:
   This function responds to a USSD request from the network

PARAMETERS:
   pInfo          [ I ] - USSD information

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AnswerUSSD( BYTE * pInfo )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->AnswerUSSD( pInfo );
}

/*===========================================================================
METHOD:
   CancelUSSD

DESCRIPTION:
   This function cancels an in-progress USSD operation

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CancelUSSD()
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->CancelUSSD();
}

/*===========================================================================
METHOD:
   UpgradeFirmware

DESCRIPTION:
   This function performs the following set of steps:
      a)   Verifies arguments
      b)   Updates firmware ID on device
      c)   Resets the device

   NOTE: Upon successful completion the above steps will have been completed, 
         however the actual upgrade of the firmware will necessarily then 
         follow.

PARAMETERS:
   pDestinationPath  [ I ] - The fully qualified path to the destination folder 
                             that the firmware download service will use 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UpgradeFirmware( CHAR * pDestinationPath )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->UpgradeFirmware( pDestinationPath );
}

/*===========================================================================
METHOD:
   GetImageInfo

DESCRIPTION:
   Returns image information obtained from the firmware image located at the
   provided path

PARAMETERS:
   pPath          [ I ] - Location of the firmware image
   pFirmwareID    [ O ] - Firmware ID obtained from the firmware image
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetImageInfo( 
   CHAR *                     pPath, 
   ULONG *                    pFirmwareID,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetImageInfo( pPath, 
                                     pFirmwareID,
                                     pTechnology,
                                     pCarrier,
                                     pRegion,
                                     pGPSCapability );
}

/*===========================================================================
METHOD:
   GetImageStore

DESCRIPTION:
   Returns the image store folder, i.e. the folder co-located with the
   QDL Service executable which (by default) contains one or more carrier 
   specific image subfolders

PARAMETERS:
   pathSize          [ I ] - Maximum number of characters (including NULL
                             terminator) that can be copied to the image
                             store path array
   pImageStorePath   [ O ] - The path to the image store


RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetImageStore( 
   WORD                       pathSize,
   CHAR *                     pImageStorePath )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->GetImageStore( pathSize, pImageStorePath ); 
}

/*===========================================================================
METHOD:
   SetSessionStateCallback

DESCRIPTION:
   This function enables/disables the session state callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetSessionStateCallback( tFNSessionState pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetSessionStateCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetByteTotalsCallback

DESCRIPTION:
   This function enables/disables the RX/TX byte counts callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)
   interval    [ I ] - Interval in seconds (ignored when disabling)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetByteTotalsCallback( 
   tFNByteTotals              pCallback,
   BYTE                       interval )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetByteTotalsCallback( pCallback, interval );
}

/*===========================================================================
METHOD:
   SetDataCapabilitiesCallback

DESCRIPTION:
   This function enables/disables the serving system data capabilities 
   callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDataCapabilitiesCallback( 
   tFNDataCapabilities        pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetDataCapabilitiesCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetDataBearerCallback

DESCRIPTION:
   This function enables/disables the data bearer status callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDataBearerCallback( tFNDataBearer pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetDataBearerCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetDormancyStatusCallback

DESCRIPTION:
   This function enables/disables the dormancy status callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDormancyStatusCallback( 
   tFNDormancyStatus          pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetDormancyStatusCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetMobileIPStatusCallback

DESCRIPTION:
   This function enables/disables the mobile IP status callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetMobileIPStatusCallback( 
   tFNMobileIPStatus          pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetMobileIPStatusCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetActivationStatusCallback

DESCRIPTION:
   This function enables/disables the activation status callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetActivationStatusCallback( 
   tFNActivationStatus        pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetActivationStatusCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetPowerCallback

DESCRIPTION:
   Enable/disable power operating mode callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPowerCallback( tFNPower pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPowerCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetWirelessDisableCallback

DESCRIPTION:
   Enable/disable wireless disable state callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetWirelessDisableCallback( 
   tFNWirelessDisable         pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetWirelessDisableCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetRoamingIndicatorCallback

DESCRIPTION:
   This function enables/disables the roaming indicator callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetRoamingIndicatorCallback( 
   tFNRoamingIndicator        pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetRoamingIndicatorCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetSignalStrengthCallback

DESCRIPTION:
   This function enables/disables the signal strength callback function

PARAMETERS:
   pCallback      [ I ] - Callback function (0 = disable)
   thresholdsSize [ I ] - Number of elements the threshold array contain
                          (a maximum of 5 thresholds is supported), must
                          be 0 when disabling the callback
   pThresholds    [ I ] - Signal threshold array (each entry in dBm),
                          must be 0 when disabling the callback

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetSignalStrengthCallback( 
   tFNSignalStrength          pCallback,
   BYTE                       thresholdsSize,
   INT8 *                     pThresholds )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   if (thresholdsSize > 0 && pThresholds == 0)
   {
      return (ULONG)eGOBI_ERR_INVALID_ARG;
   }

   std::list <INT8> thresholdsList;
   for (BYTE t = 0; t < thresholdsSize; t++)
   {
      thresholdsList.push_back( pThresholds[t] );
   }

   return (ULONG)pAPI->SetSignalStrengthCallback( pCallback, thresholdsList );
}

/*===========================================================================
METHOD:
   SetRFInfoCallback

DESCRIPTION:
   This function enables/disables the RF information callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetRFInfoCallback( tFNRFInfo pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetRFInfoCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetLURejectCallback

DESCRIPTION:
   This function enables/disables the LU reject callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetLURejectCallback( tFNLUReject pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetLURejectCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetPLMNModeCallback

DESCRIPTION:
   Enable/disable PLMN mode callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPLMNModeCallback( tFNPLMNMode pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPLMNModeCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetNewSMSCallback

DESCRIPTION:
   This function enables/disables the new SMS callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetNewSMSCallback( tFNNewSMS pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetNewSMSCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetNMEACallback

DESCRIPTION:
   This function enables/disables the NMEA sentence callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetNMEACallback( tFNNewNMEA pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetNMEACallback( pCallback );
}

/*===========================================================================
METHOD:
   SetPDSStateCallback

DESCRIPTION:
   This function enables/disables the PDS service state callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetPDSStateCallback( tFNPDSState pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetPDSStateCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetCATEventCallback

DESCRIPTION:
   This function enables/disables the CAT event callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)
   eventMask   [ I ] - Bitmask of CAT events to register for
   pErrorMask  [ O ] - Error bitmask

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetCATEventCallback( 
   tFNCATEvent                pCallback,
   ULONG                      eventMask,
   ULONG *                    pErrorMask )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetCATEventCallback( pCallback, eventMask, pErrorMask );
}

/*===========================================================================
METHOD:
   SetOMADMAlertCallback

DESCRIPTION:
   This function enables/disables the OMA-DM network initiated alert
   callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetOMADMAlertCallback( tFNOMADMAlert pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetOMADMAlertCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetOMADMStateCallback

DESCRIPTION:
   This function enables/disables the OMA-DM state callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetOMADMStateCallback( tFNOMADMState pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetOMADMStateCallback( pCallback );
}

#ifdef VOICE_SUPPORT
/*===========================================================================
METHOD:
   SetUSSDReleaseCallback

DESCRIPTION:
   Enable/disable USSD release callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetUSSDReleaseCallback( 
   tFNUSSDRelease             pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetUSSDReleaseCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetUSSDNotificationCallback

DESCRIPTION:
   Enable/disable USSD notification callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetUSSDNotificationCallback( 
   tFNUSSDNotification        pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetUSSDNotificationCallback( pCallback );
}

/*===========================================================================
METHOD:
   SetUSSDOriginationCallback

DESCRIPTION:
   Enable/disable USSD origination callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetUSSDOriginationCallback( 
   tFNUSSDOrigination         pCallback )
{
   cGobiConnectionMgmt * pAPI = gConnectionDLL.GetAPI();
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetUSSDOriginationCallback( pCallback );
}
#endif

