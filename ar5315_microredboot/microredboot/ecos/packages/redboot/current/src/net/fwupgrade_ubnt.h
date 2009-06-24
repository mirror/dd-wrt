
#ifndef __ubnt_fw__
#define __ubnt_fw__

#define MAGIC_HEADER	"OPEN"
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
	char name[16];
	char pad[12];
	u_int32_t memaddr;
	u_int32_t index;
	u_int32_t baseaddr;
	u_int32_t entryaddr;
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

extern int fw_check_image_ubnt(unsigned char *addr, unsigned long maxlen,
			       int do_flash);

/***********************************************************************************/

/* fwupdate.bin size */
#define MAX_IMAGE_SIZE		0x7E0000	/* 4mb - 64k */

/* max size for single partition - typicaly this is cramfs size */
#define MAX_PART_SIZE		0x800000	/* 3mb - valid only for ar531x */

/* these variables will be initialized in do_tftpd() */
CYG_ADDRWORD BASE_ADDR;
CYG_ADDRWORD FW_TEMP_BASE;
/***********************************************************************************/

#endif				/* __wilibox_fw__ */
