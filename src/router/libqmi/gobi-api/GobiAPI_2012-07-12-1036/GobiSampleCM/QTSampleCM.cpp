/*===========================================================================
FILE:
   QTSampleCM.cpp

DESCRIPTION:
   QT implementation of the Sample CM
   
PUBLIC CLASSES AND METHODS:
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
// Include Files
//---------------------------------------------------------------------------
#include "QTSampleCM.h"

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   AsyncConnectThread (Free Method)

DESCRIPTION:
   Start a data session
  
PARAMETERS:
   pData  [ I ] - cQTSampleCM object

RETURN VALUE:
   void * - always NULL
===========================================================================*/ 
void * AsyncConnectThread( void * pData )
{
   cQTSampleCM * pCM = (cQTSampleCM*)pData;
   if (pCM == NULL)
   {
      return NULL;
   }

   // Open the dialog window, disable the info and connection stats buttons
   pCM->mView.rootContext()->setContextProperty( "dialogText",
                                                 "Connecting, please wait..." );
   pCM->mView.rootContext()->setContextProperty( "windowState",
                                                 "connectingDialog" );

   ULONG failureCode = 0xFFFFFFFF;
   ULONG rc = pCM->OnStartDataSession( &failureCode );
   if (rc != eGOBI_ERR_NONE)
   {
      std::ostringstream error;
      error << "Failed to connect, error " << rc;

      TRACE( "rc %lu, failure code %lu", rc, failureCode );

      // Show failure code, if present
      if (rc == 1014 && failureCode != 0xFFFFFFFF)
      {
         error << "\nCall failure reason " << failureCode;
      }

      pCM->mView.rootContext()->setContextProperty( "dialogText",
                                                     error.str().c_str() );

      pCM->SetConnectButtonText( "Connect" );
   }
   else
   {
      pCM->mView.rootContext()->setContextProperty( "dialogText", "Success!" );

      // Connect button should be updated by state change indication
   }

   // Leave the dialog up for 2s
   sleep( 2 );
         
   pCM->mView.rootContext()->setContextProperty( "windowState", "" );

   return NULL;
}

/*===========================================================================
METHOD:
   OnInfosButton (Free Method)

DESCRIPTION:
   Move to the info stats page
  
PARAMETERS:
   pCM  [ I ] - cQTSampleCM object

RETURN VALUE:
   QVariant - always 0
===========================================================================*/ 
QVariant OnInfosButton( cQTSampleCM * pCM )
{
   pCM->mView.rootContext()->setContextProperty( "windowState", "infos" );

   return 0;
}

/*===========================================================================
METHOD:
   OnConnectionsButton (Free Method)

DESCRIPTION:
   Move to the connection stats page
  
PARAMETERS:
   pCM  [ I ] - cQTSampleCM object

RETURN VALUE:
   QVariant - always 0
===========================================================================*/ 
QVariant OnConnectionsButton( cQTSampleCM * pCM )
{
   // "" is the default state (connection stats page)
   pCM->mView.rootContext()->setContextProperty( "windowState", "" );

   return 0;
}

/*===========================================================================
METHOD:
   OnConnectButton (Free Method)

DESCRIPTION:
   Start, cancel, or disconnect from a data session

   NOTE: The UI is not updated until this function returns, so the connection
         will be established asynchronously         
  
PARAMETERS:
   pCM  [ I ] - cQTSampleCM object

RETURN VALUE:
   QVariant - always 0
===========================================================================*/ 
QVariant OnConnectButton( cQTSampleCM * pCM )
{
   // Double check if there a device connected
   if (pCM->mDeviceID.size() == 0
   ||  pCM->mConnectButtonText.compare( "No Device" ) == 0)
   {
      TRACE( "No Device" );
      return 0;
   }

   // Start a connection
   if (pCM->mConnectButtonText.compare( "Connect" ) == 0)
   {
      pCM->SetConnectButtonText( "Cancel" );

      // Create a detached thread to start the connection asynchronously
      pthread_attr_t attributes;
      pthread_attr_init( &attributes );
      pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
      
      pthread_create( &pCM->mAsyncConnectThreadID,
                      &attributes, 
                      AsyncConnectThread, 
                      pCM );
   }
   else if (pCM->mConnectButtonText.compare( "Cancel" ) == 0)
   {
      pCM->OnCancelDataSession();

      pCM->SetConnectButtonText( "Connect" );
   }
   else if (pCM->mConnectButtonText.compare( "Disconnect" ) == 0)
   {
      pCM->OnStopDataSession();

      pCM->SetConnectButtonText( "Connect" );
   }
   else
   {
      // Externally connected, etc
      TRACE( "Unknown connect button state %s",
             pCM->mConnectButtonText.c_str() );
   }

   return 0;
}

/*=========================================================================*/
// cQTSampleCM Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   Init (Public Method)

DESCRIPTION:
   Initialize GUI

RETURN VALUE:
   bool
===========================================================================*/
bool cQTSampleCM::Init()
{
   // Use the current screen orientation
   mView.setOrientation( QmlApplicationViewer::ScreenOrientationAuto );

   // The buttons
   mView.rootContext()->setContextProperty( "connectButton",
                                            &mConnectButton );
   mView.rootContext()->setContextProperty( "infosButton",
                                            &mInfosButton );
   mView.rootContext()->setContextProperty( "connectionsButton",
                                            &mConnectionsButton );

   // The input fields
   mView.rootContext()->setContextProperty( "apnNameText", &mAPNText );
   mView.rootContext()->setContextProperty( "usernameText", &mUsernameText );
   mView.rootContext()->setContextProperty( "passwordText", &mPasswordText );

   // Default button value
   SetConnectButtonText( "No Device" );

   // Default state
   mView.rootContext()->setContextProperty( "windowState", "" );
   mView.rootContext()->setContextProperty( "dialogText", "" );

   bool bRC = cSampleCM::Init();

   mView.setMainQmlFile( "qml/GobiSampleCM/main.qml" );
   mView.show();

   return bRC;
}

/*===========================================================================
METHOD:
   Run (Public Method)

DESCRIPTION:
   Run the GUI (blocks until exit)

RETURN VALUE:
   bool
===========================================================================*/
int cQTSampleCM::Run()
{
   return mApp.exec();
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Calls GobiDisconnect

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cQTSampleCM::Disconnect()
{
   SetConnectButtonText( "No device" );

   return cSampleCM::Disconnect();
}

/*===========================================================================
ETHOD:
   OnStartDataSession (Public Method)

DESCRIPTION:
   Updates apn, username, and password input field values before starting
   a data session

PARAMETERS:
   pFailureCode   [ O ] - Call failure code, if provided

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cQTSampleCM::OnStartDataSession( ULONG * pFailureCode )
{
   // Grab the APN, username, and password
   mAPN = mAPNText.getText().toUtf8().constData();
   mUsername = mUsernameText.getText().toUtf8().constData();
   mPassword = mPasswordText.getText().toUtf8().constData();

   return cSampleCM::OnStartDataSession( pFailureCode );
}
