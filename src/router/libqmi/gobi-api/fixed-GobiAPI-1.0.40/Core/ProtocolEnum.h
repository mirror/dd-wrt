/*===========================================================================
FILE:
   ProtocolEnum.h

DESCRIPTION:
   Generic protocol enumerations and related methods

PUBLIC ENUMERATIONS AND METHODS:
   eProtocolType

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
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// eProtocolType Enumeration
/*=========================================================================*/
enum eProtocolType
{
   ePROTOCOL_ENUM_BEGIN = -1, 

   ePROTOCOL_COMMAND,      // 00 Protocol server command
   ePROTOCOL_AT,           // 01 AT command protocol
   ePROTOCOL_NMEA,         // 02 NMEA (GPS) protocol
   ePROTOCOL_DIAG_RX,      // 03 DIAG protocol (incoming)
   ePROTOCOL_DIAG_TX,      // 04 DIAG protocol (outgoing)
   ePROTOCOL_DOWNLOAD_RX,  // 05 Download protocol (incoming)
   ePROTOCOL_DOWNLOAD_TX,  // 06 Download protocol (outgoing)
   ePROTOCOL_SDOWNLOAD_RX, // 07 Streaming download protocol (incoming)
   ePROTOCOL_SDOWNLOAD_TX, // 08 Streaming download protocol (outgoing)
   ePROTOCOL_QDL_RX,       // 09 QDL variant of streaming protocol (incoming)
   ePROTOCOL_QDL_TX,       // 10 QDL variant of streaming protocol (outgoing)
   ePROTOCOL_QMI_CTL_RX,   // 11 QMI CTL protocol (incoming)
   ePROTOCOL_QMI_CTL_TX,   // 12 QMI CTL protocol (outgoing)
   ePROTOCOL_QMI_WDS_RX,   // 13 QMI WDS protocol (incoming)
   ePROTOCOL_QMI_WDS_TX,   // 14 QMI WDS protocol (outgoing)
   ePROTOCOL_QMI_DMS_RX,   // 15 QMI DMS protocol (incoming)
   ePROTOCOL_QMI_DMS_TX,   // 16 QMI DMS protocol (outgoing)
   ePROTOCOL_QMI_NAS_RX,   // 17 QMI NAS protocol (incoming)
   ePROTOCOL_QMI_NAS_TX,   // 18 QMI NAS protocol (outgoing)
   ePROTOCOL_QMI_QOS_RX,   // 19 QMI QOS protocol (incoming)
   ePROTOCOL_QMI_QOS_TX,   // 20 QMI QOS protocol (outgoing)
   ePROTOCOL_QMI_WMS_RX,   // 21 QMI WMS protocol (incoming)
   ePROTOCOL_QMI_WMS_TX,   // 22 QMI WMS protocol (outgoing)
   ePROTOCOL_QMI_PDS_RX,   // 23 QMI PDS protocol (incoming)
   ePROTOCOL_QMI_PDS_TX,   // 24 QMI PDS protocol (outgoing)
   ePROTOCOL_QMI_AUTH_RX,  // 25 QMI AUTH protocol (incoming)
   ePROTOCOL_QMI_AUTH_TX,  // 26 QMI AUTH protocol (outgoing)
   ePROTOCOL_QMI_CAT_RX,   // 27 QMI CAT protocol (incoming)
   ePROTOCOL_QMI_CAT_TX,   // 28 QMI CAT protocol (outgoing)
   ePROTOCOL_QMI_RMS_RX,   // 29 QMI RMS protocol (incoming)
   ePROTOCOL_QMI_RMS_TX,   // 30 QMI RMS protocol (outgoing)
   ePROTOCOL_QMI_OMA_RX,   // 31 QMI OMA protocol (incoming)
   ePROTOCOL_QMI_OMA_TX,   // 32 QMI OMA protocol (outgoing)
   ePROTOCOL_QMI_VOICE_RX, // 33 QMI Voice protocol (incoming)
   ePROTOCOL_QMI_VOICE_TX, // 34 QMI Voice protocol (outgoing)

   ePROTOCOL_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eProtocolType validity check

PARAMETERS:
   pt          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eProtocolType pt )
{
   bool retVal = false;
   if (pt > ePROTOCOL_ENUM_BEGIN && pt < ePROTOCOL_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIProtocol (Inline Method)

DESCRIPTION:
   Does the passed in value represent a QMI protocol?

PARAMETERS:
   pt          [ I ] - Enum value being checked

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIProtocol( eProtocolType pt )
{
   bool retVal = false;
   if (pt >= ePROTOCOL_QMI_CTL_RX && pt <= ePROTOCOL_QMI_VOICE_TX)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIProtocolRX (Inline Method)

DESCRIPTION:
   Does the passed in value represent a QMI protocol and if so in the
   incoming direction?

PARAMETERS:
   pt          [ I ] - Enum value being checked

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIProtocolRX( eProtocolType pt )
{
   bool retVal = false;

   switch (pt)
   {
      case ePROTOCOL_QMI_CTL_RX:
      case ePROTOCOL_QMI_WDS_RX:
      case ePROTOCOL_QMI_DMS_RX:
      case ePROTOCOL_QMI_NAS_RX:
      case ePROTOCOL_QMI_QOS_RX:
      case ePROTOCOL_QMI_WMS_RX:
      case ePROTOCOL_QMI_PDS_RX:
      case ePROTOCOL_QMI_AUTH_RX:
      case ePROTOCOL_QMI_CAT_RX:
      case ePROTOCOL_QMI_RMS_RX:
      case ePROTOCOL_QMI_OMA_RX:
      case ePROTOCOL_QMI_VOICE_RX:
         retVal = true;
         break;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIProtocolTX (Inline Method)

DESCRIPTION:
   Does the passed in value represent a QMI protocol and if so in the
   outgoing direction?

PARAMETERS:
   pt          [ I ] - Enum value being checked

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIProtocolTX( eProtocolType pt )
{
   bool retVal = false;

   switch (pt)
   {
      case ePROTOCOL_QMI_CTL_TX:
      case ePROTOCOL_QMI_WDS_TX:
      case ePROTOCOL_QMI_DMS_TX:
      case ePROTOCOL_QMI_NAS_TX:
      case ePROTOCOL_QMI_QOS_TX:
      case ePROTOCOL_QMI_WMS_TX:
      case ePROTOCOL_QMI_PDS_TX:
      case ePROTOCOL_QMI_AUTH_TX:
      case ePROTOCOL_QMI_CAT_TX:
      case ePROTOCOL_QMI_RMS_TX:
      case ePROTOCOL_QMI_OMA_TX:
      case ePROTOCOL_QMI_VOICE_TX:
         retVal = true;
         break;
   }

   return retVal;
};
