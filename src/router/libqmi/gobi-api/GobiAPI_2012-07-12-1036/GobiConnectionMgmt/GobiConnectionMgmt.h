/*===========================================================================
FILE: 
   GobiConnectionMgmt.h

DESCRIPTION:
   QUALCOMM Connection Management API for Gobi

PUBLIC CLASSES AND FUNCTIONS:
   CGobiConnectionMgmtDLL
   cGobiConnectionMgmt

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

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

#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Handle to Gobi API
typedef ULONG_PTR GOBIHANDLE;

extern "C" 
{
   // Generic callback function pointer
   typedef void (* tFNGenericCallback)(
      ULONG                         svcID,
      ULONG                         msgID,
      ULONG_PTR                     userValue,
      ULONG                         outLen,
      const BYTE *                  pOut );

};

// CallbackThread prototype
// Thread to execute a callback asynchronously
void * CallbackThread( PVOID pArg );

/*=========================================================================*/
// Class cGobiCMCallback
/*=========================================================================*/
class cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cGobiCMCallback()
      { };

      // (Inline) Destructor
      virtual ~cGobiCMCallback()
      { };

      // (Inline) Initialize the callback object by starting the thread
      bool Initialize()
      {
         // Start the thread
         pthread_t threadID;
         pthread_attr_t attributes;
         pthread_attr_init( &attributes );
         pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );

         int nRC = pthread_create( &threadID,
                                   &attributes,
                                   CallbackThread,
                                   this );

         if (nRC == 0)
         {
            // Success!
            return true;
         }

         return false;
      };

   protected:
      // Call the function
      virtual void Call() = 0;

      // Function thread gets full access
      friend void * CallbackThread( PVOID pArg );
};

/*=========================================================================*/
// Class cGenericCallback
/*=========================================================================*/
class cGenericCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cGenericCallback(
         tFNGenericCallback            pCallback,
         ULONG                         svcID,
         ULONG                         msgID,
         ULONG_PTR                     userValue,
         ULONG                         outLen,
         const BYTE *                  pOut )
         :  mServiceID( svcID ),
            mMessageID( msgID ),
            mUserValue( userValue ),
            mOutputLen( 0 ),
            mpCallback( pCallback )
      { 
         memset( &mOutput[0], 0, QMI_MAX_BUFFER_SIZE );
         if (outLen <= QMI_MAX_BUFFER_SIZE && pOut != 0)
         {
            mOutputLen = outLen;
            memcpy( &mOutput[0], pOut, outLen );
         }
      };

      // (Inline) Destructor
      virtual ~cGenericCallback()
      {
         mpCallback = 0;
      };

   protected:
      /* Service ID */
      ULONG mServiceID;

      /* Message ID */
      ULONG mMessageID;

      /* User value */
      ULONG_PTR mUserValue;

      /* Actual size of output content */
      ULONG mOutputLen;

      /* Output content buffer */
      BYTE mOutput[QMI_MAX_BUFFER_SIZE];

      /* Callback function */
      tFNGenericCallback mpCallback;

      // Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mServiceID, 
                        mMessageID,
                        mUserValue,
                        mOutputLen, 
                        (const BYTE *)&mOutput[0] );
         }
      };
};

/*=========================================================================*/
// Class cGobiConnectionMgmt
/*=========================================================================*/
class cGobiConnectionMgmt : public cGobiQMICore
{
   public:
      // Constructor
      cGobiConnectionMgmt();

      // Destructor
      virtual ~cGobiConnectionMgmt();

      // Connect to the specified Gobi device interface
      virtual std::set <eQMIService> Connect( 
         LPCSTR                     pInterface,
         std::set <eQMIService> &   services );

      // Disconnect from the currently connected device interface
      virtual bool Disconnect();

      // Enable/disable generic callback function
      eGobiError SetGenericCallback( 
         ULONG                      svcID,
         ULONG                      msgID,
         tFNGenericCallback         pCallback,
         ULONG_PTR                  userValue );

   protected:
      // Process new traffic
      void ProcessTraffic( eQMIService svc );

      /* Is there an active thread? */
      bool mbThreadStarted;

      /* ID of traffic processing thread */
      pthread_t mThreadID;

      /* Traffic processing thread exit event */
      cEvent mExitEvent;

      /* Has the protocol server thread finished cleanup? */
      bool mThreadCleanupFinished;

      /* Generic callback function key/value */
      typedef std::pair <ULONG, ULONG> tCallbackKey;
      typedef std::pair <tFNGenericCallback, ULONG_PTR> tCallbackValue;

      /* Callback functions */
      std::map <tCallbackKey, tCallbackValue> mCallbacks;
      
      // Traffic process thread gets full access
      friend VOID * TrafficProcessThread( PVOID pArg );
};

/*=========================================================================*/
// Class CGobiConnectionMgmtDLL 
/*=========================================================================*/
class CGobiConnectionMgmtDLL
{
   public:
      // Constructor
      CGobiConnectionMgmtDLL();

      // Destructor
      virtual ~CGobiConnectionMgmtDLL();

      // Create a new API object 
      GOBIHANDLE CreateAPI();

      // Delete an existing API object 
      void DeleteAPI( GOBIHANDLE handle );

      // Return the requested API object
      cGobiConnectionMgmt * GetAPI( GOBIHANDLE handle );

   protected:
      /* API interface object */
      std::map <GOBIHANDLE, cGobiConnectionMgmt *> mAPI;

      /* Synchronization object */
      mutable pthread_mutex_t mSyncSection;
};

extern CGobiConnectionMgmtDLL gDLL;
