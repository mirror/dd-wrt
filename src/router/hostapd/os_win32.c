/*
 * wpa_supplicant/hostapd / OS specific functions for Win32 systems
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include <winsock2.h>
#include <wincrypt.h>

#include "os.h"

void os_sleep(os_time_t sec, os_time_t usec)
{
	if (sec)
		Sleep(sec * 1000);
	if (usec)
		Sleep(usec / 1000);
}


int os_get_time(struct os_time *t)
{
#ifdef _WIN32_WCE
	/* TODO */
	return 0;
#else /* _WIN32_WCE */
#define EPOCHFILETIME (116444736000000000ULL)
	FILETIME ft;
	LARGE_INTEGER li;
	ULONGLONG tt;

	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	tt = (li.QuadPart - EPOCHFILETIME) / 10;
	t->sec = (os_time_t) (tt / 1000000);
	t->usec = (os_time_t) (tt % 1000000);

	return 0;
#endif /* _WIN32_WCE */
}


int os_daemonize(const char *pid_file)
{
	/* TODO */
	return -1;
}


void os_daemonize_terminate(const char *pid_file)
{
}


int os_get_random(unsigned char *buf, size_t len)
{
	HCRYPTPROV prov;
	BOOL ret;

	if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL,
				 CRYPT_VERIFYCONTEXT))
		return -1;

	ret = CryptGenRandom(prov, len, buf);
	CryptReleaseContext(prov, 0);

	return ret ? 0 : -1;
}


unsigned long os_random(void)
{
	return rand();
}


char * os_rel2abs_path(const char *rel_path)
{
	return _strdup(rel_path);
}


int os_program_init(void)
{
#ifdef CONFIG_NATIVE_WINDOWS
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		printf("Could not find a usable WinSock.dll\n");
		return -1;
	}
#endif /* CONFIG_NATIVE_WINDOWS */
	return 0;
}


void os_program_deinit(void)
{
#ifdef CONFIG_NATIVE_WINDOWS
	WSACleanup();
#endif /* CONFIG_NATIVE_WINDOWS */
}


int os_setenv(const char *name, const char *value, int overwrite)
{
	return -1;
}


int os_unsetenv(const char *name)
{
	return -1;
}
