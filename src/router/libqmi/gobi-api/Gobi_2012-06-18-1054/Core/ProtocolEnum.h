/*===========================================================================
FILE:
   ProtocolEnum.h

DESCRIPTION:
   Generic protocol enumerations and related methods

PUBLIC ENUMERATIONS AND METHODS:
   eProtocolType

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
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// eProtocolType Enumeration
//
// NOTE: QMI protocol types need to be in the same order as eQMIService
// with RX added first then TX
/*=========================================================================*/
enum eProtocolType
{
   ePROTOCOL_ENUM_BEGIN = -1, 

   ePROTOCOL_COMMAND,            // 000 Protocol server command
   ePROTOCOL_AT,                 // 001 AT command protocol
   ePROTOCOL_NMEA,               // 002 NMEA (GPS) protocol
   ePROTOCOL_DIAG_RX,            // 003 DIAG protocol (incoming)
   ePROTOCOL_DIAG_TX,            // 004 DIAG protocol (outgoing)
   ePROTOCOL_DOWNLOAD_RX,        // 005 Download protocol (incoming)
   ePROTOCOL_DOWNLOAD_TX,        // 006 Download protocol (outgoing)
   ePROTOCOL_SDOWNLOAD_RX,       // 007 Streaming download protocol (incoming)
   ePROTOCOL_SDOWNLOAD_TX,       // 008 Streaming download protocol (outgoing)
   ePROTOCOL_QDL_RX,             // 009 QDL streaming protocol (incoming)
   ePROTOCOL_QDL_TX,             // 010 QDL streaming protocol (outgoing)

   ePROTOCOL_QMI_CTL_RX = 60,    // 060 QMI CTL protocol (incoming)
   ePROTOCOL_QMI_CTL_TX,         // 061 QMI CTL protocol (outgoing)
   ePROTOCOL_QMI_WDS_RX,         // 062 QMI WDS protocol (incoming)
   ePROTOCOL_QMI_WDS_TX,         // 063 QMI WDS protocol (outgoing)
   ePROTOCOL_QMI_DMS_RX,         // 064 QMI DMS protocol (incoming)
   ePROTOCOL_QMI_DMS_TX,         // 065 QMI DMS protocol (outgoing)
   ePROTOCOL_QMI_NAS_RX,         // 066 QMI NAS protocol (incoming)
   ePROTOCOL_QMI_NAS_TX,         // 067 QMI NAS protocol (outgoing)
   ePROTOCOL_QMI_QOS_RX,         // 068 QMI QOS protocol (incoming)
   ePROTOCOL_QMI_QOS_TX,         // 069 QMI QOS protocol (outgoing)
   ePROTOCOL_QMI_WMS_RX,         // 070 QMI WMS protocol (incoming)
   ePROTOCOL_QMI_WMS_TX,         // 071 QMI WMS protocol (outgoing)
   ePROTOCOL_QMI_PDS_RX,         // 072 QMI PDS protocol (incoming)
   ePROTOCOL_QMI_PDS_TX,         // 073 QMI PDS protocol (outgoing)
   ePROTOCOL_QMI_AUTH_RX,        // 074 QMI AUTH protocol (incoming)
   ePROTOCOL_QMI_AUTH_TX,        // 075 QMI AUTH protocol (outgoing)
   ePROTOCOL_QMI_AT_RX,          // 076 QMI AUTH protocol (incoming)
   ePROTOCOL_QMI_AT_TX,          // 077 QMI AUTH protocol (outgoing)
   ePROTOCOL_QMI_VOICE_RX,       // 078 QMI Voice protocol (incoming)
   ePROTOCOL_QMI_VOICE_TX,       // 079 QMI Voice protocol (outgoing)
   ePROTOCOL_QMI_CAT2_RX,        // 080 QMI CAT (new) protocol (incoming)
   ePROTOCOL_QMI_CAT2_TX,        // 081 QMI CAT (new) protocol (outgoing)
   ePROTOCOL_QMI_UIM_RX,         // 082 QMI UIM protocol (incoming)
   ePROTOCOL_QMI_UIM_TX,         // 083 QMI UIM protocol (outgoing)
   ePROTOCOL_QMI_PBM_RX,         // 084 QMI PBM protocol (incoming)
   ePROTOCOL_QMI_PBM_TX,         // 085 QMI PBM protocol (outgoing)
   ePROTOCOL_QMI_13_RX,          // 086 QMI service ID 13 protocol (incoming)
   ePROTOCOL_QMI_13_TX,          // 087 QMI service ID 13 protocol (outgoing)
   ePROTOCOL_QMI_RMTFS_RX,       // 088 QMI RMTFS protocol (incoming)
   ePROTOCOL_QMI_RMTFS_TX,       // 089 QMI RMTFS protocol (outgoing)
   ePROTOCOL_QMI_15_RX,          // 090 QMI service ID 15 protocol (incoming)
   ePROTOCOL_QMI_15_TX,          // 091 QMI service ID 15 protocol (outgoing)
   ePROTOCOL_QMI_LOC_RX,         // 092 QMI UIM protocol (incoming)
   ePROTOCOL_QMI_LOC_TX,         // 093 QMI UIM protocol (outgoing)
   ePROTOCOL_QMI_SAR_RX,         // 094 QMI PBM protocol (incoming)
   ePROTOCOL_QMI_SAR_TX,         // 095 QMI PBM protocol (outgoing)
   ePROTOCOL_QMI_18_RX,          // 096 QMI service ID 18 protocol (incoming)
   ePROTOCOL_QMI_18_TX,          // 097 QMI service ID 18 protocol (outgoing)
   ePROTOCOL_QMI_19_RX,          // 098 QMI service ID 19 protocol (incoming)
   ePROTOCOL_QMI_19_TX,          // 099 QMI service ID 19 protocol (outgoing)
   ePROTOCOL_QMI_CSD_RX,         // 100 QMI CSD protocol (incoming)
   ePROTOCOL_QMI_CSD_TX,         // 101 QMI CSD protocol (outgoing)
   ePROTOCOL_QMI_EFS_RX,         // 102 QMI EFS protocol (incoming)
   ePROTOCOL_QMI_EFS_TX,         // 103 QMI EFS protocol (outgoing)
   ePROTOCOL_QMI_22_RX,          // 104 QMI service ID 22 protocol (incoming)
   ePROTOCOL_QMI_22_TX,          // 105 QMI service ID 22 protocol (outgoing)
   ePROTOCOL_QMI_TS_RX,          // 106 QMI TS protocol (incoming)
   ePROTOCOL_QMI_TS_TX,          // 107 QMI TS protocol (outgoing)
   ePROTOCOL_QMI_TMD_RX,         // 108 QMI TMD protocol (incoming)
   ePROTOCOL_QMI_TMD_TX,         // 109 QMI TMD protocol (outgoing)
   ePROTOCOL_QMI_25_RX,          // 110 QMI service ID 25 protocol (incoming)
   ePROTOCOL_QMI_25_TX,          // 111 QMI service ID 25 protocol (outgoing)
   ePROTOCOL_QMI_26_RX,          // 112 QMI service ID 26 protocol (incoming)
   ePROTOCOL_QMI_26_TX,          // 113 QMI service ID 26 protocol (outgoing)
   ePROTOCOL_QMI_27_RX,          // 114 QMI service ID 27 protocol (incoming)
   ePROTOCOL_QMI_27_TX,          // 115 QMI service ID 27 protocol (outgoing)
   ePROTOCOL_QMI_28_RX,          // 116 QMI service ID 28 protocol (incoming)
   ePROTOCOL_QMI_28_TX,          // 117 QMI service ID 28 protocol (outgoing)

   ePROTOCOL_QMI_CAT_RX = 508,   // 508 QMI CAT protocol (incoming)
   ePROTOCOL_QMI_CAT_TX,         // 509 QMI CAT protocol (outgoing)
   ePROTOCOL_QMI_RMS_RX,         // 510 QMI RMS protocol (incoming)
   ePROTOCOL_QMI_RMS_TX,         // 511 QMI RMS protocol (outgoing)
   ePROTOCOL_QMI_OMA_RX,         // 512 QMI OMA protocol (incoming)
   ePROTOCOL_QMI_OMA_TX,         // 513 QMI OMA protocol (outgoing)

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
   if ( (pt > ePROTOCOL_ENUM_BEGIN && pt <= ePROTOCOL_QDL_TX)
   ||   (pt >= ePROTOCOL_QMI_CTL_RX && pt <= ePROTOCOL_QMI_28_TX)
   ||   (pt >= ePROTOCOL_QMI_CAT_RX && pt < ePROTOCOL_ENUM_END) )
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
   if ( (pt >= ePROTOCOL_QMI_CTL_RX && pt <= ePROTOCOL_QMI_28_TX)
   ||   (pt >= ePROTOCOL_QMI_CAT_RX && pt < ePROTOCOL_ENUM_END) )
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

   // QMI protocol values that are even are RX
   if ( (IsQMIProtocol( pt ) == true)
   &&   ((DWORD)pt % 2 == 0) )
   {
      retVal = true;
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

   // QMI protocol values that are odd are TX
   if ( (IsQMIProtocol( pt ) == true)
   &&   ((DWORD)pt % 2 == 1) )
   {
      retVal = true;
   }

   return retVal;
};
