/*
 * Copyright (C) 2013 Red Hat, Inc. All rights reserved.
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
#ifndef _LVM_PROP_COMMON_H
#define _LVM_PROP_COMMON_H

#include <stdint.h>

/*
 * Common code for getting and setting properties.
 */

struct lvm_property_type {
	unsigned type;
	const char *id;
	unsigned is_settable:1;
	unsigned is_string:1;
	unsigned is_integer:1;
	unsigned is_signed:1;
	union {
		const char *string;
		uint64_t integer;
		int64_t signed_integer;
	} value;
	int (*get) (const void *obj, struct lvm_property_type *prop);
	int (*set) (void *obj, struct lvm_property_type *prop);
};

int prop_not_implemented_get(const void *obj, struct lvm_property_type *prop);
int prop_not_implemented_set(void *obj, struct lvm_property_type *prop);

int prop_get_property(struct lvm_property_type *p, const void *obj,
		struct lvm_property_type *prop,
		unsigned type);
int prop_set_property(struct lvm_property_type *p, void *obj,
		struct lvm_property_type *prop,
		unsigned type);

#define GET_NUM_PROPERTY_FN(NAME, VALUE, TYPE, VAR)			\
static int _ ## NAME ## _get (const void *obj, struct lvm_property_type *prop) \
{ \
	const struct TYPE *VAR = (const struct TYPE *)obj; \
\
	prop->value.integer = VALUE; \
	return 1; \
}

#define SET_NUM_PROPERTY_FN(NAME, SETFN, TYPE, VAR)			\
static int _ ## NAME ## _set (void *obj, struct lvm_property_type *prop) \
{ \
	struct TYPE *VAR = (struct TYPE *)obj; \
\
	SETFN(VAR, prop->value.integer);		\
	return 1; \
}

#define SET_NUM_PROPERTY(NAME, VALUE, TYPE, VAR)			\
static int _ ## NAME ## _set (void *obj, struct lvm_property_type *prop) \
{ \
	struct TYPE *VAR = (struct TYPE *)obj; \
\
	VALUE = prop->value.integer;		\
	return 1; \
}

#define GET_STR_PROPERTY_FN(NAME, VALUE, TYPE, VAR)			\
static int _ ## NAME ## _get (const void *obj, struct lvm_property_type *prop) \
{ \
	const struct TYPE *VAR = (const struct TYPE *)obj; \
\
	prop->value.string = (char *)VALUE;	\
	return 1; \
}

/*
 * The 'FIELD' macro arguments are defined as follows:
 * 1. report_type.  An enum value that selects a specific
 * struct dm_report_object_type in the _report_types array.  The value is
 * used to select the containing base object address (see *obj_get*
 * functions) for any data values of any field in the report.
 * 2. Containing struct.  The structure that either contains the field data
 * as a member or should be used to obtain the field data.  The containing
 * struct should match the base object of the report_type.
 * 3. Field type.  This must be either 'STR' or 'NUM'.
 * 4. Report heading.  This is the field heading that is displayed by the
 * reporting commands.
 * 5. Data value pointer.  This argument is always a member of the
 * containing struct.  It may point directly to the data value (for example,
 * lv_uuid - see _uuid_disp()) or may be used to derive the data value (for
 * example, seg_count - see _lvsegcount_disp()).  In the FIELD macro
 * definition, it is used in an offset calculation to derive the offset to
 * the data value from the containing struct base address.  Note that in some
 * cases, the argument is the first member of the struct, in which case the
 * data value pointer points to the start of the struct itself (for example,
 * 'lvid' field of struct 'lv').
 * 6. Minimum display width.  This is the minimum width used to display
 * the field value, typically matching the width of the column heading.
 * 7. Display function identifier.  Used to derive the full name of the
 * function that displays this field.  Derivation is done by appending '_'
 * then prepending this argument to '_disp'.  For example, if this argument
 * is 'uuid', the display function is _uuid_disp().  Adding a new field may
 * require defining a new display function (for example _myfieldname_disp()),
 * or re-use of an existing one (for example, _uint32_disp()).
 * 8. Unique format identifier / field id.  This name must be unique and is
 * used to select fields via '-o' in the reporting commands (pvs/vgs/lvs).
 * The string used to specify the field - the 'id' member of
 * struct dm_report_field_type.
 * 9. Description of field.  This is a brief (ideally <= 52 chars) description
 * of the field used in the reporting commands.
 * 10. Flags.
 *     FIELD_MODIFIABLE.  A '_set' function exists to change the field's value.
 *     The function name is derived in a similar way to item 7 above.
 */

#define STR 1
#define NUM 2
#define BIN 3
#define SIZ 4
#define PCT 5
#define TIM 6
#define SNUM 7              /* Signed Number */
#define STR_LIST 8

#define FIELD_MODIFIABLE 0x00000001
#define FIELD(type, strct, field_type, head, field, width, fn, id, desc, settable) \
	{ type, #id, settable, (field_type == STR || field_type == STR_LIST), ((field_type == NUM) || (field_type == BIN) || (field_type == SIZ) || (field_type == PCT) || (field_type == SNUM)), ((field_type == SNUM) || (field_type == PCT)), { .integer = 0 }, _ ## id ## _get, _ ## id ## _set },

#endif
