/*
 * WPA Supplicant / WinMain() function for Windows-based applications
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
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

#include "common.h"
#include "wpa_supplicant_i.h"

#ifdef _WIN32_WCE
#define CMDLINE LPWSTR
#else /* _WIN32_WCE */
#define CMDLINE LPSTR
#endif /* _WIN32_WCE */


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   CMDLINE lpCmdLine, int nShowCmd)
{
	/* TODO */
	return 0;
}
