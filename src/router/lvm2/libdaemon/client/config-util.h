/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_DAEMON_CONFIG_UTIL_H
#define _LVM_DAEMON_CONFIG_UTIL_H

struct buffer {
	int allocated;
	int used;
	char *mem;
};

int buffer_append_vf(struct buffer *buf, va_list ap);
int buffer_append_f(struct buffer *buf, ...);
int buffer_append(struct buffer *buf, const char *string);
void buffer_init(struct buffer *buf);
void buffer_destroy(struct buffer *buf);
int buffer_realloc(struct buffer *buf, int needed);

int buffer_line(const char *line, void *baton);

int set_flag(struct dm_config_tree *cft, struct dm_config_node *parent,
	     const char *field, const char *flag, int want);

void chain_node(struct dm_config_node *cn,
                struct dm_config_node *parent,
                struct dm_config_node *pre_sib);

struct dm_config_node *make_config_node(struct dm_config_tree *cft,
					const char *key,
					struct dm_config_node *parent,
					struct dm_config_node *pre_sib);

int compare_value(struct dm_config_value *a, struct dm_config_value *b);
int compare_config(struct dm_config_node *a, struct dm_config_node *b);

struct dm_config_node *make_text_node(struct dm_config_tree *cft,
				      const char *key,
				      const char *value,
				      struct dm_config_node *parent,
				      struct dm_config_node *pre_sib);

struct dm_config_node *make_int_node(struct dm_config_tree *cft,
				     const char *key,
				     int64_t value,
				     struct dm_config_node *parent,
				     struct dm_config_node *pre_sib);

struct dm_config_node *config_make_nodes_v(struct dm_config_tree *cft,
					   struct dm_config_node *parent,
					   struct dm_config_node *pre_sib,
					   va_list ap);
struct dm_config_node *config_make_nodes(struct dm_config_tree *cft,
					 struct dm_config_node *parent,
					 struct dm_config_node *pre_sib,
					 ...);

struct dm_config_tree *config_tree_from_string_without_dup_node_check(const char *config_settings);

#endif /* _LVM_DAEMON_CONFIG_UTIL_H */
