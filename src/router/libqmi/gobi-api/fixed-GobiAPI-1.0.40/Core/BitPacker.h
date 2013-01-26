/*===========================================================================
FILE:
   BitPacker.h

DESCRIPTION:
   Declaration of cBitPacker class
   
PUBLIC CLASSES AND METHODS:
   cBitPacker
      This class packs bits into a buffer

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
#include "BitParser.h"


/*=========================================================================*/
// Class cBitPacker
//
//    Class to assist in parsing a buffer into bit/byte specified fields
/*=========================================================================*/
class cBitPacker
{
   public:
      // Constructor (default)
      cBitPacker();

      // Constructor (from a  buffer)
      cBitPacker(
         BYTE *                     pData,
         ULONG                      dataBitSize );

      // Destructor
      ~cBitPacker();

      // (Inline) Returns the number of bits left in the buffer (from the 
      // current working bit offset)
      ULONG GetNumBitsLeft() const
      {
         return (mMaxOffset - mOffset);
      };

      // (Inline) Returns the number of bits in the buffer that have been 
      // written (essentially the current working bit offset)
      ULONG GetNumBitsWritten() const
      {
         return (mOffset);
      };

      // (Inline) Returns the number of bits in the buffer that have been 
      // written (essentially the maximum value the working bit offset
      // attained up to now)
      ULONG GetTotalBitsWritten() const
      {
         return mMaxAttainedOffset;
      };

      // (Inline) Set current working bit offset
      void SetOffset( ULONG offset )
      {
         mOffset = offset;
         if (mOffset > mMaxOffset)
         {
            mOffset = mMaxOffset;
         }

         if (mOffset > mMaxAttainedOffset)
         {
            mMaxAttainedOffset = mOffset;
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

      // Write 'numBits' from a CHAR (advances offset)
      DWORD Set(
         ULONG                      numBits,  
         CHAR                       dataIn );

      // Write 'numBits' from a SHORT (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         SHORT                      dataIn );

      // Write 'numBits' from a LONG (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         LONG                       dataIn );

      // Write 'numBits' from a LONGLONG (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         LONGLONG                   dataIn );

      // Write 'numBits' from a UCHAR (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         UCHAR                      dataIn );

      // Write 'numBits' from a USHORT (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         USHORT                     dataIn );

      // Write 'numBits' from a ULONG (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         ULONG                      dataIn );

      // Write 'numBits' from a ULONGLONG (advances offset)
      DWORD Set(
         ULONG                      numBits, 
         ULONGLONG                  dataIn );

      // Release the data being parsed
      void ReleaseData();

      // Set the data being parsed
      void SetData(
         BYTE *                     pData,
         ULONG                      dataBitSize );

   protected:
      /* Data buffer */
      BYTE * mpData;

      /* Current bit-specified offset */
      ULONG mOffset;

      /* Maximum value the above bit offset attained */
      ULONG mMaxAttainedOffset;

      /* Maximum bit-specified offset */
      ULONG mMaxOffset;

      /* Are we parsing LSB -> MSB (the default)? */
      bool mbLSB;

};
