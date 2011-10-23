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

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

int install_service(wchar_t *szName, wchar_t *szDisplayName, wchar_t *szDependencies, DWORD dwServiceType);
SC_HANDLE get_service_handle(wchar_t *szName,DWORD access_type);
int start_service(wchar_t *szName,int argc,wchar_t *argv[]);
int uninstall_service(wchar_t *szName);
int stop_service(wchar_t *szName);
int set_service_description(wchar_t *szName, wchar_t *szDesc);
