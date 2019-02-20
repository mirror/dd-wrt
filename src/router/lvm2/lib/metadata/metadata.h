/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2013 Red Hat, Inc. All rights reserved.
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
 * This is the in core representation of a volume group and its
 * associated physical and logical volumes.
 */

#ifndef _LVM_METADATA_H
#define _LVM_METADATA_H

#include "ctype.h"
#include "lib/device/dev-cache.h"
#include "lib/misc/lvm-string.h"
#include "lib/metadata/metadata-exported.h"

//#define MAX_STRIPES 128U
//#define SECTOR_SHIFT 9L
//#define SECTOR_SIZE ( 1L << SECTOR_SHIFT )
//#define STRIPE_SIZE_MIN ( (unsigned) lvm_getpagesize() >> SECTOR_SHIFT)	/* PAGESIZE in sectors */
//#define STRIPE_SIZE_MAX ( 512L * 1024L >> SECTOR_SHIFT)	/* 512 KB in sectors */
//#define STRIPE_SIZE_LIMIT ((UINT_MAX >> 2) + 1)
//#define MAX_RESTRICTED_LVS 255	/* Used by FMT_RESTRICTED_LVIDS */
#define MIN_PE_SIZE     (8192L >> SECTOR_SHIFT) /* 8 KB in sectors - format1 only */
#define MAX_PE_SIZE     (16L * 1024L * (1024L >> SECTOR_SHIFT) * 1024L) /* format1 only */
#define MIRROR_LOG_OFFSET	2	/* sectors */
#define VG_MEMPOOL_CHUNK	10240	/* in bytes, hint only */

/*
 * Ceiling(n / sz)
 */
#define dm_div_up(n, sz) (((n) + (sz) - 1) / (sz))

/*
 * Ceiling(n / size) * size
 */
#define dm_round_up(n, sz) (dm_div_up((n), (sz)) * (sz))


/* Various flags */
/* See metadata-exported.h for the complete list. */
/* Note that the bits no longer necessarily correspond to LVM1 disk format */

/* May any free extents on this PV be used or must they be left free? */

#define SPINDOWN_LV          	UINT64_C(0x00000010)	/* LV */
#define BADBLOCK_ON       	UINT64_C(0x00000020)	/* LV */
//#define VIRTUAL			UINT64_C(0x00010000)	/* LV - internal use only */
#define PRECOMMITTED		UINT64_C(0x00200000)	/* VG - internal use only */
#define POSTORDER_FLAG		UINT64_C(0x02000000) /* Not real flags, reserved for  */
#define POSTORDER_OPEN_FLAG	UINT64_C(0x04000000) /* temporary use inside vg_read_internal. */

#define SHARED            	UINT64_C(0x00000800)	/* VG */

/* Format features flags */
#define FMT_PRECOMMIT		0x00000040U	/* Supports pre-commit? */

struct dm_config_tree;
struct metadata_area;
struct alloc_handle;
struct lvmcache_info;
struct cached_vg_fmtdata;

/* Per-format per-metadata area operations */
struct metadata_area_ops {
	struct dm_list list;
	struct volume_group *(*vg_read) (struct format_instance * fi,
					 const char *vg_name,
					 struct metadata_area * mda,
					 struct cached_vg_fmtdata **vg_fmtdata,
					 unsigned *use_previous_vg);
	struct volume_group *(*vg_read_precommit) (struct format_instance * fi,
					 const char *vg_name,
					 struct metadata_area * mda,
					 struct cached_vg_fmtdata **vg_fmtdata,
					 unsigned *use_previous_vg);
	/*
	 * Write out complete VG metadata.  You must ensure internal
	 * consistency before calling. eg. PEs can't refer to PVs not
	 * part of the VG.
	 *
	 * It is also the responsibility of the caller to ensure external
	 * consistency, eg by calling pv_write() if removing PVs from
	 * a VG or calling vg_write() a second time if splitting a VG
	 * into two.
	 *
	 * vg_write() should not read or write from any PVs not included
	 * in the volume_group structure it is handed.
	 * (format1 currently breaks this rule.)
	 */
	int (*vg_write) (struct format_instance * fid, struct volume_group * vg,
			 struct metadata_area * mda);
	int (*vg_precommit) (struct format_instance * fid,
			     struct volume_group * vg,
			     struct metadata_area * mda);
	int (*vg_commit) (struct format_instance * fid,
			  struct volume_group * vg, struct metadata_area * mda);
	int (*vg_revert) (struct format_instance * fid,
			  struct volume_group * vg, struct metadata_area * mda);
	int (*vg_remove) (struct format_instance * fi, struct volume_group * vg,
			  struct metadata_area * mda);

	/*
	 * Per location copy constructor.
	 */
	void *(*mda_metadata_locn_copy) (struct dm_pool *mem, void *metadata_locn);

	/*
	 * Per location description for logging.
	 */
	const char *(*mda_metadata_locn_name) (void *metadata_locn);
	uint64_t (*mda_metadata_locn_offset) (void *metadata_locn);

	/*
	 * Returns number of free sectors in given metadata area.
	 */
	uint64_t (*mda_free_sectors) (struct metadata_area *mda);

	/*
	 * Returns number of total sectors in given metadata area.
	 */
	uint64_t (*mda_total_sectors) (struct metadata_area *mda);

	/*
	 * Check if metadata area belongs to vg
	 */
	int (*mda_in_vg) (struct format_instance * fi,
			    struct volume_group * vg, struct metadata_area *mda);
	/*
	 * Analyze a metadata area on a PV.
	 */
	int (*pv_analyze_mda) (const struct format_type * fmt,
			       struct metadata_area *mda);

	/*
	 * Do these two metadata_area structures match with respect to
	 * their underlying location?
	 */
	unsigned (*mda_locns_match)(struct metadata_area *mda1,
				    struct metadata_area *mda2);

	struct device *(*mda_get_device)(struct metadata_area *mda);
	int (*mda_export_text)(struct metadata_area *mda, struct dm_config_tree *cft,
			       struct dm_config_node *parent);
	int (*mda_import_text)(struct lvmcache_info *info, const struct dm_config_node *cn);
};

#define MDA_IGNORED      0x00000001
#define MDA_INCONSISTENT 0x00000002
#define MDA_FAILED       0x00000004

/* The primary metadata area on a device if the format supports more than one. */
#define MDA_PRIMARY	 0x00000008

#define mda_is_primary(mda) (((mda->status) & MDA_PRIMARY) ? 1 : 0)
#define MDA_CONTENT_REASON(primary_mda) ((primary_mda) ? DEV_IO_MDA_CONTENT : DEV_IO_MDA_EXTRA_CONTENT)
#define MDA_HEADER_REASON(primary_mda)  ((primary_mda) ? DEV_IO_MDA_HEADER : DEV_IO_MDA_EXTRA_HEADER)

struct metadata_area {
	struct dm_list list;
	struct metadata_area_ops *ops;
	void *metadata_locn;
	uint32_t status;
};
struct metadata_area *mda_copy(struct dm_pool *mem,
			       struct metadata_area *mda);

unsigned mda_is_ignored(struct metadata_area *mda);
void mda_set_ignored(struct metadata_area *mda, unsigned mda_ignored);
unsigned mda_locns_match(struct metadata_area *mda1, struct metadata_area *mda2);
struct device *mda_get_device(struct metadata_area *mda);

/*
 * fic is used to create an fid.  It's used to pass fmt/vgname/vgid args
 * to create_instance() which creates an fid for the specified vg.
 */

struct format_instance_ctx {
	uint32_t type;
	union {
		const char *pv_id;
		struct {
			const char *vg_name;
			const char *vg_id;
		} vg_ref;
		void *private;
	} context;
};

struct format_instance *alloc_fid(const struct format_type *fmt,
				  const struct format_instance_ctx *fic);

/*
 * Format instance must always be set using pv_set_fid or vg_set_fid
 * (NULL value as well), never asign it directly! This is essential
 * for proper reference counting for the format instance.
 */
void pv_set_fid(struct physical_volume *pv, struct format_instance *fid);
void vg_set_fid(struct volume_group *vg, struct format_instance *fid);

/* FIXME: Add generic interface for mda counts based on given key. */
int fid_add_mda(struct format_instance *fid, struct metadata_area *mda,
		const char *key, size_t key_len, const unsigned sub_key);
int fid_add_mdas(struct format_instance *fid, struct dm_list *mdas,
		 const char *key, size_t key_len);
int fid_remove_mda(struct format_instance *fid, struct metadata_area *mda,
		   const char *key, size_t key_len, const unsigned sub_key);
struct metadata_area *fid_get_mda_indexed(struct format_instance *fid,
		const char *key, size_t key_len, const unsigned sub_key);
int mdas_empty_or_ignored(struct dm_list *mdas);

#define seg_pvseg(seg, s)	(seg)->areas[(s)].u.pv.pvseg
#define seg_dev(seg, s)		(seg)->areas[(s)].u.pv.pvseg->pv->dev
#define seg_pe(seg, s)		(seg)->areas[(s)].u.pv.pvseg->pe
#define seg_le(seg, s)		(seg)->areas[(s)].u.lv.le
#define seg_metale(seg, s)	(seg)->meta_areas[(s)].u.lv.le

struct name_list {
	struct dm_list list;
	char *name;
};

struct mda_list {
	struct dm_list list;
	struct device_area mda;
};

struct peg_list {
	struct dm_list list;
	struct pv_segment *peg;
};

struct seg_list {
	struct dm_list list;
	unsigned count;
	struct lv_segment *seg;
};

/*
 * Ownership of objects passes to caller.
 */
struct format_handler {
	/*
	 * Scan any metadata areas that aren't referenced in PV labels
	 */
	int (*scan) (const struct format_type * fmt, const char *vgname);

	/*
	 * Return PV with given path.
	 */
	int (*pv_read) (const struct format_type * fmt, const char *pv_name,
			struct physical_volume * pv, int scan_label_only);

	/*
	 * Initialise a new PV.
	 */
	int (*pv_initialise) (const struct format_type * fmt,
			      struct pv_create_args *pva,
			      struct physical_volume * pv);

	/*
	 * Tweak an already filled out a pv ready for importing into a
	 * vg.  eg. pe_count is format specific.
	 */
	int (*pv_setup) (const struct format_type * fmt,
			 struct physical_volume * pv,
			 struct volume_group * vg);

	/*
	 * Add metadata area to a PV. Changes will take effect on pv_write.
	 */
	int (*pv_add_metadata_area) (const struct format_type * fmt,
				     struct physical_volume * pv,
				     int pe_start_locked,
				     unsigned metadata_index,
				     uint64_t metadata_size,
				     unsigned metadata_ignored);

	/*
	 * Remove metadata area from a PV. Changes will take effect on pv_write.
	 */
	int (*pv_remove_metadata_area) (const struct format_type *fmt,
					struct physical_volume *pv,
					unsigned metadata_index);

	/*
	 * Recalculate the PV size taking into account any existing metadata areas.
	 */
	int (*pv_resize) (const struct format_type *fmt,
			  struct physical_volume *pv,
			  struct volume_group *vg,
			  uint64_t size);

	/*
	 * Write a PV structure to disk. Fails if the PV is in a VG ie
	 * pv->vg_name must be a valid orphan VG name
	 */
	int (*pv_write) (const struct format_type * fmt,
			 struct physical_volume * pv);

	/*
	 * Check if PV needs rewriting. This is used to check whether there are any
	 * format-specific changes  before actually writing the PV (by calling pv_write).
	 * With this, we can call pv_write conditionally only if it's really needed.
	 */
	int (*pv_needs_rewrite) (const struct format_type *fmt,
				 struct physical_volume *pv,
				 int *needs_rewrite);

	/*
	 * Tweak an already filled out a lv eg, check there
	 * aren't too many extents.
	 */
	int (*lv_setup) (struct format_instance * fi,
			 struct logical_volume * lv);

	/*
	 * Tweak an already filled out vg.  eg, max_pv is format
	 * specific.
	 */
	int (*vg_setup) (struct format_instance * fi, struct volume_group * vg);

	/*
	 * Check whether particular segment type is supported.
	 */
	int (*segtype_supported) (struct format_instance *fid,
				  const struct segment_type *segtype);

	/*
	 * Create format instance with a particular metadata area
	 */
	struct format_instance *(*create_instance) (const struct format_type *fmt,
						    const struct format_instance_ctx *fic);

	/*
	 * Destructor for format instance
	 */
	void (*destroy_instance) (struct format_instance * fid);

	/*
	 * Destructor for format type
	 */
	void (*destroy) (struct format_type * fmt);
};

/*
 * Utility functions
 */
unsigned long set_pe_align(struct physical_volume *pv, unsigned long data_alignment);
unsigned long set_pe_align_offset(struct physical_volume *pv,
				  unsigned long data_alignment_offset);

int pv_write_orphan(struct cmd_context *cmd, struct physical_volume *pv);

int check_dev_block_size_for_vg(struct device *dev, const struct volume_group *vg,
				unsigned int *max_phys_block_size_found);
int check_pv_dev_sizes(struct volume_group *vg);
uint32_t vg_bad_status_bits(const struct volume_group *vg, uint64_t status);
int add_pv_to_vg(struct volume_group *vg, const char *pv_name,
		 struct physical_volume *pv, int new_pv);

struct logical_volume *find_lv_in_vg_by_lvid(struct volume_group *vg,
					     const union lvid *lvid);

struct lv_list *find_lv_in_lv_list(const struct dm_list *ll,
				   const struct logical_volume *lv);

/* FIXME Merge these functions with ones above */
struct physical_volume *find_pv(struct volume_group *vg, struct device *dev);

struct pv_list *find_pv_in_pv_list(const struct dm_list *pl,
				   const struct physical_volume *pv);

/* Find LV segment containing given LE */
struct lv_segment *find_seg_by_le(const struct logical_volume *lv, uint32_t le);

/* Find pool LV segment given a thin pool data or metadata segment. */
struct lv_segment *find_pool_seg(const struct lv_segment *seg);

/* Find some unused device_id for thin pool LV segment. */
uint32_t get_free_pool_device_id(struct lv_segment *thin_pool_seg);

/* Check if the new thin-pool could be used for lvm2 thin volumes */
int check_new_thin_pool(const struct logical_volume *pool_lv);

/*
 * Remove a dev_dir if present.
 */
const char *strip_dir(const char *vg_name, const char *dev_dir);

struct logical_volume *alloc_lv(struct dm_pool *mem);

/*
 * Checks that an lv has no gaps or overlapping segments.
 * Set complete_vg to perform additional VG level checks.
 */
int check_lv_segments(struct logical_volume *lv, int complete_vg);

/*
 * Does every LV segment have the same number of stripes?
 */
int lv_has_constant_stripes(struct logical_volume *lv);

/*
 * Sometimes (eg, after an lvextend), it is possible to merge two
 * adjacent segments into a single segment.  This function trys
 * to merge as many segments as possible.
 */
int lv_merge_segments(struct logical_volume *lv);

/*
 * Ensure there's a segment boundary at a given LE, splitting if necessary
 */
int lv_split_segment(struct logical_volume *lv, uint32_t le);

/*
 * Add/remove upward link from underlying LV to the segment using it
 * FIXME: ridiculously long name
 */
int add_seg_to_segs_using_this_lv(struct logical_volume *lv, struct lv_segment *seg);
int remove_seg_from_segs_using_this_lv(struct logical_volume *lv, struct lv_segment *seg);

int add_glv_to_indirect_glvs(struct dm_pool *mem,
			     struct generic_logical_volume *origin_glv,
			     struct generic_logical_volume *glv);
int remove_glv_from_indirect_glvs(struct generic_logical_volume *origin_glv,
				  struct generic_logical_volume *glv);

int for_each_sub_lv(struct logical_volume *lv,
		    int (*fn)(struct logical_volume *lv, void *data),
		    void *data);

int move_lv_segments(struct logical_volume *lv_to,
		     struct logical_volume *lv_from,
		     uint64_t set_status, uint64_t reset_status);
/* Widen existing segment areas */
int add_lv_segment_areas(struct lv_segment *seg, uint32_t new_area_count);

/*
 * Calculate readahead from underlying PV devices
 */
void lv_calculate_readahead(const struct logical_volume *lv, uint32_t *read_ahead);

/*
 * For internal metadata caching.
 */
size_t export_vg_to_buffer(struct volume_group *vg, char **buf);
struct dm_config_tree *export_vg_to_config_tree(struct volume_group *vg);
struct volume_group *import_vg_from_config_tree(const struct dm_config_tree *cft,
						struct format_instance *fid);

/*
 * Mirroring functions
 */

/*
 * Given mirror image or mirror log segment, find corresponding mirror segment 
 */
int fixup_imported_mirrors(struct volume_group *vg);

/*
 * From thin_manip.c
 */
int attach_pool_lv(struct lv_segment *seg, struct logical_volume *pool_lv,
		   struct logical_volume *origin,
		   struct generic_logical_volume *indirect_origin,
		   struct logical_volume *merge_lv);
int detach_pool_lv(struct lv_segment *seg);
int attach_pool_message(struct lv_segment *pool_seg, dm_thin_message_t type,
			struct logical_volume *lv, uint32_t delete_id,
			int no_update);
int lv_is_merging_thin_snapshot(const struct logical_volume *lv);
int pool_has_message(const struct lv_segment *seg,
		     const struct logical_volume *lv, uint32_t device_id);
int pool_metadata_min_threshold(const struct lv_segment *pool_seg);
int pool_below_threshold(const struct lv_segment *pool_seg);
int pool_check_overprovisioning(const struct logical_volume *lv);
int create_pool(struct logical_volume *pool_lv, const struct segment_type *segtype,
		struct alloc_handle *ah, uint32_t stripes, uint32_t stripe_size);

/*
 * Begin skeleton for external LVM library
 */
struct id pv_id(const struct physical_volume *pv);
const struct format_type *pv_format_type(const struct physical_volume *pv);
struct id pv_vgid(const struct physical_volume *pv);

uint64_t find_min_mda_size(struct dm_list *mdas);
char *tags_format_and_copy(struct dm_pool *mem, const struct dm_list *tagsl);

#endif
