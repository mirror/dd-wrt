/*===========================================================================
FILE:
   HDLC.cpp

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

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "HDLC.h"
#include "CRC.h"
#include "SharedBuffer.h"
#include "ProtocolServer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/* Async HDLC defines */
const BYTE AHDLC_FLAG    = 0x7e;
const BYTE AHDLC_ESCAPE  = 0x7d;
const BYTE AHDLC_ESC_M   = 0x20;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   HDLCDecode (Free Method)

DESCRIPTION:
   HDLC decode the given buffer returning the results in an allocated buffer
  
PARAMETERS:
   pBuf        [ I ] - The data buffer to decode

RETURN VALUE:
   sSharedBuffer * : The decoded buffer (allocated), 0 on error
===========================================================================*/
sSharedBuffer * HDLCDecode( sSharedBuffer * pBuf )
{  
   // The return buffer
   sSharedBuffer * pRet = 0;

   // The has to be something to decode
   if (pBuf == 0 || pBuf->IsValid() == false)
   {
      return pRet;
   }

   // Grab raw data from shared buffer
   const BYTE * pData = pBuf->GetBuffer();
   UINT sz = pBuf->GetSize();

   // Is the first character a leading flag?
   if (pData[0] == AHDLC_FLAG)
   {
      pData++;
      sz--;
   }

   // There must be at least four bytes (data, CRC, trailing flag)
   if (sz < 4)
   {
      return pRet;
   }

   // The last character must be the trailing flag
   if (pData[sz - 1] == AHDLC_FLAG)
   {
      sz--;
   }
   else 
   {
      return pRet;      
   }

   // Allocate the decode buffer
   PBYTE pDecoded = new BYTE[sz];
   if (pDecoded == 0)
   {
      return pRet;
   }

   // Handle escaped characters and copy into decode buffer
   UINT encodeIndex = 0;
   UINT decodeIndex = 0;
   while (encodeIndex < sz)
   {
      BYTE b = pData[encodeIndex++];
      if (b == AHDLC_ESCAPE && encodeIndex < sz)
      {
         b = pData[encodeIndex++];
         b ^= AHDLC_ESC_M;
      }

      pDecoded[decodeIndex++] = b;
   }

   // Check CRC value
   if (CheckCRC( pDecoded, decodeIndex ) == false)
   {
      delete [] pDecoded;
      return pRet;
   }
      
   // Adjust decode length down for CRC
   decodeIndex -= 2;

   // ... and wrap up in a shared buffer
   pRet = new sSharedBuffer( decodeIndex, pDecoded, pBuf->GetType() );
   return pRet;
}

/*===========================================================================
METHOD:
   HDLCEncode (Free Method)

DESCRIPTION:
   HDLC encode the given buffer returning the results in an allocated buffer

PARAMETERS:
   pBuf        [ I ] - The data buffer to decode

RETURN VALUE:
   sSharedBuffer * : The decoded buffer (allocated), 0 on error
===========================================================================*/
sSharedBuffer * HDLCEncode( sSharedBuffer * pBuf )
{
   // The return buffer
   sSharedBuffer * pRet = 0;

   // The has to be something to decode
   if (pBuf == 0 || pBuf->IsValid() == false)
   {
      return pRet;
   }

   // Grab raw data from shared buffer
   const BYTE * pData = pBuf->GetBuffer();
   UINT sz = pBuf->GetSize();

   // Compute CRC
   USHORT CRC = CalculateCRC( pData, sz * 8 );

   // Allocate the encode buffer
   PBYTE pEncoded = new BYTE[sz * 2 + 4];
   if (pEncoded == 0)
   {
      return pRet;
   }

   // Add leading flag
   UINT encodeIndex = 0; 
   pEncoded[encodeIndex++] = AHDLC_FLAG;

   // Add data, escaping when necessary
   UINT decodeIndex = 0;
   while (decodeIndex < sz)
   {
      BYTE value = pData[decodeIndex++];  
      if (value == AHDLC_FLAG || value == AHDLC_ESCAPE)
      {
         value ^= AHDLC_ESC_M;
         pEncoded[encodeIndex++] = AHDLC_ESCAPE;
      }

      pEncoded[encodeIndex++] = value;
   }

   // Byte order CRC
   BYTE byteOrderedCRC[2];
   byteOrderedCRC[0] = (BYTE)(CRC & 0x00ff);
   byteOrderedCRC[1] = (BYTE)(CRC >> 8);

   // Add CRC
   UINT c = 0;
   while (c < 2)
   {
      BYTE value = byteOrderedCRC[c++];  
      if (value == AHDLC_FLAG || value == AHDLC_ESCAPE)
      {
         value ^= AHDLC_ESC_M;
         pEncoded[encodeIndex++] = AHDLC_ESCAPE;
      }

      pEncoded[encodeIndex++] = value;
   }

   // Add trailing flag
   pEncoded[encodeIndex++] = AHDLC_FLAG;

   // Wrap up in a shared buffer
   pRet = new sSharedBuffer( encodeIndex, pEncoded, pBuf->GetType() );
   return pRet;
}

/*===========================================================================
METHOD:
   HDLCUnitTest (Free Method)

DESCRIPTION:
   Simple in = out testing of HDLCEncode/HDLCDecode

RETURN VALUE:
   bool: 
      true  - Test succeeded
      false - Test failed
===========================================================================*/
#ifdef DEBUG
#include <cstdlib>

bool HDLCUnitTest()
{
   // Assume failure
   bool bRC = false;

   const UINT MAX_LEN = 2048;
   BYTE testBuf[MAX_LEN];

   srand( GetTickCount() );

   UINT len = (((UINT)rand()) % MAX_LEN) + 1;
   for (UINT i = 0; i < len; i++)
   {
      testBuf[i] = (BYTE)((UINT)rand() % 256);
   }

   sSharedBuffer * pOrig = new sSharedBuffer( testBuf, len, 0 );
   if (pOrig != 0)
   {
      // Encode buffer
      sSharedBuffer * pEnc = HDLCEncode( pOrig );
      if (pEnc != 0)
      {
         // Now decode buffer encoded above
         sSharedBuffer * pDec = HDLCDecode( pEnc );
         if (pDec != 0) 
         {
            if (pOrig->IsValid() == true && pDec->IsValid() == true)
            {
               // Compare decoded to original
               const BYTE * pOrigData = pOrig->GetBuffer();
               const BYTE * pDecData = pDec->GetBuffer();

               if (len == pDec->GetSize())
               {
                  int cmp = memcmp( (const void *)pOrigData,
                                    (const void *)pDecData,
                                    (size_t)len );

                  bRC = (cmp == 0);
               }
            }

            delete [] pDec;
         }

         delete [] pEnc;
      }

      delete [] pOrig;
   }

   return bRC;
}

#endif
