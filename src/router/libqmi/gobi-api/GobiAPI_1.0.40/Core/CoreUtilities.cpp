/*===========================================================================
FILE: 
   CoreUtilities.cpp

DESCRIPTION:
   Collection of various utility methods

PUBLIC CLASSES AND METHODS:
   StringToLONG
   StringToULONG
   StringToLONGLONG
   StringToULONGLONG

   ParseTokens()
   ParseCommandLine()
   ParseFormatSpecifier()

   FromString( CHAR )
   FromString( UCHAR )
   FromString( SHORT )
   FromString( USHORT )
   FromString( int )
   FromString( UINT )
   FromString( LONG )
   FromString( ULONG )
   FromString( LONGLONG )
   FromString( ULONGLONG )

   ToString( CHAR )
   ToString( UCHAR )
   ToString( SHORT )
   ToString( USHORT )
   ToString( int )
   ToString( UINT )
   ToString( LONG )
   ToString( ULONG )
   ToString( LONGLONG )
   ToString( ULONGLONG )

   ContainerToCSVString()
   CSVStringToContainer()
   CSVStringToValidatedContainer()

   SetDiff()
   SetIntersection()
   SetUnion()

   GetProgramPath()
   IsFolder()
   EnumerateFolders()
   IsHidden()
   DepthSearch()

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
#include "CoreUtilities.h"

#include <climits>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Format specifier states
enum eFormatState
{
   eFMT_STATE_NORMAL,      // [0] Normal state; outputting literal characters
   eFMT_STATE_PERCENT,     // [1] Just read '%'
   eFMT_STATE_FLAG,        // [2] Just read flag character
   eFMT_STATE_WIDTH,       // [3] Just read width specifier
   eFMT_STATE_DOT,         // [4] Just read '.'
   eFMT_STATE_PRECIS,      // [5] Just read precision specifier
   eFMT_STATE_SIZE,        // [6] Just read size specifier
   eFMT_STATE_TYPE,        // [7] Just read type specifier
   eFMT_STATE_INVALID,     // [8] Invalid format

   eFMT_STATES             // [9] Number of format states
};

// Format specifier character classes
enum eFormatCharClass
{
   eFMT_CH_CLASS_OTHER,    // [0] Character with no special meaning
   eFMT_CH_CLASS_PERCENT,  // [1] '%'
   eFMT_CH_CLASS_DOT,      // [2] '.'
   eFMT_CH_CLASS_STAR,     // [3] '*'
   eFMT_CH_CLASS_ZERO,     // [4] '0'
   eFMT_CH_CLASS_DIGIT,    // [5] '1'..'9'
   eFMT_CH_CLASS_FLAG,     // [6] ' ', '+', '-', '#'
   eFMT_CH_CLASS_SIZE,     // [7] 'h', 'l', 'L', 'N', 'F', 'w'
   eFMT_CH_CLASS_TYPE      // [8] Type specifying character
};

// Lookup table for determining class of a character (lower nibble) 
// and next format specifier state (upper nibble)
const UCHAR gLookupTable[] = 
{
   0x06, // ' ', FLAG
   0x80, // '!', OTHER
   0x80, // '"', OTHER
   0x86, // '#', FLAG
   0x80, // '$', OTHER
   0x81, // '%', PERCENT 
   0x80, // '&', OTHER
   0x00, // ''', OTHER
   0x00, // '(', OTHER
   0x10, // ')', OTHER
   0x03, // '*', STAR
   0x86, // '+', FLAG
   0x80, // ',', OTHER
   0x86, // '-', FLAG
   0x82, // '.', DOT
   0x80, // '/', OTHER
   0x14, // '0', ZERO
   0x05, // '1', DIGIT
   0x05, // '2', DIGIT
   0x45, // '3', DIGIT
   0x45, // '4', DIGIT
   0x45, // '5', DIGIT
   0x85, // '6', DIGIT
   0x85, // '7', DIGIT
   0x85, // '8', DIGIT
   0x05, // '9', DIGIT
   0x00, // :!', OTHER
   0x00, // ';', OTHER
   0x30, // '<', OTHER
   0x30, // '=', OTHER
   0x80, // '>', OTHER
   0x50, // '?', OTHER
   0x80, // '@', OTHER
   0x80, // 'A', OTHER
   0x00, // 'B', OTHER
   0x08, // 'C', TYPE
   0x00, // 'D', OTHER
   0x28, // 'E', TYPE
   0x27, // 'F', SIZE
   0x38, // 'G', TYPE
   0x50, // 'H', OTHER
   0x57, // 'I', SIZE
   0x80, // 'J', OTHER
   0x00, // 'K', OTHER
   0x07, // 'L', SIZE
   0x00, // 'M', OTHER
   0x37, // 'N', SIZE
   0x30, // 'O', OTHER
   0x30, // 'P', OTHER
   0x50, // 'Q', OTHER
   0x50, // 'R', OTHER
   0x88, // 'S', TYPE
   0x00, // 'T', OTHER
   0x00, // 'U', OTHER
   0x00, // 'V', OTHER
   0x20, // 'W', OTHER
   0x28, // 'X', TYPE
   0x80, // 'Y', OTHER
   0x88, // 'Z', TYPE
   0x80, // '[', OTHER
   0x80, // '\', OTHER
   0x00, // ']', OTHER
   0x00, // '^', OTHER
   0x00, // '-', OTHER
   0x60, // '`', OTHER
   0x60, // 'a', OTHER
   0x60, // 'b', OTHER
   0x68, // 'c', TYPE
   0x68, // 'd', TYPE
   0x68, // 'e', TYPE
   0x08, // 'f', TYPE
   0x08, // 'g', TYPE
   0x07, // 'h', SIZE
   0x78, // 'i', TYPE
   0x70, // 'j', OTHER
   0x70, // 'k', OTHER
   0x77, // 'l', SIZE
   0x70, // 'm', OTHER
   0x70, // 'n', OTHER
   0x08, // 'o', TYPE
   0x08, // 'p', TYPE
   0x00, // 'q', OTHER
   0x00, // 'r', OTHER
   0x08, // 's', TYPE
   0x00, // 't', OTHER
   0x08, // 'u', TYPE
   0x00, // 'v', OTHER
   0x07, // 'w', SIZE
   0x08  // 'x', TYPE
};

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   IsWhitespace (Private Free Method)

DESCRIPTION:
   Is this whitespace?

PARAMETERS:   
   pStr          [ I ] - The string 

RETURN VALUE:
   bool
===========================================================================*/
static bool IsWhitespace( LPCSTR pStr )
{
   bool bWS = false;

   int c = (int)*pStr;
   if (isspace( c ))
   {
      bWS = true;
   }

   return bWS;
}

/*===========================================================================
METHOD:
   IsHexadecimalString (Private Free Method)

DESCRIPTION:
   Is this a hexadecimal digits string?

PARAMETERS:   
   pStr          [ I ] - The string 

RETURN VALUE:
   bool
===========================================================================*/
static bool IsHexadecimalString( LPCSTR pStr )
{
   // Assume not
   bool bHex = false;

   // Skip whitespace
   LPCSTR pTmp = pStr;
   while (IsWhitespace( pTmp ) == true)
   {
      pTmp++;
   }

   // Skip leading +/-
   CHAR ch = *pTmp;
   if (ch == '+' || ch == '-')
   {
      pTmp++;
   }

   if (*pTmp == '0')
   {
      pTmp++;
      if (*pTmp == 'x' || *pTmp == 'X')
      {
         bHex = true;
      }
   }

   return bHex;
}

/*===========================================================================
METHOD:
   IsNegativeString (Private Free Method)

DESCRIPTION:
   Is this a string starting with a negative sign?

PARAMETERS:   
   pStr          [ I ] - The string 

RETURN VALUE:
   bool
===========================================================================*/
static bool IsNegativeString( LPCSTR pStr )
{
   // Assume not
   bool bNeg = false;

   // Skip whitespace
   LPCSTR pTmp = pStr;
   while (IsWhitespace( pTmp ) == true)
   {
      pTmp++;
   }

   CHAR ch = *pTmp;
   if (ch == '-')
   {
      bNeg = true;
   }

   return bNeg;
}

/*===========================================================================
METHOD:
   StringToLONG (Free Method)

DESCRIPTION:
   Replacement/front end for strtol
  
   NOTE: strtol does not correctly handle a negative integer
   when specified in hexadecimal, so we have to check for that
   first

PARAMETERS:   
   pStr          [ I ] - The string 
   base          [ I ] - Base for conversion
   val           [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool StringToLONG( 
   LPCSTR                    pStr,
   int                        base,
   LONG &                     val )
{
   // Assume failure
   bool bOK = false;
   if (pStr == 0 || *pStr == 0)
   {
      return bOK;
   }
  
   // Hexadecimal?
   if (base == 16 || (base == 0 && IsHexadecimalString( pStr ) == true))
   {
      // No negative hexadecimal strings allowed
      if (IsNegativeString( pStr ) == false)
      {
         // Reset error
         errno = 0;

         // Use the unsigned version, then cast
         LPSTR pEnd = (LPSTR)pStr;
         ULONG tmpVal = strtoul( pStr, &pEnd, base );
         if (tmpVal != ULONG_MAX || errno != ERANGE)
         {
            // Where did we end?
            if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
            {
               // Success!
               val = (LONG)tmpVal;
               bOK = true;
            }
         }
      }
   }
   else
   {
      // Proceed as normal
      LPSTR pEnd = (LPSTR)pStr;
      LONG tmpVal = strtol( pStr, &pEnd, base );
      if ((tmpVal != LONG_MAX && tmpVal != LONG_MIN) || errno != ERANGE)
      {
         // Where did we end?
         if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
         {             
            // Success!
            val = tmpVal;
            bOK = true;
         }
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   StringToULONG (Free Method)

DESCRIPTION:
   Replacement/front end for strtoul
  
PARAMETERS:   
   pStr          [ I ] - The string 
   base          [ I ] - Base for conversion
   val           [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool StringToULONG( 
   LPCSTR                    pStr,
   int                        base,
   ULONG &                    val )
{
   // Assume failure
   bool bOK = false;
   if (pStr == 0 || *pStr == 0)
   {
      return bOK;
   }

   // No negative strings allowed
   if (IsNegativeString( pStr ) == true)
   {
      return bOK;
   }
   
   // Reset error
   errno = 0;

   LPSTR pEnd = (LPSTR)pStr;
   ULONG tmpVal = strtoul( pStr, &pEnd, base );
   if (tmpVal != ULONG_MAX || errno != ERANGE)
   {
      if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
      {
         // Success!
         val = tmpVal;
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   StringToLONGLONG (Free Method)

DESCRIPTION:
   Replacement/front end for strtoll
  
   NOTE: strtoll does not correctly handle a negative integer
   when specified in hexadecimal, so we have to check for that
   first

PARAMETERS:   
   pStr          [ I ] - The string 
   base          [ I ] - Base for conversion
   val           [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool StringToLONGLONG( 
   LPCSTR                     pStr,
   int                        base,
   LONGLONG &                 val )
{
   // Assume failure
   bool bOK = false;
   if (pStr == 0 || *pStr == 0)
   {
      return bOK;
   }

   if (base == 16 || (base == 0 && IsHexadecimalString( pStr ) == true))
   {
      // No negative hexadecimal strings allowed
      if (IsNegativeString( pStr ) == false)
      {
         // Reset error
         errno = 0;

         // Use the unsigned version, then cast
         LPSTR pEnd = (LPSTR)pStr;
         ULONGLONG tmpVal = strtoull( pStr, &pEnd, base );
         if (tmpVal != ULLONG_MAX || errno != ERANGE)
         {
            // Where did we end?
            if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
            {
               // Success!
               val = (LONGLONG)tmpVal;
               bOK = true;
            }
         }
      }
   }
   else
   {
      // Proceed as normal
      LPSTR pEnd = (LPSTR)pStr;
      LONGLONG tmpVal = strtoll( pStr, &pEnd, base );
      if ((tmpVal != LLONG_MAX && tmpVal != LLONG_MIN) || errno != ERANGE)
      {
         if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
         {
            // Success!
            val = tmpVal;
            bOK = true;
         }
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   StringToULONGLONG (Free Method)

DESCRIPTION:
   Replacement/front end for strtouill
  
PARAMETERS:   
   pStr          [ I ] - The string 
   base          [ I ] - Base for conversion
   val           [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool StringToULONGLONG( 
   LPCSTR                    pStr,
   int                        base,
   ULONGLONG &                val )
{
   // Assume failure
   bool bOK = false;
   if (pStr == 0 || *pStr == 0)
   {
      return bOK;
   }
 
   // No negative strings allowed
   if (IsNegativeString( pStr ) == true)
   {
      return bOK;
   }
   
   // Reset error
   errno = 0;

   LPSTR pEnd = (LPSTR)pStr;
   ULONGLONG tmpVal = strtoull( pStr, &pEnd, base );
   if (tmpVal != ULLONG_MAX || errno != ERANGE)
   {
      if (pEnd != pStr && (*pEnd == 0 || IsWhitespace( pEnd ) == true))
      {
         // Success!
         val = tmpVal;
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ParseCommandLine (Free Method)

DESCRIPTION:
   Parse a command line to tokens (a command line is a string of
   space delimited values where a value that contains space is
   enclosed in text)
  
PARAMETERS:
   commandLine [ I ] - The characters separating tokens
   tokens      [ O ] - The resultant vector of tokens

RETURN VALUE:
   None
===========================================================================*/
void ParseCommandLine(
   std::string                    commandLine,
   std::vector <std::string> &    tokens )
{
   ULONG count = (ULONG)commandLine.size();

   for (ULONG nEndToken = 0; nEndToken < count;)
   {
      // Skip leading spaces
      int nStartToken = commandLine.find_first_not_of( " ", nEndToken );
      if (nStartToken == -1)
      {
         // All that is left is spaces
         return;
      }

      int stringLength = 0;

      // In Quotes? If so ignore spaces until next quote
      if (commandLine[ nStartToken ] == '\"')
      {
         nStartToken++;
         nEndToken = commandLine.find( '\"', nStartToken );
         if (nEndToken == -1)
         {
            // Unable to find trailing quote, fail
            return;
         }
         stringLength = nEndToken - nStartToken;
         nEndToken++;
      }
      else
      {
         nEndToken = commandLine.find( ' ', nStartToken );
         if (nEndToken == -1)
         {
            // Unable to find trailing space, use end
            nEndToken = commandLine.size();
         }

         stringLength = nEndToken - nStartToken;
      }

      std::string newToken = commandLine.substr( nStartToken, stringLength );
      tokens.push_back( newToken );
   }
}

/*===========================================================================
METHOD:
   ParseTokens (Free Method)

DESCRIPTION:
   Parse a line into individual tokens

   NOTE: No attempt is made to handle accidental separators, i.e. searching
   for ',' on 'foo, bar, "foo, bar"' will return four tokens, not three so
   pick something like '^' instead of ','!
  
PARAMETERS:
   pSeparator  [ I ] - The characters separating tokens
   pLine       [ I ] - The string being parsed
   tokens      [ O ] - The resultant vector of tokens

RETURN VALUE:
   None
===========================================================================*/
void ParseTokens(
   LPCSTR                    pSeparator,
   LPSTR                     pLine,
   std::vector <LPSTR> &     tokens )
{
   if (pSeparator != 0 && pSeparator[0] != 0 && pLine != 0 && pLine[0] != 0)
   {
      LPSTR pToken = strtok( pLine, pSeparator );
      while (pToken != 0)
      {
         // Store token
         tokens.push_back( pToken );

         // Get next token:
         pToken = strtok( 0, pSeparator );
      }
   }
}

/*===========================================================================
METHOD:
   ParseFormatSpecifier (Free Method)

DESCRIPTION:
   Parse a format specifier into individual format type tokens
  
PARAMETERS:
   pFmt        [ I ] - The format specifier (must be NULL terminated)
   fmtLen      [ I ] - Length of above format specifier
   fmtTypes    [ O ] - The individual format type tokens ('d', 'u', 'us' etc.)

RETURN VALUE:
   bool - Valid format specifier?
===========================================================================*/
bool ParseFormatSpecifier( 
   LPCSTR                    pFmt,
   ULONG                      fmtLen,
   std::vector <CHAR> &      fmtTypes )
{
   // Assume failure
   bool bOK = false;

   // Make sure string is NULL terminated
   CHAR ch; 
   ULONG chars = 0;
   while (chars < fmtLen)
   {
      if (pFmt[chars] == '\0')
      {
         break;
      }
      else
      {
         chars++;
      }
   }

   if (pFmt[chars] != '\0')
   {
      return bOK;
   }

   // Extract individual format type tokens
   eFormatState state = eFMT_STATE_NORMAL; 
   eFormatCharClass cc = eFMT_CH_CLASS_OTHER;
   while ((ch = *pFmt++) != '\0' && state != eFMT_STATE_INVALID) 
   {
      // Find character class
      cc = eFMT_CH_CLASS_OTHER;
      if (ch >= ' ' && ch <= 'x')
      {
         cc = (eFormatCharClass)(gLookupTable[ch - ' '] & 0xF);
      }

      // Find next state
      state = (eFormatState)(gLookupTable[cc * eFMT_STATES + (state)] >> 4);
      switch (state) 
      {
         case eFMT_STATE_NORMAL:
         NORMAL_STATE:
            break;

         case eFMT_STATE_PERCENT:
         case eFMT_STATE_FLAG:
         case eFMT_STATE_DOT:
            break;

         case eFMT_STATE_WIDTH:
         case eFMT_STATE_PRECIS:
            if (ch == '*') 
            {
               fmtTypes.push_back( ch );
            }
            break;

         case eFMT_STATE_SIZE:
            switch (ch) 
            {
               case 'l':
                  if (*pFmt == 'l')
                  {
                     ++pFmt;
                  }
                  break;

               case 'I':
                  if ( (*pFmt == '6') && (*(pFmt + 1) == '4') )
                  {
                     pFmt += 2;
                  }
                  else if ( (*pFmt == '3') && (*(pFmt + 1) == '2') )
                  {
                     pFmt += 2;
                  }
                  else if ( (*pFmt == 'd') 
                        ||  (*pFmt == 'i')
                        ||  (*pFmt == 'o')
                        ||  (*pFmt == 'u')
                        ||  (*pFmt == 'x')
                        ||  (*pFmt == 'X') ) 
                  {
                     // Nothing further needed
                  }
                  else 
                  {
                     state = eFMT_STATE_NORMAL;
                     goto NORMAL_STATE;
                  }
                  break;

                  case 'h':
                  case 'w':
                     break;
               }
               break;

         case eFMT_STATE_TYPE:
            fmtTypes.push_back( ch );
            break;
      }
    }

   bOK = (state == eFMT_STATE_NORMAL || state == eFMT_STATE_TYPE);
   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   CHAR &                     theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      LONG val = LONG_MAX;
      bOK = StringToLONG( pStr, 0, val );
      if (bOK == true)
      {
         // Reset status
         bOK = false;

         // Was this provided as a hexadecimal string?
         if (IsHexadecimalString( pStr ) == true)
         {
            // Yes, the return value is a LONG, so check against
            // the maximum range for a UCHAR, before casting to
            // a CHAR (to pick sign back up)
            if (val <= UCHAR_MAX)
            {
               // Success!
               theType = (CHAR)val;
               bOK = true;
            }           
         }          
         else if (val >= SCHAR_MIN && val <= SCHAR_MAX)
         {
            // Success!
            theType = (CHAR)val;
            bOK = true;
         }
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   UCHAR &                    theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      ULONG val = ULONG_MAX;
      bOK = StringToULONG( pStr, 0, val );
      if (bOK == true && val <= UCHAR_MAX)
      {
         // Success!
         theType = (UCHAR)val;         
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   SHORT &                    theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      LONG val = LONG_MAX;
      bOK = StringToLONG( pStr, 0, val );
      if (bOK == true)
      {
         // Reset status
         bOK = false;

         // Was this provided as a hexadecimal string?
         if (IsHexadecimalString( pStr ) == true)
         {
            // Yes, the return value is a LONG, so check against
            // the maximum range for a USHORT, before casting to
            // a SHORT (to pick sign back up)
            if (val <= USHRT_MAX)
            {
               // Success!
               theType = (SHORT)val;
               bOK = true;
            }
         }
         else if (val >= SHRT_MIN && val <= SHRT_MAX)
         {
            // Success!
            theType = (SHORT)val;
            bOK = true;
         }
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   USHORT &                   theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      ULONG val = ULONG_MAX;
      bOK = StringToULONG( pStr, 0, val );
      if (bOK == true && val <= USHRT_MAX)
      {
         // Success!
         theType = (USHORT)val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   int &                      theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      LONG val = LONG_MAX;
      bOK = StringToLONG( pStr, 0, val );
      if (bOK == true && (val >= INT_MIN && val <= INT_MAX))
      {
         // Success!
         theType = (int)val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   UINT &                     theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      ULONG val = ULONG_MAX;
      bOK = StringToULONG( pStr, 0, val );
      if (bOK == true && val <= UINT_MAX)
      {
         // Success!
         theType = (UINT)val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   LONG &                     theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      LONG val = LONG_MAX;
      bOK = StringToLONG( pStr, 0, val );
      if (bOK == true)
      {
         // Success!
         theType = val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   ULONG &                    theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      ULONG val = ULONG_MAX;
      bOK = StringToULONG( pStr, 0, val );
      if (bOK == true)
      {
         // Success!
         theType = val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   LONGLONG &                 theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      LONGLONG val = LLONG_MAX;
      bOK = StringToLONGLONG( pStr, 0, val );
      if (bOK == true)
      {
         // Success!
         theType = val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   FromString (Free Method)

DESCRIPTION:
   Convert a string to a value
  
PARAMETERS:   
   pStr          [ I ] - The string 
   theType       [ O ] - Resulting value

RETURN VALUE:
   bool
===========================================================================*/
bool FromString( 
   LPCSTR                    pStr,
   ULONGLONG &                theType )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      ULONGLONG val = ULLONG_MAX;
      bOK = StringToULONGLONG( pStr, 0, val );
      if (bOK == true)
      {
         // Success!
         theType = val;
      }
      else
      {
         bOK = false;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   CHAR                       val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int tmp = (int)val;
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%d", 
                         tmp ); 

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   UCHAR                      val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                       (size_t)(SUGGESTED_BUFFER_LEN - 1),
                       "%u", 
                       (UINT)val );

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   SHORT                      val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int tmp = (int)val;
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%d", 
                         tmp ); 

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   USHORT                     val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%u", 
                         (UINT)val );

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   int                        val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%d", 
                         val ); 

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}


/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   UINT                       val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%u", 
                         val );

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   LONG                       val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%ld", 
                         val ); 

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}


/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   ULONG                      val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%lu", 
                         val );

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   LONGLONG                   val,
   LPSTR                      pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%lld", 
                         val ); 

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   ToString (Free Method)

DESCRIPTION:
   Convert a value to a string
  
PARAMETERS:   
   val           [ I ] - The value to convert
   pStr          [ O ] - The resulting string 

RETURN VALUE:
   bool
===========================================================================*/
bool ToString( 
   ULONGLONG                  val,
   LPSTR                     pStr )
{
   // Assume failure
   bool bOK = false;

   if (pStr != 0)
   {
      int rc = snprintf( pStr, 
                         (size_t)(SUGGESTED_BUFFER_LEN - 1),
                         "%llu", 
                         val );

      if (rc < 0)
      {
         pStr[0] = 0;
      }
      else
      {
         // Success!
         bOK = true;
      }
   }

   return bOK;
}

/*===========================================================================
METHOD:
   GetProgramPath (Public Method)

DESCRIPTION:
   Return the special folder used for storing program files
   
RETURN VALUE:
   std::string (Static value of "/opt/Qualcomm/Gobi/")
===========================================================================*/
std::string GetProgramPath()
{
   // If running programs's path is desired we could
   //   use readlink with /proc/getpid()/exe

   // Just using static path, as we don't want them to move it
   std::string path = "/opt/Qualcomm/Gobi/";

   return path;

}

/*===========================================================================
METHOD:
   IsFolder (Free Method)

DESCRIPTION:
   Helper function for EnumerateFolders, tells if a dirent is a folder.
   This reduces the memory usage by scandir, as compared to checking after
   scandir returns.

PARAMETERS:
   pFile         [ I ] - dirent structure describing file
  
RETURN VALUE:
   int:  zero    - Ignore this file
         nonzero - Process this file
===========================================================================*/
int IsFolder( const struct dirent * pFile )
{
   // Ignore anything beginning with a '.'
   if (pFile->d_name[0] == '.')
   {
      return 0;
   }

   return (pFile->d_type == DT_DIR ? 1 : 0);
}

/*===========================================================================
METHOD:
   EnumerateFolders (Public Method)

DESCRIPTION:
   Enumerate the subfolders of the given folder (recursive)

PARAMETERS:
   baseFolder     [ I ] - Folder to search in
   foundFolders   [ O ] - Fully qualified paths of found folders
  
RETURN VALUE:
   None
===========================================================================*/
void EnumerateFolders( 
   const std::string &           baseFolder,
   std::vector <std::string> &   foundFolders )
{
   if (baseFolder.size() == 0)
   {
      return;
   }

   std::string folderSearch = baseFolder;

   // Add trailing / if not present
   int folderLen = folderSearch.size();
   if (folderSearch[folderLen - 1] != '/')
   {
      folderSearch += '/';
   }

   dirent ** ppDevFiles;
   
   // Yes, scandir really takes a triple pointer for its second param
   int nNumDevFiles = scandir( folderSearch.c_str(), 
                               &ppDevFiles, 
                               IsFolder, 
                               NULL );
   for (int nFile = 0; nFile < nNumDevFiles; nFile++)
   {
      std::string newFolder = folderSearch + ppDevFiles[nFile]->d_name;
      free( ppDevFiles[nFile] );

      foundFolders.push_back( newFolder );
      EnumerateFolders( newFolder, foundFolders );
   }
   
   if (nNumDevFiles != -1)
   {
      free( ppDevFiles );
   }
}

/*===========================================================================
METHOD:
   IsHidden (Free Method)

DESCRIPTION:
   Helper function for DepthSearch, tells if a dirent is a hidden file.
   This reduces the memory usage by scandir, as compared to checking after
   scandir returns.

PARAMETERS:
   pFile         [ I ] - dirent structure describing file
  
RETURN VALUE:
   int:  zero    - Ignore this file
         nonzero - Process this file
===========================================================================*/
int IsHidden( const struct dirent * pFile )
{
   // Ignore anything beginning with a '.'
   if (pFile->d_name[0] == '.')
   {
      return 0;
   }

   return 1;
}

/*===========================================================================
METHOD:
   DepthSearch (Public Method)

DESCRIPTION:
   Search for all matching files at a specified depth (recursive)

PARAMETERS:
   baseFolder     [ I ] - Folder to search in
   depth          [ I ] - Depth 
   name           [ I ] - Partial name of file to search for
   foundFolders   [ O ] - Fully qualified paths of found files

RETURN VALUE:
   None
===========================================================================*/
void DepthSearch( 
   const std::string &           baseFolder,
   int                           depth,
   std::string                   name,
   std::vector <std::string> &   foundFiles )
{
   if (baseFolder.size() == 0 
   ||  name.size() == 0
   ||  depth < 0)
   {
      return;
   }

   std::string folderSearch = baseFolder;

   // Add trailing / if not present
   int folderLen = folderSearch.size();
   if (folderSearch[folderLen - 1] != '/')
   {
      folderSearch += '/';
   }

   dirent ** ppDevFiles;

   // Yes, scandir really takes a triple pointer for its second param
   int nNumDevFiles = scandir( folderSearch.c_str(), 
                               &ppDevFiles, 
                               IsHidden, 
                               NULL );
   for (int nFile = 0; nFile < nNumDevFiles; nFile++)
   {
      std::string newFile = ppDevFiles[nFile]->d_name;

      // Recurse or not?
      if (depth == 0)
      {
         if (newFile.find( name ) != std::string::npos)
         {
            foundFiles.push_back( folderSearch + newFile );
         }
      }
      else
      {
         DepthSearch( folderSearch + newFile,
                      depth - 1,
                      name,
                      foundFiles );
      }
   }

   if (nNumDevFiles != -1)
   {
      free( ppDevFiles );
   }
}

