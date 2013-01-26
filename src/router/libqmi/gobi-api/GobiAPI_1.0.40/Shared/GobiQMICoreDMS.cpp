/*===========================================================================
FILE: 
   GobiQMICoreDMS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (DMS Service)

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
   GetDeviceCapabilities (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetDeviceCapabilities( 
   ULONG *                    pMaxTXChannelRate, 
   ULONG *                    pMaxRXChannelRate, 
   ULONG *                    pDataServiceCapability, 
   ULONG *                    pSimCapability, 
   ULONG *                    pRadioIfacesSize, 
   BYTE *                     pRadioIfaces )
{
   // Validate arguments
   if ( (pMaxTXChannelRate == 0)
   ||   (pMaxRXChannelRate == 0)
   ||   (pDataServiceCapability == 0)
   ||   (pSimCapability == 0)
   ||   (pRadioIfacesSize == 0)
   ||   (*pRadioIfacesSize == 0)
   ||   (pRadioIfaces == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxRadioIfaces = (ULONG)*pRadioIfacesSize;

   // Assume failure
   *pRadioIfacesSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_CAPS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 5) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the variables
   *pMaxTXChannelRate = pf[0].mValue.mU32;
   *pMaxRXChannelRate = pf[1].mValue.mU32;
   *pDataServiceCapability = pf[2].mValue.mU32;
   *pSimCapability = pf[3].mValue.mU8;

   ULONG activeRadioIfaces = (ULONG)pf[4].mValue.mU8;
   if (pf.size() < 5 + activeRadioIfaces)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (activeRadioIfaces > maxRadioIfaces)
   {
      activeRadioIfaces = maxRadioIfaces;
   }

   ULONG * pOutRadioIfaces = (ULONG *)pRadioIfaces;
   for (ULONG r = 0; r < activeRadioIfaces; r++)
   {
      *pOutRadioIfaces++ = pf[5 + r].mValue.mU32;
   }

   *pRadioIfacesSize = activeRadioIfaces;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetManufacturer (Public Method)

DESCRIPTION:
   This function returns the device manufacturer name

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetManufacturer( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_MANUFACTURER;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   LONG strLen = pf[0].mValueString.size();
   if (strLen <= 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Space to perform the copy?
   if (stringSize < strLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)pf[0].mValueString.c_str(), strLen );
   pString[strLen] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetModelID (Public Method)

DESCRIPTION:
   This function returns the device model ID

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetModelID( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_MODEL_ID;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   LONG strLen = pf[0].mValueString.size();
   if (strLen <= 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Space to perform the copy?
   if (stringSize < strLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)pf[0].mValueString.c_str(), strLen );
   pString[strLen] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetFirmwareRevision (Public Method)

DESCRIPTION:
   This function returns the device firmware revision

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetFirmwareRevision( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_REV_ID;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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

   // Parse the TLV we want (PRI revision)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string tmpPRI = pf[0].mValueString;
   ULONG lenPRI = (ULONG)tmpPRI.size();

   // Space to perform the copy?
   if (stringSize < lenPRI + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)tmpPRI.c_str(), lenPRI + 1 );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetFirmwareRevisions (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetFirmwareRevisions( 
   BYTE                       amssSize, 
   CHAR *                     pAMSSString,
   BYTE                       bootSize, 
   CHAR *                     pBootString,
   BYTE                       priSize, 
   CHAR *                     pPRIString )
{
   // Validate arguments
   if ( (amssSize == 0 || pAMSSString == 0) 
   ||   (bootSize == 0 || pBootString == 0)
   ||   (priSize == 0 || pPRIString == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pAMSSString = 0;
   *pBootString = 0;
   *pPRIString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_REV_ID;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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

   // Parse the TLV we want (PRI revision)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string tmpPRI = pf[0].mValueString;
   ULONG lenPRI = (ULONG)tmpPRI.size();

   // Space to perform the copy?
   if (priSize < lenPRI + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pPRIString, (LPCSTR)tmpPRI.c_str(), lenPRI + 1 );

   // Parse the TLV we want (boot code revision)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string tmpBoot = pf[0].mValueString;
   ULONG lenBoot = (ULONG)tmpBoot.size();

   // Space to perform the copy?
   if (bootSize < lenBoot + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pBootString, (LPCSTR)tmpBoot.c_str(), lenBoot + 1 );

   // Parse the TLV we want (AMSS revision)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string tmpAMSS = pf[0].mValueString;
   ULONG lenAMSS = (ULONG)tmpAMSS.size();

   // Space to perform the copy?
   if (amssSize < lenAMSS + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pAMSSString, (LPCSTR)tmpAMSS.c_str(), lenAMSS + 1 );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetVoiceNumber (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetVoiceNumber( 
   BYTE                       voiceNumberSize, 
   CHAR *                     pVoiceNumber,
   BYTE                       minSize, 
   CHAR *                     pMIN )
{
   // Validate arguments
   if (voiceNumberSize == 0 || pVoiceNumber == 0 || minSize == 0 || pMIN == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pVoiceNumber = 0;
   *pMIN = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_NUMBER;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   LONG strLen = pf1[0].mValueString.size();
   if (strLen <= 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Space to perform the copy?
   if (voiceNumberSize < strLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pVoiceNumber, (LPCSTR)pf1[0].mValueString.c_str(), strLen );
   pVoiceNumber[strLen] = 0;

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() >= 1) 
   {
      strLen = pf2[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (minSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pMIN, (LPCSTR)pf2[0].mValueString.c_str(), strLen );
      pMIN[strLen] = 0;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetIMSI (Public Method)

DESCRIPTION:
   This function returns the device IMSI

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetIMSI( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_IMSI;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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

   // Parse the TLV we want (IMSI)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string tmpIMSI = pf[0].mValueString;
   ULONG lenIMSI = (ULONG)tmpIMSI.size();

   // Space to perform the copy?
   if (stringSize < lenIMSI + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)tmpIMSI.c_str(), lenIMSI + 1 );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSerialNumbers (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSerialNumbers( 
   BYTE                       esnSize, 
   CHAR *                     pESNString, 
   BYTE                       imeiSize, 
   CHAR *                     pIMEIString, 
   BYTE                       meidSize, 
   CHAR *                     pMEIDString )
{
   // Validate arguments
   if ( (esnSize == 0)
   ||   (pESNString == 0)
   ||   (imeiSize == 0)
   ||   (pIMEIString == 0)
   ||   (meidSize == 0)
   ||   (pMEIDString == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pESNString = 0;
   *pIMEIString = 0;
   *pMEIDString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_IDS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() >= 1) 
   {
      LONG strLen = pf1[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (esnSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pESNString, (LPCSTR)pf1[0].mValueString.c_str(), strLen );
      pESNString[strLen] = 0;
   }

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() >= 1) 
   {
      LONG strLen = pf2[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (imeiSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pIMEIString, (LPCSTR)pf2[0].mValueString.c_str(), strLen );
      pIMEIString[strLen] = 0;
   }

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey3( eDB2_ET_QMI_DMS_RSP, msgID, 18 );
   cDataParser::tParsedFields pf3 = ParseTLV( db, rsp, tlvs, tlvKey3 );
   if (pf3.size() >= 1) 
   {
      LONG strLen = pf3[0].mValueString.size();
      if (strLen <= 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      // Space to perform the copy?
      if (meidSize < strLen + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pMEIDString, (LPCSTR)pf3[0].mValueString.c_str(), strLen );
      pMEIDString[strLen] = 0;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetLock (Public Method)

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   state       [ I ] - Desired lock state
   pCurrentPIN [ I ] - Current four digit PIN string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetLock( 
   ULONG                      state, 
   CHAR *                     pCurrentPIN )
{
   // Validate arguments
   if (pCurrentPIN == 0 || pCurrentPIN[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string thePIN( pCurrentPIN );
   if (thePIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   int nNonDigit = thePIN.find_first_not_of( "0123456789" );
   std::string digitPIN = thePIN.substr( 0, nNonDigit );
   if (digitPIN.size() != thePIN.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_DMS_SET_USER_LOCK_STATE;
   std::vector <sDB2PackingInput> piv;

   // "%u %s"
   std::ostringstream tmp;
   tmp << (UINT)state << " " << thePIN;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}

/*===========================================================================
METHOD:
   QueryLock (Public Method)

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   pState      [ O ] - Current lock state
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::QueryLock( ULONG * pState )
{
   // Validate arguments
   if (pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_USER_LOCK_STATE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the mode
   *pState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ChangeLockPIN (Public Method)

DESCRIPTION:
   This command sets the user lock code maintained by the device

PARAMETERS:
   pCurrentPIN [ O ] - Current four digit PIN string
   pDesiredPIN [ O ] - New four digit PIN string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ChangeLockPIN( 
   CHAR *                     pCurrentPIN, 
   CHAR *                     pDesiredPIN )
{
   // Validate arguments
   if ( (pCurrentPIN == 0) 
   ||   (pCurrentPIN[0] == 0)
   ||   (pDesiredPIN == 0) 
   ||   (pDesiredPIN[0] == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theCurPIN( pCurrentPIN );
   if (theCurPIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   int nNonDigit = theCurPIN.find_first_not_of( "0123456789" );
   std::string digitCurPIN = theCurPIN.substr( 0, nNonDigit );
   if (digitCurPIN.size() != theCurPIN.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theNewPIN( pDesiredPIN );
   if (theNewPIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   nNonDigit = theNewPIN.find_first_not_of( "0123456789" );
   std::string digitNewPIN = theNewPIN.substr( 0, nNonDigit );
   if (digitNewPIN.size() != theNewPIN.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_DMS_SET_USER_LOCK_CODE;
   std::vector <sDB2PackingInput> piv;

   // "%s %s"
   std::ostringstream tmp;
   tmp << theCurPIN << " " << theNewPIN;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}

/*===========================================================================
METHOD:
   GetHardwareRevision (Public Method)

DESCRIPTION:
   This function returns the device hardware revision

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetHardwareRevision( 
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Assume failure
   *pString = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_MSM_ID;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   LONG strLen = pf[0].mValueString.size();
   if (strLen <= 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Space to perform the copy?
   if (stringSize < strLen + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)pf[0].mValueString.c_str(), strLen );
   pString[strLen] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetPRLVersion (Public Method)

DESCRIPTION:
   This function returns the version of the active Preferred Roaming List 
   (PRL) in use by the device

PARAMETERS:
   pPRLVersion [ O ] - The PRL version number
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPRLVersion( WORD * pPRLVersion )
{
   // Validate arguments
   if (pPRLVersion == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_PRL_VERSION;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pPRLVersion = pf[0].mValue.mU16;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetERIFile (Public Method)

DESCRIPTION:
   This command returns the ERI file that is stored in EFS on the device

PARAMETERS:
   pFileSize   [I/O] - Upon input the maximum number of bytes that the file 
                       contents array can contain.  Upon successful output 
                       the actual number of bytes written to the file contents 
                       array
   pFile       [ O ] - The file contents
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetERIFile( 
   ULONG *                    pFileSize, 
   BYTE *                     pFile )
{
   // Validate arguments
   if (pFileSize == 0 || *pFileSize == 0 || pFile == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   ULONG maxFileSize = *pFileSize;
   *pFileSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_READ_ERI_FILE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG fileSz = pf[0].mValue.mU16;
   if (pf.size() < 1 + fileSz) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Space to copy into?
   if (fileSz > maxFileSize)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   for (ULONG f = 0; f < fileSz; f++)
   {
      pFile[f] = pf[1 + f].mValue.mU8;
   }

   *pFileSize = fileSz;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ActivateAutomatic (Public Method)

DESCRIPTION:
   This function requests the device to perform automatic service activation

PARAMETERS:
   pActivationCode   [ I ] - Activation code (maximum string length of 12) 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ActivateAutomatic( CHAR * pActivationCode )
{
   // Validate arguments
   if ( (pActivationCode == 0) 
   ||   (pActivationCode[0] == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string ac( pActivationCode );
   if (ac.size() > 12)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_DMS_ACTIVATE_AUTOMATIC;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (ULONG)ac.size() << " \"" << ac << "\"";

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   
   /// Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest, 300000 );
}

/*===========================================================================
METHOD:
   ActivateManual (Public Method)

DESCRIPTION:
   This function requests the device perform manual service activation,
   after a successful request the device is then asked to reset

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ActivateManual( 
   CHAR *                     pSPC,
   WORD                       sid, 
   CHAR *                     pMDN,
   CHAR *                     pMIN, 
   ULONG                      prlSize, 
   BYTE *                     pPRL, 
   CHAR *                     pMNHA,
   CHAR *                     pMNAAA )
{
   // Validate arguments
   if ( (pSPC == 0) 
   ||   (pSPC[0] == 0)
   ||   (pMDN == 0)
   ||   (pMDN[0] == 0)
   ||   (pMIN == 0)
   ||   (pMIN[0] == 0)
   ||   (prlSize > QMI_DMS_MAX_PRL_SIZE) )
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

   std::string theMDN( pMDN );
   if (theMDN.size() > 16)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   nNonDigit = theMDN.find_first_not_of( "0123456789" );
   std::string digitMDN = theMDN.substr( 0, nNonDigit );
   if (digitMDN.size() != theMDN.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theMIN( pMIN );
   if (theMIN.size() > 16)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   nNonDigit = theMIN.find_first_not_of( "0123456789" );
   std::string digitMIN = theMIN.substr( 0, nNonDigit );
   if (digitMIN.size() != theMIN.size())
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theMNHA;
   if (pMNHA != 0 && pMNHA[0] != 0)
   {
      theMNHA = pMNHA;
      if (theMNHA.size() > 16)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }

   std::string theMNAAA;
   if (pMNAAA != 0 && pMNAAA[0] != 0)
   {
      theMNAAA = pMNAAA;
      if (theMNAAA.size() > 16)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }

   WORD msgID = (WORD)eQMI_DMS_ACTIVATE_MANUAL;
   std::vector <sDB2PackingInput> piv;

   // "%s %u %d %s %d %s"
   std::ostringstream tmp;
   tmp << spc << " " << (UINT)sid << " " << theMDN.size() << " " << theMDN
       << " " << theMIN.size() << " " << theMIN;
   
   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   if (theMNHA.size() > 0)
   {
      sProtocolEntityKey pek1( eDB2_ET_QMI_DMS_REQ, msgID, 17 );
      std::ostringstream tmp;
      tmp << (int)theMNHA.size() << " \"" << theMNHA << "\"";

      sDB2PackingInput pi1( pek1, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi1 );
   }

   if (theMNAAA.size() > 0)
   {
      sProtocolEntityKey pek1( eDB2_ET_QMI_DMS_REQ, msgID, 18 );
      std::ostringstream tmp;
      tmp << (int)theMNAAA.size() << " \"" << theMNAAA << "\"";

      sDB2PackingInput pi1( pek1, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi1 );
   }

   // Do we need to go through the anguish of the segmented PRL?
   if (prlSize > 0)
   {
      ULONG blockSz = QMI_DMS_MAX_PRL_BLOCK;

      // Determine number of writes
      ULONG writes = prlSize / blockSz;
      if ((prlSize % blockSz) != 0)
      {
         writes++;
      }

      ULONG offset = 0;
      ULONG to = DEFAULT_GOBI_QMI_TIMEOUT;

      // Generate and send requests
      eGobiError err = eGOBI_ERR_NONE;
      for (ULONG w = 0; w < writes; w++)
      {
         if (w == writes - 1)
         {
            to = 300000;
            if ((prlSize % blockSz) != 0)
            {
               blockSz = prlSize % blockSz;
            }
         }

         std::vector <sDB2PackingInput> pivLocal = piv;

         // "%u %u %u"
         std::ostringstream tmp2;
         tmp2 << (UINT)prlSize << " " << (UINT)blockSz 
              << " " << (UINT)w;
         for (ULONG p = 0; p < blockSz; p++)
         {
            tmp2 << " " << (UINT)pPRL[offset + p];
         }
         
         sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 19 );
         sDB2PackingInput pi( pek, (LPCSTR)tmp2.str().c_str() );
         pivLocal.push_back( pi );

         // Pack up the QMI request
         const cCoreDatabase & db = GetDatabase();
         sSharedBuffer * pRequest = DB2PackQMIBuffer( db, pivLocal );

         // Send the QMI request, check result, and return 
         err = SendAndCheckReturn( eQMI_SVC_DMS, pRequest, to );
         if (err != eGOBI_ERR_NONE)
         {
            break;
         }
         else
         {
            offset += blockSz;
         }
      }

      if (err != eGOBI_ERR_NONE)
      {
         return err;
      }
   }
   else
   {
      // Pack up the QMI request
      const cCoreDatabase & db = GetDatabase();
      sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

      // Send the QMI request, check result, and return 
      eGobiError rc = SendAndCheckReturn( eQMI_SVC_DMS, pRequest, 300000 );
      if (rc != eGOBI_ERR_NONE)
      {
         return rc;
      }
   }

   // Ask device to power down
   eGobiError rc = SetPower( 5 );
   if (rc != eGOBI_ERR_NONE)
   {
      return eGOBI_ERR_RESET;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ResetToFactoryDefaults (Public Method)

DESCRIPTION:
   This function requests the device reset configuration to factory defaults,
   after a successful request the device is then asked to reset

PARAMETERS:
   pSPC        [ I ] - NULL terminated string representing the six digit 
                       service programming code

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ResetToFactoryDefaults( CHAR * pSPC )
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

   WORD msgID = (WORD)eQMI_DMS_FACTORY_DEFAULTS;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)spc.c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request and check result
   eGobiError rc = SendAndCheckReturn( eQMI_SVC_DMS, pRequest, 300000 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Ask device to power down
   rc = SetPower( 5 );
   if (rc != eGOBI_ERR_NONE)
   {
      return eGOBI_ERR_RESET;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetActivationState (Public Method)

DESCRIPTION:
   This function returns the device activation state

PARAMETERS:
   pActivationState  [ O ] - Service activation state
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetActivationState( ULONG * pActivationState )
{
   // Validate arguments
   if (pActivationState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_ACTIVATED_STATE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pActivationState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetPower (Public Method)

DESCRIPTION:
   This function sets the operating mode of the device

PARAMETERS:
   powerMode   [ I ] - Selected operating mode
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetPower( ULONG powerMode )
{
   WORD msgID = (WORD)eQMI_DMS_SET_OPERATING_MODE;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)powerMode;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}

/*===========================================================================
METHOD:
   GetPower (Public Method)

DESCRIPTION:
   This function returns the operating mode of the device

PARAMETERS:
   pPowerMode  [ O ] - Current operating mode
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPower( ULONG * pPowerMode )
{
   // Validate arguments
   if (pPowerMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pPowerMode = ULONG_MAX;

   ULONG reasonMask = 0;
   ULONG bPlatform = 0;
   eGobiError rc = GetPowerInfo( pPowerMode, &reasonMask, &bPlatform );
   return rc;
}

/*===========================================================================
METHOD:
   GetPowerInfo (Public Method)

DESCRIPTION:
   This function returns operating mode info from the device

PARAMETERS:
   pPowerMode  [ O ] - Current operating mode
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPowerInfo(
   ULONG *                    pPowerMode,
   ULONG *                    pReasonMask,
   ULONG *                    pbPlatform )
{
   // Validate arguments
   if (pPowerMode == 0 || pReasonMask == 0 || pbPlatform == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pPowerMode = ULONG_MAX;
   *pReasonMask = 0;
   *pbPlatform = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_OPERTAING_MODE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pPowerMode = pf[0].mValue.mU32;

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   
   // Convert back to a bitmask
   ULONG fieldCount = (ULONG)pf.size();
   if (fieldCount > 16)
   {
      fieldCount = 16;
   }

   for (ULONG f = 0; f < fieldCount; f++)
   {  
      ULONG val = (ULONG)pf[f].mValue.mU8 & 0x00000001;
      *pReasonMask |= (val << f);
   }

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pbPlatform = (ULONG)pf[0].mValue.mU8;
   }

   return eGOBI_ERR_NONE;
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
eGobiError cGobiQMICore::GetOfflineReason( 
   ULONG *                    pReasonMask,
   ULONG *                    pbPlatform )
{
   // Validate arguments
   if (pReasonMask == 0 || pbPlatform == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pReasonMask = 0;
   *pbPlatform = 0;

   ULONG powerMode = 0;
   eGobiError rc = GetPowerInfo( &powerMode, pReasonMask, pbPlatform );

   return rc;
}

/*===========================================================================
METHOD:
   GetNetworkTime (Public Method)

DESCRIPTION:
   This function returns the current time of the device

PARAMETERS:
   pTimeStamp  [ O ] - Count of 1.25ms that have elapsed from the start 
                       of GPS time (Jan 6, 1980)
   pTimeSource [ O ] - Source of the timestamp

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetNetworkTime( 
   ULONGLONG *                pTimeCount,  
   ULONG *                    pTimeSource )
{
   // Validate arguments
   if (pTimeCount == 0 || pTimeSource == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_TIME;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pTimeCount = pf[0].mValue.mU64;
   *pTimeSource = pf[1].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ValidateSPC (Public Method)

DESCRIPTION:
   This function validates the service programming code

PARAMETERS:
   pSPC        [ I ] - Six digit service programming code

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ValidateSPC( CHAR * pSPC )
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

   WORD msgID = (WORD)eQMI_DMS_VALIDATE_SPC;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)spc.c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request and check result
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}
