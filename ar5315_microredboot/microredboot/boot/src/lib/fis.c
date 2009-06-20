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

/*
 * searches for a directory entry named linux* vmlinux* or kernel and returns its flash address (it also initializes entrypoint and load address)
 */
static unsigned int getLinux(void)
{
	int count = 0;
	unsigned char *p =
	    (unsigned char *)(flashbase + flashsize - (sectorsize * 2));
	struct fis_image_desc *fis = (struct fis_image_desc *)p;
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
	puts("no bootable image found, try default location 0xbfc10000\n");
	bootoffset = 0x80041000;
	output_data = (uch *) 0x80041000;
	return 0xbfc10000;
}
