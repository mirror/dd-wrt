/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2011 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include "mdadm.h"

/* name/number mappings */

mapping_t r5layout[] = {
	{ "left-asymmetric", ALGORITHM_LEFT_ASYMMETRIC},
	{ "right-asymmetric", ALGORITHM_RIGHT_ASYMMETRIC},
	{ "left-symmetric", ALGORITHM_LEFT_SYMMETRIC},
	{ "right-symmetric", ALGORITHM_RIGHT_SYMMETRIC},

	{ "default", ALGORITHM_LEFT_SYMMETRIC},
	{ "la", ALGORITHM_LEFT_ASYMMETRIC},
	{ "ra", ALGORITHM_RIGHT_ASYMMETRIC},
	{ "ls", ALGORITHM_LEFT_SYMMETRIC},
	{ "rs", ALGORITHM_RIGHT_SYMMETRIC},

	{ "parity-first", ALGORITHM_PARITY_0},
	{ "parity-last", ALGORITHM_PARITY_N},
	{ "ddf-zero-restart", ALGORITHM_RIGHT_ASYMMETRIC},
	{ "ddf-N-restart", ALGORITHM_LEFT_ASYMMETRIC},
	{ "ddf-N-continue", ALGORITHM_LEFT_SYMMETRIC},

	{ NULL, UnSet }
};
mapping_t r6layout[] = {
	{ "left-asymmetric", ALGORITHM_LEFT_ASYMMETRIC},
	{ "right-asymmetric", ALGORITHM_RIGHT_ASYMMETRIC},
	{ "left-symmetric", ALGORITHM_LEFT_SYMMETRIC},
	{ "right-symmetric", ALGORITHM_RIGHT_SYMMETRIC},

	{ "default", ALGORITHM_LEFT_SYMMETRIC},
	{ "la", ALGORITHM_LEFT_ASYMMETRIC},
	{ "ra", ALGORITHM_RIGHT_ASYMMETRIC},
	{ "ls", ALGORITHM_LEFT_SYMMETRIC},
	{ "rs", ALGORITHM_RIGHT_SYMMETRIC},

	{ "parity-first", ALGORITHM_PARITY_0},
	{ "parity-last", ALGORITHM_PARITY_N},
	{ "ddf-zero-restart", ALGORITHM_ROTATING_ZERO_RESTART},
	{ "ddf-N-restart", ALGORITHM_ROTATING_N_RESTART},
	{ "ddf-N-continue", ALGORITHM_ROTATING_N_CONTINUE},

	{ "left-asymmetric-6", ALGORITHM_LEFT_ASYMMETRIC_6},
	{ "right-asymmetric-6", ALGORITHM_RIGHT_ASYMMETRIC_6},
	{ "left-symmetric-6", ALGORITHM_LEFT_SYMMETRIC_6},
	{ "right-symmetric-6", ALGORITHM_RIGHT_SYMMETRIC_6},
	{ "parity-first-6", ALGORITHM_PARITY_0_6},

	{ NULL, UnSet }
};

mapping_t pers[] = {
	{ "linear", LEVEL_LINEAR},
	{ "raid0", 0},
	{ "0", 0},
	{ "stripe", 0},
	{ "raid1", 1},
	{ "1", 1},
	{ "mirror", 1},
	{ "raid4", 4},
	{ "4", 4},
	{ "raid5", 5},
	{ "5", 5},
	{ "multipath", LEVEL_MULTIPATH},
	{ "mp", LEVEL_MULTIPATH},
	{ "raid6", 6},
	{ "6", 6},
	{ "raid10", 10},
	{ "10", 10},
	{ "faulty", LEVEL_FAULTY},
	{ "container", LEVEL_CONTAINER},
	{ NULL, UnSet }
};

mapping_t modes[] = {
	{ "assemble", ASSEMBLE},
	{ "build", BUILD},
	{ "create", CREATE},
	{ "manage", MANAGE},
	{ "misc", MISC},
	{ "monitor", MONITOR},
	{ "grow", GROW},
	{ "incremental", INCREMENTAL},
	{ "auto-detect", AUTODETECT},
	{ NULL, UnSet }
};

mapping_t faultylayout[] = {
	{ "write-transient", WriteTransient },
	{ "wt", WriteTransient },
	{ "read-transient", ReadTransient },
	{ "rt", ReadTransient },
	{ "write-persistent", WritePersistent },
	{ "wp", WritePersistent },
	{ "read-persistent", ReadPersistent },
	{ "rp", ReadPersistent },
	{ "write-all", WriteAll },
	{ "wa", WriteAll },
	{ "read-fixable", ReadFixable },
	{ "rf", ReadFixable },

	{ "clear", ClearErrors},
	{ "flush", ClearFaults},
	{ "none", ClearErrors},
	{ "default", ClearErrors},
	{ NULL, UnSet }
};

mapping_t consistency_policies[] = {
	{ "unknown", CONSISTENCY_POLICY_UNKNOWN},
	{ "none", CONSISTENCY_POLICY_NONE},
	{ "resync", CONSISTENCY_POLICY_RESYNC},
	{ "bitmap", CONSISTENCY_POLICY_BITMAP},
	{ "journal", CONSISTENCY_POLICY_JOURNAL},
	{ "ppl", CONSISTENCY_POLICY_PPL},
	{ NULL, CONSISTENCY_POLICY_UNKNOWN }
};

mapping_t sysfs_array_states[] = {
	{ "active-idle", ARRAY_ACTIVE_IDLE },
	{ "active", ARRAY_ACTIVE },
	{ "clear", ARRAY_CLEAR },
	{ "inactive", ARRAY_INACTIVE },
	{ "suspended", ARRAY_SUSPENDED },
	{ "readonly", ARRAY_READONLY },
	{ "read-auto", ARRAY_READ_AUTO },
	{ "clean", ARRAY_CLEAN },
	{ "write-pending", ARRAY_WRITE_PENDING },
	{ NULL, ARRAY_UNKNOWN_STATE }
};

char *map_num(mapping_t *map, int num)
{
	while (map->name) {
		if (map->num == num)
			return map->name;
		map++;
	}
	return NULL;
}

int map_name(mapping_t *map, char *name)
{
	while (map->name && strcmp(map->name, name) != 0)
		map++;

	return map->num;
}
