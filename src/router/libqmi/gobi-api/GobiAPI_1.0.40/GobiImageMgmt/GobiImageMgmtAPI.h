/*===========================================================================
FILE: 
   GobiImageMgmtAPI.h

DESCRIPTION:
   QUALCOMM Image Management API for Gobi 3000

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

#ifndef GOBI_TYPEDEFS
#define GOBI_TYPEDEFS

// Type Definitions
typedef unsigned long      ULONG;
typedef unsigned long long ULONGLONG;
typedef signed char        INT8;
typedef unsigned char      BYTE;
typedef char               CHAR;
typedef unsigned short     WORD;
typedef unsigned short     USHORT;
typedef const char *       LPCSTR;

#endif

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

/*=========================================================================*/
// Definitions
/*=========================================================================*/

#ifdef __cplusplus
   extern "C" {
#endif

/*=========================================================================*/
// Prototypes
/*=========================================================================*/

/*===========================================================================
METHOD:
   GobiEnumerateDevices

DESCRIPTION:
   This function enumerates the Gobi devices currently attached to the
   system

PARAMETERS:
   pDevicesSize   [I/O] - Upon input the maximum number of elements that the 
                          device array can contain.  Upon successful output 
                          the actual number of elements in the device array
   pDevices       [ O ] - The device array 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GobiEnumerateDevices( 
   BYTE *                     pDevicesSize, 
   BYTE *                     pDevices );

/*===========================================================================
METHOD:
   SetDeviceID

DESCRIPTION:
   This function sets the ID of the device to target

PARAMETERS:
   pDeviceID   [ I ] - The device ID as reported by Windows
   pDeviceKey  [ I ] - The device key (unique, stored on-device)
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetDeviceID(
   CHAR *                     pDeviceID,
   CHAR *                     pDeviceKey );

/*===========================================================================
METHOD:
   GetImagesPreference

DESCRIPTION:
   This function gets the current images preference

PARAMETERS:
   pImageListSize [I/O] - Upon input the size in BYTEs of the image list 
                          array.  Upon success the actual number of BYTEs 
                          copied to the image list array
   pImageList     [ O ] - The image info list array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetImagesPreference( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList );

/*===========================================================================
METHOD:
   SetImagesPreference

DESCRIPTION:
   This function sets the current images preference

PARAMETERS:
   imageListSize     [ I ] - The size in BYTEs of the image list array
   pImageList        [ I ] - The image list array
   bForceDownload    [ I ] - Force device to download images from host?
   modemIndex        [ I ] - Desired storage index for downloaded modem image
   pImageTypesSize   [I/O] - Upon input the maximum number of elements that 
                             the download image types array can contain.  
                             Upon successful output the actual number of 
                             elements in the download image types array 
   pImageTypes       [ O ] - The download image types array 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetImagesPreference( 
   ULONG                      imageListSize, 
   BYTE *                     pImageList,
   ULONG                      bForceDownload,
   BYTE                       modemIndex,
   ULONG *                    pImageTypesSize, 
   BYTE *                     pImageTypes );

/*===========================================================================
METHOD:
   GetBARMode

DESCRIPTION:
   This function returns the boot and recovery image download mode

PARAMETERS:
   pBARMode    [ O ] - Boot and recovery image download mode

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetBARMode( ULONG * pBARMode );

/*===========================================================================
METHOD:
   SetBARMode

DESCRIPTION:
   This function requests the device enter boot and recovery image download
   mode after the next reset

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetBARMode();

/*===========================================================================
METHOD:
   GetStoredImages

DESCRIPTION:
   This function gets the list of images stored on the device

PARAMETERS:
   pImageListSize [I/O] - Upon input the size in BYTEs of the image list 
                          array.  Upon success the actual number of BYTEs 
                          copied to the image list array
   pImageList     [ O ] - The image info list array 
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetStoredImages( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList );

/*===========================================================================
METHOD:
   GetStoredImageInfo

DESCRIPTION:
   This function returns info about the specified image from the device

PARAMETERS:
   imageInfoSize  [ I ] - The size in BYTEs of the image info array
   pImageInfo     [ I ] - The image info array
   pMajorVersion  [ O ] - Major version of compatible boot downloader
   pMinorVersion  [ O ] - Minor version of compatible boot downloader
   pVersionID     [ O ] - Image version ID
   pInfo          [ O ] - Image info string
   pLockID        [ O ] - Image OEM lock ID

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetStoredImageInfo( 
   ULONG                      imageInfoSize, 
   BYTE *                     pImageInfo,
   ULONG *                    pMajorVersion, 
   ULONG *                    pMinorVersion,
   ULONG *                    pVersionID,
   CHAR *                     pInfo,
   ULONG *                    pLockID );

/*===========================================================================
METHOD:
   DeleteStoredImage

DESCRIPTION:
   This function deletes the specified image from the device

PARAMETERS:
   imageInfoSize  [ I ] - The size in BYTEs of the image info array
   pImageInfo     [ I ] - The image info array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DeleteStoredImage( 
   ULONG                      imageInfoSize, 
   BYTE *                     pImageInfo );

/*===========================================================================
METHOD:
   ResetDevice

DESCRIPTION:
   This function resets the device

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ResetDevice();

/*===========================================================================
METHOD:
   GobiEnumerateQDLPorts

DESCRIPTION:
   This function enumerates the Gobi QDL port IDs currently attached to the
   system

PARAMETERS:
   pPortSize   [I/O] - Upon input the maximum number of elements that the 
                       port ID array can contain. Upon successful output 
                       the actual number of elements in the port ID array
   pPorts      [ O ] - Port ID array

RETURN VALUE:
   ULONG - Return code
===========================================================================*/ 
ULONG GobiEnumerateQDLPorts(
   BYTE *                     pPortSize,
   BYTE *                     pPorts );

/*===========================================================================
METHOD:
   SetQDLTimeout

DESCRIPTION:
   This function sets the timeout for all subsequent QDL transactions

PARAMETERS:
   to          [ O ] - Timeout (in milliseconds) for subsequent transactions

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG SetQDLTimeout( ULONG to );

/*===========================================================================
METHOD:
   OpenQDLPort

DESCRIPTION:
   This function opens the specified QDL port of the device

PARAMETERS:
   pPortID        [ I ] - ID of QDL port to connect to 
   bBARMode       [ I ] - Request boot and recovery mode feature
   pMajorVersion  [ O ] - Major version of the device boot downloader
   pMinorVersion  [ O ] - Minor version of the device boot downloader

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OpenQDLPort( 
   CHAR *                     pPortID,
   ULONG                      bBARMode,
   ULONG *                    pMajorVersion, 
   ULONG *                    pMinorVersion );

/*===========================================================================
METHOD:
   CloseQDLPort

DESCRIPTION:
   This function closes the currently open QDL port of the device

PARAMETERS:
   bInformDevice  [ I ] - Inform device that QDL port is being closed? 

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CloseQDLPort( ULONG bInformDevice );

/*===========================================================================
METHOD:
   GetQDLImagesPreference

DESCRIPTION:
   This function gets the current images preference as reported by the 
   device boot downloader

PARAMETERS:
   pImageListSize [I/O] - Upon input the maximum number of elements that the 
                          image info list can contain.  Upon successful output 
                          the actual number of elements in the image info list
   pImageList     [ O ] - The image info list
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GetQDLImagesPreference( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList );

/*===========================================================================
METHOD:
   PrepareQDLImageWrite

DESCRIPTION:
   This function prepares the device boot downloader for an image write

PARAMETERS:
   imageType   [ I ] - Type of image being written 
   imageSize   [ I ] - Size of image being written
   pBlockSize  [I/O] - Upon input the maximum size of image block supported 
                       by host, upon successful output the maximum size of 
                       image block supported by device

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PrepareQDLImageWrite( 
   BYTE                       imageType,
   ULONG                      imageSize,
   ULONG *                    pBlockSize );

/*===========================================================================
METHOD:
   WriteQDLImageBlock

DESCRIPTION:
   This function writes the specified image block to the device

PARAMETERS:
   sequenceNumber [ I ] - Sequence number for image write 
   blockSize      [ I ] - Size of image block
   pImageBlock    [ I ] - Image block
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WriteQDLImageBlock( 
   USHORT                     sequenceNumber,
   ULONG                      blockSize,
   BYTE *                     pImageBlock );

/*===========================================================================
METHOD:
   ValidateQDLImages

DESCRIPTION:
   This function requests the device validate the written images

PARAMETERS:
   pImageType  [ O ] - Upon failure this may contain the type of the image
                       that failed validation
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG ValidateQDLImages( BYTE * pImageType );

#ifdef __cplusplus
   };
#endif
