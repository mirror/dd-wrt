/*===========================================================================
FILE:
   DataPacker.h

DESCRIPTION:
   Declaration of sUnpackedField and cDataPacker

PUBLIC CLASSES AND METHODS:
   sUnpackedField
      Structure to represent a single unpacked (input) field - i.e. the
      field value as a string and an optional field name (either fully
      qualified) or partial

   cDataPacker
      Class to pack bit/byte specified fields into a buffer accordinging
      to a database description, uses cProtocolEntityNav to navigate the DB
      definition

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
#include "CoreDatabase.h"
#include "CoreUtilities.h"
#include "BitPacker.h"
#include "SharedBuffer.h"
#include "ProtocolEntityNav.h"

#include <list>
#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Struct sUnpackedField
//
//    Structure to represent an unpacked (input) field
/*=========================================================================*/
struct sUnpackedField
{
   public:
      // (Inline) Constructor - default
      sUnpackedField()
         :  mName( "" ),
            mValueString( "" )
      { };

      // (Inline) Constructor - parameterized
      sUnpackedField( 
         const std::string &     name,
         const std::string &     valueStr )
         :  mName( name ),
            mValueString( valueStr )
      { };

      /* Field value as a string */
      std::string mValueString;

      /* Name of field */
      std::string mName;
};

/*=========================================================================*/
// Class cDataPacker
//    Class to pack bit/byte specified fields into a buffer
/*=========================================================================*/
class cDataPacker : public cProtocolEntityNav
{
   public:
      // Constructor
      cDataPacker( 
         const cCoreDatabase &               db,
         const std::vector <ULONG> &         key,
         const std::list <sUnpackedField> &  fields );
         
      // Destructor
      virtual ~cDataPacker();

      // Pack the buffer
      virtual bool Pack();

      // Get packed buffer contents
      const BYTE * GetBuffer( ULONG & bufferLen );

      // Return the results of packing as an allocated shared buffer  
      sSharedBuffer * GetDiagBuffer( bool bNVRead );

      // Load values by parsing a 'summary' string of values
      static std::list <sUnpackedField> LoadValues( const std::string & vals );

      // Load values by parsing a vector of string values
      static std::list <sUnpackedField> LoadValues(
         std::vector <std::string> &  vals,
         ULONG                        startIndex );

   protected:
      // Working from the back of the current value list find
      // and return the value for the specified field ID as a
      // LONGLONG (field type must be able to fit)
      virtual bool GetLastValue( 
         ULONG                      fieldID,
         LONGLONG &                 val );

      // For the given field return the (input) value string
      virtual bool GetValueString(
         const sDB2Field &          field,
         const std::string &        fieldName,
         LPCSTR &                   pValueString );

      // Pack the string (described by the given arguments) into the buffer
      virtual bool PackString( 
         ULONG                      numChars,
         LPCSTR                     pStr );

      // Process the given field 
      virtual bool ProcessField(
         const sDB2Field *          pField,
         const std::string &        fieldName,
         LONGLONG                   arrayIndex = -1 );

      // (Inline) Get current working offset 
      virtual ULONG GetOffset() 
      {         
         return mBitsy.GetNumBitsWritten();
      };

      // (Inline) Set current working offset 
      virtual bool SetOffset( ULONG offset ) 
      {
         mBitsy.SetOffset( offset );
         return true;
      };

      // (Inline) Get current navigation order
      virtual bool GetLSBMode() 
      {
         return mBitsy.GetLSBMode();
      };

      // (Inline) Set current navigation order
      virtual bool SetLSBMode( bool bLSB ) 
      {
         // Assume success
         bool bOK = true;
         if (bLSB != GetLSBMode())
         {
            if ((GetOffset() % BITS_PER_BYTE) != 0)
            {
               // We need to be on a byte boundary
               bOK = false;
            }
            else
            {
               mBitsy.SetLSBMode( bLSB );
            }
         }

         return bOK;
      };

      /* Entity key */
      std::vector <ULONG> mKey;

      /* The underlying bit packer */
      cBitPacker mBitsy;

      /* The vector of fields */
      std::vector <sUnpackedField> mFields;

      /* Are we operating in value only mode, i.e. no field names given? */
      bool mbValuesOnly;
      ULONG mProcessedFields;

      /* Raw field values associated with field ID */
      std::list < std::pair <ULONG, LONGLONG> > mValues;

      /* Internal working buffer */
      BYTE mBuffer[MAX_SHARED_BUFFER_SIZE];

      /* Did we successfully pack the buffer? */
      bool mbPacked;
};
