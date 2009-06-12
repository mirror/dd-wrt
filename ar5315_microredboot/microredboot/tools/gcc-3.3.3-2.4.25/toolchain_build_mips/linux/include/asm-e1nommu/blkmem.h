/*
 * include/asm-hyperstone-nommu/blkmem.h -- `blkmem' device configuration
 *
 *  Copyright (c) 2003, Yannis Mitsos, George Thanos 
 *  					yannis.mitsos@gdt.gr george.thanos@gdt.gr	
 *  Original copyright  belongs to (C) 2001,2002  NEC Corporation and
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 */

#ifndef __ASM_HYPERSTONE_NOMMU_BLKMEM_H__
#define __ASM_HYPERSTONE_NOMMU_BLKMEM_H__

#ifndef FIXUP_ARENAS  /* provide a default */

#ifndef ROOT_FS_IMAGE_RW
#define ROOT_FS_IMAGE_RW	0 /* assume read-only */
#endif

#define CAT_ROMARRAY
/* These should be defined by the linker map.  */
extern unsigned long _ebss;
extern unsigned long _romfs_size;

/* Make the single arena point use them.  */
#define FIXUP_ARENAS							      \
        arena[0].rw = ROOT_FS_IMAGE_RW;					      \
	arena[0].address = (unsigned long)&_ebss;	      \
	arena[0].length = _romfs_size ; /* Let the others find the length */
#endif /* FIXUP_ARENAS */

#endif /* __ASM_HYPERSTONE_NOMMU_BLKMEM_H__ */
