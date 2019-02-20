

/*
 * LV types used in command definitions.  The type strings are used
 * as LV suffixes, e.g. LV_type or LV_type1_type2.
 *
 * The final NULL arg can be replaced with lv_is_type() functions
 * if the current lv_is_type #defines become functions and are
 * moved to tools.h
 *
 * Until then, the lv_is_type() functions are called indirectly
 * through _lv_is_type().
 */

lvt(LVT_NONE, "", NULL)
lvt(linear_LVT, "linear", NULL)
lvt(striped_LVT, "striped", NULL)
lvt(snapshot_LVT, "snapshot", NULL) /* lv_is_cow, lv_is_thick_snapshot */
lvt(thin_LVT, "thin", NULL)
lvt(thinpool_LVT, "thinpool", NULL)
lvt(cache_LVT, "cache", NULL)
lvt(cachepool_LVT, "cachepool", NULL)
lvt(vdo_LVT, "vdo", NULL)
lvt(vdopool_LVT, "vdopool", NULL)
lvt(vdopooldata_LVT, "vdopooldata", NULL)
lvt(mirror_LVT, "mirror", NULL)
lvt(raid_LVT, "raid", NULL) /* any raid type */
lvt(raid0_LVT, "raid0", NULL)
lvt(raid1_LVT, "raid1", NULL)
lvt(raid4_LVT, "raid4", NULL)
lvt(raid5_LVT, "raid5", NULL)
lvt(raid6_LVT, "raid6", NULL)
lvt(raid10_LVT, "raid10", NULL)
lvt(error_LVT, "error", NULL)
lvt(zero_LVT, "zero", NULL)
lvt(LVT_COUNT, "", NULL)

