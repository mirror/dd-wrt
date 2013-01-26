/*===========================================================================
FILE:
   QDLBuffers.h

DESCRIPTION:
   QDL protocol related structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sQDLRawHelloReq
   sQDLRawHelloRsp
   sQDLRawErrorRsp
   sQDLRawOpenUnframedReq
   sQDLRawOpenUnframedRsp
   sQDLRawWriteUnframedReq
   sQDLRawWriteUnframedRsp
   sQDLRawDoneRsp
   sQDLRawGetImagePrefRsp
   sQDLRawImageID

   sQDLHello
   sQDLError
   sQDLOpenUnframed
   sQDLWriteUnframed
   sQDLDone
   sQDLGetImagePref

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
#include "ProtocolBuffer.h"
#include "QDLEnum.h"

#include <list>

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sQDLRawHelloReq
//    Struct to represent a QDL hello request (raw)
/*=========================================================================*/
struct sQDLRawHelloReq
{
   public:
      BYTE mCommandCode;
      BYTE mMagicNumber[32];
      BYTE mMaxVersion;
      BYTE mMinVersion;
      BYTE mFeatures;
};

/*=========================================================================*/
// Struct sQDLRawHelloRsp
//    Struct to represent a QDL hello response (raw)
/*=========================================================================*/
struct sQDLRawHelloRsp
{
   public:
      BYTE mCommandCode;
      BYTE mMagicNumber[24];
      DWORD mReserved1;
      WORD mBootMajorVersion;
      WORD mBootMinorVersion;
      BYTE mMaxVersion;
      BYTE mMinVersion;
      DWORD mReserved2;
      DWORD mReserved3;
      BYTE mReserved4;
      WORD mReserved5;
      WORD mReserved6;
      BYTE mFeatures;
};

/*=========================================================================*/
// Struct sQDLRawErrorRsp
//    Struct to represent a QDL error response (raw)
/*=========================================================================*/
struct sQDLRawErrorRsp
{
   public:
      BYTE mCommandCode;
      DWORD mErrorCode;
      BYTE mErrorText;
};

/*=========================================================================*/
// Struct sQDLRawOpenUnframedReq
//    Struct to represent a QDL open unframed image write request (raw)
/*=========================================================================*/
struct sQDLRawOpenUnframedReq
{
   public:
      BYTE mCommandCode;
      BYTE mImageType;
      DWORD mImageLength;
      BYTE mWindowSize;
      DWORD mUnframedChunkSize;
      WORD mReserved1;
};

/*=========================================================================*/
// Struct sQDLRawOpenUnframedRsp
//    Struct to represent a QDL open unframed image write response (raw)
/*=========================================================================*/
struct sQDLRawOpenUnframedRsp
{
   public:
      BYTE mCommandCode;
      WORD mStatus;
      BYTE mWindowSize;
      DWORD mUnframedChunkSize;
};

/*=========================================================================*/
// Struct sQDLRawWriteUnframedReq
//    Struct to represent a QDL unframed image write request (raw)
/*=========================================================================*/
struct sQDLRawWriteUnframedReq
{
   public:
      BYTE mCommandCode;
      WORD mSequenceNumber;
      DWORD mReserved;
      DWORD mUnframedChunkSize;
      WORD mCRC;
};

/*=========================================================================*/
// Struct sQDLRawWriteUnframedRsp
//    Struct to represent a QDL unframed image write response (raw)
/*=========================================================================*/
struct sQDLRawWriteUnframedRsp
{
   public:
      BYTE mCommandCode;
      WORD mSequenceNumber;
      DWORD mReserved;
      WORD mStatus;
};

/*=========================================================================*/
// Struct sQDLRawDoneRsp
//    Struct to represent a QDL session done response (raw)
/*=========================================================================*/
struct sQDLRawDoneRsp
{
   public:
      BYTE mCommandCode;
      WORD mStatus;
      BYTE mImageType;
      BYTE mErrorText;
};

/*=========================================================================*/
// Struct sQDLRawGetImagePrefRsp
//    Struct to represent a QDL get image preference response (raw)
/*=========================================================================*/
struct sQDLRawGetImagePrefRsp
{
   public:
      BYTE mCommandCode;
      BYTE mEntries;

      // Array of sQDLRawImageID follows (sized by mEntries)
};

/*=========================================================================*/
// Struct sQDLRawImageID
//    Struct to represent a QDL image ID (raw)
/*=========================================================================*/
struct sQDLRawImageID
{
   public:
      BYTE mImageType;
      BYTE mImageID[16];
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )


/*=========================================================================*/
// Struct sQDLHello
//    Struct to represent a QDL hello request/response (shared buffer)
/*=========================================================================*/
struct sQDLHello : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLHello( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLHello();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bRequest = (pBuf[0] == (BYTE)eQDL_CMD_HELLO_REQ);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bResponse = (pBuf[0] == (BYTE)eQDL_CMD_HELLO_RSP);
         }

         return bResponse;
      };

      // (Inline) Return raw request
      const sQDLRawHelloReq * GetRequest() const
      {
         const sQDLRawHelloReq * pReq = 0;
         if (IsRequest() == true)
         {
            pReq = (const sQDLRawHelloReq *)GetBuffer();
         }

         return pReq;
      };

      // (Inline) Return raw response
      const sQDLRawHelloRsp * GetResponse() const 
      {
         const sQDLRawHelloRsp * pRsp = 0;
         if (IsResponse() == true)
         {
            pRsp = (const sQDLRawHelloRsp *)GetBuffer();
         }

         return pRsp;
      };

      // Extract boot downloader version info from the response
      bool GetBootVersionInfo(
         ULONG &                    major,
         ULONG &                    minor ) const;

      // Build a hello request
      static sSharedBuffer * BuildHelloReq( bool bBARMode = false );

   protected:
      // Is this hello request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLHello( const sProtocolBuffer & );
      sQDLHello & operator = ( const sProtocolBuffer & );
};

/*=========================================================================*/
// Struct sQDLError
//    Struct to represent a QDL error response (shared buffer)
/*=========================================================================*/
struct sQDLError : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLError( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLError();

      // (Inline) Return raw response
      const sQDLRawErrorRsp * GetResponse() const
      {
         const sQDLRawErrorRsp * pRsp = 0;
         if (IsValid() == true)
         {
            pRsp = (const sQDLRawErrorRsp *)GetBuffer();
         }

         return pRsp;
      };

      // (Inline) Return the (validated) error code
      eQDLError GetErrorCode() const
      {
         eQDLError err = eQDL_ERROR_ENUM_BEGIN;

         const sQDLRawErrorRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            err = (eQDLError)pRsp->mErrorCode;
         }

         return err;
      };

      // (Inline) Return the error text string
      LPCSTR GetError() const
      {
         LPCSTR pErr = 0;

         const sQDLRawErrorRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            pErr = (LPCSTR)&pRsp->mErrorText;
         }

         return pErr;
      };

   protected:
      // Is this session done request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLError( const sProtocolBuffer & );
      sQDLError & operator = ( const sProtocolBuffer & );
};

/*=========================================================================*/
// Struct sQDLOpenUnframed
//    Struct to represent a QDL open image for unframed write 
//    request/response (shared buffer)
/*=========================================================================*/
struct sQDLOpenUnframed : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLOpenUnframed( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLOpenUnframed();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bRequest = (pBuf[0] == (BYTE)eQDL_CMD_OPEN_UNFRAMED_REQ);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bResponse = (pBuf[0] == (BYTE)eQDL_CMD_OPEN_UNFRAMED_RSP);
         }

         return bResponse;
      };

      // (Inline) Return raw request
      const sQDLRawOpenUnframedReq * GetRequest() const
      {
         const sQDLRawOpenUnframedReq * pReq = 0;
         if (IsRequest() == true)
         {
            pReq = (const sQDLRawOpenUnframedReq *)GetBuffer();
         }

         return pReq;
      };

      // (Inline) Return raw response
      const sQDLRawOpenUnframedRsp * GetResponse() const
      {
         const sQDLRawOpenUnframedRsp * pRsp = 0;
         if (IsResponse() == true)
         {
            pRsp = (const sQDLRawOpenUnframedRsp *)GetBuffer();
         }

         return pRsp;
      };

      // (Inline) Does the response indicate success?
      bool IsSuccess() const
      {
         bool bSuccess = false;

         const sQDLRawOpenUnframedRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            bSuccess = (pRsp->mStatus == eQDL_OPEN_STATUS_SUCCESS);
         }

         return bSuccess;
      };

      // Extract supported chunk size from the response
      bool GetChunkSize( ULONG & chunkSize ) const;

      // Build an open image for unframed write request
      static sSharedBuffer * BuildOpenUnframedReq(
         eQDLImageType              imageType,
         ULONG                      imageSize,
         ULONG                      chunkSize );

   protected:
      // Is this open unframed request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLOpenUnframed( const sProtocolBuffer & );
      sQDLOpenUnframed & operator = ( const sProtocolBuffer & );
};

/*=========================================================================*/
// Struct sQDLWriteUnframed
//    Struct to represent a QDL unframed write of an image 
//    request/response (shared buffer)
/*=========================================================================*/
struct sQDLWriteUnframed : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLWriteUnframed( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLWriteUnframed();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bRequest = (pBuf[0] == (BYTE)eQDL_CMD_WRITE_UNFRAMED_REQ);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bResponse = (pBuf[0] == (BYTE)eQDL_CMD_WRITE_UNFRAMED_RSP);
         }

         return bResponse;
      };

      // (Inline) Return raw request
      const sQDLRawWriteUnframedReq * GetRequest() const
      {
         const sQDLRawWriteUnframedReq * pReq = 0;
         if (IsRequest() == true)
         {
            pReq = (const sQDLRawWriteUnframedReq *)GetBuffer();
         }

         return pReq;
      };

      // (Inline) Return raw response
      const sQDLRawWriteUnframedRsp * GetResponse() const
      {
         const sQDLRawWriteUnframedRsp * pRsp = 0;
         if (IsResponse() == true)
         {
            pRsp = (const sQDLRawWriteUnframedRsp *)GetBuffer();
         }

         return pRsp;
      };

      // (Inline) Does the response indicate success?
      bool IsSuccess() const
      {
         bool bSuccess = false;

         const sQDLRawWriteUnframedRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            bSuccess = (pRsp->mStatus == eQDL_WRITE_STATUS_SUCCESS);
         }

         return bSuccess;
      };

      // Extract sequence number from the response
      bool GetSequenceNumber( ULONG & sequenceNumber ) const;

      // Build an unframed write request
      static sSharedBuffer * BuildWriteUnframedReq( 
         USHORT                     sequenceNumber,
         ULONG                      chunkSize );

      // Build unframed write requests for the specified parameters
      static std::list <sSharedBuffer *> BuildWriteUnframedReqs( 
         ULONG                      chunkSize,
         ULONG                      totalSize );

   protected:
      // Is this open unframed request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLWriteUnframed( const sProtocolBuffer & );
      sQDLWriteUnframed & operator = ( const sProtocolBuffer & );
};

/*=========================================================================*/
// Struct sQDLDone
//    Struct to represent a QDL session done request/response (shared buffer)
/*=========================================================================*/
struct sQDLDone : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLDone( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLDone();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bRequest = (pBuf[0] == (BYTE)eQDL_CMD_SESSION_DONE_REQ);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bResponse = (pBuf[0] == (BYTE)eQDL_CMD_SESSION_DONE_RSP);
         }

         return bResponse;
      };

      // (Inline) Return raw response
      const sQDLRawDoneRsp * GetResponse() const
      {
         const sQDLRawDoneRsp * pRsp = 0;
         if (IsResponse() == true)
         {
            pRsp = (const sQDLRawDoneRsp *)GetBuffer();
         }

         return pRsp;
      };

      // (Inline) Does the response indicate success?
      bool IsSuccess() const
      {
         bool bSuccess = false;

         const sQDLRawDoneRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            bSuccess = (pRsp->mStatus == eQDL_DONE_STATUS_SUCCESS);
         }

         return bSuccess;
      };

      // (Inline) Return the error text string
      LPCSTR GetError() const
      {
         LPCSTR pErr = 0;

         const sQDLRawDoneRsp * pRsp = GetResponse();
         if (pRsp != 0)
         {
            if (pRsp->mStatus != eQDL_DONE_STATUS_SUCCESS)
            {
               pErr = (LPCSTR)&pRsp->mErrorText;
            }
         }

         return pErr;
      };

   protected:
      // Is this session done request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLDone( const sProtocolBuffer & );
      sQDLDone & operator = ( const sProtocolBuffer & );
};

/*=========================================================================*/
// Struct sQDLGetImagePref
//    Struct to represent a QDL get image preference 
//    request/response (shared buffer)
/*=========================================================================*/
struct sQDLGetImagePref : public sProtocolBuffer
{
   public:
      // Constructor
      sQDLGetImagePref( sSharedBuffer * pBuffer );

      // Destructor
      virtual ~sQDLGetImagePref();

      // (Inline) Is this a request?
      bool IsRequest() const
      {
         bool bRequest = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bRequest = (pBuf[0] == (BYTE)eQDL_CMD_GET_IMAGE_PREF_REQ);
         }

         return bRequest;
      };

      // (Inline) Is this a response?
      bool IsResponse() const
      {
         bool bResponse = false;
         if (IsValid() == true)
         {
            const BYTE * pBuf = GetBuffer();
            bResponse = (pBuf[0] == (BYTE)eQDL_CMD_GET_IMAGE_PREF_RSP);
         }

         return bResponse;
      };

      // (Inline) Return raw response
      const sQDLRawGetImagePrefRsp * GetResponse() const
      {
         const sQDLRawGetImagePrefRsp * pRsp = 0;
         if (IsResponse() == true)
         {
            pRsp = (const sQDLRawGetImagePrefRsp *)GetBuffer();
         }

         return pRsp;
      };

      // Return image IDs
      std::list <sQDLRawImageID> GetImageIDs() const;

   protected:
      // Is this get image preference request/response packet valid?
      virtual bool Validate();

   private:
      // Prevent 'upcopying'
      sQDLGetImagePref( const sProtocolBuffer & );
      sQDLGetImagePref & operator = ( const sProtocolBuffer & );
};
