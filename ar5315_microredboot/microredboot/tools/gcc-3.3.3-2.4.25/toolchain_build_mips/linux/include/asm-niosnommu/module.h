#ifndef _NIOS_MODULE_H
#define _NIOS_MODULE_H
/*
 * This file contains the nios architecture specific module code.
 */
unsigned long arch_init_modules(struct module *mod);

#define module_map(x)		vmalloc(x)
#define module_unmap(x)		vfree(x)
#define module_arch_init(x)	arch_init_modules((x))

#endif /* _NIOS_MODULE_H */
