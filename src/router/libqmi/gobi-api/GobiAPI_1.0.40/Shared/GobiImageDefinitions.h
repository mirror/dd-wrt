/*===========================================================================
FILE: 
   GobiImageDefinitions.h

DESCRIPTION:
   QUALCOMM Gobi Image related definitions

PUBLIC CLASSES AND FUNCTIONS:
   eGobiDeviceType
   eGobiMBNType
   eGobiImageTech
   eGobiImageCarrier
   eGobiImageRegion
   eGobiImageGPS

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

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// eGobiDeviceType Enumeration
//    Gobi Device Interface Enumeration
/*=========================================================================*/
enum eGobiDeviceType
{
   eGOBI_DEV_ENUM_BEGIN = -1, 

   eGOBI_DEV_NET,                // 0 - Network adapter
   eGOBI_DEV_NMEA,               // 1 - NMEA COM port
   eGOBI_DEV_DIAG,               // 2 - DIAG port
   eGOBI_DEV_MODEM,              // 3 - Modem
   eGOBI_DEV_AT,                 // 4 - AT port
   eGOBI_DEV_NET2,               // 5 - Auxiliary network adapter
   eGOBI_DEV_QDL,                // 6 - QDL port (should always be last)

   eGOBI_DEV_ENUM_END
};

/*=========================================================================*/
// eGobiMBNType Enumeration
//    Gobi MBN File Type Enumeration
/*=========================================================================*/
enum eGobiMBNType
{
   eGOBI_MBN_TYPE_ENUM_BEGIN = -1, 

   eGOBI_MBN_TYPE_MODEM,         // 0 - Modem/AMSS
   eGOBI_MBN_TYPE_PRI,           // 1 - PRI/UQCN

   eGOBI_MBN_TYPE_ENUM_END,
};

/*=========================================================================*/
// eGobiImageTech Enumeration
//    Gobi Image Technology Enumeration
/*=========================================================================*/
enum eGobiImageTech
{
   eGOBI_IMG_TECH_CDMA = 0,      // 0 - CDMA
   eGOBI_IMG_TECH_UMTS           // 1 - UMTS
};

/*=========================================================================*/
// eGobiImageCarrier Enumeration
//    Gobi Image Carrier Enumeration
/*=========================================================================*/
enum eGobiImageCarrier
{
   eGOBI_IMG_CAR_GENERIC = 1,    // 001
   eGOBI_IMG_CAR_FACTORY,        // 002
   eGOBI_IMG_CAR_NORF,           // 003

   eGOBI_IMG_CAR_VERIZON = 101,  // 101
   eGOBI_IMG_CAR_SPRINT,         // 102
   eGOBI_IMG_CAR_ALLTEL,         // 103
   eGOBI_IMG_CAR_BELL,           // 104
   eGOBI_IMG_CAR_TELUS,          // 105
   eGOBI_IMG_CAR_US,             // 106
   eGOBI_IMG_CAR_TELSTRA1,       // 107
   eGOBI_IMG_CAR_CHINA_UNICOM,   // 108
   eGOBI_IMG_CAR_TELCOM_NZ,      // 109
   eGOBI_IMG_CAR_SK_TELCOM1,     // 110
   eGOBI_IMG_CAR_RELIANCE1,      // 111
   eGOBI_IMG_CAR_TATA,           // 112
   eGOBI_IMG_CAR_METROPCS,       // 113
   eGOBI_IMG_CAR_LEAP,           // 114
   eGOBI_IMG_CAR_KDDI,           // 115
   eGOBI_IMG_CAR_IUSACELL,       // 116
   eGOBI_IMG_CAR_CHINA_TELECOM,  // 117
   eGOBI_IMG_CAR_OMH,            // 118

   eGOBI_IMG_CAR_ATT = 201,      // 201
   eGOBI_IMG_CAR_VODAFONE,       // 202
   eGOBI_IMG_CAR_TMOBILE,        // 203
   eGOBI_IMG_CAR_ORANGE,         // 204
   eGOBI_IMG_CAR_TELEFONICA,     // 205
   eGOBI_IMG_CAR_TELCOM_ITALIA,  // 206
   eGOBI_IMG_CAR_3,              // 207
   eGOBI_IMG_CAR_O2,             // 208
   eGOBI_IMG_CAR_SFR,            // 209
   eGOBI_IMG_CAR_SWISSCOM,       // 210
   eGOBI_IMG_CAR_CHINA_MOBILE,   // 211
   eGOBI_IMG_CAR_TELSTRA2,       // 212
   eGOBI_IMG_CAR_SINGTEL_OPTUS,  // 213
   eGOBI_IMG_CAR_RELIANCE2,      // 214
   eGOBI_IMG_CAR_BHARTI,         // 215
   eGOBI_IMG_CAR_NTT_DOCOMO,     // 216
   eGOBI_IMG_CAR_EMOBILE,        // 217
   eGOBI_IMG_CAR_SOFTBANK,       // 218
   eGOBI_IMG_CAR_KT_FREETEL,     // 219
   eGOBI_IMG_CAR_SK_TELCOM2,     // 220
   eGOBI_IMG_CAR_TELENOR,        // 221
   eGOBI_IMG_CAR_NETCOM,         // 222
   eGOBI_IMG_CAR_TELIASONERA,    // 223
   eGOBI_IMG_CAR_AMX_TELCEL,     // 224
   eGOBI_IMG_CAR_BRASIL_VIVO     // 225
};

/*=========================================================================*/
// eGobiImageRegion Enumeration
//    Gobi Image Region Enumeration
/*=========================================================================*/
enum eGobiImageRegion
{
   eGOBI_IMG_REG_NA = 0,         // 0 - North America
   eGOBI_IMG_REG_LA,             // 1 - Latin America
   eGOBI_IMG_REG_EU,             // 2 - Europe
   eGOBI_IMG_REG_ASIA,           // 3 - Asia
   eGOBI_IMG_REG_AUS,            // 4 - Australia
   eGOBI_IMG_REG_GLOBAL          // 5 - Global
};

/*=========================================================================*/
// eGobiImageGPS Enumeration
//    Gobi Image GPS Enumeration
/*=========================================================================*/
enum eGobiImageGPS
{
   eGOBI_IMG_GPS_NONE = 0,       // 0 - None
   eGOBI_IMG_GPS_STAND_ALONE,    // 1 - Stand-alone
   eGOBI_IMG_GPS_ASSISTED,       // 2 - Stand-alone + AGPS + XTRA
   eGOBI_IMG_GPS_NO_XTRA         // 3 - Stand-alone + AGPS
};
