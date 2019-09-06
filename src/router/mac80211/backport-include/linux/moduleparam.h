#ifndef __BACKPORT_LINUX_MODULEPARAM_H
#define __BACKPORT_LINUX_MODULEPARAM_H
#include_next <linux/moduleparam.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
#undef __MODULE_INFO
#ifdef MODULE
#define __MODULE_INFO(tag, name, info)					  \
static const char __UNIQUE_ID(name)[]					  \
  __used __attribute__((section(".modinfo"), unused, aligned(1)))	  \
  = __stringify(tag) "=" info
#else  /* !MODULE */
/* This struct is here for syntactic coherency, it is not used */
#define __MODULE_INFO(tag, name, info)					  \
  struct __UNIQUE_ID(name) {}
#endif
#endif /* < 3.8 */

#endif /* __BACKPORT_LINUX_MODULEPARAM_H */
