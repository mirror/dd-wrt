/*
 * fis.c - simple redboot partition parser
 *
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 */

struct fis_image_desc {
	unsigned char name[16];	// Null terminated name
	unsigned long flash_base;	// Address within FLASH of image
	unsigned long mem_base;	// Address in memory where it executes
	unsigned long size;	// Length of image
	unsigned long entry_point;	// Execution entry point
	unsigned long data_length;	// Length of actual data
	unsigned char _pad[256 - (16 + 7 * sizeof(unsigned long))];
	unsigned long desc_cksum;	// Checksum over image descriptor
	unsigned long file_cksum;	// Checksum over image data
};

#define SCANCOUNT 5
static unsigned int redboot_offset(unsigned int highoffset,
				   unsigned int erasesize)
{
	unsigned int copy = highoffset;
	int c = SCANCOUNT;
	while ((c--) > 0) {
		highoffset -= erasesize;
		unsigned char *p = (unsigned char *)highoffset;
		if (!strncmp(p, "RedBoot", 7))
			return highoffset;
	}
	return copy - erasesize;	// default offset, if not found
}

static unsigned int getPartition(char *name)
{
	int count = 0;
	unsigned char *p =
	    (unsigned char *)(flashbase + flashsize - (sectorsize * 2));
	struct fis_image_desc *fis = (struct fis_image_desc *)p;
	while (fis->name[0] != 0xff && count < 10) {
		if (!strncmp(fis->name, name, strlen(name)))
			return fis->flash_base;
	}
	return 0;
}

/*
 * searches for a directory entry named linux* vmlinux* or kernel and returns its flash address (it also initializes entrypoint and load address)
 */
static unsigned int getLinux(void)
{
	int count = 0;
	unsigned int redboot_fis =
	    redboot_offset(flashbase + flashsize, sectorsize);
	printf("Found FIS Directory on [0x%08X]\n",redboot_fis);
	unsigned char *p = (unsigned char *)redboot_fis;
	struct fis_image_desc *fis = (struct fis_image_desc *)p;
	/* search for fis partiton linux*,vmlinux* or kernel */
	while (fis->name[0] != 0xff && count < 10) {
		if (!strncmp(fis->name, "linux", 5)
		    || !strncmp(fis->name, "vmlinux", 7)
		    || !strcmp(fis->name, "kernel")) {
			printf
			    ("found bootable image: [%s] at [0x%08X] EP [0x%08X]\n",
			     fis->name, fis->flash_base, fis->entry_point);
			bootoffset = fis->entry_point;
			output_data = (uch *) fis->mem_base;
			return fis->flash_base;
		}
		p += 256;
		fis = (struct fis_image_desc *)p;
		count++;
	}
	printf("no bootable image found, try default location 0x%08X\n",
	       flashbase + 0x10000);
	bootoffset = 0x80041000;
	output_data = (uch *) 0x80041000;
	return flashbase + 0x10000;	//first available address after bootloader
}
