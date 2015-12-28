/* vi: set sw=4 ts=4: */

#ifndef __RT3052_ACCESS_HEADER__
#define __RT3052_ACCESS_HEADER__

#include <asm/rt2880/rt_mmap.h>

#define BIT(x)	(1 << x)

typedef unsigned int RTREG;

#define PHYS_TO_K1(physaddr)	KSEG1ADDR(physaddr)
#define sysRegRead(phys)		(*(volatile RTREG *)PHYS_TO_K1(phys))
#define sysRegWrite(phys, val)	((*(volatile RTREG *)PHYS_TO_K1(phys)) = (val))

/* Reset Control */
#define RT_RSTCTRL		(RALINK_SYSCTL_BASE+0x34)

/* Ethernet Switch */
#define RTES_PFC0		(RALINK_ETH_SW_BASE+0x10)
#define RTES_PFC1		(RALINK_ETH_SW_BASE+0x14)
#define RTES_PFC2		(RALINK_ETH_SW_BASE+0x18)
#define RTES_PVIDC0		(RALINK_ETH_SW_BASE+0x40)
#define RTES_PVIDC1		(RALINK_ETH_SW_BASE+0X44)
#define RTES_PVIDC2		(RALINK_ETH_SW_BASE+0x48)
#define RTES_PVIDC3		(RALINK_ETH_SW_BASE+0X4c)
#define RTES_VLANI0		(RALINK_ETH_SW_BASE+0x50)
#define RTES_VLANI1		(RALINK_ETH_SW_BASE+0x54)
#define RTES_VLANI2		(RALINK_ETH_SW_BASE+0x58)
#define RTES_VLANI3		(RALINK_ETH_SW_BASE+0x5c)
#define RTES_VLANI4		(RALINK_ETH_SW_BASE+0x60)
#define RTES_VLANI5		(RALINK_ETH_SW_BASE+0x64)
#define RTES_VLANI6		(RALINK_ETH_SW_BASE+0x68)
#define RTES_VLANI7		(RALINK_ETH_SW_BASE+0x6c)
#define RTES_VMSC0		(RALINK_ETH_SW_BASE+0x70)
#define RTES_VMSC1		(RALINK_ETH_SW_BASE+0x74)
#define RTES_VMSC2		(RALINK_ETH_SW_BASE+0x78)
#define RTES_VMSC3		(RALINK_ETH_SW_BASE+0x7c)

#define RTES_POC2		(RALINK_ETH_SW_BASE+0x98)

#define RTES_PVIDC(x)	(RTES_PVIDC0 + ((x)/2)*4)
#define RTES_VLANI(x)	(RTES_VLANI0 + ((x)/2)*4)
#define RTES_VMSC(x)	(RTES_VMSC0 + ((x)/4)*4)

#endif
