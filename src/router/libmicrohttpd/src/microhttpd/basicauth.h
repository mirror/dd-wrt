/*
     This file is part of libmicrohttpd
     Copyright (C) 2010, 2011, 2012 Daniel Pittman and Christian Grothoff
     Copyright (C) 2014-2022 Evgeny Grin (Karlson2k)

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
 * @file basicauth.c
 * @brief Implements HTTP basic authentication methods
 * @author Amr Ali
 * @author Matthieu Speder
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_BASICAUTH_H
#define MHD_BASICAUTH_H 1

#include "mhd_str.h"

/**
 * Beginning string for any valid Basic authentication header.
 */
#define _MHD_AUTH_BASIC_BASE   "Basic"

struct MHD_RqBAuth
{
  struct _MHD_str_w_len token68;
};

#endif /* ! MHD_BASICAUTH_H */

/* end of basicauth.h */
