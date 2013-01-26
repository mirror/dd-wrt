/*===========================================================================
FILE:
   CRC.h

DESCRIPTION:
   16-bit LSB CRC computation/verification

PUBLIC CLASSES AND METHODS:
   SetCRC()
   CheckCRC()
   CalculateCRC()

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
const USHORT CRC_16_L_POLYNOMIAL = 0x8408;
const USHORT CRC_16_L_SEED       = 0xFFFF;
const USHORT CRC_16_L_OK         = 0x0F47;
const USHORT CRC_TABLE_SIZE      = 256;
const USHORT CRC_SIZE            = 2;

/*=========================================================================*/
// Prototypes
/*=========================================================================*/

// Calculate and append a 16-bit CRC to given data, the calculated CRC
// value stored at pData[dataLen] & pData[dataLen + 1]
void SetCRC( 
   PBYTE                      pData, 
   ULONG                      dataLen );

// Check a CRC value for the given data, dataLen includes the 2 byte CRC
// value at the end of the buffer
bool CheckCRC( 
   const BYTE *               pData, 
   ULONG                      dataLen );

// Calculate a CRC value for the given data
USHORT CalculateCRC( 
   const BYTE *               pBuf, 
   ULONG                      bitLen );

