/*
 * Copyright (C) 2010-2015 Red Hat, Inc. All rights reserved.
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

/*
 * Wrappers around liblvm2cmd functions for dmeventd plug-ins.
 *
 * liblvm2cmd is not thread-safe so the locking in this library helps dmeventd
 * threads to co-operate in sharing a single instance.
 *
 * FIXME Either support this properly as a generic liblvm2cmd wrapper or make
 * liblvm2cmd thread-safe so this can go away.
 */

#ifndef _DMEVENTD_LVMWRAP_H
#define _DMEVENTD_LVMWRAP_H

struct dm_pool;

int dmeventd_lvm2_init(void);
void dmeventd_lvm2_exit(void);
int dmeventd_lvm2_run(const char *cmdline);

void dmeventd_lvm2_lock(void);
void dmeventd_lvm2_unlock(void);

struct dm_pool *dmeventd_lvm2_pool(void);

int dmeventd_lvm2_command(struct dm_pool *mem, char *buffer, size_t size,
			  const char *cmd, const char *device);

#define dmeventd_lvm2_run_with_lock(cmdline) \
	({\
		int rc;\
		dmeventd_lvm2_lock();\
		rc = dmeventd_lvm2_run(cmdline);\
		dmeventd_lvm2_unlock();\
		rc;\
	})

#define dmeventd_lvm2_init_with_pool(name, st) \
	({\
		struct dm_pool *mem;\
		st = NULL;\
		if (dmeventd_lvm2_init()) {\
			if ((mem = dm_pool_create(name, 2048)) &&\
			    (st = dm_pool_zalloc(mem, sizeof(*st))))\
				st->mem = mem;\
			else {\
				if (mem)\
					dm_pool_destroy(mem);\
				dmeventd_lvm2_exit();\
			}\
		}\
		st;\
	})

#define dmeventd_lvm2_exit_with_pool(pool) \
	do {\
		dm_pool_destroy(pool->mem);\
		dmeventd_lvm2_exit();\
	} while(0)

#endif /* _DMEVENTD_LVMWRAP_H */
