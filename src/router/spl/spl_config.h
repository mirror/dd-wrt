/* spl_config.h.  Generated from spl_config.h.in by configure.  */
/* spl_config.h.in.  Generated from configure.ac by autoheader.  */

/* Atomic types use spinlocks */
/* #undef ATOMIC_SPINLOCK */

/* Define to 1 to enable basic kmem accounting */
/* #undef DEBUG_KMEM */

/* Define to 1 to enable detailed kmem tracking */
/* #undef DEBUG_KMEM_TRACKING */

/* new shrinker callback wants 2 args */
/* #undef HAVE_2ARGS_NEW_SHRINKER_CALLBACK */

/* old shrinker callback wants 2 args */
/* #undef HAVE_2ARGS_OLD_SHRINKER_CALLBACK */

/* vfs_fsync() wants 2 args */
#define HAVE_2ARGS_VFS_FSYNC 1

/* vfs_getattr wants 2 args */
/* #undef HAVE_2ARGS_VFS_GETATTR */

/* vfs_unlink() wants 2 args */
/* #undef HAVE_2ARGS_VFS_UNLINK */

/* zlib_deflate_workspacesize() wants 2 args */
#define HAVE_2ARGS_ZLIB_DEFLATE_WORKSPACESIZE 1

/* old shrinker callback wants 3 args */
/* #undef HAVE_3ARGS_SHRINKER_CALLBACK */

/* vfs_getattr wants 3 args */
/* #undef HAVE_3ARGS_VFS_GETATTR */

/* vfs_unlink() wants 3 args */
#define HAVE_3ARGS_VFS_UNLINK 1


/* vfs_rename() wants 4 args */
/* #undef HAVE_4ARGS_VFS_RENAME */

/* vfs_rename() wants 5 args */
/* #undef HAVE_5ARGS_VFS_RENAME */

/* vfs_rename() wants 6 args */
#define HAVE_6ARGS_VFS_RENAME 1

/* kernel defines atomic64_t */
#define HAVE_ATOMIC64_T 1

/* struct ctl_table has ctl_name */
/* #undef HAVE_CTL_NAME */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* fops->fallocate() exists */
#define HAVE_FILE_FALLOCATE 1

/* struct fs_struct uses spinlock_t */
#define HAVE_FS_STRUCT_SPINLOCK 1

/* fops->fallocate() exists */
/* #undef HAVE_INODE_FALLOCATE */

/* yes */
/* truncate_range() inode operation is available */
/* #undef HAVE_INODE_TRUNCATE_RANGE */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* timer_list.function gets a timer_list */
/* #undef HAVE_KERNEL_TIMER_FUNCTION_TIMER_LIST */

/* struct kmem_cache has allocflags */
/* #undef HAVE_KMEM_CACHE_ALLOCFLAGS */

/* kmem_cache_create_usercopy() exists */
/* #undef HAVE_KMEM_CACHE_CREATE_USERCOPY */

/* struct kmem_cache has gfpflags */
/* #undef HAVE_KMEM_CACHE_GFPFLAGS */

/* kuid_t/kgid_t in use */
#define HAVE_KUIDGID_T 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* yes */
#define HAVE_PDE_DATA 1

/* struct rw_semaphore has member activity */
/* #undef HAVE_RWSEM_ACTIVITY */

/* linux/sched/rt.h exists */
#define HAVE_SCHED_RT_HEADER 1

/* linux/sched/signal.h exists */

/* set_fs_pwd() needs const path * */
#define HAVE_SET_FS_PWD_WITH_CONST 1

/* struct shrink_control exists */
#define HAVE_SHRINK_CONTROL_STRUCT 1

/* ->count_objects exists */
#define HAVE_SPLIT_SHRINKER_CALLBACK 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* usleep_range is available */
#define HAVE_USLEEP_RANGE 1

/* yes */
/* #undef HAVE_WAIT_ON_BIT_ACTION */

/* wait_queue_entry_t exists */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* struct rw_semaphore member wait_lock is raw_spinlock_t */
#define RWSEM_SPINLOCK_IS_RAW 1

/* Define to 1 if GPL-only symbols can be used */
#define SPL_IS_GPL_COMPATIBLE 1

/* Define the project alias string. */
#define SPL_META_ALIAS "spl-0.7.6-1"

/* Define the project author. */
#define SPL_META_AUTHOR "OpenZFS on Linux"

/* Define the project release date. */
/* #undef SPL_META_DATA */

/* Define the project license. */
#define SPL_META_LICENSE "GPL"

/* Define the libtool library 'age' version information. */
/* #undef SPL_META_LT_AGE */

/* Define the libtool library 'current' version information. */
/* #undef SPL_META_LT_CURRENT */

/* Define the libtool library 'revision' version information. */
/* #undef SPL_META_LT_REVISION */

/* Define the project name. */
#define SPL_META_NAME "spl"

/* Define the project release. */
#define SPL_META_RELEASE "1"

/* Define the project version. */
#define SPL_META_VERSION "0.7.6"

