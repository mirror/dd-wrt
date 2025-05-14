/* Copyright (c) 2013
 *      Mike Gerwitz (mtg@gnu.org)
 *
 * This file is part of GNU screen.
 *
 * GNU screen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * <https://www.gnu.org/licenses>.
 *
 ****************************************************************
 */

#ifndef SCREEN_WINMSGBUF_H
#define SCREEN_WINMSGBUF_H

#include <stddef.h>
#include <stdint.h>
#include "screen.h"

/* Default window message buffer size */
#define WINMSGBUF_SIZE MAXSTR

#define MAX_WINMSG_REND 256 /* rendition changes */

/* TODO: complete truncation and rendition API */

/* Represents a working buffer for window messages */
typedef struct {
	char     *buf;
	size_t    size;
	uint64_t  rend[MAX_WINMSG_REND];
	int       rendpos[MAX_WINMSG_REND];
	int       numrend;
} WinMsgBuf;

typedef struct {
	WinMsgBuf *buf;
	char      *p;    /* pointer within buffer */

	/* truncation mark */
	struct {
		/* starting position of truncation; TODO: make this size_t and remove
		 * -1 as inactive indicator */
		int pos;
		/* target offset percentage relative to pos and trunc operator */
		uint8_t perc;
		/* whether to show ellipses */
		bool ellip;
	} trunc;
} WinMsgBufContext;


WinMsgBuf *wmb_create(void);
void wmb_reset(WinMsgBuf *);
size_t wmb_expand(WinMsgBuf *, size_t);
void wmb_rendadd(WinMsgBuf *, uint64_t, int);
size_t wmb_size(const WinMsgBuf *);
const char *wmb_contents(const WinMsgBuf *);
void wmb_reset(WinMsgBuf *);
void wmb_free(WinMsgBuf *);

WinMsgBufContext *wmbc_create(WinMsgBuf *);
void wmbc_rewind(WinMsgBufContext *);
void wmbc_fastfw0(WinMsgBufContext *);
void wmbc_fastfw_end(WinMsgBufContext *);
void wmbc_putchar(WinMsgBufContext *, char);
const char *wmbc_strncpy(WinMsgBufContext *, const char *, size_t);
const char *wmbc_strcpy(WinMsgBufContext *, const char *);
int wmbc_printf(WinMsgBufContext *, const char *, ...)
                __attribute__((format(printf,2,3)));
size_t wmbc_offset(WinMsgBufContext *);
size_t wmbc_bytesleft(WinMsgBufContext *);
const char *wmbc_mergewmb(WinMsgBufContext *, WinMsgBuf *);
const char *wmbc_finish(WinMsgBufContext *);
void wmbc_free(WinMsgBufContext *);

#endif
