/*===========================================================================
FILE: 
   Gobi3000Translation.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

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

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "Gobi3000Translation.h"

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetTLV

DESCRIPTION:
   Return the starting location and size of TLV buffer.

   NOTE: does not include the TLV header

PARAMETERS:
   inLen             [ I ] - Length of input buffer
   pIn               [ I ] - Input buffer
   type              [ I ] - Type ID
   pOutLen           [ O ] - Length of the output buffer
   ppOut             [ O ] - Pointer to output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetTLV(
   ULONG          inLen,
   const BYTE *   pIn,
   BYTE           typeID,
   ULONG *        pOutLen,
   const BYTE **  ppOut )
{
   if (pIn == 0 || pOutLen == 0 || ppOut == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   for (ULONG offset = 0; 
        offset + sizeof( sQMIRawContentHeader ) <= inLen; 
        offset += sizeof( sQMIRawContentHeader ))
   {
      sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)(pIn + offset);

      // Is it big enough to contain this TLV?
      if (offset + sizeof( sQMIRawContentHeader ) + pHeader->mLength > inLen)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      if (pHeader->mTypeID == typeID)
      {
         *pOutLen = pHeader->mLength;
         *ppOut = pIn + offset + sizeof( sQMIRawContentHeader );

         return eGOBI_ERR_NONE;
      }

      offset += pHeader->mLength;
   }
   
   // TLV not found
   return eGOBI_ERR_INVALID_RSP;
}
