/*
 * Copyright (C) 2010-2013 Red Hat, Inc. All rights reserved.
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
#ifndef _LVM_PROPERTIES_H
#define _LVM_PROPERTIES_H

#include "device_mapper/all.h"
#include "lib/metadata/metadata.h"
#include "lib/report/report.h"
#include "lib/properties/prop_common.h"

int lvseg_get_property(const struct lv_segment *lvseg,
		       struct lvm_property_type *prop);
int lv_get_property(const struct logical_volume *lv,
		    struct lvm_property_type *prop);
int vg_get_property(const struct volume_group *vg,
		    struct lvm_property_type *prop);
int pvseg_get_property(const struct pv_segment *pvseg,
		       struct lvm_property_type *prop);
int pv_get_property(const struct physical_volume *pv,
		    struct lvm_property_type *prop);
int lv_set_property(struct logical_volume *lv,
		    struct lvm_property_type *prop);
int vg_set_property(struct volume_group *vg,
		    struct lvm_property_type *prop);
int pv_set_property(struct physical_volume *pv,
		    struct lvm_property_type *prop);

#endif
