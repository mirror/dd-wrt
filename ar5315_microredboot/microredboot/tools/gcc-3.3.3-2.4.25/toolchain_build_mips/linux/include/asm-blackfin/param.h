#ifndef _FRIONOMMU_PARAM_H
#define _FRIONOMMU_PARAM_H

#include <linux/config.h>

#ifndef HZ
#ifdef CONFIG_BLACKFIN
#define HZ 100		/* need changes accordingly	*/
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

#endif /* _FRIONOMMU_PARAM_H */
