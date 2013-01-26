/*===========================================================================
FILE:
   SharedBuffer.h

DESCRIPTION:
   Shareable buffer structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sSharedBuffer
      Simple struct to represent a reference counted shareable (no copy)
      buffer, as the basis for all buffer related classes

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
// Forward Declarations
//---------------------------------------------------------------------------
struct sProtocolBuffer;

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Maximum size of a shared buffer
const ULONG MAX_SHARED_BUFFER_SIZE = 1024 * 16 + 256;

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sSharedBuffer
//
//    Simple struct to represent a reference counted shareable (no copy)
//    buffer, as the basis for all buffer related classes
//
//    NOTE: Do *NOT* create instances of this structure on the stack, it 
//    must be dynamically allocated in order to function correctly
/*=========================================================================*/
struct sSharedBuffer
{
   public:
      // Constructor (copy passed in buffer)
      sSharedBuffer( 
         const BYTE *               pDataToCopy,
         ULONG                      dataLen,
         ULONG                      dataType );

      // Constructor (assume ownership of passed in buffer)
      sSharedBuffer( 
         ULONG                      dataLen,
         PBYTE                      pDataToOwn,
         ULONG                      dataType );

      // Destructor
      virtual ~sSharedBuffer();

      // Equality operator
      bool operator == ( const sSharedBuffer & ) const;

      // Inequality operator
      bool operator != ( const sSharedBuffer & ) const;

      // (Inline) Get buffer
      const BYTE * GetBuffer() const
      {
         return mpData;
      };   

      // (Inline) Get buffer size
      ULONG GetSize() const
      {
         return mSize;
      };

      // (Inline) Get buffer type
      ULONG GetType() const
      {
         return mType;
      };

      // (Inline) Is this buffer valid?
      bool IsValid() const
      {
         return (mpData != 0 && IsValidSize( mSize ));
      };

      // (Inline) Get reference count
      ULONG GetRefCount() const
      {
         return mRefCount;
      };

      // (Static Inline) Is the passed in size within the allowable range
      // a shared buffer?
      static bool IsValidSize( ULONG sz )
      {
         return (sz > 0 && sz <= MAX_SHARED_BUFFER_SIZE);
      };      

   protected:
      // Add reference
      void AddRef();

      // Release reference, delete if reference count zero
      void Release();

      /* Data */
      PBYTE mpData;

      /* Size of data */
      ULONG mSize;

      /* Type of data */
      ULONG mType;

      /* Reference count */
      ULONG mRefCount;

   private:
      // Leave copy constructor and assignment operator unimplemented
      // to prevent unintentional and unauthorized copying of the object
      // (which would lead to bad reference counting)
      sSharedBuffer( const sSharedBuffer & );
      sSharedBuffer & operator = ( const sSharedBuffer & );

      friend struct sProtocolBuffer;
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

