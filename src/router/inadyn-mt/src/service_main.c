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
	Install, start, stop, remove NT service routines, and test driver.
	
	Author: Bryan Hoover (bhoover@wecs.com)
	Date: November 2007
	
	History:
		November - project begin.
*/
#define MODULE_TAG "SERVICE_MAIN: "
#define W_MODULE_TAG L"SERVICE_MAIN: "

#define UNICODE

/*
flag where actual arguments from scm service call begin,
as scm may not behave the same over different NT versions.
*/
#define ARGS_START_MARK L"*"
#define MAX_SERVICE_CMD 256

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>

#include <ntverp.h>

#include "debug_if.h"
#include "get_cmd.h"
#include "service_main.h"
#include "service.h"
#include "debug_service.h"
#include "base64.h"
#include "unicode_util.h"
#include "safe_mem.h"

static RC_TYPE service_install_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_uninstall_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_start_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_exit_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_restart_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

static BOOL service_command_start(int argc,wchar_t *argv[]);
static BOOL service_command_install(int argc,wchar_t *argv[]);
static BOOL service_command_uninstall(int argc,wchar_t *argv[]);
static BOOL service_command_exit(int argc,wchar_t *argv[]);
static BOOL service_command_restart(int argc,wchar_t *argv[]);

typedef BOOL (*SERVICE_COMMAND)(int argc, wchar_t* argv[]);

typedef struct SERVICE_COMMAND_ARRAY {

	SERVICE_COMMAND service_command[MAX_SERVICE_CMD];

	int             size;

} SERVICE_COMMAND_ARRAY;

typedef struct MAIN_ARGS {

	int       argc;

	wchar_t   **argv;

	wchar_t   *appendedArgs[512];

} MAIN_ARGS;

static volatile DWORD         global_fdwControl=0;

static SERVICE_COMMAND_ARRAY  service_command_array;
static BOOL                   isWShowWindow=false;
static SERVICE_CLIENT_ENTRY   service_client=NULL;
static SERVICE_EVENT_HANDLER  p_service_event_handler=NULL;
static SERVICE_STATUS         service_status;
static SERVICE_STATUS_HANDLE  hService=NULL;

static HANDLE                 g_hServiceMainThread=NULL;

static FILE                   *pLOG_FILE=NULL;

static MAIN_ARGS              main_args;

static wchar_t                *szServiceName=NULL;
static wchar_t                *szServiceDesc=NULL;
static SERVICE_REG_ENTRY      *service_reg_params=NULL;
static wchar_t                *serviceArgs[70];

static CMD_DESCRIPTION_TYPE service_cmd_options_table[] =
    {
        {"-i",1,{service_install_handler,NULL,1},"install service"},
		{"-n",1,{service_name_handler,NULL,1},"service name"},
        {"-x",0,{service_uninstall_handler,NULL,0},"uninstall service" },
        {"-s",0,{service_start_handler, NULL,0},"start service" },
        {"-e",0,{service_exit_handler, NULL,0},"exit service" },
        {"-r",0,{service_restart_handler, NULL,0},"restart service" },
        {NULL,0,{0, NULL,0},"" }
    };

static add_service_command(SERVICE_COMMAND service_command)
{

	if (service_command_array.size<MAX_SERVICE_CMD)

		service_command_array.service_command[service_command_array.size++]=service_command;
}

static RC_TYPE service_install_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{


	add_service_command(service_command_install);

	if (current_nr<p_cmd->argc)

		if (strncmp(p_cmd->argv[current_nr],"-",1)) {

			free(szServiceDesc);

			utf_8_to_16(utf_malloc_8_to_16(&szServiceDesc,p_cmd->argv[current_nr]),p_cmd->argv[current_nr]);
		}


	return RC_OK;
}

static get_service_name(wchar_t **dest,char *src)
{
	utf_8_to_16(utf_malloc_8_to_16(dest,src),src);
}

static RC_TYPE service_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	get_service_name(&szServiceName,p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE service_uninstall_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	add_service_command(service_command_uninstall);

	return RC_OK;
}

static RC_TYPE service_start_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	add_service_command(service_command_start);

	return RC_OK;
}

static RC_TYPE service_exit_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	add_service_command(service_command_exit);

	return RC_OK;
}

static RC_TYPE service_restart_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	add_service_command(service_command_restart);

	return RC_OK;
}

static RC_TYPE service_cmd_error_handler(CMD_DATA *p_cmd, int *current_nr, void *p_context)
{

	/*ignore unknown parameters (logged)*/

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Non service server arugment at %d:  %s.  " \
						"Passing to client...\n",*current_nr-1,p_cmd->argv[*current_nr-1]));

	return RC_OK;
}

static LPSERVICE_STATUS set_service_status_struct(LPSERVICE_STATUS p_service_status)
{

	memset(p_service_status,0,sizeof(SERVICE_STATUS));

	p_service_status->dwCheckPoint=0;

	p_service_status->dwControlsAccepted=(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);

	p_service_status->dwCurrentState=SERVICE_START_PENDING;

	p_service_status->dwServiceSpecificExitCode=0;

	p_service_status->dwServiceType=SERVICE_WIN32_OWN_PROCESS;

	p_service_status->dwWaitHint=0;

	p_service_status->dwWin32ExitCode=0;

	return p_service_status;
}

static void WINAPI ServiceHandler(DWORD fdwControl)
{
	DWORD          status_ret=0;


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Entered ServiceHandler...\n"));


	global_fdwControl=fdwControl;


	switch(fdwControl)
	{

	case SERVICE_CONTROL_STOP:

	case SERVICE_CONTROL_SHUTDOWN:

		service_status.dwWin32ExitCode=0;

		if (service_status.dwCurrentState==SERVICE_STOP_PENDING) {

			service_status.dwCheckPoint+=1;

			status_ret=SetServiceStatus(hService,&service_status);

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "ServiceHandler got stop request (%d).  " \
								"Waiting.  Set pending...\n",fdwControl));

			/*ServiceMain will set stopped*/
		}
		else {

			/*tell client we got stop event -- hopefully, client will return :)*/

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "ServiceHandler got stop request (%d).  " \
								"Asking client to return...\n",fdwControl));

			p_service_event_handler(SERVICE_CONTROL_STOP);

			service_status.dwCurrentState=SERVICE_STOP_PENDING;

			service_status.dwCheckPoint+=1;

			service_status.dwWaitHint=20000;

			status_ret=SetServiceStatus(hService,&service_status);

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "ServiceHandler got stop request (%d).  " \
								"Set pending...\n",fdwControl));
		}

		break;

	case SERVICE_CONTROL_INTERROGATE:

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "ServiceHandler got SERVICE_INTERROGATE " \
							"(%d)...\n",fdwControl));

		status_ret=SetServiceStatus(hService,&service_status);

		break;
	}

	if (!(status_ret))

		dbg_printf_and_err_str(pLOG_FILE, "I:" MODULE_TAG "SetServiceStatus returned false in "\
								"ServiceHandler.",GetLastError(),LOG_ERR,LOG_DEBUG);

}

static DWORD regDataSize(DWORD regValueType,void *regValue)
{

	DWORD retVal=0;

	/*add types as needed -- presently, need only ansii REG_SZ*/

	switch(regValueType)
	{

	case REG_NONE:

		break;

	case REG_SZ:

		retVal=wcslen(regValue)*sizeof(wchar_t)+sizeof(wchar_t);

		break;

	case REG_EXPAND_SZ:

		break;

	case REG_BINARY:

		break;

	case REG_DWORD:

		break;

	case REG_DWORD_BIG_ENDIAN:

		break;

	case REG_LINK:

		break;

	case REG_MULTI_SZ:

		break;

	case REG_RESOURCE_LIST:

		break;

	case REG_FULL_RESOURCE_DESCRIPTOR:

		break;

	case REG_RESOURCE_REQUIREMENTS_LIST:

		break;

	case REG_QWORD:

		break;
	}

	return retVal;
}

static BOOL set_up_registry(SERVICE_REG_ENTRY *reg_keys_and_values)
{

	HKEY       hSubKey;
	wchar_t    *szRegStr;
	int        lenRegStr;
	DWORD      dwDisposition;
	int        i=0;
	BOOL       retVal=ERROR_SUCCESS;
	BOOL       closeRet;
	char       *utf_8_1;
	char       *utf_8_2;


	lenRegStr=(wcslen(L"SYSTEM\\CurrentControlSet\\Services\\")+wcslen(szServiceName));


	while (wcscmp(reg_keys_and_values->regKey,L"")) {

		szRegStr=safe_malloc((lenRegStr+wcslen(reg_keys_and_values->regKey))*sizeof(wchar_t)+sizeof(wchar_t));

		wcscpy(szRegStr,L"SYSTEM\\CurrentControlSet\\Services\\");

		wcscat(szRegStr,szServiceName);

		wcscat(szRegStr,reg_keys_and_values->regKey);


		retVal=RegCreateKeyEx(HKEY_LOCAL_MACHINE,szRegStr,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hSubKey,
								&dwDisposition);

		if (!(retVal==ERROR_SUCCESS)) {

			utf_malloc_16_to_8(&utf_8_1,szRegStr);

			dbg_printf_and_err_str(pLOG_FILE, (char *) "E:" MODULE_TAG "RegCreateKeyEx failed in set_up_registry.  "\
									"Attempted key:  %s...  Could not set up registry.",retVal,LOG_ERR,LOG_DEBUG,
									utf_16_to_8(utf_8_1,szRegStr));

			free(utf_8_1);
		}
		else {

			for (i=0;wcscmp(reg_keys_and_values->regKeyValueNameAr[i],L"");i++) {

				retVal=RegSetValueEx(hSubKey,reg_keys_and_values->regKeyValueNameAr[i],0,
										reg_keys_and_values->regKeyValueTypeAr[i],
										reg_keys_and_values->regKeyValueAr[i],
										regDataSize(reg_keys_and_values->regKeyValueTypeAr[i],
										reg_keys_and_values->regKeyValueAr[i]));


				if (!(retVal==ERROR_SUCCESS)) {

					utf_malloc_16_to_8(&utf_8_1,reg_keys_and_values->regKeyValueNameAr[i]);
					utf_malloc_16_to_8(&utf_8_2,reg_keys_and_values->regKeyValueAr[i]);

					dbg_printf_and_err_str(pLOG_FILE,(char *) "C:" MODULE_TAG "Error attempting to set registry "\
											"value name, %s, to %s...",retVal,LOG_CRIT,LOG_DEBUG,utf_16_to_8(utf_8_1,
											reg_keys_and_values->regKeyValueNameAr[i]),utf_16_to_8(utf_8_2,
											reg_keys_and_values->regKeyValueAr[i]));

					free(utf_8_1);
					free(utf_8_2);

					break;
				}
			}

			closeRet=RegCloseKey(hSubKey);

			if (!(closeRet==ERROR_SUCCESS)) {

				utf_malloc_16_to_8(&utf_8_1,szRegStr);

				dbg_printf_and_err_str(pLOG_FILE,(char *) "C:" MODULE_TAG "Error attempting to close "\
										"registry key, %s...",closeRet,LOG_CRIT,LOG_DEBUG,utf_16_to_8(utf_8_1,szRegStr));

				free(utf_8_1);
				free(szRegStr);
			}
		}

		free(szRegStr);


		if (!(retVal==ERROR_SUCCESS))

			return false;

		reg_keys_and_values++;
	}

	return (retVal==ERROR_SUCCESS);
}

/*set and get reg values functions -- presently string type only
*/

static BOOL set_reg_value(HKEY hKey,LPTSTR szSubKey,LPTSTR szValueName,DWORD dwType,LPBYTE bpValue,DWORD dwSize)
{

	HKEY      hSubKey;
	BOOL      retVal=ERROR_SUCCESS;
	BOOL      closeRet;
	char      *utf_8;


	retVal=RegOpenKeyEx(hKey,szSubKey,0,KEY_ALL_ACCESS,&hSubKey);

	if (!(retVal==ERROR_SUCCESS)) {

		dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "Error attempting to open registry...",
								retVal,LOG_CRIT,LOG_DEBUG);	
	}
	else {

		retVal=RegSetValueEx(hSubKey,szValueName,0,dwType,bpValue,dwSize);

		if (!(retVal==ERROR_SUCCESS)) {

			utf_malloc_16_to_8(&utf_8,szValueName);


			dbg_printf_and_err_str(pLOG_FILE,(char *) "C:" MODULE_TAG "Error attempting to "\
									"set registry value, %s...",retVal,LOG_CRIT,LOG_DEBUG,
									(char *) utf_16_to_8(utf_8,szValueName));
			free(utf_8);
		}


		closeRet=RegCloseKey(hSubKey);

		if (!(closeRet==ERROR_SUCCESS))

			dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "Error attempting to close registry key...",
									closeRet,LOG_CRIT,LOG_DEBUG);
	}

	return (retVal==ERROR_SUCCESS);
}

static BOOL get_reg_value(HKEY hKey,LPTSTR szSubKey,LPTSTR szValueName,LPBYTE *bpRetValue)
{

	HKEY      hSubKey;

	DWORD     bcSize;

	BOOL      retVal=ERROR_SUCCESS;
	BOOL      closeRet;
	char      *utf_8;


	retVal=RegOpenKeyEx(hKey,szSubKey,0,KEY_ALL_ACCESS,&hSubKey);

	if (!(retVal==ERROR_SUCCESS)) {

		dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "Error attempting to open registry...",
								retVal,LOG_CRIT,LOG_DEBUG);
	}
	else {

		bcSize=128;


		while(1) {

			*bpRetValue=safe_malloc(bcSize+2);

			memset(*bpRetValue,0,bcSize+2);

			retVal=RegQueryValueEx(hSubKey,szValueName,NULL,NULL,*bpRetValue,&bcSize);

			if (!(retVal==ERROR_MORE_DATA))

				break;

			else

				free(*bpRetValue);
		}

		if (!(retVal==ERROR_SUCCESS)) {

			utf_malloc_16_to_8(&utf_8,szValueName);


			dbg_printf_and_err_str(pLOG_FILE,(char *) "C:" MODULE_TAG "Error attempting to "\
									"get registry value, %s...",retVal,LOG_CRIT,LOG_DEBUG,
									(char *) utf_16_to_8(utf_8,szValueName));

			free(utf_8);
		}


		closeRet=RegCloseKey(hSubKey);

		if (!(closeRet==ERROR_SUCCESS))

			dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "Error attempting to close registry key...",
									closeRet,LOG_CRIT,LOG_DEBUG);
	}

	return (retVal==ERROR_SUCCESS);
}

static int get_args_from_str(wchar_t *szDest[],wchar_t *szParamStr)
{
	int     charCount=0;
	int     i=0;
	int		is_quoted=0;


	if (wcslen(szParamStr)) {

		for (;!(wcsncmp(szParamStr,L" ",1));szParamStr++); /*spaces*/


		if (*szParamStr) {

			while (1) {				

				/*move to end of argument -- separated by space, end-quote, or eol*/

				if (!(is_quoted))

					/*option, or unquoted option parameter*/

					for (charCount=0;wcsncmp((szParamStr+charCount),L"\0",1) && wcsncmp((szParamStr+charCount),L" ",1);charCount++);

				else {

					/*quoted option parameter*/

					for (charCount=0;wcsncmp((szParamStr+charCount),L"\0",1) && wcsncmp((szParamStr+charCount),L"\"",1);charCount++);

					is_quoted=0;
				}


				charCount++;

				szDest[i]=safe_malloc(charCount*sizeof(wchar_t));

				memset(szDest[i],0,charCount*sizeof(wchar_t));

				wcsncat(szDest[i],szParamStr,charCount-1);

				i++;

				szParamStr+=charCount;

				if (!(wcsncmp(szParamStr,L"\0",1)))

					break;


				if (wcsncmp((szParamStr),L"\"",1))

					for (;!(wcsncmp(szParamStr,L" ",1));szParamStr++);  /*spaces*/

				if (!(wcsncmp((szParamStr),L"\"",1))) { /*beginning of quoted option parameter?*/
					
					is_quoted=1;

					szParamStr++;
				}


				if (!(wcsncmp(szParamStr,L"\0",1)))

					break;
			}
		}
	}

	return i;
}

static int get_args_from_app_parameters(wchar_t *szDest[],wchar_t *szServiceName)
{

	wchar_t    *szParamStr=NULL;
	int        retVal;


	get_app_parameters(&szParamStr,szServiceName,L"\\Parameters",L"AppParameters");

	retVal=get_args_from_str(szDest,szParamStr);


	free(szParamStr);


	return retVal;
}

static int indexOf(wchar_t *src[],int srcSize,wchar_t *sought)
{

	int     i=0;


	if (!(i<srcSize))

		return -1;

	while (wcscmp(src[i],sought)) {

		i++;

		if (!(i<srcSize))

			return -1;
	}

	return i;
}

static void remove_service_meta_args(int *argc,wchar_t *argv[],wchar_t *dest[])
{

	int i;


	i=indexOf(argv,*argc,ARGS_START_MARK);

	if (!(i==-1)) {

		*argc=(int)*(argv[++i]);

		argv+=++i;
	}

	for (i=0;i<*argc;i++) {

		dest[i]=argv[i];
	}
}

/*if still in running state, then
  ask scm to stop service
*/
static void request_stop_if_needed()
{

	SERVICE_STATUS	service_status;
	DWORD			status_ret=NO_ERROR;
	SC_HANDLE		hService=NULL;


	memset(&service_status,0,sizeof(SERVICE_STATUS));


	if (!(hService=get_service_handle(szServiceName,SC_MANAGER_ALL_ACCESS)))

		DBG_SERVICE_PRINTF((pLOG_FILE,"C:" MODULE_TAG "Could not get handle to service in request_stop_if_needed...\n"));

	else {

		if (!(QueryServiceStatus(hService,&service_status)))

			dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "QueryServiceStatus failed in request_stop_if_client_return"\
									"...\n",GetLastError(),LOG_CRIT,LOG_DEBUG);

		else {

			if (service_status.dwCurrentState==SERVICE_RUNNING) {

				DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Service client exited.  Stopping service...\n"));


				if (!(status_ret=ControlService(hService,SERVICE_CONTROL_STOP,(LPSERVICE_STATUS) &service_status)))

					dbg_printf_and_err_str(pLOG_FILE,"C:" MODULE_TAG "ControlService failed in request_stop_if_client_return"\
											"...\n",GetLastError(),LOG_CRIT,LOG_DEBUG);

				else

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Request for service stop succeded...\n"));

			}
		}

		CloseServiceHandle(hService);
	}
}

static void service_work_thread(void *worker_args)
{

	MAIN_ARGS *p_worker_args=NULL;

	wchar_t   *dest[50];
	char      *utf_8_ar[50];


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Entered service client thread...\n"));

	p_worker_args=(MAIN_ARGS *) worker_args;

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Fixing up client arguments...\n"));

	remove_service_meta_args(&(p_worker_args->argc),p_worker_args->argv,dest);

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling client...\n"));

	service_client(p_worker_args->argc,utf_16_to_8_ar(utf_8_ar,dest,p_worker_args->argc),pLOG_FILE);

	request_stop_if_needed();

	utf_free_ar(utf_8_ar,p_worker_args->argc);

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Service client thread returned from client...\n"));

	_endthread();
}

static void WINAPI ServiceMain(DWORD dwArgc,LPTSTR lpszArgv[])
{

	DWORD      hServiceWorkThread=0;
	int        argIndex=0;

	/*
	Create a duplicate of this thread's handle so the
	primary thread can wait on it before exiting.  

	This is a safety measure -- in case service dispatcher needs
	to do any clean up, caller will wait on this handle,
	lest we pull the thread out from under service dispatcher.
	*/

	if (!(DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&g_hServiceMainThread,0,0,DUPLICATE_SAME_ACCESS ))) {

		g_hServiceMainThread=NULL;

		dbg_printf_and_err_str(pLOG_FILE,"E: " MODULE_TAG "DuplicateHandle failed in ServiceMain.  " \
								"Not fatal.  Possible awkward service shutdown...",GetLastError(),LOG_ERR,LOG_DEBUG);
	}


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Entered ServiceMain...\n"));

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Initing service_status structure...\n"));


	set_service_status_struct(&service_status);


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Registering service handler, ServiceHandler...\n"));

	hService=RegisterServiceCtrlHandler(szServiceName,ServiceHandler);

	if (!(hService)) {

		dbg_printf_and_err_str(pLOG_FILE,"I:" MODULE_TAG "RegisterServiceCtrlHandler failed " \
								"in ServiceMain.  Not good.  Aborting...",GetLastError(),LOG_CRIT,LOG_DEBUG);

	}
	else {


		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Registered handler...\n"));

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Setting status SERVICE_RUNNING...\n"));


		service_status.dwCurrentState=SERVICE_RUNNING;

		SetServiceStatus(hService,&service_status);


		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Set status SERVICE_RUNNING...\n"));


		main_args.argc=dwArgc;

		main_args.argv=lpszArgv;


		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Attempting service client thread launch...\n"));


		hServiceWorkThread=_beginthread(service_work_thread,0,&main_args);

		if (hServiceWorkThread==-1) {

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Service work thread failed launch.  Setting SERVICE_STOPPED...\n"));

			service_status.dwCurrentState=SERVICE_STOPPED;

			if (!(SetServiceStatus(hService,&service_status)))

				dbg_printf_and_err_str(pLOG_FILE,"I:" MODULE_TAG "SetServiceStatus failed in ServiceMain...",GetLastError(),LOG_CRIT,LOG_DEBUG);

		}
		else {

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Service work thread launched.  " \
								"Entering wait for service client thread...\n"));

			if (!(WaitForSingleObject((HANDLE) hServiceWorkThread,INFINITE))==WAIT_OBJECT_0)

				dbg_printf_and_err_str(pLOG_FILE,"I:" MODULE_TAG "WaitForSingleObject failed in ServiceMain...",GetLastError(),LOG_CRIT,LOG_DEBUG);

			else

				DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Returned from wait for service client thread.  " \
									"Setting SERVICE_STOPPED...\n"));


			while (!(global_fdwControl==SERVICE_CONTROL_STOP || global_fdwControl==SERVICE_CONTROL_SHUTDOWN)) {

				Sleep(1000);
			}

			service_status.dwCurrentState=SERVICE_STOPPED;

			if (!(SetServiceStatus(hService,&service_status)))

				dbg_printf_and_err_str(pLOG_FILE,"I:" MODULE_TAG "SetServiceStatus failed in ServiceMain...",GetLastError(),LOG_CRIT,LOG_DEBUG);
		}
	}
}

static void removeServiceParams(int *argc,wchar_t *argv[],wchar_t *dest[])
{
	int   paramCount=0;
	int   i=0;


	while (i<*argc) {

		if (!(!(wcscmp(argv[i],_SERVICE_INSTALL)) || !(wcscmp(argv[i],_SERVICE_UNINSTALL)) 
			|| !(wcscmp(argv[i],_SERVICE_START)) || !(wcscmp(argv[i],_SERVICE_EXIT)) 
			|| !(wcscmp(argv[i],_SERVICE_RESTART)) || !(wcscmp(argv[i],_SERVICE_NAME)))) 
		{

			dest[paramCount]=argv[i];

			paramCount+=1;

			i++;
		}
		else {

			i++;

			if (!(i<*argc))

				break;

			/*_SERVICE_INSTALL, can, and _SERVICE_NAME, will have a parameter*/

			if (!(wcscmp(argv[i-1],_SERVICE_INSTALL)))

				if (wcsncmp(argv[i],L"-",1))

					i++;

			if (!(wcscmp(argv[i-1],_SERVICE_NAME)))

				if (wcsncmp(argv[i],L"-",1))

					i++;
		}
	}

	*argc=paramCount;
}

static void work_around_service_meta_args(int *argc,wchar_t *argv[],wchar_t *dest[])
{
	int i=0;


	/*insert a marker, followed by number of parameters (*argc), followed by argv*/


	dest[0]=safe_malloc(wcslen(argv[0])*sizeof(wchar_t)+sizeof(wchar_t));

	dest[1]=safe_malloc(wcslen(ARGS_START_MARK)*sizeof(wchar_t)+sizeof(wchar_t));

	dest[2]=safe_malloc(sizeof(wchar_t)*2);

	memset(dest[2],0,sizeof(wchar_t)*2);

	wcscpy(dest[0],argv[0]);

	wcscpy(dest[1],ARGS_START_MARK);


	*dest[2]=*argc;


	for (i=0;i<*argc;i++) {

		dest[i+3]=safe_malloc(wcslen(argv[i])*sizeof(wchar_t)+sizeof(wchar_t));

		wcscpy(dest[i+3],argv[i]);
	}

	*argc+=3;
}

static BOOL service_command_exit(int argc,wchar_t *argv[])
{

	char utf_8[1536];


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "set_up_service Stopping service...\n"));

	if (stop_service(szServiceName)) {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "set_up_service returned from stop_service.  " \
							"Appears service (%s) stopped okay...\n",utf_16_to_8(utf_8,szServiceName)));

		return true;
	}
	else {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "set_up_service returned from stop_service.  " \
							"Appears service (%s) NOT stopped.  Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

		return false;
	}
}

static BOOL service_command_uninstall(int argc,wchar_t *argv[])
{

	char utf_8[1536];


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling service uninstall...\n"));

	if (uninstall_service(szServiceName)) {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) uninstalled okay...\n",utf_16_to_8(utf_8,szServiceName)));

		return true;

	}
	else {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) NOT uninstalled okay.  " \
							"Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

		return false;
	}
}

static BOOL install_add_name_param()
{

	wchar_t	*swcImagePath;
	wchar_t *swcImagePathParamed;
	BOOL	ret_val;

	
	if (!(get_app_parameters(&swcImagePath,szServiceName,L"\\",L"ImagePath")))

		return false;

	swcImagePathParamed=safe_malloc((wcslen(swcImagePath)+wcslen(L" ")+wcslen(_SERVICE_NAME)+wcslen(L" ")
									+wcslen(szServiceName)+1)*sizeof(wchar_t));

	wcscat(wcscat(wcscat(wcscat(wcscpy(swcImagePathParamed,swcImagePath),L" "),_SERVICE_NAME),L" "),
		szServiceName);

	
	ret_val=set_app_parameters(szServiceName,L"\\",L"ImagePath",swcImagePathParamed);


	free(swcImagePathParamed);


	return ret_val;
}

/*return true iff all processing attempted succeeds*/
static BOOL service_command_install(int argc,wchar_t *argv[])
{

	char utf_8[1536];


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling service install...\n"));

	if (!(install_service(szServiceName,szServiceName,NULL,SERVICE_WIN32_OWN_PROCESS))) {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) NOT installed okay.  " \
							"Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

		return false;

	}
	else {

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) installed okay...\n",
							utf_16_to_8(utf_8,szServiceName)));

		if (!(install_add_name_param())) {

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears adding service params to (%s) ImagePath failed.  " \
								"Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

			return false;
		} 
		else {

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears adding service params to (%s) ImagePath went " \
								"okay...\n",utf_16_to_8(utf_8,szServiceName)));

			if (service_reg_params)
			{

				DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Attempting default registry set up for (%s) ...\n",
									utf_16_to_8(utf_8,szServiceName)));

				if (set_up_registry(service_reg_params))

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears default registry set up (%s) went okay...\n",
										utf_16_to_8(utf_8,szServiceName)));

				else {

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears default registry set up (%s) " \
										"NOT okay.  Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

					return false;
				}
			}

			if (!(*szServiceDesc))

				return true;

			else {

				if (set_service_description(szServiceName,szServiceDesc)) {

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears set service description (%s) okay...\n",
										utf_16_to_8(utf_8,szServiceName)));

					return true;

				}
				else {

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears set service description (%s) NOT okay.  " \
										"Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));

					return false;
				}
			}
		}
	}
}

static BOOL service_command_start(int argc,wchar_t *argv[])
{

	wchar_t  *argsMinusServiceArgs[75];
	wchar_t  *serviceWorkaroundArgs[75];

	char     utf_8[1536];

	int      i=0;

	BOOL     retVal;


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling service start, after closing service log...\n"));

	if (pLOG_FILE)

		fclose(pLOG_FILE);

	removeServiceParams(&argc,argv,argsMinusServiceArgs);

	work_around_service_meta_args(&argc,argsMinusServiceArgs,serviceWorkaroundArgs);

	retVal=start_service(szServiceName,argc,serviceWorkaroundArgs);

	if (retVal)

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) started okay...\n",
							utf_16_to_8(utf_8,szServiceName)));

	else

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Appears service (%s) could NOT be started.  " \
							"Service server client returning unexpectedly?  Check paramters?.  " \
							"Check logging output...\n",utf_16_to_8(utf_8,szServiceName)));


	for (i=0;i<argc;i++) {
		
		free(serviceWorkaroundArgs[i]);
	}


	return retVal;
}

static BOOL service_command_restart(int argc,wchar_t *argv[])
{

	service_command_exit(argc,argv);

	return service_command_start(argc,argv);
}

static void init_global_data()
{

	memset(&service_command_array,0,sizeof(SERVICE_COMMAND_ARRAY));

	memset(serviceArgs,0,70);
}

/*Interface*/

BOOL set_reg_entry(wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName,
						DWORD dwType,LPBYTE bpValue,DWORD dwSize)
{
	wchar_t	*szRegStr=NULL;

	int		reg_str_len;
	BOOL	retVal;


	reg_str_len=(wcslen(L"SYSTEM\\CurrentControlSet\\Services\\")+wcslen(szServiceName)+wcslen(szRegKey))*
					sizeof(wchar_t)+sizeof(wchar_t);

	szRegStr=safe_malloc(reg_str_len);

	memset(szRegStr,0,reg_str_len);


	wcscpy(szRegStr,L"SYSTEM\\CurrentControlSet\\Services\\");

	wcscat(szRegStr,szServiceName);

	wcscat(szRegStr,szRegKey);

	retVal=set_reg_value(HKEY_LOCAL_MACHINE,szRegStr,szRegValueName,dwType,bpValue,dwSize);


	free(szRegStr);

	return retVal;
}

BOOL set_app_parameters(wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName,wchar_t *swcValue)
{

	return set_reg_entry(szServiceName,szRegKey,szRegValueName,REG_SZ,(LPBYTE) swcValue,
							(wcslen(swcValue)+1)*sizeof(wchar_t));
}

BOOL get_reg_entry(LPBYTE *bpValue,wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName)
{
	wchar_t	*szRegStr=NULL;

	int		reg_str_len;
	BOOL	retVal;


	reg_str_len=(wcslen(L"SYSTEM\\CurrentControlSet\\Services\\")+wcslen(szServiceName)+wcslen(szRegKey))*
					sizeof(wchar_t)+sizeof(wchar_t);

	szRegStr=safe_malloc(reg_str_len);

	memset(szRegStr,0,reg_str_len);

	wcscpy(szRegStr,L"SYSTEM\\CurrentControlSet\\Services\\");

	wcscat(szRegStr,szServiceName);

	wcscat(szRegStr,szRegKey);

	retVal=get_reg_value(HKEY_LOCAL_MACHINE,szRegStr,szRegValueName,bpValue);


	free(szRegStr);


	return retVal;
}

BOOL get_app_parameters(wchar_t **szParamStr,wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName)
{

	return get_reg_entry((LPBYTE *) szParamStr,szServiceName,szRegKey,szRegValueName);
}

BOOL isService()
{

	/*

	Command line service action?  Return true if scm calling, or command line with service related request (-i, -x, -s, or -e).

	Use differences in STARTUPINFO structure to tell whether called from command line, or scm:

	 From Kevin Kiley

	 STARTUPINFO from command line...

	 RCTPDW.C  @00227:main():lpsi->cb              = 68
	 RCTPDW.C  @00228:main():lpsi->lpReserved      = []
	 RCTPDW.C  @00229:main():lpsi->lpDesktop       = [WinSta0\Default]
	 RCTPDW.C  @00230:main():lpsi->lpTitle         = [S:\RCTPDW\debug\rctpdw.exe]
	 RCTPDW.C  @00231:main():lpsi->dwX             = 0
	 RCTPDW.C  @00232:main():lpsi->dwY             = 0
	 RCTPDW.C  @00233:main():lpsi->dwXSize         = 0
	 RCTPDW.C  @00234:main():lpsi->dwYSize         = 0
	 RCTPDW.C  @00235:main():lpsi->dwXCountChars   = 1310720
	 RCTPDW.C  @00236:main():lpsi->dwYCountChars   = 1
	 RCTPDW.C  @00237:main():lpsi->dwFillAttribute = 1322872
	 RCTPDW.C  @00238:main():lpsi->dwFlags         = 0
	 RCTPDW.C  @00239:main():lpsi->wShowWindow     = 1
	 RCTPDW.C  @00240:main():lpsi->cbReserved2     = 0
	 RCTPDW.C  @00241:main():lpsi->lpReserved2     = [(null)]
	 RCTPDW.C  @00242:main():lpsi->hStdInput       = -858993460
	 RCTPDW.C  @00243:main():lpsi->hStdOutput      = -858993460
	 RCTPDW.C  @00244:main():lpsi->hStdError       = -858993460

	 STARTUPINFO when Service Control Manager calls...

	 RCTPDW.C  @00227:main():lpsi->cb              = 68
	 RCTPDW.C  @00228:main():lpsi->lpReserved      = []
	 RCTPDW.C  @00229:main():lpsi->lpDesktop       = [Default]
	 RCTPDW.C  @00230:main():lpsi->lpTitle         = [S:\RCTPDW\debug\rctpdw.exe]
	 RCTPDW.C  @00231:main():lpsi->dwX             = 0
	 RCTPDW.C  @00232:main():lpsi->dwY             = 0
	 RCTPDW.C  @00233:main():lpsi->dwXSize         = 0
	 RCTPDW.C  @00234:main():lpsi->dwYSize         = 0
	 RCTPDW.C  @00235:main():lpsi->dwXCountChars   = 0
	 RCTPDW.C  @00236:main():lpsi->dwYCountChars   = 0
	 RCTPDW.C  @00237:main():lpsi->dwFillAttribute = 0
	 RCTPDW.C  @00238:main():lpsi->dwFlags         = 128
	 RCTPDW.C  @00239:main():lpsi->wShowWindow     = 0
	 RCTPDW.C  @00240:main():lpsi->cbReserved2     = 0
	 RCTPDW.C  @00241:main():lpsi->lpReserved2     = [(null)]
	 RCTPDW.C  @00242:main():lpsi->hStdInput       = -858993460
	 RCTPDW.C  @00243:main():lpsi->hStdOutput      = -858993460
	 RCTPDW.C  @00244:main():lpsi->hStdError       = -858993460

	We'll use all differences except lpDesktop.

	*/

	STARTUPINFO     startUpInfo;


	memset(&startUpInfo,0,sizeof(STARTUPINFO));

	GetStartupInfo(&startUpInfo);

	isWShowWindow=(startUpInfo.wShowWindow || !(startUpInfo.dwFlags) || startUpInfo.dwXCountChars ||
					startUpInfo.dwYCountChars || startUpInfo.dwFillAttribute);

	return (!(isWShowWindow) || (isWShowWindow && (service_command_array.size)));
}

void insert_app_paramters(int *argc,wchar_t *argv[],wchar_t *szDest[],wchar_t *szServiceName)
{
	int     i=0;
	int     commandLineArgs=0;


	if (argv) {

		szDest[0]=safe_malloc(wcslen(argv[0])*sizeof(wchar_t)+sizeof(wchar_t));

		wcscpy(szDest[0],argv[0]);

		argv++;

		i=1;
	}

	commandLineArgs=*argc-1;

	i=get_args_from_app_parameters(++szDest,szServiceName);

	*argc+=i;

	szDest+=i;


	for (i=0;i<commandLineArgs;i++) {

		szDest[i]=safe_malloc(wcslen(argv[i])*sizeof(wchar_t)+sizeof(wchar_t));

		wcscpy(szDest[i],argv[i]);
	}
}

int get_registry_parameters(wchar_t *szDest[])
{
	int i=0;
	int srcLen=0;


	while (serviceArgs[i]) {

		srcLen=wcslen(serviceArgs[i])*sizeof(wchar_t)+sizeof(wchar_t);

		szDest[i]=safe_malloc(srcLen);

		memset(szDest[i],0,srcLen);

		wcscpy(szDest[i],serviceArgs[i++]);
	}


	return i;
}

int set_up_service(int argc,wchar_t *argv[],wchar_t *szName,wchar_t *szServiceDescription,
					SERVICE_REG_ENTRY *service_reg_entry,SERVICE_CLIENT_ENTRY
					service_client_callback,SERVICE_EVENT_HANDLER service_event_handler,
					FILE *p_log_file)
{

	LPSERVICE_TABLE_ENTRY     lpServiceTableEntry=NULL;
	DWORD                     tableSize=0;

	char                      *utf_8_argv[50];

	int                       i=0;


	pLOG_FILE=p_log_file;

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Hello!  Entered set_up_service service server.  " \
						"Appending log...\n"));


	if (isWinNT()) {

		init_global_data();

		/*default to parameter -- command line may override*/
		szServiceDesc=safe_malloc(wcslen(szServiceDescription)*sizeof(wchar_t)+sizeof(wchar_t));

		wcscpy(szServiceDesc,szServiceDescription);
	}

	get_cmd_parse_data_with_error_handling(utf_16_to_8_ar(utf_8_argv,argv,argc),argc,service_cmd_options_table,service_cmd_error_handler);


	if (!(isService()) || !(isWinNT())) {

		service_client_callback(argc,utf_8_argv,pLOG_FILE);
	}

	else {

		if (!(szServiceName)) {

			szServiceName=safe_malloc((wcslen(szName)+1)*sizeof(wchar_t));

			wcscpy(szServiceName,szName); /*default passed by client*/
		}

		service_reg_params=service_reg_entry;

		if (isWShowWindow) {

			for (i=0;i<service_command_array.size;i++) {

				if (!(service_command_array.service_command[i](argc,argv))) {

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "A service command failed.  " \
										"Aborting program.  Check logging output...\n"));

					break;
				}
			}
		}

		else {

			tableSize=sizeof(SERVICE_TABLE_ENTRY)*2;

			lpServiceTableEntry=safe_malloc(tableSize);

			memset(lpServiceTableEntry,0,tableSize);

			lpServiceTableEntry->lpServiceProc=ServiceMain;

			lpServiceTableEntry->lpServiceName=szServiceName;

			p_service_event_handler=service_event_handler;

			service_client=service_client_callback;

			/*init registry parameters array in case client calls for*/
			get_args_from_app_parameters(serviceArgs,szServiceName);

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling StartServiceCtrlDispatchter for ServiceMain...\n"));

			if (!(StartServiceCtrlDispatcher(lpServiceTableEntry)))

				dbg_printf_and_err_str(pLOG_FILE,"I:" MODULE_TAG "StartServiceCtrlDispather failed " \
										"in set_up_service.",GetLastError(),LOG_ERR,LOG_DEBUG);

			else {

				DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Returned from service dispatcher...\n"));

				if (g_hServiceMainThread) {

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Entering wait for service dispatcher clean up...\n"));

					WaitForSingleObject(g_hServiceMainThread,INFINITE);

					DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Returned from wait for service dispatcher clean up...\n"));

					CloseHandle(g_hServiceMainThread);
				}
			}

			free(lpServiceTableEntry);

			DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Exiting set_up_service, returning to client (main())...\n"));
		}

		free(szServiceName);
	}

	utf_free_ar(utf_8_argv,argc);

	free(szServiceDesc);


	return 1;
}
