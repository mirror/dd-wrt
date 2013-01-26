/*===========================================================================
FILE:
   HDLC.h

DESCRIPTION:
   Encode and decode asynchronous HDLC protocol packets as described 
   by both the QUALCOMM download & SDIC (diagnostic) protocol documents

PUBLIC CLASSES AND METHODS:
   HDLCDecode()
   HDLCEncode()

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
extern const BYTE AHDLC_FLAG;
extern const BYTE AHDLC_ESCAPE;
extern const BYTE AHDLC_ESC_M;

struct sSharedBuffer;

/*=========================================================================*/
// Prototypes
/*=========================================================================*/

// HDLC encode the given buffer returning the results in an allocated buffer
sSharedBuffer * HDLCEncode( sSharedBuffer * pBuf );

// HDLC decode the given buffer returning the results in an allocated buffer
sSharedBuffer * HDLCDecode( sSharedBuffer * pBuf );

#ifdef DEBUG

// Simple in = out testing of HDLCEncode/HDLCDecode
bool HDLCUnitTest();

#endif
