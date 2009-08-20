/*
 * HND Run Time Environment for standalone MIPS programs.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte.h,v 13.101.2.2 2008/06/19 23:49:45 Exp $
 */

#ifndef	_HNDRTE_H
#define	_HNDRTE_H

#include <typedefs.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>

#if defined(_HNDRTE_SIM_)
#include <hndrte_sim.h>
#elif defined(mips)
#include <hndrte_mips.h>
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#include <hndrte_arm.h>
#endif

#include <bcmstdlib.h>
#include <hndrte_trap.h>

#define MAX_NETDEV_NAME		32

/* RTE IOCTL definitions for generic ether devices */
#define RTEGHWADDR		0x8901
#define RTESHWADDR		0x8902
#define RTEGMTU			0x8903
#define RTEGSTATS		0x8904
#define RTEGALLMULTI		0x8905
#define RTESALLMULTI		0x8906
#define RTEGPROMISC		0x8907
#define RTESPROMISC		0x8908
#define RTESMULTILIST  		0x8909
#define RTEGUP			0x890A
#define RTEGPERMADDR		0x890B

/* Forward declaration */
struct lbuf;

extern si_t *hndrte_sih;		/* Chip backplane handle */
extern osl_t *hndrte_osh;		/* Chip backplane osl */
extern chipcregs_t *hndrte_ccr;		/* Chipcommon core regs */
extern sbconfig_t *hndrte_ccsbr;	/* Chipcommon core SB config regs */

typedef struct hndrte_devfuncs hndrte_devfuncs_t;

/* happens to mirror a section of linux's net_device_stats struct */
typedef struct {
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
} hndrte_stats_t;

/* Device instance */
typedef struct hndrte_dev {
	char		dev_fullname[MAX_NETDEV_NAME];
	hndrte_devfuncs_t *dev_funcs;
	uint32		devid;
	void		*dev_softc;
	uint32		flags;		/* see RTEDEVFLAG_XXXX */
	struct hndrte_dev *dev_next;
	struct hndrte_dev *dev_chained;
	void		*pdev;
} hndrte_dev_t;

#define RTEDEVFLAG_HOSTASLEEP	0x000000001	/* host is asleep */

#ifdef SBPCI
typedef struct _pdev {
	struct _pdev	*next;
	si_t		*sih;
	uint16		vendor;
	uint16		device;
	uint		bus;
	uint		slot;
	uint		func;
	void		*address;
	bool		inuse;
} pdev_t;
#endif /* SBPCI */

/* Device entry points */
struct hndrte_devfuncs {
	void *(*probe)(hndrte_dev_t *dev, void *regs, uint bus,
	               uint16 device, uint coreid, uint unit);
	int (*open)(hndrte_dev_t *dev);
	int (*close)(hndrte_dev_t *dev);
	int (*xmit)(hndrte_dev_t *src, hndrte_dev_t *dev, struct lbuf *lb);
	int (*recv)(hndrte_dev_t *src, hndrte_dev_t *dev, void *pkt);
	int (*ioctl)(hndrte_dev_t *dev, uint32 cmd, void *buffer, int len,
	             int *used, int *needed, int set);
	void (*txflowcontrol) (hndrte_dev_t *dev, bool state, int prio);
	void (*poll)(hndrte_dev_t *dev);
	int (*xmit_ctl)(hndrte_dev_t *src, hndrte_dev_t *dev, struct lbuf *lb);
};

/* Use standard symbols for Armulator build which does not use the hndrte.lds linker script */
#if defined(_HNDRTE_SIM_) || defined(EXT_CBALL)
#define text_start	_start
#define text_end	etext
#define data_start	etext
#define data_end	edata
#define bss_start	__bss_start
#define bss_end		_end
#endif

extern char text_start[], text_end[];
extern char data_start[], data_end[];
extern char bss_start[], bss_end[], _end[];

/* Device support */
extern int hndrte_add_device(hndrte_dev_t *dev, uint16 coreid, uint16 device);
extern hndrte_dev_t *hndrte_get_dev(char *name);

/* ISR registration */
typedef void (*isr_fun_t)(void *cbdata);

extern int hndrte_add_isr(uint irq, uint coreid, uint unit,
                          isr_fun_t isr, void *cbdata);

/* Basic initialization and background */
extern void *hndrte_init(void);
extern void hndrte_poll(si_t *sih);
extern void hndrte_idle(si_t *sih);

/* Other initialization and background funcs */
extern void hndrte_isr(void);
extern void hndrte_timer_isr(void);
extern void hndrte_cpu_init(si_t *sih);
extern void hndrte_idle_init(si_t *sih);
extern void hndrte_arena_init(uintptr base, uintptr lim, uintptr stackbottom);
extern void hndrte_cons_init(si_t *sih);
extern int hndrte_log_init(void);

/* Console command support */
typedef void (*cons_fun_t)(uint32 arg, uint argc, char *argv[]);
extern	void hndrte_cons_addcmd(char *name, cons_fun_t fun, uint32 arg);

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

#ifdef BCMDBG

#define	TRACE_LOC		OSL_UNCACHED(0x18000044)	/* flash address reg in chipc */
#define	HNDRTE_TRACE(val)	do {*((uint32 *)TRACE_LOC) = val;} while (0)
#else
#define	HNDRTE_TRACE(val)	do {} while (0)
#endif

/* Enable mw console command */
#ifdef HNDRTE_CONSOLE
#ifndef BCMDBG_MEMTEST
#define BCMDBG_MEMTEST
#endif /* BCMDBG_MEMTEST */
#endif /* HNDRTE_CONSOLE */

/* debugging prints */
#ifdef BCMDBG_ERR
#define	HNDRTE_ERROR(args)	do {printf args;} while (0)
#else /* BCMDBG_ERR */
#define	HNDRTE_ERROR(args)	do {} while (0)
#endif /* BCMDBG_ERR */

/* assert */
#ifdef BCMDBG_ASSERT
#ifdef BCMSPACE
extern void hndrte_assert(char *exp, char *file, int line);
#define ASSERT(exp) \
	do { } while (0)
#else
extern void hndrte_assert(char *exp, int line);
#ifndef _FILENAME_
#define _FILENAME_ "_FILENAME_ is not defined"
#endif
#define ASSERT(exp) \
	do { } while (0)
#endif /* BCMSPACE */
#else
#define	ASSERT(exp)		do {} while (0)
#endif /* BCMDBG_ASSERT */

/* Timing */
typedef void (*to_fun_t)(void *arg);
typedef struct _ctimeout {
	struct _ctimeout *next;
	uint32 ms;
	to_fun_t fun;
	void *arg;
	bool expired;
} ctimeout_t;
extern void hndrte_delay(uint32 usec);
extern uint32 hndrte_time(void);
extern uint32 hndrte_update_now(void);
extern void hndrte_wait_irq(si_t *sih);
extern void hndrte_enable_interrupts(void);
extern void hndrte_disable_interrupts(void);
extern void hndrte_set_irq_timer(uint ms);
extern void hndrte_ack_irq_timer(void);
extern void hndrte_suspend_timer(void);
extern void hndrte_resume_timer(void);
extern void hndrte_trap_handler(trap_t *tr);

typedef struct hndrte_timer
{
	uint32	*context;		/* first field so address of context is timer struct ptr */
	void	*data;
	void	(*mainfn)(struct hndrte_timer *);
	void	(*auxfn)(void *context);
	ctimeout_t t;
	int	interval;
	int	set;
	int	periodic;
	bool	_freedone;
} hndrte_timer_t, hndrte_task_t;

extern bool hndrte_timeout(ctimeout_t *t, uint32 ms, to_fun_t fun, void *arg);
extern void hndrte_del_timeout(ctimeout_t *t);
extern void hndrte_init_timeout(ctimeout_t *t);

extern hndrte_timer_t *hndrte_init_timer(void *context, void *data,
                                         void (*mainfn)(hndrte_timer_t *),
                                         void (*auxfn)(void*));
extern void hndrte_free_timer(hndrte_timer_t *t);
extern bool hndrte_add_timer(hndrte_timer_t *t, uint ms, int periodic);
extern bool hndrte_del_timer(hndrte_timer_t *t);

extern int hndrte_schedule_work(void *context, void *data,
	void (*taskfn)(hndrte_timer_t *), int delay);

/* malloc, free */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
#define hndrte_malloc(_size)	hndrte_malloc_align((_size), 0, __FILE__, __LINE__)
extern void *hndrte_malloc_align(uint size, uint alignbits, char *file, int line);
#else
#define hndrte_malloc(_size)	hndrte_malloc_align((_size), 0)
extern void *hndrte_malloc_align(uint size, uint alignbits);
#endif /* BCMDBG_MEM */
extern void *hndrte_realloc(void *ptr, uint size);
extern int hndrte_free(void *ptr);
extern uint hndrte_memavail(void);
extern uint hndrte_hwm(void);
extern void hndrte_print_memuse(void);
#ifdef BCMDBG_MEMTEST
void hndrte_print_memwaste(uint32 arg, uint argc, char *argv[]);
#endif /* BCMDBG_MEMTEST */

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
#ifdef BCMDBG_MEM
extern void hndrte_print_malloc(void);
extern int hndrte_memcheck(char *file, int line);
#endif
extern void *hndrte_dma_alloc_consistent(uint size, void *pap, char *file, int line);
#else
extern void *hndrte_dma_alloc_consistent(uint size, void *pap);
#endif /* BCMDBG_MEM */
extern void hndrte_dma_free_consistent(void *va);

#ifdef DONGLEBUILD
extern void hndrte_reclaim(void);
#else
#define hndrte_reclaim() do {} while (0)
#endif /* DONGLEBUILD */

extern uint hndrte_arena_add(uint32 base, uint size);

#ifndef	 HNDRTE_STACK_SIZE
#define	 HNDRTE_STACK_SIZE	(8192)
#endif

#ifdef	BCMDBG
extern void hndrte_print_timers(uint32 arg, uint argc, char *argv[]);

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
extern uint __watermark;
#define	BCMDBG_TRACE(x)		__watermark = (x)
#else
#define	BCMDBG_TRACE(x)
#endif	/* !__arm__ && !__thumb__ && !__thumb2__ */
#else
#define	BCMDBG_TRACE(x)
#endif	/* BCMDBG */

#endif	/* _HNDRTE_H */
