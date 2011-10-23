/*
Copyright (C) 2003-2004 Narcis Ilisei

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
/* custom error logging system */

#ifndef _DEBUG_IF_INCLUDED
#define _DEBUG_IF_INCLUDED

#define DBG_FILE_PRINTF(msg) os_file_printf msg

#include <stdio.h>
#include "os.h"

typedef struct
{
	int level;
	int verbose;
	char *p_logfilename;
	FILE *p_file;
} DBG_TYPE;


/**
    Returns the dbg destination.
    DBG_SYS_LOG for SysLog
    DBG_STD_LOG for standard console
*/
DBG_DEST get_dbg_dest(void);
void set_dbg_dest(DBG_DEST dest);

/**
    A printf function that may print to syslog if that is activated.
*/
void os_printf(int prio, char *fmt, ... );

void os_file_printf(FILE *pFile, char *fmt, ... );
void os_info_printf(FILE *pFile, char *fmt, ... );

#ifdef _WIN32

void dbg_printf_and_err_str(FILE *pLOG_FILE,char *szDbg,DWORD errCode,DWORD logLevel,DWORD logLimit,...);

#endif

#ifndef DBG_PRINT_DISABLED 
    #ifndef DBG_PRINTF
        #ifndef PSOS_OS            
	        #define DBG_PRINTF(msg) os_printf msg
        #else
            #define _BMC_MOD_DEBUG 1             
            #include <debug_if.h>
        #endif
    #endif
#else
	#define DBG_PRINTF(msg)
#endif

#endif
