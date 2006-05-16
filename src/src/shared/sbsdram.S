/*
 * BCM47XX Sonics SiliconBackplane SDRAM/MEMC core initialization
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <sbconfig.h>
#include <sbsdram.h>
#include <sbmemc.h>
#include <sbsocram.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <bcmnvram.h>

#include <mipsinc.h>

/* #define	DEBUG_SBSDRAM	1 */
#ifdef	DEBUG_SBSDRAM
	/* Trace function
	 * Write to prog space so we can see it in
	 * the Logic Analizer
	 */
#define	TRACE_INIT		\
	li	k0,0xb8000120;	\
	li	k1,0x11;	\
	sw	k1,0(k0);	\
	li	k1,0x01020108;	\
	sw	k1,4(k0);	\
	li	k0,0xbb000000

#define	TRACE(num, reg)	sw	reg,(num << 4)(k0)
#else
#define	TRACE(num, reg)
#define	TRACE_INIT
#endif
/*
 * Register usage within this file:
 *
 *		top	ncdlsearch	test_mem	Xdr_do_init	sb_reset_core
 *	v0:	retval			retval
 *	v1:	corerev	-		-		corerev		-
 *	a0:	coreptr	coreptr		-		coreptr		coreptr
 *	a1:		x		x		x		sdr/ddr flag
 *	a2:				x
 *	a3:				x
 *	t0:	-	-				config
 *	t1:	-	-				mode
 *	t2:	-	wr/strm		off		wr/strm
 *	t3:	-	rd/strd				rd/strd
 *	t4:	-	g/clkd				g/clkd
 *	t5:				x
 *	t6:	retaddr	-				-		-
 *	t7:	-	-				retaddr		-
 *	s0:		pass_count			-		-
 *	s1:		wrsum/clkdsum			-		-
 *	s2:		rdsum/pass_countmax		-		-
 *	s3:		gsum/strmmax			-		-
 *	s4:		wrlim/stdmmax			-		-
 *	s5:		rdlim/clkdmax			-		-
 *	s6:		glim/clkdlim			-		-
 *	s7:		dll				-		-
 *	t8:	-	-				x		tmp
 *	t9:	-	-				x		retaddr
 *	k0:	trace	trace		trace		-		-
 *	k1:	trace	trace		trace		-		-
 *	gp:
 *	sp:
 *	s8:	-	step				-		-
 *	ra:
 */


	LEAF(board_draminit)
	.set	noreorder

	/* Save return address */
	move	t6,ra

	TRACE_INIT

	/* Scan for an SDRAM controller (a0) */
	li	a0,KSEG1ADDR(SB_ENUM_BASE)
1:	lw	v1,(SBCONFIGOFF + SBIDHIGH)(a0)
	and	a1,v1,SBIDH_CC_MASK
	srl	a1,a1,SBIDH_CC_SHIFT
	beq	a1,SB_MEMC,foundctrl
	nop
	beq	a1,SB_SOCRAM,foundctrl
	nop
	beq	a1,SB_SDRAM,foundctrl
	nop
	addu	a0,SB_CORE_SIZE
	bne	a1,(SBIDH_CC_MASK >> SBIDH_CC_SHIFT),1b
	nop

	/* No SDRAM controller */
	jr	t6
	li	v0, 0

foundctrl:
	/* If we are already in RAM, just go and size it */
	bal	1f
	nop
1:	li	t0,PHYSADDR_MASK
	and	t0,t0,ra
	li	t1,SB_FLASH1
	blt	t0,t1,memprio_szmem
	nop

	/* For socram we don't need any nvram parms, just do a core reset */
	bne	a1,SB_SOCRAM,read_nvram
	nop
	bal	sb_core_reset
	li	a2,0
	/* and size memory */
	b	memprio_szmem
	nop

read_nvram:
	/* Find NVRAM (a2) */
	li	t0,KSEG1ADDR(SB_ENUM_BASE)	# Is there a chipcommon core?
	lw	t1,(SBCONFIGOFF + SBIDHIGH)(t0)
	and	t1,t1,SBIDH_CC_MASK
	srl	t1,t1,SBIDH_CC_SHIFT
	bne	t1,SB_CC,notcc
	nop

	/* It is a chipcommon core: */
	/* 1: Isolate memc's corerev in v1 */
	and	t2,v1,SBIDH_RCE_MASK
	srl	t2,t2,SBIDH_RCE_SHIFT
	and	v1,v1,SBIDH_RC_MASK
	or	v1,t2

	/* 1.5: 5365a0 lies about its revision, it is really 1 */
	bnez	v1,1f
	nop

	lw	t1,CC_CHIPID(t0)		# Get chipid
	andi	t1,CID_ID_MASK			# Check chipid
	bne	t1,BCM5365_DEVICE_ID,1f
	nop
	li	v1,1

	/* 2: use the 32MB window */
1:	li	t2,KSEG1ADDR(SB_FLASH2 - NVRAM_SPACE)
	li	t4,SB_FLASH2_SZ
	b	find_nvram
	nop

notcc:
	/* else use the 4MB window */
	li	t2,KSEG1ADDR(SB_FLASH1 - NVRAM_SPACE)
	li	t4,SB_FLASH1_SZ

find_nvram:
	li	t3,FLASH_MIN 
	li	t0,NVRAM_MAGIC

1:
	add	a2,t2,t3
	lw	t1,0(a2)
	beq	t0,t1,read_parms
	nop

	sll	t3,t3,1
	ble	t3,t4,1b
	nop

	/* Try embedded NVRAM at 4 KB and 1 KB as last resorts */
	li	a2,KSEG1ADDR(SB_FLASH1 + 0x1000)
	lw	t1,0(a2)
	beq	t0,t1,read_parms
	nop

	li	a2,KSEG1ADDR(SB_FLASH1 + 0x400)
	lw	t1,0(a2)
	beq	t0,t1,read_parms
	nop

	b	init			# No NVRAM
	li	a2, 0

read_parms:
	/* Get SDRAM parameters (t0, t1, t2) from NVRAM (a2) */
	lw	t0,8(a2)		# SDRAM init
	srl	t0,16
	lw	t2,12(a2)
	andi	t1,t2,0xffff		# SDRAM config
	srl	t2,16			# SDRAM refresh
	lw	t3,16(a2)		# SDRAM ncdl

init:
	/* Initialize SDRAM controller:	 sdram core */
	beq	a1,SB_SDRAM,sdram_init
	nop
	/* else it is a memc core */

memc_init:
	bnez	a2,1f			# Already have the parms in
	nop				#  t0, t1, t2, t3

	/* No nvram parms: assume DDRM16MX16X2 */

	li	t0,MEMC_DDR_INIT
	li	t1,MEMC_DDR_MODE
	li	t3,MEMC_DDR1_NCDL	# If rev1 (4712):
	beq	v1,1,1f
	nop
					# rev0, 2:
	li	t3,MEMC_DDR_NCDL

1:
	andi	a3,t0,MEMC_CONFIG_DDR	# Is it ddr or sdr?
	beqz	a3,memc_sdr_init
	nop

	/* Initialize DDR SDRAM */
memc_ddr_init:
	beqz	t3,ddr_find_ncdl	# Do we have ncdl values? (0s)
	nop

	li	t4,-1			# or ffs
	bne	t3,t4,break_ddr_ncdl
	nop

ddr_find_ncdl:

/* Register usage */
#define	pass_count	s0
#define	wrsum		s1
#define	rdsum		s2
#define	gsum		s3
#define	wrlim		s4
#define	rdlim		s5
#define	glim		s6
#define	dll		s7
#define	step		s8
#define	wr		t2
#define	rd		t3
#define	g		t4

	/* Initialize counter & accumulators */
	move	pass_count,zero
	move	wrsum,zero
	move	rdsum,zero
	move	gsum,zero

	/* Initialize with default values */
	li	wr,5
	li	rd,5
	bal	ddr_do_init
	li	g,10

	/* Read dll value */
	lw	dll,MEMC_NCDLCTL(a0)
	TRACE(1, dll)
	andi	dll,dll,0xfe
	srl	dll,dll,1
	beqz	dll,memprio_szmem		/* If zero, leave the default values */
	nop

	move	wrlim,dll		/* dll value is lim for wr, rd and g */
	move	rdlim,dll
	move	glim,dll
	TRACE(2, wrlim)
	TRACE(3, rdlim)
	TRACE(4, glim)

	addi	step,dll,15		/* step = (dll + 16 - 1) / 16 */
	srl	step,step,4
	TRACE(5, step)

	sub	wr,zero,dll		/* Negate dll as initial value */
	move	rd,wr
	move	g,wr

	/* Inner loop:	call ddr_do_init to re-initialize and the test mem */
loop:
	TRACE(6, wr)
	TRACE(7, rd)
	TRACE(8, g)	
	bal	ddr_do_init
	nop

	bal	test_mem
	nop

	TRACE(9, v0)
	beqz	v0,nextg
	nop

	/* Memory is ok */

	addi	pass_count,1
	add	wrsum,wrsum,wr
	add	rdsum,rdsum,rd
	add	gsum,gsum,g
	TRACE(0xa, pass_count)
	TRACE(0xb, wrsum)
	TRACE(0xc, rdsum)
	TRACE(0xd, gsum)

	bne	wr,dll,1f
	nop
	sll	wrlim,dll,1
	TRACE(0xe, wrlim)
1:

	bne	rd,dll,2f
	nop
	sll	rdlim,dll,1
	TRACE(0xf, rdlim)
2:

	bne	g,dll,nextg
	nop
	sll	glim,dll,1
	TRACE(0x10, glim)

nextg:
	add	g,g,step
	ble	g,glim,loop
	nop
	sub	g,zero,dll
	move	glim,dll
	TRACE(0x11, g)

	/* nextrd: */
	add	rd,rd,step
	ble	rd,rdlim,loop
	nop
	sub	rd,zero,dll
	move	rdlim,dll
	TRACE(0x12, rd)

	/* nextwr: */
	add	wr,wr,step
	ble	wr,wrlim,loop
	nop

	/* All done, calculate average values and program them */
	
	TRACE(0x13, pass_count)
	TRACE(0x14, wrsum)
	TRACE(0x15, rdsum)
	TRACE(0x16, gsum)
	beqz	pass_count,1f
	nop

	div	zero,wrsum,pass_count
	mflo	wr

	div	zero,rdsum,pass_count
	mflo	rd

	div	zero,gsum,pass_count
	mflo	g

	b	ddr_got_ncdl
	nop

	/* No passing values, panic! (use defaults) */
1:
	li	t3,MEMC_DDR1_NCDL	# If rev1:
	beq	v1,1,2f
	nop
					# rev0, 2:
	li	t3,MEMC_DDR_NCDL
2:

break_ddr_ncdl:
	andi	t4,t3,0xff		# t4:	g
	srl	t2,t3,16		# t2:	wr
	andi	t2,t2,0xff
	srl	t3,t3,8			# t3:	rd
	andi	t3,t3,0xff

ddr_got_ncdl:
	TRACE(0x17, wr)
	TRACE(0x18, rd)
	TRACE(0x19, g)
	bal	ddr_do_init
	nop

	b	memprio_szmem
	nop

memc_sdr_init:
	beqz	t3,sdr_find_ncdl	# Do we have ncdl values?
	nop

	li	t4,-1
	bne	t3,t4,break_sdr_ncdl
	nop

sdr_find_ncdl:

/* Register usage */
#define	pass_count	s0
#define	clkdsum		s1
#define	pass_countmax	s2
#define	strmmax		s3
#define	strdmax		s4
#define	clkdmax		s5
#define	clkdlim		s6
#define	strm		t2
#define	strd		t3
#define	clkd		t4

#define	STRMLIM		4
#define	STRDLIM		16
#define	CLKDLIM		128
#define	CLKDLIM_IC	256

	/* Initialize counter & saved values */
	move	pass_countmax,zero
	move	strmmax,zero
	move	strdmax,zero
	li	clkdlim,CLKDLIM

	and	strm,t0,0x2000		/* Test for internal clock (Using strm as a temp) */
	beqz	strm,strmloop
	nop

	li	clkdlim,CLKDLIM_IC

	move	strm,zero		/* strm loop */
strmloop:
	move	strd,zero
strdloop:
	move	pass_count,zero
	move	clkdsum,zero
	move	clkd,zero

	/* Inner loop:	call sdr_do_init to re-initialize and the test mem */
clkdloop:
	TRACE(0x1a, strm)
	TRACE(0x1b, strd)
	TRACE(0x1c, clkd)	
	bal	sdr_do_init
	nop

	bal	test_mem
	nop

	TRACE(0x1d, v0)
	beqz	v0,failclkd
	nop

	/* Memory is ok */

	addi	pass_count,1
	add	clkdsum,clkdsum,clkd
	TRACE(0x1e, pass_count)
	TRACE(0x1f, clkdsum)
	b	nextclkd
	nop

failclkd:
	bnez	pass_count,clkdout	# End of passing range, leave clkd loop
	nop

nextclkd:
	addi	clkd,clkd,1
	blt	clkd,clkdlim,clkdloop
	nop

clkdout:
	/* If no passing values, skip to next strm */
	beqz	pass_count,nextstrm
	nop

	/* If this is a new max, Save the values */
	ble	pass_count,pass_countmax,nextstrd
	nop

	move	pass_countmax,pass_count
	div	zero,clkdsum,pass_count
	mflo	clkdmax
	move	strdmax,strd
	move	strmmax,strm
	TRACE(0x20, pass_count)
	TRACE(0x21, clkdmax)
	TRACE(0x22, strdmax)
	TRACE(0x23, strmmax)

nextstrd:
	addi	strd,strd,1
	blt	strd,STRDLIM,strdloop
	nop

nextstrm:
	addi	strm,strm,1
	blt	strm,STRMLIM,strmloop
	nop

	/* All done, program the new ncdl values */
	
	TRACE(0x24, pass_count)
	TRACE(0x25, clkdmax)
	TRACE(0x26, strdmax)
	TRACE(0x27, strmmax)

	beqz	pass_countmax,1f
	nop

	move	clkd,clkdmax
	move	strd,strdmax
	move	strm,strmmax
	b	sdr_got_ncdl
	nop

	/* No passing values, panic! (use defaults) */
1:
	li	t3,MEMC_SDR1_NCDL	# If rev1:
	beq	v1,1,2f
	nop
					# rev0, 2:
	li	t3,MEMC_SDR_NCDL
2:

break_sdr_ncdl:
	andi	t4,t3,0xff		# t4:	cd
	srl	t2,t3,16		# t2:	sm
	andi	t2,t2,3			#	sm is 2 bits only
	srl	t3,t3,8			# t3:	sd
	andi	t3,t3,0xf		#	sd is 4 bits

sdr_got_ncdl:
	bal	sdr_do_init
	nop

	b	memprio_szmem
	nop

sdram_init:
	bnez	a2,1f			# Already have the parms in t0, t1, t2
	nop

	/* Use default SDRAM parameters */
	li	t0, SDRAM_INIT
	li	t1, SDRAM_CONFIG
	li	t2, SDRAM_REFRESH

	/* Initialize SDRAM */
1:
	bal	sb_core_reset
	li	a2,0

	sw	t1, 4(a0)		# SDRAM config
	li	a1, 0x000a
	sw	a1, 0(a0)
	li	a1, 0x000c
	sw	a1, 0(a0)
	li	a1, 0x0009
	sw	a1, 0(a0)		# 1st refresh of power up sequence
	sw	a1, 0(a0)		# 2nd refresh of power up sequence
	sw	a1, 0(a0)		# 3rd refresh of power up sequence
	sw	a1, 0(a0)		# 4th refresh of power up sequence
	sw	a1, 0(a0)		# 5th refresh of power up sequence
	sw	a1, 0(a0)		# 6th refresh of power up sequence
	sw	a1, 0(a0)		# 7th refresh of power up sequence
	sw	a1, 0(a0)		# 8th refresh of power up sequence
	sw	t0, 0(a0)		# SDRAM init
	sw	t2, 8(a0)		# SDRAM refresh


	/* Change the memory priority inversion counter value*/
	/* Determine memory size and return */
memprio_szmem:

#ifdef APPLE
	lw	t0, MEMC_PRIORINV(a0) 
	li 	t1, 0xFFFF0000
	and     t0, t0, t1
	ori	t0, t0, 0x1
	sw      t0, MEMC_PRIORINV(a0)
#endif
	lw	t0,(SBCONFIGOFF + SBIDHIGH)(a0)
	and	t0,t0,SBIDH_CC_MASK
	srl	t0,t0,SBIDH_CC_SHIFT
	bne	t0,SB_SOCRAM,szmem_alias
	nop

	/* The socram core tells us how much memory there is */
	lw	t0,SOCRAM_MEMSIZE(a0)
	addi	t0,SOCRAM_MEMSIZE_BASESHIFT
	li	v0,1
	sll	v0,v0,t0
	jr	t6
	nop
	
szmem_alias:
	li	t0,KSEG1
	li	t2,0xaa55beef
	sw	zero,0(t0)
	li	v0,(1 << 20)

1:	or	t0,v0,KSEG1
	sw	t2,0(t0)
	li	t0,KSEG1
	lw	t1,0(t0)
	beq	t1,t2,done
	nop

	sll	v0,v0,1
	bne	v0,(128 << 20),1b
	nop
	/* Didn't find an alias, must be 128MB */

done:
	jr	t6	
	nop


	/*
	 * Test memory
	 *
	 * Uses arg in t2(wr/sd), t3(rd/sm) and t4(g/clkd)
	 * Returns success (1) or failure (0) in v0
	 * Uses a1, a2, a3 & t5
	 */
test_mem:
	/* Use t4 to generate a semi-random address in the second KB */
	li	a1,KSEG1
	addi	a2,t4,255
	sll	a2,a2,2
	add	a1,a1,a2

	/* First set: 0 & its negation */
	li	a2,0
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop

	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Second set: 0xaaaaaaaa & its negation */
	li	a2,0xaaaaaaaa
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop

	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Third set: 0x12345678 & its negation */
	li	a2,0x12345678
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop

	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Fourth set: the ncdl & its negation */
	sll	a2,t2,8
	or	a2,t3
	sll	a2,a2,8
	or	a2,t4
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop

	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Fifth set: the CPU count register & its negation */
	mfc0	a2,$9
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop

	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	jr	ra
	li	v0,1

bad_mem:
	jr	ra
	li	v0,0


	/* Do an init of the memc core for ddr
	 *	a0:	memc core pointer
	 *	t0:	memc config value
	 *	t1:	memc mode value
	 *	t2:	memc wr ncdl value
	 *	t3:	memc rd ncdl value
	 *	t4:	memc g ncdl value
	 *
	 * Uses a1, t7, t8, t9 (here and by calling sb_core_reset)
	 */
ddr_do_init:

	/* Save return address */
	move	t7,ra

	bal	sb_core_reset
	li	a1,0

	li	a1,MEMC_CONFIG_INIT
	or	a1,a1,t0
	sw	a1,MEMC_CONFIG(a0)

	li	a1,MEMC_DRAMTIM25_INIT		# Assume CAS latency of 2.5
	andi	t8,t1,0xf0			# Find out the CAS latency
	bne	t8,0x20,1f
	nop
	li	a1,MEMC_DRAMTIM2_INIT		# CAS latency is 2
1:	
	sw	a1,MEMC_DRAMTIM(a0)

1:	andi	t8,t3,0xff
	sll	a1,t8,8			# Replicate rd ncdl 4 times
	or	a1,a1,t8
	sll	t8,a1,16
	or	t8,t8,a1
	li	a1,MEMC_RDNCDLCOR_INIT
	or	a1,a1,t8
	sw	a1,MEMC_RDNCDLCOR(a0)

	li	a1,MEMC_1_WRNCDLCOR_INIT # If rev1:
	beq	v1,1,1f
	nop
					# rev0, 2
	li	a1,MEMC_WRNCDLCOR_INIT
1:
	andi	t8,t2,0xff
	or	a1,a1,t8
	sw	a1,MEMC_WRNCDLCOR(a0)

	li	a1,MEMC_DQSGATENCDL_INIT
	andi	t8,t4,0xff
	or	a1,a1,t8
	sw	a1,MEMC_DQSGATENCDL(a0)

	li	a1,MEMC_1_MISCDLYCTL_INIT # If rev1:
	beq	v1,1,2f
	nop
					# rev0,2
	li	a1,MEMC_MISCDLYCTL_INIT
2:
	sw	a1,MEMC_MISCDLYCTL(a0)
3:
	li	a1,MEMC_NCDLCTL_INIT
	sw	a1,MEMC_NCDLCTL(a0)

	li	a1,MEMC_CONTROL_INIT0
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT1
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_MODEBUF_INIT0
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_MODEBUF_INIT1
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT3
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT4
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT5
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT5
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_REFRESH_INIT
	sw	a1,MEMC_REFRESH(a0)
	
	li	a1,MEMC_MODEBUF_INIT2
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT6
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT7
	sw	a1,MEMC_CONTROL(a0)

	/* Wait for SDRAM controller to refresh.
	 * We want 8uS delay.
	 */
	li	t8,50
1:	lw	a1,(SBCONFIGOFF + SBIDLOW)(a0)
	lw	a1,(SBCONFIGOFF + SBIDHIGH)(a0)

	bnez	t8,1b
	subu	t8,1

	jr	t7
	nop

	/* Do an init of the memc core for sdr
	 *	a0:	memc core pointer
	 *	t0:	memc config value
	 *	t1:	memc mode value
	 *	t2:	memc strobe mode ncdl value
	 *	t3:	memc strobe delay ncdl value
	 *	t4:	memc clock delay ncdl value
	 *
	 * Uses a1, t7, t8, t9 (here and by calling sb_core_reset)
	 */
sdr_do_init:

	/* Save return address */
	move	t7,ra

	bal	sb_core_reset
	li	a1,0x40

	/* Initialize SDRAM */
	li	a1,MEMC_SD_CONFIG_INIT
	or	a1,a1,t0
	sw	a1,MEMC_CONFIG(a0)

	li	a1,MEMC_SD_DRAMTIM3_INIT # Assume CAS latency of 3
	andi	t8,t1,0xf0		# Find out the CAS latency
	bne	t8,0x20,1f
	nop
	li	a1,MEMC_SD_DRAMTIM2_INIT # CAS latency is 2
1:	
	sw	a1,MEMC_DRAMTIM(a0)

	andi	t8,t4,0xff
	ble	t8,MEMC_CD_THRESHOLD,1f	# if (cd <= MEMC_CD_THRESHOLD) rd = cd
	nop

	li	t8,MEMC_CD_THRESHOLD	# else rd = MEMC_CD_THRESHOLD

1:					# t8 is now rd
	sll	a1,t8,8			#  .. replicate it 4 times
	or	a1,a1,t8
	sll	t8,a1,16
	or	t8,t8,a1
	li	a1,MEMC_SD_RDNCDLCOR_INIT
	or	a1,a1,t8
	sw	a1,MEMC_RDNCDLCOR(a0)

	li	a1,MEMC_SD1_WRNCDLCOR_INIT # rev1
	beq	v1,1,1f
	nop

	li	a1,MEMC_SD_WRNCDLCOR_INIT # rev0, 2
1:
	li	t8,0
	ble	t4,MEMC_CD_THRESHOLD,2f	# if (cd <= MEMC_CD_THRESHOLD) wr = 0
	nop
	
	andi	t8,t4,0xff		# else wr = cd - MEMC_CD_THRESHOLD
	sub	t8,t8,MEMC_CD_THRESHOLD
	andi	t8,t8,0xff

2:					# t8 is now wr, a0 is extra bits
	or	a1,a1,t8
	sw	a1,MEMC_WRNCDLCOR(a0)

	andi	t8,t2,3
	sll	a1,t8,28
	andi	t8,t3,0xf
	sll	t8,t8,24
	or	t8,t8,a1
	li	a1,MEMC_SD1_MISCDLYCTL_INIT
	beq	v1,1,3f			# If rev1:
	nop
					# rev0, 2:
	li	a1,MEMC_SD_MISCDLYCTL_INIT
3:
	or	a1,a1,t8
	sw	a1,MEMC_MISCDLYCTL(a0)

	li	a1,MEMC_SD_CONTROL_INIT0
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT1
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_REFRESH_INIT
	sw	a1,MEMC_REFRESH(a0)

	li	a1,MEMC_SD_MODEBUF_INIT
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_SD_CONTROL_INIT3
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT4
	sw	a1,MEMC_CONTROL(a0)

	li	t8,50
1:	lw	a1,(SBCONFIGOFF + SBIDLOW)(a0)
	lw	a1,(SBCONFIGOFF + SBIDHIGH)(a0)
	bnez	t8,1b
	subu	t8,1

	jr	t7
	nop


	/* Special sb_core_reset that makes sure the first time
	 * clock is enabled, address line 6 is in the state specified
	 * by a1.
	 *
	 * a0:	Core pointer
	 * a1:	0x40 if a6 needs to be 1, 0 otherwise
	 * uses t8, t9
	 */

	.align 6

sb_core_reset:

	/* Save return address */
	move	t9,ra
        
	/* run uncached */
	bal     kseg1_switch
	nop                                

	/* Figure out our address */
	bal	h0
	nop
h0:	add	t8,ra,24		# This is (h1 - h0)
	andi	t8,t8,0x40
	bne	t8,a1,alt_core_reset
	nop

	/* Set reset while enabling the clock */
	li	t8,(SBTML_FGC | SBTML_CLK | SBTML_RESET)
h1:	sw	t8,(SBCONFIGOFF + SBTMSTATELOW)(a0)
	b	cont
	nop

	/* Now pad to 0x40: We want (h2 - h1) == 0x40 and there
	 * are 5 instructions inbetween them.
	 */
	.space	(0x40 - 20)

alt_core_reset:
	/* Set reset while enabling the clock */
	li	t8,(SBTML_FGC | SBTML_CLK | SBTML_RESET)
h2:	sw	t8,(SBCONFIGOFF + SBTMSTATELOW)(a0)

cont:
	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Clear reset */
	li	t8, (SBTML_FGC | SBTML_CLK)
	sw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Leave clock enabled */
	li	t8, SBTML_CLK
	sw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	jr	t9
	nop

        
kseg1_switch:
	and     ra,ra,PHYSADDR_MASK
	or      ra,ra,KSEG1
	jr      ra
	nop 
        
	.set	reorder
	END(board_draminit)
