/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_QUEUE_H
#define __ELL_QUEUE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*l_queue_foreach_func_t) (void *data, void *user_data);
typedef void (*l_queue_destroy_func_t) (void *data);
typedef int (*l_queue_compare_func_t) (const void *a, const void *b,
							void *user_data);
typedef bool (*l_queue_match_func_t) (const void *data, const void *user_data);
typedef bool (*l_queue_remove_func_t) (void *data, void *user_data);

struct l_queue;

struct l_queue_entry {
	void *data;
	struct l_queue_entry *next;
};

struct l_queue *l_queue_new(void);
void l_queue_destroy(struct l_queue *queue,
			l_queue_destroy_func_t destroy);
void l_queue_clear(struct l_queue *queue,
			l_queue_destroy_func_t destroy);

bool l_queue_push_tail(struct l_queue *queue, void *data);
bool l_queue_push_head(struct l_queue *queue, void *data);
void *l_queue_pop_head(struct l_queue *queue);
void *l_queue_peek_head(struct l_queue *queue);
void *l_queue_peek_tail(struct l_queue *queue);

bool l_queue_insert(struct l_queue *queue, void *data,
			l_queue_compare_func_t function, void *user_data);
void *l_queue_find(struct l_queue *queue,
			l_queue_match_func_t function, const void *user_data);
bool l_queue_remove(struct l_queue *queue, void *data);
void *l_queue_remove_if(struct l_queue *queue,
			l_queue_match_func_t function, const void *user_data);

bool l_queue_reverse(struct l_queue *queue);

void l_queue_foreach(struct l_queue *queue,
			l_queue_foreach_func_t function, void *user_data);
unsigned int l_queue_foreach_remove(struct l_queue *queue,
			l_queue_remove_func_t function, void *user_data);

unsigned int l_queue_length(struct l_queue *queue);
bool l_queue_isempty(struct l_queue *queue);

const struct l_queue_entry *l_queue_get_entries(const struct l_queue *queue);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_QUEUE_H */
