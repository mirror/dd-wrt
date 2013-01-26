/*===========================================================================
FILE:
   DataParser.h

DESCRIPTION:
   Declaration of sParsedField and cDataParser
   
PUBLIC CLASSES AND METHODS:
   sParsedField
      Structure to represent a single parsed field (field ID, offset,
      size, value, name, etc.)

   cDataParser
      Class to parse a buffer into bit/byte specified fields accordinging
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
#include "BitParser.h"
#include "ProtocolEntityNav.h"
#include "ProtocolBuffer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
class cColorItem;

/*=========================================================================*/
// Union uFields
//
//    Union to represent the data of a parsed field
/*=========================================================================*/
union uFields
{
   CHAR      mS8;
   UCHAR     mU8;
   SHORT     mS16;
   USHORT    mU16;
   LONG      mS32;
   ULONG     mU32;
   FLOAT     mFP32;
   LONGLONG  mS64;
   ULONGLONG mU64;
   double    mFP64;
   LPCSTR    mpAStr;
};

/*=========================================================================*/
// Struct sParsedField
//
//    Structure to represent a parsed field
/*=========================================================================*/
struct sParsedField
{
   // Give data parser full access
   friend class cDataParser;

   public:
      // (Inline) Constructor - default
      sParsedField()
         :  mField(),
            mOffset( 0 ),
            mValueString( "" ),
            mName( "" ),
            mbValid( false )
      { 
         memset( (PVOID)&mValue, 0, sizeof( mValue ) );
      };

      // Constructor - parameterized
      sParsedField( 
         const cCoreDatabase &      db,
         const sDB2Field *          pField,
         const std::string &        name,
         cBitParser &               bp,
         bool                       bGenStrings = true );

      // (Inline) Get the raw value string (i.e. unmapped enums)
      std::string GetRawValueString() const
      {
         std::string retStr = "";
         std::ostringstream tmp;
         
         if (IsValid() == true)
         {
            if (mField.mType == eDB2_FIELD_ENUM_UNSIGNED)
            {
               if (mField.mbHex == false)
               {
                  tmp << mValue.mU32;
                  retStr = tmp.str();
               }
               else
               {
                  tmp << std::ios_base::hex << std::ios_base::uppercase 
                      << std::ios_base::showbase << mValue.mU32;
                  retStr = tmp.str();
               }
            }
            else if (mField.mType == eDB2_FIELD_ENUM_SIGNED)
            {
               if (mField.mbHex == false)
               {
                  tmp << mValue.mS32;
                  retStr = tmp.str();
               }
               else
               {
                  tmp << std::ios_base::hex << std::ios_base::uppercase 
                      << std::ios_base::showbase << mValue.mU32;
                  retStr = tmp.str();
               }
            }
            else
            {
               retStr = mValueString;
            }     
         }

         return retStr;
      };

      // (Inline) Get field size in bits
      ULONG GetSize() const
      {  
         ULONG sz = 0;
         if (mField.IsValid() == true)
         {
            sz = mField.mSize;
         }

         return sz;
      };

      // (Inline) Is this field a string type?
      bool IsString() const
      {
         bool bStr = false;
         if (IsValid() == false)
         {
            return bStr;
         }

         if (mField.mType == eDB2_FIELD_STD)
         {
            switch ((eDB2StdFieldType)mField.mTypeVal)
            {
               case eDB2_FIELD_STDTYPE_STRING_A:
               case eDB2_FIELD_STDTYPE_STRING_U:
               case eDB2_FIELD_STDTYPE_STRING_U8:
               case eDB2_FIELD_STDTYPE_STRING_ANT:
               case eDB2_FIELD_STDTYPE_STRING_UNT:
               case eDB2_FIELD_STDTYPE_STRING_U8NT:
                  bStr = true;
                  break;
            }
         }

         return bStr;
      };

      // (Inline) Is this object valid?
      bool IsValid() const
      {
         return mbValid;
      };

      /* Field definition */
      sDB2Field mField;

      /* Bit offset (from start of payload) */ 
      ULONG mOffset;

      /* Field value */
      uFields mValue;

      /* Field value as a string */
      std::string mValueString;

      /* Partially qualified name of field */
      std::string mName;

   protected:
      // Parse a string
      bool ParseString(
         ULONG                      numChars,
         cBitParser &               bp );

      /* Is this object valid? */
      bool mbValid;
};

/*=========================================================================*/
// Class cParsedFieldNavigator
//
//    Class to navigate/search parsed fields produced by the above
/*=========================================================================*/
class cParsedFieldNavigator
{
   public:
      // (Inline) Constructor
      cParsedFieldNavigator( const std::vector <sParsedField> & pf )
         :  mFields( pf ),
            mLastIDIndex( ULONG_MAX )
      { };

      // Get index of the (first) field that matches the field ID,
      // the search starts from the last success index returned by
      // a previous call to this method
      ULONG GetFieldIndex( 
         ULONG                      fieldID,
         bool                       bLoop = false ) const;

      // (Inline) Get index of the (first) field that matches the 
      // given ID, the search starts from the provided index 
      ULONG GetFieldIndexFrom( 
         ULONG                      fieldID,
         ULONG                      startIndex,
         bool                       bLoop = false ) const
      {
         mLastIDIndex = startIndex;
         return GetFieldIndex( fieldID, bLoop );
      };

   protected:
      /* The list of parsed fields */
      const std::vector <sParsedField> & mFields;

      /* Index of last field we matched */
      mutable ULONG mLastIDIndex;
};

/*=========================================================================*/
// Class cDataParser
//    Class to parse a buffer into bit/byte specified fields
/*=========================================================================*/
class cDataParser : public cProtocolEntityNav
{
   public:
      // Constructor (protocol buffer)
      cDataParser( 
         const cCoreDatabase &      db,
         const sProtocolBuffer &    buffer );

      // Constructor (protocol buffer, entity key, and payload)
      cDataParser( 
         const cCoreDatabase &         db,
         const sProtocolBuffer &       buffer,
         const std::vector <ULONG> &   key,
         const BYTE *                  pData,
         ULONG                         dataLen );

      // Destructor
      virtual ~cDataParser();

      // Parse the data to a list of fields/summary text
      virtual bool Parse(
         bool                       bFieldStrings = true,
         bool                       bFieldNames = true );

      // (Inline) Get the protocol entity name
      std::string GetEntityName() const
      {
         std::string retName = "?";
         if (mEntity.mpName != 0 && mEntity.mpName[0] != 0)
         {
            retName = mEntity.mpName;
         }

         return retName;
      };

      // (Inline) Get the parsed fields
      typedef std::vector <sParsedField> tParsedFields;
      const tParsedFields & GetFields() const
      {
         return mFields;
      };

   protected:
      // Working from the back of the current field list find
      // and return the value for the specified field ID as a
      // LONGLONG (field type must be able to fit)
      virtual bool GetLastValue( 
         ULONG                      fieldID,
         LONGLONG &                 val );

      // Contiue navigation now that entity has been set?
      virtual bool ContinueNavigation();

      // Process the given field 
      virtual bool ProcessField(
         const sDB2Field *          pField,
         const std::string &            fieldName,
         LONGLONG                   arrayIndex = -1 );

      // (Inline) Get current working offset 
      virtual ULONG GetOffset() 
      {         
         return mBitsy.GetNumBitsParsed();
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

      // Handle special case processing for summary text generation
      virtual void HandleSpecialCases(
         std::string &               args,
         std::string &               fs );
         
      /* Color item containing the data we are parsing */
      sProtocolBuffer mBuffer;

      /* Entity key */
      std::vector <ULONG> mKey;

      /* The underlying bit parser */
      cBitParser mBitsy;

      /* The list of parsed fields */
      tParsedFields mFields;

      /* Generate field value strings? */
      bool mbFieldStrings;

      /* Did we successfully parse the buffer? */
      bool mbParsed;

      /* Parsed field vector index of last instance of each field (by ID) */
      std::map <ULONG, ULONG> mFieldIndices;
};

