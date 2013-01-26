/*===========================================================================
FILE: 
   Main.cpp

DESCRIPTION:
   Firmware downloader using cGobiQDLCore class

PUBLIC CLASSES AND FUNCTIONS:
   Run
   main

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
==========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "GobiQDLCore.h"
#include "QDLBuffers.h"
#include "MemoryMappedFile.h"

#include <syslog.h>

//---------------------------------------------------------------------------
// Free Methods
//---------------------------------------------------------------------------

/*===========================================================================
METHOD:
   Run (Public Method)

DESCRIPTION:
   Simple QDL download

RETURN VALUE:
   bool
===========================================================================*/
bool Run()
{
   cGobiQDLCore qdl;
   bool bRC = qdl.Initialize();
   if (bRC == false)
   {
      syslog( LOG_INFO, "Failed to initialize QDL core" );
      return bRC;
   }

   qdl.SetQDLTimeout( 10000 );

   std::vector <std::string> qdlPorts = qdl.GetAvailableQDLPorts();
   if (qdlPorts.size() == 0)
   {
      syslog( LOG_INFO, "No QDL devices found" );
      return false;
   }

   std::string portName = qdlPorts[0];
   syslog( LOG_INFO, "Download started to port %s", portName.c_str() );

   // Connect to port
   ULONG maj = ULONG_MAX;
   ULONG min = ULONG_MAX;
   eGobiError err = qdl.OpenQDLPort( portName, 0, &maj, &min );
   if (err != eGOBI_ERR_NONE)
   {
      syslog( LOG_INFO, "OpenQDLPort( %s ) = %d", portName.c_str(), err );
      return false;
   }

   ULONG bufSz = 12;
   sQDLRawImageID buf[12];
   err = qdl.GetQDLImagesPreference( &bufSz, (BYTE *)&buf[0] );
   if (err != eGOBI_ERR_NONE)
   {
      syslog( LOG_INFO, "GetQDLImagesPreference() = %d", err );
      qdl.CloseQDLPort( false );
      return false;
   }

   if (bufSz > 12)
   {
      syslog( LOG_INFO, "GetQDLImagesPreference(), bufSz = %lu", bufSz );
      qdl.CloseQDLPort( false );
      return false;
   }

   bool bErr = false;
   for (ULONG i = 0; i < bufSz; i++)
   {
      std::string img = ::GetImageByUniqueID( &buf[i].mImageID[0] );
      if (img.size() <= 0)
      {
         // Skip files we do not have access to
         syslog( LOG_INFO, "GetImageByUniqueID() failure" );
         return false;
      }

      ULONG fileMaj = 0;
      ULONG fileMin = 0;
      err = ::GetImageBootCompatibility( img.c_str(), 
                                         &fileMaj, 
                                         &fileMin );

      if (err != eGOBI_ERR_NONE || fileMaj != maj)
      {
         // Skip files that may not be compatible
         syslog( LOG_INFO, "GetImageBootCompatibility() failure [%d]", err );
         return false;
      }

      cMemoryMappedFile imgFile( img.c_str() );
      syslog( LOG_INFO, "Downloading %s", img.c_str() );

      LPVOID pImgData = imgFile.GetContents();
      ULONG imgSz = imgFile.GetSize();
      if (pImgData == 0 || imgSz == 0)
      {
         syslog( LOG_INFO, "Image file failure [%s]", img.c_str() );
         bErr = true;
         return false;
      }

      ULONG blockSz = QDL_MAX_CHUNK_SIZE;
      err = qdl.PrepareQDLImageWrite( buf[i].mImageType, imgSz, &blockSz );
      if (err != eGOBI_ERR_NONE)
      {
         if (err == eGOBI_ERR_QDL_OPEN_SKIP)
         {
            // Device already has this file
            continue;
         }
         else
         {
            syslog( LOG_INFO, "PrepareQDLImageWrite() = %d", err );
            bErr = true;
            break;
         }
      }

      err = qdl.WriteQDLImageBlock( 0, imgSz, (BYTE *)pImgData );
      if (err != eGOBI_ERR_NONE)
      {
         syslog( LOG_INFO, "WriteQDLImageBlock() = %d", err );
         bErr = true;
         break;
      }
   }

   if (bErr == false)
   {
      syslog( LOG_INFO, "Download completed" );
      BYTE errImg;
      qdl.ValidateQDLImages( &errImg );            
   }

   qdl.CloseQDLPort( true );
}

/*===========================================================================
METHOD:
   main (Public Method)

DESCRIPTION:
   Application entry point

RETURN VALUE:
   int - Process exit code
===========================================================================*/
int main()
{
   // Add PID to log statements
   openlog( "GobiQDLService", LOG_PID, LOG_USER );

   bool bSuccess = Run();

   closelog();

   return (bSuccess ? 0 : -1 );
}
