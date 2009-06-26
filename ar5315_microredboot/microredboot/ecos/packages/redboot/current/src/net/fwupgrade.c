/*
firmware upgrade code for DD-WRT webflash images
*/

#include <redboot.h>
#include <cyg/io/flash.h>
#include <fis.h>
#include <flash_config.h>
#include "fwupgrade.h"

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

#define TRX_MAGIC	0x30524448	/* "HDR0" */
#define TRX_VERSION	1
#define TRX_MAX_LEN	0x8A0000
#define TRX_NO_HEADER	1	/* Do not write TRX header */

//#if __BYTE_ORDER == __BIG_ENDIAN
unsigned int bswap_32(unsigned int *values)
{
	unsigned char *p = (unsigned char *)values;
	unsigned char a;
	a = p[3];
	p[3] = p[0];
	p[0] = a;
	a = p[2];
	p[2] = p[1];
	p[1] = a;
	return *values;
}

#define STORE32_LE(X)		bswap_32(X)

//#elif __BYTE_ORDER == __LITTLE_ENDIAN
//#define STORE32_LE(X)         (X)
//#else
//#error unkown endianness!
//#endif

struct trx_header {
	unsigned int magic;	/* "HDR0" */
	unsigned int len;	/* Length of file including header */
	unsigned int crc32;	/* 32-bit CRC from flag_version to end of file */
	unsigned int flag_version;	/* 0:15 flags, 16:31 version */
	unsigned int offsets[3];	/* Offsets of partitions from start of header */
};

extern void fis_init(int argc, char *argv[], int force);

void addPartition(char *name, unsigned int flashbase, unsigned int memaddr,
		  unsigned int entryaddr, unsigned int partsize,
		  unsigned int datasize)
{
	int index;
	struct fis_image_desc *img = (struct fis_image_desc *)fis_work_block;
	for (index = 0; index < fisdir_size / sizeof(*img); index++, img++) {
		if (img->name[0] == (unsigned char)0xFF) {
			break;
		}
	}
	strcpy(img->name, name);
	img->flash_base = flashbase;
	img->mem_base = memaddr;
	img->entry_point = entryaddr;	// Hope it's been set
	img->size = partsize;
	img->data_length = datasize;
}

int fw_check_image_ddwrt(unsigned char *addr, unsigned long maxlen,
			 int do_flash)
{
	struct trx_header *base = (struct trx_header *)addr;
	struct trx_header trx;
	memcpy(&trx, base, sizeof(trx));
	trx.magic = STORE32_LE(&trx.magic);
	trx.len = STORE32_LE(&trx.len);
	trx.crc32 = STORE32_LE(&trx.crc32);
	if (trx.magic != TRX_MAGIC || trx.len < sizeof(struct trx_header)) {
		diag_printf("DD-WRT_FW: Bad trx header\n");
		return -1;
	}
//if (STORE32_LE(&trx.flag_version) & TRX_NO_HEADER)
	trx.len -= sizeof(struct trx_header);

	unsigned int crc;
	crc =
	    cyg_crc32_accumulate(0xffffffff, (unsigned char *)&trx.flag_version,
				 sizeof(struct trx_header) - 12);
	crc =
	    cyg_crc32_accumulate(crc, addr + sizeof(struct trx_header),
				 trx.len);
	if (crc == trx.crc32) {
		if (!do_flash)
			diag_printf("DD-WRT_FW: DD-WRT FW CRC Okay\n");
	} else {
		diag_printf
		    ("DD-WRT_FW: DD-WRT FW CRC Failed (Calculated=0x%08X,Real=0x%08X\n",
		     crc, trx.crc32);
		return -1;
	}
	if (do_flash) {
		char *arg[] = { "fis", "init" };
		fis_init(2, arg, 1);
		void *err_addr;
		flash_read(fis_addr, fis_work_block, fisdir_size,
			   (void **)&err_addr);
		struct fis_image_desc *img = NULL;
		int i, stat;
		img = fis_lookup("RedBoot", &i);
		unsigned int flash_addr = img->flash_base + img->size;
		diag_printf("DD-WRT_FW: flash base is 0x%08X\n", flash_addr);
		if ((stat =
		     flash_erase((void *)flash_addr, trx.len,
				 (void **)&err_addr)) != 0) {
			diag_printf("DD-WRT_FW: Can't erase region at %p: %s\n",
				    err_addr, flash_errmsg(stat));
			return -1;
		}
		if ((stat =
		     flash_program((void *)flash_addr,
				   (void *)(addr + sizeof(struct trx_header)),
				   trx.len, (void **)&err_addr)) != 0) {
			diag_printf
			    ("DD-WRT_FW: Can't program region at %p: %s\n",
			     err_addr, flash_errmsg(stat));
			return -1;
		}
		addPartition("linux", flash_addr, 0x80041000, 0x80041000,
			     trx.len, trx.len);
		fis_update_directory();
		diag_printf("DD-WRT_FW: flashing done\n");
	}
	return 0;
}
