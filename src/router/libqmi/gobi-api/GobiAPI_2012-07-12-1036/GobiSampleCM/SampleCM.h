/*===========================================================================
FILE:
   SampleCM.h

DESCRIPTION:
   Generic class to act as Sample CM interface
   
PUBLIC CLASSES AND METHODS:
   cSampleCM
      Generic class to act as Sample CM interface

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
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "GobiCMDLL.h"
#include "Event.h"

#include <string>
#include <sstream>
#include <list>

// Prototypes
void * DeviceDetectionThread( void * pData );
void * UpdateNetworkInfo( void * pData );

// A global pointer to the CM, used by the callbacks
extern class cSampleCM * gpCM;

/*=========================================================================*/
// Class cSampleCM
/*=========================================================================*/
class cSampleCM
{
   public:
      // Constructor
      cSampleCM()
         : mDeviceID(),
           mSessionState( 0 ),
           mSessionID( 0xFFFFFFFF ),
           mInitialState( 0xFFFFFFFF ),
           mStartTime( 0 ),
           mPreviousRX( 0 ),
           mPreviousTX( 0 ),
           mTotalRX( 0 ),
           mTotalTX( 0 ),
           mLifeTotalRX( 0 ),
           mLifeTotalTX( 0 ),
           mCurrentDuration( 0 ),
           mTotalDuration( 0 ),
           mLifeTotalDuration( 0 ),
           mbInitiatedStartDataSession( false ),
           mbInitiatedStopDataSession( false ),
           mDataBearerTech( eQMIDataBearerTechnologies_Unknown ),
           mDeviceDetectionThreadID( 0 ),
           mUpdateNetworkInfoThreadID( 0 )
      {
         mDeviceDetectionStopPipe[READING] = -1;
         mDeviceDetectionStopPipe[WRITING] = -1;

         // Create an ordered list of service preferences
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_CDMA1xEVDORevB );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_CDMA1xEVDORevA );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_CDMA1xEVDORev0 );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_CDMA ); 
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_DCHSDPAPlus );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_HSDPAPlus );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_HSDPA );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_WCDMA );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_EGPRS );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_GPRS );
         mPreferredServices.push_back( eQMINASDataServiceCapabilities2_GSM );
      }
   
      // Destructor
      ~cSampleCM();

      // Initialize UI, begin waiting for devices
      virtual bool Init();

      // Connect to a device and send initial callback registrations
      virtual ULONG Connect( LPCSTR pInterface );

      // Disconnect from a device and set members as "Unknown"
      virtual ULONG Disconnect();

      // Process a start data session request
      virtual ULONG OnStartDataSession( ULONG * pFailureCode );

      // Process a cancel data session request
      virtual ULONG OnCancelDataSession();

      // Process a stop data session request
      virtual ULONG OnStopDataSession();

      // Set mState
      virtual void SetState( const std::string & state )
      {
         mState = state;
      }

      // Set mRSSI
      virtual void SetRSSI( const std::string & rssi )
      {
         mRSSI = rssi;
      }

      // Set mTech
      virtual void SetTech( const std::string & tech )
      {
         mTech = tech;
      }

      // Set mRx
      virtual void SetRx( const std::string & rx )
      {
         mRx = rx;
      }

      // Set mTx
      virtual void SetTx( const std::string & tx )
      {
         mTx = tx;
      }

      // Set mMaxRx
      virtual void SetMaxRx( const std::string & maxRx )
      {
         mMaxRx = maxRx;
      }

      // Set mMaxTx
      virtual void SetMaxTx( const std::string & maxTx )
      {
         mMaxTx = maxTx;
      }

      // Set mRoam
      virtual void SetRoam( const std::string & roam )
      {
         mRoam = roam;
      }

      // Set mDuration
      virtual void SetDuration( const std::string & duration )
      {
         mDuration = duration;
      }

      // Set mLifeDuration
      virtual void SetLifeDuration( const std::string & lifeDuration )
      {
         mLifeDuration = lifeDuration;
      }

      // Set mLifeRx
      virtual void SetLifeRx( const std::string & lifeRx )
      {
         mLifeRx = lifeRx;
      }

      // Set mLifeTx
      virtual void SetLifeTx( const std::string & lifeTx )
      {
         mLifeTx = lifeTx;
      }

      // Set mManufact
      virtual void SetManufact( const std::string & manufact )
      {
         mManufact = manufact;
      }

      // Set mModel
      virtual void SetModel( const std::string & model )
      {
         mModel = model;
      }

      // Set mHardware
      virtual void SetHardware( const std::string & hardware )
      {
         mHardware = hardware;
      }

      // Set mFirmware
      virtual void SetFirmware( const std::string & firmware )
      {
         mFirmware = firmware;
      }

      // Set mMDN
      virtual void SetMDN( const std::string & mdn )
      {
         mMDN = mdn;
      }

      // Set mMIN
      virtual void SetMIN( const std::string & min )
      {
         mMIN = min;
      }

      // Set mESN
      virtual void SetESN( const std::string & esn )
      {
         mESN = esn;
      }

      // Set mMEID
      virtual void SetMEID( const std::string & meid )
      {
         mMEID = meid;
      }

      // Set mIMEI
      virtual void SetIMEI( const std::string & imei )
      {
         mIMEI = imei;
      }

      // Set mIMSI
      virtual void SetIMSI( const std::string & imsi )
      {
         mIMSI = imsi;
      }

      // Handle signal strength callback notification
      void OnSignalStrengthCBNotificaion(
         INT8        signalStr, 
         ULONG       radioInterface );

      // Handle session state callback notification
      void OnSessionStateCBNotification( ULONG state );

      // Handle data bearer callback notification
      void OnDataBearerCBNotification( ULONG dataBearerTech );

      // Handle data capabilities callback notification
      void OnDataCapsNotification( 
         ULONG                               numDataCaps, 
         eQMINASDataServiceCapabilities2 *   pDataCaps );

      // Handle byte totals callback notification
      void OnByteTotalsNotification( ULONGLONG rx, ULONGLONG tx );

      // Update the signal strength and technology
      void UpdateSignalAndTech();

      // Update the session state
      void UpdateSessionState( 
         bool                       bExternal,
         ULONG                      state );

      // Calculate and update the connection time being displayed
      void UpdateTimeDisplay();

      // Calculate and update the tx and rx rates being displayed
      void UpdateRateDisplay();

      // Check data bearer and duration, which are only available while
      // connected
      void CheckConnectedStats();

      // Update the device info stats
      void UpdateDeviceInfo();

      // Update the Connection Stats
      void UpdateConnectionInfo();

   protected:

      /* Class for interfacing with Gobi API */
      cGobiCMDLL mGobi;

      /* Connected device's ID */
      std::string mDeviceID;

      /* All the display elements */
      std::string mState;
      std::string mRSSI;
      std::string mTech;
      std::string mRx;
      std::string mTx;
      std::string mMaxRx;
      std::string mMaxTx;
      std::string mRoam;
      std::string mDuration;
      std::string mLifeDuration;
      std::string mLifeRx;
      std::string mLifeTx;
      std::string mManufact;
      std::string mModel;
      std::string mHardware;
      std::string mFirmware;
      std::string mMDN;
      std::string mMIN;
      std::string mESN;
      std::string mMEID;
      std::string mIMEI;
      std::string mIMSI;

      /* All the input elements */
      std::string mAPN;
      std::string mUsername;
      std::string mPassword;

      /* Session state */
      ULONG mSessionState;

      /* Data session ID */
      ULONG mSessionID;

      /* Initial state, used to determine if the connection 
         was initiated internally or externally */
      ULONG mInitialState;

      /* Preferred service order */
      std::list <ULONG> mPreferredServices;
      
      /* Stores the time that connection was started */
      ULONGLONG mStartTime;

      /* Stores the connection rates updated by callbacks */
      ULONGLONG mPreviousRX;
      ULONGLONG mPreviousTX;
      ULONGLONG mTotalRX;
      ULONGLONG mTotalTX;
      ULONGLONG mLifeTotalRX;
      ULONGLONG mLifeTotalTX;

      // Current and total durations
      ULONGLONG mCurrentDuration;
      ULONGLONG mTotalDuration;
      ULONGLONG mLifeTotalDuration;

      /* Did we initiate a start data session? */
      bool mbInitiatedStartDataSession;

      /* Did we initiate a stop data session? */
      bool mbInitiatedStopDataSession;

      /* Current signal map */
      std::map <ULONG, INT8> mServiceSignals;   

      /* Current data bearer technology */
      ULONG mDataBearerTech;

      /* Current data capabilities */
      std::list <ULONG> mDataCapabilities;

      /* Handle to the device detection thread */
      pthread_t mDeviceDetectionThreadID;
      int mDeviceDetectionStopPipe[2];

      // Device detection "thread"
      friend void * DeviceDetectionThread( void * pData );
      
      /* Handle to the UpdateNetworkInfo thread */
      pthread_t mUpdateNetworkInfoThreadID;
      cEvent mUpdateNetworkInfoEvent;      

      // Async Network info updater
      friend void * UpdateNetworkInfoThread( void * pData );
};
