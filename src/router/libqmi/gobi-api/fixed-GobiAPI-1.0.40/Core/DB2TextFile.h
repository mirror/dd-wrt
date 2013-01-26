/*===========================================================================
FILE: 
   DB2TextFile.h

DESCRIPTION:
   Declaration of cDB2TextFile class

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
#include "StdAfx.h"

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// Class cDB2TextFile
/*=========================================================================*/
class cDB2TextFile
{
   public:
      // Constructor (loads file)
      cDB2TextFile( LPCSTR pMemFile );

      // Constructor (loads file from resource table)
      cDB2TextFile( 
         HMODULE                    hModule, 
         LPCSTR                     pRscTable,
         DWORD                      rscID );

      // Constructor (loads file from buffer)
      cDB2TextFile( 
         LPCSTR                     pBuffer,
         ULONG                      bufferLen );

      // Destructor
      ~cDB2TextFile();

      // (Inline) Get error status
      DWORD GetStatus()
      {
         return mStatus;
      };
      
      // (Inline) Get the file contents
      bool GetText( std::string & copy )
      {
         bool bOK = IsValid();
         if (bOK == true)
         {
            copy = mText;
         }
         
         return bOK;
      };

      // (Inline) Get file validity
      virtual bool IsValid()
      {
         return (mStatus == NO_ERROR);
      };

      // Read the next available line
      bool ReadLine( std::string & line );

   protected:
      /* File contents */
      std::string mText;

      /* Current position (in above contents) */
      int mCurrentPos;

      /* Error status */
      DWORD mStatus;
};
