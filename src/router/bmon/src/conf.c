/*
 * conf.c        Config Crap
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/conf.h>
#include <bmon/bindings.h>
#include <bmon/input.h>
#include <bmon/output.h>
#include <bmon/utils.h>

static int             show_only_running  = 1;
static int             do_signal_output   = 0;
static char            fg_char            = '*';
static char            bg_char            = '.';
static char            noise_char         = ':';
static char		unk_char	  = '?';
static y_unit_t        y_unit             = Y_DYNAMIC;
static x_unit_t        x_unit             = X_SEC;
static char *          configfile         = NULL;
static float           read_interval      = 1.0f;
static unsigned long   sleep_time         = 20000;
static int             use_si             = 0;
static int             ngraphs		  = 1;
static float           hb_factor	  = 0.1;
static float           rate_interval      = 1.0f;
static float           lifetime           = 30.0f;
static char *          itemtabfile        = "/etc/bmon/itemtab";

#if defined HAVE_CURSES
#if defined HAVE_USE_DEFAULT_COLORS
layout_t layout[] =
{
	{-1, -1, 0},                           /* dummy, not used */
	{-1, -1, 0},                           /* default */
	{-1, -1, A_REVERSE},                   /* statusbar */
	{-1, -1, 0},                           /* header */
	{-1, -1, 0},                           /* list */
	{-1, -1, A_REVERSE},                   /* selected */
};
#else
layout_t layout[] =
{
	{0, 0, 0},                              /* dummy, not used */
	{COLOR_BLACK, COLOR_WHITE, 0},          /* default */
	{COLOR_BLACK, COLOR_WHITE, A_REVERSE},  /* statusbar */
	{COLOR_BLACK, COLOR_WHITE, 0},          /* header */
	{COLOR_BLACK, COLOR_WHITE, 0},          /* list */
	{COLOR_BLACK, COLOR_WHITE, A_REVERSE},  /* selected */
};
#endif
#endif

#define SPLIT(str, var) 					\
	do {							\
		char *s;					\
		for (s = str; *s; s++) {			\
			if (*s == ' ' || *s == '\t')		\
				break;				\
		}						\
		if (!*s)					\
			break;					\
		*s = '\0';					\
		for (++s;*s == ' ' || *s == '\t'; s++);		\
		if (!*s)					\
			break;					\
		var = s;					\
	} while(0);

static void conf_read(const char *);

static inline void parse_include(char *buf)
{
	conf_read(buf);
}

#if defined HAVE_CURSES
static int parse_color(const char *color)
{
	if (!strcmp(color, "default"))
		return -1;
	else if (!strcasecmp(color, "black"))
		return COLOR_BLACK;
	else if (!strcasecmp(color, "red"))
		return COLOR_RED;
	else if (!strcasecmp(color, "green"))
		return COLOR_GREEN;
	else if (!strcasecmp(color, "yellow"))
		return COLOR_YELLOW;
	else if (!strcasecmp(color, "blue"))
		return COLOR_BLUE;
	else if (!strcasecmp(color, "magenta"))
		return COLOR_MAGENTA;
	else if (!strcasecmp(color, "cyan"))
		return COLOR_CYAN;
	else if (!strcasecmp(color, "white"))
		return COLOR_WHITE;
	else
		return -1;
}

static void parse_layout(char *buf)
{
	char *id, *fg = NULL, *bg = NULL, *attr, *s;
	layout_t l;

	id = buf;
	SPLIT(buf, fg)
	SPLIT(fg, bg)

	for (s = bg; *s; s++) {
		if (*s == ' ' || *s == '\t')
			break;
	}

	if (*s) {
		*s = '\0';
		s++;

		for (; *s == ' ' || *s == '\t'; s++);
		if (*s)
			attr = s;
		else
			attr = NULL;
	}
	else
		attr = NULL;

	if ((l.fg = parse_color(fg)) == -1)
		return;

	if ((l.bg = parse_color(bg)) == -1)
		return;

	l.attr = 0;

	if (attr)
	{
		if (!strcasecmp(attr, "reverse"))
			l.attr |= A_REVERSE;
		else if (!strcasecmp(attr, "bold"))
			l.attr |= A_BOLD;
		else if (!strcasecmp(attr, "underline"))
			l.attr |= A_UNDERLINE;
	}

#define COPY_LAYOUT(id) do {           \
		layout[id].fg = l.fg;		   \
		layout[id].bg = l.bg;		   \
		layout[id].attr = l.attr;	   \
	} while (0);

	if (!strcasecmp(id, "default"))
		COPY_LAYOUT(LAYOUT_DEFAULT)
	else if (!strcasecmp(id, "statusbar"))
		COPY_LAYOUT(LAYOUT_STATUSBAR)
	else if (!strcasecmp(id, "header"))
		COPY_LAYOUT(LAYOUT_HEADER)
	else if (!strcasecmp(id, "list"))
		COPY_LAYOUT(LAYOUT_LIST)
	else if (!strcasecmp(id, "selected"))
		COPY_LAYOUT(LAYOUT_SELECTED)

#undef COPY_LAYOUT
}
#endif

static void parse_bind(char *buf)
{
	char *ch, *cmd = NULL, *args, *s;
	binding_t *b;
	int i;

	ch = buf;
	SPLIT(buf, cmd);
	args = strdup(cmd);

	b = xcalloc(1, sizeof(binding_t));
	b->args[0] = args;

    for (s = args, i = 1; i < 255; i++) {
        s = strchr (s, ' ');
        if (s) {
            *s = '\0';
            s++;
			b->args[i] = s;
        } else
            break;
    }

	b->args[i] = NULL;

	b->ch = ch[0];
	b->cmd = strdup(b->args[0]);

	add_binding(b);
}

void conf_parse_option(const char *id, const char *value)
{
#define MATCH(STR) if (!strcasecmp(id, STR))
	MATCH("input")
		set_input(value);
	else MATCH("secondary_input")
		set_sec_input(value);
	else MATCH("output")
		set_output(value);
	else MATCH("secondary_output")
		set_sec_output(value);
	else MATCH("policy")
		item_parse_policy(value);
	else MATCH("read_interval")
		set_read_interval(value);
	else MATCH("sleep_time")
		set_sleep_time(value);
	else MATCH("show_all")
		set_show_only_running(0);
	else MATCH("use_si")
		set_use_si();
	else MATCH("nr_graphs")
		set_ngraphs(strtol(value, NULL, 0));
	else MATCH("heartbeat_factor")
		set_hb_factor(value);
	else MATCH("rate_interval")
		set_rate_interval(value);
	else MATCH("lifetime")
		set_lifetime(value);
	else MATCH("itemtab")
		set_itemtab(value);
#undef MATCH
}

tv_t * parse_tv(char *data)
{
	char *value;
	tv_t *tv = xcalloc(1, sizeof(tv_t));

	value = strchr(data, '=');

	if (value) {
		*value = '\0';
		++value;
		tv->value = strdup(value);
	}

	tv->type = strdup(data);
	return tv;
}

module_conf_t * parse_module(char *data)
{
	char *name = data, *opts = data, *next;
	module_conf_t *m;

	if (!*name)
		quit("No module name given");

	m = xcalloc(1, sizeof(module_conf_t));

	opts = strchr(data, ':');

	if (opts) {
		*opts = '\0';
		opts++;

		do {
			tv_t *tv;
			next = strchr(opts, ';');

			if (next) {
				*next = '\0';
				++next;
			}

			tv = parse_tv(opts);

			tv->next = m->attrs;
			m->attrs = tv;

			opts = next;
		} while(next);
	}

	m->name = strdup(name);
	return m;
}


module_conf_t * parse_module_param(const char *data)
{
	char *buf = strdup(data);
	char *next;
	char *current = buf;
	module_conf_t *m, *list = NULL;
	
	do {
		next = strchr(current, ',');

		if (next) {
			*next = '\0';
			++next;
		}

		m = parse_module(current);

		if (m) {
			m->next = list;
			list = m;
		}

		current = next;
	} while (next);

	free(buf);

	return list;
}


static inline void parse_config_line(char *buf)
{
	char *tok = NULL;
	
	SPLIT(buf, tok);
	if (!strcasecmp(buf, "include"))
		parse_include(tok);
#if defined HAVE_CURSES
	else if (!strcasecmp(buf, "layout"))
		parse_layout(tok);
#endif
	else if (!strcasecmp(buf, "bind"))
		parse_bind(tok);
	else
		conf_parse_option(buf, tok);
}

static void conf_read(const char *path)
{
	FILE *f;
	char buf[1024];

	if (!(f = fopen(path, "r")))
		return;

	memset(buf, 0, sizeof(buf));

	while (fgets(buf, sizeof(buf), f)) {
		char *p;

		if ('#' == *buf)
			goto skip;

		if ((p = strchr(buf, '\r')))
			*p = '\0';

		if ((p = strchr(buf, '\n')))
			*p = '\0';

		if (*buf)
			parse_config_line(buf);

skip:
		memset(buf, 0, sizeof(buf));
	}

	fclose(f);
}

void read_configfile(void)
{
	if (configfile)
		conf_read(configfile);
	else {
		conf_read("/etc/bmon.conf");
		
		if (getenv("HOME")) {
			char path[FILENAME_MAX+1];
			snprintf(path, sizeof(path), "%s/.bmonrc", getenv("HOME"));
			conf_read(path);
		}
	}
}


inline void set_configfile(const char *file)
{
	static int set = 0;
	if (!set) {
		configfile = strdup(file);
		set = 1;
	}
}

inline void set_itemtab(const char *file)
{
	static int set = 0;
	if (!set) {
		itemtabfile = strdup(file);
		set = 1;
	}
}

inline char *get_itemtab(void)
{
	return itemtabfile;
}

inline void set_read_interval(const char *i)
{
	static int set = 0;
	if (!set) {
		read_interval = (float) strtod(i, NULL);
		set = 1;
	}
}

inline float get_read_interval(void)
{
	return read_interval;
}

inline void get_read_interval_as_ts(timestamp_t *ts)
{
	float_to_ts(ts, read_interval);
}


inline void set_x_unit(const char *x, int force)
{
	static int set = 0;
	if (!set || force) {
		if (tolower(*x) == 's')
			x_unit = X_SEC;
		else if (tolower(*x) == 'm')
			x_unit = X_MIN;
		else if (tolower(*x) == 'h')
			x_unit = X_HOUR;
		else if (tolower(*x) == 'd')
			x_unit = X_DAY;
		else if (tolower(*x) == 'r')
			x_unit = X_READ;
		else
			quit("Unknown X-axis unit '%s'\n", x);
		set = 1;
	}
}

inline x_unit_t get_x_unit(void)
{
	return x_unit;
}

inline void set_y_unit(const char *y)
{
	static int set = 0;
	if (!set) {
		if (tolower(*y) == 'b')
			y_unit = Y_BYTE;
		else if (tolower(*y) == 'k')
			y_unit = Y_KILO;
		else if (tolower(*y) == 'm')
			y_unit = Y_MEGA;
		else if (tolower(*y) == 'g')
			y_unit = Y_GIGA;
		else if (tolower(*y) == 't')
			y_unit = Y_TERA;
		else
			quit("Unknown Y-axis unit '%s'\n", y);
		set = 1;
	}
}

inline y_unit_t get_y_unit(void)
{
	return y_unit;
}


inline char get_fg_char(void)
{
	return fg_char;
}

inline void set_fg_char(char c)
{
	static int set = 0;
	if (!set) {
		fg_char = c;
		set = 1;
	}
}

inline char get_unk_char(void)
{
	return unk_char;
}

inline void set_unk_char(char c)
{
	static int set = 0;
	if (!set) {
		unk_char = c;
		set = 1;
	}
}

inline char get_bg_char(void)
{
	return bg_char;
}

inline void set_bg_char(char c)
{
	static int set = 0;
	if (!set) {
		bg_char = c;
		set = 1;
	}
}

inline char get_noise_char(void)
{
	return noise_char;
}

inline void set_noise_char(char c)
{
	static int set = 0;
	if (!set) {
		noise_char = c;
		set = 1;
	}
}

inline void set_sleep_time(const char *s)
{
	static int set = 0;
	if (!set) {
		double d = strtod(s, NULL);
		sleep_time = (unsigned long) (d * 1000000.0f);
		set = 1;
	}
}

inline unsigned long get_sleep_time(void)
{
	return sleep_time;
}

inline void set_signal_output(int i)
{
	static int set = 0;
	if (!set) {
		do_signal_output = i;
		set = 1;
	}
}

inline int get_signal_output(void)
{
	return do_signal_output;
}

inline void set_show_only_running(int i)
{
	static int set = 0;
	if (!set) {
		show_only_running = i;
		set = 1;
	}
}

inline int get_show_only_running(void)
{
	return show_only_running;
}

inline void set_use_si(void)
{
	use_si = 1;
}

inline int get_use_si(void)
{
	return use_si;
}

inline void set_ngraphs(int n)
{
	static int set = 0;
	if (!set) {
		ngraphs = n;
		set = 1;
	}
}

inline int get_ngraphs(void)
{
	return ngraphs;
}

inline float get_hb_factor(void)
{
	return hb_factor;
}

inline void set_hb_factor(const char *s)
{
	static int set = 0;
	if (!set) {
		hb_factor = strtod(s, NULL);
		set = 1;
	}
}

inline void set_rate_interval(const char *i)
{
	static int set = 0;
	if (!set) {
		rate_interval = (float) strtod(i, NULL);
		set = 1;
	}
}

inline float get_rate_interval(void)
{
	return rate_interval;
}

inline void set_lifetime(const char *i)
{
	static int set = 0;
	if (!set) {
		lifetime = (float) strtod(i, NULL);
		set = 1;
	}
}

inline float get_lifetime(void)
{
	return lifetime / read_interval;
}
