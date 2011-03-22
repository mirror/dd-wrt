/*
 * BCM47XX Denali based memory controller initialization
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: aisdram.S,v 1.18.2.6 2010/03/19 01:53:59 Exp $
 */

#include <hndsoc.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <sbsdram.h>
#include <sbmemc.h>
#include <sbsocram.h>
#include <dmemc_core.h>
#include <sbchipc.h>

#include <mipsinc.h>

/* Debug macro - write a number to a chipc reg - use it with caution,
 *  it changes k0 and k1 registers.
 */
#ifdef	BCMDBG

#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
#define	FLADDR_OFF	0
#define	FLDATA_OFF	12
#else
#define	FLADDR_OFF	4
#define	FLDATA_OFF	8
#endif

#define TRACEINIT(x) \
	li	k0,KSEG1ADDR(0x18000040); \
	li	k1,x; \
	sw	k1,FLADDR_OFF(k0)
	
#define TRACE(x) \
	li	k1,x; \
	sw	k1,FLADDR_OFF(k0)

#define TRACE2(x) \
	li	k1,x; \
	sw	k1,FLDATA_OFF(k0)

#else
#define TRACEINIT(x)
#define TRACE(x)
#define TRACE2(x)
#endif	/* BCMDBG */
	.text

dmemc_init:
	.word	0,	0x00000101
	.word	1,	0x01030100
	.word	2,	0x01030101
	.word	3,	0x01010101
	.word	5,	0x00000100
	.word	7,	0x01010000
	.word	8,	0x00000100
	.word	9,	0x00000100
	.word	10,	0x01010101
	.word	12,	0x00010200
	.word	16,	0x0f00000a
	.word	20,	0x64000000
	.word	27,	0x00000001
	.word	28,	0x0039000a
	.word	29,	0x081b002b
	.word	30,	0x00000000
	.word	31,	0x00000000
	.word	33,	0x000200c8
	.word	35,	0x000000c8
	.word	36,	0x00000000
	.word	125,	0x01010000
	.word	126,	0x02000100
	.word	129,	0x02000200
	.word	132,	0x00000000
	.word	138,	0x00000000
	.word	139,	0x00000000
	.word	149,	0x26c00300
	.word	150,	0x26c00300
	.word	151,	0x00000005
	.word	DMEMC_TABLE_END

sdr_init:
	.word	4,	0x00000100
	.word	6,	0x00000000
	.word	11,	0x00030001
	.word	14,	0x00020000
	.word	15,	0x0f000102
	.word	17,	0x05000002
	.word	18,	0x00000003
	.word	19,	0x00090200
	.word	21,	0x70006400
	.word	22,	0x7f000070
	.word	23,	0x00400000
	.word	24,	0x09030600
	.word	25,	0x00170017
	.word	32,	0x00320000
	.word	34,	0x000a4186
	.word	DMEMC_TABLE_END

ddr1_init:
	.word	4,	0x00000100
	.word	6,	0x00000100
	.word	11,	0x00030000
	.word	14,	0x02030200
	.word	15,	0x0f010203
	.word	17,	0x06000002
	.word	18,	0x00000006
	.word	19,	0x0010010e
	.word	21,	0x20006400
	.word	22,	0x5700002a
	.word	23,	0x00340000
	.word	24,	0x0e030800
	.word	25,	0x000f000f
	.word	26,	0x000f000f
	.word	32,	0x00000000
	.word	34,	0x000d2d89
	.word	127,	0x00000001
	.word	128,	0x07000300
	.word	130,	0x00010103
	.word	131,	0x00000200
	.word	133,	0x06120000
	.word	134,	0x06120612
	.word	135,	0x06120612
	.word	136,	0x00000612
	.word	137,	0x00000032
	.word	140,	0x001f4008
	.word	141,	0x001f4008
	.word	142,	0x002ee004
	.word	143,	0x002ee004
	.word	146,	0x000f0033
	.word	147,	0xf4113c17
	.word	148,	0xf4113c17
	.word	DMEMC_TABLE_END

ddr2_init:
	.word	4,	0x00010100
	.word	6,	0x00000000
	.word	11,	0x00030000
	.word	14,	0x02030203
	.word	15,	0x0f030204
	.word	17,	0x08000002
	.word	18,	0x00000004
	.word	19,	0x000f020e
	.word	21,	0x16006400
	.word	22,	0x6026162a
	.word	23,	0x00340000
	.word	24,	0x35060c00
	.word	25,	0x00200020
	.word	26,	0x00200020
	.word	32,	0x006b0000
	.word	34,	0x003848e1
	.word	127,	0x00000002
	.word	128,	0x07000404
	.word	130,	0x03020304
	.word	131,	0x00000400
	.word	133,	0x081b0000
	.word	134,	0x081b081b
	.word	135,	0x081b081b
	.word	136,	0x0000081b
	.word	137,	0x00400642
	.word	140,	0x00164008
	.word	141,	0x00164008
	.word	142,	0x00236004
	.word	143,	0x00236004
	.word	146,	0x000f0133
	.word	147,	0xf4112c17
	.word	148,	0xf4112c17
	.word	DMEMC_TABLE_END

	/* Register conventions.
	 *	Inherited from sisdram.S:
	 *		s2 = SI_ENUM_BASE
	 *		s5 = Relocation factor
	 *		s6 = ChipId reg
	 *		s7 = ChipType
	 *	Local:
	 *		s0 = sdram_config + sdram_refresh values
	 *		s1 = package opt
	 *		s3 = Controller corerev
	 *		s4 = Controller coreid
	 *		s8 = config_ncdl
	 *		a1 = dmemc regs
	 *		a2 = dmemc DMP regs
	 *		a3 = memory type (sdr,ddr1,ddr2)
	 */

	LEAF(ai_draminit)
	.set	mips32
	.set	noreorder

	TRACE(0x415301)

	move	t6,ra

	/* Scan for a Denali DDR controller (a0) */
	lw	a0,CC_EROMPTR(s2)
	li	t0,KSEG1			# t0 = KSEG1
	or	a0,a0,t0			# a0 points to the EROM
	TRACE(0x415302)
1:
#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
	xor	t0,a0,4
	lw	t0,0(t0)
	add	t3,a0,4
	xor	t3,t3,4
	lw	t3,0(t3)
#else
	lw	t0,0(a0)			# t0 = CIA
	lw	t3,4(a0)			# t3 = CIB
#endif
	and	t1,t0,ER_TAG
	TRACE(0x415303)
	beq	t1,ER_END,noctrl
	nop
	TRACE(0x415304)
	beq	t1,ER_CI,2f
	nop
	TRACE(0x415305)
	b	1b
	addi	a0,4

2:	TRACE(0x415306)
	and	s4,t0,CIA_CID_MASK
	srl	s4,s4,CIA_CID_SHIFT		# s4 has controler coreid

	beq	s4,DMEMC_CORE_ID,founddmemc
	nop

	beq	s4,DMEMS_CORE_ID,founddmemc
	nop

	b	1b
	addi	a0,8				# Skip CIB too

	/* No DMEMC controller found */
noctrl:	TRACE(0x415307)
	jr	t6
	li	v0,-1

founddmemc:
	TRACE(0x415308)
	/* If we found the controller, but we are already in RAM, there is nothing
	 * to do. This will change if/when we have an AI chip with MIPS and
	 * SOCRAM only.
	 */
	bnez	s5,1f				# Not running from RAM, go ahead
	nop
	
	jr	t6				# Return with 0 rc.
	move	v0,zero
	
	/* We'll cheat a little: memory controllers don't have master ports, so
	 * the EROM entry right after the CIDs is the slave port for the registers
	 */
1:
#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
	add	a1,a0,8
	xor	a1,a1,4
	lw	a1,0(a1)
#else
	lw	a1,8(a0)
#endif
	li	t2,AD_ADDR_MASK
	and	a1,a1,t2
	li	t0,KSEG1			# t0 = KSEG1
	or	a1,a1,t0			# a1: dmemc regs
	/* after that, the first slave wrapper will be its DMP registers */
	addi	a0,12
1:
#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
	xor	t0,a0,4
	lw	t0,0(t0)
#else
	lw	t0,0(a0)
#endif
	and	t1,t0,ER_TAG
	beq	t1,ER_ADD,addesc
	nop
	b	1b
	addi	a0,4

addesc:	and	t1,t0,AD_ST_MASK
	beq	t1,AD_ST_SWRAP,swrap
	nop
	b	1b
	addi	a0,4

swrap:	and	a2,t0,t2
	li	t0,KSEG1			# t0 = KSEG1
	or	a2,a2,t0			# a2: dmemc DMP regs

	/* Got our core, reset it */
	TRACE(0x415309)
	bal	ai_core_reset
	nop

	/* Get package option for later */
	TRACE(0x41530a)
	li	t0,CID_PKG_MASK
	and	t0,t0,s6
	srl	s1,t0,CID_PKG_SHIFT		# s1 = package opt

	/* Find out the type of memory from the straps */
	/* Corerevs 0 & 1 did not have this register, so we have to
	 * check the corerev and use chipstatus for those two.
	 */
	and	t3,t3,CIB_REV_MASK
	srl	s3,t3,CIB_REV_SHIFT		# s3 = core revision
	beq     s4,DMEMS_CORE_ID,1f
	li	t0,1
	ble	s3,t0,is16x
	nop

1:	/* Not a 4716/47162 (a0?) read the stat register */
	lw	t0,DMEMC_STAT(a1)
	li	t1,DM_STAT_MASK
	and	a3,t0,t1			# a3 == 4 if ddr2, 2 if ddr1, 1 if sdr.
	b	find_nvram
	nop

	/* Check chipc:	chipstatus for the ddr1/ddr2 strapping option */
is16x:	TRACE(0x41530b)
	lw	t0,CC_CHIPST(s2)
	li	t1,0x200
	and	t0,t0,t1
	beqz	t0,find_nvram
	li	a3,DM_STAT_DDR2

	li	a3,DM_STAT_DDR1

	/* Read sdram_config from nvram */
find_nvram:
	TRACE(0x41530c)
	li	t0,KSEG1ADDR(SI_FLASH2 - NVRAM_SPACE)
	li	t1,FLASH_MIN 
	li	t2,SI_FLASH2_SZ
	li	t3,NVRAM_MAGIC

1:	add	t4,t0,t1
	lw	t5,0(t4)
	beq	t3,t5,read_config
	nop

	sll	t1,t1,1
	ble	t1,t2,1b
	nop

	/* Try embedded NVRAM at 4 KB and 1 KB as last resorts */
	TRACE(0x41530d)
	li	t4,KSEG1ADDR(SI_FLASH2 + 0x1000)
	lw	t5,0(t4)
	beq	t3,t5,read_config
	nop

	TRACE(0x41530e)
	li	t4,KSEG1ADDR(SI_FLASH2 + 0x400)
	lw	t5,0(t4)
	beq	t3,t5,read_config
	nop

	/* No nvram, pick sone defaults */

	/* assume DDRM16MX16 if ddr1 */
	TRACE(0x41530f)
	bne	a3,DM_STAT_DDR1,1f
	nop

	beq	s4,DMEMC_CORE_ID,init_dmemc
	li	s0,0x283		# Value for dmemc

	/* Use DDR1M16MX16 if QT and DDR1M32MX16 otherwise */
	beq	s1,HWSIM_PKG_ID,init_dmemc
	li	s0,0x103		# Value for dmems

	b	init_dmemc
	li	s0,0x003		# Value for dmems at 200MHz

1:	beq	a3,DM_STAT_SDR,init_dmemc
	li	s0,0x002		# Value for SDR

	/* For ddr2, use DDR2M32X16X2 if QT and DDR2M128X16X2 otherwise */
	beq	s1,HWSIM_PKG_ID,init_dmemc
	li	s0,0x104

	b	init_dmemc
	li	s0,0x144
	
read_config:
	/* sdram_config is a 16bit value 12 bytes inside the nvram hdr.
	 * Currently it is defined as:
	 *	10:8	column_size as per control13
	 *	7	reduc i.e. memory is half the width of the controller
	 *	6	8 banks
	 *	5	bypass
	 *	2:0	cas latency
	 *
	 * sdram_refresh:
	 *	15:8	delay_dqs_0
	 *	7:0	clk_wr_delay, or clk_wr_delay_0 (for corerev >= 2)
	 *
	 * sdram_ncdl:	control22
	 *	31:24	clk_dqs_delay, or clk_wr_delay_1 (for corerev >= 2)
	 *	23:16	delay_dqs_3
	 *	15:8	delay_dqs_2
	 *	7:0	delay_dqs_1
	 */
	TRACE(0x415310)
	lw	s0,12(t4)	# Pick up sdram_config & sdram_refresh
	lw	s8,16(t4)	# Pick up sdram_ncdl

	/* Initialize DMEMC/DMEMS */
init_dmemc:
	/* For DDR2, init pvt controller */
	bne	a3,DM_STAT_DDR2,init_regs
	nop

	TRACE(0x415311)
	li	t0,1
	sw	t0,DMEMC_PVTGROUPJ(a1)
	sw	zero,DMEMC_PVTGROUPA(a1)
	sw	t0,DMEMC_PVTGROUPA(a1)

	/* Init the common regs */
init_regs:
	TRACE(0x415312)
	la	a0,dmemc_init
	bal	dmemc_init_regs			# dmemc_init_regs(a0): Inits from the table @ a0
	nop

	la	a0,sdr_init
	beq	a3,DM_STAT_SDR,1f
	nop

	TRACE(0x415313)
	la	a0,ddr1_init
	beq	a3,DM_STAT_DDR1,1f
	nop

	TRACE(0x415314)
	la	a0,ddr2_init
1:	bal	dmemc_init_regs
	nop

	/* Fixup differences between dmems & dmemc */
fixs:	bne	s4,DMEMS_CORE_ID,fix2
	nop

	li	t0,0x02020002
	beq	a3,DM_STAT_SDR,1f
	nop

	li	t0,0x18006400
	sw	t0,DMEMC_CONTROL21(a1)
	li	t0,0x7f000018			# DLL DQS delay for DDR1
	sw	t0,DMEMC_CONTROL22(a1)
	li	t0,0x00500000
	sw	t0,DMEMC_CONTROL23(a1)
	li	t0,0x00320000			# Change MRS data for DDR1
	sw	t0,DMEMC_CONTROL32(a1)

	li	t0,0x02000002

1:	sw	t0,DMEMC_CONTROL52(a1)
	li	t0,0x00000001
	sw	t0,DMEMC_CONTROL53(a1)

	/* Fixup differences in 47162 */
fix2:	li	t0,CID_ID_MASK
	and	t0,t0,s6
	bne	t0,BCM47162_CHIP_ID,nvover
	nop

	li	t0,0x16006400
	sw	t0,DMEMC_CONTROL21(a1)
	li	t0,0x00480000
	sw	t0,DMEMC_CONTROL23(a1)

	/* Presumaby this is for DDR1 only? */
	li	t0,0x61161616
	sw	t0,DMEMC_CONTROL22(a1)

	/* Override the dll delays from nvram if provided */
nvover:	beqz	s8,chhalf
	nop

	li	t0,1
	ble	s3,t0,old_ncdl
	nop
	li	t0,0x7f000000
	and	t0,t0,s0		# delay_dqs_0
	srl	t0,t0,15
	lw	t1,DMEMC_CONTROL140(a1)
	li	t2,~0x0000fe00
	and	t1,t1,t2
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL140(a1)

	li	t0,0x7f
	and	t0,t0,s8		# delay_dqs_1
	sll	t0,t0,9
	lw	t1,DMEMC_CONTROL141(a1)
	and	t1,t1,t2
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL141(a1)
	
	li	t0,0x7f0000
	and	t0,t0,s0		# clk_wr_delay_0
	srl	t0,t0,8
	lw	t1,DMEMC_CONTROL142(a1)
	li	t2,~0x00007f00
	and	t1,t1,t2
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL142(a1)

	li	t0,0x7f000000
	and	t0,t0,s8		# clk_wr_delay_1
	srl	t0,t0,16
	lw	t1,DMEMC_CONTROL143(a1)
	and	t1,t1,t2
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL143(a1)

	b	chhalf
	nop

old_ncdl:
	li	t0,0x7f000000
	and	t0,t0,s0		# delay_dqs_0
	or	t0,t0,0x6400
	sw	t0,DMEMC_CONTROL21(a1)
	li	t0,0x007f0000
	and	t0,t0,s0		# clk_wr_delay
	sw	t0,DMEMC_CONTROL23(a1)

	sw	s8,DMEMC_CONTROL22(a1)
	
	/* Check for half-width */
chhalf:	li	t0,0x80
	and	t0,t0,s0
	beqz	t0,ch8banks
	nop

setreduc:
	/* Set reduc bit if half-wide */
	TRACE(0x415315)
	lw	t0,DMEMC_CONTROL08(a1)
	li	t1,0x01000000
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL08(a1)

	/* Check for 8-bank DDRs */
ch8banks:
	li	t0,0x40
	and	t0,t0,s0
	beqz	t0,docaslat
	nop

	/* Change regs for 8-bank DDRs */
do8banks:	
	lw	t0,DMEMC_CONTROL05(a1)
	li	t1,0x00010000
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL05(a1)

	lw	t0,DMEMC_CONTROL19(a1)
	li	t1,0x0000000e
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL19(a1)

	lw	t0,DMEMC_CONTROL24(a1)
	li	t1,~0xff000000
	li	t2,0x22000000
	and	t0,t0,t1
	or	t0,t0,t2
	sw	t0,DMEMC_CONTROL24(a1)

	lw	t0,DMEMC_CONTROL34(a1)
	li	t1,~0x00ff0000
	li	t2,0x00250000
	and	t0,t0,t1
	or	t0,t0,t2
	sw	t0,DMEMC_CONTROL34(a1)

	/* Set the right value for column size and CAS latency */
docaslat:
	TRACE(0x415316)
	li	t0,0x0707
	and	t0,t0,s0
	sw	t0,DMEMC_CONTROL13(a1)
	/* Set caslat_lin and caslat_lin_gate */
	lw	t2,DMEMC_CONTROL16(a1)
	andi	t0,s0,7			# Isolate cas
	/* Take care of fractional CAS latencies for DDR1 */
	li	t3,0
	bne	a3,DM_STAT_DDR1,1f
	nop
	andi	t1,t0,4
	beqz	t1,1f
	nop
	andi	t0,t0,3			# take off half bit
	li	t3,1
1:	sll	t0,t0,1			#  * 2
	add	t0,t0,t3
	addi	t1,t0,1			#  + 1 => caslin
	sll	t1,t1,8
	or	t2,t2,t1
	addi	t0,t0,-1		# and -1 => caslin_gate
	sll	t0,t0,16
	or	t2,t0,t2
	sw	t2,DMEMC_CONTROL16(a1)

	/* Finally set bypass mode if needed, but always for quickturn */
ckbyp:	beq	s1,HWSIM_PKG_ID,dobypass
	nop
	li	t0,0x20
	and	t0,t0,s0
	beqz	t0,ckvsim
	nop

dobypass:
	TRACE(0x415317)
	li	t3,1

	bgt	s3,t3,2f
	nop
	
	li	t0,0x00200020
	sw	t0,DMEMC_CONTROL25(a1)
	beq	s4,DMEMS_CORE_ID,1f
	nop
	sw	t0,DMEMC_CONTROL26(a1)

1:	lw	t0,DMEMC_CONTROL28(a1)
	li	t1,0x007f0000
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL28(a1)

2:	lw	t0,DMEMC_CONTROL29(a1)
	li	t1,~0x0000007f
	li	t2,0x0000005f
	and	t0,t0,t1
	or	t0,t0,t2
	sw	t0,DMEMC_CONTROL29(a1)

	lw	t0,DMEMC_CONTROL05(a1)
	li	t1,0x00000001
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL05(a1)

	ble	s3,t3,ckvsim
	nop
	
	lw	t0,DMEMC_CONTROL140(a1)
	li	t1,0x10000000
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL140(a1)

	lw	t0,DMEMC_CONTROL141(a1)
	or	t0,t0,t1
	sw	t0,DMEMC_CONTROL141(a1)

	/* For vsim change tinit so sims run faster */
ckvsim:	bne	s1,HDLSIM_PKG_ID,turnon
	nop

	TRACE(0x415318)
	li	t0,0x36
	sw	t0,DMEMC_CONTROL36(a1)

turnon:
	/* We are ready, turn controller on */
	TRACE(0x415319)
	lw	t0,DMEMC_CONTROL09(a1)		# Read current control09 reg
	or	t0,t0,DMC09_START		# Add start bit
	sw	t0,DMEMC_CONTROL09(a1)		# Start the controller

	li	t2,1
	ble	s3,t2,2f
	nop
1:	lw	t0,DMEMC_CONTROL133(a1)		# Poll for INT_INIT_DONE
	and	t1,t0,DM_INT_INIT_DONE
	beqz	t1,1b
	nop
	/* Bypass mode programming */
	lw	t0,DMEMC_CONTROL05(a1)
	li	t1,0x00000001
	and	t0,t0,t1
	beqz	t0,ack_ints
	nop
	lw	t1,DMEMC_CONTROL144(a1)
	srl	t1,t1,3
	lw	t2,DMEMC_CONTROL140(a1)
	li	t0,~0x03ff0000
	and	t2,t2,t0
	sll	t0,t1,16
	or	t2,t2,t0
	sw	t2,DMEMC_CONTROL140(a1)
	
	lw	t2,DMEMC_CONTROL142(a1)
	li	t0,~0x01ff8000
	and	t2,t2,t0
	sll	t0,t1,1
	add	t0,t0,t1
	li	t1,0x3ff
	and	t0,t0,t1
	sll	t0,t0,15
	or	t2,t2,t0
	sw	t2,DMEMC_CONTROL142(a1)
	
	lw	t1,DMEMC_CONTROL145(a1)
	srl	t1,t1,3
	lw	t2,DMEMC_CONTROL141(a1)
	li	t0,~0x03ff0000
	and	t2,t2,t0
	sll	t0,t1,16
	or	t2,t2,t0
	sw	t2,DMEMC_CONTROL141(a1)
	
	lw	t2,DMEMC_CONTROL143(a1)
	li	t0,~0x01ff8000
	and	t2,t2,t0
	sll	t0,t1,1
	add	t0,t0,t1
	li	t1,0x3ff
	and	t0,t0,t1
	sll	t0,t0,15
	or	t2,t2,t0
	sw	t2,DMEMC_CONTROL143(a1)
ack_ints:
	/* Clear any pending interrupts from dmemc */
	li	t1,DMC132_INTACK_MASK
	and	t0,t0,t1			# t0 = control133 & mask
	lw	t2,DMEMC_CONTROL132(a1)
	not	t1
	and	t2,t1,t2			# t2 = control132 & ~mask
	or	t0,t0,t2			# Or them and ...
	sw	t0,DMEMC_CONTROL132(a1)		# Ack all ints
	b	3f
	nop
2:	lw	t0,DMEMC_CONTROL24(a1)		# Poll for INT_INIT_DONE
	and	t1,t0,DM_INT_INIT_DONE
	beqz	t1,2b
	nop
	/* Clear any pending interrupts from dmemc */
	li	t1,DMC23_INTACK_MASK
	and	t0,t0,t1			# t0 = control24 & mask
	lw	t2,DMEMC_CONTROL23(a1)
	not	t1
	and	t2,t1,t2			# t2 = control23 & ~mask
	or	t0,t0,t2			# Or them and ...
	sw	t0,DMEMC_CONTROL23(a1)		# Ack all ints

3:	jr	t6
	li	v0,0

	/* Reset core using DMP regs at (a2) */
ai_core_reset:
	/* Set reset while enabling the clock */
	li	t9,(SICF_FGC | SICF_CLOCK_EN)
	li	t8,AIRC_RESET
	sw	t9,AI_IOCTRLSET(a2)
	sw	t8,AI_RESETCTRL(a2)

	/* Read back and delay */
	lw	t8,AI_RESETCTRL(a2)
	lw	t8,AI_RESETCTRL(a2)
	lw	t8,AI_RESETCTRL(a2)

	/* Clear reset */
	li	t8,0
	sw	t8,AI_RESETCTRL(a2)

	/* Read back and delay */
	lw	t8,AI_RESETCTRL(a2)
	lw	t8,AI_RESETCTRL(a2)
	lw	t8,AI_RESETCTRL(a2)

	/* Clear Force Gated Clock */
	li	t9,SICF_FGC
	sw	t9,AI_IOCTRLCLEAR(a2)

	/* Read back and delay */
	lw	t9,AI_IOCTRL(a2)
	lw	t9,AI_IOCTRL(a2)
	lw	t9,AI_IOCTRL(a2)

	jr	ra
	nop

	/* Use table at (a0) to init dmemc regs.
	 * Assumes (a1) points to the regs.
	 */
dmemc_init_regs:
	li	t3,128
	li	t0,1
	ble	s3,t0,loop_regs
	nop
	li	t3,256
	
loop_regs:
	add	a0,a0,s5			# Relocate address
	li	t0,DMEMC_TABLE_END
1:	lw	t1,0(a0)			# Pick up reg num
	bge	t1,t3,2f			# Return if the reg num >= num of supported regs
	nop
	beq	t1,t0,2f			# Return if done
	nop

	lw	t2,4(a0)			# Get reg value
	sll	t1,2				# Reg num * 4 is reg offset
	addu	t1,a1,t1
#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
	xor	t1,t1,4
#endif
	sw	t2,0(t1)			# Write reg
	b	1b
	addi	a0,8

2:	jr	ra
	nop

	END(ai_draminit)
