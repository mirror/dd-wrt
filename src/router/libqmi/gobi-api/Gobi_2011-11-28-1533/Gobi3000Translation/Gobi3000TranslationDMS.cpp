/*===========================================================================
FILE:
   Gobi3000TranslationDMS.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (DMS Service)

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
   ParseGetDeviceCapabilities

DESCRIPTION:
   This function gets device capabilities

PARAMETERS:
   inLen                   [ I ] - Length of input buffer
   pIn                     [ I ] - Input buffer
   pMaxTXChannelRate       [ O ] - Maximum transmission rate (bps)
   pMaxRXChannelRate       [ O ] - Maximum reception rate (bps)
   pDataServiceCapability  [ O ] - CS/PS data service capability
   pSimCapability          [ O ] - Device SIM support
   pRadioIfacesSize        [I/O] - Upon input the maximum number of elements
                                   that the radio interfaces can contain.
                                   Upon successful output the actual number
                                   of elements in the radio interface array
   pRadioIfaces            [ O ] - The radio interface array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetDeviceCapabilities(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pMaxTXChannelRate,
   ULONG *           pMaxRXChannelRate,
   ULONG *           pDataServiceCapability,
   ULONG *           pSimCapability,
   ULONG *           pRadioIfacesSize,
   BYTE *            pRadioIfaces )
{
   // Validate arguments
   if (pIn == 0
   ||  pMaxTXChannelRate == 0
   ||  pMaxRXChannelRate == 0
   ||  pDataServiceCapability == 0
   ||  pSimCapability == 0
   ||  pRadioIfacesSize == 0
   ||  *pRadioIfacesSize == 0
   ||  pRadioIfaces == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxRadioIfaces = (ULONG)*pRadioIfacesSize;

   // Assume failure
   *pRadioIfacesSize = 0;

   const sDMSGetDeviceCapabilitiesResponse_Capabilities * pTLVx01;
   ULONG structSzx01 = sizeof( sDMSGetDeviceCapabilitiesResponse_Capabilities );
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < structSzx01)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   // Populate the variables
   *pMaxTXChannelRate = pTLVx01->mMaxTXRatebps;
   *pMaxRXChannelRate = pTLVx01->mMaxRXRatebps;
   *pDataServiceCapability = pTLVx01->mDataServiceCapability;
   
   // SIM capability should be treated as a boolean, even though it's not
   *pSimCapability = (pTLVx01->mSIMSupported == 0 ? 0 : 1);

   ULONG activeRadioIfaces = pTLVx01->mRadioInterfaceCount;
   if (activeRadioIfaces > maxRadioIfaces)
   {
      activeRadioIfaces = maxRadioIfaces;
   }

   const eQMIDMSRadioInterfaces * pInRadioInterfaces;

   // Verify there is room for the array in the TLV
   if (outLenx01 < structSzx01
                 + sizeof( eQMIDMSRadioInterfaces ) * activeRadioIfaces)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Align to the first array element
   pInRadioInterfaces = (const eQMIDMSRadioInterfaces *)
                        ((const BYTE *)pTLVx01 + structSzx01);

   ULONG * pOutRadioIfaces = (ULONG *)pRadioIfaces;
   for (ULONG r = 0; r < activeRadioIfaces; r++)
   {
      *pOutRadioIfaces = *pInRadioInterfaces;
      pOutRadioIfaces++;
      pInRadioInterfaces++;
   }

   *pRadioIfacesSize = activeRadioIfaces;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetManufacturer

DESCRIPTION:
   This function returns the device manufacturer name

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetManufacturer(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString )
{
   // Validate arguments
   if (pIn == 0 || stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pString = 0;

   // Find the manufacturer
   // sDMSGetDeviceManfacturerResponse_Manfacturer only contains this
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (stringSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, pTLVx01, outLenx01 );
   pString[outLenx01] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetModelID

DESCRIPTION:
   This function returns the device model ID

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetModelID(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString )
{
   // Validate arguments
   if (pIn == 0 || stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pString = 0;

   // Find the model
   // sDMSGetDeviceModelResponse_Model only contains the model
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (stringSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, pTLVx01, outLenx01 );
   pString[outLenx01] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetFirmwareRevision

DESCRIPTION:
   This function returns the device firmware revision

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetFirmwareRevision(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString )
{
   // Validate arguments
   if (pIn == 0 || stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pString = 0;

   // Find the PRI revision
   // sDMSGetDeviceRevisionResponse_UQCNRevision only contains this
   const CHAR * pTLVx11;
   ULONG outLenx11;
   ULONG rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (stringSize < outLenx11 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, pTLVx11, outLenx11 );
   pString[outLenx11] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetFirmwareRevisions

DESCRIPTION:
   This function returns the device firmware (AMSS, boot, and PRI)
   revisions

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   amssSize    [ I ] - The maximum number of characters (including NULL
                       terminator) that the AMSS string array can contain
   pAMSSString [ O ] - NULL terminated AMSS revision string
   bootSize    [ I ] - The maximum number of characters (including NULL
                       terminator) that the boot string array can contain
   pBootString [ O ] - NULL terminated boot code revision string
   priSize     [ I ] - The maximum number of characters (including NULL
                       terminator) that the PRI string array can contain
   pPRIString  [ O ] - NULL terminated PRI revision string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetFirmwareRevisions(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              amssSize,
   CHAR *            pAMSSString,
   BYTE              bootSize,
   CHAR *            pBootString,
   BYTE              priSize,
   CHAR *            pPRIString )
{
   // Validate arguments
   if (pIn == 0
   ||  amssSize == 0 || pAMSSString == 0
   ||  bootSize == 0 || pBootString == 0
   ||  priSize == 0 || pPRIString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pAMSSString = 0;
   *pBootString = 0;
   *pPRIString = 0;

   // Find the AMSS version
   // sDMSGetDeviceRevisionResponse_Revision only contains this
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (amssSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pAMSSString, pTLVx01, outLenx01 );
   pAMSSString[outLenx01] = 0;

   // Find the Boot version
   // sDMSGetDeviceRevisionResponse_BootCodeRevision only contains this
   const CHAR * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (bootSize < outLenx10 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pBootString, pTLVx10, outLenx10 );
   pBootString[outLenx10] = 0;

   // The PRI version is returned by ParseGetFirmwareRevision()
   rc = ParseGetFirmwareRevision( inLen, pIn, priSize, pPRIString );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetVoiceNumber

DESCRIPTION:
   This function returns the voice number in use by the device

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   voiceNumberSize   [ I ] - The maximum number of characters (including NULL
                             terminator) that the voice number array can
                             contain
   pVoiceNumber      [ O ] - Voice number (MDN or ISDN) string
   minSize           [ I ] - The maximum number of characters (including NULL
                             terminator) that the MIN array can contain
   pMIN              [ O ] - MIN string (empty string returned when MIN is
                             not supported/programmed)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetVoiceNumber(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              voiceNumberSize,
   CHAR *            pVoiceNumber,
   BYTE              minSize,
   CHAR *            pMIN )
{
   // Validate arguments
   if (pIn == 0
   ||  voiceNumberSize == 0 || pVoiceNumber == 0
   ||  minSize == 0 || pMIN == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pVoiceNumber = 0;
   *pMIN = 0;

   // Find the Voice number
   // sDMSGetDeviceVoiceNumberResponse_VoiceNumber only contains this
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (voiceNumberSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pVoiceNumber, pTLVx01, outLenx01 );
   pVoiceNumber[outLenx01] = 0;

   // Find the Mobile ID (optional)
   // sDMSGetDeviceVoiceNumberResponse_MobileIDNumber only contains this
   const CHAR * pTLVx10;
   ULONG outLenx10;
   rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      // Space to perform the copy?
      if (minSize < outLenx10 + 1)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pMIN, pTLVx10, outLenx10 );
      pMIN[outLenx10] = 0;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetIMSI

DESCRIPTION:
   This function returns the device IMSI

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetIMSI(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString )
{
   // Validate arguments
   if (pIn == 0 ||  stringSize == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pString = 0;

   // Find the IMSI
   // sDMSGetDeviceVoiceNumberResponse_IMSI only contains this
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (stringSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, pTLVx01, outLenx01 );
   pString[outLenx01] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetSerialNumbers

DESCRIPTION:
   This command returns all serial numbers assigned to the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   esnSize     [ I ] - The maximum number of characters (including NULL
                       terminator) that the ESN array can contain
   pESNString  [ O ] - ESN string (empty string returned when ESN is
                       not supported/programmed)
   imeiSize    [ I ] - The maximum number of characters (including NULL
                       terminator) that the IMEI array can contain
   pIMEIString [ O ] - IMEI string (empty string returned when IMEI is
                       not supported/programmed)
   meidSize    [ I ] - The maximum number of characters (including NULL
                       terminator) that the MEID array can contain
   pMEIDString [ O ] - MEID string (empty string returned when MEID is
                       not supported/programmed)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetSerialNumbers(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              esnSize,
   CHAR *            pESNString,
   BYTE              imeiSize,
   CHAR *            pIMEIString,
   BYTE              meidSize,
   CHAR *            pMEIDString )
{
   // Validate arguments
   if (pIn == 0
   ||  esnSize == 0 || pESNString == 0
   ||  imeiSize == 0 || pIMEIString == 0
   ||  meidSize == 0 || pMEIDString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pESNString = 0;
   *pIMEIString = 0;
   *pMEIDString = 0;

   // Find the ESN
   // sDMSGetDeviceSerialNumbersResponse_ESN only contains this
   const CHAR * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (esnSize < outLenx10 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pESNString, pTLVx10, outLenx10 );
   pESNString[outLenx10] = 0;

   // Find the IMEI
   // sDMSGetDeviceSerialNumbersResponse_IMEI only contains this
   const CHAR * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (imeiSize < outLenx11 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pIMEIString, pTLVx11, outLenx11 );
   pIMEIString[outLenx11] = 0;

   // Find the MEID
   // sDMSGetDeviceSerialNumbersResponse_MEID only contains this
   const CHAR * pTLVx12;
   ULONG outLenx12;
   rc = GetTLV( inLen, pIn, 0x12, &outLenx12, (const BYTE **)&pTLVx12 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (meidSize < outLenx12 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pMEIDString, pTLVx12, outLenx12 );
   pMEIDString[outLenx12] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetLock

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   state       [ I ] - Desired lock state
   pCurrentPIN [ I ] - Current four digit PIN string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetLock(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             state,
   CHAR *            pCurrentPIN )
{
   // Validate arguments
   if (pOut == 0 || pCurrentPIN == 0 || pCurrentPIN[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string thePIN( pCurrentPIN );
   if (thePIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (thePIN.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSSetLockStateRequest_LockState );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSSetLockStateRequest_LockState * pTLVx01;
   pTLVx01 = (sDMSSetLockStateRequest_LockState*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mLockState = (eQMIDMSLockStates)state;
   memcpy( &pTLVx01->mLockCode[0], thePIN.c_str(), thePIN.size() );

   offset += tlvx01Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseQueryLock

DESCRIPTION:
   This function sets the user lock state maintained by the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pState      [ O ] - Current lock state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseQueryLock(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pState )
{
   // Validate arguments
   if (pIn == 0 || pState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the state
   const sDMSGetLockStateResponse_LockState * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSGetLockStateResponse_LockState ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pState = pTLVx01->mLockState;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackChangeLockPIN

DESCRIPTION:
   This command sets the user lock code maintained by the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   pCurrentPIN [ I ] - Current four digit PIN string
   pDesiredPIN [ I ] - New four digit PIN string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackChangeLockPIN(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pCurrentPIN,
   CHAR *            pDesiredPIN )
{
   // Validate arguments
   if (pOut == 0
   ||  pCurrentPIN == 0 || pCurrentPIN[0] == 0
   ||  pDesiredPIN == 0 || pDesiredPIN[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theCurPIN( pCurrentPIN );
   if (theCurPIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (theCurPIN.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string theNewPIN( pDesiredPIN );
   if (theNewPIN.size() > 4)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (theNewPIN.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSSetLockCodeRequest_LockCode );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSSetLockCodeRequest_LockCode * pTLVx01;
   pTLVx01 = (sDMSSetLockCodeRequest_LockCode*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mCurrentLockCode[0],
           theCurPIN.c_str(),
           theCurPIN.size() );

   memcpy( &pTLVx01->mNewLockCode[0],
           theNewPIN.c_str(),
           theNewPIN.size() );

   offset += tlvx01Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetHardwareRevision

DESCRIPTION:
   This function returns the device hardware revision

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   stringSize  [ I ] - The maximum number of characters (including NULL
                       terminator) that the string array can contain
   pString     [ O ] - NULL terminated string

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetHardwareRevision(
   ULONG             inLen,
   const BYTE *      pIn,
   BYTE              stringSize,
   CHAR *            pString )
{
   // Validate arguments
   if (pIn == 0 || pString == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the hardware revision
   // sDMSGetHardwareRevisionResponse_HardwareRevision only contains this
   const CHAR * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Space to perform the copy?
   if (stringSize < outLenx01 + 1)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( pString, pTLVx01, outLenx01 );
   pString[outLenx01] = 0;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPRLVersion

DESCRIPTION:
   This function returns the version of the active Preferred Roaming List
   (PRL) in use by the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pPRLVersion [ O ] - The PRL version number

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPRLVersion(
   ULONG             inLen,
   const BYTE *      pIn,
   WORD *            pPRLVersion )
{
   // Validate arguments
   if (pIn == 0 || pPRLVersion == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the state
   const sDMSGetPRLVersionResponse_PRLVersion * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSGetPRLVersionResponse_PRLVersion ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pPRLVersion = pTLVx01->mPRLVersion;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetERIFile

DESCRIPTION:
   This command returns the ERI file that is stored in EFS on the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pFileSize   [I/O] - Upon input the maximum number of bytes that the file
                       contents array can contain.  Upon successful output
                       the actual number of bytes written to the file contents
                       array
   pFile       [ O ] - The file contents

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetERIFile(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pFileSize,
   BYTE *            pFile )
{
   // Validate arguments
   if (pIn == 0 || pFileSize == 0 || *pFileSize == 0 || pFile == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   ULONG maxFileSize = *pFileSize;
   *pFileSize = 0;

   // Find the state
   const sDMSReadERIDataResponse_UserData * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSReadERIDataResponse_UserData ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   ULONG fileSz = pTLVx01->mDataLength;
   const BYTE * pInFile;

   // Verify there is room for the array in the TLV
   if (outLenx01 < sizeof( sDMSReadERIDataResponse_UserData )
                  + sizeof( BYTE ) * fileSz)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Space to copy into?
   if (fileSz > maxFileSize)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   // Align to the first array element
   pInFile = (const BYTE *)pTLVx01 
           + sizeof( sDMSReadERIDataResponse_UserData );

   // Perform the copy
   memcpy( pFile, pInFile, fileSz );
   *pFileSize = fileSz;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackActivateAutomatic

DESCRIPTION:
   This function requests the device to perform automatic service activation

PARAMETERS:
   pOutLen           [I/O] - Upon input the maximum number of BYTEs pOut can
                             contain, upon output the number of BYTEs copied
                             to pOut
   pOut              [ O ] - Output buffer
   pActivationCode   [ I ] - Activation code (maximum string length of 12)

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackActivateAutomatic(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pActivationCode )
{
   // Validate arguments
   if (pOut == 0 || pActivationCode == 0 || pActivationCode[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string ac( pActivationCode );
   if (ac.size() > 12)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSActivateAutomaticRequest_ActivationCode )
                 + (WORD)ac.size();
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sDMSActivateAutomaticRequest_ActivationCode * pTLVx01;
   pTLVx01 = (sDMSActivateAutomaticRequest_ActivationCode*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mCodeLength = (UINT8)ac.size();

   memcpy( (BYTE *)pTLVx01 
           + sizeof( sDMSActivateAutomaticRequest_ActivationCode ),
           ac.c_str(),
           ac.size() );

   offset += tlvx01Sz;

   *pOutLen = offset;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackResetToFactoryDefaults

DESCRIPTION:
   This function requests the device reset configuration to factory defaults

   CHANGES:
      * The client must manually reset the device after this request completes
         using DMSSetOperatingMode()

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied to pOut
   pOut        [ O ] - Output buffer
   pSPC        [ I ] - NULL terminated string representing the six digit
                       service programming code

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackResetToFactoryDefaults(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC )
{
   // Validate arguments
   if (pOut == 0 || pSPC == 0 || pSPC[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string spc( pSPC );
   if (spc.size() > 6)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (spc.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSResetFactoryDefaultsRequest_SPC );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The SPC
   sDMSResetFactoryDefaultsRequest_SPC * pTLVx01;
   pTLVx01 = (sDMSResetFactoryDefaultsRequest_SPC*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetActivationState

DESCRIPTION:
   This function returns the device activation state

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   pActivationState  [ O ] - Service activation state

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetActivationState(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pActivationState )
{
   // Validate arguments
   if (pIn == 0 || pActivationState == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the state
   const sDMSGetActivationStateResponse_ActivationState * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSGetActivationStateResponse_ActivationState ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pActivationState = pTLVx01->mActivationState;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackSetPower

DESCRIPTION:
   This function sets the operating mode of the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied to pOut
   pOut        [ O ] - Output buffer
   powerMode   [ I ] - Selected operating mode

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackSetPower(
   ULONG *           pOutLen,
   BYTE *            pOut,
   ULONG             powerMode )
{
   // Validate arguments
   if (pOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSSetOperatingModeRequest_OperatingMode );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // Set the mode
   sDMSSetOperatingModeRequest_OperatingMode * pTLVx01;
   pTLVx01 = (sDMSSetOperatingModeRequest_OperatingMode*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   pTLVx01->mOperatingMode = (eQMIDMSOperatingModes)powerMode;

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetPower

DESCRIPTION:
   This function returns the operating mode of the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pPowerMode  [ O ] - Current operating mode

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetPower(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pPowerMode )
{
   // Validate arguments
   if (pIn == 0 || pPowerMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pPowerMode = 0xffffffff;

   // Find the mode
   const sDMSGetOperatingModeResponse_OperatingMode * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSGetOperatingModeResponse_OperatingMode ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   *pPowerMode = pTLVx01->mOperatingMode;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetOfflineReason

DESCRIPTION:
   This function returns the reason why the operating mode of the device
   is currently offline

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pReasonMask [ O ] - Bitmask of offline reasons
   pbPlatform  [ O ] - Offline due to being platform retricted?

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetOfflineReason(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONG *           pReasonMask,
   ULONG *           pbPlatform )
{
   // Validate arguments
   if (pIn == 0 || pReasonMask == 0 || pbPlatform == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Assume failure
   *pReasonMask = 0;
   *pbPlatform = 0;

   // Find the reason mask (optional)
   const sDMSGetOperatingModeResponse_OfflineReason * pTLVx10;
   ULONG outLenx10;
   ULONG rc = GetTLV( inLen, pIn, 0x10, &outLenx10, (const BYTE **)&pTLVx10 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx10 < sizeof( sDMSGetOperatingModeResponse_OfflineReason ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Copy the bitmask to pReasonMask
      *pReasonMask = *(WORD*)pTLVx10;
   }

   // Find the platform restriction (optional)
   const sDMSGetOperatingModeResponse_PlatformRestricted * pTLVx11;
   ULONG outLenx11;
   rc = GetTLV( inLen, pIn, 0x11, &outLenx11, (const BYTE **)&pTLVx11 );
   if (rc == eGOBI_ERR_NONE)
   {
      if (outLenx11 < sizeof( sDMSGetOperatingModeResponse_PlatformRestricted ))
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      // Copy the value
      *pbPlatform = pTLVx11->mPlatformRestricted;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ParseGetNetworkTime

DESCRIPTION:
   This function returns the current time of the device

PARAMETERS:
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pTimeCount  [ O ] - Count of 1.25ms that have elapsed from the start
                       of GPS time (Jan 6, 1980)
   pTimeSource [ O ] - Source of the timestamp

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ParseGetNetworkTime(
   ULONG             inLen,
   const BYTE *      pIn,
   ULONGLONG *       pTimeCount,
   ULONG *           pTimeSource )
{
   // Validate arguments
   if (pIn == 0 || pTimeCount == 0 || pTimeSource == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Find the reason mask
   const sDMSGetTimestampResponse_Timestamp * pTLVx01;
   ULONG outLenx01;
   ULONG rc = GetTLV( inLen, pIn, 0x01, &outLenx01, (const BYTE **)&pTLVx01 );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   if (outLenx01 < sizeof( sDMSGetTimestampResponse_Timestamp ))
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }

   // Get the values
   *pTimeCount = pTLVx01->mTimestamp;
   // mSource is of type eQMIDMSTimestampSources
   *pTimeSource = pTLVx01->mSource;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackValidateSPC

DESCRIPTION:
   This function validates the service programming code

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied to pOut
   pOut        [ O ] - Output buffer
   pSPC        [ I ] - Six digit service programming code

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackValidateSPC(
   ULONG *           pOutLen,
   BYTE *            pOut,
   CHAR *            pSPC )
{
   // Validate arguments
   if (pOut == 0 || pSPC == 0 || pSPC[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   std::string spc( pSPC );
   if (spc.size() > 6)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (spc.find_first_not_of( "0123456789" ) != std::string::npos )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Check size
   WORD tlvx01Sz = sizeof( sDMSValidateSPCRequest_SPC );
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pOut);
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   // The SPC
   sDMSValidateSPCRequest_SPC * pTLVx01;
   pTLVx01 = (sDMSValidateSPCRequest_SPC*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the values
   memcpy( &pTLVx01->mSPC[0], spc.c_str(), spc.size() );

   offset += tlvx01Sz;
   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
