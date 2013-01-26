/*===========================================================================
FILE: 
   GobiQMICoreNAS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (NAS Service)

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

// Maximum length for a scanned network description
const ULONG MAX_SNI_DESCRIPTION_LEN = 255;

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sEVDOCustomSCPConfig
//    Struct to represent CDMA 1xEV-DO custom SCP config
/*=========================================================================*/
struct sEVDOCustomSCPConfig
{
   public:
      BYTE mbActive;
      ULONG mProtocolMask;
      ULONG mBroadcastMask;
      ULONG mApplicationMask;
};

/*=========================================================================*/
// Struct sScannedNetworkInfo
//    Struct to represent scanned network information
/*=========================================================================*/  
struct sScannedNetworkInfo
{
   public:
      USHORT mMCC;
      USHORT mMNC;
      ULONG mInUse;
      ULONG mRoaming;
      ULONG mForbidden;
      ULONG mPreferred;
      CHAR mDescription[MAX_SNI_DESCRIPTION_LEN];
};

/*=========================================================================*/
// Struct sScannedNetworkRATInfo
//    Struct to represent scanned network RAT information
/*=========================================================================*/  
struct sScannedNetworkRATInfo
{
   public:
      USHORT mMCC;
      USHORT mMNC;
      ULONG mRAT;
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
   GetANAAAAuthenticationStatus (Public Method)

DESCRIPTION:
   This function gets the AN-AAA authentication status

PARAMETERS:
   pStatus     [ O ] - AN-AAA authentication status

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetANAAAAuthenticationStatus( ULONG * pStatus )
{
   // Validate arguments
   if (pStatus == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_AAA_AUTH_STATUS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the index
   *pStatus = (ULONG)pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetSignalStrengths (Public Method)

DESCRIPTION:
   This function gets the current available signal strengths (in dBm) 
   as measured by the device

PARAMETERS:
   pArraySizes       [I/O] - Upon input the maximum number of elements 
                             that each array can contain can contain.  
                             Upon successful output the actual number 
                             of elements in each array
   pSignalStrengths  [ O ] - Received signal strength array (dBm)
   pRadioInterfaces  [ O ] - Radio interface technology array 

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetSignalStrengths( 
   ULONG *                    pArraySizes, 
   INT8 *                     pSignalStrengths, 
   ULONG *                    pRadioInterfaces )
{
   // Validate arguments
   if ( (pArraySizes == 0)
   ||   (*pArraySizes == 0)
   ||   (pSignalStrengths == 0)
   ||   (pRadioInterfaces == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSignals = (ULONG)*pArraySizes;

   // Assume failure
   *pArraySizes = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_RSSI;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 2) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Remove any values outside the legal range
   std::map <ULONG, INT8> sigMap;
   
   INT8 sigVal = pf[0].mValue.mS8;
   ULONG radioVal = pf[1].mValue.mU32;
   if (sigVal <= -30 && sigVal > -125 && radioVal != 0)
   {
      sigMap[radioVal] = sigVal;
   }

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() > 2) 
   {
      ULONG fi = 0;
      ULONG auxSigs = (ULONG)pf[fi++].mValue.mU16;
      if (pf.size() >= 1 + 2 * auxSigs)
      {
         for (ULONG s = 0; s < auxSigs; s++, fi += 2)
         {
            sigVal = pf[fi].mValue.mS8;
            radioVal = pf[fi + 1].mValue.mU32;
            if (sigVal <= -30 && sigVal > -125 && radioVal != 0)
            {
               sigMap[radioVal] = sigVal;
            }
         }
      }
   }

   ULONG sigCount = 0;
   std::map <ULONG, INT8>::const_iterator pIter;
   for (pIter = sigMap.begin(); pIter != sigMap.end(); pIter++, sigCount++)
   {
      if (sigCount < maxSignals)
      {
         pSignalStrengths[sigCount] = pIter->second;
         pRadioInterfaces[sigCount] = pIter->first;
         *pArraySizes = sigCount + 1;
      }
   }

   // No valid signals?
   if (sigCount == 0)
   {
      return eGOBI_ERR_NO_SIGNAL;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetRFInfo (Public Method)

DESCRIPTION:
   This function gets the current RF information

PARAMETERS:
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the 
                          RF info instance array can contain.  Upon success
                          the actual number of elements in the RF info 
                          instance array
   pInstances     [ O ] - The RF info instance array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetRFInfo( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances )
{
   // Validate arguments
   if (pInstanceSize == 0 || *pInstanceSize == 0 || pInstances == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   BYTE maxInstances = *pInstanceSize;
   *pInstanceSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_RF_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );

   ULONG fieldCount = (ULONG)pf.size();
   if (fieldCount < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   BYTE ifaceCount = pf[0].mValue.mU8;
   if (fieldCount < 1 + ((ULONG)ifaceCount * 3))
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (ifaceCount > maxInstances)
   {
      ifaceCount = maxInstances;
   }

   ULONG * pOutput = (ULONG *)pInstances;
   for (BYTE i = 0; i < ifaceCount; i++)
   {
      ULONG offset = 3 * (ULONG)i;

      *pOutput++ = pf[offset + 1].mValue.mU32;
      *pOutput++ = pf[offset + 2].mValue.mU32;
      *pOutput++ = (ULONG)pf[offset + 3].mValue.mU16;
   }

   *pInstanceSize = ifaceCount;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PerformNetworkScan (Public Method)

DESCRIPTION:
   This function performs a scan for available networks

PARAMETERS:
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the 
                          network info instance array can contain.  Upon 
                          success the actual number of elements in the
                          network info instance array
   pInstances     [ O ] - The network info instance array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::PerformNetworkScan( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances )
{
   // Validate arguments
   if ( (pInstanceSize == 0)
   ||   (*pInstanceSize == 0)
   ||   (pInstances == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxInstances = *pInstanceSize;

   // Assume failure
   *pInstanceSize = 0;

   // This can take a really long time
   ULONG to = MAX_REQ_TIMEOUT;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_SCAN_NETS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID, to );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );

   ULONG maxIdx = (ULONG)pf.size();
   if (maxIdx-- < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG idx = 0;
   BYTE netCount = pf[idx++].mValue.mU8;
   if (netCount > maxInstances)
   {
      netCount = maxInstances;
   }

   sScannedNetworkInfo * pNet = (sScannedNetworkInfo *)pInstances;
   for (BYTE i = 0; i < netCount; i++)
   {
      // Validate field count
      if (idx + 6 > maxIdx)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      pNet->mMCC       = pf[idx++].mValue.mU16;
      pNet->mMNC       = pf[idx++].mValue.mU16;
      pNet->mInUse     = pf[idx++].mValue.mU32;
      pNet->mRoaming   = pf[idx++].mValue.mU32;
      pNet->mForbidden = pf[idx++].mValue.mU32;
      pNet->mPreferred = pf[idx++].mValue.mU32;

      memset( &pNet->mDescription[0], 0, (SIZE_T)MAX_SNI_DESCRIPTION_LEN );

      BYTE descLen = pf[idx++].mValue.mU8;
      if (descLen > 0)
      {
         std::string netDesc( pf[idx++].mValueString );

         ULONG actualLen = netDesc.size();
         if (actualLen >= MAX_SNI_DESCRIPTION_LEN)
         {
            actualLen = MAX_SNI_DESCRIPTION_LEN - 1;
         }

         memcpy( (LPVOID)&pNet->mDescription[0], 
                 (LPCSTR)netDesc.c_str(), 
                 (SIZE_T)actualLen );
      }

      pNet++;
   }

   *pInstanceSize = netCount;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PerformNetworkRATScan (Public Method)

DESCRIPTION:
   This function performs a scan for available networks (includes RAT)

PARAMETERS:
   pInstanceSize  [I/O] - Upon input the maximum number of elements that the 
                          network info instance array can contain.  Upon 
                          success the actual number of elements in the
                          network info instance array
   pInstances     [ O ] - The network info instance array 
   pRATSize       [I/O] - Upon input the maximum number of elements that the 
                          RAT info instance array can contain.  Upon success 
                          the actual number of elements in the RAT info 
                          instance array
   pRATInstances  [ O ] - The RAT info instance array

  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::PerformNetworkRATScan( 
   BYTE *                     pInstanceSize, 
   BYTE *                     pInstances,
   BYTE *                     pRATSize, 
   BYTE *                     pRATInstances )
{
   // Validate arguments
   if ( (pInstanceSize == 0)
   ||   (*pInstanceSize == 0)
   ||   (pInstances == 0)
   ||   (pRATSize == 0)
   ||   (*pRATSize == 0)
   ||   (pRATInstances == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxInstances = *pInstanceSize;
   BYTE maxRATInstances = *pRATSize;

   // Assume failure
   *pInstanceSize = 0;
   *pRATSize = 0;

   // This can take a really long time
   ULONG to = MAX_REQ_TIMEOUT;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_SCAN_NETS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID, to );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );

   ULONG maxIdx = (ULONG)pf.size();
   if (maxIdx-- < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG idx = 0;
   BYTE netCount = pf[idx++].mValue.mU8;
   if (netCount > maxInstances)
   {
      netCount = maxInstances;
   }

   sScannedNetworkInfo * pNet = (sScannedNetworkInfo *)pInstances;
   for (BYTE i = 0; i < netCount; i++)
   {
      // Validate field count
      if (idx + 6 > maxIdx)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      pNet->mMCC       = pf[idx++].mValue.mU16;
      pNet->mMNC       = pf[idx++].mValue.mU16;
      pNet->mInUse     = pf[idx++].mValue.mU32;
      pNet->mRoaming   = pf[idx++].mValue.mU32;
      pNet->mForbidden = pf[idx++].mValue.mU32;
      pNet->mPreferred = pf[idx++].mValue.mU32;

      memset( &pNet->mDescription[0], 0, (SIZE_T)MAX_SNI_DESCRIPTION_LEN );

      BYTE descLen = pf[idx++].mValue.mU8;
      if (descLen > 0)
      {
         std::string netDesc( pf[idx++].mValueString );

         ULONG actualLen = netDesc.size();
         if (actualLen >= MAX_SNI_DESCRIPTION_LEN)
         {
            actualLen = MAX_SNI_DESCRIPTION_LEN - 1;
         }

         LPCSTR pNetDesc = netDesc.c_str();
         memcpy( (LPVOID)&pNet->mDescription[0], 
                 (LPCVOID)pNetDesc, 
                 (SIZE_T)actualLen );
      }

      pNet++;
   }

   // Parse the TLV we want (by DB key)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );

   maxIdx = (ULONG)pf.size();
   if (maxIdx-- < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   idx = 0;
   BYTE ratCount = pf[idx++].mValue.mU8;
   if (ratCount > maxRATInstances)
   {
      ratCount = maxRATInstances;
   }

   sScannedNetworkRATInfo * pRAT = (sScannedNetworkRATInfo *)pRATInstances;
   for (BYTE r = 0; r < ratCount; r++)
   {
      // Validate field count
      if (idx + 2 > maxIdx)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      pRAT->mMCC = pf[idx++].mValue.mU16;
      pRAT->mMNC = pf[idx++].mValue.mU16;
      pRAT->mRAT = pf[idx++].mValue.mU32;
      pRAT++;
   }

   *pInstanceSize = netCount;
   *pRATSize = ratCount;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   InitiateNetworkRegistration (Public Method)

DESCRIPTION:
   This function initiates a network registration

PARAMETERS:
   regType     [ I ] - Registration type 
   mcc         [ I ] - Mobile country code (ignored for auto registration)
   mnc         [ I ] - Mobile network code (ignored for auto registration)
   rat         [ I ] - Radio access type (ignored for auto registration)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::InitiateNetworkRegistration( 
   ULONG                      regType,
   WORD                       mcc, 
   WORD                       mnc, 
   ULONG                      rat )
{
   WORD msgID = (WORD)eQMI_NAS_REGISTER_NET;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)regType;
      
   sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() ); 
   piv.push_back( pi );

   if (regType == 2)
   {
      // We need to add the manual registration data
      // "%hu %hu %u"
      std::ostringstream tmp2;
      tmp2 << (USHORT)mcc << " " << (USHORT)mnc << " " 
           << (UINT)rat;
      
      pek = sProtocolEntityKey( eDB2_ET_QMI_NAS_REQ, msgID, 16 );
      pi = sDB2PackingInput( pek, (LPCSTR)tmp2.str().c_str() ); 
      piv.push_back( pi );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_NAS, pRequest, 30000 );
}

/*===========================================================================
METHOD:
   InitiateDomainAttach (Public Method)

DESCRIPTION:
   This function initiates a domain attach (or detach)

PARAMETERS:
   action      [ I ] - PS attach action (attach or detach)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::InitiateDomainAttach( ULONG action )
{
   WORD msgID = (WORD)eQMI_NAS_ATTACH_DETACH;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)action;

   sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() ); 
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_NAS, pRequest, 30000 );
}

/*===========================================================================
METHOD:
   GetServingNetwork (Public Method)

DESCRIPTION:
   Gets information regarding the system that currently provides service 
   to the device

PARAMETERS:
   pRegistrationState   [ O ] - Registration state
   pCSDomain            [ O ] - Circuit switch domain status
   pPSDomain            [ O ] - Packet switch domain status 
   pRAN                 [ O ] - Radio access network 
   pRadioIfacesSize     [I/O] - Upon input the maximum number of elements 
                                that the radio interfaces can contain.  Upon 
                                successful output the actual number of elements 
                                in the radio interface array
   pRadioIfaces         [ O ] - The radio interface array 
   pRoaming             [ O ] - Roaming indicator (0xFFFFFFFF - Unknown)
   pMCC                 [ O ] - Mobile country code (0xFFFF - Unknown)
   pMNC                 [ O ] - Mobile network code (0xFFFF - Unknown)
   nameSize             [ I ] - The maximum number of characters (including 
                                NULL terminator) that the network name array 
                                can contain
   pName                [ O ] - The network name or description represented 
                                as a NULL terminated string (empty string 
                                returned when unknown)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetServingNetwork( 
   ULONG *                    pRegistrationState, 
   ULONG *                    pCSDomain, 
   ULONG *                    pPSDomain, 
   ULONG *                    pRAN, 
   BYTE *                     pRadioIfacesSize, 
   BYTE *                     pRadioIfaces, 
   ULONG *                    pRoaming, 
   WORD *                     pMCC, 
   WORD *                     pMNC, 
   BYTE                       nameSize, 
   CHAR *                     pName )
{
   // Validate arguments
   if ( (pRegistrationState == 0)
   ||   (pCSDomain == 0)
   ||   (pPSDomain == 0)
   ||   (pRAN == 0)
   ||   (pRadioIfacesSize == 0)
   ||   (*pRadioIfacesSize == 0)
   ||   (pRadioIfaces == 0)
   ||   (pRoaming == 0)
   ||   (pMCC == 0)
   ||   (pMNC == 0)
   ||   (nameSize == 0)
   ||   (pName == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxRadioIfaces = *pRadioIfacesSize;

   // Assume failure
   *pRadioIfacesSize = 0;
   *pRoaming = ULONG_MAX;
   *pMCC = USHRT_MAX;
   *pMNC = USHRT_MAX;
   *pName = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_SS_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() < 5) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the variables
   *pRegistrationState = pf1[0].mValue.mU32;
   *pCSDomain = pf1[1].mValue.mU32;
   *pPSDomain = pf1[2].mValue.mU32;
   *pRAN = pf1[3].mValue.mU32;

   BYTE activeRadioIfaces = pf1[4].mValue.mU8;
   if (pf1.size() < 5 + (ULONG)activeRadioIfaces)
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
      *pOutRadioIfaces++ = pf1[5 + r].mValue.mU32;
   }

   *pRadioIfacesSize = activeRadioIfaces;

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() >= 1) 
   {
      *pRoaming = pf2[0].mValue.mU32;
   }

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey3( eDB2_ET_QMI_NAS_RSP, msgID, 18 );
   cDataParser::tParsedFields pf3 = ParseTLV( db, rsp, tlvs, tlvKey3 );
   if (pf3.size() >= 3) 
   {
      *pMCC = pf3[0].mValue.mU16;
      *pMNC = pf3[1].mValue.mU16;

      // Network name?
      if (pf3[2].mValue.mU8 > 0 && pf3.size() >= 4)
      {
         LONG strLen = pf3[3].mValueString.size();
         if (strLen > 0)
         {
            // Space to perform the copy?
            if (nameSize < strLen + 1)
            {
               return eGOBI_ERR_BUFFER_SZ;
            }

            memcpy( (LPVOID)pName, (LPCSTR)pf3[3].mValueString.c_str(), strLen );
            pName[strLen] = 0;
         }
      }
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetServingNetworkCapabilities (Public Method)

DESCRIPTION:
   Gets information regarding the data capabilities of the system that 
   currently provides service to the device

PARAMETERS:
   pDataCapsSize  [I/O] - Upon input the maximum number of elements that the 
                          data capabilities array can contain.  Upon success
                          the actual number of elements in the data 
                          capabilities array
   pDataCaps      [ O ] - The data capabilities array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetServingNetworkCapabilities( 
   BYTE *                     pDataCapsSize, 
   BYTE *                     pDataCaps )
{
   // Validate arguments
   if ( (pDataCapsSize == 0)
   ||   (*pDataCapsSize == 0)
   ||   (pDataCaps == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE maxDataCaps = *pDataCapsSize;

   // Assume failure
   *pDataCapsSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_SS_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   BYTE activeDataCaps = pf[0].mValue.mU8;
   if (pf.size() < 1 + (ULONG)activeDataCaps)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   if (activeDataCaps > maxDataCaps)
   {
      activeDataCaps = maxDataCaps;
   }

   ULONG * pOutDataCaps = (ULONG *)pDataCaps;
   for (ULONG d = 0; d < activeDataCaps; d++)
   {
      *pOutDataCaps++ = pf[1 + d].mValue.mU32;
   }

   *pDataCapsSize = activeDataCaps;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetDataBearerTechnology (Public Method)

DESCRIPTION:
   This function retrieves the current data bearer technology (only
   valid when connected)

PARAMETERS:
   pDataCaps      [ O ] - The data bearer technology
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetDataBearerTechnology( ULONG * pDataBearer )
{
   // Validate arguments
   if (pDataBearer == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_WDS_GET_DATA_BEARER;
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
   *pDataBearer = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetHomeNetwork (Public Method)

DESCRIPTION:
   This function retrieves information about the home network of the device

PARAMETERS:
   pMCC                 [ O ] - Mobile country code
   pMNC                 [ O ] - Mobile network code
   nameSize             [ I ] - The maximum number of characters (including 
                                NULL terminator) that the network name array 
                                can contain
   pName                [ O ] - The network name or description represented 
                                as a NULL terminated string (empty string 
                                returned when unknown)
   pSID                 [ O ] - Home network system ID (0xFFFF - Unknown)
   pNID                 [ O ] - Home network ID (0xFFFF - Unknown)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetHomeNetwork( 
   WORD *                     pMCC, 
   WORD *                     pMNC, 
   BYTE                       nameSize, 
   CHAR *                     pName, 
   WORD *                     pSID, 
   WORD *                     pNID )
{
   // Validate arguments
   if ( (pMCC == 0)
   ||   (pMNC == 0)
   ||   (nameSize == 0)
   ||   (pName == 0)
   ||   (pSID == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pName = 0;
   *pSID = USHRT_MAX;
   *pNID = USHRT_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_HOME_INFO;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf1 = ParseTLV( db, rsp, tlvs, tlvKey1 );
   if (pf1.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the variables
   *pMCC = pf1[0].mValue.mU16;
   *pMNC = pf1[1].mValue.mU16;

   // Network name?
   if (pf1[2].mValue.mU8 > 0 && pf1.size() >= 4)
   {
      LONG strLen = pf1[3].mValueString.size();
      if (strLen > 0)
      {
         // Space to perform the copy?
         if (nameSize < strLen + 1)
         {
            return eGOBI_ERR_BUFFER_SZ;
         }

         memcpy( (LPVOID)pName, (LPCSTR)pf1[3].mValueString.c_str(), strLen );
         pName[strLen] = 0;
      }
   }

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf2 = ParseTLV( db, rsp, tlvs, tlvKey2 );
   if (pf2.size() >= 2) 
   {
      *pSID = pf2[0].mValue.mU16;
      *pNID = pf2[1].mValue.mU16;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetNetworkPreference (Public Method)

DESCRIPTION:
   This function sets the network registration preference

PARAMETERS:
   technologyPref [ I ] - Technology preference bitmap
   duration       [ I ] - Duration of active preference
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetNetworkPreference( 
   ULONG                      technologyPref, 
   ULONG                      duration )
{
   // Buffer to hold technology preference TLV (ID = 1)
   const ULONG TLV_HDR_SZ = (ULONG)sizeof( sQMIRawContentHeader );
   BYTE req[3 + TLV_HDR_SZ];

   // Fill out TLV header
   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)&req[0];
   pTLV->mLength = 3;
   pTLV->mTypeID = 1;

   // Copy packed technology preference WORD as-is
   WORD * pTmp = (WORD *)&req[TLV_HDR_SZ];
   *pTmp = (WORD)technologyPref;

   // Fill out duration
   req[TLV_HDR_SZ + 2] = (BYTE)duration;

   // Pack TLV into a QMI NAS request
   sSharedBuffer * pRequest = 0;
   pRequest = sQMIServiceBuffer::BuildBuffer( eQMI_SVC_NAS, 
                                              eQMI_NAS_SET_TECH_PREF,
                                              false,
                                              false,
                                              &req[0], 
                                              3 + TLV_HDR_SZ );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_NAS, pRequest );
}

/*===========================================================================
METHOD:
   GetNetworkPreference (Public Method)

DESCRIPTION:
   This function returns the network registration preference

PARAMETERS:
   pTechnologyPref            [ O ] - Technology preference bitmap
   pDuration                  [ O ] - Duration of active preference
   pPersistentTechnologyPref  [ O ] - Persistent technology preference bitmap 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetNetworkPreference( 
   ULONG *                    pTechnologyPref, 
   ULONG *                    pDuration, 
   ULONG *                    pPersistentTechnologyPref )
{
   // Validate arguments
   if ( (pTechnologyPref == 0)
   ||   (pDuration == 0)
   ||   (pPersistentTechnologyPref == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_TECH_PREF;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey1( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   sDB2NavInput ni1 = FindTLV( tlvs, tlvKey1 );
   if (ni1.mPayloadLen < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   const BYTE * pData = ni1.mpPayload;
   const WORD * pTmp = (const WORD *)pData;
   pData += 2;

   // Populate the variables
   *pTechnologyPref = (ULONG)*pTmp;
   *pDuration = (ULONG)*pData;

   // Until we know any better the persistent setting is the current setting
   *pPersistentTechnologyPref = *pTechnologyPref;

   // Parse the optional TLV we want (by DB key)
   sProtocolEntityKey tlvKey2( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   sDB2NavInput ni2 = FindTLV( tlvs, tlvKey2 );
   if (ni2.mPayloadLen >= 2) 
   {
      pTmp = (const WORD *)ni2.mpPayload;
      *pPersistentTechnologyPref = (ULONG)*pTmp;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetCDMANetworkParameters (Public Method)

DESCRIPTION:
   This function sets the desired CDMA network parameters

PARAMETERS:
   pSPC           [ I ] - Six digit service programming code
   pForceRev0     [ I ] - (Optional) Force CDMA 1x-EV-DO Rev. 0 mode?
   pCustomSCP     [ I ] - (Optional) Use a custom config for CDMA 1x-EV-DO SCP?
   pProtocol      [ I ] - (Optional) Protocol mask for custom SCP config
   pBroadcast     [ I ] - (Optional) Broadcast mask for custom SCP config
   pApplication   [ I ] - (Optional) Application mask for custom SCP config
   pRoaming       [ I ] - (Optional) Roaming preference
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetCDMANetworkParameters( 
   CHAR *                     pSPC,
   BYTE *                     pForceRev0,
   BYTE *                     pCustomSCP,
   ULONG *                    pProtocol,
   ULONG *                    pBroadcast,
   ULONG *                    pApplication,
   ULONG *                    pRoaming )
{
   // If you specify one of the custom SCP config fields then you must
   // specify them all
   ULONG scpCount = 0;
   if (pCustomSCP != 0)
   {
      scpCount++;
   }

   if (pProtocol != 0)
   {
      scpCount++;
   }

   if (pBroadcast != 0)
   {
      scpCount++;
   }

   if (pApplication != 0)
   {
      scpCount++;
   }

   if (scpCount != 0 && scpCount != 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Rev. 0 and SCP custom config are mutually exclusive
   if (pForceRev0 != 0 && scpCount == 4)
   {
      if (*pForceRev0 != 0 && *pCustomSCP != 0)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }

   WORD msgID = (WORD)eQMI_NAS_SET_NET_PARAMS;
   std::vector <sDB2PackingInput> piv;

   // Need to start with SPC?
   if (pForceRev0 != 0 || scpCount == 4)
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

      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 16 );
      sDB2PackingInput pi( pek, (LPCSTR)spc.c_str() );
      piv.push_back( pi );
   }

   if (pForceRev0 != 0)
   {
      // "%u"
      std::ostringstream tmp;
      tmp << (UINT)(*pForceRev0 == 0 ? 0 : 1);

      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 20 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (scpCount == 4)
   {
      // "%u %u %u %u %u %u %u %u %u %u %u %u"
      std::ostringstream tmp;
      tmp << (UINT)(*pCustomSCP == 0 ? 0 : 1)
          << (UINT)(*pProtocol & 0x00000001 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000002 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000004 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000008 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000010 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000020 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000040 ? 1 : 0)
          << (UINT)(*pProtocol & 0x00000080 ? 1 : 0)
          << (UINT)(*pBroadcast & 0x00000001 ? 1 : 0)
          << (UINT)(*pApplication & 0x00000001 ? 1 : 0)
          << (UINT)(*pApplication & 0x00000002 ? 1 : 0);
      
      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 21 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pRoaming != 0)
   {
      // "%u"
      std::ostringstream tmp;
      tmp << (UINT)*pRoaming;

      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 22 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // We require something to actually configure
   if (piv.size() == 0)
   {
      // Much ado about nothing
      return eGOBI_ERR_INVALID_ARG;
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_NAS, pRequest, 5000 );
}

/*===========================================================================
METHOD:
   GetCDMANetworkParameters (Public Method)

DESCRIPTION:
   This function gets the current CDMA network parameters

PARAMETERS:
   pSCI           [ O ] - Slot cycle index
   pSCM           [ O ] - Station class mark
   pRegHomeSID    [ O ] - Register on home SID?
   pRegForeignSID [ O ] - Register on foreign SID?
   pRegForeignNID [ O ] - Register on foreign NID?
   pForceRev0     [ O ] - Force CDMA 1x-EV-DO Rev. 0 mode?
   pCustomSCP     [ O ] - Use a custom config for CDMA 1x-EV-DO SCP?
   pProtocol      [ O ] - Protocol mask for custom SCP config
   pBroadcast     [ O ] - Broadcast mask for custom SCP config
   pApplication   [ O ] - Application mask for custom SCP config
   pRoaming       [ O ] - Roaming preference
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetCDMANetworkParameters( 
   BYTE *                     pSCI,
   BYTE *                     pSCM,
   BYTE *                     pRegHomeSID,
   BYTE *                     pRegForeignSID,
   BYTE *                     pRegForeignNID,
   BYTE *                     pForceRev0,
   BYTE *                     pCustomSCP,
   ULONG *                    pProtocol,
   ULONG *                    pBroadcast,
   ULONG *                    pApplication,
   ULONG *                    pRoaming )
{
   // Validate arguments
   if ( (pSCI == 0)
   ||   (pSCM == 0)
   ||   (pRegHomeSID == 0)
   ||   (pRegForeignSID == 0)
   ||   (pRegForeignNID == 0)
   ||   (pForceRev0 == 0)
   ||   (pCustomSCP == 0)
   ||   (pProtocol == 0)
   ||   (pBroadcast == 0)
   ||   (pApplication == 0)
   ||   (pRoaming == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pSCI = UCHAR_MAX;
   *pSCM = UCHAR_MAX;
   *pRegHomeSID = UCHAR_MAX;
   *pRegForeignSID = UCHAR_MAX;
   *pRegForeignNID = UCHAR_MAX;
   *pForceRev0 = UCHAR_MAX;
   *pCustomSCP = UCHAR_MAX;
   *pProtocol = ULONG_MAX;
   *pBroadcast = ULONG_MAX;
   *pApplication = ULONG_MAX;
   *pRoaming = UCHAR_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_NET_PARAMS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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

   // Parse the TLVs we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pSCI = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 18 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pSCM = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 19 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 3) 
   {
      *pRegHomeSID = pf[0].mValue.mU8;
      *pRegForeignSID = pf[0].mValue.mU8;
      *pRegForeignNID = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 20 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pForceRev0 = pf[0].mValue.mU8;
   }

   tlvKey = sProtocolEntityKey ( eDB2_ET_QMI_NAS_RSP, msgID, 21 );
   sDB2NavInput ni = FindTLV( tlvs, tlvKey );
   if (ni.mPayloadLen >= (ULONG)sizeof( sEVDOCustomSCPConfig )) 
   {
      const sEVDOCustomSCPConfig * pData = 0;
      pData = (const sEVDOCustomSCPConfig *)ni.mpPayload;

      *pCustomSCP   = pData->mbActive;
      *pProtocol    = pData->mProtocolMask;
      *pBroadcast   = pData->mBroadcastMask;
      *pApplication = pData->mApplicationMask;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_NAS_RSP, msgID, 22 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      *pRoaming = pf[0].mValue.mU32;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetACCOLC (Public Method)

DESCRIPTION:
   This function returns the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   pACCOLC     [ O ] - The ACCOLC
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetACCOLC( BYTE * pACCOLC )
{
   // Validate arguments
   if (pACCOLC == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_ACCOLC;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the ACCOLC
   *pACCOLC = pf[0].mValue.mU8;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetACCOLC (Public Method)

DESCRIPTION:
   This function sets the Access Overload Class (ACCOLC) of the device

PARAMETERS:
   pSPC        [ I ] - NULL terminated string representing the six digit 
                       service programming code
   accolc      [ I ] - The ACCOLC
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetACCOLC( 
   CHAR *                     pSPC,
   BYTE                       accolc )
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

   WORD msgID = (WORD)eQMI_NAS_SET_ACCOLC;
   std::vector <sDB2PackingInput> piv;

   // "%s %u"
   std::ostringstream tmp;
   tmp << spc << " " << (UINT)accolc;

   sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_NAS, pRequest, 5000 );
}

/*===========================================================================
METHOD:
   GetPLMNMode (Public Method)

DESCRIPTION:
   This function returns the PLMN mode from the CSP

PARAMETERS:
   pMode       [ O ] - PLMN mode
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPLMNMode( ULONG * pMode )
{
   // Validate arguments
   if (pMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_NAS_GET_PLMN_MODE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_NAS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the PLMN mode
   *pMode = (ULONG)pf[0].mValue.mU8;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetPLMNName (Public Method)

DESCRIPTION:
   This function returns PLMN name information for the given MCC/MNC

PARAMETERS:
   mcc         [ I ] - Mobile country code
   mnc         [ I ] - Mobile network code
   pNamesSize  [I/O] - Upon input the size in BYTEs of the name structure 
                       array.  Upon success the actual number of BYTEs 
                       copied to the name structure array
   pNames      [ O ] - The name structure array

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPLMNName(
   USHORT                     mcc,
   USHORT                     mnc,
   ULONG *                    pNamesSize, 
   BYTE *                     pNames )
{
   // Validate arguments
   if ( (pNamesSize == 0)
   ||   (*pNamesSize == 0)
   ||   (pNames == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSz = *pNamesSize;
   *pNamesSize = 0;


   WORD msgID = (WORD)eQMI_NAS_GET_PLMN_NAME;
   std::vector <sDB2PackingInput> piv;

   // "%hu %hu"
   std::ostringstream tmp;
   tmp << mcc << " " << mnc;

   sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );


   sProtocolBuffer rsp = Send( eQMI_SVC_NAS, pRequest );
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

   // Try to find TLV ID 16
   std::map <ULONG, const sQMIRawContentHeader *> tlvs;
   tlvs = qmiRsp.GetContents();

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = tlvs.find( 16 );
   if (pIter == tlvs.end())
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Enough space to copy result?
   const sQMIRawContentHeader * pHdr = pIter->second;
   ULONG needSz = (ULONG)pHdr->mLength;
   if (needSz == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pNamesSize = needSz;
   if (needSz > maxSz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   pHdr++;
   const BYTE * pData = (const BYTE *)pHdr;

   memcpy( (LPVOID)pNames,
           (LPCVOID)pData,
           (SIZE_T)needSz );

   return eGOBI_ERR_NONE;
}
