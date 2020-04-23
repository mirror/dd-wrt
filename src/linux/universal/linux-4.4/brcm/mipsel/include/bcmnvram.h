/*
 * NVRAM variable manipulation
 *
 * Copyright (C) 2015, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: bcmnvram.h 496107 2014-08-11 09:29:41Z $
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#ifndef _LANGUAGE_ASSEMBLY

#include <typedefs.h>
#include <bcmdefs.h>

struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32 config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32 config_ncdl;	/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};


/*
 * Initialize NVRAM access. May be unnecessary or undefined on certain
 * platforms.
 */
extern int nvram_init(void *sih);
extern int nvram_deinit(void *sih);

/*
 * Append a chunk of nvram variables to the global list
 */
extern int nvram_append(void *si, char *vars, uint varsz);

extern void nvram_get_global_vars(char **varlst, uint *varsz);


/*
 * Check for reset button press for restoring factory defaults.
 */
extern int nvram_reset(void *sih);

/*
 * Disable NVRAM access. May be unnecessary or undefined on certain
 * platforms.
 */
extern void nvram_exit(void *sih);

/*
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
extern char * nvram_get(const char *name);

/*
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @param	bit	bit value to get
 * @return	value of variable or NULL if undefined
 */
extern char * nvram_get_bitflag(const char *name, const int bit);

/*
 * Read the reset GPIO value from the nvram and set the GPIO
 * as input
 */
extern int BCMINITFN(nvram_resetgpio_init)(void *sih);

extern int nvram_match(char *name, char *match);
 
extern int nvram_invmatch(char *name, char *invmatch);

extern void nvram_open(void);

extern void nvram_close(void);

extern int nvram_immed_set(const char *name, const char *value);

extern void nvram_store_collection(char *name,char *buf);

extern char *nvram_get_collection(char *name);

extern char *nvram_safe_get(const char *name);

extern void nvram_safe_unset(const char *name);

extern void nvram_safe_set(const char *name, char *value);

extern char *nvram_prefix_get(const char *name, const char *prefix);

extern int nvram_prefix_match(const char *name, const char *prefix, const char *match);

extern int nvram_default_match(const char *var, const char *match, const char *def);

extern int nvram_default_matchi(const char *var, const int match, const int def);

extern char *nvram_default_get(const char *var, const char *def);

extern int nvram_default_geti(const char *var, const int def);

extern char *nvram_nget(const char *fmt, ...);

extern int nvram_nset(const char *value, const char *fmt, ...);

extern int nvram_nseti(const int value, const char *fmt, ...);

extern int nvram_nmatch(const char *match, const char *fmt, ...);

extern int nvram_nmatchi(const int match, const char *fmt, ...);

extern int nvram_geti(const char *name);

extern void nvram_seti(const char *name, const int value);

extern int nvram_states(char *list);
extern int nvram_state(char *name);
extern int nvram_delstates(char *list);

int nvram_ngeti(const char *fmt, ...);

int nvhas(char *nvname, char *key);

/*
 * Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 */
extern int nvram_set(const char *name, const char *value);

/*
 * Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	name	name of variable to set
 * @param	bit	bit value to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 */
extern int nvram_set_bitflag(const char *name, const int bit, const int value);
/*
 * Unset an NVRAM variable. Pointers to previously set values
 * remain valid until a set.
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int nvram_unset(const char *name);

/*
 * Commit NVRAM variables to permanent storage. All pointers to values
 * may be invalid after a commit.
 * NVRAM values are undefined after a commit.
 * @return	0 on success and errno on failure
 */
extern int nvram_commit(void);

/*
 * Get all NVRAM variables (format name=value\0 ... \0\0).
 * @param	buf	buffer to store variables
 * @param	count	size of buffer in bytes
 * @return	0 on success and errno on failure
 */
extern int nvram_getall(char *nvram_buf, int count);

/*
 * returns the crc value of the nvram
 * @param	nvh	nvram header pointer
 */
uint8 nvram_calc_crc(struct nvram_header * nvh);

#endif /* _LANGUAGE_ASSEMBLY */

/* The NVRAM version number stored as an NVRAM variable */
#define NVRAM_SOFTWARE_VERSION	"1"

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_CLEAR_MAGIC	0x0
#define NVRAM_INVALID_MAGIC	0xFFFFFFFF
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20

/* For CFE builds this gets passed in thru the makefile */
#if defined(CONFIG_ARM)
#define NVSIZE                 0x20000
#define MAX_NVRAM_SPACE		NVSIZE
#define DEF_NVRAM_SPACE		0x10000
#else
#define MAX_NVRAM_SPACE		NVRAM_SPACE
#define DEF_NVRAM_SPACE		0x10000
#endif


#if !defined(CONFIG_BCM80211AC) && !defined(CONFIG_ARM) && !defined(HAVE_NORTHSTAR)
#if defined(CONFIG_NVRAM_60K)
#define NVRAM_SPACE		0xf000
#elif defined(CONFIG_NVRAM_64K)
#define NVRAM_SPACE		0x10000
#else
#define NVRAM_SPACE		0x8000
#endif
#else
#define NVRAM_SPACE		0x10000
#if !defined(CONFIG_ARM) && !defined(HAVE_NORTHSTAR)
#define NVRAM_SPACE_256		0x40000
#endif
#endif

#if defined(HAVE_80211AC) || defined(HAVE_NVRAM_64K)
#undef NVRAM_SPACE
#define NVRAM_SPACE		0x10000

#if !defined(CONFIG_ARM) && !defined(HAVE_NORTHSTAR)
#undef NVRAM_SPACE_256
#define NVRAM_SPACE_256		0x40000
#endif

#endif

/* debug output for NVRAM_SPACE*/
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
//#pragma message (VAR_NAME_VALUE(NVRAM_SPACE))

#define ROM_ENVRAM_SPACE	0x1000
#define NVRAM_LZMA_MAGIC	0x4c5a4d41	/* 'LZMA' */

#define NVRAM_MAX_VALUE_LEN 255
#define NVRAM_MAX_PARAM_LEN 64

#define NVRAM_CRC_START_POSITION	9 /* magic, len, crc8 to be skipped */
#define NVRAM_CRC_VER_MASK	0xffffff00 /* for crc_ver_init */

/* Offsets to embedded nvram area */
#define NVRAM_START_COMPRESSED	0x400
#define NVRAM_START		0x1000

#define BCM_JUMBO_NVRAM_DELIMIT '\n'
#define BCM_JUMBO_START "Broadcom Jumbo Nvram file"

#if !defined(BCMHIGHSDIO) && defined(BCMTRXV2)
extern char *_vars;
extern uint _varsz;
#endif  

#if (defined(FAILSAFE_UPGRADE) || defined(CONFIG_FAILSAFE_UPGRADE) || \
	defined(__CONFIG_FAILSAFE_UPGRADE_SUPPORT__))
#define IMAGE_SIZE "image_size"
#define BOOTPARTITION "bootpartition"
#define IMAGE_BOOT BOOTPARTITION
#define PARTIALBOOTS "partialboots"
#define MAXPARTIALBOOTS "maxpartialboots"
#define IMAGE_1ST_FLASH_TRX "flash0.trx"
#define IMAGE_1ST_FLASH_OS "flash0.os"
#define IMAGE_2ND_FLASH_TRX "flash0.trx2"
#define IMAGE_2ND_FLASH_OS "flash0.os2"
#define IMAGE_FIRST_OFFSET "image_first_offset"
#define IMAGE_SECOND_OFFSET "image_second_offset"
#define LINUX_FIRST "linux"
#define LINUX_SECOND "linux2"
#endif

#if (defined(DUAL_IMAGE) || defined(CONFIG_DUAL_IMAGE) || \
	defined(__CONFIG_DUAL_IMAGE_FLASH_SUPPORT__))
/* Shared by all: CFE, Linux Kernel, and Ap */
#define IMAGE_BOOT "image_boot"
#define BOOTPARTITION IMAGE_BOOT
/* CFE variables */
#define IMAGE_1ST_FLASH_TRX "flash0.trx"
#define IMAGE_1ST_FLASH_OS "flash0.os"
#define IMAGE_2ND_FLASH_TRX "flash0.trx2"
#define IMAGE_2ND_FLASH_OS "flash0.os2"
#define IMAGE_SIZE "image_size"

/* CFE and Linux Kernel shared variables */
#define IMAGE_FIRST_OFFSET "image_first_offset"
#define IMAGE_SECOND_OFFSET "image_second_offset"

/* Linux application variables */
#define LINUX_FIRST "linux"
#define LINUX_SECOND "linux2"
#define POLICY_TOGGLE "toggle"
#define LINUX_PART_TO_FLASH "linux_to_flash"
#define LINUX_FLASH_POLICY "linux_flash_policy"

#endif /* defined(DUAL_IMAGE||CONFIG_DUAL_IMAGE)||__CONFIG_DUAL_IMAGE_FLASH_SUPPORT__ */

#endif /* _bcmnvram_h_ */
