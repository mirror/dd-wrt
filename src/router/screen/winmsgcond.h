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

#ifndef SCREEN_WINMSGCOND_H
#define SCREEN_WINMSGCOND_H

#include <stdbool.h>


/* represents a window message condition (e.g. %?)*/
typedef struct {
	int  offset;  /* offset in dest string */
	bool initialized;
	bool state;   /* conditional truth value */
	bool locked;  /* when set, prevents state from changing */
} WinMsgCond;

/* WinMsgCond is intended to be used as an opaque type */
void wmc_init(WinMsgCond *, int);
void wmc_set(WinMsgCond *);
void wmc_clear(WinMsgCond *);
bool wmc_is_active(const WinMsgCond *);
bool wmc_is_set(const WinMsgCond *);
int  wmc_else(WinMsgCond *, int, bool *);
int  wmc_end(const WinMsgCond *, int, bool *);
void wmc_deinit(WinMsgCond *);

#endif
