/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "lib/metadata/metadata.h"
#include "lib/report/report.h"
#include "lib/commands/toolcontext.h"
#include "lib/misc/lvm-string.h"
#include "lib/display/display.h"
#include "lib/activate/activate.h"
#include "lib/metadata/segtype.h"
#include "lib/cache/lvmcache.h"
#include "lib/device/device-types.h"
#include "lib/datastruct/str_list.h"

#include <stddef.h> /* offsetof() */
#include <float.h> /* DBL_MAX */
#include <time.h>

struct lvm_report_object {
	struct volume_group *vg;
	struct lv_with_info_and_seg_status *lvdm;
	struct physical_volume *pv;
	struct lv_segment *seg;
	struct pv_segment *pvseg;
	struct label *label;
};

static uint32_t _log_seqnum = 1;

/*
 *  Enum for field_num index to use in per-field reserved value definition.
 *  Each field is represented by enum value with name "field_<id>" where <id>
 *  is the field_id of the field as registered in columns.h.
 */
#define FIELD(type, strct, sorttype, head, field_name, width, func, id, desc, writeable) field_ ## id,
enum {
#include "columns.h"
};
#undef FIELD

static const uint64_t _zero64 = UINT64_C(0);
static const uint64_t _one64 = UINT64_C(1);
static const uint64_t _two64 = UINT64_C(2);
static const char _str_zero[] = "0";
static const char _str_one[] = "1";
static const char _str_no[] = "no";
static const char _str_yes[] = "yes";
static const char _str_unknown[] = "unknown";
static const double _siz_max = DBL_MAX;

/*
 * 32 bit signed is casted to 64 bit unsigned in dm_report_field internally!
 * So when stored in the struct, the _reserved_num_undef_32 is actually
 * equal to _reserved_num_undef_64.
 */
static const int32_t _reserved_num_undef_32 = INT32_C(-1);

typedef enum {
	/* top-level identification */
	TIME_NULL,
	TIME_NUM,
	TIME_STR,

	/* direct numeric value */
	TIME_NUM__START,
	TIME_NUM_MULTIPLIER,
	TIME_NUM_MULTIPLIER_NEGATIVE,
	TIME_NUM_DAY,
	TIME_NUM_YEAR,
	TIME_NUM__END,

	/* direct string value */
	TIME_STR_TIMEZONE,

	/* time frame strings */
	TIME_FRAME__START,
	TIME_FRAME_AGO,
	TIME_FRAME__END,

	/* labels for dates */
	TIME_LABEL_DATE__START,

	TIME_LABEL_DATE_TODAY,
	TIME_LABEL_DATE_YESTERDAY,

	/* weekday name strings */
	TIME_WEEKDAY__START,
	TIME_WEEKDAY_SUNDAY,
	TIME_WEEKDAY_MONDAY,
	TIME_WEEKDAY_TUESDAY,
	TIME_WEEKDAY_WEDNESDAY,
	TIME_WEEKDAY_THURSDAY,
	TIME_WEEKDAY_FRIDAY,
	TIME_WEEKDAY_SATURDAY,
	TIME_WEEKDAY__END,

	TIME_LABEL_DATE__END,

	/* labels for times */
	TIME_LABEL_TIME__START,
	TIME_LABEL_TIME_NOON,
	TIME_LABEL_TIME_MIDNIGHT,
	TIME_LABEL_TIME__END,

	/* time unit strings */
	TIME_UNIT__START,
	TIME_UNIT_SECOND,
	TIME_UNIT_SECOND_REL,
	TIME_UNIT_MINUTE,
	TIME_UNIT_MINUTE_REL,
	TIME_UNIT_HOUR,
	TIME_UNIT_HOUR_REL,
	TIME_UNIT_AM,
	TIME_UNIT_PM,
	TIME_UNIT_DAY,
	TIME_UNIT_WEEK,
	TIME_UNIT_MONTH,
	TIME_UNIT_YEAR,
	TIME_UNIT_TZ_MINUTE,
	TIME_UNIT_TZ_HOUR,
	TIME_UNIT__END,

	/* month name strings */
	TIME_MONTH__START,
	TIME_MONTH_JANUARY,
	TIME_MONTH_FEBRUARY,
	TIME_MONTH_MARCH,
	TIME_MONTH_APRIL,
	TIME_MONTH_MAY,
	TIME_MONTH_JUNE,
	TIME_MONTH_JULY,
	TIME_MONTH_AUGUST,
	TIME_MONTH_SEPTEMBER,
	TIME_MONTH_OCTOBER,
	TIME_MONTH_NOVEMBER,
	TIME_MONTH_DECEMBER,
	TIME_MONTH__END,
} time_id_t;

#define TIME_PROP_DATE 0x00000001 /* date-related */
#define TIME_PROP_TIME 0x00000002 /* time-related */
#define TIME_PROP_ABS  0x00000004 /* absolute value */
#define TIME_PROP_REL  0x00000008 /* relative value */

struct time_prop {
	time_id_t id;
	uint32_t prop_flags;
	time_id_t granularity;
};

#define ADD_TIME_PROP(id, flags, granularity) [(id)] = {(id), (flags), (granularity)},

static const struct time_prop _time_props[] = {
	ADD_TIME_PROP(TIME_NULL,                    0,							TIME_NULL)
	ADD_TIME_PROP(TIME_NUM,                     0,							TIME_NULL)
	ADD_TIME_PROP(TIME_STR,                     0,							TIME_NULL)

	ADD_TIME_PROP(TIME_NUM_MULTIPLIER,          0,							TIME_NULL)
	ADD_TIME_PROP(TIME_NUM_MULTIPLIER_NEGATIVE, 0,							TIME_NULL)
	ADD_TIME_PROP(TIME_NUM_DAY,                 TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_NUM_YEAR,                TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_YEAR)

	ADD_TIME_PROP(TIME_STR_TIMEZONE,            TIME_PROP_TIME | TIME_PROP_ABS,			TIME_NULL)

	ADD_TIME_PROP(TIME_FRAME_AGO,               TIME_PROP_DATE | TIME_PROP_TIME | TIME_PROP_REL,	TIME_NULL)

	ADD_TIME_PROP(TIME_LABEL_DATE_TODAY,        TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_LABEL_DATE_YESTERDAY,    TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_SUNDAY,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_MONDAY,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_TUESDAY,         TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_WEDNESDAY,       TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_THURSDAY,        TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_FRIDAY,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_WEEKDAY_SATURDAY,        TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_DAY)

	ADD_TIME_PROP(TIME_LABEL_TIME_NOON,         TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_SECOND)
	ADD_TIME_PROP(TIME_LABEL_TIME_MIDNIGHT,     TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_SECOND)

	ADD_TIME_PROP(TIME_UNIT_SECOND,             TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_SECOND)
	ADD_TIME_PROP(TIME_UNIT_SECOND_REL,         TIME_PROP_TIME | TIME_PROP_REL,			TIME_UNIT_SECOND)
	ADD_TIME_PROP(TIME_UNIT_MINUTE,             TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_MINUTE)
	ADD_TIME_PROP(TIME_UNIT_MINUTE_REL,         TIME_PROP_TIME | TIME_PROP_REL,			TIME_UNIT_MINUTE)
	ADD_TIME_PROP(TIME_UNIT_HOUR,               TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_HOUR)
	ADD_TIME_PROP(TIME_UNIT_HOUR_REL,           TIME_PROP_TIME | TIME_PROP_REL,			TIME_UNIT_HOUR)
	ADD_TIME_PROP(TIME_UNIT_AM,                 TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_HOUR)
	ADD_TIME_PROP(TIME_UNIT_PM,                 TIME_PROP_TIME | TIME_PROP_ABS,			TIME_UNIT_HOUR)
	ADD_TIME_PROP(TIME_UNIT_DAY,                TIME_PROP_DATE | TIME_PROP_REL,			TIME_UNIT_DAY)
	ADD_TIME_PROP(TIME_UNIT_WEEK,               TIME_PROP_DATE | TIME_PROP_REL,			TIME_UNIT_WEEK)
	ADD_TIME_PROP(TIME_UNIT_MONTH,              TIME_PROP_DATE | TIME_PROP_REL,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_UNIT_YEAR,               TIME_PROP_DATE | TIME_PROP_REL,			TIME_UNIT_YEAR)
	ADD_TIME_PROP(TIME_UNIT_TZ_MINUTE,          TIME_PROP_TIME | TIME_PROP_ABS,			TIME_NULL)
	ADD_TIME_PROP(TIME_UNIT_TZ_HOUR,            TIME_PROP_TIME | TIME_PROP_ABS,			TIME_NULL)

	ADD_TIME_PROP(TIME_MONTH_JANUARY,           TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_FEBRUARY,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_MARCH,             TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_APRIL,             TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_MAY,               TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_JUNE,              TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_JULY,              TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_AUGUST,            TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_SEPTEMBER,         TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_OCTOBER,           TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_NOVEMBER,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
	ADD_TIME_PROP(TIME_MONTH_DECEMBER,          TIME_PROP_DATE | TIME_PROP_ABS,			TIME_UNIT_MONTH)
};

#define TIME_REG_PLURAL_S  0x00000001 /* also recognize plural form with "s" suffix */

struct time_reg {
	const char *name;
	const struct time_prop *prop;
	uint32_t reg_flags;
};

#define TIME_PROP(id) (_time_props + (id))

static const struct time_reg _time_reg[] = {
	/*
	 * Group of tokens representing time frame and used
	 * with relative date/time to specify different flavours
	 * of relativity.
	 */
	{"ago",       TIME_PROP(TIME_FRAME_AGO),            0},

	/*
	 * Group of tokens labeling some date and used
	 * instead of direct absolute specification.
	 */
	{"today",     TIME_PROP(TIME_LABEL_DATE_TODAY),     0}, /* 0:00 - 23:59:59 for current date */
	{"yesterday", TIME_PROP(TIME_LABEL_DATE_YESTERDAY), 0}, /* 0:00 - 23:59:59 for current date minus 1 day*/

	/*
	 * Group of tokens labeling some date - weekday
	 * names used to build up date.
	 */
	{"Sunday",    TIME_PROP(TIME_WEEKDAY_SUNDAY),       TIME_REG_PLURAL_S},
	{"Sun",       TIME_PROP(TIME_WEEKDAY_SUNDAY),       0},
	{"Monday",    TIME_PROP(TIME_WEEKDAY_MONDAY),       TIME_REG_PLURAL_S},
	{"Mon",       TIME_PROP(TIME_WEEKDAY_MONDAY),       0},
	{"Tuesday",   TIME_PROP(TIME_WEEKDAY_TUESDAY),      TIME_REG_PLURAL_S},
	{"Tue",       TIME_PROP(TIME_WEEKDAY_TUESDAY),      0},
	{"Wednesday", TIME_PROP(TIME_WEEKDAY_WEDNESDAY),    TIME_REG_PLURAL_S},
	{"Wed",       TIME_PROP(TIME_WEEKDAY_WEDNESDAY),    0},
	{"Thursday",  TIME_PROP(TIME_WEEKDAY_THURSDAY),     TIME_REG_PLURAL_S},
	{"Thu",       TIME_PROP(TIME_WEEKDAY_THURSDAY),     0},
	{"Friday",    TIME_PROP(TIME_WEEKDAY_FRIDAY),       TIME_REG_PLURAL_S},
	{"Fri",       TIME_PROP(TIME_WEEKDAY_FRIDAY),       0},
	{"Saturday",  TIME_PROP(TIME_WEEKDAY_SATURDAY),     TIME_REG_PLURAL_S},
	{"Sat",       TIME_PROP(TIME_WEEKDAY_SATURDAY),     0},

	/*
	 * Group of tokens labeling some time and used
	 * instead of direct absolute specification.
	 */
	{"noon",      TIME_PROP(TIME_LABEL_TIME_NOON),      TIME_REG_PLURAL_S}, /* 12:00:00 */
	{"midnight",  TIME_PROP(TIME_LABEL_TIME_MIDNIGHT),  TIME_REG_PLURAL_S}, /* 00:00:00 */

	/*
	 * Group of tokens used to build up time. Most of these
	 * are used either as relative or absolute time units.
	 * The absolute ones are always used with TIME_FRAME_*
	 * token, otherwise the unit is relative.
	 */
	{"second",    TIME_PROP(TIME_UNIT_SECOND),          TIME_REG_PLURAL_S},
	{"sec",       TIME_PROP(TIME_UNIT_SECOND),          TIME_REG_PLURAL_S},
	{"s",         TIME_PROP(TIME_UNIT_SECOND),          0},
	{"minute",    TIME_PROP(TIME_UNIT_MINUTE),          TIME_REG_PLURAL_S},
	{"min",       TIME_PROP(TIME_UNIT_MINUTE),          TIME_REG_PLURAL_S},
	{"m",         TIME_PROP(TIME_UNIT_MINUTE),          0},
	{"hour",      TIME_PROP(TIME_UNIT_HOUR),            TIME_REG_PLURAL_S},
	{"hr",        TIME_PROP(TIME_UNIT_HOUR),            TIME_REG_PLURAL_S},
	{"h",         TIME_PROP(TIME_UNIT_HOUR),            0},
	{"AM",        TIME_PROP(TIME_UNIT_AM),              0},
	{"PM",        TIME_PROP(TIME_UNIT_PM),              0},

	/*
	 * Group of tokens used to build up date.
	 * These are all relative ones.
	 */
	{"day",       TIME_PROP(TIME_UNIT_DAY),             TIME_REG_PLURAL_S},
	{"week",      TIME_PROP(TIME_UNIT_WEEK),            TIME_REG_PLURAL_S},
	{"month",     TIME_PROP(TIME_UNIT_MONTH),           TIME_REG_PLURAL_S},
	{"year",      TIME_PROP(TIME_UNIT_YEAR),            TIME_REG_PLURAL_S},
	{"yr",        TIME_PROP(TIME_UNIT_YEAR),            TIME_REG_PLURAL_S},

	/*
	 * Group of tokes used to build up date.
	 * These are all absolute.
	 */
	{"January",   TIME_PROP(TIME_MONTH_JANUARY),        0},
	{"Jan",       TIME_PROP(TIME_MONTH_JANUARY),        0},
	{"February",  TIME_PROP(TIME_MONTH_FEBRUARY),       0},
	{"Feb",       TIME_PROP(TIME_MONTH_FEBRUARY),       0},
	{"March",     TIME_PROP(TIME_MONTH_MARCH),          0},
	{"Mar",       TIME_PROP(TIME_MONTH_MARCH),          0},
	{"April",     TIME_PROP(TIME_MONTH_APRIL),          0},
	{"Apr",       TIME_PROP(TIME_MONTH_APRIL),          0},
	{"May",       TIME_PROP(TIME_MONTH_MAY),            0},
	{"June",      TIME_PROP(TIME_MONTH_JUNE),           0},
	{"Jun",       TIME_PROP(TIME_MONTH_JUNE),           0},
	{"July",      TIME_PROP(TIME_MONTH_JULY),           0},
	{"Jul",       TIME_PROP(TIME_MONTH_JULY),           0},
	{"August",    TIME_PROP(TIME_MONTH_AUGUST),         0},
	{"Aug",       TIME_PROP(TIME_MONTH_AUGUST),         0},
	{"September", TIME_PROP(TIME_MONTH_SEPTEMBER),      0},
	{"Sep",       TIME_PROP(TIME_MONTH_SEPTEMBER),      0},
	{"October",   TIME_PROP(TIME_MONTH_OCTOBER),        0},
	{"Oct",       TIME_PROP(TIME_MONTH_OCTOBER),        0},
	{"November",  TIME_PROP(TIME_MONTH_NOVEMBER),       0},
	{"Nov",       TIME_PROP(TIME_MONTH_NOVEMBER),       0},
	{"December",  TIME_PROP(TIME_MONTH_DECEMBER),       0},
	{"Dec",       TIME_PROP(TIME_MONTH_DECEMBER),       0},
	{NULL,        TIME_PROP(TIME_NULL),                 0},
};

struct time_item {
	struct dm_list list;
	const struct time_prop *prop;
	const char *s;
	size_t len;
};

struct time_info {
	struct dm_pool *mem;
	struct dm_list *ti_list;
	time_t *now;
	time_id_t min_abs_date_granularity;
	time_id_t max_abs_date_granularity;
	time_id_t min_abs_time_granularity;
	time_id_t min_rel_time_granularity;
};

static int _is_time_num(time_id_t id)
{
	return ((id > TIME_NUM__START) && (id < TIME_NUM__END));
};

/*
static int _is_time_frame(time_id_t id)
{
	return ((id > TIME_FRAME__START) && (id < TIME_FRAME__END));
};
*/

static int _is_time_label_date(time_id_t id)
{
	return ((id > TIME_LABEL_DATE__START) && (id < TIME_LABEL_DATE__END));
};

static int _is_time_label_time(time_id_t id)
{
	return ((id > TIME_LABEL_TIME__START) && (id < TIME_LABEL_TIME__END));
};

static int _is_time_unit(time_id_t id)
{
	return ((id > TIME_UNIT__START) && (id < TIME_UNIT__END));
};

static int _is_time_weekday(time_id_t id)
{
	return ((id > TIME_WEEKDAY__START) && (id < TIME_WEEKDAY__END));
};

static int _is_time_month(time_id_t id)
{
	return ((id > TIME_MONTH__START) && (id < TIME_MONTH__END));
};

static const char *_skip_space(const char *s)
{
	while (*s && isspace(*s))
		s++;
	return s;
}

/* Move till delim or space */
static const char *_move_till_item_end(const char *s)
{
	char c = *s;
	int is_num = isdigit(c);

	/*
	 * Allow numbers to be attached to next token, for example
	 * it's correct to write "12 hours" as well as "12hours".
	 */
	while (c && !isspace(c) && (is_num ? (is_num = isdigit(c)) : 1))
		c = *++s;

	return s;
}

static struct time_item *_alloc_time_item(struct dm_pool *mem, time_id_t id,
					  const char *s, size_t len)
{
	struct time_item *ti;

	if (!(ti = dm_pool_zalloc(mem, sizeof(struct time_item)))) {
		log_error("alloc_time_item: dm_pool_zalloc failed");
		return NULL;
	}

	ti->prop = &_time_props[id];
	ti->s = s;
	ti->len = len;

	return ti;
}

static int _add_time_part_to_list(struct dm_pool *mem, struct dm_list *list,
				  time_id_t id, int minus, const char *s, size_t len)
{
	struct time_item *ti1, *ti2;

	if (!(ti1 = _alloc_time_item(mem, minus ? TIME_NUM_MULTIPLIER_NEGATIVE
						: TIME_NUM_MULTIPLIER, s, len)) ||
	    !(ti2 = _alloc_time_item(mem, id, s + len, 0)))
		return 0;
	dm_list_add(list, &ti1->list);
	dm_list_add(list, &ti2->list);

	return 1;
}

static int _get_time(struct dm_pool *mem, const char **str,
		     struct dm_list *list, int tz)
{
	const char *end, *s = *str;
	int r = 0;

	/* hour */
	end = _move_till_item_end(s);
	if (!_add_time_part_to_list(mem, list, tz ? TIME_UNIT_TZ_HOUR : TIME_UNIT_HOUR,
				    tz == -1, s, end - s))
		goto out;

	/* minute */
	if (*end != ':')
		/* minute required */
		goto out;
	s = end + 1;
	end = _move_till_item_end(s);
	if (!_add_time_part_to_list(mem, list, tz ? TIME_UNIT_TZ_MINUTE : TIME_UNIT_MINUTE,
				    tz == -1, s, end - s))
		goto out;

	/* second */
	if (*end != ':') {
		/* second not required */
		s = end + 1;
		r = 1;
		goto out;
	} else if (tz)
		/* timezone does not have seconds */
		goto out;

	s = end + 1;
	end = _move_till_item_end(s);
	if (!_add_time_part_to_list(mem, list, TIME_UNIT_SECOND, 0, s, end - s))
		goto out;

	s = end + 1;
	r = 1;
out:
	*str = s;
	return r;
}

static int _preparse_fuzzy_time(const char *s, struct time_info *info)
{
	struct dm_list *list;
	struct time_item *ti;
	const char *end;
	int fuzzy = 0;
	time_id_t id;
	size_t len;
	int r = 0;
	char c;

	if (!(list = dm_pool_alloc(info->mem, sizeof(struct dm_list)))) {
		log_error("_preparse_fuzzy_time: dm_pool_alloc failed");
		goto out;
	}
	dm_list_init(list);
	s = _skip_space(s);

	while ((c = *s)) {
		/*
		 * If the string consists of -:+, digits or spaces,
		 * it's not worth looking for fuzzy names here -
		 * it's standard YYYY-MM-DD HH:MM:SS +-HH:MM format
		 * and that is parseable by libdm directly.
		 */
		if (!(isdigit(c) || (c == '-') || (c == ':') || (c == '+')))
			fuzzy = 1;

		end = _move_till_item_end(s);

		if (isalpha(c))
			id = TIME_STR;
		else if (isdigit(c)) {
			if (*end == ':') {
				/* we have time */
				if (!_get_time(info->mem, &s, list, 0))
					goto out;
				continue;
			}
			/* we have some other number */
			id = TIME_NUM;
		} else if ((c == '-') || (c == '+')) {
			s++;
			/* we have timezone */
			if (!_get_time(info->mem, &s, list, (c == '-') ? -1 : 1))
				goto out;
			continue;
		} else
			goto out;

		len = end - s;
		if (!(ti = _alloc_time_item(info->mem, id, s, len)))
			goto out;
		dm_list_add(list, &ti->list);
		s += len;
		s = _skip_space(s);
	}

	info->ti_list = list;
	r = 1;
out:
	if (!(r && fuzzy)) {
		dm_pool_free(info->mem, list);
		return 0;
	}

	return 1;
}

static int _match_time_str(struct dm_list *ti_list, struct time_item *ti)
{
	struct time_item *ti_context_p = (struct time_item *) dm_list_prev(ti_list, &ti->list);
	size_t reg_len;
	int i;

	ti->prop = TIME_PROP(TIME_NULL);

	for (i = 0; _time_reg[i].name; i++) {
		reg_len = strlen(_time_reg[i].name);
		if ((ti->len != reg_len) &&
		    !((_time_reg[i].reg_flags & TIME_REG_PLURAL_S) &&
		      (ti->len == reg_len+1) && (ti->s[reg_len] == 's')))
			continue;

		if (!strncasecmp(ti->s, _time_reg[i].name, reg_len)) {
			ti->prop = _time_reg[i].prop;
			if ((ti->prop->id > TIME_UNIT__START) && (ti->prop->id < TIME_UNIT__END) &&
			    ti_context_p && (ti_context_p->prop->id == TIME_NUM))
				ti_context_p->prop = TIME_PROP(TIME_NUM_MULTIPLIER);
			break;
		}
	}

	return ti->prop->id;
}

static int _match_time_num(struct dm_list *ti_list, struct time_item *ti)
{
	struct time_item *ti_context_p = (struct time_item *) dm_list_prev(ti_list, &ti->list);
	struct time_item *ti_context_n = (struct time_item *) dm_list_next(ti_list, &ti->list);
	struct time_item *ti_context_nn = ti_context_n ? (struct time_item *) dm_list_next(ti_list, &ti_context_n->list) : NULL;

	if (ti_context_n &&
	    (ti_context_n->prop->id > TIME_MONTH__START) &&
	    (ti_context_n->prop->id < TIME_MONTH__END)) {
		if (ti_context_nn && ti_context_nn->prop->id == TIME_NUM) {
			if (ti->len < ti_context_nn->len) {
				/* 24 Feb 2015 */
				ti->prop = TIME_PROP(TIME_NUM_DAY);
				ti_context_nn->prop = TIME_PROP(TIME_NUM_YEAR);
			} else {
				/* 2015 Feb 24 */
				ti->prop = TIME_PROP(TIME_NUM_YEAR);
				ti_context_nn->prop = TIME_PROP(TIME_NUM_DAY);
			}
		} else {
			if (ti->len <= 2)
				/* 24 Feb */
				ti->prop = TIME_PROP(TIME_NUM_DAY);
			else
				/* 2015 Feb */
				ti->prop = TIME_PROP(TIME_NUM_YEAR);
		}
	} else if (ti_context_p &&
		   (ti_context_p->prop->id > TIME_MONTH__START) &&
		   (ti_context_p->prop->id < TIME_MONTH__END)) {
		if (ti->len <= 2)
			/* Feb 24 */
			ti->prop = TIME_PROP(TIME_NUM_DAY);
		else
			/* Feb 2015 */
			ti->prop = TIME_PROP(TIME_NUM_YEAR);
	} else
		ti->prop = TIME_PROP(TIME_NUM_YEAR);

	return ti->prop->id;
}

static void _detect_time_granularity(struct time_info *info, struct time_item *ti)
{
	time_id_t gran = ti->prop->granularity;
	int is_date, is_abs, is_rel;

	if (gran == TIME_NULL)
		return;

	is_date = ti->prop->prop_flags & TIME_PROP_DATE;
	is_abs = ti->prop->prop_flags & TIME_PROP_ABS;
	is_rel = ti->prop->prop_flags & TIME_PROP_REL;

	if (is_date && is_abs) {
		if (gran > info->max_abs_date_granularity)
			info->max_abs_date_granularity = gran;
		if (gran < info->min_abs_date_granularity)
			info->min_abs_date_granularity = gran;
	} else {
		if (is_abs && (gran < info->min_abs_time_granularity))
			info->min_abs_time_granularity = gran;
		else if (is_rel && (gran < info->min_rel_time_granularity))
			info->min_rel_time_granularity = gran;
	}
}

static void _change_to_relative(struct time_info *info, struct time_item *ti)
{
	struct time_item *ti2;

	ti2 = ti;
	while ((ti2 = (struct time_item *) dm_list_prev(info->ti_list, &ti2->list))) {
		if (ti2->prop->id == TIME_FRAME_AGO)
			break;

		switch (ti2->prop->id) {
			case TIME_UNIT_SECOND:
				ti2->prop = TIME_PROP(TIME_UNIT_SECOND_REL);
				break;
			case TIME_UNIT_MINUTE:
				ti2->prop = TIME_PROP(TIME_UNIT_MINUTE_REL);
				break;
			case TIME_UNIT_HOUR:
				ti2->prop = TIME_PROP(TIME_UNIT_HOUR_REL);
				break;
			default:
				break;
		}
	}
}

static int _recognize_time_items(struct time_info *info)
{
	struct time_item *ti;

	/*
	 * At first, try to recognize strings.
	 * Also, if there are any items which may be absolute or
	 * relative and we have "TIME_FRAME_AGO", change them to relative.
	 */
	dm_list_iterate_items(ti, info->ti_list) {
		if ((ti->prop->id == TIME_STR) && !_match_time_str(info->ti_list, ti)) {
			log_error("Unrecognized string in date/time "
				  "specification at \"%s\".", ti->s);
			return 0;
		}
		if (ti->prop->id == TIME_FRAME_AGO)
			_change_to_relative(info, ti);
	}

	/*
	 * Now, recognize any numbers and be sensitive to the context
	 * given by strings we recognized before. Also, detect time
	 * granularity used (both for absolute and/or relative parts).
	 */
	dm_list_iterate_items(ti, info->ti_list) {
		if ((ti->prop->id == TIME_NUM) && !_match_time_num(info->ti_list, ti)) {
			log_error("Unrecognized number in date/time "
				  "specification at \"%s\".", ti->s);
			return 0;
		}
		_detect_time_granularity(info, ti);
	}

	return 1;
}

static int _check_time_items(struct time_info *info)
{
	struct time_item *ti;
	uint32_t flags;
	int rel;
	int date_is_relative = -1, time_is_relative = -1;
	int label_time = 0, label_date = 0;

	dm_list_iterate_items(ti, info->ti_list) {
		flags = ti->prop->prop_flags;
		rel = flags & TIME_PROP_REL;

		if (flags & TIME_PROP_DATE) {
			if (date_is_relative < 0)
				date_is_relative = rel;
			else if ((date_is_relative ^ rel) &&
				 (info->max_abs_date_granularity >= info->min_rel_time_granularity)) {
				log_error("Mixed absolute and relative date "
					  "specification found at \"%s\".", ti->s);
				return 0;
			}

			/* Date label can be used only once and not mixed with other date spec. */
			if (label_date) {
				log_error("Ambiguous date specification found at \"%s\".", ti->s);
				return 0;
			}

			if (_is_time_label_date(ti->prop->id))
				label_date = 1;
		}

		else if (flags & TIME_PROP_TIME) {
			if (time_is_relative < 0)
				time_is_relative = rel;
			else if ((time_is_relative ^ rel)) {
				log_error("Mixed absolute and relative time "
					  "specification found at \"%s\".", ti->s);
				return 0;
			}

			/* Time label can be used only once and not mixed with other time spec. */
			if (label_time) {
				log_error("Ambiguous time specification found at \"%s\".", ti->s);
				return 0;
			}

			if (_is_time_label_time(ti->prop->id))
				label_time = 1;
		}
	}

	return 1;
}

#define CACHE_ID_TIME_NOW "time_now"

static time_t *_get_now(struct dm_report *rh, struct dm_pool *mem)
{
	const void *cached_obj;
	time_t *now;

	if (!(cached_obj = dm_report_value_cache_get(rh, CACHE_ID_TIME_NOW))) {
		if (!(now = dm_pool_zalloc(mem, sizeof(time_t)))) {
			log_error("_get_now: dm_pool_zalloc failed");
			return NULL;
		}
		time(now);
		if (!dm_report_value_cache_set(rh, CACHE_ID_TIME_NOW, now)) {
			log_error("_get_now: failed to cache current time");
			return NULL;
		}
	} else
		now = (time_t *) cached_obj;

	return now;
}

static void _adjust_time_for_granularity(struct time_info *info, struct tm *tm, time_t *t)
{
	switch (info->min_abs_date_granularity) {
		case TIME_UNIT_YEAR:
			tm->tm_mon = 0;
			/* fall through */
		case TIME_UNIT_MONTH:
			tm->tm_mday = 1;
			break;
		default:
			break;
	}

	switch (info->min_abs_time_granularity) {
		case TIME_UNIT_HOUR:
			tm->tm_min = 0;
			/* fall through */
		case TIME_UNIT_MINUTE:
			tm->tm_sec = 0;
			break;
		case TIME_UNIT__END:
			if (info->min_rel_time_granularity == TIME_UNIT__END)
				tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
			break;
		default:
			break;
	}

	if ((info->min_abs_time_granularity == TIME_UNIT__END) &&
	    (info->min_rel_time_granularity >= TIME_UNIT_DAY) &&
	    (info->min_rel_time_granularity <= TIME_UNIT_YEAR))
		tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
}

#define SECS_PER_MINUTE 60
#define SECS_PER_HOUR   3600
#define SECS_PER_DAY    86400

static int _days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int _is_leap_year(long year)
{
	return (((year % 4==0) && (year % 100 != 0)) || (year % 400 == 0));
}

static int _get_days_in_month(long month, long year)
{
	return (month == 2 && _is_leap_year(year)) ? _days_in_month[month-1] + 1
						   : _days_in_month[month-1];
}

static void _get_resulting_time_span(struct time_info *info,
				     struct tm *tm, time_t t,
				     time_t *t_result1, time_t *t_result2)
{
	time_t t1 = mktime(tm) - t;
	time_t t2 = t1;
	struct tm tmp;

	if (info->min_abs_time_granularity != TIME_UNIT__END) {
		if (info->min_abs_time_granularity == TIME_UNIT_MINUTE)
			t2 += (SECS_PER_MINUTE - 1);
		else if (info->min_abs_time_granularity == TIME_UNIT_HOUR)
			t2 += (SECS_PER_HOUR - 1);
	} else if (info->min_rel_time_granularity != TIME_UNIT__END) {
		if (info->min_rel_time_granularity == TIME_UNIT_MINUTE)
			t1 -= (SECS_PER_MINUTE + 1);
		else if (info->min_rel_time_granularity == TIME_UNIT_HOUR)
			t1 -= (SECS_PER_HOUR + 1);
		else if ((info->min_rel_time_granularity >= TIME_UNIT_DAY) &&
			(info->min_rel_time_granularity <= TIME_UNIT_YEAR))
			t2 += (SECS_PER_DAY - 1);
	} else {
		if (info->min_abs_date_granularity == TIME_UNIT_MONTH)
			t2 += (SECS_PER_DAY * _get_days_in_month(tm->tm_mon + 1, tm->tm_year) - 1);
		else if (info->min_abs_date_granularity != TIME_UNIT__END)
			t2 += (SECS_PER_DAY - 1);
	}

	/* Adjust for DST if needed. */
	localtime_r(&t1, &tmp);
	if (tmp.tm_isdst)
		t1 -= SECS_PER_HOUR;
	localtime_r(&t2, &tmp);
	if (tmp.tm_isdst)
		t2 -= SECS_PER_HOUR;

	*t_result1 = t1;
	*t_result2 = t2;
}

static int _translate_time_items(struct dm_report *rh, struct time_info *info,
				 const char **data_out)
{
	struct time_item *ti, *ti_p = NULL;
	long multiplier = 1;
	struct tm tm_now;
	time_id_t id;
	char *end;
	long num;
	struct tm tm; /* absolute time */
	time_t t = 0; /* offset into past before absolute time */
	time_t t1, t2;
	char buf[32];

	localtime_r(info->now, &tm_now);
	tm = tm_now;
	tm.tm_isdst = 0; /* we'll adjust for dst later */
	tm.tm_wday = tm.tm_yday = -1;

	dm_list_iterate_items(ti, info->ti_list) {
		id = ti->prop->id;

		if (_is_time_num(id)) {
			errno = 0;
			num = strtol(ti->s, &end, 10);
			if (errno) {
				log_error("_translate_time_items: invalid time.");
				return 0;
			}
			switch (id) {
				case TIME_NUM_MULTIPLIER_NEGATIVE:
					multiplier = -num;
					break;
				case TIME_NUM_MULTIPLIER:
					multiplier = num;
					break;
				case TIME_NUM_DAY:
					tm.tm_mday = num;
					break;
				case TIME_NUM_YEAR:
					tm.tm_year = num - 1900;
					break;
				default:
					break;
			}
		} else if (_is_time_month(id)) {
			tm.tm_mon = id - TIME_MONTH__START - 1;
		} else if (_is_time_label_date(id)) {
			if (_is_time_weekday(id)) {
				num = id - TIME_WEEKDAY__START - 1;
				if (tm_now.tm_wday < num)
					num = 7 - num + tm_now.tm_wday;
				else
					num = tm_now.tm_wday - num;
				t += num * SECS_PER_DAY;
			} else switch (id) {
				case TIME_LABEL_DATE_YESTERDAY:
					t += SECS_PER_DAY;
					break;
				case TIME_LABEL_DATE_TODAY:
					/* Nothing to do here - we started with today. */
					break;
				default:
					break;
			}
		} else if (_is_time_label_time(id)) {
			switch (id) {
				case TIME_LABEL_TIME_NOON:
					tm.tm_hour = 12;
					tm.tm_min = tm.tm_sec = 0;
					break;
				case TIME_LABEL_TIME_MIDNIGHT:
					tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
					break;
				default:
					break;
			}
		} else if (_is_time_unit(id)) {
			switch (id) {
				case TIME_UNIT_SECOND:
					tm.tm_sec = multiplier;
					break;
				case TIME_UNIT_SECOND_REL:
					t += multiplier;
					break;
				case TIME_UNIT_MINUTE:
					tm.tm_min = multiplier;
					break;
				case TIME_UNIT_MINUTE_REL:
					t += (multiplier * SECS_PER_MINUTE);
					break;
				case TIME_UNIT_HOUR:
					tm.tm_hour = multiplier;
					break;
				case TIME_UNIT_HOUR_REL:
					t += (multiplier * SECS_PER_HOUR);
					break;
				case TIME_UNIT_AM:
					if (ti_p && ti_p->prop->id == TIME_NUM_MULTIPLIER)
						tm.tm_hour = multiplier;
					break;
				case TIME_UNIT_PM:
					if (ti_p && _is_time_unit(ti_p->prop->id))
						t -= 12 * SECS_PER_HOUR;
					else if (ti_p && ti_p->prop->id == TIME_NUM_MULTIPLIER)
						tm.tm_hour = multiplier + 12;
					break;
				case TIME_UNIT_DAY:
					t += multiplier * SECS_PER_DAY;
					break;
				case TIME_UNIT_WEEK:
					t += multiplier * 7 * SECS_PER_DAY;
					break;
				case TIME_UNIT_MONTH:
					/* if months > 12, convert to years first */
					num = multiplier / 12;
					tm.tm_year -= num;

					num = multiplier % 12;
					if (num > (tm.tm_mon + 1)) {
						tm.tm_year--;
						tm.tm_mon = 12 - num + tm.tm_mon;
					} else
						tm.tm_mon -= num;
					break;
				case TIME_UNIT_YEAR:
					tm.tm_year -= multiplier;
					break;
				default:
					break;
			}
		}

		ti_p = ti;
	}

	_adjust_time_for_granularity(info, &tm, &t);
	_get_resulting_time_span(info, &tm, t, &t1, &t2);

	dm_pool_free(info->mem, info->ti_list);
	info->ti_list = NULL;

	if (dm_snprintf(buf, sizeof(buf), "@" FMTd64 ":@" FMTd64, (int64_t)t1, (int64_t)t2) == -1) {
		log_error("_translate_time_items: dm_snprintf failed");
		return 0;
	}

	if (!(*data_out = dm_pool_strdup(info->mem, buf))) {
		log_error("_translate_time_items: dm_pool_strdup failed");
		return 0;
	}

	return 1;
}

static const char *_lv_time_handler_parse_fuzzy_name(struct dm_report *rh,
						     struct dm_pool *mem,
						     const char *data_in)
{
	const char *s = data_in;
	const char *data_out = NULL;
	struct time_info info = {.mem = mem,
				 .ti_list = NULL,
				 .now = _get_now(rh, mem),
				 .min_abs_date_granularity = TIME_UNIT__END,
				 .max_abs_date_granularity = TIME_UNIT__START,
				 .min_abs_time_granularity = TIME_UNIT__END,
				 .min_rel_time_granularity = TIME_UNIT__END};

	if (!info.now)
		goto_out;

	/* recognize top-level parts - string/number/time/timezone? */
	if (!_preparse_fuzzy_time(s, &info))
		goto out;

	/* recognize each part in more detail, also look at the context around if needed */
	if (!_recognize_time_items(&info))
		goto out;

	/* check if the combination of items is allowed or whether it makes sense at all */
	if (!_check_time_items(&info))
		goto out;

	/* translate items into final time range */
	if (!_translate_time_items(rh, &info, &data_out))
		goto out;
out:
	if (info.ti_list)
		dm_pool_free(info.mem, info.ti_list);
	return data_out;
}

static void *_lv_time_handler_get_dynamic_value(struct dm_report *rh,
						struct dm_pool *mem,
						const char *data_in)
{
	int64_t t1, t2;
	time_t *result;

	if (sscanf(data_in, "@" FMTd64 ":@" FMTd64, &t1, &t2) != 2) {
		log_error("Failed to get value for parsed time specification.");
		return NULL;
	}

	if (!(result = dm_pool_alloc(mem, 2 * sizeof(time_t)))) {
		log_error("Failed to allocate space to store time range.");
		return NULL;
	}

	result[0] = (time_t) t1; /* Validate range for 32b arch ? */
	result[1] = (time_t) t2;

	return result;
}

static int _lv_time_handler(struct dm_report *rh, struct dm_pool *mem,
			    uint32_t field_num,
			    dm_report_reserved_action_t action,
			    const void *data_in, const void **data_out)
{
	*data_out = NULL;
	if (!data_in)
		return 1;

	switch (action) {
		case DM_REPORT_RESERVED_PARSE_FUZZY_NAME:
			*data_out = _lv_time_handler_parse_fuzzy_name(rh, mem, data_in);
			break;
		case DM_REPORT_RESERVED_GET_DYNAMIC_VALUE:
			if (!(*data_out = _lv_time_handler_get_dynamic_value(rh, mem, data_in)))
				return 0;
			break;
		default:
			return -1;
	}

	return 1;
}

/*
 * Get type reserved value - the value returned is the direct value of that type.
 */
#define GET_TYPE_RESERVED_VALUE(id) _reserved_ ## id

/*
 * Get field reserved value - the value returned is always a pointer (const void *).
 */
#define GET_FIELD_RESERVED_VALUE(id) _reserved_ ## id.value

/*
 * Get first name assigned to the reserved value - this is the one that
 * should be reported/displayed. All the other names assigned for the reserved
 * value are synonyms recognized in selection criteria.
 */
#define GET_FIRST_RESERVED_NAME(id) _reserved_ ## id ## _names[0]

/*
 * Reserved values and their assigned names.
 * The first name is the one that is also used for reporting.
 * All names listed are synonyms recognized in selection criteria.
 * For binary-based values we map all reserved names listed onto value 1, blank onto value 0.
 *
 * TYPE_RESERVED_VALUE(type, reserved_value_id, description, value, reserved name, ...)
 * FIELD_RESERVED_VALUE(field_id, reserved_value_id, description, value, reserved name, ...)
 * FIELD_RESERVED_BINARY_VALUE(field_id, reserved_value_id, description, reserved name for 1, ...)
 *
 * Note: FIELD_RESERVED_BINARY_VALUE creates:
 * 		- 'reserved_value_id_y' (for 1)
 * 		- 'reserved_value_id_n' (for 0)
 */
#define NUM uint64_t
#define NUM_HND dm_report_reserved_handler
#define HND (dm_report_reserved_handler)
#define NOFLAG 0
#define NAMED DM_REPORT_FIELD_RESERVED_VALUE_NAMED
#define RANGE DM_REPORT_FIELD_RESERVED_VALUE_RANGE
#define FUZZY DM_REPORT_FIELD_RESERVED_VALUE_FUZZY_NAMES
#define DYNAMIC DM_REPORT_FIELD_RESERVED_VALUE_DYNAMIC_VALUE

#define TYPE_RESERVED_VALUE(type, flags, id, desc, value, ...) \
	static const char *_reserved_ ## id ## _names[] = { __VA_ARGS__, NULL}; \
	static const type _reserved_ ## id = value;

#define FIELD_RESERVED_VALUE(flags, field_id, id, desc, value, ...) \
	static const char *_reserved_ ## id ## _names[] = { __VA_ARGS__ , NULL}; \
	static const struct dm_report_field_reserved_value _reserved_ ## id = {field_ ## field_id, value};

#define FIELD_RESERVED_BINARY_VALUE(field_id, id, desc, ...) \
	FIELD_RESERVED_VALUE(NAMED, field_id, id ## _y, desc, &_one64, __VA_ARGS__, _str_yes) \
	FIELD_RESERVED_VALUE(NAMED, field_id, id ## _n, desc, &_zero64, __VA_ARGS__, _str_no)

#include "values.h"

#undef NUM
#undef NUM_HND
#undef HND
#undef NOFLAG
#undef NAMED
#undef RANGE
#undef TYPE_RESERVED_VALUE
#undef FIELD_RESERVED_VALUE
#undef FIELD_RESERVED_BINARY_VALUE
#undef FUZZY
#undef DYNAMIC

/*
 * Create array of reserved values to be registered with reporting code via
 * dm_report_init_with_selection function that initializes report with
 * selection criteria. Selection code then recognizes these reserved values
 * when parsing selection criteria.
*/

#define NUM DM_REPORT_FIELD_TYPE_NUMBER
#define NUM_HND DM_REPORT_FIELD_TYPE_NUMBER
#define HND 0
#define NOFLAG 0
#define NAMED DM_REPORT_FIELD_RESERVED_VALUE_NAMED
#define RANGE DM_REPORT_FIELD_RESERVED_VALUE_RANGE
#define FUZZY DM_REPORT_FIELD_RESERVED_VALUE_FUZZY_NAMES
#define DYNAMIC DM_REPORT_FIELD_RESERVED_VALUE_DYNAMIC_VALUE

#define TYPE_RESERVED_VALUE(type, flags, id, desc, value, ...) {type | flags, &_reserved_ ## id, _reserved_ ## id ## _names, desc},

#define FIELD_RESERVED_VALUE(flags, field_id, id, desc, value, ...) {DM_REPORT_FIELD_TYPE_NONE | flags, &_reserved_ ## id, _reserved_ ## id ## _names, desc},

#define FIELD_RESERVED_BINARY_VALUE(field_id, id, desc, ...) \
	FIELD_RESERVED_VALUE(NAMED, field_id, id ## _y, desc, &_one64, __VA_ARGS__) \
	FIELD_RESERVED_VALUE(NAMED, field_id, id ## _n, desc, &_zero64, __VA_ARGS__)

static const struct dm_report_reserved_value _report_reserved_values[] = {
	#include "values.h"
	{0, NULL, NULL, NULL}
};

#undef NUM
#undef NUM_HND
#undef HND
#undef NOFLAG
#undef NAMED
#undef RANGE
#undef FUZZY
#undef DYNAMIC
#undef TYPE_RESERVED_VALUE
#undef FIELD_RESERVED_VALUE
#undef FIELD_RESERVED_BINARY_VALUE

static int _field_string(struct dm_report *rh, struct dm_report_field *field, const char *data)
{
	return dm_report_field_string(rh, field, &data);
}

static int _field_set_value(struct dm_report_field *field, const void *data, const void *sort)
{
	dm_report_field_set_value(field, data, sort);

	return 1;
}

static int _field_set_string_list(struct dm_report *rh, struct dm_report_field *field,
				  const struct dm_list *list, void *private, int sorted,
				  const char *delimiter)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	return sorted ? dm_report_field_string_list(rh, field, list, delimiter ? : cmd->report_list_item_separator)
		      : dm_report_field_string_list_unsorted(rh, field, list, delimiter ? : cmd->report_list_item_separator);
}

/*
 * Data-munging functions to prepare each data type for display and sorting
 */

/*
 * Display either "0"/"1" or ""/"word" based on bin_value,
 * cmd->report_binary_values_as_numeric selects the mode to use.
*/
static int _binary_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field, int bin_value, const char *word,
			void *private)
{
	const struct cmd_context *cmd = (const struct cmd_context *) private;

	if (cmd->report_binary_values_as_numeric)
		/* "0"/"1" */
		return _field_set_value(field, bin_value ? _str_one : _str_zero, bin_value ? &_one64 : &_zero64);

	/* blank/"word" */
	return _field_set_value(field, bin_value ? word : "", bin_value ? &_one64 : &_zero64);
}

static int _binary_undef_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			      struct dm_report_field *field, void *private)
{
	const struct cmd_context *cmd = (const struct cmd_context *) private;

	if (cmd->report_binary_values_as_numeric)
		return _field_set_value(field, GET_FIRST_RESERVED_NAME(num_undef_64), &GET_TYPE_RESERVED_VALUE(num_undef_64));

	return _field_set_value(field, _str_unknown, &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _string_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	return dm_report_field_string(rh, field, (const char * const *) data);
}

static int _chars_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
		       struct dm_report_field *field,
		       const void *data, void *private __attribute__((unused)))
{
	return _field_string(rh, field, data);
}

static int _uuid_disp(struct dm_report *rh, struct dm_pool *mem,
		      struct dm_report_field *field,
		      const void *data, void *private)
{
	char *repstr;

	if (!(repstr = id_format_and_copy(mem, data)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _devminor_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	int devminor = (int) MINOR((*(const struct device * const *) data)->dev);

	return dm_report_field_int(rh, field, &devminor);
}

static int _devmajor_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	int devmajor = (int) MAJOR((*(const struct device * const *) data)->dev);

	return dm_report_field_int(rh, field, &devmajor);
}

static int _dev_name_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	return _field_string(rh, field, dev_name(*(const struct device * const *) data));
}

static int _devices_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct dm_list *list;

	if (!(list = lvseg_devices(mem, seg)))
		return_0;

	return _field_set_string_list(rh, field, list, private, 0, ",");
}

static int _metadatadevices_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct dm_list *list;

	if (!(list = lvseg_metadata_devices(mem, seg)))
		return_0;

	return _field_set_string_list(rh, field, list, private, 0, ",");
}

static int _peranges_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct dm_list *list;

	if (!(list = lvseg_seg_pe_ranges(mem, seg)))
		return_0;

	return _field_set_string_list(rh, field, list, private, 0, " ");
}

static int _leranges_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct dm_list *list;

	if (!(list = lvseg_seg_le_ranges(mem, seg)))
		return_0;

	return _field_set_string_list(rh, field, list, private, 0, NULL);
}

static int _metadataleranges_disp(struct dm_report *rh, struct dm_pool *mem,
				  struct dm_report_field *field,
				  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct dm_list *list;

	if (!(list = lvseg_seg_metadata_le_ranges(mem, seg)))
		return_0;

	return _field_set_string_list(rh, field, list, private, 0, NULL);
}

static int _tags_disp(struct dm_report *rh, struct dm_pool *mem,
		      struct dm_report_field *field,
		      const void *data, void *private)
{
	const struct dm_list *tagsl = (const struct dm_list *) data;

	return _field_set_string_list(rh, field, tagsl, private, 1, NULL);
}

struct _str_list_append_baton {
	struct dm_pool *mem;
	struct dm_list *result;
};

static int _str_list_append(const char *line, void *baton)
{
	struct _str_list_append_baton *b = baton;
	const char *line2 = dm_pool_strdup(b->mem, line);

	if (!line2)
		return_0;

	if (!str_list_add(b->mem, b->result, line2))
		return_0;

	return 1;
}

static int _cache_settings_disp(struct dm_report *rh, struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	const struct dm_config_node *settings;
	struct dm_list *result;
	struct _str_list_append_baton baton;
	struct dm_list dummy_list; /* dummy list to display "nothing" */

	if (seg_is_cache(seg))
		seg = first_seg(seg->pool_lv);
	else if (!seg_is_cache_pool(seg)) {
		dm_list_init(&dummy_list);
		return _field_set_string_list(rh, field, &dummy_list, private, 0, NULL);
		/* TODO: once we have support for STR_LIST reserved values, replace with:
		 * return _field_set_value(field,  GET_FIRST_RESERVED_NAME(cache_settings_undef), GET_FIELD_RESERVED_VALUE(cache_settings_undef));
		 */
	}

	if (seg->policy_settings)
		settings = seg->policy_settings->child;
	else {
		dm_list_init(&dummy_list);
		return _field_set_string_list(rh, field, &dummy_list, private, 0, NULL);
		/* TODO: once we have support for STR_LIST reserved values, replace with:
		 * return _field_set_value(field,  GET_FIRST_RESERVED_NAME(cache_settings_undef), GET_FIELD_RESERVED_VALUE(cache_settings_undef));
		 */
	}

	if (!(result = str_list_create(mem)))
		return_0;

	baton.mem = mem;
	baton.result = result;

	while (settings) {
		dm_config_write_one_node(settings, _str_list_append, &baton);
		settings = settings->sib;
	};

	return _field_set_string_list(rh, field, result, private, 0, NULL);
}

static int _do_get_kernel_cache_settings_list(struct dm_pool *mem,
					      int cache_argc, char **cache_argv,
					      struct dm_list *result)
{
	const char *key, *value;
	char *buf;
	size_t buf_len;
	int i;

	for (i = 0; i+1 < cache_argc; i += 2) {
		key = cache_argv[i];
		value = cache_argv[i+1];
		/* +1 for "=" char and +1 for trailing zero */
		buf_len = strlen(key) + strlen(value) + 2;
		if (!(buf = dm_pool_alloc(mem, buf_len)))
			return_0;
		if (dm_snprintf(buf, buf_len, "%s=%s", key, value) < 0)
			return_0;
		if (!str_list_add_no_dup_check(mem, result, buf))
			return_0;
	}

	return 1;
}

static int _get_kernel_cache_settings_list(struct dm_pool *mem,
					   struct dm_status_cache *cache_status,
					   struct dm_list **result)
{
	if (!(*result = str_list_create(mem)))
		return_0;

	if (!_do_get_kernel_cache_settings_list(mem, cache_status->core_argc,
						cache_status->core_argv, *result))
		return_0;

	if (!_do_get_kernel_cache_settings_list(mem, cache_status->policy_argc,
						cache_status->policy_argv, *result))
		return_0;

	return 1;
}

static int _kernel_cache_settings_disp(struct dm_report *rh, struct dm_pool *mem,
				       struct dm_report_field *field,
				       const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	struct dm_list dummy_list; /* dummy list to display "nothing" */
	struct dm_list *result;
	int r = 0;

	if (lvdm->seg_status.type != SEG_STATUS_CACHE) {
		dm_list_init(&dummy_list);
		return _field_set_string_list(rh, field, &dummy_list, private, 0, NULL);
	}

	if (!(mem = dm_pool_create("reporter_pool", 1024)))
		return_0;

	if (!_get_kernel_cache_settings_list(mem, lvdm->seg_status.cache, &result))
		goto_out;

	r = _field_set_string_list(rh, field, result, private, 0, NULL);
out:
	dm_pool_destroy(mem);
	return r;
}

static int _kernel_cache_policy_disp(struct dm_report *rh, struct dm_pool *mem,
				     struct dm_report_field *field,
				     const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if ((lvdm->seg_status.type == SEG_STATUS_CACHE) &&
	    lvdm->seg_status.cache->policy_name)
		return _field_string(rh, field, lvdm->seg_status.cache->policy_name);

	return _field_set_value(field, GET_FIRST_RESERVED_NAME(cache_policy_undef),
				GET_FIELD_RESERVED_VALUE(cache_policy_undef));
}

static int _kernelmetadataformat_disp(struct dm_report *rh, struct dm_pool *mem,
				      struct dm_report_field *field,
				      const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	unsigned format;

	if (lvdm->seg_status.type == SEG_STATUS_CACHE) {
		format = (lvdm->seg_status.cache->feature_flags & DM_CACHE_FEATURE_METADATA2);
		return dm_report_field_uint64(rh, field, format ? &_two64 : &_one64);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _cache_policy_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	if (seg_is_cache(seg))
		seg = first_seg(seg->pool_lv);
	else if (!seg_is_cache_pool(seg) || !seg->policy_name)
		return _field_set_value(field, GET_FIRST_RESERVED_NAME(cache_policy_undef),
					GET_FIELD_RESERVED_VALUE(cache_policy_undef));

	if (!seg->policy_name) {
		log_error(INTERNAL_ERROR "Unexpected NULL policy name.");
		return 0;
	}

	return _field_string(rh, field, seg->policy_name);
}

static int _modules_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct dm_list *modules;

	if (!(modules = str_list_create(mem))) {
		log_error("modules str_list allocation failed");
		return 0;
	}

	if (!(list_lv_modules(mem, lv, modules)))
		return_0;

	return _field_set_string_list(rh, field, modules, private, 1, NULL);
}

static int _lvprofile_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv->profile)
		return _field_string(rh, field, lv->profile->name);

	return _field_set_value(field, "", NULL);
}

static int _lvlockargs_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	return _field_string(rh, field, lv->lock_args ? : "");
}

static int _vgfmt_disp(struct dm_report *rh, struct dm_pool *mem,
		       struct dm_report_field *field,
		       const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;

	if (vg->fid && vg->fid->fmt)
		return _field_string(rh, field, vg->fid->fmt->name);

	return _field_set_value(field, "", NULL);
}

static int _pvfmt_disp(struct dm_report *rh, struct dm_pool *mem,
		       struct dm_report_field *field,
		       const void *data, void *private)
{
	const struct label *l = (const struct label *) data;

	if (l->labeller && l->labeller->fmt)
		return _field_string(rh, field, l->labeller->fmt->name);

	return _field_set_value(field, "", NULL);
}

static int _lvkmaj_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists && lvdm->info.major >= 0)
		return dm_report_field_int(rh, field, &lvdm->info.major);

	return dm_report_field_int32(rh, field, &GET_TYPE_RESERVED_VALUE(num_undef_32));
}

static int _lvkmin_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists && lvdm->info.minor >= 0)
		return dm_report_field_int(rh, field, &lvdm->info.minor);

	return dm_report_field_int32(rh, field, &GET_TYPE_RESERVED_VALUE(num_undef_32));
}

static int _lvstatus_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private __attribute__((unused)))
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	char *repstr;

	if (!(repstr = lv_attr_dup_with_info_and_seg_status(mem, lvdm)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _pvstatus_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private __attribute__((unused)))
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;
	char *repstr;

	if (!(repstr = pv_attr_dup(mem, pv)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _vgstatus_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private __attribute__((unused)))
{
	const struct volume_group *vg = (const struct volume_group *) data;
	char *repstr;

	if (!(repstr = vg_attr_dup(mem, vg)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _segtype_disp(struct dm_report *rh __attribute__((unused)),
			 struct dm_pool *mem __attribute__((unused)),
			 struct dm_report_field *field,
			 const void *data, void *private __attribute__((unused)))
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	char *name;

	if (!(name = lvseg_segtype_dup(mem, seg))) {
		log_error("Failed to get segtype name.");
		return 0;
	}

	return _field_set_value(field, name, NULL);
}

static int _lvname_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int is_historical = lv_is_historical(lv);
	const char *tmp_lvname;
	char *repstr, *lvname;
	size_t len;

	if (!is_historical && (lv_is_visible(lv) || !cmd->report_mark_hidden_devices))
		return _field_string(rh, field, lv->name);

	if (is_historical) {
		tmp_lvname = lv->this_glv->historical->name;
		len = strlen(tmp_lvname) + strlen(HISTORICAL_LV_PREFIX) + 1;
	} else {
		tmp_lvname = lv->name;
		len = strlen(tmp_lvname) + 3;
	}

	if (!(repstr = dm_pool_zalloc(mem, len))) {
		log_error("dm_pool_alloc failed");
		return 0;
	}

	if (dm_snprintf(repstr, len, "%s%s%s",
			is_historical ? HISTORICAL_LV_PREFIX : "[",
			tmp_lvname,
			is_historical ? "" : "]") < 0) {
		log_error("lvname snprintf failed");
		return 0;
	}

	if (!(lvname = dm_pool_strdup(mem, tmp_lvname))) {
		log_error("dm_pool_strdup failed");
		return 0;
	}

	return _field_set_value(field, repstr, lvname);
}

static int _do_loglv_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private,
			  int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *mirror_log_lv = lv_mirror_log_lv(lv);

	if (!mirror_log_lv)
		return _field_set_value(field, "", NULL);

	if (uuid)
		return _uuid_disp(rh, mem, field, &mirror_log_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, mirror_log_lv, private);
}

static int _loglv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
		       struct dm_report_field *field,
		       const void *data, void *private __attribute__((unused)))
{
	return _do_loglv_disp(rh, mem, field, data, private, 0);
}

static int _loglvuuid_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			   struct dm_report_field *field,
			   const void *data, void *private __attribute__((unused)))
{
	return _do_loglv_disp(rh, mem, field, data, private, 1);
}

static int _lvfullname_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;

	if (!(repstr = lv_fullname_dup(mem, lv)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _lvparent_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *parent_lv = lv_parent(lv);

	if (!parent_lv)
		return _field_set_value(field, "", NULL);

	return _lvname_disp(rh, mem, field, parent_lv, private);
}

static int _do_datalv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			   struct dm_report_field *field,
			   const void *data, void *private __attribute__((unused)),
			   int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *data_lv = lv_data_lv(lv);

	if (!data_lv)
		return _field_set_value(field, "", NULL);

	if (uuid)
		return _uuid_disp(rh, mem, field, &data_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, data_lv, private);
}

static int _datalv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	return _do_datalv_disp(rh, mem, field, data, private, 0);
}

static int _datalvuuid_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	return _do_datalv_disp(rh, mem, field, data, private, 1);
}

static int _do_metadatalv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			       struct dm_report_field *field,
			       const void *data, void *private __attribute__((unused)),
			       int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *metadata_lv = lv_metadata_lv(lv);

	if (!metadata_lv)
		return _field_set_value(field, "", NULL);		

	if (uuid)
		return _uuid_disp(rh, mem, field, &metadata_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, metadata_lv, private);
}

static int _metadatalv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	return _do_metadatalv_disp(rh, mem, field, data, private, 0);
}

static int _metadatalvuuid_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
				struct dm_report_field *field,
				const void *data, void *private __attribute__((unused)))
{
	return _do_metadatalv_disp(rh, mem, field, data, private, 1);
}

static int _do_poollv_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private,
			   int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *pool_lv = lv_pool_lv(lv);

	if (!pool_lv)
		return _field_set_value(field, "", NULL);

	if (uuid)
		return _uuid_disp(rh, mem, field, &pool_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, pool_lv, private);
}

static int _poollv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	return _do_poollv_disp(rh, mem, field, data, private, 0);
}

static int _poollvuuid_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	return _do_poollv_disp(rh, mem, field, data, private, 1);
}

static int _lvpath_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;

	if (!(repstr = lv_path_dup(mem, lv)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _lvdmpath_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;

	if (!(repstr = lv_dmpath_dup(mem, lv)))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _do_origin_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private,
			   int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct logical_volume *origin_lv = lv_origin_lv(lv);

	if (!origin_lv)
		return _field_set_value(field, "", NULL);

	if (uuid)
		return _uuid_disp(rh, mem, field, &origin_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, origin_lv, private);
}

static int _origin_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	return _do_origin_disp(rh, mem, field, data, private, 0);
}

static int _originuuid_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	return _do_origin_disp(rh, mem, field, data, private, 1);
}

static const char *_get_glv_str(char *buf, size_t buf_len,
				struct generic_logical_volume *glv)
{
	if (!glv->is_historical)
		return glv->live->name;

	if (dm_snprintf(buf, buf_len, "%s%s", HISTORICAL_LV_PREFIX, glv->historical->name) < 0) {
		log_error("_get_glv_str: dm_snprintf failed");
		return NULL;
	}

	return buf;
}

static int _find_ancestors(struct _str_list_append_baton *ancestors,
			   struct generic_logical_volume glv,
			   int full, int include_historical_lvs)
{
	struct lv_segment *seg;
	void *orig_p = glv.live;
	const char *ancestor_str;
	char buf[NAME_LEN + strlen(HISTORICAL_LV_PREFIX) + 1];

	if (glv.is_historical) {
		if (full && glv.historical->indirect_origin)
			glv = *glv.historical->indirect_origin;
	} else if (lv_is_cow(glv.live)) {
		glv.live = origin_from_cow(glv.live);
	} else if (lv_is_thin_volume(glv.live)) {
		seg = first_seg(glv.live);
		if (seg->origin)
			glv.live = seg->origin;
		else if (seg->external_lv)
			glv.live = seg->external_lv;
		else if (full && seg->indirect_origin)
			glv = *seg->indirect_origin;
	}

	if (orig_p != glv.live) {
		if (!(ancestor_str = _get_glv_str(buf, sizeof(buf), &glv)))
			return_0;
		if (!glv.is_historical || include_historical_lvs) {
			if (!_str_list_append(ancestor_str, ancestors))
				return_0;
		}
		if (!_find_ancestors(ancestors, glv, full, include_historical_lvs))
			return_0;
	}

	return 1;
}

static int _lvancestors_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	struct logical_volume *lv = (struct logical_volume *) data;
	struct _str_list_append_baton ancestors;
	struct generic_logical_volume glv;

	ancestors.mem = mem;
	if (!(ancestors.result = str_list_create(mem)))
		return_0;

	if ((glv.is_historical = lv_is_historical(lv)))
		glv.historical = lv->this_glv->historical;
	else
		glv.live = lv;

	if (!_find_ancestors(&ancestors, glv, 0, cmd->include_historical_lvs)) {
		dm_pool_free(ancestors.mem, ancestors.result);
		return_0;
	}

	return _field_set_string_list(rh, field, ancestors.result, private, 0, NULL);
}

static int _lvfullancestors_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	struct logical_volume *lv = (struct logical_volume *) data;
	struct _str_list_append_baton full_ancestors;
	struct generic_logical_volume glv;

	full_ancestors.mem = mem;
	if (!(full_ancestors.result = str_list_create(mem)))
		return_0;

	if ((glv.is_historical = lv_is_historical(lv)))
		glv.historical = lv->this_glv->historical;
	else
		glv.live = lv;

	if (!_find_ancestors(&full_ancestors, glv, 1, cmd->include_historical_lvs)) {
		dm_pool_free(full_ancestors.mem, full_ancestors.result);
		return_0;
	}

	return _field_set_string_list(rh, field, full_ancestors.result, private, 0, NULL);
}

static int _find_descendants(struct _str_list_append_baton *descendants,
			     struct generic_logical_volume glv,
			     int full, int include_historical_lvs)
{
	struct generic_logical_volume glv_next = {0};
	const struct seg_list *sl;
	struct lv_segment *seg;
	struct glv_list *glvl;
	struct dm_list *list;
	const char *descendant_str;
	char buf[64];

	if (glv.is_historical) {
		if (full) {
			list = &glv.historical->indirect_glvs;
			dm_list_iterate_items(glvl, list) {
				if (!glvl->glv->is_historical || include_historical_lvs) {
					if (!(descendant_str = _get_glv_str(buf, sizeof(buf), glvl->glv)))
						return_0;
					if (!_str_list_append(descendant_str, descendants))
						return_0;
				}
				if (!_find_descendants(descendants, *glvl->glv, full, include_historical_lvs))
					return_0;
			}
		}
	} else if (lv_is_origin(glv.live)) {
		list = &glv.live->snapshot_segs;
		dm_list_iterate_items_gen(seg, list, origin_list) {
			if ((glv.live = seg->cow)) {
				if (!(descendant_str = _get_glv_str(buf, sizeof(buf), &glv)))
					return_0;
				if (!_str_list_append(descendant_str, descendants))
					return_0;
				if (!_find_descendants(descendants, glv, full, include_historical_lvs))
					return_0;
			}
		}
	} else {
		list = &glv.live->segs_using_this_lv;
		dm_list_iterate_items(sl, list) {
			if (lv_is_thin_volume(sl->seg->lv)) {
				seg = first_seg(sl->seg->lv);
				if ((seg->origin == glv.live) || (seg->external_lv == glv.live)) {
					glv_next.live = sl->seg->lv;
					if (!(descendant_str = _get_glv_str(buf, sizeof(buf), &glv_next)))
						return_0;
					if (!_str_list_append(descendant_str, descendants))
						return_0;
					if (!_find_descendants(descendants, glv_next, full, include_historical_lvs))
						return_0;
				}
			}
		}

		if (full) {
			list = &glv.live->indirect_glvs;
			dm_list_iterate_items(glvl, list) {
				if (!glvl->glv->is_historical || include_historical_lvs) {
					if (!(descendant_str = _get_glv_str(buf, sizeof(buf), glvl->glv)))
						return_0;
					if (!_str_list_append(descendant_str, descendants))
						return_0;
				}
				if (!_find_descendants(descendants, *glvl->glv, full, include_historical_lvs))
					return_0;
			}
		}
	}

	return 1;
}

static int _lvdescendants_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	struct logical_volume *lv = (struct logical_volume *) data;
	struct _str_list_append_baton descendants;
	struct generic_logical_volume glv;

	descendants.mem = mem;
	if (!(descendants.result = str_list_create(mem)))
		return_0;

	if ((glv.is_historical = lv_is_historical(lv)))
		glv.historical = lv->this_glv->historical;
	else
		glv.live = lv;

	if (!_find_descendants(&descendants, glv, 0, cmd->include_historical_lvs)) {
		dm_pool_free(descendants.mem, descendants.result);
		return_0;
	}

	return _field_set_string_list(rh, field, descendants.result, private, 0, NULL);
}

static int _lvfulldescendants_disp(struct dm_report *rh, struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data, void *private)
{
	struct cmd_context *cmd = (struct cmd_context *) private;
	struct logical_volume *lv = (struct logical_volume *) data;
	struct _str_list_append_baton descendants;
	struct generic_logical_volume glv;

	descendants.mem = mem;
	if (!(descendants.result = str_list_create(mem)))
		return_0;

	if ((glv.is_historical = lv_is_historical(lv)))
		glv.historical = lv->this_glv->historical;
	else
		glv.live = lv;

	if (!_find_descendants(&descendants, glv, 1, cmd->include_historical_lvs)) {
		dm_pool_free(descendants.mem, descendants.result);
		return_0;
	}

	return _field_set_string_list(rh, field, descendants.result, private, 0, NULL);
}

static int _do_movepv_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private,
			   int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	const char *repstr;

	if (uuid)
		repstr = lv_move_pv_uuid_dup(mem, lv);
	else
		repstr = lv_move_pv_dup(mem, lv);

	if (repstr)
		return _field_string(rh, field, repstr);

	return _field_set_value(field, "", NULL);
}

static int _movepv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	return _do_movepv_disp(rh, mem, field, data, private, 0);
}

static int _movepvuuid_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	return _do_movepv_disp(rh, mem, field, data, private, 1);
}

static int _do_convertlv_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private,
			      int uuid)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	const struct logical_volume *convert_lv = lv_convert_lv(lv);

	if (!convert_lv)
		return _field_set_value(field, "", NULL);

	if (uuid)
		return _uuid_disp(rh, mem, field, &convert_lv->lvid.id[1], private);

	return _lvname_disp(rh, mem, field, convert_lv, private);
}

static int _convertlv_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			   struct dm_report_field *field,
			   const void *data, void *private __attribute__((unused)))
{
	return _do_convertlv_disp(rh, mem, field, data, private, 0);
}

static int _convertlvuuid_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			       struct dm_report_field *field,
			       const void *data, void *private __attribute__((unused)))
{
	return _do_convertlv_disp(rh, mem, field, data, private, 1);
}

static int _size32_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const uint32_t size = *(const uint32_t *) data;
	const char *disp, *repstr;
	double *sortval;

	if (!*(disp = display_size_units(private, (uint64_t) size)))
		return_0;

	if (!(repstr = dm_pool_strdup(mem, disp))) {
		log_error("dm_pool_strdup failed");
		return 0;
	}

	if (!(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
		log_error("dm_pool_alloc failed");
		return 0;
	}

	*sortval = (double) size;

	return _field_set_value(field, repstr, sortval);
}

static int _size64_disp(struct dm_report *rh __attribute__((unused)),
			struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const uint64_t size = *(const uint64_t *) data;
	const char *disp, *repstr;
	double *sortval;

	if (!*(disp = display_size_units(private, size)))
		return_0;

	if (!(repstr = dm_pool_strdup(mem, disp))) {
		log_error("dm_pool_strdup failed");
		return 0;
	}

	if (!(sortval = dm_pool_alloc(mem, sizeof(double)))) {
		log_error("dm_pool_alloc failed");
		return 0;
	}

	*sortval = (double) size;

	return _field_set_value(field, repstr, sortval);
}

static int _lv_size_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	const struct lv_segment *seg = first_seg(lv);
	uint64_t size = lv->le_count;

	if (seg && !lv_is_raid_image(lv))
		size -= seg->reshape_len * (seg->area_count > 2 ? (seg->area_count - seg->segtype->parity_devs) : 1);

	size *= lv->vg->extent_size;

	return _size64_disp(rh, mem, field, &size, private);
}

static int _uint32_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	return dm_report_field_uint32(rh, field, data);
}

static int _int8_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
		       struct dm_report_field *field,
		       const void *data, void *private __attribute__((unused)))
{
	const int32_t val = *(const int8_t *)data;

	return dm_report_field_int32(rh, field, &val);
}

static int _int32_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
		       struct dm_report_field *field,
		       const void *data, void *private __attribute__((unused)))
{
	return dm_report_field_int32(rh, field, data);
}

static int _lvwhenfull_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv_is_thin_pool(lv)) {
		if (lv->status & LV_ERROR_WHEN_FULL)
			return _field_set_value(field, GET_FIRST_RESERVED_NAME(lv_when_full_error),
						GET_FIELD_RESERVED_VALUE(lv_when_full_error));

		return _field_set_value(field, GET_FIRST_RESERVED_NAME(lv_when_full_queue),
					GET_FIELD_RESERVED_VALUE(lv_when_full_queue));
	}

	return _field_set_value(field, GET_FIRST_RESERVED_NAME(lv_when_full_undef),
				GET_FIELD_RESERVED_VALUE(lv_when_full_undef));
}

static int _lvreadahead_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv->read_ahead == DM_READ_AHEAD_AUTO)
		return _field_set_value(field, GET_FIRST_RESERVED_NAME(lv_read_ahead_auto),
					GET_FIELD_RESERVED_VALUE(lv_read_ahead_auto));

	return _size32_disp(rh, mem, field, &lv->read_ahead, private);
}

static int _lvkreadahead_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data,
			      void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (!lvdm->info.exists)
		return dm_report_field_int32(rh, field, &GET_TYPE_RESERVED_VALUE(num_undef_32));

	return _size32_disp(rh, mem, field, &lvdm->info.read_ahead, private);
}

static int _vgsize_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint64_t size = vg_size(vg);

	return _size64_disp(rh, mem, field, &size, private);
}

static int _segmonitor_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *)data;
	char *str;

	if (!(str = lvseg_monitor_dup(mem, seg)))
		return_0;

	if (*str)
		return _field_set_value(field, str, NULL);

	return _field_set_value(field, GET_FIRST_RESERVED_NAME(seg_monitor_undef),
				GET_FIELD_RESERVED_VALUE(seg_monitor_undef));
}

static int _segstart_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint64_t start = lvseg_start(seg);

	return _size64_disp(rh, mem, field, &start, private);
}

static int _segstartpe_disp(struct dm_report *rh,
			    struct dm_pool *mem __attribute__((unused)),
			    struct dm_report_field *field,
			    const void *data,
			    void *private __attribute__((unused)))
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	return dm_report_field_uint32(rh, field, &seg->le);
}

/* Hepler: get used stripes = total stripes minux any to remove after reshape */
static int _get_seg_used_stripes(const struct lv_segment *seg)
{
	uint32_t s;
	uint32_t stripes = seg->area_count;

	for (s = seg->area_count - 1; stripes && s; s--) {
		if (seg_type(seg, s) == AREA_LV &&
		    seg_lv(seg, s)->status & LV_REMOVE_AFTER_RESHAPE)
			stripes--;
		else
			break;
	}

	return stripes;
}

static int _seg_stripes_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct lv_segment *seg = ((const struct lv_segment *) data);

	return dm_report_field_uint32(rh, field, &seg->area_count);
}

/* Report the number of data stripes, which is less than total stripes (e.g. 2 less for raid6) */
static int _seg_data_stripes_disp(struct dm_report *rh, struct dm_pool *mem,
				  struct dm_report_field *field,
				  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint32_t stripes = _get_seg_used_stripes(seg) - seg->segtype->parity_devs;

	/* FIXME: in case of odd numbers of raid10 stripes */
	if (seg_is_raid10(seg))
		stripes /= seg->data_copies;

	return dm_report_field_uint32(rh, field, &stripes);
}

/* Helper: return the top-level, reshapable raid LV in case @seg belongs to an raid rimage LV */
static struct logical_volume *_lv_for_raid_image_seg(const struct lv_segment *seg, struct dm_pool *mem)
{
	char *lv_name;

	if (seg_is_reshapable_raid(seg))
		return seg->lv;

	if (seg->lv &&
	    lv_is_raid_image(seg->lv) && !seg->le &&
	    (lv_name = dm_pool_strdup(mem, seg->lv->name))) {
		char *p = strchr(lv_name, '_');

		if (p) {
			/* Handle duplicated sub LVs */
			if (strstr(p, "_dup_"))
				p = strchr(p + 5, '_');

			if (p) {
				struct lv_list *lvl;

				*p = '\0';
				if ((lvl = find_lv_in_vg(seg->lv->vg, lv_name)) &&
				    seg_is_reshapable_raid(first_seg(lvl->lv)))
					return lvl->lv;

			}
		}
	}

	return NULL;
}

/* Helper: return the top-level raid LV in case it is reshapale for @seg or @seg if it is */
static const struct lv_segment *_get_reshapable_seg(const struct lv_segment *seg, struct dm_pool *mem)
{
	return _lv_for_raid_image_seg(seg, mem) ? seg : NULL;
}

/* Display segment reshape length in current units */
static int _seg_reshape_len_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	const struct lv_segment *seg = _get_reshapable_seg((const struct lv_segment *) data, mem);

	if (seg) {
		uint32_t reshape_len = seg->reshape_len * seg->area_count * seg->lv->vg->extent_size;

		return _size32_disp(rh, mem, field, &reshape_len, private);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

/* Display segment reshape length of in logical extents */
static int _seg_reshape_len_le_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	const struct lv_segment *seg = _get_reshapable_seg((const struct lv_segment *) data, mem);

	if (seg) {
		uint32_t reshape_len = seg->reshape_len* seg->area_count;

		return dm_report_field_uint32(rh, field, &reshape_len);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

/* Display segment data copies (e.g. 3 for raid6) */
static int _seg_data_copies_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	if (seg->data_copies)
		return dm_report_field_uint32(rh, field, &seg->data_copies);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

/* Helper: display segment data offset/new data offset in sectors */
static int _segdata_offset(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private, int new_data_offset)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	struct logical_volume *lv;

	if ((lv = _lv_for_raid_image_seg(seg, mem))) {
		uint64_t data_offset = 0;

		if (lv_raid_data_offset(lv, &data_offset)) {
			if (new_data_offset && lv_is_raid_image(lv) && !lv_raid_image_in_sync(lv))
				data_offset = data_offset ? 0 : (uint64_t) seg->reshape_len * lv->vg->extent_size;

			return dm_report_field_uint64(rh, field, &data_offset);
		}

	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _seg_data_offset_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	return _segdata_offset(rh, mem, field, data, private, 0);
}

static int _seg_new_data_offset_disp(struct dm_report *rh, struct dm_pool *mem,
				     struct dm_report_field *field,
				     const void *data, void *private)
{
	return _segdata_offset(rh, mem, field, data, private, 1);
}

static int _seg_parity_chunks_disp(struct dm_report *rh, struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint32_t parity_chunks = seg->segtype->parity_devs ?: seg->data_copies - 1;

	if (parity_chunks) {
		uint32_t s, resilient_sub_lvs = 0;

		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) == AREA_LV) {
				struct lv_segment *seg1 = first_seg(seg_lv(seg, s));

				if (seg1->segtype->parity_devs ||
				    seg1->data_copies > 1)
					resilient_sub_lvs++;
			}
		}

		if (resilient_sub_lvs && resilient_sub_lvs == seg->area_count)
			parity_chunks++;

		return dm_report_field_uint32(rh, field, &parity_chunks);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _segsize_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint64_t size = lvseg_size(seg);

	return _size64_disp(rh, mem, field, &size, private);
}

static int _segsizepe_disp(struct dm_report *rh,
			   struct dm_pool *mem __attribute__((unused)),
			   struct dm_report_field *field,
			   const void *data,
			   void *private __attribute__((unused)))
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	return dm_report_field_uint32(rh, field, &seg->len);
}

static int _chunksize_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint64_t size = lvseg_chunksize(seg);

	return _size64_disp(rh, mem, field, &size, private);
}

static int _transactionid_disp(struct dm_report *rh, struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	if (seg_is_thin_pool(seg) || seg_is_thin_volume(seg))
		return dm_report_field_uint64(rh, field, &seg->transaction_id);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _thinid_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	if (seg_is_thin_volume(seg))
		return dm_report_field_uint32(rh, field, &seg->device_id);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _discards_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	const char *discards_str;

	if (seg_is_thin_volume(seg))
		seg = first_seg(seg->pool_lv);

	if (seg_is_thin_pool(seg)) {
		discards_str = get_pool_discards_name(seg->discards);
		return _field_string(rh, field, discards_str);
	}

	return _field_set_value(field, "", NULL);
}

static int _kdiscards_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	const char *discards_str;

	if (!(discards_str = lvseg_kernel_discards_dup_with_info_and_seg_status(mem, lvdm)))
		return_0;

	if (*discards_str)
		return _field_set_value(field, discards_str, NULL);

	return _field_set_value(field, GET_FIRST_RESERVED_NAME(seg_kernel_discards_undef),
				GET_FIELD_RESERVED_VALUE(seg_kernel_discards_undef));
}

static int _cachemode_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	return _field_string(rh, field, display_cache_mode(seg));
}

static int _cachemetadataformat_disp(struct dm_report *rh, struct dm_pool *mem,
				     struct dm_report_field *field,
				     const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	const uint64_t *fmt;

	if (seg_is_cache(seg))
		seg = first_seg(seg->pool_lv);

	if (seg_is_cache_pool(seg)) {
		switch (seg->cache_metadata_format) {
		case CACHE_METADATA_FORMAT_1:
		case CACHE_METADATA_FORMAT_2:
			fmt = (seg->cache_metadata_format == CACHE_METADATA_FORMAT_2) ? &_two64 : &_one64;
			return dm_report_field_uint64(rh, field, fmt);
		default: /* unselected/undefined for all other cases */;
		}
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _originsize_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	uint64_t size = lv_origin_size(lv);

	if (size)
		return _size64_disp(rh, mem, field, &size, private);

	return _field_set_value(field, "", &_zero64);
}

static int _pvused_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;

	uint64_t used = pv_used(pv);

	return _size64_disp(rh, mem, field, &used, private);
}

static int _pvfree_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;
	uint64_t freespace;

	if (is_orphan(pv) && is_used_pv(pv))
		freespace = 0;
	else
		freespace = pv_free(pv);

	return _size64_disp(rh, mem, field, &freespace, private);
}

static int _pvsize_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;
	uint64_t size = pv_size_field(pv);

	return _size64_disp(rh, mem, field, &size, private);
}

static int _devsize_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	struct device *dev = *(struct device * const *) data;
	uint64_t size;

	if (!dev || !dev->dev || !dev_get_size(dev, &size))
		size = _zero64;

	return _size64_disp(rh, mem, field, &size, private);
}

static int _vgfree_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint64_t freespace = vg_free(vg);

	return _size64_disp(rh, mem, field, &freespace, private);
}

static int _vgsystemid_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	const char *repstr = (vg->system_id && *vg->system_id) ? vg->system_id : "";

	return _field_string(rh, field, repstr);
}

static int _vglocktype_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	const char *locktype;

	if (!vg->lock_type || !strcmp(vg->lock_type, "none"))
		locktype = "";
	else
		locktype = vg->lock_type;

	return _field_string(rh, field, locktype);
}

static int _vglockargs_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;

	return _field_string(rh, field, vg->lock_args ? : "");
}

static int _lvuuid_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	const union lvid *lvid;
	char *repstr;

	if (lv_is_historical(lv))
		lvid = &lv->this_glv->historical->lvid;
	else
		lvid = &lv->lvid;

	if (!(repstr = id_format_and_copy(mem, &lvid->id[1])))
		return_0;

	return _field_set_value(field, repstr, NULL);
}

static int _pvuuid_disp(struct dm_report *rh __attribute__((unused)), struct dm_pool *mem,
		        struct dm_report_field *field,
		        const void *data, void *private __attribute__((unused)))
{
	const struct label *label = (const struct label *) data;

	if (!label->dev)
		return _field_set_value(field, "", NULL);

	return _uuid_disp(rh, mem, field, label->dev->pvid, private);
}

static int _pvmdas_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;
	uint32_t count = pv_mda_count(pv);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _pvmdasused_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct physical_volume *pv =
	    (const struct physical_volume *) data;
	uint32_t count = pv_mda_used_count(pv);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _vgmdas_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = vg_mda_count(vg);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _vgmdasused_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = vg_mda_used_count(vg);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _vgmdacopies_disp(struct dm_report *rh, struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = vg_mda_copies(vg);

	if (count == VGMETADATACOPIES_UNMANAGED)
		return _field_set_value(field, GET_FIRST_RESERVED_NAME(vg_mda_copies_unmanaged),
					GET_FIELD_RESERVED_VALUE(vg_mda_copies_unmanaged));

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _vgprofile_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;

	if (vg->profile)
		return _field_string(rh, field, vg->profile->name);

	return _field_set_value(field, "", NULL);
}

static int _vgmissingpvcount_disp(struct dm_report *rh, struct dm_pool *mem,
				  struct dm_report_field *field,
				  const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = vg_missing_pv_count(vg);

	return _uint32_disp(rh, mem, field, &count, private);
}


static int _pvmdafree_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct label *label = (const struct label *) data;
	uint64_t freespace = lvmcache_info_mda_free(label->info);

	return _size64_disp(rh, mem, field, &freespace, private);
}

static int _pvmdasize_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct label *label = (const struct label *) data;
	uint64_t min_mda_size = lvmcache_smallest_mda_size(label->info);

	return _size64_disp(rh, mem, field, &min_mda_size, private);
}

static int _pvextvsn_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	const struct label *label = (const struct label *) data;
	struct lvmcache_info *info = label->info;
	uint32_t ext_version;

	if (info) {
		ext_version = lvmcache_ext_version(info);
		return _uint32_disp(rh, mem, field, &ext_version, private);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}


static int _vgmdasize_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint64_t min_mda_size = vg_mda_size(vg);

	return _size64_disp(rh, mem, field, &min_mda_size, private);
}

static int _vgmdafree_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint64_t freespace = vg_mda_free(vg);

	return _size64_disp(rh, mem, field, &freespace, private);
}

static int _lvcount_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = vg_visible_lvs(vg);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _lvsegcount_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	uint32_t count = dm_list_size(&lv->segments);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _snapcount_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct volume_group *vg = (const struct volume_group *) data;
	uint32_t count = snapshot_count(vg);

	return _uint32_disp(rh, mem, field, &count, private);
}

static int _snpercent_disp(struct dm_report *rh, struct dm_pool *mem __attribute__((unused)),
			   struct dm_report_field *field,
			   const void *data, void *private __attribute__((unused)))
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	dm_percent_t percent = lvseg_percent_with_info_and_seg_status(lvdm, PERCENT_GET_DATA);

	return dm_report_field_percent(rh, field, &percent);
}

static int _copypercent_disp(struct dm_report *rh,
			     struct dm_pool *mem __attribute__((unused)),
			     struct dm_report_field *field,
			     const void *data, void *private __attribute__((unused)))
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	const struct logical_volume *lv = lvdm->lv;
	dm_percent_t percent = DM_PERCENT_INVALID;

	/* TODO: just cache passes through lvseg_percent... */
	if (lv_is_cache(lv) || lv_is_used_cache_pool(lv) ||
	    (!lv_is_merging_origin(lv) && lv_is_raid(lv) && !seg_is_any_raid0(first_seg(lv))))
		percent = lvseg_percent_with_info_and_seg_status(lvdm, PERCENT_GET_DIRTY);
	else if (lv_is_raid(lv) && !seg_is_any_raid0(first_seg(lv)))
		/* old way for percentage when merging snapshot into raid origin */
		(void) lv_raid_percent(lv, &percent);
	else if (((lv_is_mirror(lv) &&
		   lv_mirror_percent(lv->vg->cmd, lv, 0, &percent, NULL))) &&
		 (percent != DM_PERCENT_INVALID))
		percent = copy_percent(lv);

	return dm_report_field_percent(rh, field, &percent);
}

static int _raidsyncaction_disp(struct dm_report *rh __attribute__((unused)),
			     struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data,
			     void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *sync_action;

	if (lv_is_raid(lv) && lv_raid_sync_action(lv, &sync_action))
		return _field_string(rh, field, sync_action);

	return _field_set_value(field, "", NULL);
}

static int _raidmismatchcount_disp(struct dm_report *rh __attribute__((unused)),
				struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data,
				void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	uint64_t mismatch_count;

	if (lv_is_raid(lv) && lv_raid_mismatch_count(lv, &mismatch_count))
		return dm_report_field_uint64(rh, field, &mismatch_count);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _raidwritebehind_disp(struct dm_report *rh __attribute__((unused)),
			      struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data,
			      void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv_is_raid_type(lv) && first_seg(lv)->writebehind)
		return dm_report_field_uint32(rh, field, &first_seg(lv)->writebehind);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _raidminrecoveryrate_disp(struct dm_report *rh __attribute__((unused)),
				   struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data,
				   void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv_is_raid_type(lv) && first_seg(lv)->min_recovery_rate)
		return dm_report_field_uint32(rh, field,
					      &first_seg(lv)->min_recovery_rate);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _raidmaxrecoveryrate_disp(struct dm_report *rh __attribute__((unused)),
				   struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data,
				   void *private __attribute__((unused)))
{
	const struct logical_volume *lv = (const struct logical_volume *) data;

	if (lv_is_raid_type(lv) && first_seg(lv)->max_recovery_rate)
		return dm_report_field_uint32(rh, field,
					      &first_seg(lv)->max_recovery_rate);

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _datapercent_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	dm_percent_t percent = lvseg_percent_with_info_and_seg_status(lvdm, PERCENT_GET_DATA);

	return dm_report_field_percent(rh, field, &percent);
}

static int _metadatapercent_disp(struct dm_report *rh,
				 struct dm_pool *mem __attribute__((unused)),
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	dm_percent_t percent;

	switch (lvdm->seg_status.type) {
	case SEG_STATUS_CACHE:
	case SEG_STATUS_THIN_POOL:
		percent = lvseg_percent_with_info_and_seg_status(lvdm, PERCENT_GET_METADATA);
		break;
	default:
                percent = DM_PERCENT_INVALID;
	}

	return dm_report_field_percent(rh, field, &percent);
}

static int _lvmetadatasize_disp(struct dm_report *rh, struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	uint64_t size;

	if (lv_is_thin_pool(lv) || lv_is_cache_pool(lv)) {
		size = lv_metadata_size(lv);
		return _size64_disp(rh, mem, field, &size, private);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _thincount_disp(struct dm_report *rh, struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;
	uint32_t count;

	if (seg_is_thin_pool(seg)) {
		count = dm_list_size(&seg->lv->segs_using_this_lv);
		return _uint32_disp(rh, mem, field, &count, private);
	}

	return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64));
}

static int _lvtime_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;
	uint64_t *sortval;

	if (!(repstr = lv_creation_time_dup(mem, lv, 0)) ||
	    !(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
		log_error("Failed to allocate buffer for time.");
		return 0;
	}

	*sortval = lv_is_historical(lv) ? lv->this_glv->historical->timestamp : lv->timestamp;
	return _field_set_value(field, repstr, sortval);
}

static int _lvtimeremoved_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;
	uint64_t *sortval;

	if (!(repstr = lv_removal_time_dup(mem, lv, 0)) ||
	    !(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
		log_error("Failed to allocate buffer for time.");
		return 0;
	}

	*sortval = lv_is_historical(lv) ? lv->this_glv->historical->timestamp_removed : 0;
	return _field_set_value(field, repstr, sortval);
}

static int _lvhost_disp(struct dm_report *rh, struct dm_pool *mem,
			struct dm_report_field *field,
			const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	char *repstr;

	if (!(repstr = lv_host_dup(mem, lv))) {
		log_error("Failed to allocate buffer for host.");
		return 0;
	}

	return _field_set_value(field, repstr, NULL);
}

/* PV/VG/LV Attributes */

static int _pvallocatable_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	int allocatable = (((const struct physical_volume *) data)->status & ALLOCATABLE_PV) != 0;
	return _binary_disp(rh, mem, field, allocatable, GET_FIRST_RESERVED_NAME(pv_allocatable_y), private);
}

static int _pvexported_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	int exported = (((const struct physical_volume *) data)->status & EXPORTED_VG) != 0;
	return _binary_disp(rh, mem, field, exported, GET_FIRST_RESERVED_NAME(pv_exported_y), private);
}

static int _pvmissing_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	int missing = (((const struct physical_volume *) data)->status & MISSING_PV) != 0;
	return _binary_disp(rh, mem, field, missing, GET_FIRST_RESERVED_NAME(pv_missing_y), private);
}

static int _pvinuse_disp(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field,
			 const void *data, void *private)
{
	const struct physical_volume *pv = (const struct physical_volume *) data;
	int used = is_used_pv(pv);

	if (used < 0)
		return _binary_undef_disp(rh, mem, field, private);

	return _binary_disp(rh, mem, field, used, GET_FIRST_RESERVED_NAME(pv_in_use_y), private);
}

static int _pvduplicate_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	const struct physical_volume *pv = (const struct physical_volume *) data;
	int duplicate = lvmcache_dev_is_unchosen_duplicate(pv->dev);

	return _binary_disp(rh, mem, field, duplicate, GET_FIRST_RESERVED_NAME(pv_duplicate_y), private);
}

static int _vgpermissions_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const char *perms = ((const struct volume_group *) data)->status & LVM_WRITE ? GET_FIRST_RESERVED_NAME(vg_permissions_rw)
										     : GET_FIRST_RESERVED_NAME(vg_permissions_r);
	return _field_string(rh, field, perms);
}

static int _vgextendable_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	int extendable = (vg_is_resizeable((const struct volume_group *) data)) != 0;
	return _binary_disp(rh, mem, field, extendable, GET_FIRST_RESERVED_NAME(vg_extendable_y),private);
}

static int _vgexported_disp(struct dm_report *rh, struct dm_pool *mem,
			    struct dm_report_field *field,
			    const void *data, void *private)
{
	int exported = (vg_is_exported((const struct volume_group *) data)) != 0;
	return _binary_disp(rh, mem, field, exported, GET_FIRST_RESERVED_NAME(vg_exported_y), private);
}

static int _vgpartial_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	int partial = (vg_missing_pv_count((const struct volume_group *) data)) != 0;
	return _binary_disp(rh, mem, field, partial, GET_FIRST_RESERVED_NAME(vg_partial_y), private);
}

static int _vgallocationpolicy_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	const char *alloc_policy = get_alloc_string(((const struct volume_group *) data)->alloc) ? : _str_unknown;
	return _field_string(rh, field, alloc_policy);
}

static int _vgclustered_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	int clustered = (vg_is_clustered((const struct volume_group *) data)) != 0;
	return _binary_disp(rh, mem, field, clustered, GET_FIRST_RESERVED_NAME(vg_clustered_y), private);
}

static int _vgshared_disp(struct dm_report *rh, struct dm_pool *mem,
			  struct dm_report_field *field,
			  const void *data, void *private)
{
	int shared = (vg_is_shared((const struct volume_group *) data)) != 0;
	return _binary_disp(rh, mem, field, shared, GET_FIRST_RESERVED_NAME(vg_shared_y), private);
}

static int _lvlayout_disp(struct dm_report *rh, struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct dm_list *lv_layout;
	struct dm_list *lv_role;

	if (!lv_layout_and_role(mem, lv, &lv_layout, &lv_role)) {
		log_error("Failed to display layout for LV %s/%s.", lv->vg->name, lv->name);
		return 0;
	}

	return _field_set_string_list(rh, field, lv_layout, private, 0, NULL);
}

static int _lvrole_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	struct dm_list *lv_layout;
	struct dm_list *lv_role;

	if (!lv_layout_and_role(mem, lv, &lv_layout, &lv_role)) {
		log_error("Failed to display role for LV %s/%s.", lv->vg->name, lv->name);
		return 0;
	}

	return _field_set_string_list(rh, field, lv_role, private, 0, NULL);
}

static int _lvinitialimagesync_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int initial_image_sync;

	if (lv_is_raid(lv) || lv_is_mirrored(lv))
		initial_image_sync = !lv_is_not_synced(lv);
	else
		initial_image_sync = 0;

	return _binary_disp(rh, mem, field, initial_image_sync, GET_FIRST_RESERVED_NAME(lv_initial_image_sync_y), private);
}

static int _lvimagesynced_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int image_synced;

	if (lv_is_raid_image(lv))
		image_synced = !lv_is_visible(lv) && lv_raid_image_in_sync(lv);
	else if (lv_is_mirror_image(lv))
		image_synced = lv_mirror_image_in_sync(lv);
	else
		image_synced = 0;

	return _binary_disp(rh, mem, field, image_synced, GET_FIRST_RESERVED_NAME(lv_image_synced_y), private);
}

static int _lvmerging_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int merging;

	if (lv_is_origin(lv) || lv_is_external_origin(lv))
		merging = lv_is_merging_origin(lv);
	else if (lv_is_cow(lv))
		merging = lv_is_merging_cow(lv);
	else if (lv_is_thin_volume(lv))
		merging = lv_is_merging_thin_snapshot(lv);
	else
		merging = 0;

	return _binary_disp(rh, mem, field, merging, GET_FIRST_RESERVED_NAME(lv_merging_y), private);
}

static int _lvconverting_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	int converting = lv_is_converting((const struct logical_volume *) data);

	return _binary_disp(rh, mem, field, converting, "converting", private);
}

static int _lvpermissions_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	const char *perms = "";

	if (!lv_is_pvmove(lvdm->lv)) {
		if (lvdm->lv->status & LVM_WRITE) {
			if (!lvdm->info.exists)
				perms = _str_unknown;
			else if (lvdm->info.read_only)
				perms = GET_FIRST_RESERVED_NAME(lv_permissions_r_override);
			else
				perms = GET_FIRST_RESERVED_NAME(lv_permissions_rw);
		} else if (lvdm->lv->status & LVM_READ)
			perms = GET_FIRST_RESERVED_NAME(lv_permissions_r);
		else
			perms = _str_unknown;
	}

	return _field_string(rh, field, perms);
}

static int _lvallocationpolicy_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	const char *alloc_policy = get_alloc_string(((const struct logical_volume *) data)->alloc) ? : _str_unknown;
	return _field_string(rh, field, alloc_policy);
}

static int _lvallocationlocked_disp(struct dm_report *rh, struct dm_pool *mem,
				    struct dm_report_field *field,
				    const void *data, void *private)
{
	int alloc_locked = (((const struct logical_volume *) data)->status & LOCKED) != 0;

	return _binary_disp(rh, mem, field, alloc_locked, GET_FIRST_RESERVED_NAME(lv_allocation_locked_y), private);
}

static int _lvfixedminor_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	int fixed_minor = (((const struct logical_volume *) data)->status & FIXED_MINOR) != 0;

	return _binary_disp(rh, mem, field, fixed_minor, GET_FIRST_RESERVED_NAME(lv_fixed_minor_y), private);
}

static int _lvactive_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	char *repstr;

	if (!(repstr = lv_active_dup(mem, (const struct logical_volume *) data))) {
		log_error("Failed to allocate buffer for active.");
		return 0;
	}

	return _field_set_value(field, repstr, NULL);
}

static int _lvactivelocally_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int active_locally;

	if (!activation())
		return _binary_undef_disp(rh, mem, field, private);

	active_locally = lv_is_active(lv);

	return _binary_disp(rh, mem, field, active_locally, GET_FIRST_RESERVED_NAME(lv_active_locally_y), private);
}

static int _lvactiveremotely_disp(struct dm_report *rh, struct dm_pool *mem,
				  struct dm_report_field *field,
				  const void *data, void *private)
{
	int active_remotely;

	if (!activation())
		return _binary_undef_disp(rh, mem, field, private);

	active_remotely = 0;

	return _binary_disp(rh, mem, field, active_remotely, GET_FIRST_RESERVED_NAME(lv_active_remotely_y), private);
}

static int _lvactiveexclusively_disp(struct dm_report *rh, struct dm_pool *mem,
				     struct dm_report_field *field,
				     const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	int active_exclusively;

	if (!activation())
		return _binary_undef_disp(rh, mem, field, private);

	active_exclusively = lv_is_active(lv);

	return _binary_disp(rh, mem, field, active_exclusively, GET_FIRST_RESERVED_NAME(lv_active_exclusively_y), private);
}

static int _lvmergefailed_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->seg_status.type != SEG_STATUS_SNAPSHOT)
		return _binary_undef_disp(rh, mem, field, private);

	return _binary_disp(rh, mem, field, lvdm->seg_status.snapshot->merge_failed,
			    GET_FIRST_RESERVED_NAME(lv_merge_failed_y), private);
}

static int _lvsnapshotinvalid_disp(struct dm_report *rh, struct dm_pool *mem,
				   struct dm_report_field *field,
				   const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->seg_status.type != SEG_STATUS_SNAPSHOT)
		return _binary_undef_disp(rh, mem, field, private);

	return _binary_disp(rh, mem, field, lvdm->seg_status.snapshot->invalid,
			    GET_FIRST_RESERVED_NAME(lv_snapshot_invalid_y), private);
}

static int _lvsuspended_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists)
		return _binary_disp(rh, mem, field, lvdm->info.suspended, GET_FIRST_RESERVED_NAME(lv_suspended_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _lvlivetable_disp(struct dm_report *rh, struct dm_pool *mem,
			     struct dm_report_field *field,
			     const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists)
		return _binary_disp(rh, mem, field, lvdm->info.live_table, GET_FIRST_RESERVED_NAME(lv_live_table_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _lvinactivetable_disp(struct dm_report *rh, struct dm_pool *mem,
				 struct dm_report_field *field,
				 const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists)
		return _binary_disp(rh, mem, field, lvdm->info.inactive_table, GET_FIRST_RESERVED_NAME(lv_inactive_table_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _lvdeviceopen_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lvdm->info.exists)
		return _binary_disp(rh, mem, field, lvdm->info.open_count, GET_FIRST_RESERVED_NAME(lv_device_open_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _thinzero_disp(struct dm_report *rh, struct dm_pool *mem,
			   struct dm_report_field *field,
			   const void *data, void *private)
{
	const struct lv_segment *seg = (const struct lv_segment *) data;

	if (seg_is_thin_volume(seg))
		seg = first_seg(seg->pool_lv);

	if (seg_is_thin_pool(seg))
		return _binary_disp(rh, mem, field, (seg->zero_new_blocks == THIN_ZERO_YES), GET_FIRST_RESERVED_NAME(zero_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _lvhealthstatus_disp(struct dm_report *rh, struct dm_pool *mem,
				struct dm_report_field *field,
				const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;
	const struct logical_volume *lv = lvdm->lv;
	const char *health = "";
	uint64_t n;

	if (lv_is_partial(lv))
		health = "partial";
	else if (lv_is_raid_type(lv)) {
		if (!activation())
			health = "unknown";
		else if (!lv_raid_healthy(lv))
			health = "refresh needed";
		else if (lv_is_raid(lv)) {
			if (lv_raid_mismatch_count(lv, &n) && n)
				health = "mismatches exist";
		} else if (lv->status & LV_WRITEMOSTLY)
			health = "writemostly";
	} else if (lv_is_cache(lv) && (lvdm->seg_status.type != SEG_STATUS_NONE)) {
		if (lvdm->seg_status.type != SEG_STATUS_CACHE)
			return _field_set_value(field, GET_FIRST_RESERVED_NAME(health_undef),
						GET_FIELD_RESERVED_VALUE(health_undef));
		if (lvdm->seg_status.cache->fail)
			health = "failed";
		else if (lvdm->seg_status.cache->read_only)
			health = "metadata_read_only";
	} else if (lv_is_thin_pool(lv) && (lvdm->seg_status.type != SEG_STATUS_NONE)) {
		if (lvdm->seg_status.type != SEG_STATUS_THIN_POOL)
			return _field_set_value(field, GET_FIRST_RESERVED_NAME(health_undef),
						GET_FIELD_RESERVED_VALUE(health_undef));
		if (lvdm->seg_status.thin_pool->fail)
			health = "failed";
		else if (lvdm->seg_status.thin_pool->out_of_data_space)
			health = "out_of_data";
		else if (lvdm->seg_status.thin_pool->read_only)
			health = "metadata_read_only";
	}

	return _field_string(rh, field, health);
}

static int _lvcheckneeded_disp(struct dm_report *rh, struct dm_pool *mem,
			       struct dm_report_field *field,
			       const void *data, void *private)
{
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data;

	if (lv_is_thin_pool(lvdm->lv) && lvdm->seg_status.type == SEG_STATUS_THIN_POOL)
		return _binary_disp(rh, mem, field, lvdm->seg_status.thin_pool->needs_check,
				    GET_FIRST_RESERVED_NAME(lv_check_needed_y), private);

	if (lv_is_cache(lvdm->lv) && lvdm->seg_status.type == SEG_STATUS_CACHE)
		return _binary_disp(rh, mem, field, lvdm->seg_status.cache->needs_check,
				    GET_FIRST_RESERVED_NAME(lv_check_needed_y), private);

	return _binary_undef_disp(rh, mem, field, private);
}

static int _lvskipactivation_disp(struct dm_report *rh, struct dm_pool *mem,
				  struct dm_report_field *field,
				  const void *data, void *private)
{
	int skip_activation = (((const struct logical_volume *) data)->status & LV_ACTIVATION_SKIP) != 0;
	return _binary_disp(rh, mem, field, skip_activation, "skip activation", private);
}

static int _lvhistorical_disp(struct dm_report *rh, struct dm_pool *mem,
			      struct dm_report_field *field,
			      const void *data, void *private)
{
	const struct logical_volume *lv = (const struct logical_volume *) data;
	return _binary_disp(rh, mem, field, lv_is_historical(lv), "historical", private);
}

/*
 * Macro to generate '_cache_<cache_status_field_name>_disp' reporting function.
 * The 'cache_status_field_name' is field name from struct dm_cache_status.
 */
#define GENERATE_CACHE_STATUS_DISP_FN(cache_status_field_name) \
static int _cache_ ## cache_status_field_name ## _disp (struct dm_report *rh, \
							struct dm_pool *mem, \
							struct dm_report_field *field, \
							const void *data, \
							void *private) \
{ \
	const struct lv_with_info_and_seg_status *lvdm = (const struct lv_with_info_and_seg_status *) data; \
	if (lvdm->seg_status.type != SEG_STATUS_CACHE) \
		return _field_set_value(field, "", &GET_TYPE_RESERVED_VALUE(num_undef_64)); \
	return dm_report_field_uint64(rh, field, &lvdm->seg_status.cache->cache_status_field_name); \
}

GENERATE_CACHE_STATUS_DISP_FN(total_blocks)
GENERATE_CACHE_STATUS_DISP_FN(used_blocks)
GENERATE_CACHE_STATUS_DISP_FN(dirty_blocks)
GENERATE_CACHE_STATUS_DISP_FN(read_hits)
GENERATE_CACHE_STATUS_DISP_FN(read_misses)
GENERATE_CACHE_STATUS_DISP_FN(write_hits)
GENERATE_CACHE_STATUS_DISP_FN(write_misses)

/* Report object types */

/* necessary for displaying something for PVs not belonging to VG */
static struct format_instance _dummy_fid = {
	.metadata_areas_in_use = DM_LIST_HEAD_INIT(_dummy_fid.metadata_areas_in_use),
	.metadata_areas_ignored = DM_LIST_HEAD_INIT(_dummy_fid.metadata_areas_ignored),
};

static struct volume_group _dummy_vg = {
	.fid = &_dummy_fid,
	.name = "",
	.system_id = (char *) "",
	.pvs = DM_LIST_HEAD_INIT(_dummy_vg.pvs),
	.lvs = DM_LIST_HEAD_INIT(_dummy_vg.lvs),
	.historical_lvs = DM_LIST_HEAD_INIT(_dummy_vg.historical_lvs),
	.tags = DM_LIST_HEAD_INIT(_dummy_vg.tags),
};

static struct volume_group _unknown_vg = {
	.fid = &_dummy_fid,
	.name = "[unknown]",
	.system_id = (char *) "",
	.pvs = DM_LIST_HEAD_INIT(_unknown_vg.pvs),
	.lvs = DM_LIST_HEAD_INIT(_unknown_vg.lvs),
	.historical_lvs = DM_LIST_HEAD_INIT(_unknown_vg.historical_lvs),
	.tags = DM_LIST_HEAD_INIT(_unknown_vg.tags),
};

static void *_obj_get_vg(void *obj)
{
	struct volume_group *vg = ((struct lvm_report_object *)obj)->vg;

	return vg ? vg : &_dummy_vg;
}

static void *_obj_get_lv(void *obj)
{
	return (struct logical_volume *)((struct lvm_report_object *)obj)->lvdm->lv;
}

static void *_obj_get_lv_with_info_and_seg_status(void *obj)
{
	return ((struct lvm_report_object *)obj)->lvdm;
}

static void *_obj_get_pv(void *obj)
{
	return ((struct lvm_report_object *)obj)->pv;
}

static void *_obj_get_label(void *obj)
{
	return ((struct lvm_report_object *)obj)->label;
}

static void *_obj_get_seg(void *obj)
{
	return ((struct lvm_report_object *)obj)->seg;
}

static void *_obj_get_pvseg(void *obj)
{
	return ((struct lvm_report_object *)obj)->pvseg;
}

static void *_obj_get_devtypes(void *obj)
{
	return obj;
}

static void *_obj_get_cmdlog(void *obj)
{
	return obj;
}

static const struct dm_report_object_type _log_report_types[] = {
	{ CMDLOG, "Command Log", "log_", _obj_get_cmdlog },
	{ 0, "", "", NULL },
};

static const struct dm_report_object_type _report_types[] = {
	{ VGS, "Volume Group", "vg_", _obj_get_vg },
	{ LVS, "Logical Volume", "lv_", _obj_get_lv },
	{ LVSINFO, "Logical Volume Device Info", "lv_", _obj_get_lv_with_info_and_seg_status },
	{ LVSSTATUS, "Logical Volume Device Status", "lv_", _obj_get_lv_with_info_and_seg_status },
	{ LVSINFOSTATUS, "Logical Volume Device Info and Status Combined", "lv_", _obj_get_lv_with_info_and_seg_status },
	{ PVS, "Physical Volume", "pv_", _obj_get_pv },
	{ LABEL, "Physical Volume Label", "pv_", _obj_get_label },
	{ SEGS, "Logical Volume Segment", "seg_", _obj_get_seg },
	{ PVSEGS, "Physical Volume Segment", "pvseg_", _obj_get_pvseg },
	{ 0, "", "", NULL },
};

static const struct dm_report_object_type _devtypes_report_types[] = {
	{ DEVTYPES, "Device Types", "devtype_", _obj_get_devtypes },
	{ 0, "", "", NULL },
};

/*
 * Import column definitions
 */

#define STR DM_REPORT_FIELD_TYPE_STRING
#define NUM DM_REPORT_FIELD_TYPE_NUMBER
#define BIN DM_REPORT_FIELD_TYPE_NUMBER
#define SIZ DM_REPORT_FIELD_TYPE_SIZE
#define PCT DM_REPORT_FIELD_TYPE_PERCENT
#define TIM DM_REPORT_FIELD_TYPE_TIME
#define STR_LIST DM_REPORT_FIELD_TYPE_STRING_LIST
#define SNUM DM_REPORT_FIELD_TYPE_NUMBER
#define FIELD(type, strct, sorttype, head, field, width, func, id, desc, writeable) \
	{type, sorttype, offsetof(type_ ## strct, field), (width) ? : sizeof(head) - 1, \
	 #id, head, &_ ## func ## _disp, desc},

typedef struct cmd_log_item type_cmd_log_item;

typedef struct physical_volume type_pv;
typedef struct logical_volume type_lv;
typedef struct volume_group type_vg;
typedef struct lv_segment type_seg;
typedef struct pv_segment type_pvseg;
typedef struct label type_label;

typedef dev_known_type_t type_devtype;

static const struct dm_report_field_type _fields[] = {
#include "columns.h"
{0, 0, 0, 0, "", "", NULL, NULL},
};

static const struct dm_report_field_type _devtypes_fields[] = {
#include "columns-devtypes.h"
{0, 0, 0, 0, "", "", NULL, NULL},
};

static const struct dm_report_field_type _log_fields[] = {
#include "columns-cmdlog.h"
{0, 0, 0, 0, "", "", NULL, NULL},
};

#undef STR
#undef NUM
#undef BIN
#undef SIZ
#undef STR_LIST
#undef SNUM
#undef FIELD

void *report_init(struct cmd_context *cmd, const char *format, const char *keys,
		  report_type_t *report_type, const char *separator,
		  int aligned, int buffered, int headings, int field_prefixes,
		  int quoted, int columns_as_rows, const char *selection,
		  int multiple_output)
{
	uint32_t report_flags = 0;
	const struct dm_report_object_type *types;
	const struct dm_report_field_type *fields;
	const struct dm_report_reserved_value *reserved_values;
	void *rh;

	if (aligned)
		report_flags |= DM_REPORT_OUTPUT_ALIGNED;

	if (buffered)
		report_flags |= DM_REPORT_OUTPUT_BUFFERED;

	if (headings)
		report_flags |= DM_REPORT_OUTPUT_HEADINGS;

	if (field_prefixes)
		report_flags |= DM_REPORT_OUTPUT_FIELD_NAME_PREFIX;

	if (!quoted)
		report_flags |= DM_REPORT_OUTPUT_FIELD_UNQUOTED;

	if (columns_as_rows)
		report_flags |= DM_REPORT_OUTPUT_COLUMNS_AS_ROWS;

	if (multiple_output)
		report_flags |= DM_REPORT_OUTPUT_MULTIPLE_TIMES;

	if (*report_type & CMDLOG) {
		types = _log_report_types;
		fields = _log_fields;
		reserved_values = NULL;
	} else if (*report_type & DEVTYPES) {
		types = _devtypes_report_types;
		fields = _devtypes_fields;
		reserved_values = NULL;
	} else {
		types = _report_types;
		fields = _fields;
		reserved_values = _report_reserved_values;
	}

	rh = dm_report_init_with_selection(report_type, types, fields,
		format, separator, report_flags, keys,
		selection, reserved_values, cmd);

	if (rh && field_prefixes)
		dm_report_set_output_field_name_prefix(rh, "lvm2_");

	return rh;
}

void *report_init_for_selection(struct cmd_context *cmd,
				report_type_t *report_type,
				const char *selection_criteria)
{
	return dm_report_init_with_selection(report_type, _report_types, _fields,
					     "", DEFAULT_REP_SEPARATOR,
					     DM_REPORT_OUTPUT_FIELD_UNQUOTED,
					     "", selection_criteria,
					     _report_reserved_values,
					     cmd);
}

int report_get_prefix_and_desc(report_type_t report_type_id,
			       const char **report_prefix,
			       const char **report_desc)
{
	const struct dm_report_object_type *report_types, *report_type;

	if (report_type_id & CMDLOG)
		report_types = _log_report_types;
	else if (report_type_id & DEVTYPES)
		report_types = _devtypes_report_types;
	else
		report_types = _report_types;

	for (report_type = report_types; report_type->id; report_type++) {
		if (report_type_id & report_type->id) {
			*report_prefix = report_type->prefix;
			*report_desc = report_type->desc;
			return 1;
		}
	}

	*report_prefix = *report_desc = "";
	return 0;
}

/*
 * Create a row of data for an object
 */
int report_object(void *handle, int selection_only, const struct volume_group *vg,
		  const struct logical_volume *lv, const struct physical_volume *pv,
		  const struct lv_segment *seg, const struct pv_segment *pvseg,
		  const struct lv_with_info_and_seg_status *lvdm,
		  const struct label *label)
{
	struct selection_handle *sh = selection_only ? (struct selection_handle *) handle : NULL;
	struct device dummy_device = { .dev = 0 };
	struct label dummy_label = { .dev = &dummy_device };
	struct lvm_report_object obj = {
		.vg = (struct volume_group *) vg,
		.lvdm = (struct lv_with_info_and_seg_status *) lvdm,
		.pv = (struct physical_volume *) pv,
		.seg = (struct lv_segment *) seg,
		.pvseg = (struct pv_segment *) pvseg,
		.label = (struct label *) (label ? : (pv ? pv_label(pv) : NULL))
	};

	/* FIXME workaround for pv_label going through cache; remove once struct
	 * physical_volume gains a proper "label" pointer */
	if (!obj.label) {
		if (pv) {
			if (pv->fmt)
				dummy_label.labeller = pv->fmt->labeller;
			if (pv->dev)
				dummy_label.dev = pv->dev;
			else
				memcpy(dummy_device.pvid, &pv->id, ID_LEN);
		}
		obj.label = &dummy_label;
	}

	/* Never report orphan VGs. */
	if (vg && is_orphan_vg(vg->name)) {
		obj.vg = &_dummy_vg;
		if (pv)
			_dummy_fid.fmt = pv->fmt;
	}

	if (vg && is_orphan_vg(vg->name) && pv && is_used_pv(pv)) {
		obj.vg = &_unknown_vg;
		_dummy_fid.fmt = pv->fmt;
	}

	return sh ? dm_report_object_is_selected(sh->selection_rh, &obj, 0, &sh->selected)
		  : dm_report_object(handle, &obj);
}

static int _report_devtype_single(void *handle, const dev_known_type_t *devtype)
{
	return dm_report_object(handle, (void *)devtype);
}

int report_devtypes(void *handle)
{
	int devtypeind = 0;

	while (_dev_known_types[devtypeind].name[0])
		if (!_report_devtype_single(handle, &_dev_known_types[devtypeind++]))
			return 0;

	return 1;
}

int report_cmdlog(void *handle, const char *type, const char *context,
		  const char *object_type_name, const char *object_name,
		  const char *object_id, const char *object_group,
		  const char *object_group_id, const char *msg,
		  int current_errno, int ret_code)
{
	struct cmd_log_item log_item = {_log_seqnum++, type, context, object_type_name,
					object_name ? : "", object_id ? : "",
					object_group ? : "", object_group_id ? : "",
					msg ? : "", current_errno, ret_code};

	if (handle)
		return dm_report_object(handle, &log_item);

	return 1;
}

void report_reset_cmdlog_seqnum(void)
{
	_log_seqnum = 1;
}

int report_current_object_cmdlog(const char *type, const char *msg, int32_t ret_code)
{
	log_report_t log_state = log_get_report_state();

	return report_cmdlog(log_state.report, type, log_get_report_context_name(log_state.context),
			     log_get_report_object_type_name(log_state.object_type),
			     log_state.object_name, log_state.object_id,
			     log_state.object_group, log_state.object_group_id,
			     msg, stored_errno(), ret_code);
}
