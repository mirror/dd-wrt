/*===========================================================================
FILE:
   BitPacker.cpp

DESCRIPTION:
   Implementation of cBitPacker class
   
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
// Include Files
//---------------------------------------------------------------------------

#include "StdAfx.h"
#include "BitPacker.h"

#include <list>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   SetUnsignedVal (Public Method)

DESCRIPTION:
   Set an unsigned value in the bit source (writing least significant
   bits first)

PARAMETERS:
   pData          [ O ] - Data buffer to write to
   currentOffset  [I/O] - Current bit offset into above buffer
   maxOffset      [ I ] - Maximum bit offset into above buffer
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Input value to write
   bLSB           [ I ] - Pack LSB -> MSB?

RETURN VALUE:
   DWORD
===========================================================================*/
template <class T> 
DWORD SetUnsignedVal(
   BYTE *                     pData,
   ULONG &                    currentOffset,
   ULONG                      maxOffset,
   ULONG                      numBits,
   T                          dataIn,
   bool                       bLSB = true )
{
   ASSERT( pData != 0 );

   // Number of bits in the passed in type
   const ULONG TYPE_BIT_COUNT = (ULONG)(sizeof( T ) * BITS_PER_BYTE);
   ASSERT( numBits > 0 && numBits <= TYPE_BIT_COUNT);

   // Requesting too much?
   if (currentOffset < maxOffset)
   {
      ULONG bitsToGo = maxOffset - currentOffset;
      if (bitsToGo < numBits)
      {
         return ERROR_NOT_ENOUGH_MEMORY;
      }
   }
   else if (currentOffset == maxOffset)
   {
      // Silly rabbit, don't bother to call us if you don't want any bits!
      return ERROR_INVALID_PARAMETER;
   }
   else
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   // Advance to first valid byte
   pData += (currentOffset / BITS_PER_BYTE);

   // Since we don't really care about performance for bit packing
   // (we do not anticipate this being called as frequently as bit
   // parsing) we always use the generic approach

   // Reduce input to a bit array
   BYTE bits[MAX_TYPE_BITS];

   ULONG bitsExtracted = 0;   
   while (bitsExtracted < numBits)
   {
      if (bLSB == true)
      {
         BYTE bit = (BYTE)(dataIn & (T)1);
         bits[bitsExtracted++] = bit;
      }
      else
      {
         BYTE bit = (BYTE)(dataIn & (T)1);
         bits[numBits - ++bitsExtracted] = bit;
      }

      dataIn >>= 1;
   }


   // Store current offset
   ULONG offset = currentOffset;

   // Add in each bit - one at a time
   bitsExtracted = 0;
   while (bitsExtracted != numBits)
   {
      // How many bits are left in the current byte?
      ULONG bitsLeft = BITS_PER_BYTE - (offset % BITS_PER_BYTE);
      
      // Shift input bit over to desired destination
      BYTE tmp = bits[bitsExtracted++];

      if (bLSB == true)
      {
         tmp <<= (BITS_PER_BYTE - bitsLeft);
      }
      else
      {
         tmp <<= bitsLeft - 1;
      }

      *pData |= tmp;

      // Advance to next byte in buffer?
      offset++;
      if (offset % BITS_PER_BYTE == 0)
      {
         pData++;
      }
   }

   currentOffset += numBits;
   return NO_ERROR;
}

/*=========================================================================*/
// cBitPacker Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cBitPacker (Public Method)

DESCRIPTION:
   Constructor (default)
  
RETURN VALUE:
   None
===========================================================================*/
cBitPacker::cBitPacker()
   :  mpData( 0 ),
      mOffset( 0 ),
      mMaxAttainedOffset( 0 ),
      mMaxOffset( 0 ),
      mbLSB( true )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   cBitPacker (Public Method)

DESCRIPTION:
   Constructor (from a  buffer)

PARAMETERS:
   pData       [ I ] - Data buffer
   dataBitSize [ I ] - Size of above data buffer (in bits)
  
RETURN VALUE:
   None
===========================================================================*/
cBitPacker::cBitPacker( 
   BYTE *                     pData,
   ULONG                      dataBitSize )
   :  mpData( 0 ),
      mOffset( 0 ),
      mMaxAttainedOffset( 0 ),
      mMaxOffset( 0 ),
      mbLSB( true )
{
   SetData( pData, dataBitSize );
}

/*===========================================================================
METHOD:
   ~cBitPacker (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cBitPacker::~cBitPacker()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a CHAR (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   CHAR                       dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits,  
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a SHORT (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   SHORT                      dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits, 
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a LONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   LONG                       dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits, 
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a LONGLONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   LONGLONG                   dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits, 
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a UCHAR (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   UCHAR                      dataIn )
{
   DWORD rc =  SetUnsignedVal( mpData, 
                               mOffset,
                               mMaxOffset,
                               numBits, 
                               dataIn,
                               mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a USHORT (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   USHORT                     dataIn )
{
   DWORD rc =  SetUnsignedVal( mpData, 
                               mOffset,
                               mMaxOffset,
                               numBits, 
                               dataIn,
                               mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a ULONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   ULONG                      dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits, 
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   Set (Public Method)

DESCRIPTION:
   Write 'numBits' from a ULONGLONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to write
   dataIn         [ I ] - Value to write

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitPacker::Set(
   ULONG                      numBits, 
   ULONGLONG                  dataIn )
{
   DWORD rc = SetUnsignedVal( mpData, 
                              mOffset,
                              mMaxOffset,
                              numBits, 
                              dataIn,
                              mbLSB );

   if (rc == NO_ERROR && mOffset > mMaxAttainedOffset)
   {
      mMaxAttainedOffset = mOffset;
   }

   return rc;
}

/*===========================================================================
METHOD:
   ReleaseData (Public Method)

DESCRIPTION:
   Release the data being parsed
  
RETURN VALUE:
   None
===========================================================================*/
void cBitPacker::ReleaseData()
{
   // Clear out current buffer
   mpData = 0;
   mOffset = 0;
   mMaxAttainedOffset = 0;
   mMaxOffset = 0;
};

/*===========================================================================
METHOD:
   SetData (Public Method)

DESCRIPTION:
   Set the data being parsed

PARAMETERS:
   pData       [ I ] - Data buffer
   dataBitSize [ I ] - Size of above data buffer (in bits)
  
RETURN VALUE:
   None
===========================================================================*/
void cBitPacker::SetData( 
   BYTE *                     pData,
   ULONG                      dataBitSize )
{
   // Release current buffer
   ReleaseData();

   // Anything to parse?
   if (pData != 0)
   {
      // Yes
      mpData = pData;
      mMaxOffset = dataBitSize;
   }
   else
   {
      // No
      ASSERT( mpData != 0 );
   }
}
