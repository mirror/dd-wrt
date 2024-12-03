/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __TOOLS_H__
#define __TOOLS_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <getopt.h>
#include <glib.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

struct smbconf_global {
	int			flags;
	int			map_to_guest;
	char			*guest_account;

	char			*server_string;
	char			*work_group;
	char			*netbios_name;
	char			*server_min_protocol;
	char			*server_max_protocol;
	char			*root_dir;
	int			server_signing;
	int			sessions_cap;
	int			restrict_anon;
	unsigned short		tcp_port;
	unsigned short		ipc_timeout;
	unsigned int		deadtime;
	int			bind_interfaces_only;
	char			**interfaces;
	unsigned long		file_max;
	unsigned int		smb2_max_read;
	unsigned int		smb2_max_write;
	unsigned int		smb2_max_trans;
	unsigned int		smb2_max_credits;
	unsigned int		smbd_max_io_size;
	unsigned int		max_connections;
	unsigned int		share_fake_fscaps;
	unsigned int		gen_subauth[3];
	char			*krb5_keytab_file;
	char			*krb5_service_name;
	int			krb5_support;
	char			*pwddb;
	char			*smbconf;
	pid_t			pid;
};

#define KSMBD_RESTRICT_ANON_TYPE_1	1
#define KSMBD_RESTRICT_ANON_TYPE_2	2

extern struct smbconf_global global_conf;

#define KSMBD_CONF_MAP_TO_GUEST_NEVER		(0)
#define KSMBD_CONF_MAP_TO_GUEST_BAD_USER	(1 << 0)
#define KSMBD_CONF_MAP_TO_GUEST_BAD_PASSWORD	(1 << 1)
#define KSMBD_CONF_MAP_TO_GUEST_BAD_UID		(1 << 2)

#define KSMBD_CONF_MAX_OPEN_FILES	65536	/* TODO */
#define KSMBD_CONF_MAX_ACTIVE_SESSIONS	65536	/* TODO */
#define KSMBD_CONF_MAX_CONNECTIONS	65536

#define PATH_PWDDB		"/tmp/ksmbdpwd.db"
#define PATH_SMBCONF		"/tmp/ksmbd.conf"
#define PATH_SUBAUTH		"/tmp/ksmbd.subauth"
#define PATH_LOCK		"/var/run/ksmbd.lock"
#define PATH_FIFO		"/var/run/ksmbd.fifo"

#define KSMBD_HEALTH_START		(0)
#define KSMBD_HEALTH_RUNNING		(1 << 0)
#define KSMBD_SHOULD_RELOAD_CONFIG	(1 << 1)
#define KSMBD_SHOULD_LIST_CONFIG	(1 << 2)

extern int ksmbd_health_status;

#define TRACING_DUMP_NL_MSG	0

#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//---------------------------------------------------------------//
#define LOGAPP		"[%s/%d]:"
#define PRERR		LOGAPP" ERROR: "
#define PRINF		LOGAPP" INFO: "
#define PRDEBUG		LOGAPP" DEBUG: "

#define PR_ERROR	0
#define PR_INFO		1
#define PR_DEBUG	2

extern int log_level;

#define PR_LOGGER_STDIO         0
#define PR_LOGGER_SYSLOG        1

G_GNUC_PRINTF(2, 3)
extern void __pr_log(int level, const char *fmt, ...);
extern void pr_logger_init(int flags);
extern int set_log_level(int level);

#define pr_log(l, f, ...)				\
	do {						\
		if ((l) <= log_level)			\
			__pr_log((l),			\
				 (f),			\
				 get_tool_name(),	\
				 getpid(),		\
				 ##__VA_ARGS__);	\
	} while (0)

#ifdef HAVE_DEBUG
#define pr_debug(f, ...)	\
	pr_log(PR_DEBUG, PRDEBUG f, ##__VA_ARGS__)
#else
#define pr_debug(f, ...) do {} while (0)
#endif
#define pr_info(f, ...)	\
	pr_log(PR_INFO, PRINF f, ##__VA_ARGS__)
#define pr_err(f, ...)	\
	pr_log(PR_ERROR, PRERR f, ##__VA_ARGS__)

//---------------------------------------------------------------//

void pr_hex_dump(const void *mem, size_t sz);

char *base64_encode(unsigned char *src, size_t srclen);
unsigned char *base64_decode(char const *src, size_t *dstlen);

gchar *ksmbd_gconvert(const gchar *str,
		      gssize       str_len,
		      int          to_codeset,
		      int          from_codeset,
		      gsize       *bytes_read,
		      gsize       *bytes_written);

enum charset_idx {
	KSMBD_CHARSET_UTF8		= 0,
	KSMBD_CHARSET_UTF16LE,
	KSMBD_CHARSET_UCS2LE,
	KSMBD_CHARSET_UTF16BE,
	KSMBD_CHARSET_UCS2BE,
	KSMBD_CHARSET_MAX		= 5,
};

#define KSMBD_CHARSET_DEFAULT		KSMBD_CHARSET_UTF8

extern char *ksmbd_conv_charsets[KSMBD_CHARSET_MAX + 1];

char **gptrarray_to_strv(GPtrArray *gptrarray);
char *gptrarray_to_str(GPtrArray *gptrarray);
void gptrarray_printf(GPtrArray *gptrarray, const char *fmt, ...);
int set_conf_contents(const char *conf, const char *contents);

int load_config(char *pwddb, char *smbconf);
void remove_config(void);

extern int set_tool_main(char *name);
extern const char *get_tool_name(void);

int show_version(void);

typedef int tool_main_fn(int argc, char **argv);
tool_main_fn addshare_main, adduser_main, control_main, mountd_main;
extern tool_main_fn *tool_main;

#define TOOL_IS_ADDSHARE \
	(tool_main == addshare_main)
#define TOOL_IS_ADDUSER \
	(tool_main == adduser_main)
#define TOOL_IS_CONTROL \
	(tool_main == control_main)
#define TOOL_IS_MOUNTD \
	(tool_main == mountd_main)

#define SELECT_NAME(_1, _2, _3, _4, NAME, ...) NAME

#define ghash_for_each_3(v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, NULL, (gpointer *)&v); \
	     )

#define ghash_for_each_4(k, v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, (gpointer *)&k, (gpointer *)&v); \
	     )

#define ghash_for_each(...) \
	SELECT_NAME(__VA_ARGS__, \
		    ghash_for_each_4, \
		    ghash_for_each_3)(__VA_ARGS__)

#define ghash_for_each_remove_3(v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, NULL, (gpointer *)&v); \
	     g_hash_table_iter_remove(&iter))

#define ghash_for_each_remove_4(k, v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, (gpointer *)&k, (gpointer *)&v); \
	     g_hash_table_iter_remove(&iter))

#define ghash_for_each_remove(...) \
	SELECT_NAME(__VA_ARGS__, \
		    ghash_for_each_remove_4, \
		    ghash_for_each_remove_3)(__VA_ARGS__)

#define ghash_for_each_steal_3(v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, NULL, (gpointer *)&v); \
	     g_hash_table_iter_steal(&iter))

#define ghash_for_each_steal_4(k, v, tbl, iter) \
	for (g_hash_table_iter_init(&iter, tbl); \
	     g_hash_table_iter_next(&iter, (gpointer *)&k, (gpointer *)&v); \
	     g_hash_table_iter_steal(&iter))

#define ghash_for_each_steal(...) \
	SELECT_NAME(__VA_ARGS__, \
		    ghash_for_each_steal_4, \
		    ghash_for_each_steal_3)(__VA_ARGS__)

#endif /* __TOOLS_H__ */
