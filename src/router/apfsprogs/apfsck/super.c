/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <apfs/parameters.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "apfsck.h"
#include "btree.h"
#include "crypto.h"
#include "dir.h"
#include "extents.h"
#include "htable.h"
#include "inode.h"
#include "object.h"
#include "snapshot.h"
#include "spaceman.h"
#include "super.h"

struct super_block *sb;
struct volume_superblock *vsb;

/**
 * is_power_of_two - Check if a number is a power of two
 * @n: the number to check
 */
static bool is_power_of_2(unsigned int n)
{
        return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * blksize_bits - Find the corresponding bit shift for a blocksize
 * @size: the blocksize
 */
static inline unsigned int blksize_bits(unsigned int size)
{
	unsigned int bits = 8;

	if (size < 4096)
		report("Container superblock", "block size is too small.");
	if (!is_power_of_2(size))
		report("Container superblock", "blocksize isn't power of two.");

	do {
		bits++;
		size >>= 1;
	} while (size > 256);
	return bits;
}

/**
 * read_super_copy - Read the copy of the container superblock in block 0
 *
 * Sets sb->s_blocksize and returns a pointer to the raw superblock in memory.
 */
static struct apfs_nx_superblock *read_super_copy(void)
{
	struct apfs_nx_superblock *msb_raw;
	int bsize_tmp;

	/*
	 * For now assume a small blocksize, we only need it so that we can
	 * read the actual blocksize from disk.
	 */
	bsize_tmp = APFS_NX_DEFAULT_BLOCK_SIZE;

	msb_raw = mmap(NULL, bsize_tmp, PROT_READ, MAP_PRIVATE,
		       fd, APFS_NX_BLOCK_NUM * bsize_tmp);
	if (msb_raw == MAP_FAILED)
		system_error();
	sb->s_blocksize = le32_to_cpu(msb_raw->nx_block_size);
	sb->s_blocksize_bits = blksize_bits(sb->s_blocksize);

	if (sb->s_blocksize != bsize_tmp) {
		munmap(msb_raw, bsize_tmp);

		msb_raw = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE,
			       fd, APFS_NX_BLOCK_NUM * sb->s_blocksize);
		if (msb_raw == MAP_FAILED)
			system_error();
	}

	if (le32_to_cpu(msb_raw->nx_magic) != APFS_NX_MAGIC)
		report("Block zero", "wrong magic.");
	if (!obj_verify_csum(&msb_raw->nx_o))
		report("Block zero", "bad checksum.");
	if (le64_to_cpu(msb_raw->nx_o.o_oid) != APFS_OID_NX_SUPERBLOCK)
		report("Block zero", "bad object id.");

	return msb_raw;
}

/**
 * read_latest_super - Read the latest checkpoint superblock
 * @base:	base of the checkpoint descriptor area
 * @blocks:	block count for the checkpoint descriptor area
 */
static struct apfs_nx_superblock *read_latest_super(u64 base, u32 blocks)
{
	struct apfs_nx_superblock *latest = NULL;
	u64 xid = 0;
	u64 bno;

	assert(sb->s_blocksize);

	for (bno = base; bno < base + blocks; ++bno) {
		struct apfs_nx_superblock *current;

		current = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE,
			       fd, bno * sb->s_blocksize);
		if (current == MAP_FAILED)
			system_error();

		if (le32_to_cpu(current->nx_magic) != APFS_NX_MAGIC)
			continue; /* Not a superblock */
		if (le64_to_cpu(current->nx_o.o_xid) <= xid)
			continue; /* Old */
		if (!obj_verify_csum(&current->nx_o))
			continue; /* Corrupted */

		xid = le64_to_cpu(current->nx_o.o_xid);
		latest = current;
	}

	if (!latest)
		report("Checkpoint descriptor area", "no valid superblock.");
	return latest;
}

/**
 * main_super_compare - Compare two copies of the container superblock
 * @desc: the superblock from the latest checkpoint
 * @copy: the superblock copy in block zero
 */
static void main_super_compare(struct apfs_nx_superblock *desc,
			       struct apfs_nx_superblock *copy)
{
	char *desc_bytes = (char *)desc;
	char *copy_bytes = (char *)copy;

	if (copy->nx_o.o_xid != desc->nx_o.o_xid) {
		report_crash("Block zero");
		return;
	}

	/*
	 * The nx_counters array doesn't always match, and it seems that the
	 * same is true of the flags for some reason. Naturally, this means
	 * the checksum won't match either.
	 */
	if (memcmp(desc_bytes + 0x08, copy_bytes + 0x08, 0x3D8 - 0x08) ||
	    memcmp(desc_bytes + 0x4D8, copy_bytes + 0x4D8, 0x4F0 - 0x4D8) ||
	    memcmp(desc_bytes + 0x4F8, copy_bytes + 0x4F8, 4096 - 0x4F8))
		report("Block zero", "fields don't match the checkpoint.");
}

/**
 * get_device_size - Get the block count of the device or image being checked
 * @blocksize: the filesystem blocksize
 */
static u64 get_device_size(unsigned int blocksize)
{
	struct stat buf;
	u64 size;

	if (fstat(fd, &buf))
		system_error();

	if ((buf.st_mode & S_IFMT) == S_IFREG)
		return buf.st_size / blocksize;

	if (ioctl(fd, BLKGETSIZE64, &size))
		system_error();
	return size / blocksize;
}

/**
 * get_max_volumes - Calculate the maximum number of volumes for the container
 * @size: the container size, in bytes
 */
static u32 get_max_volumes(u64 size)
{
	u32 max_vols;

	/* Divide by 512 MiB and round up, as the reference requires */
	max_vols = DIV_ROUND_UP(size, 512 * 1024 * 1024);
	if (max_vols > APFS_NX_MAX_FILE_SYSTEMS)
		max_vols = APFS_NX_MAX_FILE_SYSTEMS;
	return max_vols;
}

/**
 * check_main_flags - Check consistency of container flags
 * @flags: the flags
 */
static void check_main_flags(u64 flags)
{
	if ((flags & APFS_NX_FLAGS_VALID_MASK) != flags)
		report("Container superblock", "invalid flag in use.");
	if (flags & (APFS_NX_RESERVED_1 | APFS_NX_RESERVED_2))
		report("Container superblock", "reserved flag in use.");
	if (flags & APFS_NX_CRYPTO_SW)
		report_unknown("Software encryption");
}

/**
 * check_optional_main_features - Check the optional features of the container
 * @flags: the optional feature flags
 */
static void check_optional_main_features(u64 flags)
{
	if ((flags & APFS_NX_SUPPORTED_FEATURES_MASK) != flags)
		report("Container superblock", "unknown optional feature.");
	if (flags & APFS_NX_FEATURE_DEFRAG)
		report_unknown("Defragmentation");
	if (flags & APFS_NX_FEATURE_LCFD)
		report_unknown("Low-capacity fusion drive");
}

/**
 * check_rocompat_main_features - Check the ro compatible features of container
 * @flags: the read-only compatible feature flags
 */
static void check_rocompat_main_features(u64 flags)
{
	if ((flags & APFS_NX_SUPPORTED_ROCOMPAT_MASK) != flags)
		report("Container superblock", "unknown ro-compat feature.");
}

/**
 * check_incompat_main_features - Check the incompatible features of a container
 * @flags: the incompatible feature flags
 */
static void check_incompat_main_features(u64 flags)
{
	if ((flags & APFS_NX_SUPPORTED_INCOMPAT_MASK) != flags)
		report("Container superblock", "unknown incompatible feature.");
	if (flags & APFS_NX_INCOMPAT_VERSION1)
		report_unknown("APFS version 1");
	if (!(flags & APFS_NX_INCOMPAT_VERSION2))
		report_unknown("APFS versions other than 2");
	if (flags & APFS_NX_INCOMPAT_FUSION)
		report_unknown("Fusion drive");
}

/**
 * check_efi_information - Check the EFI info from the container superblock
 * @oid: the physical object id for the EFI driver record
 */
static void check_efi_information(u64 oid)
{
	struct apfs_nx_efi_jumpstart *efi;
	struct object obj;
	long long num_extents;
	u64 block_count = 0;
	u32 file_length;
	int i;

	if (!oid) /* Not all containers can be booted from, of course */
		return;

	efi = read_object(oid, NULL /* omap_table */, &obj);
	if (obj.type != APFS_OBJECT_TYPE_EFI_JUMPSTART)
		report("EFI info", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("EFI info", "wrong object subtype.");

	if (le32_to_cpu(efi->nej_magic) != APFS_NX_EFI_JUMPSTART_MAGIC)
		report("EFI info", "wrong magic.");
	if (le32_to_cpu(efi->nej_version) != APFS_NX_EFI_JUMPSTART_VERSION)
		report("EFI info", "wrong version.");
	for (i = 0; i < 16; ++i)
		if (efi->nej_reserved[i])
			report("EFI info", "reserved field in use.");

	num_extents = le32_to_cpu(efi->nej_num_extents);
	if (sizeof(*efi) + num_extents * sizeof(efi->nej_rec_extents[0]) >
								sb->s_blocksize)
		report("EFI info", "number of extents cannot fit.");
	for (i = 0; i < num_extents; ++i) {
		struct apfs_prange *ext = &efi->nej_rec_extents[i];
		u64 ext_blocks = le64_to_cpu(ext->pr_block_count);
		u64 ext_bno = le64_to_cpu(ext->pr_start_paddr);

		if (!ext_blocks)
			report("EFI info", "empty extent.");
		container_bmap_mark_as_used(ext_bno, ext_blocks);
		block_count += ext_blocks;
	}

	file_length = le32_to_cpu(efi->nej_efi_file_len);
	if (!file_length)
		report("EFI info", "driver is empty.");
	if (file_length > block_count * sb->s_blocksize)
		report("EFI info", "driver doesn't fit in extents.");
	if (file_length <= (block_count - 1) * sb->s_blocksize)
		report("EFI info", "wasted space in driver extents.");

	munmap(efi, obj.size);
}

/**
 * check_ephemeral_information - Check the container's array of ephemeral info
 * @info: pointer to the nx_ephemeral_info array on the container superblock
 */
static void check_ephemeral_information(__le64 *info)
{
	u64 min_block_count;
	u64 container_size;
	int i;

	assert(sb->s_block_count);
	container_size = sb->s_block_count * sb->s_blocksize;

	if (container_size < 128 * 1024 * 1024)
		min_block_count = main_fq_node_limit(sb->s_block_count);
	else
		min_block_count = APFS_NX_EPH_MIN_BLOCK_COUNT;

	if (le64_to_cpu(info[0]) != ((min_block_count << 32)
				  | (APFS_NX_MAX_FILE_SYSTEM_EPH_STRUCTS << 16)
				  | APFS_NX_EPH_INFO_VERSION_1))
		report("Container superblock",
		       "bad first entry in ephemeral info.");

	/* Only the first entry in the info array is documented */
	for (i = 1; i < APFS_NX_EPH_INFO_COUNT; ++i)
		if (info[i])
			report_unknown("Ephemeral info array");
}

/**
 * software_strlen - Calculate the length of a software info string
 * @str: the string
 *
 * Also checks that the string has a proper null-termination.
 */
static int software_strlen(u8 *str)
{
	int length = strnlen((char *)str, APFS_MODIFIED_NAMELEN);

	if (length == APFS_MODIFIED_NAMELEN)
		report("Volume software id", "no NULL-termination.");
	return length;
}

/**
 * check_software_info - Check the fields reporting implementation info
 * @formatted_by: information about the software that created the volume
 * @modified_by:  information about the software that modified the volume
 */
static void check_software_information(struct apfs_modified_by *formatted_by,
				       struct apfs_modified_by *modified_by)
{
	struct apfs_modified_by *end_mod_by;
	int length;
	bool mods_over;
	u64 xid;

	mods_over = false;
	end_mod_by = modified_by + APFS_MAX_HIST;
	xid = sb->s_xid + 1; /* Last possible xid */

	vsb->v_first_xid = le64_to_cpu(formatted_by->last_xid);
	vsb->v_last_xid = vsb->v_first_xid;

	for (; modified_by != end_mod_by; ++modified_by) {
		length = software_strlen(modified_by->id);
		if (!length &&
		    (modified_by->timestamp || modified_by->last_xid))
			report("Volume modification info", "entry without id.");

		if (mods_over) {
			if (length)
				report("Volume modification info",
				       "empty entry should end the list.");
			continue;
		}
		if (!length) {
			mods_over = true;
			continue;
		}

		/* The first entry must be the most recent */
		if (xid <= le64_to_cpu(modified_by->last_xid))
			report("Volume modification info",
			       "entries are not in order.");
		xid = le64_to_cpu(modified_by->last_xid);

		if (xid > vsb->v_last_xid)
			vsb->v_last_xid = xid;
	}

	length = software_strlen(formatted_by->id);
	if (!length)
		report("Volume superblock", "creation information is missing.");

	if (xid <= vsb->v_first_xid)
		report("Volume creation info", "transaction is too recent.");
}

/**
 * check_volume_flags - Check consistency of volume flags
 * @flags: the flags
 */
static void check_volume_flags(u64 flags)
{
	u64 incomp = le64_to_cpu(vsb->v_raw->apfs_incompatible_features);

	if ((flags & APFS_FS_FLAGS_VALID_MASK) != flags)
		report("Volume superblock", "invalid flag in use.");
	if (flags & APFS_FS_RESERVED_4)
		report("Volume superblock", "reserved flag in use.");

	if (!(flags & APFS_FS_UNENCRYPTED))
		vsb->v_encrypted = true;
	else if (flags & (APFS_FS_EFFACEABLE | APFS_FS_ONEKEY))
		report("Volume superblock", "inconsistent crypto flags.");

	if (flags & (APFS_FS_SPILLEDOVER | APFS_FS_RUN_SPILLOVER_CLEANER))
		report_unknown("Fusion drive");
	if (flags & APFS_FS_ALWAYS_CHECK_EXTENTREF)
		report_unknown("Forced extent reference checks");

	if (flags & APFS_FS_PREVIOUSLY_SEALED)
		report_unknown("Previously sealed volume");
	if (flags & APFS_FS_PFK)
		report_unknown("PFK volume flag");
	if (flags & APFS_FS_UNKNOWN_200)
		report_unknown("0x200 volume flag");

	if ((bool)(flags & APFS_FS_PFK) != (bool)(incomp & APFS_INCOMPAT_PFK))
		report("Volume superblock", "inconsistent PFK bits");
	if ((bool)(flags & APFS_FS_PREVIOUSLY_SEALED) && (bool)(incomp & APFS_INCOMPAT_SEALED_VOLUME))
		report("Volume superblock", "both sealed and unsealed");
	/* Got this check from the official fsck, no idea what it's about */
	if (!(bool)(flags & APFS_FS_SPILLEDOVER) && (bool)(incomp & APFS_INCOMPAT_SECONDARY_FSROOT))
		report("Volume superblock", "secondary fsroot wihout spillover");
}

/**
 * check_optional_vol_features - Check the optional features of a volume
 * @flags: the optional feature flags
 */
static void check_optional_vol_features(u64 flags)
{
	if ((flags & APFS_SUPPORTED_FEATURES_MASK) != flags)
		report("Volume superblock", "unknown optional feature.");
	if (flags & APFS_FEATURE_DEFRAG_PRERELEASE)
		report("Volume superblock", "prerelease defrag enabled.");

	/* TODO: should be easy to support, but I need an image for testing */
	if (!(flags & APFS_FEATURE_HARDLINK_MAP_RECORDS))
		report_unknown("Volume without sibling map records");
}

/**
 * check_rocompat_vol_features - Check the ro compatible features of a volume
 * @flags: the read-only compatible feature flags
 */
static void check_rocompat_vol_features(u64 flags)
{
	if ((flags & APFS_SUPPORTED_ROCOMPAT_MASK) != flags)
		report("Volume superblock", "unknown ro compatible feature.");
}

/**
 * check_incompat_vol_features - Check the incompatible features of a volume
 * @flags: the incompatible feature flags
 */
static void check_incompat_vol_features(u64 flags)
{
	if ((flags & APFS_SUPPORTED_INCOMPAT_MASK) != flags)
		report("Volume superblock", "unknown incompatible feature.");
	if (flags & APFS_INCOMPAT_PFK)
		report_unknown("PFK incompatible volume feature");
	if (flags & APFS_INCOMPAT_DATALESS_SNAPS)
		report_unknown("Dataless snapshots");
	if (flags & APFS_INCOMPAT_ENC_ROLLED)
		report_unknown("Change of encryption keys");
	if (flags & APFS_INCOMPAT_INCOMPLETE_RESTORE)
		report_unknown("Incomplete restore");
	if (flags &  APFS_INCOMPAT_SECONDARY_FSROOT)
		report_unknown("Secondary fsroot");

	if ((bool)(flags & APFS_INCOMPAT_CASE_INSENSITIVE) &&
	    (bool)(flags & APFS_INCOMPAT_NORMALIZATION_INSENSITIVE))
		report("Volume superblock", "redundant flag for case sensitivity.");
}

/**
 * role_is_valid - Does the give number match a possible volume role?
 * @role: the role number to check
 */
static bool role_is_valid(u16 role)
{
	switch (role) {
	case APFS_VOL_ROLE_NONE:
	case APFS_VOL_ROLE_SYSTEM:
	case APFS_VOL_ROLE_USER:
	case APFS_VOL_ROLE_RECOVERY:
	case APFS_VOL_ROLE_VM:
	case APFS_VOL_ROLE_PREBOOT:
	case APFS_VOL_ROLE_INSTALLER:
	case APFS_VOL_ROLE_DATA:
	case APFS_VOL_ROLE_BASEBAND:
	case APFS_VOL_ROLE_UPDATE:
	case APFS_VOL_ROLE_XART:
	case APFS_VOL_ROLE_HARDWARE:
	case APFS_VOL_ROLE_BACKUP:
	case APFS_VOL_ROLE_RESERVED_7:
	case APFS_VOL_ROLE_RESERVED_8:
	case APFS_VOL_ROLE_ENTERPRISE:
	case APFS_VOL_ROLE_RESERVED_10:
	case APFS_VOL_ROLE_PRELOGIN:
		return true;
	default:
		return false;
	}
}

/**
 * check_volume_role - Check that a volume's role flags are valid
 * @role: the volume role
 */
static void check_volume_role(u16 role)
{
	if (!role_is_valid(role))
		report("Volume superblock", "invalid role in use.");
	if (role == APFS_VOL_ROLE_RESERVED_7 || role == APFS_VOL_ROLE_RESERVED_8 || role == APFS_VOL_ROLE_RESERVED_10)
		report("Volume superblock", "reserved role in use.");
}

static bool meta_crypto_is_empty(struct apfs_wrapped_meta_crypto_state *wmcs)
{
	if (wmcs->major_version || wmcs->minor_version || wmcs->cpflags)
		return false;
	if (wmcs->persistent_class || wmcs->key_os_version)
		return false;
	if (wmcs->key_revision || wmcs->unused)
		return false;
	return true;
}

/**
 * check_meta_crypto - Check a volume's meta_crypto field
 * @wmcs: the structure to check
 */
static void check_meta_crypto(struct apfs_wrapped_meta_crypto_state *wmcs)
{
	/* This seems to contradict the reference, but it happens sometimes */
	if (meta_crypto_is_empty(wmcs))
		return;

	if (le16_to_cpu(wmcs->major_version) != APFS_WMCS_MAJOR_VERSION)
		report("Volume meta_crypto", "wrong major version.");
	if (le16_to_cpu(wmcs->minor_version) != APFS_WMCS_MINOR_VERSION)
		report("Volume meta_crypto", "wrong minor version.");

	if (wmcs->cpflags)
		report("Volume meta_crypto", "unknown flag.");

	if (le32_to_cpu(wmcs->persistent_class) != APFS_PROTECTION_CLASS_F)
		report_unknown("Encrypted metadata");
	if (le16_to_cpu(wmcs->key_revision) != 1) /* Key has been changed */
		report_unknown("Encrypted metadata");

	if (wmcs->unused)
		report("Volume meta_crypto", "reserved field in use.");
}

/**
 * get_volume_group - Find or create the volume group struct with the given uuid
 */
static struct volume_group *get_volume_group(char uuid[16])
{
	struct volume_group *vg = sb->s_volume_group;

	/* This shouldn't happen according to the reference, but it does */
	if (uuid_is_null(uuid))
		report_weird("Volume group uuid");

	if (vg) {
		if (memcmp(vg->vg_id, uuid, 16) != 0)
			report_unknown("Two volume groups");
		return vg;
	}

	vg = calloc(1, sizeof(*vg));
	if (!vg)
		system_error();
	memcpy(vg->vg_id, uuid, 16);
	sb->s_volume_group = vg;
	return vg;
}

/**
 * parse_volume_group_info - Parse the current volume's metadata for group info
 */
static void parse_volume_group_info(void)
{
	struct volume_group *vg = NULL;
	char *vg_uuid = vsb->v_raw->apfs_volume_group_id;

	if (apfs_volume_is_in_group()) {
		vg = get_volume_group(vg_uuid);
		if (vsb->v_in_snapshot)
			return;
		if (apfs_is_data_volume_in_group()) {
			if (vg->vg_data_seen)
				report("Volume group", "two data volumes.");
			vg->vg_data_seen = true;
		} else if (apfs_is_system_volume_in_group()) {
			if (vg->vg_system_seen)
				report("Volume group", "two system volumes.");
			vg->vg_system_seen = true;
		} else {
			report("Volume group", "volume is neither data nor system.");
		}
	} else {
		if (!uuid_is_null(vg_uuid))
			report("Volume group", "member has no feature flag.");
	}
}

static void parse_cloneinfo_epoch(struct volume_superblock *vsb)
{
	struct apfs_superblock *raw = vsb->v_raw;
	u64 id_epoch, xid;

	/*
	 * These "cloneinfo" fields are a way to determine if this volume was
	 * modified by an older, buggy implementation that may have corrupted
	 * the INODE_WAS_EVER_CLONED flags.
	 */
	id_epoch = le64_to_cpu(raw->apfs_cloneinfo_id_epoch);
	xid = le64_to_cpu(raw->apfs_cloneinfo_xid);

	if (id_epoch) {
		/*
		 * This is the only epoch I have encountered so far. It would
		 * appear to imply that the first inode created may have a
		 * corrupted flag, but that was probably not the intention. In
		 * fact, I've even seen a volume with this epoch that never had
		 * any user inodes at all.
		 */
		if (id_epoch != APFS_MIN_USER_INO_NUM)
			report_unknown("Cloneinfo id epoch");
	}

	if (xid) {
		/* I've never seen this either */
		if (xid != vsb->v_last_xid)
			report_unknown("Out of date cloneinfo xid");
	}

	/*
	 * The reference says that no xid implies no epoch, but that doesn't
	 * seem to be true for unmodified volumes.
	 */
	if (id_epoch && !xid) {
		if (vsb->v_first_xid != vsb->v_last_xid)
			report("Volume superblock", "cloneinfo epoch with no xid.");
	}
}

/**
 * integrity_meta_256_from_off - Get a pointer to the 256 bits on an im offset
 * @raw:	pointer to the raw integrity metadata
 * @offset:	offset of the 256-bit value in @raw
 */
static char *integrity_meta_256_from_off(struct apfs_integrity_meta_phys *raw, u32 offset)
{
	char *value_p = (char *)raw + offset;
	int sz_256 = 256 / 8;

	if (offset & 0x7)
		report("Integrity metadata", "offset is not aligned to 8 bytes.");
	if (offset < sizeof(*raw))
		report("Integrity metadata", "offset overlaps with structure.");
	if (offset >= sb->s_blocksize || offset + sz_256 > sb->s_blocksize)
		report("Integrity metadata", "offset is out of bounds.");
	return value_p;
}

static void parse_integrity_meta(u64 oid)
{
	struct apfs_integrity_meta_phys *meta = NULL;
	struct object obj;
	u32 flags, hash_type;
	char *hash = NULL;
	int i;

	assert(vsb->v_omap_table);

	meta = read_object(oid, vsb->v_omap_table, &obj);
	if (obj.type != APFS_OBJECT_TYPE_INTEGRITY_META)
		report("Integrity metadata", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Integrity metadata", "wrong object subtype.");

	if (le32_to_cpu(meta->im_version) == APFS_INTEGRITY_META_VERSION_INVALID)
		report("Integrity metadata", "invalid version.");
	if (le32_to_cpu(meta->im_version) > APFS_INTEGRITY_META_VERSION_HIGHEST)
		report("Integrity metadata", "undocumented new version.");

	flags = le32_to_cpu(meta->im_flags);
	if (flags & (~((u32)APFS_SEAL_BROKEN)))
		report("Integrity metadata", "undocumented flags.");
	if (flags & APFS_SEAL_BROKEN || meta->im_broken_xid)
		report_unknown("Broken seal volume");

	hash_type = le32_to_cpu(meta->im_hash_type);
	if (hash_type < APFS_HASH_MIN || hash_type > APFS_HASH_MAX)
		report("Integrity metadata", "undocumented hash type.");
	if (hash_type != APFS_HASH_SHA256)
		report_unknown("Unusual hash for sealed volume");

	hash = integrity_meta_256_from_off(meta, le32_to_cpu(meta->im_root_hash_offset));
	memcpy(vsb->v_hash, hash, 256 / 8);
	hash = NULL;

	for (i = 0; i < 9; ++i) {
		if (meta->im_reserved[i])
			report("Integrity metadata", "reserved field is in use.");
	}

	munmap(meta, obj.size);
}

/**
 * check_doc_id_index_flags - Check consistency of document id index flags
 * @flags: the flags
 */
static void check_doc_id_index_flags(u32 flags)
{
	if ((flags & APFS_DOC_ID_VALID_FLAGS) != flags)
		report("Document id index", "invalid flag in use.");
	if (flags)
		report_unknown("Document id index flags");
}

/**
 * read_volume_super - Read the volume superblock and run some checks
 * @vol:	volume number
 * @vsb:	volume superblock struct to receive the results
 * @obj:	volume superblock object
 */
void read_volume_super(int vol, struct volume_superblock *vsb, struct object *obj)
{
	char *vol_name = NULL;
	struct spaceman *sm = &sb->s_spaceman;
	u64 alloc_count, reserve_blkcnt, quota_blkcnt;

	if (vsb->v_obj.type != APFS_OBJECT_TYPE_FS)
		report("Volume superblock", "wrong object type.");
	if (vsb->v_obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Volume superblock", "wrong object subtype.");

	vsb->v_index = le32_to_cpu(vsb->v_raw->apfs_fs_index);
	if (vsb->v_index != vol)
		report("Volume superblock", "wrong reported volume number.");
	if (le32_to_cpu(vsb->v_raw->apfs_magic) != APFS_MAGIC)
		report("Volume superblock", "wrong magic.");

	check_optional_vol_features(le64_to_cpu(vsb->v_raw->apfs_features));
	check_rocompat_vol_features(le64_to_cpu(
				vsb->v_raw->apfs_readonly_compatible_features));
	check_incompat_vol_features(le64_to_cpu(
				vsb->v_raw->apfs_incompatible_features));

	alloc_count = le64_to_cpu(vsb->v_raw->apfs_fs_alloc_count);
	reserve_blkcnt = le64_to_cpu(vsb->v_raw->apfs_fs_reserve_block_count);
	quota_blkcnt = le64_to_cpu(vsb->v_raw->apfs_fs_quota_block_count);
	if (reserve_blkcnt) {
		sm->sm_reserve_block_num += reserve_blkcnt;
		if (alloc_count > reserve_blkcnt)
			sm->sm_reserve_alloc_num += reserve_blkcnt;
		else
			sm->sm_reserve_alloc_num += alloc_count;
	}
	if (quota_blkcnt) {
		if (alloc_count > quota_blkcnt)
			report("Volume superblock", "exceeded allocation quota.");
		/*
		 * These are actually equal in the few cases I've observed, but
		 * I highly doubt that's a general rule.
		 */
		if (reserve_blkcnt > quota_blkcnt)
			report("Volume superblock", "block reserves exceed quota.");
	}

	check_meta_crypto(&vsb->v_raw->apfs_meta_crypto);

	vsb->v_next_obj_id = le64_to_cpu(vsb->v_raw->apfs_next_obj_id);
	if (vsb->v_next_obj_id < APFS_MIN_USER_INO_NUM)
		report("Volume superblock", "next catalog id is invalid.");
	vsb->v_next_doc_id = le32_to_cpu(vsb->v_raw->apfs_next_doc_id);
	if (vsb->v_next_doc_id < APFS_MIN_DOC_ID)
		report("Volume superblock", "next document id is invalid.");

	vol_name = (char *)vsb->v_raw->apfs_volname;
	if (!*vol_name)
		report("Volume superblock", "label is missing.");
	if (strnlen(vol_name, APFS_VOLNAME_LEN) == APFS_VOLNAME_LEN)
		report("Volume superblock", "name lacks NULL-termination.");

	check_volume_flags(le64_to_cpu(vsb->v_raw->apfs_fs_flags));
	check_software_information(&vsb->v_raw->apfs_formatted_by,
				   &vsb->v_raw->apfs_modified_by[0]);
	check_volume_role(le16_to_cpu(vsb->v_raw->apfs_role));

	/*
	 * The documentation suggests that other tree types could be possible,
	 * but I don't understand how that would work.
	 */
	if (le32_to_cpu(vsb->v_raw->apfs_root_tree_type) !=
				(APFS_OBJ_VIRTUAL | APFS_OBJECT_TYPE_BTREE))
		report("Volume superblock", "wrong type for catalog tree.");
	if (le32_to_cpu(vsb->v_raw->apfs_extentref_tree_type) !=
				(APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
		report("Volume superblock", "wrong type for extentref tree.");
	if (le32_to_cpu(vsb->v_raw->apfs_snap_meta_tree_type) !=
				(APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
		report("Volume superblock", "wrong type for snapshot tree.");

	/* The official fsck performs this one check on these two fields */
	if (le64_to_cpu(vsb->v_raw->apfs_total_blocks_freed) > le64_to_cpu(vsb->v_raw->apfs_total_blocks_alloced))
		report("Volume superblock", "more blocks freed than ever alloced.");

	if (le16_to_cpu(vsb->v_raw->reserved) != 0)
		report("Volume superblock", "reserved field is in use.");
	if (le64_to_cpu(vsb->v_raw->apfs_root_to_xid) != 0)
		report_unknown("Root from snapshot");
	if (le64_to_cpu(vsb->v_raw->apfs_er_state_oid) != 0)
		report_unknown("Encryption or decryption in progress");
	if (le64_to_cpu(vsb->v_raw->apfs_revert_to_xid) != 0)
		report_unknown("Revert to a snapshot");
	if (le64_to_cpu(vsb->v_raw->apfs_revert_to_sblock_oid) != 0)
		report_unknown("Revert to a volume superblock");

	parse_cloneinfo_epoch(vsb);

	if (apfs_volume_is_sealed()) {
		/* The reference seems to be wrong about the role */
		if (apfs_volume_role() && apfs_volume_role() != APFS_VOL_ROLE_SYSTEM)
			report("Sealed volume", "wrong role.");
		if (le32_to_cpu(vsb->v_raw->apfs_fext_tree_type) != (APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
			report("Sealed volume", "invalid value of fext tree type.");
		if (!vsb->v_raw->apfs_fext_tree_oid)
			report("Sealed volume", "missing fext tree.");
		if (!vsb->v_raw->apfs_integrity_meta_oid)
			report("Sealed volume", "missing integrity metadata.");
	} else {
		if (vsb->v_raw->apfs_fext_tree_oid || vsb->v_raw->apfs_integrity_meta_oid)
			report("Volume superblock", "no sealed feature flag.");
		if (vsb->v_raw->apfs_fext_tree_type && le32_to_cpu(vsb->v_raw->apfs_fext_tree_type) != (APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
			report("Volume superblock", "invalid value of fext tree type.");
	}

	if (vsb->v_raw->reserved_type && le32_to_cpu(vsb->v_raw->reserved_type) != (APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
		report("Volume superblock", "invalid value of reserved type.");
	if (vsb->v_raw->reserved_oid)
		report("Volume superblock", "reserved oid is set.");

	parse_volume_group_info();

	check_doc_id_index_flags(le32_to_cpu(vsb->v_raw->apfs_doc_id_index_flags));

	vsb->v_extref_oid = le64_to_cpu(vsb->v_raw->apfs_extentref_tree_oid);
	vsb->v_omap_oid = le64_to_cpu(vsb->v_raw->apfs_omap_oid);
	vsb->v_snap_meta_oid = le64_to_cpu(vsb->v_raw->apfs_snap_meta_tree_oid);
	vsb->v_fext_tree_oid = le64_to_cpu(vsb->v_raw->apfs_fext_tree_oid);
	vsb->v_integrity_oid = le64_to_cpu(vsb->v_raw->apfs_integrity_meta_oid);
}

/**
 * map_volume_super - Find the volume superblock and map it into memory
 * @vol:	volume number
 * @vsb:	volume superblock struct to receive the results
 *
 * Returns the in-memory location of the volume superblock, or NULL if there
 * is no volume with this number.
 */
static struct apfs_superblock *map_volume_super(int vol,
						struct volume_superblock *vsb)
{
	struct apfs_nx_superblock *msb_raw = sb->s_raw;
	u64 vol_id;

	vol_id = le64_to_cpu(msb_raw->nx_fs_oid[vol]);
	if (vol_id == 0)
		return NULL;
	if (vol > sb->s_max_vols)
		report("Container superblock", "too many volumes.");

	vsb->v_raw = read_object(vol_id, sb->s_omap_table, &vsb->v_obj);
	read_volume_super(vol, vsb, &vsb->v_obj);
	return vsb->v_raw;
}

static struct object *parse_reaper(u64 oid);

/**
 * check_volume_group - Check that a volume group (if any) was complete
 * @vg: the volume group (may be NULL)
 */
static void check_volume_group(struct volume_group *vg)
{
	if (!vg)
		return;
	if (!vg->vg_system_seen)
		report("Volume group", "system volume is missing.");
	if (!vg->vg_data_seen)
		report_weird("Volume group with no data");
}

/**
 * alloc_volume_super - Allocate an in-memory volume superblock struct
 * @snap:	is this a snapshot?
 */
struct volume_superblock *alloc_volume_super(bool snap)
{
	struct volume_superblock *ret = NULL;

	ret = calloc(1, sizeof(*ret));
	if (!ret)
		system_error();
	ret->v_in_snapshot = snap;

	if (!snap) {
		ret->v_omap_table = alloc_htable();
		ret->v_snap_table = alloc_htable();
		ret->v_extent_table = alloc_htable();
	}
	ret->v_cnid_table = alloc_htable();
	ret->v_dstream_table = alloc_htable();
	ret->v_inode_table = alloc_htable();
	ret->v_dirstat_table = alloc_htable();
	ret->v_crypto_table = alloc_htable();

	return ret;
}

static void check_snap_meta_ext(u64 oid)
{
	struct apfs_snap_meta_ext *sme = NULL;
	struct object obj;

	/* Older implementations of apfs just don't set this */
	if (!oid)
		return;

	if (vsb->v_snap_max_xid == 0)
		report("Volume superblock", "has extended snap meta but no snapshots.");

	sme = read_object(oid, vsb->v_omap_table, &obj);
	if (obj.type != APFS_OBJECT_TYPE_SNAP_META_EXT)
		report("Extended snapshot metadata", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Extended snapshot metadata", "wrong object subtype.");

	if (le32_to_cpu(sme->sme_version) != 1)
		report("Extended snapshot metadata", "wrong version.");
	if (sme->sme_flags)
		report("Extended snapshot metadata", "undocumented flags.");
	if (!sme->sme_snap_xid)
		report("Extended snapshot metadata", "null transaction id.");

	/*
	 * The current transaction has the same content as the latest snapshot,
	 * but this may be impossible to check if that snapshot got deleted.
	 */
	if (vsb->v_in_snapshot && le64_to_cpu(sme->sme_snap_xid) != sb->s_xid)
		report("Extended snapshot metadata", "wrong transaction id.");

	munmap(sme, obj.size);
}

/**
 * check_volume_super - Parse and check the whole current volume superblock
 */
void check_volume_super(void)
{
	struct apfs_superblock *vsb_raw = vsb->v_raw;

	if (!vsb->v_in_snapshot) {
		vsb->v_omap = parse_omap_btree(vsb->v_omap_oid);
		vsb->v_snap_meta = parse_snap_meta_btree(vsb->v_snap_meta_oid);
	}

	/* A virtual object, so it must be parsed after the omap */
	if (apfs_volume_is_sealed())
		parse_integrity_meta(vsb->v_integrity_oid);

	/* The first tree is for the latest xid, the others are for snapshots */
	if (!vsb->v_in_snapshot) {
		vsb->v_extent_ref = parse_extentref_btree(vsb->v_extref_oid);
	} else {
		/* We want the most recent snapshots first */
		struct listed_btree *new = NULL;

		new = calloc(1, sizeof(*new));
		if (!new)
			system_error();
		new->btree = parse_extentref_btree(vsb->v_extref_oid);
		new->next = vsb->v_snap_extrefs;
		vsb->v_snap_extrefs = new;
	}

	if (apfs_volume_is_sealed())
		vsb->v_fext = parse_fext_btree(vsb->v_fext_tree_oid);
	vsb->v_cat = parse_cat_btree(le64_to_cpu(vsb_raw->apfs_root_tree_oid), vsb->v_omap_table);

	check_snap_meta_ext(le64_to_cpu(vsb_raw->apfs_snap_meta_ext_oid));

	if (!vsb->v_in_snapshot) {
		free_snap_table(vsb->v_snap_table);
		vsb->v_snap_table = NULL;
	}
	free_inode_table(vsb->v_inode_table);
	vsb->v_inode_table = NULL;
	free_dstream_table(vsb->v_dstream_table);
	vsb->v_dstream_table = NULL;
	free_cnid_table(vsb->v_cnid_table);
	vsb->v_cnid_table = NULL;
	if (!vsb->v_in_snapshot) {
		free_extent_table(vsb->v_extent_table);
		vsb->v_extent_table = NULL;
		free_omap_table(vsb->v_omap_table);
		vsb->v_omap_table = NULL;
	} else {
		check_and_reset_extent_table(vsb->v_extent_table);
	}
	free_dirstat_table(vsb->v_dirstat_table);
	vsb->v_dirstat_table = NULL;
	free_crypto_table(vsb->v_crypto_table);
	vsb->v_crypto_table = NULL;

	if (!vsb->v_has_root)
		report("Catalog", "the root directory is missing.");
	if (!vsb->v_has_priv)
		report("Catalog", "the private directory is missing.");

	if (le64_to_cpu(vsb_raw->apfs_num_files) != vsb->v_file_count) {
		/* Sometimes this is off by one.  TODO: why? */
		report_weird("File count in volume superblock");
	}
	if (le64_to_cpu(vsb_raw->apfs_num_directories) != vsb->v_dir_count)
		report("Volume superblock", "bad directory count.");
	if (le64_to_cpu(vsb_raw->apfs_num_symlinks) != vsb->v_symlink_count)
		report("Volume superblock", "bad symlink count.");
	if (le64_to_cpu(vsb_raw->apfs_num_other_fsobjects) != vsb->v_special_count)
		report("Volume superblock", "bad special file count.");

	/*
	 * Older snapshots may have been deleted after this one was created, so
	 * there is no way to check apfs_num_snapshots exactly.
	 *
	 * TODO: check that each snapshot has more snaps than the previous one?
	 */
	if (vsb->v_in_snapshot) {
		if (le64_to_cpu(vsb_raw->apfs_num_snapshots) < vsb->v_snap_count)
			report("Volume superblock", "bad snapshot count.");
	} else {
		if (le64_to_cpu(vsb_raw->apfs_num_snapshots) != vsb->v_snap_count)
			report("Volume superblock", "bad snapshot count.");
	}

	/*
	 * The original omap for a snapshot is not preserved, so there is no way
	 * to know the real value of v_block_count back then.
	 */
	if (!vsb->v_in_snapshot) {
		if (le64_to_cpu(vsb_raw->apfs_fs_alloc_count) != vsb->v_block_count)
			report("Volume superblock", "bad block count.");
	}
}

/**
 * check_container - Check the whole container for a given checkpoint
 * @sb: checkpoint superblock
 */
static void check_container(struct super_block *sb)
{
	int vol;
	bool reaper_vol_seen = false;

	sb->s_omap_table = alloc_htable();

	/* Check for corruption in the container object map... */
	sb->s_omap = parse_omap_btree(le64_to_cpu(sb->s_raw->nx_omap_oid));
	/* ...and in the reaper */
	sb->s_reaper = parse_reaper(le64_to_cpu(sb->s_raw->nx_reaper_oid));

	for (vol = 0; vol < APFS_NX_MAX_FILE_SYSTEMS; ++vol) {
		struct apfs_superblock *vsb_raw;

		vsb = alloc_volume_super(false);

		vsb_raw = map_volume_super(vol, vsb);
		if (!vsb_raw) {
			/*
			 * Containers typically have all of their volumes at the
			 * beginning of the array, but I've encountered images
			 * where this isn't true. I guess it makes sense if
			 * volumes can be deleted?
			 */
			free(vsb);
			continue;
		}
		if (vsb->v_obj.oid == sb->s_reaper_fs_id)
			reaper_vol_seen = true;
		check_volume_super();
		sb->s_volumes[vol] = vsb;
		vsb = NULL;
	}

	/* For now we just check that the reaper's volume exists */
	if (sb->s_reaper_fs_id && !reaper_vol_seen)
		report("Reaper", "volume id is invalid.");

	free_omap_table(sb->s_omap_table);
	sb->s_omap_table = NULL;

	check_spaceman(le64_to_cpu(sb->s_raw->nx_spaceman_oid));

	check_volume_group(sb->s_volume_group);
	free(sb->s_volume_group);
	sb->s_volume_group = NULL;
}

/**
 * parse_main_super - Parse a container superblock and run generic checks
 * @sb: checkpoint superblock struct to receive the results
 */
static void parse_main_super(struct super_block *sb)
{
	u64 chunk_count;
	int i;

	assert(sb->s_raw);

	/* This field was already set from the checkpoint mappings */
	assert(sb->s_xid);

	if (sb->s_xid != le64_to_cpu(sb->s_raw->nx_o.o_xid))
		report("Container superblock", "inconsistent xid.");

	sb->s_blocksize = le32_to_cpu(sb->s_raw->nx_block_size);
	if (sb->s_blocksize != APFS_NX_DEFAULT_BLOCK_SIZE)
		report_unknown("Block size other than 4096");

	sb->s_block_count = le64_to_cpu(sb->s_raw->nx_block_count);
	if (!sb->s_block_count)
		report("Container superblock", "reports no block count.");
	if (sb->s_block_count > get_device_size(sb->s_blocksize))
		report("Container superblock", "too many blocks for device.");

	/*
	 * A chunk is the disk section covered by a single block in the
	 * allocation bitmap.
	 */
	chunk_count = DIV_ROUND_UP(sb->s_block_count, 8 * sb->s_blocksize);
	sb->s_bitmap = calloc(chunk_count, sb->s_blocksize);
	if (!sb->s_bitmap)
		system_error();
	((char *)sb->s_bitmap)[0] = 0x01; /* Block zero is always used */

	sb->s_max_vols = get_max_volumes(sb->s_block_count * sb->s_blocksize);
	if (sb->s_max_vols != le32_to_cpu(sb->s_raw->nx_max_file_systems))
		report("Container superblock", "bad maximum volume number.");

	check_main_flags(le64_to_cpu(sb->s_raw->nx_flags));
	check_optional_main_features(le64_to_cpu(sb->s_raw->nx_features));
	check_rocompat_main_features(le64_to_cpu(
				sb->s_raw->nx_readonly_compatible_features));
	check_incompat_main_features(le64_to_cpu(
				sb->s_raw->nx_incompatible_features));

	if (le32_to_cpu(sb->s_raw->nx_xp_desc_blocks) >> 31 ||
	    le32_to_cpu(sb->s_raw->nx_xp_data_blocks) >> 31 ||
	    le64_to_cpu(sb->s_raw->nx_xp_desc_base) >> 63 ||
	    le64_to_cpu(sb->s_raw->nx_xp_data_base) >> 63)
		report("Container superblock", "has checkpoint tree.");

	sb->s_data_base = le64_to_cpu(sb->s_raw->nx_xp_data_base);
	sb->s_data_blocks = le32_to_cpu(sb->s_raw->nx_xp_data_blocks);
	sb->s_data_index = le32_to_cpu(sb->s_raw->nx_xp_data_index);
	sb->s_data_len = le32_to_cpu(sb->s_raw->nx_xp_data_len);
	if (sb->s_data_index >= sb->s_data_blocks)
		report("Container superblock", "out of range checkpoint data.");
	if (sb->s_data_len > sb->s_data_blocks)
		report("Container superblock",
		       "reports too many blocks of checkpoint data.");
	if ((sb->s_data_index + sb->s_data_len) % sb->s_data_blocks !=
	    le32_to_cpu(sb->s_raw->nx_xp_data_next))
		report("Container superblock",
		       "wrong length for checkpoint data.");

	if (sb->s_raw->nx_test_type || sb->s_raw->nx_test_oid)
		report("Container superblock", "test field is set.");
	if (sb->s_raw->nx_blocked_out_prange.pr_block_count)
		report_unknown("Partition resizing");

	check_efi_information(le64_to_cpu(sb->s_raw->nx_efi_jumpstart));
	check_ephemeral_information(&sb->s_raw->nx_ephemeral_info[0]);

	for (i = 0; i < 16; ++i) {
		if (sb->s_raw->nx_fusion_uuid[i])
			report_unknown("Fusion drive");
	}

	/* Containers with no encryption may still have a value here, why? */
	check_keybag(le64_to_cpu(sb->s_raw->nx_keylocker.pr_start_paddr), le64_to_cpu(sb->s_raw->nx_keylocker.pr_block_count));
	/* TODO: actually check all this stuff */
	container_bmap_mark_as_used(le64_to_cpu(sb->s_raw->nx_mkb_locker.pr_start_paddr), le64_to_cpu(sb->s_raw->nx_mkb_locker.pr_block_count));

	if (sb->s_raw->nx_fusion_mt_oid || sb->s_raw->nx_fusion_wbc_oid ||
	    sb->s_raw->nx_fusion_wbc.pr_start_paddr ||
	    sb->s_raw->nx_fusion_wbc.pr_block_count)
		report_unknown("Fusion drive");

	sb->s_next_oid = le64_to_cpu(sb->s_raw->nx_next_oid);
	if (sb->s_xid + 1 != le64_to_cpu(sb->s_raw->nx_next_xid))
		report("Container superblock", "next transaction id is wrong.");
}

/**
 * parse_cpoint_map - Parse and verify a checkpoint mapping
 * @raw: the raw checkpoint mapping
 */
static void parse_cpoint_map(struct apfs_checkpoint_mapping *raw)
{
	struct cpoint_map *map;

	map = get_cpoint_map(le64_to_cpu(raw->cpm_oid));
	if (map->m_paddr)
		report("Checkpoint maps", "two mappings for the same oid.");
	if (!raw->cpm_paddr)
		report("Checkpoint map", "invalid physical address.");
	map->m_paddr = le64_to_cpu(raw->cpm_paddr);

	map->m_type = le32_to_cpu(raw->cpm_type);
	map->m_subtype = le32_to_cpu(raw->cpm_subtype);

	map->m_size = le32_to_cpu(raw->cpm_size);
	if (map->m_size & (sb->s_blocksize - 1))
		report("Checkpoint map", "size isn't multiple of block size.");
	if ((map->m_type & APFS_OBJECT_TYPE_MASK) != APFS_OBJECT_TYPE_SPACEMAN) {
		if (map->m_size != sb->s_blocksize)
			report_unknown("Large non-spaceman ephemeral objects");
	}

	if (raw->cpm_pad)
		report("Checkpoint map", "non-zero padding.");
	if (raw->cpm_fs_oid)
		report_unknown("Ephemeral object belonging to a volume");
}

/**
 * parse_cpoint_map_blocks - Parse and verify a checkpoint's mapping blocks
 * @desc_base:		first block of the checkpoint descriptor area
 * @desc_blocks:	block count of the checkpoint descriptor area
 * @index:		index of the first mapping block for the checkpoint
 *
 * Returns the number of checkpoint-mapping blocks, and sets @index to the
 * index of their checkpoint superblock.
 */
static u32 parse_cpoint_map_blocks(u64 desc_base, u32 desc_blocks, u32 *index)
{
	struct object obj;
	struct apfs_checkpoint_map_phys *raw;
	u32 blk_count = 0;
	u32 cpm_count;

	/*
	 * The current superblock hasn't been parsed yet, so this xid would be
	 * from the previous checkpoint.
	 */
	assert(!sb->s_xid);

	assert(!sb->s_cpoint_map_table);
	sb->s_cpoint_map_table = alloc_htable();

	while (1) {
		u64 bno = desc_base + *index;
		u32 flags;
		int i;

		raw = read_object_nocheck(bno, sb->s_blocksize, &obj);
		if (obj.oid != bno)
			report("Checkpoint map", "wrong object id.");
		if (parse_object_flags(obj.flags, false) != APFS_OBJ_PHYSICAL)
			report("Checkpoint map", "wrong storage type.");
		if (obj.type != APFS_OBJECT_TYPE_CHECKPOINT_MAP)
			report("Checkpoint map", "wrong object type.");
		if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
			report("Checkpoint map", "wrong object subtype.");

		/* Checkpoint mappings belong to the checkpoint transaction */
		if (sb->s_xid && obj.xid != sb->s_xid)
			report("Checkpoint map", "inconsistent xid.");
		if (!obj.xid)
			report("Checkpoint map", "invalid xid.");
		sb->s_xid = obj.xid;

		cpm_count = le32_to_cpu(raw->cpm_count);
		if (sizeof(*raw) + cpm_count * sizeof(raw->cpm_map[0]) >
								sb->s_blocksize)
			report("Checkpoint maps", "won't fit in block.");
		for (i = 0; i < cpm_count; ++i)
			parse_cpoint_map(&raw->cpm_map[i]);

		flags = le32_to_cpu(raw->cpm_flags);

		munmap(raw, obj.size);
		blk_count++;
		*index = (*index + 1) % desc_blocks;

		if ((flags & APFS_CHECKPOINT_MAP_LAST) != flags)
			report("Checkpoint map", "invalid flag in use.");
		if (flags & APFS_CHECKPOINT_MAP_LAST)
			return blk_count;
		if (blk_count == desc_blocks)
			report("Checkpoint", "no mapping block marked last.");
	}
}

/**
 * parse_filesystem - Parse the whole filesystem looking for corruption
 */
void parse_filesystem(void)
{
	struct apfs_nx_superblock *msb_raw_copy, *msb_raw_latest;
	u64 desc_base;
	u32 desc_blocks;
	long long valid_blocks;
	u32 desc_next, desc_index, index;

	sb = calloc(1, sizeof(*sb));
	if (!sb)
		system_error();

	/* Read the superblock from the last clean unmount */
	msb_raw_copy = read_super_copy();

	/* We want to mount the latest valid checkpoint among the descriptors */
	desc_base = le64_to_cpu(msb_raw_copy->nx_xp_desc_base);
	if (desc_base >> 63 != 0) {
		/* The highest bit is set when checkpoints are not contiguous */
		report("Block zero",
		       "checkpoint descriptor tree not yet supported.");
	}
	desc_blocks = le32_to_cpu(msb_raw_copy->nx_xp_desc_blocks);
	if (desc_blocks > 10000) /* Arbitrary loop limit, is it enough? */
		report("Block zero", "too many checkpoint descriptors?");

	/* Find the valid range, as reported by the latest descriptor */
	msb_raw_latest = read_latest_super(desc_base, desc_blocks);
	desc_next = le32_to_cpu(msb_raw_latest->nx_xp_desc_next);
	desc_index = le32_to_cpu(msb_raw_latest->nx_xp_desc_index);
	if (desc_next >= desc_blocks || desc_index >= desc_blocks)
		report("Checkpoint superblock",
		       "out of range checkpoint descriptors.");
	munmap(msb_raw_latest, sb->s_blocksize);
	msb_raw_latest = NULL;

	/*
	 * Now go through the valid checkpoints one by one, though it seems
	 * that cleanly unmounted filesystems only preserve the last one.
	 */
	index = desc_index;
	valid_blocks = (desc_blocks + desc_next - desc_index) % desc_blocks;
	while (valid_blocks > 0) {
		struct object obj;
		struct apfs_nx_superblock *raw;
		u64 bno;
		u32 map_blocks;

		/* Some fields from the previous checkpoint need to be unset */
		if (sb->s_raw)
			munmap(sb->s_raw, sb->s_blocksize);
		sb->s_raw = NULL;
		sb->s_xid = 0;
		free(sb->s_bitmap);

		/* The checkpoint-mapping blocks come before the superblock */
		map_blocks = parse_cpoint_map_blocks(desc_base, desc_blocks,
						     &index);
		valid_blocks -= map_blocks;

		bno = desc_base + index;
		raw = read_object_nocheck(bno, sb->s_blocksize, &obj);
		if (parse_object_flags(obj.flags, false) != APFS_OBJ_EPHEMERAL)
			report("Checkpoint superblock", "bad storage type.");
		if (obj.type != APFS_OBJECT_TYPE_NX_SUPERBLOCK)
			report("Checkpoint superblock", "bad object type.");
		if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
			report("Checkpoint superblock", "bad object subtype.");

		if (le32_to_cpu(raw->nx_magic) != APFS_NX_MAGIC)
			report("Checkpoint superblock", "wrong magic.");
		if (le32_to_cpu(raw->nx_xp_desc_len) != map_blocks + 1)
			report("Checkpoint superblock",
			       "wrong checkpoint descriptor block count.");

		sb->s_raw = raw;
		parse_main_super(sb);

		/* Do this now, after parse_main_super() allocated the bitmap */
		container_bmap_mark_as_used(desc_base, desc_blocks);
		container_bmap_mark_as_used(sb->s_data_base, sb->s_data_blocks);

		check_container(sb);

		free_cpoint_map_table(sb->s_cpoint_map_table);
		sb->s_cpoint_map_table = NULL;

		/* One more block for the checkpoint superblock itself */
		index = (index + 1) % desc_blocks;
		valid_blocks--;
	}

	if (valid_blocks != 0)
		report("Block zero", "bad index for checkpoint descriptors.");

	if (!sb->s_raw)
		report("Checkpoint descriptor area", "no valid superblocks.");
	main_super_compare(sb->s_raw, msb_raw_copy);
	munmap(msb_raw_copy, sb->s_blocksize);
}

/**
 * parse_reaper - Parse the reaper and check for corruption
 * @oid: object id for the reaper
 *
 * Returns a pointer to the object struct for the reaper.
 */
static struct object *parse_reaper(u64 oid)
{
	struct apfs_nx_reaper_phys *raw;
	struct object *reaper;
	u32 flags, buffer_size;
	int i;

	reaper = calloc(1, sizeof(*reaper));
	if (!reaper)
		system_error();

	raw = read_ephemeral_object(oid, reaper);
	if (reaper->type != APFS_OBJECT_TYPE_NX_REAPER)
		report("Reaper", "wrong object type.");
	if (reaper->subtype != APFS_OBJECT_TYPE_INVALID)
		report("Reaper", "wrong object subtype.");

	buffer_size = le32_to_cpu(raw->nr_state_buffer_size);
	if (buffer_size != sb->s_blocksize - sizeof(*raw))
		report("Reaper", "wrong state buffer size.");

	/* Docs on the reaper are very incomplete, so let's hope it's empty */
	if (raw->nr_head) {
		struct apfs_nx_reap_list_phys *list_raw = NULL;
		struct object list = {0};

		sb->s_reaper_fs_id = le64_to_cpu(raw->nr_fs_oid);

		if (le64_to_cpu(raw->nr_next_reap_id) <= le64_to_cpu(raw->nr_completed_id))
			report("Reaper", "next read id before completed.");

		if (raw->nr_tail != raw->nr_head)
			report_unknown("Nonempty reaper");
		if (le64_to_cpu(raw->nr_head) - le64_to_cpu(raw->nr_tail) + 1 != le32_to_cpu(raw->nr_rlcount))
			report("Reaper", "wrong reap list count.");

		list_raw = read_ephemeral_object(le64_to_cpu(raw->nr_head), &list);
		if (list.type != APFS_OBJECT_TYPE_NX_REAP_LIST)
			report("Reaper list", "wrong object type.");
		if (list.subtype != APFS_OBJECT_TYPE_INVALID)
			report("Reaper list", "wrong object subtype.");

		if (list_raw->nrl_max != cpu_to_le32((sb->s_blocksize - sizeof(*list_raw)) / sizeof(struct apfs_nx_reap_list_entry)))
			report("Reaper list", "wrong maximum entry count.");

		if (list_raw->nrl_next || list_raw->nrl_flags || list_raw->nrl_count)
			report_unknown("Nonempty reaper list");
		if (list_raw->nrl_first != cpu_to_le32(-1) || list_raw->nrl_last != cpu_to_le32(-1))
			report_unknown("Nonempty reaper list");
		/* TODO: nrl_free? */

		munmap(list_raw, list.size);
	} else {
		if (raw->nr_completed_id || raw->nr_head || raw->nr_rlcount || raw->nr_type)
			report("Reaper", "should be empty.");
		if (raw->nr_size || raw->nr_oid || raw->nr_xid || raw->nr_nrle_flags)
			report("Reaper", "should be empty.");
		if (le64_to_cpu(raw->nr_next_reap_id) != 1)
			report("Reaper", "should be empty.");
		for (i = 0; i < buffer_size; ++i) {
			if (raw->nr_state_buffer[i])
				report("Reaper", "should be empty.");
		}
		if (raw->nr_fs_oid)
			report("Reaper", "is empty but belongs to a volume.");
	}

	flags = le32_to_cpu(raw->nr_flags);
	if ((flags & APFS_NR_FLAGS_VALID_MASK) != flags)
		report("Reaper", "invalid flag in use.");
	if (!(flags & APFS_NR_BHM_FLAG))
		report("Reaper", "reserved flag must always be set.");
	if (flags & APFS_NR_CONTINUE)
		report_unknown("Object being reaped");

	munmap(raw, reaper->size);
	return reaper;
}
