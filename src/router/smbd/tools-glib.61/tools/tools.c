// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <syslog.h>

#include <unistd.h>
#include <sys/stat.h>

#include <stdio.h>

#include "tools.h"
#include "ipc.h"
#include "rpc.h"
#include "worker.h"
#include "config_parser.h"
#include "management/user.h"
#include "management/share.h"
#include "management/session.h"
#include "management/tree_conn.h"
#include "management/spnego.h"
#include "version.h"

int log_level = PR_INFO;
int ksmbd_health_status;
tool_main_fn *tool_main;

static int log_open;

typedef void (*logger)(int level, const char *fmt, va_list list);

char *ksmbd_conv_charsets[KSMBD_CHARSET_MAX + 1] = {
	"UTF-8",
	"UTF-16LE",
	"UCS-2LE",
	"UTF-16BE",
	"UCS-2BE",
	"OOPS"
};

static int syslog_level(int level)
{
	if (level == PR_ERROR)
		return LOG_ERR;
	if (level == PR_INFO)
		return LOG_INFO;
	if (level == PR_DEBUG)
		return LOG_DEBUG;

	return LOG_ERR;
}

G_GNUC_PRINTF(2, 0)
static void __pr_log_stdio(int level, const char *fmt, va_list list)
{
	char buf[1024];

	vsnprintf(buf, sizeof(buf), fmt, list);
	printf("%s", buf);
}

G_GNUC_PRINTF(2, 0)
static void __pr_log_syslog(int level, const char *fmt, va_list list)
{
	vsyslog(syslog_level(level), fmt, list);
}

static logger __logger = __pr_log_stdio;

void __pr_log(int level, const char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	__logger(level, fmt, list);
	va_end(list);
}

void pr_logger_init(int flag)
{
	if (flag == PR_LOGGER_SYSLOG) {
		if (log_open) {
			closelog();
			log_open = 0;
		}
		openlog("ksmbd", LOG_NDELAY, LOG_LOCAL5);
		__logger = __pr_log_syslog;
		log_open = 1;
	}
}

int set_log_level(int level)
{
	int old_level;

	if (log_level == PR_DEBUG)
		return log_level;

	old_level = log_level;
	log_level = level;
	return old_level;
}

#if TRACING_DUMP_NL_MSG
#define PR_HEX_DUMP_WIDTH	160
void pr_hex_dump(const void *mem, size_t sz)
{
	char xline[PR_HEX_DUMP_WIDTH];
	char sline[PR_HEX_DUMP_WIDTH];
	int xi = 0, si = 0, mi = 0;

	while (mi < sz) {
		char c = *((char *)mem + mi);

		mi++;
		xi += sprintf(xline + xi, "%02X ", 0xff & c);
		if (c > ' ' && c < '~')
			si += sprintf(sline + si, "%c", c);
		else
			si += sprintf(sline + si, ".");
		if (xi >= PR_HEX_DUMP_WIDTH / 2) {
			pr_err("%s         %s\n", xline, sline);
			xi = 0;
			si = 0;
		}
	}

	if (xi) {
		int sz = PR_HEX_DUMP_WIDTH / 2 - xi + 1;

		if (sz > 0) {
			memset(xline + xi, ' ', sz);
			xline[PR_HEX_DUMP_WIDTH / 2 + 1] = 0x00;
		}
		pr_err("%s         %s\n", xline, sline);
	}
}
#else
void pr_hex_dump(const void *mem, size_t sz)
{
}
#endif

char *base64_encode(unsigned char *src, size_t srclen)
{
	return g_base64_encode(src, srclen);
}

unsigned char *base64_decode(char const *src, size_t *dstlen)
{
	unsigned char *ret = g_base64_decode(src, dstlen);

	if (ret)
		ret[*dstlen] = 0x00;
	return ret;
}

static int codeset_has_altname(int codeset)
{
	if (codeset == KSMBD_CHARSET_UTF16LE ||
			codeset == KSMBD_CHARSET_UTF16BE)
		return 1;
	return 0;
}

gchar *ksmbd_gconvert(const gchar *str,
		      gssize       str_len,
		      int          to_codeset,
		      int          from_codeset,
		      gsize       *bytes_read,
		      gsize       *bytes_written)
{
	gchar *converted;
	GError *err;

retry:
	err = NULL;
	if (from_codeset >= KSMBD_CHARSET_MAX) {
		pr_err("Unknown source codeset: %d\n", from_codeset);
		return NULL;
	}

	if (to_codeset >= KSMBD_CHARSET_MAX) {
		pr_err("Unknown target codeset: %d\n", to_codeset);
		return NULL;
	}

	converted = g_convert(str,
			      str_len,
			      ksmbd_conv_charsets[to_codeset],
			      ksmbd_conv_charsets[from_codeset],
			      bytes_read,
			      bytes_written,
			      &err);
	if (err) {
		int has_altname = 0;

		if (codeset_has_altname(to_codeset)) {
			to_codeset++;
			has_altname = 1;
		}

		if (codeset_has_altname(from_codeset)) {
			from_codeset++;
			has_altname = 1;
		}

		pr_info("%s\n", err->message);
		g_error_free(err);

		if (has_altname) {
			pr_info("Will try `%s' and `%s'\n",
				ksmbd_conv_charsets[to_codeset],
				ksmbd_conv_charsets[from_codeset]);
			goto retry;
		}

		return NULL;
	}

	return converted;
}

char **gptrarray_to_strv(GPtrArray *gptrarray)
{
	if (!gptrarray->len ||
	    g_ptr_array_index(gptrarray, gptrarray->len - 1))
		g_ptr_array_add(gptrarray, NULL);

	return (char **)g_ptr_array_free(gptrarray, 0);
}

static char *strv_to_str(char **strv)
{
	char *str = g_strjoinv(NULL, strv);

	g_strfreev(strv);
	return str;
}

char *gptrarray_to_str(GPtrArray *gptrarray)
{
	return strv_to_str(gptrarray_to_strv(gptrarray));
}

void gptrarray_printf(GPtrArray *gptrarray, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	g_ptr_array_add(gptrarray, g_strdup_vprintf(fmt, args));
	va_end(args);
}

int set_conf_contents(const char *conf, const char *contents)
{
	GError *error = NULL;
	mode_t mask = umask(~(S_IRUSR | S_IWUSR | S_IRGRP));

	g_file_set_contents(conf, contents, -1, &error);
	umask(mask);
	if (error) {
		pr_err("%s\n", error->message);
		g_error_free(error);
		return -EINVAL;
	}
	pr_info("Wrote `%s'\n", conf);
	return 0;
}

int load_config(char *pwddb, char *smbconf)
{
	int ret;

	usm_init();

	if (TOOL_IS_MOUNTD)
		usm_remove_all_users();

	ret = cp_parse_pwddb(pwddb);
	if (ret)
		return ret;

	if (TOOL_IS_ADDSHARE)
		cp_smbconf_parser_init();

	shm_init();

	if (TOOL_IS_MOUNTD)
		shm_remove_all_shares();

	ret = cp_parse_smbconf(smbconf);
	if (ret)
		return ret;

	if (TOOL_IS_MOUNTD) {
		sm_init();
		wp_init();
		rpc_init();
		ipc_init();
		spnego_init();
	}

	return ret;
}

void remove_config(void)
{
	if (TOOL_IS_MOUNTD) {
		spnego_destroy();
		ipc_destroy();
		rpc_destroy();
		wp_destroy();
		sm_destroy();
	} else if (TOOL_IS_ADDSHARE) {
		cp_smbconf_parser_destroy();
	}

	shm_destroy();
	usm_destroy();
}

int set_tool_main(char *name)
{
	if (!strcmp(name, "ksmbd.addshare"))
		tool_main = addshare_main;
	else if (!strcmp(name, "ksmbd.adduser"))
		tool_main = adduser_main;
//	else if (!strcmp(name, "ksmbd.control"))
//		tool_main = control_main;
	else if (!strcmp(name, "ksmbd.mountd"))
		tool_main = mountd_main;
	else
		tool_main = NULL;

	return !tool_main ? -EINVAL : 0;
}

const char *get_tool_name(void)
{
	if (TOOL_IS_ADDSHARE)
		return "ksmbd.addshare";
	if (TOOL_IS_ADDUSER)
		return "ksmbd.adduser";
	if (TOOL_IS_CONTROL)
		return "ksmbd.control";
	if (TOOL_IS_MOUNTD) {
		if (getppid() == global_conf.pid)
			return "ksmbd.mountd(worker)";
		if (getpid() == global_conf.pid)
			return "ksmbd.mountd(manager)";
		return "ksmbd.mountd";
	}
	return "ksmbd.tools";
}

int show_version(void)
{
	pr_info("ksmbd-tools version : %s\n", KSMBD_TOOLS_VERSION);
	return 0;
}

int main(int argc, char **argv)
{
	char *base_name;

	if (!*argv)
		return EXIT_FAILURE;

	base_name = strrchr(*argv, '/');
	base_name = base_name ? base_name + 1 : *argv;
	if (set_tool_main(base_name)) {
		pr_err("Invalid base name `%s'\n", base_name);
		return EXIT_FAILURE;
	}

	return tool_main(argc, argv);
}
