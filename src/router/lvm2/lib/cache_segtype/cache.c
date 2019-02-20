/*
 * Copyright (C) 2013-2016 Red Hat, Inc. All rights reserved.
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
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"
#include "lib/display/display.h"
#include "lib/format_text/text_export.h"
#include "lib/config/config.h"
#include "lib/datastruct/str_list.h"
#include "lib/misc/lvm-string.h"
#include "lib/activate/activate.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/config/defaults.h"

static const char _cache_module[] = "cache";
#define CACHE_POLICY_WHEN_MISSING   "mq"
#define CACHE_MODE_WHEN_MISSING	    CACHE_MODE_WRITETHROUGH

/* TODO: using static field here, maybe should be a part of segment_type */
static unsigned _feature_mask;

#define SEG_LOG_ERROR(t, p...) \
        log_error(t " segment %s of logical volume %s.", ## p,	\
                  dm_config_parent_name(sn), seg->lv->name), 0;

static int _cache_out_line(const char *line, void *_f)
{
	log_print("    Setting\t\t%s", line);

	return 1;
}

static void _cache_display(const struct lv_segment *seg)
{
	const struct dm_config_node *n;
	const struct lv_segment *pool_seg =
		seg_is_cache_pool(seg) ? seg : first_seg(seg->pool_lv);

	log_print("  Chunk size\t\t%s",
		  display_size(seg->lv->vg->cmd, pool_seg->chunk_size));

	if (pool_seg->cache_metadata_format != CACHE_METADATA_FORMAT_UNSELECTED)
		log_print("  Metadata format\t%u", pool_seg->cache_metadata_format);

	if (pool_seg->cache_mode != CACHE_MODE_UNSELECTED)
		log_print("  Mode\t\t%s", get_cache_mode_name(pool_seg));

	if (pool_seg->policy_name)
		log_print("  Policy\t\t%s", pool_seg->policy_name);

	if (pool_seg->policy_settings &&
	    (n = pool_seg->policy_settings->child))
		dm_config_write_node(n, _cache_out_line, NULL);

	log_print(" ");
}

/*
 * When older metadata are loaded without newer settings,
 * set then to default settings (the one that could have been
 * used implicitely at that time).
 *
 * Needs both segments cache and cache_pool to be loaded.
 */
static void _fix_missing_defaults(struct lv_segment *cpool_seg)
{
	if (!cpool_seg->policy_name) {
		cpool_seg->policy_name = CACHE_POLICY_WHEN_MISSING;
		log_verbose("Cache pool %s is missing cache policy, using %s.",
			    display_lvname(cpool_seg->lv),
			    cpool_seg->policy_name);
	}

	if (cpool_seg->cache_metadata_format == CACHE_METADATA_FORMAT_UNSELECTED) {
		cpool_seg->cache_metadata_format = CACHE_METADATA_FORMAT_1;
		log_verbose("Cache pool %s uses implicit metadata format %u.",
			    display_lvname(cpool_seg->lv), cpool_seg->cache_metadata_format);
	}

	if (cpool_seg->cache_mode == CACHE_MODE_UNSELECTED) {
		cpool_seg->cache_mode = CACHE_MODE_WHEN_MISSING;
		log_verbose("Cache pool %s is missing cache mode, using %s.",
			    display_lvname(cpool_seg->lv),
			    get_cache_mode_name(cpool_seg));
	}
}

static int _cache_pool_text_import(struct lv_segment *seg,
				   const struct dm_config_node *sn,
				   struct dm_hash_table *pv_hash __attribute__((unused)))
{
	struct logical_volume *data_lv, *meta_lv;
	const char *str = NULL;
	struct dm_pool *mem = seg->lv->vg->vgmem;

	if (!dm_config_has_node(sn, "data"))
		return SEG_LOG_ERROR("Cache data not specified in");
	if (!(str = dm_config_find_str(sn, "data", NULL)))
		return SEG_LOG_ERROR("Cache data must be a string in");
	if (!(data_lv = find_lv(seg->lv->vg, str)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
				     "cache data in", str);

	if (!dm_config_has_node(sn, "metadata"))
		return SEG_LOG_ERROR("Cache metadata not specified in");
	if (!(str = dm_config_find_str(sn, "metadata", NULL)))
		return SEG_LOG_ERROR("Cache metadata must be a string in");
	if (!(meta_lv = find_lv(seg->lv->vg, str)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
				     "cache metadata in", str);

	if (!dm_config_get_uint32(sn, "chunk_size", &seg->chunk_size))
		return SEG_LOG_ERROR("Couldn't read cache chunk_size in");

	/*
	 * Read in features:
	 *   cache_mode = {passthrough|writethrough|writeback}
	 *
	 *   'cache_mode' does not have to be present.
	 */
	if (dm_config_has_node(sn, "cache_mode")) {
		if (!(str = dm_config_find_str(sn, "cache_mode", NULL)))
			return SEG_LOG_ERROR("cache_mode must be a string in");
		if (!set_cache_mode(&seg->cache_mode, str))
			return SEG_LOG_ERROR("Unknown cache_mode in");
	}

	if (dm_config_has_node(sn, "policy")) {
		if (!(str = dm_config_find_str(sn, "policy", NULL)))
			return SEG_LOG_ERROR("policy must be a string in");
		if (!(seg->policy_name = dm_pool_strdup(mem, str)))
			return SEG_LOG_ERROR("Failed to duplicate policy in");
	}

	if (dm_config_has_node(sn, "metadata_format")) {
		if (!dm_config_get_uint32(sn, "metadata_format", &seg->cache_metadata_format) ||
		    ((seg->cache_metadata_format != CACHE_METADATA_FORMAT_1) &&
		     (seg->cache_metadata_format != CACHE_METADATA_FORMAT_2)))
			return SEG_LOG_ERROR("Unknown cache metadata format %u number in",
					     seg->cache_metadata_format);
		if (seg->cache_metadata_format == CACHE_METADATA_FORMAT_2)
			seg->lv->status |= LV_METADATA_FORMAT;
	}

	/*
	 * Read in policy args:
	 *   policy_settings {
	 *	migration_threshold=2048
	 *	sequention_threashold=100
	 *	random_threashold=200
	 *	read_promote_adjustment=10
	 *	write_promote_adjustment=20
	 *	discard_promote_adjustment=40
	 *
	 *	<key> = <value>
	 *	<key> = <value>
	 *	...
	 *   }
	 *
	 *   If the policy is not present, default policy is used.
	 */
	if ((sn = dm_config_find_node(sn, "policy_settings"))) {
		if (!seg->policy_name)
			return SEG_LOG_ERROR("policy_settings must have a policy_name in");

		if (sn->v)
			return SEG_LOG_ERROR("policy_settings must be a section in");

		if (!(seg->policy_settings = dm_config_clone_node_with_mem(mem, sn, 0)))
			return_0;
	}

	if (!attach_pool_data_lv(seg, data_lv))
		return_0;
	if (!attach_pool_metadata_lv(seg, meta_lv))
		return_0;

	/* when cache pool is used, we require policy and mode to be defined */
	if (!dm_list_empty(&seg->lv->segs_using_this_lv))
		_fix_missing_defaults(seg);

	return 1;
}

static int _cache_pool_text_import_area_count(const struct dm_config_node *sn,
					      uint32_t *area_count)
{
	*area_count = 1;

	return 1;
}

static int _cache_pool_text_export(const struct lv_segment *seg,
				   struct formatter *f)
{
	const char *cache_mode;

	outf(f, "data = \"%s\"", seg_lv(seg, 0)->name);
	outf(f, "metadata = \"%s\"", seg->metadata_lv->name);
	outf(f, "chunk_size = %" PRIu32, seg->chunk_size);

	switch (seg->cache_metadata_format) {
	case CACHE_METADATA_FORMAT_UNSELECTED:
		/* Unselected format is not printed */
		break;
	case CACHE_METADATA_FORMAT_1:
		/* If format 1 was already specified with cache pool, store it,
		 * otherwise format gets stored when LV is cached.
		 * NB: format 1 could be lost anytime, it's a default format.
		 * Older lvm2 tool can easily drop it.
		 */
	case CACHE_METADATA_FORMAT_2: /* more in future ? */
		outf(f, "metadata_format = " FMTu32, seg->cache_metadata_format);
		break;
	default:
		log_error(INTERNAL_ERROR "LV %s is using unknown cache metadada format %u.",
			  display_lvname(seg->lv), seg->cache_metadata_format);
		return 0;
	}

	/*
	 * Cache pool used by a cache LV holds data. Not ideal,
	 * but not worth to break backward compatibility, by shifting
	 * content to cache segment
	 */
	if (seg->cache_mode != CACHE_MODE_UNSELECTED) {
		if (!(cache_mode = get_cache_mode_name(seg)))
			return_0;
		outf(f, "cache_mode = \"%s\"", cache_mode);
	}

	if (seg->policy_name) {
		outf(f, "policy = \"%s\"", seg->policy_name);

		if (seg->policy_settings) {
			if (strcmp(seg->policy_settings->key, "policy_settings")) {
				log_error(INTERNAL_ERROR "Incorrect policy_settings tree, %s.",
					  seg->policy_settings->key);
				return 0;
			}
			if (seg->policy_settings->child)
				out_config_node(f, seg->policy_settings);
		}
	}

	return 1;
}

static void _destroy(struct segment_type *segtype)
{
	free((void *) segtype);
}

#ifdef DEVMAPPER_SUPPORT
/*
 * Parse and look for kernel symbol in /proc/kallsyms
 * this could be our only change to figure out there is
 * cache policy symbol already in the monolithic kernel
 * where 'modprobe dm-cache-smq' will simply not work
 */
static int _lookup_kallsyms(const char *symbol)
{
	static const char _syms[] = "/proc/kallsyms";
	int ret = 0;
	char *line = NULL;
	size_t len;
	FILE *s;

	if (!(s = fopen(_syms, "r")))
		log_sys_debug("fopen", _syms);
	else {
		while (getline(&line, &len, s) != -1)
			if (strstr(line, symbol)) {
				ret = 1; /* Found symbol */
				log_debug("Found kernel symbol%s.", symbol); /* space is in symbol */
				break;
			}

		free(line);
		if (fclose(s))
			log_sys_debug("fclose", _syms);
	}

	return ret;
}


static int _target_present(struct cmd_context *cmd,
			   const struct lv_segment *seg __attribute__((unused)),
			   unsigned *attributes __attribute__((unused)))
{
	/* List of features with their kernel target version */
	static const struct feature {
		uint32_t maj;
		uint32_t min;
		unsigned cache_feature;
		unsigned cache_alias;
		const char feature[12];
		const char module[12]; /* check dm-%s */
		const char ksymbol[12]; /* check for kernel symbol */
		const char *aliasing;
	} _features[] = {
		{ 1, 10, CACHE_FEATURE_METADATA2, 0, "metadata2" },
		/* Assumption: cache >=1.9 always aliases MQ policy */
		{ 1, 9, CACHE_FEATURE_POLICY_SMQ, CACHE_FEATURE_POLICY_MQ, "policy_smq", "cache-smq",
		 " smq_exit", " and aliases cache-mq" },
		{ 1, 8, CACHE_FEATURE_POLICY_SMQ, 0, "policy_smq", "cache-smq", " smq_exit" },
		{ 1, 3, CACHE_FEATURE_POLICY_MQ, 0, "policy_mq", "cache-mq", " mq_init" },
	};
	static const char _lvmconf[] = "global/cache_disabled_features";
	static unsigned _attrs = 0;
	static int _cache_checked = 0;
	static int _cache_present = 0;
	uint32_t maj, min, patchlevel;
	unsigned i;
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	const char *str;

	if (!activation())
		return 0;

	if (!_cache_checked) {
		_cache_checked = 1;

		if (!(_cache_present = target_present_version(cmd, TARGET_NAME_CACHE, 1,
							      &maj, &min, &patchlevel)))
			return_0;

		if ((maj < 1) ||
		    ((maj == 1) && (min < 3))) {
			_cache_present = 0;
			log_warn("WARNING: The cache kernel module is version %u.%u.%u. "
				  "Version 1.3.0+ is required.",
				  maj, min, patchlevel);
			return 0;
		}

		for (i = 0; i < DM_ARRAY_SIZE(_features); ++i) {
			if (_attrs & _features[i].cache_feature)
				continue; /* already present */

			if (!_features[i].module[0]) {
				if ((maj > _features[i].maj) ||
				    (maj == _features[i].maj && min >= _features[i].min)) {
					log_debug_activation("Cache supports %s.",
							     _features[i].feature);
					_attrs |= _features[i].cache_feature;
				}
				continue;
			}
			if (((maj > _features[i].maj) ||
			     (maj == _features[i].maj && min >= _features[i].min)) &&
			    ((_features[i].ksymbol[0] && _lookup_kallsyms(_features[i].ksymbol)) ||
			     module_present(cmd, _features[i].module))) {
				log_debug_activation("Cache policy %s is available%s.",
						     _features[i].module,
						     _features[i].aliasing ? : "");
				_attrs |= (_features[i].cache_feature | _features[i].cache_alias);
			} else if (!_features[i].cache_alias)
				log_very_verbose("Target %s does not support %s.",
						 _cache_module, _features[i].feature);
		}
	}

	if (attributes) {
		if (!_feature_mask) {
			/* Support runtime lvm.conf changes, N.B. avoid 32 feature */
			if ((cn = find_config_tree_array(cmd, global_cache_disabled_features_CFG, NULL))) {
				for (cv = cn->v; cv; cv = cv->next) {
					if (cv->type != DM_CFG_STRING) {
						log_error("Ignoring invalid string in config file %s.",
							  _lvmconf);
						continue;
					}
					str = cv->v.str;
					if (!*str)
						continue;
					for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
						if (strcasecmp(str, _features[i].feature) == 0)
							_feature_mask |= _features[i].cache_feature;
				}
			}

			_feature_mask = ~_feature_mask;

			for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
				if ((_attrs & _features[i].cache_feature) &&
				    !(_feature_mask & _features[i].cache_feature))
					log_very_verbose("Target %s %s support disabled by %s",
							 _cache_module, _features[i].feature, _lvmconf);
		}
		*attributes = _attrs & _feature_mask;
	}

	return _cache_present;
}

static int _modules_needed(struct dm_pool *mem,
			   const struct lv_segment *seg __attribute__((unused)),
			   struct dm_list *modules)
{
	if (!str_list_add(mem, modules, MODULE_NAME_CACHE)) {
		log_error("String list allocation failed for cache module.");
		return 0;
	}

	return 1;
}
#endif /* DEVMAPPER_SUPPORT */

static struct segtype_handler _cache_pool_ops = {
	.display = _cache_display,
	.text_import = _cache_pool_text_import,
	.text_import_area_count = _cache_pool_text_import_area_count,
	.text_export = _cache_pool_text_export,
#ifdef DEVMAPPER_SUPPORT
	.target_present = _target_present,
	.modules_needed = _modules_needed,
#  ifdef DMEVENTD
#  endif        /* DMEVENTD */
#endif
	.destroy = _destroy,
};

static int _cache_text_import(struct lv_segment *seg,
			      const struct dm_config_node *sn,
			      struct dm_hash_table *pv_hash __attribute__((unused)))
{
	struct logical_volume *pool_lv, *origin_lv;
	const char *name;

	if (!dm_config_has_node(sn, "cache_pool"))
		return SEG_LOG_ERROR("cache_pool not specified in");
	if (!(name = dm_config_find_str(sn, "cache_pool", NULL)))
		return SEG_LOG_ERROR("cache_pool must be a string in");
	if (!(pool_lv = find_lv(seg->lv->vg, name)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
				     "cache_pool in", name);

	if (!dm_config_has_node(sn, "origin"))
		return SEG_LOG_ERROR("Cache origin not specified in");
	if (!(name = dm_config_find_str(sn, "origin", NULL)))
		return SEG_LOG_ERROR("Cache origin must be a string in");
	if (!(origin_lv = find_lv(seg->lv->vg, name)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
				     "cache origin in", name);
	if (!set_lv_segment_area_lv(seg, 0, origin_lv, 0, 0))
		return_0;

	seg->cleaner_policy = 0;
	if (dm_config_has_node(sn, "cleaner") &&
	    !dm_config_get_uint32(sn, "cleaner", &seg->cleaner_policy))
		return SEG_LOG_ERROR("Could not read cache cleaner in");

	seg->lv->status |= strstr(seg->lv->name, "_corig") ? LV_PENDING_DELETE : 0;

	if (!attach_pool_lv(seg, pool_lv, NULL, NULL, NULL))
		return_0;

	/* load order is unknown, could be cache origin or pool LV, so check for both */
	if (!dm_list_empty(&pool_lv->segments))
		_fix_missing_defaults(first_seg(pool_lv));

	return 1;
}

static int _cache_text_import_area_count(const struct dm_config_node *sn,
					 uint32_t *area_count)
{
	*area_count = 1;

	return 1;
}

static int _cache_text_export(const struct lv_segment *seg, struct formatter *f)
{
	if (!seg_lv(seg, 0))
		return_0;

	outf(f, "cache_pool = \"%s\"", seg->pool_lv->name);
	outf(f, "origin = \"%s\"", seg_lv(seg, 0)->name);

	if (seg->cleaner_policy)
		outf(f, "cleaner = 1");

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _cache_add_target_line(struct dev_manager *dm,
				 struct dm_pool *mem,
				 struct cmd_context *cmd __attribute__((unused)),
				 void **target_state __attribute__((unused)),
				 struct lv_segment *seg,
				 const struct lv_activate_opts *laopts __attribute__((unused)),
				 struct dm_tree_node *node, uint64_t len,
				 uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	struct lv_segment *cache_pool_seg;
	char *metadata_uuid, *data_uuid, *origin_uuid;
	uint64_t feature_flags = 0;
	unsigned attr;

	if (!seg->pool_lv || !seg_is_cache(seg)) {
		log_error(INTERNAL_ERROR "Passed segment is not cache.");
		return 0;
	}

	cache_pool_seg = first_seg(seg->pool_lv);
	if (seg->cleaner_policy)
		/* With cleaner policy always pass writethrough */
		feature_flags |= DM_CACHE_FEATURE_WRITETHROUGH;
	else
		switch (cache_pool_seg->cache_mode) {
		default:
			log_error(INTERNAL_ERROR "LV %s has unknown cache mode %d.",
				  display_lvname(seg->lv), cache_pool_seg->cache_mode);
			/* Fall through */
		case CACHE_MODE_WRITETHROUGH:
			feature_flags |= DM_CACHE_FEATURE_WRITETHROUGH;
			break;
		case CACHE_MODE_WRITEBACK:
			feature_flags |= DM_CACHE_FEATURE_WRITEBACK;
			break;
		case CACHE_MODE_PASSTHROUGH:
			feature_flags |= DM_CACHE_FEATURE_PASSTHROUGH;
			break;
		}

	switch (cache_pool_seg->cache_metadata_format) {
	case CACHE_METADATA_FORMAT_1: break;
	case CACHE_METADATA_FORMAT_2:
		if (!_target_present(cmd, NULL, &attr))
			return_0;

		if (!(attr & CACHE_FEATURE_METADATA2)) {
			log_error("LV %s has metadata format %u unsuported by kernel.",
				  display_lvname(seg->lv), cache_pool_seg->cache_metadata_format);
			return 0;
		}
		feature_flags |= DM_CACHE_FEATURE_METADATA2;
		log_debug_activation("Using metadata2 format for %s.", display_lvname(seg->lv));
		break;
	default:
		log_error(INTERNAL_ERROR "LV %s has unknown metadata format %u.",
			  display_lvname(seg->lv), cache_pool_seg->cache_metadata_format);
		return 0;
	}

	if (!(metadata_uuid = build_dm_uuid(mem, cache_pool_seg->metadata_lv, NULL)))
		return_0;

	if (!(data_uuid = build_dm_uuid(mem, seg_lv(cache_pool_seg, 0), NULL)))
		return_0;

	if (!(origin_uuid = build_dm_uuid(mem, seg_lv(seg, 0), NULL)))
		return_0;

	if (!dm_tree_node_add_cache_target(node, len,
					   feature_flags,
					   metadata_uuid,
					   data_uuid,
					   origin_uuid,
					   seg->cleaner_policy ? "cleaner" :
						   /* undefined policy name -> likely an old "mq" */
						   cache_pool_seg->policy_name ? : "mq",
					   seg->cleaner_policy ? NULL : cache_pool_seg->policy_settings,
					   cache_pool_seg->chunk_size))
		return_0;

	return 1;
}
#endif /* DEVMAPPER_SUPPORT */

static struct segtype_handler _cache_ops = {
	.display = _cache_display,
	.text_import = _cache_text_import,
	.text_import_area_count = _cache_text_import_area_count,
	.text_export = _cache_text_export,
#ifdef DEVMAPPER_SUPPORT
	.add_target_line = _cache_add_target_line,
	.target_present = _target_present,
	.modules_needed = _modules_needed,
#  ifdef DMEVENTD
#  endif        /* DMEVENTD */
#endif
	.destroy = _destroy,
};

#ifdef CACHE_INTERNAL /* Shared */
int init_cache_segtypes(struct cmd_context *cmd,
			struct segtype_library *seglib)
#else
int init_cache_segtypes(struct cmd_context *cmd,
			struct segtype_library *seglib);
int init_cache_segtypes(struct cmd_context *cmd,
			struct segtype_library *seglib)
#endif
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype) {
		log_error("Failed to allocate memory for cache_pool segtype");
		return 0;
	}

	segtype->name = SEG_TYPE_NAME_CACHE_POOL;
	segtype->flags = SEG_CACHE_POOL | SEG_CANNOT_BE_ZEROED | SEG_ONLY_EXCLUSIVE;
	segtype->ops = &_cache_pool_ops;

	if (!lvm_register_segtype(seglib, segtype))
		return_0;
	log_very_verbose("Initialised segtype: %s", segtype->name);

	segtype = zalloc(sizeof(*segtype));
	if (!segtype) {
		log_error("Failed to allocate memory for cache segtype");
		return 0;
	}

	segtype->name = SEG_TYPE_NAME_CACHE;
	segtype->flags = SEG_CACHE | SEG_ONLY_EXCLUSIVE;
	segtype->ops = &_cache_ops;

	if (!lvm_register_segtype(seglib, segtype))
		return_0;
	log_very_verbose("Initialised segtype: %s", segtype->name);

	/* Reset mask for recalc */
	_feature_mask = 0;

	return 1;
}
