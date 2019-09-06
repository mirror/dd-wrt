#ifndef __BACKPORT_TIMB_RADIO_
#define __BACKPORT_TIMB_RADIO_
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
#include_next <linux/platform_data/media/timb_radio.h>
#else
#include <media/timb_radio.h>
#endif /* < 4.5 */

#endif /* __BACKPORT_TIMB_RADIO_ */
