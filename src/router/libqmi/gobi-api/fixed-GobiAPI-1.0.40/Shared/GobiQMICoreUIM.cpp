/*===========================================================================
FILE: 
   GobiQMICoreUIM.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (UIM Access)

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
   UIMSetPINProtection (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMSetPINProtection( 
   ULONG                      id, 
   ULONG                      bEnable,
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if ( (pValue == 0) 
   ||   (pValue[0] == 0) 
   ||   (pVerifyRetriesLeft == 0)
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_SET_PIN_PROT;
   std::vector <sDB2PackingInput> piv;

   std::string val( pValue );
   ULONG valSz = val.size();

   if (bEnable != 0)
   {
      bEnable = 1;
   }

   // "%u %u %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)bEnable << " " << (UINT)valSz
       << " \"" << val << "\"";

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 2) 
      {
         *pVerifyRetriesLeft = (ULONG)pf[0].mValue.mU8;
         *pUnblockRetriesLeft = (ULONG)pf[1].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMVerifyPIN (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMVerifyPIN( 
   ULONG                      id, 
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if ( (pValue == 0) 
   ||   (pValue[0] == 0) 
   ||   (pVerifyRetriesLeft == 0)
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_PIN_VERIFY;
   std::vector <sDB2PackingInput> piv;

   std::string val( pValue );
   ULONG valSz = val.size();

   // "%u %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)valSz << " \"" << val << "\"";

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 2) 
      {
         *pVerifyRetriesLeft = (ULONG)pf[0].mValue.mU8;
         *pUnblockRetriesLeft = (ULONG)pf[1].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMUnblockPIN (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMUnblockPIN( 
   ULONG                      id, 
   CHAR *                     pPUKValue,
   CHAR *                     pNewValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if ( (pPUKValue == 0) 
   ||   (pPUKValue[0] == 0) 
   ||   (pNewValue == 0) 
   ||   (pNewValue[0] == 0) 
   ||   (pVerifyRetriesLeft == 0)
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_PIN_UNBLOCK;
   std::vector <sDB2PackingInput> piv;

   std::string val1( pPUKValue );
   ULONG val1Sz = val1.size();

   std::string val2( pNewValue );
   ULONG val2Sz = val2.size();

   // "%u %u \"%s\" %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)val1Sz << " \"" << val1 << "\" "
       << (UINT)val2Sz << " \"" << val2 << "\"";

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 2) 
      {
         *pVerifyRetriesLeft = (ULONG)pf[0].mValue.mU8;
         *pUnblockRetriesLeft = (ULONG)pf[1].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMChangePIN (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMChangePIN( 
   ULONG                      id, 
   CHAR *                     pOldValue,
   CHAR *                     pNewValue,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if ( (pOldValue == 0) 
   ||   (pOldValue[0] == 0) 
   ||   (pNewValue == 0) 
   ||   (pNewValue[0] == 0) 
   ||   (pVerifyRetriesLeft == 0)
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_PIN_CHANGE;
   std::vector <sDB2PackingInput> piv;

   std::string val1( pOldValue );
   ULONG val1Sz = val1.size();

   std::string val2( pNewValue );
   ULONG val2Sz = val2.size();

   // "%u %u \"%s\" %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)val1Sz << " \"" << val1 << "\" "
       << (UINT)val2Sz << " \"" << val2 << "\"";

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 2) 
      {
         *pVerifyRetriesLeft = (ULONG)pf[0].mValue.mU8;
         *pUnblockRetriesLeft = (ULONG)pf[1].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMGetPINStatus (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMGetPINStatus( 
   ULONG                      id, 
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pStatus == 0 || pVerifyRetriesLeft == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pStatus = ULONG_MAX;
   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_UIM_GET_PIN_STATUS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID, 5000 );
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

   ULONG tlvID = 16 + id;

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, tlvID );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pStatus = pf[0].mValue.mU32;
   *pVerifyRetriesLeft = (ULONG)pf[1].mValue.mU8;
   *pUnblockRetriesLeft = (ULONG)pf[2].mValue.mU8;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMGetICCID (Public Method)

DESCRIPTION:
   This function returns the UIM ICCID

PARAMETERS:
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMGetICCID( 
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
   WORD msgID = (WORD)eQMI_DMS_UIM_GET_ICCID;
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

   std::string tmpICCID = pf[0].mValueString;
   ULONG lenICCID = (ULONG)tmpICCID.size();

   // Space to perform the copy?
   if (stringSize < lenICCID + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pString, (LPCSTR)tmpICCID.c_str(), lenICCID + 1 );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMGetControlKeyBlockingStatus (Public Method)

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
   pbBlocking           [ O ] - (Optional) Is the facility blocking?
 
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMGetControlKeyBlockingStatus( 
   ULONG                      id, 
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft,
   ULONG *                    pbBlocking )
{
   // Validate arguments
   if ( (pStatus == 0) 
   ||   (pVerifyRetriesLeft == 0)
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pStatus = ULONG_MAX;
   *pVerifyRetriesLeft = ULONG_MAX;
   *pUnblockRetriesLeft = ULONG_MAX;

   if (pbBlocking != 0)
   {
      *pbBlocking = 0;
   }

   WORD msgID = (WORD)eQMI_DMS_UIM_GET_CK_STATUS;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)id;
   
   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

   // Parse the required TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pStatus = pf[0].mValue.mU32;
   *pVerifyRetriesLeft = (ULONG)pf[1].mValue.mU8;
   *pUnblockRetriesLeft = (ULONG)pf[2].mValue.mU8;

   if (pbBlocking != 0)
   {  
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() > 0)
      {
         *pbBlocking = 1;
      }
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMSetControlKeyProtection (Public Method)

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
eGobiError cGobiQMICore::UIMSetControlKeyProtection( 
   ULONG                      id, 
   ULONG                      status,
   CHAR *                     pValue,
   ULONG *                    pVerifyRetriesLeft )
{
   // Validate arguments
   if ( (pValue == 0) 
   ||   (pValue[0] == 0) 
   ||   (pVerifyRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pVerifyRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_SET_CK_PROT;
   std::vector <sDB2PackingInput> piv;

   std::string val( pValue );
   ULONG valSz = val.size();

   //"%u %u %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)status << " " << (UINT)valSz
       << " \"" << val << "\"";
   
   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 1) 
      {
         *pVerifyRetriesLeft = (ULONG)pf[0].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   UIMUnblockControlKey (Public Method)

DESCRIPTION:
   This function unblocks the specified facility control key

PARAMETERS:
   id                   [ I ] - Facility ID
   pValue               [ I ] - Control key de-personalization string
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked (0xFFFFFFFF = unknown)
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
eGobiError cGobiQMICore::UIMUnblockControlKey( 
   ULONG                      id, 
   CHAR *                     pValue,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if ( (pValue == 0) 
   ||   (pValue[0] == 0) 
   ||   (pUnblockRetriesLeft == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pUnblockRetriesLeft = ULONG_MAX;

   WORD msgID = (WORD)eQMI_DMS_UIM_UNBLOCK_CK;
   std::vector <sDB2PackingInput> piv;

   std::string val( pValue );
   ULONG valSz = val.size();

   // "%u %u \"%s\""
   std::ostringstream tmp;
   tmp << (UINT)id << " " << (UINT)valSz << " \"" << val << "\"";
   
   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
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
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest, 5000 );
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

      // Parse the optional TLV we want (by DB key)
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
      if (pf.size() >= 1) 
      {
         *pUnblockRetriesLeft = (ULONG)pf[0].mValue.mU8;
      }

      return GetCorrectedQMIError( ec );
   }

   return eGOBI_ERR_NONE;
}
