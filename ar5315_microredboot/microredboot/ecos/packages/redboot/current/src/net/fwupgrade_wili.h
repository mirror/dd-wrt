
#ifndef __wili_fw__
#define __wili_fw__

extern int fw_check_image_wili(unsigned char *addr, unsigned long maxlen,
			       int do_flash);

/***********************************************************************************/

/* these variables will be initialized in do_tftpd() */
CYG_ADDRWORD BASE_ADDR;
CYG_ADDRWORD FW_TEMP_BASE;
/***********************************************************************************/

#endif				/* __wilibox_fw__ */
