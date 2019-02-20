/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_LOCKING_H
#define _LVM_LOCKING_H

#include "lib/uuid/uuid.h"
#include "lib/config/config.h"

struct logical_volume;

int init_locking(struct cmd_context *cmd, int file_locking_sysinit, int file_locking_readonly, int file_locking_ignorefail);
void fin_locking(void);
void reset_locking(void);
int vg_write_lock_held(void);

/*
 *   Lock/unlock on-disk volume group data.
 *   Use VG_ORPHANS to lock all orphan PVs.
 *   Use VG_GLOBAL as a global lock and to wipe the internal cache.
 *   char *vol holds volume group name.
 *   If more than one lock needs to be held simultaneously, they must be
 *   acquired in alphabetical order of 'vol' (to avoid deadlocks), with
 *   VG_ORPHANS last.
 */
int lock_vol(struct cmd_context *cmd, const char *vol, uint32_t flags, const struct logical_volume *lv);

#define LCK_TYPE_MASK	0x00000007U
#define LCK_READ	0x00000001U
#define LCK_WRITE	0x00000004U
#define LCK_UNLOCK      0x00000006U

/*
 * Lock bits.
 * Bottom 8 bits except LCK_LOCAL form args[0] in cluster comms.
 */
#define LCK_NONBLOCK	0x00000010U	/* Don't block waiting for lock? */

/*
 * Special cases of VG locks.
 */
#define VG_ORPHANS	"#orphans"
#define VG_GLOBAL	"#global"

#define LCK_VG_READ		LCK_READ
#define LCK_VG_WRITE		LCK_WRITE
#define LCK_VG_UNLOCK		LCK_UNLOCK

#define unlock_vg(cmd, vg, vol)	\
	do { \
		if (is_real_vg(vol) && !sync_local_dev_names(cmd)) \
			stack; \
		if (!lock_vol(cmd, vol, LCK_VG_UNLOCK, NULL)) \
			stack;	\
	} while (0)
#define unlock_and_release_vg(cmd, vg, vol) \
	do { \
		unlock_vg(cmd, vg, vol); \
		release_vg(vg); \
	} while (0)

int sync_local_dev_names(struct cmd_context* cmd);

/* Process list of LVs */
struct volume_group;
int activate_lvs(struct cmd_context *cmd, struct dm_list *lvs, unsigned exclusive);

#endif
