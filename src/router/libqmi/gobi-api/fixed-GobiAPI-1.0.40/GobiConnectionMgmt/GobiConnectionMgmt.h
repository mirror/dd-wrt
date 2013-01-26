/*===========================================================================
FILE: 
   GobiConnectionMgmt.h

DESCRIPTION:
   QUALCOMM Connection Management API for Gobi 3000

PUBLIC CLASSES AND FUNCTIONS:
   cGobiConnectionMgmtDLL
   cGobiConnectionMgmt

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

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
extern "C" 
{

// Session state callback function
typedef void (* tFNSessionState)( 
ULONG                      state,
ULONG                      sessionEndReason );

// RX/TX byte counts callback function
typedef void (* tFNByteTotals)( 
ULONGLONG                  totalBytesTX,
ULONGLONG                  totalBytesRX );

// Dormancy status callback function
typedef void (* tFNDormancyStatus)( ULONG dormancyStatus );

// Mobile IP status callback function
typedef void (* tFNMobileIPStatus)( ULONG mipStatus );

// Activation status callback function
typedef void (* tFNActivationStatus)( ULONG activationStatus );

// Power operating mode callback function
typedef void (* tFNPower)( ULONG operatingMode );

// Wireless disable callback function
typedef void (* tFNWirelessDisable)( ULONG bState );

// Serving system data capabilities callback function
typedef void (* tFNDataCapabilities)(
BYTE                       dataCapsSize, 
BYTE *                     pDataCaps );

// Data bearer technology callback function
typedef void (* tFNDataBearer)( ULONG dataBearer );

// Roaming indicator callback function
typedef void (* tFNRoamingIndicator)( ULONG roaming );

// Signal strength callback function
typedef void (* tFNSignalStrength)(
INT8                       signalStrength, 
ULONG                      radioInterface );

// RF information callback function
typedef void (* tFNRFInfo)( 
ULONG                      radioInterface,
ULONG                      activeBandClass,
ULONG                      activeChannel );

// LU reject callback function
typedef void (* tFNLUReject)( 
ULONG                      serviceDomain,
ULONG                      rejectCause );

// PLMN mode callback function
typedef void (* tFNPLMNMode)( ULONG mode );

// New SMS message callback function
typedef void (* tFNNewSMS)( 
ULONG                      storageType,
ULONG                      messageIndex );

// New NMEA sentence callback function
typedef void (* tFNNewNMEA)( LPCSTR pNMEA );

// PDS session state callback function
typedef void (* tFNPDSState)( 
ULONG                      enabledStatus,
ULONG                      trackingStatus );

// CAT event callback function
typedef void (* tFNCATEvent)( 
ULONG                      eventID,
ULONG                      eventLen,
BYTE *                     pEventData );

// OMA-DM network initiated alert callback function
typedef void (* tFNOMADMAlert)( 
   ULONG                      sessionType,
   USHORT                     sessionID );

// OMA-DM state callback function
typedef void (* tFNOMADMState)( 
   ULONG                      sessionState,
   ULONG                      failureReason );

// USSD release callback function
typedef void (* tFNUSSDRelease)();

// USSD notification callback function
typedef void (* tFNUSSDNotification)( 
   ULONG                      type,
   BYTE *                     pNetworkInfo );

// USSD origination callback function
typedef void (* tFNUSSDOrigination)( 
   ULONG                      errorCode,
   ULONG                      failureCause,
   BYTE *                     pNetworkInfo,
   BYTE *                     pAlpha );

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
// Class cSessionStateCallback
/*=========================================================================*/
class cSessionStateCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cSessionStateCallback(
         tFNSessionState            pCallback,
         ULONG                      state,
         ULONG                      sessionEndReason )
         :  mpCallback( pCallback ),
            mState( state ),
            mSessionEndReason( sessionEndReason )
      { };

      // (Inline) Destructor
      virtual ~cSessionStateCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mState, mSessionEndReason );
         }
      };
   
      /* Callback function */
      tFNSessionState mpCallback;

      /* Callback arguments */
      ULONG mState;
      ULONG mSessionEndReason;
};

/*=========================================================================*/
// Class cByteTotalsCallback
/*=========================================================================*/
class cByteTotalsCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cByteTotalsCallback(
         tFNByteTotals              pCallback,
         ULONGLONG                  totalBytesTX,
         ULONGLONG                  totalBytesRX )
         :  mpCallback( pCallback ),
            mTotalBytesTX( totalBytesTX ),
            mTotalBytesRX( totalBytesRX )
      { };

      // (Inline) Destructor
      virtual ~cByteTotalsCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mTotalBytesTX, mTotalBytesRX );
         }
      };
   
      /* Callback function */
      tFNByteTotals mpCallback;

      /* Callback arguments */
      ULONGLONG mTotalBytesTX;
      ULONGLONG mTotalBytesRX;
};

/*=========================================================================*/
// Class cDormancyStatusCallback
/*=========================================================================*/
class cDormancyStatusCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cDormancyStatusCallback(
         tFNDormancyStatus          pCallback,
         ULONG                      dormancyStatus )
         :  mpCallback( pCallback ),
            mDormancyStatus( dormancyStatus )
      { };

      // (Inline) Destructor
      virtual ~cDormancyStatusCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mDormancyStatus );
         }
      };
   
      /* Callback function */
      tFNDormancyStatus mpCallback;

      /* Callback arguments */
      ULONG mDormancyStatus;
};

/*=========================================================================*/
// Class cMobileIPStatusCallback
/*=========================================================================*/
class cMobileIPStatusCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cMobileIPStatusCallback(
         tFNMobileIPStatus          pCallback,
         ULONG                      mobileIPStatus )
         :  mpCallback( pCallback ),
            mMobileIPStatus( mobileIPStatus )
      { };

      // (Inline) Destructor
      virtual ~cMobileIPStatusCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mMobileIPStatus );
         }
      };
   
      /* Callback function */
      tFNMobileIPStatus mpCallback;

      /* Callback arguments */
      ULONG mMobileIPStatus;
};

/*=========================================================================*/
// Class cActivationStatusCallback
/*=========================================================================*/
class cActivationStatusCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cActivationStatusCallback(
         tFNActivationStatus        pCallback,
         ULONG                      activationStatus )
         :  mpCallback( pCallback ),
            mActivationStatus( activationStatus )
      { };

      // (Inline) Destructor
      virtual ~cActivationStatusCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mActivationStatus );
         }
      };
   
      /* Callback function */
      tFNActivationStatus mpCallback;

      /* Callback arguments */
      ULONG mActivationStatus;
};

/*=========================================================================*/
// Class cPowerCallback
/*=========================================================================*/
class cPowerCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cPowerCallback(
         tFNPower                   pCallback,
         ULONG                      operatingMode )
         :  mpCallback( pCallback ),
            mOperatingMode( operatingMode )
      { };

      // (Inline) Destructor
      virtual ~cPowerCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mOperatingMode );
         }
      };
   
      /* Callback function */
      tFNPower mpCallback;

      /* Callback arguments */
      ULONG mOperatingMode;
};

/*=========================================================================*/
// Class cWirelessDisableCallback
/*=========================================================================*/
class cWirelessDisableCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cWirelessDisableCallback(
         tFNWirelessDisable         pCallback,
         ULONG                      bState )
         :  mpCallback( pCallback ),
            mbState( bState )
      { };

      // (Inline) Destructor
      virtual ~cWirelessDisableCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mbState );
         }
      };
   
      /* Callback function */
      tFNWirelessDisable mpCallback;

      /* Callback arguments */
      ULONG mbState;
};

/*=========================================================================*/
// Class cDataCapabilitiesCallback
/*=========================================================================*/
class cDataCapabilitiesCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cDataCapabilitiesCallback(
         tFNDataCapabilities        pCallback,
         BYTE                       dataCapsSize,
         ULONG *                    pDataCaps )
         :  mpCallback( pCallback ),
            mDataCapsSize( dataCapsSize )
      {
         memset( (LPVOID)&mDataCaps[0], 0, 12 * sizeof( ULONG ) );

         if (mDataCapsSize > 12)
         {
            mDataCapsSize = 12;
         }

         for (ULONG d = 0; d < mDataCapsSize; d++)
         {
            mDataCaps[d] = pDataCaps[d];
         }
      };

      // (Inline) Destructor
      virtual ~cDataCapabilitiesCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mDataCapsSize, (BYTE *)&mDataCaps[0] );
         }
      };
   
      /* Callback function */
      tFNDataCapabilities mpCallback;

      /* Callback arguments */
      BYTE mDataCapsSize;
      ULONG mDataCaps[12];
};

/*=========================================================================*/
// Class cDataBearerCallback
/*=========================================================================*/
class cDataBearerCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cDataBearerCallback(
         tFNDataBearer              pCallback,
         ULONG                      dataBearer )
         :  mpCallback( pCallback ),
            mDataBearer( dataBearer )
      { };

      // (Inline) Destructor
      virtual ~cDataBearerCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mDataBearer );
         }
      };
   
      /* Callback function */
      tFNDataBearer mpCallback;

      /* Callback arguments */
      ULONG mDataBearer;
};

/*=========================================================================*/
// Class cRoamingIndicatorCallback
/*=========================================================================*/
class cRoamingIndicatorCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cRoamingIndicatorCallback(
         tFNRoamingIndicator        pCallback,
         ULONG                      roaming )
         :  mpCallback( pCallback ),
            mRoaming( roaming )
      { };

      // (Inline) Destructor
      virtual ~cRoamingIndicatorCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mRoaming );
         }
      };
   
      /* Callback function */
      tFNRoamingIndicator mpCallback;

      /* Callback arguments */
      ULONG mRoaming;
};

/*=========================================================================*/
// Class cSignalStrengthCallback
/*=========================================================================*/
class cSignalStrengthCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cSignalStrengthCallback(
         tFNSignalStrength          pCallback,
         INT8                       signalStrength,
         ULONG                      radioInterface )
         :  mpCallback( pCallback ),
            mSignalStrength( signalStrength ),
            mRadioInterface( radioInterface )
      { };

      // (Inline) Destructor
      virtual ~cSignalStrengthCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mSignalStrength, mRadioInterface );
         }
      };
   
      /* Callback function */
      tFNSignalStrength mpCallback;

      /* Callback arguments */
      INT8 mSignalStrength;
      ULONG mRadioInterface;
};

/*=========================================================================*/
// Class cRFInfoCallback
/*=========================================================================*/
class cRFInfoCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cRFInfoCallback(
         tFNRFInfo                  pCallback,
         ULONG                      radioInterface,
         ULONG                      activeBandClass,
         ULONG                      activeChannel )
         :  mpCallback( pCallback ),
            mRadioInterface( radioInterface ),
            mActiveBandClass( activeBandClass ),
            mActiveChannel( activeChannel )
      { };

      // (Inline) Destructor
      virtual ~cRFInfoCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mRadioInterface, mActiveBandClass, mActiveChannel );
         }
      };
   
      /* Callback function */
      tFNRFInfo mpCallback;

      /* Callback arguments */
      ULONG mRadioInterface;
      ULONG mActiveBandClass;
      ULONG mActiveChannel;
};

/*=========================================================================*/
// Class cLURejectCallback
/*=========================================================================*/
class cLURejectCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cLURejectCallback(
         tFNLUReject                pCallback,
         ULONG                      serviceDomain,
         ULONG                      rejectCause )
         :  mpCallback( pCallback ),
            mServiceDomain( serviceDomain ),
            mRejectCause( rejectCause )
      { };

      // (Inline) Destructor
      virtual ~cLURejectCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mServiceDomain, mRejectCause );
         }
      };
   
      /* Callback function */
      tFNLUReject mpCallback;

      /* Callback arguments */
      ULONG mServiceDomain;
      ULONG mRejectCause;
};

/*=========================================================================*/
// Class cPLMNModeCallback
/*=========================================================================*/
class cPLMNModeCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cPLMNModeCallback(
         tFNPLMNMode                pCallback,
         ULONG                      mode )
         :  mpCallback( pCallback ),
            mMode( mode )
      { };

      // (Inline) Destructor
      virtual ~cPLMNModeCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mMode );
         }
      };
   
      /* Callback function */
      tFNPLMNMode mpCallback;

      /* Callback arguments */
      ULONG mMode;
};

/*=========================================================================*/
// Class cNewSMSCallback
/*=========================================================================*/
class cNewSMSCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cNewSMSCallback(
         tFNNewSMS                  pCallback,
         ULONG                      storageType,
         ULONG                      messageIndex )
         :  mpCallback( pCallback ),
            mStorageType( storageType ),
            mMessageIndex( messageIndex )
      { };

      // (Inline) Destructor
      virtual ~cNewSMSCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mStorageType, mMessageIndex );
         }
      };
   
      /* Callback function */
      tFNNewSMS mpCallback;

      /* Callback arguments */
      ULONG mStorageType;
      ULONG mMessageIndex;
};

/*=========================================================================*/
// Class cNewNMEACallback
/*=========================================================================*/
class cNewNMEACallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cNewNMEACallback(
         tFNNewNMEA                 pCallback,
         std::string &              nmea )
         :  mpCallback( pCallback )
      {
         memset( (LPVOID)&mNMEA[0], 0, 512 );

         ULONG len = nmea.size();
         if (len > 0 && len < 512)
         {
            memcpy( (LPVOID)&mNMEA[0], 
                    (LPCVOID)nmea.c_str(),
                    (SIZE_T)len );
         }
      };

      // (Inline) Destructor
      virtual ~cNewNMEACallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0 && mNMEA[0] != 0)
         {
            mpCallback( &mNMEA[0] );
         }
      };
   
      /* Callback function */
      tFNNewNMEA mpCallback;

      /* Callback arguments */
      CHAR mNMEA[512];
};

/*=========================================================================*/
// Class cPDSStateCallback
/*=========================================================================*/
class cPDSStateCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cPDSStateCallback(
         tFNPDSState                pCallback,
         ULONG                      enabledState,
         ULONG                      trackingState )
         :  mpCallback( pCallback ),
            mEnabledState( enabledState ),
            mTrackingState( trackingState )
      { };

      // (Inline) Destructor
      virtual ~cPDSStateCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mEnabledState, mTrackingState );
         }
      };
   
      /* Callback function */
      tFNPDSState mpCallback;

      /* Callback arguments */
      ULONG mTrackingState;
      ULONG mEnabledState;
};

/*=========================================================================*/
// Class cCATEventCallback
/*=========================================================================*/
class cCATEventCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cCATEventCallback(
         tFNCATEvent                pCallback,
         ULONG                      eventID,
         ULONG                      eventLen,
         const BYTE *               pEventData )
         :  mpCallback( pCallback ),
            mEventID( eventID ),
            mEventLen( 0 )
      {
         memset( (LPVOID)&mData[0], 0, 2048 );

         if (pEventData != 0 && eventLen > 0 && eventLen < 2048)
         {
            mEventLen = eventLen;
            memcpy( (LPVOID)&mData[0], 
                    (LPCVOID)pEventData, 
                    (SIZE_T)eventLen );
         }
      };

      // (Inline) Destructor
      virtual ~cCATEventCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mEventID, mEventLen, &mData[0] );
         }
      };
   
      /* Callback function */
      tFNCATEvent mpCallback;

      /* Callback arguments */
      ULONG mEventID;
      ULONG mEventLen;
      BYTE mData[2048];
};

/*=========================================================================*/
// Class cOMADMAlertCallback
/*=========================================================================*/
class cOMADMAlertCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cOMADMAlertCallback(
         tFNOMADMAlert              pCallback,
         ULONG                      sessionType,
         USHORT                     sessionID )
         :  mpCallback( pCallback ),
            mSessionType( sessionType ),
            mSessionID( sessionID )
      { };

      // (Inline) Destructor
      virtual ~cOMADMAlertCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mSessionType, mSessionID );
         }
      };
   
      /* Callback function */
      tFNOMADMAlert mpCallback;

      /* Callback arguments */
      ULONG mSessionType;
      USHORT mSessionID;
};

/*=========================================================================*/
// Class cOMADMStateCallback
/*=========================================================================*/
class cOMADMStateCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cOMADMStateCallback(
         tFNOMADMState              pCallback,
         ULONG                      sessionState,
         ULONG                      failureReason )
         :  mpCallback( pCallback ),
            mSessionState( sessionState ),
            mFailureReason( failureReason )
      { };

      // (Inline) Destructor
      virtual ~cOMADMStateCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback( mSessionState, mFailureReason );
         }
      };
   
      /* Callback function */
      tFNOMADMState mpCallback;

      /* Callback arguments */
      ULONG mSessionState;
      ULONG mFailureReason;
};

/*=========================================================================*/
// Class cUSSDReleaseCallback
/*=========================================================================*/
class cUSSDReleaseCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cUSSDReleaseCallback( tFNUSSDRelease pCallback )
         :  mpCallback( pCallback )
      { };

      // (Inline) Destructor
      virtual ~cUSSDReleaseCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            mpCallback();
         }
      };
   
      /* Callback function */
      tFNUSSDRelease mpCallback;
};

/*=========================================================================*/
// Class cUSSDNotificationCallback
/*=========================================================================*/
class cUSSDNotificationCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cUSSDNotificationCallback(
         tFNUSSDNotification        pCallback,
         ULONG                      type,
         const BYTE *               pData )
         :  mpCallback( pCallback ),
            mType( type ),
            mbData( false )
      {
         memset( (LPVOID)&mData[0], 0, 512 );

         // Data to copy?
         if (pData != 0)
         {
            ULONG len = (ULONG)pData[1] + (ULONG)2;
            memcpy( (LPVOID)&mData[0], (LPCVOID)pData, (SIZE_T)len );

            mbData = true;
         }
      };

      // (Inline) Destructor
      virtual ~cUSSDNotificationCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            if (mbData == true)
            {
               mpCallback( mType, &mData[0] );
            }
            else
            {
               mpCallback( mType, 0 );
            }
         }
      };
   
      /* Callback function */
      tFNUSSDNotification mpCallback;

      /* Callback arguments */
      ULONG mType;
      BYTE mData[512];

      /* Did we get data? */
      bool mbData;
};

/*=========================================================================*/
// Class cUSSDOriginationCallback
/*=========================================================================*/
class cUSSDOriginationCallback : public cGobiCMCallback
{
   public:
      // (Inline) Constructor
      cUSSDOriginationCallback(
         tFNUSSDOrigination         pCallback,
         ULONG                      errorCode,
         ULONG                      failureCause,
         const BYTE *               pNetworkInfo,
         const BYTE *               pAlpha )
         :  mpCallback( pCallback ),
            mErrorCode( errorCode ),
            mFailureCause( failureCause ),
            mbNetwork( false ),
            mbAlpha( 0 )
      {
         memset( &mNetworkInfo[0], 0, 512 );
         memset( &mAlpha[0], 0, 512 );

         // Data to copy?
         if (pNetworkInfo != 0)
         {
            ULONG len = (ULONG)pNetworkInfo[1] + (ULONG)2;
            memcpy( &mNetworkInfo[0], 
                    pNetworkInfo, 
                    len );

            mbNetwork = true;
         }

         if (pAlpha != 0)
         {
            ULONG len = (ULONG)pAlpha[1] + (ULONG)2;
            memcpy( &mAlpha[0], 
                    pAlpha, 
                    len );

            mbAlpha = true;
         }
      };

      // (Inline) Destructor
      virtual ~cUSSDOriginationCallback()
      {
         mpCallback = 0;
      };

   protected:
      // (Inline) Call the function
      virtual void Call()
      {
         if (mpCallback != 0)
         {
            BYTE * pNetworkInfo = (mbNetwork == true ? &mNetworkInfo[0] : 0);
            BYTE * pAlpha = (mbAlpha == true ? &mAlpha[0] : 0);

            mpCallback( mErrorCode, mFailureCause, pNetworkInfo, pAlpha );
         }
      };
   
      /* Callback function */
      tFNUSSDOrigination mpCallback;

      /* Callback arguments */
      ULONG mErrorCode;
      ULONG mFailureCause;
      BYTE mNetworkInfo[512];
      BYTE mAlpha[512];

      /* Did we get data? */
      bool mbNetwork;
      bool mbAlpha;
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

      // Connect to the specified Gobi device
      virtual bool Connect(
         LPCSTR                    pDeviceNode = 0,
         LPCSTR                    pDeviceKey  = 0 );

      // Disconnect from the currently connected Gobi device
      virtual bool Disconnect();

      // Enable/disable session state callback function
      eGobiError SetSessionStateCallback( tFNSessionState pCallback );

      // Enables/disables the RX/TX byte counts callback function
      eGobiError SetByteTotalsCallback( 
         tFNByteTotals              pCallback,
         BYTE                       interval );

      // Enables/disables the serving system data capabilities callback
      eGobiError SetDataCapabilitiesCallback( tFNDataCapabilities pCallback );

      // Enable/disable data bearer callback function
      eGobiError SetDataBearerCallback( tFNDataBearer pCallback );

      // Enable/disable dormancy status callback function
      eGobiError SetDormancyStatusCallback( tFNDormancyStatus pCallback );

      // Enable/disable mobile IP status callback function
      eGobiError SetMobileIPStatusCallback( tFNDormancyStatus pCallback );

      // Enable/disable activation status callback function
      eGobiError SetActivationStatusCallback( tFNActivationStatus pCallback );

      // Enable/disable power operating mode callback function
      eGobiError SetPowerCallback( tFNPower pCallback );

      // Enable/disable wireless disable state callback function
      eGobiError SetWirelessDisableCallback( tFNWirelessDisable pCallback );

      // Enable/disable roaming indicator callback function
      eGobiError SetRoamingIndicatorCallback( tFNRoamingIndicator pCallback );

      // Enable/disable signal strength callback function
      eGobiError SetSignalStrengthCallback( 
         tFNSignalStrength          pCallback,
         std::list <INT8>           thresholds );

      // Enable/disable RF information callback function
      eGobiError SetRFInfoCallback( tFNRFInfo pCallback );

      // Enable/disable LU reject callback function
      eGobiError SetLURejectCallback( tFNLUReject pCallback );

      // Enable/disable PLMN mode callback function
      eGobiError SetPLMNModeCallback( tFNPLMNMode pCallback );

      // Enable/disable new SMS callback function
      eGobiError SetNewSMSCallback( tFNNewSMS pCallback );

      // Enable/disable NMEA sentence callback function
      eGobiError SetNMEACallback( tFNNewNMEA pCallback );

      // Enable/disable PDS service state callback function
      eGobiError SetPDSStateCallback( tFNPDSState pCallback );

      // Enable/disable CAT event callback function
      eGobiError SetCATEventCallback( 
         tFNCATEvent                pCallback,
         ULONG                      eventMask,
         ULONG *                    pErrorMask );

      // Enable/disable OMA-DM NIA callback function
      eGobiError SetOMADMAlertCallback( tFNOMADMAlert pCallback );

      // Enable/disable OMA-DM state callback function
      eGobiError SetOMADMStateCallback( tFNOMADMState pCallback );

      // Enable/disable USSD release callback function
      eGobiError SetUSSDReleaseCallback( tFNUSSDRelease pCallback );

      // Enable/disable USSD notification callback function
      eGobiError SetUSSDNotificationCallback( tFNUSSDNotification pCallback );

      // Enable/disable USSD origination callback function
      eGobiError SetUSSDOriginationCallback( tFNUSSDOrigination pCallback );

   protected:
      // Process new traffic
      void ProcessTraffic( eQMIService svc );

      // Process QMI traffic
      void ProcessWDSBuffer( const sProtocolBuffer & buf );
      void ProcessDMSBuffer( const sProtocolBuffer & buf );
      void ProcessNASBuffer( const sProtocolBuffer & buf );
      void ProcessWMSBuffer( const sProtocolBuffer & buf );
      void ProcessPDSBuffer( const sProtocolBuffer & buf );
      void ProcessCATBuffer( const sProtocolBuffer & buf );
      void ProcessOMABuffer( const sProtocolBuffer & buf );
      void ProcessVoiceBuffer( const sProtocolBuffer & buf );

      /* Is there an active thread? */
      bool mbThreadStarted;

      /* ID of traffic processing thread */
      pthread_t mThreadID;

      /* Traffic processing thread exit event */
      cEvent mExitEvent;

      /* Has the protocol server thread finished cleanup? */
      bool mThreadCleanupFinished;

      /* Number of buffers processed by ProcessTraffic() (per server) */
      ULONG mWDSItemsProcessed;
      ULONG mDMSItemsProcessed;
      ULONG mNASItemsProcessed;
      ULONG mWMSItemsProcessed;
      ULONG mPDSItemsProcessed;
      ULONG mCATItemsProcessed;
      ULONG mOMAItemsProcessed;
      ULONG mVoiceItemsProcessed;

      /* Callback functions */
      tFNSessionState mpFNSessionState;
      tFNByteTotals mpFNByteTotals;
      tFNDataCapabilities mpFNDataCapabilities;
      tFNDataBearer mpFNDataBearer;
      tFNDormancyStatus mpFNDormancyStatus;
      tFNMobileIPStatus mpFNMobileIPStatus;
      tFNActivationStatus mpFNActivationStatus;
      tFNPower mpFNPower;
      tFNWirelessDisable mpFNWirelessDisable;
      tFNRoamingIndicator mpFNRoamingIndicator;
      tFNSignalStrength mpFNSignalStrength;
      tFNRFInfo mpFNRFInfo;
      tFNLUReject mpFNLUReject;
      tFNPLMNMode mpPLMNMode;
      tFNNewSMS mpFNNewSMS;
      tFNNewNMEA mpFNNewNMEA;
      tFNPDSState mpFNPDSState;
      tFNCATEvent mpFNCATEvent;
      tFNOMADMAlert mpFNOMADMAlert;
      tFNOMADMState mpFNOMADMState;
      tFNUSSDRelease mpFNUSSDRelease;
      tFNUSSDNotification mpFNUSSDNotification;
      tFNUSSDOrigination mpFNUSSDOrigination;

      // Traffic process thread gets full access
      friend VOID * TrafficProcessThread( PVOID pArg );
};

/*=========================================================================*/
// Class cGobiConnectionMgmtDLL 
/*=========================================================================*/
class cGobiConnectionMgmtDLL
{
   public:
      // Constructor
      cGobiConnectionMgmtDLL();

      // Destructor
      virtual ~cGobiConnectionMgmtDLL();

      // Return the GobiConnectionMgmt object
      cGobiConnectionMgmt * GetAPI();

   protected:
      /* API interface object */
      cGobiConnectionMgmt * mpAPI;

      /* Above object allocation attempted? */
      bool mbAllocated;

      /* Synchronization object */
      mutable pthread_mutex_t mSyncSection;
};

extern cGobiConnectionMgmtDLL gConnectionDLL;
