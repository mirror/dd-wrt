/*
 * include/asm-v850/blkmem.h -- `blkmem' device configuration
 *
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_BLKMEM_H__
#define __V850_BLKMEM_H__

/* Look for a device specific memory array location.  */
#include <asm/machdep.h>


#ifndef FIXUP_ARENAS  /* provide a default */

#ifndef ROOT_FS_IMAGE_RW
#define ROOT_FS_IMAGE_RW	0 /* assume read-only */
#endif

/* For <asm/blkmem.h> */
/* This gives us a single arena.  */
#define CAT_ROMARRAY
/* These should be defined by the linker map.  */
extern char _root_fs_image_start, _root_fs_image_end;
/* Make the single arena point use them.  */
#define FIXUP_ARENAS							      \
        arena[0].rw = ROOT_FS_IMAGE_RW;					      \
	arena[0].address = (unsigned long)&_root_fs_image_start;	      \
	arena[0].length = &_root_fs_image_end - &_root_fs_image_start;
#endif /* FIXUP_ARENAS */


/* Used by the blkmem flash programming code.  */
#define HARD_RESET_NOW() __asm__ __volatile__ ("jmp r0")


#endif /* __V850_BLKMEM_H__ */
