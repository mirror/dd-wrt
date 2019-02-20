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

#include "lib/misc/lib.h"
#include "lib/properties/prop_common.h"

int prop_not_implemented_get(const void *obj, struct lvm_property_type *prop)
{
	log_errno(ENOSYS, "Function not implemented");

	return 0;
}

int prop_not_implemented_set(void *obj, struct lvm_property_type *prop)
{
	log_errno(ENOSYS, "Function not implemented");

	return 0;
}

int prop_get_property(struct lvm_property_type *p, const void *obj,
		struct lvm_property_type *prop,
		unsigned type)
{
	while (p->id[0]) {
		if (!strcmp(p->id, prop->id))
			break;
		p++;
	}
	if (!p->id[0]) {
		log_errno(EINVAL, "Invalid property name %s", prop->id);
		return 0;
	}
	if (!(p->type & type)) {
		log_errno(EINVAL, "Property name %s does not match type %d",
			  prop->id, p->type);
		return 0;
	}

	*prop = *p;
	if (!p->get(obj, prop)) {
		return 0;
	}

	return 1;
}

int prop_set_property(struct lvm_property_type *p, void *obj,
		struct lvm_property_type *prop,
		unsigned type)
{
	while (p->id[0]) {
		if (!strcmp(p->id, prop->id))
			break;
		p++;
	}
	if (!p->id[0]) {
		log_errno(EINVAL, "Invalid property name %s", prop->id);
		return 0;
	}
	if (!p->is_settable) {
		log_errno(EINVAL, "Unable to set read-only property %s",
			  prop->id);
		return 0;
	}
	if (!(p->type & type)) {
		log_errno(EINVAL, "Property name %s does not match type %d",
			  prop->id, p->type);
		return 0;
	}

	if (p->is_string)
		p->value.string = prop->value.string;
	else
		p->value.integer = prop->value.integer;
	if (!p->set(obj, p)) {
		return 0;
	}

	return 1;
}
