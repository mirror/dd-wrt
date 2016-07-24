/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifdef LINUX_XXX
/* Apparently needed for vasprintf(3) */
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "rtpp_wi_private.h"

struct rtpp_queue
{
    struct rtpp_wi *head;
    struct rtpp_wi *tail;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int length;
    char *name;
    int qlen;
};

struct rtpp_queue *
rtpp_queue_init(int qlen, const char *fmt, ...)
{
    struct rtpp_queue *queue;
    va_list ap;
    int eval;

    queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return (NULL);
    memset(queue, '\0', sizeof(*queue));
    queue->qlen = qlen;
    if ((eval = pthread_cond_init(&queue->cond, NULL)) != 0) {
        free(queue);
        return (NULL);
    }
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        pthread_cond_destroy(&queue->cond);
        free(queue);
        return (NULL);
    }
    va_start(ap, fmt);
    vasprintf(&queue->name, fmt, ap);
    va_end(ap);
    if (queue->name == NULL) {
        pthread_cond_destroy(&queue->cond);
        pthread_mutex_destroy(&queue->mutex);
        free(queue);
        return (NULL);
    }
    return (queue);
}

void
rtpp_queue_destroy(struct rtpp_queue *queue)
{

    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->mutex);
    free(queue->name);
    free(queue);
}

void
rtpp_queue_put_item(struct rtpp_wi *wi, struct rtpp_queue *queue)
{

    pthread_mutex_lock(&queue->mutex);

    wi->next = NULL;
    if (queue->head == NULL) {
        queue->head = wi;
        queue->tail = wi;
    } else {
        queue->tail->next = wi;
        queue->tail = wi;
    }
    queue->length += 1;
#if 0
    if (queue->length > 99 && queue->length % 100 == 0)
        fprintf(stderr, "queue(%s): length %d\n", queue->name, queue->length);
#endif

    if (queue->length % queue->qlen == 0) {
        /* notify worker thread */
        pthread_cond_signal(&queue->cond);
    }

    pthread_mutex_unlock(&queue->mutex);
}

void
rtpp_queue_pump(struct rtpp_queue *queue)
{

    pthread_mutex_lock(&queue->mutex);
    if (queue->length > 0) {
        /* notify worker thread */
        pthread_cond_signal(&queue->cond);
    }

    pthread_mutex_unlock(&queue->mutex);
}

struct rtpp_wi *
rtpp_queue_get_item(struct rtpp_queue *queue, int return_on_wake)
{
    struct rtpp_wi *wi;

    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
        if (queue->head == NULL && return_on_wake != 0) {
            pthread_mutex_unlock(&queue->mutex);
            return (NULL);
        }
    }
    wi = queue->head;
    queue->head = wi->next;
    if (queue->head == NULL)
        queue->tail = NULL;
    queue->length -= 1;
    pthread_mutex_unlock(&queue->mutex);

    return (wi);
}

int
rtpp_queue_get_items(struct rtpp_queue *queue, struct rtpp_wi **items, int ilen, int return_on_wake)
{
    int i;

    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
        if (queue->head == NULL && return_on_wake != 0) {
            pthread_mutex_unlock(&queue->mutex);
            return (0);
        }
    }
    for (i = 0; i < ilen; i++) {
        items[i] = queue->head;
        queue->head = items[i]->next;
        if (queue->head == NULL) {
            queue->tail = NULL;
            i += 1;
            break;
        }
    }
    queue->length -= i;
    pthread_mutex_unlock(&queue->mutex);

    return (i);
}

int
rtpp_queue_get_length(struct rtpp_queue *queue)
{
    int length;

    pthread_mutex_lock(&queue->mutex);
    length = queue->length;
    pthread_mutex_unlock(&queue->mutex);
    return (length);
}
