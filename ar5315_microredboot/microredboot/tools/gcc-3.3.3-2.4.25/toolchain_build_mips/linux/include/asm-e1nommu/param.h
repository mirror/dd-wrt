#ifndef _HYPERSTONE_PARAM_H
#define _HYPERSTONE_PARAM_H

#include <asm/user.h>

#ifndef HZ
#define HZ 100
#endif

#if defined(__KERNEL__) && (HZ == 100)
#define hz_to_std(a) (a)
#endif

#ifndef NGROUPS
#define NGROUPS		32 
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif
