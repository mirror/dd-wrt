/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_SETTINGS_H
#define __ELL_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_settings;

typedef void (*l_settings_debug_cb_t) (const char *str, void *user_data);
typedef void (*l_settings_destroy_cb_t) (void *user_data);

struct l_settings *l_settings_new(void);
struct l_settings *l_settings_clone(const struct l_settings *settings);

void l_settings_free(struct l_settings *settings);
DEFINE_CLEANUP_FUNC(l_settings_free);

bool l_settings_load_from_data(struct l_settings *settings,
						const char *data, size_t len);
char *l_settings_to_data(const struct l_settings *settings, size_t *len);

bool l_settings_load_from_file(struct l_settings *settings,
					const char *filename);

bool l_settings_set_debug(struct l_settings *settings,
				l_settings_debug_cb_t callback,
				void *user_data,
				l_settings_destroy_cb_t destroy);

char **l_settings_get_groups(const struct l_settings *settings);
char **l_settings_get_keys(const struct l_settings *settings,
							const char *group_name);

bool l_settings_add_group(struct l_settings *settings, const char *group_name);

bool l_settings_has_group(const struct l_settings *settings,
							const char *group_name);
bool l_settings_has_key(const struct l_settings *settings,
				const char *group_name, const char *key);

const char *l_settings_get_value(const struct l_settings *settings,
					const char *group_name,
					const char *key);
bool l_settings_set_value(struct l_settings *settings, const char *group_name,
				const char *key, const char *value);

bool l_settings_get_bool(const struct l_settings *settings,
				const char *group_name,
				const char *key, bool *out);
bool l_settings_set_bool(struct l_settings *settings, const char *group_name,
				const char *key, bool in);

bool l_settings_get_int(const struct l_settings *settings,
			const char *group_name, const char *key, int *out);
bool l_settings_set_int(struct l_settings *settings, const char *group_name,
				const char *key, int in);

bool l_settings_get_uint(const struct l_settings *settings,
				const char *group_name,
				const char *key, unsigned int *out);
bool l_settings_set_uint(struct l_settings *settings, const char *group_name,
				const char *key, unsigned int in);

bool l_settings_get_int64(const struct l_settings *settings,
				const char *group_name,
				const char *key, int64_t *out);
bool l_settings_set_int64(struct l_settings *settings, const char *group_name,
				const char *key, int64_t in);

bool l_settings_get_uint64(const struct l_settings *settings,
				const char *group_name,
				const char *key, uint64_t *out);
bool l_settings_set_uint64(struct l_settings *settings, const char *group_name,
				const char *key, uint64_t in);

char *l_settings_get_string(const struct l_settings *settings,
				const char *group_name, const char *key);
bool l_settings_set_string(struct l_settings *settings, const char *group_name,
				const char *key, const char *value);

char **l_settings_get_string_list(const struct l_settings *settings,
					const char *group_name,
					const char *key, char delimiter);
bool l_settings_set_string_list(struct l_settings *settings,
					const char *group_name,
					const char *key, char **list,
					char delimiter);

bool l_settings_get_double(const struct l_settings *settings,
				const char *group_name,
				const char *key, double *out);
bool l_settings_set_double(struct l_settings *settings, const char *group_name,
				const char *key, double in);

bool l_settings_get_float(const struct l_settings *settings,
				const char *group_name,
				const char *key, float *out);
bool l_settings_set_float(struct l_settings *settings, const char *group_name,
				const char *key, float in);

uint8_t *l_settings_get_bytes(const struct l_settings *settings,
				const char *group_name, const char *key,
				size_t *out_len);
bool l_settings_set_bytes(struct l_settings *settings, const char *group_name,
				const char *key,
				const uint8_t *value, size_t value_len);

bool l_settings_remove_key(struct l_settings *settings, const char *group_name,
				const char *key);
bool l_settings_remove_group(struct l_settings *settings,
				const char *group_name);

bool l_settings_remove_embedded_groups(struct l_settings *settings);
char **l_settings_get_embedded_groups(struct l_settings *settings);
bool l_settings_has_embedded_group(struct l_settings *settings,
					const char *group);
const char *l_settings_get_embedded_value(struct l_settings *settings,
						const char *group_name,
						const char **out_type);
#ifdef __cplusplus
}
#endif

#endif /* __ELL_SETTINGS_H */
