/*===========================================================================
FILE:
   DataParser.cpp

DESCRIPTION:
   Implementation of sParsedField and cDataParser
   
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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "DataParser.h"

#include "CoreDatabase.h"
#include "DB2Utilities.h"

#include <climits>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// sParsedField Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sParsedField (Public Method)

DESCRIPTION:
   Construct a parsed field by setting the values and extracting
   the data according to the definition
  
PARAMETERS:
   db          [ I ] - Database to use
   field       [ I ] - Field description (from database)
   name        [ I ] - Desired field name
   bp          [I/O] - Bit parser to use
   bGenStrings [ I ] - Generate field value strings? 
   
   NOTE: 'bGenStrings' does not apply to fields that are string types?

RETURN VALUE:
   None
===========================================================================*/
sParsedField::sParsedField( 
   const cCoreDatabase &            db,
   const sDB2Field *                pField,
   const std::string &              name,
   cBitParser &                     bp,
   bool                             bGenStrings )
   :  mField(),
      mOffset( bp.GetNumBitsParsed() ),
      mValueString( "" ),
      mName( name ),
      mbValid( false )
{
   // Clear value
   memset( (PVOID)&mValue, 0, (SIZE_T)sizeof( mValue ) );

   // Assume failure
   bool bOK = false;
   if (pField == 0)
   {
      return;
   }
   
   mField = *pField;

   char tempValueString[128];
   memset( &tempValueString[0], 0, 128 );

   // What type is this field?
   switch (mField.mType)
   {
      case eDB2_FIELD_STD:
      {
         // Standard field, what kind?
         eDB2StdFieldType ft = (eDB2StdFieldType)mField.mTypeVal;
         switch (ft)
         {              
            // Field is a boolean (0/1, false/true)/8-bit unsigned integer
            case eDB2_FIELD_STDTYPE_BOOL:
            case eDB2_FIELD_STDTYPE_UINT8:
            {
               // We store as a UCHAR
               UCHAR val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Constrain boolean values?
                  if (ft == eDB2_FIELD_STDTYPE_BOOL && val > 1)
                  {
                     val = 1;
                  }

                  // Success!
                  mValue.mU8 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%02X", (UINT)mValue.mU8 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%u", (UINT)mValue.mU8 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 8-bit signed integer
            case eDB2_FIELD_STDTYPE_INT8:
            {
               // We store as a CHAR
               CHAR val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mS8 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%02X", (UINT)mValue.mU8 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%d", (INT)mValue.mS8 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 16-bit signed integer
            case eDB2_FIELD_STDTYPE_INT16: 
            {
               // We store as a SHORT
               SHORT val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mS16 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%04hX", mValue.mU16 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%hd", mValue.mS16 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 16-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT16:
            {
               // We store as a USHORT
               USHORT val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mU16 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%04hX", mValue.mU16 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%hu", mValue.mU16 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 32-bit signed integer
            case eDB2_FIELD_STDTYPE_INT32:
            {
               // We store as a LONG
               LONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mS32 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%08lX", mValue.mU32 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%ld", mValue.mS32 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 32-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT32:
            {
               // We store as a ULONG
               ULONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mU32 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%08lX", mValue.mU32 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%lu", mValue.mU32 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 64-bit signed integer
            case eDB2_FIELD_STDTYPE_INT64:
            {
               // We store as a LONGLONG
               LONGLONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mS64 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%016llX", mValue.mU64 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%lld", mValue.mS64 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 64-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT64:
            {
               // We store as a ULONGLONG
               ULONGLONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  mValue.mU64 = val;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%016llX", mValue.mU64 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%llu", mValue.mU64 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // ANSI/UNICODE fixed length string
            case eDB2_FIELD_STDTYPE_STRING_A:
            case eDB2_FIELD_STDTYPE_STRING_U:
            {
               // Compute the number of characters
               ULONG numChars = mField.mSize / BITS_PER_BYTE;

               // Parse out the string
               bOK = ParseString( numChars, bp );
            }
            break;

            // ANSI NULL terminated string
            case eDB2_FIELD_STDTYPE_STRING_ANT:
            {
               // Figure out the length of the string
               ULONG numChars = 0;

               // Temporarily assume success
               bOK = true;

               ULONG tmpOffset = bp.GetNumBitsParsed();

               CHAR val = 1;
               while (val != 0)
               {
                  DWORD rc = bp.Get( BITS_PER_BYTE, val );
                  if (rc == NO_ERROR)
                  {                
                     numChars++;                     
                  }
                  else
                  {
                     val = 0;
                  }
               }               

               // Now actually parse/load the string
               if (bOK == true)
               {                                
                  bp.SetOffset( tmpOffset );
                  bOK = ParseString( numChars, bp );
               }
            }
            break;

            // UNICODE NULL terminated string
            case eDB2_FIELD_STDTYPE_STRING_UNT:
            {
               // Figure out the length of the string
               ULONG numChars = 0;

               // Temporarily assume success
               bOK = true;

               ULONG tmpOffset = bp.GetNumBitsParsed();

               USHORT val = 1;
               while (val != 0)
               {
                  DWORD rc = bp.Get( BITS_PER_BYTE, val );
                  if (rc == NO_ERROR)
                  {
                     numChars++;
                  }
                  else
                  {
                     val = 0;
                  }
               }

               // Now actually parse/load the string
               if (bOK == true)
               {               
                  bp.SetOffset( tmpOffset );
                  bOK = ParseString( numChars, bp );             
               } 
            }     
            break;

            case eDB2_FIELD_STDTYPE_STRING_U8:
            case eDB2_FIELD_STDTYPE_STRING_U8NT:
               // Unsupported in the Linux adaptation
               bOK = false;
               break;

            // Field is 32-bit floating point value
            case eDB2_FIELD_STDTYPE_FLOAT32:
            {
               // We store as a ULONG
               ULONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {
                  FLOAT * pFloat = (FLOAT *)&val;

                  // Success!
                  mValue.mFP32 = *pFloat;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%04lX", mValue.mU32 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%f", mValue.mFP32 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;

            // Field is 64-bit floating point value
            case eDB2_FIELD_STDTYPE_FLOAT64:
            {
               // We store as a ULONGLONG
               ULONGLONG val;
               DWORD rc = bp.Get( mField.mSize, val );
               if (rc == NO_ERROR)
               {
                  DOUBLE * pFloat = (DOUBLE *)&val;

                  // Success!
                  mValue.mFP64 = *pFloat;
                  bOK = true;

                  if (bGenStrings == true)
                  {
                     if (mField.mbHex == true)
                     {
                        snprintf( &tempValueString[0], 0, "0x%08llX", mValue.mU64 );
                     }
                     else
                     {
                        snprintf( &tempValueString[0], 0, "%f", mValue.mFP64 );
                     }
                     mValueString = &tempValueString[0];
                  }
               }
            }
            break;
         }
      }
      break;

      // Unsigend enum value
      case eDB2_FIELD_ENUM_UNSIGNED:
      {
         // We store as a ULONG
         ULONG val;
         DWORD rc = bp.Get( mField.mSize, val );
         if (rc == NO_ERROR)
         {    
            // Success!
            mValue.mU32 = val;
            bOK = true;

            // Grab the enum ID
            ULONG id = pField->mTypeVal;

            // Map to a string?
            if (bGenStrings == true)
            {
               mValueString = db.MapEnumToString( id, 
                                                  (int)mValue.mU32, 
                                                  true,
                                                  mField.mbHex );
            }
         }
      }
      break;

      // Signed enum value
      case eDB2_FIELD_ENUM_SIGNED:
      {
         // We store as a LONG
         LONG val;
         DWORD rc = bp.Get( mField.mSize, val );
         if (rc == NO_ERROR)
         {         
            // Success!
            mValue.mS32 = val;
            bOK = true;

            // Grab the enum ID
            ULONG id = pField->mTypeVal;

            // Map to a string?
            if (bGenStrings == true)
            {
               mValueString = db.MapEnumToString( id, 
                                                  (int)mValue.mS32, 
                                                  true,
                                                  mField.mbHex );
            }
         }
      }
      break;
   }      

   mbValid = bOK;
}

/*===========================================================================
METHOD:
   ParseString (Public Method)

DESCRIPTION:
   Convert the field value to a string 
  
PARAMETERS:
   numChars    [ I ] - Number of characters to parse/load
   bp          [I/O] - Bit parser to use

RETURN VALUE:
   bool
===========================================================================*/
bool sParsedField::ParseString( 
   ULONG                      numChars,
   cBitParser &               bp )
{
   // Validate size (including null char)
   if (MAX_SHARED_BUFFER_SIZE < numChars + 1)
   {
      return false;
   }

   // Assume success
   bool bRC = true;

   // Store current offset so we can update field length
   ULONG curOffset = bp.GetNumBitsParsed();

   // Load each byte of the string individually
   BYTE buf[MAX_SHARED_BUFFER_SIZE];
   for (ULONG c = 0; c < numChars; c++)
   {
      BYTE val = 0;
      DWORD rc = bp.Get( BITS_PER_BYTE, val );
      if (rc == NO_ERROR)
      {     
         buf[c] = val;
      }
      else
      {
         bRC = false;
         break;
      }
   }

   if (bRC == true)
   {
      // Write zeros to the rest of the buffer
      ULONG size = numChars;
      ULONG end = numChars + 1;
      for (ULONG current = size; current < end; current++)
      {
         buf[current] = 0;
      }

      mValueString = (LPCSTR)&buf[0];

      mValue.mpAStr = (LPCSTR)mValueString.c_str();

      // Update field size
      mField.mSize = bp.GetNumBitsParsed() - curOffset;
   }
   
   return bRC;
}

/*=========================================================================*/
// cParsedFieldNavigator Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetFieldIndex (Public Method)

DESCRIPTION:
   Get index of the (first) field that matches the given field ID
  
PARAMETERS:
   fieldID     [ I ] - Field ID to look for
   bLoop       [ I ] - Loop around end of field list?

RETURN VALUE:
   ULONG - Index of the field (0xFFFFFFFF upon failure)
===========================================================================*/
ULONG cParsedFieldNavigator::GetFieldIndex( 
   ULONG                      fieldID,
   bool                       bLoop ) const 
{
   ULONG id = ULONG_MAX;
   ULONG count = (ULONG)mFields.size();

   // Start from last field ID?
   ULONG fp = 0;
   ULONG fi = 0;
   if (mLastIDIndex < count)
   {
      fi = mLastIDIndex;
   }
   else if (mLastIDIndex != ULONG_MAX && bLoop == false)
   {
      // Beyond end of fields with no looping
      mLastIDIndex = id;
      return id;
   }

   for (fp = 0; fp < count; fp++)
   {     
      if (mFields[fi].mField.mID == fieldID)
      {
         id = fi;
         break;
      }

      fi++;
      if (fi == count)
      {
         if (bLoop == true)
         {
            fi = 0;
         }
         else
         {
            break;
         }
      }
   }

   // Update last ID accordingly (0xFFFFFFFF upon failure), and return
   mLastIDIndex = id;
   if (mLastIDIndex != ULONG_MAX)
   {
      mLastIDIndex++;
      if (mLastIDIndex == count)
      {
         mLastIDIndex = 0;
      }
   }

   return id;
}

/*=========================================================================*/
// cDataParser Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cDataParser (Public Method)

DESCRIPTION:
   Constructor (protocol buffer, entity key, and payload)

PARAMETERS:
   db             [ I ] - Database to use
   buffer         [ I ] - The protocol buffer being parsed
   key            [ I ] - Protocol entity key
   pData          [ I ] - Payload from above protocol buffer
   dataLen        [ I ] - Size of above payload
  
RETURN VALUE:
   None
===========================================================================*/
cDataParser::cDataParser( 
   const cCoreDatabase &         db,
   const sProtocolBuffer &       buffer,
   const std::vector <ULONG> &   key,
   const BYTE *                  pData,
   ULONG                         dataLen )
   :  cProtocolEntityNav( db ),    
      mBuffer( buffer.GetSharedBuffer() ),
      mbFieldStrings( true ),
      mbParsed( false )
{
   // We must have a valid protocol buffer
   if (mBuffer.IsValid() == false)
   {
      return;
   }

   // We need something to parse
   if (pData == 0 || dataLen == 0)
   {
      return;
   }

   // Key has to be proper
   if (key.size() < 1)
   {
      return;
   }

   // Key needs to match protocol
   eProtocolType pt = (eProtocolType)mBuffer.GetType();
   eDB2EntityType et = (eDB2EntityType)key[0];

   if (pt == ePROTOCOL_DIAG_RX || pt == ePROTOCOL_DIAG_TX)
   {
      if (IsDiagEntityType( et ) == false)
      {
         return;
      }
   }

   else if (IsQMIProtocol( pt ) == true)
   {
      if (IsQMIEntityType( et ) == false)
      {
         return;
      }
   }

   // Pass data to the bit parser
   mKey = key;
   mBitsy.SetData( pData, dataLen * BITS_PER_BYTE );
}

/*===========================================================================
METHOD:
   ~cDataParser (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cDataParser::~cDataParser()
{
   // Ask bit parser to release data
   mBitsy.ReleaseData();

   // Empty fields
   mFields.clear();
}

/*===========================================================================
METHOD:
   Parse (Public Method)

DESCRIPTION:
   Parse the data to a list of fields/summary text

PARAMETERS:
   bFieldStrings  [ I ] - Generate string representations of field values?
   bFieldNames    [ I ] - Generate (partial) field names?

RETURN VALUE:
   bool
===========================================================================*/
bool cDataParser::Parse(
   bool                       bFieldStrings,
   bool                       bFieldNames )
{
   // Store parsing options
   mbFieldStrings = bFieldStrings;
   mbFieldNames   = bFieldNames;

   if (mbParsed == false)
   {
      // Allocate space for 1024 fields up front in order to increase
      // performance when parsing and accessing parsed fields
      mFields.reserve( 1024 );

      // Process (parse) the protocol entity
      mbParsed = ProcessEntity( mKey );
   }
   
   return mbParsed;
}


/*===========================================================================
METHOD:
   GetLastValue (Internal Method)

DESCRIPTION:
   Working from the back of the current field list find and return the 
   value for the specified field ID as a LONGLONG (field type must be 
   able to fit in a LONGLONG for a value to be returned)

PARAMETERS:
   fieldID     [ I ] - Field ID we are looking for
   val         [ O ] - The value
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataParser::GetLastValue( 
   ULONG                      fieldID,
   LONGLONG &                 val )
{
   // Assume failure
   bool bRC = false;

   // Use field value tracking information
   std::map <ULONG, std::pair <bool, LONGLONG> >::iterator pTF;
   pTF = mTrackedFields.find( fieldID );
   if (pTF != mTrackedFields.end() && pTF->second.first == true)
   {
      val = pTF->second.second;
      bRC = true;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ContinueNavigation (Internal Method)

DESCRIPTION:
   Continue navigation now that entity has been set?
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataParser::ContinueNavigation()
{
   // Proceed to parse?
   bool bParse = true;     

   // Is there actually something to process?
   if (mBitsy.GetNumBitsLeft() == 0)
   {
      bParse = false;
   }

   return bParse;
}

/*===========================================================================
METHOD:
   ProcessField (Internal Method)

DESCRIPTION:
   Process the given field by parsing the value

PARAMETERS:
   pField      [ I ] - The field being processed
   fieldName   [ I ] - Field name (partial)
   arrayIndex  [ I ] - Not used
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataParser::ProcessField(
   const sDB2Field *          pField,
   const std::string &            fieldName,
   LONGLONG                   /* arrayIndex */ )
{
   // Assume failure
   bool bRC = false;
   if (pField == 0)
   {
      return bRC;
   }

   // We must have a name
   sParsedField theField( mDB, 
                          pField, 
                          fieldName, 
                          mBitsy, 
                          mbFieldStrings );

   // Did that result in a valid field?
   if (theField.IsValid() == true)
   {
      // Add field 
      mFields.push_back( theField );         
      bRC = true;

      // Are we tracking the value of this field?
      std::map <ULONG, std::pair <bool, LONGLONG> >::iterator pTF;
      pTF = mTrackedFields.find( pField->mID );
      if (pTF != mTrackedFields.end())
      {           
         std::pair <bool, LONGLONG> & entry = pTF->second;

         // What type is this field?
         switch (pField->mType)
         {
            case eDB2_FIELD_STD:
            {
               // Standard field, what kind?
               eDB2StdFieldType ft = (eDB2StdFieldType)pField->mTypeVal;
               switch (ft)
               {              
                  // Field is a boolean (0/1, false/true)/8-bit unsigned
                  case eDB2_FIELD_STDTYPE_BOOL:
                  case eDB2_FIELD_STDTYPE_UINT8:
                  {
                     // Treat as UCHAR
                     entry.second = (LONGLONG)theField.mValue.mU8;
                     entry.first = true;
                  }
                  break;

                  // Field is 8-bit signed integer
                  case eDB2_FIELD_STDTYPE_INT8:
                  {
                     // Treat as CHAR
                     entry.second = (LONGLONG)theField.mValue.mS8;
                     entry.first = true;
                  }
                  break;

                  // Field is 16-bit signed integer
                  case eDB2_FIELD_STDTYPE_INT16: 
                  {
                     // Treat as SHORT
                     entry.second = (LONGLONG)theField.mValue.mS16;
                     entry.first = true;
                  }
                  break;

                  // Field is 16-bit unsigned integer
                  case eDB2_FIELD_STDTYPE_UINT16:
                  {
                     // Treat as USHORT
                     entry.second = (LONGLONG)theField.mValue.mU16;
                     entry.first = true;
                  }
                  break;

                  // Field is 32-bit signed integer
                  case eDB2_FIELD_STDTYPE_INT32:
                  {
                     // Treat as LONG
                     entry.second = (LONGLONG)theField.mValue.mS32;
                     entry.first = true;
                  }
                  break;

                  // Field is 32-bit unsigned integer
                  case eDB2_FIELD_STDTYPE_UINT32:
                  {
                     // Treat as ULONG
                     entry.second = (LONGLONG)theField.mValue.mU32;
                     entry.first = true;
                  }
                  break;

                  // Field is 64-bit signed integer
                  case eDB2_FIELD_STDTYPE_INT64:
                  {
                     // Treat as LONGLONG
                     entry.second = (LONGLONG)theField.mValue.mS64;
                     entry.first = true;
                  }
                  break;

                  // Field is 64-bit unsigned integer
                  case eDB2_FIELD_STDTYPE_UINT64:
                  {
                     // Treat as ULONGLONG          
                     if (theField.mValue.mU64 <= LLONG_MAX)
                     {                     
                        entry.second = (LONGLONG)theField.mValue.mU64;
                        entry.first = true;
                     }
                  }
                  break;
               }
            }
            break;

            case eDB2_FIELD_ENUM_UNSIGNED:
            {
               // Treat as ULONG
               entry.second = (LONGLONG)theField.mValue.mU32;
               entry.first = true;
            }
            break;

            case eDB2_FIELD_ENUM_SIGNED:
            {
               // Treat as LONG
               entry.second = (LONGLONG)theField.mValue.mS32;
               entry.first = true;
            }
            break;
         }
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   HandleSpecialCases (Internal Method)

DESCRIPTION:
   Handle special case processing for summary text generation

   NOTE: This should only be added to as a last resort

PARAMETERS:
   args        [I/O] - Current argument list/updated 'special' argument list
   fs          [I/O] - Current format specifier/updated 'special' format 
                       specifier
  
RETURN VALUE:
   None
===========================================================================*/
void cDataParser::HandleSpecialCases(
   std::string &               args,
   std::string &               fs )
{
   std::vector <ULONG> key = mEntity.GetKey();
   if (key.size() == 2 && key[0] == (ULONG)eDB2_ET_DIAG_EVENT)
   {
      ULONG id = key[1];

      mBitsy.SetOffset( 0 );
      ULONG lenInBits = mBitsy.GetNumBitsLeft();
 
      switch (id)
      {
         case 276:
            if (lenInBits == 16)
            {
               // Old style idle handoff event, remap summary
               args = "idle_handoff";
               fs   = "idle_handoff=%u"; 
            }
            break;

         case 277:
            if (lenInBits == 16)
            {
               // Old style access handoff event, remap summary
               args = "ms_access_handoff";
               fs   = "ms_access_handoff=%u"; 
            }
            break;

         case 278:
            if (lenInBits == 16)
            {
               // Old style access probe handoff event, remap summary
               args = "ms_access_probe_handoff";
               fs   = "ms_access_probe_handoff=%u"; 
            }
            break;

         case 639:         
            if (lenInBits == 16)
            {
               // Old style access entry handoff event, remap summary
               args = "ms_access_handoff";
               fs   = "ms_access_handoff=%u"; 
            }
            break;
      }
   }
}

