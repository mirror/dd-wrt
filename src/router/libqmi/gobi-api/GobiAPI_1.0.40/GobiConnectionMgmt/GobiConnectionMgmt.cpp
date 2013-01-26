/*===========================================================================
FILE: 
   GobiConnectionMgmt.cpp

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

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "GobiConnectionMgmt.h"
#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Global object
cGobiConnectionMgmtDLL gConnectionDLL;

// Interval between traffic processing loop iterations (milliseconds)
const ULONG TRAFFIC_INTERVAL_MS = 300000;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   TrafficProcessThread (Free Method)
   
DESCRIPTION:
   QMI traffic process thread - processes all traffic in order to fire
   off QMI traffic related callbacks

PARAMETERS:
   pArg        [ I ] - Object to interface to

RETURN VALUE:
   VOID * - always NULL
===========================================================================*/
VOID * TrafficProcessThread( PVOID pArg )
{
   // Keep running?
   bool bRun = false;

   TRACE( "GobiConnectionMgmt traffic thread [%u] started\n", 
          (UINT)pthread_self() );

   // Create a vector of the objects to wait on
   std::vector <cEvent *> events;

   // Store the index to service type for use later
   std::map <DWORD, eQMIService> services;

   cGobiConnectionMgmt * pAPI = (cGobiConnectionMgmt *)pArg;
   if (pAPI != 0)
   {
      // Time to go to work
      bRun = true;

      // Add the thread exit event
      events.push_back( &pAPI->mExitEvent );

      // For each Protocol server, grab the signal event
      std::set <cGobiConnectionMgmt::tServerConfig>::const_iterator pIter;
      pIter = pAPI->mServerConfig.begin();
      while (pIter != pAPI->mServerConfig.end())
      {
         eQMIService svc = pIter->first;
         cQMIProtocolServer * pServer = pAPI->GetServer( svc );
         if (pServer != 0)
         {
            // Grab the log from the server
            const cProtocolLog & log = pServer->GetLog();

            // Grab the Signal event, if it exists
            cEvent & sigEvent = log.GetSignalEvent();

            services[events.size()] = svc;
            events.push_back( &sigEvent );
         }

         pIter++;
      }
   }

   // Loop waiting for exit event
   while (bRun == true)
   {
      // Wait for activity
      DWORD ignoredVal, index;
      int nRet = WaitOnMultipleEvents( events, 
                                       TRAFFIC_INTERVAL_MS, 
                                       ignoredVal, 
                                       index );
      // Timeout
      if (nRet == -ETIME)
      {
         // Do nothing
      }
      // Error?
      else if (nRet <= 0)
      {
         TRACE( "GobiConnectionMgmt traffic thread wait error %d\n", nRet );
         bRun = false;
      }
      // Exit event?
      else if (index == 0)
      {
         bRun = false;
      }
      else if (index < events.size())
      {
         // Run ProcessTraffic() for this service type
         if (services.find( index ) != services.end())
         {
            pAPI->ProcessTraffic( services[index] );
         }
      }
      else
      {
         // Fatal error
         bRun = false;
      }
   }

   TRACE( "GobiConnectionMgmt traffic thread [%u] exited\n", 
          (UINT)pthread_self() );

   return NULL;
}

/*===========================================================================
METHOD:
   CallbackThread (Free Method)
   
DESCRIPTION:
   Thread to execute a callback asynchronously

PARAMETERS:
   pArg        [ I ] - The cAsyncFunction object

RETURN VALUE:
   void * - thread exit value (always 0)
===========================================================================*/
void * CallbackThread( PVOID pArg )
{
   cGobiCMCallback * pCB = (cGobiCMCallback *)pArg;
   if (pCB == 0)
   {
      ASSERT( 0 );
      return 0;
   }

   pCB->Call();

   delete pCB;
   pCB = 0;

   return 0;
}

/*=========================================================================*/
// cGobiConnectionMgmtDLL Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiConnectionMgmtDLL (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiConnectionMgmtDLL::cGobiConnectionMgmtDLL()
   :  mpAPI( 0 ),
      mbAllocated( false )
{
   // Create sync CS
   pthread_mutex_init( &mSyncSection, NULL );
}

/*===========================================================================
METHOD:
   ~cGobiConnectionMgmtDLL (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiConnectionMgmtDLL::~cGobiConnectionMgmtDLL()
{
   // Just in case
   if (mpAPI != 0)
   {
      mpAPI->Cleanup();
      delete mpAPI;
      mpAPI = 0;
   }

   pthread_mutex_destroy( &mSyncSection );
}

/*===========================================================================
METHOD:
   GetAPI (Public Method)

DESCRIPTION:
   Return the cGobiConnectionMgmt object
  
RETURN VALUE:
   cGobiConnectionMgmt *
===========================================================================*/ 
cGobiConnectionMgmt * cGobiConnectionMgmtDLL::GetAPI()
{
   pthread_mutex_lock( &mSyncSection );

   bool bAlloc = mbAllocated;

   pthread_mutex_unlock( &mSyncSection );

   if (bAlloc == true)
   {
      return mpAPI;
   }

   pthread_mutex_lock( &mSyncSection );

   mpAPI = new cGobiConnectionMgmt();
   if (mpAPI != 0)
   {
      mpAPI->Initialize();
   }

   // We have tried to allocate the object
   mbAllocated = true;

   pthread_mutex_unlock( &mSyncSection );
   return mpAPI;
}

/*=========================================================================*/
// cGobiConnectionMgmt Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiConnectionMgmt (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiConnectionMgmt::cGobiConnectionMgmt()
   :  cGobiQMICore(),
      mbThreadStarted( false ),
      mThreadID( 0 ),
      mWDSItemsProcessed( 0 ),
      mDMSItemsProcessed( 0 ),
      mNASItemsProcessed( 0 ),
      mWMSItemsProcessed( 0 ),
      mPDSItemsProcessed( 0 ),
      mCATItemsProcessed( 0 ),
      mOMAItemsProcessed( 0 ),
      mVoiceItemsProcessed( 0 ),
      mpFNSessionState( 0 ),
      mpFNByteTotals( 0 ),
      mpFNDataCapabilities( 0 ),
      mpFNDataBearer( 0 ),
      mpFNDormancyStatus( 0 ),
      mpFNMobileIPStatus( 0 ),
      mpFNActivationStatus( 0 ),
      mpFNPower( 0 ),
      mpFNWirelessDisable( 0 ),
      mpFNRoamingIndicator( 0 ),
      mpFNSignalStrength( 0 ),
      mpFNRFInfo( 0 ),
      mpFNLUReject( 0 ),
      mpPLMNMode( 0 ),
      mpFNNewSMS( 0 ),
      mpFNNewNMEA( 0 ),
      mpFNPDSState( 0 ),
      mpFNCATEvent( 0 ),
      mpFNOMADMAlert( 0 ),
      mpFNOMADMState( 0 ),
      mpFNUSSDRelease( 0 ),
      mpFNUSSDNotification( 0 ),
      mpFNUSSDOrigination( 0 )
{
   tServerConfig wdsSvr( eQMI_SVC_WDS, true );
   tServerConfig dmsSvr( eQMI_SVC_DMS, true );
   tServerConfig nasSvr( eQMI_SVC_NAS, true );
   tServerConfig wmsSvr( eQMI_SVC_WMS, true );
   tServerConfig pdsSvr( eQMI_SVC_PDS, true );
   tServerConfig catSvr( eQMI_SVC_CAT, false );
   tServerConfig rmsSvr( eQMI_SVC_RMS, false );
   tServerConfig omaSvr( eQMI_SVC_OMA, false );
   tServerConfig voiceSvr( eQMI_SVC_VOICE, false );
   mServerConfig.insert( wdsSvr );
   mServerConfig.insert( dmsSvr );
   mServerConfig.insert( nasSvr );
   mServerConfig.insert( wmsSvr );
   mServerConfig.insert( pdsSvr );
   mServerConfig.insert( catSvr );
   mServerConfig.insert( rmsSvr );
   mServerConfig.insert( omaSvr );
   mServerConfig.insert( voiceSvr );
}

/*===========================================================================
METHOD:
   ~cGobiConnectionMgmt (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiConnectionMgmt::~cGobiConnectionMgmt()
{
   Disconnect();
}

/*===========================================================================
METHOD:
   ProcessTraffic (Internal Method)

DESCRIPTION:
   Process traffic in a QMI server protocol log, this is done to
   exercise QMI indication related callbacks

PARAMETERS:
   svc         [ I ] - QMI Service type

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessTraffic( eQMIService svc )
{
   ULONG count = 0;

   switch (svc)
   {
      case eQMI_SVC_WDS:
      {
         cQMIProtocolServer * pWDS = GetServer( eQMI_SVC_WDS );
         if (pWDS != 0)
         {
            // Grab the WDS log from the server
            const cProtocolLog & logWDS = pWDS->GetLog();

            // New WDS items to process?
            count = logWDS.GetCount();
            if (count != INVALID_LOG_INDEX && count > mWDSItemsProcessed)
            {
               for (ULONG i = mWDSItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logWDS.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_WDS_RX) )
                  {
                     ProcessWDSBuffer( buf );
                  }
               }

               mWDSItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_DMS:
      {
         cQMIProtocolServer * pDMS = GetServer( eQMI_SVC_DMS );
         if (pDMS != 0)
         {
            // Grab the DMS log from the server
            const cProtocolLog & logDMS = pDMS->GetLog();

            // New DMS items to process?
            count = logDMS.GetCount();
            if (count != INVALID_LOG_INDEX && count > mDMSItemsProcessed)
            {
               for (ULONG i = mDMSItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logDMS.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_DMS_RX) )
                  {
                     ProcessDMSBuffer( buf );
                  }
               }

               mDMSItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_NAS:
      {
         cQMIProtocolServer * pNAS = GetServer( eQMI_SVC_NAS );
         if (pNAS != 0)
         {
            // Grab the NAS log from the server
            const cProtocolLog & logNAS = pNAS->GetLog();

            // New NAS items to process?
            count = logNAS.GetCount();
            if (count != INVALID_LOG_INDEX && count > mNASItemsProcessed)
            {
               for (ULONG i = mNASItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logNAS.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_NAS_RX) )
                  {
                     ProcessNASBuffer( buf );
                  }
               }

               mNASItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_WMS:
      {
         cQMIProtocolServer * pWMS = GetServer( eQMI_SVC_WMS );
         if (pWMS != 0)
         {
            // Grab the WMS log from the server
            const cProtocolLog & logWMS = pWMS->GetLog();

            // New WMS items to process?
            count = logWMS.GetCount();
            if (count != INVALID_LOG_INDEX && count > mWMSItemsProcessed)
            {
               for (ULONG i = mWMSItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logWMS.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_WMS_RX) )
                  {
                     ProcessWMSBuffer( buf );
                  }
               }

               mWMSItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_PDS:
      {
         cQMIProtocolServer * pPDS = GetServer( eQMI_SVC_PDS );
         if (pPDS != 0)
         {
            // Grab the PDS log from the server
            const cProtocolLog & logPDS = pPDS->GetLog();

            // New PDS items to process?
            count = logPDS.GetCount();
            if (count != INVALID_LOG_INDEX && count > mPDSItemsProcessed)
            {
               for (ULONG i = mPDSItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logPDS.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_PDS_RX) )
                  {
                     ProcessPDSBuffer( buf );
                  }
               }

               mPDSItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_CAT:
      {
         cQMIProtocolServer * pCAT = GetServer( eQMI_SVC_CAT );
         if (pCAT != 0)
         {
            // Grab the CAT log from the server
            const cProtocolLog & logCAT = pCAT->GetLog();

            // New CAT items to process?
            count = logCAT.GetCount();
            if (count != INVALID_LOG_INDEX && count > mCATItemsProcessed)
            {
               for (ULONG i = mCATItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logCAT.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_CAT_RX) )
                  {
                     ProcessCATBuffer( buf );
                  }
               }

               mCATItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_OMA:
      {
         cQMIProtocolServer * pOMA = GetServer( eQMI_SVC_OMA );
         if (pOMA != 0)
         {
            // Grab the OMA log from the server
            const cProtocolLog & logOMA = pOMA->GetLog();

            // New OMA items to process?
            count = logOMA.GetCount();
            if (count != INVALID_LOG_INDEX && count > mOMAItemsProcessed)
            {
               for (ULONG i = mOMAItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logOMA.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_OMA_RX) )
                  {
                     ProcessOMABuffer( buf );
                  }
               }

               mOMAItemsProcessed = count;
            }
         }

         break;
      }

      case eQMI_SVC_VOICE:
      {
         cQMIProtocolServer * pVoice = GetServer( eQMI_SVC_VOICE );
         if (pVoice != 0)
         {
            // Grab the voice log from the server
            const cProtocolLog & logVoice = pVoice->GetLog();

            // New voice items to process?
            count = logVoice.GetCount();
            if (count != INVALID_LOG_INDEX && count > mVoiceItemsProcessed)
            {
               for (ULONG i = mVoiceItemsProcessed; i < count; i++)
               {
                  sProtocolBuffer buf = logVoice.GetBuffer( i );
                  if ( (buf.IsValid() == true) 
                  &&   (buf.GetType() == (ULONG)ePROTOCOL_QMI_VOICE_RX) )
                  {
                     ProcessVoiceBuffer( buf );
                  }
               }

               mVoiceItemsProcessed = count;
            }
         }

         break;
      }

      default:
         break;
   }
}

/*===========================================================================
METHOD:
   ProcessWDSBuffer (Internal Method)

DESCRIPTION:
   Process a WDS buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessWDSBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   } 

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == eQMI_WDS_EVENT_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out data bearer technology
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_IND, msgID, 23 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         if (mpFNDataBearer != 0)
         {
            cDataBearerCallback * pCB = 0;
            pCB = new cDataBearerCallback( mpFNDataBearer, pf[0].mValue.mU32 );
            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }

      // Parse out dormancy status
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_IND, msgID, 24 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         if (mpFNDormancyStatus != 0)
         {
            cDormancyStatusCallback * pCB = 0;
            pCB = new cDormancyStatusCallback( mpFNDormancyStatus, 
                                               pf[0].mValue.mU32 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }

      // Parse out byte totals
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_IND, msgID, 25 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         ULONGLONG tx = pf[0].mValue.mU64;
         ULONGLONG rx = ULLONG_MAX;

         tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_IND, msgID, 26 );
         pf = ParseTLV( mDB, buf, tlvs, tlvKey );
         if (pf.size() >= 1)
         {
            rx = pf[0].mValue.mU64;
         }        

         if (mpFNByteTotals != 0)
         {
            cByteTotalsCallback * pCB = 0;
            pCB = new cByteTotalsCallback( mpFNByteTotals, tx, rx ); 
            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
      // Parse out mobile IP status
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_IND, msgID, 27 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         if (mpFNMobileIPStatus != 0)
         {
            cMobileIPStatusCallback * pCB = 0;
            pCB = new cMobileIPStatusCallback( mpFNMobileIPStatus, 
                                               pf[0].mValue.mU32 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
   }
   else if (msgID == eQMI_WDS_PKT_STATUS_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out session status
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_WDS_IND, msgID, 1 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         ULONG ss = pf[0].mValue.mU32;
         ULONG cer = ULONG_MAX;

         // Parse out call end reason (if present)
         tlvKey = sProtocolEntityKey( eDB2_ET_QMI_WDS_IND, msgID, 16 );
         pf = ParseTLV( mDB, buf, tlvs, tlvKey );
         if (pf.size() >= 1)
         {
            cer = pf[0].mValue.mU32;
         }

         if (mpFNSessionState != 0)
         {
            cSessionStateCallback * pCB = 0;
            pCB = new cSessionStateCallback( mpFNSessionState, ss, cer );
            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessDMSBuffer (Internal Method)

DESCRIPTION:
   Process a DMS buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessDMSBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   } 

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == eQMI_DMS_EVENT_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out activation status
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_IND, msgID, 19 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1 && mpFNActivationStatus != 0)
      {
         cActivationStatusCallback * pCB = 0;
         pCB = new cActivationStatusCallback( mpFNActivationStatus, 
                                              pf[0].mValue.mU32 );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }

      // Parse out operating mode
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_IND, msgID, 20 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1 && mpFNPower != 0)
      {
         cPowerCallback * pCB = 0;
         pCB = new cPowerCallback( mpFNPower, pf[0].mValue.mU32 );
         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }

      // Parse out wireless disable state
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_IND, msgID, 22 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1 && mpFNWirelessDisable != 0)
      {
         cWirelessDisableCallback * pCB = 0;
         pCB = new cWirelessDisableCallback( mpFNWirelessDisable, 
                                             pf[0].mValue.mU32 );
         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessNASBuffer (Internal Method)

DESCRIPTION:
   Process a NAS buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessNASBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   } 

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_NAS_EVENT_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse signal strength
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 2)
      {
         INT8 sigVal = pf[0].mValue.mS8;
         ULONG radioVal = pf[1].mValue.mU32;
         bool bValidSig = (sigVal <= -30 && sigVal > -125 && radioVal != 0);

         if (bValidSig == true && mpFNSignalStrength != 0)
         {
            cSignalStrengthCallback * pCB = 0;
            pCB = new cSignalStrengthCallback( mpFNSignalStrength, 
                                               pf[0].mValue.mS8,
                                               pf[1].mValue.mU32 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }

      // Parse out RF info
      sProtocolEntityKey tlvKey2( eDB2_ET_QMI_NAS_IND, msgID, 17 );
      cDataParser::tParsedFields pf2 = ParseTLV( mDB, buf, tlvs, tlvKey2 );

      ULONG fieldCount = (ULONG)pf2.size();
      if (fieldCount >= 1 && mpFNRFInfo != 0)
      {
         BYTE ifaceCount = pf2[0].mValue.mU8;
         if (fieldCount >= 1 + ((ULONG)ifaceCount * 3))
         {
            for (BYTE i = 0; i < ifaceCount; i++)
            {
               ULONG offset = 3 * (ULONG)i;

               cRFInfoCallback * pCB = 0;
               pCB = new cRFInfoCallback( mpFNRFInfo, 
                                          pf2[offset + 1].mValue.mU32,
                                          pf2[offset + 2].mValue.mU32,
                                          (ULONG)pf2[offset + 3].mValue.mU16 );

               if (pCB != 0)
               {
                  if (pCB->Initialize() == false)
                  {
                     delete pCB;
                  }
               }
            }
         }
      }

      // Parse out LU reject
      sProtocolEntityKey tlvKey3( eDB2_ET_QMI_NAS_IND, msgID, 18 );
      cDataParser::tParsedFields pf3 = ParseTLV( mDB, buf, tlvs, tlvKey3 );
      if (pf3.size() >= 2 && mpFNLUReject != 0)
      {
         cLURejectCallback * pCB = 0;
         pCB = new cLURejectCallback( mpFNLUReject, 
                                      pf3[0].mValue.mU32,
                                      (ULONG)pf3[1].mValue.mU16 );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }

   }
   else if (msgID == (ULONG)eQMI_NAS_SS_INFO_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out roaming indicator
      sProtocolEntityKey tlvKey1( eDB2_ET_QMI_NAS_IND, msgID, 16 );
      cDataParser::tParsedFields pf1 = ParseTLV( mDB, buf, tlvs, tlvKey1 );
      if (pf1.size() >= 1)
      {
         if (mpFNRoamingIndicator != 0)
         {
            cRoamingIndicatorCallback * pCB = 0;
            pCB = new cRoamingIndicatorCallback( mpFNRoamingIndicator, 
                                                 pf1[0].mValue.mU32 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }

      // Parse out data capabilities
      sProtocolEntityKey tlvKey2( eDB2_ET_QMI_NAS_IND, msgID, 17 );
      cDataParser::tParsedFields pf2 = ParseTLV( mDB, buf, tlvs, tlvKey2 );
      if (pf2.size() >= 1)
      {
         BYTE activeDataCaps = pf2[0].mValue.mU8;
         if (pf2.size() >= 1 + (ULONG)activeDataCaps)
         {
            ULONG caps[12] = { 0 };
            if (activeDataCaps > 12)
            {
               activeDataCaps = 12;
            }

            for (ULONG d = 0; d < activeDataCaps; d++)
            {
               caps[d] = pf2[1 + d].mValue.mU32;
            }

            if (mpFNDataCapabilities != 0)
            {
               cDataCapabilitiesCallback * pCB = 0;
               pCB = new cDataCapabilitiesCallback( mpFNDataCapabilities, 
                                                    activeDataCaps, 
                                                    &caps[0] );

               if (pCB != 0)
               {
                  if (pCB->Initialize() == false)
                  {
                     delete pCB;
                  }
               }
            }
         }
      }
   }
   else if (msgID == (ULONG)eQMI_NAS_PLMN_MODE_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse PLMN mode
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_NAS_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {  
         cPLMNModeCallback * pCB = 0;
         pCB = new cPLMNModeCallback( mpPLMNMode, 
                                      (ULONG)pf[0].mValue.mU8 );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessWMSBuffer (Internal Method)

DESCRIPTION:
   Process a WDS buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessWMSBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   }

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_WMS_EVENT_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out message details
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_WMS_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 2)
      {
         if (mpFNNewSMS != 0)
         {
            cNewSMSCallback * pCB = 0;
            pCB = new cNewSMSCallback( mpFNNewSMS, 
                                       pf[0].mValue.mU32,
                                       pf[1].mValue.mU32 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessPDSBuffer (Internal Method)

DESCRIPTION:
   Process a PDS buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessPDSBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   }

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_PDS_EVENT_IND)
   {
      // Prepare TLVs for extraction
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1 && mpFNNewNMEA != 0)
      {
         cNewNMEACallback * pCB = 0;
         pCB = new cNewNMEACallback( mpFNNewNMEA, pf[0].mValueString );
         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }
   }
   else if (msgID == (ULONG)eQMI_PDS_STATE_IND)
   {
      // Prepare TLVs for extraction
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out message details
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_PDS_IND, msgID, 1 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 2 && mpFNPDSState != 0)
      {
         cPDSStateCallback * pCB = 0;
         pCB = new cPDSStateCallback( mpFNPDSState, 
                                      pf[0].mValue.mU32,
                                      pf[1].mValue.mU32 );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessCATBuffer (Internal Method)

DESCRIPTION:
   Process a CAT buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessCATBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   }

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_CAT_EVENT_IND && mpFNCATEvent != 0)
   {
      // Prepare TLVs for extraction
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      ULONG tlvCount = (ULONG)tlvs.size();
      for (ULONG t = 0; t < tlvCount; t++)
      {
         const sDB2NavInput tlv = tlvs[t];
         if (tlv.mKey.size() == 3)
         {
            cCATEventCallback * pCB = 0;
            pCB = new cCATEventCallback( mpFNCATEvent, 
                                         tlv.mKey[2],
                                         tlv.mPayloadLen,
                                         tlv.mpPayload );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessOMABuffer (Internal Method)

DESCRIPTION:
   Process an OMA buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessOMABuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   }

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_OMA_EVENT_IND)
   {
      // Prepare TLVs for parsing
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );

      // Parse out NIA
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_OMA_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 2)
      {
         if (mpFNOMADMAlert != 0)
         {
            cOMADMAlertCallback * pCB = 0;
            pCB = new cOMADMAlertCallback( mpFNOMADMAlert, 
                                           pf[0].mValue.mU32,
                                           pf[1].mValue.mU16 );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }

      // Parse out failure reason (may not be present)
      ULONG failureReason = ULONG_MAX;
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_OMA_IND, msgID, 18 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         failureReason = pf[0].mValue.mU32;
      }
         
      // Parse out state
      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_OMA_IND, msgID, 17 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         if (mpFNOMADMState != 0)
         {
            cOMADMStateCallback * pCB = 0;
            pCB = new cOMADMStateCallback( mpFNOMADMState, 
                                           pf[0].mValue.mU32,
                                           failureReason );

            if (pCB != 0)
            {
               if (pCB->Initialize() == false)
               {
                  delete pCB;
               }
            }
         }
      }
   }
}

/*===========================================================================
METHOD:
   ProcessVoiceBuffer (Internal Method)

DESCRIPTION:
   Process a voice buffer

PARAMETERS:
   buf         [ I ] - QMI buffer to process

RETURN VALUE:
   None
===========================================================================*/
void cGobiConnectionMgmt::ProcessVoiceBuffer( const sProtocolBuffer & buf )
{
   sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );   
   if (qmiBuf.IsValid() == false)
   {
      return;
   }

   // Indication?
   if (qmiBuf.IsIndication() == false)
   {
      return;
   }

   // Do we even care?
   ULONG msgID = qmiBuf.GetMessageID();
   if (msgID == (ULONG)eQMI_VOICE_USSD_RELEASE_IND && mpFNUSSDRelease != 0)
   {
      cUSSDReleaseCallback * pCB = 0;
      pCB = new cUSSDReleaseCallback( mpFNUSSDRelease );
      if (pCB != 0)
      {
         if (pCB->Initialize() == false)
         {
            delete pCB;
         }
      }

   }
   else if (msgID == (ULONG)eQMI_VOICE_USSD_IND && mpFNUSSDNotification != 0)
   {
      // Prepare TLVs for extraction
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );
      std::map <ULONG, const sQMIRawContentHeader *> tlvMap;
      tlvMap = qmiBuf.GetContents();

      // Parse out message details
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_VOICE_IND, msgID, 1 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         const BYTE * pUSSData = 0;

         std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
         pIter = tlvMap.find( 16 );
         if (pIter != tlvMap.end())
         {
            const sQMIRawContentHeader * pHdr = pIter->second;
            ULONG len = (ULONG)pHdr->mLength;
            if (len >= (ULONG)2)
            {
               const BYTE * pData = (const BYTE *)++pHdr;
               if (len >= (ULONG)pData[1] + (ULONG)2)
               {
                  pUSSData = pData;
               }
            }
         }

         cUSSDNotificationCallback * pCB = 0;
         pCB = new cUSSDNotificationCallback( mpFNUSSDNotification, 
                                              pf[0].mValue.mU32,
                                              pUSSData );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }
   }
   else if ( (msgID == (ULONG)eQMI_VOICE_ASYNC_USSD_IND) 
        &&   (mpFNUSSDOrigination != 0) )
   {
      // Prepare TLVs for extraction
      std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiBuf );
      std::map <ULONG, const sQMIRawContentHeader *> tlvMap;
      tlvMap = qmiBuf.GetContents();

      ULONG ec = ULONG_MAX;
      ULONG fc = ULONG_MAX;

      // Parse out message details
      sProtocolEntityKey tlvKey( eDB2_ET_QMI_VOICE_IND, msgID, 16 );
      cDataParser::tParsedFields pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         ec = pf[0].mValue.mU32;
      }

      tlvKey = sProtocolEntityKey( eDB2_ET_QMI_VOICE_IND, msgID, 17 );
      pf = ParseTLV( mDB, buf, tlvs, tlvKey );
      if (pf.size() >= 1)
      {
         fc = pf[0].mValue.mU32;
      }

      const BYTE * pNetworkInfo = 0;

      std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
      pIter = tlvMap.find( 18 );
      if (pIter != tlvMap.end())
      {
         const sQMIRawContentHeader * pHdr = pIter->second;
         ULONG len = (ULONG)pHdr->mLength;
         if (len >= (ULONG)2)
         {
            const BYTE * pData = (const BYTE *)++pHdr;
            if (len >= (ULONG)pData[1] + (ULONG)2)
            {
               pNetworkInfo = pData;
            }
         }
      }

      const BYTE * pAlpha = 0;

      pIter = tlvMap.find( 19 );
      if (pIter != tlvMap.end())
      {
         const sQMIRawContentHeader * pHdr = pIter->second;
         ULONG len = (ULONG)pHdr->mLength;
         if (len >= (ULONG)2)
         {
            const BYTE * pData = (const BYTE *)++pHdr;
            if (len >= (ULONG)pData[1] + (ULONG)2)
            {
               pAlpha = pData;
            }
         }
      }


      cUSSDOriginationCallback * pCB = 0;
      pCB = new cUSSDOriginationCallback( mpFNUSSDOrigination, 
                                          ec,
                                          fc,
                                          pNetworkInfo,
                                          pAlpha );

      if (pCB != 0)
      {
         if (pCB->Initialize() == false)
         {
            delete pCB;
         }
      }
   }
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified (or first detected) Gobi device

PARAMETERS:
   pDeviceNode   [ I ] - The device node
   pDeviceKey    [ I ] - The device key (unique, stored on-device)
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiConnectionMgmt::Connect(
   LPCSTR                    pDeviceNode,
   LPCSTR                    pDeviceKey )
{
   // Assume failure
   bool bRC = cGobiQMICore::Connect( pDeviceNode, pDeviceKey );
   if (bRC == true)
   {
      // Clear mExitEvent;
      mExitEvent.Clear();

      pthread_create( &mThreadID,
                      NULL,
                      TrafficProcessThread,
                      this );
                      
      mbThreadStarted = true;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Disconnect from the currently connected Gobi device
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiConnectionMgmt::Disconnect()
{
   // Clear all callback function pointers (no need to turn them off at
   // the device as we are about to tear-down each QMI service client)
   mpFNSessionState = 0;
   mpFNByteTotals = 0;
   mpFNDataCapabilities = 0;
   mpFNDataBearer = 0;
   mpFNDormancyStatus = 0;
   mpFNMobileIPStatus = 0;
   mpFNActivationStatus = 0;
   mpFNPower = 0;
   mpFNWirelessDisable = 0;
   mpFNRoamingIndicator = 0;
   mpFNSignalStrength = 0;
   mpFNRFInfo = 0;
   mpFNLUReject = 0;
   mpPLMNMode = 0;
   mpFNNewSMS = 0;
   mpFNNewNMEA = 0;
   mpFNPDSState = 0;
   mpFNCATEvent = 0;
   mpFNOMADMAlert = 0;
   mpFNOMADMState = 0;
   mpFNUSSDRelease = 0;
   mpFNUSSDNotification = 0;
   mpFNUSSDOrigination = 0;

   // Exit traffic processing thread
   if (mbThreadStarted == true)
   {
      // Signal thread to exit
      mExitEvent.Set( 0 );

      // If we are not being called from the thread itself then wait for
      // it to exit, if not then it will have to exit automatically
      if (pthread_self() != mThreadID)
      {
         if (mThreadID != 0)
         {
            pthread_join( mThreadID, NULL );
         }
      }
   }

   // Clear out thread handle/ID
   mbThreadStarted = false;
   mThreadID = 0;

   bool bRC = cGobiQMICore::Disconnect();

   // Servers reset server logs so we need to reset our counters
   mWDSItemsProcessed = 0;
   mDMSItemsProcessed = 0;
   mNASItemsProcessed = 0;
   mWMSItemsProcessed = 0;
   mPDSItemsProcessed = 0;
   mCATItemsProcessed = 0;
   mOMAItemsProcessed = 0;
   mVoiceItemsProcessed = 0;

   return bRC;
}

/*===========================================================================
METHOD:
   SetSessionStateCallback (Public Method)

DESCRIPTION:
   Enable/disable session state callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetSessionStateCallback(
   tFNSessionState            pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNSessionState = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetByteTotalsCallback

DESCRIPTION:
   This function enables/disables the RX/TX byte counts callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)
   interval    [ I ] - Interval in seconds (ignored when disabling)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetByteTotalsCallback( 
   tFNByteTotals              pCallback,
   BYTE                       interval )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNByteTotals == 0);
   bool bOff = (pCallback == 0 && mpFNByteTotals != 0);
   bool bReplace = (pCallback != 0 && mpFNByteTotals != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_WDS;
      WORD msgID = (WORD)eQMI_WDS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 17 );

      if (bOn == true)
      {
         std::ostringstream tmp;
         tmp << (ULONG)interval << " 0 0 0 0 0 0 1 1";
         sDB2PackingInput pi( pek, (LPCSTR)tmp.str().c_str() ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0 0 0 0 0 0 0 0 0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNByteTotals = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNByteTotals = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetDataCapabilitiesCallback (Public Method)

DESCRIPTION:
   Enables/disables the serving system data capabilities callback
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Corrected error code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetDataCapabilitiesCallback( 
   tFNDataCapabilities        pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNDataCapabilities = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetDataBearerCallback (Public Method)

DESCRIPTION:
   Enable/disable data bearer callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return cod
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetDataBearerCallback( 
   tFNDataBearer              pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNDataBearer == 0);
   bool bOff = (pCallback == 0 && mpFNDataBearer != 0);
   bool bReplace = (pCallback != 0 && mpFNDataBearer != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_WDS;
      WORD msgID = (WORD)eQMI_WDS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 18 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNDataBearer = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNDataBearer = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetDormancyStatusCallback (Public Method)

DESCRIPTION:
   Enable/disable dormancy status callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetDormancyStatusCallback( 
   tFNDormancyStatus          pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNDormancyStatus == 0);
   bool bOff = (pCallback == 0 && mpFNDormancyStatus != 0);
   bool bReplace = (pCallback != 0 && mpFNDormancyStatus != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_WDS;
      WORD msgID = (WORD)eQMI_WDS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 19 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNDormancyStatus = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNDormancyStatus = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetMobileIPStatusCallback (Public Method)

DESCRIPTION:
   Enable/disable mobile IP status callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetMobileIPStatusCallback( 
   tFNMobileIPStatus          pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNMobileIPStatus == 0);
   bool bOff = (pCallback == 0 && mpFNMobileIPStatus != 0);
   bool bReplace = (pCallback != 0 && mpFNMobileIPStatus != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_WDS;
      WORD msgID = (WORD)eQMI_WDS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_WDS_REQ, msgID, 20 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNMobileIPStatus = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNMobileIPStatus = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetActivationStatusCallback (Public Method)

DESCRIPTION:
   Enable/disable activation status callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetActivationStatusCallback( 
   tFNActivationStatus        pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNActivationStatus == 0);
   bool bOff = (pCallback == 0 && mpFNActivationStatus != 0);
   bool bReplace = (pCallback != 0 && mpFNActivationStatus != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_DMS;
      WORD msgID = (WORD)eQMI_DMS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 19 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNActivationStatus = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNActivationStatus = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetPowerCallback (Public Method)

DESCRIPTION:
   Enable/disable power operating mode callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetPowerCallback( 
   tFNPower                   pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNPower == 0);
   bool bOff = (pCallback == 0 && mpFNPower != 0);
   bool bReplace = (pCallback != 0 && mpFNPower != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_DMS;
      WORD msgID = (WORD)eQMI_DMS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 20 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNPower = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNPower = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetWirelessDisableCallback (Public Method)

DESCRIPTION:
   Enable/disable wireless disable state callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetWirelessDisableCallback( 
   tFNWirelessDisable         pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNWirelessDisable == 0);
   bool bOff = (pCallback == 0 && mpFNWirelessDisable != 0);
   bool bReplace = (pCallback != 0 && mpFNWirelessDisable != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_DMS;
      WORD msgID = (WORD)eQMI_DMS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 22 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNWirelessDisable = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNWirelessDisable = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetRoamingIndicatorCallback (Public Method)

DESCRIPTION:
   Enable/disable roaming indicator callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Corrected error code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetRoamingIndicatorCallback( 
   tFNRoamingIndicator        pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNRoamingIndicator = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetSignalStrengthCallback (Public Method)

DESCRIPTION:
   Enable/disable signal strength callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function
   thresholds  [ I ] - Desired threholds (only valid when enabling)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetSignalStrengthCallback( 
   tFNSignalStrength          pCallback,
   std::list <INT8>           thresholds )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Grab number of thresholds
   ULONG thresholdCount = (ULONG)thresholds.size();

   // Validate arguments versus what is changing
   bool bOn = (pCallback != 0 && mpFNSignalStrength == 0);
   if (bOn == true && thresholdCount == 0)
   {
      rc = eGOBI_ERR_INVALID_ARG;
      return rc;
   }

   bool bOff = (pCallback == 0 && mpFNSignalStrength != 0);
   if (bOff == true && thresholdCount != 0)
   {
      rc = eGOBI_ERR_INVALID_ARG;
      return rc;
   }

   bool bReplace = (pCallback != 0 && mpFNSignalStrength != 0);
   if (bReplace == true && thresholdCount == 0)
   {
      rc = eGOBI_ERR_INVALID_ARG;
      return rc;
   }

   eQMIService svc = eQMI_SVC_NAS;
   WORD msgID = (WORD)eQMI_NAS_SET_EVENT;
   sSharedBuffer * pReq = 0;

   std::vector <sDB2PackingInput> piv;
   sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 16 );

   if (bOn == true || bOff == true || bReplace == true)
   {
      if (bOn == true || bReplace == true)
      {
         std::ostringstream args;
         args << "1 " << (UINT)thresholdCount;

         std::list <INT8>::const_iterator pThreshold = thresholds.begin();
         while (pThreshold != thresholds.end())
         {
            INT8 t = *pThreshold++;

            args << " " << (INT)t;
         }

         sDB2PackingInput pi( pek, (LPCSTR)args.str().c_str() ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0 0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true || bReplace == true)
      {
         mpFNSignalStrength = pCallback;
      }
   }
   else
   {
      // Turning it off redundantly
      if (thresholdCount != 0)
      {
         rc = eGOBI_ERR_INVALID_ARG;
      }
      else
      {
         rc = eGOBI_ERR_NONE;
      }
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetRFInfoCallback (Public Method)

DESCRIPTION:
   Enable/disable RF information callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetRFInfoCallback( tFNRFInfo pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Validate arguments versus what is changing
   bool bOn = (pCallback != 0 && mpFNRFInfo == 0);
   bool bOff = (pCallback == 0 && mpFNRFInfo != 0);
   bool bReplace = (pCallback != 0 && mpFNRFInfo != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_NAS;
      WORD msgID = (WORD)eQMI_NAS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 17 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNRFInfo = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNRFInfo = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetLURejectCallback (Public Method)

DESCRIPTION:
   Enable/disable LU reject callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetLURejectCallback( tFNLUReject pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Validate arguments versus what is changing
   bool bOn = (pCallback != 0 && mpFNLUReject == 0);
   bool bOff = (pCallback == 0 && mpFNLUReject != 0);
   bool bReplace = (pCallback != 0 && mpFNLUReject != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_NAS;
      WORD msgID = (WORD)eQMI_NAS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_NAS_REQ, msgID, 18 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNLUReject = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNLUReject = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetPLMNModeCallback (Public Method)

DESCRIPTION:
   Enable/disable PLMN mode callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetPLMNModeCallback( tFNPLMNMode pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpPLMNMode = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetNewSMSCallback (Public Method)

DESCRIPTION:
   Enable/disable new SMS callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetNewSMSCallback( tFNNewSMS pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNNewSMS == 0);
   bool bOff = (pCallback == 0 && mpFNNewSMS != 0);
   bool bReplace = (pCallback != 0 && mpFNNewSMS != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_WMS;
      WORD msgID = (WORD)eQMI_WMS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_WMS_REQ, msgID, 16 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNNewSMS = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNNewSMS = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetNMEACallback (Public Method)

DESCRIPTION:
   Enable/disable new NMEA sentence function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetNMEACallback( tFNNewNMEA pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNNewNMEA == 0);
   bool bOff = (pCallback == 0 && mpFNNewNMEA != 0);
   bool bReplace = (pCallback != 0 && mpFNNewNMEA != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_PDS;
      WORD msgID = (WORD)eQMI_PDS_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_PDS_REQ, msgID, 16 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNNewNMEA = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNNewNMEA = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetPDSStateCallback (Public Method)

DESCRIPTION:
   Enable/disable PDS service state callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetPDSStateCallback( tFNPDSState pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNPDSState = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetCATEventCallback (Public Method)

DESCRIPTION:
   This function enables/disables the CAT event callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)
   eventMask   [ I ] - Bitmask of CAT events to register for
   pErrorMask  [ O ] - Error bitmask

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetCATEventCallback( 
   tFNCATEvent                pCallback,
   ULONG                      eventMask,
   ULONG *                    pErrorMask )
{
   // Assume failure
   eGobiError retCode = eGOBI_ERR_GENERAL;
   *pErrorMask = ULONG_MAX;

   // Validate arguments versus what is changing
   bool bOn = (pCallback != 0 && mpFNCATEvent == 0);
   bool bOff = (pCallback == 0 && mpFNCATEvent != 0);
   bool bReplace = (pCallback != 0 && mpFNCATEvent != 0);
   if (bOn == true || bOff == true || bReplace == true)
   {
      const ULONG szTLVHdr = (ULONG)sizeof( sQMIRawContentHeader ); 
      const ULONG szData = (ULONG)sizeof( ULONG );
      BYTE buf[szTLVHdr + szData];

      sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)&buf[0];
      pTLV->mTypeID = 16;
      pTLV->mLength = (WORD)szData;

      ULONG * pData = (ULONG *)&buf[szTLVHdr];

      *pData = eventMask;

      if (bOff == true)
      {
         // Ignore event mask argument when disabling the callback
         *pData = 0;

         // We also always clear the callback regardless of the response
         mpFNCATEvent = pCallback;
      }

      sSharedBuffer * pReq = 0;
      pReq  = sQMIServiceBuffer::BuildBuffer( eQMI_SVC_CAT,
                                              (WORD)eQMI_CAT_SET_EVENT,
                                              false,
                                              false,
                                              &buf[0],
                                              szTLVHdr + szData );

      sProtocolBuffer rsp = Send( eQMI_SVC_CAT, pReq );
      if (rsp.IsValid() == false)
      {
         retCode = GetCorrectedLastError();
         return retCode;
      }

      // Did we receive a valid QMI response?
      sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
      if (qmiRsp.IsValid() == false)
      {
         retCode = eGOBI_ERR_MALFORMED_RSP;
         return retCode;
      }
      
      // Check the mandatory QMI result TLV for success
      ULONG rc = 0;
      ULONG ec = 0;
      bool bResult = qmiRsp.GetResult( rc, ec );
      if (bResult == false)
      {
         retCode = eGOBI_ERR_MALFORMED_RSP;
         return retCode;
      }
      else if (rc != 0)
      {
         // Parse out the error mask?
         if (qmiRsp.GetMessageID() == (ULONG)eQMI_CAT_SET_EVENT)
         {
            std::map <ULONG, const sQMIRawContentHeader *> tlvs; 
            tlvs = qmiRsp.GetContents();

            std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
            pIter = tlvs.find( 16 );
            if (pIter != tlvs.end())
            {
               const sQMIRawContentHeader * pHdr = pIter->second;
               if (pHdr->mLength > 4)

               {
                  pData = (ULONG *)++pHdr;
                  *pErrorMask = *pData;
               }
            } 
         }

         retCode = GetCorrectedQMIError( ec );
         return retCode;
      }

      // Success!
      mpFNCATEvent = pCallback;
      retCode = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      retCode = eGOBI_ERR_NONE;
   }

   return retCode;
}

/*===========================================================================
METHOD:
   SetOMADMAlertCallback (Public Method)

DESCRIPTION:
   This function enables/disables the OMA-DM network initiated alert
   callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetOMADMAlertCallback( 
   tFNOMADMAlert              pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNOMADMAlert == 0);
   bool bOff = (pCallback == 0 && mpFNOMADMAlert != 0);
   bool bReplace = (pCallback != 0 && mpFNOMADMAlert != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_OMA;
      WORD msgID = (WORD)eQMI_OMA_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 16 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNOMADMAlert = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNOMADMAlert = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetOMADMStateCallback (Public Method)

DESCRIPTION:
   This function enables/disables the OMA-DM state callback function

PARAMETERS:
   pCallback   [ I ] - Callback function (0 = disable)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetOMADMStateCallback( tFNOMADMState pCallback )
{
   // Assume failure
   eGobiError rc = eGOBI_ERR_GENERAL;

   // Something changing?
   bool bOn = (pCallback != 0 && mpFNOMADMState == 0);
   bool bOff = (pCallback == 0 && mpFNOMADMState != 0);
   bool bReplace = (pCallback != 0 && mpFNOMADMState != 0);
   if (bOn == true || bOff == true)
   {
      // Turning on/off
      eQMIService svc = eQMI_SVC_OMA;
      WORD msgID = (WORD)eQMI_OMA_SET_EVENT;
      sSharedBuffer * pReq = 0;

      std::vector <sDB2PackingInput> piv;
      sProtocolEntityKey pek( eDB2_ET_QMI_OMA_REQ, msgID, 17 );

      if (bOn == true)
      {
         sDB2PackingInput pi( pek, "1" ); 
         piv.push_back( pi );
      }
      else
      {
         sDB2PackingInput pi( pek, "0" ); 
         piv.push_back( pi );
      }

      pReq = DB2PackQMIBuffer( mDB, piv );

      rc = SendAndCheckReturn( svc, pReq );
      if (rc == eGOBI_ERR_NONE || bOff == true)
      {
         mpFNOMADMState = pCallback;
      }
   }
   else if (bReplace == true)
   {
      // We don't have to register for anything so a simple assignment works
      mpFNOMADMState = pCallback;
      rc = eGOBI_ERR_NONE;
   }
   else
   {
      // Turning it off redundantly
      rc = eGOBI_ERR_NONE;
   }

   return rc;
}

/*===========================================================================
METHOD:
   SetUSSDReleaseCallback (Public Method)

DESCRIPTION:
   Enable/disable USSD release callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetUSSDReleaseCallback( 
   tFNUSSDRelease             pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNUSSDRelease = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetUSSDNotificationCallback (Public Method)

DESCRIPTION:
   Enable/disable USSD notification callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetUSSDNotificationCallback( 
   tFNUSSDNotification        pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNUSSDNotification = pCallback;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetUSSDOriginationCallback (Public Method)

DESCRIPTION:
   Enable/disable USSD origination callback function
  
PARAMETERS:
   pCallback   [ I ] - Callback function

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetUSSDOriginationCallback( 
   tFNUSSDOrigination         pCallback )
{
   // We don't have to register for anything so a simple assignment works
   mpFNUSSDOrigination = pCallback;
   return eGOBI_ERR_NONE;
}

