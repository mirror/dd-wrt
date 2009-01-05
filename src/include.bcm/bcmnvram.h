/*
 * NVRAM variable manipulation
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmnvram.h,v 1.12 2004/08/29 16:30:18 honor Exp $
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#ifndef _LANGUAGE_ASSEMBLY

#include <typedefs.h>
#include <stdio.h>

struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	uint32 config_refresh;	/* 0:15 config, 16:31 refresh */
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
extern int nvram_init(void *sbh);

/*
 * Disable NVRAM access. May be unnecessary or undefined on certain
 * platforms.
 */
extern void nvram_exit(void);

/*
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
extern char * nvram_get(const char *name);

/* 
 * Get the value of an NVRAM variable.
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
extern char *nvram_safe_get(const char *name);

extern void nvram_safe_unset(const char *name);

extern void nvram_safe_set(const char *name, char *value);


/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */

/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
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

extern int nvram_prefix_match(const char *name, const char *prefix,char *match);

extern int nvram_default_match (char *var, char *match, char *def);

extern char *nvram_default_get (char *var, char *def);

extern char *nvram_nget(const char *fmt,...);

extern char *nvram_nset(char *value,const char *fmt,...);

extern int nvram_nmatch(char *match,const char *fmt,...);

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
extern int nvram_getall(char *buf, int count);

extern int file2nvram(char *filename, char *varname);
extern int nvram2file(char *varname, char *filename);

extern void fwritenvram(char *var,FILE *fp);
extern void writenvram(char *var,char *file);

#endif /* _LANGUAGE_ASSEMBLY */

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20
#if defined(HAVE_X86) || defined(HAVE_WHRAG108) || defined(HAVE_FONERA) || defined(HAVE_RT2880) || defined(HAVE_RT3052) || defined(HAVE_XSCALE) || defined(HAVE_STORM)
#define NVRAM_SPACE		0x10000
#else
#define NVRAM_SPACE		0x8000
#endif

#define NVRAM_MAX_VALUE_LEN 255
#define NVRAM_MAX_PARAM_LEN 64

#endif /* _bcmnvram_h_ */
