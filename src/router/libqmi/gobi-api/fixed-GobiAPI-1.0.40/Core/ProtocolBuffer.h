/*===========================================================================
FILE:
   ProtocolBuffer.h

DESCRIPTION:
   Generic protocol structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sProtocolBuffer
      Simple struct to represent a protocol buffer using a reference counted
      (shared) buffer, this allows us to use in in several places without
      copying it once in each place.  A few base services are provided
      but the main purpose is to provide a class to inherit off of for
      specific protocols

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
// Include Files
//---------------------------------------------------------------------------
#include "SharedBuffer.h"
#include "ProtocolEnum.h"

static const tm EMPTY_TIME = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 

/*=========================================================================*/
// Struct sProtocolBuffer
/*=========================================================================*/
struct sProtocolBuffer
{
   public:
      // Constructor (default)
      sProtocolBuffer();

      // Constructor (parameterized)
      sProtocolBuffer( sSharedBuffer * pBuffer );

      // Copy constructor
      sProtocolBuffer( const sProtocolBuffer & copyThis );

      // Assignment operator
      sProtocolBuffer & operator = ( const sProtocolBuffer & copyThis );

      // Destructor
      virtual ~sProtocolBuffer();

      // (Inline) Get buffer
      const BYTE * GetBuffer() const
      {
         BYTE * pRet = 0;
         if (IsValid() == true)
         {
            pRet = (BYTE *)mpData->GetBuffer();
         }

         return (const BYTE *)pRet;
      };

      // (Inline) Get buffer size
      ULONG GetSize() const
      {
         ULONG size = 0;
         if (IsValid() == true)
         {
            size = mpData->GetSize();
         }

         return size;
      };

      // (Inline) Return the protocol type
      eProtocolType GetType() const
      {
         eProtocolType pt = ePROTOCOL_ENUM_BEGIN;
         if (IsValid() == true)
         {
            pt = (eProtocolType)mpData->GetType();
         }   

         return pt;
      };

      // (Inline) Return the shared buffer
      sSharedBuffer * GetSharedBuffer() const
      {
         sSharedBuffer * pRet = 0;
         if (IsValid() == true)
         {
            pRet = mpData;
         }

         return pRet;
      };

      // (Inline) Return the timestamp
      tm GetTimestamp() const
      {
         tm ft = EMPTY_TIME;
         
         if (IsValid() == true)
         {
            ft = mTimestamp;
         }   

         return ft;
      };

      // (Inline) Is this buffer valid?
      virtual bool IsValid() const
      {
         return mbValid;
      };

   protected:
      // (Inline) Validate buffer
      virtual bool Validate()
      {
         // Do we have a shared buffer and is it valid?
         mbValid = (mpData != 0 && mpData->IsValid());
         return mbValid;
      };

      /* Our data buffer */
      sSharedBuffer * mpData;

      /* Time buffer was created */
      tm mTimestamp;

      /* Has this buffer been validated? (NOTE: *NOT* set in base) */
      bool mbValid;
};
