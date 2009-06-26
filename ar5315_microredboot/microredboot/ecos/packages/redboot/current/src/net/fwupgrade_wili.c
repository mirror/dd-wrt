/*
firmware upgrade code for DD-WRT webflash images
*/

#include <redboot.h>
#include <cyg/io/flash.h>
#include <fis.h>
#include <flash_config.h>
#include "fwupgrade_wili.h"

/* some variables from flash.c */
extern void *flash_start, *flash_end;
extern int flash_block_size, flash_num_blocks;
#ifdef CYGOPT_REDBOOT_FIS
extern void *fis_work_block;
extern void *fis_addr;
extern int fisdir_size;		// Size of FIS directory.
#endif
//extern void _show_invalid_flash_address(CYG_ADDRESS flash_addr, int stat); 
extern void fis_update_directory(void);

//#define TRACE diag_printf("DBG: %s:%d\n", __FUNCTION__, __LINE__)

#define TRACE

/*
 * check if update file is valid
 *  - magic signatures
 *  - crc
 *  - data size (must match partition sizes)
 * setup miniroot environment
 *  - copy libc & fwupdate to /var (new root)
 *  - create /dev/ entries in miniroot
 *  - use pivot_root to switch root, old root will be available at /oldroot
 *  - restart fwupdate from miniroot (with -f switch)
 * update flash memory (check if fwupdate is started from miniroot)
 *  - write images to flash 
 *  - reboot 
 */

/* update file structure:
 *
 * header
 * part1
 * part2
 * signature
 *
 * header:
 * magic1 [4 bytes] GEOS
 * version [128 bytes]
 * reserved [128 bytes]
 * crc [4 bytes] for [0..reserved] range
 * crypto [4 bytes] for [0..crc] range
 *
 * part:
 * magic2 [4 bytes] PART
 * name [32 bytes] "kernel", "cramfs", "etc", "cfg", "RedBoot" (as in /proc/mtd)
 * partition nr [4 bytes] 0, 1, 2... - will be appended to /dev/mtdblock
 * flags1 [4 bytes] 0x0001 - image is compressed, 0x0000 - raw
 * flags2 [4 bytes]
 * length [4 bytes] should be less than partition size
 * part_len [4bytes] partition size
 * data [length bytes]
 * crc [4 bytes] from part start till data field (incl.)
 * crypto [4 bytes] from part start till crc field (incl.)
 *
 * signature:
 * magic3 [4 bytes] END.
 * crc [4 bytes] from the start of file till signature's magic3 field (including)
 * crypto [4 bytes] from the start of file till signature's crc field (including)
 */
/* fwupdate.bin size */
#define MAX_IMAGE_SIZE		0x7E0000	/* 4mb - 64k */

/* max size for single partition - typicaly this is cramfs size */
#define MAX_PART_SIZE		0x800000	/* 3mb - valid only for ar531x */

#define MAGIC_HEADER	"GEOS"
#define MAGIC_PART	"PART"
#define MAGIC_END	"END."

#define MAGIC_LENGTH	4

typedef unsigned int u_int32_t;

typedef struct header {
	char magic[MAGIC_LENGTH];
	char version[256];
	u_int32_t crc;
	u_int32_t pad;
} __attribute__((packed)) header_t;

typedef struct part {
	char magic[MAGIC_LENGTH];
	char name[32];
	u_int32_t part_nr;
	u_int32_t flags1;
	u_int32_t flags2;
	u_int32_t data_size;
	u_int32_t part_size;
} __attribute__((packed)) part_t;

typedef struct part_crc {
	u_int32_t crc;
	u_int32_t pad;
} __attribute__((packed)) part_crc_t;

typedef struct signature {
	char magic[MAGIC_LENGTH];
	u_int32_t crc;
	u_int32_t pad;
} __attribute__((packed)) signature_t;

#define MAX_PARTS 8

typedef struct fw_part {
	part_t *header;
	unsigned char *data;
	u_int32_t data_size;
	part_crc_t *signature;
} fw_part_t;

typedef struct fw {
	u_int32_t size;
	char version[256];
	fw_part_t parts[MAX_PARTS];
	int part_count;
} fw_t;

#define crc32 cyg_ether_crc32_accumulate

extern void fis_init(int argc, char *argv[], int force);

int fw_check_image_wili(unsigned char *addr, unsigned long maxlen, int do_flash)
{
	header_t *header = (header_t *) addr;
	fw_t fw;
	int len = sizeof(header_t) - 2 * sizeof(u_int32_t);
	unsigned int crc = crc32(0L, (unsigned char *)header, len);
	signature_t *sig;
	fw.size = maxlen;
	if (strncmp(header->magic, MAGIC_HEADER, 4)) {
		return -1;
	}
	if (htonl(crc) != header->crc) {
		diag_printf("WILI_FW: header crc failed\n");
		return -1;
	}
	memcpy(fw.version, header->version, sizeof(fw.version));
	if (do_flash)
		diag_printf("WILI_FW: Firmware version: '%s'\n",
			    header->version);
	part_t *p;
	p = (part_t *) (addr + sizeof(header_t));
	int i = 0;
	while (strncmp(p->magic, MAGIC_END, MAGIC_LENGTH) != 0) {
		if (do_flash) {
			diag_printf("Partition: %s\n", p->name);
			diag_printf("Partition size: 0x%X\n",
				    ntohl(p->part_size));
			diag_printf("Data size: %u\n", ntohl(p->data_size));
		}
		if ((strncmp(p->magic, MAGIC_PART, MAGIC_LENGTH) == 0)
		    && (i < MAX_PARTS)) {
			fw_part_t *fwp = &fw.parts[i];

			fwp->header = p;
			fwp->data = (unsigned char *)p + sizeof(part_t);
			fwp->data_size = ntohl(p->data_size);
			fwp->signature =
			    (part_crc_t *) (fwp->data + fwp->data_size);
			crc =
			    htonl(crc32
				  (0L, (unsigned char *)p,
				   fwp->data_size + sizeof(part_t)));
			if (crc != fwp->signature->crc) {
				diag_printf
				    ("WILI_FW: Invalid '%s' CRC (claims: %u, but is %u)\n",
				     fwp->header->name, fwp->signature->crc,
				     crc);
				return -1;
			}
			int index;
			struct fis_image_desc *img =
			    fis_lookup(fwp->header->name, &index);
			if (!img) {
				diag_printf
				    ("WILI_FW: cannot find partition %s, not flashable!\n");
				return -1;
			}
			++i;
		}

		p = (part_t *) ((unsigned char *)p + sizeof(part_t) +
				ntohl(p->data_size) + sizeof(part_crc_t));

		/* check bounds */
		if (((unsigned char *)p - addr) >= maxlen) {
			return -3;
		}
	}

	fw.part_count = i;

	sig = (signature_t *) p;
	if (strncmp(sig->magic, MAGIC_END, MAGIC_LENGTH) != 0) {
		diag_printf("WILI_FW: Bad firmware signature\n");
		return -4;
	}

	crc = htonl(crc32(0L, addr, (unsigned char *)sig - addr));
	if (crc != sig->crc) {
		diag_printf
		    ("WILI_FW: Invalid signature CRC (claims: %u, but is %u)\n",
		     sig->crc, crc);
		return -5;
	}

	if (do_flash) {
		char *arg[] = { "fis", "init" };
		fis_init(2, arg, 1);
		void *err_addr;
		flash_read(fis_addr, fis_work_block, fisdir_size,
			   (void **)&err_addr);
		for (i = 0; i < fw.part_count; ++i) {
			fw_part_t *fwp = &fw.parts[i];
			if (!strncmp(fwp->header->name, "RedBoot", 7)
			    && ntohl(fwp->header->part_size) > 0x10000) {
				diag_printf("ignore %s\n", fwp->header->name);
				continue;
			}
			diag_printf("WILI_FW: Flashing: %s\n",
				    fwp->header->name);
			int stat;
			int index;
			struct fis_image_desc *img =
			    fis_lookup(fwp->header->name, &index);
			if (!img) {
				diag_printf
				    ("WILI_FW: cannot find partition %s, not flashable. break\n");
				return -1;
			}
			if ((stat =
			     flash_erase((void *)img->flash_base,
					 ntohl(fwp->header->part_size),
					 (void **)&err_addr)) != 0) {
				diag_printf
				    ("WILI_FW: Can't erase region at %p: %s\n",
				     err_addr, flash_errmsg(stat));
				return -1;
			}
			if ((stat = flash_program((void *)img->flash_base,
						  (void *)fwp->data,
						  ntohl(fwp->data_size),
						  (void **)&err_addr)) != 0) {
				diag_printf
				    ("WILI_FW: Can't program region at %p: %s\n",
				     err_addr, flash_errmsg(stat));
				return -1;
			}
			img->size = ntohl(fwp->header->part_size);
			img->data_length = ntohl(fwp->header->data_size);

		}
		fis_update_directory();
		diag_printf("WILI_FW: flashing done\n");
	}

	return 0;
}
