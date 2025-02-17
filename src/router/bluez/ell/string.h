/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_STRING_H
#define __ELL_STRING_H

#include <stdarg.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_string;

struct l_string *l_string_new(size_t initial_length);
void l_string_free(struct l_string *string);
DEFINE_CLEANUP_FUNC(l_string_free);
char *l_string_unwrap(struct l_string *string);

struct l_string *l_string_append(struct l_string *dest, const char *src);
struct l_string *l_string_append_c(struct l_string *dest, const char c);
struct l_string *l_string_append_fixed(struct l_string *dest, const char *src,
					size_t max);

void l_string_append_vprintf(struct l_string *dest,
					const char *format, va_list args)
					__attribute__((format(printf, 2, 0)));
void l_string_append_printf(struct l_string *dest, const char *format, ...)
					__attribute__((format(printf, 2, 3)));

struct l_string *l_string_truncate(struct l_string *string, size_t new_size);

unsigned int l_string_length(struct l_string *string);

char **l_parse_args(const char *args, int *out_n_args);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_STRING_H */
