/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2001-2008 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Command response routines
 * $Id: response.h,v 1.6 2008/05/06 17:42:48 castaglia Exp $
 */

#ifndef PR_RESPONSE_H
#define PR_RESPONSE_H

/* Response structure */

typedef struct resp_struc {
  struct resp_struc *next;
  char *num;
  char *msg;
} pr_response_t;

/* Utilize gcc's __attribute__ pragma for signalling that it should perform
 * printf-style checking of this function's arguments.
 */

void pr_response_add(const char *, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

void pr_response_add_err(const char *, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

int pr_response_block(int);
void pr_response_clear(pr_response_t **);
void pr_response_flush(pr_response_t **);

void pr_response_send(const char *, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

void pr_response_send_async(const char *, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

void pr_response_send_ml_start(const char *, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 2, 3)));
#else
       ;
#endif

void pr_response_send_ml(const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 1, 2)));
#else
       ;
#endif

void pr_response_send_ml_end(const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 1, 2)));
#else
       ;
#endif

void pr_response_send_raw(const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 1, 2)));
#else
       ;
#endif

/* Set a callback pointer to a function that can handle/adjust a response
 * line, before that response is sent to the client.  If no callback is
 * configured, the line will be sent as is.
 */
void pr_response_register_handler(char *(*)(pool *, const char *, ...));

/* Get the pool currently used for response lists. */
pool *pr_response_get_pool(void);

/* Set the pool used for the response lists. */
void pr_response_set_pool(pool *);

#endif /* PR_RESPONSE_H */
