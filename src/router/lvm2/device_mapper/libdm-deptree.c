/*
 * Copyright (C) 2005-2017 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "misc/dmlib.h"
#include "ioctl/libdm-targets.h"
#include "libdm-common.h"
#include "misc/kdev_t.h"
#include "misc/dm-ioctl.h"
#include "vdo/target.h"

#include <stdarg.h>
#include <string.h>
#include <sys/utsname.h>

#define MAX_TARGET_PARAMSIZE 500000

/* Supported segment types */
enum {
	SEG_CACHE,
	SEG_CRYPT,
	SEG_ERROR,
	SEG_LINEAR,
	SEG_MIRRORED,
	SEG_SNAPSHOT,
	SEG_SNAPSHOT_ORIGIN,
	SEG_SNAPSHOT_MERGE,
	SEG_STRIPED,
	SEG_ZERO,
	SEG_THIN_POOL,
	SEG_THIN,
	SEG_VDO,
	SEG_RAID0,
	SEG_RAID0_META,
	SEG_RAID1,
	SEG_RAID10,
	SEG_RAID4,
	SEG_RAID5_N,
	SEG_RAID5_LA,
	SEG_RAID5_RA,
	SEG_RAID5_LS,
	SEG_RAID5_RS,
	SEG_RAID6_N_6,
	SEG_RAID6_ZR,
	SEG_RAID6_NR,
	SEG_RAID6_NC,
	SEG_RAID6_LS_6,
	SEG_RAID6_RS_6,
	SEG_RAID6_LA_6,
	SEG_RAID6_RA_6,
};

/* FIXME Add crypt and multipath support */

static const struct {
	unsigned type;
	const char target[16];
} _dm_segtypes[] = {
	{ SEG_CACHE, "cache" },
	{ SEG_CRYPT, "crypt" },
	{ SEG_ERROR, "error" },
	{ SEG_LINEAR, "linear" },
	{ SEG_MIRRORED, "mirror" },
	{ SEG_SNAPSHOT, "snapshot" },
	{ SEG_SNAPSHOT_ORIGIN, "snapshot-origin" },
	{ SEG_SNAPSHOT_MERGE, "snapshot-merge" },
	{ SEG_STRIPED, "striped" },
	{ SEG_ZERO, "zero"},
	{ SEG_THIN_POOL, "thin-pool"},
	{ SEG_THIN, "thin"},
	{ SEG_VDO, "vdo" },
	{ SEG_RAID0, "raid0"},
	{ SEG_RAID0_META, "raid0_meta"},
	{ SEG_RAID1, "raid1"},
	{ SEG_RAID10, "raid10"},
	{ SEG_RAID4, "raid4"},
	{ SEG_RAID5_N,  "raid5_n"},
	{ SEG_RAID5_LA, "raid5_la"},
	{ SEG_RAID5_RA, "raid5_ra"},
	{ SEG_RAID5_LS, "raid5_ls"},
	{ SEG_RAID5_RS, "raid5_rs"},
	{ SEG_RAID6_N_6,"raid6_n_6"},
	{ SEG_RAID6_ZR, "raid6_zr"},
	{ SEG_RAID6_NR, "raid6_nr"},
	{ SEG_RAID6_NC, "raid6_nc"},
	{ SEG_RAID6_LS_6, "raid6_ls_6"},
	{ SEG_RAID6_RS_6, "raid6_rs_6"},
	{ SEG_RAID6_LA_6, "raid6_la_6"},
	{ SEG_RAID6_RA_6, "raid6_ra_6"},


	/*
	 * WARNING: Since 'raid' target overloads this 1:1 mapping table
	 * for search do not add new enum elements past them!
	 */
	{ SEG_RAID5_LS, "raid5"}, /* same as "raid5_ls" (default for MD also) */
	{ SEG_RAID6_ZR, "raid6"}, /* same as "raid6_zr" */
	{ SEG_RAID10, "raid10_near"}, /* same as "raid10" */
};

/* Some segment types have a list of areas of other devices attached */
struct seg_area {
	struct dm_list list;

	struct dm_tree_node *dev_node;

	uint64_t offset;
};

struct dm_thin_message {
	dm_thin_message_t type;
	union {
		struct {
			uint32_t device_id;
			uint32_t origin_id;
		} m_create_snap;
		struct {
			uint32_t device_id;
		} m_create_thin;
		struct {
			uint32_t device_id;
		} m_delete;
		struct {
			uint64_t current_id;
			uint64_t new_id;
		} m_set_transaction_id;
	} u;
};

struct thin_message {
	struct dm_list list;
	struct dm_thin_message message;
	int expected_errno;
};

/* Per-segment properties */
// FIXME: use a union to discriminate between target types.
struct load_segment {
	struct dm_list list;

	unsigned type;

	uint64_t size;

	unsigned area_count;		/* Linear + Striped + Mirrored + Crypt */
	struct dm_list areas;		/* Linear + Striped + Mirrored + Crypt */

	uint32_t stripe_size;		/* Striped + raid */

	int persistent;			/* Snapshot */
	uint32_t chunk_size;		/* Snapshot */
	struct dm_tree_node *cow;	/* Snapshot */
	struct dm_tree_node *origin;	/* Snapshot + Snapshot origin + Cache */
	struct dm_tree_node *merge;	/* Snapshot */

	struct dm_tree_node *log;	/* Mirror */
	uint32_t region_size;		/* Mirror + raid */
	unsigned clustered;		/* Mirror */
	unsigned mirror_area_count;	/* Mirror */
	uint32_t flags;			/* Mirror + raid + Cache */
	char *uuid;			/* Clustered mirror log */

	const char *policy_name;	/* Cache */
	unsigned policy_argc;		/* Cache */
	struct dm_config_node *policy_settings;	/* Cache */

	const char *cipher;		/* Crypt */
	const char *chainmode;		/* Crypt */
	const char *iv;			/* Crypt */
	uint64_t iv_offset;		/* Crypt */
	const char *key;		/* Crypt */

	int delta_disks;		/* raid reshape number of disks */
	int data_offset;		/* raid reshape data offset on disk to set */
	uint64_t rebuilds[RAID_BITMAP_SIZE];	/* raid */
	uint64_t writemostly[RAID_BITMAP_SIZE];	/* raid */
	uint32_t writebehind;		/* raid */
	uint32_t max_recovery_rate;	/* raid kB/sec/disk */
	uint32_t min_recovery_rate;	/* raid kB/sec/disk */
	uint32_t data_copies;		/* raid10 data_copies */

	struct dm_tree_node *metadata;	/* Thin_pool + Cache */
	struct dm_tree_node *pool;	/* Thin_pool, Thin */
	struct dm_tree_node *external;	/* Thin */
	struct dm_list thin_messages;	/* Thin_pool */
	uint64_t transaction_id;	/* Thin_pool */
	uint64_t low_water_mark;	/* Thin_pool */
	uint32_t data_block_size;       /* Thin_pool + cache */
	unsigned skip_block_zeroing;	/* Thin_pool */
	unsigned ignore_discard;	/* Thin_pool target vsn 1.1 */
	unsigned no_discard_passdown;	/* Thin_pool target vsn 1.1 */
	unsigned error_if_no_space;	/* Thin pool target vsn 1.10 */
	unsigned read_only;		/* Thin pool target vsn 1.3 */
	uint32_t device_id;		/* Thin */

	// VDO params
	struct dm_tree_node *vdo_data;  /* VDO */
	struct dm_vdo_target_params vdo_params; /* VDO */
	const char *vdo_name;           /* VDO - device name is ALSO passed as table arg */
};

/* Per-device properties */
struct load_properties {
	int read_only;
	uint32_t major;
	uint32_t minor;

	uint32_t read_ahead;
	uint32_t read_ahead_flags;

	unsigned segment_count;
	int size_changed;
	struct dm_list segs;

	const char *new_name;

	/* If immediate_dev_node is set to 1, try to create the dev node
	 * as soon as possible (e.g. in preload stage even during traversal
	 * and processing of dm tree). This will also flush all stacked dev
	 * node operations, synchronizing with udev.
	 */
	unsigned immediate_dev_node;

	/*
	 * If the device size changed from zero and this is set,
	 * don't resume the device immediately, even if the device
	 * has parents.  This works provided the parents do not
	 * validate the device size and is required by pvmove to
	 * avoid starting the mirror resync operation too early.
	 */
	unsigned delay_resume_if_new;

	/*
	 * Preload tree normally only loads and not resume, but there is
	 * automatic resume when target is extended, as it's believed
	 * there can be no i/o flying to this 'new' extedend space
	 * from any device above. Reason is that preloaded target above
	 * may actually need to see its bigger subdevice before it
	 * gets suspended. As long as devices are simple linears
	 * there is no problem to resume bigger device in preload (before commit).
	 * However complex targets like thin-pool (raid,cache...)
	 * they shall not be resumed before their commit.
	 */
	unsigned delay_resume_if_extended;

	/*
	 * Call node_send_messages(), set to 2 if there are messages
	 * When != 0, it validates matching transaction id, thus thin-pools
	 * where transation_id is passed as 0 are never validated, this
	 * allows external managment of thin-pool TID.
	 */
	unsigned send_messages;
	/* Skip suspending node's children, used when sending messages to thin-pool */
	int skip_suspend;
};

/* Two of these used to join two nodes with uses and used_by. */
struct dm_tree_link {
	struct dm_list list;
	struct dm_tree_node *node;
};

struct dm_tree_node {
	struct dm_tree *dtree;

	const char *name;
	const char *uuid;
	struct dm_info info;

	struct dm_list uses;       	/* Nodes this node uses */
	struct dm_list used_by;    	/* Nodes that use this node */

	int activation_priority;	/* 0 gets activated first */
	int implicit_deps;		/* 1 device only implicitly referenced */

	uint16_t udev_flags;		/* Udev control flags */

	void *context;			/* External supplied context */

	struct load_properties props;	/* For creation/table (re)load */

	/*
	 * If presuspend of child node is needed
	 * Note: only direct child is allowed
	 */
	struct dm_tree_node *presuspend_node;

	/* Callback */
	dm_node_callback_fn callback;
	void *callback_data;

	/*
	 * TODO:
	 *	Add advanced code which tracks of send ioctls and their
	 *	proper revert operation for more advanced recovery
	 *	Current code serves mostly only to recovery when
	 *	thin pool metadata check fails and command would
	 *	have left active thin data and metadata subvolumes.
	 */
	struct dm_list activated;	/* Head of activated nodes for preload revert */
	struct dm_list activated_list;	/* List of activated nodes for preload revert */
};

struct dm_tree {
	struct dm_pool *mem;
	struct dm_hash_table *devs;
	struct dm_hash_table *uuids;
	struct dm_tree_node root;
	int skip_lockfs;		/* 1 skips lockfs (for non-snapshots) */
	int no_flush;			/* 1 sets noflush (mirrors/multipath) */
	int retry_remove;		/* 1 retries remove if not successful */
	uint32_t cookie;
	char buf[DM_NAME_LEN + 32];	/* print buffer for device_name (major:minor) */
	const char **optional_uuid_suffixes;	/* uuid suffixes ignored when matching */
};

/*
 * Tree functions.
 */
struct dm_tree *dm_tree_create(void)
{
	struct dm_pool *dmem;
	struct dm_tree *dtree;

	if (!(dmem = dm_pool_create("dtree", 1024)) ||
	    !(dtree = dm_pool_zalloc(dmem, sizeof(*dtree)))) {
		log_error("Failed to allocate dtree.");
		if (dmem)
			dm_pool_destroy(dmem);
		return NULL;
	}

	dtree->root.dtree = dtree;
	dm_list_init(&dtree->root.uses);
	dm_list_init(&dtree->root.used_by);
	dm_list_init(&dtree->root.activated);
	dtree->skip_lockfs = 0;
	dtree->no_flush = 0;
	dtree->mem = dmem;
	dtree->optional_uuid_suffixes = NULL;

	if (!(dtree->devs = dm_hash_create(8))) {
		log_error("dtree hash creation failed");
		dm_pool_destroy(dtree->mem);
		return NULL;
	}

	if (!(dtree->uuids = dm_hash_create(32))) {
		log_error("dtree uuid hash creation failed");
		dm_hash_destroy(dtree->devs);
		dm_pool_destroy(dtree->mem);
		return NULL;
	}

	return dtree;
}

void dm_tree_free(struct dm_tree *dtree)
{
	if (!dtree)
		return;

	dm_hash_destroy(dtree->uuids);
	dm_hash_destroy(dtree->devs);
	dm_pool_destroy(dtree->mem);
}

void dm_tree_set_cookie(struct dm_tree_node *node, uint32_t cookie)
{
	node->dtree->cookie = cookie;
}

uint32_t dm_tree_get_cookie(struct dm_tree_node *node)
{
	return node->dtree->cookie;
}

void dm_tree_skip_lockfs(struct dm_tree_node *dnode)
{
	dnode->dtree->skip_lockfs = 1;
}

void dm_tree_use_no_flush_suspend(struct dm_tree_node *dnode)
{
	dnode->dtree->no_flush = 1;
}

void dm_tree_retry_remove(struct dm_tree_node *dnode)
{
	dnode->dtree->retry_remove = 1;
}

/*
 * Node functions.
 */
static int _nodes_are_linked(const struct dm_tree_node *parent,
			     const struct dm_tree_node *child)
{
	struct dm_tree_link *dlink;

	dm_list_iterate_items(dlink, &parent->uses)
		if (dlink->node == child)
			return 1;

	return 0;
}

static int _link(struct dm_list *list, struct dm_tree_node *node)
{
	struct dm_tree_link *dlink;

	if (!(dlink = dm_pool_alloc(node->dtree->mem, sizeof(*dlink)))) {
		log_error("dtree link allocation failed");
		return 0;
	}

	dlink->node = node;
	dm_list_add(list, &dlink->list);

	return 1;
}

static int _link_nodes(struct dm_tree_node *parent,
		       struct dm_tree_node *child)
{
	if (_nodes_are_linked(parent, child))
		return 1;

	if (!_link(&parent->uses, child))
		return 0;

	if (!_link(&child->used_by, parent))
		return 0;

	return 1;
}

static void _unlink(struct dm_list *list, struct dm_tree_node *node)
{
	struct dm_tree_link *dlink;

	dm_list_iterate_items(dlink, list)
		if (dlink->node == node) {
			dm_list_del(&dlink->list);
			break;
		}
}

static void _unlink_nodes(struct dm_tree_node *parent,
			  struct dm_tree_node *child)
{
	if (!_nodes_are_linked(parent, child))
		return;

	_unlink(&parent->uses, child);
	_unlink(&child->used_by, parent);
}

static int _add_to_toplevel(struct dm_tree_node *node)
{
	return _link_nodes(&node->dtree->root, node);
}

static void _remove_from_toplevel(struct dm_tree_node *node)
{
	_unlink_nodes(&node->dtree->root, node);
}

static int _add_to_bottomlevel(struct dm_tree_node *node)
{
	return _link_nodes(node, &node->dtree->root);
}

static void _remove_from_bottomlevel(struct dm_tree_node *node)
{
	_unlink_nodes(node, &node->dtree->root);
}

static int _link_tree_nodes(struct dm_tree_node *parent, struct dm_tree_node *child)
{
	/* Don't link to root node if child already has a parent */
	if (parent == &parent->dtree->root) {
		if (dm_tree_node_num_children(child, 1))
			return 1;
	} else
		_remove_from_toplevel(child);

	if (child == &child->dtree->root) {
		if (dm_tree_node_num_children(parent, 0))
			return 1;
	} else
		_remove_from_bottomlevel(parent);

	return _link_nodes(parent, child);
}

static struct dm_tree_node *_create_dm_tree_node(struct dm_tree *dtree,
						 const char *name,
						 const char *uuid,
						 struct dm_info *info,
						 void *context,
						 uint16_t udev_flags)
{
	struct dm_tree_node *node;
	dev_t dev;

	if (!(node = dm_pool_zalloc(dtree->mem, sizeof(*node))) ||
	    !(node->name = dm_pool_strdup(dtree->mem, name)) ||
	    !(node->uuid = dm_pool_strdup(dtree->mem, uuid))) {
		log_error("_create_dm_tree_node alloc failed.");
		return NULL;
	}

	node->dtree = dtree;
	node->info = *info;
	node->context = context;
	node->udev_flags = udev_flags;

	dm_list_init(&node->uses);
	dm_list_init(&node->used_by);
	dm_list_init(&node->activated);
	dm_list_init(&node->props.segs);

	dev = MKDEV(info->major, info->minor);

	if (!dm_hash_insert_binary(dtree->devs, (const char *) &dev,
				   sizeof(dev), node)) {
		log_error("dtree node hash insertion failed");
		dm_pool_free(dtree->mem, node);
		return NULL;
	}

	if (*uuid && !dm_hash_insert(dtree->uuids, uuid, node)) {
		log_error("dtree uuid hash insertion failed");
		dm_hash_remove_binary(dtree->devs, (const char *) &dev,
				      sizeof(dev));
		dm_pool_free(dtree->mem, node);
		return NULL;
	}

	return node;
}

static struct dm_tree_node *_find_dm_tree_node(struct dm_tree *dtree,
					       uint32_t major, uint32_t minor)
{
	dev_t dev = MKDEV(major, minor);

	return dm_hash_lookup_binary(dtree->devs, (const char *) &dev,
				     sizeof(dev));
}

void dm_tree_set_optional_uuid_suffixes(struct dm_tree *dtree, const char **optional_uuid_suffixes)
{
	dtree->optional_uuid_suffixes = optional_uuid_suffixes;
}

static struct dm_tree_node *_find_dm_tree_node_by_uuid(struct dm_tree *dtree,
						       const char *uuid)
{
	struct dm_tree_node *node;
	const char *default_uuid_prefix;
	size_t default_uuid_prefix_len;
	const char *suffix, *suffix_position;
	char uuid_without_suffix[DM_UUID_LEN];
	unsigned i = 0;
	const char **suffix_list = dtree->optional_uuid_suffixes;

	if ((node = dm_hash_lookup(dtree->uuids, uuid))) {
		log_debug("Matched uuid %s in deptree.", uuid);
		return node;
	}

	default_uuid_prefix = dm_uuid_prefix();
	default_uuid_prefix_len = strlen(default_uuid_prefix);

	if (suffix_list && (suffix_position = rindex(uuid, '-'))) {
		while ((suffix = suffix_list[i++])) {
			if (strcmp(suffix_position + 1, suffix))
				continue;

			(void) strncpy(uuid_without_suffix, uuid, sizeof(uuid_without_suffix));
			uuid_without_suffix[suffix_position - uuid] = '\0';

			if ((node = dm_hash_lookup(dtree->uuids, uuid_without_suffix))) {
				log_debug("Matched uuid %s (missing suffix -%s) in deptree.", uuid_without_suffix, suffix);
				return node;
			}

			break;
		};
	}
	
	if (strncmp(uuid, default_uuid_prefix, default_uuid_prefix_len))
		return NULL;

	if ((node = dm_hash_lookup(dtree->uuids, uuid + default_uuid_prefix_len))) {
		log_debug("Matched uuid %s (missing prefix) in deptree.", uuid + default_uuid_prefix_len);
		return node;
	}

	log_debug("Not matched uuid %s in deptree.", uuid);
	return NULL;
}

/* Return node's device_name (major:minor) for debug messages */
static const char *_node_name(struct dm_tree_node *dnode)
{
	if (dm_snprintf(dnode->dtree->buf, sizeof(dnode->dtree->buf),
			"%s (" FMTu32 ":" FMTu32 ")",
			dnode->name ? dnode->name : "",
			dnode->info.major, dnode->info.minor) < 0) {
		stack;
		return dnode->name;
	}

	return dnode->dtree->buf;
}

void dm_tree_node_set_udev_flags(struct dm_tree_node *dnode, uint16_t udev_flags)

{
	if (udev_flags != dnode->udev_flags)
		log_debug_activation("Resetting %s udev_flags from 0x%x to 0x%x.",
				     _node_name(dnode),
				     dnode->udev_flags, udev_flags);
	dnode->udev_flags = udev_flags;
}

void dm_tree_node_set_read_ahead(struct dm_tree_node *dnode,
				 uint32_t read_ahead,
				 uint32_t read_ahead_flags)
{
	dnode->props.read_ahead = read_ahead;
	dnode->props.read_ahead_flags = read_ahead_flags;
}

void dm_tree_node_set_presuspend_node(struct dm_tree_node *node,
				      struct dm_tree_node *presuspend_node)
{
	node->presuspend_node = presuspend_node;
}

const char *dm_tree_node_get_name(const struct dm_tree_node *node)
{
	return node->info.exists ? node->name : "";
}

const char *dm_tree_node_get_uuid(const struct dm_tree_node *node)
{
	return node->info.exists ? node->uuid : "";
}

const struct dm_info *dm_tree_node_get_info(const struct dm_tree_node *node)
{
	return &node->info;
}

void *dm_tree_node_get_context(const struct dm_tree_node *node)
{
	return node->context;
}

int dm_tree_node_size_changed(const struct dm_tree_node *dnode)
{
	return dnode->props.size_changed;
}

int dm_tree_node_num_children(const struct dm_tree_node *node, uint32_t inverted)
{
	if (inverted) {
		if (_nodes_are_linked(&node->dtree->root, node))
			return 0;
		return dm_list_size(&node->used_by);
	}

	if (_nodes_are_linked(node, &node->dtree->root))
		return 0;

	return dm_list_size(&node->uses);
}

/*
 * Returns 1 if no prefix supplied
 */
static int _uuid_prefix_matches(const char *uuid, const char *uuid_prefix, size_t uuid_prefix_len)
{
	const char *default_uuid_prefix = dm_uuid_prefix();
	size_t default_uuid_prefix_len = strlen(default_uuid_prefix);

	if (!uuid_prefix)
		return 1;

	if (!strncmp(uuid, uuid_prefix, uuid_prefix_len))
		return 1;

	/* Handle transition: active device uuids might be missing the prefix */
	if (uuid_prefix_len <= 4)
		return 0;

	if (!strncmp(uuid, default_uuid_prefix, default_uuid_prefix_len))
		return 0;

	if (strncmp(uuid_prefix, default_uuid_prefix, default_uuid_prefix_len))
		return 0;

	if (!strncmp(uuid, uuid_prefix + default_uuid_prefix_len, uuid_prefix_len - default_uuid_prefix_len))
		return 1;

	return 0;
}

/*
 * Returns 1 if no children.
 */
static int _children_suspended(struct dm_tree_node *node,
			       uint32_t inverted,
			       const char *uuid_prefix,
			       size_t uuid_prefix_len)
{
	struct dm_list *list;
	struct dm_tree_link *dlink;
	const struct dm_info *dinfo;
	const char *uuid;

	if (inverted) {
		if (_nodes_are_linked(&node->dtree->root, node))
			return 1;
		list = &node->used_by;
	} else {
		if (_nodes_are_linked(node, &node->dtree->root))
			return 1;
		list = &node->uses;
	}

	dm_list_iterate_items(dlink, list) {
		if (!(uuid = dm_tree_node_get_uuid(dlink->node))) {
			stack;
			continue;
		}

		/* Ignore if it doesn't belong to this VG */
		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		/* Ignore if parent node wants to presuspend this node */
		if (dlink->node->presuspend_node == node)
			continue;

		if (!(dinfo = dm_tree_node_get_info(dlink->node)))
			return_0;	/* FIXME Is this normal? */

		if (!dinfo->suspended)
			return 0;
	}

	return 1;
}

/*
 * Set major and minor to zero for root of tree.
 */
struct dm_tree_node *dm_tree_find_node(struct dm_tree *dtree,
				       uint32_t major,
				       uint32_t minor)
{
	if (!major && !minor)
		return &dtree->root;

	return _find_dm_tree_node(dtree, major, minor);
}

/*
 * Set uuid to NULL for root of tree.
 */
struct dm_tree_node *dm_tree_find_node_by_uuid(struct dm_tree *dtree,
					       const char *uuid)
{
	if (!uuid || !*uuid)
		return &dtree->root;

	return _find_dm_tree_node_by_uuid(dtree, uuid);
}

/*
 * First time set *handle to NULL.
 * Set inverted to invert the tree.
 */
struct dm_tree_node *dm_tree_next_child(void **handle,
					const struct dm_tree_node *parent,
					uint32_t inverted)
{
	struct dm_list **dlink = (struct dm_list **) handle;
	const struct dm_list *use_list;

	if (inverted)
		use_list = &parent->used_by;
	else
		use_list = &parent->uses;

	if (!*dlink)
		*dlink = dm_list_first(use_list);
	else
		*dlink = dm_list_next(use_list, *dlink);

	return (*dlink) ? dm_list_item(*dlink, struct dm_tree_link)->node : NULL;
}

static int _deps(struct dm_task **dmt, struct dm_pool *mem, uint32_t major, uint32_t minor,
		 const char **name, const char **uuid, unsigned inactive_table,
		 struct dm_info *info, struct dm_deps **deps)
{
	memset(info, 0, sizeof(*info));
	*name = "";
	*uuid = "";
	*deps = NULL;

	if (!dm_is_dm_major(major)) {
		info->major = major;
		info->minor = minor;
		return 1;
	}

	if (!(*dmt = dm_task_create(DM_DEVICE_DEPS)))
		return_0;

	if (!dm_task_set_major(*dmt, major) || !dm_task_set_minor(*dmt, minor)) {
		log_error("_deps: failed to set major:minor for (" FMTu32 ":" FMTu32 ").",
			  major, minor);
		goto failed;
	}

	if (inactive_table && !dm_task_query_inactive_table(*dmt)) {
		log_error("_deps: failed to set inactive table for (%" PRIu32 ":%" PRIu32 ")",
			  major, minor);
		goto failed;
	}

	if (!dm_task_run(*dmt)) {
		log_error("_deps: task run failed for (%" PRIu32 ":%" PRIu32 ")",
			  major, minor);
		goto failed;
	}

	if (!dm_task_get_info(*dmt, info)) {
		log_error("_deps: failed to get info for (%" PRIu32 ":%" PRIu32 ")",
			  major, minor);
		goto failed;
	}

	if (info->exists) {
		if (info->major != major) {
			log_error("Inconsistent dtree major number: %u != %u",
				  major, info->major);
			goto failed;
		}
		if (info->minor != minor) {
			log_error("Inconsistent dtree minor number: %u != %u",
				  minor, info->minor);
			goto failed;
		}
		*name = dm_task_get_name(*dmt);
		*uuid = dm_task_get_uuid(*dmt);
		*deps = dm_task_get_deps(*dmt);
	}

	return 1;

failed:
	dm_task_destroy(*dmt);
	*dmt = NULL;

	return 0;
}

/*
 * Deactivate a device with its dependencies if the uuid prefix matches.
 */
static int _info_by_dev(uint32_t major, uint32_t minor, int with_open_count,
			struct dm_info *info, struct dm_pool *mem,
			const char **name, const char **uuid)
{
	struct dm_task *dmt;
	int r = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_INFO)))
		return_0;

	if (!dm_task_set_major(dmt, major) || !dm_task_set_minor(dmt, minor)) {
		log_error("_info_by_dev: Failed to set device number.");
		goto out;
	}

	if (!with_open_count && !dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (!dm_task_run(dmt))
		goto_out;

	if (!dm_task_get_info(dmt, info))
		goto_out;

	if (name && !(*name = dm_pool_strdup(mem, dm_task_get_name(dmt)))) {
		log_error("name pool_strdup failed");
		goto out;
	}

	if (uuid && !(*uuid = dm_pool_strdup(mem, dm_task_get_uuid(dmt)))) {
		log_error("uuid pool_strdup failed");
		goto out;
	}

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

static int _check_device_not_in_use(const char *name, struct dm_info *info)
{
	const char *reason;

	if (!info->exists)
		return 1;

	/* If sysfs is not used, use open_count information only. */
	if (!*dm_sysfs_dir()) {
		if (!info->open_count)
			return 1;
		reason = "in use";
	} else if (dm_device_has_holders(info->major, info->minor))
		reason = "is used by another device";
	else if (dm_device_has_mounted_fs(info->major, info->minor))
		reason = "constains a filesystem in use";
	else
		return 1;

	log_error("Device %s (" FMTu32 ":" FMTu32 ") %s.",
		  name, info->major, info->minor, reason);
	return 0;
}

/* Check if all parent nodes of given node have open_count == 0 */
static int _node_has_closed_parents(struct dm_tree_node *node,
				    const char *uuid_prefix,
				    size_t uuid_prefix_len)
{
	struct dm_tree_link *dlink;
	const struct dm_info *dinfo;
	struct dm_info info;
	const char *uuid;

	/* Iterate through parents of this node */
	dm_list_iterate_items(dlink, &node->used_by) {
		if (!(uuid = dm_tree_node_get_uuid(dlink->node))) {
			stack;
			continue;
		}

		/* Ignore if it doesn't belong to this VG */
		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		if (!(dinfo = dm_tree_node_get_info(dlink->node)))
			return_0;	/* FIXME Is this normal? */

		/* Refresh open_count */
		if (!_info_by_dev(dinfo->major, dinfo->minor, 1, &info, NULL, NULL, NULL))
			return_0;

		if (!info.exists)
			continue;

		if (info.open_count) {
			log_debug_activation("Node %s %d:%d has open_count %d", uuid_prefix,
					     dinfo->major, dinfo->minor, info.open_count);
			return 0;
		}
	}

	return 1;
}

static int _deactivate_node(const char *name, uint32_t major, uint32_t minor,
			    uint32_t *cookie, uint16_t udev_flags, int retry)
{
	struct dm_task *dmt;
	int r = 0;

	log_verbose("Removing %s (%" PRIu32 ":%" PRIu32 ")", name, major, minor);

	if (!(dmt = dm_task_create(DM_DEVICE_REMOVE))) {
		log_error("Deactivation dm_task creation failed for %s", name);
		return 0;
	}

	if (!dm_task_set_major(dmt, major) || !dm_task_set_minor(dmt, minor)) {
		log_error("Failed to set device number for %s deactivation", name);
		goto out;
	}

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (cookie)
		if (!dm_task_set_cookie(dmt, cookie, udev_flags))
			goto out;

	if (retry)
		dm_task_retry_remove(dmt);

	r = dm_task_run(dmt);

	/* FIXME Until kernel returns actual name so dm-iface.c can handle it */
	rm_dev_node(name, dmt->cookie_set && !(udev_flags & DM_UDEV_DISABLE_DM_RULES_FLAG),
		    dmt->cookie_set && (udev_flags & DM_UDEV_DISABLE_LIBRARY_FALLBACK));

	/* FIXME Remove node from tree or mark invalid? */

out:
	dm_task_destroy(dmt);

	return r;
}

static int _node_clear_table(struct dm_tree_node *dnode, uint16_t udev_flags)
{
	struct dm_task *dmt = NULL, *deps_dmt = NULL;
	struct dm_info *info = &dnode->info, deps_info;
	struct dm_deps *deps = NULL;
	const char *name, *uuid, *depname, *depuuid;
	const char *default_uuid_prefix;
	size_t default_uuid_prefix_len;
	uint32_t i;
	int r = 0;

	if (!(name = dm_tree_node_get_name(dnode))) {
		log_error("_node_clear_table failed: missing name");
		return 0;
	}

	/* Is there a table? */
	if (!info->exists || !info->inactive_table)
		return 1;

	/* Get devices used by inactive table that's about to be deleted. */
	if (!_deps(&deps_dmt, dnode->dtree->mem, info->major, info->minor, &depname, &depuuid, 1, info, &deps)) {
		log_error("Failed to obtain dependencies for %s before clearing table.", name);
		return 0;
	}

	log_verbose("Clearing inactive table %s (%" PRIu32 ":%" PRIu32 ")",
		    name, info->major, info->minor);

	if (!(dmt = dm_task_create(DM_DEVICE_CLEAR))) {
		log_error("Table clear dm_task creation failed for %s", name);
		goto out;
	}

	if (!dm_task_set_major(dmt, info->major) ||
	    !dm_task_set_minor(dmt, info->minor)) {
		log_error("Failed to set device number for %s table clear", name);
		goto out;
	}

	r = dm_task_run(dmt);

	if (!dm_task_get_info(dmt, info)) {
		log_error("_node_clear_table failed: info missing after running task for %s", name);
		r = 0;
	}

	if (!r || !deps)
		goto_out;

	/*
	 * Remove (incomplete) devices that the inactive table referred to but
	 * which are not in the tree, no longer referenced and don't have a live
	 * table.
	 */
	default_uuid_prefix = dm_uuid_prefix();
	default_uuid_prefix_len = strlen(default_uuid_prefix);

	for (i = 0; i < deps->count; i++) {
		/* If already in tree, assume it's under control */
		if (_find_dm_tree_node(dnode->dtree, MAJOR(deps->device[i]), MINOR(deps->device[i])))
			continue;

		if (!_info_by_dev(MAJOR(deps->device[i]), MINOR(deps->device[i]), 1,
				  &deps_info, dnode->dtree->mem, &name, &uuid))
			goto_out;

		/* Proceed if device is an 'orphan' - unreferenced and without a live table. */
		if (!deps_info.exists || deps_info.live_table || deps_info.open_count)
			continue;

		if (strncmp(uuid, default_uuid_prefix, default_uuid_prefix_len))
			continue;

		/* Remove device. */
		if (!_deactivate_node(name, deps_info.major, deps_info.minor, &dnode->dtree->cookie, udev_flags, 0)) {
			log_error("Failed to deactivate no-longer-used device %s (%"
				  PRIu32 ":%" PRIu32 ")", name, deps_info.major, deps_info.minor);
		} else if (deps_info.suspended)
			dec_suspended();
	}

out:
	if (dmt)
		dm_task_destroy(dmt);

	if (deps_dmt)
		dm_task_destroy(deps_dmt);

	return r;
}

struct dm_tree_node *dm_tree_add_new_dev_with_udev_flags(struct dm_tree *dtree,
							 const char *name,
							 const char *uuid,
							 uint32_t major,
							 uint32_t minor,
							 int read_only,
							 int clear_inactive,
							 void *context,
							 uint16_t udev_flags)
{
	struct dm_tree_node *dnode;
	struct dm_info info = { 0 };

	if (!name || !uuid) {
		log_error("Cannot add device without name and uuid.");
		return NULL;
	}

	/* Do we need to add node to tree? */
	if (!(dnode = dm_tree_find_node_by_uuid(dtree, uuid))) {
		if (!(dnode = _create_dm_tree_node(dtree, name, uuid, &info,
						   context, 0)))
			return_NULL;

		/* Attach to root node until a table is supplied */
		if (!_add_to_toplevel(dnode) || !_add_to_bottomlevel(dnode))
			return_NULL;

		dnode->props.major = major;
		dnode->props.minor = minor;
	} else if (strcmp(name, dnode->name)) {
		/* Do we need to rename node? */
		if (!(dnode->props.new_name = dm_pool_strdup(dtree->mem, name))) {
			log_error("name pool_strdup failed");
			return NULL;
		}
	}

	dnode->props.read_only = read_only ? 1 : 0;
	dnode->props.read_ahead = DM_READ_AHEAD_AUTO;
	dnode->props.read_ahead_flags = 0;

	if (clear_inactive && !_node_clear_table(dnode, udev_flags))
		return_NULL;

	dnode->context = context;
	dnode->udev_flags = udev_flags;

	return dnode;
}

struct dm_tree_node *dm_tree_add_new_dev(struct dm_tree *dtree, const char *name,
					 const char *uuid, uint32_t major, uint32_t minor,
					 int read_only, int clear_inactive, void *context)
{
	return dm_tree_add_new_dev_with_udev_flags(dtree, name, uuid, major, minor,
						   read_only, clear_inactive, context, 0);
}

static struct dm_tree_node *_add_dev(struct dm_tree *dtree,
				     struct dm_tree_node *parent,
				     uint32_t major, uint32_t minor,
				     uint16_t udev_flags,
				     int implicit_deps)
{
	struct dm_task *dmt = NULL;
	struct dm_info info;
	struct dm_deps *deps = NULL;
	const char *name = NULL;
	const char *uuid = NULL;
	struct dm_tree_node *node = NULL;
	uint32_t i;
	int new = 0;

	/* Already in tree? */
	if (!(node = _find_dm_tree_node(dtree, major, minor))) {
		if (!_deps(&dmt, dtree->mem, major, minor, &name, &uuid, 0, &info, &deps))
			return_NULL;

		if (!(node = _create_dm_tree_node(dtree, name, uuid, &info,
						  NULL, udev_flags)))
			goto_out;
		new = 1;
		node->implicit_deps = implicit_deps;
	} else if (!implicit_deps && node->implicit_deps) {
		node->udev_flags = udev_flags;
		node->implicit_deps = 0;
	}

	if (!_link_tree_nodes(parent, node)) {
		node = NULL;
		goto_out;
	}

	/* If node was already in tree, no need to recurse. */
	if (!new)
		goto out;

	/* Can't recurse if not a mapped device or there are no dependencies */
	if (!node->info.exists || !deps || !deps->count) {
		if (!_add_to_bottomlevel(node)) {
			stack;
			node = NULL;
		}
		goto out;
	}

	/* Add dependencies to tree */
	for (i = 0; i < deps->count; i++)
		/* Implicit devices are by default temporary */
		if (!_add_dev(dtree, node, MAJOR(deps->device[i]),
			      MINOR(deps->device[i]), udev_flags |
			      DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG |
			      DM_UDEV_DISABLE_DISK_RULES_FLAG |
			      DM_UDEV_DISABLE_OTHER_RULES_FLAG, 1)) {
			node = NULL;
			goto_out;
		}

out:
	if (dmt)
		dm_task_destroy(dmt);

	return node;
}

int dm_tree_add_dev(struct dm_tree *dtree, uint32_t major, uint32_t minor)
{
	return _add_dev(dtree, &dtree->root, major, minor, 0, 0) ? 1 : 0;
}

int dm_tree_add_dev_with_udev_flags(struct dm_tree *dtree, uint32_t major,
				    uint32_t minor, uint16_t udev_flags)
{
	return _add_dev(dtree, &dtree->root, major, minor, udev_flags, 0) ? 1 : 0;
}

static int _rename_node(const char *old_name, const char *new_name, uint32_t major,
			uint32_t minor, uint32_t *cookie, uint16_t udev_flags)
{
	struct dm_task *dmt;
	int r = 0;

	log_verbose("Renaming %s (%" PRIu32 ":%" PRIu32 ") to %s", old_name, major, minor, new_name);

	if (!(dmt = dm_task_create(DM_DEVICE_RENAME))) {
		log_error("Rename dm_task creation failed for %s", old_name);
		return 0;
	}

	if (!dm_task_set_name(dmt, old_name)) {
		log_error("Failed to set name for %s rename.", old_name);
		goto out;
	}

	if (!dm_task_set_newname(dmt, new_name))
		goto_out;

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (!dm_task_set_cookie(dmt, cookie, udev_flags))
		goto out;

	r = dm_task_run(dmt);

out:
	dm_task_destroy(dmt);

	return r;
}

/* FIXME Merge with _suspend_node? */
static int _resume_node(const char *name, uint32_t major, uint32_t minor,
			uint32_t read_ahead, uint32_t read_ahead_flags,
			struct dm_info *newinfo, uint32_t *cookie,
			uint16_t udev_flags, int already_suspended)
{
	struct dm_task *dmt;
	int r = 0;

	log_verbose("Resuming %s (" FMTu32 ":" FMTu32 ").", name, major, minor);

	if (!(dmt = dm_task_create(DM_DEVICE_RESUME))) {
		log_debug_activation("Suspend dm_task creation failed for %s.", name);
		return 0;
	}

	/* FIXME Kernel should fill in name on return instead */
	if (!dm_task_set_name(dmt, name)) {
		log_debug_activation("Failed to set device name for %s resumption.", name);
		goto out;
	}

	if (!dm_task_set_major(dmt, major) || !dm_task_set_minor(dmt, minor)) {
		log_error("Failed to set device number for %s resumption.", name);
		goto out;
	}

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (!dm_task_set_read_ahead(dmt, read_ahead, read_ahead_flags))
		log_warn("WARNING: Failed to set read ahead.");

	if (!dm_task_set_cookie(dmt, cookie, udev_flags))
		goto_out;

	if (!(r = dm_task_run(dmt)))
		goto_out;

	if (already_suspended)
		dec_suspended();

	if (!(r = dm_task_get_info(dmt, newinfo)))
		stack;

out:
	dm_task_destroy(dmt);

	return r;
}

static int _suspend_node(const char *name, uint32_t major, uint32_t minor,
			 int skip_lockfs, int no_flush, struct dm_info *newinfo)
{
	struct dm_task *dmt;
	int r = 0;

	log_verbose("Suspending %s (%" PRIu32 ":%" PRIu32 ")%s%s",
		    name, major, minor,
		    skip_lockfs ? "" : " with filesystem sync",
		    no_flush ? "" : " with device flush");

	if (!(dmt = dm_task_create(DM_DEVICE_SUSPEND))) {
		log_error("Suspend dm_task creation failed for %s", name);
		return 0;
	}

	if (!dm_task_set_major(dmt, major) || !dm_task_set_minor(dmt, minor)) {
		log_error("Failed to set device number for %s suspension.", name);
		goto out;
	}

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (skip_lockfs && !dm_task_skip_lockfs(dmt))
		log_warn("WARNING: Failed to set skip_lockfs flag.");

	if (no_flush && !dm_task_no_flush(dmt))
		log_warn("WARNING: Failed to set no_flush flag.");

	if ((r = dm_task_run(dmt))) {
		inc_suspended();
		r = dm_task_get_info(dmt, newinfo);
	}
out:
	dm_task_destroy(dmt);

	return r;
}

static struct dm_task *_dm_task_create_device_status(uint32_t major, uint32_t minor)
{
	struct dm_task *dmt;

	if (!(dmt = dm_task_create(DM_DEVICE_STATUS)))
		return_NULL;

	if (!dm_task_set_major(dmt, major) || !dm_task_set_minor(dmt, minor)) {
		log_error("Failed to set major minor.");
		goto out;
	}

	if (!dm_task_no_flush(dmt))
		log_warn("WARNING: Can't set no_flush flag."); /* Non fatal */

	if (!dm_task_run(dmt))
		goto_out;

	return dmt;
out:
	dm_task_destroy(dmt);

	return NULL;
}

static int _thin_pool_get_status(struct dm_tree_node *dnode,
				 struct dm_status_thin_pool *s)
{
	struct dm_task *dmt;
	int r = 0;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;

	if (!(dmt = _dm_task_create_device_status(dnode->info.major,
						  dnode->info.minor)))
		return_0;

	dm_get_next_target(dmt, NULL, &start, &length, &type, &params);

	if (!type || (strcmp(type, "thin-pool") != 0)) {
		log_error("Expected thin-pool target for %s and got %s.",
			  _node_name(dnode), type ? : "no target");
		goto out;
	}

	if (!parse_thin_pool_status(params, s))
		goto_out;

	log_debug_activation("Found transaction id %" PRIu64 " for thin pool %s "
			     "with status line: %s.",
			     s->transaction_id, _node_name(dnode), params);

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

static int _vdo_get_status(struct dm_tree_node *dnode,
			   struct dm_vdo_status_parse_result *s)
{
	struct dm_task *dmt;
	int r = 0;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;

	if (!(dmt = _dm_task_create_device_status(dnode->info.major,
						  dnode->info.minor)))
		return_0;

	dm_get_next_target(dmt, NULL, &start, &length, &type, &params);

	if (!type || (strcmp(type, "vdo") != 0)) {
		log_error("Expected vdo target for %s and got %s.",
			  _node_name(dnode), type ? : "no target");
		goto out;
	}

	log_debug("Parsing VDO status: %s", params);

	if (!dm_vdo_status_parse(NULL, params, s))
		goto_out;

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

static int _node_message(uint32_t major, uint32_t minor,
			 int expected_errno, const char *message)
{
	struct dm_task *dmt;
	int r = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_TARGET_MSG)))
		return_0;

	if (!dm_task_set_major(dmt, major) ||
	    !dm_task_set_minor(dmt, minor)) {
		log_error("Failed to set message major minor.");
		goto out;
	}

	if (!dm_task_set_message(dmt, message))
		goto_out;

	/* Internal functionality of dm_task */
	dmt->expected_errno = expected_errno;

	if (!dm_task_run(dmt)) {
		log_error("Failed to process message \"%s\".", message);
		goto out;
	}

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}


static int _thin_pool_node_message(struct dm_tree_node *dnode, struct thin_message *tm)
{
	struct dm_thin_message *m = &tm->message;
	char buf[64];
	int r;

	switch (m->type) {
	case DM_THIN_MESSAGE_CREATE_SNAP:
		r = dm_snprintf(buf, sizeof(buf), "create_snap %u %u",
				m->u.m_create_snap.device_id,
				m->u.m_create_snap.origin_id);
		break;
	case DM_THIN_MESSAGE_CREATE_THIN:
		r = dm_snprintf(buf, sizeof(buf), "create_thin %u",
				m->u.m_create_thin.device_id);
		break;
	case DM_THIN_MESSAGE_DELETE:
		r = dm_snprintf(buf, sizeof(buf), "delete %u",
				m->u.m_delete.device_id);
		break;
	case DM_THIN_MESSAGE_SET_TRANSACTION_ID:
		r = dm_snprintf(buf, sizeof(buf),
				"set_transaction_id %" PRIu64 " %" PRIu64,
				m->u.m_set_transaction_id.current_id,
				m->u.m_set_transaction_id.new_id);
		break;
	case DM_THIN_MESSAGE_RESERVE_METADATA_SNAP: /* target vsn 1.1 */
		r = dm_snprintf(buf, sizeof(buf), "reserve_metadata_snap");
		break;
	case DM_THIN_MESSAGE_RELEASE_METADATA_SNAP: /* target vsn 1.1 */
		r = dm_snprintf(buf, sizeof(buf), "release_metadata_snap");
		break;
	default:
		r = -1;
	}

	if (r < 0) {
		log_error("Failed to prepare message.");
		return 0;
	}

	if (!_node_message(dnode->info.major, dnode->info.minor,
			   tm->expected_errno, buf))
		return_0;

	return 1;
}

static struct load_segment *_get_last_load_segment(struct dm_tree_node *node)
{
	if (dm_list_empty(&node->props.segs)) {
		log_error("Node %s is missing a segment.", _node_name(node));
		return NULL;
	}

	return dm_list_item(dm_list_last(&node->props.segs), struct load_segment);
}

/* For preload pass only validate pool's transaction_id */
static int _thin_pool_node_send_messages(struct dm_tree_node *dnode,
					 struct load_segment *seg,
					 int send)
{
	struct thin_message *tmsg;
	struct dm_status_thin_pool stp;
	int have_messages;

	if (!_thin_pool_get_status(dnode, &stp))
		return_0;

	have_messages = !dm_list_empty(&seg->thin_messages) ? 1 : 0;
	if (stp.transaction_id == seg->transaction_id) {
		dnode->props.send_messages = 0; /* messages already committed */
		if (have_messages)
			log_debug_activation("Thin pool %s transaction_id matches %"
					     PRIu64 ", skipping messages.",
					     _node_name(dnode), stp.transaction_id);
		return 1;
	}

	/* Error if there are no stacked messages or id mismatches */
	if ((stp.transaction_id + 1) != seg->transaction_id) {
		log_error("Thin pool %s transaction_id is %" PRIu64 ", while expected %" PRIu64 ".",
			  _node_name(dnode), stp.transaction_id, seg->transaction_id - have_messages);
		return 0;
	}

	if (!have_messages || !send)
		return 1; /* transaction_id is matching */

	dm_list_iterate_items(tmsg, &seg->thin_messages) {
		if (!(_thin_pool_node_message(dnode, tmsg)))
			return_0;
		if (tmsg->message.type == DM_THIN_MESSAGE_SET_TRANSACTION_ID) {
			if (!_thin_pool_get_status(dnode, &stp))
				return_0;
			if (stp.transaction_id != tmsg->message.u.m_set_transaction_id.new_id) {
				log_error("Thin pool %s transaction_id is %" PRIu64
					  " and does not match expected  %" PRIu64 ".",
					  _node_name(dnode), stp.transaction_id,
					  tmsg->message.u.m_set_transaction_id.new_id);
				return 0;
			}
		}
	}

	dnode->props.send_messages = 0; /* messages posted */

	return 1;
}

static int _vdo_node_send_messages(struct dm_tree_node *dnode,
				   struct load_segment *seg,
				   int send)
{
	struct dm_vdo_status_parse_result vdo_status;
	int send_compression_message = 0;
	int send_deduplication_message = 0;
	int r = 0;

	if (!_vdo_get_status(dnode, &vdo_status))
		return_0;

	if (seg->vdo_params.use_compression) {
		if (vdo_status.status->compression_state == DM_VDO_COMPRESSION_OFFLINE)
			send_compression_message = 1;
	} else if (vdo_status.status->compression_state != DM_VDO_COMPRESSION_OFFLINE)
		send_compression_message = 1;

	if (seg->vdo_params.use_deduplication) {
		if (vdo_status.status->index_state == DM_VDO_INDEX_OFFLINE)
			send_deduplication_message = 1;
	} else if (vdo_status.status->index_state != DM_VDO_INDEX_OFFLINE)
		send_deduplication_message = 1;

	log_debug("VDO needs message for compression %u(%u) and deduplication %u(%u).",
		  send_compression_message, vdo_status.status->index_state,
		  send_deduplication_message, vdo_status.status->compression_state);

	if (send_compression_message &&
	    !_node_message(dnode->info.major, dnode->info.minor, 0,
			   seg->vdo_params.use_compression ?
			   "compression on" : "compression off"))
		goto_out;

	if (send_deduplication_message &&
	    !_node_message(dnode->info.major, dnode->info.minor, 0,
			   seg->vdo_params.use_deduplication ?
			   "index-enable" : "index-disable"))
		goto_out;

	r = 1;
out:
	free(vdo_status.status->device);
	free(vdo_status.status);

	return r;
}


static int _node_send_messages(struct dm_tree_node *dnode,
			       const char *uuid_prefix,
			       size_t uuid_prefix_len,
			       int send)
{
	struct load_segment *seg;
	const char *uuid;

	if (!dnode->info.exists || !dnode->info.live_table)
		return 1;

	if (!(uuid = dm_tree_node_get_uuid(dnode)))
		return_0;

	if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len)) {
		log_debug_activation("UUID \"%s\" does not match.", uuid);
		return 1;
	}

	if (!(seg = _get_last_load_segment(dnode)))
		return_0;

	switch (seg->type) {
	case SEG_THIN_POOL: return _thin_pool_node_send_messages(dnode, seg, send);
	case SEG_VDO:	    return _vdo_node_send_messages(dnode, seg, send);
	}

	return 1;
}


/*
 * FIXME Don't attempt to deactivate known internal dependencies.
 */
static int _dm_tree_deactivate_children(struct dm_tree_node *dnode,
					const char *uuid_prefix,
					size_t uuid_prefix_len,
					unsigned level)
{
	int r = 1;
	void *handle = NULL;
	struct dm_tree_node *child = dnode;
	struct dm_info info;
	const struct dm_info *dinfo;
	const char *name;
	const char *uuid;

	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		if (!(dinfo = dm_tree_node_get_info(child))) {
			stack;
			continue;
		}

		if (!(name = dm_tree_node_get_name(child))) {
			stack;
			continue;
		}

		if (!(uuid = dm_tree_node_get_uuid(child))) {
			stack;
			continue;
		}

		/* Ignore if it doesn't belong to this VG */
		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		/* Refresh open_count */
		if (!_info_by_dev(dinfo->major, dinfo->minor, 1, &info, NULL, NULL, NULL))
			return_0;

		if (!info.exists)
			continue;

		if (info.open_count) {
			/* Skip internal non-toplevel opened nodes */
			/* On some old udev systems without corrrect udev rules
			 * this hack avoids 'leaking' active _mimageX legs after
			 * deactivation of mirror LV. Other suffixes are not added
			 * since it's expected newer systems with wider range of
			 * supported targets also use better udev */
			if (level && !strstr(name, "_mimage"))
				continue;

			/* When retry is not allowed, error */
			if (!child->dtree->retry_remove) {
				log_error("Unable to deactivate open %s (" FMTu32 ":"
					  FMTu32 ").", name, info.major, info.minor);
				r = 0;
				continue;
			}

			/* Check toplevel node for holders/mounted fs */
			if (!_check_device_not_in_use(name, &info)) {
				stack;
				r = 0;
				continue;
			}
			/* Go on with retry */
		}

		/* Also checking open_count in parent nodes of presuspend_node */
		if ((child->presuspend_node &&
		     !_node_has_closed_parents(child->presuspend_node,
					       uuid_prefix, uuid_prefix_len))) {
			/* Only report error from (likely non-internal) dependency at top level */
			if (!level) {
				log_error("Unable to deactivate open %s (" FMTu32 ":"
					  FMTu32 ").", name, info.major, info.minor);
				r = 0;
			}
			continue;
		}

		/* Suspend child node first if requested */
		if (child->presuspend_node &&
		    !dm_tree_suspend_children(child, uuid_prefix, uuid_prefix_len))
			continue;

		if (!_deactivate_node(name, info.major, info.minor,
				      &child->dtree->cookie, child->udev_flags,
				      child->dtree->retry_remove)) {
			log_error("Unable to deactivate %s (" FMTu32 ":"
				  FMTu32 ").", name, info.major, info.minor);
			r = 0;
			continue;
		}

		if (info.suspended && info.live_table)
			dec_suspended();

		if (child->callback &&
		    !child->callback(child, DM_NODE_CALLBACK_DEACTIVATED,
				     child->callback_data))
			stack;
			/* FIXME Deactivation must currently ignore failure
			 * here so that lvremove can continue: we need an
			 * alternative way to handle this state without 
			 * setting r=0.  Or better, skip calling thin_check
			 * entirely if the device is about to be removed. */

		if (dm_tree_node_num_children(child, 0) &&
		    !_dm_tree_deactivate_children(child, uuid_prefix, uuid_prefix_len, level + 1))
			return_0;
	}

	return r;
}

int dm_tree_deactivate_children(struct dm_tree_node *dnode,
				const char *uuid_prefix,
				size_t uuid_prefix_len)
{
	return _dm_tree_deactivate_children(dnode, uuid_prefix, uuid_prefix_len, 0);
}

int dm_tree_suspend_children(struct dm_tree_node *dnode,
			     const char *uuid_prefix,
			     size_t uuid_prefix_len)
{
	int r = 1;
	void *handle = NULL;
	struct dm_tree_node *child = dnode;
	struct dm_info info, newinfo;
	const struct dm_info *dinfo;
	const char *name;
	const char *uuid;

	/* Suspend nodes at this level of the tree */
	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		if (!(dinfo = dm_tree_node_get_info(child))) {
			stack;
			continue;
		}

		if (!(name = dm_tree_node_get_name(child))) {
			stack;
			continue;
		}

		if (!(uuid = dm_tree_node_get_uuid(child))) {
			stack;
			continue;
		}

		/* Ignore if it doesn't belong to this VG */
		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		/* Ensure immediate parents are already suspended */
		if (!_children_suspended(child, 1, uuid_prefix, uuid_prefix_len))
			continue;

		if (!_info_by_dev(dinfo->major, dinfo->minor, 0, &info, NULL, NULL, NULL))
			return_0;

		if (!info.exists || info.suspended)
			continue;

		/* If child has some real messages send them */
		if ((child->props.send_messages > 1) && r) {
			if (!(r = _node_send_messages(child, uuid_prefix, uuid_prefix_len, 1)))
				stack;
			else {
				log_debug_activation("Sent messages to thin-pool %s and "
						     "skipping suspend of its children.",
						     _node_name(child));
				child->props.skip_suspend++;
			}
			continue;
		}

		if (!_suspend_node(name, info.major, info.minor,
				   child->dtree->skip_lockfs,
				   child->dtree->no_flush, &newinfo)) {
			log_error("Unable to suspend %s (" FMTu32 ":"
				  FMTu32 ")", name, info.major, info.minor);
			r = 0;
			continue;
		}

		/* Update cached info */
		child->info = newinfo;
	}

	/* Then suspend any child nodes */
	handle = NULL;

	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		if (child->props.skip_suspend)
			continue;

		if (!(uuid = dm_tree_node_get_uuid(child))) {
			stack;
			continue;
		}

		/* Ignore if it doesn't belong to this VG */
		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		if (dm_tree_node_num_children(child, 0))
			if (!dm_tree_suspend_children(child, uuid_prefix, uuid_prefix_len))
				return_0;
	}

	return r;
}

/*
 * _rename_conflict_exists
 * @dnode
 * @node
 * @resolvable
 *
 * Check if there is a rename conflict with existing peers in
 * this tree.  'resolvable' is set if the conflicting node will
 * also be undergoing a rename.  (Allowing that node to rename
 * first would clear the conflict.)
 *
 * Returns: 1 if conflict, 0 otherwise
 */
static int _rename_conflict_exists(struct dm_tree_node *parent,
				 struct dm_tree_node *node,
				 int *resolvable)
{
	void *handle = NULL;
	const char *name = dm_tree_node_get_name(node);
	const char *sibling_name;
	struct dm_tree_node *sibling;

	*resolvable = 0;

	if (!name)
		return_0;

	while ((sibling = dm_tree_next_child(&handle, parent, 0))) {
		if (sibling == node)
			continue;

		if (!(sibling_name = dm_tree_node_get_name(sibling))) {
			stack;
			continue;
		}

		if (!strcmp(node->props.new_name, sibling_name)) {
			if (sibling->props.new_name)
				*resolvable = 1;
			return 1;
		}
	}

	return 0;
}

int dm_tree_activate_children(struct dm_tree_node *dnode,
				 const char *uuid_prefix,
				 size_t uuid_prefix_len)
{
	int r = 1;
	int resolvable_name_conflict, awaiting_peer_rename = 0;
	void *handle = NULL;
	struct dm_tree_node *child = dnode;
	const char *name;
	const char *uuid;
	int priority;

	/* Activate children first */
	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		if (!(uuid = dm_tree_node_get_uuid(child))) {
			stack;
			continue;
		}

		if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			continue;

		if (dm_tree_node_num_children(child, 0))
			if (!dm_tree_activate_children(child, uuid_prefix, uuid_prefix_len))
				return_0;
	}

	handle = NULL;

	for (priority = 0; priority < 3; priority++) {
		awaiting_peer_rename = 0;
		while ((child = dm_tree_next_child(&handle, dnode, 0))) {
			if (priority != child->activation_priority)
				continue;

			if (!(uuid = dm_tree_node_get_uuid(child))) {
				stack;
				continue;
			}

			if (!_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
				continue;

			if (!(name = dm_tree_node_get_name(child))) {
				stack;
				continue;
			}

			/* Rename? */
			if (child->props.new_name) {
				if (_rename_conflict_exists(dnode, child, &resolvable_name_conflict) &&
				    resolvable_name_conflict) {
					awaiting_peer_rename++;
					continue;
				}
				if (!_rename_node(name, child->props.new_name, child->info.major,
						  child->info.minor, &child->dtree->cookie,
						  child->udev_flags)) {
					log_error("Failed to rename %s (%" PRIu32
						  ":%" PRIu32 ") to %s", name, child->info.major,
						  child->info.minor, child->props.new_name);
					return 0;
				}
				child->name = child->props.new_name;
				child->props.new_name = NULL;
			}

			if (!child->info.inactive_table && !child->info.suspended)
				continue;

			if (!_resume_node(child->name, child->info.major, child->info.minor,
					  child->props.read_ahead, child->props.read_ahead_flags,
					  &child->info, &child->dtree->cookie, child->udev_flags, child->info.suspended)) {
				log_error("Unable to resume %s.", _node_name(child));
				r = 0;
				continue;
			}

			/*
			 * FIXME: Implement delayed error reporting
			 * activation should be stopped only in the case,
			 * the submission of transation_id message fails,
			 * resume should continue further, just whole command
			 * has to report failure.
			 */
			if (r && (child->props.send_messages > 1) &&
			    !(r = _node_send_messages(child, uuid_prefix, uuid_prefix_len, 1)))
				stack;
		}
		if (awaiting_peer_rename)
			priority--; /* redo priority level */
	}

	return r;
}

static int _create_node(struct dm_tree_node *dnode)
{
	int r = 0;
	struct dm_task *dmt;

	log_verbose("Creating %s", dnode->name);

	if (!(dmt = dm_task_create(DM_DEVICE_CREATE))) {
		log_error("Create dm_task creation failed for %s", dnode->name);
		return 0;
	}

	if (!dm_task_set_name(dmt, dnode->name)) {
		log_error("Failed to set device name for %s", dnode->name);
		goto out;
	}

	if (!dm_task_set_uuid(dmt, dnode->uuid)) {
		log_error("Failed to set uuid for %s", dnode->name);
		goto out;
	}

	if (dnode->props.major &&
	    (!dm_task_set_major(dmt, dnode->props.major) ||
	     !dm_task_set_minor(dmt, dnode->props.minor))) {
		log_error("Failed to set device number for %s creation.", dnode->name);
		goto out;
	}

	if (dnode->props.read_only && !dm_task_set_ro(dmt)) {
		log_error("Failed to set read only flag for %s", dnode->name);
		goto out;
	}

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if ((r = dm_task_run(dmt))) {
		if (!(r = dm_task_get_info(dmt, &dnode->info)))
			/*
			 * This should not be possible to occur.  However,
			 * we print an error message anyway for the more
			 * absurd cases (e.g. memory corruption) so there
			 * is never any question as to which one failed.
			 */
			log_error(INTERNAL_ERROR
				  "Unable to get DM task info for %s.",
				  dnode->name);
	}
out:
	dm_task_destroy(dmt);

	return r;
}

/*
 * _remove_node
 *
 * This function is only used to remove a DM device that has failed
 * to load any table.
 */
static int _remove_node(struct dm_tree_node *dnode)
{
	if (!dnode->info.exists)
		return 1;

	if (dnode->info.live_table || dnode->info.inactive_table) {
		log_error(INTERNAL_ERROR
			  "_remove_node called on device with loaded table(s).");
		return 0;
	}

	if (!_deactivate_node(dnode->name, dnode->info.major, dnode->info.minor,
			      &dnode->dtree->cookie, dnode->udev_flags, 0)) {
		log_error("Failed to clean-up device with no table: %s.",
			  _node_name(dnode));
		return 0;
	}
	return 1;
}

static int _build_dev_string(char *devbuf, size_t bufsize, struct dm_tree_node *node)
{
	if (!dm_format_dev(devbuf, bufsize, node->info.major, node->info.minor)) {
		log_error("Failed to format %s device number for %s as dm "
			  "target (%u,%u)",
			  node->name, node->uuid, node->info.major, node->info.minor);
		return 0;
	}

	return 1;
}

/* simplify string emiting code */
#define EMIT_PARAMS(p, str...)\
do {\
	int w;\
	if ((w = dm_snprintf(params + p, paramsize - (size_t) p, str)) < 0) {\
		stack; /* Out of space */\
		return -1;\
	}\
	p += w;\
} while (0)

/*
 * _emit_areas_line
 *
 * Returns: 1 on success, 0 on failure
 */
static int _emit_areas_line(struct dm_task *dmt __attribute__((unused)),
			    struct load_segment *seg, char *params,
			    size_t paramsize, int *pos)
{
	struct seg_area *area;
	char devbuf[DM_FORMAT_DEV_BUFSIZE];
	unsigned first_time = 1;

	dm_list_iterate_items(area, &seg->areas) {
		switch (seg->type) {
		case SEG_RAID0:
		case SEG_RAID0_META:
		case SEG_RAID1:
		case SEG_RAID10:
		case SEG_RAID4:
		case SEG_RAID5_N:
		case SEG_RAID5_LA:
		case SEG_RAID5_RA:
		case SEG_RAID5_LS:
		case SEG_RAID5_RS:
		case SEG_RAID6_N_6:
		case SEG_RAID6_ZR:
		case SEG_RAID6_NR:
		case SEG_RAID6_NC:
		case SEG_RAID6_LS_6:
		case SEG_RAID6_RS_6:
		case SEG_RAID6_LA_6:
		case SEG_RAID6_RA_6:
			if (!area->dev_node) {
				EMIT_PARAMS(*pos, " -");
				break;
			}
			if (!_build_dev_string(devbuf, sizeof(devbuf), area->dev_node))
				return_0;

			EMIT_PARAMS(*pos, " %s", devbuf);
			break;
		default:
			if (!_build_dev_string(devbuf, sizeof(devbuf), area->dev_node))
				return_0;

			EMIT_PARAMS(*pos, "%s%s %" PRIu64, first_time ? "" : " ",
				    devbuf, area->offset);
		}

		first_time = 0;
	}

	return 1;
}

/*
 * Returns: 1 on success, 0 on failure
 */
static int _mirror_emit_segment_line(struct dm_task *dmt, struct load_segment *seg,
				     char *params, size_t paramsize)
{
	int block_on_error = 0;
	int handle_errors = 0;
	int dm_log_userspace = 0;
	unsigned log_parm_count;
	int pos = 0;
	char logbuf[DM_FORMAT_DEV_BUFSIZE];
	const char *logtype;
	unsigned kmaj = 0, kmin = 0, krel = 0;

	if (!get_uname_version(&kmaj, &kmin, &krel))
		return_0;

	if ((seg->flags & DM_BLOCK_ON_ERROR)) {
		/*
		 * Originally, block_on_error was an argument to the log
		 * portion of the mirror CTR table.  It was renamed to
		 * "handle_errors" and now resides in the 'features'
		 * section of the mirror CTR table (i.e. at the end).
		 *
		 * We can identify whether to use "block_on_error" or
		 * "handle_errors" by the dm-mirror module's version
		 * number (>= 1.12) or by the kernel version (>= 2.6.22).
		 */
		if (KERNEL_VERSION(kmaj, kmin, krel) >= KERNEL_VERSION(2, 6, 22))
			handle_errors = 1;
		else
			block_on_error = 1;
	}

	if (seg->clustered) {
		/* Cluster mirrors require a UUID */
		if (!seg->uuid)
			return_0;

		/*
		 * Cluster mirrors used to have their own log
		 * types.  Now they are accessed through the
		 * userspace log type.
		 *
		 * The dm-log-userspace module was added to the
		 * 2.6.31 kernel.
		 */
		if (KERNEL_VERSION(kmaj, kmin, krel) >= KERNEL_VERSION(2, 6, 31))
			dm_log_userspace = 1;
	}

	/* Region size */
	log_parm_count = 1;

	/* [no]sync, block_on_error etc. */
	log_parm_count += hweight32(seg->flags);

	/* "handle_errors" is a feature arg now */
	if (handle_errors)
		log_parm_count--;

	/* DM_CORELOG does not count in the param list */
	if (seg->flags & DM_CORELOG)
		log_parm_count--;

	if (seg->clustered) {
		log_parm_count++; /* For UUID */

		if (!dm_log_userspace)
			EMIT_PARAMS(pos, "clustered-");
		else
			/* For clustered-* type field inserted later */
			log_parm_count++;
	}

	if (!seg->log)
		logtype = "core";
	else {
		logtype = "disk";
		log_parm_count++;
		if (!_build_dev_string(logbuf, sizeof(logbuf), seg->log))
			return_0;
	}

	if (dm_log_userspace)
		EMIT_PARAMS(pos, "userspace %u %s clustered-%s",
			    log_parm_count, seg->uuid, logtype);
	else
		EMIT_PARAMS(pos, "%s %u", logtype, log_parm_count);

	if (seg->log)
		EMIT_PARAMS(pos, " %s", logbuf);

	EMIT_PARAMS(pos, " %u", seg->region_size);

	if (seg->clustered && !dm_log_userspace)
		EMIT_PARAMS(pos, " %s", seg->uuid);

	if ((seg->flags & DM_NOSYNC))
		EMIT_PARAMS(pos, " nosync");
	else if ((seg->flags & DM_FORCESYNC))
		EMIT_PARAMS(pos, " sync");

	if (block_on_error)
		EMIT_PARAMS(pos, " block_on_error");

	EMIT_PARAMS(pos, " %u ", seg->mirror_area_count);

	if (_emit_areas_line(dmt, seg, params, paramsize, &pos) <= 0)
		return_0;

	if (handle_errors)
		EMIT_PARAMS(pos, " 1 handle_errors");

	return 1;
}

static int _2_if_value(unsigned p)
{
	return p ? 2 : 0;
}

/* Return number of bits passed in @bits assuming 2 * 64 bit size */
static int _get_params_count(const uint64_t *bits)
{
	int r = 0;
	int i = RAID_BITMAP_SIZE;

	while (i--) {
		r += 2 * hweight32(bits[i] & 0xFFFFFFFF);
		r += 2 * hweight32(bits[i] >> 32);
	}

	return r;
}

/*
 * Get target version (major, minor and patchlevel) for @target_name
 *
 * FIXME: this function is derived from liblvm.
 *        Integrate with move of liblvm functions
 *        to libdm in future library layer purge
 *        (e.g. expose as API dm_target_version()?)
 */
static int _target_version(const char *target_name, uint32_t *maj,
			   uint32_t *min, uint32_t *patchlevel)
{
	int r = 0;
	struct dm_task *dmt;
	struct dm_versions *target, *last_target = NULL;

	log_very_verbose("Getting target version for %s", target_name);
	if (!(dmt = dm_task_create(DM_DEVICE_LIST_VERSIONS)))
		return_0;

	if (!dm_task_run(dmt)) {
		log_debug_activation("Failed to get %s target versions", target_name);
		/* Assume this was because LIST_VERSIONS isn't supported */
		*maj = *min = *patchlevel = 0;
		r = 1;
	} else
		for (target = dm_task_get_versions(dmt);
		     target != last_target;
		     last_target = target, target = (struct dm_versions *)((char *) target + target->next))
			if (!strcmp(target_name, target->name)) {
				*maj = target->version[0];
				*min = target->version[1];
				*patchlevel = target->version[2];
				log_very_verbose("Found %s target "
						 "v%" PRIu32 ".%" PRIu32 ".%" PRIu32 ".",
						 target_name, *maj, *min, *patchlevel);
				r = 1;
				break;
			}

	dm_task_destroy(dmt);

	return r;
}

static int _raid_emit_segment_line(struct dm_task *dmt, uint32_t major,
				   uint32_t minor, struct load_segment *seg,
				   uint64_t *seg_start, char *params,
				   size_t paramsize)
{
	uint32_t i;
	uint32_t area_count = seg->area_count / 2;
	uint32_t maj, min, patchlevel;
	int param_count = 1; /* mandatory 'chunk size'/'stripe size' arg */
	int pos = 0;
	unsigned type;

	if (seg->area_count % 2)
		return 0;

	if ((seg->flags & DM_NOSYNC) || (seg->flags & DM_FORCESYNC))
		param_count++;

	param_count += _2_if_value(seg->data_offset) +
		       _2_if_value(seg->delta_disks) +
		       _2_if_value(seg->region_size) +
		       _2_if_value(seg->writebehind) +
		       _2_if_value(seg->min_recovery_rate) +
		       _2_if_value(seg->max_recovery_rate) +
		       _2_if_value(seg->data_copies > 1);

	/* rebuilds and writemostly are BITMAP_SIZE * 64 bits */
	param_count += _get_params_count(seg->rebuilds);
	param_count += _get_params_count(seg->writemostly);

	if ((seg->type == SEG_RAID1) && seg->stripe_size)
		log_info("WARNING: Ignoring RAID1 stripe size");

	/* Kernel only expects "raid0", not "raid0_meta" */
	type = seg->type;
	if (type == SEG_RAID0_META)
		type = SEG_RAID0;

	EMIT_PARAMS(pos, "%s %d %u",
		    type == SEG_RAID10 ? "raid10" : _dm_segtypes[type].target,
		    param_count, seg->stripe_size);

	if (!_target_version("raid", &maj, &min, &patchlevel))
		return_0;

	/*
	 * Target version prior to 1.9.0 and >= 1.11.0 emit
	 * order of parameters as of kernel target documentation
	 */
	if (maj > 1 || (maj == 1 && (min < 9 || min >= 11))) {
		if (seg->flags & DM_NOSYNC)
			EMIT_PARAMS(pos, " nosync");
		else if (seg->flags & DM_FORCESYNC)
			EMIT_PARAMS(pos, " sync");

		for (i = 0; i < area_count; i++)
			if (seg->rebuilds[i/64] & (1ULL << (i%64)))
				EMIT_PARAMS(pos, " rebuild %u", i);

		if (seg->min_recovery_rate)
			EMIT_PARAMS(pos, " min_recovery_rate %u",
				    seg->min_recovery_rate);

		if (seg->max_recovery_rate)
			EMIT_PARAMS(pos, " max_recovery_rate %u",
				    seg->max_recovery_rate);

		for (i = 0; i < area_count; i++)
			if (seg->writemostly[i/64] & (1ULL << (i%64)))
				EMIT_PARAMS(pos, " write_mostly %u", i);

		if (seg->writebehind)
			EMIT_PARAMS(pos, " max_write_behind %u", seg->writebehind);

		if (seg->region_size)
			EMIT_PARAMS(pos, " region_size %u", seg->region_size);

		if (seg->data_copies > 1 && type == SEG_RAID10)
			EMIT_PARAMS(pos, " raid10_copies %u", seg->data_copies);

		if (seg->delta_disks)
			EMIT_PARAMS(pos, " delta_disks %d", seg->delta_disks);

		/* If seg-data_offset == 1, kernel needs a zero offset to adjust to it */
		if (seg->data_offset)
			EMIT_PARAMS(pos, " data_offset %d", seg->data_offset == 1 ? 0 : seg->data_offset);

	/* Target version >= 1.9.0 && < 1.11.0 had a table line parameter ordering flaw */
	} else {
		if (seg->data_copies > 1 && type == SEG_RAID10)
			EMIT_PARAMS(pos, " raid10_copies %u", seg->data_copies);

		if (seg->flags & DM_NOSYNC)
			EMIT_PARAMS(pos, " nosync");
		else if (seg->flags & DM_FORCESYNC)
			EMIT_PARAMS(pos, " sync");

		if (seg->region_size)
			EMIT_PARAMS(pos, " region_size %u", seg->region_size);

		/* If seg-data_offset == 1, kernel needs a zero offset to adjust to it */
		if (seg->data_offset)
			EMIT_PARAMS(pos, " data_offset %d", seg->data_offset == 1 ? 0 : seg->data_offset);

		if (seg->delta_disks)
			EMIT_PARAMS(pos, " delta_disks %d", seg->delta_disks);

		for (i = 0; i < area_count; i++)
			if (seg->rebuilds[i/64] & (1ULL << (i%64)))
				EMIT_PARAMS(pos, " rebuild %u", i);

		for (i = 0; i < area_count; i++)
			if (seg->writemostly[i/64] & (1ULL << (i%64)))
				EMIT_PARAMS(pos, " write_mostly %u", i);

		if (seg->writebehind)
			EMIT_PARAMS(pos, " max_write_behind %u", seg->writebehind);

		if (seg->max_recovery_rate)
			EMIT_PARAMS(pos, " max_recovery_rate %u",
				    seg->max_recovery_rate);

		if (seg->min_recovery_rate)
			EMIT_PARAMS(pos, " min_recovery_rate %u",
				    seg->min_recovery_rate);
	}

	/* Print number of metadata/data device pairs */
	EMIT_PARAMS(pos, " %u", area_count);

	if (_emit_areas_line(dmt, seg, params, paramsize, &pos) <= 0)
		return_0;

	return 1;
}

static int _cache_emit_segment_line(struct dm_task *dmt,
				    struct load_segment *seg,
				    char *params, size_t paramsize)
{
	int pos = 0;
	/* unsigned feature_count; */
	char data[DM_FORMAT_DEV_BUFSIZE];
	char metadata[DM_FORMAT_DEV_BUFSIZE];
	char origin[DM_FORMAT_DEV_BUFSIZE];
	const char *name;
	struct dm_config_node *cn;

	/* Cache Dev */
	if (!_build_dev_string(data, sizeof(data), seg->pool))
		return_0;

	/* Metadata Dev */
	if (!_build_dev_string(metadata, sizeof(metadata), seg->metadata))
		return_0;

	/* Origin Dev */
	if (!_build_dev_string(origin, sizeof(origin), seg->origin))
		return_0;

	EMIT_PARAMS(pos, "%s %s %s", metadata, data, origin);

	/* Data block size */
	EMIT_PARAMS(pos, " %u", seg->data_block_size);

	/* Features */
	/* feature_count = hweight32(seg->flags); */
	/* EMIT_PARAMS(pos, " %u", feature_count); */
	if (seg->flags & DM_CACHE_FEATURE_METADATA2)
		EMIT_PARAMS(pos, " 2 metadata2 ");
	else
		EMIT_PARAMS(pos, " 1 ");

	if (seg->flags & DM_CACHE_FEATURE_PASSTHROUGH)
		EMIT_PARAMS(pos, "passthrough");
        else if (seg->flags & DM_CACHE_FEATURE_WRITEBACK)
		EMIT_PARAMS(pos, "writeback");
	else
		EMIT_PARAMS(pos, "writethrough");

	/* Cache Policy */
	name = seg->policy_name ? : "default";

	EMIT_PARAMS(pos, " %s", name);

	EMIT_PARAMS(pos, " %u", seg->policy_argc * 2);
	if (seg->policy_settings)
		for (cn = seg->policy_settings->child; cn; cn = cn->sib)
			EMIT_PARAMS(pos, " %s %" PRIu64, cn->key, cn->v->v.i);

	return 1;
}

static int _thin_pool_emit_segment_line(struct dm_task *dmt,
					struct load_segment *seg,
					char *params, size_t paramsize)
{
	int pos = 0;
	char pool[DM_FORMAT_DEV_BUFSIZE], metadata[DM_FORMAT_DEV_BUFSIZE];
	int features = (seg->error_if_no_space ? 1 : 0) +
		 (seg->read_only ? 1 : 0) +
		 (seg->ignore_discard ? 1 : 0) +
		 (seg->no_discard_passdown ? 1 : 0) +
		 (seg->skip_block_zeroing ? 1 : 0);

	if (!_build_dev_string(metadata, sizeof(metadata), seg->metadata))
		return_0;

	if (!_build_dev_string(pool, sizeof(pool), seg->pool))
		return_0;

	EMIT_PARAMS(pos, "%s %s %d %" PRIu64 " %d%s%s%s%s%s", metadata, pool,
		    seg->data_block_size, seg->low_water_mark, features,
		    seg->skip_block_zeroing ? " skip_block_zeroing" : "",
		    seg->ignore_discard ? " ignore_discard" : "",
		    seg->no_discard_passdown ? " no_discard_passdown" : "",
		    seg->error_if_no_space ? " error_if_no_space" : "",
		    seg->read_only ? " read_only" : ""
		   );

	return 1;
}

static int _vdo_emit_segment_line(struct dm_task *dmt,
				  struct load_segment *seg,
				  char *params, size_t paramsize)
{
	int pos = 0;
	char data[DM_FORMAT_DEV_BUFSIZE];
	char data_dev[128]; // for /dev/dm-XXXX

	if (!_build_dev_string(data, sizeof(data), seg->vdo_data))
		return_0;
	/* Unlike normal targets, current VDO requires device path */
	if (dm_snprintf(data_dev, sizeof(data_dev), "/dev/dm-%u", seg->vdo_data->info.minor) < 0) {
		log_error("Can create VDO data volume path for %s.", data);
		return 0;
	}

	EMIT_PARAMS(pos, "%s %u %s " FMTu64 " " FMTu64 " %u on %s %s "
		    "ack=%u,bio=%u,bioRotationInterval=%u,cpu=%u,hash=%u,logical=%u,physical=%u",
		    data_dev,
		    (seg->vdo_params.emulate_512_sectors == 0) ? 4096 : 512,
		    seg->vdo_params.use_read_cache ? "enabled" : "disabled",
		    seg->vdo_params.read_cache_size_mb * UINT64_C(256),		// 1MiB -> 4KiB units
		    seg->vdo_params.block_map_cache_size_mb * UINT64_C(256),	// 1MiB -> 4KiB units
		    seg->vdo_params.block_map_period,
		    (seg->vdo_params.write_policy == DM_VDO_WRITE_POLICY_SYNC) ? "sync" :
			(seg->vdo_params.write_policy == DM_VDO_WRITE_POLICY_ASYNC) ? "async" : "auto", // policy
		    seg->vdo_name,
		    seg->vdo_params.ack_threads,
		    seg->vdo_params.bio_threads,
		    seg->vdo_params.bio_rotation,
		    seg->vdo_params.cpu_threads,
		    seg->vdo_params.hash_zone_threads,
		    seg->vdo_params.logical_threads,
		    seg->vdo_params.physical_threads);

	return 1;
}

static int _thin_emit_segment_line(struct dm_task *dmt,
				   struct load_segment *seg,
				   char *params, size_t paramsize)
{
	int pos = 0;
	char pool[DM_FORMAT_DEV_BUFSIZE];
	char external[DM_FORMAT_DEV_BUFSIZE + 1];

	if (!_build_dev_string(pool, sizeof(pool), seg->pool))
		return_0;

	if (!seg->external)
		*external = 0;
	else {
		*external = ' ';
		if (!_build_dev_string(external + 1, sizeof(external) - 1,
				       seg->external))
			return_0;
	}

	EMIT_PARAMS(pos, "%s %d%s", pool, seg->device_id, external);

	return 1;
}

static int _emit_segment_line(struct dm_task *dmt, uint32_t major,
			      uint32_t minor, struct load_segment *seg,
			      uint64_t *seg_start, char *params,
			      size_t paramsize)
{
	int pos = 0;
	int r;
	int target_type_is_raid = 0;
	char originbuf[DM_FORMAT_DEV_BUFSIZE], cowbuf[DM_FORMAT_DEV_BUFSIZE];

	switch(seg->type) {
	case SEG_ERROR:
	case SEG_ZERO:
	case SEG_LINEAR:
		break;
	case SEG_MIRRORED:
		/* Mirrors are pretty complicated - now in separate function */
		r = _mirror_emit_segment_line(dmt, seg, params, paramsize);
		if (!r)
			return_0;
		break;
	case SEG_SNAPSHOT:
	case SEG_SNAPSHOT_MERGE:
		if (!_build_dev_string(originbuf, sizeof(originbuf), seg->origin))
			return_0;
		if (!_build_dev_string(cowbuf, sizeof(cowbuf), seg->cow))
			return_0;
		EMIT_PARAMS(pos, "%s %s %c %d", originbuf, cowbuf,
			    seg->persistent ? 'P' : 'N', seg->chunk_size);
		break;
	case SEG_SNAPSHOT_ORIGIN:
		if (!_build_dev_string(originbuf, sizeof(originbuf), seg->origin))
			return_0;
		EMIT_PARAMS(pos, "%s", originbuf);
		break;
	case SEG_STRIPED:
		EMIT_PARAMS(pos, "%u %u ", seg->area_count, seg->stripe_size);
		break;
	case SEG_VDO:
		if (!(r = _vdo_emit_segment_line(dmt, seg, params, paramsize)))
		      return_0;
		break;
	case SEG_CRYPT:
		EMIT_PARAMS(pos, "%s%s%s%s%s %s %" PRIu64 " ", seg->cipher,
			    seg->chainmode ? "-" : "", seg->chainmode ?: "",
			    seg->iv ? "-" : "", seg->iv ?: "", seg->key,
			    seg->iv_offset != DM_CRYPT_IV_DEFAULT ?
			    seg->iv_offset : *seg_start);
		break;
	case SEG_RAID0:
	case SEG_RAID0_META:
	case SEG_RAID1:
	case SEG_RAID10:
	case SEG_RAID4:
	case SEG_RAID5_N:
	case SEG_RAID5_LA:
	case SEG_RAID5_RA:
	case SEG_RAID5_LS:
	case SEG_RAID5_RS:
	case SEG_RAID6_N_6:
	case SEG_RAID6_ZR:
	case SEG_RAID6_NR:
	case SEG_RAID6_NC:
	case SEG_RAID6_LS_6:
	case SEG_RAID6_RS_6:
	case SEG_RAID6_LA_6:
	case SEG_RAID6_RA_6:
		target_type_is_raid = 1;
		r = _raid_emit_segment_line(dmt, major, minor, seg, seg_start,
					    params, paramsize);
		if (!r)
			return_0;

		break;
	case SEG_THIN_POOL:
		if (!_thin_pool_emit_segment_line(dmt, seg, params, paramsize))
			return_0;
		break;
	case SEG_THIN:
		if (!_thin_emit_segment_line(dmt, seg, params, paramsize))
			return_0;
		break;
	case SEG_CACHE:
		if (!_cache_emit_segment_line(dmt, seg, params, paramsize))
			return_0;
		break;
	}

	switch(seg->type) {
	case SEG_ERROR:
	case SEG_SNAPSHOT:
	case SEG_SNAPSHOT_ORIGIN:
	case SEG_SNAPSHOT_MERGE:
	case SEG_ZERO:
	case SEG_THIN_POOL:
	case SEG_THIN:
	case SEG_CACHE:
		break;
	case SEG_CRYPT:
	case SEG_LINEAR:
	case SEG_STRIPED:
		if ((r = _emit_areas_line(dmt, seg, params, paramsize, &pos)) <= 0) {
			stack;
			return r;
		}
		if (!params[0]) {
			log_error("No parameters supplied for %s target "
				  "%u:%u.", _dm_segtypes[seg->type].target,
				  major, minor);
			return 0;
		}
		break;
	}

	log_debug_activation("Adding target to (%" PRIu32 ":%" PRIu32 "): %" PRIu64
			     " %" PRIu64 " %s %s", major, minor,
			     *seg_start, seg->size, target_type_is_raid ? "raid" :
			     _dm_segtypes[seg->type].target, params);

	if (!dm_task_add_target(dmt, *seg_start, seg->size,
				target_type_is_raid ? "raid" :
				_dm_segtypes[seg->type].target, params))
		return_0;

	*seg_start += seg->size;

	return 1;
}

#undef EMIT_PARAMS

static int _emit_segment(struct dm_task *dmt, uint32_t major, uint32_t minor,
			 struct load_segment *seg, uint64_t *seg_start)
{
	char *params;
	size_t paramsize = 4096; /* FIXME: too small for long RAID lines when > 64 devices supported */
	int ret;

	do {
		if (!(params = malloc(paramsize))) {
			log_error("Insufficient space for target parameters.");
			return 0;
		}

		params[0] = '\0';
		ret = _emit_segment_line(dmt, major, minor, seg, seg_start,
					 params, paramsize);
		free(params);

		if (!ret)
			stack;

		if (ret >= 0)
			return ret;

		log_debug_activation("Insufficient space in params[%" PRIsize_t
				     "] for target parameters.", paramsize);

		paramsize *= 2;
	} while (paramsize < MAX_TARGET_PARAMSIZE);

	log_error("Target parameter size too big. Aborting.");
	return 0;
}

static int _load_node(struct dm_tree_node *dnode)
{
	int r = 0;
	struct dm_task *dmt;
	struct load_segment *seg;
	uint64_t seg_start = 0, existing_table_size;

	log_verbose("Loading table for %s.", _node_name(dnode));

	if (!(dmt = dm_task_create(DM_DEVICE_RELOAD))) {
		log_error("Reload dm_task creation failed for %s.", _node_name(dnode));
		return 0;
	}

	if (!dm_task_set_major(dmt, dnode->info.major) ||
	    !dm_task_set_minor(dmt, dnode->info.minor)) {
		log_error("Failed to set device number for %s reload.", _node_name(dnode));
		goto out;
	}

	if (dnode->props.read_only && !dm_task_set_ro(dmt)) {
		log_error("Failed to set read only flag for %s.", _node_name(dnode));
		goto out;
	}

	if (!dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	dm_list_iterate_items(seg, &dnode->props.segs)
		if (!_emit_segment(dmt, dnode->info.major, dnode->info.minor,
				   seg, &seg_start))
			goto_out;

	if (!dm_task_suppress_identical_reload(dmt))
		log_warn("WARNING: Failed to suppress reload of identical tables.");

	if ((r = dm_task_run(dmt))) {
		r = dm_task_get_info(dmt, &dnode->info);
		if (r && !dnode->info.inactive_table)
			log_verbose("Suppressed %s identical table reload.",
				    _node_name(dnode));

		existing_table_size = dm_task_get_existing_table_size(dmt);
		if ((dnode->props.size_changed =
		     (existing_table_size == seg_start) ? 0 :
		     (existing_table_size > seg_start) ? -1 : 1)) {
			/*
			 * Kernel usually skips size validation on zero-length devices
			 * now so no need to preload them.
			 */
			/* FIXME In which kernel version did this begin? */
			if (!existing_table_size && dnode->props.delay_resume_if_new)
				dnode->props.size_changed = 0;

			log_debug_activation("Table size changed from %" PRIu64 " to %"
					     PRIu64 " for %s.%s", existing_table_size,
					     seg_start, _node_name(dnode),
					     dnode->props.size_changed ? "" : " (Ignoring.)");

			/*
			 * FIXME: code here has known design problem.
			 *  LVM2 does NOT resize thin-pool on top of other LV in 2 steps -
			 *  where raid would be resized with 1st. transaction
			 *  followed by 2nd. thin-pool resize - RHBZ #1285063
			 */
			if (existing_table_size && dnode->props.delay_resume_if_extended) {
				log_debug_activation("Resume of table of extended device %s delayed.",
						     _node_name(dnode));
				dnode->props.size_changed = 0;
			}
		}
	}

	dnode->props.segment_count = 0;

out:
	dm_task_destroy(dmt);

	return r;
}

/*
 * Currently try to deactivate only nodes created during preload.
 * New node is always attached to the front of activated_list
 */
static int _dm_tree_revert_activated(struct dm_tree_node *parent)
{
	struct dm_tree_node *child;

	dm_list_iterate_items_gen(child, &parent->activated, activated_list) {
		log_debug_activation("Reverting %s.", _node_name(child));
		if (child->callback) {
			log_debug_activation("Dropping callback for %s.", _node_name(child));
			child->callback = NULL;
		}
		if (!_deactivate_node(child->name, child->info.major, child->info.minor,
				      &child->dtree->cookie, child->udev_flags, 0)) {
			log_error("Unable to deactivate %s.", _node_name(child));
			return 0;
		}
		if (!_dm_tree_revert_activated(child))
			return_0;
	}

	return 1;
}

int dm_tree_preload_children(struct dm_tree_node *dnode,
			     const char *uuid_prefix,
			     size_t uuid_prefix_len)
{
	int r = 1, node_created = 0;
	void *handle = NULL;
	struct dm_tree_node *child;
	int update_devs_flag = 0;

	/* Preload children first */
	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		/* Propagate delay of resume from parent node */
		if (dnode->props.delay_resume_if_new > 1)
			child->props.delay_resume_if_new = dnode->props.delay_resume_if_new;

		/* Skip existing non-device-mapper devices */
		if (!child->info.exists && child->info.major)
			continue;

		/* Ignore if it doesn't belong to this VG */
		if (child->info.exists &&
		    !_uuid_prefix_matches(child->uuid, uuid_prefix, uuid_prefix_len))
			continue;

		if (dm_tree_node_num_children(child, 0))
			if (!dm_tree_preload_children(child, uuid_prefix, uuid_prefix_len))
				return_0;

		/* FIXME Cope if name exists with no uuid? */
		if (!child->info.exists && !(node_created = _create_node(child)))
			return_0;

		/* Propagate delayed resume from exteded child node */
		if (child->props.delay_resume_if_extended)
			dnode->props.delay_resume_if_extended = 1;

		if (!child->info.inactive_table &&
		    child->props.segment_count &&
		    !_load_node(child)) {
			/*
			 * If the table load does not succeed, we remove the
			 * device in the kernel that would otherwise have an
			 * empty table.  This makes the create + load of the
			 * device atomic.  However, if other dependencies have
			 * already been created and loaded; this code is
			 * insufficient to remove those - only the node
			 * encountering the table load failure is removed.
			 */
			if (node_created) {
				if (!_remove_node(child))
					return_0;
				if (!dm_udev_wait(dm_tree_get_cookie(dnode)))
					stack;
				dm_tree_set_cookie(dnode, 0);
				(void) _dm_tree_revert_activated(child);
			}
			return_0;
		}

		/* No resume for a device without parents or with unchanged or smaller size */
		if (!dm_tree_node_num_children(child, 1) || (child->props.size_changed <= 0))
			continue;

		if (!child->info.inactive_table && !child->info.suspended)
			continue;

		if (!_resume_node(child->name, child->info.major, child->info.minor,
				  child->props.read_ahead, child->props.read_ahead_flags,
				  &child->info, &child->dtree->cookie, child->udev_flags,
				  child->info.suspended)) {
			log_error("Unable to resume %s.", _node_name(child));
			/* If the device was not previously active, we might as well remove this node. */
			if (!child->info.live_table &&
			    !_deactivate_node(child->name, child->info.major, child->info.minor,
					      &child->dtree->cookie, child->udev_flags, 0))
				log_error("Unable to deactivate %s.", _node_name(child));
			r = 0;
			/* Each child is handled independently */
			continue;
		}

		if (node_created) {
			/* Collect newly introduced devices for revert */
			dm_list_add_h(&dnode->activated, &child->activated_list);

			/* When creating new node also check transaction_id. */
			if (child->props.send_messages &&
			    !_node_send_messages(child, uuid_prefix, uuid_prefix_len, 0)) {
				stack;
				if (!dm_udev_wait(dm_tree_get_cookie(dnode)))
					stack;
				dm_tree_set_cookie(dnode, 0);
				(void) _dm_tree_revert_activated(dnode);
				r = 0;
				continue;
			}
		}

		/*
		 * Prepare for immediate synchronization with udev and flush all stacked
		 * dev node operations if requested by immediate_dev_node property. But
		 * finish processing current level in the tree first.
		 */
		if (child->props.immediate_dev_node)
			update_devs_flag = 1;
	}

	if (update_devs_flag ||
	    (r && !dnode->info.exists && dnode->callback)) {
		if (!dm_udev_wait(dm_tree_get_cookie(dnode)))
			stack;
		dm_tree_set_cookie(dnode, 0);

		if (r && !dnode->info.exists && dnode->callback &&
		    !dnode->callback(dnode, DM_NODE_CALLBACK_PRELOADED,
				     dnode->callback_data))
		{
			/* Try to deactivate what has been activated in preload phase */
			(void) _dm_tree_revert_activated(dnode);
			return_0;
		}
	}

	return r;
}

/*
 * Returns 1 if unsure.
 */
int dm_tree_children_use_uuid(struct dm_tree_node *dnode,
				 const char *uuid_prefix,
				 size_t uuid_prefix_len)
{
	void *handle = NULL;
	struct dm_tree_node *child = dnode;
	const char *uuid;

	while ((child = dm_tree_next_child(&handle, dnode, 0))) {
		if (!(uuid = dm_tree_node_get_uuid(child))) {
			log_warn("WARNING: Failed to get uuid for dtree node %s.",
				 _node_name(child));
			return 1;
		}

		if (_uuid_prefix_matches(uuid, uuid_prefix, uuid_prefix_len))
			return 1;

		if (dm_tree_node_num_children(child, 0))
			dm_tree_children_use_uuid(child, uuid_prefix, uuid_prefix_len);
	}

	return 0;
}

/*
 * Target functions
 */
static struct load_segment *_add_segment(struct dm_tree_node *dnode, unsigned type, uint64_t size)
{
	struct load_segment *seg;

	if (!(seg = dm_pool_zalloc(dnode->dtree->mem, sizeof(*seg)))) {
		log_error("dtree node segment allocation failed");
		return NULL;
	}

	seg->type = type;
	seg->size = size;
	dm_list_init(&seg->areas);
	dm_list_add(&dnode->props.segs, &seg->list);
	dnode->props.segment_count++;

	return seg;
}

int dm_tree_node_add_snapshot_origin_target(struct dm_tree_node *dnode,
					    uint64_t size,
					    const char *origin_uuid)
{
	struct load_segment *seg;
	struct dm_tree_node *origin_node;

	if (!(seg = _add_segment(dnode, SEG_SNAPSHOT_ORIGIN, size)))
		return_0;

	if (!(origin_node = dm_tree_find_node_by_uuid(dnode->dtree, origin_uuid))) {
		log_error("Couldn't find snapshot origin uuid %s.", origin_uuid);
		return 0;
	}

	seg->origin = origin_node;
	if (!_link_tree_nodes(dnode, origin_node))
		return_0;

	/* Resume snapshot origins after new snapshots */
	dnode->activation_priority = 1;

	/*
	 * Don't resume the origin immediately in case it is a non-trivial 
	 * target that must not be active more than once concurrently!
	 */
	origin_node->props.delay_resume_if_new = 1;

	return 1;
}

static int _add_snapshot_target(struct dm_tree_node *node,
				uint64_t size,
				const char *origin_uuid,
				const char *cow_uuid,
				const char *merge_uuid,
				int persistent,
				uint32_t chunk_size)
{
	struct load_segment *seg;
	struct dm_tree_node *origin_node, *cow_node, *merge_node;
	unsigned seg_type;

	seg_type = !merge_uuid ? SEG_SNAPSHOT : SEG_SNAPSHOT_MERGE;

	if (!(seg = _add_segment(node, seg_type, size)))
		return_0;

	if (!(origin_node = dm_tree_find_node_by_uuid(node->dtree, origin_uuid))) {
		log_error("Couldn't find snapshot origin uuid %s.", origin_uuid);
		return 0;
	}

	seg->origin = origin_node;
	if (!_link_tree_nodes(node, origin_node))
		return_0;

	if (!(cow_node = dm_tree_find_node_by_uuid(node->dtree, cow_uuid))) {
		log_error("Couldn't find snapshot COW device uuid %s.", cow_uuid);
		return 0;
	}

	seg->cow = cow_node;
	if (!_link_tree_nodes(node, cow_node))
		return_0;

	seg->persistent = persistent ? 1 : 0;
	seg->chunk_size = chunk_size;

	if (merge_uuid) {
		if (!(merge_node = dm_tree_find_node_by_uuid(node->dtree, merge_uuid))) {
			/* not a pure error, merging snapshot may have been deactivated */
			log_verbose("Couldn't find merging snapshot uuid %s.", merge_uuid);
		} else {
			seg->merge = merge_node;
			/* must not link merging snapshot, would undermine activation_priority below */
		}

		/* Resume snapshot-merge (acting origin) after other snapshots */
		node->activation_priority = 1;
		if (seg->merge) {
			/* Resume merging snapshot after snapshot-merge */
			seg->merge->activation_priority = 2;
		}
	}

	return 1;
}


int dm_tree_node_add_snapshot_target(struct dm_tree_node *node,
				     uint64_t size,
				     const char *origin_uuid,
				     const char *cow_uuid,
				     int persistent,
				     uint32_t chunk_size)
{
	return _add_snapshot_target(node, size, origin_uuid, cow_uuid,
				    NULL, persistent, chunk_size);
}

int dm_tree_node_add_snapshot_merge_target(struct dm_tree_node *node,
					   uint64_t size,
					   const char *origin_uuid,
					   const char *cow_uuid,
					   const char *merge_uuid,
					   uint32_t chunk_size)
{
	return _add_snapshot_target(node, size, origin_uuid, cow_uuid,
				    merge_uuid, 1, chunk_size);
}

int dm_tree_node_add_error_target(struct dm_tree_node *node,
				  uint64_t size)
{
	if (!_add_segment(node, SEG_ERROR, size))
		return_0;

	return 1;
}

int dm_tree_node_add_zero_target(struct dm_tree_node *node,
				 uint64_t size)
{
	if (!_add_segment(node, SEG_ZERO, size))
		return_0;

	return 1;
}

int dm_tree_node_add_linear_target(struct dm_tree_node *node,
				   uint64_t size)
{
	if (!_add_segment(node, SEG_LINEAR, size))
		return_0;

	return 1;
}

int dm_tree_node_add_striped_target(struct dm_tree_node *node,
				    uint64_t size,
				    uint32_t stripe_size)
{
	struct load_segment *seg;

	if (!(seg = _add_segment(node, SEG_STRIPED, size)))
		return_0;

	seg->stripe_size = stripe_size;

	return 1;
}

int dm_tree_node_add_crypt_target(struct dm_tree_node *node,
				  uint64_t size,
				  const char *cipher,
				  const char *chainmode,
				  const char *iv,
				  uint64_t iv_offset,
				  const char *key)
{
	struct load_segment *seg;

	if (!(seg = _add_segment(node, SEG_CRYPT, size)))
		return_0;

	seg->cipher = cipher;
	seg->chainmode = chainmode;
	seg->iv = iv;
	seg->iv_offset = iv_offset;
	seg->key = key;

	return 1;
}

int dm_tree_node_add_mirror_target_log(struct dm_tree_node *node,
				       uint32_t region_size,
				       unsigned clustered,
				       const char *log_uuid,
				       unsigned area_count,
				       uint32_t flags)
{
	struct dm_tree_node *log_node = NULL;
	struct load_segment *seg;

	if (!(seg = _get_last_load_segment(node)))
		return_0;

	if (log_uuid) {
		if (!(seg->uuid = dm_pool_strdup(node->dtree->mem, log_uuid))) {
			log_error("log uuid pool_strdup failed");
			return 0;
		}
		if ((flags & DM_CORELOG))
			/* For pvmove: immediate resume (for size validation) isn't needed. */
			/* pvmove flag passed via unused UUID and its suffix */
			node->props.delay_resume_if_new = strstr(log_uuid, "pvmove") ? 2 : 1;
		else {
			if (!(log_node = dm_tree_find_node_by_uuid(node->dtree, log_uuid))) {
				log_error("Couldn't find mirror log uuid %s.", log_uuid);
				return 0;
			}

			if (clustered)
				log_node->props.immediate_dev_node = 1;

			/* The kernel validates the size of disk logs. */
			/* FIXME Propagate to any devices below */
			log_node->props.delay_resume_if_new = 0;

			if (!_link_tree_nodes(node, log_node))
				return_0;
		}
	}

	seg->log = log_node;
	seg->region_size = region_size;
	seg->clustered = clustered;
	seg->mirror_area_count = area_count;
	seg->flags = flags;

	return 1;
}

int dm_tree_node_add_mirror_target(struct dm_tree_node *node,
				   uint64_t size)
{
	if (!_add_segment(node, SEG_MIRRORED, size))
		return_0;

	return 1;
}

int dm_tree_node_add_raid_target_with_params(struct dm_tree_node *node,
					     uint64_t size,
					     const struct dm_tree_node_raid_params *p)
{
	unsigned i;
	struct load_segment *seg = NULL;

	for (i = 0; i < DM_ARRAY_SIZE(_dm_segtypes) && !seg; ++i)
		if (!strcmp(p->raid_type, _dm_segtypes[i].target))
			if (!(seg = _add_segment(node,
						 _dm_segtypes[i].type, size)))
				return_0;
	if (!seg) {
		log_error("Unsupported raid type %s.", p->raid_type);
		return 0;
	}

	seg->region_size = p->region_size;
	seg->stripe_size = p->stripe_size;
	seg->area_count = 0;
	memset(seg->rebuilds, 0, sizeof(seg->rebuilds));
	seg->rebuilds[0] = p->rebuilds;
	memset(seg->writemostly, 0, sizeof(seg->writemostly));
	seg->writemostly[0] = p->writemostly;
	seg->writebehind = p->writebehind;
	seg->min_recovery_rate = p->min_recovery_rate;
	seg->max_recovery_rate = p->max_recovery_rate;
	seg->flags = p->flags;

	return 1;
}

int dm_tree_node_add_raid_target(struct dm_tree_node *node,
				 uint64_t size,
				 const char *raid_type,
				 uint32_t region_size,
				 uint32_t stripe_size,
				 uint64_t rebuilds,
				 uint64_t flags)
{
	struct dm_tree_node_raid_params params = {
		.raid_type = raid_type,
		.region_size = region_size,
		.stripe_size = stripe_size,
		.rebuilds = rebuilds,
		.flags = flags
	};

	return dm_tree_node_add_raid_target_with_params(node, size, &params);
}

/*
 * Version 2 of dm_tree_node_add_raid_target() allowing for:
 *
 * - maximum 253 legs in a raid set (MD kernel limitation)
 * - delta_disks for disk add/remove reshaping
 * - data_offset for out-of-place reshaping
 * - data_copies to cope witth odd numbers of raid10 disks
 */
int dm_tree_node_add_raid_target_with_params_v2(struct dm_tree_node *node,
					        uint64_t size,
						const struct dm_tree_node_raid_params_v2 *p)
{
	unsigned i;
	struct load_segment *seg = NULL;

	for (i = 0; i < DM_ARRAY_SIZE(_dm_segtypes) && !seg; ++i)
		if (!strcmp(p->raid_type, _dm_segtypes[i].target))
			if (!(seg = _add_segment(node,
						 _dm_segtypes[i].type, size)))
				return_0;
	if (!seg) {
		log_error("Unsupported raid type %s.", p->raid_type);
		return 0;
	}

	seg->region_size = p->region_size;
	seg->stripe_size = p->stripe_size;
	seg->area_count = 0;
	seg->delta_disks = p->delta_disks;
	seg->data_offset = p->data_offset;
	memcpy(seg->rebuilds, p->rebuilds, sizeof(seg->rebuilds));
	memcpy(seg->writemostly, p->writemostly, sizeof(seg->writemostly));
	seg->writebehind = p->writebehind;
	seg->data_copies = p->data_copies;
	seg->min_recovery_rate = p->min_recovery_rate;
	seg->max_recovery_rate = p->max_recovery_rate;
	seg->flags = p->flags;

	return 1;
}

int dm_tree_node_add_cache_target(struct dm_tree_node *node,
				  uint64_t size,
				  uint64_t feature_flags, /* DM_CACHE_FEATURE_* */
				  const char *metadata_uuid,
				  const char *data_uuid,
				  const char *origin_uuid,
				  const char *policy_name,
				  const struct dm_config_node *policy_settings,
				  uint32_t data_block_size)
{
	struct dm_config_node *cn;
	struct load_segment *seg;
	static const uint64_t _modemask =
		DM_CACHE_FEATURE_PASSTHROUGH |
		DM_CACHE_FEATURE_WRITETHROUGH |
		DM_CACHE_FEATURE_WRITEBACK;

	/* Detect unknown (bigger) feature bit */
	if (feature_flags >= (DM_CACHE_FEATURE_METADATA2 * 2)) {
		log_error("Unsupported cache's feature flags set " FMTu64 ".",
			  feature_flags);
		return 0;
	}

	switch (feature_flags & _modemask) {
	case DM_CACHE_FEATURE_PASSTHROUGH:
	case DM_CACHE_FEATURE_WRITEBACK:
		if (strcmp(policy_name, "cleaner") == 0) {
			/* Enforce writethrough mode for cleaner policy */
			feature_flags = ~_modemask;
			feature_flags |= DM_CACHE_FEATURE_WRITETHROUGH;
		}
                /* Fall through */
	case DM_CACHE_FEATURE_WRITETHROUGH:
		break;
	default:
		log_error("Invalid cache's feature flag " FMTu64 ".",
			  feature_flags);
		return 0;
	}

	if (data_block_size < DM_CACHE_MIN_DATA_BLOCK_SIZE) {
		log_error("Data block size %u is lower then %u sectors.",
			  data_block_size, DM_CACHE_MIN_DATA_BLOCK_SIZE);
		return 0;
	}

	if (data_block_size > DM_CACHE_MAX_DATA_BLOCK_SIZE) {
		log_error("Data block size %u is higher then %u sectors.",
			  data_block_size, DM_CACHE_MAX_DATA_BLOCK_SIZE);
		return 0;
	}

	if (!(seg = _add_segment(node, SEG_CACHE, size)))
		return_0;

	if (!(seg->pool = dm_tree_find_node_by_uuid(node->dtree,
						    data_uuid))) {
		log_error("Missing cache's data uuid %s.",
			  data_uuid);
		return 0;
	}
	if (!_link_tree_nodes(node, seg->pool))
		return_0;

	if (!(seg->metadata = dm_tree_find_node_by_uuid(node->dtree,
							metadata_uuid))) {
		log_error("Missing cache's metadata uuid %s.",
			  metadata_uuid);
		return 0;
	}
	if (!_link_tree_nodes(node, seg->metadata))
		return_0;

	if (!(seg->origin = dm_tree_find_node_by_uuid(node->dtree,
						      origin_uuid))) {
		log_error("Missing cache's origin uuid %s.",
			  metadata_uuid);
		return 0;
	}
	if (!_link_tree_nodes(node, seg->origin))
		return_0;

	seg->data_block_size = data_block_size;
	seg->flags = feature_flags;
	seg->policy_name = policy_name;

	/* FIXME: better validation missing */
	if (policy_settings) {
		if (!(seg->policy_settings = dm_config_clone_node_with_mem(node->dtree->mem, policy_settings, 0)))
			return_0;

		for (cn = seg->policy_settings->child; cn; cn = cn->sib) {
			if (!cn->v || (cn->v->type != DM_CFG_INT)) {
				/* For now only  <key> = <int>  pairs are supported */
				log_error("Cache policy parameter %s is without integer value.", cn->key);
				return 0;
			}
			seg->policy_argc++;
		}
	}

	return 1;
}

int dm_tree_node_add_replicator_target(struct dm_tree_node *node,
				       uint64_t size,
				       const char *rlog_uuid,
				       const char *rlog_type,
				       unsigned rsite_index,
				       dm_replicator_mode_t mode,
				       uint32_t async_timeout,
				       uint64_t fall_behind_data,
				       uint32_t fall_behind_ios)
{
	log_error("Replicator segment is unsupported.");
	return 0;
}

/* Appends device node to Replicator */
int dm_tree_node_add_replicator_dev_target(struct dm_tree_node *node,
					   uint64_t size,
					   const char *replicator_uuid,
					   uint64_t rdevice_index,
					   const char *rdev_uuid,
					   unsigned rsite_index,
					   const char *slog_uuid,
					   uint32_t slog_flags,
					   uint32_t slog_region_size)
{
	log_error("Replicator targer is unsupported.");
	return 0;
}

static struct load_segment *_get_single_load_segment(struct dm_tree_node *node,
						     unsigned type)
{
	struct load_segment *seg;

	if (!(seg = _get_last_load_segment(node)))
		return_NULL;

	/* Never used past _load_node(), so can test segment_count */
	if (node->props.segment_count != 1) {
		log_error("Node %s must have only one segment.",
			  _dm_segtypes[type].target);
		return NULL;
	}

	if (seg->type != type) {
		log_error("Node %s has segment type %s.",
			  _dm_segtypes[type].target,
			  _dm_segtypes[seg->type].target);
		return NULL;
	}

	return seg;
}

static int _thin_validate_device_id(uint32_t device_id)
{
	if (device_id > DM_THIN_MAX_DEVICE_ID) {
		log_error("Device id %u is higher then %u.",
			  device_id, DM_THIN_MAX_DEVICE_ID);
		return 0;
	}

	return 1;
}

int dm_tree_node_add_thin_pool_target(struct dm_tree_node *node,
				      uint64_t size,
				      uint64_t transaction_id,
				      const char *metadata_uuid,
				      const char *pool_uuid,
				      uint32_t data_block_size,
				      uint64_t low_water_mark,
				      unsigned skip_block_zeroing)
{
	struct load_segment *seg, *mseg;
	uint64_t devsize = 0;

	if (data_block_size < DM_THIN_MIN_DATA_BLOCK_SIZE) {
		log_error("Data block size %u is lower then %u sectors.",
			  data_block_size, DM_THIN_MIN_DATA_BLOCK_SIZE);
		return 0;
	}

	if (data_block_size > DM_THIN_MAX_DATA_BLOCK_SIZE) {
		log_error("Data block size %u is higher then %u sectors.",
			  data_block_size, DM_THIN_MAX_DATA_BLOCK_SIZE);
		return 0;
	}

	if (!(seg = _add_segment(node, SEG_THIN_POOL, size)))
		return_0;

	if (!(seg->metadata = dm_tree_find_node_by_uuid(node->dtree, metadata_uuid))) {
		log_error("Missing metadata uuid %s.", metadata_uuid);
		return 0;
	}

	if (!_link_tree_nodes(node, seg->metadata))
		return_0;

	/* FIXME: more complex target may need more tweaks */
	dm_list_iterate_items(mseg, &seg->metadata->props.segs) {
		devsize += mseg->size;
		if (devsize > DM_THIN_MAX_METADATA_SIZE) {
			log_debug_activation("Ignoring %" PRIu64 " of device.",
					     devsize - DM_THIN_MAX_METADATA_SIZE);
			mseg->size -= (devsize - DM_THIN_MAX_METADATA_SIZE);
			devsize = DM_THIN_MAX_METADATA_SIZE;
			/* FIXME: drop remaining segs */
		}
	}

	if (!(seg->pool = dm_tree_find_node_by_uuid(node->dtree, pool_uuid))) {
		log_error("Missing pool uuid %s.", pool_uuid);
		return 0;
	}

	if (!_link_tree_nodes(node, seg->pool))
		return_0;

	/* Clean flag delay_resume_if_new - so corelog gets resumed */
	seg->metadata->props.delay_resume_if_new = 0;
	seg->pool->props.delay_resume_if_new = 0;

	/* Preload must not resume extended running thin-pool before it's committed */
	node->props.delay_resume_if_extended = 1;

	/* Validate only transaction_id > 0 when activating thin-pool */
	node->props.send_messages = transaction_id ? 1 : 0;
	seg->transaction_id = transaction_id;
	seg->low_water_mark = low_water_mark;
	seg->data_block_size = data_block_size;
	seg->skip_block_zeroing = skip_block_zeroing;
	dm_list_init(&seg->thin_messages);

	return 1;
}

int dm_tree_node_add_thin_pool_message(struct dm_tree_node *node,
				       dm_thin_message_t type,
				       uint64_t id1, uint64_t id2)
{
	struct thin_message *tm;
	struct load_segment *seg;

	if (!(seg = _get_single_load_segment(node, SEG_THIN_POOL)))
		return_0;

	if (!(tm = dm_pool_zalloc(node->dtree->mem, sizeof (*tm)))) {
		log_error("Failed to allocate thin message.");
		return 0;
	}

	switch (type) {
	case DM_THIN_MESSAGE_CREATE_SNAP:
		/* If the thin origin is active, it must be suspend first! */
		if (id1 == id2) {
			log_error("Cannot use same device id for origin and its snapshot.");
			return 0;
		}
		if (!_thin_validate_device_id(id1) ||
		    !_thin_validate_device_id(id2))
			return_0;
		tm->message.u.m_create_snap.device_id = id1;
		tm->message.u.m_create_snap.origin_id = id2;
		break;
	case DM_THIN_MESSAGE_CREATE_THIN:
		if (!_thin_validate_device_id(id1))
			return_0;
		tm->message.u.m_create_thin.device_id = id1;
		tm->expected_errno = EEXIST;
		break;
	case DM_THIN_MESSAGE_DELETE:
		if (!_thin_validate_device_id(id1))
			return_0;
		tm->message.u.m_delete.device_id = id1;
		tm->expected_errno = ENODATA;
		break;
	case DM_THIN_MESSAGE_SET_TRANSACTION_ID:
		if ((id1 + 1) != id2) {
			log_error("New transaction id must be sequential.");
			return 0; /* FIXME: Maybe too strict here? */
		}
		if (id2 != seg->transaction_id) {
			log_error("Current transaction id is different from thin pool.");
			return 0; /* FIXME: Maybe too strict here? */
		}
		tm->message.u.m_set_transaction_id.current_id = id1;
		tm->message.u.m_set_transaction_id.new_id = id2;
		break;
	default:
		log_error("Unsupported message type %d.", (int) type);
		return 0;
	}

	tm->message.type = type;
	dm_list_add(&seg->thin_messages, &tm->list);
	/* Higher value >1 identifies there are really some messages */
	node->props.send_messages = 2;

	return 1;
}

int dm_tree_node_set_thin_pool_discard(struct dm_tree_node *node,
				       unsigned ignore,
				       unsigned no_passdown)
{
	struct load_segment *seg;

	if (!(seg = _get_single_load_segment(node, SEG_THIN_POOL)))
		return_0;

	seg->ignore_discard = ignore;
	seg->no_discard_passdown = no_passdown;

	return 1;
}

int dm_tree_node_set_thin_pool_error_if_no_space(struct dm_tree_node *node,
						 unsigned error_if_no_space)
{
	struct load_segment *seg;

	if (!(seg = _get_single_load_segment(node, SEG_THIN_POOL)))
		return_0;

	seg->error_if_no_space = error_if_no_space;

	return 1;
}

int dm_tree_node_set_thin_pool_read_only(struct dm_tree_node *node,
					 unsigned read_only)
{
	struct load_segment *seg;

	if (!(seg = _get_single_load_segment(node, SEG_THIN_POOL)))
		return_0;

	seg->read_only = read_only;

	return 1;
}

int dm_tree_node_add_thin_target(struct dm_tree_node *node,
				 uint64_t size,
				 const char *pool_uuid,
				 uint32_t device_id)
{
	struct dm_tree_node *pool;
	struct load_segment *seg;

	if (!(pool = dm_tree_find_node_by_uuid(node->dtree, pool_uuid))) {
		log_error("Missing thin pool uuid %s.", pool_uuid);
		return 0;
	}

	if (!_link_tree_nodes(node, pool))
		return_0;

	if (!_thin_validate_device_id(device_id))
		return_0;

	if (!(seg = _add_segment(node, SEG_THIN, size)))
		return_0;

	seg->pool = pool;
	seg->device_id = device_id;

	return 1;
}

int dm_tree_node_set_thin_external_origin(struct dm_tree_node *node,
					  const char *external_uuid)
{
	struct dm_tree_node *external;
	struct load_segment *seg;

	if (!(seg = _get_single_load_segment(node, SEG_THIN)))
		return_0;

	if (!(external = dm_tree_find_node_by_uuid(node->dtree,
						   external_uuid))) {
		log_error("Missing thin external origin uuid %s.",
			  external_uuid);
		return 0;
	}

	if (!_link_tree_nodes(node, external))
		return_0;

	seg->external = external;

	return 1;
}

static int _add_area(struct dm_tree_node *node, struct load_segment *seg, struct dm_tree_node *dev_node, uint64_t offset)
{
	struct seg_area *area;

	if (!(area = dm_pool_zalloc(node->dtree->mem, sizeof (*area)))) {
		log_error("Failed to allocate target segment area.");
		return 0;
	}

	area->dev_node = dev_node;
	area->offset = offset;

	dm_list_add(&seg->areas, &area->list);
	seg->area_count++;

	return 1;
}

int dm_tree_node_add_target_area(struct dm_tree_node *node,
				 const char *dev_name,
				 const char *uuid,
				 uint64_t offset)
{
	struct load_segment *seg;
	struct stat info;
	struct dm_tree_node *dev_node;

	if ((!dev_name || !*dev_name) && (!uuid || !*uuid)) {
		log_error("dm_tree_node_add_target_area called without device");
		return 0;
	}

	if (uuid) {
		if (!(dev_node = dm_tree_find_node_by_uuid(node->dtree, uuid))) {
			log_error("Couldn't find area uuid %s.", uuid);
			return 0;
		}
		if (!_link_tree_nodes(node, dev_node))
			return_0;
	} else {
		if (stat(dev_name, &info) < 0) {
			log_error("Device %s not found.", dev_name);
			return 0;
		}

		if (!S_ISBLK(info.st_mode)) {
			log_error("Device %s is not a block device.", dev_name);
			return 0;
		}

		/* FIXME Check correct macro use */
		if (!(dev_node = _add_dev(node->dtree, node, MAJOR(info.st_rdev),
					  MINOR(info.st_rdev), 0, 0)))
			return_0;
	}

	if (!(seg = _get_last_load_segment(node)))
		return_0;

	if (!_add_area(node, seg, dev_node, offset))
		return_0;

	return 1;
}

int dm_tree_node_add_null_area(struct dm_tree_node *node, uint64_t offset)
{
	struct load_segment *seg;

	if (!(seg = _get_last_load_segment(node)))
		return_0;

	switch (seg->type) {
	case SEG_RAID0:
	case SEG_RAID0_META:
	case SEG_RAID1:
	case SEG_RAID4:
	case SEG_RAID5_N:
	case SEG_RAID5_LA:
	case SEG_RAID5_RA:
	case SEG_RAID5_LS:
	case SEG_RAID5_RS:
	case SEG_RAID6_N_6:
	case SEG_RAID6_ZR:
	case SEG_RAID6_NR:
	case SEG_RAID6_NC:
	case SEG_RAID6_LS_6:
	case SEG_RAID6_RS_6:
	case SEG_RAID6_LA_6:
	case SEG_RAID6_RA_6:
		break;
	default:
		log_error("dm_tree_node_add_null_area() called on an unsupported segment type");
		return 0;
	}

	if (!_add_area(node, seg, NULL, offset))
		return_0;

	return 1;
}

void dm_tree_node_set_callback(struct dm_tree_node *dnode,
			       dm_node_callback_fn cb, void *data)
{
	dnode->callback = cb;
	dnode->callback_data = data;
}

#if defined(__GNUC__)
/*
 * Backward compatible implementations.
 *
 * Keep these at the end of the file to make sure that
 * no code in this file accidentally calls it.
 */

/* Backward compatible dm_tree_node_size_changed() implementations. */
int dm_tree_node_size_changed_base(const struct dm_tree_node *dnode);
int dm_tree_node_size_changed_base(const struct dm_tree_node *dnode)
{
	/* Base does not make difference between smaller and bigger */
	return dm_tree_node_size_changed(dnode) ? 1 : 0;
}

/*
 * Retain ABI compatibility after adding the DM_CACHE_FEATURE_METADATA2
 * in version 1.02.138.
 *
 * Binaries compiled against version 1.02.138 onwards will use
 * the new function dm_tree_node_add_cache_target which detects unknown
 * feature flags and returns error for them.
 */
int dm_tree_node_add_cache_target_base(struct dm_tree_node *node,
				       uint64_t size,
				       uint64_t feature_flags, /* DM_CACHE_FEATURE_* */
				       const char *metadata_uuid,
				       const char *data_uuid,
				       const char *origin_uuid,
				       const char *policy_name,
				       const struct dm_config_node *policy_settings,
				       uint32_t data_block_size);
int dm_tree_node_add_cache_target_base(struct dm_tree_node *node,
				       uint64_t size,
				       uint64_t feature_flags,
				       const char *metadata_uuid,
				       const char *data_uuid,
				       const char *origin_uuid,
				       const char *policy_name,
				       const struct dm_config_node *policy_settings,
				       uint32_t data_block_size)
{
	/* Old version supported only these FEATURE bits, others were ignored so masked them */
	static const uint64_t _mask =
		DM_CACHE_FEATURE_WRITEBACK |
		DM_CACHE_FEATURE_WRITETHROUGH |
		DM_CACHE_FEATURE_PASSTHROUGH;

	return dm_tree_node_add_cache_target(node, size, feature_flags & _mask,
					     metadata_uuid, data_uuid, origin_uuid,
					     policy_name, policy_settings, data_block_size);
}
#endif

int dm_tree_node_add_vdo_target(struct dm_tree_node *node,
				uint64_t size,
				const char *data_uuid,
				const struct dm_vdo_target_params *vtp)
{
	struct load_segment *seg;

	if (!(seg = _add_segment(node, SEG_VDO, size)))
		return_0;

	if (!(seg->vdo_data = dm_tree_find_node_by_uuid(node->dtree, data_uuid))) {
		log_error("Missing VDO's data uuid %s.", data_uuid);
		return 0;
	}

	if (!dm_vdo_validate_target_params(vtp, size))
		return_0;

	if (!_link_tree_nodes(node, seg->vdo_data))
		return_0;

	seg->vdo_params = *vtp;
	seg->vdo_name = node->name;

	node->props.send_messages = 2;

	return 1;
}
