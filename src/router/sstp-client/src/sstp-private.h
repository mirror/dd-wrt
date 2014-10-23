/*!
 * @brief Provide a global include point for most files
 *
 * @file sstp-private.h
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __SSTP_PRIVATE_H__
#define __SSTP_PRIVATE_H__

/*< Defined in netdb.h */
struct addrinfo;
typedef struct addrinfo addrinfo_st;
typedef struct timeval timeval_st;

/*< The max path size */
#define SSTP_PATH_MAX       255

/*< The default buffer size */
#define SSTP_DFLT_BUFSZ     255

/*< The default port (HTTPS) */
#define SSTP_DFLT_PORT      443

/*< The default HTTP method */
#define SSTP_DFLT_METHOD    "SSTP_DUPLEX_POST"

/*< The default URI to connect to */
#define SSTP_DFLT_URI       "sra_{BA195980-CD49-458b-9E23-C84EE0ADCD75}/"

/*< Temprorary path to store the uds socket */
#define SSTP_TMP_PATH       _PATH_TMP

/*< The unix domain socket name */
#define SSTP_SOCK_NAME       "uds-sock"

/*< The default location for CA certificates */
#define SSTP_DFLT_PATH_CERT  "/etc/sstp/certs"

/*< Are we running in client mode */
#define SSTP_MODE_CLIENT     0x01

/*< Are we running in server mode */
#define SSTP_MODE_SERVER     0x02

#include <sstp-compat.h>
#include <sstp-common.h>
#include <sstp-log.h>

#include "sstp-buff.h"
#include "sstp-stream.h"
#include "sstp-chap.h"
#include "sstp-state.h"
#include "sstp-util.h"
#include "sstp-option.h"
#include "sstp-event.h"
#include "sstp-pppd.h"
#include "sstp-cmac.h"
#include "sstp-packet.h"
#include "sstp-route.h"
#include "sstp-task.h"
#include "sstp-fcs.h"
#include "sstp-http.h"

#endif /* #ifndef __SSTP_PRIVATE_H__ */
