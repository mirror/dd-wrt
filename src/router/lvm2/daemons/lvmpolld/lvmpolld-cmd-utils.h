/*
 * Copyright (C) 2015 Red Hat, Inc.
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

#ifndef _LVM_LVMPOLLD_CMD_UTILS_H
#define _LVM_LVMPOLLD_CMD_UTILS_H

#include "lvmpolld-data-utils.h"

const char **cmdargv_ctr(const struct lvmpolld_lv *pdlv, const char *lvm_binary, unsigned abort, unsigned handle_missing_pvs);
const char **cmdenvp_ctr(const struct lvmpolld_lv *pdlv);

const char *polling_op(enum poll_type);

#endif /* _LVM_LVMPOLLD_CMD_UTILS_H */
