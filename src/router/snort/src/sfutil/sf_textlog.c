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
 * @file   sf_textlog.c
 * @author Russ Combs <rcombs@sourcefire.com>
 * @date
 *
 * @brief  implements buffered text stream for logging
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"

#include "sf_textlog.h"
#include "log.h"
#include "util.h"

/* some reasonable minimums */
#define MIN_BUF  (1*K_BYTES)
#define MIN_FILE (MIN_BUF)

/*-------------------------------------------------------------------
 * TextLog_Open/Close: open/close associated log file
 *-------------------------------------------------------------------
 */
static FILE* TextLog_Open (const char* name)
{
    if ( !name ) return OpenAlertFile(NULL);
    if ( !strcasecmp(name, "stdout") ) return stdout;
    return OpenAlertFile(name);
}

static void TextLog_Close (FILE* file)
{
    if ( !file ) return;
    if ( file != stdout ) fclose(file);
}

static size_t TextLog_Size (FILE* file)
{
    struct stat sbuf;
    int fd = fileno(file);
    int err = fstat(fd, &sbuf);
    return err ? 0 : sbuf.st_size;
}

/*-------------------------------------------------------------------
 * TextLog_Init: constructor
 *-------------------------------------------------------------------
 */
TextLog* TextLog_Init (
    const char* name, unsigned int maxBuf, size_t maxFile
) {
    TextLog* this;

    if ( maxBuf < MIN_BUF ) maxBuf = MIN_BUF;
    if ( maxFile < MIN_FILE ) maxFile = MIN_FILE;
    if ( maxFile < maxBuf ) maxFile = maxBuf;

    this = (TextLog*)malloc(sizeof(TextLog)+maxBuf);

    if ( !this )
    {
        FatalError("Unable to allocate a TextLog(%u)!\n", maxBuf);
    }
    this->name = name ? SnortStrdup(name) : NULL;
    this->file = TextLog_Open(this->name);
    this->size = TextLog_Size(this->file);
    this->last = time(NULL);
    this->maxFile = maxFile;

    this->maxBuf = maxBuf;
    TextLog_Reset(this);

    return this;
}

/*-------------------------------------------------------------------
 * TextLog_Term: destructor
 *-------------------------------------------------------------------
 */
void TextLog_Term (TextLog* this)
{
    if ( !this ) return;

    TextLog_Flush(this);
    TextLog_Close(this->file);

    if ( this->name ) free(this->name);
    free(this);
}

/*-------------------------------------------------------------------
 * TextLog_Flush: start writing to new file
 * but don't roll over stdout or any sooner
 * than resolution of filename discriminator
 *-------------------------------------------------------------------
 */
static void TextLog_Roll (TextLog* this)
{
    if ( this->file == stdout ) return;
    if ( this->last >= time(NULL) ) return;

    TextLog_Close(this->file);
    RollAlertFile(this->name);
    this->file = TextLog_Open(this->name);

    this->last = time(NULL);
    this->size = 0;
}

/*-------------------------------------------------------------------
 * TextLog_Flush: write buffered stream to file
 *-------------------------------------------------------------------
 */
bool TextLog_Flush(TextLog* this)
{
    int ok;

    if ( !this->pos ) return FALSE;
    if ( this->size + this->pos > this->maxFile ) TextLog_Roll(this);

    ok = fwrite(this->buf, this->pos, 1, this->file);

    if ( ok == 1 )
    {
        this->size += this->pos;
        TextLog_Reset(this);
        return TRUE;
    }
    return FALSE;
}

/*-------------------------------------------------------------------
 * TextLog_Putc: append char to buffer
 *-------------------------------------------------------------------
 */
bool TextLog_Putc (TextLog* this, char c)
{
    if ( TextLog_Avail(this) < 1 )
    {
        TextLog_Flush(this);
    }
    this->buf[this->pos++] = c;
    this->buf[this->pos] = '\0';

    return TRUE;
}

/*-------------------------------------------------------------------
 * TextLog_Write: append string to buffer
 *-------------------------------------------------------------------
 */
bool TextLog_Write (TextLog* this, const char* str, int len)
{
    int avail = TextLog_Avail(this);

    if ( len >= avail )
    {
        TextLog_Flush(this);
        avail = TextLog_Avail(this);
    }
    len = snprintf(this->buf+this->pos, avail, "%s", str);

    if ( len >= avail )
    {
        this->pos = this->maxBuf - 1;
        this->buf[this->pos] = '\0';
        return FALSE;
    }
    else if ( len < 0 )
    {
        return FALSE;
    }
    this->pos += len;
    return TRUE;
}

/*-------------------------------------------------------------------
 * TextLog_Printf: append formatted string to buffer
 *-------------------------------------------------------------------
 */
bool TextLog_Print (TextLog* this, const char* fmt, ...)
{
    int avail = TextLog_Avail(this);
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(this->buf+this->pos, avail, fmt, ap);
    va_end(ap);

    if ( len >= avail )
    {
        TextLog_Flush(this);
        avail = TextLog_Avail(this);

        va_start(ap, fmt);
        len = vsnprintf(this->buf+this->pos, avail, fmt, ap);
        va_end(ap);
    }
    if ( len >= avail )
    {
        this->pos = this->maxBuf - 1;
        this->buf[this->pos] = '\0';
        return FALSE;
    }
    else if ( len < 0 )
    {
        return FALSE;
    }
    this->pos += len;
    return TRUE;
}

/*-------------------------------------------------------------------
 * TextLog_PrintUnicode: append formatted string to buffer
 *-------------------------------------------------------------------
 */
bool TextLog_PrintUnicode(TextLog* this, uint8_t *buffer, uint32_t length, uint8_t is_little_endian)
{
    uint32_t out_len = length/2 + 1, i, j;
    wchar_t *outbuf = (wchar_t *)malloc(out_len*sizeof(wchar_t));

    if (!outbuf)
    {
        FatalError("TextLog_PrintUnicode: Failed to allocate memory for outbuf\n");
        return FALSE;
    }

    for(i = 0,j = 0; i<length; i+=2,j++)
    {
        unsigned short value = 0;
        if(is_little_endian)
           value = *((uint8_t *)(buffer + i)) + ((*(uint8_t *)(buffer + i + 1)) << 8);
        else
           value = ((*(uint8_t *)(buffer + i)) << 8) + *((uint8_t *)(buffer + i + 1));

        if(0 == value)
            break;
        outbuf[j] = value;
    }
    outbuf[j] = 0;
    fprintf(this->file, "%ls", outbuf);
    free(outbuf);
    return TRUE;
}

/*-------------------------------------------------------------------
 * TextLog_Quote: write string escaping quotes
 * TBD could be smarter by counting required escapes instead of
 * checking for 3
 *-------------------------------------------------------------------
 */
bool TextLog_Quote (TextLog* this, const char* qs)
{
    int pos = this->pos;

    if ( TextLog_Avail(this) < 3 )
    {
        TextLog_Flush(this);
    }
    this->buf[pos++] = '"';

    while ( *qs && (this->maxBuf - pos > 2) )
    {
        if ( *qs == '"' || *qs == '\\' )
        {
            this->buf[pos++] = '\\';
        }
        this->buf[pos++] = *qs++;
    }
    if ( *qs ) return FALSE;

    this->buf[pos++] = '"';
    this->pos = pos;

    return TRUE;
}

