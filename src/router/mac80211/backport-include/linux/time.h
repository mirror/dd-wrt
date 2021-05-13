#ifndef __BACKPORT_LINUX_TIME_H
#define __BACKPORT_LINUX_TIME_H
#include_next <linux/time.h>

#include <linux/time64.h>

#ifndef time_after32
#define time_after32(a, b)	((s32)((u32)(b) - (u32)(a)) < 0)
#endif
#ifndef time_before32
#define time_before32(b, a)	time_after32(a, b)
#endif
#ifndef time_between32
#define time_between32(t, l, h) ((u32)(h) - (u32)(l) >= (u32)(t) - (u32)(l))
#endif
#endif /* __BACKPORT_LINUX_TIME_H */
