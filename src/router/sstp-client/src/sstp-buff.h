/*!
 * @brief Declarations for buffer handling routines
 *
 * @file sstp-buff.h
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

#ifndef __SSTP_BUFF_H__
#define __SSTP_BUFF_H__


/*!
 * @brief The buffer structure
 */
typedef struct
{
    /*< The current length of the buffer */
    int len;

    /*< The maximum size of the buffer */
    int max;

    /*< The current number of bytes read/written */
    int off;

    /*< The buffer (variable size) */
    char data[0];

} sstp_buff_st;


/*!
 * @brief Set the number of HTTP headers in the packet
 */
typedef struct
{
    /*< The typical header name */
    char name[32];

    /*< The typical header value */
    char value[128];

} http_header_st;


/*!
 * @brief Get the HTTP headers and HTTP status code
 */
status_t sstp_http_get(sstp_buff_st *buf, int *code, int *count,
    http_header_st *array);


/*!
 * @brief Get the HTTP header as specified
 */
http_header_st *sstp_http_get_header(const char *name, int count,
    http_header_st *array);


/*! 
 * @brief Check if there is space available
 */
status_t sstp_buff_space(sstp_buff_st *buf, int length);


/*!
 * @brief Reset the length and offset
 */
void sstp_buff_reset(sstp_buff_st *buf);


/*!
 * @brief Print a formatted string to the buffer
 */
status_t sstp_buff_print(sstp_buff_st *buf, const char *fmt, ...);


/*!
 * @brief Get a pointer to the data section
 */
void *sstp_buff_data(sstp_buff_st *buf, int index);


/*!
 * @brief Create a buffer
 */
status_t sstp_buff_create(sstp_buff_st **buf, int size);


/*!
 * @brief Destroy the buffer
 */
void sstp_buff_destroy(sstp_buff_st *buf);


#endif /* #ifndef __SSTP_BUFF_H__ */
