/*!
 * @brief Utility Functions
 *
 * @file sstp-util.c
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
#ifndef __SSTP_UTIL_H__
#define __SSTP_UTIL_H__


/*!
 * @brief structure to hold URL components
 */
typedef struct
{
    /* The service string, or NULL if none */
    char *schema;
    
    /*< The host, either a domain or a ip address */
    char *host;

    /*< The username if specified */
    char *user;

    /*< The password if specified */
    char *password;

    /*< The port component */
    char *port;

    /*< The path component of the url */
    char *path;

    /* Pointer to the initial buffer */
    char *ptr;

} sstp_url_st;


/*!
 * @brief Set socket non-blocking
 */
status_t sstp_set_nonbl(int sock, int state);


/*!
 * @brief Generate a UUID string
 */
char *sstp_get_guid(char *buf, int len);


/*!
 * @brief Set socket send buffer size
 */
status_t sstp_set_sndbuf(int sock, int size);


/*!
 * @brief Split the URL up into components (non-rfc complient) 
 */
status_t sstp_url_parse(sstp_url_st **url, const char *path);


/*!
 * @brief Normalize into Kb, Mb, Gb, or Tb
 */
const char *sstp_norm_data(unsigned long long count, char *buf, int len);


/*!
 * @brief Normilize into hour, min or sec.
 */
const char *sstp_norm_time(unsigned long t, char *buf, int len);


/*!
 * @brief Free the url structure
 */ 
void sstp_url_free(sstp_url_st *url);


/*!
 * @brief Convert sockaddr structure to a string
 */
const char *sstp_ipaddr(struct sockaddr *addr, char *buf, int len);


/*!
 * @brief Convert a username into the actual uid value
 */
int sstp_get_uid(const char *name);


/*!
 * @brief Convert a group name into the actua gid value
 */
int sstp_get_gid(const char *name);


/*!
 * @brief Create a new directory @a path and change it's permissions to user and group
 */
int sstp_create_dir(const char *path, const char *user, const char *group, mode_t mode);


/*!
 * @brief Enter a sandbox given the new root @a newroot directory, user and group id.
 */
int sstp_sandbox(const char *newroot, const char *user, const char *group);


#endif
