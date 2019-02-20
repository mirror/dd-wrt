/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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

#include <asm/types.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <syslog.h>
#include <sched.h>
#include <dirent.h>
#include <ctype.h>
#include <getopt.h>

/*
 * This file can be compiled by itself as a man page generator.
 */
#ifdef MAN_PAGE_GENERATOR

#define stack

struct cmd_context {
	void *libmem;
};

#define log_error(fmt, args...) \
do { \
	printf(fmt "\n", ##args); \
} while (0)

#define dm_snprintf snprintf

static int dm_strncpy(char *dest, const char *src, size_t n)
{
	if (memccpy(dest, src, 0, n))
		return 1;

	if (n > 0)
		dest[n - 1] = '\0';

	return 0;
}

static char *dm_pool_strdup(void *p, const char *str)
{
	return strdup(str);
}

static void *dm_pool_alloc(void *p, size_t size)
{
	return malloc(size);
}

/* needed to include args.h */
#define ARG_COUNTABLE 0x00000001
#define ARG_GROUPABLE 0x00000002
struct cmd_context;
struct arg_values;

/* needed to include vals.h */
static inline int yes_no_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int activation_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int cachemetadataformat_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int cachemode_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int discards_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int mirrorlog_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int size_kb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int ssize_kb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int size_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int ssize_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int psize_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int nsize_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int int_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int uint32_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int int_arg_with_sign(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int int_arg_with_plus(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int extents_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int sextents_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int pextents_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int nextents_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int major_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int minor_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int string_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int tag_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int permission_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int metadatatype_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int units_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int segtype_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int alloc_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int locktype_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int readahead_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int regionsize_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int vgmetadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int pvmetadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int metadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int polloperation_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int writemostly_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int syncaction_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int reportformat_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int configreport_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }
static inline int configtype_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av) { return 0; }

/* needed to include commands.h when building man page generator */
#define CACHE_VGMETADATA        0x00000001
#define PERMITTED_READ_ONLY     0x00000002
#define ALL_VGS_IS_DEFAULT      0x00000004
#define ENABLE_ALL_DEVS         0x00000008      
#define ALLOW_UUID_AS_NAME      0x00000010
#define LOCKD_VG_SH             0x00000020
#define NO_METADATA_PROCESSING  0x00000040
#define MUST_USE_ALL_ARGS        0x00000100
#define ENABLE_DUPLICATE_DEVS    0x00000400
#define DISALLOW_TAG_ARGS        0x00000800
#define GET_VGNAME_FROM_OPTIONS  0x00001000
#define CAN_USE_ONE_SCAN	 0x00002000

/* create foo_CMD enums for command def ID's in command-lines.in */

enum {
#define cmd(a, b) a ,
#include "cmds.h"
#undef cmd
};

/* create foo_VAL enums for option and position values */

enum {
#define val(a, b, c, d) a ,
#include "vals.h"
#undef val
};

/* create foo_ARG enums for --option's */

enum {
#define arg(a, b, c, d, e, f, g) a ,
#include "args.h"
#undef arg
};

/* create foo_LVP enums for LV properties */

enum {
#define lvp(a, b, c) a,
#include "lv_props.h"
#undef lvp
};

/* create foo_LVT enums for LV types */

enum {
#define lvt(a, b, c) a,
#include "lv_types.h"
#undef lvt
};

#else  /* MAN_PAGE_GENERATOR */

#include "tools.h"

#endif /* MAN_PAGE_GENERATOR */

#include "command.h"       /* defines struct command */
#include "command-count.h" /* defines COMMAND_COUNT */

/* see cmd_names[] below, one for each unique "ID" in command-lines.in */

struct cmd_name {
	const char *enum_name; /* "foo_CMD" */
	int cmd_enum;          /* foo_CMD */
	const char *name;      /* "foo" from string after ID: */
};

/* create table of value names, e.g. String, and corresponding enum from vals.h */

struct val_name val_names[VAL_COUNT + 1] = {
#define val(a, b, c, d) { # a, a, b, c, d },
#include "vals.h"
#undef val
};

/* create table of option names, e.g. --foo, and corresponding enum from args.h */

struct opt_name opt_names[ARG_COUNT + 1] = {
#define arg(a, b, c, d, e, f, g) { # a, a, b, "", "--" c, d, e, f, g },
#include "args.h"
#undef arg
};

/* create table of lv property names, e.g. lv_is_foo, and corresponding enum from lv_props.h */

struct lv_prop lv_props[LVP_COUNT + 1] = {
#define lvp(a, b, c) { # a, a, b, c },
#include "lv_props.h"
#undef lvp
};

/* create table of lv type names, e.g. linear and corresponding enum from lv_types.h */

struct lv_type lv_types[LVT_COUNT + 1] = {
#define lvt(a, b, c) { # a, a, b, c },
#include "lv_types.h"
#undef lvt
};

/* create table of command IDs */

struct cmd_name cmd_names[CMD_COUNT + 1] = {
#define cmd(a, b) { # a, a, # b },
#include "cmds.h"
#undef cmd
};

/*
 * command_names[] and commands[] are defined in lvmcmdline.c when building lvm,
 * but need to be defined here when building the stand-alone man page generator.
 */

#ifdef MAN_PAGE_GENERATOR

struct command_name command_names[MAX_COMMAND_NAMES] = {
#define xx(a, b, c...) { # a, b, c },
#include "commands.h"
#undef xx
};
struct command commands[COMMAND_COUNT];

#else /* MAN_PAGE_GENERATOR */

struct command_name command_names[MAX_COMMAND_NAMES] = {
#define xx(a, b, c...) { # a, b, c, a},
#include "commands.h"
#undef xx
};
extern struct command commands[COMMAND_COUNT]; /* defined in lvmcmdline.c */

#endif /* MAN_PAGE_GENERATOR */

/* array of pointers into opt_names[] that is sorted alphabetically (by long opt name) */

struct opt_name *opt_names_alpha[ARG_COUNT + 1];

/* lvm_all is for recording options that are common for all lvm commands */

struct command lvm_all;

/* saves OO_FOO lines (groups of optional options) to include in multiple defs */

static int _oo_line_count;
#define MAX_OO_LINES 256

struct oo_line {
	char *name;
	char *line;
};
static struct oo_line _oo_lines[MAX_OO_LINES];

#define REQUIRED 1  /* required option */
#define OPTIONAL 0  /* optional option */
#define IGNORE (-1)   /* ignore option */

#define MAX_LINE 1024
#define MAX_LINE_ARGC 256
#define DESC_LINE 1024

/*
 * Contains _command_input[] which is command-lines.in with comments
 * removed and wrapped as a string.  The _command_input[] string is
 * used to populate commands[].
 */
#include "command-lines-input.h"

static void __add_optional_opt_line(struct cmd_context *cmdtool, struct command *cmd, int argc, char *argv[]);

/*
 * modifies buf, replacing the sep characters with \0
 * argv pointers point to positions in buf
 */

static char *_split_line(char *buf, int *argc, char **argv, char sep)
{
	char *p = buf, *rp = NULL;
	int i;

	argv[0] = p;

	for (i = 1; i < MAX_LINE_ARGC; i++) {
		p = strchr(buf, sep);
		if (!p)
			break;
		*p = '\0';

		argv[i] = p + 1;
		buf = p + 1;
	}
	*argc = i;

	/* we ended by hitting \0, return the point following that */
	if (!rp)
		rp = strchr(buf, '\0') + 1;

	return rp;
}

/* convert value string, e.g. Number, to foo_VAL enum */

static int _val_str_to_num(char *str)
{
	char name[MAX_LINE_ARGC];
	char *new;
	int i;

	/* compare the name before any suffix like _new or _<lvtype> */

	if (!dm_strncpy(name, str, sizeof(name)))
		return 0; /* Buffer is too short */

	if ((new = strchr(name, '_')))
		*new = '\0';

	for (i = 0; i < VAL_COUNT; i++) {
		if (!val_names[i].name)
			break;
		if (!strncmp(name, val_names[i].name, strlen(val_names[i].name)))
			return val_names[i].val_enum;
	}

	return 0;
}

/* convert "--option" to foo_ARG enum */

#define MAX_LONG_OPT_NAME_LEN 32

static int _opt_str_to_num(struct command *cmd, char *str)
{
	char long_name[MAX_LONG_OPT_NAME_LEN];
	char *p;
	int i;
	int first = 0, last = ARG_COUNT - 1, middle;

	dm_strncpy(long_name, str, sizeof(long_name));

	if ((p = strstr(long_name, "_long")))
		/*
		 * --foo_long means there are two args entries
		 * for --foo, one with a short option and one
		 * without, and we want the one without the
		 * short option (== 0).
		 */
		*p = '\0';

	/* Binary search in sorted array of long options (with duplicates) */
	while (first <= last) {
		middle = first + (last - first) / 2;
		if ((i = strcmp(opt_names_alpha[middle]->long_opt, long_name)) < 0)
			first = middle + 1;
		else if (i > 0)
			last = middle - 1;
		else {
			/* Matching long option string found.
			 * As sorted array contains duplicates, we need to also
			 * check left & right side for possible match
			 */
			for (i = middle;;) {
				if ((!p && !strstr(opt_names_alpha[i]->name, "_long_ARG")) ||
				    (p && !opt_names_alpha[i]->short_opt))
					return opt_names_alpha[i]->opt_enum; /* Found */
				/* Check if there is something on the 'left-side' */
				if ((i <= first) || strcmp(opt_names_alpha[--i]->long_opt, long_name))
					break;
			}

			/* Nothing on the left, so look on the 'right-side' */
			for (i = middle + 1; i <= last; ++i) {
				if (strcmp(opt_names_alpha[i]->long_opt, long_name))
					break;
				if ((!p && !strstr(opt_names_alpha[i]->name, "_long_ARG")) ||
				    (p && !opt_names_alpha[i]->short_opt))
					return opt_names_alpha[i]->opt_enum; /* Found */
			}

			break; /* Nothing... */
		}
	}

	log_error("Parsing command defs: unknown opt str: \"%s\"%s%s.",
		  str, p ? " ": "", p ? long_name : "");
	cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;

	return ARG_UNUSED;
}

/* "foo" string to foo_CMD int */

int command_id_to_enum(const char *str)
{
	int i;

	for (i = 1; i < CMD_COUNT; i++) {
		if (!strcmp(str, cmd_names[i].name))
			return cmd_names[i].cmd_enum;
	}

	return CMD_NONE;
}

/* "lv_is_prop" to is_prop_LVP */

static int _lvp_name_to_enum(struct command *cmd, char *str)
{
	int i;

	for (i = 1; i < LVP_COUNT; i++) {
		if (!strcmp(str, lv_props[i].name))
			return lv_props[i].lvp_enum;
	}

	log_error("Parsing command defs: unknown lv property %s.", str);
	cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
	return LVP_NONE;
}

/* "type" to type_LVT */

static int _lvt_name_to_enum(struct command *cmd, char *str)
{
	int i;

	for (i = 1; i < LVT_COUNT; i++) {
		if (!strcmp(str, lv_types[i].name))
			return lv_types[i].lvt_enum;
	}

	log_error("Parsing command defs: unknown lv type %s.", str);
	cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
	return LVT_NONE;
}

/* LV_<type> to <type>_LVT */

static int _lv_to_enum(struct command *cmd, char *name)
{
	return _lvt_name_to_enum(cmd, name + 3);
}

/*
 * LV_<type1>_<type2> to lvt_bits
 *
 * type1 to lvt_enum
 * lvt_bits |= lvt_enum_to_bit(lvt_enum)
 * type2 to lvt_enum
 * lvt_bits |= lvt_enum_to_bit(lvt_enum)
 */

#define LVTYPE_LEN 64

static uint64_t _lv_to_bits(struct command *cmd, char *name)
{
	char buf[LVTYPE_LEN];
	char *argv[MAX_LINE_ARGC];
	uint64_t lvt_bits = 0;
	int lvt_enum;
	int argc;
	int i;

	(void) dm_strncpy(buf, name, LVTYPE_LEN);

	_split_line(buf, &argc, argv, '_');

	/* 0 is "LV" */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "new"))
			continue;
		lvt_enum = _lvt_name_to_enum(cmd, argv[i]);
		lvt_bits |= lvt_enum_to_bit(lvt_enum);
	}

	return lvt_bits;
}

static struct command_name *_find_command_name(const char *name)
{
	int i;

	if (!islower(name[0]))
		return NULL; /* Commands starts with lower-case */

	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
		if (!command_names[i].name)
			break;
		if (!strcmp(command_names[i].name, name))
			return &command_names[i];
	}

	return NULL;
}

static const char *_is_command_name(char *str)
{
	const struct command_name *c;

	if ((c = _find_command_name(str)))
		return c->name;

	return NULL;
}

static int _is_opt_name(char *str)
{
	if ((str[0] == '-') && (str[1] == '-'))
		return 1;

	if ((str[0] == '-') && (str[1] != '-'))
		log_error("Parsing command defs: options must be specified in long form: %s.", str);

	return 0;
}

/*
 * "Select" as a pos name means that the position
 * can be empty if the --select option is used.
 */

static int _is_pos_name(char *str)
{
	switch (str[0]) {
	case 'V': return (str[1] == 'G'); /* VG */
	case 'L': return (str[1] == 'V'); /* LV */
	case 'P': return (str[1] == 'V'); /* PV */
	case 'T': return (strncmp(str, "Tag", 3) == 0);
	case 'S': return ((strncmp(str, "String", 6) == 0) ||
			  (strncmp(str, "Select", 6) == 0));
	}

	return 0;
}

static int _is_oo_definition(char *str)
{
	if (!strncmp(str, "OO_", 3) && strchr(str, ':'))
		return 1;
	return 0;
}

static int _is_oo_line(char *str)
{
	if (!strncmp(str, "OO:", 3))
		return 1;
	return 0;
}

static int _is_io_line(char *str)
{
	if (!strncmp(str, "IO:", 3))
		return 1;
	return 0;
}

static int _is_op_line(char *str)
{
	if (!strncmp(str, "OP:", 3))
		return 1;
	return 0;
}

static int _is_desc_line(char *str)
{
	if (!strncmp(str, "DESC:", 5))
		return 1;
	return 0;
}

static int _is_flags_line(char *str)
{
	if (!strncmp(str, "FLAGS:", 6))
		return 1;
	return 0;
}

static int _is_rule_line(char *str)
{
	if (!strncmp(str, "RULE:", 5))
		return 1;
	return 0;
}

static int _is_id_line(char *str)
{
	if (!strncmp(str, "ID:", 3))
		return 1;
	return 0;
}

/*
 * Save a positional arg in a struct arg_def.
 * Parse str for anything that can appear in a position,
 * like VG, VG|LV, VG|LV_linear|LV_striped, etc.
 */

static void _set_pos_def(struct command *cmd, char *str, struct arg_def *def)
{
	char *argv[MAX_LINE_ARGC];
	int argc;
	char *name;
	int val_enum;
	int i;

	_split_line(str, &argc, argv, '|');

	for (i = 0; i < argc; i++) {
		name = argv[i];

		val_enum = _val_str_to_num(name);

		if (!val_enum) {
			log_error("Parsing command defs: unknown pos arg: %s.", name);
			cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
			return;
		}

		def->val_bits |= val_enum_to_bit(val_enum);

		if ((val_enum == lv_VAL) && strchr(name, '_'))
			def->lvt_bits = _lv_to_bits(cmd, name);

		if (strstr(name, "_new")) {
			if (val_enum == lv_VAL)
				def->flags |= ARG_DEF_FLAG_NEW_LV;
			else if (val_enum == vg_VAL)
				def->flags |= ARG_DEF_FLAG_NEW_VG;
		}
	}
}

/*
 * Save an option arg in a struct arg_def.
 * Parse str for anything that can follow --option.
 */

static void _set_opt_def(struct cmd_context *cmdtool, struct command *cmd, char *str, struct arg_def *def)
{
	char *argv[MAX_LINE_ARGC];
	int argc;
	char *name;
	int val_enum;
	int i;

	_split_line(str, &argc, argv, '|');

	for (i = 0; i < argc; i++) {
		name = argv[i];

		val_enum = _val_str_to_num(name);

		if (!val_enum) {
			/* a literal number or string */

			if (isdigit(name[0]))
				val_enum = constnum_VAL;

			else if (isalpha(name[0]))
				val_enum = conststr_VAL;

			else {
				log_error("Parsing command defs: unknown opt arg: %s.", name);
				cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
				return;
			}
		}


		def->val_bits |= val_enum_to_bit(val_enum);

		if (val_enum == constnum_VAL)
			def->num = (uint64_t)atoi(name);

		if (val_enum == conststr_VAL) {
			def->str = dm_pool_strdup(cmdtool->libmem, name);

			if (!def->str) {
				/* FIXME */
				stack;
				return;
			}
		}

		if (val_enum == lv_VAL) {
			if (strchr(name, '_'))
				def->lvt_bits = _lv_to_bits(cmd, name);
		}

		if (strstr(name, "_new")) {
			if (val_enum == lv_VAL)
				def->flags |= ARG_DEF_FLAG_NEW_LV;
			else if (val_enum == vg_VAL)
				def->flags |= ARG_DEF_FLAG_NEW_VG;
				
		}
	}
}

/*
 * Save a set of common options so they can be included in
 * multiple command defs.
 *
 * OO_FOO: --opt1 ...
 *
 * oo->name = "OO_FOO";
 * oo->line = "--opt1 ...";
 */

static void _add_oo_definition_line(const char *name, const char *line)
{
	struct oo_line *oo;
	char *colon;
	char *start;

	oo = &_oo_lines[_oo_line_count++];

	if (!(oo->name = strdup(name))) {
		log_error("Failer to duplicate name %s.", name);
		return; /* FIXME: return code */
	}

	if ((colon = strchr(oo->name, ':')))
		*colon = '\0';
	else {
		log_error("Parsing command defs: invalid OO definition.");
		return;
	}

	start = strchr(line, ':') + 2;
	if (!(oo->line = strdup(start))) {
		log_error("Failer to duplicate line %s.", start);
		return;
	}
}

/* Support OO_FOO: continuing on multiple lines. */

static void _append_oo_definition_line(const char *new_line)
{
	struct oo_line *oo;
	char *old_line;
	char *line;
	int len;

	oo = &_oo_lines[_oo_line_count - 1];

	old_line = oo->line;

	/* +2 = 1 space between old and new + 1 terminating \0 */
	len = strlen(old_line) + strlen(new_line) + 2;
	line = malloc(len);
	if (!line) {
		log_error("Parsing command defs: no memory.");
		return;
	}

	(void) dm_snprintf(line, len, "%s %s", old_line, new_line);
	free(oo->line);
	oo->line = line;
}

/* Find a saved OO_FOO definition. */

#define OO_NAME_LEN 64

static char *_get_oo_line(const char *str)
{
	char *name;
	char *end;
	char str2[OO_NAME_LEN];
	int i;

	dm_strncpy(str2, str, sizeof(str2));
	if ((end = strchr(str2, ':')))
		*end = '\0';
	if ((end = strchr(str2, ',')))
		*end = '\0';

	for (i = 0; i < _oo_line_count; i++) {
		name = _oo_lines[i].name;
		if (!strcmp(name, str2))
			return _oo_lines[i].line;
	}
	return NULL;
}

/*
 * Add optional_opt_args entries when OO_FOO appears on OO: line,
 * i.e. include common options from an OO_FOO definition.
 */

static void _include_optional_opt_args(struct cmd_context *cmdtool, struct command *cmd, const char *str)
{
	char *oo_line;
	char *line;
	char *line_argv[MAX_LINE_ARGC];
	int line_argc;

	if (!(oo_line = _get_oo_line(str))) {
		log_error("Parsing command defs: no OO line found for %s.", str);
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	if (!(line = strdup(oo_line))) {
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	_split_line(line, &line_argc, line_argv, ' ');
	__add_optional_opt_line(cmdtool, cmd, line_argc, line_argv);
	free(line);
}

/*
 * When an --option is seen, add a new opt_args entry for it.
 * This function sets the opt_args.opt value for it.
 */

static void _add_opt_arg(struct command *cmd, char *str,
			int *takes_arg, int *already, int required)
{
	char *comma;
	int opt;
	int i;

	/* opt_arg.opt set here */
	/* opt_arg.def will be set in _update_prev_opt_arg() if needed */

	if ((comma = strchr(str, ',')))
		*comma = '\0';

	/*
	 * Work around nasty hack where --uuid is used for both uuid_ARG
	 * and uuidstr_ARG.  The input uses --uuidstr, where an actual
	 * command uses --uuid string.
	 */
	if (!strcmp(str, "--uuidstr")) {
		opt = uuidstr_ARG;
		goto skip;
	}

	opt = _opt_str_to_num(cmd, str);

	/* If the binary-search finds uuidstr_ARG switch to uuid_ARG */
	if (opt == uuidstr_ARG)
		opt = uuid_ARG;

	/* Skip adding an optional opt if it is already included. */
	if (already && !required) {
		for (i = 0; i < cmd->oo_count; i++) {
			if (cmd->optional_opt_args[i].opt == opt) {
				*already = 1;
				*takes_arg = opt_names[opt].val_enum ? 1 : 0;
				return;
			}
		}
	}

skip:
	if (required > 0)
		cmd->required_opt_args[cmd->ro_count++].opt = opt;
	else if (!required)
		cmd->optional_opt_args[cmd->oo_count++].opt = opt;
	else if (required < 0)
		cmd->ignore_opt_args[cmd->io_count++].opt = opt;

	*takes_arg = opt_names[opt].val_enum ? 1 : 0;
}

/*
 * After --option has been seen, this function sets opt_args.def value
 * for the value that appears after --option.
 */

static void _update_prev_opt_arg(struct cmd_context *cmdtool, struct command *cmd, char *str, int required)
{
	struct arg_def def = { 0 };
	char *comma;

	if (str[0] == '-') {
		log_error("Parsing command defs: option %s must be followed by an arg.", str);
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	/* opt_arg.def set here */
	/* opt_arg.opt was previously set in _add_opt_arg() when --foo was read */

	if ((comma = strchr(str, ',')))
		*comma = '\0';

	_set_opt_def(cmdtool, cmd, str, &def);

	if (required > 0)
		cmd->required_opt_args[cmd->ro_count-1].def = def;
	else if (!required)
		cmd->optional_opt_args[cmd->oo_count-1].def = def;
	else if (required < 0)
		cmd->ignore_opt_args[cmd->io_count-1].def = def;
}

/*
 * When an position arg is seen, add a new pos_args entry for it.
 * This function sets the pos_args.pos and pos_args.def.
 */

static void _add_pos_arg(struct command *cmd, char *str, int required)
{
	struct arg_def def = { 0 };

	/* pos_arg.pos and pos_arg.def are set here */

	_set_pos_def(cmd, str, &def);

	if (required) {
		cmd->required_pos_args[cmd->rp_count].pos = cmd->pos_count++;
		cmd->required_pos_args[cmd->rp_count].def = def;
		cmd->rp_count++;
	} else {
		cmd->optional_pos_args[cmd->op_count].pos = cmd->pos_count++;;
		cmd->optional_pos_args[cmd->op_count].def = def;
		cmd->op_count++;
	}
}

/* Process something that follows a pos arg, which is not a new pos arg. */

static void _update_prev_pos_arg(struct command *cmd, char *str, int required)
{
	struct arg_def *def;

	/* a previous pos_arg.def is modified here */

	if (required)
		def = &cmd->required_pos_args[cmd->rp_count-1].def;
	else
		def = &cmd->optional_pos_args[cmd->op_count-1].def;

	if (!strcmp(str, "..."))
		def->flags |= ARG_DEF_FLAG_MAY_REPEAT;
	else {
		log_error("Parsing command defs: unknown pos arg: %s.", str);
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}
}

/* Process what follows OO:, which are the optional opt args for the cmd def. */

static void __add_optional_opt_line(struct cmd_context *cmdtool, struct command *cmd, int argc, char *argv[])
{
	int takes_arg = 0;
	int already;
	int i;

	for (i = 0; i < argc; i++) {
		if (!i && !strncmp(argv[i], "OO:", 3))
			continue;

		already = 0;

		if (_is_opt_name(argv[i]))
			_add_opt_arg(cmd, argv[i], &takes_arg, &already, OPTIONAL);
		else if (!strncmp(argv[i], "OO_", 3))
			_include_optional_opt_args(cmdtool, cmd, argv[i]);
		else if (takes_arg)
			_update_prev_opt_arg(cmdtool, cmd, argv[i], OPTIONAL);
		else {
			log_error("Parsing command defs: can't parse argc %d argv %s prev %s.",
				i, argv[i], argv[i-1]);
			cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
			return;
		}

		if (already && takes_arg)
			i++;
	}
}

/* Process what follows IO:, which are the ignore options for the cmd def. */

static void _add_ignore_opt_line(struct cmd_context *cmdtool, struct command *cmd, int argc, char *argv[])
{
	int takes_arg = 0;
	int i;

	for (i = 0; i < argc; i++) {
		if (!i && !strncmp(argv[i], "IO:", 3))
			continue;
		if (_is_opt_name(argv[i]))
			_add_opt_arg(cmd, argv[i], &takes_arg, NULL, IGNORE);
		else if (takes_arg)
			_update_prev_opt_arg(cmdtool, cmd, argv[i], IGNORE);
		else {
			log_error("Parsing command defs: can't parse argc %d argv %s prev %s.",
				i, argv[i], argv[i-1]);
			cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
			return;
		}
	}
}

/* Process what follows OP:, which are optional pos args for the cmd def. */

static void _add_optional_pos_line(struct command *cmd, int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!i && !strncmp(argv[i], "OP:", 3))
			continue;
		if (_is_pos_name(argv[i]))
			_add_pos_arg(cmd, argv[i], OPTIONAL);
		else
			_update_prev_pos_arg(cmd, argv[i], OPTIONAL);
	}
}

static void _add_required_opt_line(struct cmd_context *cmdtool, struct command *cmd, int argc, char *argv[])
{
	int takes_arg = 0;
	int i;

	for (i = 0; i < argc; i++) {
		if (_is_opt_name(argv[i]))
			_add_opt_arg(cmd, argv[i], &takes_arg, NULL, REQUIRED);
		else if (takes_arg)
			_update_prev_opt_arg(cmdtool, cmd, argv[i], REQUIRED);
		else {
			log_error("Parsing command defs: can't parse argc %d argv %s prev %s.",
				  i, argv[i], argv[i-1]);
			cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
			return;
		}
	}
}

/*
 * Add to required_opt_args from an OO_FOO definition.
 * (This is the special case of vgchange/lvchange where one
 * optional option is required, and others are then optional.)
 * The set of options from OO_FOO are saved in required_opt_args,
 * and flag CMD_FLAG_ONE_REQUIRED_OPT is set on the cmd indicating
 * this special case.
 */
 
static void _include_required_opt_args(struct cmd_context *cmdtool, struct command *cmd, char *str)
{
	char *oo_line;
	char *line;
	char *line_argv[MAX_LINE_ARGC];
	int line_argc;

	if (!(oo_line = _get_oo_line(str))) {
		log_error("Parsing command defs: no OO line found for %s.", str);
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	if (!(line = strdup(oo_line))) {
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	_split_line(line, &line_argc, line_argv, ' ');
	_add_required_opt_line(cmdtool, cmd, line_argc, line_argv);
	free(line);
}

/* Process what follows command_name, which are required opt/pos args. */

static void _add_required_line(struct cmd_context *cmdtool, struct command *cmd, int argc, char *argv[])
{
	int i;
	int takes_arg;
	int prev_was_opt = 0, prev_was_pos = 0;

	/* argv[0] is command name */

	for (i = 1; i < argc; i++) {

		if (_is_opt_name(argv[i])) {
			/* add new required_opt_arg */
			_add_opt_arg(cmd, argv[i], &takes_arg, NULL, REQUIRED);
			prev_was_opt = 1;
			prev_was_pos = 0;

		} else if (prev_was_opt && takes_arg) {
			/* set value for previous required_opt_arg */
			_update_prev_opt_arg(cmdtool, cmd, argv[i], REQUIRED);
			prev_was_opt = 0;
			prev_was_pos = 0;

		} else if (_is_pos_name(argv[i])) {
			/* add new required_pos_arg */
			_add_pos_arg(cmd, argv[i], REQUIRED);
			prev_was_opt = 0;
			prev_was_pos = 1;

		} else if (!strncmp(argv[i], "OO_", 3)) {
			/* one required_opt_arg is required, special case lv/vgchange */
			cmd->cmd_flags |= CMD_FLAG_ONE_REQUIRED_OPT;
			_include_required_opt_args(cmdtool, cmd, argv[i]);

		} else if (prev_was_pos) {
			/* set property for previous required_pos_arg */
			_update_prev_pos_arg(cmd, argv[i], REQUIRED);
		} else {
			log_error("Parsing command defs: can't parse argc %d argv %s prev %s.",
				  i, argv[i], argv[i-1]);
			cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
			return;
		}
	}
}

static void _add_flags(struct command *cmd, char *line)
{
	if (strstr(line, "SECONDARY_SYNTAX"))
		cmd->cmd_flags |= CMD_FLAG_SECONDARY_SYNTAX;
	if (strstr(line, "PREVIOUS_SYNTAX"))
		cmd->cmd_flags |= CMD_FLAG_PREVIOUS_SYNTAX;
}

#define MAX_RULE_OPTS 64

static void _add_rule(struct cmd_context *cmdtool, struct command *cmd, char *line)
{
	struct cmd_rule *rule;
	char *line_argv[MAX_LINE_ARGC];
	char *arg;
	int line_argc;
	int i, lvt_enum, lvp_enum;
	int check = 0;

	if (cmd->rule_count == CMD_MAX_RULES) {
		log_error("Parsing command defs: too many rules for cmd.");
		cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
		return;
	}

	rule = &cmd->rules[cmd->rule_count++];

	_split_line(line, &line_argc, line_argv, ' ');

	for (i = 0; i < line_argc; i++) {
		arg = line_argv[i];

		if (!strcmp(arg, "not")) {
			rule->rule = RULE_INVALID;
			check = 1;
		}

		else if (!strcmp(arg, "and")) {
			rule->rule = RULE_REQUIRE;
			check = 1;
		}

		else if (!strncmp(arg, "all", 3)) {
			/* opt/lvt_bits/lvp_bits all remain 0 to mean all */
			continue;
		}

		else if (!strncmp(arg, "--", 2)) {
			if (!rule->opts) {
				if (!(rule->opts = dm_pool_alloc(cmdtool->libmem, MAX_RULE_OPTS * sizeof(int)))) {
					log_error("Parsing command defs: no mem.");
					cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
					return;
				}
				memset(rule->opts, 0, MAX_RULE_OPTS * sizeof(int));
			}

			if (!rule->check_opts) {
				if (!(rule->check_opts = dm_pool_alloc(cmdtool->libmem, MAX_RULE_OPTS * sizeof(int)))) {
					log_error("Parsing command defs: no mem.");
					cmd->cmd_flags |= CMD_FLAG_PARSE_ERROR;
					return;
				}
				memset(rule->check_opts, 0, MAX_RULE_OPTS * sizeof(int));
			}

			if (check)
				rule->check_opts[rule->check_opts_count++] = _opt_str_to_num(cmd, arg);
			else
				rule->opts[rule->opts_count++] = _opt_str_to_num(cmd, arg);
		}

		else if (!strncmp(arg, "LV_", 3)) {
			lvt_enum = _lv_to_enum(cmd, arg);

			if (check)
				rule->check_lvt_bits |= lvt_enum_to_bit(lvt_enum);
			else
				rule->lvt_bits |= lvt_enum_to_bit(lvt_enum);
		}

		else if (!strncmp(arg, "lv_is_", 6)) {
			lvp_enum = _lvp_name_to_enum(cmd, arg);

			if (check)
				rule->check_lvp_bits |= lvp_enum_to_bit(lvp_enum);
			else
				rule->lvp_bits |= lvp_enum_to_bit(lvp_enum);
		}
	}
}

/* The given option is common to all lvm commands (set in lvm_all). */

static int _is_lvm_all_opt(int opt)
{
	int oo;

	for (oo = 0; oo < lvm_all.oo_count; oo++) {
		if (lvm_all.optional_opt_args[oo].opt == opt)
			return 1;
	}
	return 0;
}

/* Find common options for all variants of each command name. */

void factor_common_options(void)
{
	int cn, opt_enum, ci, oo, ro, found;
	struct command *cmd;

	for (cn = 0; cn < MAX_COMMAND_NAMES; cn++) {
		if (!command_names[cn].name)
			break;

		/* already factored */
		if (command_names[cn].variants)
			continue;

		for (ci = 0; ci < COMMAND_COUNT; ci++) {
			cmd = &commands[ci];

			if (strcmp(cmd->name, command_names[cn].name))
				continue;

			command_names[cn].variants++;
		}

		for (opt_enum = 0; opt_enum < ARG_COUNT; opt_enum++) {

			for (ci = 0; ci < COMMAND_COUNT; ci++) {
				cmd = &commands[ci];

				if (strcmp(cmd->name, command_names[cn].name))
					continue;

				if (cmd->ro_count)
					command_names[cn].variant_has_ro = 1;
				if (cmd->rp_count)
					command_names[cn].variant_has_rp = 1;
				if (cmd->oo_count)
					command_names[cn].variant_has_oo = 1;
				if (cmd->op_count)
					command_names[cn].variant_has_op = 1;

				for (ro = 0; ro < cmd->ro_count; ro++) {
					command_names[cn].all_options[cmd->required_opt_args[ro].opt] = 1;

					if ((cmd->required_opt_args[ro].opt == size_ARG) && !strncmp(cmd->name, "lv", 2))
						command_names[cn].all_options[extents_ARG] = 1;
				}
				for (oo = 0; oo < cmd->oo_count; oo++)
					command_names[cn].all_options[cmd->optional_opt_args[oo].opt] = 1;

				found = 0;

				for (oo = 0; oo < cmd->oo_count; oo++) {
					if (cmd->optional_opt_args[oo].opt == opt_enum) {
						found = 1;
						break;
					}
				}

				if (!found)
					goto next_opt;
			}

			/* all commands starting with this name use this option */
			command_names[cn].common_options[opt_enum] = 1;
 next_opt:
			;
		}
	}
}

/* FIXME: use a flag in command_name struct? */

int command_has_alternate_extents(const char *name)
{
	if (name[0] != 'l')
		return 0;
	if (!strcmp(name, "lvcreate") ||
	    !strcmp(name, "lvresize") ||
	    !strcmp(name, "lvextend") ||
	    !strcmp(name, "lvreduce"))
		return 1;
	return 0;
}

static int _long_name_compare(const void *on1, const void *on2)
{
	const struct opt_name * const *optname1 = on1;
	const struct opt_name * const *optname2 = on2;

	return strcmp((*optname1)->long_opt + 2, (*optname2)->long_opt + 2);
}

/* Create list of option names for printing alphabetically. */

static void _create_opt_names_alpha(void)
{
	int i;

	for (i = 0; i < ARG_COUNT; i++)
		opt_names_alpha[i] = &opt_names[i];

	qsort(opt_names_alpha, ARG_COUNT, sizeof(long), _long_name_compare);
}

static int _copy_line(char *line, int max_line, int *position)
{
	int p = *position;
	int i = 0;

	memset(line, 0, max_line);

	while (1) {
		line[i] = _command_input[p];
		i++;
		p++;

		if (_command_input[p] == '\n') {
			p++;
			break;
		}

		if (i == (max_line - 1))
			break;
	}
	*position = p;
	return 1;
}

int define_commands(struct cmd_context *cmdtool, const char *run_name)
{
	struct command *cmd = NULL;
	char line[MAX_LINE];
	char line_orig[MAX_LINE];
	char *line_argv[MAX_LINE_ARGC];
	const char *name;
	char *n;
	int line_argc;
	int cmd_count = 0;
	int prev_was_oo_def = 0;
	int prev_was_oo = 0;
	int prev_was_op = 0;
	int copy_pos = 0;
	int skip = 0;
	int i;

	if (run_name && !strcmp(run_name, "help"))
		run_name = NULL;

	_create_opt_names_alpha();

	/* Process each line of command-lines-input.h (from command-lines.in) */

	while (_copy_line(line, MAX_LINE, &copy_pos)) {
		if (line[0] == '\n')
			break;

		if ((n = strchr(line, '\n')))
			*n = '\0';

		memcpy(line_orig, line, sizeof(line));
		_split_line(line, &line_argc, line_argv, ' ');

		if (!line_argc)
			continue;

		/* New cmd def begins: command_name <required opt/pos args> */
		if ((name = _is_command_name(line_argv[0]))) {
			if (cmd_count >= COMMAND_COUNT) {
				return 0;
			}

			/*
			 * FIXME: when running one specific command name,
			 * we can optimize by not parsing command defs
			 * that don't start with that command name.
			 */

			cmd = &commands[cmd_count];
			cmd->command_index = cmd_count;
			cmd_count++;
			cmd->name = dm_pool_strdup(cmdtool->libmem, name);

			if (!cmd->name) {
				/* FIXME */
				stack;
				return 0;
			}

			if (run_name && strcmp(run_name, name)) {
				skip = 1;
				prev_was_oo_def = 0;
				prev_was_oo = 0;
				prev_was_op = 0;
				continue;
			}
			skip = 0;

			cmd->pos_count = 1;
			_add_required_line(cmdtool, cmd, line_argc, line_argv);

			/* Every cmd gets the OO_ALL options */
			_include_optional_opt_args(cmdtool, cmd, "OO_ALL:");
			continue;
		}

		/*
		 * All other kinds of lines are processed in the
		 * context of the existing command[].
		 */

		if (_is_desc_line(line_argv[0]) && !skip && cmd) {
			char *desc = dm_pool_strdup(cmdtool->libmem, line_orig);
			if (cmd->desc) {
				int newlen = strlen(cmd->desc) + strlen(desc) + 2;
				char *newdesc = dm_pool_alloc(cmdtool->libmem, newlen);
				if (newdesc) {
					memset(newdesc, 0, newlen);
					snprintf(newdesc, newlen, "%s %s", cmd->desc, desc);
					cmd->desc = newdesc;
				} else {
					/* FIXME */
					stack;
					return 0;
				}
			} else
				cmd->desc = desc;
			continue;
		}

		if (_is_flags_line(line_argv[0]) && !skip && cmd) {
			_add_flags(cmd, line_orig);
			continue;
		}

		if (_is_rule_line(line_argv[0]) && !skip && cmd) {
			_add_rule(cmdtool, cmd, line_orig);
			continue;
		}

		if (_is_id_line(line_argv[0]) && cmd) {
			cmd->command_id = dm_pool_strdup(cmdtool->libmem, line_argv[1]);

			if (!cmd->command_id) {
				/* FIXME */
				stack;
				return 0;
			}
			continue;
		}

		/* OO_FOO: ... */
		if (_is_oo_definition(line_argv[0])) {
			_add_oo_definition_line(line_argv[0], line_orig);
			prev_was_oo_def = 1;
			prev_was_oo = 0;
			prev_was_op = 0;
			continue;
		}

		/* OO: ... */
		if (_is_oo_line(line_argv[0]) && !skip && cmd) {
			__add_optional_opt_line(cmdtool, cmd, line_argc, line_argv);
			prev_was_oo_def = 0;
			prev_was_oo = 1;
			prev_was_op = 0;
			continue;
		}

		/* OP: ... */
		if (_is_op_line(line_argv[0]) && !skip && cmd) {
			_add_optional_pos_line(cmd, line_argc, line_argv);
			prev_was_oo_def = 0;
			prev_was_oo = 0;
			prev_was_op = 1;
			continue;
		}

		/* IO: ... */
		if (_is_io_line(line_argv[0]) && !skip && cmd) {
			_add_ignore_opt_line(cmdtool, cmd, line_argc, line_argv);
			prev_was_oo = 0;
			prev_was_op = 0;
			continue;
		}

		/* handle OO_FOO:, OO:, OP: continuing on multiple lines */

		if (prev_was_oo_def) {
			_append_oo_definition_line(line_orig);
			continue;
		}

		if (prev_was_oo && cmd) {
			__add_optional_opt_line(cmdtool, cmd, line_argc, line_argv);
			continue;
		}

		if (prev_was_op && cmd) {
			_add_optional_pos_line(cmd, line_argc, line_argv);
			continue;
		}

		if (!skip)
			log_error("Parsing command defs: can't process input line %s.", line_orig);
	}

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (commands[i].cmd_flags & CMD_FLAG_PARSE_ERROR)
			return 0;
	}

	_include_optional_opt_args(cmdtool, &lvm_all, "OO_ALL");

	for (i = 0; i < _oo_line_count; i++) {
		struct oo_line *oo = &_oo_lines[i];
		free(oo->name);
		free(oo->line);
	}
	memset(&_oo_lines, 0, sizeof(_oo_lines));
	_oo_line_count = 0;

	return 1;
}

/*
 * The opt_names[] table describes each option.  It is indexed by the
 * option typedef, e.g. size_ARG.  The size_ARG entry specifies the
 * option name, e.g. --size, and the kind of value it accepts,
 * e.g. sizemb_VAL.
 *
 * The val_names[] table describes each option value type.  It is indexed by
 * the value typedef, e.g. sizemb_VAL.  The sizemb_VAL entry specifies the
 * function used to parse the value, e.g. size_mb_arg(), the string used to
 * refer to the value in the command-lines.in specifications, e.g. SizeMB,
 * and how the value should be displayed in a man page, e.g. Size[m|UNIT].
 *
 * A problem is that these tables are independent of a particular command
 * (they are created at build time), but different commands accept different
 * types of values for the same option, e.g. one command will accept
 * signed size values (ssizemb_VAL), while another does not accept a signed
 * number, (sizemb_VAL).  This function deals with this problem by tweaking
 * the opt_names[] table at run time according to the specific command being run.
 * i.e. it changes size_ARG to accept sizemb_VAL or ssizemb_VAL depending
 * on the command.
 *
 * By default, size_ARG in opt_names[] is set up to accept a standard
 * sizemb_VAL.  The same is done for other opt_names[] entries that
 * take different option values.
 *
 * This function overrides default opt_names[] entries at run time according
 * to the command name, adjusting the value types accepted by various options.
 * So, for lvresize, opt_names[sizemb_VAL] is overriden to accept
 * the relative (+ or -) value type ssizemb_VAL, instead of the default
 * sizemb_VAL.  This way, when lvresize processes the --size value, it
 * will use the ssize_mb_arg() function which accepts relative size values.
 * When lvcreate processes the --size value, it uses size_mb_arg() which
 * rejects signed values.
 *
 * The command defs in commands[] do not need to be overriden because
 * the command-lines.in defs have the context of a command, and are
 * described using the proper value type, e.g. this cmd def already
 * uses the relative size value: "lvresize --size SSizeMB LV",
 * so the commands[] entry for the cmd def already references the
 * correct ssizemb_VAL.
 */
void configure_command_option_values(const char *name)
{
	if (!strcmp(name, "lvresize")) {
		/* relative +|- allowed for LV, + allowed for metadata */
		opt_names[size_ARG].val_enum = ssizemb_VAL;
		opt_names[extents_ARG].val_enum = sextents_VAL;
		opt_names[poolmetadatasize_ARG].val_enum = psizemb_VAL;
		return;
	}

	if (!strcmp(name, "lvextend")) {
		/* relative + allowed */
		opt_names[size_ARG].val_enum = psizemb_VAL;
		opt_names[extents_ARG].val_enum = pextents_VAL;
		opt_names[poolmetadatasize_ARG].val_enum = psizemb_VAL;
		return;
	}

	if (!strcmp(name, "lvreduce")) {
		/* relative - allowed */
		opt_names[size_ARG].val_enum = nsizemb_VAL;
		opt_names[extents_ARG].val_enum = nextents_VAL;
		return;
	}

	if (!strcmp(name, "lvconvert")) {
		opt_names[mirrors_ARG].val_enum = snumber_VAL;
		return;
	}

	if (!strcmp(name, "lvcreate")) {
		/*
		 * lvcreate is a bit of a mess because it has previously
		 * accepted + but used it as an absolute value, so we
		 * have to recognize it.  (We don't want to show the +
		 * option in man/help, though, since it's confusing,
		 * so there's a special case when printing man/help
		 * output to show sizemb_VAL/extents_VAL rather than
		 * psizemb_VAL/pextents_VAL.)
		 */
		opt_names[size_ARG].val_enum = psizemb_VAL;
		opt_names[extents_ARG].val_enum = pextents_VAL;
		opt_names[poolmetadatasize_ARG].val_enum = psizemb_VAL;
		opt_names[mirrors_ARG].val_enum = pnumber_VAL;
		return;
	}
}

/* type_LVT to "type" */

static const char *_lvt_enum_to_name(int lvt_enum)
{
	return lv_types[lvt_enum].name;
}

static void _print_usage_description(struct command *cmd)
{
	const char *desc = cmd->desc;
	char buf[MAX_LINE] = {0};
	unsigned di = 0;
	int bi = 0;

	for (di = 0; di < strlen(desc); di++) {
		if (!strncmp(&desc[di], "DESC:", 5)) {
			if (bi) {
				buf[bi] = '\0';
				printf("  %s\n", buf);
				memset(buf, 0, sizeof(buf));
				bi = 0;
			}
			/* skip DESC: */
			di += 5;
			continue;
		}

		if (!bi && desc[di] == ' ')
			continue;

		if (desc[di] != '\\')
			buf[bi++] = desc[di];

		if (bi == (MAX_LINE - 1))
			break;
	}

	if (bi) {
		buf[bi] = '\0';
		printf("  %s\n", buf);
	}
}

static void _print_val_usage(struct command *cmd, int opt_enum, int val_enum)
{
	int is_relative_opt = (opt_enum == size_ARG) ||
			      (opt_enum == extents_ARG) ||
			      (opt_enum == poolmetadatasize_ARG) ||
			      (opt_enum == mirrors_ARG);

	/*
	 * Suppress the [+] prefix for lvcreate which we have to
	 * accept for backwards compat, but don't want to advertise.
	 */
	if (!strcmp(cmd->name, "lvcreate") && is_relative_opt) {
		if (val_enum == psizemb_VAL)
			val_enum = sizemb_VAL;
		else if (val_enum == pextents_VAL)
			val_enum = extents_VAL;
		else if ((val_enum == pnumber_VAL) && (opt_enum == mirrors_ARG))
			val_enum = number_VAL;
	}

	if (!val_names[val_enum].usage)
		printf("%s", val_names[val_enum].name);
	else
		printf("%s", val_names[val_enum].usage);
}

static void _print_usage_def(struct command *cmd, int opt_enum, struct arg_def *def)
{
	int val_enum;
	int lvt_enum;
	int sep = 0;

	for (val_enum = 0; val_enum < VAL_COUNT; val_enum++) {
		if (def->val_bits & val_enum_to_bit(val_enum)) {

			if (val_enum == conststr_VAL)
				printf("%s", def->str);

			else if (val_enum == constnum_VAL)
				printf("%llu", (unsigned long long)def->num);

			else {
				if (sep) printf("|");
				_print_val_usage(cmd, opt_enum, val_enum);
				sep = 1;
			}

			if (val_enum == lv_VAL && def->lvt_bits) {
				for (lvt_enum = 1; lvt_enum < LVT_COUNT; lvt_enum++) {
					if (lvt_bit_is_set(def->lvt_bits, lvt_enum))
						printf("_%s", _lvt_enum_to_name(lvt_enum));
				}
			}

			if ((val_enum == vg_VAL) && (def->flags & ARG_DEF_FLAG_NEW_VG))
				printf("_new");
			if ((val_enum == lv_VAL) && (def->flags & ARG_DEF_FLAG_NEW_LV))
				printf("_new");
		}
	}

	if (def->flags & ARG_DEF_FLAG_MAY_REPEAT)
		printf(" ...");
}

void print_usage(struct command *cmd, int longhelp, int desc_first)
{
	struct command_name *cname = _find_command_name(cmd->name);
	int onereq = (cmd->cmd_flags & CMD_FLAG_ONE_REQUIRED_OPT) ? 1 : 0;
	int include_extents = 0;
	int ro, rp, oo, op, opt_enum, first;

	/*
	 * Looks at all variants of each command name and figures out
	 * which options are common to all variants (for compact output)
	 */
	factor_common_options();

	if (desc_first && cmd->desc)
		_print_usage_description(cmd);

	printf("  %s", cmd->name);

	if (onereq && cmd->ro_count) {
		/* one required option in a set */
		first = 1;

		/* options with short and long */
		for (ro = 0; ro < cmd->ro_count; ro++) {
			opt_enum = cmd->required_opt_args[ro].opt;

			if (!opt_names[opt_enum].short_opt)
				continue;

			if ((opt_enum == size_ARG) && command_has_alternate_extents(cmd->name))
				include_extents = 1;

			if (first)
				printf("\n\t(");
			else
				printf(",\n\t ");
			first = 0;

			printf(" -%c|%s", opt_names[opt_enum].short_opt, opt_names[opt_enum].long_opt);

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->required_opt_args[ro].def);
			}
		}

		/* options with only long */
		for (ro = 0; ro < cmd->ro_count; ro++) {
			opt_enum = cmd->required_opt_args[ro].opt;

			if (opt_names[opt_enum].short_opt)
				continue;

			if ((opt_enum == size_ARG) && command_has_alternate_extents(cmd->name))
				include_extents = 1;

			if (first)
				printf("\n\t(");
			else
				printf(",\n\t ");
			first = 0;

			printf("    %s", opt_names[opt_enum].long_opt);

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->required_opt_args[ro].def);
			}
		}

		printf(" )\n");
	}

	if (!onereq && cmd->ro_count) {
		for (ro = 0; ro < cmd->ro_count; ro++) {
			opt_enum = cmd->required_opt_args[ro].opt;

			if ((opt_enum == size_ARG) && command_has_alternate_extents(cmd->name))
				include_extents = 1;

			if (opt_names[opt_enum].short_opt)
				printf(" -%c|%s", opt_names[opt_enum].short_opt, opt_names[opt_enum].long_opt);
			else
				printf(" %s", opt_names[opt_enum].long_opt);

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->required_opt_args[ro].def);
			}
		}
	}

	if (cmd->rp_count) {
		if (onereq)
			printf("\t");
		for (rp = 0; rp < cmd->rp_count; rp++) {
			if (cmd->required_pos_args[rp].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, 0, &cmd->required_pos_args[rp].def);
			}
		}
	}

	if (!longhelp)
		goto done;

	if (!cmd->oo_count)
		goto op_count;

	if (cmd->oo_count) {
		if (include_extents) {
			printf("\n\t[ -l|--extents ");
			_print_val_usage(cmd, extents_ARG, opt_names[extents_ARG].val_enum);
			printf(" ]");
		}

		/* print optional options with short opts */

		for (oo = 0; oo < cmd->oo_count; oo++) {
			opt_enum = cmd->optional_opt_args[oo].opt;

			if (!opt_names[opt_enum].short_opt)
				continue;

			/*
			 * Skip common lvm options in lvm_all which
			 * are printed at the end under "Common options for lvm"
			 * see print_common_options_lvm()
			 */

			if (_is_lvm_all_opt(opt_enum))
				continue;

			/*
			 * When there is more than one variant,
			 * skip common command options from
			 * cname->common_options (options common
			 * to all variants), which are printed at
			 * the end under "Common options for command"
			 * see print_common_options_cmd()
			 */

			if ((cname->variants > 1) && cname->common_options[opt_enum])
				continue;

			printf("\n\t[");

			printf(" -%c|%s", opt_names[opt_enum].short_opt, opt_names[opt_enum].long_opt);
			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->optional_opt_args[oo].def);
			}

			printf(" ]");
		}

		/* print optional options without short opts */

		for (oo = 0; oo < cmd->oo_count; oo++) {
			opt_enum = cmd->optional_opt_args[oo].opt;

			if (opt_names[opt_enum].short_opt)
				continue;

			/*
			 * Skip common lvm options in lvm_all which
			 * are printed at the end under "Common options for lvm"
			 * see print_common_options_lvm()
			 */

			if (_is_lvm_all_opt(opt_enum))
				continue;

			/*
			 * When there is more than one variant,
			 * skip common command options from
			 * cname->common_options (options common
			 * to all variants), which are printed at
			 * the end under "Common options for command"
			 * see print_common_options_cmd()
			 */

			if ((cname->variants > 1) && cname->common_options[opt_enum])
				continue;

			printf("\n\t[");

			printf("    %s", opt_names[opt_enum].long_opt);
			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->optional_opt_args[oo].def);
			}

			printf(" ]");
		}

		printf("\n\t[ COMMON_OPTIONS ]");
	}

 op_count:
	if (!cmd->op_count)
		goto done;

	printf("\n\t[");

	if (cmd->op_count) {
		for (op = 0; op < cmd->op_count; op++) {
			if (cmd->optional_pos_args[op].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, 0, &cmd->optional_pos_args[op].def);
			}
		}
	}

	printf(" ]");
 done:
	printf("\n");

	if (!desc_first && cmd->desc)
		_print_usage_description(cmd);

	printf("\n");
}

void print_usage_common_lvm(struct command_name *cname, struct command *cmd)
{
	int oo, opt_enum;

	printf("  Common options for lvm:");

	/* print options with short opts */

	for (oo = 0; oo < lvm_all.oo_count; oo++) {
		opt_enum = lvm_all.optional_opt_args[oo].opt;

		if (!opt_names[opt_enum].short_opt)
			continue;

		printf("\n\t[");

		printf(" -%c|%s", opt_names[opt_enum].short_opt, opt_names[opt_enum].long_opt);
		if (lvm_all.optional_opt_args[oo].def.val_bits) {
			printf(" ");
			_print_usage_def(cmd, opt_enum, &lvm_all.optional_opt_args[oo].def);
		}
		printf(" ]");
	}

	/* print options without short opts */

	for (oo = 0; oo < lvm_all.oo_count; oo++) {
		opt_enum = lvm_all.optional_opt_args[oo].opt;

		if (opt_names[opt_enum].short_opt)
			continue;

		printf("\n\t[");

		printf("    %s", opt_names[opt_enum].long_opt);
		if (lvm_all.optional_opt_args[oo].def.val_bits) {
			printf(" ");
			_print_usage_def(cmd, opt_enum, &lvm_all.optional_opt_args[oo].def);
		}
		printf(" ]");
	}

	printf("\n\n");
}

void print_usage_common_cmd(struct command_name *cname, struct command *cmd)
{
	int oo, opt_enum;

	/*
	 * when there's more than one variant, options that
	 * are common to all commands with a common name.
	 */

	if (cname->variants < 2)
		return;

	printf("  Common options for command:");

	/* print options with short opts */

	for (opt_enum = 0; opt_enum < ARG_COUNT; opt_enum++) {
		if (!cname->common_options[opt_enum])
			continue;

		if (_is_lvm_all_opt(opt_enum))
			continue;

		if (!opt_names[opt_enum].short_opt)
			continue;

		printf("\n\t[");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			printf(" -%c|%s", opt_names[opt_enum].short_opt, opt_names[opt_enum].long_opt);
			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->optional_opt_args[oo].def);
			}
			break;
		}
		printf(" ]");
	}

	/* print options without short opts */

	for (opt_enum = 0; opt_enum < ARG_COUNT; opt_enum++) {
		if (!cname->common_options[opt_enum])
			continue;

		if (_is_lvm_all_opt(opt_enum))
			continue;

		if (opt_names[opt_enum].short_opt)
			continue;

		printf("\n\t[");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			printf("    %s", opt_names[opt_enum].long_opt);
			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_usage_def(cmd, opt_enum, &cmd->optional_opt_args[oo].def);
			}
			break;
		}
		printf(" ]");
	}

	printf("\n\n");
}

void print_usage_notes(struct command_name *cname)
{
	if (cname && command_has_alternate_extents(cname->name)) {
		printf("  Special options for command:\n");
		printf("        [ --extents Number[PERCENT] ]\n"
		       "        The --extents option can be used in place of --size.\n"
		       "        The number allows an optional percent suffix.\n");
		printf("\n");
	}

	if (cname && !strcmp(cname->name, "lvcreate")) {
		printf("        [ --name String ]\n"
		       "        The --name option is not required but is typically used.\n"
		       "        When a name is not specified, a new LV name is generated\n"
		       "        with the \"lvol\" prefix and a unique numeric suffix.\n");
		printf("\n");
	}

	printf("  Common variables for lvm:\n"
	       "        Variables in option or position args are capitalized,\n"
	       "        e.g. PV, VG, LV, Size, Number, String, Tag.\n");
	printf("\n");

	printf("        PV\n"
	       "        Physical Volume name, a device path under /dev.\n"
	       "        For commands managing physical extents, a PV positional arg\n"
	       "        generally accepts a suffix indicating a range (or multiple ranges)\n"
	       "        of PEs. When the first PE is omitted, it defaults to the start of\n"
	       "        the device, and when the last PE is omitted it defaults to the end.\n"
	       "        PV[:PE-PE]... is start and end range (inclusive),\n"
	       "        PV[:PE+PE]... is start and length range (counting from 0).\n");
	printf("\n");

	printf("        LV\n"
	       "        Logical Volume name. See lvm(8) for valid names. An LV positional\n"
	       "        arg generally includes the VG name and LV name, e.g. VG/LV.\n"
	       "        LV followed by _<type> indicates that an LV of the given type is\n"
	       "        required. (raid represents raid<N> type).\n"
	       "        The _new suffix indicates that the LV name is new.\n");
	printf("\n");

	printf("        Tag\n"
	       "        Tag name. See lvm(8) for information about tag names and using\n"
	       "        tags in place of a VG, LV or PV.\n");
	printf("\n");

	printf("        Select\n"
	       "        Select indicates that a required positional arg can be omitted\n"
	       "        if the --select option is used. No arg appears in this position.\n");
	printf("\n");

	printf("        Size[UNIT]\n"
	       "        Size is an input number that accepts an optional unit.\n"
               "        Input units are always treated as base two values, regardless of\n"
               "        capitalization, e.g. 'k' and 'K' both refer to 1024.\n"
               "        The default input unit is specified by letter, followed by |UNIT.\n"
               "        UNIT represents other possible input units: BbBsSkKmMgGtTpPeE.\n"
               "        (This should not be confused with the output control --units, where\n"
               "        capital letters mean multiple of 1000.)\n");
	printf("\n");
}

#ifdef MAN_PAGE_GENERATOR

/*
 * FIXME: this just replicates the val usage strings
 * that officially lives in vals.h.  Should there
 * be some programatic way to add man markup to
 * the strings in vals.h without replicating it?
 * Otherwise, this function has to be updated in
 * sync with any string changes in vals.h
 */
static void _print_val_man(struct command_name *cname, int opt_enum, int val_enum)
{
	const char *str;
	char *line;
	char *line_argv[MAX_LINE_ARGC];
	int line_argc;
	int i;
	int is_relative_opt = (opt_enum == size_ARG) ||
			      (opt_enum == extents_ARG) ||
			      (opt_enum == poolmetadatasize_ARG) ||
			      (opt_enum == mirrors_ARG);

	/*
	 * Suppress the [+] prefix for lvcreate which we have to
	 * accept for backwards compat, but don't want to advertise.
	 */
	if (!strcmp(cname->name, "lvcreate") && is_relative_opt) {
		if (val_enum == psizemb_VAL)
			val_enum = sizemb_VAL;
		else if (val_enum == pextents_VAL)
			val_enum = extents_VAL;
		else if ((val_enum == pnumber_VAL) && (opt_enum == mirrors_ARG))
			val_enum = number_VAL;
	}

	if (val_enum == sizemb_VAL) {
		printf("\\fISize\\fP[m|UNIT]");
		return;
	}

	if (val_enum == ssizemb_VAL) {
		printf("[\\fB+\\fP|\\fB-\\fP]\\fISize\\fP[m|UNIT]");
		return;
	}

	if (val_enum == psizemb_VAL) {
		printf("[\\fB+\\fP]\\fISize\\fP[m|UNIT]");
		return;
	}

	if (val_enum == nsizemb_VAL) {
		printf("[\\fB-\\fP]\\fISize\\fP[m|UNIT]");
		return;
	}

	if (val_enum == extents_VAL) {
		printf("\\fINumber\\fP[PERCENT]");
		return;
	}

	if (val_enum == sextents_VAL) {
		printf("[\\fB+\\fP|\\fB-\\fP]\\fINumber\\fP[PERCENT]");
		return;
	}

	if (val_enum == pextents_VAL) {
		printf("[\\fB+\\fP]\\fINumber\\fP[PERCENT]");
		return;
	}

	if (val_enum == nextents_VAL) {
		printf("[\\fB-\\fP]\\fINumber\\fP[PERCENT]");
		return;
	}

	if (val_enum == sizekb_VAL) {
		printf("\\fISize\\fP[k|UNIT]");
		return;
	}

	if (val_enum == ssizekb_VAL) {
		printf("[\\fB+\\fP|\\fB-\\fP]\\fISize\\fP[k|UNIT]");
		return;
	}

	if (val_enum == regionsizemb_VAL) {
		printf("\\fISize\\fP[m|UNIT]");
		return;
	}

	if (val_enum == snumber_VAL) {
		printf("[\\fB+\\fP|\\fB-\\fP]\\fINumber\\fP");
		return;
	}

	if (val_enum == pnumber_VAL) {
		printf("[\\fB+\\fP]\\fINumber\\fP");
		return;
	}

	str = val_names[val_enum].usage;
	if (!str)
		str = val_names[val_enum].name;

	if (!strcmp(str, "PV[:t|n|y]")) {
		printf("\\fIPV\\fP[\\fB:t\\fP|\\fBn\\fP|\\fBy\\fP]");
		return;
	}

	if (!strcmp(str, "Number") ||
	    !strcmp(str, "String") ||
	    !strncmp(str, "VG", 2) ||
	    !strncmp(str, "LV", 2) ||
	    !strncmp(str, "PV", 2) ||
	    !strcmp(str, "Tag")) {
		printf("\\fI%s\\fP", str);
		return;
	}

	if (strchr(str, '|')) {
		line = strdup(str);
		_split_line(line, &line_argc, line_argv, '|');
		for (i = 0; i < line_argc; i++) {
			if (i)
				printf("|");
			if (strstr(line_argv[i], "Number"))
				printf("\\fI%s\\fP", line_argv[i]);
			else
				printf("\\fB%s\\fP", line_argv[i]);
		}
		free(line);
		return;
	}

	printf("\\fB%s\\fP", str);
}

static void _print_def_man(struct command_name *cname, int opt_enum, struct arg_def *def, int usage)
{
	int val_enum;
	int lvt_enum;
	int sep = 0;

	for (val_enum = 0; val_enum < VAL_COUNT; val_enum++) {
		if (def->val_bits & val_enum_to_bit(val_enum)) {

			if (val_enum == conststr_VAL)
				printf("\\fB%s\\fP", def->str);

			else if (val_enum == constnum_VAL)
				printf("\\fB%llu\\fP", (unsigned long long)def->num);

			else {
				if (sep) printf("|");

				if (!usage || !val_names[val_enum].usage)
					printf("\\fI%s\\fP", val_names[val_enum].name);
				else
					_print_val_man(cname, opt_enum, val_enum);

				sep = 1;
			}

			if (val_enum == lv_VAL && def->lvt_bits) {
				printf("\\fI");
				for (lvt_enum = 1; lvt_enum < LVT_COUNT; lvt_enum++) {
					if (lvt_bit_is_set(def->lvt_bits, lvt_enum))
						printf("_%s", _lvt_enum_to_name(lvt_enum));
				}
				printf("\\fP");
			}

			if (((val_enum == vg_VAL) && (def->flags & ARG_DEF_FLAG_NEW_VG)) ||
			    ((val_enum == lv_VAL) && (def->flags & ARG_DEF_FLAG_NEW_LV)))
				printf("\\fI_new\\fP");
		}
	}

	if (def->flags & ARG_DEF_FLAG_MAY_REPEAT)
		printf(" ...");
}

#define	LONG_OPT_NAME_LEN	64
static const char *_man_long_opt_name(const char *cmdname, int opt_enum)
{
	static char long_opt_name[LONG_OPT_NAME_LEN];
	const char *long_opt;

	memset(&long_opt_name, 0, sizeof(long_opt_name));

	switch (opt_enum) {
	case syncaction_ARG:
		long_opt = "--[raid]syncaction";
		break;
	case writemostly_ARG:
		long_opt = "--[raid]writemostly";
		break;
	case minrecoveryrate_ARG:
		long_opt = "--[raid]minrecoveryrate";
		break;
	case maxrecoveryrate_ARG:
		long_opt = "--[raid]maxrecoveryrate";
		break;
	case writebehind_ARG:
		long_opt = "--[raid]writebehind";
		break;
	case vgmetadatacopies_ARG:
		if (!strncmp(cmdname, "vg", 2))
			long_opt = "--[vg]metadatacopies";
		else
			long_opt = "--vgmetadatacopies";
		break;
	case pvmetadatacopies_ARG:
		if (!strncmp(cmdname, "pv", 2))
			long_opt = "--[pv]metadatacopies";
		else
			long_opt = "--pvmetadatacopies";
		break;
	default:
		long_opt = opt_names[opt_enum].long_opt;
		break;
	}

	return long_opt;
}

static void _print_man_usage(char *lvmname, struct command *cmd)
{
	struct command_name *cname;
	int onereq = (cmd->cmd_flags & CMD_FLAG_ONE_REQUIRED_OPT) ? 1 : 0;
	int sep, ro, rp, oo, op, opt_enum;
	int need_ro_indent_end = 0;
	int include_extents = 0;

	if (!(cname = _find_command_name(cmd->name)))
		return;

	printf("\\fB%s\\fP", lvmname);

	if (!onereq)
		goto ro_normal;

	/*
	 * one required option in a set, print as:
	 * ( -a|--a,
	 *   -b|--b,
	 *      --c,
	 *      --d )
	 *
	 * First loop through ro prints those with short opts,
	 * and the second loop prints those without short opts.
	 */

	if (cmd->ro_count) {
		printf("\n");
		printf(".RS 4\n");
		printf("(");

		sep = 0;

		/* print required options with a short opt */
		for (ro = 0; ro < cmd->ro_count; ro++) {
			opt_enum = cmd->required_opt_args[ro].opt;

			if (!opt_names[opt_enum].short_opt)
				continue;

			if (sep) {
				printf(",\n");
				printf(".ad b\n");
				printf(".br\n");
				printf(".ad l\n");
				printf(" ");
			}

			if (opt_names[opt_enum].short_opt) {
				printf(" \\fB-%c\\fP|\\fB%s\\fP",
				       opt_names[opt_enum].short_opt,
				       _man_long_opt_name(cmd->name, opt_enum));
			} else {
				printf("   ");
				printf(" \\fB%s\\fP", _man_long_opt_name(cmd->name, opt_enum));
			}

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->required_opt_args[ro].def, 1);
			}

			sep++;
		}

		/* print required options without a short opt */
		for (ro = 0; ro < cmd->ro_count; ro++) {
			opt_enum = cmd->required_opt_args[ro].opt;

			if (opt_names[opt_enum].short_opt)
				continue;

			if (sep) {
				printf(",\n");
				printf(".ad b\n");
				printf(".br\n");
				printf(".ad l\n");
				printf(" ");
			} else
				printf(".ad l\n");

			printf("   ");
			printf(" \\fB%s\\fP", _man_long_opt_name(cmd->name, opt_enum));

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->required_opt_args[ro].def, 1);
			}

			sep++;
		}

		printf(" )\n");
		printf(".RE\n");
	}

	/* print required position args on a new line after the onereq set */
	if (cmd->rp_count) {
		printf(".RS 4\n");
		for (rp = 0; rp < cmd->rp_count; rp++) {
			if (cmd->required_pos_args[rp].def.val_bits) {
				printf(" ");
				_print_def_man(cname, 0, &cmd->required_pos_args[rp].def, 1);
			}
		}

		printf("\n");
		printf(".RE\n");
	} else {
		/* printf("\n"); */
	}

	printf(".br\n");
	goto oo_count;

 ro_normal:

	/*
	 * all are required options, print as:
	 * -a|--aaa <val> -b|--bbb <val>
	 */

	if (cmd->ro_count) {
		sep = 0;

		for (ro = 0; ro < cmd->ro_count; ro++) {

			/* avoid long line wrapping */
			if ((cmd->ro_count > 2) && (sep == 2)) {
				printf("\n.RS 5\n");
				need_ro_indent_end = 1;
			}

			opt_enum = cmd->required_opt_args[ro].opt;

			if ((opt_enum == size_ARG) && command_has_alternate_extents(cmd->name))
				include_extents = 1;

			if (opt_names[opt_enum].short_opt) {
				printf(" \\fB-%c\\fP|\\fB%s\\fP",
				       opt_names[opt_enum].short_opt,
				       _man_long_opt_name(cmd->name, opt_enum));
			} else
				printf(" \\fB%s\\fP", opt_names[cmd->required_opt_args[ro].opt].long_opt);

			if (cmd->required_opt_args[ro].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->required_opt_args[ro].def, 1);
			}

			sep++;
		}
	}

	/* print required position args on the same line as the required options */
	if (cmd->rp_count) {
		for (rp = 0; rp < cmd->rp_count; rp++) {
			if (cmd->required_pos_args[rp].def.val_bits) {
				printf(" ");
				_print_def_man(cname, 0, &cmd->required_pos_args[rp].def, 1);
			}
		}

		printf("\n");
	} else {
		printf("\n");
	}

	if (need_ro_indent_end)
		printf(".RE\n");

	printf(".br\n");

 oo_count:
	if (!cmd->oo_count)
		goto op_count;

	sep = 0;

	if (cmd->oo_count) {
		printf(".RS 4\n");

		if (include_extents) {
			/*
			 * NB we don't just pass extents_VAL here because the
			 * actual val type for extents_ARG has been adjusted
			 * in opt_names[] according to the command name.
			 */
			printf(".ad l\n");
			printf("[ \\fB-l\\fP|\\fB--extents\\fP ");
			_print_val_man(cname, extents_ARG, opt_names[extents_ARG].val_enum);
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
		}

		/* print optional options with short opts */

		for (oo = 0; oo < cmd->oo_count; oo++) {
			opt_enum = cmd->optional_opt_args[oo].opt;

			if (!opt_names[opt_enum].short_opt)
				continue;

			if (_is_lvm_all_opt(opt_enum))
				continue;

			if ((cname->variants > 1) && cname->common_options[opt_enum])
				continue;

			if (sep)
				printf(".br\n");
			printf(".ad l\n");

			printf("[ \\fB-%c\\fP|\\fB%s\\fP",
				opt_names[opt_enum].short_opt,
				_man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
		}

		/* print optional options without short opts */

		for (oo = 0; oo < cmd->oo_count; oo++) {
			opt_enum = cmd->optional_opt_args[oo].opt;

			if (opt_names[opt_enum].short_opt)
				continue;

			if (_is_lvm_all_opt(opt_enum))
				continue;

			if ((cname->variants > 1) && cname->common_options[opt_enum])
				continue;

			if (sep)
				printf(".br\n");
			printf(".ad l\n");

			/* space alignment without short opt */
			printf("[   ");

			printf(" \\fB%s\\fP", _man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
		}

		if (sep) {
			printf(".br\n");
			/* space alignment without short opt */
			/* printf("   "); */
		}
		printf("[ COMMON_OPTIONS ]\n");
		printf(".RE\n");
		printf(".br\n");
	}

 op_count:
	if (!cmd->op_count)
		return;

	printf(".RS 4\n");
	printf("[");

	if (cmd->op_count) {
		for (op = 0; op < cmd->op_count; op++) {
			if (cmd->optional_pos_args[op].def.val_bits) {
				printf(" ");
				_print_def_man(cname, 0, &cmd->optional_pos_args[op].def, 1);
			}
		}
	}

	printf(" ]\n");
	printf(".RE\n");
}

/*
 * common options listed in the usage section.
 *
 * For commands with only one variant, this is only
 * the options which are common to all lvm commands
 * (in lvm_all, see _is_lvm_all_opt).
 *
 * For commands with more than one variant, this
 * is the set of options common to all variants
 * (in cname->common_options), (which obviously
 * includes the options common to all lvm commands.)
 *
 * List ordering:
 * options with short+long names, alphabetically,
 * then options with only long names, alphabetically
 */

static void _print_man_usage_common_lvm(struct command *cmd)
{
	struct command_name *cname;
	int i, sep, oo, opt_enum;

	if (!(cname = _find_command_name(cmd->name)))
		return;

	printf("Common options for lvm:\n");
	printf(".\n");

	sep = 0;

	printf(".RS 4\n");

	/* print those with short opts */
	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (!opt_names[opt_enum].short_opt)
			continue;

		if (!_is_lvm_all_opt(opt_enum))
			continue;

		if (sep)
			printf(".br\n");
		printf(".ad l\n");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			printf("[ \\fB-%c\\fP|\\fB%s\\fP",
				opt_names[opt_enum].short_opt,
				_man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
			break;
		}

	}

	/* print those without short opts */
	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (opt_names[opt_enum].short_opt)
			continue;

		if (!_is_lvm_all_opt(opt_enum))
			continue;

		if (sep)
			printf(".br\n");
		printf(".ad l\n");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			/* space alignment without short opt */
			printf("[   ");

			printf(" \\fB%s\\fP", _man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
			break;
		}
	}

	printf(".RE\n");
	return;
}

static void _print_man_usage_common_cmd(struct command *cmd)
{
	struct command_name *cname;
	int i, sep, oo, opt_enum;

	if (!(cname = _find_command_name(cmd->name)))
		return;

	if (cname->variants < 2)
		return;

	printf("Common options for command:\n");
	printf(".\n");

	sep = 0;

	printf(".RS 4\n");

	/* print those with short opts */
	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (!cname->common_options[opt_enum])
			continue;

		if (!opt_names[opt_enum].short_opt)
			continue;

		/* common cmd options only used with variants */
		if (cname->variants < 2)
			continue;

		if (_is_lvm_all_opt(opt_enum))
			continue;

		if (sep)
			printf(".br\n");
		printf(".ad l\n");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			printf("[ \\fB-%c\\fP|\\fB%s\\fP",
				opt_names[opt_enum].short_opt,
				_man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
			break;
		}

	}

	/* print those without short opts */
	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (!cname->common_options[opt_enum])
			continue;

		if (opt_names[opt_enum].short_opt)
			continue;

		/* common cmd options only used with variants */
		if (cname->variants < 2)
			continue;

		if (_is_lvm_all_opt(opt_enum))
			continue;

		if (sep)
			printf(".br\n");
		printf(".ad l\n");

		for (oo = 0; oo < cmd->oo_count; oo++) {
			if (cmd->optional_opt_args[oo].opt != opt_enum)
				continue;

			/* space alignment without short opt */
			printf("[   ");

			printf(" \\fB%s\\fP", _man_long_opt_name(cmd->name, opt_enum));

			if (cmd->optional_opt_args[oo].def.val_bits) {
				printf(" ");
				_print_def_man(cname, opt_enum, &cmd->optional_opt_args[oo].def, 1);
			}
			printf(" ]\n");
			printf(".ad b\n");
			sep = 1;
			break;
		}
	}

	printf(".RE\n");
	printf("\n");
	return;
}

/*
 * Format of description, when different command names have
 * different descriptions:
 *
 * "#cmdname1"
 * "text foo goes here"
 * "a second line of text."
 * "#cmdname2"
 * "text bar goes here"
 * "another line of text."
 *
 * When called for cmdname2, this function should just print:
 *
 * "text bar goes here"
 * "another line of text."
 */

static void _print_man_option_desc(struct command_name *cname, int opt_enum)
{
	const char *desc = opt_names[opt_enum].desc;
	char buf[DESC_LINE];
	int started_cname = 0;
	int line_count = 0;
	int bi = 0;
	unsigned di;

	if (desc[0] != '#') {
		printf("%s", desc);
		return;
	}

	for (di = 0; di < strlen(desc); di++) {
		buf[bi++] = desc[di];

		if (bi == DESC_LINE) {
			log_error("Parsing command defs: print_man_option_desc line too long.");
			exit(EXIT_FAILURE);
		}

		if (buf[bi-1] != '\n')
			continue;

		if (buf[0] != '#') {
			if (started_cname) {
				printf("%s", buf);
				line_count++;
			}

			memset(buf, 0, sizeof(buf));
			bi = 0;
			continue;
		}

		/* Line starting with #cmdname */

		/*
		 * Must be starting a new command name.
		 * If no lines have been printed, multiple command names
		 * are using the same text. If lines have been printed,
		 * then the start of a new command name means the end
		 * of text for the current command name.
		 */
		if (line_count && started_cname)
			return;

		if (!strncmp(buf + 1, cname->name, strlen(cname->name))) {
			/* The start of our command name. */
			started_cname = 1;
			memset(buf, 0, sizeof(buf));
			bi = 0;
		} else {
			/* The start of another command name. */
			memset(buf, 0, sizeof(buf));
			bi = 0;
		}
	}

	if (bi && started_cname)
		printf("%s", buf);
}

/*
 * Print a list of all options names for a given command name.
 */

static void _print_man_all_options_list(struct command_name *cname)
{
	int opt_enum, val_enum;
	int sep = 0;
	int i;

	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (!cname->all_options[opt_enum])
			continue;

		if (sep)
			printf(".br\n");
		printf(".ad l\n");

		if (opt_names[opt_enum].short_opt) {
			printf(" \\fB-%c\\fP|\\fB%s\\fP",
				opt_names[opt_enum].short_opt,
				_man_long_opt_name(cname->name, opt_enum));
		} else {
			/* spaces for alignment without short opt */
			printf("    \\fB%s\\fP", _man_long_opt_name(cname->name, opt_enum));
		}

		val_enum = opt_names[opt_enum].val_enum;

		if (!val_names[val_enum].fn) {
			/* takes no arg */
		} else if (!val_names[val_enum].usage) {
			printf(" ");
			printf("\\fI");
			printf("%s", val_names[val_enum].name);
			printf("\\fP");
		} else {
			printf(" ");
			_print_val_man(cname, opt_enum, val_enum);
		}

		printf("\n.ad b\n");

		sep = 1;
	}
}

/*
 * All options used for a given command name, along with descriptions.
 */

static void _print_man_all_options_desc(struct command_name *cname)
{
	int opt_enum, val_enum;
	int i;

	for (i = 0; i < ARG_COUNT; i++) {
		opt_enum = opt_names_alpha[i]->opt_enum;

		if (!cname->all_options[opt_enum])
			continue;

		printf(".HP\n");

		printf(".ad l\n");

		if (opt_names[opt_enum].short_opt) {
			printf("\\fB-%c\\fP|\\fB%s\\fP",
			       opt_names[opt_enum].short_opt,
			       _man_long_opt_name(cname->name, opt_enum));
		} else {
			printf("\\fB%s\\fP", _man_long_opt_name(cname->name, opt_enum));
		}

		val_enum = opt_names[opt_enum].val_enum;

		if (!val_names[val_enum].fn) {
			/* takes no arg */
		} else if (!val_names[val_enum].usage) {
			printf(" ");
			printf("\\fI");
			printf("%s", val_names[val_enum].name);
			printf("\\fP");
		} else {
			printf(" ");
			_print_val_man(cname, opt_enum, val_enum);
		}

		if (opt_names[opt_enum].flags & ARG_COUNTABLE)
			printf(" ...");

		if (opt_names[opt_enum].desc) {
			printf("\n");
			printf(".br\n");
			_print_man_option_desc(cname, opt_enum);
		}

		printf(".ad b\n");
	}
}

static void _print_man_all_positions_desc(struct command_name *cname)
{
	struct command *cmd;
	int ci, rp, op;
	int has_vg_val = 0;
	int has_lv_val = 0;
	int has_pv_val = 0;
	int has_tag_val = 0;
	int has_select_val = 0;
	int has_lv_type = 0;

	for (ci = 0; ci < COMMAND_COUNT; ci++) {
		cmd = &commands[ci];

		if (strcmp(cmd->name, cname->name))
			continue;

		for (rp = 0; rp < cmd->rp_count; rp++) {
			if (cmd->required_pos_args[rp].def.val_bits & val_enum_to_bit(vg_VAL))
				has_vg_val = 1;

			if (cmd->required_pos_args[rp].def.val_bits & val_enum_to_bit(lv_VAL)) {
				has_lv_val = 1;
				if (cmd->required_pos_args[rp].def.lvt_bits)
					has_lv_type = 1;
			}

			if (cmd->required_pos_args[rp].def.val_bits & val_enum_to_bit(pv_VAL))
				has_pv_val = 1;

			if (cmd->required_pos_args[rp].def.val_bits & val_enum_to_bit(tag_VAL))
				has_tag_val = 1;

			if (cmd->required_pos_args[rp].def.val_bits & val_enum_to_bit(select_VAL))
				has_select_val = 1;
		}

		for (op = 0; op < cmd->op_count; op++) {
			if (cmd->optional_pos_args[op].def.val_bits & val_enum_to_bit(vg_VAL))
				has_vg_val = 1;

			if (cmd->optional_pos_args[op].def.val_bits & val_enum_to_bit(lv_VAL)) {
				has_lv_val = 1;
				if (cmd->optional_pos_args[op].def.lvt_bits)
					has_lv_type = 1;
			}

			if (cmd->optional_pos_args[op].def.val_bits & val_enum_to_bit(pv_VAL))
				has_pv_val = 1;

			if (cmd->optional_pos_args[op].def.val_bits & val_enum_to_bit(tag_VAL))
				has_tag_val = 1;

			if (cmd->optional_pos_args[op].def.val_bits & val_enum_to_bit(select_VAL))
				has_select_val = 1;
		}
	}

	if (has_vg_val) {
		printf(".HP\n");

		printf("\\fI%s\\fP", val_names[vg_VAL].name);
		printf("\n");
		printf(".br\n");
		printf("Volume Group name.  See \\fBlvm\\fP(8) for valid names.\n");

		if (!strcmp(cname->name, "lvcreate"))
			printf("For lvcreate, the required VG positional arg may be\n"
			       "omitted when the VG name is included in another option,\n"
			       "e.g. --name VG/LV.\n");
	}

	if (has_lv_val) {
		printf(".HP\n");

		printf("\\fI%s\\fP", val_names[lv_VAL].name);
		printf("\n");
		printf(".br\n");
		printf("Logical Volume name.  See \\fBlvm\\fP(8) for valid names.\n"
		       "An LV positional arg generally includes the VG name and LV name, e.g. VG/LV.\n");

		if (has_lv_type)
			printf("LV followed by _<type> indicates that an LV of the\n"
			       "given type is required. (raid represents raid<N> type)\n");
	}

	if (has_pv_val) {
		printf(".HP\n");

		printf("\\fI%s\\fP", val_names[pv_VAL].name);
		printf("\n");
		printf(".br\n");
		printf("Physical Volume name, a device path under /dev.\n"
		       "For commands managing physical extents, a PV positional arg\n"
		       "generally accepts a suffix indicating a range (or multiple ranges)\n"
		       "of physical extents (PEs). When the first PE is omitted, it defaults\n"
		       "to the start of the device, and when the last PE is omitted it defaults to end.\n"
		       "Start and end range (inclusive): \\fIPV\\fP[\\fB:\\fP\\fIPE\\fP\\fB-\\fP\\fIPE\\fP]...\n"
		       "Start and length range (counting from 0): \\fIPV\\fP[\\fB:\\fP\\fIPE\\fP\\fB+\\fP\\fIPE\\fP]...\n");
	}

	if (has_tag_val) {
		printf(".HP\n");

		printf("\\fI%s\\fP", val_names[tag_VAL].name);
		printf("\n");
		printf(".br\n");
		printf("Tag name.  See \\fBlvm\\fP(8) for information about tag names and using tags\n"
		       "in place of a VG, LV or PV.\n");
	}

	if (has_select_val) {
		printf(".HP\n");

		printf("\\fI%s\\fP", val_names[select_VAL].name);
		printf("\n");
		printf(".br\n");
		printf("Select indicates that a required positional parameter can\n"
		       "be omitted if the \\fB--select\\fP option is used.\n"
		       "No arg appears in this position.\n");
	}

	/* Every command uses a string arg somewhere. */

	printf(".HP\n");
	printf("\\fI%s\\fP", val_names[string_VAL].name);
	printf("\n");
	printf(".br\n");
	printf("See the option description for information about the string content.\n");

	/*
	 * We could possibly check if the command accepts any option that
	 * uses Size, and only print this in those cases, but this seems
	 * so common that we should probably always print it.
	 */

	printf(".HP\n");
	printf("\\fISize\\fP[UNIT]");
	printf("\n");
	printf(".br\n");
	printf("Size is an input number that accepts an optional unit.\n"
	       "Input units are always treated as base two values, regardless of\n"
	       "capitalization, e.g. 'k' and 'K' both refer to 1024.\n"
	       "The default input unit is specified by letter, followed by |UNIT.\n"
	       "UNIT represents other possible input units: \\fBbBsSkKmMgGtTpPeE\\fP.\n"
	       "b|B is bytes, s|S is sectors of 512 bytes, k|K is kilobytes,\n"
	       "m|M is megabytes, g|G is gigabytes, t|T is terabytes,\n"
	       "p|P is petabytes, e|E is exabytes.\n"
	       "(This should not be confused with the output control --units, where\n"
	       "capital letters mean multiple of 1000.)\n");

	printf(".SH ENVIRONMENT VARIABLES\n");
	printf("See \\fBlvm\\fP(8) for information about environment variables used by lvm.\n"
	       "For example, LVM_VG_NAME can generally be substituted for a required VG parameter.\n");
}

static void _print_desc_man(const char *desc)
{
	char buf[DESC_LINE] = {0};
	unsigned di;
	int bi = 0;

	for (di = 0; di < strlen(desc); di++) {
		if (desc[di] == '\0')
			break;
		if (desc[di] == '\n')
			continue;

		if (!strncmp(&desc[di], "DESC:", 5)) {
			if (bi) {
				printf("%s\n", buf);
				printf(".br\n");
				memset(buf, 0, sizeof(buf));
				bi = 0;
			}
			di += 5;
			continue;
		}

		if (!bi && desc[di] == ' ')
			continue;

		buf[bi++] = desc[di];

		if (bi == (DESC_LINE - 1))
			break;
	}

	if (bi) {
		printf("%s\n", buf);
		printf(".br\n");
	}
}

static const char *_upper_command_name(char *str)
{
	static char str_upper[32];
	int i = 0;

	while (*str) {
		str_upper[i++] = toupper(*str);
		str++;
	}
	str_upper[i] = '\0';
	return str_upper;
}

#define MAX_MAN_DESC (1024 * 1024)

static int _include_description_file(char *name, char *des_file)
{
	char *buf;
	int fd, r = 0;
	ssize_t sz;
	struct stat statbuf;

	if ((fd = open(des_file, O_RDONLY)) < 0) {
		log_error("Failed to open description file %s.", des_file);
		return 0;
	}

	if (fstat(fd, &statbuf) < 0) {
		log_error("Failed to stat description file %s.", des_file);
		goto out_close;
	}

	if (statbuf.st_size > MAX_MAN_DESC) {
		log_error("Description file %s is too large.", des_file);
		goto out_close;
	}

	if (!(buf = malloc(statbuf.st_size + 1))) {
		log_error("Failed to allocate buffer for description file %s.", des_file);
		goto out_close;
	}

	if ((sz = read(fd, buf, statbuf.st_size)) < 0) {
		log_error("Failed to read description file %s.", des_file);
		goto out_free;
	}

	buf[sz] = '\0';
	printf(".SH DESCRIPTION\n%s", buf);
	r = 1;

out_free:
	free(buf);
out_close:
	(void) close(fd);

	return r;
}

static int _print_man(char *name, char *des_file, int secondary)
{
	struct command_name *cname;
	struct command *cmd, *prev_cmd = NULL;
	char *lvmname = name;
	int i;

	if (!strncmp(name, "lvm-", 4)) {
		name[3] = ' ';
		name += 4;
	}

	cname = _find_command_name(name);

	printf(".TH %s 8 \"LVM TOOLS #VERSION#\" \"Red Hat, Inc.\"\n",
		_upper_command_name(lvmname));

	for (i = 0; i < COMMAND_COUNT; i++) {

		cmd = &commands[i];

		if (prev_cmd && strcmp(prev_cmd->name, cmd->name)) {
			_print_man_usage_common_cmd(prev_cmd);
			_print_man_usage_common_lvm(prev_cmd);

			printf(".SH OPTIONS\n");
			_print_man_all_options_desc(cname);
			printf(".SH VARIABLES\n");
			_print_man_all_positions_desc(cname);

			prev_cmd = NULL;
		}

		if (cmd->cmd_flags & CMD_FLAG_PREVIOUS_SYNTAX)
			continue;

		if ((cmd->cmd_flags & CMD_FLAG_SECONDARY_SYNTAX) && !secondary)
			continue;

		if (strcmp(name, cmd->name))
			continue;

		if (!prev_cmd || strcmp(prev_cmd->name, cmd->name)) {
			printf(".SH NAME\n");
			if (cname->desc)
				printf("%s - %s\n", lvmname, cname->desc);
			else
				printf("%s\n", lvmname);

			printf(".\n");
			printf(".SH SYNOPSIS\n");
			prev_cmd = cmd;

			if (!(cname = _find_command_name(cmd->name)))
				return 0;

			if (cname->variant_has_ro && cname->variant_has_rp)
				printf("\\fB%s\\fP \\fIoption_args\\fP \\fIposition_args\\fP\n", lvmname);
			else if (cname->variant_has_ro && !cname->variant_has_rp)
				printf("\\fB%s\\fP \\fIoption_args\\fP\n", lvmname);
			else if (!cname->variant_has_ro && cname->variant_has_rp)
				printf("\\fB%s\\fP \\fIposition_args\\fP\n", lvmname);
			else if (!cname->variant_has_ro && !cname->variant_has_rp)
				printf("\\fB%s\\fP\n", lvmname);

			printf(".br\n");

			if (cname->variant_has_oo) {
				printf("    [ \\fIoption_args\\fP ]\n");
				printf(".br\n");
			}

			if (cname->variant_has_op) {
				printf("    [ \\fIposition_args\\fP ]\n");
				printf(".br\n");
			}

			/* listing them all when there's only 1 or 2 is just repetative */
			if (cname->variants > 2) {
				printf(".P\n");
				_print_man_all_options_list(cname);
			}

			if (des_file && !_include_description_file(lvmname, des_file))
				return 0;

			printf(".SH USAGE\n");
		}

		if (cmd->desc) {
			_print_desc_man(cmd->desc);
			printf(".P\n");
		}

		_print_man_usage(lvmname, cmd);

		if (i == (COMMAND_COUNT - 1)) {
			_print_man_usage_common_cmd(cmd);
			_print_man_usage_common_lvm(cmd);

			printf("\n");
			printf(".SH OPTIONS\n");
			_print_man_all_options_desc(cname);
			printf(".SH VARIABLES\n");
			_print_man_all_positions_desc(cname);
		} else {
			if (cname->variants > 2)
				printf("-\n");
		}

		printf("\n");
		continue;
	}

	return 1;
}

static void _print_man_secondary(char *name)
{
	struct command *cmd;
	char *lvmname = name;
	int header = 0;
	int i;

	if (!strncmp(name, "lvm-", 4))
		name += 4;

	for (i = 0; i < COMMAND_COUNT; i++) {

		cmd = &commands[i];

		if (cmd->cmd_flags & CMD_FLAG_PREVIOUS_SYNTAX)
			continue;

		if (!(cmd->cmd_flags & CMD_FLAG_SECONDARY_SYNTAX))
			continue;

		if (strcmp(name, cmd->name))
			continue;

		if (!header) {
			printf(".SH ADVANCED USAGE\n");
			printf("Alternate command forms, advanced command usage, and listing of all valid syntax for completeness.\n");
			printf(".P\n");
			header = 1;
		}

		if (cmd->desc) {
			_print_desc_man(cmd->desc);
			printf(".P\n");
		}

		_print_man_usage(lvmname, cmd);

		printf("-\n");
		printf("\n");
	}
}

#define	STDOUT_BUF_SIZE	 (MAX_MAN_DESC + 4 * 1024)

int main(int argc, char *argv[])
{
	struct cmd_context cmdtool;
	char *cmdname = NULL;
	char *desfile = NULL;
	char *stdout_buf;
	int primary = 0;
	int secondary = 0;
	int r = 0;
	size_t sz = STDOUT_BUF_SIZE;

	static struct option long_options[] = {
		{"primary", no_argument, 0, 'p' },
		{"secondary", no_argument, 0, 's' },
		{0, 0, 0, 0 }
	};

	memset(&commands, 0, sizeof(commands));

	if (!(stdout_buf = malloc(sz)))
		log_error("Failed to allocate stdout buffer; carrying on with default buffering.");
	else
		setbuffer(stdout, stdout_buf, sz);

	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long(argc, argv, "ps", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case '0':
			break;
		case 'p':
			primary = 1;
			break;
		case 's':
			secondary = 1;
			break;
		}
	}

	if (!primary && !secondary) {
		log_error("Usage: %s --primary|--secondary <command> [/path/to/description-file].", argv[0]);
		goto out_free;
	}

	if (optind < argc)
		cmdname = strdup(argv[optind++]);
	else {
		log_error("Missing command name.");
		goto out_free;
	}

	if (optind < argc)
		desfile = argv[optind++];

	define_commands(&cmdtool, NULL);

	configure_command_option_values(cmdname);

	factor_common_options();

	if (primary)
		r = _print_man(cmdname, desfile, secondary);
	else if (secondary) {
		r = 1;
		_print_man_secondary(cmdname);
	}

out_free:
	if (stdout_buf) {
		fflush(stdout);
		setlinebuf(stdout);
		free(stdout_buf);
	}

	exit(r ? EXIT_SUCCESS: EXIT_FAILURE);
}

#endif
