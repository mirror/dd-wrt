/*===========================================================================
FILE: 
   CoreUtilities.h

DESCRIPTION:
   Collection of various utility methods

PUBLIC CLASSES AND METHODS:
   StringToLONG
   StringToULONG
   StringToLONGLONG
   StringToULONGLONG

   ParseCommandLine()
   ParseTokens()
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

   GetProgramPath()
   EnumerateFolders()
   DepthSearch()

   ContainerToCSVString()
   CSVStringToContainer()
   CSVStringToValidatedContainer()

   SetDiff()
   SetIntersection()
   SetUnion()

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
#include <vector>
#include <set>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Suggested size of an argument buffer to ToString() for any key, each
// ToString() should not write more than this size to the passed in buffer
const ULONG SUGGESTED_BUFFER_LEN = 64;

/*=========================================================================*/
// Prototypes
/*=========================================================================*/

// Replacement/front end for _tcstol
bool StringToLONG( 
   LPCSTR                    pStr,
   int                        base,
   LONG &                     val );

// Replacement/front end for _tcstoul
bool StringToULONG( 
   LPCSTR                    pStr,
   int                        base,
   ULONG &                    val );

// Replacement/front end for _tcstoi64
bool StringToLONGLONG( 
   LPCSTR                    pStr,
   int                        base,
   LONGLONG &                 val );

// Replacement/front end for _tcstoui64
bool StringToULONGLONG( 
   LPCSTR                    pStr,
   int                        base,
   ULONGLONG &                val );

// Parse a command line to tokens (a command line is a string of
// space delimited values where a value that contains space is
// enclosed in text)
void ParseCommandLine(
   std::string                commandLine,
   std::vector <std::string> &    tokens );

// Parse a line into individual tokens
void ParseTokens(
   LPCSTR                    pSeparator,
   LPSTR                     pLine,
   std::vector <LPSTR> &     tokens );

// Parse a format specifier into individual format type tokens
bool ParseFormatSpecifier( 
   LPCSTR                    pFmt,
   ULONG                      fmtLen,
   std::vector <CHAR> &      fmtTypes );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   CHAR &                     theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   UCHAR &                    theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   SHORT &                    theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   USHORT &                   theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   int &                      theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   UINT &                     theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   LONG &                     theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   ULONG &                    theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   LONGLONG &                 theType );

// Convert a string to a value
bool FromString( 
   LPCSTR                    pStr,
   ULONGLONG &                theType );


// Convert a value to a string
bool ToString( 
   CHAR                       val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   UCHAR                      val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   SHORT                      val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   USHORT                     val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   int                        val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   UINT                       val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   LONG                       val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   ULONG                      val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   LONGLONG                   val,
   LPSTR                      pStr );

// Convert a value to a string
bool ToString( 
   ULONGLONG                  val,
   LPSTR                      pStr );

// Return the special folder used for storing program files
std::string GetProgramPath();

// Enumerate the subfolders of the given folder (recursive)
void EnumerateFolders( 
   const std::string &           baseFolder,
   std::vector <std::string> &   foundFolders );

// Search for a file at a given depth
void DepthSearch( 
   const std::string &           baseFolder,
   int                           depth,
   std::string                   name,
   std::vector <std::string> &   foundFiles );

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   ContainerToCSVString (Free Method)

DESCRIPTION:
   Convert the contents of a container to a CSV string

   NOTE: ToString() must be defined for the container value type
  
PARAMETERS:   
   cont        [ I ] - The container
   sep         [ I ] - The character separating tokens
   csv         [ O ] - The resulting comma separated string

RETURN VALUE:
   None
===========================================================================*/
template <class Container>
void ContainerToCSVString( 
   const Container &          cont,
   CHAR                       sep,
   std::string &              csv )
{
   csv = "";
   if ((ULONG)cont.size() > (ULONG)0)
   {
      CHAR keyBuf[SUGGESTED_BUFFER_LEN];

      typename Container::const_iterator pIter = cont.begin();
      while (pIter != cont.end())
      {
         const typename Container::value_type & theKey = *pIter;
         bool bOK = ToString( theKey, &keyBuf[0] );

         if (bOK == true && keyBuf[0] != 0)
         {
            if (pIter != cont.begin())
            {
               csv += sep;
            }

            csv += (LPCSTR)&keyBuf[0];
         }

         pIter++;
      }
   }
}

/*===========================================================================
METHOD:
   CSVStringToContainer (Free Method)

DESCRIPTION:
   Populate a container from a parsed CSV string

   NOTE: FromString() must be defined for the container value type 
   NOTE: The container is emptied before this operation is attempted

PARAMETERS:   
   pSeparator  [ I ] - The characters separating tokens
   pCSV        [ I ] - The comma separated string (will be modified)
   cont        [ O ] - The resulting container
   bClear      [ I ] - Clear the container first?  NOTE: if the container
                       is not cleared first then insertions may fail for
                       duplicate keys

RETURN VALUE:
   None
===========================================================================*/
template <class Container>
void CSVStringToContainer( 
   LPCSTR                     pSeparator, 
   LPSTR                      pCSV,
   Container &                cont,
   bool                       bClear = true )
{
   if (pCSV != 0 && *pCSV != 0)
   {
      // Remove a leading quote?
      if (*pCSV == '\"')
      {
         pCSV++;
      }

      // Remove a trailing quote?
      ULONG len = (ULONG)strlen( pCSV );
      if (len > 0)
      {
         if (pCSV[len - 1] == '\"')
         {
            pCSV[len - 1] = 0;
         }
      }

      // Clear the container first?
      if (bClear == true)
      {
         cont.clear();
      }

      std::vector <LPSTR> tokens;
      ParseTokens( pSeparator, pCSV, tokens );

      std::vector <LPSTR>::const_iterator pIter = tokens.begin();
      while (pIter != tokens.end())
      {
         LPCSTR pTok  = *pIter;
  
         typename Container::value_type theKey;
         bool bOK = ::FromString( pTok, theKey );

         if (bOK == true)
         {      
            std::insert_iterator <Container> is( cont, cont.end() );
            *is = theKey;
         }
       
         pIter++;
      }
   }
}

/*===========================================================================
METHOD:
   CSVStringToValidatedContainer (Free Method)

DESCRIPTION:
   Populate a container from a parsed CSV string

   NOTE: FromString() and IsValid() must be defined for the container 
         value type (the later need not do anything but return true)

   NOTE: The container is emptied before this operation is attempted
  
PARAMETERS:   
   pSeparator  [ I ] - The characters separating tokens
   pCSV        [ I ] - The comma separated string (will be modified)
   cont        [ O ] - The resulting container

RETURN VALUE:
   None
===========================================================================*/
template <class Container>
void CSVStringToValidatedContainer( 
   LPCSTR                     pSeparator, 
   LPSTR                      pCSV,
   Container &                cont )
{
   cont.clear();
   if (pCSV != 0 && *pCSV != 0)
   {
      // Remove a leading quote?
      if (*pCSV == '\"')
      {
         pCSV++;
      }

      // Remove a trailing quote?
      ULONG len = (ULONG)strlen( pCSV );
      if (len > 0)
      {
         if (pCSV[len - 1] == '\"')
         {
            pCSV[len - 1] = 0;
         }
      }

      cont.clear();

      std::vector <LPSTR> tokens;
      ParseTokens( pSeparator, pCSV, tokens );

      std::vector <LPSTR>::const_iterator pIter = tokens.begin();
      while (pIter != tokens.end())
      {
         LPCSTR pTok  = *pIter;

         typename Container::value_type theKey;
         bool bOK = ::FromString( pTok, theKey );

         if (bOK == true)
         {
            bool bValid = IsValid( theKey );
            if (bValid == true)
            {
               std::insert_iterator <Container> is( cont, cont.end() );
               *is = theKey;
            }
         }
      
         pIter++;
      }
   }
}

/*===========================================================================
METHOD:
   SetDiff (Free Method)

DESCRIPTION:
   Given two sets return a third that contains everything in the first
   set but not the second set

PARAMETERS:
   setA        [ I ] - The first set
   setB        [ I ] - The second set
  
RETURN VALUE:
   std::set <T> - the difference
===========================================================================*/
template <class T>
std::set <T> SetDiff(
   const std::set <T> &       setA,
   const std::set <T> &       setB )
{
   std::set <T> retSet;

   if (setB.size() == 0)
   {
      // Everything that is in the first set but not the second
      // (empty) set is ... the first set!
      retSet = setA;
   }
   else if (setA.size() == 0)
   {
      // The first set is empty, hence the return set is empty
   }
   else
   {
      // Both sets have elements, therefore the iterators will
      // be valid and we can use the standard approach
      typename std::set <T>::const_iterator pIterA = setA.begin();
      typename std::set <T>::const_iterator pIterB = setB.begin();

      for ( ; pIterA != setA.end() && pIterB != setB.end(); )
      {
         if (*pIterA < *pIterB)
         {
            retSet.insert( *pIterA );
            pIterA++;
         }
         else if (*pIterB < *pIterA)
         {
            pIterB++;
         }
         else
         {
            pIterA++;
            pIterB++;
         }
      }

      while (pIterA != setA.end())
      {
         retSet.insert( *pIterA );
         pIterA++;
      }
   }

   return retSet;
}

/*===========================================================================
METHOD:
   SetIntersection (Free Method)

DESCRIPTION:
   Given two sets return a third that contains everything that is in both
   sets

PARAMETERS:
   setA        [ I ] - The first set
   setB        [ I ] - The second set
  
RETURN VALUE:
   std::set <T> - the union
===========================================================================*/
template <class T>
std::set <T> SetIntersection(
   const std::set <T> &       setA,
   const std::set <T> &       setB )
{
   std::set <T> retSet;

   // Neither set can be empty
   if (setA.size() != 0 && setA.size() != 0)
   {
      // Both sets have elements, therefore the iterators will
      // be valid and we can use the standard approach
      typename std::set <T>::const_iterator pIterA = setA.begin();
      typename std::set <T>::const_iterator pIterB = setB.begin();

      for ( ; pIterA != setA.end() && pIterB != setB.end(); )
      {
         if (*pIterA < *pIterB)
         {
            pIterA++;
         }
         else if (*pIterB < *pIterA)
         {
            pIterB++;
         }
         else
         {
            retSet.insert( *pIterA );
            pIterA++;
            pIterB++;
         }
      }
   }

   return retSet;
}

/*===========================================================================
METHOD:
   SetUnion (Free Method)

DESCRIPTION:
   Given two sets return a third that contains everything that is either
   in the first set or in the second set

PARAMETERS:
   setA        [ I ] - The first set
   setB        [ I ] - The second set
  
RETURN VALUE:
   std::set <T> - the union
===========================================================================*/
template <class T>
std::set <T> SetUnion(
   const std::set <T> &       setA,
   const std::set <T> &       setB )
{
   std::set <T> retSet;

   if (setB.size() == 0)
   {
      // Everything that is in the first (possibly empty) set or in 
      // the second (empty) set is ... the first set!
      retSet = setA;
   }
   else if (setA.size() == 0)
   {
      // Everything that is in the first (empty) set or in the 
      // second (possibly empty) set is ... the second set!
      retSet = setB;
   }
   else
   {
      // Both sets have elements, therefore the iterators will
      // be valid and we can use the standard approach
      typename std::set <T>::const_iterator pIterA = setA.begin();
      typename std::set <T>::const_iterator pIterB = setB.begin();

      for ( ; pIterA != setA.end() && pIterB != setB.end(); )
      {
         if (*pIterA < *pIterB)
         {
            retSet.insert( *pIterA );
            pIterA++;
         }
         else if (*pIterB < *pIterA)
         {
            retSet.insert( *pIterB );
            pIterB++;
         }
         else
         {
            retSet.insert( *pIterA );
            pIterA++;
            pIterB++;
         }
      }

      while (pIterA != setA.end())
      {
         retSet.insert( *pIterA );
         pIterA++;
      }

      while (pIterB != setB.end())
      {
         retSet.insert( *pIterB );
         pIterB++;
      }
   }

   return retSet;
}

