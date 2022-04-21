#ifndef __BACKPORT_LINUX_MODULEPARAM_H
#define __BACKPORT_LINUX_MODULEPARAM_H
#include_next <linux/moduleparam.h>

#if LINUX_VERSION_IS_LESS(4,2,0)
#define kernel_param_lock LINUX_BACKPORT(kernel_param_lock)
static inline void kernel_param_lock(struct module *mod)
{
	__kernel_param_lock();
}
#define kernel_param_unlock LINUX_BACKPORT(kernel_param_unlock)
static inline void kernel_param_unlock(struct module *mod)
{
	__kernel_param_unlock();
}
#endif

#ifndef module_param_hw_array
#define module_param_hw_array(name, type, hwtype, nump, perm) \
	module_param_array(name, type, nump, perm)
#endif

#endif /* __BACKPORT_LINUX_MODULEPARAM_H */
