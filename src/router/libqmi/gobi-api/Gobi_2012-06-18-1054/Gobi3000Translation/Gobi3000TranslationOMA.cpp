/*===========================================================================
FILE: 
   Gobi3000TranslationOMA.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (OMADM Service)

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
==========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "Gobi3000Translation.h"

/*===========================================================================
METHOD:
   PackOMADMStartSession

DESCRIPTION:
   This function starts an OMA-DM session

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   sessionType    [ I ] - Type of session to initiate

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackOMADMStartSession(
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      sessionType )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add sessionType
   
   // Check size
   WORD tlvx10Sz = sizeof( sOMAStartSessionRequest_Type );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sOMAStartSessionRequest_Type * pTLVx10;
   pTLVx10 = (sOMAStartSessionRequest_Type*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mSessionType = (eQMIOMASessionTypes)sessionType;
   
   offset += tlvx10Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseOMADMGetSessionInfo

DESCRIPTION:
   This function returns information related to the current (or previous
   if no session is active) OMA-DM session

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pSessionState  [ O ] - State of session
   pSessionType   [ O ] - Type of session
   pFailureReason [ O ] - Session failure reason (when state indicates failure)
   pRetryCount    [ O ] - Session retry count (when state indicates retrying)
   pSessionPause  [ O ] - Session pause timer (when state indicates retrying)
   pTimeRemaining [ O ] - Pause time remaining (when state indicates retrying)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseOMADMGetSessionInfo( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pSessionState,
   ULONG *                    pSessionType,
   ULONG *                    pFailureReason,
   BYTE *                     pRetryCount,
   WORD *                     pSessionPause,
   WORD *                     pTimeRemaining )
{
   // Validate arguments
   if (pIn == 0 || pSessionState == 0 || pSessionType == 0 
   || pFailureReason == 0 || pRetryCount == 0 || pSessionPause == 0 
   || pTimeRemaining == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the first TLV
   const sOMAGetSessionInfoResponse_Info * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sOMAGetSessionInfoResponse_Info ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pSessionState = pTLVx10->mSessionState;
   *pSessionType = pTLVx10->mSessionType;
   
   // Find the second TLV
   const sOMAGetSessionInfoResponse_Failure * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx11 < sizeof( sOMAGetSessionInfoResponse_Failure ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pFailureReason = pTLVx11->mSessionFailure;

   // Find the third TLV
   const sOMAGetSessionInfoResponse_Retry * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx12 < sizeof( sOMAGetSessionInfoResponse_Retry ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pRetryCount = pTLVx12->mRetryCount;
   *pSessionPause = pTLVx12->mRetryPauseTimer;
   *pTimeRemaining = pTLVx12->mRemainingTime;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseOMADMGetPendingNIA

DESCRIPTION:
   This function returns information about the pending network initiated
   alert

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pSessionType   [ O ] - Type of session
   pSessionID     [ O ] - Unique session ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseOMADMGetPendingNIA( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pSessionType,
   USHORT *                   pSessionID )
{
   // Validate arguments
   if (pIn == 0 || pSessionType == 0 || pSessionID == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sOMAGetSessionInfoResponse_NIA * pTLVx13;
   ULONG outLenx13;
   ULONG rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx13 < sizeof( sOMAGetSessionInfoResponse_NIA ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pSessionID = pTLVx13->mSessionID;
   *pSessionType = pTLVx13->mSessionType;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackOMADMSendSelection

DESCRIPTION:
   This function sends the specified OMA-DM selection for the current 
   network initiated session

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   selection   [ I ] - Selection
   sessionID   [ I ] - Unique session ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackOMADMSendSelection( 
   ULONG *  pOutLen,
   BYTE *   pOut,
   ULONG    selection,
   USHORT   sessionID )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add selection and session ID
   
   // Check size
   WORD tlvx10Sz = sizeof( sOMASendSelectionRequest_Type );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sOMASendSelectionRequest_Type * pTLVx10;
   pTLVx10 = (sOMASendSelectionRequest_Type*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the values
   pTLVx10->mSelection = (eQMIOMASelections)selection;
   pTLVx10->mSessionID = sessionID;
   
   offset += tlvx10Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseOMADMGetFeatureSettings

DESCRIPTION:
   This function returns the OMA-DM feature settings

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pbProvisioning [ O ] - Device provisioning service update enabled
   pbPRLUpdate    [ O ] - PRL service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseOMADMGetFeatureSettings( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pbProvisioning,
   ULONG *                    pbPRLUpdate )
{
   // Validate arguments
   if (pIn == 0 || pbProvisioning == 0 || pbPRLUpdate == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the first TLV
   const sOMAGetFeaturesResponse_Provisioning * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sOMAGetFeaturesResponse_Provisioning ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbProvisioning = pTLVx10->mDeviceProvisioningServiceUpdateEnabled;

   // Find the second TLV
   const sOMAGetFeaturesResponse_PRLUpdate * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sOMAGetFeaturesResponse_PRLUpdate ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbPRLUpdate = pTLVx11->mPRLServiceUpdateEnabled;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackOMADMSetProvisioningFeature

DESCRIPTION:
   This function sets the OMA-DM device provisioning service 
   update feature setting

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   bProvisioning  [ I ] - Device provisioning service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackOMADMSetProvisioningFeature( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bProvisioning )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add bProvisioning
   
   // Check size
   WORD tlvx10Sz = sizeof( sOMASetFeaturesRequest_Provisioning );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sOMASetFeaturesRequest_Provisioning * pTLVx10;
   pTLVx10 = (sOMASetFeaturesRequest_Provisioning*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mDeviceProvisioningServiceUpdateEnabled = (INT8)bProvisioning;
   
   offset += tlvx10Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackOMADMSetPRLUpdateFeature

DESCRIPTION:
   This function sets the OMA-DM PRL service update feature setting

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   bPRLUpdate     [ I ] - PRL service update enabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackOMADMSetPRLUpdateFeature( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bPRLUpdate )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add bPRLUpdate
   
   // Check size
   WORD tlvx11Sz = sizeof( sOMASetFeaturesRequest_PRLUpdate );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx11Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x11;
   pHeader->mLength = tlvx11Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sOMASetFeaturesRequest_PRLUpdate * pTLVx11;
   pTLVx11 = (sOMASetFeaturesRequest_PRLUpdate*)(pOut + offset);
   memset( pTLVx11, 0, tlvx11Sz );

   // Set the value
   pTLVx11->mPRLServiceUpdateEnabled = (INT8)bPRLUpdate;
   
   offset += tlvx11Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
