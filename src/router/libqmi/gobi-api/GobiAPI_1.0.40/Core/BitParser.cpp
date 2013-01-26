/*===========================================================================
FILE:
   BitParser.cpp

DESCRIPTION:
   Implementation of cBitParser class
   
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
// Include Files
//---------------------------------------------------------------------------

#include "StdAfx.h"
#include "BitParser.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

static BYTE MASK[BITS_PER_BYTE + 1] =
{
   0x00,
   0x01,
   0x03,
   0x07,
   0x0F,
   0x1F,
   0x3F,
   0x7F,
   0xFF
};

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetUnsignedVal (Public Method)

DESCRIPTION:
   Get an unsigned value from the bit source (reading least significant
   bits first)

PARAMETERS:
   pData          [ I ] - Data buffer
   currentOffset  [I/O] - Current bit offset into above buffer
   maxOffset      [ I ] - Maximum bit offset into above buffer
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value
   bLSB           [ I ] - Parse LSB -> MSB?

RETURN VALUE:
   DWORD
===========================================================================*/
template <class T> 
DWORD GetUnsignedVal(
   const BYTE *               pData,
   ULONG &                    currentOffset,
   ULONG                      maxOffset,
   ULONG                      numBits,
   T &                        dataOut,
   bool                       bLSB = true )
{
   ASSERT( pData != 0 );

   // Number of bits in the passed in type
   const ULONG TYPE_BIT_COUNT = (ULONG)(sizeof( T ) * BITS_PER_BYTE);

   // Bad parameters?
   if (numBits == 0 || numBits > TYPE_BIT_COUNT || numBits > MAX_TYPE_BITS)
   {
      return ERROR_INVALID_PARAMETER;
   }

   // Requesting too much?   
   if (currentOffset < maxOffset)
   {
      ULONG bitsToGo = maxOffset - currentOffset;
      if (bitsToGo < numBits)
      {
         return ERROR_NOT_ENOUGH_MEMORY;
      }
   }
   else
   {
      // No bits left!
      return ERROR_NOT_ENOUGH_MEMORY;
   }
  
   // Advance to first valid bit
   pData += (currentOffset / BITS_PER_BYTE);

   // Number of bits left in current byte
   ULONG bitsLeft = BITS_PER_BYTE - (currentOffset % BITS_PER_BYTE);

   if (bLSB == true)
   {
      // Extracting native types on byte boundaries?
      if (numBits == TYPE_BIT_COUNT && bitsLeft == BITS_PER_BYTE)
      {
         // Yes, a simple cast will suffice
         dataOut = *((T *)pData);

         currentOffset += numBits;
         return NO_ERROR;
      }

      // Extracting some small number of bits?
      if (numBits <= bitsLeft)
      {
         // Yes, simply shift back to origin and AND with correct mask
         BYTE tmp = *pData;
         tmp >>= (BITS_PER_BYTE - bitsLeft);
         tmp &= MASK[numBits];

         dataOut = (T)tmp;

         currentOffset += numBits;
         return NO_ERROR;
      }
   }

   // Not either of the simple cases - extract the relevant bits
   // and then build the output

   // Extract bits
   BYTE bits[MAX_TYPE_BITS];
   ULONG bitsExtracted = 0;
   
   while (bitsExtracted < numBits)
   {
      BYTE bit = *pData;

      if (bLSB == true)
      {
         bit <<= (bitsLeft - 1);
         bit >>= (BITS_PER_BYTE - 1);
         bits[bitsExtracted++] = bit;
      }
      else
      {
         bit >>= (bitsLeft - 1);
         bit &= 0x01;
         bits[numBits - ++bitsExtracted] = bit;
      }

      bitsLeft--;
      if (bitsLeft == 0)
      {
         pData++;
         bitsLeft = BITS_PER_BYTE;
      }
   }

   // Reassemble to form output value
   dataOut = 0;
   T tmp = 0;

   for (ULONG b = 0; b < numBits; b++)
   {
      tmp = bits[b];
      tmp <<= b;

      dataOut |= tmp;
   }

   currentOffset += numBits;
   return NO_ERROR;
}

/*===========================================================================
METHOD:
   GetSignedVal (Public Method)

DESCRIPTION:
   Get an signed value from the bit source (reading least significant
   bits first), just gets the equivalent unsigned representation and 
   then sign-extends as necessary

PARAMETERS:
   pData          [ I ] - Data buffer
   currentOffset  [I/O] - Current bit offset into above buffer
   maxOffset      [ I ] - Maximum bit offset into above buffer
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value
   bLSB           [ I ] - Parse LSB -> MSB?

RETURN VALUE:
   DWORD
===========================================================================*/
template <class T> 
DWORD GetSignedVal(
   const BYTE *               pData,
   ULONG &                    currentOffset,
   ULONG                      maxOffset,
   ULONG                        numBits,
   T &                        dataOut,
   bool                       bLSB = true )
{
   DWORD rc = GetUnsignedVal( pData, 
                              currentOffset,
                              maxOffset,
                              numBits, 
                              dataOut,
                              bLSB );

   if (rc == NO_ERROR)
   {
      // If the highest-order bit is one, we must sign-extend
      bool bSignExtend = (numBits < (sizeof( T ) * BITS_PER_BYTE)) 
                      && ((dataOut >> (numBits - 1)) & 1) == 1;

      if (bSignExtend == true)
      {
         T mask = (T)((~0) << numBits);
         dataOut |= mask;
      }
   }

   return rc;
}

/*=========================================================================*/
// cBitParser Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cBitParser (Public Method)

DESCRIPTION:
   Constructor (default)
  
RETURN VALUE:
   None
===========================================================================*/
cBitParser::cBitParser()
   :  mpData( 0 ),
      mOffset( 0 ),
      mMaxOffset( 0 ),
      mbLSB( true )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   cBitParser (Public Method)

DESCRIPTION:
   Constructor (from a  buffer)

PARAMETERS:
   pData       [ I ] - Data buffer
   dataBitSize [ I ] - Size of above data buffer (in bits)
  
RETURN VALUE:
   None
===========================================================================*/
cBitParser::cBitParser( 
   const BYTE *               pData,
   ULONG                      dataBitSize )
   :  mpData( 0 ),
      mOffset( 0 ),
      mMaxOffset( 0 ),
      mbLSB( true )
{
   SetData( pData, dataBitSize );
}

/*===========================================================================
METHOD:
   cBitParser (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cBitParser::~cBitParser()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a CHAR (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   CHAR &                     dataOut )
{
   return GetSignedVal( mpData, 
                        mOffset,
                        mMaxOffset,
                        numBits, 
                        dataOut,
                        mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a SHORT (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   SHORT &                    dataOut )
{
   return GetSignedVal( mpData, 
                        mOffset,
                        mMaxOffset,
                        numBits, 
                        dataOut,
                        mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a LONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   LONG &                     dataOut )
{
   return GetSignedVal( mpData, 
                        mOffset,
                        mMaxOffset,
                        numBits, 
                        dataOut,
                        mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a LONGLONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   LONGLONG &                 dataOut )
{
   return GetSignedVal( mpData, 
                        mOffset,
                        mMaxOffset,
                        numBits, 
                        dataOut,
                        mbLSB );
}


/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a UCHAR (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   UCHAR &                    dataOut )
{
   return GetUnsignedVal( mpData, 
                          mOffset,
                          mMaxOffset,
                          numBits, 
                          dataOut,
                          mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a USHORT (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   USHORT &                   dataOut )
{
   return GetUnsignedVal( mpData, 
                          mOffset,
                          mMaxOffset,
                          numBits, 
                          dataOut,
                          mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a ULONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   ULONG &                    dataOut )
{
   return GetUnsignedVal( mpData, 
                          mOffset,
                          mMaxOffset,
                          numBits, 
                          dataOut,
                          mbLSB );
}

/*===========================================================================
METHOD:
   Get (Public Method)

DESCRIPTION:
   Return 'numBits' as a ULONGLONG (advances offset)

PARAMETERS:
   numBits        [ I ] - Number of bits to process
   dataOut        [ O ] - Processed value

RETURN VALUE:
   DWORD
===========================================================================*/
DWORD cBitParser::Get(
   ULONG                      numBits, 
   ULONGLONG &                dataOut )
{
   return GetUnsignedVal( mpData, 
                          mOffset,
                          mMaxOffset,
                          numBits, 
                          dataOut,
                          mbLSB );
}

/*===========================================================================
METHOD:
   ReleaseData (Public Method)

DESCRIPTION:
   Release the data being parsed
  
RETURN VALUE:
   None
===========================================================================*/
void cBitParser::ReleaseData()
{
   // Clear out current buffer
   mpData = 0;
   mOffset = 0;
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
void cBitParser::SetData( 
   const BYTE *               pData,
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
