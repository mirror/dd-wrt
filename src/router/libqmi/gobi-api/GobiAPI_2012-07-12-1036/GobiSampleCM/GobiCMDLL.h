/*===========================================================================
FILE:
   GobiCMDLL.h

DESCRIPTION:
   Simple class to load and interface to the Gobi CM DLL
   
PUBLIC CLASSES AND METHODS:
   cGobiCMDLL
      This class loads the Gobi CM DLL and then interfaces to it

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
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "GobiConnectionMgmtAPI.h"
#include "GobiConnectionMgmtAPIStructs.h"
#include <map>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
// Handle to Gobi API
typedef ULONG_PTR GOBIHANDLE;

// The maximum number of signals
const ULONG MAX_SIGNALS = 12;

// The maximum number of data capabilities
const ULONG MAX_DATA_CAPABILITIES = 12;

// Gobi input/output function pointer
typedef ULONG (* tFNGobiInputOutput)( 
   GOBIHANDLE                    handle,
   ULONG                         to,
   ULONG                         inLen,
   const BYTE *                  pIn,
   ULONG *                       pOutLen,
   BYTE *                        pOut );

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetTLVs (Internal Method)

DESCRIPTION:
   Convert response buffer to a TLV map

PARAMETERS:
   pRsp        [ I ] - The response buffer
   rspSz       [ I ] - The size of the above buffer
  
RETURN VALUE:
   std::map <UINT8, const sQMIRawContentHeader *>
===========================================================================*/ 
inline std::map <UINT8, const sQMIRawContentHeader *> GetTLVs( 
   const UINT8 *              pRsp,
   ULONG                      rspSz )
{
   std::map <UINT8, const sQMIRawContentHeader *> retMap;
   
   ULONG dataProcessed = 0;
   const UINT8 * pData = &pRsp[0];
   while (dataProcessed < rspSz)
   {
      dataProcessed += (ULONG)sizeof( sQMIRawContentHeader );
      if (dataProcessed > rspSz)
      {
         break;
      }

      const sQMIRawContentHeader * pTLV = (const sQMIRawContentHeader *)pData;
      dataProcessed += (ULONG)pTLV->mLength;
      if (dataProcessed > rspSz)
      {
         break;
      }

      retMap[pTLV->mTypeID] = pTLV;
      pData = &pRsp[dataProcessed];
   }

   return retMap;
};

/*=========================================================================*/
// Class cGobiCMDLL
/*=========================================================================*/
class cGobiCMDLL
{
   public:
      // Constructor
      cGobiCMDLL()
         : mhGobi( 0 )
      { }
   
      // Destructor
      ~cGobiCMDLL()
      { }

      // Connect
      ULONG Connect( LPCSTR pInterface );

      // Disconnect
      ULONG Disconnect();

      // Start data session
      ULONG StartDataSession(
         LPCSTR                     pAPN,
         LPCSTR                     pUser,
         LPCSTR                     pPwd,
         ULONG *                    pSessionID,
         ULONG *                    pFailureCode );

      // Cancel data session
      ULONG CancelDataSession();

      // Stop data session
      ULONG StopDataSession( ULONG sessionID );

      // Get session state
      ULONG GetSessionState( ULONG * pSessionState );

      // Get session duration
      ULONG GetSessionDuration( ULONGLONG * pSessionDuration );

      // Get data bearer technology
      ULONG GetDataBearerTechnology( ULONG * pDataBearerTech );

      // Get connection rate
      ULONG GetConnectionRate(
         ULONG *                    pCurTX,
         ULONG *                    pCurRX,
         ULONG *                    pMaxTX,
         ULONG *                    pMaxRX );

      // Get firmware revision
      ULONG GetFirmwareRevision(
         BYTE                       strSz,
         CHAR *                     pStr );

      // Get manufacturer
      ULONG GetManufacturer(
         BYTE                       strSz,
         CHAR *                     pStr );

      // Get model ID
      ULONG GetModelID(
         BYTE                       strSz,
         CHAR *                     pStr );

      // Get hardware revision
      ULONG GetHardwareRevision(
         BYTE                       strSz,
         CHAR *                     pStr );

      // Get voice number
      ULONG GetVoiceNumber(
         BYTE                       voiceSz,
         CHAR *                     pVoiceStr,
         BYTE                       minSz,
         CHAR *                     pMINStr );

      // Get serial numbers
      ULONG GetSerialNumbers(
         BYTE                       esnSz,
         CHAR *                     pESNStr,
         BYTE                       imeiSz,
         CHAR *                     pIMEIStr,
         BYTE                       meidSz,
         CHAR *                     pMEIDStr );

      // Get IMSI
      ULONG GetIMSI(
         BYTE                       imsiSz,
         CHAR *                     pIMSIStr );

      // Get signal strengths
      ULONG GetSignalStrengths(
         INT8 *                     pSigStrengths,
         ULONG *                    pRadioInterfaces );

      // Get serving network
      ULONG GetServingNetwork(
         ULONG *                    pDataCapabilities,
         WORD *                     pMCC,
         WORD *                     pMNC,
         BYTE                       nameSize,
         CHAR *                     pName,
         WORD *                     pSID,
         WORD *                     pNID,
         ULONG *                    pRoam );

      // Get home network
      ULONG GetHomeNetwork(
         WORD *                     pHomeMCC,
         WORD *                     pHomeMNC,
         BYTE                       homeNameSize,
         CHAR *                     pHomeName,
         WORD *                     pSID,
         WORD *                     pNID );

      // Set WDS event report callback
      ULONG SetWDSEventReportCB(
         tFNGenericCallback         pCallback,
         BYTE                       interval );

      // Set WDS packet service status callback
      ULONG SetWDSSessionStateCB( tFNGenericCallback pCallback );

      // Set NAS event report callback
      ULONG SetNASEventReportCB(
         tFNGenericCallback      pCallback,
         BYTE                    thresholdsSize,
         INT8 *                  pThresholds );

      // Set NAS serving system callback
      ULONG SetNASServingSystemCB( tFNGenericCallback pCallback );

   protected:

      // Call a Gobi CM API function that returns a string
      ULONG GetString(
         tFNGobiInputOutput         mpFnString,
         BYTE                       tlvID,
         BYTE                       strSz,
         CHAR *                     pStr );

      /* Handle to Gobi API */
      GOBIHANDLE mhGobi;
};
