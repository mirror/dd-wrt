#ifndef __BACKPORT_TIMB_VIDEO_
#define __BACKPORT_TIMB_VIDEO_
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
#include_next <linux/platform_data/media/timb_video.h>
#else
#include <media/timb_video.h>
#endif /* < 4.5 */

#endif /* __BACKPORT_TIMB_VIDEO_ */
