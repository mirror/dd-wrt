/*===========================================================================
FILE: 
   GobiImageMgmt.h

DESCRIPTION:
   QUALCOMM Image Management API for Gobi 3000

PUBLIC CLASSES AND FUNCTIONS:
   cGobiImageMgmtDLL
   cGobiImageMgmt

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
#include "GobiQMICore.h"
#include "GobiQDLCore.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cGobiImageMgmt
/*=========================================================================*/
class cGobiImageMgmt : public cGobiQMICore
{
   public:
      // Constructor
      cGobiImageMgmt();

      // Destructor
      virtual ~cGobiImageMgmt();

      // Initialize the object
      virtual bool Initialize();

      // Cleanup the object
      virtual bool Cleanup();

      // Get the ID of the device to target
      void GetDeviceID(
         std::string &              deviceID,
         std::string &              deviceKey );

      // Set the ID of the device to target
      bool SetDeviceID(
         LPCSTR                     pDeviceID = 0,
         LPCSTR                     pDeviceKey = 0 );

      // This function resets the device
      eGobiError ResetDevice();

      // (Inline) Return the set of available Gobi QDL ports
      std::vector <std::string> GetAvailableQDLPorts()
      {
         return mQDL.GetAvailableQDLPorts();
      };

      // (Inline) Set the timeout for QDL transactions
      eGobiError SetQDLTimeout( ULONG to )
      {
         return mQDL.SetQDLTimeout( to );
      };

      // (Inline) Open the specified QDL port of the device
      eGobiError OpenQDLPort( 
         std::string &              portID,
         ULONG                      bBARMode,
         ULONG *                    pMajorVersion, 
         ULONG *                    pMinorVersion )
      {
         return mQDL.OpenQDLPort( portID,
                                  bBARMode,
                                  pMajorVersion,
                                  pMinorVersion );
      };

      // (Inline) Close the specified QDL port of the device
      eGobiError CloseQDLPort( bool bInformDevice )
      {
         return mQDL.CloseQDLPort( bInformDevice );
      };

      // (Inline) Get the images preference as from the device boot downloader
      eGobiError GetQDLImagesPreference( 
         ULONG *                    pImageListSize, 
         BYTE *                     pImageList )
      {
         return mQDL.GetQDLImagesPreference( pImageListSize, pImageList );
      };

      // (Inline) Prepare the device boot downloader for an image write
      eGobiError PrepareQDLImageWrite( 
         BYTE                       imageType,
         ULONG                      imageSize,
         ULONG *                    pBlockSize )
      {
         return mQDL.PrepareQDLImageWrite( imageType, imageSize, pBlockSize );
      };

      // (Inline) Write the specified image block to the device
      eGobiError WriteQDLImageBlock( 
         USHORT                     sequenceNumber,
         ULONG                      chunkSize,
         BYTE *                     pImageBlock )
      {
         return mQDL.WriteQDLImageBlock( sequenceNumber, 
                                         chunkSize, 
                                         pImageBlock );
      };
      
      // (Inline) Request the device validate the written images
      eGobiError ValidateQDLImages( BYTE * pImageType )
      {
         return mQDL.ValidateQDLImages( pImageType );
      };

   protected:
      /* Device node/key of the device to target */
      std::string mTargetDeviceNode;
      std::string mTargetDeviceKey;

      /* QDL protocol server */
      cGobiQDLCore mQDL;
};

/*=========================================================================*/
// Class cGobiImageMgmtDLL 
/*=========================================================================*/
class cGobiImageMgmtDLL
{
   public:
      // Constructor
      cGobiImageMgmtDLL();

      // Destructor
      virtual ~cGobiImageMgmtDLL();

      // Return the GobiImageMgmt object
      cGobiImageMgmt * GetAPI();

   protected:
      /* API interface object */
      cGobiImageMgmt * mpAPI;

      /* Above object allocation attempted? */
      bool mbAllocated;

      /* Synchronization object */
      mutable pthread_mutex_t mSyncSection;
};

extern cGobiImageMgmtDLL gImageDLL;
