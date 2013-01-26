/*===========================================================================
FILE:
   SampleCM.cpp

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
// Include Files
//---------------------------------------------------------------------------
#include "SampleCM.h"
#include "GobiCMDLL.h"
#include "GobiCMCallback.h"
#include "GobiConnectionMgmtAPIStructs.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <dirent.h>
#include <iomanip>

// Global pointer for callbacks to reference
class cSampleCM * gpCM;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetTickCount (Free Method)
   
DESCRIPTION:
   Provide a number for sequencing reference, similar to the windows
   ::GetTickCount().  
   
   NOTE: This number is based on the time since epoc, not 
   uptime.

PARAMETERS:

RETURN VALUE:
   ULONGLONG - Number of milliseconds system has been up
===========================================================================*/
ULONGLONG GetTickCount()
{
   timespec curtime;
   clock_gettime( CLOCK_REALTIME, &curtime );

   ULONGLONG outtime = curtime.tv_sec * 1000LL;
   outtime += curtime.tv_nsec / 1000000LL;
   
   return outtime;
}

/*===========================================================================
METHOD:
   DeviceDetectionThread (Free Method)

DESCRIPTION:
   Scans for and detects devices
  
PARAMETERS:
   pData  [ I ] - cSampleCM object

RETURN VALUE:
   void * - always NULL
===========================================================================*/ 
void * DeviceDetectionThread( void * pData )
{
   cSampleCM * pCM = (cSampleCM *)pData;
   if (pCM == NULL)
   {
      return NULL;
   }

   // Get inotify handle
   int inotify = inotify_init();
   if (inotify == -1)
   {
      TRACE( "inotify_init failed\n" );
      return NULL;
   }

   // Start a watch on the /dev directory
   int inotifyFlags = IN_CREATE | IN_MODIFY | IN_DELETE;
   int watchD = inotify_add_watch( inotify, "/dev/", inotifyFlags );
   if (watchD == -1)
   {
      TRACE( "inotify_add_watch failed\n" );
      close( inotify );
      return NULL;
   }

   // Does a device already exist?
   dirent ** ppDevFiles;
   
   // Yes, scandir really takes a triple pointer for its second param
   int numDevs = scandir( "/dev/", &ppDevFiles, NULL, NULL );
   for (int i = 0; i < numDevs; i++)
   {      
      std::string deviceID = "/dev/";
      deviceID += ppDevFiles[i]->d_name;
      free( ppDevFiles[i] );

      if (deviceID.find( "qcqmi" ) != std::string::npos)
      {
         pCM->Connect( deviceID.c_str() );
         break;
      }
   }

   // Cleanup from scandir
   if (numDevs != -1)
   {
      free( ppDevFiles );
   }
   else
   {
      TRACE( "Scandir failed\n" );
   }

   // Begin async reading
   fd_set inputSet, outputSet;
   FD_ZERO( &inputSet );
   FD_SET( pCM->mDeviceDetectionStopPipe[READING], &inputSet );
   FD_SET( inotify, &inputSet );
   int largestFD = std::max( pCM->mDeviceDetectionStopPipe[READING], inotify );

   while (true)
   {
      // No FD_COPY() available in android
      memcpy( &outputSet, &inputSet, sizeof( fd_set ) );

      // Wait for data on either the inotify or the stop pipe
      int status = select( largestFD + 1, &outputSet, NULL, NULL, NULL );
      if (status <= 0)
      {
         break;
      }
      else if (FD_ISSET( pCM->mDeviceDetectionStopPipe[READING], &outputSet ) == true)
      {
         // Time to close
         break;
      }
      else if (FD_ISSET( inotify, &outputSet ) == true)
      {
         // Perform a read
         BYTE buffer[1024] = { 0 };
         int rc = read( inotify, &buffer[0], 1024 );
         if (rc < (int)sizeof( inotify_event ))
         {
            continue;
         }

         // Typecast the buffer to an inotify event
         struct inotify_event * pIEvent = (struct inotify_event *)&buffer[0];
         
         // Check the length of the name
         if (pIEvent->len < 5)
         {
            continue;
         }

         // Is this a matching device?
         if (strncmp( pIEvent->name, "qcqmi", 5 ) != 0)
         {
            continue;
         }

         std::string deviceID = "/dev/";
         deviceID += pIEvent->name;

         if (pIEvent->mask & IN_CREATE
         ||  pIEvent->mask & IN_MODIFY)
         {
            pCM->Connect( deviceID.c_str() );
         }

         if (pIEvent->mask & IN_DELETE)
         {
            // Was it the connected device which was removed?
            if (pCM->mDeviceID.compare( deviceID ) == 0)
            {
               pCM->Disconnect();
            }
         }
      }
   }

   // Cleanup
   inotify_rm_watch( inotify, watchD );
   close( inotify );

   return NULL;
}

/*===========================================================================
METHOD:
   UpdateNetworkInfoThread (Free Method)

DESCRIPTION:
   Updates the network stats every 1s (while device is present)
  
PARAMETERS:
   pData  [ I ] - cSampleCM object

RETURN VALUE:
   void * - always NULL
===========================================================================*/ 
void * UpdateNetworkInfoThread( void * pData )
{
   cSampleCM * pCM = (cSampleCM*)pData;
   if (pCM == NULL)
   {
      return NULL;
   }

   // Update once
   pCM->CheckConnectedStats();

   int rc;
   do
   {
      // Update rates and times every 1s
      pCM->UpdateRateDisplay();
      pCM->UpdateTimeDisplay();

      DWORD temp;
      rc = pCM->mUpdateNetworkInfoEvent.Wait( 1000, temp );

   } while (rc == ETIME);

   return NULL;
}

/*===========================================================================
METHOD:
   ParseStats (Free Method)

DESCRIPTION:
   Parse the stats file and obtain the TX/RX bytes and connection duration
 
   NOTE: The stats text file consists of one line in the following format:
      Total RX Bytes; Total TX Bytes; Total Duration

PARAMETERS:
   line               [ I ] - Line to parse
   lifeTotalRX        [ O ] - Total RX bytes   
   lifeTotalTX        [ O ] - Total TX bytes   
   lifeTotalDuration  [ O ] - Total connection duration

RETURN VALUE:
   bool
===========================================================================*/
bool ParseStats( 
   std::istringstream &          line,
   ULONGLONG &                   lifeTotalRX,
   ULONGLONG &                   lifeTotalTX,
   ULONGLONG &                   lifeTotalDuration )
{

   ULONGLONG temp1, temp2, temp3;
   char c1, c2;

   // Attempt to parse into temp variables, skipping whitespace
   line >> std::skipws >> temp1 >> c1 >> temp2 >> c2 >> temp3;
   
   // Was parsing successful?
   if (line.fail() == true)
   {
      TRACE( "failed to parse stats file\n" );
      return false;
   }
   else
   {
      TRACE( "read %llu, %llu, %llu", temp1, temp2, temp3 );
      lifeTotalRX = temp1;
      lifeTotalTX = temp2;
      lifeTotalDuration = temp3;

      return true;
   }
}

/*=========================================================================*/
// cSampleCM Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   ~cSampleCM (Public Method)

DESCRIPTION:
   Destructor - Stops the data session, 
                Stops device detection thread,
                Writes stats to file
  
RETURN VALUE:
   None
===========================================================================*/ 
cSampleCM::~cSampleCM()
{
   // These functions will most likely fail.  That's ok.
   OnCancelDataSession();
   OnStopDataSession();

   Disconnect();

   // Close device detection thread, if open
   if (mDeviceDetectionStopPipe[WRITING] != -1)
   {
      if (mDeviceDetectionThreadID != 0)
      {
         BYTE byte = 1;
         write( mDeviceDetectionStopPipe[WRITING], &byte, 1 );

         pthread_join( mDeviceDetectionThreadID, NULL );

         mDeviceDetectionThreadID = 0;
      }

      close( mDeviceDetectionStopPipe[WRITING] );
      close( mDeviceDetectionStopPipe[READING] );
      mDeviceDetectionStopPipe[READING] = -1;
      mDeviceDetectionStopPipe[WRITING] = -1;
   }

   // Write stats to file
   std::string config = getenv( "HOME" );
   config += "/.GobiSampleCMStats.txt";

   int flags = O_CREAT | O_TRUNC | O_WRONLY;
   mode_t mode = S_IRUSR | S_IWUSR;
   int statsFile = open( config.c_str(), flags, mode );
   if (statsFile < 0)
   {
      TRACE( "Unable to create stats file\n" );
   }
   else
   {
      std::ostringstream out;
      out << mLifeTotalRX << "; "
          << mLifeTotalTX << "; "
          << mLifeTotalDuration;

      int rc = write( statsFile, out.str().c_str(), out.str().size() );
      if (rc < 0)
      {
         TRACE( "Unable to write stats to file\n" );
      }

      close( statsFile );
   }
}

/*===========================================================================
METHOD:
   Init (Public Method)

DESCRIPTION:
   Initialize GUI, begin waiting for devices

RETURN VALUE:
   bool
===========================================================================*/
bool cSampleCM::Init()
{
   Disconnect();

   // Read in the stats file
   std::string config = getenv( "HOME" );
   config += "/.GobiSampleCMStats.txt";

   int statsFile = open( config.c_str(), O_RDONLY );
   if (statsFile < 0)
   {
      // Non-fatal error
      TRACE( "Unable to open config file %s", config.c_str() );
   }
   else
   {
      CHAR buf[100];
      int len = read( statsFile, &buf[0], 100 );
      if (len < 0)
      {
         // Non-fatal error
         TRACE( "failed to read from file\n" );
      }
      else
      {
         std::string asString( &buf[0], len );
         std::istringstream line( asString );

         ULONGLONG lrx, ltx, ld;
         if (ParseStats( line, lrx, ltx, ld ) == true)
         {
            mLifeTotalRX = lrx;
            mLifeTotalTX = ltx;
            mLifeTotalDuration = ld;
         }
      }

      close( statsFile );
   }

   // Life totals will just be zeros if the file was not present
   std::ostringstream tmp;
   tmp << mLifeTotalRX;
   SetLifeRx( tmp.str() );

   tmp.str( "" );
   tmp << mLifeTotalTX;
   SetLifeTx( tmp.str() );

   tmp.str( "" );
   tmp << std::setfill( '0' ) << std::setw( 2 ) << (mLifeTotalDuration / 3600) % 60
       << std::setw( 1 ) << ":"
       << std::setfill( '0' ) << std::setw( 2 ) << (mLifeTotalDuration / 60) % 60
       << std::setw( 1 ) << ":"
       << std::setfill( '0' ) << std::setw( 2 ) << mLifeTotalDuration % 60;
   SetLifeDuration( tmp.str() );

   // Set the global pointer, used by callbacks
   gpCM = this;

   // Initialize command pipe
   int ret = pipe( mDeviceDetectionStopPipe );
   if (ret != 0)
   {
      // Should never happen, but just in case...
      return false;
   }

   // Begin scanning for devices
   ret = pthread_create( &mDeviceDetectionThreadID,
                         0,
                         DeviceDetectionThread,
                         this );
   if (ret != 0)
   {
      return false;
   }

   return true;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Initializes the Gobi API to the current device

PARAMETERS:
   pInterface  [ I ] - Interace to connect to

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cSampleCM::Connect( LPCSTR pInterface )
{
   // Are we already connected to a device?
   if (mDeviceID.size() != 0)
   {
      return eGOBI_ERR_MULTIPLE_DEVICES;
   }

   // Connect to the device
   ULONG rc = mGobi.Connect( pInterface );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "GobiConnect error %lu\n", rc );
      return rc;
   }

   UpdateDeviceInfo();
   UpdateConnectionInfo();

   // Any connection at this point is an external connection
   UpdateSessionState( true, mSessionState );

   // Success
   mDeviceID = pInterface;
   return rc;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Calls GobiDisconnect

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cSampleCM::Disconnect()
{
   // Stop a data session, if present
   OnCancelDataSession();
   OnStopDataSession();

   mGobi.Disconnect();

   // Reset state
   mSessionID = 0xFFFFFFFF;
   mInitialState = 0xFFFFFFFF;
   mDataBearerTech = 0;

   // Store life total
   mLifeTotalDuration += mTotalDuration;

   // Reset all totals
   mStartTime = 0;
   mPreviousRX = 0;
   mPreviousTX = 0;
   mTotalRX = 0;
   mTotalTX = 0;
   mTotalDuration = 0;

   SetState( "No Device" );
   SetRSSI( "Unknown" );
   SetTech( "Unknown" );
   SetRx( "Unknown" );
   SetTx( "Unknown" );
   SetMaxRx( "Unknown" );
   SetMaxTx( "Unknown" );
   SetRoam( "Unknown" );
   SetDuration( "Unknown" );
   SetLifeDuration( "Unknown" );
   SetLifeRx( "Unknown" );
   SetLifeTx( "Unknown" );
   SetManufact( "Unknown" );
   SetModel( "Unknown" );
   SetHardware( "Unknown" );
   SetFirmware( "Unknown" );
   SetMDN( "Unknown" );
   SetMIN( "Unknown" );
   SetESN( "Unknown" );
   SetMEID( "Unknown" );
   SetIMEI( "Unknown" );
   SetIMSI( "Unknown" );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
ETHOD:
   OnStartDataSession (Public Method)

DESCRIPTION:
   Starts a data session

PARAMETERS:
   pFailureCode   [ O ] - Call failure code, if provided

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cSampleCM::OnStartDataSession( ULONG * pFailureCode )
{
   mbInitiatedStartDataSession = true;

   // Use provided values, if not empty strings
   LPCSTR pAPN = (mAPN.size() == 0) ? 0 : mAPN.c_str();
   LPCSTR pUsername = (mUsername.size() == 0) ? 0 : mUsername.c_str();
   LPCSTR pPassword = (mPassword.size() == 0) ? 0 : mPassword.c_str();

   ULONG rc = mGobi.StartDataSession( pAPN,
                                      pUsername,
                                      pPassword,
                                      &mSessionID,
                                      pFailureCode );
   if (rc == eGOBI_ERR_NONE)
   {
      mPreviousRX = 0;
      mPreviousTX = 0;
      mTotalRX = 0;
      mTotalTX = 0;
      mTotalDuration = 0;
      mStartTime = GetTickCount();
   }
   else
   {
      mbInitiatedStartDataSession = false;
   }

   return rc;
}

/*===========================================================================
METHOD:
   CancelDataSession (Public Method)

DESCRIPTION:
   Cancels an in progress data session request

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cSampleCM::OnCancelDataSession()
{
   return mGobi.CancelDataSession();
}

/*===========================================================================
METHOD:
   OnStopDataSession (Public Method)

DESCRIPTION:
   Calls WDSStopNetworkInterface

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG cSampleCM::OnStopDataSession()
{
   if (mSessionID == 0xFFFFFFFF)
   {
      return eGOBI_ERR_GENERAL;
   }

   mbInitiatedStopDataSession = true;
   mDataBearerTech = 0;

   ULONG rc = mGobi.StopDataSession( mSessionID );
   mSessionID = 0xFFFFFFFF;

   return rc;
}

/*===========================================================================
METHOD:
   OnSignalStrengthCBNotification (Internal Method)

DESCRIPTION:
   Handle signal strength callback notification
 
PARAMETERS:
   signalStr        [ I ] - Signal strength
   radioInterface   [ I ] - Radio interface
===========================================================================*/
void cSampleCM::OnSignalStrengthCBNotificaion( 
   INT8        signalStr, 
   ULONG       radioInterface )
{
   // Update mServiceSignals
   mServiceSignals[radioInterface] = signalStr;

   if (mSessionState == eQMIConnectionStatus_Connected)
   {
      mDataBearerTech = 0;
   }

   UpdateSignalAndTech();
}

/*===========================================================================
METHOD:
   OnSessionStateCBNotification (Internal Method)

DESCRIPTION:
   Handle session state callback notification
 
PARAMETERS:
   state       [ I ] - Session state
===========================================================================*/
void cSampleCM::OnSessionStateCBNotification( ULONG state )
{
   // Was the state change triggered externally?
   bool bStateChangeExternal = false;

   // Save session state
   if (state == eQMIConnectionStatus_Connected)
   {
      // Started externally 
      if (mbInitiatedStartDataSession == false)
      {
         bStateChangeExternal = true;
         mStartTime = GetTickCount();
      }
   }
   else
   {
      mDataBearerTech = 0;

      // Stopped externally
      if (mbInitiatedStopDataSession == false)
      {
         // Store all life total variables
         mLifeTotalDuration += mTotalDuration;

         // Reset all totals
         bStateChangeExternal = true;
         mStartTime = 0;
         mPreviousRX = 0;
         mPreviousTX = 0;
         mTotalRX = 0;
         mTotalTX = 0;
         mTotalDuration = 0;
      }
   }

   // Update the session state
   UpdateSessionState( bStateChangeExternal, state );

   // Update the signal strength and technology
   UpdateSignalAndTech();

   // Reset to false
   mbInitiatedStartDataSession = false;
   mbInitiatedStopDataSession = false;
}

/*===========================================================================
METHOD:
   OnDataBearerCBNotification (Internal Method)

DESCRIPTION:
   Handle data bearer callback notification
 
PARAMETERS:
   dataBearerTech  [ I ] - Data bearer technology
===========================================================================*/
void cSampleCM::OnDataBearerCBNotification( ULONG dataBearerTech )
{
   if (dataBearerTech != eQMIDataBearerTechnologies_Unknown)
   {
      mDataBearerTech = dataBearerTech;
      UpdateSignalAndTech();
   }
}

/*===========================================================================
METHOD:
   OnDataCapsNotification (Internal Method)

DESCRIPTION:
   Handle data capabilities callback notification
 
PARAMETERS:
   numDataCaps     [ I ] - Number of data capabilities
   pDataCaps       [ I ] - Data Capabilites
===========================================================================*/
void cSampleCM::OnDataCapsNotification( 
   ULONG                               numDataCaps, 
   eQMINASDataServiceCapabilities2 *   pDataCaps )
{
   // Clear saved data capabilities in order to update
   mDataCapabilities.clear();

   // Populate list with new capabilities
   for (ULONG c = 0; c < numDataCaps; c++)
   {
      ULONG dataCaps = pDataCaps[c];
      if ( (dataCaps != 0xFFFFFFFF)
      &&   (dataCaps != 0) )
      {
         mDataCapabilities.push_back( pDataCaps[c] );
      }
   }

   UpdateSignalAndTech();
}

/*===========================================================================
METHOD:
   OnByteTotalsNotification (Internal Method)

DESCRIPTION:
   Handle byte totals callback notification
 
PARAMETERS:
   rx       [ I ] - received bytes
   tx       [ I ] - transmitted bytes
===========================================================================*/
void cSampleCM::OnByteTotalsNotification( ULONGLONG rx, ULONGLONG tx )
{
   mTotalTX = tx;
   mTotalRX = rx;
}

/*===========================================================================
METHOD:
   UpdateSignalAndTech (Public Method)

DESCRIPTION:
   Update the signal strength and technology display
===========================================================================*/ 
void cSampleCM::UpdateSignalAndTech()
{
   std::string radioStr = "Unknown";
   std::string ssStr = "Unknown";
   ULONG radioVal = eQMINASRadioInterfaces_NoneNoService;

   // If connected, use data bearer   
   if (mDataBearerTech != 0)
   {
      switch (mDataBearerTech)
      {
         case eQMIDataBearerTechnologies_CDMA20001x:
            radioStr = "CDMA 1xRTT";
            radioVal = eQMINASRadioInterfaces_CDMA20001x;
            break;

         case eQMIDataBearerTechnologies_CDMA20001xEVDORev0:
            radioStr = "CDMA 1xEVDO Rev 0";
            radioVal = eQMINASRadioInterfaces_CDMA2000HRPD;
            break;

         case eQMIDataBearerTechnologies_GPRS:
            radioStr = "GRPS";
            radioVal = eQMINASRadioInterfaces_GSM;
            break;

         case eQMIDataBearerTechnologies_WCDMA:
            radioStr = "WCDMA";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_CDMA20001xEVDORevA:
            radioStr = "CDMA 1xEVDO Rev A";
            radioVal = eQMINASRadioInterfaces_CDMA2000HRPD;
            break;

         case eQMIDataBearerTechnologies_EGPRS:
            radioStr = "EDGE";
            radioVal = eQMINASRadioInterfaces_GSM;
            break;

         case eQMIDataBearerTechnologies_HSDPAWCDMA:
            radioStr = "HSDPA DL, WCDMA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_WCDMAHSUPA:
            radioStr = "WCDMA DL, HSUPA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_HSDPAHSUPA:
            radioStr = "HSDPA DL, HSUPA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_LTE:
            radioStr = "LTE";
            radioVal = eQMINASRadioInterfaces_LTE;
            break;

         case eQMIDataBearerTechnologies_CDMA2000EHRPD:
            radioStr = "CDMA 1xEVDO eHRPD";
            radioVal = eQMINASRadioInterfaces_CDMA2000HRPD;
            break;

         case eQMIDataBearerTechnologies_HSDPAPlusWCDMA:
            radioStr = "HSDPA+ DL, WCDMA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_HSDPAPlusHSUPA:
            radioStr = "HSDPA+ DL, HSUPA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_DualCellHSDPAPlusWCDMA:
            radioStr = "Dual Cell HSDPA+ DL, WCDMA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;

         case eQMIDataBearerTechnologies_DualCellHSDPAPlusHSUPA:
            radioStr = "Dual Cell HSDPA+ DL, HSUPA UL";
            radioVal = eQMINASRadioInterfaces_UMTS;
            break;
      }

      if ( (radioVal != eQMINASRadioInterfaces_NoneNoService)
      &&   (mServiceSignals.find( radioVal ) != mServiceSignals.end())
      &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
      {
         std::ostringstream temp;
         temp << (INT)mServiceSignals[radioVal];
         ssStr = temp.str();
      }
   }
   else
   {
      // When not connected we have to pick the most preferred data capability
      eQMINASDataServiceCapabilities2 mostPreferredCap 
         = (eQMINASDataServiceCapabilities2)0xff;
      std::list <ULONG>::const_iterator pIter;
      std::list <ULONG>::const_iterator pDataCapsItr;

      pIter = mPreferredServices.begin();

      bool bDone = false;
      while ( (pIter != mPreferredServices.end())
      &&      (bDone != true) )
      {
         pDataCapsItr = mDataCapabilities.begin();
         while (pDataCapsItr != mDataCapabilities.end())
         {
            if (*pIter == *pDataCapsItr)
            {
               mostPreferredCap = (eQMINASDataServiceCapabilities2)*pIter;
               bDone = true;
               break;
            }

            pDataCapsItr++;
         }

         pIter++;
      }

      // Determine the best radio interface reported
      eQMINASRadioInterfaces preferredRadioIf;
      preferredRadioIf = eQMINASRadioInterfaces_NoneNoService;

      if (mServiceSignals.find( eQMINASRadioInterfaces_LTE ) != mServiceSignals.end())
      {
         preferredRadioIf = eQMINASRadioInterfaces_LTE;
      }
      else if (mServiceSignals.find( eQMINASRadioInterfaces_CDMA2000HRPD ) != mServiceSignals.end())
      {
         preferredRadioIf = eQMINASRadioInterfaces_CDMA2000HRPD;
      }
      else if (mServiceSignals.find( eQMINASRadioInterfaces_CDMA20001x ) != mServiceSignals.end())
      {
         preferredRadioIf = eQMINASRadioInterfaces_CDMA20001x;
      }
      else if (mServiceSignals.find( eQMINASRadioInterfaces_UMTS ) != mServiceSignals.end())
      {
         preferredRadioIf = eQMINASRadioInterfaces_UMTS;
      }
      else if (mServiceSignals.find( eQMINASRadioInterfaces_GSM ) != mServiceSignals.end())
      {
         preferredRadioIf = eQMINASRadioInterfaces_GSM;
      }

      radioStr = "Unknown";
      ssStr = "Unknown";
      radioVal = eQMINASRadioInterfaces_NoneNoService;

      // Determine sig strength and radio interface to display based on 
      // most preferred data capabilities
      switch (preferredRadioIf)
      {
         case eQMINASRadioInterfaces_CDMA2000HRPD:
            radioStr = "CDMA 1xEVDO";
            if ( (mostPreferredCap == eQMINASDataServiceCapabilities2_CDMA1xEVDORevB) 
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_CDMA1xEVDORevA) 
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_CDMA1xEVDORev0) )
            {
               radioVal = eQMINASRadioInterfaces_CDMA2000HRPD;
               if ( (mServiceSignals.find( radioVal ) != mServiceSignals.end())
               &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
               {
                  std::ostringstream temp;
                  temp << (INT)mServiceSignals[radioVal];
                  ssStr = temp.str();
               }
            }
            break;

         case eQMINASRadioInterfaces_CDMA20001x:
            radioStr = "CDMA 1xRTT";
            if (mostPreferredCap == eQMINASDataServiceCapabilities2_CDMA)
            {
               radioVal = eQMINASRadioInterfaces_CDMA20001x;
               if ( (mServiceSignals.find( radioVal ) != mServiceSignals.end())
               &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
               {
                  std::ostringstream temp;
                  temp << (INT)mServiceSignals[radioVal];
                  ssStr = temp.str();
               }
            }
            break;

         case eQMINASRadioInterfaces_UMTS:
            radioStr = "WCDMA";
            if ( (mostPreferredCap == eQMINASDataServiceCapabilities2_DCHSDPAPlus)
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_HSDPAPlus)
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_HSDPA) 
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_HSUPA)
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_WCDMA) )
            {
               radioVal = eQMINASRadioInterfaces_UMTS;
               if ( (mServiceSignals.find( radioVal ) != mServiceSignals.end())
               &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
               {
                  std::ostringstream temp;
                  temp << (INT)mServiceSignals[radioVal];
                  ssStr = temp.str();
               }
            }
            break;

         case eQMINASRadioInterfaces_GSM:
            radioStr = "GSM";
            if ( (mostPreferredCap == eQMINASDataServiceCapabilities2_GPRS) 
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_EGPRS)
            ||   (mostPreferredCap == eQMINASDataServiceCapabilities2_GSM) )
            {
               radioVal = eQMINASRadioInterfaces_GSM;
               if ( (mServiceSignals.find( radioVal ) != mServiceSignals.end())
               &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
               {
                  std::ostringstream temp;
                  temp << (INT)mServiceSignals[radioVal];
                  ssStr = temp.str();
               }
            }
            break;

         case eQMINASRadioInterfaces_LTE:
            radioStr = "LTE";
            if (mostPreferredCap == eQMINASDataServiceCapabilities2_LTE)
            {
               radioVal = eQMINASRadioInterfaces_LTE;
               if ( (mServiceSignals.find( radioVal ) != mServiceSignals.end())
               &&   (mServiceSignals[radioVal] != SCHAR_MAX) )
               {
                  std::ostringstream temp;
                  temp << (INT)mServiceSignals[radioVal];
                  ssStr = temp.str();
               }
            }
            break;

         case eQMINASRadioInterfaces_NoneNoService:
            if ( (mServiceSignals.find( eQMINASRadioInterfaces_CDMA2000HRPD ) != mServiceSignals.end())
            &&   (mServiceSignals[eQMINASRadioInterfaces_CDMA2000HRPD] != SCHAR_MAX) )
            {
               radioStr = "CDMA 1xEVDO";
               std::ostringstream temp;
               temp << (INT)mServiceSignals[eQMINASRadioInterfaces_CDMA2000HRPD];
               ssStr = temp.str();
            }
            else if ( (mServiceSignals.find( eQMINASRadioInterfaces_CDMA20001x ) != mServiceSignals.end())
            &&   (mServiceSignals[eQMINASRadioInterfaces_CDMA20001x] != SCHAR_MAX) )
            {
               radioStr = "CDMA 1xRTT";
               std::ostringstream temp;
               temp << (INT)mServiceSignals[eQMINASRadioInterfaces_CDMA20001x];
               ssStr = temp.str();
            }
            else if ( (mServiceSignals.find( eQMINASRadioInterfaces_UMTS ) != mServiceSignals.end())
            &&   (mServiceSignals[eQMINASRadioInterfaces_UMTS] != SCHAR_MAX) )
            {
               radioStr = "WCDMA";
               std::ostringstream temp;
               temp << (INT)mServiceSignals[eQMINASRadioInterfaces_UMTS];
               ssStr = temp.str();
            }
            else if ( (mServiceSignals.find( eQMINASRadioInterfaces_GSM ) != mServiceSignals.end())
            &&   (mServiceSignals[eQMINASRadioInterfaces_GSM] != SCHAR_MAX) )
            {
               radioStr = "GSM";
               std::ostringstream temp;
               temp << (INT)mServiceSignals[eQMINASRadioInterfaces_GSM];
               ssStr = temp.str();
            }
            else if ( (mServiceSignals.find( eQMINASRadioInterfaces_LTE ) != mServiceSignals.end())
            &&   (mServiceSignals[eQMINASRadioInterfaces_LTE] != SCHAR_MAX) )
            {
               radioStr = "LTE";
               std::ostringstream temp;
               temp << (INT)mServiceSignals[eQMINASRadioInterfaces_LTE];
               ssStr = temp.str();
            }
            break;

         default:
            radioStr = "Unknown";
            ssStr = "Unknown";
      }
   }

   SetRSSI( ssStr );
   SetTech( radioStr );
}

/*===========================================================================
METHOD:
   UpdateSessionState (Public Method)

DESCRIPTION:
   Update the session state display
   Start/stop UpdateNetworkInfo thread

PARAMETERS:
   bExternal         [ I ] - Was the state change triggered externally
   state             [ I ] - State of session

RETURN VALUE:
   None
===========================================================================*/ 
void cSampleCM::UpdateSessionState(
   bool                       bExternal,
   ULONG                      state )
{
   mSessionState = state;

   if (bExternal == true)
   {
      mInitialState = state;
   }

   LPCSTR pState = "Unknown";
   switch (state)
   {
      // Disconnected
      case 1:
      {
         pState = "Disconnected";

         // Stop the network info thread, if running
         if (mUpdateNetworkInfoThreadID != 0)
         {
            mUpdateNetworkInfoEvent.Set( 0 );

            pthread_join( mUpdateNetworkInfoThreadID, NULL );
            mUpdateNetworkInfoThreadID = 0;

            mLifeTotalDuration += mTotalDuration;
         }

         // Reset all totals
         mPreviousRX = 0;
         mPreviousTX = 0;
         mTotalRX = 0;
         mTotalTX = 0;
         mTotalDuration = 0;
         mStartTime = 0;

         UpdateRateDisplay();
         UpdateTimeDisplay();
      }
      break;

      // Connected
      case 2:
      {
         if (bExternal == true)
         {
            pState = "External Con";
         }
         else
         {
            pState = "Connected";
         }

         // Start the network info thread, if not running
         if (mUpdateNetworkInfoThreadID == 0)
         {
            // Begin updating network info
            int rc = pthread_create( &mUpdateNetworkInfoThreadID,
                                     0,
                                     UpdateNetworkInfoThread,
                                     this );
            if (rc != 0)
            {
               TRACE( "error starting network info thread\n" );
            }
         }
      }
      break;

      // Suspended
      case 3:
         pState = "Suspended";
         break;

      // Connecting
      case 4:
         if (bExternal == true)
         {
            pState = "Ext Connecting";
         }
         else
         {
            pState = "Connecting";
         }
         break;
   }

   SetState( pState );
}

/*===========================================================================
METHOD:
   UpdateRateDisplay (Public Method)

DESCRIPTION:
   Calculate and update the tx, rx rates being displayed

PARAMETERS:
   None

RETURN VALUE:
   None
===========================================================================*/ 
void cSampleCM::UpdateRateDisplay()
{
   // Update TX/RX Bytes
   ULONGLONG deltaRX = 0;
   ULONGLONG deltaTX = 0;

   // Only update rates if connected
   if (mSessionState == eQMIConnectionStatus_Connected)
   {
      // First time through, don't use the deltas
      if (mTotalRX != 0)
      {
         deltaRX = mTotalRX - mPreviousRX;
      }
         
      if (mTotalTX != 0)
      {
         deltaTX = mTotalTX - mPreviousTX;
      }

      // Update life total byte variables
      mLifeTotalRX += deltaRX;
      mLifeTotalTX += deltaTX;

      mPreviousRX = mTotalRX;
      mPreviousTX = mTotalTX;
   }

   std::ostringstream tmp;
   tmp << deltaRX;
   SetRx( tmp.str() );

   tmp.str( "" );
   tmp << deltaTX;
   SetTx( tmp.str() );

   tmp.str( "" );
   tmp << mLifeTotalRX;
   SetLifeRx( tmp.str() );

   tmp.str( "" );
   tmp << mLifeTotalTX;
   SetLifeTx( tmp.str() );
}

/*===========================================================================
METHOD:
   UpdateTimeDisplay (Public Method)

DESCRIPTION:
   Calculate and update the connection time being displayed

PARAMETERS:
   None

RETURN VALUE:
   None
===========================================================================*/ 
void cSampleCM::UpdateTimeDisplay()
{
   DWORD elapsedTime = 0;
   DWORD lifeTotalTime = 0;

   // Update session duration
   if ( (mSessionState == eQMIConnectionStatus_Connected)
   &&   (mStartTime != 0) )
   {
      // Convert ms to seconds
      mTotalDuration = (GetTickCount() - mStartTime) / 1000;
      elapsedTime = (DWORD)mTotalDuration;
      lifeTotalTime = (DWORD)(mLifeTotalDuration + mTotalDuration);
   }
   else
   {
      elapsedTime = 0;
      lifeTotalTime = (DWORD)mLifeTotalDuration;
   }

   std::ostringstream timeStr;

   // Format both into hh:mm:ss
   // "%02d:%02d:%02d"
   timeStr << std::setfill( '0' ) << std::setw( 2 ) << (elapsedTime / 3600) % 60
           << std::setw( 1 ) << ":"
           << std::setfill( '0' ) << std::setw( 2 ) << (elapsedTime / 60) % 60
           << std::setw( 1 ) << ":"
           << std::setfill( '0' ) << std::setw( 2 ) << elapsedTime % 60;
   SetDuration( timeStr.str() );

   timeStr.str( "" );
   timeStr << std::setfill( '0' ) << std::setw( 2 ) << (lifeTotalTime / 3600) % 60
           << std::setw( 1 ) << ":"
           << std::setfill( '0' ) << std::setw( 2 ) << (lifeTotalTime / 60) % 60
           << std::setw( 1 ) << ":"
           << std::setfill( '0' ) << std::setw( 2 ) << lifeTotalTime % 60;
   SetLifeDuration( timeStr.str() );
}

/*===========================================================================
METHOD:
   CheckConnectedStats (Public Method)

DESCRIPTION:
   Check stats which are only valid during a connection
===========================================================================*/
void cSampleCM::CheckConnectedStats()
{
   ULONG rc = mGobi.GetDataBearerTechnology( &mDataBearerTech );
   if (rc != eGOBI_ERR_NONE || mDataBearerTech == ULONG_MAX)
   {
      TRACE( "GetDataBearerTechnology error %lu\n", rc );
      return;
   }

   ULONGLONG duration;
   rc = mGobi.GetSessionDuration( &duration );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "GetSessionDuration error %lu\n", rc );
      return;
   }

   mStartTime = GetTickCount() - duration;

   UpdateSignalAndTech();
}

/*===========================================================================
METHOD:
   UpdateDeviceInfo (Public Method)

DESCRIPTION:
   Update the device info stats
===========================================================================*/
void cSampleCM::UpdateDeviceInfo()
{
   ULONG status = 0;

   BYTE strSz1 = 255;
   BYTE strSz2 = 255;
   BYTE strSz3 = 255;
   CHAR str1[255];
   CHAR str2[255];
   CHAR str3[255];
   str1[0] = 0;
   str2[0] = 0;
   str3[0] = 0;

   // Get manufacturer
   status = mGobi.GetManufacturer( strSz1, str1 );
   if (status != 0)
   {
      TRACE( "GetManufacturer() = %lu\n", status );
      return;
   }
     
   if (str1[0] != 0)
   {
      SetManufact( &str1[0] );
   }
   
   // Get model ID
   str1[0] = 0;
   status = mGobi.GetModelID( strSz1, str1 );
   if (status != 0)
   {
      TRACE( "GetModelID() = %lu\n", status );
      return;
   }

   if (str1[0] != 0)
   {
      std::ostringstream tmp;
      if (strncmp( "88", &str1[0], 2 ) == 0)
      {
         tmp << "Gobi MDM-1000";
      }
      else if (strncmp( "12", &str1[0], 2 ) == 0)
      {
         tmp << "Gobi MDM-2000";
      }
      else
      {
         tmp << "Unknown (" << &str1[0] << ")";
      }

      SetModel( tmp.str() );
   }

   // Get firmware revision
   str1[0] = 0;
   status = mGobi.GetFirmwareRevision( strSz1, str1 );
   if (status != 0)
   {
      TRACE( "GetFirmwareRevision() = %lu\n", status );
      return;
   }

   if (str1[0] != 0)
   {
      SetFirmware( &str1[0] );
   }

   // Get hardware revision
   str1[0] = 0;
   status = mGobi.GetHardwareRevision( strSz1, str1 );
   if (status != 0)
   {
      TRACE( "GetHardwareRevision() = %lu\n", status );
      return;
   }

   if (str1[0] != 0)
   {
      SetHardware( &str1[0] );
   }

   // Get MDN/MIN
   str1[0] = 0;
   str2[0] = 0;
   status = mGobi.GetVoiceNumber( strSz1, str1, strSz2, str2 );
   if (status != 0)
   {
      if (status == 1016)
      {
         // Not provisioned
         SetMDN( "Not provisioned" );
         SetMIN( "Not provisioned" );
      }
      else
      {
         TRACE( "GetVoiceNumber() = %lu\n", status );
         return;
      }
   }

   if (str1[0] != 0)
   {
      SetMDN( &str1[0] );
   }

   if (str2[0] != 0)
   {
      SetMIN( &str2[0] );
   }

   // Get ESN/IMEI/MEID
   str1[0] = 0;
   str2[0] = 0;
   str3[0] = 0;
   status = mGobi.GetSerialNumbers( strSz1, 
                                    str1, 
                                    strSz2, 
                                    str2, 
                                    strSz3, 
                                    str3 );
   if (status != 0)
   {
      if (status == 1016)
      {
         // Not provisioned
         SetESN( "Not provisioned" );
         SetIMEI( "Not provisioned" );
         SetMEID( "Not provisioned" );
      }
      else
      {
         TRACE( "GetSerialNumbers() = %lu\n", status );
         return;
      }
   }

   if (str1[0] != 0)
   {
      SetESN( &str1[0] );
   }

   if (str2[0] != 0)
   {
      SetIMEI( &str2[0] );
   }

   if (str3[0] != 0)
   {
      SetMEID( &str3[0] );
   }

   // Get IMSI
   str1[0] = 0;
   status = mGobi.GetIMSI( strSz1, str1 );
   if (status != 0)
   {
      if (status == 1016)
      {
         // Not provisioned
         SetIMSI( "Not provisioned" );
      }
      else
      {
         TRACE( "GetIMSI() = %lu\n", status );
         return;
      }
   }

   if (str1[0] != 0)
   {
      SetIMSI( &str1[0] );
   }
}

/*===========================================================================
METHOD:
   UpdateConnectionInfo (Public Method)

DESCRIPTION:
   Update the connection info stats
===========================================================================*/
void cSampleCM::UpdateConnectionInfo()
{
   // Re-usable buffer
   std::ostringstream tmp;

   // Get session state
   ULONG rc = mGobi.GetSessionState( &mSessionState );
   if (rc != eGOBI_ERR_NONE || mSessionState == ULONG_MAX)
   {
      TRACE( "GetSessionState error %lu\n", rc );
      return;
   }

   UpdateSessionState( mSessionID != 0xFFFFFFFF, mSessionState );

   // If connected, refresh data bearer technology and session duration
   if (mSessionState == eQMIConnectionStatus_Connected)
   {
      CheckConnectedStats();
   }

   INT8 signalStrengths[MAX_SIGNALS];
   ULONG radioInterfaces[MAX_SIGNALS];

   // Get the signal strengths
   rc = mGobi.GetSignalStrengths( &signalStrengths[0], 
                                  &radioInterfaces[0] );
   if (rc != eGOBI_ERR_NONE && rc != eGOBI_ERR_NO_SIGNAL)
   {
      TRACE( "GetSignalStrengths error %lu\n", rc );
      return;
   }

   // Map signal strengths to RadioIf types
   for (ULONG s = 0; s < MAX_SIGNALS; s++)
   {
      INT8 signal = signalStrengths[s];

      switch (radioInterfaces[s])
      {
         case eQMINASRadioInterfaces_NoneNoService:
            mServiceSignals[eQMINASRadioInterfaces_NoneNoService] = signal;
            break;

         case eQMINASRadioInterfaces_CDMA20001x:
            mServiceSignals[eQMINASRadioInterfaces_CDMA20001x] = signal;
            break;

         case eQMINASRadioInterfaces_CDMA2000HRPD:
            mServiceSignals[eQMINASRadioInterfaces_CDMA2000HRPD] = signal;
            break;

         case eQMINASRadioInterfaces_AMPS:
            mServiceSignals[eQMINASRadioInterfaces_AMPS] = signal;
            break;

         case eQMINASRadioInterfaces_GSM:
            mServiceSignals[eQMINASRadioInterfaces_GSM] = signal;
            break;

         case eQMINASRadioInterfaces_UMTS:
            mServiceSignals[eQMINASRadioInterfaces_UMTS] = signal;
            break;

         case eQMINASRadioInterfaces_LTE:
            mServiceSignals[eQMINASRadioInterfaces_LTE] = signal;
            break;
      }
   }

   ULONG curTX = 0;
   ULONG curRX = 0;
   ULONG maxTX = 0;
   ULONG maxRX = 0;

   // Get the connection rate
   rc = mGobi.GetConnectionRate( &curTX, &curRX, &maxTX, &maxRX );
   if (rc != 0)
   {
      TRACE( "GetConnectionRate error %lu\n", rc );
      return;
   }
   
   // Store the max data rates
   tmp.str( "" );
   tmp << maxRX;
   SetMaxRx( tmp.str() );

   tmp.str( "" );
   tmp << maxTX;
   SetMaxTx( tmp.str() );



   ULONG dataCapabilities[MAX_DATA_CAPABILITIES];
   WORD srvMCC = 0;
   WORD srvMNC = 0;
   BYTE srvNameSize = 255;
   CHAR srvName[255];
   srvName[0] = 0;
   WORD sid = 0;
   WORD nid = 0;
   ULONG roam = 0;

   // Get the serving network info   
   rc = mGobi.GetServingNetwork( &dataCapabilities[0],
                                 &srvMCC,
                                 &srvMNC,
                                 srvNameSize,
                                 &srvName[0],
                                 &sid,
                                 &nid,
                                 &roam );
   if (rc != 0)
   {
      TRACE( "GetServingNetwork error %lu\n", rc );
      return;
   }

   // Store data capabilities
   for (ULONG c = 0; c < MAX_DATA_CAPABILITIES; c++)
   {
      if ( (dataCapabilities[c] != ULONG_MAX)
      &&   (dataCapabilities[c] != 0) )
      {
         mDataCapabilities.push_back( dataCapabilities[c] );
      }
   }

   if (roam == 0xFFFFFFFF)
   {
      SetRoam( "Unknown" );
   }
   else
   {
      tmp.str( "" );
      tmp << roam;
      SetRoam( tmp.str() );
   }

   // Update the signal strength and technology fields
   if (mSessionState != eQMIConnectionStatus_Connected)
   {
      mDataBearerTech = 0;
   }

   UpdateSignalAndTech();

   // Setup callbacks
   rc = mGobi.SetWDSEventReportCB( WDSEventReportCallback, 1 );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "SetWDSEventReportCB error %lu\n", rc );
      return;
   }

   rc = mGobi.SetWDSSessionStateCB( WDSSessionStateCallback );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "SetWDSSessionStateCB error %lu\n", rc );
      return;
   }

   BYTE thresSz = 5;
   INT8 thres[5] = {-90, -85, -80, -75, -70};
   rc = mGobi.SetNASEventReportCB( NASEventReportCallback,
                                   thresSz,
                                   &thres[0] );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "SetNASEventReportCB error %lu\n", rc );
      return;
   }

   rc = mGobi.SetNASServingSystemCB( NASServingSystemCallback );
   if (rc != eGOBI_ERR_NONE)
   {
      TRACE( "SetNASServingSystemCB error %lu\n", rc );
      return;
   }
}
