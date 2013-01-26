/*===========================================================================
FILE: 
   CoreDatabase.cpp

DESCRIPTION:
   Implementation of cCoreDatabase class

PUBLIC CLASSES AND METHODS:
   cCoreDatabase
      This class represents the run-time (read only) version of the 
      core library database

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

#include "CoreDatabase.h"
#include "DB2NavTree.h"

#include "CoreUtilities.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Uncomment out to enable database load/save timing through cCoreDatabase
// #define TIME_DB 1

// Database table file names
LPCSTR DB2_FILE_PROTOCOL_FIELD   = "Field.txt";
LPCSTR DB2_FILE_PROTOCOL_STRUCT  = "Struct.txt";
LPCSTR DB2_FILE_PROTOCOL_ENTITY  = "Entity.txt";
LPCSTR DB2_FILE_ENUM_MAIN        = "Enum.txt";
LPCSTR DB2_FILE_ENUM_ENTRY       = "EnumEntry.txt";

// Database table file names
LPCSTR DB2_TABLE_PROTOCOL_FIELD  = "Field";
LPCSTR DB2_TABLE_PROTOCOL_STRUCT = "Struct";
LPCSTR DB2_TABLE_PROTOCOL_ENTITY = "Entity";
LPCSTR DB2_TABLE_ENUM_MAIN       = "Enum";
LPCSTR DB2_TABLE_ENUM_ENTRY      = "Enum Entry";

// An empty (but not NULL) string
LPCSTR EMPTY_STRING = "";

// Value seperator for database text
LPCSTR DB2_VALUE_SEP = "^";

// Sub-value (i.e. within a particular value) seperator for database text
LPCSTR DB2_SUBVAL_SEP = ",";

// Maximum amount of recursion allowed in protocol entity structure processing
const ULONG MAX_NESTING_LEVEL = 32;

// The default logger (for backwards compatibility)
cDB2TraceLog gDB2DefaultLog;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   CopyQuotedString (Public Method)

DESCRIPTION:
   Convert a string (in quotes) to a string (minus) quotes and copy
   into an allocated buffer
  
PARAMETERS:
   pString     [ I ] - The string being de-quoted/copied

RETURN VALUE:
   LPSTR: The copy (returns 0 upon error)
===========================================================================*/
LPCSTR CopyQuotedString( LPSTR pString )
{
   // Get string length
   ULONG len = (ULONG)strlen( pString );

   // Adjust to remove trailing spaces
   while (len > 0 && pString[len - 1] == ' ')
   {
      pString[len - 1] = 0;
      len--;
   }

   // Long enough (and quoted?)
   if ( (len >= 2)
   &&   (pString[0] == '\"')
   &&   (pString[len - 1] == '\"') )
   {
      if (len == 2)
      {
         return EMPTY_STRING;
      }
      else
      {
         // Attempt to allocate a copy
         LPSTR pRet = new char[len - 1];
         if (pRet != 0)
         {  
            ULONG bytes = (len - 2) * sizeof( char );
            memcpy( (PVOID)pRet, (LPCVOID)&pString[1], (SIZE_T)bytes );
            pRet[len - 2] = 0;

            return pRet;
         }
      }
   }

   return 0;
}

/*=========================================================================*/
// sDB2ProtocolEntity Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2ProtocolEntity::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be
   //   0: Type
   //   1: "Key"
   //   2: "Name"
   //   3: Struct ID
   //   4: Format specifier ID (optional)
   //   5: Internal only flag (optional)
   //   6: Extended format specifier ID (optional)
   const ULONG NUM_REQ_VALS = 4;
   
   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {    
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[2] );
      if (pCopy != 0)
      {      
         mpName    = pCopy;
         mType     = (eDB2EntityType)strtol( tokens[0], 0, 10 );

         // Convert key/populate ID
         mID.push_back( (ULONG)mType );
         CSVStringToContainer( DB2_SUBVAL_SEP, tokens[1], mID, false );    

         mStructID = strtol( tokens[3], 0, 10  );
         
         // Format specifier?
         if (toks > NUM_REQ_VALS)
         {
            mFormatID = strtol( tokens[NUM_REQ_VALS], 0, 10  );
         }

         // Internal only flag?
         if (toks > NUM_REQ_VALS + 1)
         {
            mbInternal = (strtoul( tokens[NUM_REQ_VALS + 1], 0, 10 ) != 0);
         }

         // Extended format specifier ID?
         if (toks > NUM_REQ_VALS + 2)
         {
            mFormatExID = strtol( tokens[NUM_REQ_VALS + 2], 0, 10 );
         }

         bRC = IsValid();
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2ProtocolEntity::IsValid() const
{
   // The type has to be valid
   if (::IsValid( mType ) == false)
   {
      return false;
   }

   // The ID must consists of at least two entries
   if (mID.size() < 2)
   {
      return false;
   }

   // The first entry in the ID has to be the type
   if (mID[0] != (ULONG)mType)
   {
      return false;
   }

   // The structure ID has to be >= -1)
   if (mStructID < -1)
   {
      return false;
   }      

   // The format specifier has to be >= -1)
   if (mFormatID < -1)
   {
      return false;
   }      

   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// sDB2Fragment Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be 
   //    0: ID
   //    1: Order
   //    2: Type
   //    3: Val
   //    4: "Name"
   //    5: Offset
   //    6: Mod Type
   //    7: "Mod Value"
   const ULONG NUM_REQ_VALS = 8;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {
      // Remove quotes from modifier value and copy
      LPCSTR pVal = CopyQuotedString( tokens[7] );
      if (pVal != 0)
      {
         // Remove quotes from name string and copy
         LPCSTR pCopy = CopyQuotedString( tokens[4] );
         if (pCopy != 0)
         {
            mStructID       = strtoul( tokens[0], 0, 10  );
            mFragmentOrder  = strtoul( tokens[1], 0, 10  );
            mFragmentValue  = strtoul( tokens[3], 0, 10  );
            mFragmentOffset = strtol( tokens[5], 0, 10  );
            mFragmentType   = (eDB2FragmentType)strtol( tokens[2], 0, 10 );
            mModifierType   = (eDB2ModifierType)strtol( tokens[6], 0, 10 );;
            mpModifierValue = pVal;
            mpName          = pCopy;

            bRC = IsValid();
         }
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::IsValid() const
{
   // The fragment type has to be valid
   if (::IsValid( mFragmentType ) == false)
   {
      return false;
   }

   // The modifier type has to be valid
   if (::IsValid( mModifierType ) == false)
   {
      return false;
   }

   // There has to be a name (possibly empty)
   if (mpName == 0)
   {
      return false;
   }

   // There has to be a modifier value (possibly empty)
   if (mpModifierValue == 0)
   {
      return false;
   }

   // Directives can only be given for the first fragment
   if ( (mFragmentType == eDB2_FRAGMENT_MSB_2_LSB)
   ||   (mFragmentType == eDB2_FRAGMENT_LSB_2_MSB) )
   {
      if (mFragmentOrder > 0)
      {
         return false;
      }
   }

   // Validate modifier
   switch (mModifierType)
   {
      case eDB2_MOD_NONE:
         if (mpModifierValue != 0 && mpModifierValue[0] != 0)
         {
            // Modifier string needs to be empty
            return false;
         }
         break;

      case eDB2_MOD_CONSTANT_ARRAY:
      case eDB2_MOD_VARIABLE_ARRAY:
      case eDB2_MOD_OPTIONAL:
      case eDB2_MOD_VARIABLE_ARRAY2:
      case eDB2_MOD_VARIABLE_ARRAY3:
         if (mpModifierValue == 0 || mpModifierValue[0] == 0)
         {
            // Needs to be a modifier string
            return false;
         }
         break;

      case eDB2_MOD_VARIABLE_STRING1:
      case eDB2_MOD_VARIABLE_STRING2:
      case eDB2_MOD_VARIABLE_STRING3:
         if (mpModifierValue == 0 || mpModifierValue[0] == 0)
         {
            // Needs to be a modifier string
            return false;
         }

         if (mFragmentType != eDB2_FRAGMENT_FIELD)
         {
            // Only valid when modifying fields (strings)
            return false;
         }
         break;

   }

   if (mFragmentType == eDB2_FRAGMENT_CONSTANT_PAD && mFragmentValue == 0)
   {
      return false;
   }

   return true;
}

/*===========================================================================
METHOD:
   BuildCondition (Static Public Method)

DESCRIPTION:
   Build a simple condition string
  
PARAMETERS:
   id          [ I ] - Field ID
   op          [ I ] - Operator
   val         [ I ] - Value (or field ID)
   bF2F        [ I ] - Field to field expression?

RETURN VALUE:
   std::string
===========================================================================*/
std::string sDB2Fragment::BuildCondition( 
   ULONG                      id,
   eDB2Operator               op,
   LONGLONG                   val,
   bool                       bF2F )
{
   std::ostringstream tmp;
   
   if (::IsValid( op ) == true)
   {
      if (bF2F == false)
      {
         switch (op)
         {
            case eDB2_OP_LT:
               tmp << (UINT)id << " " << "<" << val;
               break;

            case eDB2_OP_LTE:
               tmp << (UINT)id << " " << "<=" << val;
               break;

            case eDB2_OP_EQ:
               tmp << (UINT)id << " " << "=" << val;
               break;

            case eDB2_OP_NEQ:
               tmp << (UINT)id << " " << "!=" << val;
               break;

            case eDB2_OP_GTE:
               tmp << (UINT)id << " " << ">=" << val;
               break;

            case eDB2_OP_GT:
               tmp << (UINT)id << " " << ">" << val;
               break;

            case eDB2_OP_DIV:
               tmp << (UINT)id << " " << "%" << val;
               break;

            case eDB2_OP_NDIV:
               tmp << (UINT)id << " " << "!%" << val;
               break;
         }
      }
      else
      {
         switch (op)
         {
            case eDB2_OP_LT:
               tmp << (UINT)id << " " << "f<" << val;
               break;

            case eDB2_OP_LTE:
               tmp << (UINT)id << " " << "f<=" << val;
               break;

            case eDB2_OP_EQ:
               tmp << (UINT)id << " " << "f=" << val;
               break;

            case eDB2_OP_NEQ:
               tmp << (UINT)id << " " << "f!=" << val;
               break;

            case eDB2_OP_GTE:
               tmp << (UINT)id << " " << "f>=" << val;
               break;

            case eDB2_OP_GT:
               tmp << (UINT)id << " " << "f>" << val;
               break;

            case eDB2_OP_DIV:
               tmp << (UINT)id << " " << "f%" << val;
               break;

            case eDB2_OP_NDIV:
               tmp << (UINT)id << " " << "f!%" << val;
               break;
         }
      }
   }

   std::string retStr = tmp.str();

   return retStr;
}

/*===========================================================================
METHOD:
   EvaluateCondition (Static Public Method)

DESCRIPTION:
   Evaluate a simple condition
  
PARAMETERS:
   valA        [ I ] - Left value
   op          [ I ] - Operator
   valB        [ I ] - Right value

RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::EvaluateCondition( 
   LONGLONG                   valA,
   eDB2Operator               op,
   LONGLONG                   valB )
{
   bool bOK = false;
   if (::IsValid( op ) == true)
   {
      switch (op)
      {
         case eDB2_OP_LT:
            bOK = (valA < valB);
            break;

         case eDB2_OP_LTE:
            bOK = (valA <= valB);
            break;

         case eDB2_OP_EQ:
            bOK = (valA == valB);
            break;

         case eDB2_OP_NEQ:
            bOK = (valA != valB);
            break;

         case eDB2_OP_GTE:
            bOK = (valA >= valB);
            break;

         case eDB2_OP_GT:
            bOK = (valA > valB);
            break;

         case eDB2_OP_DIV:
            bOK = ((valA % valB) == 0);
            break;

         case eDB2_OP_NDIV:
            bOK = ((valA % valB) != 0);
            break;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ParseCondition (Static Public Method)

DESCRIPTION:
   Parse a simple condition
  
PARAMETERS:
   pCondition  [ I ] - Condition string
   id          [ O ] - Field ID
   op          [ O ] - Operator
   val         [ O ] - Value (or field ID)
   bF2F        [ O ] - Field to field expression?

RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::ParseCondition( 
   LPCSTR                     pCondition,
   ULONG &                    id,
   eDB2Operator &             op,
   LONGLONG &                 val,
   bool &                     bF2F )
{
   // Assume error
   bool bOK = false;

   // Even a condition to start with?
   if (pCondition == 0 || pCondition == EMPTY_STRING)
   {
      return bOK;
   }

   // Parse condition to tokens (field ID operator value)
   int nSize = strlen( pCondition ) + 1;
   
   char * pCopy = new char[ nSize ];
   if (pCopy == NULL)
   {
      return false;
   }
   
   memcpy( pCopy, pCondition, nSize );
   
   std::vector <LPSTR> tokens; 
   ParseTokens( " ", pCopy, tokens );

   if (tokens.size() == 3)
   {
      // Covert first token to field ID
      ULONG fieldID = strtoul( tokens[0], 0, 10 );

      // Grab the value for the given field ID
      LONGLONG fieldVal = 0;
      bOK = StringToLONGLONG( tokens[2], 0, fieldVal );
      if (bOK == true)
      {
         std::string opStr = tokens[1];
         
         // std::string version of Trim()
         int nFirst = opStr.find_first_not_of( ' ' );
         int nLast = opStr.find_last_not_of( ' ' );
         if (nFirst == -1 || nLast == -1)
         {
            // Something went horribly wrong, empty string or all spaces
            delete [] pCopy;
            return false;
         }
         
         opStr = opStr.substr( nFirst, nLast - nFirst + 1 );
         
         // std::string version of MakeLower()
         transform( opStr.begin(), opStr.end(), opStr.begin(), tolower );

         bF2F = false;
         if (opStr == "<")
         {
            op = eDB2_OP_LT;
         }
         else if (opStr == "<=")
         {
            op = eDB2_OP_LTE;
         }
         else if (opStr == "=")
         {
            op = eDB2_OP_EQ;
         }
         else if (opStr == "!=")
         {
            op = eDB2_OP_NEQ;
         }
         else if (opStr == ">=")
         {
            op = eDB2_OP_GTE;
         }
         else if (opStr == ">")
         {
            op = eDB2_OP_GT;
         }
         else if (opStr == "%")
         {
            op = eDB2_OP_DIV;
         }
         else if (opStr == "!%")
         {
            op = eDB2_OP_NDIV;
         }
         else if (opStr == "f<")
         {
            bF2F = true;
            op = eDB2_OP_LT;
         }
         else if (opStr == "f<=")
         {
            bF2F = true;
            op = eDB2_OP_LTE;
         }
         else if (opStr == "f=")
         {
            bF2F = true;
            op = eDB2_OP_EQ;
         }
         else if (opStr == "f!=")
         {
            bF2F = true;
            op = eDB2_OP_NEQ;
         }
         else if (opStr == "f>=")
         {
            bF2F = true;
            op = eDB2_OP_GTE;
         }
         else if (opStr == "f>")
         {
            bF2F = true;
            op = eDB2_OP_GT;
         }
         else if (opStr == "f%")
         {
            bF2F = true;
            op = eDB2_OP_DIV;
         }
         else if (opStr == "f!%")
         {
            bF2F = true;
            op = eDB2_OP_NDIV;
         }
         else
         {
            bOK = false;
         }

         if (bOK == true)
         {
            id = fieldID;
            val = fieldVal;
         }
      }
   }

   delete [] pCopy;
   
   return bOK;
}

/*===========================================================================
METHOD:
   BuildExpression (Static Public Method)

DESCRIPTION:
   Build a simple expression string
  
PARAMETERS:
   id          [ I ] - Field ID
   op          [ I ] - Operator
   val         [ I ] - Value (or field ID)
   bF2F        [ I ] - Field to field expression?

RETURN VALUE:
   std::string
===========================================================================*/
std::string sDB2Fragment::BuildExpression( 
   ULONG                      id,
   eDB2ExpOperator            op,
   LONGLONG                   val,
   bool                       bF2F )
{
   std::ostringstream tmp;

   if (::IsValid( op ) == true)
   {
      if (bF2F == false)
      {
         switch (op)
         {
            case eDB2_EXPOP_ADD:
               tmp << (UINT)id << " " << "+" << val;
               break;

            case eDB2_EXPOP_SUB:
               tmp << (UINT)id << " " << "-" << val;
               break;

            case eDB2_EXPOP_MUL:
               tmp << (UINT)id << " " << "*" << val;
               break;

            case eDB2_EXPOP_DIV:
               tmp << (UINT)id << " " << "/" << val;
               break;

            case eDB2_EXPOP_REM:
               tmp << (UINT)id << " " << "%" << val;
               break;

            case eDB2_EXPOP_MIN:
               tmp << (UINT)id << " " << "min" << val;
               break;

            case eDB2_EXPOP_MAX:
               tmp << (UINT)id << " " << "max" << val;
               break;
         }
      }
      else
      {
         switch (op)
         {
            case eDB2_EXPOP_ADD:
               tmp << (UINT)id << " " << "f+" << val;
               break;

            case eDB2_EXPOP_SUB:
               tmp << (UINT)id << " " << "f-" << val;
               break;

            case eDB2_EXPOP_MUL:
               tmp << (UINT)id << " " << "f*" << val;
               break;

            case eDB2_EXPOP_DIV:
               tmp << (UINT)id << " " << "f/" << val;
               break;

            case eDB2_EXPOP_REM:
               tmp << (UINT)id << " " << "f%" << val;
               break;

            case eDB2_EXPOP_MIN:
               tmp << (UINT)id << " " << "fmin" << val;
               break;

            case eDB2_EXPOP_MAX:
               tmp << (UINT)id << " " << "fmax" << val;
               break;
         }
      }
   }

   std::string retStr = tmp.str();

   return retStr;
}

/*===========================================================================
METHOD:
   EvaluateExpression (Static Public Method)

DESCRIPTION:
   Evaluate a simple expression
  
PARAMETERS:
   valA        [ I ] - Left value
   op          [ I ] - Operator
   valB        [ I ] - Right value
   res         [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::EvaluateExpression( 
   LONGLONG                   valA,
   eDB2ExpOperator            op,
   LONGLONG                   valB,
   LONGLONG &                 res )
{
   bool bOK = false;
   if (::IsValid( op ) == true)
   {
      bOK = true;
      switch (op)
      {
         case eDB2_EXPOP_ADD:
            res = valA + valB;
            break;

         case eDB2_EXPOP_SUB:
            res = valA - valB;
            break;

         case eDB2_EXPOP_MUL:
            res = valA * valB;
            break;

         case eDB2_EXPOP_DIV:
            res = valA / valB;
            break;

         case eDB2_EXPOP_REM:
            res = valA % valB;
            break;

         case eDB2_EXPOP_MIN:
            res = valA;
            if (valA > valB)
            {
               res = valB;
            }
            break;

         case eDB2_EXPOP_MAX:
            res = valA;
            if (valA < valB)
            {
               res = valB;
            }
            break;

         default:
            bOK = false;
            break;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ParseExpression (Static Public Method)

DESCRIPTION:
   Parse a simple expression
  
PARAMETERS:
   pExpr       [ I ] - Expression string
   id          [ O ] - Field ID
   op          [ O ] - Operator
   val         [ O ] - Value (or Field ID)
   bF2F        [ O ] - Field to field expression?

RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Fragment::ParseExpression( 
   LPCSTR                    pExpr,
   ULONG &                    id,
   eDB2ExpOperator &          op,
   LONGLONG &                 val,
   bool &                     bF2F )
{
   // Assume error
   bool bOK = false;

   // Even a condition to start with?
   if (pExpr == 0 || pExpr == EMPTY_STRING)
   {
      return bOK;
   }

   // Parse condition to tokens (field ID operator value)
   int nSize = strlen( pExpr ) + 1;
   
   char * pCopy = new char[ nSize ];
   if (pCopy == NULL)
   {
      return false;
   }
   
   memcpy( pCopy, pExpr, nSize );

   std::vector <LPSTR> tokens; 
   ParseTokens( " ", pCopy, tokens );

   if (tokens.size() == 3)
   {
      // Covert first token to field ID
      ULONG fieldID = strtoul( tokens[0], 0, 10 );

      // Grab the value for the given field ID
      LONGLONG fieldVal = 0;
      bOK = StringToLONGLONG( tokens[2], 0, fieldVal );
      if (bOK == true)
      {
         std::string opStr = tokens[1];
         
         // std::string version of Trim()
         int nFirst = opStr.find_first_not_of( ' ' );
         int nLast = opStr.find_last_not_of( ' ' );
         if (nFirst == -1 || nLast == -1)
         {
            // Something went horribly wrong, empty string or all spaces
            delete [] pCopy;
            return false;
         }
         
         opStr = opStr.substr( nFirst, nLast - nFirst + 1 );
         
         // std::string version of MakeLower()
         transform( opStr.begin(), opStr.end(), opStr.begin(), tolower );

         bF2F = false;
         if (opStr == "+")
         {
            op = eDB2_EXPOP_ADD;
         }
         else if (opStr == "-")
         {
            op = eDB2_EXPOP_SUB;
         }
         else if (opStr == "*")
         {
            op = eDB2_EXPOP_MUL;
         }
         else if (opStr == "/")
         {
            op = eDB2_EXPOP_DIV;
         }
         else if (opStr == "%")
         {
            op = eDB2_EXPOP_REM;
         }
         else if (opStr == "min")
         {
            op = eDB2_EXPOP_MIN;
         }
         else if (opStr == "max")
         {
            op = eDB2_EXPOP_MAX;
         }
         else if (opStr == "f+")
         {
            bF2F = true;
            op = eDB2_EXPOP_ADD;
         }
         else if (opStr == "f-")
         {
            bF2F = true;
            op = eDB2_EXPOP_SUB;
         }
         else if (opStr == "f*")
         {
            bF2F = true;
            op = eDB2_EXPOP_MUL;
         }
         else if (opStr == "f/")
         {
            bF2F = true;
            op = eDB2_EXPOP_DIV;
         }
         else if (opStr == "f%")
         {
            bF2F = true;
            op = eDB2_EXPOP_REM;
         }
         else if (opStr == "fmin")
         {
            bF2F = true;
            op = eDB2_EXPOP_MIN;
         }
         else if (opStr == "fmax")
         {
            bF2F = true;
            op = eDB2_EXPOP_MAX;
         }
         else
         {
            bOK = false;
         }

         if (bOK == true)
         {
            id = fieldID;
            val = fieldVal;
         }
      }
   }

   delete [] pCopy;
   return bOK;
}

/*=========================================================================*/
// sDB2Field Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Field::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be 
   //    0: ID 
   //    1: "Name"
   //    2: Size
   //    3: Field type
   //    4: Field type value
   //    5: Hexadecimal
   //    6: Description ID (optional)
   //    7: Internal only flag (optional)
   const ULONG NUM_REQ_VALS = 6;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();

   if (toks >= NUM_REQ_VALS)
   {
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[1] );
      if (pCopy != 0)
      {
         mID      = strtoul( tokens[0], 0, 10 );
         mSize    = strtoul( tokens[2], 0, 10 );
         mpName   = pCopy;
         mType    = (eDB2FieldType)strtol( tokens[3], 0, 10 );
         mTypeVal = strtoul( tokens[4], 0, 10 );
         mbHex    = (strtoul( tokens[5], 0, 10 ) != 0);
         
         // Description ID?
         if (toks > NUM_REQ_VALS)
         {
            mDescriptionID = strtol( tokens[NUM_REQ_VALS], 0, 10 );
         }

         // Internal only flag?
         if (toks > NUM_REQ_VALS + 1)
         {
            mbInternal = (strtoul( tokens[NUM_REQ_VALS + 1], 0, 10 ) != 0);
         }

         bRC = IsValid();
      }                
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Field::IsValid() const
{
   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   // The field type must be valid
   if (::IsValid( mType ) == false)
   {
      return false;
   }

   // For validating size
   ULONG minSz = 1;
   ULONG maxSz = 8;
   ULONG modVal = 0;

   // What type of field is this?
   if (mType == eDB2_FIELD_STD)
   {
      eDB2StdFieldType ft = (eDB2StdFieldType)mTypeVal;
      if (::IsValid( ft ) == false)
      {
         return false;
      }

      switch (ft)
      {
         case eDB2_FIELD_STDTYPE_BOOL:
            maxSz = 64;
            break;

         case eDB2_FIELD_STDTYPE_INT16:
         case eDB2_FIELD_STDTYPE_UINT16:
            maxSz = 16;
            break;

         case eDB2_FIELD_STDTYPE_INT32:
         case eDB2_FIELD_STDTYPE_UINT32:
         case eDB2_FIELD_STDTYPE_FLOAT32:
            maxSz = 32;
            break;

         case eDB2_FIELD_STDTYPE_INT64:
         case eDB2_FIELD_STDTYPE_UINT64:
         case eDB2_FIELD_STDTYPE_FLOAT64:
            maxSz = 64;
            break;

         case eDB2_FIELD_STDTYPE_STRING_A:
         case eDB2_FIELD_STDTYPE_STRING_U8:
            // One character, no maximum
            minSz = 8;
            maxSz = 0;
            modVal = 8;
            break;

         case eDB2_FIELD_STDTYPE_STRING_U:
            // One UNICODE character, no maximum
            minSz = 16;
            maxSz = 0;
            modVal = 16;
            break;

         case eDB2_FIELD_STDTYPE_STRING_ANT:
         case eDB2_FIELD_STDTYPE_STRING_UNT:
         case eDB2_FIELD_STDTYPE_STRING_U8NT:
            // Size needs to be specified as 0
            minSz = maxSz = 0;
            break;
      }
   }
   else 
   {
      // Enum must be between 1 - 32 bits in size
      maxSz = 32;
   }
   
   if (mSize < minSz)
   {
      return false;
   }

   if (maxSz != 0 && mSize > maxSz)
   {
      return false;
   }

   if (modVal != 0 && (mSize % modVal) != 0) 
   {
      return false;
   }

   if (mDescriptionID < -1)
   {
      return false;
   }

   // The name must be valid
   std::string name = mpName;
   if (name.find( DB2_VALUE_SEP ) != -1 || name.find( DB2_SUBVAL_SEP ) != -1)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// sDB2Category Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Category::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be
   //    0: ID
   //    1: "Name"
   //    2: Description ID
   //    3: Parent ID
   const ULONG NUM_REQ_VALS = 4;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[1] );
      if (pCopy != 0)
      {
         mID       = strtoul( tokens[0], 0, 10 );
         mParentID = strtol( tokens[3], 0, 10 );
         mpName    = pCopy;

         // Old format used to be a description string, so
         // first check for quotes
         if (tokens[2][0] != '\"')
         {
            mDescriptionID = strtol( tokens[2], 0, 10 );
         }

         bRC = IsValid();
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Category::IsValid() const
{
   // The parent ID has to be greater than or equal to -1
   if (mParentID < -1)
   {
      return false;
   }

   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   if (mDescriptionID < -1)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// sDB2NVItem Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2NVItem::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be
   //    0: NV Item number
   //    1: "Name"
   //    2: "Categories"
   //    3: Description ID
   const ULONG NUM_REQ_VALS = 4;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[1] );
      if (pCopy != 0)
      {
         CSVStringToContainer( DB2_SUBVAL_SEP, tokens[2], mCategoryIDs );

         mItem  = strtoul( tokens[0], 0, 10 );
         mpName = pCopy;

         // Old format used to be a description string, so
         // first check for quotes
         if (tokens[3][0] != '\"')
         {
            mDescriptionID = strtol( tokens[3], 0, 10 );
         }

         bRC = IsValid();
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2NVItem::IsValid() const
{
   // There has to be at least one category ID
   ULONG cats = (ULONG)mCategoryIDs.size();
   if (cats < 1)
   {
      return false;
   }

   // The category IDs have to be greater than or equal to -1
   std::set <int>::const_iterator pIter = mCategoryIDs.begin();
   while (pIter != mCategoryIDs.end())
   {
      if (*pIter++ < -1)
      {
         return false;
      }
   }

   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   if (mDescriptionID < -1)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// sDB2Enum Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Enum::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be 
   //    0: ID
   //    1: "Name"
   //    2: Description ID
   //    3: Internal?
   const ULONG NUM_REQ_VALS = 4;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {    
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[1] );
      if (pCopy != 0)
      {               
         mID         = strtoul( tokens[0], 0, 10  );
         mbInternal  = (strtoul( tokens[3], 0, 10 ) != 0);
         mpName      = pCopy;

         // Old format used to be a description string, so
         // first check for quotes
         if (tokens[2][0] != '\"')
         {
            mDescriptionID = strtol( tokens[2], 0, 10 );
         }

         bRC = IsValid();
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2Enum::IsValid() const
{
   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   if (mDescriptionID < -1)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// sDB2EnumEntry Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FromString (Public Method)

DESCRIPTION:
   Populate this object from a string

PARAMETERS:
   pStr        [ I ] - String to populate object from
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2EnumEntry::FromString( LPSTR pStr )
{
   bool bRC = false;

   // Should be 
   //    0: ID
   //    1: Value
   //    2: "Name"
   //    3: Description ID (optional)
   const ULONG NUM_REQ_VALS = 3;

   std::vector <LPSTR> tokens; 
   ParseTokens( DB2_VALUE_SEP, pStr, tokens );

   ULONG toks = (ULONG)tokens.size();
   if (toks >= NUM_REQ_VALS)
   {    
      // Remove quotes from name string and copy
      LPCSTR pCopy = CopyQuotedString( tokens[2] );
      if (pCopy != 0)
      {               
         mID    = strtoul( tokens[0], 0, 10  );
         mpName = pCopy;

         // Enum entries are signed by definition, but can be entered 
         // in hexadecimal as they may be unsigned in practice
         LONG val = -1;
         StringToLONG( tokens[1], 0, val );
         mValue = (INT)val;

         // Determine hexadecimal flag by performing case-insensitve comparison
         // of the value string's first two characters with "0x"
         mbHex = (strncmp( tokens[1], "0x", 2 ) == 0);

         // Description ID?
         if (toks > NUM_REQ_VALS)
         {
            mDescriptionID = strtol( tokens[NUM_REQ_VALS], 0, 10 );
         }

         bRC = IsValid();
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   IsValid (Public Method)

DESCRIPTION:
   Is this object valid?
  
RETURN VALUE:
   bool
===========================================================================*/
bool sDB2EnumEntry::IsValid() const
{
   // There has to be a non-empty name
   if (mpName == 0 || mpName[0] == 0)
   {
      return false;
   }

   if (mDescriptionID < -1)
   {
      return false;
   }

   return true;
}

/*=========================================================================*/
// cCoreDatabase Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cCoreDatabase (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cCoreDatabase::cCoreDatabase()
   :  mpLog( &gDB2DefaultLog )
{
   // Nothing to do - database empty, call Initialize()
}

/*===========================================================================
METHOD:
   ~cCoreDatabase (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cCoreDatabase::~cCoreDatabase()
{
   Exit();
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Version to Load from file
   Initialize the database - this must be done once (and only once)
   prior to the database being accessed
  
PARAMETERS
   pBasePath         [ I ] - Base path to database files

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::Initialize( LPCSTR pBasePath )
{
   bool bRC = true;

   // Cleanup the last database (if necessary)
   Exit();

   bRC &= LoadEnumTables( pBasePath );
   bRC &= LoadStructureTables( pBasePath );

   // Build the modifier tables
   bRC &= BuildModifierTables();

   return bRC;
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Version to Load from internal pointers
   Initialize the database - this must be done once (and only once)
   prior to the database being accessed
  
PARAMETERS

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::Initialize()
{
   bool bRC = true;

   // Cleanup the last database (if necessary)
   Exit();

   bRC &= LoadEnumTables();
   bRC &= LoadStructureTables();

   // Build the modifier tables
   bRC &= BuildModifierTables();

   return bRC;
}

/*===========================================================================
METHOD:
   Exit (Public Method)

DESCRIPTION:
   Exit (cleanup) the database
  
RETURN VALUE:
   None
===========================================================================*/
void cCoreDatabase::Exit()
{
   FreeDB2Table( mEntityFields );
   FreeDB2Table( mEntityStructs );
   FreeDB2Table( mProtocolEntities );

   FreeDB2Table( mEnumNameMap );
   FreeDB2Table( mEnumEntryMap );

   tDB2EntityNavMap::iterator pIter = mEntityNavMap.begin();
   while (pIter != mEntityNavMap.end())
   {
      cDB2NavTree * pNav = pIter->second;
      if (pNav != 0)
      {
         delete pNav;
      }

      pIter++;
   }

   mEntityNavMap.clear();
}

/*===========================================================================
METHOD:
   GetEntityNavTree (Public Method)

DESCRIPTION:
   Get the entity navigation tree for the given protocol entity, if none 
   exists one will be built and returned
  
PARAMETERS   
   key         [ I ] - Protocol entity key

RETURN VALUE:
   const cDB2NavTree * (0 upon error)
===========================================================================*/
const cDB2NavTree * cCoreDatabase::GetEntityNavTree( 
   const std::vector <ULONG> &   key ) const
{
   // Look up entity definition (to find DB string address)
   sDB2ProtocolEntity tmpEntity;
   bool bFound = FindEntity( key, tmpEntity );

   // Did we find it?
   if (bFound == false)
   {
      // No matching definition in database
      return 0;
   }

   // Obtain the canonical key and use it to look up the nav tree
   tDB2EntityNavMap::const_iterator pIter = mEntityNavMap.find( key );
   if (pIter != mEntityNavMap.end())
   {
      return pIter->second;
   }

   // None found, go ahead and build one
   cDB2NavTree * pNavTree = new cDB2NavTree( *this );
   if (pNavTree != 0)
   {
      bool bOK = pNavTree->BuildTree( key );
      if (bOK == true)
      {
         // Store it and return it to the user
         std::pair <std::vector <ULONG>, cDB2NavTree *> e( key, pNavTree );
         mEntityNavMap.insert( e );
      }
      else
      {
         delete pNavTree;
         pNavTree = 0;
      }
   }

   return pNavTree;
}

/*===========================================================================
METHOD:
   FindEntity (Public Method)

DESCRIPTION:
   Find the protocol entity with the specified extended ID
  
PARAMETERS   
   key         [ I ] - Protocol entity key to find
   entity      [ O ] - Protocol entity (if found)

RETURN VALUE:
   bool - Success?
===========================================================================*/
bool cCoreDatabase::FindEntity( 
   const std::vector <ULONG> &   key,
   sDB2ProtocolEntity &          entity ) const
{
   // Assume failure
   bool bFound = false;

   tDB2EntityMap::const_iterator pEntity = mProtocolEntities.find( key );
   if (pEntity != mProtocolEntities.end())
   {
      entity = pEntity->second;
      bFound = true;
   }

   return bFound;
}

/*===========================================================================
METHOD:
   FindEntity (Public Method)

DESCRIPTION:
   Find the protocol entity with the specified name
  
PARAMETERS   
   pEntityName [ I ] - Protocol entity name to find
   entity      [ O ] - Protocol entity (if found)

RETURN VALUE:
   bool - Success?
===========================================================================*/
bool cCoreDatabase::FindEntity( 
   LPCSTR                   pEntityName,
   sDB2ProtocolEntity &     entity ) const
{
   // Assume failure
   bool bFound = false;
   if (pEntityName != 0 && pEntityName[0] != 0)
   {
      tDB2EntityNameMap::const_iterator pIter = mEntityNames.find( pEntityName );
      if (pIter != mEntityNames.end())
      {
         const std::vector <ULONG> & key = pIter->second;
         bFound = FindEntity( key, entity );
      }
   }

   return bFound;
}

/*===========================================================================
METHOD:
   MapEntityNameToID (Public Method)

DESCRIPTION:
   Map a protocol entity name to an ID
  
PARAMETERS
   pName       [ I ] - Protocol entity name
   key         [ O ] - Upon success, the ID corresponding to protocol entity

RETURN VALUE:
   bool - Success?
===========================================================================*/
bool cCoreDatabase::MapEntityNameToID( 
   LPCSTR                       pName,
   std::vector <ULONG> &        key ) const
{
   // Assume failure
   bool bOK = false;

   if (pName != 0 && pName[0] != 0)
   {
      std::string tmp = pName;
      
      // std::string version of Trim()
      int nFirst = tmp.find_first_not_of( ' ' );
      int nLast = tmp.find_last_not_of( ' ' );
      if (nFirst == -1 || nLast == -1)
      {
         // Something went wrong, empty string or all spaces
         return false;
      }

      tmp = tmp.substr( nFirst, nLast - nFirst + 1 );
     
      
      tDB2EntityNameMap::const_iterator pIter = mEntityNames.find( tmp.c_str() );
      if (pIter != mEntityNames.end())
      {
         key = pIter->second;
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   MapEnumToString (Public Method)

DESCRIPTION:
   Map the given enum value (specified by enum ID, and enum value) to
   the enum value name string
  
PARAMETERS
   enumID         [ I ] - ID of the enumeration
   enumVal        [ I ] - Enum value to map
   bSimpleErrFmt  [ I ] - If the eunum value cannot be mapped to a string
                          what should this method return?

                          If 'true' then just the value as a string
                          If 'false' then the enum ID, value, and 'Unknown'

   bHex           [ I ] - Hexadecimal output on mapping error?

RETURN VALUE:
   std::string - The enum name (or error string if enum value is not found)
===========================================================================*/
std::string cCoreDatabase::MapEnumToString( 
   ULONG                      enumID,
   int                        enumVal,
   bool                       bSimpleErrFmt,
   bool                       bHex ) const
{
   std::string retStr = "";

   // Form the lookup key
   std::pair <ULONG, int> key( enumID, enumVal );

   // Look up the enum value descriptor
   tDB2EnumEntryMap::const_iterator pVals = mEnumEntryMap.find( key );
   if (pVals != mEnumEntryMap.end())
   {
      const sDB2EnumEntry & entry = pVals->second;
      retStr = entry.mpName;
   }

   // No string?
   if (retStr.size() <= 0)
   {
      std::ostringstream tmp;

      if (bSimpleErrFmt == false)
      {
         tmp << "Unknown [" << (UINT)enumID << "/";
      }
      
      if (bHex == true)
      {
         tmp << std::ios_base::hex << std::ios_base::uppercase 
                << std::ios_base::showbase << enumVal;
      }
      else
      {
         tmp << enumVal;
      }
      
      retStr = tmp.str();
   }
   
   return retStr;
}

/*===========================================================================
METHOD:
   MapEnumToString (Public Method)

DESCRIPTION:
   Map the given enum value (specified by enum name, and enum value) to
   the enum value name string
  
PARAMETERS
   pEnumName      [ I ] - Name of the enumeration
   enumVal        [ I ] - Enum value to map
   bSimpleErrFmt  [ I ] - If the eunum value cannot be mapped to a string
                          what should this method return?

                          If 'true' then just the value as a string
                          If 'false' then the enum ID, value, and 'Unknown'

   bHex           [ I ] - Hexadecimal output on mapping error?

RETURN VALUE:
   std::string - The enum name (or error string if enum value is not found)
===========================================================================*/
std::string cCoreDatabase::MapEnumToString( 
   LPCSTR                     pEnumName,
   int                        enumVal,
   bool                       bSimpleErrFmt,
   bool                       bHex ) const
{
   std::string retStr = "";

   tDB2EnumMap::const_iterator pEnumMapIter = mEnumMap.find( pEnumName );      
   if (pEnumMapIter != mEnumMap.end())
   {
      const std::map <int, LPCSTR> & entries = pEnumMapIter->second.second;
      std::map <int, LPCSTR>::const_iterator pEntry;

      pEntry = entries.find( enumVal );
      if (pEntry != entries.end())
      {
         retStr = pEntry->second;
      }
   }

   // No string?
   if (retStr.size() <= 0)
   {
      std::ostringstream tmp;

      if (bSimpleErrFmt == false)
      {
         if (pEnumName == 0)
         {
            pEnumName = "?";
         }
         
         tmp << "Unknown [" << pEnumName << "/";
      }
      
      if (bHex == true)
      {
         tmp << std::ios_base::hex << std::ios_base::uppercase 
                << std::ios_base::showbase << enumVal;
      }
      else
      {
         tmp << enumVal;
      }
      
      retStr = tmp.str();
   }
   
   return retStr;
}

/*===========================================================================
METHOD:
   AssembleEnumMap (Internal Method)

DESCRIPTION:
   Assemble the internal enum map from the enum and enum entry tables
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::AssembleEnumMap()
{
   bool bOK = true;

   // Empty it out first
   mEnumMap.clear();

   tDB2EnumEntryMap::const_iterator pEntry = mEnumEntryMap.begin();
   if (pEntry == mEnumEntryMap.end())
   {
      return bOK;
   }

   // Set initial enum ID
   ULONG currentID = pEntry->second.mID;

   std::map <int, LPCSTR> entries;   
   while (pEntry != mEnumEntryMap.end())
   {
      const sDB2EnumEntry & entry = pEntry->second;
      pEntry++;

      if (entry.IsValid() == false)
      {
         continue;
      }

      if (currentID != entry.mID)
      {
         if (entries.size() > 0)
         {
            // Look up the enum name
            tDB2EnumNameMap::const_iterator pEnum;
            pEnum = mEnumNameMap.find( currentID );

            if (pEnum != mEnumNameMap.end())
            {
               const sDB2Enum & dbEnum = pEnum->second;
               if (mEnumMap.find( dbEnum.mpName ) == mEnumMap.end())
               {
                  tDB2EnumMapPair tmp( dbEnum.mID, entries );
                  mEnumMap[dbEnum.mpName] = tmp;
               }
               else
               {
                  // Hmm, duplicate enum names discovered
                  std::ostringstream tmp;
                  tmp << "DB [" << DB2_TABLE_ENUM_MAIN 
                      << "] Duplicate enum (by name) detected \'" 
                      << dbEnum.mpName << "\'";

                  mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

                  bOK = false;
               }
            }
            else
            {
               // Hmm, missing enum ID discovered                
               std::ostringstream tmp;
               tmp << "DB [" << DB2_TABLE_ENUM_MAIN 
                   << "] Missing enum ID detected " 
                   << currentID;

               mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

               bOK = false;
            }

            // Clear out enum entries for next pass and add this entry
            entries.clear();
            entries[entry.mValue] = entry.mpName;

            // Adjust current enum ID
            currentID = entry.mID;            
         }
      }
      else
      {
         if (entries.find( entry.mValue ) == entries.end())
         {
            entries[entry.mValue] = entry.mpName;
         }
         else
         {
            // Hmm, duplicate enum entry values discovered                 
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_ENUM_ENTRY 
                << "] Duplicate enum entries detected \'" 
                << entry.mpName << "\', " << entry.mValue;

            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            bOK = false;
         }
      }      
   }

   // Add in the last enum
   if (mEnumEntryMap.size() > 0 && entries.size() > 0)
   {
      // Look up the enum name
      tDB2EnumNameMap::const_iterator pEnum;
      pEnum = mEnumNameMap.find( currentID );

      if (pEnum != mEnumNameMap.end())
      {
         const sDB2Enum & dbEnum = pEnum->second;
         if (mEnumMap.find( dbEnum.mpName ) == mEnumMap.end())
         {
            tDB2EnumMapPair tmp( dbEnum.mID, entries );
            mEnumMap[dbEnum.mpName] = tmp;
         }
         else
         {
            // Hmm, duplicate enum names discovered
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_ENUM_MAIN 
                << "] Duplicate enum (by name) detected \'" 
                << dbEnum.mpName << "\'";

            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            bOK = false;
         }
      }
      else
      {
         // Hmm, missing enum ID discovered                
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_ENUM_MAIN 
             << "] Missing enum ID detected " << currentID; 

         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   AssembleEntityNameMap (Internal Method)

DESCRIPTION:
   Assemble the internal protocol entity name map
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::AssembleEntityNameMap()
{
   // Assume success
   bool bOK = true;

   // Empty it out first
   mEntityNames.clear();

   // Go through and build the event name table
   tDB2EntityMap::const_iterator pIter = mProtocolEntities.begin();
   while (pIter != mProtocolEntities.end())
   {
      const sDB2ProtocolEntity & obj = pIter->second;
      pIter++;

      if (obj.IsValid() == false)
      {
         continue;
      }
    
      tDB2EntityNameMap::const_iterator pNames;
      pNames = mEntityNames.find( obj.mpName );
      if (pNames == mEntityNames.end())
      {
         mEntityNames[obj.mpName] = obj.GetKey();
      }
      else
      {
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_PROTOCOL_ENTITY 
             << "] Duplicate protocol entity (by name) detected \'" 
             << obj.mpName << "\'";

         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   BuildModifierTables (Internal Method)

DESCRIPTION:
   Build the parsed fragment modifier maps, i.e. convert the modifier
   text string to something more useful by database clients
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::BuildModifierTables()
{
   // Assume success
   bool bOK = true;

   // Parse all fragment modifiers
   tDB2FragmentMap::const_iterator pFragIter = mEntityStructs.begin();
   while (pFragIter != mEntityStructs.end())
   {
      // Grab new fragment
      const sDB2Fragment & frag = pFragIter->second;
      pFragIter++;

      // Skip invalid/unmodified fragments
      if ( (frag.IsValid() == false) 
      ||   (frag.mpModifierValue == 0) 
      ||   (frag.mpModifierValue == EMPTY_STRING) )
      {
         continue;
      }  

      switch (frag.mModifierType)
      {
         case eDB2_MOD_CONSTANT_ARRAY:
         case eDB2_MOD_VARIABLE_ARRAY:
         case eDB2_MOD_VARIABLE_STRING1:
         case eDB2_MOD_VARIABLE_STRING2:
         case eDB2_MOD_VARIABLE_STRING3:
         {
            ULONG val = strtoul( frag.mpModifierValue, 0, 0 );
            mArray1ModMap[frag.mpModifierValue] = val;
         }
         break;

         case eDB2_MOD_VARIABLE_ARRAY2:
         {
            // Parse modifier to tokens (start stop)
            int nSize = strlen( frag.mpModifierValue ) + 1;
            
            char * pCopy = new char[ nSize ];
            if (pCopy == NULL)
            {
               return false;
            }
            
            memcpy( pCopy, frag.mpModifierValue, nSize );

            std::vector <ULONG> indices; 
            CSVStringToContainer( " ", pCopy, indices );    

            delete [] pCopy;
            if (indices.size() == 2)
            {
               std::pair <ULONG, ULONG> val;
               val.first  = indices[0];
               val.second = indices[1];
               mArray2ModMap[frag.mpModifierValue] = val;
            }
         }
         break;

         case eDB2_MOD_OPTIONAL:
         {
            sDB2SimpleCondition con;

            // Parse condition to tokens (field ID operator value)
            bool bRC = sDB2Fragment::ParseCondition( frag.mpModifierValue, 
                                                     con.mID, 
                                                     con.mOperator,
                                                     con.mValue,
                                                     con.mbF2F );

            if (bRC == true)
            {
               mOptionalModMap[frag.mpModifierValue] = con;
            }
         }
         break;

         case eDB2_MOD_VARIABLE_ARRAY3:
         {
            sDB2SimpleExpression exp;

            // Parse condition to tokens (field ID operator value)
            bool bRC = sDB2Fragment::ParseExpression( frag.mpModifierValue, 
                                                      exp.mID, 
                                                      exp.mOperator,
                                                      exp.mValue,
                                                      exp.mbF2F );

            if (bRC == true)
            {
               mExpressionModMap[frag.mpModifierValue] = exp;
            }
         }
         break;
      }
   }

   return bOK;
}


/*===========================================================================
METHOD:
   CheckAndSetBasePath (Internal Method)

DESCRIPTION:
   Check and set the passed in path to something that is useful
  
PARAMETERS
   pBasePath   [ I ] - Base path

RETURN VALUE:
   std::string - The enum name (or error string if enum value is not found)
===========================================================================*/
std::string cCoreDatabase::CheckAndSetBasePath( LPCSTR pBasePath ) const
{
   std::string basePath = ".";
   if (pBasePath != 0 && pBasePath[0] != 0)
   {
      struct stat fileInfo;
      if (stat( pBasePath, &fileInfo ) == 0)
      {
         if (S_ISDIR( fileInfo.st_mode ) == true)
         {
            // It's a directory
            basePath = pBasePath;
         }
      }
   }

   return basePath;
}

/*===========================================================================
METHOD:
   LoadStructureTables (Internal Method)

DESCRIPTION:
   Load all tables related to structure (entity, struct, field, format spec)
  
PARAMETERS
   pBasePath   [ I ] - Base path to database files

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::LoadStructureTables( LPCSTR pBasePath )
{
   bool bRC = true;

   std::string basePath = CheckAndSetBasePath( pBasePath );
   basePath += "/";

   std::string fn = basePath;
   fn += DB2_FILE_PROTOCOL_FIELD;
   bRC &= LoadDB2Table( (LPCSTR)fn.c_str(), 
                        mEntityFields, 
                        false, 
                        DB2_TABLE_PROTOCOL_FIELD, 
                        *mpLog );

   fn = basePath;
   fn += DB2_FILE_PROTOCOL_STRUCT;
   bRC &= LoadDB2Table( (LPCSTR)fn.c_str(), 
                        mEntityStructs, 
                        false,
                        DB2_TABLE_PROTOCOL_STRUCT,
                        *mpLog );

   fn = basePath;
   fn += DB2_FILE_PROTOCOL_ENTITY;
   bRC &= LoadDB2Table( (LPCSTR)fn.c_str(), 
                        mProtocolEntities, 
                        false,
                        DB2_TABLE_PROTOCOL_ENTITY,
                        *mpLog );

   // Validate protocol entities
   bRC &= ValidateStructures();

   // Build internal protocol entity name map
   bRC &= AssembleEntityNameMap();

   return bRC;
}

/*===========================================================================
METHOD:
   LoadStructureTables (Internal Method)

DESCRIPTION:
   Load all tables related to structure (entity, struct, field)
  
PARAMETERS

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::LoadStructureTables()
{
   bool bRC = true;

   // Calculate sizes
   int nFieldSize =  (const char*)&_binary_QMI_Field_txt_end - 
                           (const char*)&_binary_QMI_Field_txt_start;
   int nStructSize = (const char*)&_binary_QMI_Struct_txt_end - 
                           (const char*)&_binary_QMI_Struct_txt_start;
   int nEntitySize = (const char*)&_binary_QMI_Entity_txt_end - 
                           (const char*)&_binary_QMI_Entity_txt_start;

   bRC &= LoadDB2Table( (const char*)&_binary_QMI_Field_txt_start,
                        nFieldSize, 
                        mEntityFields, 
                        false,
                        DB2_TABLE_PROTOCOL_FIELD,
                        *mpLog );

   bRC &= LoadDB2Table( (const char*)&_binary_QMI_Struct_txt_start,
                        nStructSize, 
                        mEntityStructs, 
                        false,
                        DB2_TABLE_PROTOCOL_STRUCT,
                        *mpLog );

   bRC &= LoadDB2Table( (const char*)&_binary_QMI_Entity_txt_start,
                        nEntitySize, 
                        mProtocolEntities, 
                        false,
                        DB2_TABLE_PROTOCOL_ENTITY,
                        *mpLog );

   // Validate protocol entities
   bRC &= ValidateStructures();

   // Build internal protocol entity name map
   bRC &= AssembleEntityNameMap();

   return bRC;
}

/*===========================================================================
METHOD:
   LoadEnumTables (Internal Method)

DESCRIPTION:
   Load all enumeration tables 
  
PARAMETERS
   pBasePath   [ I ] - Base path to database files

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::LoadEnumTables( LPCSTR pBasePath )
{
   bool bRC = true;

   std::string basePath = CheckAndSetBasePath( pBasePath );
   basePath += "/";


   std::string fn = basePath;
   fn += DB2_FILE_ENUM_MAIN;
   bRC &= LoadDB2Table( (LPCSTR)fn.c_str(), 
                        mEnumNameMap, 
                        false,
                        DB2_TABLE_ENUM_MAIN,
                        *mpLog );

   fn = basePath;
   fn += DB2_FILE_ENUM_ENTRY;
   bRC &= LoadDB2Table( (LPCSTR)fn.c_str(), 
                        mEnumEntryMap, 
                        false,
                        DB2_TABLE_ENUM_ENTRY,
                        *mpLog );

   // Build the enum map
   bRC &= AssembleEnumMap();

   return bRC;
}

/*===========================================================================
METHOD:
   LoadEnumTables (Internal Method)

DESCRIPTION:
   Load all enumeration tables 
  
PARAMETERS

RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::LoadEnumTables()
{
   bool bRC = true;
   // Calculate sizes
   int nEnumSize = (const char*)&_binary_QMI_Enum_txt_end - 
                           (const char*)&_binary_QMI_Enum_txt_start;
   int nEnumEntrySize = (const char*)&_binary_QMI_EnumEntry_txt_end -
                           (const char*)&_binary_QMI_EnumEntry_txt_start;

   bRC &= LoadDB2Table( (const char*)&_binary_QMI_Enum_txt_start,
                        nEnumSize,
                        mEnumNameMap, 
                        false,
                        DB2_TABLE_ENUM_MAIN,
                        *mpLog );

   bRC &= LoadDB2Table( (const char*)&_binary_QMI_EnumEntry_txt_start,
                        nEnumEntrySize,
                        mEnumEntryMap, 
                        false,
                        DB2_TABLE_ENUM_ENTRY,
                        *mpLog );

   // Build the enum map
   bRC &= AssembleEnumMap();

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateStructures (Internal Method)

DESCRIPTION:
   Validate (and attempt repair of) structure related tables
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateStructures()
{
   // Assume success
   bool bRC = true;

   tDB2EntityMap::iterator pEntity = mProtocolEntities.begin();
   while (pEntity != mProtocolEntities.end())
   {
      sDB2ProtocolEntity & entity = pEntity->second;

      // Structure ID given?
      if (entity.mStructID != -1)
      {
         // Yes, validate individual structure
         std::set <ULONG> fields;
         bool bValid = ValidateStructure( (ULONG)entity.mStructID, fields, 0 );

         // Not valid?
         if (bValid == false)
         {
            // Invalid structure, reset to none
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                << "] Invalid struct, ID " << entity.mStructID;
            
            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            entity.mStructID = -1;

            // We found at least one bad structure
            bRC = false;
         }
      }

      pEntity++;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateStructure (Internal Method)

DESCRIPTION:
   Validate a single structure

PARAMETERS:
   structID    [ I ] - ID of structure being evaluated
   fields      [I/O] - List of 'known' field IDs
   depth       [I/O] - Recursion depth
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateStructure(
   ULONG                      structID,
   std::set <ULONG> &         fields,
   ULONG                      depth )
{
   // Assume success
   bool bRC = true;
   
   // Reached our limit?
   if (depth++ >= MAX_NESTING_LEVEL)
   {
      // Invalid structure
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Max depth exceeded, possible loop, struct ID " << structID;
      
      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

      return false;
   }

   // Grab first fragment of structure
   std::pair <ULONG, ULONG> id( structID, 0 );
   tDB2FragmentMap::const_iterator pFrag = mEntityStructs.find( id );

   // Did we find the first fragment?
   if (pFrag == mEntityStructs.end())
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Missing initial fragment, struct ID " << structID;
      
      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );
      
      bRC = false;
   }

   // Iterate over each fragment in the structure
   while (pFrag != mEntityStructs.end() && pFrag->second.mStructID == structID)
   {
      // Grab fragment
      const sDB2Fragment & frag = pFrag->second;

      // Variable array or optional fragment?
      if ( (frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY)
      ||   (frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY2) )      
      {
         bRC = ValidateArraySpecifier( frag, fields );
      }
      else if (frag.mModifierType == eDB2_MOD_OPTIONAL)
      {
         bRC = ValidateOptionalSpecifier( frag, fields );
      }
      else if (frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY3)
      {
         bRC = ValidateExpressionSpecifier( frag, fields );
      }
      else if ( (frag.mModifierType == eDB2_MOD_VARIABLE_STRING1)
      ||        (frag.mModifierType == eDB2_MOD_VARIABLE_STRING2)
      ||        (frag.mModifierType == eDB2_MOD_VARIABLE_STRING3) )
      {
         bRC = ValidateArraySpecifier( frag, fields );
         if (bRC == true)
         {
            // The field being modified has to be a fixed length string
            ULONG fieldID = frag.mFragmentValue;
            tDB2FieldMap::const_iterator pIter = mEntityFields.find( fieldID );
            if (pIter != mEntityFields.end())
            {
               bool bString = false;

               const sDB2Field & ft = pIter->second;
               if (ft.mType == eDB2_FIELD_STD)
               {
                  if ( (ft.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_A)
                  ||   (ft.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_U)
                  ||   (ft.mTypeVal == (ULONG)eDB2_FIELD_STDTYPE_STRING_U8) )
                  {
                     if ( (ft.mTypeVal != (ULONG)eDB2_FIELD_STDTYPE_STRING_U8)
                     ||   (frag.mModifierType != eDB2_MOD_VARIABLE_STRING3) )
                     {
                        // Not the invalid combination of character length and
                        // varaible length characters
                        bString = true;
                     }
                  }
               }

               if (bString == false)
               {
                  // Not a string so why the string modifier?
                  std::ostringstream tmp;
                  tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                      << "] Invalid string modifier, struct ID " << structID
                      << ", ID " << fieldID;
                  
                  mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

                  bRC = false;
               }
            }
         }
      }

      if (bRC == true)
      {
         // What type of fragment is this?
         switch (frag.mFragmentType)
         {
            case eDB2_FRAGMENT_FIELD:
            {
               ULONG fieldID = frag.mFragmentValue;
               bRC = ValidateField( structID, fieldID, fields );
            }
            break;

            case eDB2_FRAGMENT_VARIABLE_PAD_BITS:
            case eDB2_FRAGMENT_VARIABLE_PAD_BYTES:
            {
               // Does this field exist in the entity?
               ULONG fieldID = frag.mFragmentValue;               
               if (fields.find( fieldID ) == fields.end())
               {
                  // No
                  std::ostringstream tmp;
                  tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                      << "] Invalid pad, struct ID " << structID
                      << ", ID " << fieldID;
                  
                  mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

                  bRC = false;
               }
            }
            break;

            case eDB2_FRAGMENT_STRUCT:                     
            {
               // Grab structure ID and recurse
               ULONG structID = frag.mFragmentValue;
               bRC = ValidateStructure( structID, fields, depth );
            }
            break;

            default:
               break;
         }
      }

      // Did an error occur?
      if (bRC == false)
      {
         break;
      }

      pFrag++;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateField (Internal Method)

DESCRIPTION:
   Validate a single field

PARAMETERS:
   structID    [ I ] - ID of referencing structure
   fieldID     [ I ] - ID of field being evaluated
   fields      [I/O] - List of 'known' field IDs
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateField(
   ULONG                      structID,
   ULONG                      fieldID,
   std::set <ULONG> &         fields )
{
   // Assume success
   bool bRC = true;

   tDB2FieldMap::const_iterator pIter = mEntityFields.find( fieldID );
   if (pIter == mEntityFields.end())
   {
      // No
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Invalid field, struct ID " << structID
          << ", ID " << fieldID;
      
      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

      bRC = false;
   }
   else
   {
      // Mark field as part of this structure
      fields.insert( fieldID );

      // Is this field an enumeration?
      const sDB2Field & theField = pIter->second;
      if ( (theField.mType == eDB2_FIELD_ENUM_UNSIGNED)
      ||   (theField.mType == eDB2_FIELD_ENUM_SIGNED) )
      {
         // Yes, check that the enum exists
         if (mEnumNameMap.find( theField.mTypeVal ) == mEnumNameMap.end())
         {
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_PROTOCOL_FIELD 
                << "] Invalid enumeration ID, field ID " << fieldID
                << ", enum ID " << theField.mTypeVal;
            
            mpLog->Log( tmp.str(), eDB2_STATUS_WARNING );
         }
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateArraySpecifier (Internal Method)

DESCRIPTION:
   Validate an array specifier

PARAMETERS:
   frag        [ I ] - Fragment containing array specifier
   fields      [ I ] - List of 'known' field IDs
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateArraySpecifier(
   const sDB2Fragment &      frag,
   const std::set <ULONG> &   fields )
{
   // Assume success
   bool bRC = true;

   // Even an array specifier to start with?
   if (frag.mpModifierValue == 0 || frag.mpModifierValue == EMPTY_STRING)
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Missing array specifier, struct ID " << frag.mStructID;
      
      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

      bRC = false;
   }
   else if ( (frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY)
   ||        (frag.mModifierType == eDB2_MOD_VARIABLE_STRING1)
   ||        (frag.mModifierType == eDB2_MOD_VARIABLE_STRING2)
   ||        (frag.mModifierType == eDB2_MOD_VARIABLE_STRING3) )
   {
      ULONG id = strtoul( frag.mpModifierValue, 0, 0 );
      if (fields.find( id ) == fields.end())
      {
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
             << "] Invalid modifier specifier, struct ID " << frag.mStructID
             << ", ID " << id;
         
         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );
   
         bRC = false;
      }
   }
   else if (frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY2)
   {
      // Parse condition to tokens (start stop)
      int nSize = strlen( frag.mpModifierValue ) + 1;
      
      char * pCopy = new char[ nSize ];
      if (pCopy == NULL)
      {
         return false;
      }
      
      memcpy( pCopy, frag.mpModifierValue, nSize );

      std::vector <ULONG> indices; 
      CSVStringToContainer( " ", pCopy, indices );    

      delete [] pCopy;
      if (indices.size() != 2)
      {
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
             << "] Invalid array specifier, struct ID " << frag.mStructID
             << ", " << frag.mpModifierValue;

         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );
   
         bRC = false;
      }
      else
      {
         ULONG sID = indices[0];
         ULONG eID = indices[1];

         if ( (fields.find( sID ) == fields.end())
         ||   (fields.find( eID ) == fields.end()) )
         {
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                << "] Invalid array specifier, struct ID " << frag.mStructID
                << ", IDs " << sID << " " << eID;

            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            bRC = false;
         }
      }
   }
   else
   {
      ASSERT( 0 );
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateOptionalSpecifier (Internal Method)

DESCRIPTION:
   Validate a simple optional fragment specifier

PARAMETERS:
   frag        [ I ] - Fragment containing optional fragment specifier
   fields      [ I ] - List of 'known' field IDs
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateOptionalSpecifier(
   const sDB2Fragment &       frag,
   const std::set <ULONG> &   fields )
{
   // Assume success
   bool bRC = true;

   ASSERT( frag.mModifierType == eDB2_MOD_OPTIONAL );

   // Even an optional specifier to start with?
   if (frag.mpModifierValue == 0 || frag.mpModifierValue == EMPTY_STRING)
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Missing optional specifier, struct ID " << frag.mStructID;

      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

      return false;
   }

   ULONG conID;
   eDB2Operator conOp;
   LONGLONG conVal;
   bool bF2F;

   // Parse condition
   LPCSTR pCon = frag.mpModifierValue;
   bRC = sDB2Fragment::ParseCondition( pCon, conID, conOp, conVal, bF2F );
   if (bRC == true)
   {
      // Does the given field ID exist as part of this entity?
      if (fields.find( conID ) == fields.end())
      {
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
             << "] Invalid optional specifier, struct ID " << frag.mStructID
             << ", unknown field ID " << conID << "/" << frag.mpModifierValue;

         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

         bRC = false;
      }

      if (bF2F == true)
      {
         // Does the given field ID exist as part of this entity?
         if (fields.find( (ULONG)conVal ) == fields.end())
         {
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                << "] Invalid optional specifier, struct ID " << frag.mStructID
                << ", unknown field ID " << (ULONG)conVal 
                << "/" << frag.mpModifierValue;

            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            bRC = false;
         }
      }
   }
   else
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Invalid optional specifier, struct ID " << frag.mStructID
          << ", " << frag.mpModifierValue;

      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ValidateExpressionSpecifier (Internal Method)

DESCRIPTION:
   Validate a simple expression fragment specifier

PARAMETERS:
   frag        [ I ] - Fragment containing expression fragment specifier
   fields      [ I ] - List of 'known' field IDs
  
RETURN VALUE:
   bool
===========================================================================*/
bool cCoreDatabase::ValidateExpressionSpecifier(
   const sDB2Fragment &       frag,
   const std::set <ULONG> &   fields )
{
   // Assume success
   bool bRC = true;

   ASSERT( frag.mModifierType == eDB2_MOD_VARIABLE_ARRAY3 );

   // Even an expression specifier to start with?
   if (frag.mpModifierValue == 0 || frag.mpModifierValue == EMPTY_STRING)
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Missing array specifier, struct ID " << frag.mStructID;

      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

      return false;
   }

   ULONG exprID;
   eDB2ExpOperator exprOp;
   LONGLONG exprVal;
   bool bF2F;

   // Parse expression
   LPCSTR pExpr = frag.mpModifierValue;
   bRC = sDB2Fragment::ParseExpression( pExpr, exprID, exprOp, exprVal, bF2F );
   if (bRC == true)
   {
      // Does the given field ID exist as part of this entity?
      if (fields.find( exprID ) == fields.end())
      {
         std::ostringstream tmp;
         tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
             << "] Invalid optional specifier, struct ID " << frag.mStructID
             << ", unknown field ID " << exprID 
             << "/" << frag.mpModifierValue;

         mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

         bRC = false;
      }

      if (bF2F == true)
      {
         // Does the given field ID exist as part of this entity?
         if (fields.find( (ULONG)exprVal ) == fields.end())
         {
            std::ostringstream tmp;
            tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
                << "] Invalid optional specifier, struct ID " << frag.mStructID
                << ", unknown field ID " << (ULONG)exprID 
                << "/" << frag.mpModifierValue;

            mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );

            bRC = false;
         }
      }
   }
   else
   {
      std::ostringstream tmp;
      tmp << "DB [" << DB2_TABLE_PROTOCOL_STRUCT 
          << "] Invalid optional specifier, struct ID " << frag.mStructID
          << ", " << frag.mpModifierValue;

      mpLog->Log( tmp.str(), eDB2_STATUS_ERROR );
   }

   return bRC;
}

