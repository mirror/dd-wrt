/*===========================================================================
FILE:
   ProtocolEntityNav.cpp

DESCRIPTION:
   Implementation of cProtocolEntityNav
   
PUBLIC CLASSES AND METHODS:
   cProtocolEntityNav
      This calss serves as a base for all class that need to
      'navigate' a protocol entity database description.  It is
      necessary in order to seperate the structural aspects 
      from parsing/packing details

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
#include "ProtocolEntityNav.h"
#include "BitParser.h"
#include "CoreUtilities.h"
#include "DB2NavTree.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Field seperator string
LPCSTR PE_NAV_FIELD_SEP = ".";

/*=========================================================================*/
// cProtocolEntityNav Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cProtocolEntityNav (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   db             [ I ] - Database to use
   bSummaryOnly   [ I ] - Only navigate if a format specifier exists?
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolEntityNav::cProtocolEntityNav( const cCoreDatabase & db )
   :  mDB( db ),
      mbFieldNames( true ),
      mConditions( db.GetOptionalMods() ),
      mExpressions( db.GetExpressionMods() ),
      mArrays1( db.GetArray1Mods() ),
      mArrays2( db.GetArray2Mods() )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cProtocolEntityNav (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cProtocolEntityNav::~cProtocolEntityNav()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   EvaluateCondition (Internal Method)

DESCRIPTION:
   Evaluate the given condition
  
PARAMETERS:
   pCondition  [ I ] - Condition to evaluate
   bResult     [ O ] - Result of evaluating the condition (true/false)

RETURN VALUE:
   bool :
      true  - We were able to evaluate the condition
      false - Unable to evaluate condition
===========================================================================*/
bool cProtocolEntityNav::EvaluateCondition( 
   LPCSTR                    pCondition,
   bool &                     bResult )
{
   // Assume error
   bool bRC = false;

   tDB2OptionalModMap::const_iterator pIter;
   pIter = mConditions.find( pCondition );

   if (pIter != mConditions.end())
   {
      const sDB2SimpleCondition & con = pIter->second;

      // Grab the value for the given field ID
      LONGLONG valA = 0;
      bRC = GetLastValue( con.mID, valA );

      // Field to field?
      LONGLONG valB = con.mValue;
      if (con.mbF2F == true)
      {
         // Yes, grab value of the second field
         bRC &= GetLastValue( (ULONG)con.mValue, valB );
      }

      if (bRC == true)
      {
         bResult = sDB2Fragment::EvaluateCondition( valA, 
                                                    con.mOperator, 
                                                    valB );
      }
      else
      {
         // We could not find the field used in the condition, this
         // can either be because of a bad entity (which is ruled
         // out prior to reaching this point) or the existence of
         // the field itself is based on another condition.  The 
         // former should not happen and the later is not an error         
         bResult = false;
         bRC = true;
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetArrayBounds (Internal Method)

DESCRIPTION:
   Get the array bounds described by the fragment descriptor

PARAMETERS:
   frag        [ I ] - Fragment descriptor
   arraySz     [ O ] - Size of array
   arrayAdj    [ O ] - Adjust for array indices 
  
RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolEntityNav::GetArrayBounds( 
   const sDB2Fragment &       frag,
   LONGLONG &                 arraySz,
   LONGLONG &                 arrayAdj )
{
   // Assume failure
   bool bRC = false;

   // Figure out the array size/adjust
   arraySz = 0;
   arrayAdj = 0;

   switch (frag.mModifierType)
   {
      case eDB2_MOD_CONSTANT_ARRAY:
      {
         tDB2Array1ModMap::const_iterator pIter;
         pIter = mArrays1.find( frag.mpModifierValue );

         if (pIter != mArrays1.end())
         {
            arraySz = (LONGLONG)pIter->second;
            bRC = true;
         }
      }
      break;

      case eDB2_MOD_VARIABLE_ARRAY:      
      {
         tDB2Array1ModMap::const_iterator pIter;
         pIter = mArrays1.find( frag.mpModifierValue );

         if (pIter != mArrays1.end())
         {
            ULONG id = pIter->second;

            // Now find last occurence of this field ID and grab the value
            bRC = GetLastValue( id, arraySz );                
            if (bRC == true)
            {
               // It makes no sense to have a negative sized array
               if (arraySz < 0)
               {          
                  bRC = false;
               }
            }
         }
      }
      break;

      case eDB2_MOD_VARIABLE_ARRAY2:
      {
         tDB2Array2ModMap::const_iterator pIter;
         pIter = mArrays2.find( frag.mpModifierValue );

         if (pIter != mArrays2.end())
         {
            ULONG sID = pIter->second.first;
            ULONG eID = pIter->second.second;

            LONGLONG s;
            LONGLONG e;

            // Now find last occurence of these field IDs and
            // grab the values
            bRC = GetLastValue( sID, s );
            bRC &= GetLastValue( eID, e );
            if (bRC == true)
            {
               // It makes no sense to have an negative sized array
               if (e < s)
               {          
                  bRC = false;
               }
               else
               {
                  arrayAdj = s;
                  arraySz = (e - s) + 1;
               }
            }
         }
      }
      break;

      case eDB2_MOD_VARIABLE_ARRAY3:
      {
         tDB2ExpressionModMap::const_iterator pIter;
         pIter = mExpressions.find( frag.mpModifierValue );

         if (pIter != mExpressions.end())
         {
            const sDB2SimpleExpression & expr = pIter->second;

            // Grab the value for the given field ID
            LONGLONG valA = 0;
            bRC = GetLastValue( expr.mID, valA );

            // Field to field?
            LONGLONG valB = expr.mValue;
            if (expr.mbF2F == true)
            {
               // Yes, grab value of the second field
               bRC &= GetLastValue( (ULONG)expr.mValue, valB );
            }

            if (bRC == true)
            {
               bRC = sDB2Fragment::EvaluateExpression( valA, 
                                                       expr.mOperator, 
                                                       valB,
                                                       arraySz );

               // It makes no sense to have a negative sized array
               if (bRC == true && arraySz < 0)
               {          
                  bRC = false;
               }
            }
         }
      }
      break;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ModifyStringLength (Internal Method)

DESCRIPTION:
   Modify string length based on existing field value, at the end
   of this function the field size will be the string length in bits

PARAMETERS:
   frag        [ I ] - Fragment descriptor
   field       [ O ] - Field to modify
  
RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolEntityNav::ModifyStringLength( 
   const sDB2Fragment &       frag,
   sDB2Field &                field )
{
   // Assume failure
   bool bRC = false;

   if (field.mType != eDB2_FIELD_STD)
   {
      // Why are we here?
      ASSERT( 0 );
      return false;
   }

   if ( (field.mTypeVal != (ULONG)eDB2_FIELD_STDTYPE_STRING_A)
   &&   (field.mTypeVal != (ULONG)eDB2_FIELD_STDTYPE_STRING_U)
   &&   (field.mTypeVal != (ULONG)eDB2_FIELD_STDTYPE_STRING_U8) )
   {
      // Why are we here?
      ASSERT( 0 );
      return false;
   }

   if ( (frag.mModifierType == eDB2_MOD_VARIABLE_STRING3)
   &&   (field.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_U8) )
   {
      // We can't have the size specified in characters when the
      // size of the character itself is variable length
      ASSERT( 0 );
      return false;
   }

   tDB2Array1ModMap::const_iterator pIter;
   pIter = mArrays1.find( frag.mpModifierValue );
   if (pIter == mArrays1.end())
   {
      // Unable to obtain string length
      return bRC;
   }

   ULONG id = pIter->second;

   // Now find last occurence of this field ID and grab the value
   LONGLONG strSz;
   bRC = GetLastValue( id, strSz );                
   if (bRC == false || strSz < 0)
   {
      // Unable to obtain size or invalid size
      bRC = false;
      return bRC;
   }

   // Compute character size
   ULONG charSz = BITS_PER_BYTE;
   if (field.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_U)
   {
      charSz *= 2;
   }

   if (frag.mModifierType == eDB2_MOD_VARIABLE_STRING2)
   {
      strSz *= BITS_PER_BYTE;
   }
   else if (frag.mModifierType == eDB2_MOD_VARIABLE_STRING3)
   {
      strSz *= charSz;
   }

   if (strSz > ULONG_MAX)
   {
      // String length far too large
      bRC = false;
      return bRC;
   }

   if (strSz != 0)
   {
      if (strSz < charSz || (strSz % charSz) != 0)
      {
         // String length not a proper multiple of character size
         bRC = false;
         return bRC;
      }
   }

   field.mSize = (ULONG)strSz;
   return bRC;
}

/*===========================================================================
METHOD:
   ProcessEntity (Internal Method)

DESCRIPTION:
   Process a protocol entity

PARAMETERS:
   key         [ I ] - Key into the protocol entity table
  
RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolEntityNav::ProcessEntity( const std::vector <ULONG> & key )
{
   // Assume failure
   bool bRC = false;

   // Look up entity definition
   const cDB2NavTree * pNavTree = mDB.GetEntityNavTree( key );

   // Did we find it?
   if (pNavTree == 0)
   {
      return bRC;
   }

   // Is it valid?
   mEntity = pNavTree->GetEntity();
   if (mEntity.IsValid() == false)
   {
      // No definition in database
      return bRC;
   }

   // Check if we should continue
   if (ContinueNavigation() == false)
   {
      // Success!
      bRC = true;
      return bRC;
   }

   // A structure to navigate?
   if (mEntity.mStructID == -1)
   {
      // Success!
      bRC = true;
      return bRC;
   }

   // Grab navigation fragments   
   const std::list <sDB2NavFragment *> & frags = pNavTree->GetFragments();

   // Nothing to navigate?
   if (frags.size() == 0)
   {
      ASSERT( 0 );
      return bRC;
   }

   // No name?
   if (mEntity.mpName == 0 || mEntity.mpName[0] == 0)
   {
      ASSERT( 0 );
      return bRC;
   }

   // Grab tracked fields
   mTrackedFields = pNavTree->GetTrackedFields();

   std::string preamble = "";

   // Process the initial structure
   EnterStruct( mEntity.mpName, -1 );
   bRC = ProcessStruct( frags.front(), preamble, -1 );
   ExitStruct( mEntity.mpName, -1 );
   
   return bRC;
}

/*===========================================================================
METHOD:
   ProcessStruct (Internal Method)

DESCRIPTION:
   Process a structure described by the given initial fragment

PARAMETERS:
   pFrag       [ I ] - First fragment in structure
   preamable   [ I ] - String to prepend to any field/struct names
   arrayIndex  [ I ] - Array index (-1 = not part of an array)
  
RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolEntityNav::ProcessStruct(
   const sDB2NavFragment *       pFrag,
   const std::string &           preamble,
   LONGLONG                      /* arrayIndex */ )
{
   // Assume success
   bool bRC = true;
  
   ULONG structSz = 0;
   ULONG structOffset = GetOffset();

   // Grab current navigation order
   bool bOldLSB = GetLSBMode();
   bool bNewLSB = bOldLSB;

   // Check for directives
   if (pFrag != 0)
   {
      bool bDirective = false;

      const sDB2Fragment & frag = *pFrag->mpFragment;
      if (frag.mFragmentType == eDB2_FRAGMENT_MSB_2_LSB)
      {
         bDirective = true;
         if (bOldLSB == true)
         {
            bNewLSB = false;
            bRC = SetLSBMode( bNewLSB );
         }
      }

      if (frag.mFragmentType == eDB2_FRAGMENT_LSB_2_MSB)
      {
         bDirective = true;
         if (bOldLSB == false)
         {
            bNewLSB = true;
            bRC = SetLSBMode( bNewLSB );
         }
      }

      if (bDirective == true)
      {
         // We process directives here so move on to the next fragment
         // upon success
         if (bRC == true)
         {
            pFrag = pFrag->mpNextFragment;
         }
         else
         {
            pFrag = 0;
         }
      }      
   }

   // Process each fragment in the structure
   while (pFrag != 0)
   { 
      bRC = ProcessFragment( pFrag, structOffset, structSz, preamble );
      if (bRC == true)
      {
         pFrag = pFrag->mpNextFragment;
      }
      else
      {
         break;
      }
   }

   // Restore navigation order
   if (bRC == true && bOldLSB != bNewLSB)
   {
      bRC = SetLSBMode( bOldLSB );
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ProcessFragment (Internal Method)

DESCRIPTION:
   Process the given fragment

PARAMETERS:
   pFrag          [ I ] - Fragment to be processed
   structOffset   [ I ] - Offset (from start of payload) of enclosing struct
   structSize     [ I ] - Current size of enclosing struct
   preamble       [ I ] - String to prepend to any field/struct names
  
RETURN VALUE:
   bool
===========================================================================*/
bool cProtocolEntityNav::ProcessFragment(
   const sDB2NavFragment *       pFrag,
   ULONG                         structOffset,
   ULONG &                       structSize,
   const std::string &           preamble )
{
   // Assume failure
   bool bRC = false;
   if (pFrag == 0 || pFrag->mpFragment == 0)
   {
      return bRC;
   }

   // Grab database fragment
   const sDB2Fragment & frag = *pFrag->mpFragment;

   // Is this fragment optional?   
   if (frag.mModifierType == eDB2_MOD_OPTIONAL)
   {      
      bool bParse = false;
      bool bOK = EvaluateCondition( frag.mpModifierValue, bParse );
      if (bOK == false)
      {
         // Error evaluating the condition
         bRC = false;
         return bRC;
      }

      if (bParse == false)
      {
         // Condition not satisfied, nothing to parse
         bRC = true;
         return bRC;
      }
   }

   // Is this an array?
   LONGLONG arraySz = -1;
   LONGLONG arrayAdj = 0;
   bool bArray = ModifiedToArray( frag.mModifierType );
   if (bArray == true)
   {
      bool bOK = GetArrayBounds( frag, arraySz, arrayAdj );
      if (bOK == false)
      {
         // Error obtaining array dimensions
         bRC = false;
         return bRC;
      }
      else if (arraySz == 0)
      {
         // No array to process
         bRC = true;
         return bRC;
      }
   }

   // Set base name
   std::string baseName = "";
   if (mbFieldNames == true)
   {
      baseName = preamble;

      // Add in fragment name?
      if (frag.mpName != EMPTY_STRING)
      {
         if (baseName.size() > 0)
         {
            baseName += PE_NAV_FIELD_SEP;
         }

         // Yes, add to the preamble            
         baseName += frag.mpName;
      }
   }

   // Is this fragment offset?
   if (frag.mFragmentOffset != -1)
   {
      // Yes, add in offset to structure offset and save
      ULONG newOffset = frag.mFragmentOffset + structOffset;
      SetOffset( newOffset );
   }   

   // What type of fragment is this?
   switch (frag.mFragmentType)
   {
      case eDB2_FRAGMENT_FIELD:
      {
         const sDB2Field * pField = pFrag->mpField;
         if (pField != 0)
         {
            if (mbFieldNames == true)
            {
               if (baseName.size() > 0)
               {
                  baseName += PE_NAV_FIELD_SEP;
               }

               // Add in field name
               baseName += pField->mpName;
            }

            // Variable string?
            sDB2Field modField;
            if ( (frag.mModifierType == eDB2_MOD_VARIABLE_STRING1)
            ||   (frag.mModifierType == eDB2_MOD_VARIABLE_STRING2)
            ||   (frag.mModifierType == eDB2_MOD_VARIABLE_STRING3) )
            {
               modField = *pField;
               bRC = ModifyStringLength( frag, modField );
               if (bRC == false)
               {
                  // Unable to obtain string length
                  return bRC;
               }

               if (modField.mSize == 0)
               {
                  // String has no length - treat like an optional fragment
                  bRC = true;
                  return bRC;
               }

               pField = &modField;
            }

            // Handle an array?
            if (bArray == true)
            {
               EnterArray( frag, arraySz );

               if (mbFieldNames == true)
               {
                  ULONG baseLen = baseName.size();

                  std::string fieldName;
                  fieldName.reserve( baseLen + 16 );
                  fieldName = baseName;

                  CHAR arraySpec[32];

                  for (LONGLONG i = 0; i < arraySz; i++)
                  { 
                     snprintf( arraySpec, 31, "[%lld]", i + arrayAdj );
                     fieldName += arraySpec;

                     bRC = ProcessField( pField, fieldName, i );
                     if (bRC == false)
                     {                  
                        break;
                     }

                     // Remove the array specifier for the next pass
                     fieldName.resize( baseLen );
                  }
               }
               else
               {
                  for (LONGLONG i = 0; i < arraySz; i++)
                  { 
                     bRC = ProcessField( pField, baseName, i );
                     if (bRC == false)
                     {                  
                        break;
                     }
                  }
               }

               ExitArray( frag, arraySz );
            }
            else
            {
               bRC = ProcessField( pField, baseName );
            }
         }
      }
      break;

      case eDB2_FRAGMENT_STRUCT:                     
      {
         if (pFrag->mpLinkFragment != 0)
         {
            // Handle an array?
            if (bArray == true)
            {
               EnterArray( frag, arraySz );

               if (mbFieldNames == true)
               {
                  ULONG baseLen = baseName.size();

                  std::string structName;
                  structName.reserve( baseLen + 16 );
                  structName = baseName;

                  CHAR arraySpec[32];

                  for (LONGLONG i = 0; i < arraySz; i++)
                  {          
                     snprintf( arraySpec, 31, "[%lld]", i + arrayAdj );
                     structName += arraySpec;

                     EnterStruct( frag.mpName, i );

                     bRC = ProcessStruct( pFrag->mpLinkFragment, 
                                          structName, 
                                          i );

                     ExitStruct( frag.mpName, i );

                     if (bRC == false)
                     {                  
                        break;
                     } 

                     // Remove the array specifier for the next pass
                     structName.resize( baseLen );
                  }
               }
               else
               {

                  for (LONGLONG i = 0; i < arraySz; i++)
                  {          
                     EnterStruct( frag.mpName, i );

                     bRC = ProcessStruct( pFrag->mpLinkFragment, 
                                          baseName, 
                                          i );

                     ExitStruct( frag.mpName, i );

                     if (bRC == false)
                     {                  
                        break;
                     } 
                  }
               }

               ExitArray( frag, arraySz );
            }
            else
            {
               EnterStruct( frag.mpName, -1 );
               bRC = ProcessStruct( pFrag->mpLinkFragment, baseName );
               ExitStruct( frag.mpName, -1 );
            }
         }
      }
      break;

      case eDB2_FRAGMENT_CONSTANT_PAD:
      {
         // Is the structure is smaller than the specified
         // value that we are to pad out to?
         ULONG totalSz = frag.mFragmentValue;
         if (totalSz >= structSize)
         {
            ULONG newOffset = structOffset + totalSz;
            SetOffset( newOffset );
         }

         // Succcess!
         bRC = true;
      }
      break;

      case eDB2_FRAGMENT_VARIABLE_PAD_BITS:
      case eDB2_FRAGMENT_VARIABLE_PAD_BYTES:
      {
         // Find last occurence of this field ID and grab the value
         LONGLONG totalSz = 0;
         bRC = GetLastValue( frag.mFragmentValue, totalSz );
         if (bRC == true)
         {
            // Convert to bits?
            if (frag.mFragmentType == eDB2_FRAGMENT_VARIABLE_PAD_BYTES)
            {
               totalSz *= BITS_PER_BYTE;
            }

            // Is the structure is smaller than the specified
            // value that we are to pad out to?
            if ((ULONG)totalSz >= structSize)
            {
               ULONG newOffset = structOffset + (ULONG)totalSz; 
               SetOffset( newOffset );
            }
         }
      }
      break;

      case eDB2_FRAGMENT_FULL_BYTE_PAD:
      {
         ULONG totalSz = structSize;
         while ((totalSz % BITS_PER_BYTE) != 0)
         {
            totalSz++;
         }

         if (totalSz > structSize)
         {
            ULONG newOffset = structOffset + totalSz;
            SetOffset( newOffset );
         }

         // Succcess!
         bRC = true;
      }
      break;

      default:
         bRC = false;
         break;
   }
      
   // Adjust struct size?
   if (bRC == true)
   {
      ULONG newOffset = GetOffset();
      if (newOffset > structOffset)
      {
         ULONG newSz = newOffset - structOffset;
         if (newSz > structSize)
         {
            structSize = newSz;
         }
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetPartialFieldName (Public Method)

DESCRIPTION:
   Return the fully qualified field name given the partial name

PARAMETERS:
   partialName [ I ] - Partial field name
  
RETURN VALUE:
   std::string
===========================================================================*/
std::string cProtocolEntityNav::GetFullFieldName( 
   const std::string &            partialName ) const
{
   std::string retStr = EMPTY_STRING;

   if (mEntity.mpName != 0 && mEntity.mpName != EMPTY_STRING)
   {
      retStr = mEntity.mpName;
      retStr += PE_NAV_FIELD_SEP;
      retStr += partialName;
   }

   return retStr;
}

/*===========================================================================
METHOD:
   GetPartialFieldName (Public Method)

DESCRIPTION:
   Return the partial field name given the fully qualified name

PARAMETERS:
   fieldNameFQ [ I ] - Fully qualified name
  
RETURN VALUE:
   std::string
===========================================================================*/
std::string cProtocolEntityNav::GetPartialFieldName( 
   const std::string &            fieldNameFQ ) const
{
   std::string retStr = EMPTY_STRING;

   if (mEntity.mpName != 0 && mEntity.mpName != EMPTY_STRING)
   {
      int idx = fieldNameFQ.find( mEntity.mpName, 0 );
      if (idx == 0)
      {
         idx = fieldNameFQ.find( PE_NAV_FIELD_SEP );
         if (idx != -1)
         {
            retStr = fieldNameFQ.substr( idx - 1 );
         }
         
      }
   }

   return retStr;
}
