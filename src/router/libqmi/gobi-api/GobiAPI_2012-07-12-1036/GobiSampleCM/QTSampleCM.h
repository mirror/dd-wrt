/*===========================================================================
FILE:
   QTSampleCM.h

DESCRIPTION:
   QT implementation of the Sample CM
   
PUBLIC CLASSES AND METHODS:
   cButton
      Generic clickable button for QT
   cTextInput
      Generic text input field for QT
   cQTSampleCM
      QT implementation of the Sample CM

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
#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include <qvariant.h>
#include <qdeclarativecontext.h>

#include "SampleCM.h"

// Prototypes
class cQTSampleCM;
void * AsyncConnectThread( void * pData );
QVariant OnInfosButton( cQTSampleCM * pCM );
QVariant OnConnectionsButton( cQTSampleCM * pCM );
QVariant OnConnectButton( cQTSampleCM * pCM );

/*=========================================================================*/
// Class cButton
//    Generic clickable button for QT
/*=========================================================================*/
class cButton : public QObject
{
   Q_OBJECT

public:

   // Constructor
   cButton( cQTSampleCM * pCM,
            QVariant (*pOnClick)( cQTSampleCM * ) )
   {
      mpCM = pCM;
      mpOnClick = pOnClick;
   }

public slots:

   // Function to be run on a click event
   QVariant Click()
   {
      if (mpOnClick != 0)
      {
         return mpOnClick( mpCM );
      }

      return "";
   }

protected:

   /* The main object */
   cQTSampleCM * mpCM;

   /* Function to run when clicked */
   QVariant (* mpOnClick)( cQTSampleCM * );
};

/*=========================================================================*/
// cTextInput
//    Generic text input field for QT
/*=========================================================================*/
class cTextInput : public QObject
{
   Q_OBJECT
   Q_PROPERTY( QString text READ getText WRITE setText )

public slots:

   // Get the value
   QString getText() const
   {
      return mText;
   }

   // Set the value
   void setText( const QString & text )
   {
      mText = text;
   }

private:

   /* The text */
   QString mText;
};

/*=========================================================================*/
// Class cQTSampleCM
/*=========================================================================*/
class cQTSampleCM : public cSampleCM
{
   public:
      // Constructor
      cQTSampleCM( int argc, char ** argv )
         : mApp( argc, argv ),
           mConnectButtonText( "No Device" ),
           mConnectButton( this, OnConnectButton ),
           mInfosButton( this, OnInfosButton ),
           mConnectionsButton( this, OnConnectionsButton )
      { }

      // Initialize UI, begin waiting for devices
      bool Init();

      // Run the GUI (blocks until exit)
      int Run();

      // Disconnect
      ULONG Disconnect();

      // Process a start data session request
      ULONG OnStartDataSession( ULONG * pFailureCode );

      // Set mState and the connection button
      void SetState( const std::string & state )
      {
         cSampleCM::SetState( state );

         // Update the connection button as well
         switch (mSessionState)
         {
            case eQMIConnectionStatus_Disconnected:
            {
               SetConnectButtonText( "Connect" );

               if (mInitialState != eQMIConnectionStatus_Disconnected
               &&  mInitialState != eQMIConnectionStatus_Suspended)
               {
                  // Clear the initial state
                  mInitialState = eQMIConnectionStatus_Disconnected;
               }               
            }
            break;

            case eQMIConnectionStatus_Connected:
            {
               if (mInitialState != eQMIConnectionStatus_Disconnected
               &&  mInitialState != eQMIConnectionStatus_Suspended)
               {
                  SetConnectButtonText( "External Con" );
               }
               else
               {
                  SetConnectButtonText( "Disconnect" );
               }
            }
            break;

            case eQMIConnectionStatus_Authenticating:
            {
               if (mInitialState != eQMIConnectionStatus_Disconnected
               &&  mInitialState != eQMIConnectionStatus_Suspended)
               {
                  SetConnectButtonText( "Ext Connecting" );
               }
               else
               {
                  SetConnectButtonText( "Cancel" );
               }
            }
            break;

            case eQMIConnectionStatus_Suspended:
            default:
               break;
         }
            
         // No more than 12 characters
         if (mState.size() > 12)
         {
            mState.resize( 12 );
         }

         // Note: "state" is already a property, can't duplicate
         // using "status" instead
         mView.rootContext()->setContextProperty( "status", mState.c_str() );
      }

      // Set mRSSI
      void SetRSSI( const std::string & rssi )
      {
         cSampleCM::SetRSSI( rssi );

         mView.rootContext()->setContextProperty( "rssi", mRSSI.c_str() );
      }

      // Set mTech
      void SetTech( const std::string & tech )
      {
         cSampleCM::SetTech( tech );

         // No more than 12 characters
         if (mTech.size() > 12)
         {
            mTech.resize( 12 );
         }

         mView.rootContext()->setContextProperty( "tech", mTech.c_str() );
      }

      // Set mRx
      void SetRx( const std::string & rx )
      {
         cSampleCM::SetRx( rx );

         mView.rootContext()->setContextProperty( "rx", mRx.c_str() );
      }

      // Set mTx
      void SetTx( const std::string & tx )
      {
         cSampleCM::SetTx( tx );

         mView.rootContext()->setContextProperty( "tx", mTx.c_str() );
      }

      // Set mMaxRx
      void SetMaxRx( const std::string & maxRx )
      {
         cSampleCM::SetMaxRx( maxRx );

         mView.rootContext()->setContextProperty( "maxRx", mMaxRx.c_str() );
      }

      // Set mMaxTx
      void SetMaxTx( const std::string & maxTx )
      {
         cSampleCM::SetMaxTx( maxTx );

         mView.rootContext()->setContextProperty( "maxTx", mMaxTx.c_str() );
      }

      // Set mRoam
      void SetRoam( const std::string & roam )
      {
         cSampleCM::SetRoam( roam );

         mView.rootContext()->setContextProperty( "roam", mRoam.c_str() );
      }

      // Set mDuration
      void SetDuration( const std::string & duration )
      {
         cSampleCM::SetDuration( duration );

         mView.rootContext()->setContextProperty( "duration", mDuration.c_str() );
      }

      // Set mLifeDuration
      void SetLifeDuration( const std::string & lifeDuration )
      {
         cSampleCM::SetLifeDuration( lifeDuration );

         mView.rootContext()->setContextProperty( "lifeDuration", mLifeDuration.c_str() );
      }

      // Set mLifeRx
      void SetLifeRx( const std::string & lifeRx )
      {
         cSampleCM::SetLifeRx( lifeRx );

         mView.rootContext()->setContextProperty( "lifeRx", mLifeRx.c_str() );
      }

      // Set mLifeTx
      void SetLifeTx( const std::string & lifeTx )
      {
         cSampleCM::SetLifeTx( lifeTx );

         mView.rootContext()->setContextProperty( "lifeTx", mLifeTx.c_str() );
      }

      // Set mManufact
      void SetManufact( const std::string & manufact )
      {
         cSampleCM::SetManufact( manufact );

         mView.rootContext()->setContextProperty( "manufact", mManufact.c_str() );
      }

      // Set mModel
      void SetModel( const std::string & model )
      {
         cSampleCM::SetModel( model );

         // No more than 20 characters
         if (mModel.size() > 20)
         {
            mModel.resize( 20 );
         }

         mView.rootContext()->setContextProperty( "model", mModel.c_str() );
      }

      // Set mHardware
      void SetHardware( const std::string & hardware )
      {
         cSampleCM::SetHardware( hardware );

         mView.rootContext()->setContextProperty( "hardware", mHardware.c_str() );
      }

      // Set mFirmware
      void SetFirmware( const std::string & firmware )
      {
         cSampleCM::SetFirmware( firmware );

         // No more than 20 characters
         if (mFirmware.size() > 20)
         {
            mFirmware.resize( 20 );
         }

         mView.rootContext()->setContextProperty( "firmware", mFirmware.c_str() );
      }

      // Set mMDN
      void SetMDN( const std::string & mdn )
      {
         cSampleCM::SetMDN( mdn );

         mView.rootContext()->setContextProperty( "mdn", mMDN.c_str() );
      }

      // Set mMIN
      void SetMIN( const std::string & min )
      {
         cSampleCM::SetMIN( min );

         mView.rootContext()->setContextProperty( "min", mMIN.c_str() );
      }

      // Set mESN
      void SetESN( const std::string & esn )
      {
         cSampleCM::SetESN( esn );

         mView.rootContext()->setContextProperty( "esn", mESN.c_str() );
      }

      // Set mMEID
      void SetMEID( const std::string & meid )
      {
         cSampleCM::SetMEID( meid );

         mView.rootContext()->setContextProperty( "meid", mMEID.c_str() );
      }

      // Set mIMEI
      void SetIMEI( const std::string & imei )
      {
         cSampleCM::SetIMEI( imei );

         mView.rootContext()->setContextProperty( "imei", mIMEI.c_str() );
      }

      // Set mIMSI
      void SetIMSI( const std::string & imsi )
      {
         cSampleCM::SetIMSI( imsi );

         mView.rootContext()->setContextProperty( "imsi", mIMSI.c_str() );
      }

      // Set mConnectButtonText
      void SetConnectButtonText( const std::string & connectButtonText )
      {
         mConnectButtonText = connectButtonText;

         mView.rootContext()->setContextProperty( "connectButtonText",
                                                  connectButtonText.c_str() );
      }

   protected:

      /* QApplication object */
      QApplication mApp;

      /* QmlApplicationViewer object */
      QmlApplicationViewer mView;

      /* APN text input field */
      cTextInput mAPNText;

      /* Username text input field */
      cTextInput mUsernameText;

      /* Password text input field */
      cTextInput mPasswordText;

      /* "Connect" button's text */
      std::string mConnectButtonText;

      /* "Connect" button */
      cButton mConnectButton;

      /* "Info Stats" button */
      cButton mInfosButton;

      /* "Connection Stats" button */
      cButton mConnectionsButton;

      /* Async connection thread ID */
      pthread_t mAsyncConnectThreadID;

      // Friend functions
      friend void * AsyncConnectThread( void * pData );
      friend QVariant OnInfosButton( cQTSampleCM * pCM );
      friend QVariant OnConnectionsButton( cQTSampleCM * pCM );
      friend QVariant OnConnectButton( cQTSampleCM * pCM );
};
