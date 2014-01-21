/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef _CONFIG_H
#define _CONFIG_H 1

#define DEBUG 0

#define PROGRAM_VERSION "0.4.1"

#define AUTOLOGIN_PATH "~/.mactelnet"

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_NAME "Mac OS X"

#elif defined(__FreeBSD__)
#define PLATFORM_NAME "FreeBSD"

#elif defined(__NetBSD__)
#define PLATFORM_NAME "NetBSD"

#elif defined(__OpenBSD__)
#define PLATFORM_NAME "OpenBSD"

#elif defined(__MINT__)
#define PLATFORM_NAME "FreeMiNT"

#elif defined(__bsdi__)
#define PLATFORM_NAME "BSD/OS"

#elif defined(linux) || defined(__linux__)
#define PLATFORM_NAME "Linux"

#elif defined(sun)
#define PLATFORM_NAME "Solaris"

#elif defined(__hpux)
#define PLATFORM_NAME "HPUX"

#elif defined(__riscos__)
#define PLATFORM_NAME "RISC OS"

#elif defined(__FreeBSD_kernel__)
#define PLATFORM_NAME "kFreeBSD"
#else
#define PLATFORM_NAME "Unknown"

#endif

#endif
