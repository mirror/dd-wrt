
/*
 * NULL in the last arg can be replaced with actual
 * calls to the lv_is_prop() function when those
 * become functions (are #define now), take uniform
 * args (e.g. some take cmd others don't), and are
 * exposed in tools.h
 *
 * Until then, the lv_is_prop() functions are
 * called indirectly through _lv_is_prop().
 */

lvp(LVP_NONE, "", NULL) /* enum value 0 means none */
lvp(is_locked_LVP, "lv_is_locked", NULL)
lvp(is_partial_LVP, "lv_is_partial", NULL)
lvp(is_virtual_LVP, "lv_is_virtual", NULL)
lvp(is_merging_LVP, "lv_is_merging", NULL)
lvp(is_merging_origin_LVP, "lv_is_merging_origin", NULL)
lvp(is_converting_LVP, "lv_is_converting", NULL)
lvp(is_external_origin_LVP, "lv_is_external_origin", NULL)
lvp(is_virtual_origin_LVP, "lv_is_virtual_origin", NULL)
lvp(is_not_synced_LVP, "lv_is_not_synced", NULL)
lvp(is_pending_delete_LVP, "lv_is_pending_delete", NULL)
lvp(is_error_when_full_LVP, "lv_is_error_when_full", NULL)
lvp(is_pvmove_LVP, "lv_is_pvmove", NULL)
lvp(is_removed_LVP, "lv_is_removed", NULL)
lvp(is_vg_writable_LVP, "lv_is_vg_writable", NULL)

/* kinds of sub LV */
lvp(is_thinpool_data_LVP, "lv_is_thinpool_data", NULL)
lvp(is_thinpool_metadata_LVP, "lv_is_thinpool_metadata", NULL)
lvp(is_cachepool_data_LVP, "lv_is_cachepool_data", NULL)
lvp(is_cachepool_metadata_LVP, "lv_is_cachepool_metadata", NULL)
lvp(is_mirror_image_LVP, "lv_is_mirror_image", NULL)
lvp(is_mirror_log_LVP, "lv_is_mirror_log", NULL)
lvp(is_raid_image_LVP, "lv_is_raid_image", NULL)
lvp(is_raid_metadata_LVP, "lv_is_raid_metadata", NULL)

/*
 * is_thick_origin should be used instead of is_origin
 * is_thick_snapshot is generally used as LV_snapshot from lv_types.h
 */
lvp(is_origin_LVP, "lv_is_origin", NULL)
lvp(is_thick_origin_LVP, "lv_is_thick_origin", NULL)
lvp(is_thick_snapshot_LVP, "lv_is_thick_snapshot", NULL)
lvp(is_thin_origin_LVP, "lv_is_thin_origin", NULL)
lvp(is_thin_snapshot_LVP, "lv_is_thin_snapshot", NULL)

lvp(is_cache_origin_LVP, "lv_is_cache_origin", NULL)
lvp(is_merging_cow_LVP, "lv_is_merging_cow", NULL)
lvp(is_cow_covering_origin_LVP, "lv_is_cow_covering_origin", NULL)
lvp(is_visible_LVP, "lv_is_visible", NULL)
lvp(is_historical_LVP, "lv_is_historical", NULL)
lvp(is_raid_with_tracking_LVP, "lv_is_raid_with_tracking", NULL)
lvp(LVP_COUNT, "", NULL)

