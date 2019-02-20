/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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
#include "import-export.h"
#include "format-text.h"
#include "layout.h"
#include "lib/device/device.h"
#include "lib/misc/lvm-file.h"
#include "lib/config/config.h"
#include "lib/display/display.h"
#include "lib/commands/toolcontext.h"
#include "lib/misc/lvm-string.h"
#include "lib/uuid/uuid.h"
#include "lib/misc/crc.h"
#include "lib/mm/xlate.h"
#include "lib/label/label.h"
#include "lib/cache/lvmcache.h"

#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>

static struct format_instance *_text_create_text_instance(const struct format_type *fmt,
							  const struct format_instance_ctx *fic);

struct text_fid_context {
	char *raw_metadata_buf;
	uint32_t raw_metadata_buf_size;
};

int rlocn_is_ignored(const struct raw_locn *rlocn)
{
	return (rlocn->flags & RAW_LOCN_IGNORED ? 1 : 0);
}

void rlocn_set_ignored(struct raw_locn *rlocn, unsigned mda_ignored)
{
	if (mda_ignored)
		rlocn->flags |= RAW_LOCN_IGNORED;
	else
		rlocn->flags &= ~RAW_LOCN_IGNORED;
}

/*
 * NOTE: Currently there can be only one vg per text file.
 */

/*
 * Only used by vgcreate.
 */
static int _text_vg_setup(struct format_instance *fid,
			  struct volume_group *vg)
{
	if (!vg_check_new_extent_size(vg->fid->fmt, vg->extent_size))
		return_0;

	return 1;
}

static uint64_t _mda_free_sectors_raw(struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;

	return mdac->free_sectors;
}

static uint64_t _mda_total_sectors_raw(struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;

	return mdac->area.size >> SECTOR_SHIFT;
}

/*
 * Check if metadata area belongs to vg
 */
static int _mda_in_vg_raw(struct format_instance *fid __attribute__((unused)),
			     struct volume_group *vg, struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, &vg->pvs)
		if (pvl->pv->dev == mdac->area.dev)
			return 1;

	return 0;
}

static unsigned _mda_locns_match_raw(struct metadata_area *mda1,
				     struct metadata_area *mda2)
{
	struct mda_context *mda1c = (struct mda_context *) mda1->metadata_locn;
	struct mda_context *mda2c = (struct mda_context *) mda2->metadata_locn;

	if ((mda1c->area.dev == mda2c->area.dev) &&
	    (mda1c->area.start == mda2c->area.start) &&
	    (mda1c->area.size == mda2c->area.size))
		return 1;

	return 0;
}

static struct device *_mda_get_device_raw(struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	return mdac->area.dev;
}

/*
 * For circular region between region_start and region_start + region_size,
 * back up one SECTOR_SIZE from 'region_ptr' and return the value.
 * This allows reverse traversal through text metadata area to find old
 * metadata.
 *
 * Parameters:
 *   region_start: start of the region (bytes)
 *   region_size: size of the region (bytes)
 *   region_ptr: pointer within the region (bytes)
 *   NOTE: region_start <= region_ptr <= region_start + region_size
 */
static uint64_t _get_prev_sector_circular(uint64_t region_start,
					  uint64_t region_size,
					  uint64_t region_ptr)
{
	if (region_ptr >= region_start + SECTOR_SIZE)
		return region_ptr - SECTOR_SIZE;

	return (region_start + region_size - SECTOR_SIZE);
}

/*
 * Analyze a metadata area for old metadata records in the circular buffer.
 * This function just looks through and makes a first pass at the data in
 * the sectors for particular things.
 * FIXME: do something with each metadata area (try to extract vg, write
 * raw data to file, etc)
 */
static int _pv_analyze_mda_raw (const struct format_type * fmt,
				struct metadata_area *mda)
{
	struct mda_header *mdah;
	struct raw_locn *rlocn;
	uint64_t area_start;
	uint64_t area_size;
	uint64_t prev_sector, prev_sector2;
	uint64_t latest_mrec_offset;
	uint64_t offset;
	uint64_t offset2;
	size_t size;
	size_t size2;
	char *buf=NULL;
	struct device_area *area;
	struct mda_context *mdac;
	int r=0;

	mdac = (struct mda_context *) mda->metadata_locn;

	log_print("Found text metadata area: offset=" FMTu64 ", size="
		  FMTu64, mdac->area.start, mdac->area.size);
	area = &mdac->area;

	if (!(mdah = raw_read_mda_header(fmt, area, mda_is_primary(mda))))
		goto_out;

	rlocn = mdah->raw_locns;

	/*
	 * The device area includes the metadata header as well as the
	 * records, so remove the metadata header from the start and size
	 */
	area_start = area->start + MDA_HEADER_SIZE;
	area_size = area->size - MDA_HEADER_SIZE;
	latest_mrec_offset = rlocn->offset + area->start;

	/*
	 * Start searching at rlocn (point of live metadata) and go
	 * backwards.
	 */
	prev_sector = _get_prev_sector_circular(area_start, area_size,
					       latest_mrec_offset);
	offset = prev_sector;
	size = SECTOR_SIZE;
	offset2 = size2 = 0;

	while (prev_sector != latest_mrec_offset) {
		prev_sector2 = prev_sector;
		prev_sector = _get_prev_sector_circular(area_start, area_size,
							prev_sector);
		if (prev_sector > prev_sector2)
			goto_out;
		/*
		 * FIXME: for some reason, the whole metadata region from
		 * area->start to area->start+area->size is not used.
		 * Only ~32KB seems to contain valid metadata records
		 * (LVM2 format - format_text).  As a result, I end up with
		 * "dm_config_maybe_section" returning true when there's no valid
		 * metadata in a sector (sectors with all nulls).
		 */
		if (!(buf = malloc(size + size2)))
			goto_out;

		if (!dev_read_bytes(area->dev, offset, size, buf)) {
			log_error("Failed to read dev %s offset %llu size %llu",
				  dev_name(area->dev),
				  (unsigned long long)offset,
				  (unsigned long long)size);
			goto out;
		}

		if (size2) {
			if (!dev_read_bytes(area->dev, offset2, size2, buf + size)) {
				log_error("Failed to read dev %s offset %llu size %llu",
				  	  dev_name(area->dev),
					  (unsigned long long)offset2,
				          (unsigned long long)size2);
				goto out;
			}
		}

		/*
		 * FIXME: We could add more sophisticated metadata detection
		 */
		if (dm_config_maybe_section(buf, size + size2)) {
			/* FIXME: Validate region, pull out timestamp?, etc */
			/* FIXME: Do something with this region */
			log_verbose ("Found LVM2 metadata record at "
				     "offset=" FMTu64 ", size=" FMTsize_t ", "
				     "offset2=" FMTu64 " size2=" FMTsize_t,
				     offset, size, offset2, size2);
			offset = prev_sector;
			size = SECTOR_SIZE;
			offset2 = size2 = 0;
		} else {
			/*
			 * Not a complete metadata record, assume we have
			 * metadata and just increase the size and offset.
			 * Start the second region if the previous sector is
			 * wrapping around towards the end of the disk.
			 */
			if (prev_sector > offset) {
				offset2 = prev_sector;
				size2 += SECTOR_SIZE;
			} else {
				offset = prev_sector;
				size += SECTOR_SIZE;
			}
		}
		free(buf);
		buf = NULL;
	}

	r = 1;
 out:
	free(buf);
	return r;
}



static int _text_lv_setup(struct format_instance *fid __attribute__((unused)),
			  struct logical_volume *lv)
{
/******** FIXME Any LV size restriction?
	uint64_t max_size = UINT_MAX;

	if (lv->size > max_size) {
		char *dummy = display_size(max_size);
		log_error("logical volumes cannot be larger than %s", dummy);
		free(dummy);
		return 0;
	}
*/

	if (!*lv->lvid.s && !lvid_create(&lv->lvid, &lv->vg->id)) {
		log_error("Random lvid creation failed for %s/%s.",
			  lv->vg->name, lv->name);
		return 0;
	}

	return 1;
}

static void _xlate_mdah(struct mda_header *mdah)
{
	struct raw_locn *rl;

	mdah->version = xlate32(mdah->version);
	mdah->start = xlate64(mdah->start);
	mdah->size = xlate64(mdah->size);

	rl = &mdah->raw_locns[0];
	while (rl->offset) {
		rl->checksum = xlate32(rl->checksum);
		rl->offset = xlate64(rl->offset);
		rl->size = xlate64(rl->size);
		rl++;
	}
}

static int _raw_read_mda_header(struct mda_header *mdah, struct device_area *dev_area, int primary_mda)
{
	log_debug_metadata("Reading mda header sector from %s at %llu",
			   dev_name(dev_area->dev), (unsigned long long)dev_area->start);

	if (!dev_read_bytes(dev_area->dev, dev_area->start, MDA_HEADER_SIZE, mdah)) {
		log_error("Failed to read metadata area header on %s at %llu",
			  dev_name(dev_area->dev), (unsigned long long)dev_area->start);
		return 0;
	}

	if (mdah->checksum_xl != xlate32(calc_crc(INITIAL_CRC, (uint8_t *)mdah->magic,
						  MDA_HEADER_SIZE -
						  sizeof(mdah->checksum_xl)))) {
		log_error("Incorrect checksum in metadata area header on %s at %llu",
			  dev_name(dev_area->dev), (unsigned long long)dev_area->start);
		return 0;
	}

	_xlate_mdah(mdah);

	if (strncmp((char *)mdah->magic, FMTT_MAGIC, sizeof(mdah->magic))) {
		log_error("Wrong magic number in metadata area header on %s at %llu",
			  dev_name(dev_area->dev), (unsigned long long)dev_area->start);
		return 0;
	}

	if (mdah->version != FMTT_VERSION) {
		log_error("Incompatible version %u metadata area header on %s at %llu",
			  mdah->version,
			  dev_name(dev_area->dev), (unsigned long long)dev_area->start);
		return 0;
	}

	if (mdah->start != dev_area->start) {
		log_error("Incorrect start sector %llu in metadata area header on %s at %llu",
			  (unsigned long long)mdah->start,
			  dev_name(dev_area->dev), (unsigned long long)dev_area->start);
		return 0;
	}

	return 1;
}

struct mda_header *raw_read_mda_header(const struct format_type *fmt,
				       struct device_area *dev_area, int primary_mda)
{
	struct mda_header *mdah;

	if (!(mdah = dm_pool_alloc(fmt->cmd->mem, MDA_HEADER_SIZE))) {
		log_error("struct mda_header allocation failed");
		return NULL;
	}

	if (!_raw_read_mda_header(mdah, dev_area, primary_mda)) {
		dm_pool_free(fmt->cmd->mem, mdah);
		return NULL;
	}

	return mdah;
}

static int _raw_write_mda_header(const struct format_type *fmt,
				 struct device *dev, int primary_mda,
				 uint64_t start_byte, struct mda_header *mdah)
{
	strncpy((char *)mdah->magic, FMTT_MAGIC, sizeof(mdah->magic));
	mdah->version = FMTT_VERSION;
	mdah->start = start_byte;

	_xlate_mdah(mdah);
	mdah->checksum_xl = xlate32(calc_crc(INITIAL_CRC, (uint8_t *)mdah->magic,
					     MDA_HEADER_SIZE -
					     sizeof(mdah->checksum_xl)));

	dev_set_last_byte(dev, start_byte + MDA_HEADER_SIZE);

	if (!dev_write_bytes(dev, start_byte, MDA_HEADER_SIZE, mdah)) {
		dev_unset_last_byte(dev);
		log_error("Failed to write mda header to %s fd %d", dev_name(dev), dev->bcache_fd);
		return 0;
	}
	dev_unset_last_byte(dev);

	return 1;
}

/*
 * FIXME: unify this with read_metadata_location() which is used
 * in the label scanning path.
 */

static struct raw_locn *_read_metadata_location_vg(struct device_area *dev_area,
				       struct mda_header *mdah, int primary_mda,
				       const char *vgname,
				       int *precommitted)
{
	size_t len;
	char vgnamebuf[NAME_LEN + 2] __attribute__((aligned(8)));
	struct raw_locn *rlocn, *rlocn_precommitted;
	struct lvmcache_info *info;
	struct lvmcache_vgsummary vgsummary_orphan = {
		.vgname = FMT_TEXT_ORPHAN_VG_NAME,
	};
	int rlocn_was_ignored;

	memcpy(&vgsummary_orphan.vgid, FMT_TEXT_ORPHAN_VG_NAME, sizeof(FMT_TEXT_ORPHAN_VG_NAME));

	rlocn = mdah->raw_locns;	/* Slot 0 */
	rlocn_precommitted = rlocn + 1;	/* Slot 1 */

	rlocn_was_ignored = rlocn_is_ignored(rlocn);

	/* Should we use precommitted metadata? */
	if (*precommitted && rlocn_precommitted->size &&
	    (rlocn_precommitted->offset != rlocn->offset)) {
		rlocn = rlocn_precommitted;
		log_debug_metadata("VG %s metadata check %s mda %llu slot1 offset %llu size %llu",
				   vgname ?: "",
				   dev_name(dev_area->dev),
				   (unsigned long long)dev_area->start,
				   (unsigned long long)rlocn->offset,
				   (unsigned long long)rlocn->size);
	} else {
		*precommitted = 0;
		log_debug_metadata("VG %s metadata check %s mda %llu slot0 offset %llu size %llu",
				   vgname ?: "",
				   dev_name(dev_area->dev),
				   (unsigned long long)dev_area->start,
				   (unsigned long long)rlocn->offset,
				   (unsigned long long)rlocn->size);
	}

	/* Do not check non-existent metadata. */
	if (!rlocn->offset && !rlocn->size)
		return NULL;

	/*
	 * Don't try to check existing metadata
	 * if given vgname is an empty string.
	 */
	if (!*vgname)
		return rlocn;

	/*
	 * If live rlocn has ignored flag, data will be out-of-date so skip further checks.
	 */
	if (rlocn_was_ignored)
		return rlocn;

	/*
	 * Verify that the VG metadata pointed to by the rlocn
	 * begins with a valid vgname.
	 */
	memset(vgnamebuf, 0, sizeof(vgnamebuf));

	if (!dev_read_bytes(dev_area->dev, dev_area->start + rlocn->offset, NAME_LEN, vgnamebuf))
		goto fail;

	if (!strncmp(vgnamebuf, vgname, len = strlen(vgname)) &&
	    (isspace(vgnamebuf[len]) || vgnamebuf[len] == '{'))
		return rlocn;
 fail:
	log_error("Metadata on %s at %llu has wrong VG name \"%s\" expected %s.",
		  dev_name(dev_area->dev),
		  (unsigned long long)(dev_area->start + rlocn->offset),
		  vgnamebuf, vgname);

	if ((info = lvmcache_info_from_pvid(dev_area->dev->pvid, dev_area->dev, 0)) &&
	    !lvmcache_update_vgname_and_id(info, &vgsummary_orphan))
		stack;

	return NULL;
}

/*
 * Determine offset for new metadata
 *
 * FIXME: The rounding can have a negative effect: when the current metadata
 * text size is just below the max, a command to remove something, that
 * *reduces* the text metadata size, can still be rejected for being too large,
 * even though it's smaller than the current size.  In this case, the user
 * would need to find something in the VG to remove that uses more text space
 * to compensate for the increase due to rounding.
 */

static uint64_t _next_rlocn_offset(struct raw_locn *rlocn_old, uint64_t old_last, struct mda_header *mdah, uint64_t mdac_area_start, uint64_t alignment)
{
	uint64_t next_start;
	uint64_t new_start;
	uint64_t adjust;

	/*
	 * No metadata has been written yet, begin at MDA_HEADER_SIZE offset
	 * from the start of the area.
	 */
	if (!rlocn_old)
		return MDA_HEADER_SIZE;

	/*
	 * If new start would be less than alignment bytes from the end of the
	 * metadata area, then start at beginning.
	 */
	if (mdah->size - old_last < alignment) {
		log_debug_metadata("new metadata offset adjusted from %llu to beginning %u",
				   (unsigned long long)(old_last + 1), MDA_HEADER_SIZE);
		return MDA_HEADER_SIZE;
	}

	/*
	 * New metadata begins after the old, rounded up to alignment.
	 */

	next_start = old_last + 1;

	adjust = alignment - (next_start % alignment);

	new_start = next_start + adjust;

	log_debug_metadata("new metadata offset adjusted from %llu to %llu (+%llu) for alignment %llu",
			   (unsigned long long)next_start,
			   (unsigned long long)new_start,
			   (unsigned long long)adjust,
			   (unsigned long long)alignment);

	/*
	 * If new_start is beyond the end of the metadata area or within
	 * alignment bytes of the end, then start at the beginning.
	 */
	if (new_start > mdah->size - alignment) {
		log_debug_metadata("new metadata offset adjusted from %llu to beginning %u",
				   (unsigned long long)new_start, MDA_HEADER_SIZE);
		return MDA_HEADER_SIZE;
	}

	return new_start;
}

static struct volume_group *_vg_read_raw_area(struct format_instance *fid,
					      const char *vgname,
					      struct device_area *area,
					      struct cached_vg_fmtdata **vg_fmtdata,
					      unsigned *use_previous_vg,
					      int precommitted,
					      int primary_mda)
{
	struct volume_group *vg = NULL;
	struct raw_locn *rlocn;
	struct mda_header *mdah;
	time_t when;
	char *desc;
	uint32_t wrap = 0;

	if (!(mdah = raw_read_mda_header(fid->fmt, area, primary_mda))) {
		log_error("Failed to read vg %s from %s", vgname, dev_name(area->dev));
		goto_out;
	}

	if (!(rlocn = _read_metadata_location_vg(area, mdah, primary_mda, vgname, &precommitted))) {
		log_debug_metadata("VG %s not found on %s", vgname, dev_name(area->dev));
		goto out;
	}

	if (rlocn->offset + rlocn->size > mdah->size)
		wrap = (uint32_t) ((rlocn->offset + rlocn->size) - mdah->size);

	vg = text_read_metadata(fid, NULL, vg_fmtdata, use_previous_vg, area->dev, primary_mda,
				(off_t) (area->start + rlocn->offset),
				(uint32_t) (rlocn->size - wrap),
				(off_t) (area->start + MDA_HEADER_SIZE),
				wrap,
				calc_crc,
				rlocn->checksum,
				&when, &desc);

	if (!vg) {
		/* FIXME: detect and handle errors, and distinguish from the optimization
		   that skips parsing the metadata which also returns NULL. */
	}

	log_debug_metadata("Found metadata on %s at %llu size %llu for VG %s",
			   dev_name(area->dev),
			   (unsigned long long)(area->start + rlocn->offset),
			   (unsigned long long)rlocn->size,
			   vgname);

	if (vg && precommitted)
		vg->status |= PRECOMMITTED;

      out:
	return vg;
}

static struct volume_group *_vg_read_raw(struct format_instance *fid,
					 const char *vgname,
					 struct metadata_area *mda,
					 struct cached_vg_fmtdata **vg_fmtdata,
					 unsigned *use_previous_vg)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct volume_group *vg;

	vg = _vg_read_raw_area(fid, vgname, &mdac->area, vg_fmtdata, use_previous_vg, 0, mda_is_primary(mda));

	return vg;
}

static struct volume_group *_vg_read_precommit_raw(struct format_instance *fid,
						   const char *vgname,
						   struct metadata_area *mda,
						   struct cached_vg_fmtdata **vg_fmtdata,
						   unsigned *use_previous_vg)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct volume_group *vg;

	vg = _vg_read_raw_area(fid, vgname, &mdac->area, vg_fmtdata, use_previous_vg, 1, mda_is_primary(mda));

	return vg;
}

/*
 * VG metadata updates:
 *
 * [mda_header] [raw_locn_0] [raw_locn_1] [text metadata circular buffer]
 *
 * raw_locn.offset points into the metadata circular buffer to the
 * start of metadata.
 *
 * When vg_read wants to read metadata from disk, it looks at the
 * raw_locn_0 offset and reads the text metadata from that location
 * in the circular buffer.
 *
 * Two full copies of the text metadata always exist in the circular
 * buffer.  When new metadata needs to be written, the following
 * process is followed:
 *
 * - vg_write is called and writes the new text metadata into the
 *   circular buffer after the end of the current copy.  vg_write saves
 *   an in-memory raw_locn struct (mdac->rlocn) pointing to the new
 *   metadata in the buffer.  No raw_locn structs are written to disk.
 *
 * - vg_precommit is called and writes the in-memory raw_locn struct that
 *   was saved by vg_write into raw_locn_1 (slot 1, the "precommit" slot.)
 *   raw_locn_0 still points to the old metadata, and raw_locn_1 points
 *   to the new metadata.
 *
 * - vg_commit is called and writes the new raw_locn struct into raw_locn_0
 *   (slot 0, the "committed" slot).
 */

/*
 * Writes new text metadata into the circular metadata buffer following the
 * current (old) text metadata that's already in the metadata buffer.
 *
 * vg_write does *not* write new raw_locn fields pointing to the new metadata.
 * The new raw_locn fields for the new metadata are saved in mdac->rlocn and
 * are written later by both vg_precommit and vg_commit.  vg_precommit will
 * write the new raw_locn into slot 1 and vg_commit will write the new raw_locn
 * into slot 0.
 */

static int _vg_write_raw(struct format_instance *fid, struct volume_group *vg,
			 struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct text_fid_context *fidtc = (struct text_fid_context *) fid->private;
	struct raw_locn *rlocn_old;
	struct raw_locn *rlocn_new;
	struct mda_header *mdah;
	struct pv_list *pvl;
	uint64_t old_start = 0, old_last = 0, old_size = 0, old_wrap = 0;
	uint64_t new_start = 0, new_last = 0, new_size = 0, new_wrap = 0;
	uint64_t max_size;
	char *new_buf = NULL;
	int overlap;
	int found = 0;
	int r = 0;

	/* Ignore any mda on a PV outside the VG. vgsplit relies on this */
	dm_list_iterate_items(pvl, &vg->pvs) {
		if (pvl->pv->dev == mdac->area.dev) {
			found = 1;
			break;
		}
	}
	if (!found)
		return 1;

	if (!(mdah = raw_read_mda_header(fid->fmt, &mdac->area, mda_is_primary(mda))))
		goto_out;

	/*
	 * Create a text metadata representation of struct vg in buffer.
	 * This buffer is written to disk below.  This function is called
	 * to write metadata to each device/mda in the VG.  The first time
	 * the metadata text is saved in raw_metadata_buf and subsequent
	 * mdas use that.
	 */
	if (fidtc->raw_metadata_buf) {
		new_buf = fidtc->raw_metadata_buf;
		new_size = fidtc->raw_metadata_buf_size;
	} else {
		new_size = text_vg_export_raw(vg, "", &new_buf);
		fidtc->raw_metadata_buf = new_buf;
		fidtc->raw_metadata_buf_size = new_size;
	}

	if (!new_size || !new_buf) {
		log_error("VG %s metadata writing failed", vg->name);
		goto out;
	}

	/*
	 * The max size of a single copy of text metadata.
	 *
	 * The space available for all text metadata is the size of the
	 * metadata area (mdah->size) minus the sector used for the header.
	 * Two copies of the text metadata must fit in this space, so it is
	 * divided in two.  This result is then reduced by 512 because any
	 * single copy of metadata is rounded to begin on a sector boundary.
	 */

	max_size = ((mdah->size - MDA_HEADER_SIZE) / 2) - 512;

	if (new_size > max_size) {
		log_error("VG %s metadata on %s (%llu bytes) exceeds maximum metadata size (%llu bytes)",
			  vg->name,
			  dev_name(mdac->area.dev),
			  (unsigned long long)new_size,
			  (unsigned long long)max_size);
		goto out;
	}

	/*
	 * rlocn_old is the current, committed, raw_locn data in slot0 on disk.
	 *
	 * rlocn_new (mdac->rlocn) is the new, in-memory, raw_locn data for the
	 * new metadata.  It is in-memory only, not yet written to disk.
	 *
	 * rlocn_new is not written to disk by vg_write.  vg_write only writes
	 * the new text metadata into the circular buffer, it does not update any
	 * raw_locn slot to point to that new metadata.  vg_write saves raw_locn
	 * values for the new metadata in memory at mdac->rlocn so that
	 * vg_precommit and vg_commit can find it later and write it to disk.
	 *
	 * rlocn/raw_locn values, old_start, old_last, old_size, new_start,
	 * new_last, new_size, are all in bytes.
	 *
	 * The start and last values are the first and last bytes that hold
	 * the metadata inclusively, e.g.
	 * metadata_v1 start = 512, last = 611, size = 100
	 * metadata_v2 start = 612, last = 711, size = 100
	 *
	 * {old,new}_{start,last} values are all offset values from the
	 * beginning of the metadata area mdac->area.start.  At the beginning
	 * of the metadata area (area.start), the first 512 bytes
	 * (MDA_HEADER_SIZE) is reserved for the mda_header/raw_locn structs,
	 * after which the circular buffer of text metadata begins.
	 * So, the when the text metadata wraps around, it starts again at
	 * area.start + MDA_HEADER_SIZE.
	 */

	rlocn_old = &mdah->raw_locns[0];  /* slot0, committed metadata */

	if (rlocn_is_ignored(rlocn_old))
		rlocn_old = NULL;

	else if (!rlocn_old->offset && !rlocn_old->size)
		rlocn_old = NULL;

	else {
		old_start = rlocn_old->offset;
		old_size = rlocn_old->size;

		if (rlocn_old->offset + rlocn_old->size > mdah->size) {
			old_wrap = (old_start + old_size) - mdah->size;
			old_last = old_wrap + MDA_HEADER_SIZE - 1;
		} else {
			old_wrap = 0;
			old_last = old_start + old_size - 1;
		}
	}

	/*
	 * _next_rlocn_offset returns the new offset to use for the new
	 * metadata.  It is set to follow the end of the old metadata, plus
	 * some adjustment to start the new metadata on a 512 byte alignment.
	 * If the new metadata would start beyond the end of the metadata area,
	 * or would start less than 512 bytes before the end of the metadata
	 * area, then the new start is set back at the beginning
	 * (metadata begins MDA_HEADER_SIZE after start of metadata area).
	 */
	new_start = _next_rlocn_offset(rlocn_old, old_last, mdah, mdac->area.start, MDA_ORIGINAL_ALIGNMENT);

	if (new_start + new_size > mdah->size) {
		new_wrap = (new_start + new_size) - mdah->size;
		new_last = new_wrap + MDA_HEADER_SIZE - 1;
	} else {
		new_wrap = 0;
		new_last = new_start + new_size - 1;
	}

	/*
	 * Save the new metadata location in memory for vg_precommit and
	 * vg_commit.  The new location is not written to disk here.
	 */
	rlocn_new = &mdac->rlocn;
	rlocn_new->offset = new_start;
	rlocn_new->size = new_size;

	log_debug_metadata("VG %s metadata offsets: old start %llu last %llu size %llu wrap %llu",
			   vg->name,
			   (unsigned long long)old_start,
			   (unsigned long long)old_last,
			   (unsigned long long)old_size,
			   (unsigned long long)old_wrap);

	log_debug_metadata("VG %s metadata offsets: new start %llu last %llu size %llu wrap %llu",
			   vg->name,
			   (unsigned long long)new_start,
			   (unsigned long long)new_last,
			   (unsigned long long)new_size,
			   (unsigned long long)new_wrap);


	/*
	 * If the new copy of the metadata would overlap the old copy of the
	 * metadata, it means that the circular metadata buffer is full.
	 *
	 * Given the max_size restriction above, two copies of metadata should
	 * never overlap, so these overlap checks should not be technically
	 * necessary, and a failure should not occur here.  It's left as a
	 * sanity check.  For some unknown time, lvm did not enforce a
	 * max_size, but rather detected the too-large failure by checking for
	 * overlap between old and new.
	 */

	if (new_wrap && old_wrap) {

		/* old and new can't both wrap without overlapping */
		overlap = 1;

	} else if (!new_wrap && !old_wrap &&
		(new_start > old_last) && (new_last > new_start)) {

		/* new metadata is located entirely after the old metadata */
		overlap = 0;

	} else if (!new_wrap && !old_wrap &&
		(new_start < old_start) && (new_last < old_start)) {

		/* new metadata is located entirely before the old metadata */
		overlap = 0;

	} else if (old_wrap && !new_wrap &&
		(old_last < new_start) && (new_start < new_last) && (new_last < old_start)) {

		/* when old wraps and the new doesn't, then no overlap is:
		   old_last followed by new_start followed by new_last
		   followed by old_start */
		overlap = 0;

	} else if (new_wrap && !old_wrap &&
		(new_last < old_start) && (old_start < old_last) && (old_last < new_start)) {

		/* when new wraps and the old doesn't, then no overlap is:
		   new_last followed by old_start followed by old_last
		   followed by new_start. */
		overlap = 0;

	} else {
		overlap = 1;
	}

	if (overlap) {
		log_error("VG %s metadata on %s (%llu bytes) too large for circular buffer (%llu bytes with %llu used)",
			  vg->name,
			  dev_name(mdac->area.dev),
			  (unsigned long long)new_size,
			  (unsigned long long)(mdah->size - MDA_HEADER_SIZE),
			  (unsigned long long)old_size);
		goto out;
	}

	log_debug_metadata("VG %s metadata write to %s at %llu len %llu (wrap %llu)", 
			    vg->name, dev_name(mdac->area.dev),
			    (unsigned long long)(mdac->area.start + rlocn_new->offset),
			    (unsigned long long)(rlocn_new->size - new_wrap),
			    (unsigned long long)new_wrap);

	dev_set_last_byte(mdac->area.dev, mdac->area.start + mdah->size);

	if (!dev_write_bytes(mdac->area.dev, mdac->area.start + rlocn_new->offset,
		                (size_t) (rlocn_new->size - new_wrap), new_buf)) {
		log_error("Failed to write metadata to %s fd %d", dev_name(mdac->area.dev), mdac->area.dev->bcache_fd);
		dev_unset_last_byte(mdac->area.dev);
		goto out;
	}

	if (new_wrap) {
		log_debug_metadata("VG %s metadata write to %s at %llu len %llu (wrapped)",
				   vg->name, dev_name(mdac->area.dev),
				   (unsigned long long)(mdac->area.start + MDA_HEADER_SIZE),
				   (unsigned long long)new_wrap);

		if (!dev_write_bytes(mdac->area.dev, mdac->area.start + MDA_HEADER_SIZE,
			                (size_t) new_wrap, new_buf + rlocn_new->size - new_wrap)) {
			log_error("Failed to write metadata wrap to %s fd %d", dev_name(mdac->area.dev), mdac->area.dev->bcache_fd);
			dev_unset_last_byte(mdac->area.dev);
			goto out;
		}
	}

	dev_unset_last_byte(mdac->area.dev);

	rlocn_new->checksum = calc_crc(INITIAL_CRC,
				       (uint8_t *)new_buf,
				       (uint32_t)(rlocn_new->size - new_wrap));
	if (new_wrap)
		rlocn_new->checksum = calc_crc(rlocn_new->checksum,
					(uint8_t *)new_buf + rlocn_new->size - new_wrap,
					(uint32_t)new_wrap);

	r = 1;

      out:
	if (!r) {
		free(fidtc->raw_metadata_buf);
		fidtc->raw_metadata_buf = NULL;
		fidtc->raw_metadata_buf_size = 0;
	}

	return r;
}

/*
 * Writes new raw_locn to disk that was saved by vg_write_raw (in mdac->rlocn).
 * The new raw_locn points to the new metadata that was written by vg_write_raw.
 *
 * After vg_write writes the new text metadata into the circular buffer,
 * vg_precommit writes the new raw_locn (pointing to the new metadata)
 * into slot1 (raw_locns[1]).  Then vg_commit writes the same raw_locn
 * values again, but into slot0 (raw_locns[0]).  slot0 is the committed
 * slot, and once slot0 is written, subsequent vg_reads will see the new
 * metadata.
 */

static int _vg_commit_raw_rlocn(struct format_instance *fid,
				struct volume_group *vg,
				struct metadata_area *mda,
				int precommit)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct text_fid_context *fidtc = (struct text_fid_context *) fid->private;
	struct mda_header *mdab;
	struct raw_locn *rlocn_slot0;
	struct raw_locn *rlocn_slot1;
	struct raw_locn *rlocn_new;
	struct pv_list *pvl;
	int r = 0;
	int found = 0;

	/* Ignore any mda on a PV outside the VG. vgsplit relies on this */
	dm_list_iterate_items(pvl, &vg->pvs) {
		if (pvl->pv->dev == mdac->area.dev) {
			found = 1;
			break;
		}
	}
	if (!found)
		return 1;

	/*
	 * Data is read into the mdab buffer, the mdab buffer is then modified
	 * with new raw_locn values, then the mdab buffer is written.  Note
	 * this is different than _vg_write_raw, where data is read into the
	 * mdah buffer, but the mdah buffer is not modified and mdac->rlocn is
	 * modified.
	 */
	if (!(mdab = raw_read_mda_header(fid->fmt, &mdac->area, mda_is_primary(mda))))
		goto_out;

	/*
	 * rlocn_slot0/rlocn_slot1 point into mdab which is the buffer that
	 * will be modified and written.
	 */
	rlocn_slot0 = &mdab->raw_locns[0];
	rlocn_slot1 = &mdab->raw_locns[1];

	if (rlocn_is_ignored(rlocn_slot0) || (!rlocn_slot0->offset && !rlocn_slot0->size)) {
		rlocn_slot0->offset = 0;
		rlocn_slot0->size = 0;
		rlocn_slot0->checksum = 0;
		rlocn_slot1->offset = 0;
		rlocn_slot1->size = 0;
		rlocn_slot1->checksum = 0;
	}

	/*
	 * mdac->rlocn is the in-memory copy of the new metadata's location on
	 * disk.  mdac->rlocn was saved by vg_write after it wrote the new text
	 * metadata to disk.  This location of the new metadata is now written
	 * to disk by vg_precommit and vg_commit.  vg_precommit writes the new
	 * location into the precommit slot (slot1 / raw_locns[1]) and
	 * vg_commit writes the new location into committed slot (slot0 /
	 * raw_locns[0]).
	 *
	 * vg_revert sets the size of the im-memory mdac->rlocn to 0 and calls
	 * this function to clear the precommit slot.
	 */

	rlocn_new = &mdac->rlocn;

	if (!rlocn_new->size) {
		/*
		 * When there is no new metadata, the precommit slot is
		 * cleared and the committed slot is left alone. (see revert)
		 */
		rlocn_slot1->offset   = 0;
		rlocn_slot1->size     = 0;
		rlocn_slot1->checksum = 0;

	} else if (precommit) {
		/*
		 * vg_precommit writes the new raw_locn into slot 1,
		 * and keeps the existing committed raw_locn in slot 0.
		 */
		rlocn_slot1->offset   = rlocn_new->offset;
		rlocn_slot1->size     = rlocn_new->size;
		rlocn_slot1->checksum = rlocn_new->checksum;
	} else {
		/*
		 * vg_commit writes the new raw_locn into slot 0,
		 * and zeros the precommitted values in slot 1.
		 */
		rlocn_slot0->offset   = rlocn_new->offset;
		rlocn_slot0->size     = rlocn_new->size;
		rlocn_slot0->checksum = rlocn_new->checksum;

		rlocn_slot1->offset   = 0;
		rlocn_slot1->size     = 0;
		rlocn_slot1->checksum = 0;
	}

	rlocn_set_ignored(rlocn_slot0, mda_is_ignored(mda));

	if (mdac->rlocn.size) {
		if (precommit) {
			log_debug_metadata("VG %s metadata precommit seq %u on %s mda header at %llu %s",
					   vg->name, vg->seqno, dev_name(mdac->area.dev),
					   (unsigned long long)mdac->area.start,
					   mda_is_ignored(mda) ? "(ignored)" : "(used)");

			log_debug_metadata("VG %s metadata precommit slot0 offset %llu size %llu slot1 offset %llu size %llu",
					   vg->name,
					   (unsigned long long)mdab->raw_locns[0].offset,
					   (unsigned long long)mdab->raw_locns[0].size,
					   (unsigned long long)mdab->raw_locns[1].offset,
					   (unsigned long long)mdab->raw_locns[1].size);

		} else {
			log_debug_metadata("VG %s metadata commit seq %u on %s mda header at %llu %s",
					   vg->name, vg->seqno, dev_name(mdac->area.dev),
					   (unsigned long long)mdac->area.start,
					   mda_is_ignored(mda) ? "(ignored)" : "(used)");

			log_debug_metadata("VG %s metadata commit slot0 offset %llu size %llu slot1 offset %llu size %llu",
					   vg->name,
					   (unsigned long long)mdab->raw_locns[0].offset,
					   (unsigned long long)mdab->raw_locns[0].size,
					   (unsigned long long)mdab->raw_locns[1].offset,
					   (unsigned long long)mdab->raw_locns[1].size);
		}
	} else {
		if (precommit) {
			log_debug_metadata("VG %s metadata precommit empty seq %u on %s mda header at %llu %s",
					   vg->name, vg->seqno, dev_name(mdac->area.dev),
					   (unsigned long long)mdac->area.start,
					   mda_is_ignored(mda) ? "(ignored)" : "(used)");

			log_debug_metadata("VG %s metadata precommit empty slot0 offset %llu size %llu slot1 offset %llu size %llu",
					   vg->name,
					   (unsigned long long)mdab->raw_locns[0].offset,
					   (unsigned long long)mdab->raw_locns[0].size,
					   (unsigned long long)mdab->raw_locns[1].offset,
					   (unsigned long long)mdab->raw_locns[1].size);

		} else {
			log_debug_metadata("VG %s metadata commit empty seq %u on %s mda header at %llu %s",
					   vg->name, vg->seqno, dev_name(mdac->area.dev),
					   (unsigned long long)mdac->area.start,
					   mda_is_ignored(mda) ? "(ignored)" : "(used)");

			log_debug_metadata("VG %s metadata commit empty slot0 offset %llu size %llu slot1 offset %llu size %llu",
					   vg->name,
					   (unsigned long long)mdab->raw_locns[0].offset,
					   (unsigned long long)mdab->raw_locns[0].size,
					   (unsigned long long)mdab->raw_locns[1].offset,
					   (unsigned long long)mdab->raw_locns[1].size);
		}
	}

	rlocn_set_ignored(mdab->raw_locns, mda_is_ignored(mda));

	if (!_raw_write_mda_header(fid->fmt, mdac->area.dev, mda_is_primary(mda), mdac->area.start,
				   mdab)) {
		dm_pool_free(fid->fmt->cmd->mem, mdab);
		log_error("Failed to write metadata area header");
		goto out;
	}

	r = 1;

      out:
	if (!precommit) {
		free(fidtc->raw_metadata_buf);
		fidtc->raw_metadata_buf = NULL;
	}

	return r;
}

static int _vg_commit_raw(struct format_instance *fid, struct volume_group *vg,
			  struct metadata_area *mda)
{
	return _vg_commit_raw_rlocn(fid, vg, mda, 0);
}

static int _vg_precommit_raw(struct format_instance *fid,
			     struct volume_group *vg,
			     struct metadata_area *mda)
{
	return _vg_commit_raw_rlocn(fid, vg, mda, 1);
}

/* Close metadata area devices */
static int _vg_revert_raw(struct format_instance *fid, struct volume_group *vg,
			  struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct pv_list *pvl;
	int found = 0;

	/* Ignore any mda on a PV outside the VG. vgsplit relies on this */
	dm_list_iterate_items(pvl, &vg->pvs) {
		if (pvl->pv->dev == mdac->area.dev) {
			found = 1;
			break;
		}
	}

	if (!found)
		return 1;

	/* Wipe pre-committed metadata */
	mdac->rlocn.size = 0;
	return _vg_commit_raw_rlocn(fid, vg, mda, 0);
}

/*
 * vg_remove clears the two raw_locn slots but leaves the circular metadata
 * buffer alone.
 */

static int _vg_remove_raw(struct format_instance *fid, struct volume_group *vg,
			  struct metadata_area *mda)
{
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct mda_header *mdah;
	struct raw_locn *rlocn_slot0;
	struct raw_locn *rlocn_slot1;
	int r = 0;

	if (!(mdah = dm_pool_alloc(fid->fmt->cmd->mem, MDA_HEADER_SIZE))) {
		log_error("struct mda_header allocation failed");
		return 0;
	}

	/*
	 * FIXME: what's the point of reading the mda_header and metadata,
	 * since we zero the rlocn fields whether we can read them or not.
	 * Just to print the warning?
	 */

	if (!_raw_read_mda_header(mdah, &mdac->area, mda_is_primary(mda)))
		log_warn("WARNING: Removing metadata location on %s with bad mda header.",
			  dev_name(mdac->area.dev));

	rlocn_slot0 = &mdah->raw_locns[0];
	rlocn_slot1 = &mdah->raw_locns[1];

	rlocn_slot0->offset = 0;
	rlocn_slot0->size = 0;
	rlocn_slot0->checksum = 0;
	rlocn_set_ignored(rlocn_slot0, mda_is_ignored(mda));

	rlocn_slot1->offset = 0;
	rlocn_slot1->size = 0;
	rlocn_slot1->checksum = 0;

	if (!_raw_write_mda_header(fid->fmt, mdac->area.dev, mda_is_primary(mda), mdac->area.start,
				   mdah)) {
		dm_pool_free(fid->fmt->cmd->mem, mdah);
		log_error("Failed to write metadata area header");
		goto out;
	}

	r = 1;

      out:
	return r;
}

static struct volume_group *_vg_read_file_name(struct format_instance *fid,
					       const char *vgname,
					       const char *read_path)
{
	struct volume_group *vg;
	time_t when;
	char *desc;

	if (!(vg = text_read_metadata_file(fid, read_path, &when, &desc))) {
		log_error("Failed to read VG %s from %s", vgname, read_path);
		return NULL;
	}

	/*
	 * Currently you can only have a single volume group per
	 * text file (this restriction may remain).  We need to
	 * check that it contains the correct volume group.
	 */
	if (vgname && strcmp(vgname, vg->name)) {
		fid->ref_count++; /* Preserve FID after vg release */
		release_vg(vg);
		log_error("'%s' does not contain volume group '%s'.",
			  read_path, vgname);
		return NULL;
	}

	log_debug_metadata("Read volume group %s from %s", vg->name, read_path);

	return vg;
}

static struct volume_group *_vg_read_file(struct format_instance *fid,
					  const char *vgname,
					  struct metadata_area *mda,
					  struct cached_vg_fmtdata **vg_fmtdata,
					  unsigned *use_previous_vg __attribute__((unused)))
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;

	return _vg_read_file_name(fid, vgname, tc->path_live);
}

static struct volume_group *_vg_read_precommit_file(struct format_instance *fid,
						    const char *vgname,
						    struct metadata_area *mda,
						    struct cached_vg_fmtdata **vg_fmtdata,
						    unsigned *use_previous_vg __attribute__((unused)))
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;
	struct volume_group *vg;

	if ((vg = _vg_read_file_name(fid, vgname, tc->path_edit)))
		vg->status |= PRECOMMITTED;
	else
		vg = _vg_read_file_name(fid, vgname, tc->path_live);

	return vg;
}

static int _vg_write_file(struct format_instance *fid __attribute__((unused)),
			  struct volume_group *vg, struct metadata_area *mda)
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;

	FILE *fp;
	int fd;
	char *slash;
	char temp_file[PATH_MAX], temp_dir[PATH_MAX];

	slash = strrchr(tc->path_edit, '/');

	if (slash == 0)
		strcpy(temp_dir, ".");
	else if (slash - tc->path_edit < PATH_MAX) {
		(void) dm_strncpy(temp_dir, tc->path_edit,
				  (size_t) (slash - tc->path_edit + 1));
	} else {
		log_error("Text format failed to determine directory.");
		return 0;
	}

	if (!create_temp_name(temp_dir, temp_file, sizeof(temp_file), &fd,
			      &vg->cmd->rand_seed)) {
		log_error("Couldn't create temporary text file name.");
		return 0;
	}

	if (!(fp = fdopen(fd, "w"))) {
		log_sys_error("fdopen", temp_file);
		if (close(fd))
			log_sys_error("fclose", temp_file);
		return 0;
	}

	log_debug_metadata("Writing %s metadata to %s", vg->name, temp_file);

	if (!text_vg_export_file(vg, tc->desc, fp)) {
		log_error("Failed to write metadata to %s.", temp_file);
		if (fclose(fp))
			log_sys_error("fclose", temp_file);
		return 0;
	}

	if (fsync(fd) && (errno != EROFS) && (errno != EINVAL)) {
		log_sys_error("fsync", tc->path_edit);
		if (fclose(fp))
			log_sys_error("fclose", tc->path_edit);
		return 0;
	}

	if (lvm_fclose(fp, tc->path_edit))
		return_0;

	log_debug_metadata("Renaming %s to %s", temp_file, tc->path_edit);
	if (rename(temp_file, tc->path_edit)) {
		log_error("%s: rename to %s failed: %s", temp_file,
			  tc->path_edit, strerror(errno));
		return 0;
	}

	return 1;
}

static int _vg_commit_file_backup(struct format_instance *fid __attribute__((unused)),
				  struct volume_group *vg,
				  struct metadata_area *mda)
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;

	if (test_mode()) {
		log_verbose("Test mode: Skipping committing %s metadata (%u)",
			    vg->name, vg->seqno);
		if (unlink(tc->path_edit)) {
			log_debug_metadata("Unlinking %s", tc->path_edit);
			log_sys_error("unlink", tc->path_edit);
			return 0;
		}
	} else {
		log_debug_metadata("Committing file %s metadata (%u)", vg->name, vg->seqno);
		log_debug_metadata("Renaming %s to %s", tc->path_edit, tc->path_live);
		if (rename(tc->path_edit, tc->path_live)) {
			log_error("%s: rename to %s failed: %s", tc->path_edit,
				  tc->path_live, strerror(errno));
			return 0;
		}
	}

	sync_dir(tc->path_edit);

	return 1;
}

static int _vg_commit_file(struct format_instance *fid, struct volume_group *vg,
			   struct metadata_area *mda)
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;
	const char *slash;
	char new_name[PATH_MAX];
	size_t len;

	if (!_vg_commit_file_backup(fid, vg, mda))
		return 0;

	/* vgrename? */
	if ((slash = strrchr(tc->path_live, '/')))
		slash = slash + 1;
	else
		slash = tc->path_live;

	if (strcmp(slash, vg->name)) {
		len = slash - tc->path_live;
		if ((len + strlen(vg->name)) > (sizeof(new_name) - 1)) {
			log_error("Renaming path %s is too long for VG %s.",
				  tc->path_live, vg->name);
			return 0;
		}
		strncpy(new_name, tc->path_live, len);
		strcpy(new_name + len, vg->name);
		log_debug_metadata("Renaming %s to %s", tc->path_live, new_name);
		if (test_mode())
			log_verbose("Test mode: Skipping rename");
		else {
			if (rename(tc->path_live, new_name)) {
				log_error("%s: rename to %s failed: %s",
					  tc->path_live, new_name,
					  strerror(errno));
				sync_dir(new_name);
				return 0;
			}
		}
	}

	return 1;
}

static int _vg_remove_file(struct format_instance *fid __attribute__((unused)),
			   struct volume_group *vg __attribute__((unused)),
			   struct metadata_area *mda)
{
	struct text_context *tc = (struct text_context *) mda->metadata_locn;

	if (path_exists(tc->path_edit) && unlink(tc->path_edit)) {
		log_sys_error("unlink", tc->path_edit);
		return 0;
	}

	if (path_exists(tc->path_live) && unlink(tc->path_live)) {
		log_sys_error("unlink", tc->path_live);
		return 0;
	}

	sync_dir(tc->path_live);

	return 1;
}

int read_metadata_location_summary(const struct format_type *fmt,
		    struct mda_header *mdah, int primary_mda, struct device_area *dev_area,
		    struct lvmcache_vgsummary *vgsummary, uint64_t *mda_free_sectors)
{
	struct raw_locn *rlocn;
	uint32_t wrap = 0;
	unsigned int len = 0;
	char namebuf[NAME_LEN + 1] __attribute__((aligned(8)));
	uint64_t max_size;

	if (!mdah) {
		log_error(INTERNAL_ERROR "read_metadata_location_summary called with NULL pointer for mda_header");
		return 0;
	}

	/*
	 * For the case where the metadata area is unused, half is available.
	 */
	if (mda_free_sectors) {
		max_size = ((mdah->size - MDA_HEADER_SIZE) / 2) - 512;
		*mda_free_sectors = max_size >> SECTOR_SHIFT;
	}

	rlocn = mdah->raw_locns; /* slot0, committed metadata */

	/*
	 * If no valid offset, do not try to search for vgname
	 */
	if (!rlocn->offset) {
		log_debug_metadata("Metadata location on %s at %llu has offset 0.",
				   dev_name(dev_area->dev),
				   (unsigned long long)(dev_area->start + rlocn->offset));
		vgsummary->zero_offset = 1;
		return 0;
	}

	memset(namebuf, 0, sizeof(namebuf));

	if (!dev_read_bytes(dev_area->dev, dev_area->start + rlocn->offset, NAME_LEN, namebuf))
		stack;

	while (namebuf[len] && !isspace(namebuf[len]) && namebuf[len] != '{' &&
	       len < (NAME_LEN - 1))
		len++;

	namebuf[len] = '\0';

	/*
	 * Check that the text metadata in the circular buffer begins with a
	 * valid vg name.
	 */
	if (!validate_name(namebuf)) {
		log_error("Metadata location on %s at %llu begins with invalid VG name.",
			  dev_name(dev_area->dev),
			  (unsigned long long)(dev_area->start + rlocn->offset));
		return 0;
	}

	/*
	 * When the current metadata wraps around the end of the metadata area
	 * (so some is located at the end and some is located at the
	 * beginning), then "wrap" is the number of bytes that was written back
	 * at the beginning.  The end of this wrapped metadata is located at an
	 * offset of wrap+MDA_HEADER_SIZE from area.start.
	 */
	if (rlocn->offset + rlocn->size > mdah->size)
		wrap = (uint32_t) ((rlocn->offset + rlocn->size) - mdah->size);

	/*
	 * Did we see this metadata before?
	 * Look in lvmcache to see if there is vg info matching
	 * the checksum/size that we see in the mda_header (rlocn)
	 * on this device.  If so, then vgsummary->name is is set
	 * and controls if the "checksum_only" flag passed to
	 * text_read_metadata_summary() is 1 or 0.
	 *
	 * If checksum_only = 1, then text_read_metadata_summary()
	 * will read the metadata from this device, and run the
	 * checksum function on it.  If the calculated checksum
	 * of the metadata matches the checksum in the mda_header,
	 * which also matches the checksum saved in vginfo from
	 * another device, then it skips parsing the metadata into
	 * a config tree, which saves considerable cpu time.
	 *
	 * (NB. there can be different VGs with different metadata
	 * and checksums, but with the same name.)
	 *
	 * FIXME: handle the case where mda_header checksum is bad
	 * but metadata checksum is good.
	 */

	/*
	 * If the checksum we compute of the metadata differs from
	 * the checksum from mda_header that we save here, then we
	 * ignore the device.  FIXME: we need to classify a device
	 * with errors like this as defective.
	 *
	 * If the checksum from mda_header and computed from metadata
	 * does not match the checksum saved in lvmcache from a prev
	 * device, then we do not skip parsing/saving metadata from
	 * this dev.  It's parsed, fields saved in vgsummary, which
	 * is passed into lvmcache (update_vgname_and_id), and
	 * there we'll see a checksum mismatch.
	 */
	vgsummary->mda_checksum = rlocn->checksum;
	vgsummary->mda_size = rlocn->size;
	lvmcache_lookup_mda(vgsummary);

	if (!text_read_metadata_summary(fmt, dev_area->dev, MDA_CONTENT_REASON(primary_mda),
				(off_t) (dev_area->start + rlocn->offset),
				(uint32_t) (rlocn->size - wrap),
				(off_t) (dev_area->start + MDA_HEADER_SIZE),
				wrap, calc_crc, vgsummary->vgname ? 1 : 0,
				vgsummary)) {
		log_error("Metadata location on %s at %llu has invalid summary for VG.",
			  dev_name(dev_area->dev),
			  (unsigned long long)(dev_area->start + rlocn->offset));
		return 0;
	}

	/* Ignore this entry if the characters aren't permissible */
	if (!validate_name(vgsummary->vgname)) {
		log_error("Metadata location on %s at %llu has invalid VG name.",
			  dev_name(dev_area->dev),
			  (unsigned long long)(dev_area->start + rlocn->offset));
		return 0;
	}

	log_debug_metadata("Found metadata summary on %s at %llu size %llu for VG %s",
			   dev_name(dev_area->dev),
			   (unsigned long long)(dev_area->start + rlocn->offset),
			   (unsigned long long)rlocn->size,
			   vgsummary->vgname);

	if (mda_free_sectors) {
		/*
		 * Report remaining space given that a single copy of metadata
		 * can be as large as half the total metadata space, minus 512
		 * because each copy is rounded to begin on a sector boundary.
		 */
		max_size = ((mdah->size - MDA_HEADER_SIZE) / 2) - 512;

		if (rlocn->size >= max_size)
			*mda_free_sectors = UINT64_C(0);
		else
			*mda_free_sectors = (max_size - rlocn->size) >> SECTOR_SHIFT;
	}

	return 1;
}

struct _write_single_mda_baton {
	const struct format_type *fmt;
	struct physical_volume *pv;
};

static int _write_single_mda(struct metadata_area *mda, void *baton)
{
	struct _write_single_mda_baton *p = baton;
	struct mda_context *mdac;

	char buf[MDA_HEADER_SIZE] __attribute__((aligned(8))) = { 0 };
	struct mda_header *mdah = (struct mda_header *) buf;

	mdac = mda->metadata_locn;
	mdah->size = mdac->area.size;
	rlocn_set_ignored(mdah->raw_locns, mda_is_ignored(mda));

	if (!_raw_write_mda_header(p->fmt, mdac->area.dev, mda_is_primary(mda),
				   mdac->area.start, mdah)) {
		return_0;
	}
	return 1;
}

static int _set_ext_flags(struct physical_volume *pv, struct lvmcache_info *info)
{
	uint32_t ext_flags = lvmcache_ext_flags(info);

	if (is_orphan(pv))
		ext_flags &= ~PV_EXT_USED;
	else
		ext_flags |= PV_EXT_USED;

	lvmcache_set_ext_version(info, PV_HEADER_EXTENSION_VSN);
	lvmcache_set_ext_flags(info, ext_flags);

	return 1;
}

/* Only for orphans - FIXME That's not true any more */
static int _text_pv_write(const struct format_type *fmt, struct physical_volume *pv)
{
	struct format_instance *fid = pv->fid;
	const char *pvid = (const char *) (*pv->old_id.uuid ? &pv->old_id : &pv->id);
	struct label *label;
	struct lvmcache_info *info;
	struct mda_context *mdac;
	struct metadata_area *mda;
	struct _write_single_mda_baton baton;
	unsigned mda_index;

	/* Add a new cache entry with PV info or update existing one. */
	if (!(info = lvmcache_add(fmt->labeller, (const char *) &pv->id,
				  pv->dev, pv->vg_name,
				  is_orphan_vg(pv->vg_name) ? pv->vg_name : pv->vg ? (const char *) &pv->vg->id : NULL, 0)))
		return_0;

	label = lvmcache_get_label(info);
	label->sector = pv->label_sector;
	label->dev = pv->dev;

	lvmcache_update_pv(info, pv, fmt);

	/* Flush all cached metadata areas, we will reenter new/modified ones. */
	lvmcache_del_mdas(info);

	/*
	 * Add all new or modified metadata areas for this PV stored in
	 * its format instance. If this PV is not part of a VG yet,
	 * pv->fid will be used. Otherwise pv->vg->fid will be used.
	 * The fid_get_mda_indexed fn can handle that transparently,
	 * just pass the right format_instance in.
	 */
	for (mda_index = 0; mda_index < FMT_TEXT_MAX_MDAS_PER_PV; mda_index++) {
		if (!(mda = fid_get_mda_indexed(fid, pvid, ID_LEN, mda_index)))
			continue;

		mdac = (struct mda_context *) mda->metadata_locn;
		log_debug_metadata("Creating metadata area on %s at sector "
				   FMTu64 " size " FMTu64 " sectors",
				   dev_name(mdac->area.dev),
				   mdac->area.start >> SECTOR_SHIFT,
				   mdac->area.size >> SECTOR_SHIFT);

		// if fmt is not the same as info->fmt we are in trouble
		if (!lvmcache_add_mda(info, mdac->area.dev,
				      mdac->area.start, mdac->area.size,
				      mda_is_ignored(mda)))
			return_0;
	}

	if (!lvmcache_update_bas(info, pv))
		return_0;

	/*
	 * FIXME: Allow writing zero offset/size data area to disk.
	 *        This requires defining a special value since we can't
	 *        write offset/size that is 0/0 - this is already reserved
	 *        as a delimiter in data/metadata area area list in PV header
	 *        (needs exploring compatibility with older lvm2).
	 */

	/*
	 * We can't actually write pe_start = 0 (a data area offset)
	 * in PV header now. We need to replace this value here. This can
	 * happen with vgcfgrestore with redefined pe_start or
	 * pvcreate --restorefile. However, we can can have this value in
	 * metadata which will override the value in the PV header.
	 */

	if (!lvmcache_update_das(info, pv))
		return_0;

	baton.pv = pv;
	baton.fmt = fmt;

	if (!lvmcache_foreach_mda(info, _write_single_mda, &baton))
		return_0;

	if (!_set_ext_flags(pv, info))
		return_0;

	if (!label_write(pv->dev, label)) {
		stack;
		return 0;
	}

	/*
	 *  FIXME: We should probably use the format instance's metadata
	 *        areas for label_write and only if it's successful,
	 *        update the cache afterwards?
	 */

	return 1;
}

static int _text_pv_needs_rewrite(const struct format_type *fmt, struct physical_volume *pv,
				  int *needs_rewrite)
{
	struct lvmcache_info *info;
	uint32_t ext_vsn;

	*needs_rewrite = 0;

	if (!pv->is_labelled)
		return 1;

	if (!(info = lvmcache_info_from_pvid((const char *)&pv->id, pv->dev, 0))) {
		log_error("Failed to find cached info for PV %s.", pv_dev_name(pv));
		return 0;
	}

	ext_vsn = lvmcache_ext_version(info);

	if (ext_vsn < PV_HEADER_EXTENSION_VSN)
		*needs_rewrite = 1;

	return 1;
}

/*
 * Copy constructor for a metadata_locn.
 */
static void *_metadata_locn_copy_raw(struct dm_pool *mem, void *metadata_locn)
{
	struct mda_context *mdac, *mdac_new;

	mdac = (struct mda_context *) metadata_locn;
	if (!(mdac_new = dm_pool_alloc(mem, sizeof(*mdac_new)))) {
		log_error("mda_context allocation failed");
		return NULL;
	}
	memcpy(mdac_new, mdac, sizeof(*mdac));

	return mdac_new;
}

/*
 * Return a string description of the metadata location.
 */
static const char *_metadata_locn_name_raw(void *metadata_locn)
{
	struct mda_context *mdac = (struct mda_context *) metadata_locn;

	return dev_name(mdac->area.dev);
}

static uint64_t _metadata_locn_offset_raw(void *metadata_locn)
{
	struct mda_context *mdac = (struct mda_context *) metadata_locn;

	return mdac->area.start;
}

static int _text_pv_initialise(const struct format_type *fmt,
			       struct pv_create_args *pva,
			       struct physical_volume *pv)
{
	unsigned long data_alignment = pva->data_alignment;
	unsigned long data_alignment_offset = pva->data_alignment_offset;
	unsigned long adjustment, final_alignment = 0;

	if (!data_alignment)
		data_alignment = find_config_tree_int(pv->fmt->cmd, devices_data_alignment_CFG, NULL) * 2;

	if (set_pe_align(pv, data_alignment) != data_alignment &&
	    data_alignment) {
		log_error("%s: invalid data alignment of "
			  "%lu sectors (requested %lu sectors)",
			  pv_dev_name(pv), pv->pe_align, data_alignment);
		return 0;
	}

	if (set_pe_align_offset(pv, data_alignment_offset) != data_alignment_offset &&
	    data_alignment_offset) {
		log_error("%s: invalid data alignment offset of "
			  "%lu sectors (requested %lu sectors)",
			  pv_dev_name(pv), pv->pe_align_offset, data_alignment_offset);
		return 0;
	}

	if (pv->pe_align < pv->pe_align_offset) {
		log_error("%s: pe_align (%lu sectors) must not be less "
			  "than pe_align_offset (%lu sectors)",
			  pv_dev_name(pv), pv->pe_align, pv->pe_align_offset);
		return 0;
	}

	final_alignment = pv->pe_align + pv->pe_align_offset;

	if (pv->size < final_alignment) {
		log_error("%s: Data alignment must not exceed device size.",
			  pv_dev_name(pv));
		return 0;
	}

	if (pv->size < final_alignment + pva->ba_size) {
		log_error("%s: Bootloader area with data-aligned start must "
			  "not exceed device size.", pv_dev_name(pv));
		return 0;
	}

	if (pva->pe_start == PV_PE_START_CALC) {
		/*
		 * Calculate new PE start and bootloader area start value.
		 * Make sure both are properly aligned!
		 * If PE start can't be aligned because BA is taking
		 * the whole space, make PE start equal to the PV size
		 * which effectively disables DA - it will have zero size.
		 * This needs to be done as we can't have a PV without any DA.
		 * But we still want to support a PV with BA only!
		 */
		if (pva->ba_size) {
			pv->ba_start = final_alignment;
			pv->ba_size = pva->ba_size;
			if ((adjustment = pva->ba_size % pv->pe_align))
				pv->ba_size += pv->pe_align - adjustment;
			if (pv->size < pv->ba_start + pv->ba_size)
				pv->ba_size = pv->size - pv->ba_start;
			pv->pe_start = pv->ba_start + pv->ba_size;
		} else
			pv->pe_start = final_alignment;
	} else {
		/*
		 * Try to keep the value of PE start set to a firm value if
		 * requested. This is useful when restoring existing PE start
		 * value (e.g. backups). Also, if creating a BA, try to place
		 * it in between the final alignment and existing PE start
		 * if possible.
		 */
		pv->pe_start = pva->pe_start;
		if (pva->ba_size) {
			if ((pva->ba_start && pva->ba_start + pva->ba_size > pva->pe_start) ||
			    (pva->pe_start <= final_alignment) ||
			    (pva->pe_start - final_alignment < pva->ba_size)) {
				log_error("%s: Bootloader area would overlap "
					  "data area.", pv_dev_name(pv));
				return 0;
			}

			pv->ba_start = pva->ba_start ? : final_alignment;
			pv->ba_size = pva->ba_size;
		}
	}

	if (pva->extent_size)
		pv->pe_size = pva->extent_size;

	if (pva->extent_count)
		pv->pe_count = pva->extent_count;

	if ((pv->pe_start + pv->pe_count * (uint64_t)pv->pe_size - 1) > pv->size) {
		log_error("Physical extents end beyond end of device %s.",
			  pv_dev_name(pv));
		return 0;
	}

	if (pva->label_sector != -1)
                pv->label_sector = pva->label_sector;

	return 1;
}

static void _text_destroy_instance(struct format_instance *fid)
{
	if (--fid->ref_count <= 1) {
		if (fid->metadata_areas_index)
			dm_hash_destroy(fid->metadata_areas_index);
		dm_pool_destroy(fid->mem);
	}
}

static void _text_destroy(struct format_type *fmt)
{
	if (fmt->orphan_vg)
		free_orphan_vg(fmt->orphan_vg);

	if (fmt->private)
		free(fmt->private);

	free(fmt);
}

static struct metadata_area_ops _metadata_text_file_ops = {
	.vg_read = _vg_read_file,
	.vg_read_precommit = _vg_read_precommit_file,
	.vg_write = _vg_write_file,
	.vg_remove = _vg_remove_file,
	.vg_commit = _vg_commit_file
};

static struct metadata_area_ops _metadata_text_file_backup_ops = {
	.vg_read = _vg_read_file,
	.vg_write = _vg_write_file,
	.vg_remove = _vg_remove_file,
	.vg_commit = _vg_commit_file_backup
};

static struct metadata_area_ops _metadata_text_raw_ops = {
	.vg_read = _vg_read_raw,
	.vg_read_precommit = _vg_read_precommit_raw,
	.vg_write = _vg_write_raw,
	.vg_remove = _vg_remove_raw,
	.vg_precommit = _vg_precommit_raw,
	.vg_commit = _vg_commit_raw,
	.vg_revert = _vg_revert_raw,
	.mda_metadata_locn_copy = _metadata_locn_copy_raw,
	.mda_metadata_locn_name = _metadata_locn_name_raw,
	.mda_metadata_locn_offset = _metadata_locn_offset_raw,
	.mda_free_sectors = _mda_free_sectors_raw,
	.mda_total_sectors = _mda_total_sectors_raw,
	.mda_in_vg = _mda_in_vg_raw,
	.pv_analyze_mda = _pv_analyze_mda_raw,
	.mda_locns_match = _mda_locns_match_raw,
	.mda_get_device = _mda_get_device_raw,
};

static int _text_pv_setup(const struct format_type *fmt,
			  struct physical_volume *pv,
			  struct volume_group *vg)
{
	struct format_instance *fid = pv->fid;
	const char *pvid = (const char *) (*pv->old_id.uuid ? &pv->old_id : &pv->id);
	struct lvmcache_info *info;
	unsigned mda_index;
	struct metadata_area *pv_mda, *pv_mda_copy;
	struct mda_context *pv_mdac;
	uint64_t pe_count;
	uint64_t size_reduction = 0;

	/* If PV has its own format instance, add mdas from pv->fid to vg->fid. */
	if (pv->fid != vg->fid) {
		for (mda_index = 0; mda_index < FMT_TEXT_MAX_MDAS_PER_PV; mda_index++) {
			if (!(pv_mda = fid_get_mda_indexed(fid, pvid, ID_LEN, mda_index)))
				continue;

			/* Be sure it's not already in VG's format instance! */
			if (!fid_get_mda_indexed(vg->fid, pvid, ID_LEN, mda_index)) {
				if (!(pv_mda_copy = mda_copy(vg->fid->mem, pv_mda)))
					return_0;
				fid_add_mda(vg->fid, pv_mda_copy, pvid, ID_LEN, mda_index);
			}
		}
	}
	/*
	 * Otherwise, if the PV is already a part of the VG (pv->fid == vg->fid),
	 * reread PV mda information from the cache and add it to vg->fid.
	 */
	else {
		if (!pv->dev ||
		    !(info = lvmcache_info_from_pvid(pv->dev->pvid, pv->dev, 0))) {
			log_error("PV %s missing from cache", pv_dev_name(pv));
			return 0;
		}

		if (!lvmcache_check_format(info, fmt))
			return_0;

		if (!lvmcache_fid_add_mdas_pv(info, fid))
			return_0;
	}

	/* If there's the 2nd mda, we need to reduce
	 * usable size for further pe_count calculation! */
	if ((pv_mda = fid_get_mda_indexed(fid, pvid, ID_LEN, 1)) &&
	    (pv_mdac = pv_mda->metadata_locn))
		size_reduction = pv_mdac->area.size >> SECTOR_SHIFT;

	/* From now on, VG format instance will be used. */
	pv_set_fid(pv, vg->fid);

	/* FIXME Cope with genuine pe_count 0 */

	/* If missing, estimate pv->size from file-based metadata */
	if (!pv->size && pv->pe_count)
		pv->size = pv->pe_count * (uint64_t) vg->extent_size +
			   pv->pe_start + size_reduction;

	/* Recalculate number of extents that will fit */
	if (!pv->pe_count && vg->extent_size) {
		pe_count = (pv->size - pv->pe_start - size_reduction) /
			   vg->extent_size;
		if (pe_count > UINT32_MAX) {
			log_error("PV %s too large for extent size %s.",
				  pv_dev_name(pv),
				  display_size(vg->cmd, (uint64_t) vg->extent_size));
			return 0;
		}
		pv->pe_count = (uint32_t) pe_count;
	}

	return 1;
}

static void *_create_text_context(struct dm_pool *mem, struct text_context *tc)
{
	struct text_context *new_tc;
	const char *path;
	char *tmp;

	if (!tc)
		return NULL;

	path = tc->path_live;

	if ((tmp = strstr(path, ".tmp")) && (tmp == path + strlen(path) - 4)) {
		log_error("%s: Volume group filename may not end in .tmp",
			  path);
		return NULL;
	}

	if (!(new_tc = dm_pool_alloc(mem, sizeof(*new_tc))))
		return_NULL;

	if (!(new_tc->path_live = dm_pool_strdup(mem, path)))
		goto_bad;

	/* If path_edit not defined, create one from path_live with .tmp suffix. */
	if (!tc->path_edit) {
		if (!(tmp = dm_pool_alloc(mem, strlen(path) + 5)))
			goto_bad;
		sprintf(tmp, "%s.tmp", path);
		new_tc->path_edit = tmp;
	}
	else if (!(new_tc->path_edit = dm_pool_strdup(mem, tc->path_edit)))
		goto_bad;

	if (!(new_tc->desc = tc->desc ? dm_pool_strdup(mem, tc->desc)
				      : dm_pool_strdup(mem, "")))
		goto_bad;

	return (void *) new_tc;

      bad:
	dm_pool_free(mem, new_tc);

	log_error("Couldn't allocate text format context object.");
	return NULL;
}

static int _create_vg_text_instance(struct format_instance *fid,
                                    const struct format_instance_ctx *fic)
{
	uint32_t type = fic->type;
	struct text_fid_context *fidtc;
	struct metadata_area *mda;
	struct lvmcache_vginfo *vginfo;
	const char *vg_name, *vg_id;

	if (!(fidtc = (struct text_fid_context *)
			dm_pool_zalloc(fid->mem, sizeof(*fidtc)))) {
		log_error("Couldn't allocate text_fid_context.");
		return 0;
	}

	fid->private = (void *) fidtc;

	if (type & FMT_INSTANCE_PRIVATE_MDAS) {
		if (!(mda = dm_pool_zalloc(fid->mem, sizeof(*mda))))
			return_0;
		mda->ops = &_metadata_text_file_backup_ops;
		mda->metadata_locn = _create_text_context(fid->mem, fic->context.private);
		mda->status = 0;
		fid->metadata_areas_index = NULL;
		fid_add_mda(fid, mda, NULL, 0, 0);
	} else {
		vg_name = fic->context.vg_ref.vg_name;
		vg_id = fic->context.vg_ref.vg_id;

		if (!(fid->metadata_areas_index = dm_hash_create(128))) {
			log_error("Couldn't create metadata index for format "
				  "instance of VG %s.", vg_name);
			return 0;
		}

		if (type & FMT_INSTANCE_MDAS) {
			if (!(vginfo = lvmcache_vginfo_from_vgname(vg_name, vg_id)))
				goto_out;
			if (!lvmcache_fid_add_mdas_vg(vginfo, fid))
				goto_out;
		}
	}

out:
	return 1;
}

static int _add_metadata_area_to_pv(struct physical_volume *pv,
				    unsigned mda_index,
				    uint64_t mda_start,
				    uint64_t mda_size,
				    unsigned mda_ignored)
{
	struct metadata_area *mda;
	struct mda_context *mdac;
	struct mda_lists *mda_lists = (struct mda_lists *) pv->fmt->private;

	if (mda_index >= FMT_TEXT_MAX_MDAS_PER_PV) {
		log_error(INTERNAL_ERROR "can't add metadata area with "
					 "index %u to PV %s. Metadata "
					 "layout not supported by %s format.",
					  mda_index, dev_name(pv->dev),
					  pv->fmt->name);
	}

	if (!(mda = dm_pool_zalloc(pv->fid->mem, sizeof(struct metadata_area)))) {
		log_error("struct metadata_area allocation failed");
		return 0;
	}

	if (!(mdac = dm_pool_zalloc(pv->fid->mem, sizeof(struct mda_context)))) {
		log_error("struct mda_context allocation failed");
		free(mda);
		return 0;
	}

	mda->ops = mda_lists->raw_ops;
	mda->metadata_locn = mdac;
	mda->status = 0;

	mdac->area.dev = pv->dev;
	mdac->area.start = mda_start;
	mdac->area.size = mda_size;
	mdac->free_sectors = UINT64_C(0);
	memset(&mdac->rlocn, 0, sizeof(mdac->rlocn));
	mda_set_ignored(mda, mda_ignored);

	fid_add_mda(pv->fid, mda, (char *) &pv->id, ID_LEN, mda_index);

	return 1;
}

static int _text_pv_remove_metadata_area(const struct format_type *fmt,
					 struct physical_volume *pv,
					 unsigned mda_index);

static int _text_pv_add_metadata_area(const struct format_type *fmt,
				      struct physical_volume *pv,
				      int pe_start_locked,
				      unsigned mda_index,
				      uint64_t mda_size,
				      unsigned mda_ignored)
{
	struct format_instance *fid = pv->fid;
	const char *pvid = (const char *) (*pv->old_id.uuid ? &pv->old_id : &pv->id);
	uint64_t ba_size, pe_start, first_unallocated;
	uint64_t alignment, alignment_offset;
	uint64_t disk_size;
	uint64_t mda_start;
	uint64_t adjustment, limit, tmp_mda_size;
	uint64_t wipe_size = 8 << SECTOR_SHIFT;
	uint64_t zero_len;
	size_t page_size = lvm_getpagesize();
	struct metadata_area *mda;
	struct mda_context *mdac;
	const char *limit_name;
	int limit_applied = 0;

	if (mda_index >= FMT_TEXT_MAX_MDAS_PER_PV) {
		log_error(INTERNAL_ERROR "invalid index of value %u used "
			      "while trying to add metadata area on PV %s. "
			      "Metadata layout not supported by %s format.",
			       mda_index, pv_dev_name(pv), fmt->name);
		return 0;
	}

	pe_start = pv->pe_start << SECTOR_SHIFT;
	ba_size = pv->ba_size << SECTOR_SHIFT;
	alignment = pv->pe_align << SECTOR_SHIFT;
	alignment_offset = pv->pe_align_offset << SECTOR_SHIFT;
	disk_size = pv->size << SECTOR_SHIFT;
	mda_size = mda_size << SECTOR_SHIFT;

	if (fid_get_mda_indexed(fid, pvid, ID_LEN, mda_index)) {
		if (!_text_pv_remove_metadata_area(fmt, pv, mda_index)) {
			log_error(INTERNAL_ERROR "metadata area with index %u already "
				  "exists on PV %s and removal failed.",
				  mda_index, pv_dev_name(pv));
			return 0;
		}
	}

	/* First metadata area at the start of the device. */
	if (mda_index == 0) {
		/*
		 * Try to fit MDA0 end within given pe_start limit if its value
		 * is locked. If it's not locked, count with any existing MDA1.
		 * If there's no MDA1, just use disk size as the limit.
		 */
		if (pe_start_locked) {
			limit = pe_start;
			limit_name = "pe_start";
		}
		else if ((mda = fid_get_mda_indexed(fid, pvid, ID_LEN, 1)) &&
			 (mdac = mda->metadata_locn)) {
			limit = mdac->area.start;
			limit_name = "MDA1 start";
		}
		else {
			limit = disk_size;
			limit_name = "disk size";
		}

		/* Adjust limits for bootloader area if present. */
		if (ba_size) {
			limit -= ba_size;
			limit_name = "ba_start";
		}

		if (limit > disk_size)
			goto bad;

		mda_start = LABEL_SCAN_SIZE;

		/* Align MDA0 start with page size if possible. */
		if (limit - mda_start >= MDA_SIZE_MIN) {
			if ((adjustment = mda_start % page_size))
				mda_start += (page_size - adjustment);
		}

		/* Align MDA0 end position with given alignment if possible. */
		if (alignment &&
		    (adjustment = (mda_start + mda_size) % alignment)) {
			tmp_mda_size = mda_size + alignment - adjustment;
			if (mda_start + tmp_mda_size <= limit)
				mda_size = tmp_mda_size;
		}

		/* Align MDA0 end position with given alignment offset if possible. */
		if (alignment && alignment_offset &&
		    (((mda_start + mda_size) % alignment) == 0)) {
			tmp_mda_size = mda_size + alignment_offset;
			if (mda_start + tmp_mda_size <= limit)
				mda_size = tmp_mda_size;
		}

		if (mda_start + mda_size > limit) {
			/*
			 * Try to decrease the MDA0 size with twice the
			 * alignment and then align with given alignment.
			 * If pe_start is locked, skip this type of
			 * alignment since it would be useless.
			 * Check first whether we can apply that!
			 */
			if (!pe_start_locked && alignment &&
			    ((limit - mda_start) > alignment * 2)) {
				mda_size = limit - mda_start - alignment * 2;

				if ((adjustment = (mda_start + mda_size) % alignment))
					mda_size += (alignment - adjustment);

				/* Still too much? Then there's nothing else to do. */
				if (mda_start + mda_size > limit)
					goto bad;
			}
			/* Otherwise, give up and take any usable space. */
			else
				mda_size = limit - mda_start;

			limit_applied = 1;
		}

		/*
		 * If PV's pe_start is not locked, update pe_start value with the
		 * start of the area that follows the MDA0 we've just calculated.
		 */
		if (!pe_start_locked) {
			if (ba_size) {
				pv->ba_start = (mda_start + mda_size) >> SECTOR_SHIFT;
				pv->pe_start = pv->ba_start + pv->ba_size;
			} else
				pv->pe_start = (mda_start + mda_size) >> SECTOR_SHIFT;
		}
	}
	/* Second metadata area at the end of the device. */
	else {
		/*
		 * Try to fit MDA1 start within given pe_end or pe_start limit
		 * if defined or locked. If pe_start is not defined yet, count
		 * with any existing MDA0. If MDA0 does not exist, just use
		 * LABEL_SCAN_SIZE.
		 *
		 * The first_unallocated here is the first unallocated byte
		 * beyond existing pe_end if there is any preallocated data area
		 * reserved already so we can take that as lower limit for our MDA1
		 * start calculation. If data area is not reserved yet, we set
		 * first_unallocated to 0, meaning this is not our limiting factor
		 * and we will look at other limiting factors if they exist.
		 * Of course, if we have preallocated data area, we also must
		 * have pe_start assigned too (simply, data area needs its start
		 * and end specification).
		 */
		first_unallocated = pv->pe_count ? (pv->pe_start + pv->pe_count *
						    (uint64_t)pv->pe_size) << SECTOR_SHIFT
						 : 0;

		if (pe_start || pe_start_locked) {
			limit = first_unallocated ? first_unallocated : pe_start;
			limit_name = first_unallocated ? "pe_end" : "pe_start";
		} else {
			if ((mda = fid_get_mda_indexed(fid, pvid, ID_LEN, 0)) &&
				 (mdac = mda->metadata_locn)) {
				limit = mdac->area.start + mdac->area.size;
				limit_name = "MDA0 end";
			}
			else {
				limit = LABEL_SCAN_SIZE;
				limit_name = "label scan size";
			}

			/* Adjust limits for bootloader area if present. */
			if (ba_size) {
				limit += ba_size;
				limit_name = "ba_end";
			}
		}

		if (limit >= disk_size)
			goto bad;

		if (mda_size > disk_size) {
			mda_size = disk_size - limit;
			limit_applied = 1;
		}

		mda_start = disk_size - mda_size;

		/* If MDA1 size is too big, just take any usable space. */
		if (disk_size - mda_size < limit) {
			mda_size = disk_size - limit;
			mda_start = disk_size - mda_size;
			limit_applied = 1;
		}
		/* Otherwise, try to align MDA1 start if possible. */
		else if (alignment &&
		    (adjustment = mda_start % alignment)) {
			tmp_mda_size = mda_size + adjustment;
			if (tmp_mda_size < disk_size &&
			    disk_size - tmp_mda_size >= limit) {
				mda_size = tmp_mda_size;
				mda_start = disk_size - mda_size;
			}
		}
	}

	if (limit_applied)
		log_very_verbose("Using limited metadata area size on %s "
				 "with value " FMTu64 " (limited by %s of "
				 FMTu64 ").", pv_dev_name(pv),
				  mda_size, limit_name, limit);

	if (mda_size) {
		if (mda_size < MDA_SIZE_MIN) {
			log_error("Metadata area size too small: " FMTu64 " bytes. "
				  "It must be at least %u bytes.", mda_size, MDA_SIZE_MIN);
			goto bad;
		}

		/* Wipe metadata area with zeroes. */

		zero_len = (mda_size > wipe_size) ? wipe_size : mda_size;

		if (!dev_write_zeros(pv->dev, mda_start, zero_len)) {
			log_error("Failed to wipe new metadata area on %s at %llu len %llu",
				   pv_dev_name(pv),
				   (unsigned long long)mda_start,
				   (unsigned long long)zero_len);
			return 0;
		}

		/* Finally, add new metadata area to PV's format instance. */
		if (!_add_metadata_area_to_pv(pv, mda_index, mda_start,
					      mda_size, mda_ignored))
			return_0;
	}

	return 1;

bad:
	log_error("Not enough space available for metadata area "
		  "with index %u on PV %s.", mda_index, pv_dev_name(pv));
	return 0;
}

static int _remove_metadata_area_from_pv(struct physical_volume *pv,
					 unsigned mda_index)
{
	if (mda_index >= FMT_TEXT_MAX_MDAS_PER_PV) {
		log_error(INTERNAL_ERROR "can't remove metadata area with "
					 "index %u from PV %s. Metadata "
					 "layou not supported by %s format.",
					  mda_index, dev_name(pv->dev),
					  pv->fmt->name);
		return 0;
	}

	return fid_remove_mda(pv->fid, NULL, (const char *) &pv->id,
			      ID_LEN, mda_index);
}

static int _text_pv_remove_metadata_area(const struct format_type *fmt,
					 struct physical_volume *pv,
					 unsigned mda_index)
{
	return _remove_metadata_area_from_pv(pv, mda_index);
}

static int _text_pv_resize(const struct format_type *fmt,
			   struct physical_volume *pv,
			   struct volume_group *vg,
			   uint64_t size)
{
	struct format_instance *fid = pv->fid;
	const char *pvid = (const char *) (*pv->old_id.uuid ? &pv->old_id : &pv->id);
	struct metadata_area *mda;
	struct mda_context *mdac;
	uint64_t size_reduction;
	uint64_t mda_size;
	unsigned mda_ignored;

	/*
	 * First, set the new size and update the cache and reset pe_count.
	 * (pe_count must be reset otherwise it would be considered as
	 * a limiting factor while moving the mda!)
	 */
	pv->size = size;
	pv->pe_count = 0;

	/* If there's an mda at the end, move it to a new position. */
	if ((mda = fid_get_mda_indexed(fid, pvid, ID_LEN, 1)) &&
	    (mdac = mda->metadata_locn)) {
		/* FIXME: Maybe MDA0 size would be better? */
		mda_size = mdac->area.size >> SECTOR_SHIFT;
		mda_ignored = mda_is_ignored(mda);

		if (!_text_pv_remove_metadata_area(fmt, pv, 1) ||
		    !_text_pv_add_metadata_area(fmt, pv, 1, 1, mda_size,
						mda_ignored)) {
			log_error("Failed to move metadata area with index 1 "
				  "while resizing PV %s.", pv_dev_name(pv));
			return 0;
		}
	}

	/* If there's a VG, reduce size by counting in pe_start and metadata areas. */
	if (vg && !is_orphan_vg(vg->name)) {
		size_reduction = pv_pe_start(pv);
		if ((mda = fid_get_mda_indexed(fid, pvid, ID_LEN, 1)) &&
		    (mdac = mda->metadata_locn))
			size_reduction += mdac->area.size >> SECTOR_SHIFT;
		pv->size -= size_reduction;
	}

	return 1;
}

static struct format_instance *_text_create_text_instance(const struct format_type *fmt,
							  const struct format_instance_ctx *fic)
{
	struct format_instance *fid;

	if (!(fid = alloc_fid(fmt, fic)))
		return_NULL;

	if (!_create_vg_text_instance(fid, fic)) {
		dm_pool_destroy(fid->mem);
		return_NULL;
	}

	return fid;
}

static struct format_handler _text_handler = {
	.pv_initialise = _text_pv_initialise,
	.pv_setup = _text_pv_setup,
	.pv_add_metadata_area = _text_pv_add_metadata_area,
	.pv_remove_metadata_area = _text_pv_remove_metadata_area,
	.pv_resize = _text_pv_resize,
	.pv_write = _text_pv_write,
	.pv_needs_rewrite = _text_pv_needs_rewrite,
	.vg_setup = _text_vg_setup,
	.lv_setup = _text_lv_setup,
	.create_instance = _text_create_text_instance,
	.destroy_instance = _text_destroy_instance,
	.destroy = _text_destroy
};

struct format_type *create_text_format(struct cmd_context *cmd)
{
	struct format_instance_ctx fic;
	struct format_instance *fid;
	struct format_type *fmt;
	struct mda_lists *mda_lists;

	if (!(fmt = malloc(sizeof(*fmt)))) {
		log_error("Failed to allocate text format type structure.");
		return NULL;
	}

	fmt->cmd = cmd;
	fmt->ops = &_text_handler;
	fmt->name = FMT_TEXT_NAME;
	fmt->alias = FMT_TEXT_ALIAS;
	fmt->orphan_vg_name = ORPHAN_VG_NAME(FMT_TEXT_NAME);
	fmt->features = FMT_SEGMENTS | FMT_TAGS | FMT_PRECOMMIT |
			FMT_UNLIMITED_VOLS | FMT_RESIZE_PV |
			FMT_UNLIMITED_STRIPESIZE | FMT_CONFIG_PROFILE |
			FMT_NON_POWER2_EXTENTS | FMT_PV_FLAGS;

	if (!(mda_lists = malloc(sizeof(struct mda_lists)))) {
		log_error("Failed to allocate dir_list");
		free(fmt);
		return NULL;
	}

	mda_lists->file_ops = &_metadata_text_file_ops;
	mda_lists->raw_ops = &_metadata_text_raw_ops;
	fmt->private = (void *) mda_lists;

	dm_list_init(&fmt->mda_ops);
	dm_list_add(&fmt->mda_ops, &_metadata_text_raw_ops.list);

	if (!(fmt->labeller = text_labeller_create(fmt))) {
		log_error("Couldn't create text label handler.");
		goto bad;
	}

	if (!(label_register_handler(fmt->labeller))) {
		log_error("Couldn't register text label handler.");
		fmt->labeller->ops->destroy(fmt->labeller);
		goto bad;
	}

	if (!(fmt->orphan_vg = alloc_vg("text_orphan", cmd, fmt->orphan_vg_name)))
		goto_bad;

	fic.type = FMT_INSTANCE_AUX_MDAS;
	fic.context.vg_ref.vg_name = fmt->orphan_vg_name;
	fic.context.vg_ref.vg_id = NULL;
	if (!(fid = _text_create_text_instance(fmt, &fic)))
		goto_bad;

	vg_set_fid(fmt->orphan_vg, fid);

	log_very_verbose("Initialised format: %s", fmt->name);

	return fmt;
bad:
	_text_destroy(fmt);

	return NULL;
}
