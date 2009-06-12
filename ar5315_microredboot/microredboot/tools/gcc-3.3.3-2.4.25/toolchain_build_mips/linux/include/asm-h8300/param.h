#ifndef _H8300_PARAM_H
#define _H8300_PARAM_H

#include <linux/config.h>

#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */
#define CLOCKS_PER_SEC HZ

#endif /* _H8300_PARAM_H */
