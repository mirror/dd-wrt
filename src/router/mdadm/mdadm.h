/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#define	_GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include	<unistd.h>
#ifdef __GLIBC__
extern __off64_t lseek64 __P ((int __fd, __off64_t __offset, int __whence));
#elif !defined(lseek64)
# if defined(__NO_STAT64) || __WORDSIZE != 32
# define lseek64 lseek
# endif
#endif

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<time.h>
#include	<sys/time.h>
#include	<getopt.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<syslog.h>
#ifdef __GLIBC__
/* Newer glibc requires sys/sysmacros.h directly for makedev() */
#include	<sys/sysmacros.h>
#endif
#ifdef __dietlibc__
#include	<strings.h>
/* dietlibc has deprecated random and srandom!! */
#define random rand
#define srandom srand
#endif

#ifdef NO_COROSYNC
#define CS_OK 1
typedef uint64_t cmap_handle_t;
#else
#include	<corosync/cmap.h>
#endif

#ifndef NO_DLM
#include	<libdlm.h>
#include	<errno.h>
#else
#define LKF_NOQUEUE	0x00000001
#define LKM_PWMODE	4
#define EUNLOCK		0x10002

typedef void *dlm_lshandle_t;

struct dlm_lksb {
	int sb_status;
	uint32_t sb_lkid;
	char sb_flags;
	char *sb_lvbptr;
};
#endif

#include	<linux/kdev_t.h>
/*#include	<linux/fs.h> */
#include	<sys/mount.h>
#include	<asm/types.h>
#include	<sys/ioctl.h>
#define	MD_MAJOR 9
#define MdpMinorShift 6

#ifndef BLKGETSIZE64
#define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size in bytes (u64 *arg) */
#endif

#define DEFAULT_CHUNK 512
#define DEFAULT_BITMAP_CHUNK 4096
#define DEFAULT_BITMAP_DELAY 5
#define DEFAULT_MAX_WRITE_BEHIND 256

/* MAP_DIR should be somewhere that persists across the pivotroot
 * from early boot to late boot.
 * /run  seems to have emerged as the best standard.
 */
#ifndef MAP_DIR
#define MAP_DIR "/run/mdadm"
#endif /* MAP_DIR */
/* MAP_FILE is what we name the map file we put in MAP_DIR, in case you
 * want something other than the default of "map"
 */
#ifndef MAP_FILE
#define MAP_FILE "map"
#endif /* MAP_FILE */
/* MDMON_DIR is where pid and socket files used for communicating
 * with mdmon normally live.  Best is /var/run/mdadm as
 * mdmon is needed at early boot then it needs to write there prior
 * to /var/run being mounted read/write, and it also then needs to
 * persist beyond when /var/run is mounter read-only.  So, to be
 * safe, the default is somewhere that is read/write early in the
 * boot process and stays up as long as possible during shutdown.
 */
#ifndef MDMON_DIR
#define MDMON_DIR "/run/mdadm"
#endif /* MDMON_DIR */

/* FAILED_SLOTS is where to save files storing recent removal of array
 * member in order to allow future reuse of disk inserted in the same
 * slot for array recovery
 */
#ifndef FAILED_SLOTS_DIR
#define FAILED_SLOTS_DIR "/run/mdadm/failed-slots"
#endif /* FAILED_SLOTS */

#include	"md_u.h"
#include	"md_p.h"
#include	"bitmap.h"
#include	"msg.h"

#include <endian.h>
/* Redhat don't like to #include <asm/byteorder.h>, and
 * some time include <linux/byteorder/xxx_endian.h> isn't enough,
 * and there is no standard conversion function so... */
/* And dietlibc doesn't think byteswap is ok, so.. */
/*  #include <byteswap.h> */
#define __mdadm_bswap_16(x) (((x) & 0x00ffU) << 8 | \
			     ((x) & 0xff00U) >> 8)
#define __mdadm_bswap_32(x) (((x) & 0x000000ffU) << 24 | \
			     ((x) & 0xff000000U) >> 24 | \
			     ((x) & 0x0000ff00U) << 8  | \
			     ((x) & 0x00ff0000U) >> 8)
#define __mdadm_bswap_64(x) (((x) & 0x00000000000000ffULL) << 56 | \
			     ((x) & 0xff00000000000000ULL) >> 56 | \
			     ((x) & 0x000000000000ff00ULL) << 40 | \
			     ((x) & 0x00ff000000000000ULL) >> 40 | \
			     ((x) & 0x0000000000ff0000ULL) << 24 | \
			     ((x) & 0x0000ff0000000000ULL) >> 24 | \
			     ((x) & 0x00000000ff000000ULL) << 8 |  \
			     ((x) & 0x000000ff00000000ULL) >> 8)

#if !defined(__KLIBC__)
#if BYTE_ORDER == LITTLE_ENDIAN
#define	__cpu_to_le16(_x) (unsigned int)(_x)
#define __cpu_to_le32(_x) (unsigned int)(_x)
#define __cpu_to_le64(_x) (unsigned long long)(_x)
#define	__le16_to_cpu(_x) (unsigned int)(_x)
#define __le32_to_cpu(_x) (unsigned int)(_x)
#define __le64_to_cpu(_x) (unsigned long long)(_x)

#define	__cpu_to_be16(_x) __mdadm_bswap_16(_x)
#define __cpu_to_be32(_x) __mdadm_bswap_32(_x)
#define __cpu_to_be64(_x) __mdadm_bswap_64(_x)
#define	__be16_to_cpu(_x) __mdadm_bswap_16(_x)
#define __be32_to_cpu(_x) __mdadm_bswap_32(_x)
#define __be64_to_cpu(_x) __mdadm_bswap_64(_x)
#elif BYTE_ORDER == BIG_ENDIAN
#define	__cpu_to_le16(_x) __mdadm_bswap_16(_x)
#define __cpu_to_le32(_x) __mdadm_bswap_32(_x)
#define __cpu_to_le64(_x) __mdadm_bswap_64(_x)
#define	__le16_to_cpu(_x) __mdadm_bswap_16(_x)
#define __le32_to_cpu(_x) __mdadm_bswap_32(_x)
#define __le64_to_cpu(_x) __mdadm_bswap_64(_x)

#define	__cpu_to_be16(_x) (unsigned int)(_x)
#define __cpu_to_be32(_x) (unsigned int)(_x)
#define __cpu_to_be64(_x) (unsigned long long)(_x)
#define	__be16_to_cpu(_x) (unsigned int)(_x)
#define __be32_to_cpu(_x) (unsigned int)(_x)
#define __be64_to_cpu(_x) (unsigned long long)(_x)
#else
#  error "unknown endianness."
#endif
#endif /* __KLIBC__ */

/*
  * Check at compile time that something is of a particular type.
  * Always evaluates to 1 so you may use it easily in comparisons.
*/

#define typecheck(type,x) \
({	   type __dummy; \
	   typeof(x) __dummy2; \
	   (void)(&__dummy == &__dummy2); \
	   1; \
})

/*
 *  These inlines deal with timer wrapping correctly.
 *
 * time_after(a,b) returns true if the time a is after time b.
*/

#define time_after(a,b)	\
        (typecheck(unsigned int, a) && \
         typecheck(unsigned int, b) && \
         ((int)((b) - (a)) < 0))

#define time_before(a,b)        time_after(b,a)

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                            \
	typeof(x) _min1 = (x);                  \
	typeof(y) _min2 = (y);                  \
	(void) (&_min1 == &_min2);              \
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                            \
	typeof(x) _max1 = (x);                  \
	typeof(y) _max2 = (y);                  \
	(void) (&_max1 == &_max2);              \
	_max1 > _max2 ? _max1 : _max2; })

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

extern const char Name[];

struct md_bb_entry {
	unsigned long long sector;
	int length;
};

struct md_bb {
	int supported;
	int count;
	struct md_bb_entry *entries;
};

/* general information that might be extracted from a superblock */
struct mdinfo {
	mdu_array_info_t	array;
	mdu_disk_info_t		disk;
	__u64			events;
	int			uuid[4];
	char			name[33];
	unsigned long long	data_offset;
	unsigned long long	new_data_offset;
	unsigned long long	component_size; /* same as array.size, except in
						 * sectors and up to 64bits.
						 */
	unsigned long long	custom_array_size; /* size for non-default sized
						    * arrays (in sectors)
						    */
#define NO_RESHAPE		0
#define VOLUME_RESHAPE		1
#define CONTAINER_RESHAPE	2
#define RESHAPE_NO_BACKUP	16 /* Mask 'or'ed in */
	int			reshape_active;
	unsigned long long	reshape_progress;
	int			recovery_blocked; /* for external metadata it
						   * indicates that there is
						   * reshape in progress in
						   * container,
						   * for native metadata it is
						   * reshape_active field mirror
						   */
	int journal_device_required;
	int journal_clean;

	enum {
		CONSISTENCY_POLICY_UNKNOWN,
		CONSISTENCY_POLICY_NONE,
		CONSISTENCY_POLICY_RESYNC,
		CONSISTENCY_POLICY_BITMAP,
		CONSISTENCY_POLICY_JOURNAL,
		CONSISTENCY_POLICY_PPL,
	} consistency_policy;

	/* During reshape we can sometimes change the data_offset to avoid
	 * over-writing still-valid data.  We need to know if there is space.
	 * So getinfo_super will fill in space_before and space_after in sectors.
	 * data_offset can be increased or decreased by this amount.
	 */
	unsigned long long	space_before, space_after;
	union {
		unsigned long long resync_start; /* per-array resync position */
		unsigned long long recovery_start; /* per-device rebuild position */
		#define MaxSector  (~0ULL) /* resync/recovery complete position */
	};
	long			bitmap_offset;	/* 0 == none, 1 == a file */
	unsigned int		ppl_size;
	int			ppl_offset;
	unsigned long long	ppl_sector;
	unsigned long		safe_mode_delay; /* ms delay to mark clean */
	int			new_level, delta_disks, new_layout, new_chunk;
	int			errors;
	unsigned long		cache_size; /* size of raid456 stripe cache*/
	int			mismatch_cnt;
	char			text_version[50];

	int container_member; /* for assembling external-metatdata arrays
			       * This is to be used internally by metadata
			       * handler only */
	int container_enough; /* flag external handlers can set to
			       * indicate that subarrays have not enough (-1),
			       * enough to start (0), or all expected disks (1) */
	char		sys_name[32];
	struct mdinfo *devs;
	struct mdinfo *next;

	/* Device info for mdmon: */
	int recovery_fd;
	int state_fd;
	int bb_fd;
	int ubb_fd;
	#define DS_FAULTY	1
	#define	DS_INSYNC	2
	#define	DS_WRITE_MOSTLY	4
	#define	DS_SPARE	8
	#define DS_BLOCKED	16
	#define	DS_REMOVE	1024
	#define	DS_UNBLOCK	2048
	int prev_state, curr_state, next_state;

	/* info read from sysfs */
	enum {
		ARRAY_CLEAR,
		ARRAY_INACTIVE,
		ARRAY_SUSPENDED,
		ARRAY_READONLY,
		ARRAY_READ_AUTO,
		ARRAY_CLEAN,
		ARRAY_ACTIVE,
		ARRAY_WRITE_PENDING,
		ARRAY_ACTIVE_IDLE,
		ARRAY_UNKNOWN_STATE,
	} array_state;
	struct md_bb bb;
};

struct createinfo {
	int	uid;
	int	gid;
	int	autof;
	int	mode;
	int	symlinks;
	int	names;
	int	bblist;
	struct supertype *supertype;
};

struct spare_criteria {
	unsigned long long min_size;
	unsigned int sector_size;
};

enum mode {
	ASSEMBLE=1,
	BUILD,
	CREATE,
	MANAGE,
	MISC,
	MONITOR,
	GROW,
	INCREMENTAL,
	AUTODETECT,
	mode_count
};

extern char short_options[];
extern char short_bitmap_options[];
extern char short_bitmap_auto_options[];
extern struct option long_options[];
extern char Version[], Usage[], Help[], OptionHelp[],
	*mode_help[],
	Help_create[], Help_build[], Help_assemble[], Help_grow[],
	Help_incr[],
	Help_manage[], Help_misc[], Help_monitor[], Help_config[];

/* for option that don't have short equivilents, we assign arbitrary
 * numbers later than any 'short' character option.
 */
enum special_options {
	AssumeClean = 300,
	BitmapChunk,
	WriteBehind,
	ReAdd,
	NoDegraded,
	Sparc22,
	BackupFile,
	HomeHost,
	AutoHomeHost,
	Symlinks,
	AutoDetect,
	Waitclean,
	DetailPlatform,
	KillSubarray,
	UpdateSubarray,
	IncrementalPath,
	NoSharing,
	HelpOptions,
	Brief,
	ManageOpt,
	Add,
	AddSpare,
	AddJournal,
	Remove,
	Fail,
	Replace,
	With,
	MiscOpt,
	WaitOpt,
	ConfigFile,
	ChunkSize,
	WriteMostly,
	FailFast,
	NoFailFast,
	Layout,
	Auto,
	Force,
	SuperMinor,
	EMail,
	ProgramOpt,
	Increment,
	Fork,
	Bitmap,
	RebuildMapOpt,
	InvalidBackup,
	UdevRules,
	FreezeReshape,
	Continue,
	OffRootOpt,
	Prefer,
	KillOpt,
	DataOffset,
	ExamineBB,
	Dump,
	Restore,
	Action,
	Nodes,
	ClusterName,
	ClusterConfirm,
	WriteJournal,
	ConsistencyPolicy,
};

enum prefix_standard {
	JEDEC,
	IEC
};

enum bitmap_update {
    NoUpdate,
    NameUpdate,
    NodeNumUpdate,
};

enum flag_mode {
	FlagDefault, FlagSet, FlagClear,
};

/* structures read from config file */
/* List of mddevice names and identifiers
 * Identifiers can be:
 *    uuid=128-hex-uuid
 *    super-minor=decimal-minor-number-from-superblock
 *    devices=comma,separated,list,of,device,names,with,wildcards
 *
 * If multiple fields are present, the intersection of all matching
 * devices is considered
 */
#define UnSet (0xfffe)
struct mddev_ident {
	char	*devname;

	int	uuid_set;
	int	uuid[4];
	char	name[33];

	int super_minor;

	char	*devices;	/* comma separated list of device
				 * names with wild cards
				 */
	int	level;
	int raid_disks;
	int spare_disks;
	struct supertype *st;
	int	autof;		/* 1 for normal, 2 for partitioned */
	char	*spare_group;
	char	*bitmap_file;
	int	bitmap_fd;

	char	*container;	/* /dev/whatever name of container, or
				 * uuid of container.  You would expect
				 * this to be the 'devname' or UUID
				 * of some other entry.
				 */
	char	*member;	/* subarray within a container */

	struct mddev_ident *next;
	union {
		/* fields needed by different users of this structure */
		int assembled;	/* set when assembly succeeds */
	};
};

struct context {
	int	readonly;
	int	runstop;
	int	verbose;
	int	brief;
	int	force;
	char	*homehost;
	int	require_homehost;
	char	*prefer;
	int	export;
	int	test;
	char	*subarray;
	char	*update;
	int	scan;
	int	SparcAdjust;
	int	autof;
	int	delay;
	int	freeze_reshape;
	char	*backup_file;
	int	invalid_backup;
	char	*action;
	int	nodes;
	char	*homecluster;
};

struct shape {
	int	raiddisks;
	int	sparedisks;
	int	journaldisks;
	int	level;
	int	layout;
	char	*layout_str;
	int	chunk;
	int	bitmap_chunk;
	char	*bitmap_file;
	int	assume_clean;
	int	write_behind;
	unsigned long long size;
	int	consistency_policy;
};

/* List of device names - wildcards expanded */
struct mddev_dev {
	char *devname;
	int disposition;	/* 'a' for add, 'r' for remove, 'f' for fail,
				 * 'A' for re_add.
				 * Not set for names read from .config
				 */
	enum flag_mode writemostly;
	enum flag_mode failfast;
	int used;		/* set when used */
	long long data_offset;
	struct mddev_dev *next;
};

typedef struct mapping {
	char *name;
	int num;
} mapping_t;

struct mdstat_ent {
	char		devnm[32];
	int		active;
	char		*level;
	char		*pattern; /* U for up, _ for down */
	int		percent; /* -1 if no resync */
	int		resync; /* 3 if check, 2 if reshape, 1 if resync, 0 if recovery */
	int		devcnt;
	int		raid_disks;
	char *		metadata_version;
	struct dev_member {
		char			*name;
		struct dev_member	*next;
	}		*members;
	struct mdstat_ent *next;
};

extern struct mdstat_ent *mdstat_read(int hold, int start);
extern void mdstat_close(void);
extern void free_mdstat(struct mdstat_ent *ms);
extern void mdstat_wait(int seconds);
extern void mdstat_wait_fd(int fd, const sigset_t *sigmask);
extern int mddev_busy(char *devnm);
extern struct mdstat_ent *mdstat_by_component(char *name);
extern struct mdstat_ent *mdstat_by_subdev(char *subdev, char *container);

struct map_ent {
	struct map_ent *next;
	char	devnm[32];
	char	metadata[20];
	int	uuid[4];
	int	bad;
	char	*path;
};
extern int map_update(struct map_ent **mpp, char *devnm, char *metadata,
		      int uuid[4], char *path);
extern void map_remove(struct map_ent **map, char *devnm);
extern struct map_ent *map_by_uuid(struct map_ent **map, int uuid[4]);
extern struct map_ent *map_by_devnm(struct map_ent **map, char *devnm);
extern void map_free(struct map_ent *map);
extern struct map_ent *map_by_name(struct map_ent **map, char *name);
extern void map_read(struct map_ent **melp);
extern int map_write(struct map_ent *mel);
extern void map_delete(struct map_ent **mapp, char *devnm);
extern void map_add(struct map_ent **melp,
		    char *devnm, char *metadata, int uuid[4], char *path);
extern int map_lock(struct map_ent **melp);
extern void map_unlock(struct map_ent **melp);
extern void map_fork(void);

/* various details can be requested */
enum sysfs_read_flags {
	GET_LEVEL	= (1 << 0),
	GET_LAYOUT	= (1 << 1),
	GET_COMPONENT	= (1 << 2),
	GET_CHUNK	= (1 << 3),
	GET_CACHE	= (1 << 4),
	GET_MISMATCH	= (1 << 5),
	GET_VERSION	= (1 << 6),
	GET_DISKS	= (1 << 7),
	GET_SAFEMODE	= (1 << 9),
	GET_BITMAP_LOCATION = (1 << 10),

	GET_DEVS	= (1 << 20), /* gets role, major, minor */
	GET_OFFSET	= (1 << 21),
	GET_SIZE	= (1 << 22),
	GET_STATE	= (1 << 23),
	GET_ERROR	= (1 << 24),
	GET_ARRAY_STATE = (1 << 25),
	GET_CONSISTENCY_POLICY	= (1 << 26),
};

/* If fd >= 0, get the array it is open on,
 * else use devnm.
 */
extern int sysfs_open(char *devnm, char *devname, char *attr);
extern int sysfs_init(struct mdinfo *mdi, int fd, char *devnm);
extern void sysfs_init_dev(struct mdinfo *mdi, dev_t devid);
extern void sysfs_free(struct mdinfo *sra);
extern struct mdinfo *sysfs_read(int fd, char *devnm, unsigned long options);
extern int sysfs_attr_match(const char *attr, const char *str);
extern int sysfs_match_word(const char *word, char **list);
extern int sysfs_set_str(struct mdinfo *sra, struct mdinfo *dev,
			 char *name, char *val);
extern int sysfs_set_num(struct mdinfo *sra, struct mdinfo *dev,
			 char *name, unsigned long long val);
extern int sysfs_set_num_signed(struct mdinfo *sra, struct mdinfo *dev,
				char *name, long long val);
extern int sysfs_uevent(struct mdinfo *sra, char *event);
extern int sysfs_get_fd(struct mdinfo *sra, struct mdinfo *dev,
			char *name);
extern int sysfs_fd_get_ll(int fd, unsigned long long *val);
extern int sysfs_get_ll(struct mdinfo *sra, struct mdinfo *dev,
			char *name, unsigned long long *val);
extern int sysfs_fd_get_two(int fd, unsigned long long *v1, unsigned long long *v2);
extern int sysfs_get_two(struct mdinfo *sra, struct mdinfo *dev,
			 char *name, unsigned long long *v1, unsigned long long *v2);
extern int sysfs_fd_get_str(int fd, char *val, int size);
extern int sysfs_attribute_available(struct mdinfo *sra, struct mdinfo *dev,
				     char *name);
extern int sysfs_get_str(struct mdinfo *sra, struct mdinfo *dev,
			 char *name, char *val, int size);
extern int sysfs_set_safemode(struct mdinfo *sra, unsigned long ms);
extern int sysfs_set_array(struct mdinfo *info, int vers);
extern int sysfs_add_disk(struct mdinfo *sra, struct mdinfo *sd, int resume);
extern int sysfs_disk_to_scsi_id(int fd, __u32 *id);
extern int sysfs_unique_holder(char *devnm, long rdev);
extern int sysfs_freeze_array(struct mdinfo *sra);
extern int sysfs_wait(int fd, int *msec);
extern int load_sys(char *path, char *buf, int len);
extern int zero_disk_range(int fd, unsigned long long sector, size_t count);
extern int reshape_prepare_fdlist(char *devname,
				  struct mdinfo *sra,
				  int raid_disks,
				  int nrdisks,
				  unsigned long blocks,
				  char *backup_file,
				  int *fdlist,
				  unsigned long long *offsets);
extern void reshape_free_fdlist(int *fdlist,
				unsigned long long *offsets,
				int size);
extern int reshape_open_backup_file(char *backup,
				    int fd,
				    char *devname,
				    long blocks,
				    int *fdlist,
				    unsigned long long *offsets,
				    char *sysfs_name,
				    int restart);
extern unsigned long compute_backup_blocks(int nchunk, int ochunk,
					   unsigned int ndata, unsigned int odata);
extern char *locate_backup(char *name);
extern char *make_backup(char *name);

extern int save_stripes(int *source, unsigned long long *offsets,
			int raid_disks, int chunk_size, int level, int layout,
			int nwrites, int *dest,
			unsigned long long start, unsigned long long length,
			char *buf);
extern int restore_stripes(int *dest, unsigned long long *offsets,
			   int raid_disks, int chunk_size, int level, int layout,
			   int source, unsigned long long read_offset,
			   unsigned long long start, unsigned long long length,
			   char *src_buf);

#ifndef Sendmail
#define Sendmail "/usr/lib/sendmail -t"
#endif

#define SYSLOG_FACILITY LOG_DAEMON

extern char *map_num(mapping_t *map, int num);
extern int map_name(mapping_t *map, char *name);
extern mapping_t r5layout[], r6layout[], pers[], modes[], faultylayout[];
extern mapping_t consistency_policies[], sysfs_array_states[];

extern char *map_dev_preferred(int major, int minor, int create,
			       char *prefer);
static inline char *map_dev(int major, int minor, int create)
{
	return map_dev_preferred(major, minor, create, NULL);
}

struct active_array;
struct metadata_update;

/* 'struct reshape' records the intermediate states of
 * a general reshape.
 * The starting geometry is converted to the 'before' geometry
 * by at most an atomic level change. They could be the same.
 * Similarly the 'after' geometry is converted to the final
 * geometry by at most a level change.
 * Note that 'before' and 'after' must have the same level.
 * 'blocks' is the minimum number of sectors for a reshape unit.
 * This will be a multiple of the stripe size in each of the
 * 'before' and 'after' geometries.
 * If 'blocks' is 0, no restriping is necessary.
 * 'min_offset_change' is the minimum change to data_offset to
 * allow the reshape to happen.  It is at least the larger of
 * the old  and new chunk sizes, and typically the same as 'blocks'
 * divided by number of data disks.
 */
struct reshape {
	int level;
	int parity; /* number of parity blocks/devices */
	struct {
		int layout;
		int data_disks;
	} before, after;
	unsigned long long backup_blocks;
	unsigned long long min_offset_change;
	unsigned long long stripes; /* number of old stripes that comprise 'blocks'*/
	unsigned long long new_size; /* New size of array in sectors */
};

/* A superswitch provides entry point to a metadata handler.
 *
 * The superswitch primarily operates on some "metadata" that
 * is accessed via the 'supertype'.
 * This metadata has one of three possible sources.
 * 1/ It is read from a single device.  In this case it may not completely
 *    describe the array or arrays as some information might be on other
 *    devices.
 * 2/ It is read from all devices in a container.  In this case all
 *    information is present.
 * 3/ It is created by ->init_super / ->add_to_super.  In this case it will
 *    be complete once enough ->add_to_super calls have completed.
 *
 * When creating an array inside a container, the metadata will be
 * formed by a combination of 2 and 3.  The metadata or the array is read,
 * then new information is added.
 *
 * The metadata must sometimes have a concept of a 'current' array
 * and a 'current' device.
 * The 'current' array is set by init_super to be the newly created array,
 * or is set by super_by_fd when it finds it is looking at an array inside
 * a container.
 *
 * The 'current' device is either the device that the metadata was read from
 * in case 1, or the last device added by add_to_super in case 3.
 * Case 2 does not identify a 'current' device.
 */
extern struct superswitch {

	/* Used to report details of metadata read from a component
	 * device. ->load_super has been called.
	 */
	void (*examine_super)(struct supertype *st, char *homehost);
	void (*brief_examine_super)(struct supertype *st, int verbose);
	void (*brief_examine_subarrays)(struct supertype *st, int verbose);
	void (*export_examine_super)(struct supertype *st);
	int (*examine_badblocks)(struct supertype *st, int fd, char *devname);
	int (*copy_metadata)(struct supertype *st, int from, int to);

	/* Used to report details of an active array.
	 * ->load_super was possibly given a 'component' string.
	 */
	void (*detail_super)(struct supertype *st, char *homehost);
	void (*brief_detail_super)(struct supertype *st);
	void (*export_detail_super)(struct supertype *st);

	/* Optional: platform hardware / firmware details */
	int (*detail_platform)(int verbose, int enumerate_only, char *controller_path);
	int (*export_detail_platform)(int verbose, char *controller_path);

	/* Used:
	 *   to get uuid to storing in bitmap metadata
	 *   and 'reshape' backup-data metadata
	 *   To see if a device is being re-added to an array it was part of.
	 */
	void (*uuid_from_super)(struct supertype *st, int uuid[4]);

	/* Extract generic details from metadata.  This could be details about
	 * the container, or about an individual array within the container.
	 * The determination is made either by:
	 *   load_super being given a 'component' string.
	 *   validate_geometry determining what to create.
	 * The info includes both array information and device information.
	 * The particular device should be:
	 *   The last device added by add_to_super
	 *   The device the metadata was loaded from by load_super
	 * If 'map' is present, then it is an array raid_disks long
	 * (raid_disk must already be set and correct) and it is filled
	 * with 1 for slots that are thought to be active and 0 for slots which
	 * appear to be failed/missing.
	 * *info is zeroed out before data is added.
	 */
	void (*getinfo_super)(struct supertype *st, struct mdinfo *info, char *map);
	struct mdinfo *(*getinfo_super_disks)(struct supertype *st);
	/* Check if the given metadata is flagged as belonging to "this"
	 * host.  0 for 'no', 1 for 'yes', -1 for "Don't record homehost"
	 */
	int (*match_home)(struct supertype *st, char *homehost);

	/* Make one of several generic modifications to metadata
	 * prior to assembly (or other times).
	 *   sparc2.2  - first bug in early 0.90 metadata
	 *   super-minor - change name of 0.90 metadata
	 *   summaries - 'correct' any redundant data
	 *   resync - mark array as dirty to trigger a resync.
	 *   uuid - set new uuid - only 0.90 or 1.x
	 *   name - change the name of the array (where supported)
	 *   homehost - change which host this array is tied to.
	 *   devicesize - If metadata is at start of device, change recorded
	 *               device size to match actual device size
	 *   byteorder - swap bytes for 0.90 metadata
	 *
	 *   force-one  - mark that device as uptodate, not old or failed.
	 *   force-array - mark array as clean if it would not otherwise
	 *               assemble
	 *   assemble   - not sure how this is different from force-one...
	 *   linear-grow-new - add a new device to a linear array, but don't
	 *                   change the size: so superblock still matches
	 *   linear-grow-update - now change the size of the array.
	 *   writemostly - set the WriteMostly1 bit in the superblock devflags
	 *   readwrite - clear the WriteMostly1 bit in the superblock devflags
	 *   failfast - set the FailFast1 bit in the superblock
	 *   nofailfast - clear the FailFast1 bit
	 *   no-bitmap - clear any record that a bitmap is present.
	 *   bbl       - add a bad-block-log if possible
	 *   no-bbl    - remove any bad-block-log is it is empty.
	 *   force-no-bbl - remove any bad-block-log even if empty.
	 *   revert-reshape - If a reshape is in progress, modify metadata so
	 *                    it will resume going in the opposite direction.
	 */
	int (*update_super)(struct supertype *st, struct mdinfo *info,
			    char *update,
			    char *devname, int verbose,
			    int uuid_set, char *homehost);

	/* Create new metadata for new array as described.  This could
	 * be a new container, or an array in a pre-existing container.
	 * Also used to zero metadata prior to writing it to invalidate old
	 * metadata.
	 */
	int (*init_super)(struct supertype *st, mdu_array_info_t *info,
			  struct shape *s, char *name,
			  char *homehost, int *uuid,
			  unsigned long long data_offset);

	/* update the metadata to include new device, either at create or
	 * when hot-adding a spare.
	 */
	int (*add_to_super)(struct supertype *st, mdu_disk_info_t *dinfo,
			    int fd, char *devname,
			    unsigned long long data_offset);
	/* update the metadata to delete a device,
	 * when hot-removing.
	 */
	int (*remove_from_super)(struct supertype *st, mdu_disk_info_t *dinfo);

	/* Write metadata to one device when fixing problems or adding
	 * a new device.
	 */
	int (*store_super)(struct supertype *st, int fd);

	/*  Write all metadata for this array.
	 */
	int (*write_init_super)(struct supertype *st);
	/* Check if metadata read from one device is compatible with an array,
	 * used when assembling an array, or pseudo-assembling was with
	 * "--examine --brief"
	 * If "st" has not yet been loaded the superblock from, "tst" is
	 * moved in, otherwise the superblock in 'st' is compared with
	 * 'tst'.
	 */
	int (*compare_super)(struct supertype *st, struct supertype *tst);
	/* Load metadata from a single device.  If 'devname' is not NULL
	 * print error messages as appropriate */
	int (*load_super)(struct supertype *st, int fd, char *devname);
	/* 'fd' is a 'container' md array - load array metadata from the
	 * whole container.
	 */
	int (*load_container)(struct supertype *st, int fd, char *devname);
	/* If 'arg' is a valid name of this metadata type, allocate and
	 * return a 'supertype' for the particular minor version */
	struct supertype * (*match_metadata_desc)(char *arg);
	/* If a device has the given size, and the data_offset has been
	 * requested - work out how much space is available for data.
	 * This involves adjusting for reserved space (e.g. bitmaps)
	 * and for any rounding.
	 * 'mdadm' only calls this for existing arrays where a possible
	 * spare is being added.  However some super-handlers call it
	 * internally from validate_geometry when creating an array.
	 */
	__u64 (*avail_size)(struct supertype *st, __u64 size,
			    unsigned long long data_offset);
	/*
	 * Return spare criteria for array:
	 * - minimum disk size can be used in array;
	 * - sector size can be used in array.
	 * Return values: 0 - for success and -EINVAL on error.
	 */
	int (*get_spare_criteria)(struct supertype *st,
				  struct spare_criteria *sc);
	/* Find somewhere to put a bitmap - possibly auto-size it - and
	 * update the metadata to record this.  The array may be newly
	 * created, in which case data_size may be updated, or it might
	 * already exist.  Metadata handler can know if init_super
	 * has been called, but not write_init_super.
	 *  0:     Success
	 * -Exxxx: On error
	 */
	int (*add_internal_bitmap)(struct supertype *st, int *chunkp,
				   int delay, int write_behind,
				   unsigned long long size, int may_change, int major);
	/* Seek 'fd' to start of write-intent-bitmap.  Must be an
	 * md-native format bitmap
	 */
	int (*locate_bitmap)(struct supertype *st, int fd, int node_num);
	/* if add_internal_bitmap succeeded for existing array, this
	 * writes it out.
	 */
	int (*write_bitmap)(struct supertype *st, int fd, enum bitmap_update update);
	/* Free the superblock and any other allocated data */
	void (*free_super)(struct supertype *st);

	/* validate_geometry is called with an st returned by
	 * match_metadata_desc.
	 * It should check that the geometry described is compatible with
	 * the metadata type.  It will be called repeatedly as devices
	 * added to validate changing size and new devices.  If there are
	 * inter-device dependencies, it should record sufficient details
	 * so these can be validated.
	 * Both 'size' and '*freesize' are in sectors.  chunk is KiB.
	 * Return value is:
	 *  1: everything is OK
	 *  0: not OK for some reason - if 'verbose', then error was reported.
	 * -1: st->sb was NULL, 'subdev' is a member of a container of this
	 *     type, but array is not acceptable for some reason
	 *     message was reported even if verbose is 0.
	 */
	int (*validate_geometry)(struct supertype *st, int level, int layout,
				 int raiddisks,
				 int *chunk, unsigned long long size,
				 unsigned long long data_offset,
				 char *subdev, unsigned long long *freesize,
				 int consistency_policy, int verbose);

	/* Return a linked list of 'mdinfo' structures for all arrays
	 * in the container.  For non-containers, it is like
	 * getinfo_super with an allocated mdinfo.*/
	struct mdinfo *(*container_content)(struct supertype *st, char *subarray);
	/* query the supertype for default geometry */
	void (*default_geometry)(struct supertype *st, int *level, int *layout, int *chunk); /* optional */
	/* Permit subarray's to be deleted from inactive containers */
	int (*kill_subarray)(struct supertype *st); /* optional */
	/* Permit subarray's to be modified */
	int (*update_subarray)(struct supertype *st, char *subarray,
			       char *update, struct mddev_ident *ident); /* optional */
	/* Check if reshape is supported for this external format.
	 * st is obtained from super_by_fd() where st->subarray[0] is
	 * initialized to indicate if reshape is being performed at the
	 * container or subarray level
	 */
#define APPLY_METADATA_CHANGES		1
#define ROLLBACK_METADATA_CHANGES	0

	int (*reshape_super)(struct supertype *st,
			     unsigned long long size, int level,
			     int layout, int chunksize, int raid_disks,
			     int delta_disks, char *backup, char *dev,
			     int direction,
			     int verbose); /* optional */
	int (*manage_reshape)( /* optional */
		int afd, struct mdinfo *sra, struct reshape *reshape,
		struct supertype *st, unsigned long blocks,
		int *fds, unsigned long long *offsets,
		int dests, int *destfd, unsigned long long *destoffsets);

/* for mdmon */
	int (*open_new)(struct supertype *c, struct active_array *a,
			char *inst);

	/* Tell the metadata handler the current state of the array.
	 * This covers whether it is known to be consistent (no pending writes)
	 * and how far along a resync is known to have progressed
	 * (in a->resync_start).
	 * resync status is really irrelevant if the array is not consistent,
	 * but some metadata (DDF!) have a place to record the distinction.
	 * If 'consistent' is '2', then the array can mark it dirty if a
	 * resync/recovery/whatever is required, or leave it clean if not.
	 * Return value is 0 dirty (not consistent) and 1 if clean.
	 * it is only really important if consistent is passed in as '2'.
	 */
	int (*set_array_state)(struct active_array *a, int consistent);

	/* When the state of a device might have changed, we call set_disk to
	 * tell the metadata what the current state is.
	 * Typically this happens on spare->in_sync and (spare|in_sync)->faulty
	 * transitions.
	 * set_disk might be called when the state of the particular disk has
	 * not in fact changed.
	 */
	void (*set_disk)(struct active_array *a, int n, int state);
	void (*sync_metadata)(struct supertype *st);
	void (*process_update)(struct supertype *st,
			       struct metadata_update *update);
	/* Prepare updates allocates extra memory that might be
	 * needed.  If the update cannot be understood,  return 0.
	 */
	int (*prepare_update)(struct supertype *st,
			       struct metadata_update *update);

	/* activate_spare will check if the array is degraded and, if it
	 * is, try to find some spare space in the container.
	 * On success, it add appropriate updates (For process_update) to
	 * to the 'updates' list and returns a list of 'mdinfo' identifying
	 * the device, or devices as there might be multiple missing
	 * devices and multiple spares available.
	 */
	struct mdinfo *(*activate_spare)(struct active_array *a,
					 struct metadata_update **updates);
	/*
	 * Return statically allocated string that represents metadata specific
	 * controller domain of the disk. The domain is used in disk domain
	 * matching functions. Disks belong to the same domain if the they have
	 * the same domain from mdadm.conf and belong the same metadata domain.
	 * Returning NULL or not providing this handler means that metadata
	 * does not distinguish the differences between disks that belong to
	 * different controllers. They are in the domain specified by
	 * configuration file (mdadm.conf).
	 * In case when the metadata has the notion of domains based on disk
	 * it shall return NULL for disks that do not belong to the controller
	 * the supported domains. Such disks will form another domain and won't
	 * be mixed with supported ones.
	 */
	const char *(*get_disk_controller_domain)(const char *path);

	/* for external backup area */
	int (*recover_backup)(struct supertype *st, struct mdinfo *info);

	/* validate container after assemble */
	int (*validate_container)(struct mdinfo *info);

	/* write initial empty PPL on device */
	int (*write_init_ppl)(struct supertype *st, struct mdinfo *info, int fd);

	/* validate ppl before assemble */
	int (*validate_ppl)(struct supertype *st, struct mdinfo *info,
			    struct mdinfo *disk);

	/* records new bad block in metadata */
	int (*record_bad_block)(struct active_array *a, int n,
					unsigned long long sector, int length);

	/* clears bad block from metadata */
	int (*clear_bad_block)(struct active_array *a, int n,
					unsigned long long sector, int length);

	/* get list of bad blocks from metadata */
	struct md_bb *(*get_bad_blocks)(struct active_array *a, int n);

	int swapuuid; /* true if uuid is bigending rather than hostendian */
	int external;
	const char *name; /* canonical metadata name */
} *superlist[];

extern struct superswitch super0, super1;
extern struct superswitch super_imsm, super_ddf;
extern struct superswitch mbr, gpt;

struct metadata_update {
	int	len;
	char	*buf;
	void	*space; /* allocated space that monitor will use */
	void	**space_list; /* list of allocated spaces that monitor can
			       * use or that it returned.
			       */
	struct metadata_update *next;
};

/* A supertype holds a particular collection of metadata.
 * It identifies the metadata type by the superswitch, and the particular
 * sub-version of that metadata type.
 * metadata read in or created is stored in 'sb' and 'info'.
 * There are also fields used by mdmon to track containers.
 *
 * A supertype may refer to:
 *   Just an array, possibly in a container
 *   A container, not identifying any particular array
 *   Info read from just one device, not yet fully describing the array/container.
 *
 *
 * A supertype is created by:
 *   super_by_fd
 *   guess_super
 *   dup_super
 */
struct supertype {
	struct superswitch *ss;
	int minor_version;
	int max_devs;
	char container_devnm[32];    /* devnm of container */
	void *sb;
	void *info;
	void *other; /* Hack used to convert v0.90 to v1.0 */
	unsigned long long devsize;
	unsigned long long data_offset; /* used by v1.x only */
	int ignore_hw_compat; /* used to inform metadata handlers that it should ignore
				 HW/firmware related incompatability to load metadata.
				 Used when examining metadata to display content of disk
				 when user has no hw/firmare compatible system.
			      */
	struct metadata_update *updates;
	struct metadata_update **update_tail;

	/* extra stuff used by mdmon */
	struct active_array *arrays;
	int sock; /* listen to external programs */
	char devnm[32]; /* e.g. md0.  This appears in metadata_version:
			 *  external:/md0/12
			 */
	int devcnt;
	int retry_soon;
	int nodes;
	char *cluster_name;

	struct mdinfo *devs;

};

extern struct supertype *super_by_fd(int fd, char **subarray);
enum guess_types { guess_any, guess_array, guess_partitions };
extern struct supertype *guess_super_type(int fd, enum guess_types guess_type);
static inline struct supertype *guess_super(int fd) {
	return guess_super_type(fd, guess_any);
}
extern struct supertype *dup_super(struct supertype *st);
extern int get_dev_size(int fd, char *dname, unsigned long long *sizep);
extern int get_dev_sector_size(int fd, char *dname, unsigned int *sectsizep);
extern int must_be_container(int fd);
extern int dev_size_from_id(dev_t id, unsigned long long *size);
extern int dev_sector_size_from_id(dev_t id, unsigned int *size);
void wait_for(char *dev, int fd);

/*
 * Data structures for policy management.
 * Each device can have a policy structure that lists
 * various name/value pairs each possibly with a metadata associated.
 * The policy list is sorted by name/value/metadata
 */
struct dev_policy {
	struct dev_policy *next;
	char *name;	/* None of these strings are allocated.  They are
			 * all just references to strings which are known
			 * to exist elsewhere.
			 * name and metadata can be compared by address equality.
			 */
	const char *metadata;
	const char *value;
};

extern char pol_act[], pol_domain[], pol_metadata[], pol_auto[];

/* iterate over the sublist starting at list, having the same
 * 'name' as 'list', and matching the given metadata (Where
 * NULL matches anything
 */
#define pol_for_each(item, list, _metadata)				\
	for (item = list;						\
	     item && item->name == list->name;				\
	     item = item->next)						\
		if (!(!_metadata || !item->metadata || _metadata == item->metadata)) \
			; else

/*
 * policy records read from mdadm are largely just name-value pairs.
 * The names are constants, not strdupped
 */
struct pol_rule {
	struct pol_rule *next;
	char *type;	/* rule_policy or rule_part */
	struct rule {
		struct rule *next;
		char *name;
		char *value;
		char *dups; /* duplicates of 'value' with a partNN appended */
	} *rule;
};

extern char rule_policy[], rule_part[];
extern char rule_path[], rule_type[];
extern char type_part[], type_disk[];

extern void policyline(char *line, char *type);
extern void policy_add(char *type, ...);
extern void policy_free(void);

extern struct dev_policy *path_policy(char *path, char *type);
extern struct dev_policy *disk_policy(struct mdinfo *disk);
extern struct dev_policy *devid_policy(int devid);
extern void dev_policy_free(struct dev_policy *p);

//extern void pol_new(struct dev_policy **pol, char *name, char *val, char *metadata);
extern void pol_add(struct dev_policy **pol, char *name, char *val, char *metadata);
extern struct dev_policy *pol_find(struct dev_policy *pol, char *name);

enum policy_action {
	act_default,
	act_include,
	act_re_add,
	act_spare,	/* This only applies to bare devices */
	act_spare_same_slot, /* this allows non-bare devices,
			      * but only if recent removal */
	act_force_spare, /* this allow non-bare devices in any case */
	act_err
};

extern int policy_action_allows(struct dev_policy *plist, const char *metadata,
				enum policy_action want);
extern int disk_action_allows(struct mdinfo *disk, const char *metadata,
			      enum policy_action want);

struct domainlist {
	struct domainlist *next;
	const char *dom;
};

extern int domain_test(struct domainlist *dom, struct dev_policy *pol,
		       const char *metadata);
extern struct domainlist *domain_from_array(struct mdinfo *mdi,
					    const char *metadata);
extern void domainlist_add_dev(struct domainlist **dom, int devid,
			       const char *metadata);
extern void domain_free(struct domainlist *dl);
extern void domain_merge(struct domainlist **domp, struct dev_policy *pol,
			 const char *metadata);
void domain_add(struct domainlist **domp, char *domain);

extern void policy_save_path(char *id_path, struct map_ent *array);
extern int policy_check_path(struct mdinfo *disk, struct map_ent *array);

#if __GNUC__ < 3
struct stat64;
#endif

#define HAVE_NFTW  we assume
#define HAVE_FTW

#ifdef __UCLIBC__
# include <features.h>
# ifndef __UCLIBC_HAS_LFS__
#  define lseek64 lseek
# endif
# ifndef  __UCLIBC_HAS_FTW__
#  undef HAVE_FTW
#  undef HAVE_NFTW
# endif
#endif

#ifdef __dietlibc__
# undef HAVE_NFTW
#endif

#if defined(__KLIBC__)
# undef HAVE_NFTW
# undef HAVE_FTW
#endif

#ifndef HAVE_NFTW
# define FTW_PHYS 1
# ifndef HAVE_FTW
  struct FTW {};
# endif
#endif

#ifdef HAVE_FTW
# include <ftw.h>
#endif

extern int add_dev(const char *name, const struct stat *stb, int flag, struct FTW *s);

extern int Manage_ro(char *devname, int fd, int readonly);
extern int Manage_run(char *devname, int fd, struct context *c);
extern int Manage_stop(char *devname, int fd, int quiet,
		       int will_retry);
extern int Manage_subdevs(char *devname, int fd,
			  struct mddev_dev *devlist, int verbose, int test,
			  char *update, int force);
extern int autodetect(void);
extern int Grow_Add_device(char *devname, int fd, char *newdev);
extern int Grow_addbitmap(char *devname, int fd,
			  struct context *c, struct shape *s);
extern int Grow_reshape(char *devname, int fd,
			struct mddev_dev *devlist,
			unsigned long long data_offset,
			struct context *c, struct shape *s);
extern int Grow_restart(struct supertype *st, struct mdinfo *info,
			int *fdlist, int cnt, char *backup_file, int verbose);
extern int Grow_continue(int mdfd, struct supertype *st,
			 struct mdinfo *info, char *backup_file,
			 int forked, int freeze_reshape);
extern int Grow_consistency_policy(char *devname, int fd,
				   struct context *c, struct shape *s);

extern int restore_backup(struct supertype *st,
			  struct mdinfo *content,
			  int working_disks,
			  int spares,
			  char **backup_filep,
			  int verbose);
extern int Grow_continue_command(char *devname, int fd,
				 char *backup_file, int verbose);

extern int Assemble(struct supertype *st, char *mddev,
		    struct mddev_ident *ident,
		    struct mddev_dev *devlist,
		    struct context *c);

extern int Build(char *mddev, struct mddev_dev *devlist,
		 struct shape *s, struct context *c);

extern int Create(struct supertype *st, char *mddev,
		  char *name, int *uuid,
		  int subdevs, struct mddev_dev *devlist,
		  struct shape *s,
		  struct context *c,
		  unsigned long long data_offset);

extern int Detail(char *dev, struct context *c);
extern int Detail_Platform(struct superswitch *ss, int scan, int verbose, int export, char *controller_path);
extern int Query(char *dev);
extern int ExamineBadblocks(char *devname, int brief, struct supertype *forcest);
extern int Examine(struct mddev_dev *devlist, struct context *c,
		   struct supertype *forcest);
extern int Monitor(struct mddev_dev *devlist,
		   char *mailaddr, char *alert_cmd,
		   struct context *c,
		   int daemonise, int oneshot,
		   int dosyslog, char *pidfile, int increments,
		   int share);

extern int Kill(char *dev, struct supertype *st, int force, int verbose, int noexcl);
extern int Kill_subarray(char *dev, char *subarray, int verbose);
extern int Update_subarray(char *dev, char *subarray, char *update, struct mddev_ident *ident, int quiet);
extern int Wait(char *dev);
extern int WaitClean(char *dev, int verbose);
extern int SetAction(char *dev, char *action);

extern int Incremental(struct mddev_dev *devlist, struct context *c,
		       struct supertype *st);
extern void RebuildMap(void);
extern int IncrementalScan(struct context *c, char *devnm);
extern int IncrementalRemove(char *devname, char *path, int verbose);
extern int CreateBitmap(char *filename, int force, char uuid[16],
			unsigned long chunksize, unsigned long daemon_sleep,
			unsigned long write_behind,
			unsigned long long array_size,
			int major);
extern int ExamineBitmap(char *filename, int brief, struct supertype *st);
extern int Write_rules(char *rule_name);
extern int bitmap_update_uuid(int fd, int *uuid, int swap);

/* calculate the size of the bitmap given the array size and bitmap chunksize */
static inline unsigned long long
bitmap_bits(unsigned long long array_size, unsigned long chunksize)
{
	return (array_size * 512 + chunksize - 1) / chunksize;
}

extern int Dump_metadata(char *dev, char *dir, struct context *c,
			 struct supertype *st);
extern int Restore_metadata(char *dev, char *dir, struct context *c,
			    struct supertype *st, int only);

int md_array_valid(int fd);
int md_array_active(int fd);
int md_array_is_active(struct mdinfo *info);
int md_get_array_info(int fd, struct mdu_array_info_s *array);
int md_set_array_info(int fd, struct mdu_array_info_s *array);
int md_get_disk_info(int fd, struct mdu_disk_info_s *disk);
extern int get_linux_version(void);
extern int mdadm_version(char *version);
extern unsigned long long parse_size(char *size);
extern int parse_uuid(char *str, int uuid[4]);
extern int is_near_layout_10(int layout);
extern int parse_layout_10(char *layout);
extern int parse_layout_faulty(char *layout);
extern long parse_num(char *num);
extern int parse_cluster_confirm_arg(char *inp, char **devname, int *slot);
extern int check_ext2(int fd, char *name);
extern int check_reiser(int fd, char *name);
extern int check_raid(int fd, char *name);
extern int check_partitions(int fd, char *dname,
			    unsigned long long freesize,
			    unsigned long long size);
extern int fstat_is_blkdev(int fd, char *devname, dev_t *rdev);
extern int stat_is_blkdev(char *devname, dev_t *rdev);

extern int get_mdp_major(void);
extern int get_maj_min(char *dev, int *major, int *minor);
extern int dev_open(char *dev, int flags);
extern int open_dev(char *devnm);
extern void reopen_mddev(int mdfd);
extern int open_dev_flags(char *devnm, int flags);
extern int open_dev_excl(char *devnm);
extern int is_standard(char *dev, int *nump);
extern int same_dev(char *one, char *two);
extern int compare_paths (char* path1,char* path2);
extern void enable_fds(int devices);

extern int parse_auto(char *str, char *msg, int config);
extern struct mddev_ident *conf_get_ident(char *dev);
extern struct mddev_dev *conf_get_devs(void);
extern int conf_test_dev(char *devname);
extern int conf_test_metadata(const char *version, struct dev_policy *pol, int is_homehost);
extern struct createinfo *conf_get_create_info(void);
extern void set_conffile(char *file);
extern char *conf_get_mailaddr(void);
extern char *conf_get_mailfrom(void);
extern char *conf_get_program(void);
extern char *conf_get_homehost(int *require_homehostp);
extern char *conf_get_homecluster(void);
extern char *conf_line(FILE *file);
extern char *conf_word(FILE *file, int allow_key);
extern void print_quoted(char *str);
extern void print_escape(char *str);
extern int use_udev(void);
extern unsigned long GCD(unsigned long a, unsigned long b);
extern int conf_name_is_free(char *name);
extern int conf_verify_devnames(struct mddev_ident *array_list);
extern int devname_matches(char *name, char *match);
extern struct mddev_ident *conf_match(struct supertype *st,
				      struct mdinfo *info,
				      char *devname,
				      int verbose, int *rvp);

extern void free_line(char *line);
extern int match_oneof(char *devices, char *devname);
extern void uuid_from_super(int uuid[4], mdp_super_t *super);
extern const int uuid_zero[4];
extern int same_uuid(int a[4], int b[4], int swapuuid);
extern void copy_uuid(void *a, int b[4], int swapuuid);
extern char *__fname_from_uuid(int id[4], int swap, char *buf, char sep);
extern char *fname_from_uuid(struct supertype *st,
			     struct mdinfo *info, char *buf, char sep);
extern unsigned long calc_csum(void *super, int bytes);
extern int enough(int level, int raid_disks, int layout, int clean,
		   char *avail);
extern int ask(char *mesg);
extern unsigned long long get_component_size(int fd);
extern void remove_partitions(int fd);
extern int test_partition(int fd);
extern int test_partition_from_id(dev_t id);
extern int get_data_disks(int level, int layout, int raid_disks);
extern unsigned long long calc_array_size(int level, int raid_disks, int layout,
				   int chunksize, unsigned long long devsize);
extern int flush_metadata_updates(struct supertype *st);
extern void append_metadata_update(struct supertype *st, void *buf, int len);
extern int assemble_container_content(struct supertype *st, int mdfd,
				      struct mdinfo *content,
				      struct context *c,
				      char *chosen_name, int *result);
#define	INCR_NO		1
#define	INCR_UNSAFE	2
#define	INCR_ALREADY	4
#define	INCR_YES	8
extern struct mdinfo *container_choose_spares(struct supertype *st,
					      struct spare_criteria *criteria,
					      struct domainlist *domlist,
					      char *spare_group,
					      const char *metadata, int get_one);
extern int move_spare(char *from_devname, char *to_devname, dev_t devid);
extern int add_disk(int mdfd, struct supertype *st,
		    struct mdinfo *sra, struct mdinfo *info);
extern int remove_disk(int mdfd, struct supertype *st,
		       struct mdinfo *sra, struct mdinfo *info);
extern int hot_remove_disk(int mdfd, unsigned long dev, int force);
extern int sys_hot_remove_disk(int statefd, int force);
extern int set_array_info(int mdfd, struct supertype *st, struct mdinfo *info);
unsigned long long min_recovery_start(struct mdinfo *array);

extern char *human_size(long long bytes);
extern char *human_size_brief(long long bytes, int prefix);
extern void print_r10_layout(int layout);

extern char *find_free_devnm(int use_partitions);

extern void put_md_name(char *name);
extern char *devid2kname(dev_t devid);
extern char *devid2devnm(dev_t devid);
extern dev_t devnm2devid(char *devnm);
extern char *get_md_name(char *devnm);

extern char DefaultConfFile[];

extern int create_mddev(char *dev, char *name, int autof, int trustworthy,
			char *chosen, int block_udev);
/* values for 'trustworthy' */
#define	LOCAL	1
#define	LOCAL_ANY 10
#define	FOREIGN	2
#define	METADATA 3
extern int open_mddev(char *dev, int report_errors);
extern int open_container(int fd);
extern int metadata_container_matches(char *metadata, char *devnm);
extern int metadata_subdev_matches(char *metadata, char *devnm);
extern int is_container_member(struct mdstat_ent *ent, char *devname);
extern int is_subarray_active(char *subarray, char *devname);
extern int open_subarray(char *dev, char *subarray, struct supertype *st, int quiet);
extern struct superswitch *version_to_superswitch(char *vers);

extern int mdmon_running(char *devnm);
extern int mdmon_pid(char *devnm);
extern int check_env(char *name);
extern __u32 random32(void);
extern void random_uuid(__u8 *buf);
extern int start_mdmon(char *devnm);

extern int child_monitor(int afd, struct mdinfo *sra, struct reshape *reshape,
			 struct supertype *st, unsigned long stripes,
			 int *fds, unsigned long long *offsets,
			 int dests, int *destfd, unsigned long long *destoffsets);
void abort_reshape(struct mdinfo *sra);

void *super1_make_v0(struct supertype *st, struct mdinfo *info, mdp_super_t *sb0);

extern char *stat2kname(struct stat *st);
extern char *fd2kname(int fd);
extern char *stat2devnm(struct stat *st);
extern char *fd2devnm(int fd);
extern void udev_block(char *devnm);
extern void udev_unblock(void);

extern int in_initrd(void);

struct cmap_hooks {
	void *cmap_handle;      /* corosync lib related */

	int (*initialize)(cmap_handle_t *handle);
	int (*get_string)(cmap_handle_t handle,
			  const char *string,
			  char **name);
	int (*finalize)(cmap_handle_t handle);
};

extern void set_cmap_hooks(void);
extern void set_hooks(void);

struct dlm_hooks {
	void *dlm_handle;	/* dlm lib related */

	dlm_lshandle_t (*create_lockspace)(const char *name,
					   unsigned int mode);
	dlm_lshandle_t (*open_lockspace)(const char *name);
	int (*release_lockspace)(const char *name, dlm_lshandle_t ls,
				 int force);
	int (*ls_lock)(dlm_lshandle_t lockspace, uint32_t mode,
		       struct dlm_lksb *lksb, uint32_t flags,
		       const void *name, unsigned int namelen,
		       uint32_t parent, void (*astaddr) (void *astarg),
		       void *astarg, void (*bastaddr) (void *astarg),
		       void *range);
	int (*ls_unlock_wait)(dlm_lshandle_t lockspace, uint32_t lkid,
			      uint32_t flags, struct dlm_lksb *lksb);
	int (*ls_get_fd)(dlm_lshandle_t ls);
	int (*dispatch)(int fd);
};

extern int get_cluster_name(char **name);
extern int dlm_funs_ready(void);
extern int cluster_get_dlmlock(void);
extern int cluster_release_dlmlock(void);
extern void set_dlm_hooks(void);

#define _ROUND_UP(val, base)	(((val) + (base) - 1) & ~(base - 1))
#define ROUND_UP(val, base)	_ROUND_UP(val, (typeof(val))(base))
#define ROUND_UP_PTR(ptr, base)	((typeof(ptr)) \
				 (ROUND_UP((unsigned long)(ptr), base)))

static inline int is_subarray(char *vers)
{
	/* The version string for a 'subarray' (an array in a container)
	 * is
	 *    /containername/componentname    for normal read-write arrays
	 *    -containername/componentname    for arrays which mdmon must not
	 *				      reconfigure.  They might be read-only
	 *				      or might be undergoing reshape etc.
	 * containername is e.g. md0, md_d1
	 * componentname is dependant on the metadata. e.g. '1' 'S1' ...
	 */
	return (*vers == '/' || *vers == '-');
}

static inline char *to_subarray(struct mdstat_ent *ent, char *container)
{
	return &ent->metadata_version[10+strlen(container)+1];
}

#ifdef DEBUG
#define dprintf(fmt, arg...) \
	fprintf(stderr, "%s: %s: "fmt, Name, __func__, ##arg)
#define dprintf_cont(fmt, arg...) \
	fprintf(stderr, fmt, ##arg)
#else
#define dprintf(fmt, arg...) \
        ({ if (0) fprintf(stderr, "%s: %s: " fmt, Name, __func__, ##arg); 0; })
#define dprintf_cont(fmt, arg...) \
        ({ if (0) fprintf(stderr, fmt, ##arg); 0; })
#endif
#include <assert.h>
#include <stdarg.h>
static inline int xasprintf(char **strp, const char *fmt, ...) {
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vasprintf(strp, fmt, ap);
	va_end(ap);
	assert(ret >= 0);
	return ret;
}

#ifdef DEBUG
#define pr_err(fmt, args...) fprintf(stderr, "%s: %s: "fmt, Name, __func__, ##args)
#else
#define pr_err(fmt, args...) fprintf(stderr, "%s: "fmt, Name, ##args)
#endif
#define cont_err(fmt ...) fprintf(stderr, "       " fmt)

void *xmalloc(size_t len);
void *xrealloc(void *ptr, size_t len);
void *xcalloc(size_t num, size_t size);
char *xstrdup(const char *str);

#define	LEVEL_MULTIPATH		(-4)
#define	LEVEL_LINEAR		(-1)
#define	LEVEL_FAULTY		(-5)

/* kernel module doesn't know about these */
#define LEVEL_CONTAINER		(-100)
#define	LEVEL_UNSUPPORTED	(-200)

/* the kernel does know about this one ... */
#define	LEVEL_NONE		(-1000000)

/* faulty stuff */

#define	WriteTransient	0
#define	ReadTransient	1
#define	WritePersistent	2
#define	ReadPersistent	3
#define	WriteAll	4 /* doesn't go to device */
#define	ReadFixable	5
#define	Modes	6

#define	ClearErrors	31
#define	ClearFaults	30

#define AllPersist	100 /* internal use only */
#define	NoPersist	101

#define	ModeMask	0x1f
#define	ModeShift	5

#ifdef __TINYC__
#undef minor
#undef major
#undef makedev
#define minor(x) ((x)&0xff)
#define major(x) (((x)>>8)&0xff)
#define makedev(M,m) (((M)<<8) | (m))
#endif

/* for raid4/5/6 */
#define ALGORITHM_LEFT_ASYMMETRIC	0
#define ALGORITHM_RIGHT_ASYMMETRIC	1
#define ALGORITHM_LEFT_SYMMETRIC	2
#define ALGORITHM_RIGHT_SYMMETRIC	3

/* Define non-rotating (raid4) algorithms.  These allow
 * conversion of raid4 to raid5.
 */
#define ALGORITHM_PARITY_0		4 /* P or P,Q are initial devices */
#define ALGORITHM_PARITY_N		5 /* P or P,Q are final devices. */

/* DDF RAID6 layouts differ from md/raid6 layouts in two ways.
 * Firstly, the exact positioning of the parity block is slightly
 * different between the 'LEFT_*' modes of md and the "_N_*" modes
 * of DDF.
 * Secondly, or order of datablocks over which the Q syndrome is computed
 * is different.
 * Consequently we have different layouts for DDF/raid6 than md/raid6.
 * These layouts are from the DDFv1.2 spec.
 * Interestingly DDFv1.2-Errata-A does not specify N_CONTINUE but
 * leaves RLQ=3 as 'Vendor Specific'
 */

#define ALGORITHM_ROTATING_ZERO_RESTART	8 /* DDF PRL=6 RLQ=1 */
#define ALGORITHM_ROTATING_N_RESTART	9 /* DDF PRL=6 RLQ=2 */
#define ALGORITHM_ROTATING_N_CONTINUE	10 /*DDF PRL=6 RLQ=3 */

/* For every RAID5 algorithm we define a RAID6 algorithm
 * with exactly the same layout for data and parity, and
 * with the Q block always on the last device (N-1).
 * This allows trivial conversion from RAID5 to RAID6
 */
#define ALGORITHM_LEFT_ASYMMETRIC_6	16
#define ALGORITHM_RIGHT_ASYMMETRIC_6	17
#define ALGORITHM_LEFT_SYMMETRIC_6	18
#define ALGORITHM_RIGHT_SYMMETRIC_6	19
#define ALGORITHM_PARITY_0_6		20
#define ALGORITHM_PARITY_N_6		ALGORITHM_PARITY_N

/* Define PATH_MAX in case we don't use glibc or standard library does
 * not have PATH_MAX defined. Assume max path length is 4K characters.
 */
#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

#define RESYNC_NONE -1
#define RESYNC_DELAYED -2
#define RESYNC_PENDING -3
#define RESYNC_UNKNOWN -4

/* When using "GET_DISK_INFO" it isn't certain how high
 * we need to check.  So we impose an absolute limit of
 * MAX_DISKS.  This needs to be much more than the largest
 * number of devices any metadata can support.  Currently
 * v1.x can support 1920
 */
#define MAX_DISKS	4096

/* Sometimes the 'size' value passed needs to mean "Maximum".
 * In those cases with use MAX_SIZE
 */
#define MAX_SIZE	1

/* We want to use unsigned numbers for sector counts, but need
 * a value for 'invalid'.  Use '1'.
 */
#define INVALID_SECTORS 1
/* And another special number needed for --data_offset=variable */
#define VARIABLE_OFFSET 3
