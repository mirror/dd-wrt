/*===========================================================================
FILE: 
   GobiConnectionMgmtExports.cpp

DESCRIPTION:
   QUALCOMM Gobi Connection Management API exports

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

/*=========================================================================*/
// Exported Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GobiConnect

DESCRIPTION:
   This function connects the CM API library to the specified Gobi 
   device

PARAMETERS:
   pQMIFile       [ I ] - Device interface to connect to
   pServicesCount [I/O] - Upon input the number of QMI services to connect to,
                          upon output the number of QMI services successfully
                          connected to
   pServices      [I/O] - Upon input the array of QMI service IDs to connect 
                          to, upon output the array of QMI service IDs 
                          successfully connected to
   pHandle        [ O ] - The returned Gobi interface handle

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GobiConnect(
   LPCSTR                     pInterface,
   ULONG *                    pServicesCount,
   ULONG *                    pServices,
   GOBIHANDLE *               pHandle )
{
   // Validate arguments
   if ( (pInterface == 0) 
   ||   (pServicesCount == 0) 
   ||   (*pServicesCount == 0) 
   ||   (pServices == 0)
   ||   (pHandle == 0) )
   {
      return (ULONG)eGOBI_ERR_INVALID_ARG;
   }

   GOBIHANDLE handle = gDLL.CreateAPI();
   if (handle == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcCount = *pServicesCount;
   *pServicesCount = 0;
   *pHandle = 0;

   std::set <eQMIService> inSvcs;
   std::set <eQMIService> outSvcs;
   
   ULONG s = 0;
   for (s = 0; s < svcCount; s++)
   {
      inSvcs.insert( (eQMIService)pServices[s] );
   }

   outSvcs = pAPI->Connect( pInterface, inSvcs );

   ULONG outSvcsCount = (ULONG)outSvcs.size();
   if (outSvcsCount > svcCount)
   {
      outSvcsCount = svcCount;
   }

   if (outSvcsCount == 0)
   {
      ULONG rc = (ULONG)pAPI->GetCorrectedLastError();
      pAPI = 0;

      gDLL.DeleteAPI( handle );
      return rc;
   }

   std::set <eQMIService>::const_iterator pOutSvc = outSvcs.begin();
   for (s = 0; s < svcCount; s++)
   {
      pServices[s] = UCHAR_MAX; 
      if (s < outSvcsCount)
      {
         pServices[s] = (ULONG)*pOutSvc++;
      }
   }

   *pHandle = handle;
   *pServicesCount = outSvcsCount;

   return (ULONG)eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GobiCancel

DESCRIPTION:
   This function cancels the most recent outstanding request for the
   specified QMI service
  
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   svcID       [ I ] - Service whose outstanding request is to be cancelled
   pTXID       [ O ] - QMI transaction ID of outstanding request

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GobiCancel( 
   GOBIHANDLE                 handle,
   ULONG                      svcID,
   ULONG *                    pTXID )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->CancelSend( svcID, pTXID );
}

/*===========================================================================
METHOD:
   GobiDisconnect

DESCRIPTION:
   This function disconnects the CM API library from the currently 
   connected Gobi device

PARAMETERS:
   handle      [ I ] - Gobi interface handle
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG GobiDisconnect( GOBIHANDLE handle )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   bool bDisco = pAPI->Disconnect();
   if (bDisco == false)
   {
      return (ULONG)pAPI->GetCorrectedLastError();
   }

   return (ULONG)eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetGenericCallback

DESCRIPTION:
   This function enables/disables a generic callback
  
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   svcID       [ I ] - Service ID to monitor
   msgID       [ I ] - Message ID to look for
   pCallback   [ I ] - Callback function

RETURN VALUE:
   ULONG
===========================================================================*/
ULONG SetGenericCallback( 
   GOBIHANDLE                 handle,
   ULONG                      svcID,
   ULONG                      msgID,
   tFNGenericCallback         pCallback )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   return (ULONG)pAPI->SetGenericCallback( svcID,
                                           msgID,
                                           pCallback,
                                           handle );
}

/*===========================================================================
METHOD:
   WDSReset
   
DESCRIPTION:
   The function sends 'WDS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetEventReport
   
DESCRIPTION:
   The function sends 'WDS/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSAbort
   
DESCRIPTION:
   The function sends 'WDS/Abort Request' (0x0002)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSAbort( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 2;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetIndication
   
DESCRIPTION:
   The function sends 'WDS/Set Indication Request' (0x0003)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetIndication( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 3;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSStartNetworkInterface
   
DESCRIPTION:
   The function sends 'WDS/Start Network Interface Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSStartNetworkInterface( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSStopNetworkInterface
   
DESCRIPTION:
   The function sends 'WDS/Stop Network Interface Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSStopNetworkInterface( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetPacketServiceStatus
   
DESCRIPTION:
   The function sends 'WDS/Get Packet Service Status Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetPacketServiceStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetChannelRates
   
DESCRIPTION:
   The function sends 'WDS/Get Channel Rates Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetChannelRates( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetPacketStatistics
   
DESCRIPTION:
   The function sends 'WDS/Get Packet Statistics Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetPacketStatistics( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGoDormant
   
DESCRIPTION:
   The function sends 'WDS/Go Dormant Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGoDormant( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGoActive
   
DESCRIPTION:
   The function sends 'WDS/Go Active Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGoActive( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSCreateProfile
   
DESCRIPTION:
   The function sends 'WDS/Create Profile Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSCreateProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSModifyProfile
   
DESCRIPTION:
   The function sends 'WDS/Modify Profile Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSModifyProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSDeleteProfile
   
DESCRIPTION:
   The function sends 'WDS/Delete Profile Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSDeleteProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetProfileList
   
DESCRIPTION:
   The function sends 'WDS/Get Profile List Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetProfileList( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetProfileSettings
   
DESCRIPTION:
   The function sends 'WDS/Get Profile Settings Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetProfileSettings( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDefaultSettings
   
DESCRIPTION:
   The function sends 'WDS/Get Default Settings Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDefaultSettings( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCurrentSettings
   
DESCRIPTION:
   The function sends 'WDS/Get Current Settings Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCurrentSettings( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetMIPMode
   
DESCRIPTION:
   The function sends 'WDS/Set MIP Mode Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetMIPMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetMIPMode
   
DESCRIPTION:
   The function sends 'WDS/Get MIP Mode Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetMIPMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDormancy
   
DESCRIPTION:
   The function sends 'WDS/Get Dormancy Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDormancy( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetAutoconnectSetting
   
DESCRIPTION:
   The function sends 'WDS/Get Autoconnect Setting Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetAutoconnectSetting( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDataSessionDuration
   
DESCRIPTION:
   The function sends 'WDS/Get Data Session Duration Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDataSessionDuration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetModemStatus
   
DESCRIPTION:
   The function sends 'WDS/Get Modem Status Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetModemStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDataBearerTechnology
   
DESCRIPTION:
   The function sends 'WDS/Get Data Bearer Technology Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDataBearerTechnology( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetModemInfo
   
DESCRIPTION:
   The function sends 'WDS/Get Modem Info Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetModemInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetActiveMIPProfile
   
DESCRIPTION:
   The function sends 'WDS/Get Active MIP Profile Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetActiveMIPProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetActiveMIPProfile
   
DESCRIPTION:
   The function sends 'WDS/Set Active MIP Profile Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetActiveMIPProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetMIPProfile
   
DESCRIPTION:
   The function sends 'WDS/Get MIP Profile Request' (0x003E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetMIPProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 62;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetMIPProfile
   
DESCRIPTION:
   The function sends 'WDS/Set MIP Profile Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetMIPProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetMIPParameters
   
DESCRIPTION:
   The function sends 'WDS/Get MIP Parameters Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetMIPParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetMIPParameters
   
DESCRIPTION:
   The function sends 'WDS/Set MIP Parameters Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetMIPParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetLastMIPStatus
   
DESCRIPTION:
   The function sends 'WDS/Get Last MIP Status Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetLastMIPStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetANAAAAuthenticationStatus
   
DESCRIPTION:
   The function sends 'WDS/Get AN-AAA Authentication Status Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetANAAAAuthenticationStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCurrentDataBearerTechnology
   
DESCRIPTION:
   The function sends 'WDS/Get Current Data Bearer Technology Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCurrentDataBearerTechnology( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCallList
   
DESCRIPTION:
   The function sends 'WDS/Get Call List Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCallList( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCallRecord
   
DESCRIPTION:
   The function sends 'WDS/Get Call Record Request' (0x0046)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCallRecord( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 70;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSClearCallList
   
DESCRIPTION:
   The function sends 'WDS/Clear Call List Request' (0x0047)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSClearCallList( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 71;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCallListMaxSize
   
DESCRIPTION:
   The function sends 'WDS/Get Call List Max Size Request' (0x0048)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCallListMaxSize( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 72;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDefaultProfileNumber
   
DESCRIPTION:
   The function sends 'WDS/Get Default Profile Number Request' (0x0049)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDefaultProfileNumber( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 73;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetDefaultProfileNumber
   
DESCRIPTION:
   The function sends 'WDS/Set Default Profile Number Request' (0x004A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetDefaultProfileNumber( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 74;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSResetProfile
   
DESCRIPTION:
   The function sends 'WDS/Reset Profile Request' (0x004B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSResetProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 75;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSResetProfileParamToInvalid
   
DESCRIPTION:
   The function sends 'WDS/Reset Profile Param To Invalid Request' (0x004C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSResetProfileParamToInvalid( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 76;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetIPFamilyPreference
   
DESCRIPTION:
   The function sends 'WDS/Set IP Family Preference Request' (0x004D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetIPFamilyPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 77;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetFMCTunnelParameters
   
DESCRIPTION:
   The function sends 'WDS/Set FMC Tunnel Parameters Request' (0x004E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetFMCTunnelParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 78;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSClearFMCTunnelParameters
   
DESCRIPTION:
   The function sends 'WDS/Clear FMC Tunnel Parameters Request' (0x004F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSClearFMCTunnelParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 79;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetFMCTunnelParameters
   
DESCRIPTION:
   The function sends 'WDS/Get FMC Tunnel Parameters Request' (0x0050)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetFMCTunnelParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 80;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetAutoconnectSetting
   
DESCRIPTION:
   The function sends 'WDS/Set Autoconnect Setting Request' (0x0051)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetAutoconnectSetting( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 81;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDNSSetting
   
DESCRIPTION:
   The function sends 'WDS/Get DNS Setting Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDNSSetting( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetDNSSetting
   
DESCRIPTION:
   The function sends 'WDS/Set DNS Setting Request' (0x0053)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetDNSSetting( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 83;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCDMAPreDormancySettings
   
DESCRIPTION:
   The function sends 'WDS/Get CDMA Pre-Dormancy Settings Request' (0x0054)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCDMAPreDormancySettings( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 84;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetCAMTimer
   
DESCRIPTION:
   The function sends 'WDS/Set CAM Timer Request' (0x0055)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetCAMTimer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 85;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCAMTimer
   
DESCRIPTION:
   The function sends 'WDS/Get CAM Timer Request' (0x0056)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCAMTimer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 86;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetSCRM
   
DESCRIPTION:
   The function sends 'WDS/Set SCRM Request' (0x0057)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetSCRM( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 87;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetSCRM
   
DESCRIPTION:
   The function sends 'WDS/Get SCRM Request' (0x0058)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetSCRM( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 88;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetRDUD
   
DESCRIPTION:
   The function sends 'WDS/Set RDUD Request' (0x0059)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetRDUD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 89;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetRDUD
   
DESCRIPTION:
   The function sends 'WDS/Get RDUD Request' (0x005A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetRDUD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 90;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetSIPMIPCallType
   
DESCRIPTION:
   The function sends 'WDS/Get SIP/MIP Call Type Request' (0x005B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetSIPMIPCallType( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 91;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetEVDOPageMonitorPeriod
   
DESCRIPTION:
   The function sends 'WDS/Set EV-DO Page Monitor Period Request' (0x005C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetEVDOPageMonitorPeriod( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 92;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetEVDOLongSleep
   
DESCRIPTION:
   The function sends 'WDS/Set EV-DO Long Sleep Request' (0x005D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetEVDOLongSleep( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 93;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetEVDOPageMonitorPeriod
   
DESCRIPTION:
   The function sends 'WDS/Get EV-DO Page Monitor Period Request' (0x005E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetEVDOPageMonitorPeriod( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 94;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCallThrottleInfo
   
DESCRIPTION:
   The function sends 'WDS/Get Call Throttle Info Request' (0x005F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCallThrottleInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 95;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetNSAPI
   
DESCRIPTION:
   The function sends 'WDS/Get NSAPI Request' (0x0060)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetNSAPI( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 96;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetDUNCallControlPreference
   
DESCRIPTION:
   The function sends 'WDS/Set DUN Call Control Preference Request' (0x0061)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetDUNCallControlPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 97;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetDUNCallControlInfo
   
DESCRIPTION:
   The function sends 'WDS/Get DUN Call Control Info Request' (0x0062)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetDUNCallControlInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 98;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSSetDUNCallControlEventReport
   
DESCRIPTION:
   The function sends 'WDS/Set DUN Call Control Event Report Request' (0x0063)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSSetDUNCallControlEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 99;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSPendingDUNCallControl
   
DESCRIPTION:
   The function sends 'WDS/Pending DUN Call Control Request' (0x0064)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSPendingDUNCallControl( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 100;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSEMBMSTMGIActivate
   
DESCRIPTION:
   The function sends 'WDS/EMBMS TMGI Activate Request' (0x0065)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSEMBMSTMGIActivate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 101;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSEMBMSTMGIDeactivate
   
DESCRIPTION:
   The function sends 'WDS/EMBMS TMGI Deactivate Request' (0x0066)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSEMBMSTMGIDeactivate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 102;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSEMBMSTMGIListQuery
   
DESCRIPTION:
   The function sends 'WDS/EMBMS TMGI List Query Request' (0x0067)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSEMBMSTMGIListQuery( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 103;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetPreferredDataSystem
   
DESCRIPTION:
   The function sends 'WDS/Get Preferred Data System Request' (0x0069)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetPreferredDataSystem( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 105;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetLastDataCallStatus
   
DESCRIPTION:
   The function sends 'WDS/Get Last Data Call Status Request' (0x006A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetLastDataCallStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 106;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetCurrentDataSystems
   
DESCRIPTION:
   The function sends 'WDS/Get Current Data Systems Request' (0x006B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetCurrentDataSystems( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 107;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetPDNThrottleInfo
   
DESCRIPTION:
   The function sends 'WDS/Get PDN Throttle Info Request' (0x006C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetPDNThrottleInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 108;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetLTEAttachParameters
   
DESCRIPTION:
   The function sends 'WDS/Get LTE Attach Parameters Request' (0x0085)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetLTEAttachParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 133;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSResetPacketStatistics
   
DESCRIPTION:
   The function sends 'WDS/Reset Packet Statistics Request' (0x0086)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSResetPacketStatistics( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 134;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WDSGetFlowControlStatus
   
DESCRIPTION:
   The function sends 'WDS/Get Flow Control Status Request' (0x0087)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WDSGetFlowControlStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 1;
   ULONG msgID = 135;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSReset
   
DESCRIPTION:
   The function sends 'DMS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetEventReport
   
DESCRIPTION:
   The function sends 'DMS/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceCapabilities
   
DESCRIPTION:
   The function sends 'DMS/Get Device Capabilities Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceCapabilities( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceManfacturer
   
DESCRIPTION:
   The function sends 'DMS/Get Device Manfacturer Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceManfacturer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceModel
   
DESCRIPTION:
   The function sends 'DMS/Get Device Model Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceModel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceRevision
   
DESCRIPTION:
   The function sends 'DMS/Get Device Revision Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceRevision( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceVoiceNumber
   
DESCRIPTION:
   The function sends 'DMS/Get Device Voice Number Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceVoiceNumber( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetDeviceSerialNumbers
   
DESCRIPTION:
   The function sends 'DMS/Get Device Serial Numbers Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetDeviceSerialNumbers( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetPowerState
   
DESCRIPTION:
   The function sends 'DMS/Get Power State Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetPowerState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMSetPINProtection
   
DESCRIPTION:
   The function sends 'DMS/UIM Set PIN Protection Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMSetPINProtection( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMVerifyPIN
   
DESCRIPTION:
   The function sends 'DMS/UIM Verify PIN Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMVerifyPIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMUnblockPIN
   
DESCRIPTION:
   The function sends 'DMS/UIM Unblock PIN Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMUnblockPIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMChangePIN
   
DESCRIPTION:
   The function sends 'DMS/UIM Change PIN Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMChangePIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMGetPINStatus
   
DESCRIPTION:
   The function sends 'DMS/UIM Get PIN Status Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMGetPINStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetHardwareRevision
   
DESCRIPTION:
   The function sends 'DMS/Get Hardware Revision Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetHardwareRevision( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetOperatingMode
   
DESCRIPTION:
   The function sends 'DMS/Get Operating Mode Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetOperatingMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetOperatingMode
   
DESCRIPTION:
   The function sends 'DMS/Set Operating Mode Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetOperatingMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetTimestamp
   
DESCRIPTION:
   The function sends 'DMS/Get Timestamp Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetTimestamp( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetPRLVersion
   
DESCRIPTION:
   The function sends 'DMS/Get PRL Version Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetPRLVersion( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetActivationState
   
DESCRIPTION:
   The function sends 'DMS/Get Activation State Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetActivationState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSActivateAutomatic
   
DESCRIPTION:
   The function sends 'DMS/Activate Automatic Request' (0x0032)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSActivateAutomatic( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 50;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSActivateManual
   
DESCRIPTION:
   The function sends 'DMS/Activate Manual Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSActivateManual( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetLockState
   
DESCRIPTION:
   The function sends 'DMS/Get Lock State Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetLockState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetLockState
   
DESCRIPTION:
   The function sends 'DMS/Set Lock State Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetLockState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetLockCode
   
DESCRIPTION:
   The function sends 'DMS/Set Lock Code Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetLockCode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSReadUserData
   
DESCRIPTION:
   The function sends 'DMS/Read User Data Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSReadUserData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSWriteUserData
   
DESCRIPTION:
   The function sends 'DMS/Write User Data Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSWriteUserData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSReadERIData
   
DESCRIPTION:
   The function sends 'DMS/Read ERI Data Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSReadERIData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSResetFactoryDefaults
   
DESCRIPTION:
   The function sends 'DMS/Reset Factory Defaults Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSResetFactoryDefaults( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSValidateSPC
   
DESCRIPTION:
   The function sends 'DMS/Validate SPC Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSValidateSPC( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMGetICCID
   
DESCRIPTION:
   The function sends 'DMS/UIM Get ICCID Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMGetICCID( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMGetHostLockID
   
DESCRIPTION:
   The function sends 'DMS/UIM Get Host Lock ID Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMGetHostLockID( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMGetControlKeyStatus
   
DESCRIPTION:
   The function sends 'DMS/UIM Get Control Key Status Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMGetControlKeyStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMSetControlKeyProtection
   
DESCRIPTION:
   The function sends 'DMS/UIM Set Control Key Protection Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMSetControlKeyProtection( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSUIMUnblockControlKey
   
DESCRIPTION:
   The function sends 'DMS/UIM Unblock Control Key Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSUIMUnblockControlKey( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetIMSI
   
DESCRIPTION:
   The function sends 'DMS/Get IMSI Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetIMSI( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetUIMState
   
DESCRIPTION:
   The function sends 'DMS/Get UIM State Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetUIMState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetBandCapabilities
   
DESCRIPTION:
   The function sends 'DMS/Get Band Capabilities Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetBandCapabilities( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetFactorySerialNumber
   
DESCRIPTION:
   The function sends 'DMS/Get Factory Serial Number Request' (0x0046)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetFactorySerialNumber( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 70;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetDeviceTime
   
DESCRIPTION:
   The function sends 'DMS/Set Device Time Request' (0x004B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetDeviceTime( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 75;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetSoftwareVersion
   
DESCRIPTION:
   The function sends 'DMS/Get Software Version Request' (0x0051)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetSoftwareVersion( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 81;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSSetSPC
   
DESCRIPTION:
   The function sends 'DMS/Set SPC Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSSetSPC( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   DMSGetCurrentPRLInfo
   
DESCRIPTION:
   The function sends 'DMS/Get Current PRL Info Request' (0x0053)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG DMSGetCurrentPRLInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 2;
   ULONG msgID = 83;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASReset
   
DESCRIPTION:
   The function sends 'NAS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASAbort
   
DESCRIPTION:
   The function sends 'NAS/Abort Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASAbort( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetEventReport
   
DESCRIPTION:
   The function sends 'NAS/Set Event Report Request' (0x0002)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 2;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetRegistrationEventReport
   
DESCRIPTION:
   The function sends 'NAS/Set Registration Event Report Request' (0x0003)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetRegistrationEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 3;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetSignalStrength
   
DESCRIPTION:
   The function sends 'NAS/Get Signal Strength Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetSignalStrength( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASPerformNetworkScan
   
DESCRIPTION:
   The function sends 'NAS/Perform Network Scan Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASPerformNetworkScan( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASInitiateNetworkRegister
   
DESCRIPTION:
   The function sends 'NAS/Initiate Network Register Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASInitiateNetworkRegister( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASInitiateAttach
   
DESCRIPTION:
   The function sends 'NAS/Initiate Attach Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASInitiateAttach( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetServingSystem
   
DESCRIPTION:
   The function sends 'NAS/Get Serving System Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetServingSystem( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetHomeNetwork
   
DESCRIPTION:
   The function sends 'NAS/Get Home Network Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetHomeNetwork( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetPreferredNetworks
   
DESCRIPTION:
   The function sends 'NAS/Get Preferred Networks Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetPreferredNetworks( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetPreferredNetworks
   
DESCRIPTION:
   The function sends 'NAS/Set Preferred Networks Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetPreferredNetworks( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetForbiddenNetworks
   
DESCRIPTION:
   The function sends 'NAS/Get Forbidden Networks Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetForbiddenNetworks( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetForbiddenNetworks
   
DESCRIPTION:
   The function sends 'NAS/Set Forbidden Networks Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetForbiddenNetworks( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetTechnologyPreference
   
DESCRIPTION:
   The function sends 'NAS/Set Technology Preference Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetTechnologyPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetTechnologyPreference
   
DESCRIPTION:
   The function sends 'NAS/Get Technology Preference Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetTechnologyPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetACCOLC
   
DESCRIPTION:
   The function sends 'NAS/Get ACCOLC Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetACCOLC( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetACCOLC
   
DESCRIPTION:
   The function sends 'NAS/Set ACCOLC Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetACCOLC( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetSystemPreference
   
DESCRIPTION:
   The function sends 'NAS/Get System Preference' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetSystemPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetNetworkParameters
   
DESCRIPTION:
   The function sends 'NAS/Get Network Parameters Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetNetworkParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetNetworkParameters
   
DESCRIPTION:
   The function sends 'NAS/Set Network Parameters Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetNetworkParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetRFInfo
   
DESCRIPTION:
   The function sends 'NAS/Get RF Info Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetRFInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetANAAAAuthenticationStatus
   
DESCRIPTION:
   The function sends 'NAS/Get AN-AAA Authentication Status Request' (0x0032)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetANAAAAuthenticationStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 50;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetSystemSelectionPref
   
DESCRIPTION:
   The function sends 'NAS/Set System Selection Pref Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetSystemSelectionPref( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetSystemSelectionPref
   
DESCRIPTION:
   The function sends 'NAS/Get System Selection Pref Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetSystemSelectionPref( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetDDTMPreference
   
DESCRIPTION:
   The function sends 'NAS/Set DDTM Preference Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetDDTMPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetDDTMPreference
   
DESCRIPTION:
   The function sends 'NAS/Get DDTM Preference Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetDDTMPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetOperatorNameData
   
DESCRIPTION:
   The function sends 'NAS/Get Operator Name Data Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetOperatorNameData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetCSPPLMNMode
   
DESCRIPTION:
   The function sends 'NAS/Get CSP PLMN Mode Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetCSPPLMNMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASUpdateAKEY
   
DESCRIPTION:
   The function sends 'NAS/Update AKEY Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASUpdateAKEY( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGet3GPP2SubscriptionInfo
   
DESCRIPTION:
   The function sends 'NAS/Get 3GPP2 Subscription Info Request' (0x003E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGet3GPP2SubscriptionInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 62;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSet3GPP2SubscriptionInfo
   
DESCRIPTION:
   The function sends 'NAS/Set 3GPP2 Subscription Info Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSet3GPP2SubscriptionInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetMobileCAIRevision
   
DESCRIPTION:
   The function sends 'NAS/Get Mobile CAI Revision Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetMobileCAIRevision( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetRTREConfig
   
DESCRIPTION:
   The function sends 'NAS/Get RTRE Config Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetRTREConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetRTREConfig
   
DESCRIPTION:
   The function sends 'NAS/Set RTRE Config Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetRTREConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetCellLocationInfo
   
DESCRIPTION:
   The function sends 'NAS/Get Cell Location Info Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetCellLocationInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetPLMNName
   
DESCRIPTION:
   The function sends 'NAS/Get PLMN Name Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetPLMNName( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASBindSubscription
   
DESCRIPTION:
   The function sends 'NAS/Bind Subscription Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASBindSubscription( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetModePref
   
DESCRIPTION:
   The function sends 'NAS/Get Mode Pref Request' (0x0049)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetModePref( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 73;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetDualStandbyPreference
   
DESCRIPTION:
   The function sends 'NAS/Set Dual Standby Preference Request' (0x004B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetDualStandbyPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 75;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetSystemInfo
   
DESCRIPTION:
   The function sends 'NAS/Get System Info Request' (0x004D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetSystemInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 77;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetSignalInfo
   
DESCRIPTION:
   The function sends 'NAS/Get Signal Info Request' (0x004F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetSignalInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 79;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASConfigureSignalInfo
   
DESCRIPTION:
   The function sends 'NAS/Configure Signal Info Request' (0x0050)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASConfigureSignalInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 80;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetErrorRate
   
DESCRIPTION:
   The function sends 'NAS/Get Error Rate Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetErrorRate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetEVDOProtocolSubtype
   
DESCRIPTION:
   The function sends 'NAS/Get EV-DO Protocol Subtype Request' (0x0056)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetEVDOProtocolSubtype( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 86;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetEVDOColorCode
   
DESCRIPTION:
   The function sends 'NAS/Get EV-DO Color Code Request' (0x0057)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetEVDOColorCode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 87;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetAcquisitionSystemMode
   
DESCRIPTION:
   The function sends 'NAS/Get Acquisition System Mode Request' (0x0058)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetAcquisitionSystemMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 88;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASSetRXDiversity
   
DESCRIPTION:
   The function sends 'NAS/Set RX Diversity Request' (0x0059)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASSetRXDiversity( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 89;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetRXTXInfo
   
DESCRIPTION:
   The function sends 'NAS/Get RX/TX Info Request' (0x005A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetRXTXInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 90;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASUpdateAKEYExtended
   
DESCRIPTION:
   The function sends 'NAS/Update A-KEY Extended Request' (0x005B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASUpdateAKEYExtended( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 91;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetDualStandbyPreference
   
DESCRIPTION:
   The function sends 'NAS/Get Dual Standby Preference Request' (0x005C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetDualStandbyPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 92;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASDetachLTE
   
DESCRIPTION:
   The function sends 'NAS/Detach LTE Request' (0x005D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASDetachLTE( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 93;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASBlockLTEPLMN
   
DESCRIPTION:
   The function sends 'NAS/Block LTE PLMN Request' (0x005E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASBlockLTEPLMN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 94;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASUnblockLTEPLMN
   
DESCRIPTION:
   The function sends 'NAS/Unblock LTE PLMN Request' (0x005F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASUnblockLTEPLMN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 95;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASResetLTEPLMNBlock
   
DESCRIPTION:
   The function sends 'NAS/Reset LTE PLMN Block Request' (0x0060)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASResetLTEPLMNBlock( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 96;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASConfigureEMBMS
   
DESCRIPTION:
   The function sends 'NAS/Configure EMBMS Request' (0x0062)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASConfigureEMBMS( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 98;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetEMBMSStatus
   
DESCRIPTION:
   The function sends 'NAS/Get EMBMS Status Request' (0x0063)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetEMBMSStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 99;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetCDMAPositionInfo
   
DESCRIPTION:
   The function sends 'NAS/Get CDMA Position Info Request' (0x0065)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetCDMAPositionInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 101;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASForceNetworkSearch
   
DESCRIPTION:
   The function sends 'NAS/Force Network Search Request' (0x0067)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASForceNetworkSearch( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 103;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetManagedRoamingConfig
   
DESCRIPTION:
   The function sends 'NAS/Get Managed Roaming Config Request' (0x0069)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetManagedRoamingConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 105;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetCentralizedEONSSupport
   
DESCRIPTION:
   The function sends 'NAS/Get Centralized EONS Support Request' (0x006B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetCentralizedEONSSupport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 107;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASConfigureSignalInfo2
   
DESCRIPTION:
   The function sends 'NAS/Configure Signal Info 2 Request' (0x006C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASConfigureSignalInfo2( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 108;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   NASGetTDSCDMACellInfo
   
DESCRIPTION:
   The function sends 'NAS/Get TD-SCDMA Cell Info Request' (0x006D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG NASGetTDSCDMACellInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 3;
   ULONG msgID = 109;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSReset
   
DESCRIPTION:
   The function sends 'WMS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetEventReport
   
DESCRIPTION:
   The function sends 'WMS/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSRawSend
   
DESCRIPTION:
   The function sends 'WMS/Raw Send Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSRawSend( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSRawWrite
   
DESCRIPTION:
   The function sends 'WMS/Raw Write Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSRawWrite( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSRawRead
   
DESCRIPTION:
   The function sends 'WMS/Raw Read Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSRawRead( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSModifyTag
   
DESCRIPTION:
   The function sends 'WMS/Modify Tag Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSModifyTag( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSDelete
   
DESCRIPTION:
   The function sends 'WMS/Delete Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSDelete( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetMessageProtocol
   
DESCRIPTION:
   The function sends 'WMS/Get Message Protocol Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetMessageProtocol( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSListMessages
   
DESCRIPTION:
   The function sends 'WMS/List Messages Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSListMessages( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetRoutes
   
DESCRIPTION:
   The function sends 'WMS/Set Routes Request' (0x0032)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetRoutes( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 50;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetRoutes
   
DESCRIPTION:
   The function sends 'WMS/Get Routes Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetRoutes( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetSMSCAddress
   
DESCRIPTION:
   The function sends 'WMS/Get SMSC Address Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetSMSCAddress( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetSMSCAddress
   
DESCRIPTION:
   The function sends 'WMS/Set SMSC Address Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetSMSCAddress( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetStorageMaxSize
   
DESCRIPTION:
   The function sends 'WMS/Get Storage Max Size Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetStorageMaxSize( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSendACK
   
DESCRIPTION:
   The function sends 'WMS/Send ACK Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSendACK( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetRetryPeriod
   
DESCRIPTION:
   The function sends 'WMS/Set Retry Period Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetRetryPeriod( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetRetryInterval
   
DESCRIPTION:
   The function sends 'WMS/Set Retry Interval Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetRetryInterval( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetDCDisconnectTimer
   
DESCRIPTION:
   The function sends 'WMS/Set DC Disconnect Timer Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetDCDisconnectTimer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetMemoryStatus
   
DESCRIPTION:
   The function sends 'WMS/Set Memory Status Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetMemoryStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetBroadcastActivation
   
DESCRIPTION:
   The function sends 'WMS/Set Broadcast Activation Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetBroadcastActivation( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetBroadcastConfig
   
DESCRIPTION:
   The function sends 'WMS/Set Broadcast Config Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetBroadcastConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetBroadcastConfig
   
DESCRIPTION:
   The function sends 'WMS/Get Broadcast Config Request' (0x003E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetBroadcastConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 62;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetDomainPreference
   
DESCRIPTION:
   The function sends 'WMS/Get Domain Preference Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetDomainPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetDomainPreference
   
DESCRIPTION:
   The function sends 'WMS/Set Domain Preference Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetDomainPreference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSendFromMemoryStore
   
DESCRIPTION:
   The function sends 'WMS/Send From Memory Store Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSendFromMemoryStore( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetWaitingMessage
   
DESCRIPTION:
   The function sends 'WMS/Get Waiting Message Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetWaitingMessage( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetPrimaryClient
   
DESCRIPTION:
   The function sends 'WMS/Set Primary Client Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetPrimaryClient( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSIndicatorRegistration
   
DESCRIPTION:
   The function sends 'WMS/Indicator Registration Request' (0x0047)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSIndicatorRegistration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 71;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetTransportLayerInfo
   
DESCRIPTION:
   The function sends 'WMS/Get Transport Layer Info Request' (0x0048)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetTransportLayerInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 72;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetNetworkRegistrationInfo
   
DESCRIPTION:
   The function sends 'WMS/Get Network Registration Info Request' (0x004A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetNetworkRegistrationInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 74;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSBindSubscription
   
DESCRIPTION:
   The function sends 'WMS/Bind Subscription Request' (0x004C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSBindSubscription( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 76;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetIndicatorRegistration
   
DESCRIPTION:
   The function sends 'WMS/Get Indicator Registration Request' (0x004D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetIndicatorRegistration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 77;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetSMSParameters
   
DESCRIPTION:
   The function sends 'WMS/Get SMS Parameters Request' (0x004E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetSMSParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 78;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetSMSParameters
   
DESCRIPTION:
   The function sends 'WMS/Set SMS Parameters Request' (0x004F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetSMSParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 79;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetDomainPreferenceConfig
   
DESCRIPTION:
   The function sends 'WMS/Get Domain Preference Config Request' (0x0051)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetDomainPreferenceConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 81;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSSetDomainPreferenceConfig
   
DESCRIPTION:
   The function sends 'WMS/Set Domain Preference Config Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSSetDomainPreferenceConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetRetryPeriod
   
DESCRIPTION:
   The function sends 'WMS/Get Retry Period Request' (0x0053)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetRetryPeriod( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 83;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetRetryInterval
   
DESCRIPTION:
   The function sends 'WMS/Get Retry Interval Request' (0x0054)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetRetryInterval( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 84;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetDCDisconnectTimer
   
DESCRIPTION:
   The function sends 'WMS/Get DC Disconnect Timer Request' (0x0055)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetDCDisconnectTimer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 85;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetMemoryStatus
   
DESCRIPTION:
   The function sends 'WMS/Get Memory Status Request' (0x0056)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetMemoryStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 86;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetPrimaryClient
   
DESCRIPTION:
   The function sends 'WMS/Get Primary Client Request' (0x0057)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetPrimaryClient( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 87;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetSubscriptionBinding
   
DESCRIPTION:
   The function sends 'WMS/Get Subscription Binding Request' (0x0058)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetSubscriptionBinding( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 88;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSAsyncRawSend
   
DESCRIPTION:
   The function sends 'WMS/Async Raw Send Request' (0x0059)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSAsyncRawSend( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 89;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSAsyncSendACK
   
DESCRIPTION:
   The function sends 'WMS/Async Send ACK Request' (0x005A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSAsyncSendACK( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 90;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSAsyncSendFromMemoryStore
   
DESCRIPTION:
   The function sends 'WMS/Async Send From Memory Store Request' (0x005B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSAsyncSendFromMemoryStore( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 91;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   WMSGetServiceReadyStatus
   
DESCRIPTION:
   The function sends 'WMS/Get Service Ready Status Request' (0x005C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG WMSGetServiceReadyStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 5;
   ULONG msgID = 92;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSReset
   
DESCRIPTION:
   The function sends 'PDS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetEventReport
   
DESCRIPTION:
   The function sends 'PDS/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetServiceState
   
DESCRIPTION:
   The function sends 'PDS/Get Service State Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetServiceState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetServiceState
   
DESCRIPTION:
   The function sends 'PDS/Set Service State Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetServiceState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSStartTrackingSession
   
DESCRIPTION:
   The function sends 'PDS/Start Tracking Session Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSStartTrackingSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetTrackingSessionInfo
   
DESCRIPTION:
   The function sends 'PDS/Get Tracking Session Info Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetTrackingSessionInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSFixPosition
   
DESCRIPTION:
   The function sends 'PDS/Fix Position Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSFixPosition( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSEndTrackingSession
   
DESCRIPTION:
   The function sends 'PDS/End Tracking Session Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSEndTrackingSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetNMEAConfig
   
DESCRIPTION:
   The function sends 'PDS/Get NMEA Config Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetNMEAConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetNMEAConfig
   
DESCRIPTION:
   The function sends 'PDS/Set NMEA Config Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetNMEAConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectTimeReference
   
DESCRIPTION:
   The function sends 'PDS/Inject Time Reference Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectTimeReference( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetDefaults
   
DESCRIPTION:
   The function sends 'PDS/Get Defaults Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetDefaults( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetDefaults
   
DESCRIPTION:
   The function sends 'PDS/Set Defaults Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetDefaults( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetXTRAParameters
   
DESCRIPTION:
   The function sends 'PDS/Get XTRA Parameters Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetXTRAParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetXTRAParameters
   
DESCRIPTION:
   The function sends 'PDS/Set XTRA Parameters Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetXTRAParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSForceXTRADownload
   
DESCRIPTION:
   The function sends 'PDS/Force XTRA Download Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSForceXTRADownload( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetAGPSConfig
   
DESCRIPTION:
   The function sends 'PDS/Get AGPS Config Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetAGPSConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetAGPSConfig
   
DESCRIPTION:
   The function sends 'PDS/Set AGPS Config Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetAGPSConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetServiceAutoTrackingState
   
DESCRIPTION:
   The function sends 'PDS/Get Service Auto-Tracking State Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetServiceAutoTrackingState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetServiceAutoTrackingState
   
DESCRIPTION:
   The function sends 'PDS/Set Service Auto-Tracking State Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetServiceAutoTrackingState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetCOMPortAutoTrackingConfig
   
DESCRIPTION:
   The function sends 'PDS/Get COM Port Auto-Tracking Config Request' (0x0032)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetCOMPortAutoTrackingConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 50;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetCOMPortAutoTrackingConfig
   
DESCRIPTION:
   The function sends 'PDS/Set COM Port Auto-Tracking Config Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetCOMPortAutoTrackingConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSResetPDSData
   
DESCRIPTION:
   The function sends 'PDS/Reset PDS Data Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSResetPDSData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSinglePositionFix
   
DESCRIPTION:
   The function sends 'PDS/Single Position Fix Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSinglePositionFix( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetServiceVersion
   
DESCRIPTION:
   The function sends 'PDS/Get Service Version Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetServiceVersion( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectXTRAData
   
DESCRIPTION:
   The function sends 'PDS/Inject XTRA Data Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectXTRAData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectPositionData
   
DESCRIPTION:
   The function sends 'PDS/Inject Position Data Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectPositionData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectWiFiPositionData
   
DESCRIPTION:
   The function sends 'PDS/Inject Wi-Fi Position Data Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectWiFiPositionData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetSBASConfig
   
DESCRIPTION:
   The function sends 'PDS/Get SBAS Config Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetSBASConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSBASConfig
   
DESCRIPTION:
   The function sends 'PDS/Set SBAS Config Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSBASConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSendNetworkInitiatedResponse
   
DESCRIPTION:
   The function sends 'PDS/Send Network Initiated Response Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSendNetworkInitiatedResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectAbsoluteTime
   
DESCRIPTION:
   The function sends 'PDS/Inject Absolute Time Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectAbsoluteTime( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectEFSData
   
DESCRIPTION:
   The function sends 'PDS/Inject EFS Data Request' (0x003E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectEFSData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 62;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetDPOConfig
   
DESCRIPTION:
   The function sends 'PDS/Get DPO Config Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetDPOConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetDPOConfig
   
DESCRIPTION:
   The function sends 'PDS/Set DPO Config Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetDPOConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetODPConfig
   
DESCRIPTION:
   The function sends 'PDS/Get ODP Config Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetODPConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetODPConfig
   
DESCRIPTION:
   The function sends 'PDS/Set ODP Config Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetODPConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSCancelSinglePositionFix
   
DESCRIPTION:
   The function sends 'PDS/Cancel Single Position Fix Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSCancelSinglePositionFix( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetGPSState
   
DESCRIPTION:
   The function sends 'PDS/Get GPS State Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetGPSState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetPPMEventReport
   
DESCRIPTION:
   The function sends 'PDS/Set PPM Event Report Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetPPMEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSPIStreamingReport
   
DESCRIPTION:
   The function sends 'PDS/Set SPI Streaming Report Request' (0x0046)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSPIStreamingReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 70;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSPIStatus
   
DESCRIPTION:
   The function sends 'PDS/Set SPI Status Request' (0x0047)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSPIStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 71;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetPPMReportingState
   
DESCRIPTION:
   The function sends 'PDS/Set PPM Reporting State Request' (0x0048)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetPPMReportingState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 72;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSForceReceiverOff
   
DESCRIPTION:
   The function sends 'PDS/Force Receiver Off Request' (0x0049)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSForceReceiverOff( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 73;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetPositionMethodsState
   
DESCRIPTION:
   The function sends 'PDS/Get Position Methods State Request' (0x0050)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetPositionMethodsState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 80;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetPositionMethodsState
   
DESCRIPTION:
   The function sends 'PDS/Set Position Methods State Request' (0x0051)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetPositionMethodsState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 81;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectSensorData
   
DESCRIPTION:
   The function sends 'PDS/Inject Sensor Data Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectSensorData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectTimeSyncData
   
DESCRIPTION:
   The function sends 'PDS/Inject Time Sync Data Request' (0x0053)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectTimeSyncData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 83;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetSensorConfig
   
DESCRIPTION:
   The function sends 'PDS/Get Sensor Config Request' (0x0054)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetSensorConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 84;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSensorConfig
   
DESCRIPTION:
   The function sends 'PDS/Set Sensor Config Request' (0x0055)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSensorConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 85;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetSensorNavigation
   
DESCRIPTION:
   The function sends 'PDS/Get Sensor Navigation Request' (0x0056)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetSensorNavigation( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 86;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetNavigationConfig
   
DESCRIPTION:
   The function sends 'PDS/Set Navigation Config Request' (0x0057)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetNavigationConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 87;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetWLANBlanking
   
DESCRIPTION:
   The function sends 'PDS/Set WLAN Blanking Request' (0x005A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetWLANBlanking( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 90;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSecurityChallengeReport
   
DESCRIPTION:
   The function sends 'PDS/Set Security Challenge Report Request' (0x005B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSecurityChallengeReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 91;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSecurityChallenge
   
DESCRIPTION:
   The function sends 'PDS/Set Security Challenge Request' (0x005C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSecurityChallenge( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 92;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSGetSecurityEncryptionConfig
   
DESCRIPTION:
   The function sends 'PDS/Get Security Encryption Config Request' (0x005D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSGetSecurityEncryptionConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 93;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetSecurityUpdateRate
   
DESCRIPTION:
   The function sends 'PDS/Set Security Update Rate Request' (0x005E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetSecurityUpdateRate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 94;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetCellDatabaseControl
   
DESCRIPTION:
   The function sends 'PDS/Set Cell Database Control Request' (0x005F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetCellDatabaseControl( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 95;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectMotionData
   
DESCRIPTION:
   The function sends 'PDS/Inject Motion Data Request' (0x0061)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectMotionData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 97;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetGNSSEngineErrorRecoveryReport
   
DESCRIPTION:
   The function sends 'PDS/Set GNSS Engine Error Recovery Report Request' (0x0062)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetGNSSEngineErrorRecoveryReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 98;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSResetLocationService
   
DESCRIPTION:
   The function sends 'PDS/Reset Location Service Request' (0x0063)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSResetLocationService( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 99;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSInjectTestData
   
DESCRIPTION:
   The function sends 'PDS/Inject Test Data Request' (0x0064)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSInjectTestData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 100;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PDSSetGNSSRFConfig
   
DESCRIPTION:
   The function sends 'PDS/Set GNSS RF Config Request' (0x0065)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PDSSetGNSSRFConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 6;
   ULONG msgID = 101;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   AUTHStartEAPSession
   
DESCRIPTION:
   The function sends 'AUTH/Start EAP Session Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AUTHStartEAPSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 7;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   AUTHSendEAPPacket
   
DESCRIPTION:
   The function sends 'AUTH/Send EAP Packet Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AUTHSendEAPPacket( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 7;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   AUTHGetEAPSessionKeys
   
DESCRIPTION:
   The function sends 'AUTH/Get EAP Session Keys Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AUTHGetEAPSessionKeys( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 7;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   AUTHEndEAPSession
   
DESCRIPTION:
   The function sends 'AUTH/End EAP Session Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AUTHEndEAPSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 7;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   AUTHRunAKA
   
DESCRIPTION:
   The function sends 'AUTH/Run AKA Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG AUTHRunAKA( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 7;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceIndicationRegistration
   
DESCRIPTION:
   The function sends 'Voice/Indication Registration Request' (0x0003)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceIndicationRegistration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 3;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceCallOriginate
   
DESCRIPTION:
   The function sends 'Voice/Call Originate Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceCallOriginate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceCallEnd
   
DESCRIPTION:
   The function sends 'Voice/Call End Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceCallEnd( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceCallAnswer
   
DESCRIPTION:
   The function sends 'Voice/Call Answer Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceCallAnswer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCallInfo
   
DESCRIPTION:
   The function sends 'Voice/Get Call Info Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCallInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceSendFlash
   
DESCRIPTION:
   The function sends 'Voice/Send Flash Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceSendFlash( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceBurstDTMF
   
DESCRIPTION:
   The function sends 'Voice/Burst DTMF Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceBurstDTMF( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceStartContinuousDTMF
   
DESCRIPTION:
   The function sends 'Voice/Start Continuous DTMF Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceStartContinuousDTMF( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceStopContinuousDTMF
   
DESCRIPTION:
   The function sends 'Voice/Stop Continuous DTMF Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceStopContinuousDTMF( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceSetPreferredPrivacy
   
DESCRIPTION:
   The function sends 'Voice/Set Preferred Privacy Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceSetPreferredPrivacy( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetAllCallInfo
   
DESCRIPTION:
   The function sends 'Voice/Get All Call Info Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetAllCallInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceManageCalls
   
DESCRIPTION:
   The function sends 'Voice/Manage Calls Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceManageCalls( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceSetSupplementaryService
   
DESCRIPTION:
   The function sends 'Voice/Set Supplementary Service Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceSetSupplementaryService( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCallWaiting
   
DESCRIPTION:
   The function sends 'Voice/Get Call Waiting Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCallWaiting( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCallBarring
   
DESCRIPTION:
   The function sends 'Voice/Get Call Barring Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCallBarring( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCLIP
   
DESCRIPTION:
   The function sends 'Voice/Get CLIP Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCLIP( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCLIR
   
DESCRIPTION:
   The function sends 'Voice/Get CLIR Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCLIR( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCallForwarding
   
DESCRIPTION:
   The function sends 'Voice/Get Call Forwarding Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCallForwarding( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceSetCallBarringPassword
   
DESCRIPTION:
   The function sends 'Voice/Set Call Barring Password Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceSetCallBarringPassword( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceInitiateUSSD
   
DESCRIPTION:
   The function sends 'Voice/Initiate USSD Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceInitiateUSSD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceAnswerUSSD
   
DESCRIPTION:
   The function sends 'Voice/Answer USSD Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceAnswerUSSD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceCancelUSSD
   
DESCRIPTION:
   The function sends 'Voice/Cancel USSD Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceCancelUSSD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceSetConfig
   
DESCRIPTION:
   The function sends 'Voice/Set Config Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceSetConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetConfig
   
DESCRIPTION:
   The function sends 'Voice/Get Config Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceAsyncInitiateUSSD
   
DESCRIPTION:
   The function sends 'Voice/Async Initiate USSD Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceAsyncInitiateUSSD( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceBindSubscription
   
DESCRIPTION:
   The function sends 'Voice/Bind Subscription Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceBindSubscription( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceALSSetLineSwitching
   
DESCRIPTION:
   The function sends 'Voice/ALS Set Line Switching Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceALSSetLineSwitching( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceALSSelectLine
   
DESCRIPTION:
   The function sends 'Voice/ALS Select Line Request' (0x0046)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceALSSelectLine( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 70;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceAOCResetACM
   
DESCRIPTION:
   The function sends 'Voice/AOC Reset ACM Request' (0x0047)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceAOCResetACM( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 71;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceAOCSetACMMaximum
   
DESCRIPTION:
   The function sends 'Voice/AOC Set ACM Maximum Request' (0x0048)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceAOCSetACMMaximum( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 72;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceAOCGetCallMeterInfo
   
DESCRIPTION:
   The function sends 'Voice/AOC Get Call Meter Info Request' (0x0049)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceAOCGetCallMeterInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 73;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCOLP
   
DESCRIPTION:
   The function sends 'Voice/Get COLP Request' (0x004B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCOLP( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 75;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCOLR
   
DESCRIPTION:
   The function sends 'Voice/Get COLR Request' (0x004C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCOLR( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 76;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceGetCNAP
   
DESCRIPTION:
   The function sends 'Voice/Get CNAP Request' (0x004D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceGetCNAP( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 77;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceManageIPCalls
   
DESCRIPTION:
   The function sends 'Voice/Manage IP Calls Request' (0x004E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceManageIPCalls( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 78;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceALSGetLineSwitchingStatus
   
DESCRIPTION:
   The function sends 'Voice/ALS Get Line Switching Status Request' (0x004F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceALSGetLineSwitchingStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 79;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   VoiceALSGetSelectedLine
   
DESCRIPTION:
   The function sends 'Voice/ALS Get Selected Line Request' (0x0050)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG VoiceALSGetSelectedLine( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 9;
   ULONG msgID = 80;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2Reset
   
DESCRIPTION:
   The function sends 'CAT2/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2Reset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SetEventReport
   
DESCRIPTION:
   The function sends 'CAT2/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2GetServiceState
   
DESCRIPTION:
   The function sends 'CAT2/Get Service State Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2GetServiceState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SendTerminalResponse
   
DESCRIPTION:
   The function sends 'CAT2/Send Terminal Response Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SendTerminalResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2EnvelopeCommand
   
DESCRIPTION:
   The function sends 'CAT2/Envelope Command Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2EnvelopeCommand( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2GetEventReport
   
DESCRIPTION:
   The function sends 'CAT2/Get Event Report Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2GetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SendDecodedTerminalResponse
   
DESCRIPTION:
   The function sends 'CAT2/Send Decoded Terminal Response Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SendDecodedTerminalResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SendDecodedEnvelopeCommand
   
DESCRIPTION:
   The function sends 'CAT2/Send Decoded Envelope Command Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SendDecodedEnvelopeCommand( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2EventConfirmation
   
DESCRIPTION:
   The function sends 'CAT2/Event Confirmation Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2EventConfirmation( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SCWSOpenChannel
   
DESCRIPTION:
   The function sends 'CAT2/SCWS Open Channel Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SCWSOpenChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SCWSCloseChannel
   
DESCRIPTION:
   The function sends 'CAT2/SCWS Close Channel Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SCWSCloseChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SCWSSendData
   
DESCRIPTION:
   The function sends 'CAT2/SCWS Send Data Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SCWSSendData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SCWSDataAvailable
   
DESCRIPTION:
   The function sends 'CAT2/SCWS Data Available Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SCWSDataAvailable( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SCWSChannelStatus
   
DESCRIPTION:
   The function sends 'CAT2/SCWS Channel Status Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SCWSChannelStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2GetTerminalProfile
   
DESCRIPTION:
   The function sends 'CAT2/Get Terminal Profile Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2GetTerminalProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2SetConfiguration
   
DESCRIPTION:
   The function sends 'CAT2/Set Configuration Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2SetConfiguration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CAT2GetConfiguration
   
DESCRIPTION:
   The function sends 'CAT2/Get Configuration Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CAT2GetConfiguration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 10;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMReset
   
DESCRIPTION:
   The function sends 'UIM/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMReadTransparent
   
DESCRIPTION:
   The function sends 'UIM/Read Transparent Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMReadTransparent( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMReadRecord
   
DESCRIPTION:
   The function sends 'UIM/Read Record Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMReadRecord( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMWriteTransparent
   
DESCRIPTION:
   The function sends 'UIM/Write Transparent Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMWriteTransparent( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMWriteRecord
   
DESCRIPTION:
   The function sends 'UIM/Write Record Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMWriteRecord( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetFileAttributes
   
DESCRIPTION:
   The function sends 'UIM/Get File Attributes Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetFileAttributes( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSetPINProtection
   
DESCRIPTION:
   The function sends 'UIM/Set PIN Protection Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSetPINProtection( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMVerifyPIN
   
DESCRIPTION:
   The function sends 'UIM/Verify PIN Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMVerifyPIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMUnblockPIN
   
DESCRIPTION:
   The function sends 'UIM/Unblock PIN Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMUnblockPIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMChangePIN
   
DESCRIPTION:
   The function sends 'UIM/Change PIN Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMChangePIN( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMDepersonalization
   
DESCRIPTION:
   The function sends 'UIM/Depersonalization Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMDepersonalization( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMRefreshRegister
   
DESCRIPTION:
   The function sends 'UIM/Refresh Register Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMRefreshRegister( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMRefreshOK
   
DESCRIPTION:
   The function sends 'UIM/Refresh OK Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMRefreshOK( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMRefreshComplete
   
DESCRIPTION:
   The function sends 'UIM/Refresh Complete Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMRefreshComplete( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetLastRefreshEvent
   
DESCRIPTION:
   The function sends 'UIM/Get Last Refresh Event Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetLastRefreshEvent( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMEventRegistration
   
DESCRIPTION:
   The function sends 'UIM/Event Registration Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMEventRegistration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetCardStatus
   
DESCRIPTION:
   The function sends 'UIM/Get Card Status Request' (0x002F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetCardStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 47;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMPowerDown
   
DESCRIPTION:
   The function sends 'UIM/Power Down Request' (0x0030)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMPowerDown( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 48;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMPowerUp
   
DESCRIPTION:
   The function sends 'UIM/Power Up Request' (0x0031)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMPowerUp( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 49;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMAuthenticate
   
DESCRIPTION:
   The function sends 'UIM/Authenticate Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMAuthenticate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMCloseSession
   
DESCRIPTION:
   The function sends 'UIM/Close Session Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMCloseSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetServiceStatus
   
DESCRIPTION:
   The function sends 'UIM/Get Service Status Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetServiceStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSetServiceStatus
   
DESCRIPTION:
   The function sends 'UIM/Set Service Status Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSetServiceStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMChangeProvisioningSession
   
DESCRIPTION:
   The function sends 'UIM/Change Provisioning Session Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMChangeProvisioningSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetLabel
   
DESCRIPTION:
   The function sends 'UIM/Get Label Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetLabel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetConfiguration
   
DESCRIPTION:
   The function sends 'UIM/Get Configuration Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetConfiguration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSendADPU
   
DESCRIPTION:
   The function sends 'UIM/Send ADPU Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSendADPU( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSAPConnection
   
DESCRIPTION:
   The function sends 'UIM/SAP Connection Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSAPConnection( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSAPRequest
   
DESCRIPTION:
   The function sends 'UIM/SAP Request Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSAPRequest( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMLogicalChannel
   
DESCRIPTION:
   The function sends 'UIM/Logical Channel Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMLogicalChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMSubscriptionOK
   
DESCRIPTION:
   The function sends 'UIM/Subscription OK Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMSubscriptionOK( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMGetATR
   
DESCRIPTION:
   The function sends 'UIM/Get ATR Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMGetATR( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   UIMOpenLogicalChannel
   
DESCRIPTION:
   The function sends 'UIM/Open Logical Channel Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG UIMOpenLogicalChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 11;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMSetIndicationRegistrationState
   
DESCRIPTION:
   The function sends 'PBM/Set Indication Registration State Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMSetIndicationRegistrationState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetCapabilities
   
DESCRIPTION:
   The function sends 'PBM/Get Capabilities Request' (0x0002)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetCapabilities( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 2;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetAllCapabilities
   
DESCRIPTION:
   The function sends 'PBM/Get All Capabilities Request' (0x0003)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetAllCapabilities( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 3;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMReadRecords
   
DESCRIPTION:
   The function sends 'PBM/Read Records Request' (0x0004)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMReadRecords( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 4;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMWriteRecord
   
DESCRIPTION:
   The function sends 'PBM/Write Record Request' (0x0005)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMWriteRecord( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 5;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMDeleteRecord
   
DESCRIPTION:
   The function sends 'PBM/Delete Record Request' (0x0006)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMDeleteRecord( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 6;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMDeleteAllRecords
   
DESCRIPTION:
   The function sends 'PBM/Delete All Records Request' (0x0007)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMDeleteAllRecords( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 7;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMSearchRecords
   
DESCRIPTION:
   The function sends 'PBM/Search Records Request' (0x0008)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMSearchRecords( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 8;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetEmergencyList
   
DESCRIPTION:
   The function sends 'PBM/Get Emergency List Request' (0x000E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetEmergencyList( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 14;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetAllGroups
   
DESCRIPTION:
   The function sends 'PBM/Get All Groups Request' (0x000F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetAllGroups( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 15;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMSetGroupInfo
   
DESCRIPTION:
   The function sends 'PBM/Set Group Info Request' (0x0010)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMSetGroupInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 16;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetState
   
DESCRIPTION:
   The function sends 'PBM/Get State Request' (0x0011)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 17;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMReadAllHiddenRecords
   
DESCRIPTION:
   The function sends 'PBM/Read All Hidden Records Request' (0x0012)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMReadAllHiddenRecords( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 18;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetNextEmptyRecordID
   
DESCRIPTION:
   The function sends 'PBM/Get Next Empty Record ID Request' (0x0014)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetNextEmptyRecordID( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 20;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetNextRecordID
   
DESCRIPTION:
   The function sends 'PBM/Get Next Record ID Request' (0x0015)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetNextRecordID( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 21;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetAASList
   
DESCRIPTION:
   The function sends 'PBM/Get AAS List Request' (0x0016)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetAASList( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 22;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMSetAAS
   
DESCRIPTION:
   The function sends 'PBM/Set AAS Request' (0x0017)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMSetAAS( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 23;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMBindSubscription
   
DESCRIPTION:
   The function sends 'PBM/Bind Subscription Request' (0x001A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMBindSubscription( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 26;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   PBMGetSubscription
   
DESCRIPTION:
   The function sends 'PBM/Get Subscription Request' (0x001B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PBMGetSubscription( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 12;
   ULONG msgID = 27;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCClientRevision
   
DESCRIPTION:
   The function sends 'LOC/Client Revision Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCClientRevision( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCRegisterEvents
   
DESCRIPTION:
   The function sends 'LOC/Register Events Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCRegisterEvents( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCStart
   
DESCRIPTION:
   The function sends 'LOC/Start Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCStart( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCStop
   
DESCRIPTION:
   The function sends 'LOC/Stop Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCStop( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetServiceRevision
   
DESCRIPTION:
   The function sends 'LOC/Get Service Revision Request' (0x0032)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetServiceRevision( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 50;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetFixCriteria
   
DESCRIPTION:
   The function sends 'LOC/Get Fix Criteria Request' (0x0033)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetFixCriteria( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 51;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCProvideNIUserResponse
   
DESCRIPTION:
   The function sends 'LOC/Provide NI User Response Request' (0x0034)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCProvideNIUserResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 52;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectPredictedOrbitsData
   
DESCRIPTION:
   The function sends 'LOC/Inject Predicted Orbits Data Request' (0x0035)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectPredictedOrbitsData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 53;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetPredictedOrbitsDataSource
   
DESCRIPTION:
   The function sends 'LOC/Get Predicted Orbits Data Source Request' (0x0036)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetPredictedOrbitsDataSource( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 54;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetPredictedOrbitsDataValidity
   
DESCRIPTION:
   The function sends 'LOC/Get Predicted Orbits Data Validity Request' (0x0037)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetPredictedOrbitsDataValidity( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 55;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectUTCTime
   
DESCRIPTION:
   The function sends 'LOC/Inject UTC Time Request' (0x0038)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectUTCTime( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 56;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectPosition
   
DESCRIPTION:
   The function sends 'LOC/Inject Position Request' (0x0039)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectPosition( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 57;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetEngineLock
   
DESCRIPTION:
   The function sends 'LOC/Set Engine Lock Request' (0x003A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetEngineLock( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 58;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetEngineLock
   
DESCRIPTION:
   The function sends 'LOC/Get Engine Lock Request' (0x003B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetEngineLock( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 59;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetSBASConfig
   
DESCRIPTION:
   The function sends 'LOC/Set SBAS Config Request' (0x003C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetSBASConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 60;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetSBASConfig
   
DESCRIPTION:
   The function sends 'LOC/Get SBAS Config Request' (0x003D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetSBASConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 61;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetNMEATypes
   
DESCRIPTION:
   The function sends 'LOC/Set NMEA Types Request' (0x003E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetNMEATypes( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 62;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetNMEATypes
   
DESCRIPTION:
   The function sends 'LOC/Get NMEA Types Request' (0x003F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetNMEATypes( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 63;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetLowPowerMode
   
DESCRIPTION:
   The function sends 'LOC/Set Low Power Mode Request' (0x0040)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetLowPowerMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 64;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetLowPowerMode
   
DESCRIPTION:
   The function sends 'LOC/Get Low Power Mode Request' (0x0041)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetLowPowerMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 65;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetLocationServer
   
DESCRIPTION:
   The function sends 'LOC/Set Location Server Request' (0x0042)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetLocationServer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 66;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetLocationServer
   
DESCRIPTION:
   The function sends 'LOC/Get Location Server Request' (0x0043)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetLocationServer( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 67;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCDeleteAssistData
   
DESCRIPTION:
   The function sends 'LOC/Delete Assist Data Request' (0x0044)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCDeleteAssistData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 68;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetXTRATSessionControl
   
DESCRIPTION:
   The function sends 'LOC/Set XTRA-T Session Control Request' (0x0045)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetXTRATSessionControl( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 69;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOC
   
DESCRIPTION:
   The function sends 'LOC' (0x0046)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOC( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 70;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectWiFiPosition
   
DESCRIPTION:
   The function sends 'LOC/Inject Wi-Fi Position Request' (0x0047)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectWiFiPosition( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 71;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCProvideWiFiStatus
   
DESCRIPTION:
   The function sends 'LOC/Provide Wi-Fi Status Request' (0x0048)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCProvideWiFiStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 72;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetRegisteredEvents
   
DESCRIPTION:
   The function sends 'LOC/Get Registered Events Request' (0x0049)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetRegisteredEvents( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 73;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetOperationMode
   
DESCRIPTION:
   The function sends 'LOC/Set Operation Mode Request' (0x004A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetOperationMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 74;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetOperationMode
   
DESCRIPTION:
   The function sends 'LOC/Get Operation Mode Request' (0x004B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetOperationMode( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 75;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetSPIStatus
   
DESCRIPTION:
   The function sends 'LOC/Set SPI Status Request' (0x004C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetSPIStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 76;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectSensorData
   
DESCRIPTION:
   The function sends 'LOC/Inject Sensor Data Request' (0x004D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectSensorData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 77;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectTimeSyncData
   
DESCRIPTION:
   The function sends 'LOC/Inject Time Sync Data Request' (0x004E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectTimeSyncData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 78;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetCradleMountConfig
   
DESCRIPTION:
   The function sends 'LOC/Set Cradle Mount Config Request' (0x004F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetCradleMountConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 79;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetCradleMountConfig
   
DESCRIPTION:
   The function sends 'LOC/Get Cradle Mount Config Request' (0x0050)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetCradleMountConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 80;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetExternalPowerConfig
   
DESCRIPTION:
   The function sends 'LOC/Set External Power Config Request' (0x0051)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetExternalPowerConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 81;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetExternalPowerConfig
   
DESCRIPTION:
   The function sends 'LOC/Get External Power Config Request' (0x0052)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetExternalPowerConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 82;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCProvideConnectionStatus
   
DESCRIPTION:
   The function sends 'LOC/Provide Connection Status Request' (0x0053)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCProvideConnectionStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 83;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetProtocolConfigParameters
   
DESCRIPTION:
   The function sends 'LOC/Set Protocol Config Parameters Request' (0x0054)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetProtocolConfigParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 84;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetProtocolConfigParameters
   
DESCRIPTION:
   The function sends 'LOC/Get Protocol Config Parameters Request' (0x0055)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetProtocolConfigParameters( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 85;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetSensorControlConfig
   
DESCRIPTION:
   The function sends 'LOC/Set Sensor Control Config Request' (0x0056)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetSensorControlConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 86;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetSensorControlConfig
   
DESCRIPTION:
   The function sends 'LOC/Get Sensor Control Config Request' (0x0057)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetSensorControlConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 87;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetSensorProperties
   
DESCRIPTION:
   The function sends 'LOC/Set Sensor Properties Request' (0x0058)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetSensorProperties( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 88;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetSensorProperties
   
DESCRIPTION:
   The function sends 'LOC/Get Sensor Properties Request' (0x0059)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetSensorProperties( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 89;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetSensorPerformanceConfig
   
DESCRIPTION:
   The function sends 'LOC/Set Sensor Performance Config Request' (0x005A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetSensorPerformanceConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 90;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetSensorPerformanceConfig
   
DESCRIPTION:
   The function sends 'LOC/Get Sensor Performance Config Request' (0x005B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetSensorPerformanceConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 91;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCInjectSUPLCertificate
   
DESCRIPTION:
   The function sends 'LOC/Inject SUPL Certificate Request' (0x005C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCInjectSUPLCertificate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 92;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCDeleteSUPLCertificate
   
DESCRIPTION:
   The function sends 'LOC/Delete SUPL Certificate Request' (0x005D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCDeleteSUPLCertificate( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 93;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCSetPositionEngineConfig
   
DESCRIPTION:
   The function sends 'LOC/Set Position Engine Config Request' (0x005E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCSetPositionEngineConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 94;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetPositionEngineConfig
   
DESCRIPTION:
   The function sends 'LOC/Get Position Engine Config Request' (0x005F)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetPositionEngineConfig( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 95;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCAddCircularGeofence
   
DESCRIPTION:
   The function sends 'LOC/Add Circular Geofence Request' (0x0063)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCAddCircularGeofence( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 99;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCDeleteGeofence
   
DESCRIPTION:
   The function sends 'LOC/Delete Geofence Request' (0x0064)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCDeleteGeofence( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 100;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCQueryGeofence
   
DESCRIPTION:
   The function sends 'LOC/Query Geofence Request' (0x0065)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCQueryGeofence( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 101;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCEditGeofence
   
DESCRIPTION:
   The function sends 'LOC/Edit Geofence Request' (0x0066)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCEditGeofence( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 102;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   LOCGetBestAvailablePosition
   
DESCRIPTION:
   The function sends 'LOC/Get Best Available Position Request' (0x0067)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG LOCGetBestAvailablePosition( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 18;
   ULONG msgID = 103;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATReset
   
DESCRIPTION:
   The function sends 'CAT/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSetEventReport
   
DESCRIPTION:
   The function sends 'CAT/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATGetServiceState
   
DESCRIPTION:
   The function sends 'CAT/Get Service State Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATGetServiceState( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSendTerminalResponse
   
DESCRIPTION:
   The function sends 'CAT/Send Terminal Response Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSendTerminalResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATEnvelopeCommand
   
DESCRIPTION:
   The function sends 'CAT/Envelope Command Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATEnvelopeCommand( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATGetEventReport
   
DESCRIPTION:
   The function sends 'CAT/Get Event Report Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATGetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSendDecodedTerminalResponse
   
DESCRIPTION:
   The function sends 'CAT/Send Decoded Terminal Response Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSendDecodedTerminalResponse( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSendDecodedEnvelopeCommand
   
DESCRIPTION:
   The function sends 'CAT/Send Decoded Envelope Command Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSendDecodedEnvelopeCommand( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATEventConfirmation
   
DESCRIPTION:
   The function sends 'CAT/Event Confirmation Request' (0x0026)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATEventConfirmation( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 38;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSCWSOpenChannel
   
DESCRIPTION:
   The function sends 'CAT/SCWS Open Channel Request' (0x0027)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSCWSOpenChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 39;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSCWSCloseChannel
   
DESCRIPTION:
   The function sends 'CAT/SCWS Close Channel Request' (0x0028)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSCWSCloseChannel( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 40;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSCWSSendData
   
DESCRIPTION:
   The function sends 'CAT/SCWS Send Data Request' (0x0029)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSCWSSendData( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 41;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSCWSDataAvailable
   
DESCRIPTION:
   The function sends 'CAT/SCWS Data Available Request' (0x002A)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSCWSDataAvailable( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 42;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSCWSChannelStatus
   
DESCRIPTION:
   The function sends 'CAT/SCWS Channel Status Request' (0x002B)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSCWSChannelStatus( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 43;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATGetTerminalProfile
   
DESCRIPTION:
   The function sends 'CAT/Get Terminal Profile Request' (0x002C)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATGetTerminalProfile( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 44;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATSetConfiguration
   
DESCRIPTION:
   The function sends 'CAT/Set Configuration Request' (0x002D)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATSetConfiguration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 45;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   CATGetConfiguration
   
DESCRIPTION:
   The function sends 'CAT/Get Configuration Request' (0x002E)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG CATGetConfiguration( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 224;
   ULONG msgID = 46;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   RMSReset
   
DESCRIPTION:
   The function sends 'RMS/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG RMSReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 225;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   RMSGetSMSWake
   
DESCRIPTION:
   The function sends 'RMS/Get SMS Wake Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG RMSGetSMSWake( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 225;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   RMSSetSMSWake
   
DESCRIPTION:
   The function sends 'RMS/Set SMS Wake Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG RMSSetSMSWake( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 225;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMAReset
   
DESCRIPTION:
   The function sends 'OMA/Reset Request' (0x0000)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMAReset( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 0;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMASetEventReport
   
DESCRIPTION:
   The function sends 'OMA/Set Event Report Request' (0x0001)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMASetEventReport( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 1;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMAStartSession
   
DESCRIPTION:
   The function sends 'OMA/Start Session Request' (0x0020)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMAStartSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 32;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMACancelSession
   
DESCRIPTION:
   The function sends 'OMA/Cancel Session Request' (0x0021)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMACancelSession( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 33;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMAGetSessionInfo
   
DESCRIPTION:
   The function sends 'OMA/Get Session Info Request' (0x0022)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMAGetSessionInfo( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 34;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMASendSelection
   
DESCRIPTION:
   The function sends 'OMA/Send Selection Request' (0x0023)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMASendSelection( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 35;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMAGetFeatures
   
DESCRIPTION:
   The function sends 'OMA/Get Features Request' (0x0024)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Reserved for future use (set to 0)
   pIn         [ I ] - Reserved for future use (set to 0)
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMAGetFeatures( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 36;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

/*===========================================================================
METHOD:
   OMASetFeatures
   
DESCRIPTION:
   The function sends 'OMA/Set Features Request' (0x0025)
   and returns the response
   
PARAMETERS:
   handle      [ I ] - Gobi interface handle
   to          [ I ] - Timeout for transaction (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
  
RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG OMASetFeatures( 
   GOBIHANDLE                 handle,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   cGobiConnectionMgmt * pAPI = gDLL.GetAPI( handle );
   if (pAPI == 0)
   {
      return (ULONG)eGOBI_ERR_INTERNAL;
   }

   ULONG svcID = 226;
   ULONG msgID = 37;
   return pAPI->Send( svcID, msgID, to, inLen, pIn, pOutLen, pOut );
}

