#ifndef __BACKPORT_UAPI__RFKILL_H
#define __BACKPORT_UAPI__RFKILL_H
#include_next <linux/rfkill.h>


#if LINUX_VERSION_IS_LESS(5,11,0)

/* This should come from uapi/linux/rfkill.h, but it was much easier
 * to do it this way.
 */
enum rfkill_hard_block_reasons {
	RFKILL_HARD_BLOCK_SIGNAL        = 1 << 0,
	RFKILL_HARD_BLOCK_NOT_OWNER     = 1 << 1,
};

static inline bool rfkill_set_hw_state_reason(struct rfkill *rfkill,
					      bool blocked, unsigned long reason)
{
	return rfkill_set_hw_state(rfkill, blocked);
}

#endif /* 5.11 */

#endif /* __BACKPORT_UAPI__RFKILL_H */
