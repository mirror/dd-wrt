/*

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
	Install, start, stop, remove NT service routines.
	
	Author: Bryan Hoover (bhoover@wecs.com)
	Date:   November 2007

	Based on: mickem, date 03-13-2004
	
	History:
		November 2007 - project begin.
*/

#define MODULE_TAG "SERVICE: "

#define UNICODE

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>

#include "service.h"
#include "debug_service.h"
#include "base64.h"
#include "os.h"


int install_service(wchar_t *szName,wchar_t *szDisplayName,wchar_t *szDependencies,DWORD dwServiceType)
{

	SC_HANDLE   hSCManager;
	SC_HANDLE   hService;

	wchar_t     szPath[1024];

	BOOL        retVal=true;


	SetLastError(0);


	if (!(GetModuleFileName(NULL,szPath,1024))) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "GetModuleFileName failed.  Could not install service",
		                       GetLastError(),LOG_ERR,LOG_DEBUG);

		retVal=false;
	}
	else {

		hSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

		if (!hSCManager) {

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not install service",
			                       GetLastError(),LOG_ERR,LOG_DEBUG);

			retVal=false;
		}
		else {

			hService=CreateService(hSCManager,szName,szDisplayName,SERVICE_ALL_ACCESS,dwServiceType,
			                       SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,szPath,NULL,NULL,szDependencies,NULL,NULL);

			if (hService)

				CloseServiceHandle(hService);

			else {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "CreateService failed.  Could not install service",
				                       GetLastError(),LOG_ERR,LOG_DEBUG);

				retVal=false;
			}

			CloseServiceHandle(hSCManager);
		}
	}

	return retVal;
}

SC_HANDLE get_service_handle(wchar_t *szName,DWORD access_type)
{

	SC_HANDLE      hService=NULL;
	SC_HANDLE      hSCManager;


	SetLastError(0);


	hSCManager=OpenSCManager(NULL,NULL,access_type);


	if (!hSCManager) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not get service handle",GetLastError(),LOG_ERR,LOG_DEBUG);

	}
	else {

		hService=OpenService(hSCManager,szName,SERVICE_ALL_ACCESS);

		if (!(hService)) {

			CloseServiceHandle(hSCManager);

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open service.  Could not get service handle",GetLastError(),LOG_ERR,LOG_DEBUG);
		}
	}

	return hService;
}

int start_service(wchar_t *szName,int argc,wchar_t *argv[])
{

	SC_HANDLE      hService;
	SC_HANDLE      hSCManager;

	SERVICE_STATUS service_status;

	int            retVal=true;


	SetLastError(0);


	hSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	if (!hSCManager) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not start service",GetLastError(),LOG_ERR,LOG_DEBUG);

		retVal=false;

	}
	else {

		hService=OpenService(hSCManager,szName,SERVICE_ALL_ACCESS);

		if (!(hService)) {

			CloseServiceHandle(hSCManager);

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open service.  Could not start service",GetLastError(),LOG_ERR,LOG_DEBUG);

			retVal=false;

		}

		else {

			if (!(StartService(hService,argc,argv))) {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "StartService failed.  Could not start service",GetLastError(),LOG_ERR,LOG_DEBUG);

				retVal=false;
			}
			else {

				INFO_PRINTF((NULL,"Attempting service start..."));

				Sleep(1000);

				INFO_PRINTF((NULL,"."));

				while(QueryServiceStatus(hService,&service_status)) {

					if (!(service_status.dwCurrentState==SERVICE_START_PENDING))

						break;

					else {

						INFO_PRINTF((NULL,"."));

						Sleep(1000);

						INFO_PRINTF((NULL,"."));
					}
				}

				INFO_PRINTF((NULL,"\n"));

				if (!(service_status.dwCurrentState==SERVICE_RUNNING )) {

					dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Failed service start.  Could not start service",GetLastError(),LOG_ERR,LOG_DEBUG);

					retVal=false;
				}
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hSCManager);
	}

	return retVal;
}

int uninstall_service(wchar_t *szName)
{

	SC_HANDLE   hService;
	SC_HANDLE   hSCManager;

	int         retVal=true;


	SetLastError(0);


	stop_service(szName);

	hSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	if (!hSCManager) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not delete service",GetLastError(),LOG_ERR,LOG_DEBUG);

		retVal=false;
	}
	else {

		hService=OpenService(hSCManager,szName,SERVICE_ALL_ACCESS);

		if (!(hService)) {

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open service.  Could not delete service",GetLastError(),LOG_ERR,LOG_DEBUG);

			retVal=false;
		}
		else {

			if(!(DeleteService(hService))) {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "DeleteService failed.  Could not delete service",GetLastError(),LOG_ERR,LOG_DEBUG);

				retVal=false;
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hSCManager);
	}

	return retVal;
}

int stop_service(wchar_t *szName)
{

	SC_HANDLE      hService;
	SC_HANDLE      hSCManager;

	SERVICE_STATUS service_status;

	int            retVal=true;


	SetLastError(0);


	hSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	if (!hSCManager) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not stop service",GetLastError(),LOG_ERR,LOG_DEBUG);

		retVal=false;

	}
	else {

		hService=OpenService(hSCManager,szName,SERVICE_ALL_ACCESS);

		if (!(hService)) {

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open service.  Could not delete service",GetLastError(),LOG_ERR,LOG_DEBUG);

			retVal=false;
		}
		else {

			if (!(ControlService(hService,SERVICE_CONTROL_STOP,&service_status))) {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "ControlService failed.  Could not stop service",GetLastError(),LOG_ERR,LOG_DEBUG);

				retVal=false;
			}
			else {

				INFO_PRINTF((NULL,"Attempting service stop..."));

				Sleep(1000);

				INFO_PRINTF((NULL,"."));

				while(QueryServiceStatus(hService,&service_status)) {

					if (!(service_status.dwCurrentState==SERVICE_STOP_PENDING))

						break;

					else {

						INFO_PRINTF((NULL,"."));

						Sleep(1000);

						INFO_PRINTF((NULL,"."));
					}
				}

				INFO_PRINTF((NULL,"\n"));

				if (service_status.dwCurrentState!=SERVICE_STOPPED) {

					dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "QueryServiceStatus failed.  Could not stop service",GetLastError(),LOG_ERR,LOG_DEBUG);

					retVal=false;
				}
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hSCManager);
	}

	return retVal;
}

int set_service_description(wchar_t *szName, wchar_t *szDesc)
{

	SERVICE_DESCRIPTION    service_desc;

	SC_HANDLE              hSCManager;
	SC_HANDLE              hService;

	BOOL                   retVal=true;

	HINSTANCE              hAdvapi32_lib=NULL;
	FARPROC                pf_ChangeServiceConfig2=NULL;


	SetLastError(0);


	hAdvapi32_lib=LoadLibrary(L"advapi32");

	if (!(hAdvapi32_lib)) {

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "LoadLibrary failed in set_service_description.  " \
		                       "Could not load advapi32.dll in set_service_description.  Could not add service " \
		                       "description",GetLastError(),LOG_ERR,LOG_DEBUG);

		return false;
	}
	else {

		pf_ChangeServiceConfig2=GetProcAddress(hAdvapi32_lib,"ChangeServiceConfig2W");

		if (!(pf_ChangeServiceConfig2)) {

			dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "GetProcAddress failed in set_service_description.  " \
			                       "Could not add service description",GetLastError(),LOG_ERR,LOG_DEBUG);

			retVal=false;
		}
		else {

			hSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

			if (!hSCManager) {

				dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open scm.  Could not add service description",GetLastError(),LOG_ERR,LOG_DEBUG);

				retVal=false;
			}
			else {

				hService=OpenService(hSCManager,szName,SERVICE_ALL_ACCESS);

				if (!hService) {

					dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "Could not open service.  Could not delete service",GetLastError(),LOG_ERR,LOG_DEBUG);

					retVal=false;
				}

				else {

					service_desc.lpDescription=szDesc;

					if (!(pf_ChangeServiceConfig2(hService,SERVICE_CONFIG_DESCRIPTION,&service_desc))) {

						dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "ChangeServiceConfig2 failed.  Could " \
						                       "not add service description",GetLastError(),LOG_ERR,LOG_DEBUG);

						retVal=false;
					}

					CloseServiceHandle(hService);
				}

				CloseServiceHandle(hSCManager);
			}
		}

		FreeLibrary(hAdvapi32_lib);
	}

	return retVal;
}
