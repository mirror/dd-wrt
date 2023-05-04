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
#ifndef __SPO_PLUGBASE_H__
#define __SPO_PLUGBASE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "event.h"
#include "decode.h"

typedef enum _OutputType
{
    OUTPUT_TYPE__ALERT = 1,
    OUTPUT_TYPE__LOG,
    OUTPUT_TYPE__MAX

} OutputType;

typedef enum _OutputTypeFlag
{
    OUTPUT_TYPE_FLAG__ALERT = 0x00000001,
    OUTPUT_TYPE_FLAG__LOG   = 0x00000002,
    OUTPUT_TYPE_FLAG__ALL   = 0x7fffffff

} OutputTypeFlag;


/***************************** Output Plugin API  *****************************/
typedef void (*OutputConfigFunc)(struct _SnortConfig *, char *);
typedef void (*OutputFunc)(Packet *, const char *, void *, Event *);

typedef struct _OutputConfigFuncNode
{
    char *keyword;
    int output_type_flags;
    union {
        OutputConfigFunc fptr;
        void *void_fptr;
    } cfptr;
    struct _OutputConfigFuncNode *next;

} OutputConfigFuncNode;

typedef struct _OutputFuncNode
{
    void *arg;
    union {
        OutputFunc fptr;
        void *vfptr;
    } fptr;
#ifdef DUMP_BUFFER
    /*A function pointer to point LogBufferDump function.
    This is used only for BufferDump output plugin. For other plugins,
    this would point to NULL. This would help in identifying the BufferDump
    output plugin while traversing the Loglist linked list*/
    OutputFunc bdfptr;
#endif
    struct _OutputFuncNode *next;

} OutputFuncNode;

void RegisterOutputPlugins(void);
void RegisterOutputPlugin(char *, int, OutputConfigFunc);
OutputConfigFunc GetOutputConfigFunc(char *);
void RemoveOutputPlugin(char *);
int GetOutputTypeFlags(char *);
void DumpOutputPlugins(void);
void AddFuncToOutputList(struct _SnortConfig *, OutputFunc, OutputType, void *);
#ifdef DUMP_BUFFER
void AddBDFuncToOutputList(struct _SnortConfig *, OutputFunc, OutputType, void *);
#endif
void FreeOutputConfigFuncs(void);
void FreeOutputList(OutputFuncNode *);

#endif /* __SPO_PLUGBASE_H__ */
