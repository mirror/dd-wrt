/*===========================================================================
FILE: 
   GobiQMICorePDS.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (PDS Service)

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
   GetPDSState (Public Method)

DESCRIPTION:
   This function returns the current PDS state

PARAMETERS:
   pEnabled    [ O ] - Current PDS state (0 = disabled)
   pTracking   [ O ] - Current PDS tracking session state


RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPDSState( 
   ULONG *                    pEnabled,
   ULONG *                    pTracking )
{
   // Validate arguments
   if (pEnabled == 0 || pTracking == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pEnabled = ULONG_MAX;
   *pTracking = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_STATE;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 2) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pEnabled = pf[0].mValue.mU32;
   *pTracking = pf[1].mValue.mU32;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetPDSState (Public Method)

DESCRIPTION:
   This function sets the PDS state

PARAMETERS:
   enable      [ I ] - Desired PDS state (0 = disable)
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetPDSState( ULONG enable )
{
   WORD msgID = (WORD)eQMI_PDS_SET_STATE;
   std::vector <sDB2PackingInput> piv;

   LPCSTR pVal = enable != 0 ? "1" : "0";

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, pVal );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest, 5000 );
}

/*===========================================================================
METHOD:
   PDSInjectTimeReference (Public Method)

DESCRIPTION:
   This function injects a system time into the PDS engine

PARAMETERS:
   sysTime              [ I ] - System time
   sysDiscontinuities   [ I ] - Number of system time discontinuities
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::PDSInjectTimeReference( 
   ULONGLONG                  systemTime,
   USHORT                     systemDiscontinuities )
{
   WORD msgID = (WORD)eQMI_PDS_INJECT_TIME;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;   
   tmp << systemTime << " " << systemDiscontinuities;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 1 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetPDSDefaults (Public Method)

DESCRIPTION:
   This function returns the default tracking session configuration

PARAMETERS:
   pOperation  [ O ] - Current session operating mode 
   pTimeout    [ O ] - Maximum amount of time (seconds) to work on each fix
   pInterval   [ O ] - Interval (milliseconds) between fix requests
   pAccuracy   [ O ] - Current accuracy threshold (meters)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPDSDefaults( 
   ULONG *                    pOperation,
   BYTE *                     pTimeout,
   ULONG *                    pInterval,
   ULONG *                    pAccuracy )
{
   // Validate arguments
   if (pOperation == 0 || pTimeout == 0 || pInterval == 0 || pAccuracy == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pOperation = ULONG_MAX;
   *pTimeout = UCHAR_MAX;
   *pInterval = ULONG_MAX;
   *pAccuracy = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_DEFAULTS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 4) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Original QMI doc claimed milliseconds, turned out to be seconds
   ULONG apiInterval = pf[2].mValue.mU32 * 1000;

   *pOperation = pf[0].mValue.mU32;
   *pTimeout   = pf[1].mValue.mU8;
   *pInterval  = apiInterval;
   *pAccuracy  = pf[3].mValue.mU32;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetPDSDefaults (Public Method)

DESCRIPTION:
   This function sets the default tracking session configuration

PARAMETERS:
   operation   [ I ] - Desired session operating mode 
   timeout     [ I ] - Maximum amount of time (seconds) to work on each fix
   interval    [ I ] - Interval (milliseconds) between fix requests
   accuracy    [ I ] - Desired accuracy threshold (meters)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetPDSDefaults( 
   ULONG                      operation,
   BYTE                       timeout,
   ULONG                      interval,
   ULONG                      accuracy )
{
   WORD msgID = (WORD)eQMI_PDS_SET_DEFAULTS;
   std::vector <sDB2PackingInput> piv;

   // Original QMI doc claimed milliseconds, turned out to be seconds
   ULONG qmiInterval = interval / 1000;

   // "%u %u %u %u"
   std::ostringstream tmp;
   tmp << (UINT)operation << " " << (UINT)timeout << " " << (UINT)qmiInterval
       << " " << (UINT)accuracy;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 1 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetXTRAAutomaticDownload (Public Method)

DESCRIPTION:
   This function returns the XTRA automatic download configuration

PARAMETERS:
   pbEnabled   [ O ] - Automatic download enabled?
   pInterval   [ O ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetXTRAAutomaticDownload( 
   ULONG *                    pbEnabled,
   USHORT *                   pInterval )
{
   // Validate arguments
   if (pbEnabled == 0 || pInterval == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pbEnabled = ULONG_MAX;
   *pInterval = USHRT_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_XTRA_PARAMS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 2) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pbEnabled = pf[0].mValue.mU32;
   *pInterval = pf[1].mValue.mU16;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetXTRAAutomaticDownload (Public Method)

DESCRIPTION:
   This function sets the XTRA automatic download configuration

PARAMETERS:
   bEnabled    [ I ] - Automatic download enabled?
   interval    [ I ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetXTRAAutomaticDownload( 
   ULONG                      bEnabled,
   USHORT                     interval )
{
   WORD msgID = (WORD)eQMI_PDS_SET_XTRA_PARAMS;
   std::vector <sDB2PackingInput> piv;

   // "%u %hu"
   std::ostringstream tmp;
   tmp << (UINT)bEnabled << " " << (USHORT)interval;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 16 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetXTRANetwork (Public Method)

DESCRIPTION:
   This function returns the XTRA WWAN network preference

PARAMETERS:
   pPreference [ O ] - XTRA WWAN network preference

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetXTRANetwork( ULONG * pPreference )
{
   // Validate arguments
   if (pPreference == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pPreference = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_XTRA_PARAMS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 18 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pPreference = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetXTRANetwork (Public Method)

DESCRIPTION:
   This function sets the XTRA WWAN network preference

PARAMETERS:
   preference  [ I ] - XTRA WWAN network preference

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetXTRANetwork( ULONG preference )
{
   WORD msgID = (WORD)eQMI_PDS_SET_XTRA_PARAMS;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (UINT)preference;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 18 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetXTRAValidity (Public Method)

DESCRIPTION:
   This function returns the XTRA database validity period

PARAMETERS:
   pGPSWeek       [ O ] - Starting GPS week of validity period
   pGPSWeekOffset [ O ] - Starting GPS week offset (minutes) of validity period
   pDuration      [ O ] - Length of validity period (hours)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetXTRAValidity(
   USHORT *                   pGPSWeek,
   USHORT *                   pGPSWeekOffset, 
   USHORT *                   pDuration )
{
   // Validate arguments
   if (pGPSWeek == 0 || pGPSWeekOffset == 0 || pDuration == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pGPSWeek = USHRT_MAX;
   *pGPSWeekOffset = USHRT_MAX;
   *pDuration = USHRT_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_XTRA_PARAMS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 19 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 3) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pGPSWeek = pf[0].mValue.mU16;
   *pGPSWeekOffset = pf[1].mValue.mU16;
   *pDuration = pf[2].mValue.mU16;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ForceXTRADownload (Public Method)

DESCRIPTION:
   This function forces the XTRA database to be downloaded to the device

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ForceXTRADownload()
{
   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_FORCE_XTRA_DL;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   GetXTRADataState (Public Method)

DESCRIPTION:
   This function returns the XTRA data positioning state

PARAMETERS:
   pState      [ O ] - XTRA data positioning state

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetXTRADataState( ULONG * pState )
{
   // Validate arguments
   if (pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pState = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_METHODS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetXTRADataState (Public Method)

DESCRIPTION:
   This function sets the XTRA data positioning state

PARAMETERS:
   state       [ I ] - XTRA data positioning state

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetXTRADataState( ULONG state )
{
   WORD msgID = (WORD)eQMI_PDS_SET_METHODS;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << state;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 17 );
   sDB2PackingInput pi( pek, tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetXTRATimeState (Public Method)

DESCRIPTION:
   This function returns the XTRA time positioning state

PARAMETERS:
   pState      [ O ] - XTRA time positioning state

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetXTRATimeState( ULONG * pState )
{
   // Validate arguments
   if (pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pState = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_METHODS;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pState = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetXTRATimeState (Public Method)

DESCRIPTION:
   This function sets the XTRA time positioning state

PARAMETERS:
   state       [ I ] - XTRA time positioning state

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetXTRATimeState( ULONG state )
{
   WORD msgID = (WORD)eQMI_PDS_SET_METHODS;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << state;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 16 );
   sDB2PackingInput pi( pek, tmp.str().c_str() );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetAGPSConfig (Public Method)

DESCRIPTION:
   This function returns the PDS AGPS configuration

PARAMETERS:
   pServerAddress [ O ] - IPv4 address of AGPS server
   pServerPort    [ O ] - Port number of AGPS server

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetAGPSConfig( 
   ULONG *                    pServerAddress,
   ULONG *                    pServerPort )
{
   // Validate arguments
   if (pServerAddress == 0 || pServerPort == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pServerAddress = ULONG_MAX;
   *pServerPort = ULONG_MAX;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_AGPS_CONFIG;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 5) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG ip4 = (ULONG)pf[0].mValue.mU8;
   ULONG ip3 = (ULONG)pf[1].mValue.mU8 << 8;
   ULONG ip2 = (ULONG)pf[2].mValue.mU8 << 16;
   ULONG ip1 = (ULONG)pf[3].mValue.mU8 << 24;
   *pServerAddress = (ip4 | ip3 | ip2 | ip1);
   *pServerPort = pf[4].mValue.mU32;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetAGPSConfig (Public Method)

DESCRIPTION:
   This function sets the PDS AGPS configuration

PARAMETERS:
   serverAddress [ I ] - IPv4 address of AGPS server
   serverPort    [ I ] - Port number of AGPS server

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetAGPSConfig( 
   ULONG                      serverAddress,
   ULONG                      serverPort )
{
   WORD msgID = (WORD)eQMI_PDS_SET_AGPS_CONFIG;
   std::vector <sDB2PackingInput> piv;

   ULONG ip4 = (serverAddress & 0x000000FF);
   ULONG ip3 = (serverAddress & 0x0000FF00) >> 8;
   ULONG ip2 = (serverAddress & 0x00FF0000) >> 16;
   ULONG ip1 = (serverAddress & 0xFF000000) >> 24;

   
   // "%u %u %u %u %u"
   std::ostringstream tmp;
   tmp << (UINT)ip4 << " " << (UINT)ip3 << " " << (UINT)ip2 
       << " " << (UINT)ip1 << " " << (UINT)serverPort;

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 16 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetServiceAutomaticTracking (Public Method)

DESCRIPTION:
   This function returns the automatic tracking state for the service

PARAMETERS:
   pbAuto      [ O ] - Automatic tracking session started for service?

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetServiceAutomaticTracking( ULONG * pbAuto )
{
   // Validate arguments
   if (pbAuto == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_SVC_AUTOTRACK;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pbAuto = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetServiceAutomaticTracking (Public Method)

DESCRIPTION:
   This function sets the automatic tracking state for the service

PARAMETERS:
   pbAuto      [ I ] - Start automatic tracking session for service?

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetServiceAutomaticTracking( ULONG bAuto )
{
   WORD msgID = (WORD)eQMI_PDS_SET_SVC_AUTOTRACK;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (ULONG)(bAuto != 0);
   
   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 1 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   GetPortAutomaticTracking (Public Method)

DESCRIPTION:
   This function returns the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   pbAuto      [ O ] - Automatic tracking enabled for NMEA COM port?

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetPortAutomaticTracking( ULONG * pbAuto )
{
   // Validate arguments
   if (pbAuto == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_PDS_GET_COM_AUTOTRACK;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_PDS, msgID );
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_RSP, msgID, 1 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pbAuto = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetPortAutomaticTracking (Public Method)

DESCRIPTION:
   This function sets the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   pbAuto      [ I ] - Enable automatic tracking for NMEA COM port?

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetPortAutomaticTracking( ULONG bAuto )
{
   WORD msgID = (WORD)eQMI_PDS_SET_COM_AUTOTRACK;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream tmp;
   tmp << (ULONG)(bAuto != 0);

   sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 1 );
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
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}

/*===========================================================================
METHOD:
   ResetPDSData (Public Method)

DESCRIPTION:
   This function resets the specified PDS data

PARAMETERS:
   pGPSDataMask   [ I ] - Bitmask of GPS data to clear (optional)
   pCellDataMask  [ I ] - Bitmask of cell data to clear (optional)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::ResetPDSData( 
   ULONG *                    pGPSDataMask,
   ULONG *                    pCellDataMask )
{
   // Validate arguments (one must be present)
   if (pGPSDataMask == 0 && pCellDataMask == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_PDS_RESET_DATA;
   std::vector <sDB2PackingInput> piv;

   if (pGPSDataMask != 0)
   {
      ULONG mask = *pGPSDataMask;

      // Note that we are being lazy here by specifying more arguments
      // than the DB description defines; that will not cause a problem 
      // and we don't want to have to update this code should more bits
      // be defined
      std::ostringstream tmp;
      for (ULONG b = 0; b < 32; b++)
      {
         ULONG bit = mask & 0x00000001;
         mask = mask >> 1;

         tmp << bit << " ";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 16 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   if (pCellDataMask != 0)
   {
      ULONG mask = *pCellDataMask;

      std::ostringstream tmp;
      for (ULONG b = 0; b < 32; b++)
      {
         ULONG bit = mask & 0x00000001;
         mask = mask >> 1;

         tmp << bit << " ";
      }

      sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 17 );
      sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() );
      piv.push_back( pi );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }
   
   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_PDS, pRequest );
}
