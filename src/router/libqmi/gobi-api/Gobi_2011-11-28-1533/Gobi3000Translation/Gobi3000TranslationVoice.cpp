/*===========================================================================
FILE: 
   Gobi3000TranslationVoice.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (Voice Service for USSD)

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

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sUSSDInfo
//    Struct to represent USSD/Alpha information header
/*=========================================================================*/
struct sUSSDInfoHdr
{
   public:
      BYTE mDCS;
      BYTE mLength;

      // Data of 'mLength' follows
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

/*===========================================================================
METHOD:
   PackOriginateUSSD

DESCRIPTION:
   This function initiates a USSD operation

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   pInfo          [ I ] - USSD information

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackOriginateUSSD( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   BYTE *                     pInfo )
{
   // Validate arguments
   if (pOut == 0 || pInfo == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   const WORD INFO_HDR_SZ = sizeof( sUSSDInfoHdr );

   // This assumes that pInfo is at least 2 bytes long
   sUSSDInfoHdr * pInInfo = (sUSSDInfoHdr *)pInfo;
   WORD infoLen = pInInfo->mLength + INFO_HDR_SZ;

   // Check size
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + infoLen)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   // Add pInfo
   
   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = infoLen;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // No pTLVx01 since pInfo is our TLV
   memcpy( (pOut + offset), pInfo, infoLen );

   offset += infoLen;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackAnswerUSSD

DESCRIPTION:
   This function responds to a USSD request from the network

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   pInfo          [ I ] - USSD information

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackAnswerUSSD( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   BYTE *                     pInfo )
{
   // Validate arguments
   if (pOut == 0 || pInfo == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   const WORD INFO_HDR_SZ = sizeof( sUSSDInfoHdr );

   // This assumes that pInfo is at least 2 bytes long
   sUSSDInfoHdr * pInInfo = (sUSSDInfoHdr *)pInfo;
   WORD infoLen = pInInfo->mLength + INFO_HDR_SZ;

   // Check size
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + infoLen)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = infoLen;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // No pTLVx01 since pInfo is our TLV
   memcpy( (pOut + offset), pInfo, infoLen );

   offset += infoLen;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
