/*===========================================================================
FILE: 
   MemoryMappedFile.h

DESCRIPTION:
   Declaration of cMemoryMappedFile class

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

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// Class cMemoryMappedFile
/*=========================================================================*/
class cMemoryMappedFile
{
   public:
      // Constructor (loads file)
      cMemoryMappedFile( LPCSTR pFile );

      // Constructor (loads file from resource)
      cMemoryMappedFile( 
         const char *               pStart,
         const int                  nSize );

      // Destructor
      virtual ~cMemoryMappedFile();

      // (Inline) Get error status
      DWORD GetStatus()
      {
         DWORD stat = mStatus;
         if (mStatus == NO_ERROR)
         {
            if (mpBuffer == 0 || mFileSize == 0)
            {
               // We failed but GetLastError() return NO_ERROR
               stat = ERROR_NO_MORE_ITEMS;
            }
         }

         return stat;
      };

      // (Inline) Return the size of the file (contents)
      ULONG GetSize()
      {
         ULONG sz = 0;
         if (GetStatus() == NO_ERROR)
         {
            sz = mFileSize;
         }

         return sz;
      };

      // (Inline) Return the file contents
      LPVOID GetContents()
      {
         LPVOID pContents = 0;
         if (GetStatus() == NO_ERROR)
         {
            pContents = mpBuffer;
         }

         return pContents;
      };

   protected:
      /* Resource based file? */
      bool mbResourceBased;

      /* File handle */
      int mFile;

      /* File contents*/
      LPVOID mpBuffer;

      /* File size */
      ULONG mFileSize;

      /* Error status */
      DWORD mStatus;
};
