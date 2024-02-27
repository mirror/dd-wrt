/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2022 Evgeny Grin (Karlson2k)

     GNU libmicrohttpd is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with GNU libmicrohttpd.
     If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file testzzuf/mhd_debug_funcs.h
 * @brief  Declarations of MHD private debug functions
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_DEBUG_FUNCS_H
#define MHD_DEBUG_FUNCS_H 1

struct MHD_Daemon;

/**
 * Checks whether MHD can use accept() syscall and
 * avoid accept4() syscall.
 * @return non-zero if accept() is possible,
 *         zero if accept4() is always used by MHD
 */
int
MHD_is_avoid_accept4_possible_ (void);

/**
 * Switch MHD daemon to use accept() syscalls for new connections.
 * @param daemon the daemon to operate
 */
void
MHD_avoid_accept4_ (struct MHD_Daemon *daemon);

/**
 * Checks whether any know sanitizer is enabled for this build.
 * zzuf does not work together with sanitizers as both are intercepting
 * standard library calls.
 * @return non-zero if any sanitizer is enabled,
 *         zero otherwise
 */
int
MHD_are_sanitizers_enabled_ (void);

#endif /* MHD_DEBUG_FUNCS_H */
