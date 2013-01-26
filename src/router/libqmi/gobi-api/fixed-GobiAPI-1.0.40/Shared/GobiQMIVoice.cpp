/*===========================================================================
FILE: 
   GobiQMICoreVoice.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (Voice Service)

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

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   OriginateUSSD (Public Method)

DESCRIPTION:
   This function initiates a USSD operation

PARAMETERS:
   pInfo          [ I ] - USSD information

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::OriginateUSSD( BYTE * pInfo )
{
   // Validate arguments
   if (pInfo == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   const ULONG INFO_HDR_SZ = (ULONG)sizeof( sUSSDInfoHdr );

   sUSSDInfoHdr * pInInfo = (sUSSDInfoHdr *)pInfo;
   ULONG infoLen = pInInfo->mLength + INFO_HDR_SZ;

   WORD msgID = (WORD)eQMI_VOICE_ASYNC_ORIG_USSD;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_VOICE_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (const BYTE *)pInfo, infoLen );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   return SendAndCheckReturn( eQMI_SVC_VOICE, pRequest, 300000 );
}

/*===========================================================================
METHOD:
   AnswerUSSD (Public Method)

DESCRIPTION:
   This function responds to a USSD request from the network

PARAMETERS:
   pInfo          [ I ] - USSD information

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::AnswerUSSD( BYTE * pInfo )
{
   // Validate arguments
   if (pInfo == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   const ULONG INFO_HDR_SZ = (ULONG)sizeof( sUSSDInfoHdr );

   sUSSDInfoHdr * pInInfo = (sUSSDInfoHdr *)pInfo;
   ULONG infoLen = pInInfo->mLength + INFO_HDR_SZ;

   WORD msgID = (WORD)eQMI_VOICE_ANSWER_USSD;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_VOICE_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (const BYTE *)pInfo, infoLen );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_VOICE, pRequest, 300000 );
}

/*===========================================================================
METHOD:
   CancelUSSD (Public Method)

DESCRIPTION:
   This function cancels an in-progress USSD operation

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::CancelUSSD()
{
   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_VOICE_CANCEL_USSD;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_VOICE, msgID, 30000 );
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

