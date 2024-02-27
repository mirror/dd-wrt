/*
  This file is part of libmicrohttpd
  Copyright (C) 2022 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.
  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/gen_auth.h
 * @brief  Declarations for HTTP authorisation general functions
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_GET_AUTH_H
#define MHD_GET_AUTH_H 1

#include "mhd_options.h"
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */

struct MHD_Connection; /* Forward declaration to avoid include of the large headers */

#ifdef BAUTH_SUPPORT

/* Forward declaration to avoid additional headers inclusion */
struct MHD_RqBAuth;
/**
 * Return request's Basic Authorisation parameters.
 *
 * Function return result of parsing of the request's "Authorization" header or
 * returns cached parsing result if the header was already parsed for
 * the current request.
 * @param connection the connection to process
 * @return the pointer to structure with Authentication parameters,
 *         NULL if no memory in memory pool, if called too early (before
 *         header has been received) or if no valid Basic Authorisation header
 *         found.
 */
const struct MHD_RqBAuth *
MHD_get_rq_bauth_params_ (struct MHD_Connection *connection);

#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
/* Forward declaration to avoid additional headers inclusion */
struct MHD_RqDAuth;

/**
 * Return request's Digest Authorisation parameters.
 *
 * Function return result of parsing of the request's "Authorization" header or
 * returns cached parsing result if the header was already parsed for
 * the current request.
 * @param connection the connection to process
 * @return the pointer to structure with Authentication parameters,
 *         NULL if no memory in memory pool, if called too early (before
 *         header has been received) or if no valid Basic Authorisation header
 *         found.
 */
const struct MHD_RqDAuth *
MHD_get_rq_dauth_params_ (struct MHD_Connection *connection);
#endif /* DAUTH_SUPPORT */

#endif /* MHD_GET_AUTH_H */
