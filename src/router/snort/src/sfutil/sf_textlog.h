/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/**
 * @file   sf_textlog.h
 * @author Russ Combs <rcombs@sourcefire.com>
 * @date   Fri Jun 27 10:34:37 2003
 *
 * @brief  declares buffered text stream for logging
 *
 * Declares a TextLog_*() api for buffered logging.  This allows
 * relatively painless transition from fprintf(), fwrite(), etc.
 * to a buffer that is formatted in memory and written with one
 * fwrite().
 *
 * Additionally, the file is capped at a maximum size.  Beyond
 * that, the file is closed, renamed, and reopened.  The current
 * file always has the same name.  Old files are renamed to that
 * name plus a timestamp.
 */

#ifndef _SF_TEXT_LOG_H
#define _SF_TEXT_LOG_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#define K_BYTES (1024)
#define M_BYTES (K_BYTES*K_BYTES)
#define G_BYTES (K_BYTES*M_BYTES)

/*
 * DO NOT ACCESS STRUCT MEMBERS DIRECTLY
 * EXCEPT FROM WITHIN THE IMPLEMENTATION!
 */
typedef struct _TextLog
{
/* private: */
/* file attributes: */
    FILE* file;
    char* name;
    size_t size;
    size_t maxFile;
    time_t last;

/* buffer attributes: */
    unsigned int pos;
    unsigned int maxBuf;
    char buf[1];
} TextLog;

TextLog* TextLog_Init (
    const char* name, unsigned int maxBuf, size_t maxFile
);
void TextLog_Term (TextLog* this);

bool TextLog_Putc(TextLog*, char);
bool TextLog_Quote(TextLog*, const char*);
bool TextLog_Write(TextLog*, const char*, int len);
bool TextLog_Print(TextLog*, const char* format, ...);
bool TextLog_PrintUnicode(TextLog*, uint8_t *, uint32_t, uint8_t);

bool TextLog_Flush(TextLog*);

/*-------------------------------------------------------------------
  * helper functions
  *-------------------------------------------------------------------
  */
 static inline int TextLog_Tell (TextLog* this)
 {
     return this->pos;
 }

 static inline int TextLog_Avail (TextLog* this)
 {
     return this->maxBuf - this->pos - 1;
 }

 static inline void TextLog_Reset (TextLog* this)
 {
     this->pos = 0;
     this->buf[this->pos] = '\0';
 }

static inline bool TextLog_NewLine (TextLog* this)
{
    return TextLog_Putc(this, '\n');
}

static inline bool TextLog_Puts (TextLog* this, const char* str)
{
    return TextLog_Write(this, str, strlen(str));
}

#endif /* _SF_TEXT_LOG_H */

