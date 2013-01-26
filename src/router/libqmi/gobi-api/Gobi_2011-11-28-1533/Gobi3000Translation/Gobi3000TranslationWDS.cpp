/*===========================================================================
FILE:
   Gobi3000TranslationWDS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (WDS Service)

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

/*===========================================================================
METHOD:
   ParseGetSessionState

DESCRIPTION:
   This function returns the state of the current packet data session

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pState            [ O ] - State of the current packet session

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSessionState(
   ULONG          inLen,
   const BYTE *   pIn,
   ULONG *        pState )
{
   // Validate arguments
   if (pIn == 0 || pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sWDSGetPacketServiceStatusResponse_Status * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetPacketServiceStatusResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pState = pTLVx01->mConnectionStatus;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSessionDuration

DESCRIPTION:
   This function returns the duration of the current packet data session

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pDuration   [ O ] - Duration of the current packet session

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSessionDuration(
   ULONG          inLen,
   const BYTE *   pIn,
   ULONGLONG *    pDuration )
{
      // Validate arguments
   if (pIn == 0 || pDuration == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sWDSGetDataSessionDurationResponse_Duration * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetDataSessionDurationResponse_Duration ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pDuration = pTLVx01->mDataSessionDuration;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetDormancyState

DESCRIPTION:
   This function returns the dormancy state of the current packet
   data session (when connected)

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pState      [ O ] - Dormancy state of the current packet session

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetDormancyState(
   ULONG          inLen,
   const BYTE *   pIn,
   ULONG *        pState )
{
   // Validate arguments
   if (pIn == 0 || pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sWDSGetDormancyResponse_DormancyStatus * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetDormancyResponse_DormancyStatus ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pState = pTLVx01->mDormancyStatus;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetEnhancedAutoconnect

DESCRIPTION:
   This function returns the current autoconnect data session setting

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pSetting       [ O ] - NDIS autoconnect setting
   pRoamSetting   [ O ] - NDIS autoconnect roam setting

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetEnhancedAutoconnect(
   ULONG          inLen,
   const BYTE *   pIn,
   ULONG *        pSetting,
   ULONG *        pRoamSetting )
{
   // Validate arguments
   if (pIn == 0 || pSetting == 0 || pRoamSetting == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSetting = 0xffffffff;
   *pRoamSetting = 0xffffffff;

   // Find the first TLV
   const sWDSGetAutoconnectSettingResponse_Autoconnect * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetAutoconnectSettingResponse_Autoconnect ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pSetting = pTLVx01->mAutoconnectSetting;

   // Find the second TLV (optional)
   const sWDSGetAutoconnectSettingResponse_Roam * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      // Is the TLV large enough?
      if (outLenx10 < sizeof( sWDSGetAutoconnectSettingResponse_Roam ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRoamSetting = pTLVx10->mAutoconnectRoamSetting;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetEnhancedAutoconnect

DESCRIPTION:
   This function sets the autoconnect data session setting

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   setting        [ I ] - NDIS autoconnect setting
   pRoamSetting   [ I ] - (Optional) NDIS autoconnect roam setting


RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetEnhancedAutoconnect(
   ULONG *  pOutLen,
   BYTE *   pOut,
   ULONG    setting,
   ULONG *  pRoamSetting )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add setting

   // Check size
   WORD tlvx01Sz = sizeof( sWDSSetAutoconnectSettingRequest_Autoconnect );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSSetAutoconnectSettingRequest_Autoconnect * pTLVx01;
   pTLVx01 = (sWDSSetAutoconnectSettingRequest_Autoconnect*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mAutoconnectSetting = (eQMIWDSAutoconnectSettings)setting;

   offset += tlvx01Sz;

   // Add roam setting, if specified
   if (pRoamSetting != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWDSSetAutoconnectSettingRequest_Roam );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetAutoconnectSettingRequest_Roam * pTLVx10;
      pTLVx10 = (sWDSSetAutoconnectSettingRequest_Roam*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the value
      pTLVx10->mAutoconnectRoamSetting = (eQMIWDSAutoconnectRoamSettings)*pRoamSetting;

      offset += tlvx10Sz;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetDefaultProfile

DESCRIPTION:
   This function writes the default profile settings to the device, the
   default profile is used during autoconnect

   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
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
ULONG PackSetDefaultProfile(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
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
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add profileType

   // Check size
   WORD tlvx01Sz = sizeof( sWDSModifyProfileRequest_ProfileIdentifier );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSModifyProfileRequest_ProfileIdentifier * pTLVx01;
   pTLVx01 = (sWDSModifyProfileRequest_ProfileIdentifier*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mProfileType = (eQMIProfileTypes)profileType;
   pTLVx01->mProfileIndex = 1;

   offset += tlvx01Sz;

   // Add name, if specified
   if (pName != 0)
   {
      std::string name( pName );

      // Check size
      WORD tlvx10Sz = (WORD)name.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( pOut + offset, name.c_str(), name.size() );

      offset += tlvx10Sz;
   }

   // Add PDP type, if specified
   if (pPDPType != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWDSModifyProfileRequest_PDPType );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSModifyProfileRequest_PDPType * pTLVx11;
      pTLVx11 = (sWDSModifyProfileRequest_PDPType*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      // Set the value
      pTLVx11->mPDPType = (eQMIPDPTypes)*pPDPType;

      offset += tlvx11Sz;
   }

   // Add APN Name, if specified
   if (pAPNName != 0)
   {
      std::string apnName( pAPNName );

      // Check size
      WORD tlvx14Sz = (WORD)apnName.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx14Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x14;
      pHeader->mLength = tlvx14Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), apnName.c_str(), apnName.size() );

      offset += tlvx14Sz;
   }

   // Add Primary DNS, if specified
   if (pPrimaryDNS != 0)
   {
      // Check size
      WORD tlvx15Sz = sizeof( sWDSModifyProfileRequest_PrimaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx15Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x15;
      pHeader->mLength = tlvx15Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSModifyProfileRequest_PrimaryDNS * pTLVx15;
      pTLVx15 = (sWDSModifyProfileRequest_PrimaryDNS*)(pOut + offset);
      memset( pTLVx15, 0, tlvx15Sz );

      ULONG ip0 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip1 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx15->mIPV4Address[0] = (INT8)ip0;
      pTLVx15->mIPV4Address[1] = (INT8)ip1;
      pTLVx15->mIPV4Address[2] = (INT8)ip2;
      pTLVx15->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx15Sz;
   }

   // Add Secondary DNS, if specified
   if (pSecondaryDNS != 0)
   {
      // Check size
      WORD tlvx16Sz = sizeof( sWDSModifyProfileRequest_SecondaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx16Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x16;
      pHeader->mLength = tlvx16Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSModifyProfileRequest_SecondaryDNS * pTLVx16;
      pTLVx16 = (sWDSModifyProfileRequest_SecondaryDNS*)(pOut + offset);
      memset( pTLVx16, 0, tlvx16Sz );

      ULONG ip0 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip1 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx16->mIPV4Address[0] = (INT8)ip0;
      pTLVx16->mIPV4Address[1] = (INT8)ip1;
      pTLVx16->mIPV4Address[2] = (INT8)ip2;
      pTLVx16->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx16Sz;
   }

   // Add Username, if specified
   if (pUsername != 0)
   {
      std::string username( pUsername );

      // Check size
      WORD tlvx1BSz = (WORD)username.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx1BSz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x1B;
      pHeader->mLength = tlvx1BSz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), username.c_str(), username.size() );

      offset += tlvx1BSz;
   }

   // Add Password, if specified
   if (pPassword != 0)
   {
      std::string password( pPassword );

      // Check size
      WORD tlvx1CSz = (WORD)password.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx1CSz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x1C;
      pHeader->mLength = tlvx1CSz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), password.c_str(), password.size() );

      offset += tlvx1CSz;
   }

   // Add Authentication, if specified
   if (pAuthentication != 0)
   {
      // Check size
      WORD tlvx1DSz = sizeof( sWDSModifyProfileRequest_Authentication );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx1DSz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x1D;
      pHeader->mLength = tlvx1DSz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSModifyProfileRequest_Authentication * pTLVx1D;
      pTLVx1D = (sWDSModifyProfileRequest_Authentication*)(pOut + offset);
      memset( pTLVx1D, 0, tlvx1DSz );

      // Set the value
      pTLVx1D->mEnablePAP = ((*pAuthentication & 0x00000001) != 0);
      pTLVx1D->mEnableCHAP = ((*pAuthentication & 0x00000002) != 0);

      offset += tlvx1DSz;
   }

   // Add IP Address, if specified
   if (pIPAddress != 0)
   {
      // Check size
      WORD tlvx1ESz = sizeof( sWDSModifyProfileRequest_IPAddress );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx1ESz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x1E;
      pHeader->mLength = tlvx1ESz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSModifyProfileRequest_IPAddress * pTLVx1E;
      pTLVx1E = (sWDSModifyProfileRequest_IPAddress*)(pOut + offset);
      memset( pTLVx1E, 0, tlvx1ESz );

      ULONG ip0 = (*pIPAddress & 0x000000FF);
      ULONG ip1 = (*pIPAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pIPAddress & 0x00FF0000) >> 16;
      ULONG ip3 = (*pIPAddress & 0xFF000000) >> 24;

      // Set the value
      pTLVx1E->mIPV4Address[0] = (INT8)ip0;
      pTLVx1E->mIPV4Address[1] = (INT8)ip1;
      pTLVx1E->mIPV4Address[2] = (INT8)ip2;
      pTLVx1E->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx1ESz;
   }

   // At least one of the optional parameters must have been set
   if (offset <= sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetDefaultProfile

DESCRIPTION:
   This function reads the default profile settings from the device, the
   default profile is used during autoconnect

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
   profileType       [ I ] - Profile type being read

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetDefaultProfile(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      profileType )
{
      // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add profileType

   // Check size
   WORD tlvx01Sz = sizeof( sWDSGetDefaultSettingsRequest_ProfileType );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSGetDefaultSettingsRequest_ProfileType * pTLVx01;
   pTLVx01 = (sWDSGetDefaultSettingsRequest_ProfileType*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mProfileType = (eQMIProfileTypes)profileType;

   offset += tlvx01Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetDefaultProfile

DESCRIPTION:
   This function reads the default profile settings from the device, the
   default profile is used during autoconnect

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
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
ULONG ParseGetDefaultProfile(
   ULONG                      inLen,
   const BYTE *               pIn,
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
   // Validate arguments
   if (pIn == 0
   ||  pPDPType == 0
   ||  pIPAddress == 0
   ||  pPrimaryDNS == 0
   ||  pSecondaryDNS == 0
   ||  pAuthentication == 0
   ||  nameSize == 0
   ||  pName == 0
   ||  apnSize == 0
   ||  pAPNName == 0
   ||  userSize == 0
   ||  pUsername == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Set defaults
   *pPDPType = 0xffffffff;
   *pIPAddress = 0xffffffff;
   *pPrimaryDNS = 0xffffffff;
   *pSecondaryDNS = 0xffffffff;
   *pAuthentication = 0xffffffff;
   pName[0] = 0;
   pAPNName[0] = 0;
   pUsername[0] = 0;

   // Find the name
   const sWDSGetDefaultSettingsResponse_ProfileName * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (nameSize < outLenx10 + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pName, (const BYTE *)pTLVx10, outLenx10 );

      // Null terminate
      pName[outLenx10] = 0;
   }

   // Find the PDP type
   const sWDSGetDefaultSettingsResponse_PDPType * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sWDSGetDefaultSettingsResponse_PDPType ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pPDPType = pTLVx11->mPDPType;
   }

   // Find the APN name
   const sWDSGetDefaultSettingsResponse_APNName * pTLVx14;
   ULONG outLenx14;
   rc = GetTLV( inLen, pIn, 0x14, &outLenx14, (const BYTE **)&pTLVx14 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (apnSize < outLenx14 + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pAPNName, (const BYTE *)pTLVx14, outLenx14 );

      // Null terminate
      pAPNName[outLenx14] = 0;
   }

   // Find the Primary DNS
   const sWDSGetDefaultSettingsResponse_PrimaryDNS * pTLVx15;
   ULONG outLenx15;
   rc = GetTLV( inLen, pIn, 0x15, &outLenx15, (const BYTE **)&pTLVx15 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx15 < sizeof( sWDSGetDefaultSettingsResponse_PrimaryDNS ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx15->mIPV4Address[0];
      ULONG ip1 = pTLVx15->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx15->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx15->mIPV4Address[3] << 24;

      *pPrimaryDNS = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Secondary DNS
   const sWDSGetDefaultSettingsResponse_SecondaryDNS * pTLVx16;
   ULONG outLenx16;
   rc = GetTLV( inLen, pIn, 0x16, &outLenx16, (const BYTE **)&pTLVx16 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx16 < sizeof( sWDSGetDefaultSettingsResponse_SecondaryDNS ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx16->mIPV4Address[0];
      ULONG ip1 = pTLVx16->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx16->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx16->mIPV4Address[3] << 24;

      *pSecondaryDNS = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Username
   const sWDSGetDefaultSettingsResponse_APNName * pTLVx1B;
   ULONG outLenx1B;
   rc = GetTLV( inLen, pIn, 0x1B, &outLenx1B, (const BYTE **)&pTLVx1B );
   if (rc == eGOBI_ERR_NONE)
   {
      if (userSize < outLenx1B + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pAPNName, (const BYTE *)pTLVx1B, outLenx1B );

      // Null terminate
      pAPNName[outLenx1B] = 0;
   }

   // Find the Authentication
   const sWDSGetDefaultSettingsResponse_Authentication * pTLVx1D;
   ULONG outLenx1D;
   rc = GetTLV( inLen, pIn, 0x1D, &outLenx1D, (const BYTE **)&pTLVx1D );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1D < sizeof( sWDSGetDefaultSettingsResponse_Authentication ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG pap = pTLVx1D->mEnablePAP;
      ULONG chap = pTLVx1D->mEnableCHAP << 1;

      *pAuthentication = (pap | chap);
   }

   // Find the IP Address
   const sWDSGetDefaultSettingsResponse_IPAddress * pTLVx1E;
   ULONG outLenx1E;
   rc = GetTLV( inLen, pIn, 0x1E, &outLenx1E, (const BYTE **)&pTLVx1E );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1E < sizeof( sWDSGetDefaultSettingsResponse_IPAddress ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx1E->mIPV4Address[0];
      ULONG ip1 = pTLVx1E->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx1E->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx1E->mIPV4Address[3] << 24;

      *pIPAddress = (ip0 | ip1 | ip2 | ip3);
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackStartDataSession

DESCRIPTION:
   This function activates a packet data session

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
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

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackStartDataSession(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG *                    pTechnology,
   ULONG *                    pPrimaryDNS,
   ULONG *                    pSecondaryDNS,
   ULONG *                    pPrimaryNBNS,
   ULONG *                    pSecondaryNBNS,
   CHAR *                     pAPNName,
   ULONG *                    pIPAddress,
   ULONG *                    pAuthentication,
   CHAR *                     pUsername,
   CHAR *                     pPassword )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   sQMIRawContentHeader * pHeader;
   ULONG offset = 0;

   // Add technology, if specified
   if (pTechnology != 0)
   {
      // Check size
      WORD tlvx30Sz = sizeof( sWDSStartNetworkInterfaceRequest_TechnologyPreference );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx30Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x30;
      pHeader->mLength = tlvx30Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_TechnologyPreference * pTLVx30;
      pTLVx30 = (sWDSStartNetworkInterfaceRequest_TechnologyPreference*)(pOut + offset);
      memset( pTLVx30, 0, tlvx30Sz );

      // Set the value
      pTLVx30->mEnable3GPP = ((*pTechnology & 0x00000001) != 0);
      pTLVx30->mEnable3GPP2 = ((*pTechnology & 0x00000002) != 0);

      offset += tlvx30Sz;
   }

   // Add Primary DNS, if specified
   if (pPrimaryDNS != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWDSStartNetworkInterfaceRequest_PrimaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_PrimaryDNS * pTLVx10;
      pTLVx10 = (sWDSStartNetworkInterfaceRequest_PrimaryDNS*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      ULONG ip0 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip1 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx10->mIPV4Address[0] = (INT8)ip0;
      pTLVx10->mIPV4Address[1] = (INT8)ip1;
      pTLVx10->mIPV4Address[2] = (INT8)ip2;
      pTLVx10->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx10Sz;
   }

   // Add Secondary DNS, if specified
   if (pSecondaryDNS != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWDSStartNetworkInterfaceRequest_SecondaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_SecondaryDNS * pTLVx11;
      pTLVx11 = (sWDSStartNetworkInterfaceRequest_SecondaryDNS*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      ULONG ip0 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip1 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx11->mIPV4Address[0] = (INT8)ip0;
      pTLVx11->mIPV4Address[1] = (INT8)ip1;
      pTLVx11->mIPV4Address[2] = (INT8)ip2;
      pTLVx11->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx11Sz;
   }

   // Add Primary NBNS, if specified
   if (pPrimaryNBNS != 0)
   {
      // Check size
      WORD tlvx12Sz = sizeof( sWDSStartNetworkInterfaceRequest_PrimaryNBNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx12Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x12;
      pHeader->mLength = tlvx12Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_PrimaryNBNS * pTLVx12;
      pTLVx12 = (sWDSStartNetworkInterfaceRequest_PrimaryNBNS*)(pOut + offset);
      memset( pTLVx12, 0, tlvx12Sz );

      ULONG ip0 = (*pPrimaryNBNS & 0x000000FF);
      ULONG ip1 = (*pPrimaryNBNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryNBNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pPrimaryNBNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx12->mIPV4Address[0] = (INT8)ip0;
      pTLVx12->mIPV4Address[1] = (INT8)ip1;
      pTLVx12->mIPV4Address[2] = (INT8)ip2;
      pTLVx12->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx12Sz;
   }

   // Add Secondary NBNS, if specified
   if (pSecondaryNBNS != 0)
   {
      // Check size
      WORD tlvx13Sz = sizeof( sWDSStartNetworkInterfaceRequest_SecondaryNBNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx13Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x13;
      pHeader->mLength = tlvx13Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_SecondaryNBNS * pTLVx13;
      pTLVx13 = (sWDSStartNetworkInterfaceRequest_SecondaryNBNS*)(pOut + offset);
      memset( pTLVx13, 0, tlvx13Sz );

      ULONG ip0 = (*pSecondaryNBNS & 0x000000FF);
      ULONG ip1 = (*pSecondaryNBNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryNBNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pSecondaryNBNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx13->mIPV4Address[0] = (INT8)ip0;
      pTLVx13->mIPV4Address[1] = (INT8)ip1;
      pTLVx13->mIPV4Address[2] = (INT8)ip2;
      pTLVx13->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx13Sz;
   }

   // Add APN Name, if specified
   if (pAPNName != 0)
   {
      std::string apnName( pAPNName );

      // Check size
      WORD tlvx14Sz = (WORD)apnName.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx14Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x14;
      pHeader->mLength = tlvx14Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), apnName.c_str(), apnName.size() );

      offset += tlvx14Sz;
   }

   // Add IP Address, if specified
   if (pIPAddress != 0)
   {
      // Check size
      WORD tlvx15Sz = sizeof( sWDSStartNetworkInterfaceRequest_IPAddress );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx15Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x15;
      pHeader->mLength = tlvx15Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_IPAddress * pTLVx15;
      pTLVx15 = (sWDSStartNetworkInterfaceRequest_IPAddress*)(pOut + offset);
      memset( pTLVx15, 0, tlvx15Sz );

      ULONG ip0 = (*pIPAddress & 0x000000FF);
      ULONG ip1 = (*pIPAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pIPAddress & 0x00FF0000) >> 16;
      ULONG ip3 = (*pIPAddress & 0xFF000000) >> 24;

      // Set the value
      pTLVx15->mIPV4Address[0] = (INT8)ip0;
      pTLVx15->mIPV4Address[1] = (INT8)ip1;
      pTLVx15->mIPV4Address[2] = (INT8)ip2;
      pTLVx15->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx15Sz;
   }

   // Add Authentication, if specified
   if (pAuthentication != 0)
   {
      // Check size
      WORD tlvx16Sz = sizeof( sWDSStartNetworkInterfaceRequest_Authentication );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx16Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x16;
      pHeader->mLength = tlvx16Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSStartNetworkInterfaceRequest_Authentication * pTLVx16;
      pTLVx16 = (sWDSStartNetworkInterfaceRequest_Authentication*)(pOut + offset);
      memset( pTLVx16, 0, tlvx16Sz );

      // Set the value
      pTLVx16->mEnablePAP = ((*pAuthentication & 0x00000001) != 0);
      pTLVx16->mEnableCHAP = ((*pAuthentication & 0x00000002) != 0);

      offset += tlvx16Sz;
   }

   // Add Username, if specified
   if (pUsername != 0)
   {
      std::string username( pUsername );

      // Check size
      WORD tlvx17Sz = (WORD)username.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx17Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x17;
      pHeader->mLength = tlvx17Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), username.c_str(), username.size() );

      offset += tlvx17Sz;
   }

   // Add Password, if specified
   if (pPassword != 0)
   {
      std::string password( pPassword );

      // Check size
      WORD tlvx18Sz = (WORD)password.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx18Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x18;
      pHeader->mLength = tlvx18Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), password.c_str(), password.size() );

      offset += tlvx18Sz;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseStartDataSession

DESCRIPTION:
   This function activates a packet data session

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pSessionId        [ O ] - The assigned session ID
   pFailureReason    [ O ] - Upon call failure the failure reason provided

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseStartDataSession(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pSessionId,
   ULONG *                    pFailureReason )
{
   // Validate arguments
   if (pIn == 0
   ||  pSessionId == 0
   ||  pFailureReason == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check mandatory response
   const sResultCode * pTLVx02;
   ULONG outLenx02;
   ULONG rc = GetTLV( inLen, pIn, 0x02, &outLenx02, (const BYTE **)&pTLVx02 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx02 < sizeof( sResultCode ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   if (pTLVx02->mQMIResult != eQMIResults_Success)
   {
      rc = pTLVx02->mQMIError + eGOBI_ERR_QMI_OFFSET;
   }

   if (rc != eGOBI_ERR_NONE)
   {
      // Still parse call end reason, if present
      const sWDSStartNetworkInterfaceResponse_CallEndReason * pTLVx10;
      ULONG outLenx10;
      ULONG rc2 = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
      if (rc2 == eGOBI_ERR_NONE)
      {
         if (outLenx10 >= sizeof( sWDSStartNetworkInterfaceResponse_CallEndReason ))
         {
            *pFailureReason = pTLVx10->mCallEnd;
         }
      }

      return rc;
   }

   // Find the Session ID
   const sWDSStartNetworkInterfaceResponse_PacketDataHandle * pTLVx01;
   ULONG outLenx01;
   rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx01 < sizeof( sWDSStartNetworkInterfaceResponse_PacketDataHandle ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pSessionId = pTLVx01->mPacketDataHandle;
   }

   // Session ID is mandatory, if it failed return that error
   return rc;
}

/*===========================================================================
METHOD:
   PackStopDataSession

DESCRIPTION:
   This function stops the current data session

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   sessionId   [ I ] - The ID of the session to terminate

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackStopDataSession(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      sessionId )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add session ID

   // Check size
   WORD tlvx01Sz = sizeof( sWDSStopNetworkInterfaceRequest_PacketDataHandle );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSStopNetworkInterfaceRequest_PacketDataHandle * pTLVx01;
   pTLVx01 = (sWDSStopNetworkInterfaceRequest_PacketDataHandle*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mPacketDataHandle = sessionId;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetIPAddress

DESCRIPTION:
   This function returns the current packet data session IP address

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetIPAddress(
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Request the settings

   // Check size
   WORD tlvx10Sz = sizeof( sWDSGetCurrentSettingsRequest_RequestedSettings );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSGetCurrentSettingsRequest_RequestedSettings * pTLVx10;
   pTLVx10 = (sWDSGetCurrentSettingsRequest_RequestedSettings*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mIPAddress = true;

   offset += tlvx10Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetIPAddress

DESCRIPTION:
   This function returns the current packet data session IP address

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pIPAddress        [ O ] - Assigned IPv4 address

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetIPAddress(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pIPAddress )
{
   // Validate arguments
   if (pIn == 0 || pIPAddress == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the IP Address
   const sWDSGetDefaultSettingsResponse_IPAddress * pTLVx1E;
   ULONG outLenx1E;
   ULONG rc = GetTLV( inLen, pIn, 0x1E, &outLenx1E, (const BYTE **)&pTLVx1E );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1E < sizeof( sWDSGetDefaultSettingsResponse_IPAddress ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx1E->mIPV4Address[0];
      ULONG ip1 = pTLVx1E->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx1E->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx1E->mIPV4Address[3] << 24;

      *pIPAddress = (ip0 | ip1 | ip2 | ip3);
   }

   // If no IP address is found, fail
   return rc;
}

/*===========================================================================
METHOD:
   ParseGetConnectionRate

DESCRIPTION:
   This function returns connection rate information for the packet data
   connection

PARAMETERS:
   inLen                   [ I ] - Length of input buffer
   pIn                     [ I ] - Input buffer
   pCurrentChannelTXRate   [ O ] - Current channel TX rate (bps)
   pCurrentChannelRXRate   [ O ] - Current channel RX rate (bps)
   pMaxChannelTXRate       [ O ] - Maximum channel TX rate (bps)
   pMaxChannelRXRate       [ O ] - Maximum channel RX rate (bps)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetConnectionRate(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pCurrentChannelTXRate,
   ULONG *                    pCurrentChannelRXRate,
   ULONG *                    pMaxChannelTXRate,
   ULONG *                    pMaxChannelRXRate )
{
   // Validate arguments
   if (pIn == 0
   ||  pCurrentChannelTXRate == 0
   ||  pCurrentChannelRXRate == 0
   ||  pMaxChannelTXRate == 0
   ||  pMaxChannelRXRate == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the rates
   const sWDSGetChannelRatesResponse_ChannelRates * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx01 < sizeof( sWDSGetChannelRatesResponse_ChannelRates ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Get the values
      *pCurrentChannelTXRate = pTLVx01->mChannelTXRatebps;
      *pCurrentChannelRXRate = pTLVx01->mChannelRXRatebps;
      *pMaxChannelTXRate = pTLVx01->mMaxChannelTXRatebps;
      *pMaxChannelRXRate = pTLVx01->mMaxChannelRXRatebps;
   }

   // If no rates are found, fail
   return rc;
}

/*===========================================================================
METHOD:
   PackGetPacketStatus

DESCRIPTION:
   This function returns the packet data transfer statistics since the start
   of the current packet data session

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetPacketStatus(
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Request the settings

   // Check size
   WORD tlvx01Sz = sizeof( sWDSGetPacketStatisticsRequest_PacketStatsMask );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSGetPacketStatisticsRequest_PacketStatsMask * pTLVx01;
   pTLVx01 = (sWDSGetPacketStatisticsRequest_PacketStatsMask*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mReportTXPacketSuccesses = true;
   pTLVx01->mReportRXPacketSuccesses = true;
   pTLVx01->mReportTXPacketErrors = true;
   pTLVx01->mReportRXPacketErrors = true;
   pTLVx01->mReportTXOverflows = true;
   pTLVx01->mReportRXOverflows = true;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPacketStatus

DESCRIPTION:
   This function returns the packet data transfer statistics since the start
   of the current packet data session

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pTXPacketSuccesses   [ O ] - Packets transmitted without error
   pRXPacketSuccesses   [ O ] - Packets received without error
   pTXPacketErrors      [ O ] - Outgoing packets with framing errors
   pRXPacketErrors      [ O ] - Incoming packets with framing errors
   pTXPacketOverflows   [ O ] - Packets dropped because TX buffer overflowed
   pRXPacketOverflows   [ O ] - Packets dropped because RX buffer overflowed

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPacketStatus(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pTXPacketSuccesses,
   ULONG *                    pRXPacketSuccesses,
   ULONG *                    pTXPacketErrors,
   ULONG *                    pRXPacketErrors,
   ULONG *                    pTXPacketOverflows,
   ULONG *                    pRXPacketOverflows )
{
   // Validate arguments
   if (pIn == 0
   ||  pTXPacketSuccesses == 0
   ||  pRXPacketSuccesses == 0
   ||  pTXPacketErrors == 0
   ||  pRXPacketErrors == 0
   ||  pTXPacketOverflows == 0
   ||  pRXPacketOverflows == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // NOTE: All TLVs are required.  If any fail then all fail

   // Find the TX packet sucesses
   const sWDSGetPacketStatisticsResponse_TXPacketSuccesses * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sWDSGetPacketStatisticsResponse_TXPacketSuccesses ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the RX packet sucesses
   const sWDSGetPacketStatisticsResponse_RXPacketSuccesses * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sWDSGetPacketStatisticsResponse_RXPacketSuccesses ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the TX packet errors
   const sWDSGetPacketStatisticsResponse_TXPacketErrors * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx12 < sizeof( sWDSGetPacketStatisticsResponse_TXPacketErrors ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the RX packet errors
   const sWDSGetPacketStatisticsResponse_RXPacketErrors * pTLVx13;
   ULONG outLenx13;
   rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx13 < sizeof( sWDSGetPacketStatisticsResponse_RXPacketErrors ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the TX packet overflows
   const sWDSGetPacketStatisticsResponse_TXOverflows * pTLVx14;
   ULONG outLenx14;
   rc = GetTLV( inLen, pIn, 0x14, &outLenx14, (const BYTE **)&pTLVx14 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx14 < sizeof( sWDSGetPacketStatisticsResponse_TXOverflows ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the RX packet overflows
   const sWDSGetPacketStatisticsResponse_RXOverflows * pTLVx15;
   ULONG outLenx15;
   rc = GetTLV( inLen, pIn, 0x15, &outLenx15, (const BYTE **)&pTLVx15 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx15 < sizeof( sWDSGetPacketStatisticsResponse_RXOverflows ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Populate the statistics
   *pTXPacketSuccesses = pTLVx10->mTXPacketSuccesses;
   *pRXPacketSuccesses = pTLVx11->mRXPacketSuccesses;
   *pTXPacketErrors = pTLVx12->mTXPacketErrors;
   *pRXPacketErrors = pTLVx13->mRXPacketErrors;
   *pTXPacketOverflows = pTLVx14->mTXOverflows;
   *pRXPacketOverflows = pTLVx15->mRXOverflows;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetByteTotals

DESCRIPTION:
   This function returns the RX/TX byte counts since the start of the
   current packet data session

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetByteTotals(
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Request the settings

   // Check size
   WORD tlvx01Sz = sizeof( sWDSGetPacketStatisticsRequest_PacketStatsMask );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSGetPacketStatisticsRequest_PacketStatsMask * pTLVx01;
   pTLVx01 = (sWDSGetPacketStatisticsRequest_PacketStatsMask*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mTXByteTotal = true;
   pTLVx01->mRXByteTotal = true;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetByteTotals

DESCRIPTION:
   This function returns the RX/TX byte counts since the start of the
   current packet data session

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pTXTotalBytes  [ O ] - Bytes transmitted without error
   pRXTotalBytes  [ O ] - Bytes received without error

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetByteTotals(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONGLONG *                pTXTotalBytes,
   ULONGLONG *                pRXTotalBytes )
{
   // Validate arguments
   if (pIn == 0
   ||  pTXTotalBytes == 0
   ||  pRXTotalBytes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // NOTE: All TLVs are required.  If any fail then all fail

   // Find the TX bytes
   const sWDSGetPacketStatisticsResponse_TXBytes * pTLVx19;
   ULONG outLenx19;
   ULONG rc = GetTLV( inLen, pIn, 0x19, &outLenx19, (const BYTE **)&pTLVx19 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx19 < sizeof( sWDSGetPacketStatisticsResponse_TXBytes ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Find the RX bytes
   const sWDSGetPacketStatisticsResponse_RXBytes * pTLVx1A;
   ULONG outLenx1A;
   rc = GetTLV( inLen, pIn, 0x1A, &outLenx1A, (const BYTE **)&pTLVx1A );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1A < sizeof( sWDSGetPacketStatisticsResponse_RXBytes ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }
   }
   else
   {
      return rc;
   }

   // Populate the statistics
   *pTXTotalBytes = pTLVx19->mTXByteTotal;
   *pRXTotalBytes = pTLVx1A->mRXByteTotal;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetMobileIP

DESCRIPTION:
   This function sets the current mobile IP setting

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   mode        [ I ] - Desired mobile IP setting

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetMobileIP(
   ULONG *              pOutLen,
   BYTE *               pOut,
   ULONG                mode )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Set the mode

   // Check size
   WORD tlvx01Sz = sizeof( sWDSSetMIPModeRequest_MobileIPMode );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSSetMIPModeRequest_MobileIPMode * pTLVx01;
   pTLVx01 = (sWDSSetMIPModeRequest_MobileIPMode*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mMIPMode = (eQMIMobileIPModes)mode;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetMobileIP

DESCRIPTION:
   This function gets the current mobile IP setting

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pMode       [ O ] - Current mobile IP setting

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetMobileIP(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMode )
{
   // Validate arguments
   if (pIn == 0 || pMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the mode
   const sWDSGetMIPModeResponse_MobileIPMode * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx01 < sizeof( sWDSGetMIPModeResponse_MobileIPMode ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pMode = pTLVx01->mMIPMode;
   }

   return rc;
}

/*===========================================================================
METHOD:
   PackSetActiveMobileIPProfile

DESCRIPTION:
   This function sets the active mobile IP profile index

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   pSPC        [ I ] - Six digit service programming code
   index       [ I ] - Desired mobile IP profile index

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetActiveMobileIPProfile(
   ULONG *              pOutLen,
   BYTE *               pOut,
   CHAR *               pSPC,
   BYTE                 index )
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
   WORD tlvx01Sz = sizeof( sWDSSetActiveMIPProfileRequest_Index );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSSetActiveMIPProfileRequest_Index * pTLVx01;
   pTLVx01 = (sWDSSetActiveMIPProfileRequest_Index*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );
   pTLVx01->mProfileIndex = index;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetActiveMobileIPProfile

DESCRIPTION:
   This function gets the the active mobile IP profile index

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pIndex      [ O ] - Active mobile IP profile index

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetActiveMobileIPProfile(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE *            pIndex )
{
   // Validate arguments
   if (pIn == 0 || pIndex == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the mode
   const sWDSGetActiveMIPProfileResponse_Index * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx01 < sizeof( sWDSGetActiveMIPProfileResponse_Index ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pIndex = pTLVx01->mProfileIndex;
   }

   return rc;
}

/*===========================================================================
METHOD:
   PackSetMobileIPProfile

DESCRIPTION:
   This function sets the specified mobile IP profile settings

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
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
ULONG PackSetMobileIPProfile(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
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
   WORD tlvx01Sz = sizeof( sWDSSetMIPProfileRequest_Index );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSSetMIPProfileRequest_Index * pTLVx01;
   pTLVx01 = (sWDSSetMIPProfileRequest_Index*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );
   pTLVx01->mProfileIndex = index;

   offset += tlvx01Sz;

   // Add Enabled, if specified
   if (pEnabled != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWDSSetMIPProfileRequest_State );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_State * pTLVx10;
      pTLVx10 = (sWDSSetMIPProfileRequest_State*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the value
      pTLVx10->mEnabled = (*pEnabled == 0 ? 0 : 1);

      offset += tlvx10Sz;
   }

   // Add Home Address, if specified
   if (pAddress != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWDSSetMIPProfileRequest_HomeAddress );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_HomeAddress * pTLVx11;
      pTLVx11 = (sWDSSetMIPProfileRequest_HomeAddress*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      ULONG ip0 = (*pAddress & 0x000000FF);
      ULONG ip1 = (*pAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pAddress & 0x00FF0000) >> 16;
      ULONG ip3 = (*pAddress & 0xFF000000) >> 24;

      // Set the value
      pTLVx11->mIPV4Address[0] = (INT8)ip0;
      pTLVx11->mIPV4Address[1] = (INT8)ip1;
      pTLVx11->mIPV4Address[2] = (INT8)ip2;
      pTLVx11->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx11Sz;
   }

   // Add Primary Home Agent Address, if specified
   if (pPrimaryHA != 0)
   {
      // Check size
      WORD tlvx12Sz = sizeof( sWDSSetMIPProfileRequest_PrimaryHomeAgentAddress );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx12Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x12;
      pHeader->mLength = tlvx12Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_PrimaryHomeAgentAddress * pTLVx12;
      pTLVx12 = (sWDSSetMIPProfileRequest_PrimaryHomeAgentAddress*)(pOut + offset);
      memset( pTLVx12, 0, tlvx12Sz );

      ULONG ip0 = (*pPrimaryHA & 0x000000FF);
      ULONG ip1 = (*pPrimaryHA & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryHA & 0x00FF0000) >> 16;
      ULONG ip3 = (*pPrimaryHA & 0xFF000000) >> 24;

      // Set the value
      pTLVx12->mIPV4Address[0] = (INT8)ip0;
      pTLVx12->mIPV4Address[1] = (INT8)ip1;
      pTLVx12->mIPV4Address[2] = (INT8)ip2;
      pTLVx12->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx12Sz;
   }

   // Add Secondary Home Agent Address, if specified
   if (pSecondaryHA != 0)
   {
      // Check size
      WORD tlvx13Sz = sizeof( sWDSSetMIPProfileRequest_SecondaryHomeAgentAddress );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx13Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x13;
      pHeader->mLength = tlvx13Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_SecondaryHomeAgentAddress * pTLVx13;
      pTLVx13 = (sWDSSetMIPProfileRequest_SecondaryHomeAgentAddress*)(pOut + offset);
      memset( pTLVx13, 0, tlvx13Sz );

      ULONG ip0 = (*pSecondaryHA & 0x000000FF);
      ULONG ip1 = (*pSecondaryHA & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryHA & 0x00FF0000) >> 16;
      ULONG ip3 = (*pSecondaryHA & 0xFF000000) >> 24;

      // Set the value
      pTLVx13->mIPV4Address[0] = (INT8)ip0;
      pTLVx13->mIPV4Address[1] = (INT8)ip1;
      pTLVx13->mIPV4Address[2] = (INT8)ip2;
      pTLVx13->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx13Sz;
   }

   // Add reverse tunneling, if specified
   if (pRevTunneling != 0)
   {
      // Check size
      WORD tlvx14Sz = sizeof( sWDSSetMIPProfileRequest_ReverseTunneling );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx14Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x14;
      pHeader->mLength = tlvx14Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_ReverseTunneling * pTLVx14;
      pTLVx14 = (sWDSSetMIPProfileRequest_ReverseTunneling*)(pOut + offset);
      memset( pTLVx14, 0, tlvx14Sz );

      // Set the value
      pTLVx14->mReverseTunneling = (*pRevTunneling == 0 ? 0 : 1);

      offset += tlvx14Sz;
   }

   // Add NAI, if specified
   if (pNAI != 0)
   {
      std::string nai( pNAI );

      // Check size
      WORD tlvx15Sz = (WORD)nai.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx15Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x15;
      pHeader->mLength = tlvx15Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), nai.c_str(), nai.size() );

      offset += tlvx15Sz;
   }

   // Add HA SPI, if specified
   if (pHASPI != 0)
   {
      // Check size
      WORD tlvx16Sz = sizeof( sWDSSetMIPProfileRequest_HASPI );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx16Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x16;
      pHeader->mLength = tlvx16Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequest_HASPI * pTLVx16;
      pTLVx16 = (sWDSSetMIPProfileRequest_HASPI*)(pOut + offset);
      memset( pTLVx16, 0, tlvx16Sz );

      // Set the value
      pTLVx16->mHASPI = *pHASPI;

      offset += tlvx16Sz;
   }

   // Add AAA SPI, if specified
   if (pAAASPI != 0)
   {
      // Check size
      WORD tlvx17Sz = sizeof( sWDSSetMIPProfileRequeste_AAASPI );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx17Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x17;
      pHeader->mLength = tlvx17Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPProfileRequeste_AAASPI * pTLVx17;
      pTLVx17 = (sWDSSetMIPProfileRequeste_AAASPI*)(pOut + offset);
      memset( pTLVx17, 0, tlvx17Sz );

      // Set the value
      pTLVx17->mAAASPI = *pAAASPI;

      offset += tlvx17Sz;
   }

   // Add MN-HA key, if specified
   if (pMNHA != 0)
   {
      std::string mnha( pMNHA );

      // Check size
      WORD tlvx18Sz = (WORD)mnha.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx18Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x18;
      pHeader->mLength = tlvx18Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), mnha.c_str(), mnha.size() );

      offset += tlvx18Sz;
   }

   // Add MN-AAA key, if specified
   if (pMNHA != 0)
   {
      std::string mnaaa( pMNAAA );

      // Check size
      WORD tlvx19Sz = (WORD)mnaaa.size();
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx19Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x19;
      pHeader->mLength = tlvx19Sz;

      offset += sizeof( sQMIRawContentHeader );

      // Set the value
      memcpy( (pOut + offset), mnaaa.c_str(), mnaaa.size() );

      offset += tlvx19Sz;
   }

   // At least one of the optional parameters must have been set
   if (offset <= sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackGetMobileIPProfile

DESCRIPTION:
   This function gets the specified mobile IP profile settings

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   index          [ I ] - Mobile IP profile ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackGetMobileIPProfile(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   BYTE                       index )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sWDSGetMIPProfileRequest_Index );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSGetMIPProfileRequest_Index * pTLVx01;
   pTLVx01 = (sWDSGetMIPProfileRequest_Index*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mProfileIndex = index;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetMobileIPProfile

DESCRIPTION:
   This function gets the specified mobile IP profile settings

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
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
ULONG ParseGetMobileIPProfile(
   ULONG                      inLen,
   const BYTE *               pIn,
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
   // Validate arguments
   if (pIn == 0
   ||  pEnabled == 0
   ||  pAddress == 0
   ||  pPrimaryHA == 0
   ||  pSecondaryHA == 0
   ||  pRevTunneling == 0
   ||  naiSize == 0
   ||  pNAI == 0
   ||  pHASPI == 0
   ||  pAAASPI == 0
   ||  pHAState == 0
   ||  pAAAState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume errors
   *pEnabled = 0xff;
   *pAddress = 0xffffffff;
   *pPrimaryHA = 0xffffffff;
   *pSecondaryHA = 0xffffffff;
   *pRevTunneling = 0xff;
   *pHASPI = 0xffffffff;
   *pAAASPI = 0xffffffff;
   *pHAState = 0xffffffff;
   *pAAAState = 0xffffffff;

   // Find the State
   const sWDSGetMIPProfileResponse_State * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sWDSGetMIPProfileResponse_State ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pEnabled = pTLVx10->mEnabled;
   }

   // Find the Home Address
   const sWDSGetMIPProfileResponse_HomeAddress * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sWDSGetMIPProfileResponse_HomeAddress ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx11->mIPV4Address[0];
      ULONG ip1 = pTLVx11->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx11->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx11->mIPV4Address[3] << 24;

      *pAddress = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Primary Home Agent Address
   const sWDSGetMIPProfileResponse_PrimaryHomeAgentAddress * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx12 < sizeof( sWDSGetMIPProfileResponse_PrimaryHomeAgentAddress ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx12->mIPV4Address[0];
      ULONG ip1 = pTLVx12->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx12->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx12->mIPV4Address[3] << 24;

      *pPrimaryHA = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Secondary Home Agent Address
   const sWDSGetMIPProfileResponse_SecondaryHomeAgentAddress * pTLVx13;
   ULONG outLenx13;
   rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx13 < sizeof( sWDSGetMIPProfileResponse_SecondaryHomeAgentAddress ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx13->mIPV4Address[0];
      ULONG ip1 = pTLVx13->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx13->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx13->mIPV4Address[3] << 24;

      *pSecondaryHA = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Reverse tunneling, if enabled
   const sWDSGetMIPProfileResponse_ReverseTunneling * pTLVx14;
   ULONG outLenx14;
   rc = GetTLV( inLen, pIn, 0x14, &outLenx14, (const BYTE **)&pTLVx14 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sWDSGetMIPProfileResponse_ReverseTunneling ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRevTunneling = pTLVx14->mReverseTunneling;
   }

   // Find the NAI, if enabled
   const sWDSGetMIPProfileResponse_NAI * pTLVx15;
   ULONG outLenx15;
   rc = GetTLV( inLen, pIn, 0x15, &outLenx15, (const BYTE **)&pTLVx15 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (naiSize < outLenx15 + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pNAI, (const BYTE *)pTLVx15, outLenx15 );

      // Null terminate
      pNAI[outLenx15] = 0;
   }

   // Find the HA SPI
   const sWDSGetMIPProfileResponse_HASPI * pTLVx16;
   ULONG outLenx16;
   rc = GetTLV( inLen, pIn, 0x16, &outLenx16, (const BYTE **)&pTLVx16 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx16 < sizeof( sWDSGetMIPProfileResponse_HASPI ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pHASPI = pTLVx16->mHASPI;
   }

   // Find the AAA SPI
   const sWDSGetMIPProfileResponse_AAASPI * pTLVx17;
   ULONG outLenx17;
   rc = GetTLV( inLen, pIn, 0x17, &outLenx17, (const BYTE **)&pTLVx17 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx17 < sizeof( sWDSGetMIPProfileResponse_AAASPI ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pAAASPI = pTLVx17->mAAASPI;
   }

   // Find the HA state
   const sWDSGetMIPProfileResponse_HAState * pTLVx1A;
   ULONG outLenx1A;
   rc = GetTLV( inLen, pIn, 0x1A, &outLenx1A, (const BYTE **)&pTLVx1A );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1A < sizeof( sWDSGetMIPProfileResponse_HAState ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pHAState = pTLVx1A->mKeyState;
   }

   // Find the AAA state
   const sWDSGetMIPProfileResponse_AAAState * pTLVx1B;
   ULONG outLenx1B;
   rc = GetTLV( inLen, pIn, 0x1B, &outLenx1B, (const BYTE **)&pTLVx1B );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx1B < sizeof( sWDSGetMIPProfileResponse_AAAState ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pAAAState = pTLVx1B->mKeyState;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetMobileIPParameters

DESCRIPTION:
   This function sets the specified mobile IP parameters

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
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
ULONG PackSetMobileIPParameters(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   CHAR *                     pSPC,
   ULONG *                    pMode,
   BYTE *                     pRetryLimit,
   BYTE *                     pRetryInterval,
   BYTE *                     pReRegPeriod,
   BYTE *                     pReRegTraffic,
   BYTE *                     pHAAuthenticator,
   BYTE *                     pHA2002bis )
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
   WORD tlvx01Sz = sizeof( sWDSSetMIPParametersRequest_SPC );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sWDSSetMIPParametersRequest_SPC * pTLVx01;
   pTLVx01 = (sWDSSetMIPParametersRequest_SPC*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );

   offset += tlvx01Sz;

   // Add Mode, if specified
   if (pMode != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWDSSetMIPParametersRequest_MobileIPMode );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_MobileIPMode * pTLVx10;
      pTLVx10 = (sWDSSetMIPParametersRequest_MobileIPMode*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Set the value
      pTLVx10->mMIPMode = (eQMIMobileIPModes)*pMode;

      offset += tlvx10Sz;
   }

   // Add Retry Limit, if specified
   if (pRetryLimit != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWDSSetMIPParametersRequest_RetryAttemptLimit );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_RetryAttemptLimit * pTLVx11;
      pTLVx11 = (sWDSSetMIPParametersRequest_RetryAttemptLimit*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      // Set the value
      pTLVx11->mRetryAttemptLimit = *pRetryLimit;

      offset += tlvx11Sz;
   }

   // Add Retry interval, if specified
   if (pRetryInterval != 0)
   {
      // Check size
      WORD tlvx12Sz = sizeof( sWDSSetMIPParametersRequest_RetryAttemptInterval );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx12Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x12;
      pHeader->mLength = tlvx12Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_RetryAttemptInterval * pTLVx12;
      pTLVx12 = (sWDSSetMIPParametersRequest_RetryAttemptInterval*)(pOut + offset);
      memset( pTLVx12, 0, tlvx12Sz );

      // Set the value
      pTLVx12->mRetryAttemptInterval = *pRetryInterval;

      offset += tlvx12Sz;
   }

   // Add Re-registration period, if specified
   if (pReRegPeriod != 0)
   {
      // Check size
      WORD tlvx13Sz = sizeof( sWDSSetMIPParametersRequest_ReRegistrationPeriod );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx13Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x13;
      pHeader->mLength = tlvx13Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_ReRegistrationPeriod * pTLVx13;
      pTLVx13 = (sWDSSetMIPParametersRequest_ReRegistrationPeriod*)(pOut + offset);
      memset( pTLVx13, 0, tlvx13Sz );

      // Set the value
      pTLVx13->mReRegistrationPeriod = *pReRegPeriod;

      offset += tlvx13Sz;
   }

   // Add Re-registration on traffic flag, if specified
   if (pReRegTraffic != 0)
   {
      // Check size
      WORD tlvx14Sz = sizeof( sWDSSetMIPParametersRequest_ReRegistrationOnlyWithTraffic );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx14Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x14;
      pHeader->mLength = tlvx14Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_ReRegistrationOnlyWithTraffic * pTLVx14;
      pTLVx14 = (sWDSSetMIPParametersRequest_ReRegistrationOnlyWithTraffic*)(pOut + offset);
      memset( pTLVx14, 0, tlvx14Sz );

      // Set the value
      pTLVx14->mReRegistrationOnlyWithTraffic = (*pReRegTraffic == 0 ? 0 : 1);

      offset += tlvx14Sz;
   }

   // Add HA authenticator flag, if specified
   if (pHAAuthenticator != 0)
   {
      // Check size
      WORD tlvx15Sz = sizeof( sWDSSetMIPParametersRequest_MNHAAuthenticatorCalculator );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx15Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x15;
      pHeader->mLength = tlvx15Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_MNHAAuthenticatorCalculator * pTLVx15;
      pTLVx15 = (sWDSSetMIPParametersRequest_MNHAAuthenticatorCalculator*)(pOut + offset);
      memset( pTLVx15, 0, tlvx15Sz );

      // Set the value
      pTLVx15->mMNHAAuthenticatorCalculator = (*pHAAuthenticator == 0 ? 0 : 1);

      offset += tlvx15Sz;
   }

   // Add HA RFC2002bis authentication flag, if specified
   if (pHA2002bis != 0)
   {
      // Check size
      WORD tlvx16Sz = sizeof( sWDSSetMIPParametersRequest_MNHARFC2002BISAuthentication );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx16Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x16;
      pHeader->mLength = tlvx16Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetMIPParametersRequest_MNHARFC2002BISAuthentication * pTLVx16;
      pTLVx16 = (sWDSSetMIPParametersRequest_MNHARFC2002BISAuthentication*)(pOut + offset);
      memset( pTLVx16, 0, tlvx16Sz );

      // Set the value
      pTLVx16->mMNHARFC2002BISAuthentication = (*pHA2002bis == 0 ? 0 : 1);

      offset += tlvx16Sz;
   }

   // At least one of the optional parameters must have been set
   if (offset <= sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetMobileIPParameters

DESCRIPTION:
   This function gets the mobile IP parameters

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
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
ULONG ParseGetMobileIPParameters(
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pMode,
   BYTE *                     pRetryLimit,
   BYTE *                     pRetryInterval,
   BYTE *                     pReRegPeriod,
   BYTE *                     pReRegTraffic,
   BYTE *                     pHAAuthenticator,
   BYTE *                     pHA2002bis )
{
   // Validate arguments
   if (pIn == 0
   ||  pMode == 0
   ||  pRetryLimit == 0
   ||  pRetryInterval == 0
   ||  pReRegPeriod == 0
   ||  pReRegTraffic == 0
   ||  pHAAuthenticator == 0
   ||  pHA2002bis == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pMode = 0xffffffff;
   *pRetryLimit = 0xff;
   *pRetryInterval = 0xff;
   *pReRegPeriod = 0xff;
   *pReRegTraffic = 0xff;
   *pHAAuthenticator = 0xff;
   *pHA2002bis = 0xff;

   // Find the mode
   const sWDSGetMIPParametersResponse_MobileIPMode * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sWDSGetMIPParametersResponse_MobileIPMode ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pMode = pTLVx10->mMIPMode;
   }

   // Find the Retry limit
   const sWDSGetMIPParametersResponse_RetryAttemptLimit * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sWDSGetMIPParametersResponse_RetryAttemptLimit ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRetryLimit = pTLVx11->mRetryAttemptLimit;
   }

   // Find the Retry Interval
   const sWDSGetMIPParametersResponse_RetryAttemptInterval * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx12 < sizeof( sWDSGetMIPParametersResponse_RetryAttemptInterval ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pRetryInterval = pTLVx12->mRetryAttemptInterval;
   }

   // Find the Re-registration period
   const sWDSGetMIPParametersResponse_ReRegistrationPeriod * pTLVx13;
   ULONG outLenx13;
   rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx13 < sizeof( sWDSGetMIPParametersResponse_ReRegistrationPeriod ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pReRegPeriod = pTLVx13->mReRegistrationPeriod;
   }

   // Find the Re-register on traffic flag
   const sWDSGetMIPParametersResponse_ReRegistrationOnlyWithTraffic * pTLVx14;
   ULONG outLenx14;
   rc = GetTLV( inLen, pIn, 0x14, &outLenx14, (const BYTE **)&pTLVx14 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx14 < sizeof( sWDSGetMIPParametersResponse_ReRegistrationOnlyWithTraffic ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pReRegTraffic = pTLVx14->mReRegistrationOnlyWithTraffic;
   }

   // Find the HA authenticator
   const sWDSGetMIPParametersResponse_MNHAAuthenticatorCalculator * pTLVx15;
   ULONG outLenx15;
   rc = GetTLV( inLen, pIn, 0x15, &outLenx15, (const BYTE **)&pTLVx15 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx15 < sizeof( sWDSGetMIPParametersResponse_MNHAAuthenticatorCalculator ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pHAAuthenticator = pTLVx15->mMNHAAuthenticatorCalculator;
   }

   // Find the HA RFC2002bis authentication flag
   const sWDSGetMIPParametersResponse_MNHARFC2002BISAuthentication * pTLVx16;
   ULONG outLenx16;
   rc = GetTLV( inLen, pIn, 0x16, &outLenx16, (const BYTE **)&pTLVx16 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx16 < sizeof( sWDSGetMIPParametersResponse_MNHARFC2002BISAuthentication ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pHA2002bis = pTLVx16->mMNHARFC2002BISAuthentication;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetLastMobileIPError

DESCRIPTION:
   This function gets the last mobile IP error

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pError      [ O ] - Last mobile IP error

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetLastMobileIPError(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pError )
{
   // Validate arguments
   if (pIn == 0 || pError == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sWDSGetLastMIPStatusResponse_Status * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetLastMIPStatusResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pError = pTLVx01->mLastMIPStatus;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetDNSSettings

DESCRIPTION:
   This function sets the DNS settings for the device

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
   pPrimaryDNS       [ I ] - (Optional) Primary DNS IPv4 address
   pSecondaryDNS     [ I ] - (Optional) Secondary DNS IPv4 address

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetDNSSettings(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG *                    pPrimaryDNS,
   ULONG *                    pSecondaryDNS )
{
   // Validate arguments
   // At least one must be specified
   if (pOut == 0 || (pPrimaryDNS == 0 && pSecondaryDNS == 0))
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   sQMIRawContentHeader * pHeader;
   ULONG offset = 0;

   // Add Primary DNS, if specified
   if (pPrimaryDNS != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sWDSSetDNSSettingRequest_PrimaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetDNSSettingRequest_PrimaryDNS * pTLVx10;
      pTLVx10 = (sWDSSetDNSSettingRequest_PrimaryDNS*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      ULONG ip0 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip1 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx10->mIPV4Address[0] = (INT8)ip0;
      pTLVx10->mIPV4Address[1] = (INT8)ip1;
      pTLVx10->mIPV4Address[2] = (INT8)ip2;
      pTLVx10->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx10Sz;
   }

   // Add Secondary DNS, if specified
   if (pSecondaryDNS != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sWDSSetDNSSettingRequest_SecondaryDNS );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sWDSSetDNSSettingRequest_SecondaryDNS * pTLVx11;
      pTLVx11 = (sWDSSetDNSSettingRequest_SecondaryDNS*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      ULONG ip0 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip1 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip3 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // Set the value
      pTLVx11->mIPV4Address[0] = (INT8)ip0;
      pTLVx11->mIPV4Address[1] = (INT8)ip1;
      pTLVx11->mIPV4Address[2] = (INT8)ip2;
      pTLVx11->mIPV4Address[3] = (INT8)ip3;

      offset += tlvx11Sz;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetDNSSettings

DESCRIPTION:
   This function gets the DNS settings for the device

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pPrimaryDNS       [ O ] - Primary DNS IPv4 address
   pSecondaryDNS     [ O ] - Secondary DNS IPv4 address

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetDNSSettings(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPrimaryDNS,
   ULONG *           pSecondaryDNS )
{
   // Validate arguments
   if (pIn == 0 || pPrimaryDNS == 0 || pSecondaryDNS == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the Primary DNS
   const sWDSGetDNSSettingResponse_PrimaryDNS * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sWDSGetDNSSettingResponse_PrimaryDNS ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx10->mIPV4Address[0];
      ULONG ip1 = pTLVx10->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx10->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx10->mIPV4Address[3] << 24;

      *pPrimaryDNS = (ip0 | ip1 | ip2 | ip3);
   }

   // Find the Secondary DNS
   const sWDSGetDNSSettingResponse_SecondaryDNS * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sWDSGetDNSSettingResponse_SecondaryDNS ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      ULONG ip0 = pTLVx11->mIPV4Address[0];
      ULONG ip1 = pTLVx11->mIPV4Address[1] << 8;
      ULONG ip2 = pTLVx11->mIPV4Address[2] << 16;
      ULONG ip3 = pTLVx11->mIPV4Address[3] << 24;

      *pSecondaryDNS = (ip0 | ip1 | ip2 | ip3);
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetDataBearerTechnology

DESCRIPTION:
   This function retrieves the current data bearer technology (only
   valid when connected)

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pDataCaps      [ O ] - The data bearer technology

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetDataBearerTechnology(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pDataBearer )
{
   // Validate arguments
   if (pIn == 0 || pDataBearer == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the TLV
   const sWDSGetDataBearerTechnologyResponse_Technology * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sWDSGetDataBearerTechnologyResponse_Technology ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pDataBearer = pTLVx01->mDataBearerTechnology;

   return eGOBI_ERR_NONE;
}
