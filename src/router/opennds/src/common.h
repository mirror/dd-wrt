/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/** @file common.h
    @brief Common constants and other bits
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
    @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
*/

#ifndef _COMMON_H_
#define _COMMON_H_

/** @brief Big Buffer */
#define MAX_BUF 8192

/** @brief Medium Buffer */
#define MID_BUF 4096

/** @brief Small Buffer */
#define SMALL_BUF 1024

/** @brief exec return status buffer size */
#define STATUS_BUF 16

/* Max length of a query string in bytes */
#define QUERYMAXLEN 8192

/* Separator for Preauth and Encrypted query string */
#define QUERYSEPARATOR ", "

/* Separator for html query string */
#define HTMLQUERYSEPARATOR "&"

// Define a custom separator for custom parameter name/value pairs ie urlencoded "="
#define CUSTOM_SEPARATOR "%3d"
#define FORMAT_SPECIFIER "%s"
#define URL_COMMASPACE "%2c%20"

/* Max dynamic html page size in bytes */
#define HTMLMAXSIZE config->max_page_size

#endif /* _COMMON_H_ */
