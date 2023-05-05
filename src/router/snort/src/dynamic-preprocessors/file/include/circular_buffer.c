/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2013-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **
 **  Circular buffer is thread safe for one writer and one reader thread
 **
 **  This implementaton is inspired by one slot open approach.
 **  See http://en.wikipedia.org/wiki/Circular_buffer
 **
 **  5.25.13 - Initial Source Code. Hui Cao
 */

#include "sf_types.h"
#include "circular_buffer.h"

#include <stdio.h>
#include <stdlib.h>

/* Circular buffer object */
struct _CircularBuffer{

    uint64_t   size;   /* maximum number of elements           */
    uint64_t   start;  /* index of oldest element, reader update only */
    uint64_t   end;    /* index to write new element, writer update only*/
    uint64_t   under_run;
    uint64_t   over_run;
    ElemType   *elems;  /* vector of elements                   */
    uint64_t   total_write;
    uint64_t   total_read;
};

/* This approach adds one bit to end and start pointers */

CircularBuffer * cbuffer_init(uint64_t size)
{
    CircularBuffer* cb = calloc(1, sizeof(*cb));

    if ( !cb )
        return NULL;

    cb->size  = size + 1;

    cb->elems = (ElemType *)calloc(cb->size, sizeof(ElemType));
    if (!cb->elems)
    {
        free(cb);
        return NULL;
    }

    return cb;
}


void cbuffer_free(CircularBuffer *cb)
{
    if(cb && cb->elems)
    {
        free(cb->elems);
        cb->elems = NULL;
    }

    free(cb);
}
/* We use mirror flag to detection full or empty efficiently*/
int cbuffer_is_full(CircularBuffer *cb)
{
    uint64_t next = cb->end + 1;

    if ( next == cb->size )
        next = 0;

    return (next == cb->start);
}

/* We use mirror flag to detection full or empty efficiently*/
int cbuffer_is_empty(CircularBuffer *cb)
{
    return (cb->end == cb->start);
}

/* Returns number of elements in use*/
uint64_t cbuffer_used(CircularBuffer *cb)
{
    /* cb->end < cb->start means passing the end of buffer */
    if (cb->end < cb->start)
    {
        return (cb->size + cb->end - cb->start);
    }
    else
    {
        return (cb->end - cb->start);
    }
}

/* Returns number of free elements*/
uint64_t cbuffer_available(CircularBuffer *cb)
{
    return (cbuffer_size(cb) - cbuffer_used(cb));
}

/* Returns total number of elements*/
uint64_t cbuffer_size(CircularBuffer *cb)
{
    return (cb->size - 1);
}

/*
 * Add one element to the buffer,
 *
 * Args:
 *   CircularBuffer *: buffer
 *   ElemType elem: the element to be added
 * Return:
 *   CB_FAIL
 *   CB_SUCCESS
 */
int cbuffer_write(CircularBuffer *cb, const ElemType elem)
{
    uint64_t w = cb->end;

    if ( cbuffer_is_full (cb))  /* full, return error */
    {
        cb->over_run++;
        return CB_FAIL;
    }

    cb->elems[w++] = elem;
    if ( w == cb->size )
        w = 0;

    cb->end = w;
    cb->total_write++;

    return CB_SUCCESS;
}

/*
 * Read one element from the buffer and remove it from buffer,
 *
 * Args:
 *   CircularBuffer *: buffer
 *   ElemType *elem: the element pointer to be stored
 * Return:
 *   CB_FAIL
 *   CB_SUCCESS
 */
int cbuffer_read(CircularBuffer *cb, ElemType *elem)
{
    uint64_t r = cb->start;

    if (cbuffer_is_empty(cb)) /* Empty, return error */
    {
        cb->under_run++;
        return CB_FAIL;
    }

    *elem = cb->elems[r++];
    if ( r == cb->size )
        r = 0;

    cb->start = r;
    cb->total_read++;

    return CB_SUCCESS;
}

/*
 * Read one element from the buffer and no change on buffer
 *
 * Args:
 *   CircularBuffer *: buffer
 *   ElemType *elem: the element pointer to be stored
 * Return:
 *   CB_FAIL
 *   CB_SUCCESS
 */
int cbuffer_peek(CircularBuffer *cb, ElemType *elem) {

    if (cbuffer_is_empty(cb)) /* Empty, return error */
        return CB_FAIL;

    *elem = cb->elems[cb->start];

    return CB_SUCCESS;
}

/* Returns total number of reads*/
uint64_t cbuffer_num_reads(CircularBuffer *cb)
{
    return (cb->total_read);
}

/* Returns total number of writes*/
uint64_t cbuffer_num_writes(CircularBuffer *cb)
{
    return (cb->total_write);
}

/* Returns total number of writer overruns*/
uint64_t cbuffer_num_over_runs(CircularBuffer *cb)
{
    return (cb->over_run);
}

/* Returns total number of reader overruns*/
uint64_t cbuffer_num_under_runs(CircularBuffer *cb)
{
    return (cb->under_run);
}
