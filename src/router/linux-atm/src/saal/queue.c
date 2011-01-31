/* queue.c - Quick and very very dirty buffer and queue handling */

/* Written 1995-1996 by Werner Almesberger, EPFL-LRC */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "atmd.h"

#include "queue.h"


BUFFER *buffer_create(int length,int key)
{
    BUFFER *buf;

    buf = alloc_t(BUFFER);
    buf->data = alloc(length);
    buf->length = length;
    buf->key = key;
    return buf;
}


BUFFER *buffer_clone(BUFFER *b)
{
    BUFFER *buf;

    buf = buffer_create(b->length,b->key);
    memcpy(buf->data,b->data,b->length);
    return buf;
}


void buffer_discard(BUFFER *b)
{
    free(b->data);
    free(b);
}


void queue_init(QUEUE *q)
{
    q->first = q->last = NULL;
}


void queue_put(QUEUE *q,BUFFER *b)
{
    Q_INSERT_AFTER(q->first,b,q->last);
    q->last = b;
}


void queue_remove(QUEUE *q,BUFFER *b)
{
    if (q->last == b) q->last = b->prev;
    Q_REMOVE(q->first,b);
}


BUFFER *queue_peek(QUEUE *q)
{
    return q->first;
}


BUFFER *queue_get(QUEUE *q)
{
    BUFFER *buf;

    buf = queue_peek(q);
    if (buf) queue_remove(q,buf);
    return buf;
}


BUFFER *queue_lookup(QUEUE *q,int key)
{
    BUFFER *walk;

    for (walk = q->first; walk; walk = walk->next)
	if (walk->key == key) break;
    return walk;
}


void queue_clear(QUEUE *q)
{
    BUFFER *next;

    while (q->first) {
	next = q->first->next;
	buffer_discard(q->first);
	q->first = next;
    }
    q->last = NULL;
}
