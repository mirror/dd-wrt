/*
 * Please note that the name are used in mkflash script. Therefore
 * don't change them.  If you want to add different partitions, you
 * will need to modify mkflash script as well so that the end image
 * is what you include here!
 *
 * Also, the 7th item is always the size, so please don't add extra
 * spaces in the name or other items.
 *
 *  - Alan
 */

static struct mtd_partition sl2312_partitions[] = {
	{ name: "RedBoot", 	 offset: 0x00000000, size: 0x00020000, },
	{ name: "linux", 	 offset: 0x00020000, size: 0x007A0000, },
	{ name: "rootfs", 	 offset: 0x00000000, size: 0x00000000, }, // variable size
	{ name: "ddwrt", 	 offset: 0x00000000, size: 0x00000000, }, // variable size
	{ name: "VCTL", 	 offset: 0x007C0000, size: 0x00010000, },
	{ name: "nvram", 	 offset: 0x007D0000, size: 0x00020000, },
	{ name: "FIS directory", offset: 0x007F0000, size: 0x00010000, }
};
