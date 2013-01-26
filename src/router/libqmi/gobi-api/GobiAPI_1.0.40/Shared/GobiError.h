/*===========================================================================
FILE: 
   GobiError.h

DESCRIPTION:
   QUALCOMM Gobi Errors

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
// Include Files
//---------------------------------------------------------------------------
#include "QMIEnum.h"
#include "QDLEnum.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// eGobiError Enumeration
//    Gobi API Error Enumeration
/*=========================================================================*/
enum eGobiError
{
   eGOBI_ERR_ENUM_BEGIN = -1, 

   eGOBI_ERR_NONE,               // 00 Success
   eGOBI_ERR_GENERAL,            // 01 General error
   eGOBI_ERR_INTERNAL,           // 02 Internal error
   eGOBI_ERR_MEMORY,             // 03 Memory error
   eGOBI_ERR_INVALID_ARG,        // 04 Invalid argument
   eGOBI_ERR_BUFFER_SZ,          // 05 Buffer too small
   eGOBI_ERR_NO_DEVICE,          // 06 Unable to detect device
   eGOBI_ERR_INVALID_DEVID,      // 07 Invalid device ID
   eGOBI_ERR_NO_CONNECTION,      // 08 No connection to device
   eGOBI_ERR_IFACE,              // 09 Unable to obtain required interace
   eGOBI_ERR_CONNECT,            // 10 Unable to connect to interface
   eGOBI_ERR_REQ_SCHEDULE,       // 11 Unable to schedule request
   eGOBI_ERR_REQUEST,            // 12 Error sending request
   eGOBI_ERR_RESPONSE,           // 13 Error receiving response
   eGOBI_ERR_REQUEST_TO,         // 14 Timeout while sending request
   eGOBI_ERR_RESPONSE_TO,        // 15 Timeout while receiving response
   eGOBI_ERR_MALFORMED_RSP,      // 16 Malformed response received
   eGOBI_ERR_INVALID_RSP,        // 17 Invalid/error response received
   eGOBI_ERR_INVALID_FILE,       // 18 Invalid file path
   eGOBI_ERR_FILE_OPEN,          // 19 Unable to open file
   eGOBI_ERR_FILE_COPY,          // 20 Unable to copy file
   eGOBI_ERR_QDL_SCM,            // 21 Unable to open service control mgr
   eGOBI_ERR_NO_QDL_SVC,         // 22 Unable to detect QDL service
   eGOBI_ERR_NO_QDL_SVC_INFO,    // 23 Unable to obtain QDL service info
   eGOBI_ERR_NO_QDL_SVC_PATH,    // 24 Unable to locate QSL service 
   eGOBI_ERR_QDL_SVC_CFG,        // 25 Unable to reconfigure QDL service
   eGOBI_ERR_QDL_SVC_IFACE,      // 26 Unable to interface to QDL service
   eGOBI_ERR_OFFLINE,            // 27 Unable to set device offline
   eGOBI_ERR_RESET,              // 28 Unable to reset device
   eGOBI_ERR_NO_SIGNAL,          // 29 No available signal 
   eGOBI_ERR_MULTIPLE_DEVICES,   // 30 Multiple devices detected
   eGOBI_ERR_DRIVER,             // 31 Error interfacing to driver
   eGOBI_ERR_NO_CANCELABLE_OP,   // 32 No cancelable operation is pending
   eGOBI_ERR_CANCEL_OP,          // 33 Error canceling outstanding operation
   eGOBI_ERR_QDL_CRC,            // 34 QDL image data CRC error
   eGOBI_ERR_QDL_PARSING,        // 35 QDL image data parsing error
   eGOBI_ERR_QDL_AUTH,           // 36 QDL image authentication error
   eGOBI_ERR_QDL_WRITE,          // 37 QDL image write error
   eGOBI_ERR_QDL_OPEN_SIZE,      // 38 QDL image size error
   eGOBI_ERR_QDL_OPEN_TYPE,      // 39 QDL image type error
   eGOBI_ERR_QDL_OPEN_PROT,      // 40 QDL memory protection error
   eGOBI_ERR_QDL_OPEN_SKIP,      // 41 QDL image not required
   eGOBI_ERR_QDL_ERR_GENERAL,    // 42 QDL general error
   eGOBI_ERR_QDL_BAR_MODE,       // 43 QDL BAR mode error

   eGOBI_ERR_ENUM_END,

   // Offset from which mapped QMI error codes start from (see eQMIErrorCode)
   eGOBI_ERR_QMI_OFFSET = 1000,

   // Offset from which mapped QDL errors start from (see eQDLError)
   eGOBI_ERR_QDL_OFFSET = 100000
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eGobiError validity check

PARAMETERS:
   ec          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eGobiError ec )
{
   bool retVal = false;
   if (ec > eGOBI_ERR_ENUM_BEGIN && ec < eGOBI_ERR_ENUM_END)
   {
      retVal = true;
   }

   if (ec >= eGOBI_ERR_QMI_OFFSET && ec < eGOBI_ERR_QDL_OFFSET)
   {
      ULONG tmp = (ULONG)ec - (ULONG)eGOBI_ERR_QMI_OFFSET;
      retVal = ::IsValid( (eQMIErrorCode)tmp );
   }

   if (ec >= eGOBI_ERR_QDL_OFFSET)
   {
      ULONG tmp = (ULONG)ec - (ULONG)eGOBI_ERR_QDL_OFFSET;
      retVal = ::IsValid( (eQDLError)tmp );
   }

   return retVal;
};
