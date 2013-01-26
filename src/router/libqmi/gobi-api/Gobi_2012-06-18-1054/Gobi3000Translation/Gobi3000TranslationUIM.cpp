/*===========================================================================
FILE: 
   Gobi3000TranslationUIM.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (DMS_UIM Service)

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
   ParseUIMUnblockControlKey

DESCRIPTION:
   This function unblocks the specified facility control key

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked 
                                (0xFFFFFFFF = unknown)
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseUIMUnblockControlKey( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMUnblockControlKeyResponse_Status * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMUnblockControlKeyResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pUnblockRetriesLeft = pTLVx10->mRemainingUnblockRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMUnblockControlKey

DESCRIPTION:
   This function unblocks the specified facility control key

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                              contain, upon output the number of BYTEs copied
                              to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - Facility ID
   pValue               [ I ] - Control key de-personalization string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMUnblockControlKey( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id, 
   CHAR *                     pValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  pValue == 0 
   ||  pValue[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string val( pValue );
   UINT8 valSz = (UINT8)val.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMUnblockControlKeyRequest_Facility ) + valSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSUIMUnblockControlKeyRequest_Facility * pTLVx01;
   pTLVx01 = (sDMSUIMUnblockControlKeyRequest_Facility*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mFacility = (eQMIDMSUIMFacility)id;
   pTLVx01->mControlKeyLength = valSz;

   offset += sizeof( sDMSUIMUnblockControlKeyRequest_Facility );
   
   memcpy( (pOut + offset), (LPCSTR)val.c_str(), valSz );
   offset += valSz;
   
   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMSetControlKeyProtection

DESCRIPTION:
   This function changes the specified facility control key

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pVerifyRetriesLeft   [ O ] - Upon operational failure this will indicate
                                the number of retries left, after which the
                                control key will be blocked 
                                (0xFFFFFFFF = unknown)
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseUIMSetControlKeyProtection( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pVerifyRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pVerifyRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMSetControlKeyProtectionResponse_Status * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMSetControlKeyProtectionResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx10->mRemainingVerifyRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMSetControlKeyProtection

DESCRIPTION:
   This function changes the specified facility control key

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - Facility ID
   status               [ I ] - Control key status
   pValue               [ I ] - Control key de-personalization string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMSetControlKeyProtection( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id, 
   ULONG                      status,
   CHAR *                     pValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  pValue == 0 
   ||  pValue[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string val( pValue );
   UINT8 valSz = (UINT8)val.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMSetControlKeyProtectionRequest_Facility ) + valSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSUIMSetControlKeyProtectionRequest_Facility * pTLVx01;
   pTLVx01 = (sDMSUIMSetControlKeyProtectionRequest_Facility*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mFacility = (eQMIDMSUIMFacility)id;
   pTLVx01->mFacilityState = (eQMIDMSUIMFacilityStates)status;
   pTLVx01->mControlKeyLength = (UINT8)valSz;

   offset += sizeof( sDMSUIMSetControlKeyProtectionRequest_Facility );
   
   memcpy( (pOut + offset), val.c_str(), valSz );
   offset += valSz;
   
   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMGetControlKeyBlockingStatus

DESCRIPTION:
   This function returns the status of the specified facility control key

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMGetControlKeyBlockingStatus( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft,
   ULONG *                    pbBlocking )
{
   // Validate arguments
   if (pIn == 0
   ||  pStatus == 0 
   ||  pVerifyRetriesLeft == 0 
   ||  pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the first arguments
   const sDMSUIMGetControlKeyStatusResponse_Status * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sDMSUIMGetControlKeyStatusResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx01->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx01->mRemainingUnblockRetries;
   *pStatus = pTLVx01->mFacilityState;

   // Find the last (optional) argument
   if (pbBlocking != 0)
   {
      const sDMSUIMGetControlKeyStatusResponse_Blocking * pTLVx10;
      ULONG tlvLenx10;
      rc = GetTLV( inLen, pIn, 0x10, &tlvLenx10, (const BYTE **)&pTLVx10 );
      if (rc != eGOBI_ERR_NONE)
      {
         return rc;
      }

      // Is the TLV large enough?
      if (tlvLenx10 < sizeof( sDMSUIMGetControlKeyStatusResponse_Blocking ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pbBlocking = pTLVx10->mOperationBlocking;
   }
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMGetControlKeyBlockingStatus

DESCRIPTION:
   This function returns the status of the specified facility control key

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - Facility ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMGetControlKeyBlockingStatus( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add id

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMGetControlKeyStatusRequest_Facility );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSUIMGetControlKeyStatusRequest_Facility * pTLVx01;
   pTLVx01 = (sDMSUIMGetControlKeyStatusRequest_Facility*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mFacility = (eQMIDMSUIMFacility)id;

   offset += tlvx01Sz;
   
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMGetControlKeyStatus

DESCRIPTION:
   This function returns the status of the specified facility control key

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
   pStatus              [ O ] - Control key status
   pVerifyRetriesLeft   [ O ] - The number of retries left, after which the 
                                control key will be blocked 
   pUnblockRetriesLeft  [ O ] - The number of unblock retries left, after 
                                which the control key  will be permanently 
                                blocked 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseUIMGetControlKeyStatus( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0
   ||  pStatus == 0 
   ||  pVerifyRetriesLeft == 0 
   ||  pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the arguments
   const sDMSUIMGetControlKeyStatusResponse_Status * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sDMSUIMGetControlKeyStatusResponse_Status ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx01->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx01->mRemainingUnblockRetries;
   *pStatus = pTLVx01->mFacilityState;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMGetControlKeyStatus

DESCRIPTION:
   This function requests the status of the specified facility control key

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - Facility ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMGetControlKeyStatus( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id )
{
   // Request is the same as PackUIMGetControlKeyBlockingStatus
   return PackUIMGetControlKeyBlockingStatus( pOutLen,
                                              pOut,
                                              id );
}

/*===========================================================================
METHOD:
   ParseUIMGetICCID

DESCRIPTION:
   This function returns the UIM ICCID

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL 
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseUIMGetICCID( 
   ULONG                      inLen,
   const BYTE *               pIn,
   BYTE                       stringSize, 
   CHAR *                     pString )
{
   // Validate arguments
   if (pIn == 0 || stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMGetICCIDResponse_ICCID * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sDMSUIMGetICCIDResponse_ICCID ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // The TLV only contains the string

   // Space to perform the copy?
   if (stringSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, (const CHAR*)pTLVx01, outLenx01 );
   pString[outLenx01] = 0;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMGetPINStatus

DESCRIPTION:
   This function returns the status of the pin

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMGetPINStatus( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG                      id,
   ULONG *                    pStatus,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 
   ||  id < 1 
   ||  id > 2
   ||  pStatus == 0 
   ||  pVerifyRetriesLeft == 0 
   ||  pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   ULONG tlvLen;

   // The typeID is either 0x11 or 0x12
   if (id == 1)
   {
      const sDMSUIMGetPINStatusResponse_PIN1Status * pTLV11;
      ULONG rc = GetTLV( inLen, pIn, 0x11, &tlvLen, (const BYTE **)&pTLV11 );

      if (rc != eGOBI_ERR_NONE)
      {
         return rc;
      }

      // Is the TLV large enough?
      if (tlvLen < sizeof( sDMSUIMGetPINStatusResponse_PIN1Status ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pVerifyRetriesLeft = pTLV11->mRemainingVerifyRetries;
      *pUnblockRetriesLeft = pTLV11->mRemainingUnblockRetries;
      *pStatus = pTLV11->mPINStatus;
   }
   else if (id == 2)
   {
      const sDMSUIMGetPINStatusResponse_PIN2Status * pTLV12;
      ULONG rc = GetTLV( inLen, pIn, 0x12, &tlvLen, (const BYTE **)&pTLV12 );

      if (rc != eGOBI_ERR_NONE)
      {
         return rc;
      }

      // Is the TLV large enough?
      if (tlvLen < sizeof( sDMSUIMGetPINStatusResponse_PIN2Status ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      *pVerifyRetriesLeft = pTLV12->mRemainingVerifyRetries;
      *pUnblockRetriesLeft = pTLV12->mRemainingUnblockRetries;
      *pStatus = pTLV12->mPINStatus;
   }
   else
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMChangePIN

DESCRIPTION:
   This function changes the PIN value

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMChangePIN( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pVerifyRetriesLeft == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMChangePINResponse_RetryInfo * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMChangePINResponse_RetryInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx10->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx10->mRemainingUnblockRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMChangePIN

DESCRIPTION:
   This function changes the PIN value

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - PIN ID (1/2)
   pOldValue            [ I ] - Old PIN value of the PIN to change
   pNewValue            [ I ] - New PIN value of the PIN to change

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMChangePIN( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id,
   CHAR *                     pOldValue,
   CHAR *                     pNewValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  id < 1
   ||  id > 2
   ||  pOldValue == 0 
   ||  pOldValue[0] == 0
   ||  pNewValue == 0 
   ||  pNewValue[0] == 0 )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string oldVal( pOldValue );
   ULONG oldValSz = (ULONG)oldVal.size();
   std::string newVal( pNewValue );
   ULONG newValSz = (ULONG)newVal.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMChangePINRequest_Info ) 
                 + (WORD)oldValSz + (WORD)newValSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // First part of the TLV
   sDMSUIMChangePINRequest_Info1 * pTLVx01_1;
   pTLVx01_1 = (sDMSUIMChangePINRequest_Info1*)(pOut + offset);
   memset( pTLVx01_1, 0, tlvx01Sz );

   pTLVx01_1->mPINID = (UINT8)id;
   pTLVx01_1->mOldPINLength = (UINT8)oldValSz;
   offset += sizeof( sDMSUIMChangePINRequest_Info1 );

   // mOldPINValue string
   memcpy( (pOut + offset), oldVal.c_str(), oldValSz );
   offset += oldValSz;

   // Second part of the TLV
   sDMSUIMChangePINRequest_Info2 * pTLVx01_2;
   pTLVx01_2 = (sDMSUIMChangePINRequest_Info2*)(pOut + offset);
   
   pTLVx01_2->mNewPINLength = (UINT8)newValSz;
   offset += sizeof( sDMSUIMChangePINRequest_Info2 );

   // mNewPINValue string
   memcpy( (pOut + offset), newVal.c_str(), newValSz );
   offset += newValSz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMUnblockPIN

DESCRIPTION:
   This function unblocks a blocked PIN

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMUnblockPIN( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pVerifyRetriesLeft == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMUnblockPINResponse_RetryInfo * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMUnblockPINResponse_RetryInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx10->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx10->mRemainingUnblockRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMUnblockPIN

DESCRIPTION:
   This function unblocks a blocked PIN

PARAMETERS:
   id                   [ I ] - PIN ID (1/2)
   pPUKValue            [ I ] - PUK value of the PIN to unblock
   pNewValue            [ I ] - New PIN value of the PIN to unblock

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMUnblockPIN( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id,
   CHAR *                     pPUKValue,
   CHAR *                     pNewValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  id < 1
   ||  id > 2
   ||  pPUKValue == 0 
   ||  pPUKValue[0] == 0
   ||  pNewValue == 0 
   ||  pNewValue[0] == 0 )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string oldVal( pPUKValue );
   ULONG oldValSz = (ULONG)oldVal.size();
   std::string newVal( pNewValue );
   ULONG newValSz = (ULONG)newVal.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMUnblockPINRequest_Info ) 
                 + (WORD)oldValSz + (WORD)newValSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // First part of the TLV
   sDMSUIMUnblockPINRequest_Info1 * pTLVx01_1;
   pTLVx01_1 = (sDMSUIMUnblockPINRequest_Info1*)(pOut + offset);
   memset( pTLVx01_1, 0, tlvx01Sz );

   pTLVx01_1->mPINID = (UINT8)id;
   pTLVx01_1->mPUKLength = (UINT8)oldValSz;
   offset += sizeof( sDMSUIMUnblockPINRequest_Info1 );

   // mPUKValue string
   memcpy( (pOut + offset), oldVal.c_str(), oldValSz );
   offset += oldValSz;

   // Second part of the TLV
   sDMSUIMUnblockPINRequest_Info2 * pTLVx01_2;
   pTLVx01_2 = (sDMSUIMUnblockPINRequest_Info2*)(pOut + offset);
   
   pTLVx01_2->mNewPINLength = (UINT8)newValSz;
   offset += sizeof( sDMSUIMUnblockPINRequest_Info2 );

   // mNewPINValue string
   memcpy( (pOut + offset), newVal.c_str(), newValSz );
   offset += newValSz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMVerifyPIN

DESCRIPTION:
   This function verifies the PIN before accessing the UIM contents

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMVerifyPIN( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pVerifyRetriesLeft == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMVerifyPINResponse_RetryInfo * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMVerifyPINResponse_RetryInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx10->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx10->mRemainingUnblockRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMVerifyPIN

DESCRIPTION:
   This function verifies the PIN before accessing the UIM contents

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - PIN ID (1/2)
   pValue               [ I ] - PIN value of the PIN to verify

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMVerifyPIN( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id,
   CHAR *                     pValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  id < 1
   ||  id > 2
   ||  pValue == 0 
   ||  pValue[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string val( pValue );
   UINT8 valSz = (UINT8)val.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMVerifyPINRequest_Info ) + valSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSUIMVerifyPINRequest_Info * pTLVx01;
   pTLVx01 = (sDMSUIMVerifyPINRequest_Info*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mPINID = (UINT8)id;
   pTLVx01->mPINLength = valSz;
   offset += sizeof( sDMSUIMVerifyPINRequest_Info );

   // Add mPINValue
   memcpy( (pOut + offset), val.c_str(), valSz );
   offset += valSz;
   
   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseUIMSetPINProtection

DESCRIPTION:
   This function enables or disables protection of UIM contents by a 
   given PIN

PARAMETERS:
   inLen                [ I ] - Length of input buffer
   pIn                  [ I ] - Input buffer
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
ULONG ParseUIMSetPINProtection( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pVerifyRetriesLeft,
   ULONG *                    pUnblockRetriesLeft )
{
   // Validate arguments
   if (pIn == 0 || pVerifyRetriesLeft == 0 || pUnblockRetriesLeft == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the TLV
   const sDMSUIMSetPINProtectionResponse_RetryInfo * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sDMSUIMSetPINProtectionResponse_RetryInfo ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pVerifyRetriesLeft = pTLVx10->mRemainingVerifyRetries;
   *pUnblockRetriesLeft = pTLVx10->mRemainingUnblockRetries;
   
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackUIMSetPINProtection

DESCRIPTION:
   This function enables or disables protection of UIM contents by a 
   given PIN

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   id                   [ I ] - PIN ID (1/2)
   bEnable              [ I ] - Enable/disable PIN protection (0 = disable)?
   pValue               [ I ] - PIN value of the PIN to be enabled/disabled

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackUIMSetPINProtection( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      id,
   ULONG                      bEnable,
   CHAR *                     pValue )
{
   // Validate arguments
   if (pOut == 0 
   ||  id < 1
   ||  id > 2
   ||  pValue == 0 
   ||  pValue[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   std::string val( pValue );
   ULONG valSz = (ULONG)val.size();

   // Check size
   WORD tlvx01Sz = sizeof( sDMSUIMSetPINProtectionRequest_Info ) + (WORD)valSz;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSUIMSetPINProtectionRequest_Info * pTLVx01;
   pTLVx01 = (sDMSUIMSetPINProtectionRequest_Info*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mPINID = (UINT8)id;
   pTLVx01->mPINEnabled = (bEnable == 0 ? 0 : 1);
   pTLVx01->mPINLength = (UINT8)valSz;

   offset += sizeof( sDMSUIMSetPINProtectionRequest_Info );
   
   // Add mPINValue
   memcpy( (pOut + offset), val.c_str(), valSz );
   offset += valSz;
   
   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}
