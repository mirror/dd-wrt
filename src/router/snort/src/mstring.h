/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* $Id$ */

#ifndef __MSTRING_H__
#define __MSTRING_H__

/*  D E F I N E S  *******************************************************/
#define TOKS_BUF_SIZE   100


/*  P R O T O T Y P E S  *************************************************/
char ** mSplit(const char *, const char *, const int, int *, const char);
void mSplitFree(char ***toks, int numtoks);
int mContainsSubstr(const char *, int, const char *, int);
int mSearch(const char *, int, const char *, int, int *, int *);
int mSearchCI(const char *, int, const char *, int, int *, int *);
int mSearchREG(const char *, int, const char *, int, int *, int *);
int *make_skip(char *, int);
int *make_shift(char *, int);

#endif  /* __MSTRING_H__ */
