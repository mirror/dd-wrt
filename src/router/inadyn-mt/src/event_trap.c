/*
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
    Win32s RAS events trapping thread.  Co-exists 
    with dyndns.c main update loop.  If any part 
    of setup fails, returns, and system reverts to 
    main thread only execution.

	Author: bhoover@wecs.com
	Date: Oct 2007

	History:
		- first implemetnation
*/

#define MODULE_TAG     "RASTRP: "

/*

array of event_trap instance data instead? a structure
with an instance count, and an arrary of rasthreaddata
--overkill -- don't really need instances

#define DEBUG_LEVEL    p_ras_thread_data->dw_debug_level

#define TEST_URL       p_ras_thread_data->szTestURL

*/

#define DEBUG_LEVEL    dw_debug_level

#define TEST_URL       szTestURL

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <ras.h>
#include <process.h>
#include <sensapi.h>

#include "event_trap.h"
#include "debug_if.h"
#include "base64.h"
#include "errorcode.h"
#include "safe_mem.h"

static OSVERSIONINFO    *p_osVersionInfo=NULL;
static HANDLE           hRASMutex=0;
static int				dw_debug_level=LOG_ERR;
static char				*szTestURL=NULL;


/*************PRIVATE FUNCTIONS ******************/

static BOOL is_destination_reachable(const char *test_url,int debug_level)
{

	BOOL retVal=false;


	retVal=IsDestinationReachable(test_url,NULL);

	if (!(retVal))

		dbg_printf_and_err_str(NULL,"I:" MODULE_TAG "IsDestinationReachable returned false...",GetLastError(),LOG_INFO,debug_level);

	else

		if (debug_level>=LOG_INFO)

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "IsDestinationReachable (%s) returning true...\n",test_url));


	return retVal;
}

static BOOL is_network_up(const char *test_url,int debug_level)
{
	DWORD       dwFlags=0;
	DWORD       netErrCode=0;
	DWORD       retVal=false;

	/*

	  This can tell us whether on a lan, wan (RAS connection), or AOL (on win 95/98).

	lpdwFlags 
	Provides information on the type of network connection available when the return value is TRUE. The flags can be: 
	NETWORK_ALIVE_LAN 
	The computer has one or more LAN cards that are active. 
	NETWORK_ALIVE_WAN 
	The computer has one or more active RAS connections. 
	NETWORK_ALIVE_AOL 
	This flag is only valid in Windows 95 and Windows 98. Indicates the computer is connected to the America Online network. 

	#define NETWORK_ALIVE_LAN   0x00000001
	#define NETWORK_ALIVE_WAN   0x00000002
	#define NETWORK_ALIVE_AOL   0x00000004

	isNetworkAlive dwflags:  1 (lan) when vmware is running a networked vm, and offline, and 2 (wan) when online with 
	vmware not running, or running without a vm running.  When online, and vmware running networked vm, 3 (wan, and lan).

	Though, this is not reliable.  Returned offline, correctly, first time through, and true, when offline, iterations
	after.  Using Windows IsDestinationReachable instead.

	On windows 98 (vmware with virtual lan), returning not wan or better.  Mmmm.  Might need to switch over to 
	isdestreachable for win32s too.  Seems it should notice the lan gets outside.

	*/

	SetLastError(0);

	if (!(IsNetworkAlive(&dwFlags))) {

		netErrCode=GetLastError();

		if (netErrCode)

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Network offline, apparently.",netErrCode,LOG_ERR,debug_level);

		else if (debug_level>=LOG_WARNING)

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Network offline, apparently.  Error code of zero, %d, assuming really must be offline...\n",netErrCode));

		return false;

	}

	else {

		if (debug_level>=LOG_INFO)

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "IsNetworkAlive returning true...\n"));

		retVal=(dwFlags>=NETWORK_ALIVE_WAN);

		if (retVal) {

			if (debug_level>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "IsNetworkAlive says net is WAN or better.  Returning online true...\n"));

		}
		else {

			if (debug_level>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "IsNetworkAlive says not WAN or better...\n"));

			retVal=is_destination_reachable(test_url,debug_level);

			if (debug_level>=LOG_INFO)

				if (retVal)

					DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "is_destination_reachable returning true.  Returning online true...\n"));

				else

					DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "is_destination_reachable returning false.  Returning online false...\n"));
		}

		return retVal;
	}
}

static BOOL is_ras_online(BOOL isOptimistic,const char *test_url,int debug_level)

/*
  In:  isOptimistic -- if true, err on the side of returning true, indicating online status, else if false, err on safe
       side, returning offline status -- see notes below.

  return:  return true if online, otherwise, return false.

  The RAS functions used here require unicode.  Unicode is not supported in Windows Windows 95/98/ME.

  I've read, determining network status can be tricky.  isOptimistic parameter gives option of erring on side of returning
  true, or false.  If isOptimistic is true, will return true, eventhough whether online was indeterminate.  If isOptimistic
  is false, will return false when whether online was indeterminate.  This makes the function no more or less reliable than
  otherwise, when false, strictly speaking.  It gives caller more flexibility, whereas without the parameter, it's unknown
  whether a false was a definate offline determination, for instance.  Caller could call twice with different truth values
  for that matter.  This technique is an interesting concept, I think.  I don't think I've seen it before (perhaps for
  good reason :)), but, understood (not difficult), it's harmless enough.  An obvious alternative would be to expand on the
  number of return values (in an array 'out' parameter for instance), and let caller figure it out.
*/

{

	#define           NUM_RAS                        1
	#define           RAS_ENUM_EXPECTED_BYTES     1340

	LPRASCONNW        lpRASConn=NULL;

	DWORD             numRAS=0;
	DWORD             dwSize;

	DWORD             rasRet=false;

	LPRASCONNSTATUSW  lpRASCONNStatus=NULL;

	char              lpErrStr[1024];

	int               retVal=false;

	DWORD             waitRet=0;

	HINSTANCE         hRAS_API32=NULL;

	FARPROC           pf_RasEnumConnectionsW=NULL;
	FARPROC           pf_RasGetConnectStatusW=NULL;



	memset(lpErrStr,0,1024);

	dwSize=sizeof(RASCONNW)*NUM_RAS;

	lpRASConn=safe_malloc(dwSize);

	memset(lpRASConn,0,dwSize);

	/*
	  RAS_ENUM_EXPECTED_BYTES a workaround -- the ras enum expects one of several sizes
	  and this is the closest (smaller than actual), otherwise returns ras memory error 632
	*/
	lpRASConn[0].dwSize=RAS_ENUM_EXPECTED_BYTES;


	waitRet=WaitForSingleObject(hRASMutex,INFINITE);


	if (!(waitRet==WAIT_OBJECT_0)) {

		dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "WaitForSingleObject failed in is_ras_online.",GetLastError(),LOG_CRIT,debug_level);

	}
	else {

		SetLastError(0);

		hRAS_API32=LoadLibrary("rasapi32");

		if (!(hRAS_API32)) {

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "LoadLibrary failed",GetLastError(),LOG_ERR,debug_level);

			retVal=false;

		}
		else {

			pf_RasEnumConnectionsW=GetProcAddress(hRAS_API32,"RasEnumConnectionsW");

			if (!(pf_RasEnumConnectionsW)) {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "GetProcAddress failed",GetLastError(),LOG_ERR,debug_level);

				retVal=false;

			}
			else {

				rasRet=pf_RasEnumConnectionsW(lpRASConn,&dwSize,&numRAS);

				if (rasRet) {

					if (debug_level>=LOG_ERR) {

						DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "RasEnumConnections failed (error code:  %d) -- Memory corruption?  No RAS connections?  RAS error string follows...\n",rasRet));

						if (!(RasGetErrorString(rasRet,lpErrStr,1024)))

							DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "RAS enumeration error string (error code %d):  %s...Need:  %d bytes...\n",rasRet,lpErrStr,dwSize));

						else

							DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "RAS get error string failed for error code %d...\n",rasRet));
					}

					if (isOptimistic) {

						if (debug_level>=LOG_INFO)

							DBG_PRINTF((LOG_ERR,"I:" MODULE_TAG "RasEnumConnections failed (error code:  %d), though is Optimistic, returning 'connected' status\n",rasRet));

						retVal=true;
					}
				}
				else {

					if (debug_level>=LOG_INFO)

						DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS enumeration returned success...\n"));

					if (!(lpRASConn[0].hrasconn)) {

						if (debug_level>=LOG_INFO)

							DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "No valid RAS connections enumerated...\n"));

					}
					else {

						dwSize=sizeof(RASCONNSTATUSW);

						lpRASCONNStatus=safe_malloc(dwSize);

						memset(lpRASCONNStatus,0,dwSize);

						lpRASCONNStatus->dwSize=dwSize;

						pf_RasGetConnectStatusW=GetProcAddress(hRAS_API32,"RasGetConnectStatusW");

						if (!(pf_RasGetConnectStatusW)) {

							dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "GetProcAddress failed",GetLastError(),LOG_ERR,debug_level);

							retVal=false;

						}
						else {

							rasRet=pf_RasGetConnectStatusW(lpRASConn[0].hrasconn,lpRASCONNStatus);

							/* call succeeded */
							if (!(rasRet)) {

								retVal=(lpRASCONNStatus->rasconnstate==RASCS_Connected);

								if (debug_level>=LOG_INFO)

									DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Connection status query succeeded.  Returning %d, status...\n",retVal));
							}
							else {

								/*err (guess) on side of being offline*/

								if (!(isOptimistic)) {

									if (debug_level>=LOG_ERR) {

										DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Connection status query failed (error code:  %d, %d).  Returning 'not connected', status...\n",rasRet,lpRASCONNStatus->dwError));

										memset(lpErrStr,0,1024);

										if (!(RasGetErrorString(rasRet,lpErrStr,1024)))

											DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "RAS status query error string (error:  %d):  %s...\n",rasRet,lpErrStr));

										retVal=false;
									}
								}
								else {

									if (debug_level>=LOG_WARNING) {

										DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Connection (%d) status query failed (error code:  %d).  Is Optimistic, returning 'connected', status...\n",numRAS,rasRet));

										retVal=true;
									}
								}
							}

							free(lpRASCONNStatus);
						}
					}
				}
			}

			FreeLibrary(hRAS_API32);
		}

		free(lpRASConn);

		if (!(ReleaseMutex(hRASMutex)))

			dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "ReleaseMutex failed in is_ras_online.",GetLastError(),LOG_CRIT,debug_level);


		if (!(retVal)) { /*double check -- if actually are online, it's a very slow method of checking*/

			retVal=is_destination_reachable(test_url,debug_level);

			if (retVal)

				if (debug_level>=LOG_WARNING)

					DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Connection status query failed, but able to reach %s.  Returning 'connected', status...\n",test_url));
		}
	}

	return retVal;
}

static void wait_for_ras_notifications_thread(void *p_thread_data)
{
  #define _EVENT     retVal-WAIT_OBJECT_0

	RAS_THREAD_DATA    *p_ras_thread_data;

	DWORD              retVal=false;


	p_ras_thread_data=p_thread_data;

	while (1) {

		if (DEBUG_LEVEL>=LOG_DEBUG)

			DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Entering RAS wait...\n"));

		p_ras_thread_data->dwEvent=0;

		retVal=WaitForMultipleObjects(TOTAL_EVENTS,p_ras_thread_data->hEvent,false,INFINITE);

		p_ras_thread_data->dwEvent=_EVENT;

		if (DEBUG_LEVEL>=LOG_DEBUG)

			DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Returned from RAS wait...\n"));


		if (_EVENT==THREAD_RETURN) {

			if (DEBUG_LEVEL>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread got quit signal...\n"));

			/*give it to client, before we quit*/
			p_ras_thread_data->p_func(p_ras_thread_data);

			if (!(ReleaseSemaphore(p_ras_thread_data->hRASManSignal,1,NULL)))

				dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "ReleaseSemaphore failed in wait_for_ras_notifications_thread.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

			break; /*got quit signal*/
		}

		if (_EVENT==RAS_DISCONNECT)

			if (DEBUG_LEVEL>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread got disconnect event...\n"));

		if (_EVENT==RAS_CONNECT)

			if (DEBUG_LEVEL>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread got connect event...\n"));


		/*give it to client*/
		p_ras_thread_data->p_func(p_ras_thread_data);

		if (!(ResetEvent(p_ras_thread_data->hEvent[_EVENT])))

			dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "RAS thread ResetEvent failed.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

	} /*while*/

	_endthread();
}

static void manage_ras_thread(void *thread_data)
{

	RAS_THREAD_DATA *p_ras_thread_data;

	p_ras_thread_data=thread_data;

	p_ras_thread_data->ras_notifications_thread=_beginthread(wait_for_ras_notifications_thread,0,thread_data);

	if (p_ras_thread_data->ras_notifications_thread==-1) {

		if (DEBUG_LEVEL>=LOG_ERR)

			DBG_PRINTF((LOG_ERR,"I:" MODULE_TAG "RAS thread manager could not create RAS thread.  Low resources?...\n"));
	}
	else {

		if (DEBUG_LEVEL>=LOG_INFO) {

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread manager launched RAS thread...\n"));

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread manager entering wait for RAS thread...\n"));
		}

		/*wait til destroy signals thread kill*/

		if (!(WaitForSingleObject(p_ras_thread_data->hRASManSignal,INFINITE)==WAIT_OBJECT_0))

			dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "WaitForSingleObject (hRASManSignal) failed in manage_ras_thread.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

		if (DEBUG_LEVEL>=LOG_INFO)

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread manager raising RAS thread return event...\n"));

		if (!(SetEvent((p_ras_thread_data)->hEvent[THREAD_RETURN])))

			dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "SetEvent failed in manage_ras_thread.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

		else {

			if (DEBUG_LEVEL>=LOG_INFO)

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "RAS thread manager entering wait for RAS return...\n"));

			if (!(WaitForSingleObject((HANDLE) p_ras_thread_data->ras_notifications_thread,INFINITE)==WAIT_OBJECT_0))

				dbg_printf_and_err_str(NULL,"N:"MODULE_TAG "WaitForSingleObject (thread handle) failed in manage_ras_thread.  Too fast?",GetLastError(),LOG_NOTICE,DEBUG_LEVEL);

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "RAS thread manager returning from RAS thread wait.  Cleaning up...\n"));

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "RAS thread manager cleaned up.  Signaling we're done...\n"));

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Signaled done...\n"));
		}
	}

	_endthread();
}

static BOOL create_event(HANDLE *hEvent,char *szEventName,DWORD debugLevel)
{

	char    szError[1024];
	char    szANSIIEventName[256];
	DWORD   errCode=0;


	SetLastError(0);

	memset(&szANSIIEventName,0,256);

	*hEvent=CreateEvent(NULL, FALSE, FALSE, szEventName);

	if (*hEvent) {

		return true;
	}
	else {

		if (debugLevel>=LOG_ERR) {

			DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Could not create event: %s.  Possibly caused by system instability, low memory, etcetera.  Reboot machine?  Free some memory?  Try restarting remote access service (Win NT).\n",szEventName));

			errCode=GetLastError();

			if (errCode) {

				if (!(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				                    NULL,errCode,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),szError,1024,NULL)))

					DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Could not create event: %s.  Error code: %d...\n",szEventName,errCode));

				else {

					DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Could not create event: %s.  Error code:  %d, %s...\n",szEventName,errCode,szError));
				}
			}
		}

		return false;
	}
}

static BOOL create_ras_event(HANDLE *hEvent,char *szEventName,DWORD rasEvent,DWORD debugLevel)
{

	DWORD dwErrCode=0;
	char  szErr[1024];


	if (!(create_event(hEvent,szEventName,debugLevel)))

		return false;

	else {

		dwErrCode=RasConnectionNotification((HRASCONN)INVALID_HANDLE_VALUE,*hEvent,rasEvent);

		if (dwErrCode) {

			if (!(RasGetErrorString(dwErrCode,szErr,1024)))

				DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "RasConnectionNotifications failed in create_ras_event:  Error code:  %d: %s...\n",dwErrCode,szErr));

			else

				DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "RasConnectionNotifications failed in create_ras_event:  Error code:  %d...RasGetErrorStr failed...\n",dwErrCode));

			return false;
		}
		else {

			if (ResetEvent(*hEvent))

				return true;

			else {

				dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "ResetEvent failed in create_ras_event.",GetLastError(),LOG_CRIT,debugLevel);

				return false;
			}
		}
	}
}

static BOOL create_event_trap_semaphore(HANDLE *hSem,DWORD debugLevel)
{

	*hSem=NULL;

	*hSem=CreateSemaphore(NULL,0,1,NULL);

	if (*hSem)

		return true;

	else {

		dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "CreateSemaphore failed in create_event_trap_semaphore.",GetLastError(),LOG_CRIT,debugLevel);

		return false;
	}
}

static BOOL init_event_thread_control(RAS_THREAD_DATA *p_ras_thread_data)
{

	if (!(hRASMutex))

		hRASMutex=CreateMutex(NULL,false,NULL);

	if (!(hRASMutex)) {

		dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "CreateMutex failed in init_event_thread_control.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

		return false;

	}
	else {

		if (!(create_event_trap_semaphore(&(p_ras_thread_data->hClientInitSignal),DEBUG_LEVEL)))

			return false;

		if (!(create_event_trap_semaphore(&(p_ras_thread_data->hRASInitSignal),DEBUG_LEVEL)))

			return false;

		if (!(create_event_trap_semaphore(&(p_ras_thread_data->hRASManSignal),DEBUG_LEVEL)))

			return false;

		return true;
	}
}

static BOOL init_ras_events(RAS_THREAD_DATA *p_ras_thread_data)
{

	if (!(create_ras_event(&p_ras_thread_data->hEvent[RAS_CONNECT],NULL,RASCN_Connection,DEBUG_LEVEL)))

		return false;

	if (!(create_ras_event(&p_ras_thread_data->hEvent[RAS_DISCONNECT],NULL,RASCN_Disconnection,DEBUG_LEVEL)))

		return false;

	if(!(create_event(&p_ras_thread_data->hEvent[THREAD_RETURN],NULL,DEBUG_LEVEL)))

		return false;

	return true;
}

static BOOL init_os_version_info(RAS_THREAD_DATA *p_ras_thread_data)
{
	DWORD infoSize=0;


	infoSize=sizeof(OSVERSIONINFO);

	p_osVersionInfo=safe_malloc(infoSize);

	memset(p_osVersionInfo,0,infoSize);

	p_osVersionInfo->dwOSVersionInfoSize=infoSize;


	if (!(GetVersionEx(p_osVersionInfo))) {

		dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "GetVersionEx failed in init_os_version_info.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

		return false;
	}
	else {

		if (DEBUG_LEVEL>=LOG_DEBUG)

			DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "OS major version, %d, minor version, %d\n",p_osVersionInfo->dwMajorVersion,
			            p_osVersionInfo->dwMinorVersion));

		return true;
	}
}

/*************PUBLIC FUNCTIONS ******************/

BOOL is_online(const char *test_url,int debug_level)
{

	if (!(p_osVersionInfo))

		return false;

	if (p_osVersionInfo->dwMajorVersion<5)

		return (is_network_up(test_url,debug_level));

	else

		return is_ras_online(false,test_url,debug_level);
}

void set_debug_level(RAS_THREAD_DATA *p_ras_thread_data,DWORD debug_level)
{

	DEBUG_LEVEL=debug_level;
}

void set_online_test(RAS_THREAD_DATA *p_ras_thread_data,const char *szURL)
{

	//should be unicode

	if (szURL) {

		TEST_URL=safe_malloc(strlen(szURL)+1);

		strcpy(TEST_URL,szURL);
	}
}

void launch_ras_events_trap(void **p_data)
{
	RAS_THREAD_DATA      *p_ras_thread_data=NULL;


	p_ras_thread_data=*p_data;

	if (DEBUG_LEVEL>=LOG_DEBUG)

		DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Entering wait for client init...\n"));

	if (!(WaitForSingleObject(p_ras_thread_data->hClientInitSignal,INFINITE))==WAIT_OBJECT_0)

		dbg_printf_and_err_str(NULL,"W:" MODULE_TAG "WaitForSingleObject failed in launch_ras_events.",GetLastError(),LOG_WARNING,DEBUG_LEVEL);

	if (DEBUG_LEVEL>=LOG_DEBUG)

		DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Returning from wait for client init...\n"));

	if (!(_beginthread(manage_ras_thread,0,p_ras_thread_data)==-1)) {

		if (DEBUG_LEVEL>=LOG_INFO)

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Launched ras thread manager thread...\n"));
	}
	else {

		if (DEBUG_LEVEL>=LOG_ERR)

			DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Could not launch ras thread manager thread.  Aborting RAS trapping thread.  Low resources?\n"));

		destroy_trap_ras_events((RAS_THREAD_DATA**)p_data);
	}

	if (DEBUG_LEVEL>=LOG_DEBUG)

		DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Releasing client init wait...\n"));

	if (!(ReleaseSemaphore(p_ras_thread_data->hRASInitSignal,1,NULL)))

		dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "ReleaseSemaphore failed in launch_ras_trap_events.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

	if (DEBUG_LEVEL>=LOG_DEBUG)

		DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Released client init wait...\n"));

	_endthread();
}

BOOL trap_ras_events(RAS_THREAD_DATA **p_ras_thread_data,BOOL init_wait)
{

	if (!(init_wait))

		if (!(ReleaseSemaphore((*p_ras_thread_data)->hClientInitSignal,1,NULL)))

			dbg_printf_and_err_str(NULL,"W:" MODULE_TAG "ReleaseSemaphore (hClientInitSignal) failed in trap_ras_events.",GetLastError(),LOG_WARNING,DEBUG_LEVEL);

	if (_beginthread(launch_ras_events_trap,0,p_ras_thread_data)==-1)

		return false;

	else {

		if (WaitForSingleObject((*p_ras_thread_data)->hRASInitSignal,INFINITE)==WAIT_OBJECT_0)

			return true;

		else {

			dbg_printf_and_err_str(NULL,"W:" MODULE_TAG "WaitForSingleObject (hRASInitSignal) failed in trap_ras_events.",GetLastError(),LOG_WARNING,DEBUG_LEVEL);

			return true;
		}
	}
}

BOOL init_thread_data(RAS_THREAD_DATA *p_ras_thread_data)
{

	BOOL     retVal=false;

	retVal=init_ras_events(p_ras_thread_data);

	if (retVal)

		retVal=init_event_thread_control(p_ras_thread_data);

	return retVal;
}

RAS_THREAD_DATA* construct_trap_ras_events(EVENT_TRAP_HANDLER p_func,void *p_context,const char *test_url,DWORD debug_level)
{

	RAS_THREAD_DATA *p_ras_thread_data=NULL;

	DWORD           dwSize=0;



	dwSize=sizeof(RAS_THREAD_DATA);

	p_ras_thread_data=safe_malloc(dwSize);

	memset(p_ras_thread_data,0,dwSize);

	if (!(init_thread_data(p_ras_thread_data))) {

		destroy_trap_ras_events(&p_ras_thread_data);

		if (DEBUG_LEVEL>=LOG_ERR)

			DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Trouble initializing essential RAS events thread related data.  Thread aborted.  Low resources?\n"));

	}
	else {

		p_ras_thread_data->p_func=p_func;

		p_ras_thread_data->p_context=p_context;

		set_online_test(p_ras_thread_data,test_url);

		set_debug_level(p_ras_thread_data,debug_level);

		if (!(init_os_version_info(p_ras_thread_data)))

			destroy_trap_ras_events(&p_ras_thread_data);
	}

	return p_ras_thread_data;
}

RAS_THREAD_DATA* construct_and_launch_trap_ras_events(EVENT_TRAP_HANDLER p_func,void *p_context,const char *test_url,DWORD debug_level)
{

	RAS_THREAD_DATA *p_ras_thread_data=NULL;

	p_ras_thread_data=construct_trap_ras_events(p_func,p_context,test_url,debug_level);

	if (!(p_ras_thread_data==NULL))

		trap_ras_events(&p_ras_thread_data,false);


	return p_ras_thread_data;
}

void destroy_trap_ras_events(RAS_THREAD_DATA **p_p_ras_thread_data)
{

	RAS_THREAD_DATA     *p_ras_thread_data=NULL;
	DWORD               hRASThread=0;


	p_ras_thread_data=*p_p_ras_thread_data;

	if (!(p_ras_thread_data)) {

		DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "No RAS thread to kill -- NULL RAS_THREAD_DATA...\n"));

		return;
	}
	else {

		if (DEBUG_LEVEL>=LOG_DEBUG)

			DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Destroying ras thread data data structure...\n"));


		hRASThread=(*p_p_ras_thread_data)->ras_notifications_thread;

		if ((hRASThread==0) || (hRASThread==-1)) {

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "No RAS thread to kill...\n"));

		}
		else {

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "RAS events destroy, signaling RAS thread manager...\n"));

			if (!(ReleaseSemaphore((*p_p_ras_thread_data)->hRASManSignal,1,NULL)))

				dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "ReleaseSemaphore failed in destroy_trap_ras_events.",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Signaled RAS thread manager.  Entering RAS kill wait...\n"));

			if (!(WaitForSingleObject((HANDLE) hRASThread,INFINITE)==WAIT_OBJECT_0))

				dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "WaitForSingleObject failed in destroy_trap_ras_events.  Too fast?",GetLastError(),LOG_CRIT,DEBUG_LEVEL);

			if (DEBUG_LEVEL>=LOG_DEBUG)

				DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Returned from RAS kill wait.  RAS wait thread killed...\n"));

		}

		if (DEBUG_LEVEL>=LOG_DEBUG)

			DBG_PRINTF((LOG_DEBUG,"I:" MODULE_TAG "Closing thread structure related handles, freeing structure...\n"));

		CloseHandle((*p_p_ras_thread_data)->hEvent[RAS_CONNECT]);

		CloseHandle((*p_p_ras_thread_data)->hEvent[RAS_DISCONNECT]);

		CloseHandle((*p_p_ras_thread_data)->hEvent[THREAD_RETURN]);

		CloseHandle((*p_p_ras_thread_data)->hClientInitSignal);

		CloseHandle((*p_p_ras_thread_data)->hRASInitSignal);

		CloseHandle((*p_p_ras_thread_data)->hRASManSignal);

		if (p_osVersionInfo)

			free(p_osVersionInfo);

		p_osVersionInfo=NULL;

		free(*p_p_ras_thread_data);

		*p_p_ras_thread_data=NULL;
	}
}
