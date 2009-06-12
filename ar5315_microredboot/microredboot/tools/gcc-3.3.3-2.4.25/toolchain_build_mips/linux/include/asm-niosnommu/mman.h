#ifndef __NIOS_MMAN_H__
#define __NIOS_MMAN_H__

#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */
#define PROT_EXEC	0x4		/* page can be executed */
#define PROT_NONE	0x0		/* page can not be accessed */

#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */
#define MAP_TYPE	0x0f		/* Mask for type of mapping */
#define MAP_FIXED	0x10		/* Interpret addr exactly */
#define MAP_ANONYMOUS	0x20		/* don't use a file */

#define MAP_GROWSDOWN	0x0100		/* stack-like segment */
#define MAP_DENYWRITE	0x0800		/* ETXTBSY */
#define MAP_EXECUTABLE	0x1000		/* mark it as a executable */
#define MAP_LOCKED	0x2000		/* pages are locked */

#define MS_ASYNC	1		/* sync memory asynchronously */
#define MS_INVALIDATE	2		/* invalidate the caches */
#define MS_SYNC		4		/* synchronous memory sync */

#define MCL_CURRENT	1		/* lock all current mappings */
#define MCL_FUTURE	2		/* lock all future mappings */

/* compatibility flags */
#define MAP_ANON	MAP_ANONYMOUS
#define MAP_FILE	0

#endif /* __NIOS_MMAN_H__ */
