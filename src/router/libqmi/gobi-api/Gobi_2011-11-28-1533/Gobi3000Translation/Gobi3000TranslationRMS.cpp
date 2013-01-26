/*===========================================================================
FILE: 
   Gobi3000TranslationRMS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (Remote Management Service)

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
   ParseGetSMSWake

DESCRIPTION:
   This function queries the state of the SMS wake functionality

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pbEnabled   [ O ] - SMS wake functionality enabled?
   pWakeMask   [ O ] - SMS wake mask (only relevant when enabled)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSMSWake( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pbEnabled,
   ULONG *                    pWakeMask )
{
   // Validate arguments
   if (pIn == 0 || pbEnabled == 0 || pWakeMask == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find the first TLV
   const sRMSGetSMSWakeResponse_State * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sRMSGetSMSWakeResponse_State ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Find the second TLV
   const sRMSGetSMSWakeRequest_Mask * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx11 < sizeof( sRMSGetSMSWakeRequest_Mask ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbEnabled = pTLVx10->mSMSWakeEnabled;
   *pWakeMask = pTLVx11->mMask;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetSMSWake

DESCRIPTION:
   This function enables/disables the SMS wake functionality

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   bEnable     [ I ] - Enable SMS wake functionality?
   wakeMask    [ I ] - SMS wake mask (only relevant when enabling)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetSMSWake( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bEnable,
   ULONG                      wakeMask )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add bEnable
   
   // Check size
   WORD tlvx10Sz = sizeof( sRMSSetSMSWakeRequest_State );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sRMSSetSMSWakeRequest_State * pTLVx10;
   pTLVx10 = (sRMSSetSMSWakeRequest_State*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mSMSWakeEnabled = (INT8)bEnable;
   
   offset += tlvx10Sz;

   // Add wakeMask if enabled
   if (bEnable != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sRMSSetSMSWakeRequest_Mask );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;

      offset += sizeof( sQMIRawContentHeader );

      sRMSSetSMSWakeRequest_Mask * pTLVx11;
      pTLVx11 = (sRMSSetSMSWakeRequest_Mask*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      // Set the value
      pTLVx11->mMask = wakeMask;
   
      offset += tlvx11Sz;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
