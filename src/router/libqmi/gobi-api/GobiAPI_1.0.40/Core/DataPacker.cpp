/*===========================================================================
FILE:
   DataPacker.cpp

DESCRIPTION:
   Implementation of sUnpackedField and cDataPacker
   
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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "DataPacker.h"

#include "CoreDatabase.h"
#include "DB2Utilities.h"

#include <climits>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cDataPacker Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cDataPacker (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   db             [ I ] - Database to use
   key            [ I ] - Key into protocol entity table
   fields         [ I ] - Fields to pack into buffer
  
RETURN VALUE:
   None
===========================================================================*/
cDataPacker::cDataPacker( 
   const cCoreDatabase &               db,
   const std::vector <ULONG> &         key,
   const std::list <sUnpackedField> &  fields )
   :  cProtocolEntityNav( db ),
      mKey( key ),
      mbValuesOnly( true ),
      mProcessedFields( 0 ),
      mbPacked( false )
{
   // Initialize internal buffer
   memset( &mBuffer[0], 0, (SIZE_T)MAX_SHARED_BUFFER_SIZE );

   // Compute bits left in buffer
   ULONG bits = MAX_SHARED_BUFFER_SIZE * BITS_PER_BYTE;
   if (mKey.size() > 0)
   {
      eDB2EntityType et = (eDB2EntityType)mKey[0];
      bits = DB2GetMaxBufferSize( et ) * BITS_PER_BYTE;
   }
   
   // Setup the bit packer
   mBitsy.SetData( mBuffer, bits );

   // Copy fields/set value only flag
   std::list <sUnpackedField>::const_iterator pIter = fields.begin();
   while (pIter != fields.end())
   {
      if (pIter->mName.size() > 0)
      {
         mbValuesOnly = false;
      }

      mFields.push_back( *pIter );
      pIter++;
   }
}

/*===========================================================================
METHOD:
   ~cDataPacker (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cDataPacker::~cDataPacker()
{
   // Ask bit packer to release data
   mBitsy.ReleaseData();
}

/*===========================================================================
METHOD:
   Pack (Public Method)

DESCRIPTION:
   Pack the buffer
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataPacker::Pack()
{
   // Process (pack) the protocol entity
   if (mbPacked == false)
   {
      mbPacked = ProcessEntity( mKey );
      if (mbPacked == false)
      {
         // Failed due to no structure ID?
         if (mEntity.IsValid() == true && mEntity.mStructID == -1)
         {
            // Yes, for us that means instant success (no payload to pack)
            mbPacked = true;
         }
      }
   }

   return mbPacked;
}

/*===========================================================================
METHOD:
   GetBuffer (Public Method)

DESCRIPTION:
   Get packed buffer contents
  
PARAMETERS:
   bufferLen   [ O ] - Length of packed buffer (in bytes)

RETURN VALUE:
   const BYTE * - Packed buffer (0 upon error)
===========================================================================*/
const BYTE * cDataPacker::GetBuffer( ULONG & bufferLen )
{
   if (mbPacked == false)
   {
      bufferLen = 0;
      return 0;
   }

   // Payload size in bytes
   bufferLen = mBitsy.GetTotalBitsWritten() + BITS_PER_BYTE - 1;
   bufferLen /= BITS_PER_BYTE;

   // Payload is our buffer
   const BYTE * pBuffer = 0;
   if (bufferLen > 0)
   {
      pBuffer = (const BYTE *)&mBuffer[0];
   }

   return pBuffer;
}

/*===========================================================================
METHOD:
   LoadValues (Static Public Method)

DESCRIPTION:
   Load values by parsing a 'summary' string of values, an example of
   which would be:

      0 1 100 "Foo Foo Foo" 15 -1

PARAMETERS:
   vals        [ I ] - Value string
  
RETURN VALUE:
   std::list <sUnpackedField>
===========================================================================*/
std::list <sUnpackedField> cDataPacker::LoadValues( const std::string & vals )
{
   std::list <sUnpackedField> retList;
   if (vals.size() <= 0)
   {
      return retList;
   }

   std::vector <std::string> tokens;
   ParseCommandLine( vals, tokens );

   std::string name = "";
   std::string val = "";

   std::vector <std::string>::const_iterator pIter = tokens.begin();
   while (pIter != tokens.end())
   {
      val = *pIter++;

      sUnpackedField entry( name, val );
      retList.push_back( entry );
   }

   return retList;
}

/*===========================================================================
METHOD:
   LoadValues (Static Public Method)

DESCRIPTION:
   Load values by parsing a vector of string values, an example of
   which would be:

      [0] 0 
      [1] 1 
      [2] 100 
      [3] "Foo Foo Foo" 
      [4] 15 
      [5] -1

PARAMETERS:
   vals        [ I ] - Vector of values
   startIndex  [ I ] - Where in above vector values start
  
RETURN VALUE:
   std::list <sUnpackedField>
===========================================================================*/
std::list <sUnpackedField> cDataPacker::LoadValues(
   std::vector <std::string> &    vals,
   ULONG                      startIndex )
{
   std::list <sUnpackedField> retList;

   ULONG sz = (ULONG)vals.size();
   if (startIndex >= sz)
   {
      return retList;
   }

   std::string name = "";
   std::string val = "";

   for (ULONG v = startIndex; v < sz; v++)
   {
      val = vals[v];

      sUnpackedField entry( name, val );
      retList.push_back( entry );
   }

   return retList;
}

/*===========================================================================
METHOD:
   GetLastValue (Internal Method)

DESCRIPTION:
   Working from the back of the current value list find and return the 
   value for the specified field ID as a LONGLONG (field type must have 
   been able to fit in a LONGLONG for a value to be stored in value list
   and thus returned)

PARAMETERS:
   fieldID     [ I ] - Field ID we are looking for
   val         [ O ] - The value
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataPacker::GetLastValue( 
   ULONG                      fieldID,
   LONGLONG &                 val )
{
   // Assume failure
   bool bRC = false;

   std::list < std::pair <ULONG, LONGLONG> >::reverse_iterator pValues;
   pValues = mValues.rbegin();
   while (pValues != mValues.rend())
   {
      std::pair <ULONG, LONGLONG> & entry = *pValues;
      if (entry.first == fieldID)
      {
         val = entry.second;

         // Success!
         bRC = true;
         break;
      }

      pValues++;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetValueString (Internal Method)

DESCRIPTION:
   For the given field return the (input) value string

PARAMETERS:
   field          [ I ] - The field being processed
   fieldName      [ I ] - Partial (or fully qualified) field name
   pValueString   [ O ] - Value string

RETURN VALUE:
   bool
===========================================================================*/
bool cDataPacker::GetValueString( 
   const sDB2Field &         field,
   const std::string &       fieldName,
   LPCSTR &                  pValueString )
{
   // Assume failure
   pValueString = 0;

   // Create fully qualified field name
   std::string fullName = GetFullFieldName( fieldName );

   std::vector <sUnpackedField>::const_iterator pVals = mFields.begin();
   while (pVals != mFields.end())
   {
      const std::string & inName = pVals->mName;
      if (fieldName == inName || fullName == inName)
      {
         pValueString = (LPCSTR)pVals->mValueString.c_str();
         break;
      }

      pVals++;
   }

   // Value provided?
   if (pValueString == 0)
   {  
      // No, are we in value only mode?
      if (mbValuesOnly == true)
      {
         if (mProcessedFields < (ULONG)mFields.size())
         {
            sUnpackedField & upf = mFields[mProcessedFields++];

            // Set field name (partial)
            upf.mName = fieldName;

            // Grab field value
            pValueString = (LPCSTR)upf.mValueString.c_str();
         }
      }
      else
      {
         return false;
      }
   }

   // Value provided?
   if (pValueString == 0)
   {
      return false;
   }
      
   // Meaningful value provided?   
   if (pValueString[0] == 0)
   {
      // No value provided for field?  Is it a string?
      if (field.mType == eDB2_FIELD_STD)
      {
         if ( (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_A)
         &&   (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_U)
         &&   (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_U8)
         &&   (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_ANT)
         &&   (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_UNT)
         &&   (field.mTypeVal != eDB2_FIELD_STDTYPE_STRING_U8NT) )
         {
            // No, unable to proceed
            return false;
         }
      }
   }

   return true;
}

/*===========================================================================
METHOD:
   PackString (Internal Method)

DESCRIPTION:
   Pack the string (described by the given arguments) into the buffer
  
PARAMETERS:
   numChars    [ I ] - Number of characters to pack (0 = NULL terminated)
   pStr        [ I ] - String to pack

RETURN VALUE:
   bool
===========================================================================*/
bool cDataPacker::PackString( 
   ULONG                      numChars,
   LPCSTR                     pStr )
{
   // Sanity check string pointer
   if (pStr == 0)
   {
      return false;
   }

   // Assume success
   bool bOK = true;

   // Convert native string type to desired output type
   const BYTE * pTmp = (const BYTE *)pStr;

   // Have we reached the practical end of a fixed length string?
   ULONG numBytes = 0;

   numBytes = (ULONG)strlen( (LPCSTR)pTmp );
   if (numChars == 0)
   {
      // We pack the string plus the NULL character
      numChars = (ULONG)strlen( (LPCSTR)pTmp ) + 1;
   }

   // String size too long?
   if (numBytes > numChars)
   {      
      return false;
   }

   // Pack the string one byte at a time
   for (ULONG c = 0; c < numChars; c++)
   {
      BYTE val = 0;
      if (c < numBytes)
      {
         val = pTmp[c];
      }

      DWORD rc = mBitsy.Set( BITS_PER_BYTE, val );
      if (rc != NO_ERROR)
      {     
         bOK = false;
         break;
      }
   }   

   return bOK;
}

/*===========================================================================
METHOD:
   ProcessField (Internal Method)

DESCRIPTION:
   Process the given field (described by the given arguments) by packing 
   the value into the buffer
  
PARAMETERS:
   pField      [ I ] - The field being processed
   fieldName   [ I ] - Field name (partial)
   arrayIndex  [ I ] - Not used
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDataPacker::ProcessField( 
   const sDB2Field *          pField,
   const std::string &            fieldName,
   LONGLONG                   /* arrayIndex */ )
{
   // Assume failure
   bool bOK = false;
   if (pField == 0)
   {
      return bOK;
   }

   // Find given value for field
   LPCSTR pVal = 0;
   bool bVal = GetValueString( *pField, fieldName, pVal );
   if (bVal == false)
   {
      return bOK;
   }

   // Grab field ID
   ULONG id = pField->mID;

   // What type is this field?    
   switch (pField->mType)
   {
      case eDB2_FIELD_STD:
      {
         // Standard field, what kind?
         eDB2StdFieldType ft = (eDB2StdFieldType)pField->mTypeVal;
         switch (ft)
         {              
            // Field is a boolean (0/1, false/true)/8-bit unsigned integer
            case eDB2_FIELD_STDTYPE_BOOL:
            case eDB2_FIELD_STDTYPE_UINT8:
            {
               // We pack as a UCHAR
               UCHAR val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  if (ft == eDB2_FIELD_STDTYPE_BOOL && val > 1)
                  {
                     val = 1;
                  }

                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 8-bit signed integer
            case eDB2_FIELD_STDTYPE_INT8:
            {
               // We pack as a CHAR
               CHAR val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!         
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 16-bit signed integer
            case eDB2_FIELD_STDTYPE_INT16: 
            {
               // We pack as a SHORT
               SHORT val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!               
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 16-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT16:
            {
               // We pack as a USHORT
               USHORT val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  } 
               }
            }
            break;

            // Field is 32-bit signed integer
            case eDB2_FIELD_STDTYPE_INT32:
            {
               // We pack as a LONG
               LONG val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 32-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT32:
            {              
               // We pack as a ULONG
               ULONG val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 64-bit signed integer
            case eDB2_FIELD_STDTYPE_INT64:
            {
               // We pack as a LONGLONG
               LONGLONG val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     std::pair <ULONG, LONGLONG> entry( id, val );
                     mValues.push_back( entry );
                     bOK = true;
                  }
               }
            }
            break;

            // Field is 64-bit unsigned integer
            case eDB2_FIELD_STDTYPE_UINT64:
            {
               // We pack as a ULONGLONG
               ULONGLONG val = 0;
               bool bVal = ::FromString( pVal, val );
               if (bVal == true)
               {
                  DWORD rc = mBitsy.Set( pField->mSize, val );
                  if (rc == NO_ERROR)
                  {         
                     // Success!
                     if (val <= LLONG_MAX)
                     {                     
                        std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
                        mValues.push_back( entry );
                     }

                     bOK = true;
                  }
               }
            }
            break;

            // ANSI/UNICODE strings
            case eDB2_FIELD_STDTYPE_STRING_A:
            case eDB2_FIELD_STDTYPE_STRING_U:
            case eDB2_FIELD_STDTYPE_STRING_ANT:
            case eDB2_FIELD_STDTYPE_STRING_UNT:
            {
               // Set the character size
               ULONG charSz = sizeof(CHAR);
               if ( (ft == eDB2_FIELD_STDTYPE_STRING_U)
               ||   (ft == eDB2_FIELD_STDTYPE_STRING_UNT) )
               {
                  charSz = sizeof(USHORT);
               }

               // Compute the number of characters?
               ULONG numChars = 0;
               if ( (ft == eDB2_FIELD_STDTYPE_STRING_A)
               ||   (ft == eDB2_FIELD_STDTYPE_STRING_U) )
               {
                  numChars = (pField->mSize / BITS_PER_BYTE) / charSz;
               }

               // Pack the string
               bOK = PackString( numChars, pVal );
            }
            break;

            // UTF-8 strings
            case eDB2_FIELD_STDTYPE_STRING_U8:
            case eDB2_FIELD_STDTYPE_STRING_U8NT:
            {
               // Unsupported in the Linux adaptation
               bOK = false;
            }
            break;

            // Field is 32-bit floating point value
            case eDB2_FIELD_STDTYPE_FLOAT32:
            {
               // We pack as a ULONG
               FLOAT val = (float)atof( (LPCSTR)pVal );
               ULONG * pTmp = (ULONG *)&val;

               DWORD rc = mBitsy.Set( pField->mSize, *pTmp );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  bOK = true;
               }
            }
            break;

            // Field is 64-bit floating point value
            case eDB2_FIELD_STDTYPE_FLOAT64:
            {
               // We pack as a ULONGLONG
               double val = atof( (LPCSTR)pVal );
               ULONGLONG * pTmp = (ULONGLONG *)&val;

               DWORD rc = mBitsy.Set( pField->mSize, *pTmp );
               if (rc == NO_ERROR)
               {         
                  // Success!
                  bOK = true;
               }
            }
            break;

            default:
            {
               bOK = false;                  
            }
            break;
         }
      }
      break;

      case eDB2_FIELD_ENUM_UNSIGNED:
      {
         // We pack as a ULONG
         ULONG val = 0;
         bool bVal = ::FromString( pVal, val );
         if (bVal == true)
         {
            DWORD rc = mBitsy.Set( pField->mSize, val );
            if (rc == NO_ERROR)
            {         
               // Success!
               std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
               mValues.push_back( entry );
               bOK = true;
            }
         }
      }
      break;

      case eDB2_FIELD_ENUM_SIGNED:
      {
         // We pack as a LONG
         LONG val = 0;
         bool bVal = ::FromString( pVal, val );
         if (bVal == true)
         {
            DWORD rc = mBitsy.Set( pField->mSize, val );
            if (rc == NO_ERROR)
            {         
               // Success!
               std::pair <ULONG, LONGLONG> entry( id, (LONGLONG)val );
               mValues.push_back( entry );
               bOK = true;
            }
         }
      }
      break;

      default:
      {
         bOK = false;                  
      }
      break;
   }      

   return bOK;
}
