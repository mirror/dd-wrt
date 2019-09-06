#ifndef __BACKPORT_PM_RUNTIME_H
#define __BACKPORT_PM_RUNTIME_H
#include_next <linux/pm_runtime.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
#define pm_runtime_active LINUX_BACKPORT(pm_runtime_active)
#ifdef CONFIG_PM
static inline bool pm_runtime_active(struct device *dev)
{
	return dev->power.runtime_status == RPM_ACTIVE
		|| dev->power.disable_depth;
}
#else
static inline bool pm_runtime_active(struct device *dev) { return true; }
#endif /* CONFIG_PM */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0) */

#endif /* __BACKPORT_PM_RUNTIME_H */
