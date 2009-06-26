/*
firmware upgrade code for senao webflash images
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

extern void addPartition(char *name, unsigned int flashbase,
			 unsigned int memaddr, unsigned int entryaddr,
			 unsigned int partsize, unsigned int datasize);

extern void fis_init(int argc, char *argv[], int force);

int fw_check_image_senao(unsigned char *addr, unsigned long maxlen,
			 int do_flash)
{
	unsigned char *base = (unsigned char *)addr + 10;
	if (strncmp(addr, "AP51-3660", 9)) {
		diag_printf("SENAO_FW: bad header\n");
		return -1;
	}

	if (do_flash) {
		maxlen -= 10;
		char *arg[] = { "fis", "init" };
		fis_init(2, arg, 1);
		void *err_addr;
		flash_read(fis_addr, fis_work_block, fisdir_size,
			   (void **)&err_addr);
		struct fis_image_desc *img = NULL;
		int i, stat;
		img = fis_lookup("RedBoot", &i);
		unsigned int flash_addr = img->flash_base + img->size;
		if ((stat =
		     flash_erase((void *)flash_addr, maxlen,
				 (void **)&err_addr)) != 0) {
			diag_printf("SENAO_FW: Can't erase region at %p: %s\n",
				    err_addr, flash_errmsg(stat));
			return -1;
		}
		if ((stat =
		     flash_program((void *)flash_addr,
				   (void *)(base),
				   maxlen, (void **)&err_addr)) != 0) {
			diag_printf
			    ("SENAO_FW: Can't program region at %p: %s\n",
			     err_addr, flash_errmsg(stat));
			return -1;
		}
		img = (struct fis_image_desc *)fis_work_block;
		for (i = 0; i < fisdir_size / sizeof(*img); i++, img++) {
			if (img->name[0] == (unsigned char)0xFF) {
				break;
			}
		}

		unsigned int filesyssize = 0x3f0000;
		unsigned int linuxsize = 0xa0000;
		unsigned int cfgsize = 0x20000;
		unsigned int exec = 0x80041798;
		if (maxlen == (3670026 - 10))	// detect 4M images (EAP3660 etc.)
		{
			filesyssize = 0x2f0000;
			exec = 0x80170040;	//weired entrypoint
		}
		addPartition("rootfs", flash_addr, 0x80041000, 0, filesyssize,
			     filesyssize);
		addPartition("vmlinux.bin.l7", flash_addr + filesyssize,
			     0x80041000, exec, linuxsize, linuxsize);
		addPartition("cfg", flash_addr + filesyssize + linuxsize,
			     0x80041000, 0, cfgsize, cfgsize);
		fis_update_directory();
		diag_printf("SENAO_FW: flashing done\n");
	}
	return 0;
}
