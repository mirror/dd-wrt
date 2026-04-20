/* Compatibility header for <asm/unaligned.h> moved to <linux/unaligned.h> in kernel 6.10+ */
#ifndef _ASM_UNALIGNED_H
#define _ASM_UNALIGNED_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,10,0)
#include <linux/unaligned.h>
#else
#include_next <asm/unaligned.h>
#endif

#endif /* _ASM_UNALIGNED_H */
