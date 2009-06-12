#ifndef _ASM_H8300_MODULE_H
#define _ASM_H8300_MODULE_H
/*
 * This file contains the H8/300 architecture specific module code.
 */

#define module_map(x)		vmalloc(x)
#define module_unmap(x)		vfree(x)
#define module_arch_init(x)	(0)
#define arch_init_modules(x)	do { } while (0)

#endif /* _ASM_H8/300_MODULE_H */
