/*===========================================================================
FILE: 
   MemoryMappedFile.cpp

DESCRIPTION:
   Implementation of cMemoryMappedFile class

PUBLIC CLASSES AND METHODS:
   cMemoryMappedFile
      The cMemoryMappedFile class provides the ability to read in a file 
      via Linux memory maps

      Note: cMemoryMappedFiles are read only

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
#include "MemoryMappedFile.h"

#include <sys/mman.h>

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

// Maximum size of a file this interface will try to open (64 MB)
const DWORD MAX_FILE_SZ = 1024 * 1024 * 64;

/*=========================================================================*/
// cMemoryMappedFile Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cMemoryMappedFile (Public Method)

DESCRIPTION:
   Construct object/load file into memory

PARAMETERS
   pFile       [ I ] - File name

RETURN VALUE:
   None
===========================================================================*/
cMemoryMappedFile::cMemoryMappedFile( LPCSTR pFile )
   :  mbResourceBased( false ),
      mpBuffer( 0 ),
      mFileSize( 0 ),
      mStatus( ERROR_FILE_NOT_FOUND )
{
   if (pFile == 0 || pFile[0] == 0)
   {
      return;
   }

   // Open the file
   mFile = open( pFile, O_RDONLY );
   if (mFile == -1)
   {
      TRACE( "Unable to Map %s to memory.  Error %d: %s\n",
             pFile,
             errno,
             strerror( errno ) );
      return;
   }

   // Grab the file size
   struct stat fileInfo;
   if (fstat( mFile, &fileInfo ) != 0)
   {
      TRACE( "Unable to get info for %s.  Error %d: %s\n",
             pFile,
             errno,
             strerror( errno ) );
      return;
   }
   DWORD fileSize = fileInfo.st_size;

   // Map to mpBuffer
   mpBuffer = mmap( 0, 
                    fileSize, 
                    PROT_READ, 
                    MAP_SHARED | MAP_POPULATE, 
                    mFile, 
                    0 );
   if (mpBuffer == 0 || mpBuffer == MAP_FAILED )
   {
      TRACE( "Memory map failed error %d:, %s\n",
             errno,
             strerror( errno ) );
      return;
   }

   // Success!
   mFileSize = fileSize;
   mStatus = NO_ERROR;
}

/*===========================================================================
METHOD:
   cMemoryMappedFile (Public Method)

DESCRIPTION:
   Construct object/load resource based file into memory

PARAMETERS
   pStart         [ I ] - Start location of object
   nSize          [ I ] - Size of object

RETURN VALUE:
   None
===========================================================================*/
cMemoryMappedFile::cMemoryMappedFile( 
   const char *               pStart,
   const int                  nSize )
   :  mbResourceBased( true ),
      mpBuffer( 0 ),
      mFileSize( 0 ),
      mStatus( INVALID_HANDLE_VALUE )
{
   // Set size
   mFileSize = nSize;

   // Set memory pointer
   mpBuffer = (void * )pStart;   
   
   // Success!
   mStatus = NO_ERROR;
}

/*===========================================================================
METHOD:
   ~cMemoryMappedFile (Public Method)

DESCRIPTION:
   Destructor

RETURN VALUE:
   None
===========================================================================*/
cMemoryMappedFile::~cMemoryMappedFile()
{
   if (mbResourceBased == false)
   {
      if (munmap( mpBuffer, mFileSize ) == -1)
      {
         TRACE( "Memory unmapping error %d: %s\n",
                errno,
                strerror( errno ) );
         return;
      }
   }
}
