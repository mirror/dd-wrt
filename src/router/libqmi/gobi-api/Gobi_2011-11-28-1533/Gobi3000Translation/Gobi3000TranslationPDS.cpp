/*===========================================================================
FILE: 
   Gobi3000TranslationPDS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (Position Determination Service)

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
   PackResetPDSData

DESCRIPTION:
   This function resets the specified PDS data

PARAMETERS:
   pOutLen        [I/O] - Upon input the maximum number of BYTEs pOut can
                          contain, upon output the number of BYTEs copied
                          to pOut
   pOut           [ O ] - Output buffer
   pGPSDataMask   [ I ] - Bitmask of GPS data to clear (optional)
   pCellDataMask  [ I ] - Bitmask of cell data to clear (optional)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackResetPDSData( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG *                    pGPSDataMask,
   ULONG *                    pCellDataMask )
{
   // Validate arguments (at least one mask must be present)
   if (pOut == 0 || (pGPSDataMask == 0 && pCellDataMask == 0))
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   sQMIRawContentHeader * pHeader;
   ULONG offset = 0;

   // Optionally add pGPSDataMask
   if (pGPSDataMask != 0)
   {
      // Check size
      WORD tlvx10Sz = sizeof( sPDSResetPDSDataRequest_GPSData );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx10Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x10;
      pHeader->mLength = tlvx10Sz;
      offset += sizeof( sQMIRawContentHeader );

      sPDSResetPDSDataRequest_GPSData * pTLVx10;
      pTLVx10 = (sPDSResetPDSDataRequest_GPSData*)(pOut + offset);
      memset( pTLVx10, 0, tlvx10Sz );

      // Typecast the input over the bitmask
	   *(ULONG *)pTLVx10 = *pGPSDataMask;
      offset += tlvx10Sz;
   }

   // Optionally add pCellDataMask
   if (pCellDataMask != 0)
   {
      // Check size
      WORD tlvx11Sz = sizeof( sPDSResetPDSDataRequest_CellData );
      if (*pOutLen < offset + sizeof( sQMIRawContentHeader ) + tlvx11Sz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pHeader = (sQMIRawContentHeader*)(pOut + offset);
      pHeader->mTypeID = 0x11;
      pHeader->mLength = tlvx11Sz;
      offset += sizeof( sQMIRawContentHeader );

      sPDSResetPDSDataRequest_CellData * pTLVx11;
      pTLVx11 = (sPDSResetPDSDataRequest_CellData*)(pOut + offset);
      memset( pTLVx11, 0, tlvx11Sz );

      // Typecast the input over the bitmask
	   *(ULONG *)pTLVx11 = *pCellDataMask;
      offset += tlvx11Sz;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetPortAutomaticTracking

DESCRIPTION:
   This function sets the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   bAuto       [ I ] - Enable automatic tracking for NMEA COM port?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetPortAutomaticTracking( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bAuto )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add bAuto
   
   // Check size
   WORD tlvx01Sz = sizeof( sPDSSetCOMPortAutoTrackingConfigRequest_Config );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetCOMPortAutoTrackingConfigRequest_Config * pTLVx01;
   pTLVx01 = (sPDSSetCOMPortAutoTrackingConfigRequest_Config*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mAutoTrackingEnabled = (bAuto == 0 ? 0 : 1);
   
   offset += tlvx01Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPortAutomaticTracking

DESCRIPTION:
   This function returns the automatic tracking configuration for the NMEA
   COM port

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pbAuto      [ O ] - Automatic tracking enabled for NMEA COM port?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPortAutomaticTracking( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pbAuto )
{
   // Validate arguments
   if (pIn == 0 || pbAuto == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find pbAuto
   const sPDSGetCOMPortAutoTrackingConfigResponse_Config * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sPDSGetCOMPortAutoTrackingConfigResponse_Config ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbAuto = pTLVx01->mAutoTrackingEnabled;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetServiceAutomaticTracking

DESCRIPTION:
   This function sets the automatic tracking state for the service

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   pbAuto      [ I ] - Start automatic tracking session for service?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetServiceAutomaticTracking( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bAuto )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add bAuto
   
   // Check size
   WORD tlvx01Sz = sizeof( sPDSSetServiceAutoTrackingStateRequest_State );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetServiceAutoTrackingStateRequest_State * pTLVx01;
   pTLVx01 = (sPDSSetServiceAutoTrackingStateRequest_State*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mAutoTrackingEnabled = (bAuto == 0 ? 0 : 1);
   
   offset += tlvx01Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
/*===========================================================================
METHOD:
   ParseGetServiceAutomaticTracking

DESCRIPTION:
   This function returns the automatic tracking state for the service

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pbAuto      [ O ] - Automatic tracking session started for service?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetServiceAutomaticTracking( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pbAuto )
{
   // Validate arguments
   if (pIn == 0 || pbAuto == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find pbAuto
   const sPDSGetServiceAutoTrackingStateResponse_State * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sPDSGetServiceAutoTrackingStateResponse_State ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbAuto = pTLVx01->mAutoTrackingEnabled;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetAGPSConfig

DESCRIPTION:
   This function sets the PDS AGPS configuration

PARAMETERS:
   pOutLen       [I/O] - Upon input the maximum number of BYTEs pOut can
                         contain, upon output the number of BYTEs copied
                         to pOut
   pOut          [ O ] - Output buffer
   serverAddress [ I ] - IPv4 address of AGPS server
   serverPort    [ I ] - Port number of AGPS server

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetAGPSConfig( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      serverAddress,
   ULONG                      serverPort )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx10Sz = sizeof( sPDSSetAGPSConfigRequest_Server );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetAGPSConfigRequest_Server * pTLVx10;
   pTLVx10 = (sPDSSetAGPSConfigRequest_Server*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   ULONG ip0 = (serverAddress & 0x000000FF);
   ULONG ip1 = (serverAddress & 0x0000FF00) >> 8;
   ULONG ip2 = (serverAddress & 0x00FF0000) >> 16;
   ULONG ip3 = (serverAddress & 0xFF000000) >> 24;

   // Set the values
   pTLVx10->mServerAddress[0] = (INT8)ip0; 
   pTLVx10->mServerAddress[1] = (INT8)ip1;
   pTLVx10->mServerAddress[2] = (INT8)ip2;
   pTLVx10->mServerAddress[3] = (INT8)ip3;
   pTLVx10->mServerPort = serverPort;
   
   offset += tlvx10Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetAGPSConfig

DESCRIPTION:
   This function returns the PDS AGPS configuration

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pServerAddress [ O ] - IPv4 address of AGPS server
   pServerPort    [ O ] - Port number of AGPS server

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetAGPSConfig( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pServerAddress,
   ULONG *                    pServerPort )
{
   // Validate arguments
   if (pIn == 0 || pServerAddress == 0 || pServerPort == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find arguments
   const sPDSGetAGPSConfigResponse_ServerAddress * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sPDSGetAGPSConfigResponse_ServerAddress ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pServerPort = pTLVx01->mServerPort;

   ULONG ip0 = (ULONG)pTLVx01->mServerAddress[0];
   ULONG ip1 = (ULONG)pTLVx01->mServerAddress[1] << 8;
   ULONG ip2 = (ULONG)pTLVx01->mServerAddress[2] << 16;
   ULONG ip3 = (ULONG)pTLVx01->mServerAddress[3] << 24;
   *pServerAddress = (ip0 | ip1 | ip2 | ip3);

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetXTRATimeState

DESCRIPTION:
   This function sets the XTRA time positioning state

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   state       [ I ] - XTRA time positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetXTRATimeState( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      state )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add state
   
   // Check size
   WORD tlvx10Sz = sizeof( sPDSSetPositionMethodsStateRequest_XTRATime );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetPositionMethodsStateRequest_XTRATime * pTLVx10;
   pTLVx10 = (sPDSSetPositionMethodsStateRequest_XTRATime*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mMethodState = (eQMIPDSMethodStates)state;
   
   offset += tlvx10Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetXTRATimeState

DESCRIPTION:
   This function returns the XTRA time positioning state

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pState      [ O ] - XTRA time positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetXTRATimeState( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pState )
{
   // Validate arguments
   if (pIn == 0 || pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find pState
   const sPDSGetPositionMethodsStateResponse_XTRATime * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sPDSGetPositionMethodsStateResponse_XTRATime ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pState = pTLVx10->mMethodState;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetXTRADataState

DESCRIPTION:
   This function sets the XTRA data positioning state

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   state       [ I ] - XTRA data positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetXTRADataState( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      state )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add state
   
   // Check size
   WORD tlvx10Sz = sizeof( sPDSSetPositionMethodsStateRequest_XTRAData );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetPositionMethodsStateRequest_XTRAData * pTLVx10;
   pTLVx10 = (sPDSSetPositionMethodsStateRequest_XTRAData*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mMethodState = (eQMIPDSMethodStates)state;
   
   offset += tlvx10Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetXTRADataState

DESCRIPTION:
   This function returns the XTRA data positioning state

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pState      [ O ] - XTRA data positioning state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetXTRADataState( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pState )
{
   // Validate arguments
   if (pIn == 0 || pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find pState
   const sPDSGetPositionMethodsStateResponse_XTRAData * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sPDSGetPositionMethodsStateResponse_XTRAData ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pState = pTLVx10->mMethodState;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetXTRAValidity

DESCRIPTION:
   This function returns the XTRA database validity period

PARAMETERS:
   inLen          [ I ] - Length of input buffer
   pIn            [ I ] - Input buffer
   pGPSWeek       [ O ] - Starting GPS week of validity period
   pGPSWeekOffset [ O ] - Starting GPS week offset (minutes) of validity period
   pDuration      [ O ] - Length of validity period (hours)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetXTRAValidity(
   ULONG                      inLen,
   const BYTE *               pIn,
   USHORT *                   pGPSWeek,
   USHORT *                   pGPSWeekOffset, 
   USHORT *                   pDuration )
{
   // Validate arguments
   if (pIn == 0 || pGPSWeek == 0 || pGPSWeekOffset == 0 || pDuration == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find arguments
   const sPDSGetXTRAParametersResponse_Validity * pTLVx13;
   ULONG outLenx13;
   ULONG rc = GetTLV( inLen, pIn, 0x13, &outLenx13, (const BYTE **)&pTLVx13 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx13 < sizeof( sPDSGetXTRAParametersResponse_Validity ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pDuration = pTLVx13->mValidPeriodDurationInHours;
   *pGPSWeek = pTLVx13->mValidPeriodGPSStartWeek;
   *pGPSWeekOffset = pTLVx13->mValidPeriodGPSStartWeekOffsetInMinutes;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetXTRANetwork

DESCRIPTION:
   This function sets the XTRA WWAN network preference

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   preference  [ I ] - XTRA WWAN network preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetXTRANetwork( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      preference )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add preference
   
   // Check size
   WORD tlvx12Sz = sizeof( sPDSSetXTRAParametersRequest_Network );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx12Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x12;
   pHeader->mLength = tlvx12Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetXTRAParametersRequest_Network * pTLVx12;
   pTLVx12 = (sPDSSetXTRAParametersRequest_Network*)(pOut + offset);
   memset( pTLVx12, 0, tlvx12Sz );

   // Set the value
   pTLVx12->mWWANNetworkPreference = (eQMIPDSWWANNetworkPreferences)preference;
   
   offset += tlvx12Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetXTRANetwork

DESCRIPTION:
   This function returns the XTRA WWAN network preference

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pPreference [ O ] - XTRA WWAN network preference

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetXTRANetwork( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pPreference )
{
   // Validate arguments
   if (pIn == 0 || pPreference == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find pPreference
   const sPDSGetXTRAParametersResponse_Network * pTLVx12;
   ULONG outLenx12;
   ULONG rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx12 < sizeof( sPDSGetXTRAParametersResponse_Network ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pPreference = pTLVx12->mWWANNetworkPreference;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetXTRAAutomaticDownload

DESCRIPTION:
   This function sets the XTRA automatic download configuration

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   bEnabled    [ I ] - Automatic download enabled?
   interval    [ I ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetXTRAAutomaticDownload( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      bEnabled,
   USHORT                     interval )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx10Sz = sizeof( sPDSSetXTRAParametersRequest_Automatic );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx10Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x10;
   pHeader->mLength = tlvx10Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetXTRAParametersRequest_Automatic * pTLVx10;
   pTLVx10 = (sPDSSetXTRAParametersRequest_Automatic*)(pOut + offset);
   memset( pTLVx10, 0, tlvx10Sz );

   // Set the value
   pTLVx10->mAutomaticDownloadEnabled = (bEnabled == 0 ? 0 : 1);
   pTLVx10->mDownloadIntervalInHours = interval;
   
   offset += tlvx10Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetXTRAAutomaticDownload

DESCRIPTION:
   This function returns the XTRA automatic download configuration

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pbEnabled   [ O ] - Automatic download enabled?
   pInterval   [ O ] - Interval (hours) between XTRA downloads

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetXTRAAutomaticDownload( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pbEnabled,
   USHORT *                   pInterval )
{
   // Validate arguments
   if (pIn == 0 || pbEnabled == 0 || pInterval == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find arguments
   const sPDSGetXTRAParametersResponse_Automatic * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx10 < sizeof( sPDSGetXTRAParametersResponse_Automatic ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pbEnabled = pTLVx10->mAutomaticDownloadEnabled;
   *pInterval = pTLVx10->mDownloadIntervalInHours;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPDSState

DESCRIPTION:
   This function returns the current PDS state

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pEnabled    [ O ] - Current PDS state (0 = disabled)
   pTracking   [ O ] - Current PDS tracking session state


RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPDSState( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pEnabled,
   ULONG *                    pTracking )
{
   // Validate arguments
   if (pIn == 0 || pEnabled == 0 || pTracking == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }
   
   // Find arguments
   const sPDSGetServiceStateResponse_State * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sPDSGetServiceStateResponse_State ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pEnabled = pTLVx01->mServiceEnabled;
   *pTracking = pTLVx01->mTrackingSessionState;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetPDSState

DESCRIPTION:
   This function sets the PDS state

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   enable      [ I ] - Desired PDS state (0 = disable)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetPDSState( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      enable )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add enable
   
   // Check size
   WORD tlvx01Sz = sizeof( sPDSSetServiceStateRequest_State );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetServiceStateRequest_State * pTLVx01;
   pTLVx01 = (sPDSSetServiceStateRequest_State*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mServiceEnabled = (enable == 0 ? 0 : 1);
   
   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackPDSInjectTimeReference

DESCRIPTION:
   This function injects a system time into the PDS engine

PARAMETERS:
   pOutLen              [I/O] - Upon input the maximum number of BYTEs pOut can
                                contain, upon output the number of BYTEs copied
                                to pOut
   pOut                 [ O ] - Output buffer
   sysTime              [ I ] - System time
   sysDiscontinuities   [ I ] - Number of system time discontinuities
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackPDSInjectTimeReference( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONGLONG                  systemTime,
   USHORT                     systemDiscontinuities )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx01Sz = sizeof( sPDSInjectTimeReferenceRequest_Time );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSInjectTimeReferenceRequest_Time * pTLVx01;
   pTLVx01 = (sPDSInjectTimeReferenceRequest_Time*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mSystemTimeMilliseconds = systemTime;
   pTLVx01->mSystemDiscontinuties = systemDiscontinuities;
   
   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPDSDefaults

DESCRIPTION:
   This function returns the default tracking session configuration

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOperation  [ O ] - Current session operating mode 
   pTimeout    [ O ] - Maximum amount of time (seconds) to work on each fix
   pInterval   [ O ] - Interval (milliseconds) between fix requests
   pAccuracy   [ O ] - Current accuracy threshold (meters)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPDSDefaults( 
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOperation,
   BYTE *                     pTimeout,
   ULONG *                    pInterval,
   ULONG *                    pAccuracy )
{
   // Validate arguments
   if (pIn == 0 || pOperation == 0 || pTimeout == 0
   ||  pInterval == 0 || pAccuracy == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find arguments
   const sPDSGetDefaultsResponse_Defaults * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Is the TLV large enough?
   if (outLenx01 < sizeof( sPDSGetDefaultsResponse_Defaults ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pOperation = pTLVx01->mSessionOperation;
   *pTimeout = pTLVx01->mTimeoutSeconds;
   *pInterval = pTLVx01->mFixRequestIntervalSeconds;
   *pAccuracy = pTLVx01->mDesiredAccuracyMeters;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetPDSDefaults

DESCRIPTION:
   This function sets the default tracking session configuration

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   operation   [ I ] - Desired session operating mode 
   timeout     [ I ] - Maximum amount of time (seconds) to work on each fix
   interval    [ I ] - Interval (milliseconds) between fix requests
   accuracy    [ I ] - Desired accuracy threshold (meters)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetPDSDefaults( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      operation,
   BYTE                       timeout,
   ULONG                      interval,
   ULONG                      accuracy )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx01Sz = sizeof( sPDSSetDefaultsRequest_Defaults );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sPDSSetDefaultsRequest_Defaults * pTLVx01;
   pTLVx01 = (sPDSSetDefaultsRequest_Defaults*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mDesiredAccuracyMeters = accuracy;
   pTLVx01->mFixRequestIntervalSeconds = interval;
   pTLVx01->mSessionOperation = (eQMIPDSOperationTypes)operation;
   pTLVx01->mTimeoutSeconds = timeout;
   
   offset += tlvx01Sz;

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
