/*===========================================================================
FILE: 
   GobiQMICoreRMS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (RMS Service)

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
   GetSMSWake

DESCRIPTION:
   This function queries the state of the SMS wake functionality

PARAMETERS:
   pbEnabled   [ O ] - SMS wake functionality enabled?
   pWakeMask   [ O ] - SMS wake mask (only relevant when enabled)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSMSWake( 
   ULONG *                    pbEnabled,
   ULONG *                    pWakeMask )
{
   // Validate arguments
   if (pbEnabled == 0 || pWakeMask == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pbEnabled = ULONG_MAX;
   *pWakeMask = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_RMS_GET_SMS_WAKE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_RMS, msgID );
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

   // Try to find TLVs ID 16/17
   std::map <ULONG, const sQMIRawContentHeader *> tlvs;
   tlvs = qmiRsp.GetContents();

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = tlvs.find( 16 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pHdr = pIter->second;
      if (pHdr->mLength < (WORD)1)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      const BYTE * pData = (const BYTE *)++pHdr;
      *pbEnabled = (ULONG)*pData;
   }

   pIter = tlvs.find( 17 );
   if (pIter != tlvs.end())
   {
      const sQMIRawContentHeader * pHdr = pIter->second;
      if (pHdr->mLength < (WORD)4)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      const ULONG * pData = (const ULONG *)++pHdr;
      *pWakeMask = *pData;
   }

   return eGOBI_ERR_NONE;
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
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetSMSWake( 
   ULONG                      bEnable,
   ULONG                      wakeMask )
{
   WORD msgID = (WORD)eQMI_RMS_SET_SMS_WAKE;
   std::vector <sDB2PackingInput> piv;

   BYTE enableTmp = (BYTE)(bEnable == 0 ? 0 : 1 );
   sProtocolEntityKey pek1( eDB2_ET_QMI_RMS_REQ, msgID, 16 );
   sDB2PackingInput pi1( pek1, &enableTmp, 1 );
   piv.push_back( pi1 );

   if (bEnable != 0)
   {
      sProtocolEntityKey pek2( eDB2_ET_QMI_RMS_REQ, msgID, 17 );
      sDB2PackingInput pi2( pek2, (const BYTE *)&wakeMask, 4 );
      piv.push_back( pi2 );
   }

   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_RMS, pRequest );
}

