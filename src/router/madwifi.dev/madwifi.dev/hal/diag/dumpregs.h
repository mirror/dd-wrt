#ifndef _DUMPREGS_
#define	_DUMPREGS_
/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/dumpregs.h#1 $
 */

extern	u_int32_t regdata[0xffff / sizeof(u_int32_t)];
#undef OS_REG_READ
#define	OS_REG_READ(ah, off)	regdata[(off) >> 2]

typedef struct {
	const char*	label;
	u_int		reg;
} HAL_REG;

enum {
	DUMP_BASIC	= 0x0001,	/* basic/default registers */
	DUMP_KEYCACHE	= 0x0002,	/* key cache */
	DUMP_BASEBAND	= 0x0004,	/* baseband */
	DUMP_INTERRUPT	= 0x0008,	/* interrupt state */
	DUMP_XR		= 0x0010,	/* XR state */
	DUMP_QCU	= 0x0020,	/* QCU state */
	DUMP_DCU	= 0x0040,	/* DCU state */

	DUMP_PUBLIC	= 0x0061,	/* public = BASIC+QCU+DCU */
	DUMP_ALL	= 0xffff
};

extern	u_int ath_hal_setupdiagregs(const HAL_REGRANGE regs[], u_int nr);
extern	void ath_hal_dumprange(FILE *fd, u_int a, u_int b);
extern	void ath_hal_dumpregs(FILE *fd, const HAL_REG regs[], u_int nregs);
extern	void ath_hal_dumpkeycache(FILE *fd, int nkeys, int micEnabled);
#endif /* _DUMPREGS_ */
