/*===========================================================================
FILE: 
   GobiQMICoreWDS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (WDS Service)

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

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "GobiQMICore.h"

#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetSessionState (Public Method)

DESCRIPTION:
   This function returns the state of the current packet data session

PARAMETERS:
   pState      [ O ] - State of the current packet session
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSessionState( ULONG * pState )
{
   // Validate arguments
   if (pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_PKT_STATUS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the state
   *pState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSessionDuration (Public Method)

DESCRIPTION:
   This function returns the duration of the current packet data session

PARAMETERS:
   pDuration   [ O ] - Duration of the current packet session
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSessionDuration( ULONGLONG * pDuration )
{
   // Validate arguments
   if (pDuration == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_DURATION;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the state
   *pDuration = pf[0].mValue.mU64;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSessionDurations (Public Method)

DESCRIPTION:
   This function returns the the active/total durations of the current 
   packet data session

PARAMETERS:
   pActiveDuration   [ O ] - Active duration of the current packet session
   pTotalDuration    [ O ] - Total duration of the current packet session
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSessionDurations(
   ULONGLONG *                pActiveDuration,
   ULONGLONG *                pTotalDuration )
{
   // Validate arguments
   if (pActiveDuration == 0 || pTotalDuration == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_DURATION;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the total duration
   *pTotalDuration = pf[0].mValue.mU64;

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the active duration
   *pActiveDuration = pf[0].mValue.mU64;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetDormancyState (Public Method)

DESCRIPTION:
   This function returns the dormancy state of the current packet 
   data session (when connected)

PARAMETERS:
   pState      [ O ] - Dormancy state of the current packet session
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetDormancyState( ULONG * pState )
{
   // Validate arguments
   if (pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_DORMANCY;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the state
   *pState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetEnhancedAutoconnect (Public Method)

DESCRIPTION:
   This function returns the current autoconnect data session setting

PARAMETERS:
   pSetting       [ O ] - NDIS autoconnect setting
   pRoamSetting   [ O ] - NDIS autoconnect roam setting 

 
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetEnhancedAutoconnect( 
   ULONG *                    pSetting,
   ULONG *                    pRoamSetting )
{
   // Validate arguments
   if (pSetting == 0 || pRoamSetting == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSetting = ULONG_MAX;
   *pRoamSetting = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_AUTOCONNECT;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pSetting = (ULONG)pf[0].mValue.mU32;

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() > 0) 
   {
      *pRoamSetting = (ULONG)pf[0].mValue.mU32;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetEnhancedAutoconnect (Public Method)

DESCRIPTION:
   This function sets the autoconnect data session setting

PARAMETERS:
   setting        [ I ] - NDIS autoconnect setting
   pRoamSetting   [ I ] - (Optional) NDIS autoconnect roam setting 
 
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetEnhancedAutoconnect(
   ULONG                      setting,
   ULONG *                    pRoamSetting )
{
   WORD msgID = (WORD)eQMI_WDS_SET_AUTOCONNECT;
   std::vector <sDB2PackingInput> piv;

   // "%u"
   std::ostringstream tmp;
   tmp << setting;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, tmp.str().c_str() ); 
   piv.push_back( pi );

   if (pRoamSetting != 0)
   {
      std::ostringstream tmp2;
      tmp2 << *pRoamSetting;

      sProtocolEntityKey pek1( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      sDB2PackingInput pi1( pek1, tmp2.str().c_str() ); 
      piv.push_back( pi1 );
   }

   ULONG to = 5000;
   if (setting == 1)
   {
      // Connections can take a long time
      to = 300000;
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest, to );
}

/*===========================================================================
METHOD:
   SetDefaultProfile (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetDefaultProfile( 
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
   WORD msgID = (WORD)eQMI_WDS_MODIFY_PROFILE;
   std::vector <sDB2PackingInput> piv;

   // "%u 1"
   std::ostringstream tmp;
   tmp << (UINT)profileType << " 1";

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   if (pName != 0)
   {
      std::ostringstream tmp2;
      if (pName[0] != 0)
      {
         tmp2 << "\"" << pName << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pPDPType != 0)
   {
      // "%u"
      std::ostringstream tmp2;
      tmp2 << (UINT)*pPDPType;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 17 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pAPNName != 0)
   {
      std::ostringstream tmp2;
      if (pAPNName[0] != 0)
      {
         tmp2 << "\"" << pAPNName << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 20 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pPrimaryDNS != 0)
   {
      ULONG ip4 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip3 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 21 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pSecondaryDNS != 0)
   {
      ULONG ip4 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip3 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 22 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pUsername != 0)
   {
      std::ostringstream tmp2;
      if (pUsername[0] != 0)
      {
         tmp2 << "\"" << pUsername << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 27 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pPassword != 0)
   {
      std::ostringstream tmp2;
      if (pPassword[0] != 0)
      {
         tmp2 << "\"" << pPassword << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 28 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pAuthentication != 0)
   {
      ULONG pap = *pAuthentication & 0x00000001;
      ULONG chap = *pAuthentication & 0x00000002;
      
      // "%u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)pap << " " << (UINT)chap;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 29 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   if (pIPAddress != 0)
   {
      ULONG ip4 = (*pIPAddress & 0x000000FF);
      ULONG ip3 = (*pIPAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pIPAddress & 0x00FF0000) >> 16;
      ULONG ip1 = (*pIPAddress & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;


      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 30 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // We need to be doing something here (beyond profile type)
   if (piv.size() <= 1)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Pack up and send the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
}

/*===========================================================================
METHOD:
   GetDefaultProfile (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetDefaultProfile( 
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
   // Validate arguments
   if ( (pPDPType == 0)
   ||   (pIPAddress == 0)
   ||   (pPrimaryDNS == 0)
   ||   (pSecondaryDNS == 0)
   ||   (pAuthentication == 0)
   ||   (nameSize == 0)
   ||   (pName == 0)
   ||   (apnSize == 0)
   ||   (pAPNName == 0)
   ||   (userSize == 0)
   ||   (pUsername == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pPDPType = ULONG_MAX;
   *pIPAddress = ULONG_MAX;
   *pPrimaryDNS = ULONG_MAX;
   *pSecondaryDNS = ULONG_MAX;
   *pAuthentication = ULONG_MAX;
   pName[0] = 0;
   pAPNName[0] = 0;
   pUsername[0] = 0;

   WORD msgID = (WORD)eQMI_WDS_GET_DEFAULTS;
   std::vector <sDB2PackingInput> piv;

   // "%u 0"
   std::ostringstream tmp;
   tmp << (UINT)profileType << " 0";

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WDS, pRequest );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      LONG strLen = pf[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (nameSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pName, (LPCSTR)pf[0].mValueString.c_str(), strLen );
      pName[strLen] = 0;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pPDPType = pf[0].mValue.mU32;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 20 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      LONG strLen = pf[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (apnSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pAPNName, (LPCSTR)pf[0].mValueString.c_str(), strLen );
      pAPNName[strLen] = 0;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 21 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 4)
   {
      ULONG ip4 = (ULONG)pf[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
      *pPrimaryDNS = (ip4 | ip3 | ip2 | ip1);   
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 22 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 4)
   {
      ULONG ip4 = (ULONG)pf[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
      *pSecondaryDNS = (ip4 | ip3 | ip2 | ip1);   
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 27 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      LONG strLen = pf[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (userSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pUsername, (LPCSTR)pf[0].mValueString.c_str(), strLen );
      pUsername[strLen] = 0;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 29 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 2)
   {
      ULONG pap = (ULONG)pf[0].mValue.mU8;
      ULONG chap = (ULONG)pf[1].mValue.mU8 << 1;

      *pAuthentication = (pap | chap);   
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 30 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 4)
   {
      ULONG ip4 = (ULONG)pf[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
      *pIPAddress = (ip4 | ip3 | ip2 | ip1);   
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   StartDataSession (Public Method)

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
   pFailureReason    [ O ] - Upon call failure the failure reason

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::StartDataSession( 
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
   *pFailureReason = (ULONG)eQMI_CALL_END_REASON_UNSPECIFIED;

   // Validate arguments
   if (pSessionId == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_START_NET;
   std::vector <sDB2PackingInput> piv;

   if (pTechnology != 0)
   {
      ULONG umts = *pTechnology & 0x00000001;
      ULONG cdma = *pTechnology & 0x00000002;
      
      // "%u %u"
      std::ostringstream tmp;
      tmp << (UINT)umts << " " << (UINT)cdma;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 48 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pPrimaryDNS != 0)
   {
      ULONG ip4 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip3 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pSecondaryDNS != 0)
   {
      ULONG ip4 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip3 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 17 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pPrimaryNBNS != 0)
   {
      ULONG ip4 = (*pPrimaryNBNS & 0x000000FF);
      ULONG ip3 = (*pPrimaryNBNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryNBNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pPrimaryNBNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 18 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pSecondaryNBNS != 0)
   {
      ULONG ip4 = (*pSecondaryNBNS & 0x000000FF);
      ULONG ip3 = (*pSecondaryNBNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryNBNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pSecondaryNBNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 19 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pAPNName != 0)
   {
      std::ostringstream tmp;
      if (pAPNName[0] != 0)
      {
         tmp << "\"" << pAPNName << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 20 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pIPAddress != 0)
   {
      ULONG ip4 = (*pIPAddress & 0x000000FF);
      ULONG ip3 = (*pIPAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pIPAddress & 0x00FF0000) >> 16;
      ULONG ip1 = (*pIPAddress & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 21 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pAuthentication != 0)
   {
      ULONG pap = *pAuthentication & 0x00000001;
      ULONG chap = *pAuthentication & 0x00000002;
      
      // "%u %u"
      std::ostringstream tmp;
      tmp << (UINT)pap << " " << (UINT)chap;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 22 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pUsername != 0)
   {
      std::ostringstream tmp;
      if (pUsername[0] != 0)
      {
         tmp << "\"" << pUsername << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 23 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pPassword != 0)
   {
      std::ostringstream tmp;
      if (pPassword[0] != 0)
      {
         tmp << "\"" << pPassword << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 24 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   sProtocolBuffer rsp;
   if (piv.size() > 0)
   {
      // Pack up and send the QMI request
      const cCoreDatabase & db = GetDatabase();
      sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
      if (pRequest == 0)
      {
         return eGOBI_ERR_MEMORY;
      }
      else
      {
         rsp = Send( eQMI_SVC_WDS, pRequest, 300000 );        
      }
   }
   else
   {
      // Generate and send the QMI request
      rsp = SendSimple( eQMI_SVC_WDS, msgID, 300000 );
   }

   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
      const cCoreDatabase & db = GetDatabase();

      // Parse the TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 1) 
      {
         *pFailureReason = pf[0].mValue.mU32;
      }

      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the session ID
   *pSessionId = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   CancelDataSession (Public Method)

DESCRIPTION:
   Cancel an in-progress packet data session activation

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::CancelDataSession()
{
   if (mLastNetStartID == (WORD)INVALID_QMI_TRANSACTION_ID)
   {
      return eGOBI_ERR_NO_CANCELABLE_OP;
   }

   WORD msgID = (WORD)eQMI_WDS_ABORT;
   std::vector <sDB2PackingInput> piv;

   // %hu
   std::ostringstream tmp;
   tmp << mLastNetStartID;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   mLastNetStartID = (WORD)INVALID_QMI_TRANSACTION_ID;
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest, 60000 );
}

/*===========================================================================
METHOD:
   StopDataSession (Public Method)

DESCRIPTION:
   This function stops the current data session

PARAMETERS:
   sessionId   [ I ] - The ID of the session to terminate
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::StopDataSession( ULONG sessionId )
{
   WORD msgID = (WORD)eQMI_WDS_STOP_NET;
   std::vector <sDB2PackingInput> piv;

   // "%u"
   std::ostringstream tmp;
   tmp << (UINT)sessionId;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest, 60000 );
}

/*===========================================================================
METHOD:
   GetIPAddress

DESCRIPTION:
   This function returns the current packet data session IP address

PARAMETERS:
   pIPAddress        [ I ] - Assigned IPv4 address
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetIPAddress( ULONG * pIPAddress )
{
   // Validate arguments
   if (pIPAddress == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pIPAddress = ULONG_MAX;

   WORD msgID = (WORD)eQMI_WDS_GET_SETTINGS;
   std::vector <sDB2PackingInput> piv;

   std::string tmp = "0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0";

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WDS, pRequest );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   // Parse the TLVs we want (IP address)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 30 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 4) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG ip4 = (ULONG)pf[0].mValue.mU8;
   ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
   ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
   ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
   *pIPAddress = (ip4 | ip3 | ip2 | ip1);

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetConnectionRate (Public Method)

DESCRIPTION:
   This function returns connection rate information for the packet data 
   connection

PARAMETERS:
   pCurrentChannelTXRate   [ O ] - Current channel TX rate (bps)
   pCurrentChannelRXRate   [ O ] - Current channel RX rate (bps)
   pMaxChannelTXRate       [ O ] - Maximum channel TX rate (bps)
   pMaxChannelRXRate       [ O ] - Maximum channel RX rate (bps)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetConnectionRate(
   ULONG *                    pCurrentChannelTXRate,
   ULONG *                    pCurrentChannelRXRate,
   ULONG *                    pMaxChannelTXRate,
   ULONG *                    pMaxChannelRXRate )
{
   // Validate arguments
   if ( (pCurrentChannelTXRate == 0)
   ||   (pCurrentChannelRXRate == 0)
   ||   (pMaxChannelTXRate == 0)
   ||   (pMaxChannelRXRate == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_RATES;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 4) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the rates
   *pCurrentChannelTXRate = pf[0].mValue.mU32;
   *pCurrentChannelRXRate = pf[1].mValue.mU32;
   *pMaxChannelTXRate = pf[2].mValue.mU32;
   *pMaxChannelRXRate = pf[3].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetPacketStatus (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPacketStatus(  
   ULONG *                    pTXPacketSuccesses, 
   ULONG *                    pRXPacketSuccesses, 
   ULONG *                    pTXPacketErrors, 
   ULONG *                    pRXPacketErrors, 
   ULONG *                    pTXPacketOverflows, 
   ULONG *                    pRXPacketOverflows )
{
   // Validate arguments
   if ( (pTXPacketSuccesses == 0)
   ||   (pRXPacketSuccesses == 0)
   ||   (pTXPacketErrors == 0)
   ||   (pRXPacketErrors == 0)
   ||   (pTXPacketOverflows == 0)
   ||   (pRXPacketOverflows == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_GET_STATISTICS;
   std::vector <sDB2PackingInput> piv;

   std::string tmp = "1 1 1 1 1 1 0 0";

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WDS, pRequest );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   // Parse the TLVs we want (by DB key)
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey3( eDB2_ET_QMI_WDS_RSP, msgID, 18 );
   cDataParser::tParsedFields pf3 = ParseTLV( db, rsp, tlvs, tlvKey3 );
   if (pf3.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey4( eDB2_ET_QMI_WDS_RSP, msgID, 19 );
   cDataParser::tParsedFields pf4 = ParseTLV( db, rsp, tlvs, tlvKey4 );
   if (pf4.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey5( eDB2_ET_QMI_WDS_RSP, msgID, 20 );
   cDataParser::tParsedFields pf5 = ParseTLV( db, rsp, tlvs, tlvKey5 );
   if (pf5.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey6( eDB2_ET_QMI_WDS_RSP, msgID, 21 );
   cDataParser::tParsedFields pf6 = ParseTLV( db, rsp, tlvs, tlvKey5 );
   if (pf6.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the statistics
   *pTXPacketSuccesses = pf1[0].mValue.mU32;
   *pRXPacketSuccesses = pf2[0].mValue.mU32;
   *pTXPacketErrors = pf3[0].mValue.mU32;
   *pRXPacketErrors = pf4[0].mValue.mU32;
   *pTXPacketOverflows = pf5[0].mValue.mU32;
   *pRXPacketOverflows = pf6[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetByteTotals (Public Method)

DESCRIPTION:
   This function returns the RX/TX byte counts since the start of the 
   current packet data session

PARAMETERS:
   pTXTotalBytes  [ O ] - Bytes transmitted without error
   pRXTotalBytes  [ O ] - Bytes received without error
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetByteTotals(   
   ULONGLONG *                pTXTotalBytes, 
   ULONGLONG *                pRXTotalBytes )
{
   // Validate arguments
   if (pTXTotalBytes == 0 || pRXTotalBytes == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_GET_STATISTICS;
   std::vector <sDB2PackingInput> piv;

   std::string tmp = "0 0 0 0 0 0 1 1";

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WDS, pRequest );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_WDS_RSP, msgID, 25 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_WDS_RSP, msgID, 26 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the statistics
   *pTXTotalBytes = pf1[0].mValue.mU64;
   *pRXTotalBytes = pf2[0].mValue.mU64;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetMobileIP (Public Method)

DESCRIPTION:
   This function sets the current mobile IP setting

PARAMETERS:
   mode        [ I ] - Desired mobile IP setting
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetMobileIP( ULONG mode )
{
   WORD msgID = (WORD)eQMI_WDS_SET_MIP;
   std::vector <sDB2PackingInput> piv;

   // "%u"
   std::ostringstream tmp;
   tmp << (UINT)mode;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
}

/*===========================================================================
METHOD:
   GetMobileIP (Public Method)

DESCRIPTION:
   This function gets the current mobile IP setting

PARAMETERS:
   pMode       [ I ] - Desired mobile IP setting
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetMobileIP( ULONG * pMode )
{
   // Validate arguments
   if (pMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_MIP;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the mode
   *pMode = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetActiveMobileIPProfile (Public Method)

DESCRIPTION:
   This function sets the active mobile IP profile index

PARAMETERS:
   pSPC        [ I ] - Six digit service programming code
   index       [ I ] - Desired mobile IP profile index
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetActiveMobileIPProfile( 
   CHAR *                     pSPC,
   BYTE                       index )
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

   int nNonDigit = spc.find_first_not_of( "0123456789" );
   std::string digitSPC = spc.substr( 0, nNonDigit );
   if (digitSPC.size() != spc.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_SET_ACTIVE_MIP;
   std::vector <sDB2PackingInput> piv;

   // "%s %u"
   std::ostringstream tmp;
   tmp << spc << " " << (UINT)index;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
}

/*===========================================================================
METHOD:
   GetActiveMobileIPProfile (Public Method)

DESCRIPTION:
   This function gets the the active mobile IP profile index

PARAMETERS:
   pIndex      [ O ] - Active mobile IP profile index
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetActiveMobileIPProfile( BYTE * pIndex )
{
   // Validate arguments
   if (pIndex == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_ACTIVE_MIP;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the index
   *pIndex = pf[0].mValue.mU8;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetMobileIPProfile (Public Method)

DESCRIPTION:
   This function sets the specified mobile IP profile settings

PARAMETERS:
   pSPC           [ I ] - Six digit service programming code
   index          [ I ] - Mobile IP profile ID
   pEnabled       [ I ] - (Optional) Enable MIP profile?
   pAddress       [ I ] - (Optional) Home IPv4 address
   pPrimaryHA     [ I ] - (Optional) Primary home agent IPv4 address
   pSecondaryHA   [ I ] - (Optional) Secondary home agent IPv4 address
   pRevTunneling  [ I ] - (Optional) Enable reverse tunneling?
   pNAI           [ I ] - (Optional) Network access identifier string
   pHASPI         [ I ] - (Optional) HA security parameter index
   pAAASPI        [ I ] - (Optional) AAA security parameter index
   pMNHA          [ I ] - (Optional) MN-HA string
   pMNAAA         [ I ] - (Optional) MN-AAA string

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetMobileIPProfile( 
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
   if (pSPC == 0 || pSPC[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string spc( pSPC );
   if (spc.size() > 6)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   int nNonDigit = spc.find_first_not_of( "0123456789" );
   std::string digitSPC = spc.substr( 0, nNonDigit );
   if (digitSPC.size() != spc.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_SET_MIP_PROFILE;
   std::vector <sDB2PackingInput> piv;

   // "%s %u"
   std::ostringstream tmp;
   tmp << spc << " " << (UINT)index;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Enabled flag provided?
   if (pEnabled != 0)
   {
      // "%u"
      std::ostringstream tmp2;
      tmp2 << (UINT)(*pEnabled == 0 ? 0 : 1);

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // Home address provided?
   if (pAddress != 0)
   {
      ULONG ip4 = (*pAddress & 0x000000FF);
      ULONG ip3 = (*pAddress & 0x0000FF00) >> 8;
      ULONG ip2 = (*pAddress & 0x00FF0000) >> 16;
      ULONG ip1 = (*pAddress & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 17 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // Primary HA address provided?
   if (pPrimaryHA != 0)
   {
      ULONG ip4 = (*pPrimaryHA & 0x000000FF);
      ULONG ip3 = (*pPrimaryHA & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryHA & 0x00FF0000) >> 16;
      ULONG ip1 = (*pPrimaryHA & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 18 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // Secondary HA address provided?
   if (pSecondaryHA != 0)
   {
      ULONG ip4 = (*pSecondaryHA & 0x000000FF);
      ULONG ip3 = (*pSecondaryHA & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryHA & 0x00FF0000) >> 16;
      ULONG ip1 = (*pSecondaryHA & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp2;
      tmp2 << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
           << " " << (UINT)ip1;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 19 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // Reverse tunneling flag provided?
   if (pRevTunneling != 0)
   {
      // "%u"
      std::ostringstream tmp2;
      tmp2 << (UINT)(*pRevTunneling == 0 ? 0 : 1);

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 20 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // NAI provided?
   if (pNAI != 0)
   {
      std::ostringstream tmp2;
      if (pNAI[0] != 0)
      {
         tmp2 << "\"" << pNAI << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 21 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // HA SPI provided?
   if (pHASPI != 0)
   {
      // "%u"
      std::ostringstream tmp2;
      tmp2 << (UINT)*pHASPI;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 22 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // AAA SPI provided?
   if (pAAASPI != 0)
   {
      // "%u"
      std::ostringstream tmp2;
      tmp2 << (UINT)*pAAASPI;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 23 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // MN-HA key provided?
   if (pMNHA != 0)
   {
      std::ostringstream tmp2;
      if (pMNHA[0] != 0)
      {
         tmp2 << "\"" << pMNHA << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 24 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
      piv.push_back( pi );
   }

   // MN-AAA key provided?
   if (pMNAAA != 0)
   {
      std::ostringstream tmp2;
      if (pMNAAA[0] != 0)
      {
         tmp2 << "\"" << pMNAAA << "\"";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 25 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // We require at least one of the optional arguments
   if (piv.size() <= 1)
   {
      // Much ado about nothing
      return eGOBI_ERR_INVALID_ARG;
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
}

/*===========================================================================
METHOD:
   GetMobileIPProfile (Public Method)

DESCRIPTION:
   This function gets the specified mobile IP profile settings

PARAMETERS:
   index          [ I ] - Mobile IP profile ID
   pEnabled       [ O ] - MIP profile enabled?
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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetMobileIPProfile( 
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

   // Validate arguments
   if ( (pEnabled == 0)
   ||   (pAddress == 0)
   ||   (pPrimaryHA == 0)
   ||   (pSecondaryHA == 0)
   ||   (pRevTunneling == 0)
   ||   (naiSize == 0)
   ||   (pNAI == 0)
   ||   (pHASPI == 0)
   ||   (pAAASPI == 0)
   ||   (pHAState == 0)
   ||   (pAAAState == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume errors
   *pEnabled = UCHAR_MAX;
   *pAddress = ULONG_MAX;
   *pPrimaryHA = ULONG_MAX;
   *pSecondaryHA = ULONG_MAX;
   *pRevTunneling = UCHAR_MAX;
   *pHASPI = ULONG_MAX;
   *pAAASPI = ULONG_MAX;
   *pHAState = ULONG_MAX;
   *pAAAState = ULONG_MAX;

   WORD msgID = (WORD)eQMI_WDS_GET_MIP_PROFILE;
   std::vector <sDB2PackingInput> piv;

   // "%u"
   std::ostringstream arg;
   arg << (UINT)index;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)arg.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_WDS, pRequest );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );

   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() >= 1) 
   {
      *pEnabled = pf1[0].mValue.mU8;
   }

   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() >= 4) 
   {
      ULONG ip4 = (ULONG)pf2[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf2[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf2[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf2[3].mValue.mU8 << 24;
      *pAddress = (ip4 | ip3 | ip2 | ip1);
   }


   sProtocolEntityKey tlvKey3( eDB2_ET_QMI_WDS_RSP, msgID, 18 );
   cDataParser::tParsedFields pf3 = ParseTLV( db, rsp, tlvs, tlvKey3 );
   if (pf3.size() >= 4) 
   {
      ULONG ip4 = (ULONG)pf3[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf3[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf3[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf3[3].mValue.mU8 << 24;
      *pPrimaryHA = (ip4 | ip3 | ip2 | ip1);
   }

   sProtocolEntityKey tlvKey4( eDB2_ET_QMI_WDS_RSP, msgID, 19 );
   cDataParser::tParsedFields pf4 = ParseTLV( db, rsp, tlvs, tlvKey4 );
   if (pf4.size() >= 4) 
   {
      ULONG ip4 = (ULONG)pf4[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf4[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf4[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf4[3].mValue.mU8 << 24;
      *pSecondaryHA = (ip4 | ip3 | ip2 | ip1);   
   }

   sProtocolEntityKey tlvKey5( eDB2_ET_QMI_WDS_RSP, msgID, 20 );
   cDataParser::tParsedFields pf5 = ParseTLV( db, rsp, tlvs, tlvKey5 );
   if (pf5.size() >= 1) 
   {
      *pRevTunneling = pf5[0].mValue.mU8;
   }

   sProtocolEntityKey tlvKey6( eDB2_ET_QMI_WDS_RSP, msgID, 21 );
   cDataParser::tParsedFields pf6 = ParseTLV( db, rsp, tlvs, tlvKey6 );
   if (pf6.size() >= 1) 
   {
      LONG strLen = pf6[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (naiSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pNAI, (LPCSTR)pf6[0].mValueString.c_str(), strLen );
      pNAI[strLen] = 0;
   }

   sProtocolEntityKey tlvKey7( eDB2_ET_QMI_WDS_RSP, msgID, 22 );
   cDataParser::tParsedFields pf7 = ParseTLV( db, rsp, tlvs, tlvKey7 );
   if (pf7.size() >= 1) 
   {
      *pHASPI = pf7[0].mValue.mU32;
   }

   sProtocolEntityKey tlvKey8( eDB2_ET_QMI_WDS_RSP, msgID, 23 );
   cDataParser::tParsedFields pf8 = ParseTLV( db, rsp, tlvs, tlvKey8 );
   if (pf8.size() >= 1) 
   {
      *pAAASPI = pf8[0].mValue.mU32;
   }
   sProtocolEntityKey tlvKey9( eDB2_ET_QMI_WDS_RSP, msgID, 26 );
   cDataParser::tParsedFields pf9 = ParseTLV( db, rsp, tlvs, tlvKey9 );
   if (pf9.size() >= 1) 
   {
      *pHAState = pf9[0].mValue.mU32;
   }

   sProtocolEntityKey tlvKey10( eDB2_ET_QMI_WDS_RSP, msgID, 27 );
   cDataParser::tParsedFields pf10 = ParseTLV( db, rsp, tlvs, tlvKey10 );
   if (pf10.size() >= 1) 
   {
      *pAAAState = pf10[0].mValue.mU32;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetMobileIPParameters (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetMobileIPParameters( 
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
   if (pSPC == 0 || pSPC[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string spc( pSPC );
   if (spc.size() > 6)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   int nNonDigit = spc.find_first_not_of( "0123456789" );
   std::string digitSPC = spc.substr( 0, nNonDigit );
   if (digitSPC.size() != spc.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_SET_MIP_PARAMS;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)spc.c_str() );
   piv.push_back( pi );

   // Mode provided?
   if (pMode != 0)
   {
      // "%u"
      std::ostringstream tmp;
      tmp << (UINT)*pMode;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Retry limit provided?
   if (pRetryLimit != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)*pRetryLimit;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 17 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Retry interval provided?
   if (pRetryInterval != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)*pRetryInterval;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 18 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Re-registration period provided?
   if (pReRegPeriod != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)*pReRegPeriod;

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 19 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Re-registration on traffic flag provided?
   if (pReRegTraffic != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)(*pReRegTraffic == 0 ? 0 : 1);

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 20 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // HA authenticator flag provided?
   if (pHAAuthenticator != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)(*pHAAuthenticator == 0 ? 0 : 1);

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 21 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // HA RFC2002bis authentication flag provided?
   if (pHA2002bis != 0)
   {
      std::ostringstream tmp;
      tmp << (UINT)(*pHA2002bis == 0 ? 0 : 1);

      pek = sProtocolEntityKey( eDB2_ET_QMI_WDS_REQ, msgID, 22 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // We require at least one of the optional arguments
   if (piv.size() <= 1)
   {
      // Much ado about nothing
      return eGOBI_ERR_INVALID_ARG;
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
}

/*===========================================================================
METHOD:
   GetMobileIPParameters (Public Method)

DESCRIPTION:
   This function gets the mobile IP parameters

PARAMETERS:
   pMode             [ 0 ] - Current mobile IP setting
   pRetryLimit       [ 0 ] - Retry attempt limit
   pRetryInterval    [ 0 ] - Retry attempt interval
   pReRegPeriod      [ 0 ] - Re-registration period
   pReRegTraffic     [ 0 ] - Re-registration only with traffic?
   pHAAuthenticator  [ 0 ] - MH-HA authenticator calculator?
   pHA2002bis        [ 0 ] - MH-HA RFC 2002bis authentication?

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetMobileIPParameters( 
   ULONG *                    pMode,
   BYTE *                     pRetryLimit,
   BYTE *                     pRetryInterval,
   BYTE *                     pReRegPeriod,
   BYTE *                     pReRegTraffic,
   BYTE *                     pHAAuthenticator,
   BYTE *                     pHA2002bis )
{
   // Validate arguments
   if ( (pMode == 0)
   ||   (pRetryLimit == 0)
   ||   (pRetryInterval == 0)
   ||   (pReRegPeriod == 0)
   ||   (pReRegTraffic == 0)
   ||   (pHAAuthenticator == 0)
   ||   (pHA2002bis == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pMode = ULONG_MAX;
   *pRetryLimit = UCHAR_MAX;
   *pRetryInterval = UCHAR_MAX;
   *pReRegPeriod = UCHAR_MAX;
   *pReRegTraffic = UCHAR_MAX;
   *pHAAuthenticator = UCHAR_MAX;
   *pHA2002bis = UCHAR_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_MIP_PARAMS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pMode = pf[0].mValue.mU32;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pRetryLimit = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 18 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pRetryInterval = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 19 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pReRegPeriod = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 20 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pReRegTraffic = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 21 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pHAAuthenticator = pf[0].mValue.mU8;
   }


   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_WDS_RSP, msgID, 22 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pHA2002bis = pf[0].mValue.mU8;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetLastMobileIPError (Public Method)

DESCRIPTION:
   This function gets the last mobile IP error

PARAMETERS:
   pError      [ 0 ] - Last mobile IP error

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetLastMobileIPError( ULONG * pError )
{
   // Validate arguments
   if (pError == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_LAST_MIP_STATUS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the index
   *pError = (ULONG)pf[0].mValue.mU8;
   return eGOBI_ERR_NONE;
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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetDNSSettings( 
   ULONG *                    pPrimaryDNS, 
   ULONG *                    pSecondaryDNS )
{
   // Validate arguments
   if (pPrimaryDNS == 0 && pSecondaryDNS == 0)
   {
      // At least one must be specified
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_WDS_SET_DNS;
   std::vector <sDB2PackingInput> piv;

   if (pPrimaryDNS != 0)
   {
      ULONG ip4 = (*pPrimaryDNS & 0x000000FF);
      ULONG ip3 = (*pPrimaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pPrimaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pPrimaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 16 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pSecondaryDNS != 0)
   {
      ULONG ip4 = (*pSecondaryDNS & 0x000000FF);
      ULONG ip3 = (*pSecondaryDNS & 0x0000FF00) >> 8;
      ULONG ip2 = (*pSecondaryDNS & 0x00FF0000) >> 16;
      ULONG ip1 = (*pSecondaryDNS & 0xFF000000) >> 24;

      // "%u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
          << " " << (UINT)ip1;

      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 17 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Pack up and send the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_WDS, pRequest );
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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetDNSSettings( 
   ULONG *                    pPrimaryDNS, 
   ULONG *                    pSecondaryDNS )
{
   // Validate arguments
   if (pPrimaryDNS == 0 || pSecondaryDNS == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pPrimaryDNS = 0;
   *pSecondaryDNS = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_DNS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_WDS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 4)
   {
      ULONG ip4 = (ULONG)pf[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
      *pPrimaryDNS = (ip4 | ip3 | ip2 | ip1);   
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 4)
   {
      ULONG ip4 = (ULONG)pf[0].mValue.mU8;
      ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
      ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
      ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
      *pSecondaryDNS = (ip4 | ip3 | ip2 | ip1);   
   }

   return eGOBI_ERR_NONE;
}
