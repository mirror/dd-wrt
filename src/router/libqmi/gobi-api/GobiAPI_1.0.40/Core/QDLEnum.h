/*===========================================================================
FILE:
   QDLEnum.h

DESCRIPTION:
   QDL protocol enumerations and related methods

PUBLIC ENUMERATIONS AND METHODS:
   eQDLCommand
   eQDLError
   eQDLImageType
   eQDLOpenStatus
   eQDLWriteStatus
   eQDLDoneStatus

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

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// QDL feature bits
const BYTE QDL_FEATURE_GENERIC_UNFRAMED = 0x10;
const BYTE QDL_FEATURE_QDL_UNFRAMED     = 0x20;
const BYTE QDL_FEATURE_BAR_MODE         = 0x40;

// QDL protocol version
const BYTE QDL_MIN_VERSION = 6;
const BYTE QDL_MAX_VERSION = 6;

const BYTE QDL_HELLO_MAGIC_REQ[32] = 
{
   'Q',
   'C',
   'O',
   'M',
   ' ',
   'h',
   'i',
   'g',
   'h',
   ' ',
   's',
   'p',
   'e',
   'e',
   'd',
   ' ',
   'p',
   'r',
   'o',
   't',
   'o',
   'c',
   'o',
   'l',
   ' ',
   'h',
   's',
   't',
   0,
   0,
   0,
   0
};

const BYTE QDL_HELLO_MAGIC_RSP[24] =
{
   'Q',
   'C',
   'O',
   'M',
   ' ',
   'h',
   'i',
   'g',
   'h',
   ' ',
   's',
   'p',
   'e',
   'e',
   'd',
   ' ',
   'p',
   'r',
   'o',
   't',
   'o',
   'c',
   'o',
   'l'
};

// QDL maximum chunk size we support
const ULONG QDL_MAX_CHUNK_SIZE = 1024 * 1024 * 64;

/*=========================================================================*/
// eQDLCommand Enumeration
//    QDL Command Code Enumeration
/*=========================================================================*/
enum eQDLCommand
{
   eQDL_CMD_ENUM_BEGIN = -1, 

   eQDL_CMD_HELLO_REQ = 1,          // 001 Hello request
   eQDL_CMD_HELLO_RSP,              // 002 Hello response

   eQDL_CMD_ERROR = 13,             // 013 Error report

   eQDL_CMD_OPEN_UNFRAMED_REQ = 37, // 037 Open unframed image write request
   eQDL_CMD_OPEN_UNFRAMED_RSP,      // 038 Open unframed image write response
   eQDL_CMD_WRITE_UNFRAMED_REQ,     // 039 Unframed image write request
   eQDL_CMD_WRITE_UNFRAMED_RSP,     // 040 Unframed image write response
   eQDL_CMD_SESSION_DONE_REQ,       // 041 Unframed session done request
   eQDL_CMD_SESSION_DONE_RSP,       // 042 Unframed session done response
   eQDL_CMD_DOWNLOAD_REQ,           // 043 Switch to download protocol request 
  
   eQDL_CMD_SESSION_CLOSE_REQ = 45, // 045 Close unframed session request
   eQDL_CMD_GET_IMAGE_PREF_REQ,     // 046 Get image preference request
   eQDL_CMD_GET_IMAGE_PREF_RSP,     // 047 Get image preference response

   eQDL_CMD_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLCommand validity check

PARAMETERS:
   cmd         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLCommand cmd )
{
   bool retVal = false;

   switch (cmd)
   {
      case eQDL_CMD_HELLO_REQ:
      case eQDL_CMD_HELLO_RSP:
      case eQDL_CMD_ERROR:
      case eQDL_CMD_OPEN_UNFRAMED_REQ:
      case eQDL_CMD_OPEN_UNFRAMED_RSP:
      case eQDL_CMD_WRITE_UNFRAMED_REQ:
      case eQDL_CMD_WRITE_UNFRAMED_RSP:
      case eQDL_CMD_SESSION_DONE_REQ:
      case eQDL_CMD_SESSION_DONE_RSP:
      case eQDL_CMD_DOWNLOAD_REQ:
      case eQDL_CMD_SESSION_CLOSE_REQ:
      case eQDL_CMD_GET_IMAGE_PREF_REQ:
      case eQDL_CMD_GET_IMAGE_PREF_RSP:
         retVal = true;
         break;
   }

   return retVal;
};

/*=========================================================================*/
// eQDLError Enumeration
//    QDL Error Enumeration
/*=========================================================================*/
enum eQDLError
{
   eQDL_ERROR_ENUM_BEGIN = 0, 

   eQDL_ERROR_01,                // 01 Reserved
   eQDL_ERROR_BAD_ADDR,          // 02 Invalid destination address
   eQDL_ERROR_BAD_LEN,           // 03 Invalid length
   eQDL_ERROR_BAD_PACKET,        // 04 Unexpected end of packet
   eQDL_ERROR_BAD_CMD,           // 05 Invalid command
   eQDL_ERROR_06,                // 06 Reserved
   eQDL_ERROR_OP_FAILED,         // 07 Operation failed
   eQDL_ERROR_BAD_FLASH_ID,      // 08 Invalid flash intelligent ID
   eQDL_ERROR_BAD_VOLTAGE,       // 09 Invalid programming voltage
   eQDL_ERROR_WRITE_FAILED,      // 10 Write verify failed
   eQDL_ERROR_11,                // 11 Reserved
   eQDL_ERROR_BAD_SPC,           // 12 Invalid security code
   eQDL_ERROR_POWERDOWN,         // 13 Power-down failed
   eQDL_ERROR_UNSUPPORTED,       // 14 NAND flash programming not supported
   eQDL_ERROR_CMD_SEQ,           // 15 Command out of sequence
   eQDL_ERROR_CLOSE,             // 16 Close failed
   eQDL_ERROR_BAD_FEATURES,      // 17 Invalid feature bits
   eQDL_ERROR_SPACE,             // 18 Out of space
   eQDL_ERROR_BAD_SECURITY,      // 19 Invalid security mode
   eQDL_ERROR_MULTI_UNSUPPORTED, // 20 Multi-image NAND not supported
   eQDL_ERROR_POWEROFF,          // 21 Power-off command not supported
   eQDL_ERROR_CMD_UNSUPPORTED,   // 22 Command not supported
   eQDL_ERROR_BAD_CRC,           // 23 Invalid CRC
   eQDL_ERROR_STATE,             // 24 Command received in invalid state
   eQDL_ERROR_TIMEOUT,           // 25 Receive timeout
   eQDL_ERROR_IMAGE_AUTH,        // 26 Image authentication error

   eQDL_ERROR_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLError validity check

PARAMETERS:
   err         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLError err )
{
   bool retVal = false;
   if (err > eQDL_ERROR_ENUM_BEGIN && err < eQDL_ERROR_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQDLImageType Enumeration
//    QDL Download Image Type Enumeration
/*=========================================================================*/
enum eQDLImageType
{
   eQDL_IMAGE_ENUM_BEGIN = -1, 

   eQDL_IMAGE_AMSS_MODEM = 5,         // 05 AMSS modem image
   eQDL_IMAGE_AMSS_APPLICATION,       // 06 AMSS application image

   eQDL_IMAGE_AMSS_UQCN = 13,         // 13 Provisioning information

   eQDL_IMAGE_DBL = 15,               // 15 DBL image
   eQDL_IMAGE_OSBL,                   // 16 OSBL image

   eQDL_IMAGE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLImageType validity check

PARAMETERS:
   it          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLImageType it )
{
   bool retVal = false;

   switch (it)
   {
      case eQDL_IMAGE_AMSS_MODEM:
      case eQDL_IMAGE_AMSS_APPLICATION:
      case eQDL_IMAGE_AMSS_UQCN:
      case eQDL_IMAGE_DBL:
      case eQDL_IMAGE_OSBL:
         retVal = true;
         break;
   }

   return retVal;
};

/*=========================================================================*/
// eQDLOpenStatus Enumeration
//    QDL Unframed Open Status Enumeration
/*=========================================================================*/
enum eQDLOpenStatus
{
   eQDL_OPEN_STATUS_ENUM_BEGIN = -1, 

   eQDL_OPEN_STATUS_SUCCESS,     // 00 Success
   eQDL_OPEN_STATUS_SIZE,        // 01 Reported image size error
   eQDL_OPEN_STATUS_BAD_TYPE,    // 02 Invalid image type for downloader 
   eQDL_OPEN_STATUS_HDR_SIZE,    // 03 Reported image header size error
   eQDL_OPEN_STATUS_HDR1,        // 04 Image header incorrectly present
   eQDL_OPEN_STATUS_HDR2,        // 05 Image header required
   eQDL_OPEN_STATUS_PROTECTION,  // 06 Memory block protection error
   eQDL_OPEN_STATUS_NOT_NEEDED,  // 07 Image type not required

   eQDL_OPEN_STATUS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLOpenStatus validity check

PARAMETERS:
   os          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLOpenStatus os )
{
   bool retVal = false;
   if (os > eQDL_OPEN_STATUS_ENUM_BEGIN && os < eQDL_OPEN_STATUS_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQDLWriteStatus Enumeration
//    QDL Unframed Write Status Enumeration
/*=========================================================================*/
enum eQDLWriteStatus
{
   eQDL_WRITE_STATUS_ENUM_BEGIN = -1, 

   eQDL_WRITE_STATUS_SUCCESS,    // 00 Success
   eQDL_WRITE_STATUS_CRC,        // 01 Error with CRC
   eQDL_WRITE_STATUS_CONTENT,    // 02 Error with chunk content 

   eQDL_WRITE_STATUS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLWriteStatus validity check

PARAMETERS:
   ws          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLWriteStatus ws )
{
   bool retVal = false;
   if (ws > eQDL_WRITE_STATUS_ENUM_BEGIN && ws < eQDL_WRITE_STATUS_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQDLDoneStatus Enumeration
//    QDL Done Status Enumeration
/*=========================================================================*/
enum eQDLDoneStatus
{
   eQDL_DONE_STATUS_ENUM_BEGIN = -1, 

   eQDL_DONE_STATUS_SUCCESS,     // 00 Success
   eQDL_DONE_STATUS_AUTH,        // 01 Authentication failure
   eQDL_DONE_STATUS_WRITE,       // 02 Write failure

   eQDL_DONE_STATUS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQDLDoneStatus validity check

PARAMETERS:
   ds          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQDLDoneStatus ds )
{
   bool retVal = false;
   if (ds > eQDL_DONE_STATUS_ENUM_BEGIN && ds < eQDL_DONE_STATUS_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};
