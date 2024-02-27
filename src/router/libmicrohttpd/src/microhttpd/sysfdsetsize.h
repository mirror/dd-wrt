/*
  This file is part of libmicrohttpd
  Copyright (C) 2015-2023 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/sysfdsetsize.h
 * @brief  Helper for obtaining FD_SETSIZE system default value
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef SYSFDSETSIZE_H
#define SYSFDSETSIZE_H 1

#include "mhd_options.h"

#ifndef MHD_SYS_FD_SETSIZE_
/**
 * Get system default value of FD_SETSIZE
 * @return system default value of FD_SETSIZE
 */
unsigned int
get_system_fdsetsize_value (void);

#else  /* MHD_SYS_FD_SETSIZE_ */
/**
 * Get system default value of FD_SETSIZE
 * @return system default value of FD_SETSIZE
 */
_MHD_static_inline unsigned int
get_system_fdsetsize_value (void)
{
  return (unsigned int) MHD_SYS_FD_SETSIZE_;
}


#endif /* MHD_SYS_FD_SETSIZE_ */

#endif /* !SYSFDSETSIZE_H */
