#ifndef __YAFFS_CONFIG_H__
#define __YAFFS_CONFIG_H__

#ifdef YAFFS_OUT_OF_TREE

/* DO NOT UNSET THESE THREE. YAFFS2 will not compile if you do. */
#define CONFIG_YAFFS_FS
#define CONFIG_YAFFS_YAFFS1
#define CONFIG_YAFFS_YAFFS2

/* These options are independent of each other.  Select those that matter. */

/* Default: Not selected */
/* Meaning: Yaffs does its own ECC, rather than using MTD ECC */
//#define CONFIG_YAFFS_DOES_ECC

/* Default: Not selected */
/* Meaning: ECC byte order is 'wrong'.  Only meaningful if */
/*          CONFIG_YAFFS_DOES_ECC is set */
//#define CONFIG_YAFFS_ECC_WRONG_ORDER

/* Default: Selected */
/* Meaning: Disables testing whether chunks are erased before writing to them*/
#define CONFIG_YAFFS_DISABLE_CHUNK_ERASED_CHECK

/* Default: Selected */
/* Meaning: Cache short names, taking more RAM, but faster look-ups */
#define CONFIG_YAFFS_SHORT_NAMES_IN_RAM

#endif /* YAFFS_OUT_OF_TREE */

#endif /* __YAFFS_CONFIG_H__ */
