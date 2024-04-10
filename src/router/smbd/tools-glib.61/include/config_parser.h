/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_CONFIG_H__
#define __KSMBD_CONFIG_H__

#include <glib.h>

struct smbconf_group {
	char			*name;
	GHashTable		*kv;
};

struct smbconf_parser {
	GHashTable		*groups;
	struct smbconf_group	*current, *global, *ipc;
};

extern struct smbconf_parser parser;

static inline int cp_printable(unsigned char *p)
{
	/* eighth bit is ok due to utf-8 mb */
	return *p >= 0x20 && *p != 0x7F;
}

static inline int cp_smbconf_eol(char *p)
{
	return *p == 0x00 || *p == ';' || *p == '#';
}

static inline int cp_pwddb_eol(char *p)
{
	return *p == 0x00;
}

void cp_parse_external_smbconf_group(char *name, char **options);
void cp_smbconf_parser_init(void);
void cp_smbconf_parser_destroy(void);

int cp_parse_smbconf(char *smbconf);
int cp_parse_pwddb(char *pwddb);
int cp_parse_subauth(void);
int cp_parse_lock(void);

unsigned long long cp_memparse(char *v);
char *cp_ltrim(const char *v);
char *cp_rtrim(const char *v, const char *p);
int cp_key_cmp(const char *lk, const char *rk);
char *cp_get_group_kv_string(char *v);
int cp_get_group_kv_bool(char *v);
unsigned long cp_get_group_kv_long_base(char *v, int base);
unsigned long cp_get_group_kv_long(char *v);
int cp_get_group_kv_config_opt(char *v);
char **cp_get_group_kv_list(char *v);
void cp_group_kv_list_free(char **list);

#endif /* __KSMBD_CONFIG_H__ */
