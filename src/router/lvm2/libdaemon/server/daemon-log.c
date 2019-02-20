/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools/tool.h"

#include "daemon-server.h"
#include "daemon-log.h"

#include <syslog.h>

struct backend {
	int id;
	void (*log)(log_state *s, void **state, int type, const char *message);
};

static void _log_syslog(log_state *s, void **state, int type, const char *message)
{
	int prio;

	if (!*state) { /* initialize */
		*state = (void *)1;
		openlog(s->name, LOG_PID, LOG_DAEMON);
	}

	switch (type) {
	case DAEMON_LOG_INFO: prio = LOG_INFO; break;
	case DAEMON_LOG_WARN: prio = LOG_WARNING; break;
	case DAEMON_LOG_ERROR: prio = LOG_ERR; break;
	case DAEMON_LOG_FATAL: prio = LOG_CRIT; break;
	default: prio = LOG_DEBUG; break;
	}

	syslog(prio, "%s", message);
}

static void _log_stderr(log_state *s, void **state, int type, const char *message)
{
	const char *prefix;

	switch (type) {
	case DAEMON_LOG_INFO: prefix = "I: "; break;
	case DAEMON_LOG_WARN: prefix = "W: " ; break;
	case DAEMON_LOG_ERROR: /* fall through */
	case DAEMON_LOG_FATAL: prefix = "E: " ; break;
	default: prefix = ""; break;
	}

	fprintf(stderr, "%s%s\n", prefix, message);
}

struct backend backend[] = {
	{ DAEMON_LOG_OUTLET_SYSLOG, _log_syslog },
	{ DAEMON_LOG_OUTLET_STDERR, _log_stderr },
	{ 0, 0 }
};

void daemon_log(log_state *s, int type, const char *message) {
	int i = 0;
	while ( backend[i].id ) {
		if ((int)(s->log_config[type] & backend[i].id) == backend[i].id )
			backend[i].log( s, &s->backend_state[i], type, message );
		++ i;
	}
}

static int _type_interesting(log_state *s, int type) {
	int i = 0;
	while ( backend[i].id ) {
		if ((int)(s->log_config[type] & backend[i].id) == backend[i].id )
			return 1;
		++ i;
	}
	return 0;
}

void daemon_logf(log_state *s, int type, const char *fmt, ...) {
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	if (dm_vasprintf(&buf, fmt, ap) >= 0) {
		daemon_log(s, type, buf);
		free(buf);
	} /* else return_0 */
	va_end(ap);
}

struct log_line_baton {
	log_state *s;
	int type;
	const char *prefix;
};

static int _log_line(const char *line, void *baton) {
	struct log_line_baton *b = baton;
	daemon_logf(b->s, b->type, "%s%s", b->prefix, line);
	return 0;
}

void daemon_log_cft(log_state *s, int type, const char *prefix, const struct dm_config_node *n)
{
	struct log_line_baton b = { .s = s, .type = type, .prefix = prefix };

	if (!_type_interesting(s, type))
		return;

	(void) dm_config_write_node(n, &_log_line, &b);
}

void daemon_log_multi(log_state *s, int type, const char *prefix, const char *msg)
{
	struct log_line_baton b = { .s = s, .type = type, .prefix = prefix };
	char *buf;
	char *pos;

	if (!_type_interesting(s, type))
		return;

	buf = strdup(msg);
	pos = buf;

	if (!buf)
		return; /* _0 */

	while (pos) {
		char *next = strchr(pos, '\n');
		if (next)
			*next = 0;
		_log_line(pos, &b);
		pos = next ? next + 1 : 0;
	}
	free(buf);
}

void daemon_log_enable(log_state *s, int outlet, int type, int enable)
{
	if (type >= 32)
		return;

	if (enable)
		s->log_config[type] |= outlet;
	else
		s->log_config[type] &= ~outlet;
}

static int _parse_one(log_state *s, int outlet, const char *type, int enable)
{
	int i;
	if (!strcmp(type, "all"))
		for (i = 0; i < 32; ++i)
			daemon_log_enable(s, outlet, i, enable);
	else if (!strcmp(type, "fatal"))
		daemon_log_enable(s, outlet, DAEMON_LOG_FATAL, enable);
	else if (!strcmp(type, "error"))
		daemon_log_enable(s, outlet, DAEMON_LOG_ERROR, enable);
	else if (!strcmp(type, "warn"))
		daemon_log_enable(s, outlet, DAEMON_LOG_WARN, enable);
	else if (!strcmp(type, "info"))
		daemon_log_enable(s, outlet, DAEMON_LOG_INFO, enable);
	else if (!strcmp(type, "wire"))
		daemon_log_enable(s, outlet, DAEMON_LOG_WIRE, enable);
	else if (!strcmp(type, "debug"))
		daemon_log_enable(s, outlet, DAEMON_LOG_DEBUG, enable);

	return 1;
}

int daemon_log_parse(log_state *s, int outlet, const char *types, int enable)
{
	char *buf;
	char *pos;

	if (!types || !types[0])
		return 1;

	if (!(buf = strdup(types)))
		return 0;

	pos = buf;
	while (pos) {
		char *next = strchr(pos, ',');
		if (next)
			*next = 0;
		if (!_parse_one(s, outlet, pos, enable)) {
			free(buf);
			return 0;
		}
		pos = next ? next + 1 : 0;
	}

	free(buf);

	return 1;
}
