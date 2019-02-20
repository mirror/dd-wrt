/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "lib/format_text/format-text.h"
#include "layout.h"
#include "lib/label/label.h"
#include "lib/mm/xlate.h"
#include "lib/cache/lvmcache.h"

#include <sys/stat.h>
#include <fcntl.h>

static int _text_can_handle(struct labeller *l __attribute__((unused)),
			    void *buf,
			    uint64_t sector __attribute__((unused)))
{
	struct label_header *lh = (struct label_header *) buf;

	if (!strncmp((char *)lh->type, LVM2_LABEL, sizeof(lh->type)))
		return 1;

	return 0;
}

struct _dl_setup_baton {
	struct disk_locn *pvh_dlocn_xl;
	struct device *dev;
};

static int _da_setup(struct disk_locn *da, void *baton)
{
	struct _dl_setup_baton *p = baton;
	p->pvh_dlocn_xl->offset = xlate64(da->offset);
	p->pvh_dlocn_xl->size = xlate64(da->size);
	p->pvh_dlocn_xl++;
	return 1;
}

static int _ba_setup(struct disk_locn *ba, void *baton)
{
	return _da_setup(ba, baton);
}

static int _mda_setup(struct metadata_area *mda, void *baton)
{
	struct _dl_setup_baton *p = baton;
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;

	if (mdac->area.dev != p->dev)
		return 1;

	p->pvh_dlocn_xl->offset = xlate64(mdac->area.start);
	p->pvh_dlocn_xl->size = xlate64(mdac->area.size);
	p->pvh_dlocn_xl++;

	return 1;
}

static int _dl_null_termination(void *baton)
{
	struct _dl_setup_baton *p = baton;

	p->pvh_dlocn_xl->offset = xlate64(UINT64_C(0));
	p->pvh_dlocn_xl->size = xlate64(UINT64_C(0));
	p->pvh_dlocn_xl++;

	return 1;
}

static int _text_write(struct label *label, void *buf)
{
	struct label_header *lh = (struct label_header *) buf;
	struct pv_header *pvhdr;
	struct pv_header_extension *pvhdr_ext;
	struct lvmcache_info *info;
	struct _dl_setup_baton baton;
	char buffer[64] __attribute__((aligned(8)));
	int ba1, da1, mda1, mda2;

	/*
	 * PV header base
	 */
	/* FIXME Move to where label is created */
	strncpy(label->type, LVM2_LABEL, sizeof(label->type));

	strncpy((char *)lh->type, label->type, sizeof(label->type));

	pvhdr = (struct pv_header *) ((char *) buf + xlate32(lh->offset_xl));
	info = (struct lvmcache_info *) label->info;
	pvhdr->device_size_xl = xlate64(lvmcache_device_size(info));
	memcpy(pvhdr->pv_uuid, &lvmcache_device(info)->pvid, sizeof(struct id));
	if (!id_write_format((const struct id *)pvhdr->pv_uuid, buffer,
			     sizeof(buffer))) {
		stack;
		buffer[0] = '\0';
	}

	baton.dev = lvmcache_device(info);
	baton.pvh_dlocn_xl = &pvhdr->disk_areas_xl[0];

	/* List of data areas (holding PEs) */
	lvmcache_foreach_da(info, _da_setup, &baton);
	_dl_null_termination(&baton);

	/* List of metadata area header locations */
	lvmcache_foreach_mda(info, _mda_setup, &baton);
	_dl_null_termination(&baton);

	/*
	 * PV header extension
	 */
	pvhdr_ext = (struct pv_header_extension *) ((char *) baton.pvh_dlocn_xl);
	pvhdr_ext->version = xlate32(PV_HEADER_EXTENSION_VSN);
	pvhdr_ext->flags = xlate32(lvmcache_ext_flags(info));

	/* List of bootloader area locations */
	baton.pvh_dlocn_xl = &pvhdr_ext->bootloader_areas_xl[0];
	lvmcache_foreach_ba(info, _ba_setup, &baton);
	_dl_null_termination(&baton);

	/* Create debug message with ba, da and mda locations */
	ba1 = (xlate64(pvhdr_ext->bootloader_areas_xl[0].offset) ||
	       xlate64(pvhdr_ext->bootloader_areas_xl[0].size)) ? 0 : -1;

	da1 = (xlate64(pvhdr->disk_areas_xl[0].offset) ||
	       xlate64(pvhdr->disk_areas_xl[0].size)) ? 0 : -1;

	mda1 = da1 + 2;
	mda2 = mda1 + 1;
	
	if (!xlate64(pvhdr->disk_areas_xl[mda1].offset) &&
	    !xlate64(pvhdr->disk_areas_xl[mda1].size))
		mda1 = mda2 = 0;
	else if (!xlate64(pvhdr->disk_areas_xl[mda2].offset) &&
		 !xlate64(pvhdr->disk_areas_xl[mda2].size))
		mda2 = 0;

	log_debug_metadata("%s: Preparing PV label header %s size " FMTu64 " with"
			   "%s%.*" PRIu64 "%s%.*" PRIu64 "%s"
			   "%s%.*" PRIu64 "%s%.*" PRIu64 "%s"
			   "%s%.*" PRIu64 "%s%.*" PRIu64 "%s"
			   "%s%.*" PRIu64 "%s%.*" PRIu64 "%s",
			   dev_name(lvmcache_device(info)), buffer, lvmcache_device_size(info),
			   (ba1 > -1) ? " ba1 (" : "",
			   (ba1 > -1) ? 1 : 0,
			   (ba1 > -1) ? xlate64(pvhdr_ext->bootloader_areas_xl[ba1].offset) >> SECTOR_SHIFT : 0,
			   (ba1 > -1) ? "s, " : "",
			   (ba1 > -1) ? 1 : 0,
			   (ba1 > -1) ? xlate64(pvhdr_ext->bootloader_areas_xl[ba1].size) >> SECTOR_SHIFT : 0,
			   (ba1 > -1) ? "s)" : "",
			   (da1 > -1) ? " da1 (" : "",
			   (da1 > -1) ? 1 : 0,
			   (da1 > -1) ? xlate64(pvhdr->disk_areas_xl[da1].offset) >> SECTOR_SHIFT : 0,
			   (da1 > -1) ? "s, " : "",
			   (da1 > -1) ? 1 : 0,
			   (da1 > -1) ? xlate64(pvhdr->disk_areas_xl[da1].size) >> SECTOR_SHIFT : 0,
			   (da1 > -1) ? "s)" : "",
			   mda1 ? " mda1 (" : "",
			   mda1 ? 1 : 0,
			   mda1 ? xlate64(pvhdr->disk_areas_xl[mda1].offset) >> SECTOR_SHIFT : 0,
			   mda1 ? "s, " : "",
			   mda1 ? 1 : 0,
			   mda1 ? xlate64(pvhdr->disk_areas_xl[mda1].size) >> SECTOR_SHIFT : 0,
			   mda1 ? "s)" : "",
			   mda2 ? " mda2 (" : "",
			   mda2 ? 1 : 0,
			   mda2 ? xlate64(pvhdr->disk_areas_xl[mda2].offset) >> SECTOR_SHIFT : 0,
			   mda2 ? "s, " : "",
			   mda2 ? 1 : 0,
			   mda2 ? xlate64(pvhdr->disk_areas_xl[mda2].size) >> SECTOR_SHIFT : 0,
			   mda2 ? "s)" : "");

	if (da1 < 0) {
		log_error(INTERNAL_ERROR "%s label header currently requires "
			  "a data area.", dev_name(lvmcache_device(info)));
		return 0;
	}

	return 1;
}

int add_da(struct dm_pool *mem, struct dm_list *das,
	   uint64_t start, uint64_t size)
{
	struct data_area_list *dal;

	if (!mem) {
		if (!(dal = malloc(sizeof(*dal)))) {
			log_error("struct data_area_list allocation failed");
			return 0;
		}
	} else {
		if (!(dal = dm_pool_alloc(mem, sizeof(*dal)))) {
			log_error("struct data_area_list allocation failed");
			return 0;
		}
	}

	dal->disk_locn.offset = start;
	dal->disk_locn.size = size;

	dm_list_add(das, &dal->list);

	return 1;
}

void del_das(struct dm_list *das)
{
	struct dm_list *dah, *tmp;
	struct data_area_list *da;

	dm_list_iterate_safe(dah, tmp, das) {
		da = dm_list_item(dah, struct data_area_list);
		dm_list_del(&da->list);
		free(da);
	}
}

int add_ba(struct dm_pool *mem, struct dm_list *eas,
	   uint64_t start, uint64_t size)
{
	return add_da(mem, eas, start, size);
}

void del_bas(struct dm_list *bas)
{
	del_das(bas);
}

/* FIXME: refactor this function with other mda constructor code */
int add_mda(const struct format_type *fmt, struct dm_pool *mem, struct dm_list *mdas,
	    struct device *dev, uint64_t start, uint64_t size, unsigned ignored)
{
/* FIXME List size restricted by pv_header SECTOR_SIZE */
	struct metadata_area *mdal, *mda;
	struct mda_lists *mda_lists = (struct mda_lists *) fmt->private;
	struct mda_context *mdac, *mdac2;

	if (!mem) {
		if (!(mdal = malloc(sizeof(struct metadata_area)))) {
			log_error("struct mda_list allocation failed");
			return 0;
		}

		if (!(mdac = malloc(sizeof(struct mda_context)))) {
			log_error("struct mda_context allocation failed");
			free(mdal);
			return 0;
		}
	} else {
		if (!(mdal = dm_pool_alloc(mem, sizeof(struct metadata_area)))) {
			log_error("struct mda_list allocation failed");
			return 0;
		}

		if (!(mdac = dm_pool_alloc(mem, sizeof(struct mda_context)))) {
			log_error("struct mda_context allocation failed");
			return 0;
		}
	}

	mdal->ops = mda_lists->raw_ops;
	mdal->metadata_locn = mdac;

	mdac->area.dev = dev;
	mdac->area.start = start;
	mdac->area.size = size;
	mdac->free_sectors = UINT64_C(0);
	memset(&mdac->rlocn, 0, sizeof(mdac->rlocn));

	/* Set MDA_PRIMARY only if this is the first metadata area on this device. */
	mdal->status = MDA_PRIMARY;
	dm_list_iterate_items(mda, mdas) {
		mdac2 = mda->metadata_locn;
		if (mdac2->area.dev == dev) {
			mdal->status = 0;
			break;
		}
	}

	mda_set_ignored(mdal, ignored);

	dm_list_add(mdas, &mdal->list);
	return 1;
}

void del_mdas(struct dm_list *mdas)
{
	struct dm_list *mdah, *tmp;
	struct metadata_area *mda;

	dm_list_iterate_safe(mdah, tmp, mdas) {
		mda = dm_list_item(mdah, struct metadata_area);
		free(mda->metadata_locn);
		dm_list_del(&mda->list);
		free(mda);
	}
}

static int _text_initialise_label(struct labeller *l __attribute__((unused)),
				  struct label *label)
{
	strncpy(label->type, LVM2_LABEL, sizeof(label->type));

	return 1;
}

struct _update_mda_baton {
	struct lvmcache_info *info;
	struct label *label;
};

static int _read_mda_header_and_metadata(struct metadata_area *mda, void *baton)
{
	struct _update_mda_baton *p = baton;
	const struct format_type *fmt = p->label->labeller->fmt;
	struct mda_context *mdac = (struct mda_context *) mda->metadata_locn;
	struct mda_header *mdah;
	struct lvmcache_vgsummary vgsummary = { 0 };

	if (!(mdah = raw_read_mda_header(fmt, &mdac->area, mda_is_primary(mda)))) {
		log_error("Failed to read mda header from %s", dev_name(mdac->area.dev));
		goto fail;
	}

	mda_set_ignored(mda, rlocn_is_ignored(mdah->raw_locns));

	if (mda_is_ignored(mda)) {
		log_debug_metadata("Ignoring mda on device %s at offset " FMTu64,
				   dev_name(mdac->area.dev),
				   mdac->area.start);
		return 1;
	}

	if (!read_metadata_location_summary(fmt, mdah, mda_is_primary(mda), &mdac->area,
					    &vgsummary, &mdac->free_sectors)) {
		if (vgsummary.zero_offset)
			return 1;

		log_error("Failed to read metadata summary from %s", dev_name(mdac->area.dev));
		goto fail;
	}

	if (!lvmcache_update_vgname_and_id(p->info, &vgsummary)) {
		log_error("Failed to save lvm summary for %s", dev_name(mdac->area.dev));
		goto fail;
	}

	return 1;

fail:
	lvmcache_del(p->info);
	return 0;
}

static int _text_read(struct labeller *l, struct device *dev, void *label_buf,
		      struct label **label)
{
	struct label_header *lh = (struct label_header *) label_buf;
	struct pv_header *pvhdr;
	struct pv_header_extension *pvhdr_ext;
	struct lvmcache_info *info;
	struct disk_locn *dlocn_xl;
	uint64_t offset;
	uint32_t ext_version;
	struct _update_mda_baton baton;

	/*
	 * PV header base
	 */
	pvhdr = (struct pv_header *) ((char *) label_buf + xlate32(lh->offset_xl));

	if (!(info = lvmcache_add(l, (char *)pvhdr->pv_uuid, dev,
				  FMT_TEXT_ORPHAN_VG_NAME,
				  FMT_TEXT_ORPHAN_VG_NAME, 0)))
		return_0;

	*label = lvmcache_get_label(info);

	lvmcache_set_device_size(info, xlate64(pvhdr->device_size_xl));

	lvmcache_del_das(info);
	lvmcache_del_mdas(info);
	lvmcache_del_bas(info);

	/* Data areas holding the PEs */
	dlocn_xl = pvhdr->disk_areas_xl;
	while ((offset = xlate64(dlocn_xl->offset))) {
		lvmcache_add_da(info, offset, xlate64(dlocn_xl->size));
		dlocn_xl++;
	}

	/* Metadata area headers */
	dlocn_xl++;
	while ((offset = xlate64(dlocn_xl->offset))) {
		lvmcache_add_mda(info, dev, offset, xlate64(dlocn_xl->size), 0);
		dlocn_xl++;
	}

	dlocn_xl++;

	/*
	 * PV header extension
	 */
	pvhdr_ext = (struct pv_header_extension *) ((char *) dlocn_xl);
	if (!(ext_version = xlate32(pvhdr_ext->version)))
		goto out;

	log_debug_metadata("%s: PV header extension version " FMTu32 " found",
			   dev_name(dev), ext_version);

	/* Extension version */
	lvmcache_set_ext_version(info, xlate32(pvhdr_ext->version));

	/* Extension flags */
	lvmcache_set_ext_flags(info, xlate32(pvhdr_ext->flags));

	/* Bootloader areas */
	dlocn_xl = pvhdr_ext->bootloader_areas_xl;
	while ((offset = xlate64(dlocn_xl->offset))) {
		lvmcache_add_ba(info, offset, xlate64(dlocn_xl->size));
		dlocn_xl++;
	}
out:
	baton.info = info;
	baton.label = *label;

	/*
	 * In the vg_read phase, we compare all mdas and decide which to use
	 * which are bad and need repair.
	 *
	 * FIXME: this quits if the first mda is bad, but we need something
	 * smarter to be able to use the second mda if it's good.
	 */
	if (!lvmcache_foreach_mda(info, _read_mda_header_and_metadata, &baton)) {
		log_error("Failed to scan VG from %s", dev_name(dev));
		return 0;
	}

	return 1;
}

static void _text_destroy_label(struct labeller *l __attribute__((unused)),
				struct label *label)
{
	struct lvmcache_info *info = (struct lvmcache_info *) label->info;

	lvmcache_del_mdas(info);
	lvmcache_del_das(info);
	lvmcache_del_bas(info);
}

static void _fmt_text_destroy(struct labeller *l)
{
	free(l);
}

struct label_ops _text_ops = {
	.can_handle = _text_can_handle,
	.write = _text_write,
	.read = _text_read,
	.initialise_label = _text_initialise_label,
	.destroy_label = _text_destroy_label,
	.destroy = _fmt_text_destroy,
};

struct labeller *text_labeller_create(const struct format_type *fmt)
{
	struct labeller *l;

	if (!(l = zalloc(sizeof(*l)))) {
		log_error("Couldn't allocate labeller object.");
		return NULL;
	}

	l->ops = &_text_ops;
	l->fmt = fmt;

	return l;
}
