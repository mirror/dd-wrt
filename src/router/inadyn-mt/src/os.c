/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Steve Horbachuk
Copyright (C) 2006 Steve Horbachuk
Modifications by Bryan Hoover (bhoover@wecs.com)
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

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
/*
    History:
        - November 2007 - Added mutex for debug output serialization
		- Febrary 2008	- Added unicode debug/text string output related
		- October 2010	- Added timer abstraction routines

*/

#define SLEEP_INTERVAL	125
#define MAXSTRING		2048

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <wchar.h>  
#include <stdio.h>
#include <string.h>
#include <stdarg.h> 
#include <time.h>


#include "os.h"
#include "debug_if.h"
#include "lang.h"
#include "safe_mem.h"

static int	debug_level=LOG_DEBUG;

#ifdef _WIN32

#include <windows.h>
#include "unicode_util.h"

HANDLE	hLogWriteMutex=0;

/*Win32 unicode console output*/
int		is_console=1;

#endif

static void flush_buffers()
{

	fflush(NULL);
}

char *print_time(void)
{
	time_t now;
	struct tm *timeptr;
	static const char wday_name[7][3] = {
	                                        "Sun", "Mon", "Tue", "Wed",
	                                        "Thu", "Fri", "Sat"
	                                    };
	static const char mon_name[12][3] = {
	                                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	                                    };
	static char result[26];


	time(&now);
	timeptr = localtime(&now);

	sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d:",
	        wday_name[timeptr->tm_wday], mon_name[timeptr->tm_mon],
	        timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min,
	        timeptr->tm_sec, 1900 + timeptr->tm_year);

	return result;
}

#ifdef _WIN32

static BOOL getMutex(HANDLE *hMutex)
{

	if (!(*hMutex))

		*hMutex=CreateMutex(NULL,0,NULL);

	return (WaitForSingleObject(*hMutex,INFINITE)==WAIT_OBJECT_0);
}

static BOOL releaseMutex(HANDLE *hMutex)
{

	return ReleaseMutex(*hMutex);
}

#endif

static void do_os_file_printf(FILE *pFile, char *message, char *time,int prio)
{
	
	if (!(prio<=debug_level))

		return;

	flush_buffers();


#ifdef HAVE_OS_SYSLOG

	if (get_dbg_dest() == DBG_SYS_LOG && prio != -1)
	{
		syslog(prio, "%s %s", print_time(), message);
	}

#endif

	if(pFile) {

#ifndef _WIN32


		if (time)

			fprintf(pFile,"%s %s", time, message);

		else

			fprintf(pFile,"%s", message);

#else

		if (getMutex(&hLogWriteMutex)) {


			if (!(isWinNT()))

  #ifndef UNICOWS

				if (time)

					fprintf(pFile,"%s %s", time, message);

				else

					fprintf(pFile,"%s", message);

  #else

		;

  #endif

  #ifndef UNICOWS

			else

  #endif

			{
/*				HANDLE	io_handle;*/
				LPDWORD	charsWritten=0;

				wchar_t *utf_16_message=NULL;
				wchar_t *utf_16_buff=NULL;


				if (!(time)) {

					utf_8_to_16(utf_malloc_8_to_16(&utf_16_message,message),message);
				}
				else {

					utf_16_message=safe_malloc(utf_8_len(message)*sizeof(wchar_t)+utf_8_len(time)*sizeof(wchar_t)+sizeof(wchar_t));

					wcscat(utf_8_to_16(utf_16_message,time),
						   utf_8_to_16(utf_malloc_8_to_16(&utf_16_buff,message),message));

					free(utf_16_buff);
				}


				if (!(is_console) || !(pFile==stdout))

					fwprintf(pFile,L"%s", utf_16_message);

				else {

					
					/*DWORD	console_mode;*/

					fwprintf(pFile,L"%s", utf_16_message);


					/*
					io_handle=GetStdHandle(STD_OUTPUT_HANDLE);

					GetConsoleMode(io_handle,&console_mode);

					SetConsoleMode(io_handle,console_mode | ENABLE_PROCESSED_OUTPUT);


					WriteConsoleW(io_handle,utf_16_message,wcslen(utf_16_message),charsWritten,NULL);
					*/
				}


				free(utf_16_message);
			}


			ReleaseMutex(hLogWriteMutex);
		}

#endif

		fflush(pFile);
	}
}

static char *formatted_message(char *message,char *fmt,va_list args,int *is_wide,
                               int buff_size)
{

	char szLangStr[MAXSTRING];
	char tmp_buff[MAXSTRING];

#ifdef _WIN32

	if (!(isWinNT()))

  #ifndef UNICOWS

		*is_wide=0;

  #else

		;

  #endif

  #ifndef UNICOWS

	else

  #endif

		*is_wide=1;

#endif


	strncpy(tmp_buff,fmt,MAXSTRING-2);

	tmp_buff[MAXSTRING-1]='\0';


	vsnprintf(message, buff_size-1, langStr(szLangStr,tmp_buff,MAXSTRING), args);

	return message;
}

#ifdef _WIN32 /*quiet linux compile warn*/

static char *va_list_formatted_message(char *message,char *fmt,int buff_size,...)
{

	va_list args;
	int     is_wide=0;


	va_start(args, buff_size);

	formatted_message(message,fmt,args,&is_wide,buff_size);

	va_end(args);


	return message;
}

#endif

/*
    returns the inet addr of a char* begining with an IP address: "nnn.nnn.nnn.nnn"
*/
RC_TYPE os_convert_ip_to_inet_addr(unsigned long *p_addr, const char *p_name)
{
	RC_TYPE rc = RC_OK;

	if (p_name == NULL || p_addr == NULL)
	{
		return RC_INVALID_POINTER;
	}

	*p_addr = 0;
	if (strlen(p_name) == 0)
	{
		rc = RC_OS_INVALID_IP_ADDRESS;
	}

	{
		unsigned int b3;
		unsigned int b2;
		unsigned int b1;
		unsigned int b0;
		unsigned long ipa;
		int n;

		ipa = 0x0;
		n = sscanf(p_name, IP_V4_IP_ADDR_FORMAT, &b3, &b2, &b1, &b0);
		if (n == 4)
		{
			ipa = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
			*p_addr = htonl(ipa);

		}
		else
		{
			rc = RC_OS_INVALID_IP_ADDRESS;
		}
	}
	return rc;
}

/**
    The dbg destination.
    DBG_SYS_LOG for SysLog
    DBG_STD_LOG for standard console
*/
static DBG_DEST global_mod_dbg_dest = DBG_STD_LOG;
/**
    Returns the dbg destination.
    DBG_SYS_LOG for SysLog
    DBG_STD_LOG for standard console
*/
DBG_DEST get_dbg_dest(void)
{
	return global_mod_dbg_dest;
}

void set_dbg_dest(DBG_DEST dest)
{
	global_mod_dbg_dest = dest;
}

#ifdef _WIN32

int isWinNT()
{

	return (GetVersion() < 0x80000000);
}

void dbg_printf_and_err_str(FILE *pLOG_FILE,char *szDbg,DWORD errCode,DWORD logLevel,DWORD logLimit,...)
{

	/*
	  Win32/NT

	  Output using format string szDbg, or that corresponding to lang file message number, msgNum.  use va_list args
	  to create the output message, with variable arguments list corresponding to format string specifiers.  Cat this
	  with any system message corresponding to errCode, and cat that with any system error returned when getting errCode
	  related message.  System errors are also matched up with format/specifiers, and corresponding arguments, as with
	  szDbg/msgNum, before catting.
	*/

	char     szError[MAXSTRING];
	DWORD    msgFmtErrCode=0;
	int      msgFmtStrSize=MAXSTRING;
	va_list  args;
	int      is_wide=0;
	char     message[MAXSTRING];
	char     subMessage[MAXSTRING];
	char     utf_8[MAXSTRING];


	FARPROC  p_win_message_format=FormatMessage;


	if (logLevel<=logLimit) {

		flush_buffers();

		memset(message,0,MAXSTRING);

		va_start(args, logLimit);

		formatted_message(message,szDbg,args,&is_wide,MAXSTRING);

		va_end(args);


		if (is_wide) {

			p_win_message_format=FormatMessageW;

			msgFmtStrSize=MAXSTRING/2;
		}
		if (!(pLOG_FILE))

			pLOG_FILE=stdout;

		if (!(errCode))

			DBG_FILE_PRINTF((pLOG_FILE,message));

		else
		{

			memset(subMessage,0,MAXSTRING);

			if (p_win_message_format(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,errCode,
			                         MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),szError,msgFmtStrSize,NULL)) {

				/*could be Win32s with or without unicows, or Win NT*/

				if (is_wide)

					/*is_wide -- convert utf-16 to utf-8*/

					do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
					                  "  Non-zero error code: %d; related error string:  %s",MAXSTRING,
					                  errCode,utf_16_to_8(utf_8,(wchar_t *) szError))),MAXSTRING),print_time(),-1);

				else

					do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
					                  "  Non-zero error code: %d; related error string:  %s",MAXSTRING,
					                  errCode,szError)),MAXSTRING),print_time(),-1);

			}
			else {

				msgFmtErrCode=GetLastError();

				if (!(msgFmtErrCode))

					do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
					                  "  Non-zero error code: %d; unknown failure getting related error string.\n",
					                  MAXSTRING,errCode)),MAXSTRING),print_time(),-1);

				else

					if (!(p_win_message_format(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					                           NULL,msgFmtErrCode,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),szError,msgFmtStrSize,NULL)))

						do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
						                  "  Non-zero error code: %d; failure (error code: %d) getting related error string.\n",
						                  MAXSTRING,errCode,msgFmtErrCode)),MAXSTRING),print_time(),-1);

					else

						if (is_wide)

							do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
							                  "  Non-zero error code: %d; failure (error code: %d: %s) " \
							                  "getting related error string.\n",MAXSTRING,errCode,
							                  msgFmtErrCode,utf_16_to_8(utf_8,(wchar_t *) szError))),MAXSTRING),print_time(),-1);

						else

							do_os_file_printf(pLOG_FILE,str_buff_safe(strcat(message,va_list_formatted_message(subMessage,
							                  "  Non-zero error code: %d; failure (error code: %d: %s) " \
							                  "getting related error string.\n",MAXSTRING,errCode,
							                  msgFmtErrCode,szError)),MAXSTRING),print_time(),-1);
			}
		}
	}
}

#endif

void os_printf(int prio,char *fmt,...)
{

	va_list		args;
	int			is_wide=0;
	char		message[MAXSTRING];


	flush_buffers();


#ifndef _WIN32	/*default quiet on unix/linux*/
	
  #ifndef DEBUG

	if (prio<LOG_INFO)

  #endif

#else			/*default noisy on windows*/

  #ifdef QUIET

	if (prio<LOG_INFO)

  #endif

#endif

	{
		memset(message,0,MAXSTRING);

		va_start(args, fmt);

		formatted_message(message,fmt,args,&is_wide,MAXSTRING);

		va_end(args);

		do_os_file_printf(stdout,message,print_time(),prio);
	}
}

void os_file_printf(FILE *pFile, char *fmt, ... )
{

	va_list args;
	int     is_wide=0;
	char    message[MAXSTRING];



	flush_buffers();


	memset(message,0,MAXSTRING);

	va_start(args, fmt);

	formatted_message(message,fmt,args,&is_wide,MAXSTRING);

	va_end(args);

	if(!(pFile))

		pFile=stdout;


	do_os_file_printf(pFile,message,print_time(),-1);
}

void os_info_printf(FILE *pFile, char *fmt, ... )
{
	va_list	args;
	int		is_wide=0;
	char	message[MAXSTRING];


	flush_buffers();


	memset(message,0,MAXSTRING);

	va_start(args, fmt);

	formatted_message(message,fmt,args,&is_wide,MAXSTRING);

	va_end(args);

	if(!(pFile))

		pFile=stdout;


	do_os_file_printf(pFile,message,NULL,-1);
}

/**
 * Opens the dbg output for the required destination.
 * 
 * WARNING : Open and Close bg output are quite error prone!
 * They should be called din pairs!
 * TODO: 
 *  some simple solution that involves storing the dbg output device name (and filename)
 */
RC_TYPE os_open_dbg_output(DBG_DEST dest, const char *p_prg_name, const char *p_logfile_name,const int dbg_level)
{

	RC_TYPE rc = RC_OK;
	FILE *pF=NULL;


	flush_buffers();

	debug_level=dbg_level;

	set_dbg_dest(dest);


	switch (get_dbg_dest())
	{
	case DBG_SYS_LOG:
		if (p_prg_name == NULL)
		{
			rc = RC_INVALID_POINTER;
			break;
		}
		rc = os_syslog_open(p_prg_name);
		break;

	case DBG_FILE_LOG:
		if (p_logfile_name == NULL)
		{
			rc = RC_INVALID_POINTER;
			break;
		}

		{

#ifndef _WIN32

			pF = freopen(p_logfile_name, "wb", stdout);
#else 

			if (!(isWinNT()))

  #ifndef UNICOWS

				pF = freopen(p_logfile_name, "wb", stdout);

  #else

			;

  #endif

  #ifndef UNICOWS

			else

  #endif

			{
				wchar_t *utf_16;


				pF = _wfreopen(utf_8_to_16(utf_malloc_8_to_16(&utf_16,(char *) p_logfile_name),
										   (char *) p_logfile_name), L"wb", stdout);


				free(utf_16);


				if (pF) {

					putc(0xff,pF);
					putc(0xfe,pF);
				}
			}

#endif
			if (pF == NULL)
			{
				rc = RC_FILE_IO_OPEN_ERROR;
			}

#ifdef _WIN32

			else {

				is_console=0;
			}
#endif
			break;
		}
	case DBG_STD_LOG:
	default:
		rc = RC_OK;
	}

	return rc;
}

/**
 * Closes the dbg output device.
 */
RC_TYPE os_close_dbg_output(void)
{
	RC_TYPE rc = RC_OK;
	switch (get_dbg_dest())
	{
	case DBG_SYS_LOG:
		rc = os_syslog_close();
		break;

	case DBG_FILE_LOG:
		fclose(stdout);
		rc = RC_OK;
		break;

	case DBG_STD_LOG:
	default:
		rc = RC_OK;
	}

#ifdef _WIN32

	CloseHandle(hLogWriteMutex);
#endif

	return rc;
}

long sleep_lightly_ms(long sleep_interval,CB_WAIT_COND p_func,void *p_func_data)
{

	int ms=sleep_interval;


	while (1) {

		os_sleep_ms((ms<SLEEP_INTERVAL) ? ms:SLEEP_INTERVAL);

		ms-=SLEEP_INTERVAL;

		if (ms<=0)

			break;

		if (p_func)

			if (p_func(p_func_data))

				break;
	}

	/*return how long slept*/
	return (ms>=0) ? (sleep_interval-ms):sleep_interval;
}

#ifdef USE_THREADS

#ifndef _WIN32
static void *timer_thread(void *p_data)
#else
static void timer_thread(void *p_data)
#endif
{
	my_timer_t	*timer=(my_timer_t *) p_data;


	if (timer)

		if (timer->is_init) {			


			while (!(timer->is_exit)) {

				os_sleep_ms(timer->interval_ms);				

				get_mutex(&timer->mutex_timer); 

				/*be careful of overflow*/
				timer->time_ms+=timer->interval_ms;

				release_mutex(&timer->mutex_timer);
			}			
		}

#ifdef _WIN32
	_endthread();
#else
	pthread_exit(timer);

	/*compiler complaints (pthread_exit does not return)*/
	return timer;
#endif
}

my_timer_t *create_timer(my_timer_t *timer,unsigned interval_ms)
{

	if (!(timer))

		return NULL;


	memset(timer,0,sizeof(my_timer_t));

	create_mutex(&timer->mutex_timer);

	timer->is_init=1;

	timer->interval_ms=interval_ms;


	return timer;
}

static void do_stop_timer(my_timer_t *timer)
{

	timer->is_exit=1;

#ifndef _WIN32	

	if (timer->timer_thread)

		pthread_join(timer->timer_thread,NULL);		

#else

	if (timer->timer_thread)

		WaitForSingleObject((HANDLE) timer->timer_thread,INFINITE);
#endif

	timer->timer_thread=0;
}

void destroy_timer(my_timer_t *timer)
{

	if (!(timer))

		return;

	if (!(timer->is_init))

		return;

	do_stop_timer(timer);

	destroy_mutex(&timer->mutex_timer);

	memset(timer,0,sizeof(my_timer_t));
}

static void do_start_timer(my_timer_t *timer)
{

	timer->is_exit=0;
	
#ifdef _WIN32

	timer->timer_thread=_beginthread(timer_thread,0,(void *) timer);
#else

	pthread_create(&timer->timer_thread,NULL,timer_thread,(void *) timer);
#endif

}

void start_timer(my_timer_t *timer)
{

	if (!(timer))

		return;

	if (!(timer->is_init))

		return;

	do_start_timer(timer);
}

void stop_timer(my_timer_t *timer)
{

	if (!(timer))

		return;

	if (!(timer->is_init))

		return;

	do_stop_timer(timer);
}

unsigned long get_timer(my_timer_t *timer)
{

	unsigned long this_time_ms;


	if (!(timer))

		return 0;

	if (!(timer->is_init))

		return 0;


	get_mutex(&timer->mutex_timer);

	this_time_ms=timer->time_ms;

	release_mutex(&timer->mutex_timer);


	return this_time_ms;
}

static void do_reset_timer(my_timer_t *timer)
{

	do_stop_timer(timer);

	timer->is_exit=0;
	timer->time_ms=0;
}

void reset_timer(my_timer_t *timer)
{

	if (!(timer))

		return;

	if (!(timer->is_init))

		return;

	do_reset_timer(timer);
}

void restart_timer(my_timer_t *timer)
{

	if (!(timer))

		return;

	if (!(timer->is_init))

		return;

	do_reset_timer(timer);
	do_start_timer(timer);
}

void pause_timer(my_timer_t *timer)
{

	stop_timer(timer);
}

void resume_timer(my_timer_t *timer)
{

	start_timer(timer);
}


#endif


