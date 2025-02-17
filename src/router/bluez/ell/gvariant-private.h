/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

struct l_dbus_message_iter;
struct dbus_builder;

bool _gvariant_iter_init(struct l_dbus_message_iter *iter,
				struct l_dbus_message *message,
				const char *sig_start, const char *sig_end,
				const void *data, size_t len);
bool _gvariant_iter_next_entry_basic(struct l_dbus_message_iter *iter,
					char type, void *out_p);
bool _gvariant_iter_enter_struct(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *structure);
bool _gvariant_iter_enter_variant(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *variant);
bool _gvariant_iter_enter_array(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *array);
bool _gvariant_iter_skip_entry(struct l_dbus_message_iter *iter);

bool _gvariant_valid_signature(const char *sig);
int _gvariant_get_alignment(const char *signature);
bool _gvariant_is_fixed_size(const char *signature);
int _gvariant_get_fixed_size(const char *signature);
int _gvariant_num_children(const char *sig);

struct dbus_builder *_gvariant_builder_new(void *body, size_t body_size);
void _gvariant_builder_free(struct dbus_builder *builder);
bool _gvariant_builder_append_basic(struct dbus_builder *builder,
					char type, const void *value);
bool _gvariant_builder_mark(struct dbus_builder *builder);
bool _gvariant_builder_rewind(struct dbus_builder *builder);
char *_gvariant_builder_finish(struct dbus_builder *builder,
				void **body, size_t *body_size);
bool _gvariant_builder_enter_struct(struct dbus_builder *builder,
					const char *signature);
bool _gvariant_builder_leave_struct(struct dbus_builder *builder);
bool _gvariant_builder_enter_dict(struct dbus_builder *builder,
					const char *signature);
bool _gvariant_builder_leave_dict(struct dbus_builder *builder);
bool _gvariant_builder_enter_variant(struct dbus_builder *builder,
					const char *signature);
bool _gvariant_builder_leave_variant(struct dbus_builder *builder);
bool _gvariant_builder_enter_array(struct dbus_builder *builder,
					const char *signature);
bool _gvariant_builder_leave_array(struct dbus_builder *builder);

size_t _gvariant_message_finalize(size_t header_end,
					void *body, size_t body_size,
					const char *signature);
