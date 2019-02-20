/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_DEFAULTS_H
#define _LVM_DEFAULTS_H

#include "device_mapper/vdo/vdo_limits.h"

#define DEFAULT_PE_ALIGN 2048
#define DEFAULT_PE_ALIGN_OLD 128

#define DEFAULT_ARCHIVE_ENABLED 1
#define DEFAULT_BACKUP_ENABLED 1

#define DEFAULT_CACHE_FILE_PREFIX ""

#define DEFAULT_ARCHIVE_DAYS 30
#define DEFAULT_ARCHIVE_NUMBER 10

#define DEFAULT_DEV_DIR "/dev"
#define DEFAULT_PROC_DIR "/proc"
#define DEFAULT_SYSTEM_ID_SOURCE "none"
#define DEFAULT_OBTAIN_DEVICE_LIST_FROM_UDEV 1
#define DEFAULT_EXTERNAL_DEVICE_INFO_SOURCE "none"
#define DEFAULT_SYSFS_SCAN 1
#define DEFAULT_MD_COMPONENT_DETECTION 1
#define DEFAULT_FW_RAID_COMPONENT_DETECTION 0
#define DEFAULT_MD_CHUNK_ALIGNMENT 1
#define DEFAULT_IGNORE_LVM_MIRRORS 1
#define DEFAULT_MULTIPATH_COMPONENT_DETECTION 1
#define DEFAULT_IGNORE_SUSPENDED_DEVICES 0
#define DEFAULT_REQUIRE_RESTOREFILE_WITH_UUID 1
#define DEFAULT_DATA_ALIGNMENT_OFFSET_DETECTION 1
#define DEFAULT_DATA_ALIGNMENT_DETECTION 1
#define DEFAULT_ISSUE_DISCARDS 0
#define DEFAULT_PV_MIN_SIZE_KB 2048
#define DEFAULT_ALLOW_CHANGES_WITH_DUPLICATE_PVS 0

#define DEFAULT_LOCKING_LIB "liblvm2clusterlock.so"
#define DEFAULT_ERROR_WHEN_FULL 0
#define DEFAULT_FALLBACK_TO_LOCAL_LOCKING 1
#define DEFAULT_FALLBACK_TO_CLUSTERED_LOCKING 1
#define DEFAULT_WAIT_FOR_LOCKS 1
#define DEFAULT_LVMLOCKD_LOCK_RETRIES 3
#define DEFAULT_LVMETAD_UPDATE_WAIT_TIME 10
#define DEFAULT_PRIORITISE_WRITE_LOCKS 1
#define DEFAULT_USE_MLOCKALL 0
#define DEFAULT_METADATA_READ_ONLY 0
#define DEFAULT_LVDISPLAY_SHOWS_FULL_DEVICE_PATH 0
#define DEFAULT_UNKNOWN_DEVICE_NAME "[unknown]"
#define DEFAULT_USE_AIO 1

#define DEFAULT_SANLOCK_LV_EXTEND_MB 256

#define DEFAULT_MIRRORLOG MIRROR_LOG_DISK
#define DEFAULT_MIRROR_LOG_FAULT_POLICY "allocate"
#define DEFAULT_MIRROR_IMAGE_FAULT_POLICY "remove"
#define DEFAULT_MIRROR_MAX_IMAGES 8 /* limited by kernel DM_KCOPYD_MAX_REGIONS */
/* Limited by kernel failed devices bitfield in superblock (raid4/5/6 MD max 253) */
/*
 * FIXME: Increase these to 64 and further to the MD maximum
 *	  once the SubLVs split and name shift got enhanced
 */
#define DEFAULT_RAID1_MAX_IMAGES 64
#define DEFAULT_RAID_MAX_IMAGES 64
#define DEFAULT_ALLOCATION_STRIPE_ALL_DEVICES 0 /* Don't stripe across all devices if not -i/--stripes given */

#define DEFAULT_RAID_FAULT_POLICY "warn"

#define DEFAULT_DMEVENTD_RAID_LIB "libdevmapper-event-lvm2raid.so"
#define DEFAULT_DMEVENTD_MIRROR_LIB "libdevmapper-event-lvm2mirror.so"
#define DEFAULT_DMEVENTD_SNAPSHOT_LIB "libdevmapper-event-lvm2snapshot.so"
#define DEFAULT_DMEVENTD_THIN_LIB "libdevmapper-event-lvm2thin.so"
#define DEFAULT_DMEVENTD_THIN_COMMAND "lvm lvextend --use-policies"
#define DEFAULT_DMEVENTD_VDO_LIB "libdevmapper-event-lvm2vdo.so"
#define DEFAULT_DMEVENTD_VDO_COMMAND "lvm lvextend --use-policies"
#define DEFAULT_DMEVENTD_MONITOR 1
#define DEFAULT_BACKGROUND_POLLING 1

#ifndef DMEVENTD_PATH
#  define DEFAULT_DMEVENTD_PATH ""
#else
#  define DEFAULT_DMEVENTD_PATH DMEVENTD_PATH
#endif

#ifdef THIN_CHECK_NEEDS_CHECK
#  define DEFAULT_THIN_CHECK_OPTION1 "-q"
#  define DEFAULT_THIN_CHECK_OPTION2 "--clear-needs-check-flag"
#  define DEFAULT_THIN_CHECK_OPTIONS_CONFIG "#S" DEFAULT_THIN_CHECK_OPTION1 "#S" DEFAULT_THIN_CHECK_OPTION2
#else
#  define DEFAULT_THIN_CHECK_OPTION1 "-q"
#  define DEFAULT_THIN_CHECK_OPTION2 ""
#  define DEFAULT_THIN_CHECK_OPTIONS_CONFIG "#S" DEFAULT_THIN_CHECK_OPTION1
#endif

#define DEFAULT_THIN_REPAIR_OPTION1 ""
#define DEFAULT_THIN_REPAIR_OPTIONS_CONFIG "#S" DEFAULT_THIN_REPAIR_OPTION1
#define DEFAULT_THIN_POOL_METADATA_REQUIRE_SEPARATE_PVS 0
#define DEFAULT_THIN_POOL_MAX_METADATA_SIZE (DM_THIN_MAX_METADATA_SIZE / 2)  /* KB */
#define DEFAULT_THIN_POOL_MIN_METADATA_SIZE 2048  /* KB */
#define DEFAULT_THIN_POOL_OPTIMAL_METADATA_SIZE (128 * 1024) /* KB */
#define DEFAULT_THIN_POOL_CHUNK_SIZE_POLICY "generic"
#define DEFAULT_THIN_POOL_CHUNK_SIZE	    64	  /* KB */
#define DEFAULT_THIN_POOL_CHUNK_SIZE_PERFORMANCE 512 /* KB */
#define DEFAULT_THIN_POOL_DISCARDS "passdown"
#define DEFAULT_THIN_POOL_ZERO 1
#define DEFAULT_POOL_METADATA_SPARE 1 /* thin + cache */

#ifdef CACHE_CHECK_NEEDS_CHECK
#  define DEFAULT_CACHE_CHECK_OPTION1 "-q"
#  define DEFAULT_CACHE_CHECK_OPTION2 "--clear-needs-check-flag"
#  define DEFAULT_CACHE_CHECK_OPTIONS_CONFIG "#S" DEFAULT_CACHE_CHECK_OPTION1 "#S" DEFAULT_CACHE_CHECK_OPTION2
#else
#  define DEFAULT_CACHE_CHECK_OPTION1 "-q"
#  define DEFAULT_CACHE_CHECK_OPTION2 ""
#  define DEFAULT_CACHE_CHECK_OPTIONS_CONFIG "#S" DEFAULT_CACHE_CHECK_OPTION1
#endif

#define DEFAULT_CACHE_REPAIR_OPTION1 ""
#define DEFAULT_CACHE_REPAIR_OPTIONS_CONFIG "#S" DEFAULT_CACHE_REPAIR_OPTION1
#define DEFAULT_CACHE_POOL_METADATA_REQUIRE_SEPARATE_PVS 0
#define DEFAULT_CACHE_POOL_CHUNK_SIZE 64 /* KB */
#define DEFAULT_CACHE_POOL_MAX_CHUNKS 1000000
#define DEFAULT_CACHE_POOL_MIN_METADATA_SIZE 2048  /* KB */
#define DEFAULT_CACHE_POOL_MAX_METADATA_SIZE (16 * 1024 * 1024)  /* KB */
#define DEFAULT_CACHE_POLICY "mq"
#define DEFAULT_CACHE_METADATA_FORMAT CACHE_METADATA_FORMAT_UNSELECTED /* Autodetect */
#define DEFAULT_CACHE_MODE "writethrough"


/* VDO defaults */
#define DEFAULT_VDO_USE_COMPRESSION	(true)
#define DEFAULT_VDO_USE_DEDUPLICATION	(true)
#define DEFAULT_VDO_EMULATE_512_SECTORS	(false)
#define DEFAULT_VDO_BLOCK_MAP_CACHE_SIZE_MB	(DM_VDO_BLOCK_MAP_CACHE_SIZE_MINIMUM_MB)
#define DEFAULT_VDO_BLOCK_MAP_PERIOD	(DM_VDO_BLOCK_MAP_PERIOD_MAXIMUM)
#define DEFAULT_VDO_USE_SPARSE_INDEX	(false)
#define DEFAULT_VDO_CHECK_POINT_FREQUENCY	(0)
#define DEFAULT_VDO_INDEX_MEMORY_SIZE_MB	(DM_VDO_INDEX_MEMORY_SIZE_MINIMUM_MB)
#define DEFAULT_VDO_USE_READ_CACHE	(false)
#define DEFAULT_VDO_READ_CACHE_SIZE_MB	(0)
#define DEFAULT_VDO_SLAB_SIZE_MB	(2 * 1024)  // 2GiB ... 19 slabbits
#define DEFAULT_VDO_ACK_THREADS		(1)
#define DEFAULT_VDO_BIO_THREADS		(1)
#define DEFAULT_VDO_BIO_ROTATION	(64)
#define DEFAULT_VDO_CPU_THREADS		(2)
#define DEFAULT_VDO_HASH_ZONE_THREADS	(1)
#define DEFAULT_VDO_LOGICAL_THREADS	(1)
#define DEFAULT_VDO_PHYSICAL_THREADS	(1)
#define DEFAULT_VDO_WRITE_POLICY	"auto"

#define DEFAULT_VDO_FORMAT_OPTIONS_CONFIG "#S" ""
/*
 * VDO pool will reverve some sectors in the front and the back of pool device to avoid
 * seeing same device twice in the system.
 */
#define DEFAULT_VDO_POOL_HEADER_SIZE  (1024)   // 512KiB



#define DEFAULT_FSADM_PATH FSADM_PATH

#define DEFAULT_UMASK 0077

#define DEFAULT_FORMAT "lvm2"

#define DEFAULT_STRIPESIZE 64	/* KB */
#define DEFAULT_RECORD_LVS_HISTORY 0
#define DEFAULT_LVS_HISTORY_RETENTION_TIME 0
#define DEFAULT_PVMETADATAIGNORE 0
#define DEFAULT_PVMETADATASIZE 255
#define DEFAULT_PVMETADATACOPIES 1
#define DEFAULT_VGMETADATACOPIES 0
#define DEFAULT_LABELSECTOR UINT64_C(1)
#define DEFAULT_READ_AHEAD "auto"
#define DEFAULT_UDEV_RULES 1
#define DEFAULT_UDEV_SYNC 1
#define DEFAULT_NOTIFY_DBUS 1
#define DEFAULT_VERIFY_UDEV_OPERATIONS 0
#define DEFAULT_RETRY_DEACTIVATION 1
#define DEFAULT_ACTIVATION_CHECKS 0
#define DEFAULT_EXTENT_SIZE 4096	/* In KB */
#define DEFAULT_MAX_PV 0
#define DEFAULT_MAX_LV 0
#define DEFAULT_ALLOC_POLICY ALLOC_NORMAL
#define DEFAULT_MIRROR_LOGS_REQUIRE_SEPARATE_PVS 0
#define DEFAULT_MAXIMISE_CLING 1
#define DEFAULT_CLUSTERED 0

#define DEFAULT_MSG_PREFIX "  "
#define DEFAULT_CMD_NAME 0
#define DEFAULT_OVERWRITE 0

#ifndef DEFAULT_LOG_FACILITY
#  define DEFAULT_LOG_FACILITY LOG_USER
#endif

#define DEFAULT_COMMAND_LOG_REPORT 0
#define DEFAULT_SYSLOG 1
#define DEFAULT_VERBOSE 0
#define DEFAULT_SILENT 0
#define DEFAULT_LOGLEVEL 0
#define DEFAULT_INDENT 1
#define DEFAULT_ABORT_ON_INTERNAL_ERRORS 0
#define DEFAULT_UNITS "r"
#define DEFAULT_SUFFIX 1
#define DEFAULT_HOSTTAGS 0

#ifndef DEFAULT_SI_UNIT_CONSISTENCY
#  define DEFAULT_SI_UNIT_CONSISTENCY 1
#endif

#ifdef DEVMAPPER_SUPPORT
#  define DEFAULT_ACTIVATION 1
#else
#  define DEFAULT_ACTIVATION 0
#endif

#define DEFAULT_RESERVED_MEMORY 8192
#define DEFAULT_RESERVED_STACK 64 /* KB */
#define DEFAULT_PROCESS_PRIORITY -18

#define DEFAULT_AUTO_SET_ACTIVATION_SKIP 1
#define DEFAULT_ACTIVATION_MODE "degraded"
#define DEFAULT_USE_LINEAR_TARGET 1
#define DEFAULT_STRIPE_FILLER "error"
#define DEFAULT_RAID_REGION_SIZE   2048	/* KB */
#define DEFAULT_INTERVAL 15

#define DEFAULT_MAX_HISTORY 100

#define DEFAULT_REP_COMPACT_OUTPUT 0
#define DEFAULT_REP_ALIGNED 1
#define DEFAULT_REP_BUFFERED 1
#define DEFAULT_REP_COLUMNS_AS_ROWS 0
#define DEFAULT_REP_HEADINGS 1
#define DEFAULT_REP_PREFIXES 0
#define DEFAULT_REP_QUOTED 1
#define DEFAULT_REP_SEPARATOR " "
#define DEFAULT_REP_LIST_ITEM_SEPARATOR ","
#define DEFAULT_TIME_FORMAT "%Y-%m-%d %T %z"

#define DEFAULT_REP_OUTPUT_FORMAT "basic"
#define DEFAULT_COMPACT_OUTPUT_COLS ""

#define DEFAULT_COMMAND_LOG_SELECTION "!(log_type=status && message=success)"

#define DEFAULT_LVS_COLS "lv_name,vg_name,lv_attr,lv_size,pool_lv,origin,data_percent,metadata_percent,move_pv,mirror_log,copy_percent,convert_lv"
#define DEFAULT_VGS_COLS "vg_name,pv_count,lv_count,snap_count,vg_attr,vg_size,vg_free"
#define DEFAULT_PVS_COLS "pv_name,vg_name,pv_fmt,pv_attr,pv_size,pv_free"
#define DEFAULT_SEGS_COLS "lv_name,vg_name,lv_attr,stripes,segtype,seg_size"
#define DEFAULT_PVSEGS_COLS "pv_name,vg_name,pv_fmt,pv_attr,pv_size,pv_free,pvseg_start,pvseg_size"
#define DEFAULT_DEVTYPES_COLS "devtype_name,devtype_max_partitions,devtype_description"
#define DEFAULT_COMMAND_LOG_COLS "log_seq_num,log_type,log_context,log_object_type,log_object_name,log_object_id,log_object_group,log_object_group_id,log_message,log_errno,log_ret_code"

#define DEFAULT_LVS_COLS_VERB "lv_name,vg_name,seg_count,lv_attr,lv_size,lv_major,lv_minor,lv_kernel_major,lv_kernel_minor,pool_lv,origin,data_percent,metadata_percent,move_pv,copy_percent,mirror_log,convert_lv,lv_uuid,lv_profile"
#define DEFAULT_VGS_COLS_VERB "vg_name,vg_attr,vg_extent_size,pv_count,lv_count,snap_count,vg_size,vg_free,vg_uuid,vg_profile"
#define DEFAULT_PVS_COLS_VERB "pv_name,vg_name,pv_fmt,pv_attr,pv_size,pv_free,dev_size,pv_uuid"
#define DEFAULT_SEGS_COLS_VERB "lv_name,vg_name,lv_attr,seg_start,seg_size,stripes,segtype,stripesize,chunksize"
#define DEFAULT_PVSEGS_COLS_VERB "pv_name,vg_name,pv_fmt,pv_attr,pv_size,pv_free,pvseg_start,pvseg_size,lv_name,seg_start_pe,segtype,seg_pe_ranges"
#define DEFAULT_DEVTYPES_COLS_VERB "devtype_name,devtype_max_partitions,devtype_description"

#define DEFAULT_VGS_COLS_FULL "vg_all"
#define DEFAULT_PVS_COLS_FULL "pv_all"
#define DEFAULT_LVS_COLS_FULL "lv_all"
#define DEFAULT_PVSEGS_COLS_FULL "pvseg_all,pv_uuid,lv_uuid"
#define DEFAULT_SEGS_COLS_FULL "seg_all,lv_uuid"

#define DEFAULT_LVS_SORT "vg_name,lv_name"
#define DEFAULT_VGS_SORT "vg_name"
#define DEFAULT_PVS_SORT "pv_name"
#define DEFAULT_SEGS_SORT "vg_name,lv_name,seg_start"
#define DEFAULT_PVSEGS_SORT "pv_name,pvseg_start"
#define DEFAULT_DEVTYPES_SORT "devtype_name"
#define DEFAULT_COMMAND_LOG_SORT "log_seq_num"

#define DEFAULT_VGS_SORT_FULL "vg_name"
#define DEFAULT_PVS_SORT_FULL "pv_name"
#define DEFAULT_LVS_SORT_FULL "vg_name,lv_name"
#define DEFAULT_PVSEGS_SORT_FULL "pv_uuid,pvseg_start"
#define DEFAULT_SEGS_SORT_FULL "lv_uuid,seg_start"

#define DEFAULT_MIRROR_DEVICE_FAULT_POLICY "remove"
#define DEFAULT_MIRROR_LOG_FAULT_POLICY "allocate"
#define DEFAULT_SNAPSHOT_AUTOEXTEND_THRESHOLD 100
#define DEFAULT_SNAPSHOT_AUTOEXTEND_PERCENT 20
#define DEFAULT_THIN_POOL_AUTOEXTEND_THRESHOLD 100
#define DEFAULT_THIN_POOL_AUTOEXTEND_PERCENT 20
#define DEFAULT_VDO_POOL_AUTOEXTEND_THRESHOLD 100
#define DEFAULT_VDO_POOL_AUTOEXTEND_PERCENT 20

#define DEFAULT_SCAN_LVS 1

#endif				/* _LVM_DEFAULTS_H */
