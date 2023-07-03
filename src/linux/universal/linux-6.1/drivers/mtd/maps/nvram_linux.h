#ifndef __NVRAM_LINUX__
#define	__NVRAM_LINUX__

//typedef unsigned int  uint;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#define	u32		uint32
#define	u16		uint16
#define	u8		uint8
#define	ulong	uint32
#define	ushort	uint16
#define	uchar	uint8

#define	INLINE	__inline__
#define	ltoh32
#define	htol32
#define	bzero(b, len)		memset((b), '\0', (len))
extern void *MALLOC(size_t size);
extern void MFREE(void *addr);

extern uint8 nvram_crc8(uint8 * pdata, uint nbytes, uint8 crc);
#define CRC8_INIT_VALUE  0xff	/* Initial CRC8 checksum value */
#define CRC8_GOOD_VALUE  0x9f	/* Good final CRC8 checksum value */

#ifndef MIN
#define	MIN(a, b)		(((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a, b)		(((a)>(b))?(a):(b))
#endif

#define CEIL(x, y)		(((x) + ((y)-1)) / (y))
#define	ROUNDUP(x, y)		((((ulong)(x)+((y)-1))/(y))*(y))
#define	ISALIGNED(a, x)		(((uint)(a) & ((x)-1)) == 0)
#define	ISPOWEROF2(x)		((((x)-1)&(x))==0)
#define	OFFSETOF(type, member)	((uint) &((type *)0)->member)
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

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
extern char *nvram_get(const char *name);

/* 
 * Get the value of an NVRAM variable.
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
#define nvram_safe_get(name) (nvram_get(name) ? : "")

/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
static INLINE int nvram_match(char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
static INLINE int nvram_invmatch(char *name, char *invmatch)
{
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

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

#define NVRAM_MAGIC			0x48534C46	/* 'NVFL' */
#define NVRAM_SPACE_MAGIC			0x50534341	/* 'SPAC' */
#define NVRAM_ASYNC_MAGIC			0x4153594E	/* 'ASYN' */
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	sizeof(struct nvram_header)
#if defined(CONFIG_WZR_HP_G300NH) || defined(CONFIG_ARCH_ALPINE) || defined(CONFIG_X86) || defined(CONFIG_MACH_CAMBRIA)  || defined(CONFIG_SOC_IMX6) || defined(CONFIG_ARCH_QCOM) || defined(CONFIG_TL_WDR4900_V1) || defined(CONFIG_ARCH_THUNDER) || defined(CONFIG_ARCH_MVEBU)
#define NVRAM_SPACE			0x20000
#else
#define NVRAM_SPACE			0x10000
#endif

#define	NVRAM_CRC_MASK		0x000000ff
#define	NVRAM_CRC_SHIFT		0
#define	NVRAM_VER_MASK		0x0000ff00
#define	NVRAM_VER_SHIFT		8
#define	NVRAM_OPT_MASK		0xffff0000
#define	NVRAM_OPT_SHIFT		16

#define	NVRAM_RUNTIME		1
#define	NVRAM_DEFAULT		2
#define	NVRAM_RT_SPACE		NVRAM_SPACE
#define	NVRAM_DF_SPACE		0x1000
#define	NVRAM_DF_OFFS		(0x200 + 0x400)

#endif	/*__NVRAM_LINUX__*/
