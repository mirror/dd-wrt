#ifndef _M68KNOMMU_PARAM_H
#define _M68KNOMMU_PARAM_H

#include <linux/config.h>


#ifndef HZ
#ifdef CONFIG_COLDFIRE
#if defined(CONFIG_HW_FEITH)
#define HZ 1000
#else
#define HZ 100
#endif
#endif
#ifdef CONFIG_M68EN302
#define HZ 100
#endif
#ifdef CONFIG_M68328
#define HZ 100
#endif
#ifdef CONFIG_M68EZ328
#ifdef CONFIG_M68EZ328_USE_RTC
#define HZ 128
#else
#define HZ 100
#endif
#endif

#ifdef CONFIG_M68VZ328
#ifdef CONFIG_DRAGONBALL_USE_RTC
#define HZ 128
#else
#define HZ 100
#endif
#endif

#ifdef CONFIG_SHGLCORE
#define HZ 50
#endif
#ifdef CONFIG_M68360
#define HZ 100
#endif

#endif

#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#ifdef __KERNEL__
#define CLOCKS_PER_SEC HZ
#endif /* __KERNEL__ */

#endif /* _M68KNOMMU_PARAM_H */
