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

#include <stdio.h>
#include <stdlib.h>

#define _SERVICE_INSTALL	L"-i"
#define _SERVICE_NAME		L"-n"
#define _SERVICE_START		L"-s"
#define _SERVICE_RESTART	L"-r"
#define _SERVICE_EXIT		L"-e"
#define _SERVICE_UNINSTALL	L"-x"
#define _SERVICE_LOG		L"--service_log"

typedef DWORD SERVICE_EVENT;
typedef DWORD (*SERVICE_CLIENT_ENTRY)(int argc, char* argv[],FILE *pLOG_FILE);
typedef DWORD (*SERVICE_EVENT_HANDLER)(SERVICE_EVENT p_service_event);

typedef struct SERVICE_REG_ENTRY {

	wchar_t     regKey[1024];
	wchar_t     regKeyValueNameAr[256][256];
	void        *regKeyValueAr[256];

	DWORD       regKeyValueTypeAr[256];

} SERVICE_REG_ENTRY;

BOOL isService();
int set_up_service(int argc,wchar_t *argv[],wchar_t *szName,wchar_t *szServiceDescription,SERVICE_REG_ENTRY *service_reg_entry,
					SERVICE_CLIENT_ENTRY service_client_callback,SERVICE_EVENT_HANDLER service_event_handler,FILE *p_log_file);
void insert_app_paramters(int *argc,wchar_t *argv[],wchar_t *szDest[],wchar_t *szServiceName);
int get_registry_parameters(wchar_t *szDest[]);
BOOL get_app_parameters(wchar_t **szParamStr,wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName);
BOOL set_app_parameters(wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName,wchar_t *swcValue);
BOOL get_reg_entry(LPBYTE *bpValue,wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName);
BOOL set_reg_entry(wchar_t *szServiceName,wchar_t *szRegKey,wchar_t *szRegValueName,DWORD dwType,LPBYTE bpValue,DWORD dwSize);

