// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 */

#ifndef _EXFAT_API_H
#define _EXFAT_API_H

#include "config.h"
#include "exfat_fs.h"

/* cache size (in number of sectors)                */
/* (should be an exponential value of 2)            */
#define FAT_CACHE_SIZE          128
#define FAT_CACHE_HASH_SIZE     64
#define BUF_CACHE_SIZE          256
#define BUF_CACHE_HASH_SIZE     64

/* Read-ahead related                                */
/* First config vars. should be pow of 2             */
#define FCACHE_MAX_RA_SIZE	(PAGE_SIZE)
#define DCACHE_MAX_RA_SIZE	(128*1024)

/* type values */
#define TYPE_UNUSED             0x0000
#define TYPE_DELETED            0x0001
#define TYPE_INVALID            0x0002
#define TYPE_CRITICAL_PRI       0x0100
#define TYPE_BITMAP             0x0101
#define TYPE_UPCASE             0x0102
#define TYPE_VOLUME             0x0103
#define TYPE_DIR                0x0104
#define TYPE_FILE               0x011F
#define TYPE_SYMLINK            0x015F
#define TYPE_CRITICAL_SEC       0x0200
#define TYPE_STREAM             0x0201
#define TYPE_EXTEND             0x0202
#define TYPE_ACL                0x0203
#define TYPE_BENIGN_PRI         0x0400
#define TYPE_GUID               0x0401
#define TYPE_PADDING            0x0402
#define TYPE_ACLTAB             0x0403
#define TYPE_BENIGN_SEC         0x0800
#define TYPE_ALL                0x0FFF

/* eio values */
#define EXFAT_EIO_NONE		(0x00000000)
#define EXFAT_EIO_READ		(0x00000001)
#define EXFAT_EIO_WRITE		(0x00000002)
#define EXFAT_EIO_BDI		(0x00000004)

/* modes for volume allocation unit status */
#define VOL_AU_STAT_TOTAL	(0)
#define VOL_AU_STAT_CLEAN	(1)
#define VOL_AU_STAT_FULL	(2)

/* DOS name structure */
typedef struct {
	u8       name[DOS_NAME_LENGTH];
	u8       name_case;
} DOS_NAME_T;

/* unicode name structure */
typedef struct {
	u16      name[MAX_NAME_LENGTH+3]; /* +3 for null and for converting */
	u16      name_hash;
	u8       name_len;
} UNI_NAME_T;

/* should be merged it to DATE_TIME_T */
typedef struct {
	u16      sec;        /* 0 ~ 59               */
	u16      min;        /* 0 ~ 59               */
	u16      hour;       /* 0 ~ 23               */
	u16      day;        /* 1 ~ 31               */
	u16      mon;        /* 1 ~ 12               */
	u16      year;       /* 0 ~ 127 (since 1980) */
} TIMESTAMP_T;


typedef struct {
	u16      Year;
	u16      Month;
	u16      Day;
	u16      Hour;
	u16      Minute;
	u16      Second;
	u16      MilliSecond;
} DATE_TIME_T;

typedef struct {
	u64      Offset;    // start sector number of the partition
	u64      Size;      // in sectors
} PART_INFO_T;

typedef struct {
	u32      SecSize;    // sector size in bytes
	u64      DevSize;    // block device size in sectors
} DEV_INFO_T;

typedef struct {
	u32      ClusterSize;
	u32      NumClusters;
	u32      FreeClusters;
	u32      UsedClusters;
} VOL_INFO_T;

/* directory structure */
typedef struct {
	u32      dir;
	u32      size;
	u8       flags;
} CHAIN_T;

/* hint structure */
typedef struct {
	u32      clu;
	union {
		u32 off;     // cluster offset
		s32 eidx;    // entry index
	};
} HINT_T;

typedef struct {
	spinlock_t cache_lru_lock;
	struct list_head cache_lru;
	s32 nr_caches;
	u32 cache_valid_id;	// for avoiding the race between alloc and free
} EXTENT_T;

/* first empty entry hint information */
typedef struct {
	s32 eidx;		// entry index of a directory
	s32 count;		// count of continuous empty entry
	CHAIN_T cur;		// the cluster that first empty slot exists in
} HINT_FEMP_T;

/* file id structure */
typedef struct {
	CHAIN_T dir;
	s32 entry;
	u32 type;
	u32 attr;
	u32 start_clu;
	u64 size;
	u8  flags;
	u8  reserved[3];	// padding
	u32 version;		// the copy of low 32bit of i_version to check the validation of hint_stat
	s64 rwoffset;		// file offset or dentry index for readdir
	EXTENT_T extent;	// extent cache for a file
	HINT_T	hint_bmap;	// hint for cluster last accessed
	HINT_T	hint_stat;	// hint for entry index we try to lookup next time
	HINT_FEMP_T hint_femp;	// hint for first empty entry
} FILE_ID_T;

typedef struct {
	s8 *lfn;
	s8 *sfn;
	s32 lfnbuf_len;	//usally MAX_UNINAME_BUF_SIZE
	s32 sfnbuf_len; //usally MAX_DOSNAME_BUF_SIZE, used only for vfat, not for exfat
} DENTRY_NAMEBUF_T;

typedef struct {
	u32 Attr;
	u64 Size;
	u32 NumSubdirs;
	DATE_TIME_T CreateTimestamp;
	DATE_TIME_T ModifyTimestamp;
	DATE_TIME_T AccessTimestamp;
	DENTRY_NAMEBUF_T NameBuf;
} DIR_ENTRY_T;

/* cache information */
typedef struct __cache_entry {
	struct __cache_entry *next;
	struct __cache_entry *prev;
	struct {
		struct __cache_entry *next;
		struct __cache_entry *prev;
	} hash;
	u64 sec;
	u32 flag;
	struct buffer_head   *bh;
} cache_ent_t;

typedef struct {
	s32      (*alloc_cluster)(struct super_block *, u32, CHAIN_T *, s32);
	s32      (*free_cluster)(struct super_block *, CHAIN_T *, s32);
	s32      (*count_used_clusters)(struct super_block *, u32 *);
	s32      (*init_dir_entry)(struct super_block *, CHAIN_T *, s32, u32, u32, u64);
	s32      (*init_ext_entry)(struct super_block *, CHAIN_T *, s32, s32, UNI_NAME_T *, DOS_NAME_T *);
	s32      (*find_dir_entry)(struct super_block *, FILE_ID_T *, CHAIN_T *, UNI_NAME_T *, s32, DOS_NAME_T *, u32);
	s32      (*delete_dir_entry)(struct super_block *, CHAIN_T *, s32, s32, s32);
	void     (*get_uniname_from_ext_entry)(struct super_block *, CHAIN_T *, s32, u16 *);
	s32      (*count_ext_entries)(struct super_block *, CHAIN_T *, s32, DENTRY_T *);
	s32      (*calc_num_entries)(UNI_NAME_T *);
	s32      (*check_max_dentries)(FILE_ID_T *);
	u32      (*get_entry_type)(DENTRY_T *);
	void     (*set_entry_type)(DENTRY_T *, u32);
	u32      (*get_entry_attr)(DENTRY_T *);
	void     (*set_entry_attr)(DENTRY_T *, u32);
	u8       (*get_entry_flag)(DENTRY_T *);
	void     (*set_entry_flag)(DENTRY_T *, u8);
	u32      (*get_entry_clu0)(DENTRY_T *);
	void     (*set_entry_clu0)(DENTRY_T *, u32);
	u64      (*get_entry_size)(DENTRY_T *);
	void     (*set_entry_size)(DENTRY_T *, u64);
	void     (*get_entry_time)(DENTRY_T *, TIMESTAMP_T *, u8);
	void     (*set_entry_time)(DENTRY_T *, TIMESTAMP_T *, u8);
	u32      (*get_au_stat)(struct super_block *, s32);
} FS_FUNC_T;

typedef struct __FS_INFO_T {
	s32	 bd_opened;              // opened or not
	u32      vol_id;                 // volume serial number
	u64      num_sectors;            // num of sectors in volume
	u32      num_clusters;           // num of clusters in volume
	u32      cluster_size;           // cluster size in bytes
	u32      cluster_size_bits;
	u32      sect_per_clus;        // cluster size in sectors
	u32      sect_per_clus_bits;
	u64      FAT1_start_sector;      // FAT1 start sector
	u64      FAT2_start_sector;      // FAT2 start sector
	u64      root_start_sector;      // root dir start sector
	u64      data_start_sector;      // data area start sector
	u32      num_FAT_sectors;        // num of FAT sectors
	u32      root_dir;               // root dir cluster
	u32      dentries_in_root;       // num of dentries in root dir
	u32      dentries_per_clu;       // num of dentries per cluster
	u32      vol_flag;               // volume dirty flag
	struct buffer_head *pbr_bh;      // buffer_head of PBR sector

	u32      map_clu;                // allocation bitmap start cluster
	u32      map_sectors;            // num of allocation bitmap sectors
	struct buffer_head **vol_amap;      // allocation bitmap

	u16      **vol_utbl;               // upcase table

	u32      clu_srch_ptr;           // cluster search pointer
	u32      used_clusters;          // number of used clusters

	u32      prev_eio;            // block device operation error flag

	FS_FUNC_T   *fs_func;

	s32       reserved_clusters;  // # of reserved clusters (DA)
	void        *amap;                  // AU Allocation Map

	/* fat cache */
	struct {
		cache_ent_t pool[FAT_CACHE_SIZE];
		cache_ent_t lru_list;
		cache_ent_t hash_list[FAT_CACHE_HASH_SIZE];
	} fcache;

	/* meta cache */
	struct {
		cache_ent_t pool[BUF_CACHE_SIZE];
		cache_ent_t lru_list;
		cache_ent_t keep_list;        // CACHEs in this list will not be kicked by normal lru operations
		cache_ent_t hash_list[BUF_CACHE_HASH_SIZE];
	} dcache;
} FS_INFO_T;

#endif /* _EXFAT_API_H */
