/*===========================================================================
FILE: 
   GobiConnectionMgmt.cpp

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
CGobiConnectionMgmtDLL gDLL;

// Interval between traffic processing loop iterations (milliseconds)
const ULONG TRAFFIC_INTERVAL_MS = 300000;

// Maximum amount of time to wait for the traffic thread to exit
const ULONG THREAD_EXIT_TIME = 2000;

/*===========================================================================
METHOD:
   TrafficProcessThread (Free Method)
   
DESCRIPTION:
   QMI traffic process thread - processes all traffic in order to fire
   off QMI traffic related callbacks

PARAMETERS:
   pArg        [ I ] - Object to interface to

RETURN VALUE:
   void * - thread exit value (always 0)
===========================================================================*/
void * TrafficProcessThread( PVOID pArg )
{  
   // Keep running?
   bool bRun = false;

   // Create a vector of the objects to wait on
   std::vector <cEvent *> events;

   // Store the index to service type for use later
   std::map <DWORD, eQMIService> services;

   // Grab API object
   cGobiConnectionMgmt * pAPI = (cGobiConnectionMgmt *)pArg;
   if (pAPI != 0)
   {
      // Time to go to work
      bRun = true;

      // Add the thread exit event
      events.push_back( &pAPI->mExitEvent );

      // Grab signal events for our protocol servers
      std::map <eQMIService, cGobiQMICore::sServerInfo>::const_iterator pIter;
      pIter = pAPI->mServers.begin();
      while (pIter != pAPI->mServers.end())
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

   TRACE( "GobiConnectionMgmt traffic thread [%u] started\n", 
          (UINT)pthread_self() );

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

   if (pAPI != 0)
   {
      pAPI->mThreadCleanupFinished = true;
   }

   return 0;
}

/*===========================================================================
METHOD:
   CallbackThread (Free Method)
   
DESCRIPTION:
   Thread to execute a callback asynchronously

PARAMETERS:
   pArg        [ I ] - The cGobiCMCallback object

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
// CGobiConnectionMgmtDLL Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   CGobiConnectionMgmtDLL (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
CGobiConnectionMgmtDLL::CGobiConnectionMgmtDLL()
{
   // Create sync CS
   pthread_mutex_init( &mSyncSection, NULL );
}

/*===========================================================================
METHOD:
   ~CGobiConnectionMgmtDLL (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
CGobiConnectionMgmtDLL::~CGobiConnectionMgmtDLL()
{
   std::map <GOBIHANDLE, cGobiConnectionMgmt *> tmpAPI = mAPI;
   std::map <GOBIHANDLE, cGobiConnectionMgmt *>::const_iterator pIter;
   pIter = tmpAPI.begin();

   while (pIter != tmpAPI.end())
   {
      cGobiConnectionMgmt * pAPI = pIter->second;
      if (pAPI != 0)
      {
         pAPI->Cleanup();
         delete pAPI;
      }

      pIter++;
   }

   mAPI.clear();

   pthread_mutex_destroy( &mSyncSection );
}

/*===========================================================================
METHOD:
   CreateAPI (Public Method)

DESCRIPTION:
   Create a new API object 
 
RETURN VALUE:
   GOBIHANDLE - Handle to new API object (0 upon failure)
===========================================================================*/ 
GOBIHANDLE CGobiConnectionMgmtDLL::CreateAPI()
{
   pthread_mutex_lock( &mSyncSection );

   cGobiConnectionMgmt * pAPI = new cGobiConnectionMgmt;
   if (pAPI != 0)
   {
      bool bInit = pAPI->Initialize();
      if (bInit == true)
      {
         mAPI[(GOBIHANDLE)pAPI] = pAPI;
      }
   }

   pthread_mutex_unlock( &mSyncSection );

   return (GOBIHANDLE)pAPI;
}

/*===========================================================================
METHOD:
   DeleteAPI (Public Method)

DESCRIPTION:
   Delete an existing API object

PARAMETERS:
   handle      [ I ] - Handle to API object to return
 
RETURN VALUE:
   None
===========================================================================*/ 
void CGobiConnectionMgmtDLL::DeleteAPI( GOBIHANDLE handle )
{
   pthread_mutex_lock( &mSyncSection );

   std::map <GOBIHANDLE, cGobiConnectionMgmt *>::iterator pIter;
   pIter = mAPI.find( handle );
   if (pIter != mAPI.end())
   {
      cGobiConnectionMgmt * pAPI = pIter->second;
      delete pAPI;

      mAPI.erase( pIter );
   }

   pthread_mutex_unlock( &mSyncSection );
}

/*===========================================================================
METHOD:
   GetAPI (Public Method)

DESCRIPTION:
   Return the requested API object

PARAMETERS:
   handle      [ I ] - Handle to API object to return
  
RETURN VALUE:
   cGobiConnectionMgmt *
===========================================================================*/ 
cGobiConnectionMgmt * CGobiConnectionMgmtDLL::GetAPI( GOBIHANDLE handle )
{
   cGobiConnectionMgmt * pAPI = 0;

   pthread_mutex_lock( &mSyncSection );

   std::map <GOBIHANDLE, cGobiConnectionMgmt *>::const_iterator pIter;
   pIter = mAPI.find( handle );
   if (pIter != mAPI.end())
   {
      pAPI = pIter->second;
   }

   pthread_mutex_unlock( &mSyncSection );

   return pAPI;
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
      mThreadCleanupFinished( false )
{
   // Nothing to do but init those variables
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

   std::map <eQMIService, sServerInfo>::iterator pIter;
   pIter = mServers.find( svc );
   if (pIter == mServers.end())
   {
      return;
   }

   sServerInfo & si = pIter->second;
   cQMIProtocolServer * pSvr = si.mpServer;
   if (pSvr == 0)
   {
      return;
   }

   // Grab the service ID from the service
   eQMIService svcID = pSvr->GetServiceType(); 
   if (svcID == eQMI_SVC_ENUM_BEGIN)
   {
      return;
   }

   // Grab the log from the server
   const cProtocolLog & logSvr = pSvr->GetLog();

   // New items to process?
   count = logSvr.GetCount();
   if (count != INVALID_LOG_INDEX && count > si.mLogsProcessed)
   {
      for (ULONG i = si.mLogsProcessed; i < count; i++)
      {
         sProtocolBuffer buf = logSvr.GetBuffer( i );
         if (buf.IsValid() == false)
         {
            continue;
         }

         eProtocolType pt = buf.GetType();
         if (IsQMIProtocolRX( pt ) == false)
         {
            continue;
         }

         sQMIServiceBuffer qmiBuf( buf.GetSharedBuffer() );
         if (qmiBuf.IsIndication() == false)
         {
            continue;
         }
               
         ULONG msgID = qmiBuf.GetMessageID();

         tCallbackKey ck( svcID, msgID );
         std::map <tCallbackKey, tCallbackValue>::iterator pIter;
         pIter = mCallbacks.find( ck );
         if (pIter == mCallbacks.end())
         {
            continue;
         }

         ULONG outLen = 0;
         const BYTE * pOutput = (const BYTE *)qmiBuf.GetRawContents( outLen );
         tCallbackValue cv = pIter->second;

         cGenericCallback * pCB = 0;
         pCB = new cGenericCallback( cv.first, 
                                     svcID,
                                     msgID,
                                     cv.second,
                                     outLen,
                                     pOutput );

         if (pCB != 0)
         {
            if (pCB->Initialize() == false)
            {
               delete pCB;
            }
         }
      }

      si.mLogsProcessed = count;
   }
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified Gobi device
  
PARAMETERS:
   pQMIFile    [ I ] - QMI control file to connect to
   services    [ I ] - QMI services to connect to

RETURN VALUE:
   std::set <eQMIService> - Services successfuly configured
===========================================================================*/
std::set <eQMIService> cGobiConnectionMgmt::Connect( 
   LPCSTR                     pQMIFile,
   std::set <eQMIService> &   services )
{
   std::set <eQMIService> svcs = cGobiQMICore::Connect( pQMIFile, services );
   if (svcs.size() > 0)
   {
      // Start the traffic processing thread?
      if (mbThreadStarted == false)
      {
         // Clear mExitEvent;
         mExitEvent.Clear();

         pthread_create( &mThreadID,
                         NULL,
                         TrafficProcessThread,
                         this );
                      
         mbThreadStarted = true;
      }
   }

   return svcs;
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
   // Clear all callback function pointers
   mCallbacks.clear();

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

   return cGobiQMICore::Disconnect();
}

/*===========================================================================
METHOD:
   SetGenericCallback (Public Method)

DESCRIPTION:
   Enable/disable generic callback function
  
PARAMETERS:
   svcID       [ I ] - Service ID to monitor
   msgID       [ I ] - Message ID to look for
   pCallback   [ I ] - Generic callback pointer
   userValue   [ I ] - User value to pass back to callback

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiConnectionMgmt::SetGenericCallback( 
   ULONG                      svcID,
   ULONG                      msgID,
   tFNGenericCallback         pCallback,
   ULONG_PTR                  userValue )
{
   // Assume success
   eGobiError rc = eGOBI_ERR_NONE;

   tCallbackKey ck( svcID, msgID );
   std::map <tCallbackKey, tCallbackValue>::iterator pIter;
   pIter = mCallbacks.find( ck );

   bool bOn = (pCallback != 0 && pIter == mCallbacks.end());
   bool bOff = (pCallback == 0 && pIter != mCallbacks.end());
   bool bReplace = (pCallback != 0 && pIter != mCallbacks.end());
   if (bOn == true || bReplace == true)
   {
      tCallbackValue cv( pCallback, userValue );
      mCallbacks[ck] = cv;
   }
   else if (bOff == true) 
   {
      mCallbacks.erase( pIter );
   }

   return rc;
}
