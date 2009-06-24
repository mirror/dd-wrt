
#ifndef __ddwrt_fw__
#define __ddwrt_fw__

/* do header, partition and final signature check */
extern int fw_check_image_ddwrt(unsigned char *addr, unsigned long maxlen,
				int do_flash);
extern int fw_check_image_ubnt(unsigned char *addr, unsigned long maxlen,
			       int do_flash);
extern int fw_check_image_wili(unsigned char *addr, unsigned long maxlen,
			       int do_flash);

#define MAX_IMAGE_SIZE		0x7E0000	/* 4mb - 64k */

#define MAX_PART_SIZE		0x800000	/* 3mb - valid only for ar531x */

/* these variables will be initialized in do_tftpd() */
CYG_ADDRWORD BASE_ADDR;
CYG_ADDRWORD FW_TEMP_BASE;
/***********************************************************************************/

#endif				/* __wilibox_fw__ */
