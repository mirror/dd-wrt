/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Steve Horbachuk
Copyright (C) 2006 Steve Horbachuk
Modification by Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define MODULE_TAG "OS_WIN: "
#define W_MODULE_TAG L"OS_WIN: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdio.h>
#include <locale.h>

#include "debug_if.h"

#include "os.h"
#include "dyndns.h"
#include "get_cmd.h"
#include "base64.h"
#include "lang.h"
#include "safe_mem.h"

#ifdef _WIN32

#include "debug_service.h"
#include "unicode_util.h"

static BOOL		isServiceStart=false;
static wchar_t	*service_name=NULL;

static RC_TYPE service_start_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE service_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

static CMD_DESCRIPTION_TYPE service_cmd_options_table[] =
    {
        {"-s",0,{service_start_handler,NULL,0},""},
        {"-n",0,{service_name_handler,NULL,0},""},
        {NULL,0,{0, NULL,0},"" }
    };

static RC_TYPE service_start_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	isServiceStart=true;

	return RC_OK;
}

static get_service_name(wchar_t **dest,char *src)
{
	utf_8_to_16(utf_malloc_8_to_16(dest,src),src);
}

static RC_TYPE service_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	get_service_name(&service_name,p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE service_cmd_error_handler(CMD_DATA *p_cmd, int *current_nr, void *p_context)
{

	/*ignore unknown parameters -- running parser that events here on error -- only interested in
	  _SERVICE_START parameter (see service_main.h) to determine whether going to start service
	*/

	return RC_OK;
}

void os_sleep_ms(int ms)
{
	Sleep(ms);
}

int  os_get_socket_error (void)
{
	return WSAGetLastError();
}

RC_TYPE os_ip_support_startup(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );	


	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return RC_IP_OS_SOCKET_INIT_FAILED;
	}

	/* The WinSock DLL is acceptable. Proceed. */
	return RC_OK;
}

RC_TYPE os_ip_support_cleanup(void)
{
	WSACleanup();
	return RC_OK;
}


/* OS SIGNALS Support */
static OS_SIGNAL_HANDLER_TYPE global_os_handler = { NULL, NULL};

typedef struct
{
	DWORD	original;
	int		translated;
} OS_EVENT_TYPE;

static const OS_EVENT_TYPE os_events_table [] =
    {
        {CTRL_C_EVENT, OS_CTRL_C_SIGNAL},
        {CTRL_CLOSE_EVENT, OS_CTRL_CLOSE_SIGNAL},
        {CTRL_BREAK_EVENT, OS_CTRL_BREAK_SIGNAL},
        {CTRL_LOGOFF_EVENT, OS_CTRL_LOGOFF_SIGNAL},
        {CTRL_SHUTDOWN_EVENT, OS_CTRL_SHUTDOWN_SIGNAL},

        {-1, LAST_SIGNAL}
    };

static int translate_os_signal(DWORD in)
{
	const OS_EVENT_TYPE *it = os_events_table;
	while (it->original != -1)
	{
		if (it->original == in)
		{
			return it->translated;
		}
		++it;
	}
	return -1;
}

BOOL WINAPI os_signal_wrapper_handler(DWORD dwCtrlType)
{
	OS_SIGNAL_TYPE signal;

	memset(&signal,0,sizeof(OS_SIGNAL_TYPE));

	signal.signal = translate_os_signal(dwCtrlType);
	signal.p_in_data = NULL;
	signal.p_out_data = NULL;

	if (global_os_handler.p_func != NULL)
	{
		return global_os_handler.p_func(signal, global_os_handler.p_in_data) ? 1 : 0;
	}
	else
	{
		return 0;
	}
}

/**
	The actual signal handler for Windows.
	Does not respond on LOGOFF signal. 
	Exits on shutdown ..., Ctl-C,...
*/
int dyndns_win32_signal_handler_func(OS_SIGNAL_TYPE signal, void *p_in)
{
	int ret_flag = 0;

	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_in;
	if (p_self == NULL)
	{
		return 0;
	}

	switch (signal.signal)
	{
	case OS_CTRL_C_SIGNAL :
	case OS_CTRL_CLOSE_SIGNAL :
	case OS_CTRL_BREAK_SIGNAL :
	case OS_CTRL_SHUTDOWN_SIGNAL :
		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Signal '0x%x' received. Sending 'Shutdown cmd'.\n", signal));
		ret_flag = 1;
		p_self->cmd = CMD_STOP;
		break;

	case OS_CTRL_LOGOFF_SIGNAL :
	default:
		DBG_PRINTF((LOG_DEBUG,"D:" MODULE_TAG "Signal '0x%x' received. NO ACTION.\n", signal));
	}
	return ret_flag;
}

/* MAIN FUNCTION */
RC_TYPE os_install_signal_handler(void *p_dyndns)
{
	BOOL fSuccess;
	if (global_os_handler.p_func != NULL || p_dyndns == NULL)
	{
		return RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER;
	}

	fSuccess = SetConsoleCtrlHandler(
	               (PHANDLER_ROUTINE) os_signal_wrapper_handler,  /* handler function */
	               TRUE);                           /* add to list */

	if (fSuccess)
	{
		global_os_handler.p_func = dyndns_win32_signal_handler_func;
		global_os_handler.p_in_data = p_dyndns;
		return RC_OK;
	}
	else
	{
		return RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER;
	}
}

/*
    closes current console 
    A rather bad function. I am pretty sure that there is 
    another way to into background under Windows.
 
*/
RC_TYPE close_console_window(void)
{
	fclose(stdin);
	fclose(stderr);
	FreeConsole( );
	return RC_OK;
}

RC_TYPE os_syslog_open(const char *p_prg_name)
{
	return RC_OK;
}

RC_TYPE os_syslog_close(void)
{
	return RC_OK;
}

/*
    thanks pagedude
*/
RC_TYPE os_shell_execute(char * p_cmd)
{
	RC_TYPE	rc = RC_OK;
	HANDLE	hProcess = NULL;


	if (!(isWinNT())) {

#ifndef UNICOWS

		SHELLEXECUTEINFO shellInfo;


		ZeroMemory(&shellInfo, sizeof(shellInfo));
		shellInfo.cbSize = sizeof(shellInfo);
		shellInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;


		shellInfo.lpFile = p_cmd;

		/*shellInfo.lpParameters = args;*/
		if(!ShellExecuteEx(&shellInfo))

		{
			rc = RC_OS_FORK_FAILURE;
		}

	}

#else
		;

	}

#endif

#ifndef UNICOWS 

	else

#endif

	{

		/*NT, or Win32s with UNICOWS defined*/

		wchar_t *utf_16;
		SHELLEXECUTEINFOW shellInfo;


		ZeroMemory(&shellInfo, sizeof(shellInfo));
		shellInfo.cbSize = sizeof(shellInfo);
		shellInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;


		shellInfo.lpFile = utf_8_to_16(utf_malloc_8_to_16(&utf_16,p_cmd),p_cmd);


		free(utf_16);

		if(!ShellExecuteExW(&shellInfo))

		{
			rc = RC_OS_FORK_FAILURE;
		}
	}


	return rc;
}

static unsigned long os_get_inet_addr(char* addr)
{
	unsigned long b3;
	unsigned long b2;
	unsigned long b1;
	unsigned long b0;
	unsigned long ipa;
	int n;

	ipa = 0x0;
	n = sscanf(addr, "%d.%d.%d.%d", &b3, &b2, &b1, &b0);
	if (n == 4)
	{
		ipa = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	}
	ipa = htonl(ipa);
	return(ipa);
}

static FILE* openServiceLog(wchar_t *szServiceLog)
{

	/*a separate log for service server specific*/

	if (szServiceLog && (isService() || isServiceStart))

		return _wfopen(szServiceLog,L"a");

	else

		return NULL;
}

static void init_reg_entry(SERVICE_REG_ENTRY *reg_entry)
{

	memset(reg_entry,0,sizeof(SERVICE_REG_ENTRY)*2);

	wcscpy(reg_entry->regKey,L"\\Parameters");

	wcscpy(reg_entry->regKeyValueNameAr[0],L"AppParameters");

	wcscpy(reg_entry->regKeyValueNameAr[1],L"ServiceLog");

	reg_entry->regKeyValueTypeAr[0]=REG_SZ;

	reg_entry->regKeyValueTypeAr[1]=REG_SZ;
}


/*MAIN - Dyn DNS update entry point.*/
int wmain(int argc, char* argv[])
{
	#define SERVICE_NAME	L"inadyn-mt"
	#define SERVICE_DESC	L"Dynamic DNS Updater"


	#define REG_DEFAULT		L"--dyndns_system default@dyndns.org --dyndns_server_name members.dyndns.org "\
							L"--dyndns_server_url /nic/update? --alias test.dyndns.org --username test "\
							L"--password test --update_period 60000 --log_file ENTER_LOGFILE "\
							L"--cache_dir ENTER_CACHDIR --debug ENTER_DEBUG_LEVEL_0_to_7"

	SERVICE_REG_ENTRY		inadyn_registry[2];
	wchar_t					*szServiceLog=NULL;
	FILE					*pLOG_FILE=NULL;

	char					*utf_8_argv[50];

	int						retVal=0;

	wchar_t *szImagePath=NULL;


	if (isWinNT()) {

		/*set up Windows registry entry structure in case this is an install*/

		init_reg_entry(inadyn_registry);
		inadyn_registry->regKeyValueAr[0]=(REG_DEFAULT);
		inadyn_registry->regKeyValueAr[1]=(L"");

		get_cmd_parse_data_with_error_handling(utf_16_to_8_ar(utf_8_argv,(wchar_t **) argv,argc),argc,service_cmd_options_table,service_cmd_error_handler);

		/*need this instance of service_name for any pre-
		  service instantiation service related registry
		  parameters, *and* set_up_service needs a default
		  if service_name NULL.
		*/
		if (!(service_name)) {

			service_name=safe_malloc((wcslen(SERVICE_NAME)+1)*sizeof(wchar_t));

			wcscpy(service_name,SERVICE_NAME);
		}

		/*
		  if registry contains a service_log parameter, and we're setting up to run
		  as a service, use it.  this parameter is not supported on command line.
		*/

		get_app_parameters(&szServiceLog,service_name,L"\\Parameters",L"ServiceLog");

		utf_free_ar(utf_8_argv,argc);

		pLOG_FILE=openServiceLog(szServiceLog);

		free(szServiceLog);
	}

	init_lang_strings(pLOG_FILE,setlocale(LC_ALL, ""));


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Hello!  main() calling set_up_service...\n"));


	retVal=set_up_service(argc,(wchar_t **) argv,service_name,SERVICE_DESC,inadyn_registry,inadyn_main,service_event_handler,pLOG_FILE);


	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "main() returned from set_up_service.  Closing log file...\n"));

	if (pLOG_FILE)

		fclose(pLOG_FILE);


	dealloc_lang_strings();

	return retVal;
}

RC_TYPE os_change_persona(OS_USER_INFO *p_usr_info)
{
	return RC_OS_CHANGE_PERSONA_FAILURE;
}

#endif /*WIN32*/

