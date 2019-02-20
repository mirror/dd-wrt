/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
 * Copyright (C) 2006 Rackable Systems All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef LIB_DEVICE_MAPPER_H
#define LIB_DEVICE_MAPPER_H

#include "configure.h"

#include "base/data-struct/list.h"
#include "base/data-struct/hash.h"
#include "vdo/target.h"

#include <inttypes.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#  include <linux/types.h>
#endif

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __GNUC__
# define __typeof__ typeof
#endif

/* Macros to make string defines */
#define DM_TO_STRING_EXP(A) #A
#define DM_TO_STRING(A) DM_TO_STRING_EXP(A)

#define DM_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/*****************************************************************
 * The first section of this file provides direct access to the
 * individual device-mapper ioctls.  Since it is quite laborious to
 * build the ioctl arguments for the device-mapper, people are
 * encouraged to use this library.
 ****************************************************************/

/*
 * The library user may wish to register their own
 * logging function.  By default errors go to stderr.
 * Use dm_log_with_errno_init(NULL) to restore the default log fn.
 * Error messages may have a non-zero errno.
 * Debug messages may have a non-zero class.
 * Aborts on internal error when env DM_ABORT_ON_INTERNAL_ERRORS is 1
 */

typedef void (*dm_log_with_errno_fn) (int level, const char *file, int line,
				      int dm_errno_or_class, const char *f, ...)
    __attribute__ ((format(printf, 5, 6)));

void dm_log_with_errno_init(dm_log_with_errno_fn fn);
void dm_log_init_verbose(int level);

/*
 * Original version of this function.
 * dm_errno is set to 0.
 *
 * Deprecated: Use the _with_errno_ versions above instead.
 */
typedef void (*dm_log_fn) (int level, const char *file, int line,
			   const char *f, ...)
    __attribute__ ((format(printf, 4, 5)));

void dm_log_init(dm_log_fn fn);
/*
 * For backward-compatibility, indicate that dm_log_init() was used
 * to set a non-default value of dm_log().
 */
int dm_log_is_non_default(void);

/*
 * Number of devices currently in suspended state (via the library).
 */
int dm_get_suspended_counter(void);

enum {
	DM_DEVICE_CREATE,
	DM_DEVICE_RELOAD,
	DM_DEVICE_REMOVE,
	DM_DEVICE_REMOVE_ALL,

	DM_DEVICE_SUSPEND,
	DM_DEVICE_RESUME,

	DM_DEVICE_INFO,
	DM_DEVICE_DEPS,
	DM_DEVICE_RENAME,

	DM_DEVICE_VERSION,

	DM_DEVICE_STATUS,
	DM_DEVICE_TABLE,
	DM_DEVICE_WAITEVENT,

	DM_DEVICE_LIST,

	DM_DEVICE_CLEAR,

	DM_DEVICE_MKNODES,

	DM_DEVICE_LIST_VERSIONS,

	DM_DEVICE_TARGET_MSG,

	DM_DEVICE_SET_GEOMETRY,

	DM_DEVICE_ARM_POLL
};

/*
 * You will need to build a struct dm_task for
 * each ioctl command you want to execute.
 */

struct dm_pool;
struct dm_task;
struct dm_timestamp;

struct dm_task *dm_task_create(int type);
void dm_task_destroy(struct dm_task *dmt);

int dm_task_set_name(struct dm_task *dmt, const char *name);
int dm_task_set_uuid(struct dm_task *dmt, const char *uuid);

/*
 * Retrieve attributes after an info.
 */
struct dm_info {
	int exists;
	int suspended;
	int live_table;
	int inactive_table;
	int32_t open_count;
	uint32_t event_nr;
	uint32_t major;
	uint32_t minor;		/* minor device number */
	int read_only;		/* 0:read-write; 1:read-only */

	int32_t target_count;

	int deferred_remove;
	int internal_suspend;
};

struct dm_deps {
	uint32_t count;
	uint32_t filler;
	uint64_t device[0];
};

struct dm_names {
	uint64_t dev;
	uint32_t next;		/* Offset to next struct from start of this struct */
	char name[0];
};

struct dm_versions {
	uint32_t next;		/* Offset to next struct from start of this struct */
	uint32_t version[3];

	char name[0];
};

int dm_get_library_version(char *version, size_t size);
int dm_task_get_driver_version(struct dm_task *dmt, char *version, size_t size);
int dm_task_get_info(struct dm_task *dmt, struct dm_info *dmi);

/*
 * This function returns dm device's UUID based on the value
 * of the mangling mode set during preceding dm_task_run call:
 *   - unmangled UUID for DM_STRING_MANGLING_{AUTO, HEX},
 *   - UUID without any changes for DM_STRING_MANGLING_NONE.
 *
 * To get mangled or unmangled form of the UUID directly, use
 * dm_task_get_uuid_mangled or dm_task_get_uuid_unmangled function.
 */
const char *dm_task_get_uuid(const struct dm_task *dmt);

struct dm_deps *dm_task_get_deps(struct dm_task *dmt);
struct dm_versions *dm_task_get_versions(struct dm_task *dmt);
const char *dm_task_get_message_response(struct dm_task *dmt);

/*
 * These functions return device-mapper names based on the value
 * of the mangling mode set during preceding dm_task_run call:
 *   - unmangled name for DM_STRING_MANGLING_{AUTO, HEX},
 *   - name without any changes for DM_STRING_MANGLING_NONE.
 *
 * To get mangled or unmangled form of the name directly, use
 * dm_task_get_name_mangled or dm_task_get_name_unmangled function.
 */
const char *dm_task_get_name(const struct dm_task *dmt);
struct dm_names *dm_task_get_names(struct dm_task *dmt);

int dm_task_set_ro(struct dm_task *dmt);
int dm_task_set_newname(struct dm_task *dmt, const char *newname);
int dm_task_set_newuuid(struct dm_task *dmt, const char *newuuid);
int dm_task_set_minor(struct dm_task *dmt, int minor);
int dm_task_set_major(struct dm_task *dmt, int major);
int dm_task_set_major_minor(struct dm_task *dmt, int major, int minor, int allow_default_major_fallback);
int dm_task_set_uid(struct dm_task *dmt, uid_t uid);
int dm_task_set_gid(struct dm_task *dmt, gid_t gid);
int dm_task_set_mode(struct dm_task *dmt, mode_t mode);
/* See also description for DM_UDEV_DISABLE_LIBRARY_FALLBACK flag! */
int dm_task_set_cookie(struct dm_task *dmt, uint32_t *cookie, uint16_t flags);
int dm_task_set_event_nr(struct dm_task *dmt, uint32_t event_nr);
int dm_task_set_geometry(struct dm_task *dmt, const char *cylinders, const char *heads, const char *sectors, const char *start);
int dm_task_set_message(struct dm_task *dmt, const char *message);
int dm_task_set_sector(struct dm_task *dmt, uint64_t sector);
int dm_task_no_flush(struct dm_task *dmt);
int dm_task_no_open_count(struct dm_task *dmt);
int dm_task_skip_lockfs(struct dm_task *dmt);
int dm_task_query_inactive_table(struct dm_task *dmt);
int dm_task_suppress_identical_reload(struct dm_task *dmt);
int dm_task_secure_data(struct dm_task *dmt);
int dm_task_retry_remove(struct dm_task *dmt);
int dm_task_deferred_remove(struct dm_task *dmt);

/*
 * Record timestamp immediately after the ioctl returns.
 */
int dm_task_set_record_timestamp(struct dm_task *dmt);
struct dm_timestamp *dm_task_get_ioctl_timestamp(struct dm_task *dmt);

/*
 * Enable checks for common mistakes such as issuing ioctls in an unsafe order.
 */
int dm_task_enable_checks(struct dm_task *dmt);

typedef enum {
	DM_ADD_NODE_ON_RESUME, /* add /dev/mapper node with dmsetup resume */
	DM_ADD_NODE_ON_CREATE  /* add /dev/mapper node with dmsetup create */
} dm_add_node_t;
int dm_task_set_add_node(struct dm_task *dmt, dm_add_node_t add_node);

/*
 * Control read_ahead.
 */
#define DM_READ_AHEAD_AUTO UINT32_MAX	/* Use kernel default readahead */
#define DM_READ_AHEAD_NONE 0		/* Disable readahead */

#define DM_READ_AHEAD_MINIMUM_FLAG	0x1	/* Value supplied is minimum */

/*
 * Read ahead is set with DM_DEVICE_CREATE with a table or DM_DEVICE_RESUME.
 */
int dm_task_set_read_ahead(struct dm_task *dmt, uint32_t read_ahead,
			   uint32_t read_ahead_flags);
uint32_t dm_task_get_read_ahead(const struct dm_task *dmt,
				uint32_t *read_ahead);

/*
 * Use these to prepare for a create or reload.
 */
int dm_task_add_target(struct dm_task *dmt,
		       uint64_t start,
		       uint64_t size, const char *ttype, const char *params);

/*
 * Format major/minor numbers correctly for input to driver.
 */
#define DM_FORMAT_DEV_BUFSIZE	13	/* Minimum bufsize to handle worst case. */
int dm_format_dev(char *buf, int bufsize, uint32_t dev_major, uint32_t dev_minor);

/* Use this to retrive target information returned from a STATUS call */
void *dm_get_next_target(struct dm_task *dmt,
			 void *next, uint64_t *start, uint64_t *length,
			 char **target_type, char **params);

/*
 * Following dm_get_status_* functions will allocate approriate status structure
 * from passed mempool together with the necessary character arrays.
 * Destroying the mempool will release all asociated allocation.
 */

/* Parse params from STATUS call for mirror target */
typedef enum {
	DM_STATUS_MIRROR_ALIVE	      = 'A',/* No failures */
	DM_STATUS_MIRROR_FLUSH_FAILED = 'F',/* Mirror out-of-sync */
	DM_STATUS_MIRROR_WRITE_FAILED = 'D',/* Mirror out-of-sync */
	DM_STATUS_MIRROR_SYNC_FAILED  = 'S',/* Mirror out-of-sync */
	DM_STATUS_MIRROR_READ_FAILED  = 'R',/* Mirror data unaffected */
	DM_STATUS_MIRROR_UNCLASSIFIED = 'U' /* Bug */
} dm_status_mirror_health_t;

struct dm_status_mirror {
	uint64_t total_regions;
	uint64_t insync_regions;
	uint32_t dev_count;             /* # of devs[] elements (<= 8) */
	struct {
		dm_status_mirror_health_t health;
		uint32_t major;
		uint32_t minor;
	} *devs;                        /* array with individual legs */
	const char *log_type;           /* core, disk,.... */
	uint32_t log_count;		/* # of logs[] elements */
	struct {
		dm_status_mirror_health_t health;
		uint32_t major;
		uint32_t minor;
	} *logs;			/* array with individual logs */
};

int dm_get_status_mirror(struct dm_pool *mem, const char *params,
			 struct dm_status_mirror **status);

/* Parse params from STATUS call for raid target */
struct dm_status_raid {
	uint64_t reserved;
	uint64_t total_regions;		/* sectors */
	uint64_t insync_regions;	/* sectors */
	uint64_t mismatch_count;
	uint32_t dev_count;
	char *raid_type;
	/* A - alive,  a - alive not in-sync,  D - dead/failed */
	char *dev_health;
	/* idle, frozen, resync, recover, check, repair */
	char *sync_action;
	uint64_t data_offset; /* RAID out-of-place reshaping */
};

int dm_get_status_raid(struct dm_pool *mem, const char *params,
		       struct dm_status_raid **status);

/* Parse params from STATUS call for cache target */
struct dm_status_cache {
	uint64_t version;  /* zero for now */

	uint32_t metadata_block_size;   /* in 512B sectors */
	uint32_t block_size;            /* AKA 'chunk_size' */

	uint64_t metadata_used_blocks;
	uint64_t metadata_total_blocks;

	uint64_t used_blocks;
	uint64_t dirty_blocks;
	uint64_t total_blocks;

	uint64_t read_hits;
	uint64_t read_misses;
	uint64_t write_hits;
	uint64_t write_misses;

	uint64_t demotions;
	uint64_t promotions;

	uint64_t feature_flags;		/* DM_CACHE_FEATURE_? */

	int core_argc;
	char **core_argv;

	char *policy_name;
	int policy_argc;
	char **policy_argv;

	unsigned error : 1;		/* detected error (switches to fail soon) */
	unsigned fail : 1;		/* all I/O fails */
	unsigned needs_check : 1;	/* metadata needs check */
	unsigned read_only : 1;		/* metadata may not be changed */
	uint32_t reserved : 28;
};

int dm_get_status_cache(struct dm_pool *mem, const char *params,
			struct dm_status_cache **status);

/*
 * Parse params from STATUS call for snapshot target
 *
 * Snapshot target's format:
 * <= 1.7.0: <used_sectors>/<total_sectors>
 * >= 1.8.0: <used_sectors>/<total_sectors> <metadata_sectors>
 */
struct dm_status_snapshot {
	uint64_t used_sectors;          /* in 512b units */
	uint64_t total_sectors;
	uint64_t metadata_sectors;
	unsigned has_metadata_sectors : 1; /* set when metadata_sectors is present */
	unsigned invalid : 1;		/* set when snapshot is invalidated */
	unsigned merge_failed : 1;	/* set when snapshot merge failed */
	unsigned overflow : 1;		/* set when snapshot overflows */
};

int dm_get_status_snapshot(struct dm_pool *mem, const char *params,
			   struct dm_status_snapshot **status);

/* Parse params from STATUS call for thin_pool target */
typedef enum {
	DM_THIN_DISCARDS_IGNORE,
	DM_THIN_DISCARDS_NO_PASSDOWN,
	DM_THIN_DISCARDS_PASSDOWN
} dm_thin_discards_t;

struct dm_status_thin_pool {
	uint64_t transaction_id;
	uint64_t used_metadata_blocks;
	uint64_t total_metadata_blocks;
	uint64_t used_data_blocks;
	uint64_t total_data_blocks;
	uint64_t held_metadata_root;
	uint32_t read_only;		/* metadata may not be changed */
	dm_thin_discards_t discards;
	uint32_t fail : 1;		/* all I/O fails */
	uint32_t error_if_no_space : 1;	/* otherwise queue_if_no_space */
	uint32_t out_of_data_space : 1;	/* metadata may be changed, but data may not be allocated (no rw) */
	uint32_t needs_check : 1;	/* metadata needs check */
	uint32_t error : 1;		/* detected error (switches to fail soon) */
	uint32_t reserved : 27;
};

int dm_get_status_thin_pool(struct dm_pool *mem, const char *params,
			    struct dm_status_thin_pool **status);

/* Parse params from STATUS call for thin target */
struct dm_status_thin {
	uint64_t mapped_sectors;
	uint64_t highest_mapped_sector;
	uint32_t fail : 1;              /* Thin volume fails I/O */
	uint32_t reserved : 31;
};

int dm_get_status_thin(struct dm_pool *mem, const char *params,
		       struct dm_status_thin **status);

/*
 * Call this to actually run the ioctl.
 */
int dm_task_run(struct dm_task *dmt);

/*
 * The errno from the last device-mapper ioctl performed by dm_task_run.
 */
int dm_task_get_errno(struct dm_task *dmt);

/*
 * Call this to make or remove the device nodes associated with previously
 * issued commands.
 */
void dm_task_update_nodes(void);

/*
 * Mangling support
 *
 * Character whitelist: 0-9, A-Z, a-z, #+-.:=@_
 * HEX mangling format: \xNN, NN being the hex value of the character.
 * (whitelist and format supported by udev)
*/
typedef enum {
	DM_STRING_MANGLING_NONE, /* do not mangle at all */
	DM_STRING_MANGLING_AUTO, /* mangle only if not already mangled with hex, error when mixed */
	DM_STRING_MANGLING_HEX	 /* always mangle with hex encoding, no matter what the input is */
} dm_string_mangling_t;

/*
 * Set/get mangling mode used for device-mapper names and uuids.
 */
int dm_set_name_mangling_mode(dm_string_mangling_t name_mangling);
dm_string_mangling_t dm_get_name_mangling_mode(void);

/*
 * Get mangled/unmangled form of the device-mapper name or uuid
 * irrespective of the global setting (set by dm_set_name_mangling_mode).
 * The name or uuid returned needs to be freed after use by calling free!
 */
char *dm_task_get_name_mangled(const struct dm_task *dmt);
char *dm_task_get_name_unmangled(const struct dm_task *dmt);
char *dm_task_get_uuid_mangled(const struct dm_task *dmt);
char *dm_task_get_uuid_unmangled(const struct dm_task *dmt);

/*
 * Configure the device-mapper directory
 */
int dm_set_dev_dir(const char *dir);
const char *dm_dir(void);

/*
 * Configure sysfs directory, /sys by default
 */
int dm_set_sysfs_dir(const char *dir);
const char *dm_sysfs_dir(void);

/*
 * Configure default UUID prefix string.
 * Conventionally this is a short capitalised prefix indicating the subsystem
 * that is managing the devices, e.g. "LVM-" or "MPATH-".
 * To support stacks of devices from different subsystems, recursive functions
 * stop recursing if they reach a device with a different prefix.
 */
int dm_set_uuid_prefix(const char *uuid_prefix);
const char *dm_uuid_prefix(void);

/*
 * Determine whether a major number belongs to device-mapper or not.
 */
int dm_is_dm_major(uint32_t major);

/*
 * Get associated device name for given major and minor number by reading
 * the sysfs content. If this is a dm device, get associated dm name, the one
 * that appears in /dev/mapper. DM names could be resolved this way only if
 * kernel used >= 2.6.29, kernel name is found otherwise (e.g. dm-0).
 * If prefer_kernel_name is set, the kernel name is always preferred over
 * device-mapper name for dm devices no matter what the kernel version is.
 * For non-dm devices, we always get associated kernel name, e.g sda, md0 etc.
 * Returns 0 on error or if sysfs is not used (or configured incorrectly),
 * otherwise returns 1 and the supplied buffer holds the device name.
 */
int dm_device_get_name(uint32_t major, uint32_t minor,
		       int prefer_kernel_name,
		       char *buf, size_t buf_size);

/*
 * Determine whether a device has any holders (devices
 * using this device). If sysfs is not used (or configured
 * incorrectly), returns 0.
 */
int dm_device_has_holders(uint32_t major, uint32_t minor);

/*
 * Determine whether a device contains mounted filesystem.
 * If sysfs is not used (or configured incorrectly), returns 0.
 */
int dm_device_has_mounted_fs(uint32_t major, uint32_t minor);


/*
 * Callback is invoked for individal mountinfo lines,
 * minor, major and mount target are parsed and unmangled.
 */
typedef int (*dm_mountinfo_line_callback_fn) (char *line, unsigned maj, unsigned min,
					      char *target, void *cb_data);

/*
 * Read all lines from /proc/self/mountinfo,
 * for each line calls read_fn callback.
 */
int dm_mountinfo_read(dm_mountinfo_line_callback_fn read_fn, void *cb_data);

/*
 * Initialise library
 */
void dm_lib_init(void) __attribute__((constructor));

/*
 * Release library resources
 */
void dm_lib_release(void);
void dm_lib_exit(void) __attribute__((destructor));

/* An optimisation for clients making repeated calls involving dm ioctls */
void dm_hold_control_dev(int hold_open);

/*
 * Use NULL for all devices.
 */
int dm_mknodes(const char *name);
int dm_driver_version(char *version, size_t size);

/******************************************************
 * Functions to build and manipulate trees of devices *
 ******************************************************/
struct dm_tree;
struct dm_tree_node;

/*
 * Initialise an empty dependency tree.
 *
 * The tree consists of a root node together with one node for each mapped
 * device which has child nodes for each device referenced in its table.
 *
 * Every node in the tree has one or more children and one or more parents.
 *
 * The root node is the parent/child of every node that doesn't have other
 * parents/children.
 */
struct dm_tree *dm_tree_create(void);
void dm_tree_free(struct dm_tree *tree);

/*
 * List of suffixes to be ignored when matching uuids against existing devices.
 */
void dm_tree_set_optional_uuid_suffixes(struct dm_tree *dtree, const char **optional_uuid_suffixes);

/*
 * Add nodes to the tree for a given device and all the devices it uses.
 */
int dm_tree_add_dev(struct dm_tree *tree, uint32_t major, uint32_t minor);
int dm_tree_add_dev_with_udev_flags(struct dm_tree *tree, uint32_t major,
				    uint32_t minor, uint16_t udev_flags);

/*
 * Add a new node to the tree if it doesn't already exist.
 */
struct dm_tree_node *dm_tree_add_new_dev(struct dm_tree *tree,
					 const char *name,
					 const char *uuid,
					 uint32_t major, uint32_t minor,
					 int read_only,
					 int clear_inactive,
					 void *context);
struct dm_tree_node *dm_tree_add_new_dev_with_udev_flags(struct dm_tree *tree,
							 const char *name,
							 const char *uuid,
							 uint32_t major,
							 uint32_t minor,
							 int read_only,
							 int clear_inactive,
							 void *context,
							 uint16_t udev_flags);

/*
 * Search for a node in the tree.
 * Set major and minor to 0 or uuid to NULL to get the root node.
 */
struct dm_tree_node *dm_tree_find_node(struct dm_tree *tree,
				       uint32_t major,
				       uint32_t minor);
struct dm_tree_node *dm_tree_find_node_by_uuid(struct dm_tree *tree,
					       const char *uuid);

/*
 * Use this to walk through all children of a given node.
 * Set handle to NULL in first call.
 * Returns NULL after the last child.
 * Set inverted to use inverted tree.
 */
struct dm_tree_node *dm_tree_next_child(void **handle,
					const struct dm_tree_node *parent,
					uint32_t inverted);

/*
 * Get properties of a node.
 */
const char *dm_tree_node_get_name(const struct dm_tree_node *node);
const char *dm_tree_node_get_uuid(const struct dm_tree_node *node);
const struct dm_info *dm_tree_node_get_info(const struct dm_tree_node *node);
void *dm_tree_node_get_context(const struct dm_tree_node *node);
/*
 * Returns  0 when node size and its children is unchanged.
 * Returns  1 when node or any of its children has increased size.
 * Rerurns -1 when node or any of its children has reduced size.
 */
int dm_tree_node_size_changed(const struct dm_tree_node *dnode);

/*
 * Returns the number of children of the given node (excluding the root node).
 * Set inverted for the number of parents.
 */
int dm_tree_node_num_children(const struct dm_tree_node *node, uint32_t inverted);

/*
 * Deactivate a device plus all dependencies.
 * Ignores devices that don't have a uuid starting with uuid_prefix.
 */
int dm_tree_deactivate_children(struct dm_tree_node *dnode,
				const char *uuid_prefix,
				size_t uuid_prefix_len);
/*
 * Preload/create a device plus all dependencies.
 * Ignores devices that don't have a uuid starting with uuid_prefix.
 */
int dm_tree_preload_children(struct dm_tree_node *dnode,
			     const char *uuid_prefix,
			     size_t uuid_prefix_len);

/*
 * Resume a device plus all dependencies.
 * Ignores devices that don't have a uuid starting with uuid_prefix.
 */
int dm_tree_activate_children(struct dm_tree_node *dnode,
			      const char *uuid_prefix,
			      size_t uuid_prefix_len);

/*
 * Suspend a device plus all dependencies.
 * Ignores devices that don't have a uuid starting with uuid_prefix.
 */
int dm_tree_suspend_children(struct dm_tree_node *dnode,
			     const char *uuid_prefix,
			     size_t uuid_prefix_len);

/*
 * Skip the filesystem sync when suspending.
 * Does nothing with other functions.
 * Use this when no snapshots are involved.
 */
void dm_tree_skip_lockfs(struct dm_tree_node *dnode);

/*
 * Set the 'noflush' flag when suspending devices.
 * If the kernel supports it, instead of erroring outstanding I/O that
 * cannot be completed, the I/O is queued and resubmitted when the
 * device is resumed.  This affects multipath devices when all paths
 * have failed and queue_if_no_path is set, and mirror devices when
 * block_on_error is set and the mirror log has failed.
 */
void dm_tree_use_no_flush_suspend(struct dm_tree_node *dnode);

/*
 * Retry removal of each device if not successful.
 */
void dm_tree_retry_remove(struct dm_tree_node *dnode);

/*
 * Is the uuid prefix present in the tree?
 * Only returns 0 if every node was checked successfully.
 * Returns 1 if the tree walk has to be aborted.
 */
int dm_tree_children_use_uuid(struct dm_tree_node *dnode,
			      const char *uuid_prefix,
			      size_t uuid_prefix_len);

/*
 * Construct tables for new nodes before activating them.
 */
int dm_tree_node_add_snapshot_origin_target(struct dm_tree_node *dnode,
					    uint64_t size,
					    const char *origin_uuid);
int dm_tree_node_add_snapshot_target(struct dm_tree_node *node,
				     uint64_t size,
				     const char *origin_uuid,
				     const char *cow_uuid,
				     int persistent,
				     uint32_t chunk_size);
int dm_tree_node_add_snapshot_merge_target(struct dm_tree_node *node,
					   uint64_t size,
					   const char *origin_uuid,
					   const char *cow_uuid,
					   const char *merge_uuid,
					   uint32_t chunk_size);
int dm_tree_node_add_error_target(struct dm_tree_node *node,
				  uint64_t size);
int dm_tree_node_add_zero_target(struct dm_tree_node *node,
				 uint64_t size);
int dm_tree_node_add_linear_target(struct dm_tree_node *node,
				   uint64_t size);
int dm_tree_node_add_striped_target(struct dm_tree_node *node,
				    uint64_t size,
				    uint32_t stripe_size);

#define DM_CRYPT_IV_DEFAULT	UINT64_C(-1)	/* iv_offset == seg offset */
/*
 * Function accepts one string in cipher specification
 * (chainmode and iv should be NULL because included in cipher string)
 *   or
 * separate arguments which will be joined to "cipher-chainmode-iv"
 */
int dm_tree_node_add_crypt_target(struct dm_tree_node *node,
				  uint64_t size,
				  const char *cipher,
				  const char *chainmode,
				  const char *iv,
				  uint64_t iv_offset,
				  const char *key);
int dm_tree_node_add_mirror_target(struct dm_tree_node *node,
				   uint64_t size);

/* Mirror log flags */
#define DM_NOSYNC		0x00000001	/* Known already in sync */
#define DM_FORCESYNC		0x00000002	/* Force resync */
#define DM_BLOCK_ON_ERROR	0x00000004	/* On error, suspend I/O */
#define DM_CORELOG		0x00000008	/* In-memory log */

int dm_tree_node_add_mirror_target_log(struct dm_tree_node *node,
				       uint32_t region_size,
				       unsigned clustered,
				       const char *log_uuid,
				       unsigned area_count,
				       uint32_t flags);

int dm_tree_node_add_raid_target(struct dm_tree_node *node,
				 uint64_t size,
				 const char *raid_type,
				 uint32_t region_size,
				 uint32_t stripe_size,
				 uint64_t rebuilds,
				 uint64_t flags);

/*
 * Defines below are based on kernel's dm-cache.c defines
 * DM_CACHE_MIN_DATA_BLOCK_SIZE (32 * 1024 >> SECTOR_SHIFT)
 * DM_CACHE_MAX_DATA_BLOCK_SIZE (1024 * 1024 * 1024 >> SECTOR_SHIFT)
 */
#define DM_CACHE_MIN_DATA_BLOCK_SIZE (UINT32_C(64))
#define DM_CACHE_MAX_DATA_BLOCK_SIZE (UINT32_C(2097152))
/*
 * Max supported size for cache pool metadata device.
 * Limitation is hardcoded into the kernel and bigger device sizes
 * are not accepted.
 *
 * Limit defined in drivers/md/dm-cache-metadata.h
 */
#define DM_CACHE_METADATA_MAX_SECTORS DM_THIN_METADATA_MAX_SECTORS

/*
 * Define number of elements in rebuild and writemostly arrays
 * 'of struct dm_tree_node_raid_params'.
 */

struct dm_tree_node_raid_params {
	const char *raid_type;

	uint32_t stripes;
	uint32_t mirrors;
	uint32_t region_size;
	uint32_t stripe_size;

	/*
	 * 'rebuilds' and 'writemostly' are bitfields that signify
	 * which devices in the array are to be rebuilt or marked
	 * writemostly.  The kernel supports up to 253 legs.
	 * We limit ourselves by choosing a lower value
	 * for DEFAULT_RAID{1}_MAX_IMAGES in defaults.h.
	 */
	uint64_t rebuilds;
	uint64_t writemostly;
	uint32_t writebehind;	    /* I/Os (kernel default COUNTER_MAX / 2) */
	uint32_t sync_daemon_sleep; /* ms (kernel default = 5sec) */
	uint32_t max_recovery_rate; /* kB/sec/disk */
	uint32_t min_recovery_rate; /* kB/sec/disk */
	uint32_t stripe_cache;      /* sectors */

	uint64_t flags;             /* [no]sync */
	uint32_t reserved2;
};

/*
 * Version 2 of above node raid params struct to keeep API compatibility.
 *
 * Extended for more than 64 legs (max 253 in the MD kernel runtime!),
 * delta_disks for disk add/remove reshaping,
 * data_offset for out-of-place reshaping
 * and data_copies for odd number of raid10 legs.
 */
#define	RAID_BITMAP_SIZE 4 /* 4 * 64 bit elements in rebuilds/writemostly arrays */
struct dm_tree_node_raid_params_v2 {
	const char *raid_type;

	uint32_t stripes;
	uint32_t mirrors;
	uint32_t region_size;
	uint32_t stripe_size;

	int delta_disks; /* +/- number of disks to add/remove (reshaping) */
	int data_offset; /* data offset to set (out-of-place reshaping) */

	/*
	 * 'rebuilds' and 'writemostly' are bitfields that signify
	 * which devices in the array are to be rebuilt or marked
	 * writemostly.  The kernel supports up to 253 legs.
	 * We limit ourselvs by choosing a lower value
	 * for DEFAULT_RAID_MAX_IMAGES.
	 */
	uint64_t rebuilds[RAID_BITMAP_SIZE];
	uint64_t writemostly[RAID_BITMAP_SIZE];
	uint32_t writebehind;	    /* I/Os (kernel default COUNTER_MAX / 2) */
	uint32_t data_copies;	    /* RAID # of data copies */
	uint32_t sync_daemon_sleep; /* ms (kernel default = 5sec) */
	uint32_t max_recovery_rate; /* kB/sec/disk */
	uint32_t min_recovery_rate; /* kB/sec/disk */
	uint32_t stripe_cache;      /* sectors */

	uint64_t flags;             /* [no]sync */
};

int dm_tree_node_add_raid_target_with_params(struct dm_tree_node *node,
					     uint64_t size,
					     const struct dm_tree_node_raid_params *p);

/* Version 2 API function taking dm_tree_node_raid_params_v2 for aforementioned extensions. */
int dm_tree_node_add_raid_target_with_params_v2(struct dm_tree_node *node,
						uint64_t size,
						const struct dm_tree_node_raid_params_v2 *p);

/* Cache feature_flags */
#define DM_CACHE_FEATURE_WRITEBACK    0x00000001
#define DM_CACHE_FEATURE_WRITETHROUGH 0x00000002
#define DM_CACHE_FEATURE_PASSTHROUGH  0x00000004
#define DM_CACHE_FEATURE_METADATA2    0x00000008 /* cache v1.10 */

struct dm_config_node;
/*
 * Use for passing cache policy and all its args e.g.:
 *
 * policy_settings {
 *    migration_threshold=2048
 *    sequention_threashold=100
 *    ...
 * }
 *
 * For policy without any parameters use NULL.
 */
int dm_tree_node_add_cache_target(struct dm_tree_node *node,
				  uint64_t size,
				  uint64_t feature_flags, /* DM_CACHE_FEATURE_* */
				  const char *metadata_uuid,
				  const char *data_uuid,
				  const char *origin_uuid,
				  const char *policy_name,
				  const struct dm_config_node *policy_settings,
				  uint32_t data_block_size);

/*
 * VDO target
 */
int dm_tree_node_add_vdo_target(struct dm_tree_node *node,
				uint64_t size,
				const char *data_uuid,
				const struct dm_vdo_target_params *param);

/*
 * FIXME Add individual cache policy pairs  <key> = value, like:
 * int dm_tree_node_add_cache_policy_arg(struct dm_tree_node *dnode,
 *				      const char *key, uint64_t value);
 */

/*
 * Replicator operation mode
 * Note: API for Replicator is not yet stable
 */
typedef enum {
	DM_REPLICATOR_SYNC,			/* Synchronous replication */
	DM_REPLICATOR_ASYNC_WARN,		/* Warn if async replicator is slow */
	DM_REPLICATOR_ASYNC_STALL,		/* Stall replicator if not fast enough */
	DM_REPLICATOR_ASYNC_DROP,		/* Drop sites out of sync */
	DM_REPLICATOR_ASYNC_FAIL,		/* Fail replicator if slow */
	NUM_DM_REPLICATOR_MODES
} dm_replicator_mode_t;

int dm_tree_node_add_replicator_target(struct dm_tree_node *node,
				       uint64_t size,
				       const char *rlog_uuid,
				       const char *rlog_type,
				       unsigned rsite_index,
				       dm_replicator_mode_t mode,
				       uint32_t async_timeout,
				       uint64_t fall_behind_data,
				       uint32_t fall_behind_ios);

int dm_tree_node_add_replicator_dev_target(struct dm_tree_node *node,
					   uint64_t size,
					   const char *replicator_uuid,	/* Replicator control device */
					   uint64_t rdevice_index,
					   const char *rdev_uuid,	/* Rimage device name/uuid */
					   unsigned rsite_index,
					   const char *slog_uuid,
					   uint32_t slog_flags,		/* Mirror log flags */
					   uint32_t slog_region_size);
/* End of Replicator API */

/*
 * FIXME: Defines bellow are based on kernel's dm-thin.c defines
 * DATA_DEV_BLOCK_SIZE_MIN_SECTORS (64 * 1024 >> SECTOR_SHIFT)
 * DATA_DEV_BLOCK_SIZE_MAX_SECTORS (1024 * 1024 * 1024 >> SECTOR_SHIFT)
 */
#define DM_THIN_MIN_DATA_BLOCK_SIZE (UINT32_C(128))
#define DM_THIN_MAX_DATA_BLOCK_SIZE (UINT32_C(2097152))
/*
 * Max supported size for thin pool  metadata device (17112760320 bytes)
 * Limitation is hardcoded into the kernel and bigger device size
 * is not accepted.
 * drivers/md/dm-thin-metadata.h THIN_METADATA_MAX_SECTORS
 */
#define DM_THIN_MAX_METADATA_SIZE   (UINT64_C(255) * (1 << 14) * (4096 / (1 << 9)) - 256 * 1024)

int dm_tree_node_add_thin_pool_target(struct dm_tree_node *node,
				      uint64_t size,
				      uint64_t transaction_id,
				      const char *metadata_uuid,
				      const char *pool_uuid,
				      uint32_t data_block_size,
				      uint64_t low_water_mark,
				      unsigned skip_block_zeroing);

/* Supported messages for thin provision target */
typedef enum {
	DM_THIN_MESSAGE_CREATE_SNAP,		/* device_id, origin_id */
	DM_THIN_MESSAGE_CREATE_THIN,		/* device_id */
	DM_THIN_MESSAGE_DELETE,			/* device_id */
	DM_THIN_MESSAGE_SET_TRANSACTION_ID,	/* current_id, new_id */
	DM_THIN_MESSAGE_RESERVE_METADATA_SNAP,	/* target version >= 1.1 */
	DM_THIN_MESSAGE_RELEASE_METADATA_SNAP,	/* target version >= 1.1 */
} dm_thin_message_t;

int dm_tree_node_add_thin_pool_message(struct dm_tree_node *node,
				       dm_thin_message_t type,
				       uint64_t id1, uint64_t id2);

/*
 * Set thin pool discard features
 *   ignore      - Disable support for discards
 *   no_passdown - Don't pass discards down to underlying data device,
 *                 just remove the mapping
 * Feature is available since version 1.1 of the thin target.
 */
int dm_tree_node_set_thin_pool_discard(struct dm_tree_node *node,
				       unsigned ignore,
				       unsigned no_passdown);
/*
 * Set error if no space, instead of queueing for thin pool.
 */
int dm_tree_node_set_thin_pool_error_if_no_space(struct dm_tree_node *node,
						 unsigned error_if_no_space);
/* Start thin pool with metadata in read-only mode */
int dm_tree_node_set_thin_pool_read_only(struct dm_tree_node *node,
					 unsigned read_only);
/*
 * FIXME: Defines bellow are based on kernel's dm-thin.c defines
 * MAX_DEV_ID ((1 << 24) - 1)
 */
#define DM_THIN_MAX_DEVICE_ID (UINT32_C((1 << 24) - 1))
int dm_tree_node_add_thin_target(struct dm_tree_node *node,
				 uint64_t size,
				 const char *pool_uuid,
				 uint32_t device_id);

int dm_tree_node_set_thin_external_origin(struct dm_tree_node *node,
					  const char *external_uuid);

void dm_tree_node_set_udev_flags(struct dm_tree_node *node, uint16_t udev_flags);

void dm_tree_node_set_presuspend_node(struct dm_tree_node *node,
				      struct dm_tree_node *presuspend_node);

int dm_tree_node_add_target_area(struct dm_tree_node *node,
				    const char *dev_name,
				    const char *dlid,
				    uint64_t offset);

/*
 * Only for temporarily-missing raid devices where changes are tracked.
 */
int dm_tree_node_add_null_area(struct dm_tree_node *node, uint64_t offset);

/*
 * Set readahead (in sectors) after loading the node.
 */
void dm_tree_node_set_read_ahead(struct dm_tree_node *dnode,
				 uint32_t read_ahead,
				 uint32_t read_ahead_flags);

/*
 * Set node callback hook before de/activation.
 * Callback is called before 'activation' of node for activation tree,
 * or 'deactivation' of node for deactivation tree.
 */
typedef enum {
	DM_NODE_CALLBACK_PRELOADED,   /* Node has preload deps */
	DM_NODE_CALLBACK_DEACTIVATED, /* Node is deactivated */
} dm_node_callback_t;
typedef int (*dm_node_callback_fn) (struct dm_tree_node *node,
				    dm_node_callback_t type, void *cb_data);
void dm_tree_node_set_callback(struct dm_tree_node *node,
			       dm_node_callback_fn cb, void *cb_data);

void dm_tree_set_cookie(struct dm_tree_node *node, uint32_t cookie);
uint32_t dm_tree_get_cookie(struct dm_tree_node *node);

/*****************************************************************************
 * Library functions
 *****************************************************************************/

#define malloc_aligned(s, a) malloc_aligned_wrapper((s), (a),  __FILE__, __LINE__)

/*
 * The pool allocator is useful when you are going to allocate
 * lots of memory, use the memory for a bit, and then free the
 * memory in one go.  A surprising amount of code has this usage
 * profile.
 *
 * You should think of the pool as an infinite, contiguous chunk
 * of memory.  The front of this chunk of memory contains
 * allocated objects, the second half is free.  dm_pool_alloc grabs
 * the next 'size' bytes from the free half, in effect moving it
 * into the allocated half.  This operation is very efficient.
 *
 * dm_pool_free frees the allocated object *and* all objects
 * allocated after it.  It is important to note this semantic
 * difference from malloc/free.  This is also extremely
 * efficient, since a single dm_pool_free can dispose of a large
 * complex object.
 *
 * dm_pool_destroy frees all allocated memory.
 *
 * eg, If you are building a binary tree in your program, and
 * know that you are only ever going to insert into your tree,
 * and not delete (eg, maintaining a symbol table for a
 * compiler).  You can create yourself a pool, allocate the nodes
 * from it, and when the tree becomes redundant call dm_pool_destroy
 * (no nasty iterating through the tree to free nodes).
 *
 * eg, On the other hand if you wanted to repeatedly insert and
 * remove objects into the tree, you would be better off
 * allocating the nodes from a free list; you cannot free a
 * single arbitrary node with pool.
 */

struct dm_pool;

/* constructor and destructor */
struct dm_pool *dm_pool_create(const char *name, size_t chunk_hint)
	__attribute__((__warn_unused_result__));
void dm_pool_destroy(struct dm_pool *p);

/* simple allocation/free routines */
void *dm_pool_alloc(struct dm_pool *p, size_t s)
	__attribute__((__warn_unused_result__));
void *dm_pool_alloc_aligned(struct dm_pool *p, size_t s, unsigned alignment)
	__attribute__((__warn_unused_result__));
void dm_pool_empty(struct dm_pool *p);
void dm_pool_free(struct dm_pool *p, void *ptr);

/*
 * To aid debugging, a pool can be locked. Any modifications made
 * to the content of the pool while it is locked can be detected.
 * Default compilation is using a crc checksum to notice modifications.
 * The pool locking is using the mprotect with the compilation flag
 * DEBUG_ENFORCE_POOL_LOCKING to enforce the memory protection.
 */
/* query pool lock status */
int dm_pool_locked(struct dm_pool *p);
/* mark pool as locked */
int dm_pool_lock(struct dm_pool *p, int crc)
	__attribute__((__warn_unused_result__));
/* mark pool as unlocked */
int dm_pool_unlock(struct dm_pool *p, int crc)
	__attribute__((__warn_unused_result__));

/*
 * Object building routines:
 *
 * These allow you to 'grow' an object, useful for
 * building strings, or filling in dynamic
 * arrays.
 *
 * It's probably best explained with an example:
 *
 * char *build_string(struct dm_pool *mem)
 * {
 *      int i;
 *      char buffer[16];
 *
 *      if (!dm_pool_begin_object(mem, 128))
 *              return NULL;
 *
 *      for (i = 0; i < 50; i++) {
 *              snprintf(buffer, sizeof(buffer), "%d, ", i);
 *              if (!dm_pool_grow_object(mem, buffer, 0))
 *                      goto bad;
 *      }
 *
 *	// add null
 *      if (!dm_pool_grow_object(mem, "\0", 1))
 *              goto bad;
 *
 *      return dm_pool_end_object(mem);
 *
 * bad:
 *
 *      dm_pool_abandon_object(mem);
 *      return NULL;
 *}
 *
 * So start an object by calling dm_pool_begin_object
 * with a guess at the final object size - if in
 * doubt make the guess too small.
 *
 * Then append chunks of data to your object with
 * dm_pool_grow_object.  Finally get your object with
 * a call to dm_pool_end_object.
 *
 * Setting delta to 0 means it will use strlen(extra).
 */
int dm_pool_begin_object(struct dm_pool *p, size_t hint);
int dm_pool_grow_object(struct dm_pool *p, const void *extra, size_t delta);
void *dm_pool_end_object(struct dm_pool *p);
void dm_pool_abandon_object(struct dm_pool *p);

/* utilities */
char *dm_pool_strdup(struct dm_pool *p, const char *str)
	__attribute__((__warn_unused_result__));
char *dm_pool_strndup(struct dm_pool *p, const char *str, size_t n)
	__attribute__((__warn_unused_result__));
void *dm_pool_zalloc(struct dm_pool *p, size_t s)
	__attribute__((__warn_unused_result__));

/******************
 * bitset functions
 ******************/

typedef uint32_t *dm_bitset_t;

dm_bitset_t dm_bitset_create(struct dm_pool *mem, unsigned num_bits);
void dm_bitset_destroy(dm_bitset_t bs);

int dm_bitset_equal(dm_bitset_t in1, dm_bitset_t in2);

void dm_bit_and(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2);
void dm_bit_union(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2);
int dm_bit_get_first(dm_bitset_t bs);
int dm_bit_get_next(dm_bitset_t bs, int last_bit);
int dm_bit_get_last(dm_bitset_t bs);
int dm_bit_get_prev(dm_bitset_t bs, int last_bit);

#define DM_BITS_PER_INT (sizeof(int) * CHAR_BIT)

#define dm_bit(bs, i) \
   ((bs)[((i) / DM_BITS_PER_INT) + 1] & (0x1 << ((i) & (DM_BITS_PER_INT - 1))))

#define dm_bit_set(bs, i) \
   ((bs)[((i) / DM_BITS_PER_INT) + 1] |= (0x1 << ((i) & (DM_BITS_PER_INT - 1))))

#define dm_bit_clear(bs, i) \
   ((bs)[((i) / DM_BITS_PER_INT) + 1] &= ~(0x1 << ((i) & (DM_BITS_PER_INT - 1))))

#define dm_bit_set_all(bs) \
   memset((bs) + 1, -1, ((*(bs) / DM_BITS_PER_INT) + 1) * sizeof(int))

#define dm_bit_clear_all(bs) \
   memset((bs) + 1, 0, ((*(bs) / DM_BITS_PER_INT) + 1) * sizeof(int))

#define dm_bit_copy(bs1, bs2) \
   memcpy((bs1) + 1, (bs2) + 1, ((*(bs2) / DM_BITS_PER_INT) + 1) * sizeof(int))

/*
 * Parse a string representation of a bitset into a dm_bitset_t. The
 * notation used is identical to the kernel bitmap parser (cpuset etc.)
 * and supports both lists ("1,2,3") and ranges ("1-2,5-8"). If the mem
 * parameter is NULL memory for the bitset will be allocated using
 * malloc(). Otherwise the bitset will be allocated using the supplied
 * dm_pool.
 */
dm_bitset_t dm_bitset_parse_list(const char *str, struct dm_pool *mem,
				 size_t min_num_bits);

/* Returns number of set bits */
static inline unsigned hweight32(uint32_t i)
{
	unsigned r = (i & 0x55555555) + ((i >> 1) & 0x55555555);

	r =    (r & 0x33333333) + ((r >>  2) & 0x33333333);
	r =    (r & 0x0F0F0F0F) + ((r >>  4) & 0x0F0F0F0F);
	r =    (r & 0x00FF00FF) + ((r >>  8) & 0x00FF00FF);
	return (r & 0x0000FFFF) + ((r >> 16) & 0x0000FFFF);
}

/*********
 * selinux
 *********/

/*
 * Obtain SELinux security context assigned for the path and set this
 * context for creating a new file system object. This security context
 * is global and it is used until reset to default policy behaviour
 * by calling 'dm_prepare_selinux_context(NULL, 0)'.
 */
int dm_prepare_selinux_context(const char *path, mode_t mode);
/*
 * Set SELinux context for existing file system object.
 */
int dm_set_selinux_context(const char *path, mode_t mode);

/*********************
 * string manipulation
 *********************/

/*
 * Break up the name of a mapped device into its constituent
 * Volume Group, Logical Volume and Layer (if present).
 * If mem is supplied, the result is allocated from the mempool.
 * Otherwise the strings are changed in situ.
 */
int dm_split_lvm_name(struct dm_pool *mem, const char *dmname,
		      char **vgname, char **lvname, char **layer);

/*
 * Destructively split buffer into NULL-separated words in argv.
 * Returns number of words.
 */
int dm_split_words(char *buffer, unsigned max,
		   unsigned ignore_comments, /* Not implemented */
		   char **argv);

/*
 * Returns -1 if buffer too small
 */
int dm_snprintf(char *buf, size_t bufsize, const char *format, ...)
    __attribute__ ((format(printf, 3, 4)));

/*
 * Returns pointer to the last component of the path.
 */
const char *dm_basename(const char *path);

/*
 * Returns number of occurrences of 'c' in 'str' of length 'size'.
 */
unsigned dm_count_chars(const char *str, size_t len, const int c);

/*
 * Length of string after escaping double quotes and backslashes.
 */
size_t dm_escaped_len(const char *str);

/*
 * <vg>-<lv>-<layer> or if !layer just <vg>-<lv>.
 */
char *dm_build_dm_name(struct dm_pool *mem, const char *vgname,
		       const char *lvname, const char *layer);
char *dm_build_dm_uuid(struct dm_pool *mem, const char *prefix, const char *lvid, const char *layer);

/*
 * Copies a string, quoting double quotes with backslashes.
 */
char *dm_escape_double_quotes(char *out, const char *src);

/*
 * Undo quoting in situ.
 */
void dm_unescape_double_quotes(char *src);

/*
 * Unescape colons and "at" signs in situ and save the substrings
 * starting at the position of the first unescaped colon and the
 * first unescaped "at" sign. This is normally used to unescape
 * device names used as PVs.
 */
void dm_unescape_colons_and_at_signs(char *src,
				     char **substr_first_unquoted_colon,
				     char **substr_first_unquoted_at_sign);

/*
 * Replacement for strncpy() function.
 *
 * Copies no more than n bytes from string pointed by src to the buffer
 * pointed by dest and ensure string is finished with '\0'.
 * Returns 0 if the whole string does not fit.
 */
int dm_strncpy(char *dest, const char *src, size_t n);

/*
 * Recognize unit specifier in the 'units' arg and return a factor
 * representing that unit. If the 'units' contains a prefix with digits,
 * the 'units' is considered to be a custom unit.
 *
 * Also, set 'unit_type' output arg to the character that represents
 * the unit specified. The 'unit_type' character equals to the unit
 * character itself recognized in the 'units' arg for canonical units.
 * Otherwise, the 'unit_type' character is set to 'U' for custom unit.
 *
 * An example for k/K canonical units and 8k/8K custom units:
 *
 *   units  unit_type  return value (factor)
 *   k      k          1024
 *   K      K          1000
 *   8k     U          1024*8
 *   8K     U          1000*8
 *   etc...
 *
 * Recognized units:
 *
 *   h/H - human readable (returns 1 for both)
 *   b/B - byte (returns 1 for both)
 *   s/S - sector (returns 512 for both)
 *   k/K - kilo (returns 1024/1000 respectively)
 *   m/M - mega (returns 1024^2/1000^2 respectively)
 *   g/G - giga (returns 1024^3/1000^3 respectively)
 *   t/T - tera (returns 1024^4/1000^4 respectively)
 *   p/P - peta (returns 1024^5/1000^5 respectively)
 *   e/E - exa (returns 1024^6/1000^6 respectively)
 *
 * Only one units character is allowed in the 'units' arg
 * if strict mode is enabled by 'strict' arg.
 *
 * The 'endptr' output arg, if not NULL, saves the pointer
 * in the 'units' string which follows the unit specifier
 * recognized (IOW the position where the parsing of the
 * unit specifier stopped).
 *
 * Returns the unit factor or 0 if no unit is recognized.
 */
uint64_t dm_units_to_factor(const char *units, char *unit_type,
			    int strict, const char **endptr);

/*
 * Type of unit specifier used by dm_size_to_string().
 */
typedef enum {
	DM_SIZE_LONG = 0,	/* Megabyte */
	DM_SIZE_SHORT = 1,	/* MB or MiB */
	DM_SIZE_UNIT = 2	/* M or m */
} dm_size_suffix_t;

/*
 * Convert a size (in 512-byte sectors) into a printable string using units of unit_type.
 * An upper-case unit_type indicates output units based on powers of 1000 are
 * required; a lower-case unit_type indicates powers of 1024.
 * For correct operation, unit_factor must be one of:
 * 	0 - the correct value will be calculated internally;
 *   or the output from dm_units_to_factor() corresponding to unit_type;
 *   or 'u' or 'U', an arbitrary number of bytes to use as the power base.
 * Set include_suffix to 1 to include a suffix of suffix_type.
 * Set use_si_units to 0 for suffixes that don't distinguish between 1000 and 1024.
 * Set use_si_units to 1 for a suffix that does distinguish.
 */
const char *dm_size_to_string(struct dm_pool *mem, uint64_t size,
			      char unit_type, int use_si_units,
			      uint64_t unit_factor, int include_suffix,
			      dm_size_suffix_t suffix_type);

/**************************
 * file/stream manipulation
 **************************/

/*
 * Create a directory (with parent directories if necessary).
 * Returns 1 on success, 0 on failure.
 */
int dm_create_dir(const char *dir);

int dm_is_empty_dir(const char *dir);

/*
 * Close a stream, with nicer error checking than fclose's.
 * Derived from gnulib's close-stream.c.
 *
 * Close "stream".  Return 0 if successful, and EOF (setting errno)
 * otherwise.  Upon failure, set errno to 0 if the error number
 * cannot be determined.  Useful mainly for writable streams.
 */
int dm_fclose(FILE *stream);

/*
 * Returns size of a buffer which is allocated with malloc.
 * Pointer to the buffer is stored in *buf.
 * Returns -1 on failure leaving buf undefined.
 */
int dm_asprintf(char **buf, const char *format, ...)
    __attribute__ ((format(printf, 2, 3)));
int dm_vasprintf(char **buf, const char *format, va_list ap)
    __attribute__ ((format(printf, 2, 0)));

/*
 * create lockfile (pidfile) - create and lock a lock file
 * @lockfile: location of lock file
 *
 * Returns: 1 on success, 0 otherwise, errno is handled internally
 */
int dm_create_lockfile(const char* lockfile);

/*
 * Query whether a daemon is running based on its lockfile
 *
 * Returns: 1 if running, 0 if not
 */
int dm_daemon_is_running(const char* lockfile);

/*********************
 * regular expressions
 *********************/
struct dm_regex;

/*
 * Initialise an array of num patterns for matching.
 * Uses memory from mem.
 */
struct dm_regex *dm_regex_create(struct dm_pool *mem, const char * const *patterns,
				 unsigned num_patterns);

/*
 * Match string s against the patterns.
 * Returns the index of the highest pattern in the array that matches,
 * or -1 if none match.
 */
int dm_regex_match(struct dm_regex *regex, const char *s);

/*
 * This is useful for regression testing only.  The idea is if two
 * fingerprints are different, then the two dfas are certainly not
 * isomorphic.  If two fingerprints _are_ the same then it's very likely
 * that the dfas are isomorphic.
 *
 * This function must be called before any matching is done.
 */
uint32_t dm_regex_fingerprint(struct dm_regex *regex);

/******************
 * percent handling
 ******************/
/*
 * A fixed-point representation of percent values. One percent equals to
 * DM_PERCENT_1 as defined below. Values that are not multiples of DM_PERCENT_1
 * represent fractions, with precision of 1/1000000 of a percent. See
 * dm_percent_to_float for a conversion to a floating-point representation.
 *
 * You should always use dm_make_percent when building dm_percent_t values. The
 * implementation of dm_make_percent is biased towards the middle: it ensures that
 * the result is DM_PERCENT_0 or DM_PERCENT_100 if and only if this is the actual
 * value -- it never rounds any intermediate value (> 0 or < 100) to either 0
 * or 100.
*/
#define DM_PERCENT_CHAR '%'

typedef enum {
	DM_PERCENT_0 = 0,
	DM_PERCENT_1 = 1000000,
	DM_PERCENT_100 = 100 * DM_PERCENT_1,
	DM_PERCENT_INVALID = -1,
	DM_PERCENT_FAILED = -2
} dm_percent_range_t;

typedef int32_t dm_percent_t;

float dm_percent_to_float(dm_percent_t percent);
/*
 * Return adjusted/rounded float for better percent value printing.
 * Function ensures for given precision of digits:
 * 100.0% returns only when the value is DM_PERCENT_100
 *        for close smaller values rounds to nearest smaller value
 * 0.0% returns only for value DM_PERCENT_0
 *        for close bigger values rounds to nearest bigger value
 * In all other cases returns same value as dm_percent_to_float()
 */
float dm_percent_to_round_float(dm_percent_t percent, unsigned digits);
dm_percent_t dm_make_percent(uint64_t numerator, uint64_t denominator);

/********************
 * timestamp handling
 ********************/

/*
 * Create a dm_timestamp object to use with dm_timestamp_get.
 */
struct dm_timestamp *dm_timestamp_alloc(void);

/*
 * Update dm_timestamp object to represent the current time.
 */
int dm_timestamp_get(struct dm_timestamp *ts);

/*
 * Copy a timestamp from ts_old to ts_new.
 */
void dm_timestamp_copy(struct dm_timestamp *ts_new, struct dm_timestamp *ts_old);

/*
 * Compare two timestamps.
 *
 * Return: -1 if ts1 is less than ts2
 *  	    0 if ts1 is equal to ts2
 *          1 if ts1 is greater than ts2
 */
int dm_timestamp_compare(struct dm_timestamp *ts1, struct dm_timestamp *ts2);

/*
 * Return the absolute difference in nanoseconds between
 * the dm_timestamp objects ts1 and ts2.
 *
 * Callers that need to know whether ts1 is before, equal to, or after ts2
 * in addition to the magnitude should use dm_timestamp_compare.
 */
uint64_t dm_timestamp_delta(struct dm_timestamp *ts1, struct dm_timestamp *ts2);

/*
 * Destroy a dm_timestamp object.
 */
void dm_timestamp_destroy(struct dm_timestamp *ts);

/*********************
 * reporting functions
 *********************/

struct dm_report_object_type {
	uint32_t id;			/* Powers of 2 */
	const char *desc;
	const char *prefix;		/* field id string prefix (optional) */
	/* FIXME: convert to proper usage of const pointers here */
	void *(*data_fn)(void *object);	/* callback from report_object() */
};

struct dm_report_field;

/*
 * dm_report_field_type flags
 */
#define DM_REPORT_FIELD_MASK				0x00000FFF
#define DM_REPORT_FIELD_ALIGN_MASK			0x0000000F
#define DM_REPORT_FIELD_ALIGN_LEFT			0x00000001
#define DM_REPORT_FIELD_ALIGN_RIGHT			0x00000002
#define DM_REPORT_FIELD_TYPE_MASK			0x00000FF0
#define DM_REPORT_FIELD_TYPE_NONE			0x00000000
#define DM_REPORT_FIELD_TYPE_STRING			0x00000010
#define DM_REPORT_FIELD_TYPE_NUMBER			0x00000020
#define DM_REPORT_FIELD_TYPE_SIZE			0x00000040
#define DM_REPORT_FIELD_TYPE_PERCENT			0x00000080
#define DM_REPORT_FIELD_TYPE_STRING_LIST		0x00000100
#define DM_REPORT_FIELD_TYPE_TIME			0x00000200

/* For use with reserved values only! */
#define DM_REPORT_FIELD_RESERVED_VALUE_MASK		0x0000000F
#define DM_REPORT_FIELD_RESERVED_VALUE_NAMED		0x00000001 /* only named value, less strict form of reservation */
#define DM_REPORT_FIELD_RESERVED_VALUE_RANGE		0x00000002 /* value is range - low and high value defined */
#define DM_REPORT_FIELD_RESERVED_VALUE_DYNAMIC_VALUE	0x00000004 /* value is computed in runtime */
#define DM_REPORT_FIELD_RESERVED_VALUE_FUZZY_NAMES	0x00000008 /* value names are recognized in runtime */

#define DM_REPORT_FIELD_TYPE_ID_LEN 32
#define DM_REPORT_FIELD_TYPE_HEADING_LEN 32

struct dm_report;
struct dm_report_field_type {
	uint32_t type;		/* object type id */
	uint32_t flags;		/* DM_REPORT_FIELD_* */
	uint32_t offset;	/* byte offset in the object */
	int32_t width;		/* default width */
	/* string used to specify the field */
	const char id[DM_REPORT_FIELD_TYPE_ID_LEN];
	/* string printed in header */
	const char heading[DM_REPORT_FIELD_TYPE_HEADING_LEN];
	int (*report_fn)(struct dm_report *rh, struct dm_pool *mem,
			 struct dm_report_field *field, const void *data,
			 void *private_data);
	const char *desc;	/* description of the field */
};

/*
 * Per-field reserved value.
 */
struct dm_report_field_reserved_value {
	/* field_num is the position of the field in 'fields'
	   array passed to dm_report_init_with_selection */
	uint32_t field_num;
	/* the value is of the same type as the field
	   identified by field_num */
	const void *value;
};

/*
 * Reserved value is a 'value' that is used directly if any of the 'names' is hit
 * or in case of fuzzy names, if such fuzzy name matches.
 *
 * If type is any of DM_REPORT_FIELD_TYPE_*, the reserved value is recognized
 * for all fields of that type.
 *
 * If type is DM_REPORT_FIELD_TYPE_NONE, the reserved value is recognized
 * for the exact field specified - hence the type of the value is automatically
 * the same as the type of the field itself.
 *
 * The array of reserved values is used to initialize reporting with
 * selection enabled (see also dm_report_init_with_selection function).
 */
struct dm_report_reserved_value {
	const uint32_t type;		/* DM_REPORT_FIELD_RESERVED_VALUE_* and DM_REPORT_FIELD_TYPE_*  */
	const void *value;		/* reserved value:
						uint64_t for DM_REPORT_FIELD_TYPE_NUMBER
						uint64_t for DM_REPORT_FIELD_TYPE_SIZE (number of 512-byte sectors)
						uint64_t for DM_REPORT_FIELD_TYPE_PERCENT
						const char* for DM_REPORT_FIELD_TYPE_STRING
						struct dm_report_field_reserved_value for DM_REPORT_FIELD_TYPE_NONE
						dm_report_reserved_handler* if DM_REPORT_FIELD_RESERVED_VALUE_{DYNAMIC_VALUE,FUZZY_NAMES} is used */
	const char **names;		/* null-terminated array of static names for this reserved value */
	const char *description;	/* description of the reserved value */
};

/*
 * Available actions for dm_report_reserved_value_handler.
 */
typedef enum {
	DM_REPORT_RESERVED_PARSE_FUZZY_NAME,
	DM_REPORT_RESERVED_GET_DYNAMIC_VALUE,
} dm_report_reserved_action_t;

/*
 * Generic reserved value handler to process reserved value names and/or values.
 *
 * Actions and their input/output:
 *
 * 	DM_REPORT_RESERVED_PARSE_FUZZY_NAME
 *		data_in:  const char *fuzzy_name
 *		data_out: const char *canonical_name, NULL if fuzzy_name not recognized
 *
 * 	DM_REPORT_RESERVED_GET_DYNAMIC_VALUE
 * 		data_in:  const char *canonical_name
 * 		data_out: void *value, NULL if canonical_name not recognized
 *
 * All actions return:
 *
 *	-1 if action not implemented
 * 	0 on error
 * 	1 on success
 */
typedef int (*dm_report_reserved_handler) (struct dm_report *rh,
					   struct dm_pool *mem,
					   uint32_t field_num,
					   dm_report_reserved_action_t action,
					   const void *data_in,
					   const void **data_out);

/*
 * The dm_report_value_cache_{set,get} are helper functions to store and retrieve
 * various values used during reporting (dm_report_field_type.report_fn) and/or
 * selection processing (dm_report_reserved_handler instances) to avoid
 * recalculation of these values or to share values among calls.
 */
int dm_report_value_cache_set(struct dm_report *rh, const char *name, const void *data);
const void *dm_report_value_cache_get(struct dm_report *rh, const char *name);
/*
 * dm_report_init output_flags
 */
#define DM_REPORT_OUTPUT_MASK			0x000000FF
#define DM_REPORT_OUTPUT_ALIGNED		0x00000001
#define DM_REPORT_OUTPUT_BUFFERED		0x00000002
#define DM_REPORT_OUTPUT_HEADINGS		0x00000004
#define DM_REPORT_OUTPUT_FIELD_NAME_PREFIX	0x00000008
#define DM_REPORT_OUTPUT_FIELD_UNQUOTED		0x00000010
#define DM_REPORT_OUTPUT_COLUMNS_AS_ROWS	0x00000020
#define DM_REPORT_OUTPUT_MULTIPLE_TIMES		0x00000040

struct dm_report *dm_report_init(uint32_t *report_types,
				 const struct dm_report_object_type *types,
				 const struct dm_report_field_type *fields,
				 const char *output_fields,
				 const char *output_separator,
				 uint32_t output_flags,
				 const char *sort_keys,
				 void *private_data);
struct dm_report *dm_report_init_with_selection(uint32_t *report_types,
						const struct dm_report_object_type *types,
						const struct dm_report_field_type *fields,
						const char *output_fields,
						const char *output_separator,
						uint32_t output_flags,
						const char *sort_keys,
						const char *selection,
						const struct dm_report_reserved_value reserved_values[],
						void *private_data);
/*
 * Report an object, pass it through the selection criteria if they
 * are present and display the result on output if it passes the criteria.
 */
int dm_report_object(struct dm_report *rh, void *object);
/*
 * The same as dm_report_object, but display the result on output only if
 * 'do_output' arg is set. Also, save the result of selection in 'selected'
 * arg if it's not NULL (either 1 if the object passes, otherwise 0).
 */
int dm_report_object_is_selected(struct dm_report *rh, void *object, int do_output, int *selected);

/*
 * Compact report output so that if field value is empty for all rows in
 * the report, drop the field from output completely (including headers).
 * Compact output is applicable only if report is buffered, otherwise
 * this function has no effect.
 */
int dm_report_compact_fields(struct dm_report *rh);

/*
 * The same as dm_report_compact_fields, but for selected fields only.
 * The "fields" arg is comma separated list of field names (the same format
 * as used for "output_fields" arg in dm_report_init fn).
 */
int dm_report_compact_given_fields(struct dm_report *rh, const char *fields);

/*
 * Returns 1 if there is no data waiting to be output.
 */
int dm_report_is_empty(struct dm_report *rh);

/*
 * Destroy report content without doing output.
 */
void dm_report_destroy_rows(struct dm_report *rh);

int dm_report_output(struct dm_report *rh);

/*
 * Output the report headings for a columns-based report, even if they
 * have already been shown. Useful for repeating reports that wish to
 * issue a periodic reminder of the column headings.
 */
int dm_report_column_headings(struct dm_report *rh);

void dm_report_free(struct dm_report *rh);

/*
 * Prefix added to each field name with DM_REPORT_OUTPUT_FIELD_NAME_PREFIX
 */
int dm_report_set_output_field_name_prefix(struct dm_report *rh,
					   const char *report_prefix);

int dm_report_set_selection(struct dm_report *rh, const char *selection);

/*
 * Report functions are provided for simple data types.
 * They take care of allocating copies of the data.
 */
int dm_report_field_string(struct dm_report *rh, struct dm_report_field *field,
			   const char *const *data);
int dm_report_field_string_list(struct dm_report *rh, struct dm_report_field *field,
				const struct dm_list *data, const char *delimiter);
int dm_report_field_string_list_unsorted(struct dm_report *rh, struct dm_report_field *field,
					 const struct dm_list *data, const char *delimiter);
int dm_report_field_int32(struct dm_report *rh, struct dm_report_field *field,
			  const int32_t *data);
int dm_report_field_uint32(struct dm_report *rh, struct dm_report_field *field,
			   const uint32_t *data);
int dm_report_field_int(struct dm_report *rh, struct dm_report_field *field,
			const int *data);
int dm_report_field_uint64(struct dm_report *rh, struct dm_report_field *field,
			   const uint64_t *data);
int dm_report_field_percent(struct dm_report *rh, struct dm_report_field *field,
			    const dm_percent_t *data);

/*
 * For custom fields, allocate the data in 'mem' and use
 * dm_report_field_set_value().
 * 'sortvalue' may be NULL if it matches 'value'
 */
void dm_report_field_set_value(struct dm_report_field *field, const void *value,
			       const void *sortvalue);

/*
 * Report group support.
 */
struct dm_report_group;

typedef enum {
	DM_REPORT_GROUP_SINGLE,
	DM_REPORT_GROUP_BASIC,
	DM_REPORT_GROUP_JSON
} dm_report_group_type_t;

struct dm_report_group *dm_report_group_create(dm_report_group_type_t type, void *data);
int dm_report_group_push(struct dm_report_group *group, struct dm_report *report, void *data);
int dm_report_group_pop(struct dm_report_group *group);
int dm_report_group_output_and_pop_all(struct dm_report_group *group);
int dm_report_group_destroy(struct dm_report_group *group);

/*************************
 * config file parse/print
 *************************/
typedef enum {
	DM_CFG_INT,
	DM_CFG_FLOAT,
	DM_CFG_STRING,
	DM_CFG_EMPTY_ARRAY
} dm_config_value_type_t;

struct dm_config_value {
	dm_config_value_type_t type;

	union {
		int64_t i;
		float f;
		double d;       	/* Unused. */
		const char *str;
	} v;

	struct dm_config_value *next;	/* For arrays */
	uint32_t format_flags;
};

struct dm_config_node {
	const char *key;
	struct dm_config_node *parent, *sib, *child;
	struct dm_config_value *v;
	int id;
};

struct dm_config_tree {
	struct dm_config_node *root;
	struct dm_config_tree *cascade;
	struct dm_pool *mem;
	void *custom;
};

struct dm_config_tree *dm_config_create(void);
struct dm_config_tree *dm_config_from_string(const char *config_settings);
int dm_config_parse(struct dm_config_tree *cft, const char *start, const char *end);
int dm_config_parse_without_dup_node_check(struct dm_config_tree *cft, const char *start, const char *end);

void *dm_config_get_custom(struct dm_config_tree *cft);
void dm_config_set_custom(struct dm_config_tree *cft, void *custom);

/*
 * When searching, first_cft is checked before second_cft.
 */
struct dm_config_tree *dm_config_insert_cascaded_tree(struct dm_config_tree *first_cft, struct dm_config_tree *second_cft);

/*
 * If there's a cascaded dm_config_tree, remove the top layer
 * and return the layer below.  Otherwise return NULL.
 */
struct dm_config_tree *dm_config_remove_cascaded_tree(struct dm_config_tree *cft);

/*
 * Create a new, uncascaded config tree equivalent to the input cascade.
 */
struct dm_config_tree *dm_config_flatten(struct dm_config_tree *cft);

void dm_config_destroy(struct dm_config_tree *cft);

/* Simple output line by line. */
typedef int (*dm_putline_fn)(const char *line, void *baton);
/* More advaced output with config node reference. */
typedef int (*dm_config_node_out_fn)(const struct dm_config_node *cn, const char *line, void *baton);

/*
 * Specification for advanced config node output.
 */
struct dm_config_node_out_spec {
	dm_config_node_out_fn prefix_fn; /* called before processing config node lines */
	dm_config_node_out_fn line_fn; /* called for each config node line */
	dm_config_node_out_fn suffix_fn; /* called after processing config node lines */
};

/* Write the node and any subsequent siblings it has. */
int dm_config_write_node(const struct dm_config_node *cn, dm_putline_fn putline, void *baton);
int dm_config_write_node_out(const struct dm_config_node *cn, const struct dm_config_node_out_spec *out_spec, void *baton);

/* Write given node only without subsequent siblings. */
int dm_config_write_one_node(const struct dm_config_node *cn, dm_putline_fn putline, void *baton);
int dm_config_write_one_node_out(const struct dm_config_node *cn, const struct dm_config_node_out_spec *out_spec, void *baton);

struct dm_config_node *dm_config_find_node(const struct dm_config_node *cn, const char *path);
int dm_config_has_node(const struct dm_config_node *cn, const char *path);
int dm_config_remove_node(struct dm_config_node *parent, struct dm_config_node *remove);
const char *dm_config_find_str(const struct dm_config_node *cn, const char *path, const char *fail);
const char *dm_config_find_str_allow_empty(const struct dm_config_node *cn, const char *path, const char *fail);
int dm_config_find_int(const struct dm_config_node *cn, const char *path, int fail);
int64_t dm_config_find_int64(const struct dm_config_node *cn, const char *path, int64_t fail);
float dm_config_find_float(const struct dm_config_node *cn, const char *path, float fail);

const struct dm_config_node *dm_config_tree_find_node(const struct dm_config_tree *cft, const char *path);
const char *dm_config_tree_find_str(const struct dm_config_tree *cft, const char *path, const char *fail);
const char *dm_config_tree_find_str_allow_empty(const struct dm_config_tree *cft, const char *path, const char *fail);
int dm_config_tree_find_int(const struct dm_config_tree *cft, const char *path, int fail);
int64_t dm_config_tree_find_int64(const struct dm_config_tree *cft, const char *path, int64_t fail);
float dm_config_tree_find_float(const struct dm_config_tree *cft, const char *path, float fail);
int dm_config_tree_find_bool(const struct dm_config_tree *cft, const char *path, int fail);

/*
 * Understands (0, ~0), (y, n), (yes, no), (on,
 * off), (true, false).
 */
int dm_config_find_bool(const struct dm_config_node *cn, const char *path, int fail);
int dm_config_value_is_bool(const struct dm_config_value *v);

int dm_config_get_uint32(const struct dm_config_node *cn, const char *path, uint32_t *result);
int dm_config_get_uint64(const struct dm_config_node *cn, const char *path, uint64_t *result);
int dm_config_get_str(const struct dm_config_node *cn, const char *path, const char **result);
int dm_config_get_list(const struct dm_config_node *cn, const char *path, const struct dm_config_value **result);
int dm_config_get_section(const struct dm_config_node *cn, const char *path, const struct dm_config_node **result);

unsigned dm_config_maybe_section(const char *str, unsigned len);

const char *dm_config_parent_name(const struct dm_config_node *n);

struct dm_config_node *dm_config_clone_node_with_mem(struct dm_pool *mem, const struct dm_config_node *node, int siblings);
struct dm_config_node *dm_config_create_node(struct dm_config_tree *cft, const char *key);
struct dm_config_value *dm_config_create_value(struct dm_config_tree *cft);
struct dm_config_node *dm_config_clone_node(struct dm_config_tree *cft, const struct dm_config_node *cn, int siblings);

/*
 * Common formatting flags applicable to all config node types (lower 16 bits).
 */
#define DM_CONFIG_VALUE_FMT_COMMON_ARRAY             0x00000001 /* value is array */
#define DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES      0x00000002 /* add spaces in "key = value" pairs in constrast to "key=value" for better readability */

/*
 * Type-related config node formatting flags (higher 16 bits).
 */
/* int-related formatting flags */
#define DM_CONFIG_VALUE_FMT_INT_OCTAL                0x00010000 /* print number in octal form */

/* string-related formatting flags */
#define DM_CONFIG_VALUE_FMT_STRING_NO_QUOTES         0x00010000 /* do not print quotes around string value */

void dm_config_value_set_format_flags(struct dm_config_value *cv, uint32_t format_flags);
uint32_t dm_config_value_get_format_flags(struct dm_config_value *cv);

struct dm_pool *dm_config_memory(struct dm_config_tree *cft);

/* Udev device directory. */
#define DM_UDEV_DEV_DIR "/dev/"

/* Cookie prefixes.
 *
 * The cookie value consists of a prefix (16 bits) and a base (16 bits).
 * We can use the prefix to store the flags. These flags are sent to
 * kernel within given dm task. When returned back to userspace in
 * DM_COOKIE udev environment variable, we can control several aspects
 * of udev rules we use by decoding the cookie prefix. When doing the
 * notification, we replace the cookie prefix with DM_COOKIE_MAGIC,
 * so we notify the right semaphore.
 *
 * It is still possible to use cookies for passing the flags to udev
 * rules even when udev_sync is disabled. The base part of the cookie
 * will be zero (there's no notification semaphore) and prefix will be
 * set then. However, having udev_sync enabled is highly recommended.
 */
#define DM_COOKIE_MAGIC 0x0D4D
#define DM_UDEV_FLAGS_MASK 0xFFFF0000
#define DM_UDEV_FLAGS_SHIFT 16

/*
 * DM_UDEV_DISABLE_DM_RULES_FLAG is set in case we need to disable
 * basic device-mapper udev rules that create symlinks in /dev/<DM_DIR>
 * directory. However, we can't reliably prevent creating default
 * nodes by udev (commonly /dev/dm-X, where X is a number).
 */
#define DM_UDEV_DISABLE_DM_RULES_FLAG 0x0001
/*
 * DM_UDEV_DISABLE_SUBSYTEM_RULES_FLAG is set in case we need to disable
 * subsystem udev rules, but still we need the general DM udev rules to
 * be applied (to create the nodes and symlinks under /dev and /dev/disk).
 */
#define DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG 0x0002
/*
 * DM_UDEV_DISABLE_DISK_RULES_FLAG is set in case we need to disable
 * general DM rules that set symlinks in /dev/disk directory.
 */
#define DM_UDEV_DISABLE_DISK_RULES_FLAG 0x0004
/*
 * DM_UDEV_DISABLE_OTHER_RULES_FLAG is set in case we need to disable
 * all the other rules that are not general device-mapper nor subsystem
 * related (the rules belong to other software or packages). All foreign
 * rules should check this flag directly and they should ignore further
 * rule processing for such event.
 */
#define DM_UDEV_DISABLE_OTHER_RULES_FLAG 0x0008
/*
 * DM_UDEV_LOW_PRIORITY_FLAG is set in case we need to instruct the
 * udev rules to give low priority to the device that is currently
 * processed. For example, this provides a way to select which symlinks
 * could be overwritten by high priority ones if their names are equal.
 * Common situation is a name based on FS UUID while using origin and
 * snapshot devices.
 */
#define DM_UDEV_LOW_PRIORITY_FLAG 0x0010
/*
 * DM_UDEV_DISABLE_LIBRARY_FALLBACK is set in case we need to disable
 * libdevmapper's node management. We will rely on udev completely
 * and there will be no fallback action provided by libdevmapper if
 * udev does something improperly. Using the library fallback code has
 * a consequence that you need to take into account: any device node
 * or symlink created without udev is not recorded in udev database
 * which other applications may read to get complete list of devices.
 * For this reason, use of DM_UDEV_DISABLE_LIBRARY_FALLBACK is
 * recommended on systems where udev is used. Keep library fallback
 * enabled just for exceptional cases where you need to debug udev-related
 * problems. If you hit such problems, please contact us through upstream
 * LVM2 development mailing list (see also README file). This flag is
 * currently not set by default in libdevmapper so you need to set it
 * explicitly if you're sure that udev is behaving correctly on your
 * setups.
 */
#define DM_UDEV_DISABLE_LIBRARY_FALLBACK 0x0020
/*
 * DM_UDEV_PRIMARY_SOURCE_FLAG is automatically appended by
 * libdevmapper for all ioctls generating udev uevents. Once used in
 * udev rules, we know if this is a real "primary sourced" event or not.
 * We need to distinguish real events originated in libdevmapper from
 * any spurious events to gather all missing information (e.g. events
 * generated as a result of "udevadm trigger" command or as a result
 * of the "watch" udev rule).
 */
#define DM_UDEV_PRIMARY_SOURCE_FLAG 0x0040

/*
 * Udev flags reserved for use by any device-mapper subsystem.
 */
#define DM_SUBSYSTEM_UDEV_FLAG0 0x0100
#define DM_SUBSYSTEM_UDEV_FLAG1 0x0200
#define DM_SUBSYSTEM_UDEV_FLAG2 0x0400
#define DM_SUBSYSTEM_UDEV_FLAG3 0x0800
#define DM_SUBSYSTEM_UDEV_FLAG4 0x1000
#define DM_SUBSYSTEM_UDEV_FLAG5 0x2000
#define DM_SUBSYSTEM_UDEV_FLAG6 0x4000
#define DM_SUBSYSTEM_UDEV_FLAG7 0x8000

int dm_cookie_supported(void);

/*
 * Udev synchronisation functions.
 */
void dm_udev_set_sync_support(int sync_with_udev);
int dm_udev_get_sync_support(void);
void dm_udev_set_checking(int checking);
int dm_udev_get_checking(void);

/*
 * Default value to get new auto generated cookie created
 */
#define DM_COOKIE_AUTO_CREATE 0
int dm_udev_create_cookie(uint32_t *cookie);
int dm_udev_complete(uint32_t cookie);
int dm_udev_wait(uint32_t cookie);

/*
 * dm_dev_wait_immediate 
 * If *ready is 1 on return, the wait is complete.
 * If *ready is 0 on return, the wait is incomplete and either
 * this function or dm_udev_wait() must be called again.
 * Returns 0 on error, when neither function should be called again.
 */
int dm_udev_wait_immediate(uint32_t cookie, int *ready);

#define DM_DEV_DIR_UMASK 0022
#define DM_CONTROL_NODE_UMASK 0177

#endif				/* LIB_DEVICE_MAPPER_H */
