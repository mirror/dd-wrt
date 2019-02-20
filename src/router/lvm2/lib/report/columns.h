/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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
 * This file defines the fields (columns) for the reporting commands
 * (pvs/vgs/lvs).
 *
 * The preferred order of the field descriptions in the help text
 * determines the order the entries appear in this file.
 *
 * When adding new entries take care to use the existing style.
 *
 * Do not interleave fields from different report types - for example,
 * if you have a field of type "LVS" add it in between "LVS type fields"
 * and "End of LVS type fields" comment. If you interleaved fields of
 * different types here in this file, they would end up interleaved in
 * the <reporting_command> -o help output too which may be confusing
 * for users.
 *
 * Displayed fields names normally have a type prefix and use underscores.
 *
 * Field-specific internal functions names normally match the displayed
 * field names but without underscores.
 *
 * Help text ends with a full stop.
 */

/*
 * FIELD(report_object_type, structure, sort_type, heading, structure_field, output_width, reporting_function, field_id, description, settable_via_lib)
 */

/* *INDENT-OFF* */
/*
 * LVS type fields
 */
FIELD(LVS, lv, STR, "LV UUID", lvid, 38, lvuuid, lv_uuid, "Unique identifier.", 0)
FIELD(LVS, lv, STR, "LV", lvid, 4, lvname, lv_name, "Name.  LVs created for internal use are enclosed in brackets.", 0)
FIELD(LVS, lv, STR, "LV", lvid, 4, lvfullname, lv_full_name, "Full name of LV including its VG, namely VG/LV.", 0)
FIELD(LVS, lv, STR, "Path", lvid, 0, lvpath, lv_path, "Full pathname for LV. Blank for internal LVs.", 0)
FIELD(LVS, lv, STR, "DMPath", lvid, 0, lvdmpath, lv_dm_path, "Internal device-mapper pathname for LV (in /dev/mapper directory).", 0)
FIELD(LVS, lv, STR, "Parent", lvid, 0, lvparent, lv_parent, "For LVs that are components of another LV, the parent LV.", 0)
FIELD(LVS, lv, STR_LIST, "Layout", lvid, 10, lvlayout, lv_layout, "LV layout.", 0)
FIELD(LVS, lv, STR_LIST, "Role", lvid, 10, lvrole, lv_role, "LV role.", 0)
FIELD(LVS, lv, BIN, "InitImgSync", lvid, 10, lvinitialimagesync, lv_initial_image_sync, "Set if mirror/RAID images underwent initial resynchronization.", 0)
FIELD(LVS, lv, BIN, "ImgSynced", lvid, 10, lvimagesynced, lv_image_synced, "Set if mirror/RAID image is synchronized.", 0)
FIELD(LVS, lv, BIN, "Merging", lvid, 10, lvmerging, lv_merging, "Set if snapshot LV is being merged to origin.", 0)
FIELD(LVS, lv, BIN, "Converting", lvid, 0, lvconverting, lv_converting, "Set if LV is being converted.", 0)
FIELD(LVS, lv, STR, "AllocPol", lvid, 10, lvallocationpolicy, lv_allocation_policy, "LV allocation policy.", 0)
FIELD(LVS, lv, BIN, "AllocLock", lvid, 10, lvallocationlocked, lv_allocation_locked, "Set if LV is locked against allocation changes.", 0)
FIELD(LVS, lv, BIN, "FixMin", lvid, 10, lvfixedminor, lv_fixed_minor, "Set if LV has fixed minor number assigned.", 0)
FIELD(LVS, lv, BIN, "SkipAct", lvid, 15, lvskipactivation, lv_skip_activation, "Set if LV is skipped on activation.", 0)
FIELD(LVS, lv, STR, "WhenFull", lvid, 15, lvwhenfull, lv_when_full, "For thin pools, behavior when full.", 0)
FIELD(LVS, lv, STR, "Active", lvid, 0, lvactive, lv_active, "Active state of the LV.", 0)
FIELD(LVS, lv, BIN, "ActLocal", lvid, 10, lvactivelocally, lv_active_locally, "Set if the LV is active locally.", 0)
FIELD(LVS, lv, BIN, "ActRemote", lvid, 10, lvactiveremotely, lv_active_remotely, "Set if the LV is active remotely.", 0)
FIELD(LVS, lv, BIN, "ActExcl", lvid, 10, lvactiveexclusively, lv_active_exclusively, "Set if the LV is active exclusively.", 0)
FIELD(LVS, lv, SNUM, "Maj", major, 0, int32, lv_major, "Persistent major number or -1 if not persistent.", 0)
FIELD(LVS, lv, SNUM, "Min", minor, 0, int32, lv_minor, "Persistent minor number or -1 if not persistent.", 0)
FIELD(LVS, lv, SIZ, "Rahead", lvid, 0, lvreadahead, lv_read_ahead, "Read ahead setting in current units.", 0)
FIELD(LVS, lv, SIZ, "LSize", lvid, 0, lv_size, lv_size, "Size of LV in current units.", 0)
FIELD(LVS, lv, SIZ, "MSize", lvid, 0, lvmetadatasize, lv_metadata_size, "For thin and cache pools, the size of the LV that holds the metadata.", 0)
FIELD(LVS, lv, NUM, "#Seg", lvid, 0, lvsegcount, seg_count, "Number of segments in LV.", 0)
FIELD(LVS, lv, STR, "Origin", lvid, 0, origin, origin, "For snapshots and thins, the origin device of this LV.", 0)
FIELD(LVS, lv, STR, "Origin UUID", lvid, 38, originuuid, origin_uuid, "For snapshots and thins, the UUID of origin device of this LV.", 0)
FIELD(LVS, lv, SIZ, "OSize", lvid, 0, originsize, origin_size, "For snapshots, the size of the origin device of this LV.", 0)
FIELD(LVS, lv, STR_LIST, "Ancestors", lvid, 0, lvancestors, lv_ancestors, "LV ancestors ignoring any stored history of the ancestry chain.", 0)
FIELD(LVS, lv, STR_LIST, "FAncestors", lvid, 0, lvfullancestors, lv_full_ancestors, "LV ancestors including stored history of the ancestry chain.", 0)
FIELD(LVS, lv, STR_LIST, "Descendants", lvid, 0, lvdescendants, lv_descendants, "LV descendants ignoring any stored history of the ancestry chain.", 0)
FIELD(LVS, lv, STR_LIST, "FDescendants", lvid, 0, lvfulldescendants, lv_full_descendants, "LV descendants including stored history of the ancestry chain.", 0)
FIELD(LVS, lv, NUM, "Mismatches", lvid, 0, raidmismatchcount, raid_mismatch_count, "For RAID, number of mismatches found or repaired.", 0)
FIELD(LVS, lv, STR, "SyncAction", lvid, 0, raidsyncaction, raid_sync_action, "For RAID, the current synchronization action being performed.", 0)
FIELD(LVS, lv, NUM, "WBehind", lvid, 0, raidwritebehind, raid_write_behind, "For RAID1, the number of outstanding writes allowed to writemostly devices.", 0)
FIELD(LVS, lv, NUM, "MinSync", lvid, 0, raidminrecoveryrate, raid_min_recovery_rate, "For RAID1, the minimum recovery I/O load in kiB/sec/disk.", 0)
FIELD(LVS, lv, NUM, "MaxSync", lvid, 0, raidmaxrecoveryrate, raid_max_recovery_rate, "For RAID1, the maximum recovery I/O load in kiB/sec/disk.", 0)
FIELD(LVS, lv, STR, "Move", lvid, 0, movepv, move_pv, "For pvmove, Source PV of temporary LV created by pvmove.", 0)
FIELD(LVS, lv, STR, "Move UUID", lvid, 38, movepvuuid, move_pv_uuid, "For pvmove, the UUID of Source PV of temporary LV created by pvmove.", 0)
FIELD(LVS, lv, STR, "Convert", lvid, 0, convertlv, convert_lv, "For lvconvert, Name of temporary LV created by lvconvert.", 0)
FIELD(LVS, lv, STR, "Convert UUID", lvid, 38, convertlvuuid, convert_lv_uuid, "For lvconvert, UUID of temporary LV created by lvconvert.", 0)
FIELD(LVS, lv, STR, "Log", lvid, 0, loglv, mirror_log, "For mirrors, the LV holding the synchronisation log.", 0)
FIELD(LVS, lv, STR, "Log UUID", lvid, 38, loglvuuid, mirror_log_uuid, "For mirrors, the UUID of the LV holding the synchronisation log.", 0)
FIELD(LVS, lv, STR, "Data", lvid, 0, datalv, data_lv, "For thin and cache pools, the LV holding the associated data.", 0)
FIELD(LVS, lv, STR, "Data UUID", lvid, 38, datalvuuid, data_lv_uuid, "For thin and cache pools, the UUID of the LV holding the associated data.", 0)
FIELD(LVS, lv, STR, "Meta", lvid, 0, metadatalv, metadata_lv, "For thin and cache pools, the LV holding the associated metadata.", 0)
FIELD(LVS, lv, STR, "Meta UUID", lvid, 38, metadatalvuuid, metadata_lv_uuid, "For thin and cache pools, the UUID of the LV holding the associated metadata.", 0)
FIELD(LVS, lv, STR, "Pool", lvid, 0, poollv, pool_lv, "For thin volumes, the thin pool LV for this volume.", 0)
FIELD(LVS, lv, STR, "Pool UUID", lvid, 38, poollvuuid, pool_lv_uuid, "For thin volumes, the UUID of the thin pool LV for this volume.", 0)
FIELD(LVS, lv, STR_LIST, "LV Tags", tags, 0, tags, lv_tags, "Tags, if any.", 0)
FIELD(LVS, lv, STR, "LProfile", lvid, 0, lvprofile, lv_profile, "Configuration profile attached to this LV.", 0)
FIELD(LVS, lv, STR, "LLockArgs", lvid, 0, lvlockargs, lv_lockargs, "Lock args of the LV used by lvmlockd.", 0)
FIELD(LVS, lv, TIM, "CTime", lvid, 26, lvtime, lv_time, "Creation time of the LV, if known", 0)
FIELD(LVS, lv, TIM, "RTime", lvid, 26, lvtimeremoved, lv_time_removed, "Removal time of the LV, if known", 0)
FIELD(LVS, lv, STR, "Host", lvid, 10, lvhost, lv_host, "Creation host of the LV, if known.", 0)
FIELD(LVS, lv, STR_LIST, "Modules", lvid, 0, modules, lv_modules, "Kernel device-mapper modules required for this LV.", 0)
FIELD(LVS, lv, BIN, "Historical", lvid, 0, lvhistorical, lv_historical, "Set if the LV is historical.", 0)
/*
 * End of LVS type fields
 */

/*
 * LVSINFO type fields
 */
FIELD(LVSINFO, lv, SNUM, "KMaj", lvid, 0, lvkmaj, lv_kernel_major, "Currently assigned major number or -1 if LV is not active.", 0)
FIELD(LVSINFO, lv, SNUM, "KMin", lvid, 0, lvkmin, lv_kernel_minor, "Currently assigned minor number or -1 if LV is not active.", 0)
FIELD(LVSINFO, lv, SIZ, "KRahead", lvid, 0, lvkreadahead, lv_kernel_read_ahead, "Currently-in-use read ahead setting in current units.", 0)
FIELD(LVSINFO, lv, STR, "LPerms", lvid, 8, lvpermissions, lv_permissions, "LV permissions.", 0)
FIELD(LVSINFO, lv, BIN, "Suspended", lvid, 10, lvsuspended, lv_suspended, "Set if LV is suspended.", 0)
FIELD(LVSINFO, lv, BIN, "LiveTable", lvid, 20, lvlivetable, lv_live_table, "Set if LV has live table present.", 0)
FIELD(LVSINFO, lv, BIN, "InactiveTable", lvid, 20, lvinactivetable, lv_inactive_table, "Set if LV has inactive table present.", 0)
FIELD(LVSINFO, lv, BIN, "DevOpen", lvid, 10, lvdeviceopen, lv_device_open, "Set if LV device is open.", 0)
/*
 * End of LVSINFO type fields
 */

/*
 * LVSSTATUS type fields
 */
FIELD(LVSSTATUS, lv, PCT, "Data%", lvid, 6, datapercent, data_percent, "For snapshot, cache and thin pools and volumes, the percentage full if LV is active.", 0)
FIELD(LVSSTATUS, lv, PCT, "Snap%", lvid, 6, snpercent, snap_percent, "For snapshots, the percentage full if LV is active.", 0)
FIELD(LVSSTATUS, lv, PCT, "Meta%", lvid, 6, metadatapercent, metadata_percent, "For cache and thin pools, the percentage of metadata full if LV is active.", 0)
FIELD(LVSSTATUS, lv, PCT, "Cpy%Sync", lvid, 0, copypercent, copy_percent, "For Cache, RAID, mirrors and pvmove, current percentage in-sync.", 0)
FIELD(LVSSTATUS, lv, PCT, "Cpy%Sync", lvid, 0, copypercent, sync_percent, "For Cache, RAID, mirrors and pvmove, current percentage in-sync.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheTotalBlocks", lvid, 0, cache_total_blocks, cache_total_blocks, "Total cache blocks.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheUsedBlocks", lvid, 16, cache_used_blocks, cache_used_blocks, "Used cache blocks.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheDirtyBlocks", lvid, 0, cache_dirty_blocks, cache_dirty_blocks, "Dirty cache blocks.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheReadHits", lvid, 16, cache_read_hits, cache_read_hits, "Cache read hits.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheReadMisses", lvid, 16, cache_read_misses, cache_read_misses, "Cache read misses.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheWriteHits", lvid, 16, cache_write_hits, cache_write_hits, "Cache write hits.", 0)
FIELD(LVSSTATUS, lv, NUM, "CacheWriteMisses", lvid, 0, cache_write_misses, cache_write_misses, "Cache write misses.", 0)
FIELD(LVSSTATUS, lv, STR_LIST, "KCacheSettings", lvid, 18, kernel_cache_settings, kernel_cache_settings, "Cache settings/parameters as set in kernel, including default values (cached segments only).", 0)
FIELD(LVSSTATUS, lv, STR, "KCachePolicy", lvid, 18, kernel_cache_policy, kernel_cache_policy, "Cache policy used in kernel.", 0)
FIELD(LVSSTATUS, lv, NUM, "KMFmt", lvid, 0, kernelmetadataformat, kernel_metadata_format, "Cache metadata format used in kernel.", 0)
FIELD(LVSSTATUS, lv, STR, "Health", lvid, 15, lvhealthstatus, lv_health_status, "LV health status.", 0)
FIELD(LVSSTATUS, lv, STR, "KDiscards", lvid, 0, kdiscards, kernel_discards, "For thin pools, how discards are handled in kernel.", 0)
FIELD(LVSSTATUS, lv, BIN, "CheckNeeded", lvid, 15, lvcheckneeded, lv_check_needed, "For thin pools and cache volumes, whether metadata check is needed.", 0)
FIELD(LVSSTATUS, lv, BIN, "MergeFailed", lvid, 15, lvmergefailed, lv_merge_failed, "Set if snapshot merge failed.", 0)
FIELD(LVSSTATUS, lv, BIN, "SnapInvalid", lvid, 15, lvsnapshotinvalid, lv_snapshot_invalid, "Set if snapshot LV is invalid.", 0)
/*
 * End of LVSSTATUS type fields
 */

/*
 * LVSINFOSTATUS type fields
 */
FIELD(LVSINFOSTATUS, lv, STR, "Attr", lvid, 0, lvstatus, lv_attr, "Various attributes - see man page.", 0)
/*
 * End of LVSINFOSTATUS type fields
 */

/*
 * LABEL type fields
 */
FIELD(LABEL, label, STR, "Fmt", type, 0, pvfmt, pv_fmt, "Type of metadata.", 0)
FIELD(LABEL, label, STR, "PV UUID", type, 38, pvuuid, pv_uuid, "Unique identifier.", 0)
FIELD(LABEL, label, SIZ, "DevSize", dev, 0, devsize, dev_size, "Size of underlying device in current units.", 0)
FIELD(LABEL, label, STR, "PV", dev, 10, dev_name, pv_name, "Name.", 0)
FIELD(LABEL, label, STR, "Maj", dev, 0, devmajor, pv_major, "Device major number.", 0)
FIELD(LABEL, label, STR, "Min", dev, 0, devminor, pv_minor, "Device minor number.", 0)
FIELD(LABEL, label, SIZ, "PMdaFree", type, 9, pvmdafree, pv_mda_free, "Free metadata area space on this device in current units.", 0)
FIELD(LABEL, label, SIZ, "PMdaSize", type, 9, pvmdasize, pv_mda_size, "Size of smallest metadata area on this device in current units.", 0)
FIELD(LABEL, label, NUM, "PExtVsn", type, 0, pvextvsn, pv_ext_vsn, "PV header extension version.", 0)
/*
 * End of LABEL type fields
 */

/*
 * PVS type fields
 */
FIELD(PVS, pv, NUM, "1st PE", pe_start, 7, size64, pe_start, "Offset to the start of data on the underlying device.", 0)
FIELD(PVS, pv, SIZ, "PSize", id, 0, pvsize, pv_size, "Size of PV in current units.", 0)
FIELD(PVS, pv, SIZ, "PFree", id, 0, pvfree, pv_free, "Total amount of unallocated space in current units.", 0)
FIELD(PVS, pv, SIZ, "Used", id, 0, pvused, pv_used, "Total amount of allocated space in current units.", 0)
FIELD(PVS, pv, STR, "Attr", id, 0, pvstatus, pv_attr, "Various attributes - see man page.", 0)
FIELD(PVS, pv, BIN, "Allocatable", id, 0, pvallocatable, pv_allocatable, "Set if this device can be used for allocation.", 0)
FIELD(PVS, pv, BIN, "Exported", id, 10, pvexported, pv_exported, "Set if this device is exported.", 0)
FIELD(PVS, pv, BIN, "Missing", id, 10, pvmissing, pv_missing, "Set if this device is missing in system.", 0)
FIELD(PVS, pv, NUM, "PE", pe_count, 3, uint32, pv_pe_count, "Total number of Physical Extents.", 0)
FIELD(PVS, pv, NUM, "Alloc", pe_alloc_count, 0, uint32, pv_pe_alloc_count, "Total number of allocated Physical Extents.", 0)
FIELD(PVS, pv, STR_LIST, "PV Tags", tags, 0, tags, pv_tags, "Tags, if any.", 0)
FIELD(PVS, pv, NUM, "#PMda", id, 0, pvmdas, pv_mda_count, "Number of metadata areas on this device.", 0)
FIELD(PVS, pv, NUM, "#PMdaUse", id, 0, pvmdasused, pv_mda_used_count, "Number of metadata areas in use on this device.", 0)
FIELD(PVS, pv, SIZ, "BA Start", ba_start, 0, size64, pv_ba_start, "Offset to the start of PV Bootloader Area on the underlying device in current units.", 0)
FIELD(PVS, pv, SIZ, "BA Size", ba_size, 0, size64, pv_ba_size, "Size of PV Bootloader Area in current units.", 0)
FIELD(PVS, pv, BIN, "PInUse", id, 0, pvinuse, pv_in_use, "Set if PV is used.", 0)
FIELD(PVS, pv, BIN, "Duplicate", id, 0, pvduplicate, pv_duplicate, "Set if PV is an unchosen duplicate.", 0)
/*
 * End of PVS type fields
 */

/*
 * VGS type fields
 */
FIELD(VGS, vg, STR, "Fmt", cmd, 0, vgfmt, vg_fmt, "Type of metadata.", 0)
FIELD(VGS, vg, STR, "VG UUID", id, 38, uuid, vg_uuid, "Unique identifier.", 0)
FIELD(VGS, vg, STR, "VG", name, 0, string, vg_name, "Name.", 0)
FIELD(VGS, vg, STR, "Attr", cmd, 5, vgstatus, vg_attr, "Various attributes - see man page.", 0)
FIELD(VGS, vg, STR, "VPerms", cmd, 10, vgpermissions, vg_permissions, "VG permissions.", 0)
FIELD(VGS, vg, BIN, "Extendable", cmd, 0, vgextendable, vg_extendable, "Set if VG is extendable.", 0)
FIELD(VGS, vg, BIN, "Exported", cmd, 10, vgexported, vg_exported, "Set if VG is exported.", 0)
FIELD(VGS, vg, BIN, "Partial", cmd, 10, vgpartial, vg_partial, "Set if VG is partial.", 0)
FIELD(VGS, vg, STR, "AllocPol", cmd, 10, vgallocationpolicy, vg_allocation_policy, "VG allocation policy.", 0)
FIELD(VGS, vg, BIN, "Clustered", cmd, 10, vgclustered, vg_clustered, "Set if VG is clustered.", 0)
FIELD(VGS, vg, BIN, "Shared", cmd, 7, vgshared, vg_shared, "Set if VG is shared.", 0)
FIELD(VGS, vg, SIZ, "VSize", cmd, 0, vgsize, vg_size, "Total size of VG in current units.", 0)
FIELD(VGS, vg, SIZ, "VFree", cmd, 0, vgfree, vg_free, "Total amount of free space in current units.", 0)
FIELD(VGS, vg, STR, "SYS ID", cmd, 0, vgsystemid, vg_sysid, "System ID of the VG indicating which host owns it.", 0)
FIELD(VGS, vg, STR, "System ID", cmd, 0, vgsystemid, vg_systemid, "System ID of the VG indicating which host owns it.", 0)
FIELD(VGS, vg, STR, "LockType", cmd, 0, vglocktype, vg_lock_type, "Lock type of the VG used by lvmlockd.", 0)
FIELD(VGS, vg, STR, "VLockArgs", cmd, 0, vglockargs, vg_lock_args, "Lock args of the VG used by lvmlockd.", 0)
FIELD(VGS, vg, SIZ, "Ext", extent_size, 0, size32, vg_extent_size, "Size of Physical Extents in current units.", 0)
FIELD(VGS, vg, NUM, "#Ext", extent_count, 0, uint32, vg_extent_count, "Total number of Physical Extents.", 0)
FIELD(VGS, vg, NUM, "Free", free_count, 0, uint32, vg_free_count, "Total number of unallocated Physical Extents.", 0)
FIELD(VGS, vg, NUM, "MaxLV", max_lv, 0, uint32, max_lv, "Maximum number of LVs allowed in VG or 0 if unlimited.", 0)
FIELD(VGS, vg, NUM, "MaxPV", max_pv, 0, uint32, max_pv, "Maximum number of PVs allowed in VG or 0 if unlimited.", 0)
FIELD(VGS, vg, NUM, "#PV", pv_count, 0, uint32, pv_count, "Number of PVs in VG.", 0)
FIELD(VGS, vg, NUM, "#PV Missing", cmd, 0, vgmissingpvcount, vg_missing_pv_count, "Number of PVs in VG which are missing.", 0)
FIELD(VGS, vg, NUM, "#LV", cmd, 0, lvcount, lv_count, "Number of LVs.", 0)
FIELD(VGS, vg, NUM, "#SN", cmd, 0, snapcount, snap_count, "Number of snapshots.", 0)
FIELD(VGS, vg, NUM, "Seq", seqno, 0, uint32, vg_seqno, "Revision number of internal metadata.  Incremented whenever it changes.", 0)
FIELD(VGS, vg, STR_LIST, "VG Tags", tags, 0, tags, vg_tags, "Tags, if any.", 0)
FIELD(VGS, vg, STR, "VProfile", cmd, 0, vgprofile, vg_profile, "Configuration profile attached to this VG.", 0)
FIELD(VGS, vg, NUM, "#VMda", cmd, 0, vgmdas, vg_mda_count, "Number of metadata areas on this VG.", 0)
FIELD(VGS, vg, NUM, "#VMdaUse", cmd, 0, vgmdasused, vg_mda_used_count, "Number of metadata areas in use on this VG.", 0)
FIELD(VGS, vg, SIZ, "VMdaFree", cmd, 9, vgmdafree, vg_mda_free, "Free metadata area space for this VG in current units.", 0)
FIELD(VGS, vg, SIZ, "VMdaSize", cmd, 9, vgmdasize, vg_mda_size, "Size of smallest metadata area for this VG in current units.", 0)
FIELD(VGS, vg, NUM, "#VMdaCps", cmd, 0, vgmdacopies, vg_mda_copies, "Target number of in use metadata areas in the VG.", 1)
/*
 * End of VGS type fields
 */

/*
 * SEGS type fields
 */
FIELD(SEGS, seg, STR, "Type", list, 0, segtype, segtype, "Type of LV segment.", 0)
FIELD(SEGS, seg, NUM, "#Str", list, 0, seg_stripes, stripes, "Number of stripes or mirror/raid1 legs.", 0)
FIELD(SEGS, seg, NUM, "#DStr", list, 0, seg_data_stripes, data_stripes, "Number of data stripes or mirror/raid1 legs.", 0)
FIELD(SEGS, seg, SIZ, "RSize", list, 0, seg_reshape_len, reshape_len, "Size of out-of-place reshape space in current units.", 0)
FIELD(SEGS, seg, NUM, "RSize", list, 0, seg_reshape_len_le, reshape_len_le, "Size of out-of-place reshape space in logical extents.", 0)
FIELD(SEGS, seg, NUM, "#Cpy", list, 0, seg_data_copies, data_copies, "Number of data copies.", 0)
FIELD(SEGS, seg, NUM, "DOff", list, 0, seg_data_offset, data_offset, "Data offset on each image device.", 0)
FIELD(SEGS, seg, NUM, "NOff", list, 0, seg_new_data_offset, new_data_offset, "New data offset after any reshape on each image device.", 0)
FIELD(SEGS, seg, NUM, "#Par", list, 0, seg_parity_chunks, parity_chunks, "Number of (rotating) parity chunks.", 0)
FIELD(SEGS, seg, SIZ, "Stripe", stripe_size, 0, size32, stripe_size, "For stripes, amount of data placed on one device before switching to the next.", 0)
FIELD(SEGS, seg, SIZ, "Region", region_size, 0, size32, region_size, "For mirrors/raids, the unit of data per leg when synchronizing devices.", 0)
FIELD(SEGS, seg, SIZ, "Chunk", list, 0, chunksize, chunk_size, "For snapshots, the unit of data used when tracking changes.", 0)
FIELD(SEGS, seg, NUM, "#Thins", list, 0, thincount, thin_count, "For thin pools, the number of thin volumes in this pool.", 0)
FIELD(SEGS, seg, STR, "Discards", list, 0, discards, discards, "For thin pools, how discards are handled.", 0)
FIELD(SEGS, seg, NUM, "CMFmt", list, 0, cachemetadataformat, cache_metadata_format, "For cache, metadata format in use.", 0)
FIELD(SEGS, seg, STR, "CacheMode", list, 0, cachemode, cache_mode, "For cache, how writes are cached.", 0)
FIELD(SEGS, seg, BIN, "Zero", list, 0, thinzero, zero, "For thin pools and volumes, if zeroing is enabled.", 0)
FIELD(SEGS, seg, NUM, "TransId", list, 0, transactionid, transaction_id, "For thin pools, the transaction id and creation transaction id for thins.", 0)
FIELD(SEGS, seg, NUM, "ThId", list, 0, thinid, thin_id, "For thin volume, the thin device id.", 0)
FIELD(SEGS, seg, SIZ, "Start", list, 0, segstart, seg_start, "Offset within the LV to the start of the segment in current units.", 0)
FIELD(SEGS, seg, NUM, "Start", list, 0, segstartpe, seg_start_pe, "Offset within the LV to the start of the segment in physical extents.", 0)
FIELD(SEGS, seg, SIZ, "SSize", list, 0, segsize, seg_size, "Size of segment in current units.", 0)
FIELD(SEGS, seg, SIZ, "SSize", list, 0, segsizepe, seg_size_pe, "Size of segment in physical extents.", 0)
FIELD(SEGS, seg, STR_LIST, "Seg Tags", tags, 0, tags, seg_tags, "Tags, if any.", 0)
FIELD(SEGS, seg, STR_LIST, "PE Ranges", list, 0, peranges, seg_pe_ranges, "Ranges of Physical Extents of underlying devices in command line format (deprecated, use seg_le_ranges for common format).", 0)
FIELD(SEGS, seg, STR_LIST, "LE Ranges", list, 0, leranges, seg_le_ranges, "Ranges of Logical Extents of underlying devices in command line format.", 0)
FIELD(SEGS, seg, STR_LIST, "Metadata LE Ranges", list, 0, metadataleranges, seg_metadata_le_ranges, "Ranges of Logical Extents of underlying metadata devices in command line format.", 0)
FIELD(SEGS, seg, STR_LIST, "Devices", list, 0, devices, devices, "Underlying devices used with starting extent numbers.", 0)
FIELD(SEGS, seg, STR_LIST, "Metadata Devs", list, 0, metadatadevices, metadata_devices, "Underlying metadata devices used with starting extent numbers.", 0)
FIELD(SEGS, seg, STR, "Monitor", list, 0, segmonitor, seg_monitor, "Dmeventd monitoring status of the segment.", 0)
FIELD(SEGS, seg, STR, "CachePolicy", list, 0, cache_policy, cache_policy, "The cache policy (cached segments only).", 0)
FIELD(SEGS, seg, STR_LIST, "CacheSettings", list, 0, cache_settings, cache_settings, "Cache settings/parameters (cached segments only).", 0)
/*
 * End of SEGS type fields
 */

/*
 * PVSEGS type fields
 */
FIELD(PVSEGS, pvseg, NUM, "Start", pe, 0, uint32, pvseg_start, "Physical Extent number of start of segment.", 0)
FIELD(PVSEGS, pvseg, NUM, "SSize", len, 0, uint32, pvseg_size, "Number of extents in segment.", 0)
/*
 * End of PVSEGS type fields
 */
/* *INDENT-ON* */
