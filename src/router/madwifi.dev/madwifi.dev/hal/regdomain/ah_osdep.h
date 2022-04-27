/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/regdomain/ah_osdep.h#2 $
 */
#ifndef _ATH_AH_OSDEP_H_
#define _ATH_AH_OSDEP_H_
/*
 * Atheros Hardware Access Layer (HAL) OS Dependent Definitions.
 */
#include <sys/param.h>

#define	OS_DELAY(_n)	0
#define	OS_INLINE	__inline
#define	OS_MEMZERO(_a, _size)		bzero((_a), (_size))
#define	OS_MEMCPY(_dst, _src, _size)	bcopy((_src), (_dst), (_size))
#define	OS_MACEQU(_a, _b) \
	(bcmp((_a), (_b), IEEE80211_ADDR_LEN) == 0)

struct ath_hal;
extern 	u_int32_t OS_GETUPTIME(struct ath_hal *);
extern	void OS_REG_WRITE(struct ath_hal *, u_int32_t, u_int32_t);
extern	u_int32_t OS_REG_READ(struct ath_hal *, u_int32_t);
extern	void OS_MARK(struct ath_hal *, u_int id, u_int32_t value);
#define	OS_GETUPTIME(_ah)	0
#define	OS_REG_WRITE(_ah, _reg, _val)
#define	OS_REG_READ(_ah, _reg)	0
#define	OS_MARK(_ah, _id, _v)

/*
 * Linux/BSD gcc compatibility shims.
 */
#ifndef __printflike
#define	__printflike(_a,_b) \
	__attribute__ ((__format__ (__printf__, _a, _b)))
#endif
#include <stdarg.h>
#ifndef __va_list
#define	__va_list	va_list
#endif
#define	OS_INLINE	__inline
#ifndef __packed
#define	__packed	__attribute__((__packed__))
#endif
#endif /* _ATH_AH_OSDEP_H_ */
