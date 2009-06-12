/* changes origined from m68k.....   Lineo, Inc		May 2001  */

#ifndef _ASM_FRIO_MODULE_H
#define _ASM_FRIO_MODULE_H
/*
 * This file contains the frio architecture specific module code.
 */

#define module_map(x)		vmalloc(x)
#define module_unmap(x)		vfree(x)
#define module_arch_init(x)	(0)

#endif /* _ASM_FRIO_MODULE_H */
