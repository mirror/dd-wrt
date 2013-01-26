/*===========================================================================
FILE: 
   DB2TextFile.h

DESCRIPTION:
   Implementation of cDB2TextFile class

PUBLIC CLASSES AND METHODS:
   cDB2TextFile
      The cDB2TextFile class provides the simple ability to read in an
      ANSI/UNICODE file and copy it to a dynamically allocated buffer

      The sole difference between this and CStdioFile is that the issues
      stemming from supporting both ANSI and UNICODE files are handled
      in a simpler fashion at the expense of a bit of performance

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

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "DB2TextFile.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

// Maximum size of a file this interface will try to open (8 MB)
const DWORD MAX_FILE_SZ = 1024 * 1024 * 8;

// Maximum number of characters to run UNICODE check over
const INT MAX_UNICODE_CHECK_LEN = 128;

/*=========================================================================*/
// cDB2TextFile Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cDB2TextFile (Public Method)

DESCRIPTION:
   Construct object/load file into memory

PARAMETERS
   pFileName         [ I ] - File name

RETURN VALUE:
   None
===========================================================================*/
cDB2TextFile::cDB2TextFile( LPCSTR pFileName )
   :  mText( "" ),
      mStatus( ERROR_FILE_NOT_FOUND ),
      mCurrentPos( 0 )
{
   if (pFileName == 0 || pFileName[0] == 0)
   {
      return;
   }

   // Open the file
   std::ifstream tempFStream;
   tempFStream.open( pFileName, std::ios::in | std::ios::binary );
   if (tempFStream.fail() == true)
   {
      mStatus = ERROR_FILE_NOT_FOUND;
      return;
   }

   // Get size
   LONG nFileSize = tempFStream.rdbuf()->in_avail();
   if (nFileSize == -1 || nFileSize > MAX_FILE_SZ)
   {
      tempFStream.close();
      mStatus = ERROR_GEN_FAILURE;
      return;
   }

   // Read file into pTempBuffer
   CHAR * pTempBuffer = new char[ nFileSize ];
   if (pTempBuffer == NULL)
   {
      tempFStream.close();
      mStatus = ERROR_GEN_FAILURE;
      return;
   }

   tempFStream.read( pTempBuffer, nFileSize );
   if (tempFStream.fail() == true)
   {
      tempFStream.close();
      delete [] pTempBuffer;
      mStatus = ERROR_GEN_FAILURE;
      return;
   }

   tempFStream.close();
   
   // Convert to string
   mText = std::string( pTempBuffer, nFileSize );

   delete [] pTempBuffer;

   // Final check
   if (mText.size() != nFileSize)
   {
      mStatus = ERROR_GEN_FAILURE;      
      return;
   }

   // Success!
   mStatus = NO_ERROR;
}

/*===========================================================================
METHOD:
   cDB2TextFile (Public Method)

DESCRIPTION:
   Construct object/copy from buffer into memory

PARAMETERS
   pBuffer     [ I ] - Buffer to copy from
   bufferLen   [ I ] - Size of above buffer

RETURN VALUE:
   None
===========================================================================*/
cDB2TextFile::cDB2TextFile(
   LPCSTR                     pBuffer, 
   ULONG                      bufferLen )
   :  mText( "" ),
      mStatus( ERROR_FILE_NOT_FOUND ),
      mCurrentPos( 0 )
{
   // Convert to string
   mText = std::string( pBuffer, bufferLen );

   // Final check
   if (mText.size() != bufferLen)
   {
      mStatus = ERROR_GEN_FAILURE;      
      return;
   }

   // Success!
   mStatus = NO_ERROR;
}

/*===========================================================================
METHOD:
   ~cDB2TextFile (Public Method)

DESCRIPTION:
   Destructor

RETURN VALUE:
   None
===========================================================================*/
cDB2TextFile::~cDB2TextFile()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ReadLine (Public Method)

DESCRIPTION:
   Read the next available line

PARAMETERS
   line        [ O ] - Line (minus CR/LF)

RETURN VALUE:
   None
===========================================================================*/
bool cDB2TextFile::ReadLine( std::string & line )
{
   if (IsValid() == false)
   {
      return false;
   }

   int len = mText.size();
   if (mCurrentPos >= len)
   {
      return false;
   }

   int newIdx = mText.find( '\n', mCurrentPos );
   if (newIdx == -1)
   {
      // Possibly the end of the file
      newIdx = len;
   }

   if (newIdx < mCurrentPos)
   {
      return false;
   }

   if (newIdx == mCurrentPos)
   {
      // Return an empty line
      mCurrentPos++;    
      line = "";
   }
   else
   {
      // Grab line
      line = mText.substr( mCurrentPos, (newIdx - mCurrentPos) );

      // Check for CR
      int nCR = line.find( '\r' );
      if (nCR != -1)
      {
         line.erase( nCR, 1 );
      }

      mCurrentPos = newIdx + 1;
   }

   return true;
}

