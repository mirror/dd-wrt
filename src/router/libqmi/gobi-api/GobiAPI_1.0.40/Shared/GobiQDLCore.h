/*===========================================================================
FILE: 
   GobiQDLCore.h

DESCRIPTION:
   QUALCOMM Gobi QDL Based API Core

PUBLIC CLASSES AND FUNCTIONS:
   cGobiQDLCore

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

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "ProtocolBuffer.h"
#include "QDLProtocolServer.h"
#include "GobiError.h"
#include "GobiMBNMgmt.h"

#include <map>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cGobiQDLCore
/*=========================================================================*/
class cGobiQDLCore
{
   public:
      // Constructor
      cGobiQDLCore();

      // Destructor
      virtual ~cGobiQDLCore();

      // Initialize the object
      virtual bool Initialize();

      // Cleanup the object
      virtual bool Cleanup();

      // Return the set of available Gobi QDL ports
      std::vector <std::string> GetAvailableQDLPorts();

      // Set the timeout for QDL transactions
      eGobiError SetQDLTimeout( ULONG to );

      // Open the specified QDL port of the device
      eGobiError OpenQDLPort( 
         std::string &              portID,
         ULONG                      bBARMode,
         ULONG *                    pMajorVersion, 
         ULONG *                    pMinorVersion );

      // Close the specified QDL port of the device
      eGobiError CloseQDLPort( bool bInformDevice );

      // Get the images preference as from the device boot downloader
      eGobiError GetQDLImagesPreference( 
         ULONG *                    pImageListSize, 
         BYTE *                     pImageList );

      // Prepare the device boot downloader for an image write
      eGobiError PrepareQDLImageWrite( 
         BYTE                       imageType,
         ULONG                      imageSize,
         ULONG *                    pBlockSize );

      // Write the specified image block to the device
      eGobiError WriteQDLImageBlock( 
         USHORT                     sequenceNumber,
         ULONG                      chunkSize,
         BYTE *                     pImageBlock );
      
      // Request the device validate the written images
      eGobiError ValidateQDLImages( BYTE * pImageType );

      // Send a QDL request and wait for and return response
      sProtocolBuffer SendQDL(
         sSharedBuffer *            pRequest,
         const BYTE *               pAuxData = 0,
         ULONG                      auxDataSz = 0,
         bool                       bWAitForResponse = true );

      // Get currently connected port ID
      bool GetConnectedPortID( std::string & portNode );

      // (Inline) Clear last error recorded
      void ClearLastError()
      {
         mLastError = eGOBI_ERR_NONE;
      };

      // (Inline) Get last error recorded
      eGobiError GetLastError()
      {
         return mLastError;
      };

      // (Inline) Return the last recorded error (if this happens to indicate 
      // that no error occurred then return eGOBI_ERR_INTERNAL)
      eGobiError GetCorrectedLastError()
      {
         eGobiError ec = GetLastError();
         if (ec == eGOBI_ERR_NONE)
         {
            ec = eGOBI_ERR_INTERNAL;
         }

         return ec;
      };

      // (Inline) Return the correct QDL error
      eGobiError GetCorrectedQDLError( ULONG qdlError )
      {
         ULONG ec = qdlError + (ULONG)eGOBI_ERR_QDL_OFFSET;
         return (eGobiError)ec;
      };


   protected:
      /* QDL protocol server */
      cQDLProtocolServer mQDL;

      /* ID of QDL port device node is connected to */
      std::string mQDLPortNode;

      /* Timeout for QDL transactions (in milliseconds) */
      ULONG mQDLTimeout;

      /* Last error recorded */
      eGobiError mLastError;
};

