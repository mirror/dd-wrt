/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_STRV_H
#define __ELL_STRV_H

#include <stdarg.h>
#include <stdbool.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

void l_strfreev(char **strlist);
char **l_strsplit(const char *str, const char sep);
char **l_strsplit_set(const char *str, const char *separators);
char *l_strjoinv(char **str_array, const char delim);

char **l_strv_new(void);
void l_strv_free(char **str_array);
DEFINE_CLEANUP_FUNC(l_strv_free);
unsigned int l_strv_length(char **str_array);
bool l_strv_contains(char **str_array, const char *item);
char **l_strv_append(char **str_array, const char *str);
char **l_strv_append_printf(char **str_array, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
char **l_strv_append_vprintf(char **str_array, const char *format,
							va_list args)
					__attribute__((format(printf, 2, 0)));
char **l_strv_copy(char **str_array);
bool l_strv_eq(char **a, char **b);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_STRV_H */
