/*===========================================================================
FILE: 
   GobiQMICoreOMA.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (OMA-DM Service)

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
   OMADMStartSession (Public Method)

DESCRIPTION:
   This function starts an OMA-DM session

PARAMETERS:
   sessionType [ I ] - Type of session to initiate

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMStartSession( ULONG sessionType )
{
   WORD msgID = (WORD)eQMI_OMA_START_SESSION;
   std::vector <sDB2PackingInput> piv;
   
   std::ostringstream tmp;
   tmp << sessionType;

   sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_OMA, pRequest );
}

/*===========================================================================
METHOD:
   OMADMCancelSession (Public Method)

DESCRIPTION:
   This function cancels an ongoing OMA-DM session

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMCancelSession()
{
   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_OMA_CANCEL_SESSION;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_OMA, msgID );
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

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   OMADMGetSessionInfo (Public Method)

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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMGetSessionInfo( 
   ULONG *                    pSessionState,
   ULONG *                    pSessionType,
   ULONG *                    pFailureReason,
   BYTE *                     pRetryCount,
   WORD *                     pSessionPause,
   WORD *                     pTimeRemaining )
{
   // Validate arguments
   if ( (pSessionState == 0)
   ||   (pSessionType == 0)
   ||   (pFailureReason == 0)
   ||   (pRetryCount == 0)
   ||   (pSessionPause == 0)
   ||   (pTimeRemaining == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSessionState = ULONG_MAX;
   *pSessionType = ULONG_MAX;
   *pFailureReason = ULONG_MAX;
   *pRetryCount = UCHAR_MAX;
   *pSessionPause = USHRT_MAX;
   *pTimeRemaining = USHRT_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_OMA_GET_SESSION_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_OMA, msgID );
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

   // How many parameters did we populate?
   ULONG params = 0;

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLVs we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_OMA_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 2) 
   {
      *pSessionState = pf[0].mValue.mU32;
      *pSessionType = pf[1].mValue.mU32;
      params += 2;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_OMA_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pFailureReason = pf[0].mValue.mU32;
      params++;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_OMA_RSP, msgID, 18 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 3) 
   {
      *pRetryCount = pf[0].mValue.mU8;
      *pSessionPause = pf[1].mValue.mU16;
      *pTimeRemaining = pf[2].mValue.mU16;
      params += 3;
   }

   if (params == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   OMADMGetPendingNIA (Public Method)

DESCRIPTION:
   This function returns information about the pending network initiated
   alert

PARAMETERS:
   pSessionType   [ O ] - Type of session
   pSessionID     [ O ] - Unique session ID

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMGetPendingNIA( 
   ULONG *                    pSessionType,
   USHORT *                   pSessionID )
{
   // Validate arguments
   if (pSessionType == 0 || pSessionID == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSessionType = ULONG_MAX;
   *pSessionID = USHRT_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_OMA_GET_SESSION_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_OMA, msgID );
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

   // How many parameters did we populate?
   ULONG params = 0;

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLVs we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_OMA_RSP, msgID, 19 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 2) 
   {
      *pSessionType = pf[0].mValue.mU32;
      *pSessionID = pf[1].mValue.mU16;
      params += 2;
   }

   if (params == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   OMADMSendSelection (Public Method)

DESCRIPTION:
   This function sends the specified OMA-DM selection for the current 
   network initiated session

PARAMETERS:
   selection   [ I ] - Selection
   sessionID   [ I ] - Unique session ID

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMSendSelection( 
   ULONG                      selection,
   USHORT                     sessionID )
{
   WORD msgID = (WORD)eQMI_OMA_SEND_SELECTION;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << selection << " " << sessionID;

   sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_OMA, pRequest );
}

/*===========================================================================
METHOD:
   OMADMGetFeatureSettings (Public Method)

DESCRIPTION:
   This function returns the OMA-DM feature settings

PARAMETERS:
   pbProvisioning [ O ] - Device provisioning service update enabled
   pbPRLUpdate    [ O ] - PRL service update enabled

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMGetFeatureSettings( 
   ULONG *                    pbProvisioning,
   ULONG *                    pbPRLUpdate )
{
   // Validate arguments
   if (pbProvisioning == 0 || pbPRLUpdate == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pbProvisioning = ULONG_MAX;
   *pbPRLUpdate = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_OMA_GET_FEATURES;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_OMA, msgID );
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

   // How many parameters did we populate?
   ULONG params = 0;

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLVs we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_OMA_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pbProvisioning = pf[0].mValue.mU32;
      params++;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_OMA_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pbPRLUpdate = pf[0].mValue.mU32;
      params++;
   }

   if (params == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   OMADMSetProvisioningFeature (Public Method)

DESCRIPTION:
   This function sets the OMA-DM device provisioning service 
   update feature setting

PARAMETERS:
   bProvisioning  [ I ] - Device provisioning service update enabled

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMSetProvisioningFeature( 
   ULONG                      bProvisioning )
{
   WORD msgID = (WORD)eQMI_OMA_SET_FEATURES;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (ULONG)(bProvisioning != 0);

   sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_OMA, pRequest );
}

/*===========================================================================
METHOD:
   OMADMSetPRLUpdateFeature (Public Method)

DESCRIPTION:
   This function sets the OMA-DM PRL service update feature setting

PARAMETERS:
   bPRLUpdate     [ I ] - PRL service update enabled

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OMADMSetPRLUpdateFeature( 
   ULONG                      bPRLUpdate )
{
   WORD msgID = (WORD)eQMI_OMA_SET_FEATURES;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (ULONG)(bPRLUpdate != 0);

   sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 17 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_OMA, pRequest );
}
