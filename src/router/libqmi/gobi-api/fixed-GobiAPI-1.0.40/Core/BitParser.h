/*===========================================================================
FILE:
   BitParser.h

DESCRIPTION:
   Declaration of cBitParser class
   
PUBLIC CLASSES AND METHODS:
   cBitParser
      This class extracts bits from a buffer

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

// Number of bits in a byte
const ULONG BITS_PER_BYTE = 8;

// Maximum number of bits we will parse for any supported type
const ULONG MAX_TYPE_BITS = (ULONG)sizeof(ULONGLONG) * BITS_PER_BYTE;

/*===========================================================================
METHOD:
   ByteSwap (Inline Public Method)

DESCRIPTION:
   Changes little-endian values to big-endian, and vice versa

PARAMETERS:
   data        [ I ] - Data being byte-swapped
  
RETURN VALUE:
   None
===========================================================================*/
template <class T>
void ByteSwap( T & data )
{
   // Just reverse the order of the bytes
   PBYTE pL;
   PBYTE pR;

   for (pL = (PBYTE)&data, pR = pL + sizeof( T ) - 1; pL < pR; ++pL, --pR)
   {
      *pL = *pL ^ *pR;
      *pR = *pL ^ *pR;
      *pL = *pL ^ *pR;
   }
};

/*=========================================================================*/
// Class cBitParser
//
//    Class to assist in parsing a buffer into bit/byte specified fields
/*=========================================================================*/
class cBitParser
{
   public:
      // Constructor (default)
      cBitParser();

      // Constructor (from a  buffer)
      cBitParser(
         const BYTE *               pData,
         ULONG                      dataBitSize );

      // Destructor
      ~cBitParser();

      // (Inline) Returns the number of bits left in the buffer (from the 
      // current working bit offset)
      ULONG GetNumBitsLeft() const
      {
         return (mMaxOffset - mOffset);
      };

      // (Inline) Returns the number of bits in the buffer that have been 
      // processed (essentially the current working bit offset)
      ULONG GetNumBitsParsed() const
      {
         return (mOffset);
      };

      // (Inline) Set current working bit offset
      void SetOffset( ULONG offset )
      {
         mOffset = offset;
         if (mOffset > mMaxOffset)
         {
            mOffset = mMaxOffset;
         }
      };

      // (Inline) Are we parsing LSB -> MSB (the default)?
      bool GetLSBMode()
      {
         return mbLSB;
      };

      // (Inline) Parse LSB -> MSB (if true) or MSB -> LSB
      void SetLSBMode( bool bLSB )
      {
         mbLSB = bLSB;
      };

      // Return 'numBits' as a CHAR (advances offset)
      DWORD Get(
         ULONG                      numBits,  
         CHAR &                     dataOut );

      // Return 'numBits' as a SHORT (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         SHORT &                    dataOut );

      // Return 'numBits' as a LONG (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         LONG &                     dataOut );

      // Return 'numBits' as a LONGLONG (advances offset)
      DWORD Get(
         ULONG                      numBits,  
         LONGLONG &                 dataOut );

      // Return 'numBits' as a UCHAR (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         UCHAR &                    dataOut );

      // Return 'numBits' as a USHORT (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         USHORT &                   dataOut );

      // Return 'numBits' as a ULONG (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         ULONG &                    dataOut );

      // Return 'numBits' as a ULONGLONG (advances offset)
      DWORD Get(
         ULONG                      numBits, 
         ULONGLONG &                dataOut );

      // Release the data being parsed
      void ReleaseData();

      // Set the data being parsed
      void SetData(
         const BYTE *               pData,
         ULONG                      dataBitSize );

   protected:
      /* Data buffer */
      const BYTE * mpData;

      /* Current bit-specified offset */
      ULONG mOffset;

      /* Maximum bit-specified offset */
      ULONG mMaxOffset;

      /* Are we parsing LSB -> MSB (the default)? */
      bool mbLSB;
};
