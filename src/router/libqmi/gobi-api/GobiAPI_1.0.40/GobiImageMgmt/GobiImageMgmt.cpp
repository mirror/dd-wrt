/*===========================================================================
FILE: 
   GobiQMIImageMgmt.cpp

DESCRIPTION:
   QUALCOMM Image Management API for Gobi 3000

PUBLIC CLASSES AND FUNCTIONS:
   cGobiImageMgmtDLL
   cGobiImageMgmt

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
#include "GobiImageMgmt.h"

#include "QMIBuffers.h"
#include "QDLBuffers.h"


// Global object
cGobiImageMgmtDLL gImageDLL;

/*=========================================================================*/
// cGobiImageMgmtDLL Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiImageMgmtDLL (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiImageMgmtDLL::cGobiImageMgmtDLL()
   :  mpAPI( 0 ),
      mbAllocated( false )
{
   // Create sync CS
   pthread_mutex_init( &mSyncSection, NULL );
}

/*===========================================================================
METHOD:
   ~cGobiImageMgmtDLL (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiImageMgmtDLL::~cGobiImageMgmtDLL()
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
   Return the cGobiImageMgmt object
  
RETURN VALUE:
   cGobiImageMgmt *
===========================================================================*/ 
cGobiImageMgmt * cGobiImageMgmtDLL::GetAPI()
{
   pthread_mutex_lock( &mSyncSection );

   bool bAlloc = mbAllocated;

   pthread_mutex_unlock( &mSyncSection );

   if (bAlloc == true)
   {
      return mpAPI;
   }

   pthread_mutex_lock( &mSyncSection );

   mpAPI = new cGobiImageMgmt;
   if (mpAPI != 0)
   {
      bool bAPI = mpAPI->Initialize();
      if (bAPI == false)
      {
         delete mpAPI;
         mpAPI = 0;
      }
   }

   // We have tried to allocate/initialize the object
   mbAllocated = true;

   pthread_mutex_unlock( &mSyncSection );
   return mpAPI;
}

/*=========================================================================*/
// cGobiImageMgmt Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiImageMgmt (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiImageMgmt::cGobiImageMgmt()
   :  cGobiQMICore(),
      mTargetDeviceNode( "" ),
      mTargetDeviceKey( "" ),
      mQDL()
{
   // We require a DMS server
   tServerConfig dmsSvr( eQMI_SVC_DMS, true );
   mServerConfig.insert( dmsSvr );
}

/*===========================================================================
METHOD:
   ~cGobiImageMgmt (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiImageMgmt::~cGobiImageMgmt()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Initialize the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiImageMgmt::Initialize()
{
   bool bRC = cGobiQMICore::Initialize();
   if (bRC == false)
   {
      return bRC;
   }

   bRC = mQDL.Initialize();
   return bRC;
}

/*===========================================================================
METHOD:
   Cleanup (Public Method)

DESCRIPTION:
   Cleanup the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiImageMgmt::Cleanup()
{
   mQDL.Cleanup();
   return cGobiQMICore::Cleanup();
}

/*===========================================================================
METHOD:
   GetDeviceID (Public Method)

DESCRIPTION:
   Set the ID of the device to target
  
PARAMETERS:
   deviceID    [ I ] - The device ID as reported by Windows
   deviceKey   [ I ] - The device key (unique, stored on-device)

RETURN VALUE:
   void
===========================================================================*/
void cGobiImageMgmt::GetDeviceID(
   std::string &                 deviceID,
   std::string &                 deviceKey )
{
   deviceID = mTargetDeviceNode;
   deviceKey = mTargetDeviceKey;
}

/*===========================================================================
METHOD:
   SetDeviceID (Public Method)

DESCRIPTION:
   Set the ID of the device to target
  
PARAMETERS:
   pDeviceID   [ I ] - The device ID as reported by Windows
   pDeviceKey  [ I ] - The device key (unique, stored on-device)

RETURN VALUE:
   bool
===========================================================================*/
bool cGobiImageMgmt::SetDeviceID(
   LPCSTR                     pDeviceID,
   LPCSTR                     pDeviceKey )
{
   // Clear last error recorded
   ClearLastError();

   // If you specify a device key then you have to specify a device ID
   if (pDeviceID == 0 && pDeviceKey != 0)
   {
      mLastError = eGOBI_ERR_INVALID_ARG;
      return false;
   }

   if (pDeviceID == 0 || pDeviceID[0] == 0)
   {
      mTargetDeviceNode.clear();
      mTargetDeviceKey.clear();
   }
   else
   {
      mTargetDeviceNode = pDeviceID;
      if (pDeviceKey == 0 || pDeviceKey[0] == 0)
      {
         mTargetDeviceKey.clear();
      }
      else
      {
         mTargetDeviceKey = pDeviceKey;
      }
   }

   return true;
}

/*===========================================================================
METHOD:
   ResetDevice (Public Method)

DESCRIPTION:
   This function resets the device

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiImageMgmt::ResetDevice()
{
   WORD msgID = (WORD)eQMI_DMS_SET_OPERATING_MODE;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, "5" );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}
