/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/flashfun.c#5 $
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: #5 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains FLASH-PROM functions
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include <stdio.h>
#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/flash.h"
#ifdef MRVL_UEFI
#include "unistd.h"
#endif /* MRVL_UEFI */

#if defined MSDOS && !defined DJGPP && !defined DO_PROTO
#include <dos.h>
#endif	/* MSDOS && !DJGPP && !DO_PROTO */

/* defines ******************************************************************/

#if defined DJGPP || !defined MSDOS
#define	FP_OFF(x)	(short *)(&(x))[0]
#define	FP_SEG(x)	(short *)(&(x))[1]

#define far
#define huge
#endif /* DJGPP || !MSDOS */

/*
 * commands for AMD Am28F010
 */
#define	FC_READ			0x00		/* read memory */
#define FC_AUTO			0x90		/* read auto select */
#define FC_SETUP_ERASE	0x20		/* setup erase */
#define FC_ERASE_VERIFY	0xa0		/* erase verify */
#define FC_SETUP_PROGRAM 0x40		/* setup program */
#define FC_PRGRAM_VERIFY 0xc0		/* program verify */
#define FC_RESET		0xff		/* reset state machine */

/*
 * definitions for Intel 28F001BX-B
 */

/*
 * status register definitions
 */
#define FP_WSMR			(1<<7)		/* wait state machine */
#define FP_ES			(1<<6)		/* erase suspended */
#define FP_EF			(1<<5)		/* erase failure */
#define FP_PF			(1<<4)		/* program failure */
#define FP_VPPL			(1<<3)		/* Vpp low detect */

#define FP_CM_ARRAY		0xff		/* read array command */
#define FP_CM_IDENT		0x90		/* intelligent identifier */
#define FP_CM_RSTATUS	0x70		/* read status register */
#define FP_CM_CSTATUS	0x50		/* clear status register */
#define FP_CM_ERASE		0x20		/* erase setup */
#define FP_CM_CONFIRM	0xd0		/* erase confirm */
#define FP_CM_SUSPEND	0xb0		/* erase suspend */
#define FP_CM_RESUME	0xd0		/* erase resume */
#define FP_CM_PROGRAM	0x40		/* program setup */

/*
 * definitions for AMD Am29F010
 */
#define F29_5555		0x5555		/* first address */
#define F29_AA			0xaa		/* first data */
#define F29_2AAA		0x2aaa		/* 2nd address */
#define F29_55			0x55		/* 2nd data */
#define F29_RESET		0xf0		/* read/reset */
#define F29_SELECT		0x90		/* autoselect */
#define F29_PROGRAM		0xa0		/* byte program */
#define F29_ERASE		0x80		/* erase */
#define F29_CERASE		0x10		/* chip erase */
#define F29_SERASE		0x30		/* sector erase */

#define	F29_SEC_SIZE	0x4000		/* sector size of the Am29F010 */

#define TIMEOUT_ERASE	200000
#define TIMEOUT_PROGRAM	30

#define TIMEOUT(IoC, x)	\
	for (timeout = (x); timeout && !(fl_in(pAC, IoC, 0) & FP_WSMR); timeout--)

extern void bank_select(SK_IOC IoC, long lp);
extern void vpp_on(SK_AC *pAC, SK_IOC IoC);
extern void vpp_off(SK_AC *pAC, SK_IOC IoC);
#if !defined(HASE) && !defined(MRVL_UEFI)
extern int  spl5();
extern void splx(int);
#endif /* !HASE && !MRVL_UEFI */

#ifdef FLASH_ACC_EXT

/*
 *	define	FLASH_ACC_EXT
 *
 *	If this define is set, the user of this module has to write his
 *	own flash access functions. Those functions must be named:
 *
 *	fl_out(SK_AC *pAC, SK_IOC IoC, long a, unsigned char x);
 *	- Write a byte into the flash without bank select
 *
 *	fl_in(SK_AC *pAC, SK_IOC IoC, long a);
 *	- Read a byte out of the flash without bank selecting
 *
 *	fl_set(SK_AC *pAC, SK_IOC IoC, long a, unsigned char x);
 *	- Write into the flash, but select the bank before
 *
 *	fl_get(SK_AC *pAC, SK_IOC IoC, long	a);
 *	- Read a byte out of the flash, but select the bank before
 */
extern unsigned char fl_out(SK_AC *pAC, SK_IOC IoC, long a, unsigned char x);
extern unsigned char fl_in(SK_AC *pAC, SK_IOC IoC, long a);
extern unsigned char fl_set(SK_AC *pAC, SK_IOC IoC, long a, unsigned char x);
extern unsigned char fl_get(SK_AC *pAC, SK_IOC IoC, long a);

#else	/* !FLASH_ACC_EXT */

/*
 * macros for ROM read/write
 */
#define fl_out(pAC, IoC, a, x)	(pAC->spi.fprom[a] = (x))
#define fl_in(pAC, IoC, a)		(pAC->spi.fprom[a])
/* fl_get is a static function */
#define fl_set(pAC, IoC, a, x)	(bank_select(IoC, (a) / pAC->spi.max_faddr), \
								pAC->spi.fprom[(a) % pAC->spi.max_faddr] = (x))

/*****************************************************************************
 *
 * fl_get - Gets a byte out of the flash
 *
 * Description:
 *	This function gets a byte out of the flash.
 *
 * Returns:
 *	byte read out of the flash
 */
static unsigned char fl_get(
SK_AC	*pAC,
SK_IOC	IoC,
long	addr)	/* address of the byte */
{
	bank_select(IoC, addr / pAC->spi.max_faddr);

	return (pAC->spi.fprom[addr % pAC->spi.max_faddr]);
}

#endif /* !FLASH_ACC_EXT */

/*
 * delay macros
 * WARNING: use braces after IF-statement
 */

#ifdef	FLASH_DELAY_EXT

/*
 * In the powerPC firmware environment we cannot use assembler statements
 */
#define DELAY6uS()	utl_sleep(6)
#define DELAY10uS()	utl_sleep(10)
#define DELAY10mS()	utl_sleep(10000)

#endif	/* FLASH_DELAY_EXT */

#ifdef	MSDOS

extern void fl_print(char *str, ...);
extern void fl_err(char *str, ...);

#ifndef DJGPP

/*
 * calibrate extension
 */
int	mul_loop_cnt = 1;

#define DELAY6uS()	delay(1,del_6us)
#define DELAY10uS()	delay(1,del_10us)
#define DELAY10mS()	delay(del_10ms_out,del_10ms_in)

#else	/* DJGPP */

#define DELAY6uS()	delay(1)
#define DELAY10uS()	delay(2)
#define DELAY10mS()	delay(10)

#endif	/* DJGPP */

#endif	/* MSDOS */

#ifdef Core

#define fl_print	pAC->Gui.WPrint
#define fl_err		pAC->Gui.WPrint

#define DELAY6uS()	SkDgDelayMs(6)
#define DELAY10uS()	SkDgDelayMs(10)
#define DELAY10mS()	SkDgDelayMs(10)

#endif	/* Core */

#ifdef VCPU

#define fl_print	c_print
#define fl_err		c_print

#define DELAY6uS()
#define DELAY10uS()
#define DELAY10mS()

#endif	/* VCPU */

#ifdef USE_DGLIB

extern void fl_print(char *msg, ...);
extern void fl_err(char *msg, ...);
extern int SkDgDelayMs(unsigned long ms);

#define DELAY6uS()	SkDgDelayMs(6)
#define DELAY10uS()	SkDgDelayMs(10)
#define DELAY10mS()	SkDgDelayMs(10)

#endif	/* USE_DGLIB */

#ifdef MRVL_UEFI
#define DELAY6uS()	usleep(6)
#define DELAY10uS()	usleep(10)
#define DELAY10mS()	usleep(10000)
#define fl_err w_print
extern void fl_print(char *str, ...);
#endif /* MRVL_UEFI */

/* typedefs ******************************************************************/

/* global variables **********************************************************/

/* local variables **********************************************************/

/*
 * command options
 */
int	verbose = 0;
int	wait_for_erase = 0;

#if defined(MSDOS) && !defined(DJGPP)
/*
 * timing parameters
 */
static unsigned int del_6us;					/* timing constant   6 usec */
static unsigned int del_10us;					/* timing constant  10 usec */
static unsigned int del_10ms_out, del_10ms_in;	/* timing constants 10 msec */
#endif	/* MSDOS && !DJGPP */

static struct {
	char	*man_name;
	int		man_code;
	char	*dev_name;
	int		dev_code;
	int		dev_id;
} dev_table[] = {
	{ "AMD",		0x01,	"Am29F010",		0x20,	FT_29		},
	{ "AMD",		0x01,	"Am29LV010B",	0x6e,	FT_29		},
	{ "AMD",		0x01,	"Am29LV040",	0x4f,	FT_29		},
	{ "AMD",		0x01,	"Am29F040",		0xa4,	FT_29		},
	{ "AMD",		0x01,	"Am28F010",		0xa7,	FT_AMD		},
	{ "Intel",		0x89,	"iN28F010",		0xb4,	FT_AMD		},
	{ "Intel",		0x89,	"iN28F001BX-T",	0x94,	FT_INTEL	},
	{ "Intel",		0x89,	"iN28F001BX-B",	0x95,	FT_INTEL	},
	{ "Catalyst",	0x31,	"CAT28F010",	0xb4,	FT_AMD		},
	{ "STM",		0x20,	"M29W010B90N1",	0x23,	FT_29		},
	{ "SST",		0xbf,	"SST39VF010",	0xd5,	FT_29		},
	{ "Atmel",		0x1f,	"AT49BV001N",	0x05,	FT_29		},
};

#define MAX_DEVS	(sizeof(dev_table) / sizeof(dev_table[0]))

/* function prototypes *******************************************************/

/* local functions ***********************************************************/

#if defined MSDOS && !defined DJGPP

/*****************************************************************************
 *
 * delay - Delay loop in assembler
 *
 * Description:
 *	ASM delay loop for calibration.
 *
 * Returns:
 *	Nothing
 */
static void	delay(
int	outer,	/* outer loop value */
int	inner)	/* inner loop value */
{
	_asm {
		mov	ax, outer
		mov	bx, inner
	oloop:
		mov	cx, bx
	iloop:
		push	ax
		push	dx
		push	cx

		mov	cx, mul_loop_cnt
		cmp	cx, 0
		jz	ml_end
	mloop:
		mul	ax
		loop	mloop
	ml_end:
		pop	cx
		pop	dx
		pop	ax
		loop	iloop
		dec	ax
		jne	oloop
	}
}

#endif /* MSDOS */

/*****************************************************************************
 *
 * apply_vpp - Turns on Vpp and waits
 *
 * Description:
 *	This function turns on Vpp and waits 10 msec
 *	(capacitor charge time is about 1 msec).
 *
 * Returns:
 *	Nothing
 */
static void apply_vpp(
SK_AC	*pAC,
SK_IOC	IoC)
{
	if (pAC->spi.fl_type == FT_29 ||
		pAC->spi.fl_type == FT_SPI ||
		pAC->spi.fl_type == FT_SPI_Y2)
		return;

	vpp_on(pAC, IoC);

	if (pAC->spi.vpp)
		return;

	DELAY10mS();
	pAC->spi.vpp = 1;
}

/*****************************************************************************
 *
 * off_vpp - Turns off Vpp and waits
 *
 * Description:
 *	This function turns off Vpp and wait 3 sec
 *	(capacitor discharge time is about 2 sec).
 *
 * Returns:
 *	Nothing
 */
static void off_vpp(
SK_AC	*pAC,
SK_IOC	IoC)
{
	int	i;

	if (pAC->spi.fl_type == FT_SPI || pAC->spi.fl_type == FT_SPI_Y2)
		return;

	bank_select(IoC, 0);

	if (pAC->spi.fl_type == FT_29)
		return;

	vpp_off(pAC, IoC);

	if (!pAC->spi.vpp)
		return;

	for (i = 0; i < 300; i++) {
		DELAY10mS();		/* 3 sec. */
	}
	pAC->spi.vpp = 0;
}

/*****************************************************************************
 *
 * amd_flash_erase - Erases AMD-type flashes
 *
 * Description:
 *	This function erases AMD-type flashes.
 *
 * Returns:
 *	0	O.K., chip erased
 *	1	error occured during erase
 */
static int amd_flash_erase(
SK_AC	*pAC,
SK_IOC	IoC)
{
	long	bank;
	long	addr;
	int		x;
	int		cycles;
	int		fl;

	fl = spl5();

	/* reset state machine */
	fl_out(pAC, IoC, 0, FC_RESET);
	fl_out(pAC, IoC, 0, FC_RESET);

	/* 1st cycle: setup erase */
	fl_out(pAC, IoC, 0, FC_SETUP_ERASE);
	/* 2nd cycle: erase */
	fl_out(pAC, IoC, 0, FC_SETUP_ERASE);

	DELAY10mS();

	/* verify erase */
	cycles = 1;

	for (bank = 0; bank < pAC->spi.max_pages; bank++) {
		bank_select(IoC, bank);
		for (addr = 0; addr < pAC->spi.max_faddr; ) {
			/* verify erase : address is memory address */
			fl_out(pAC, IoC, addr, FC_ERASE_VERIFY);
			DELAY6uS();
			if ((x = fl_in(pAC, IoC, addr)) != 0xff) {
				if (cycles++ >= 1000) {
					fl_out(pAC, IoC, 0, FC_READ);
					splx(fl);
					fl_err("E001: erase verify error at address %lx, is %x, "
						"should be %x\n", bank * pAC->spi.max_faddr + addr, x, 0xff);
					return (1);
				}
				/* 1st cycle: setup erase */
				fl_out(pAC, IoC, 0, FC_SETUP_ERASE);
				/* 2nd cycle: erase */
				fl_out(pAC, IoC, 0, FC_SETUP_ERASE);

				DELAY10mS();
			}
			else
				addr++;
		}
	}
	bank_select(IoC, 0);
	fl_out(pAC, IoC, 0, FC_READ);
	pAC->spi.erase_cycles += cycles;
	splx(fl);
	return (0);
}

/*****************************************************************************
 *
 * fprom_erase - Erases a block in the flash
 *
 * Description:
 *	This function erases a block in the flash.
 *
 * Returns:
 *	0	Everything O.K.
 *	pointer	to error message in case of an error
 */
static char *fprom_erase(
SK_AC	*pAC,
SK_IOC	IoC,
long	addr)
{
	int	status;
	long	timeout;

	fl_out(pAC, IoC, addr, FP_CM_ERASE);
	fl_out(pAC, IoC, addr, FP_CM_CONFIRM);
	TIMEOUT(IoC, TIMEOUT_ERASE);
	if (!timeout) {
		fl_out(pAC, IoC, 0, FP_CM_ARRAY);
		return ("WSMR ERASE timeout");
	}

	/* read status */
	fl_out(pAC, IoC, 0, FP_CM_RSTATUS);
	status = fl_in(pAC, IoC, 0);

	/* clear status */
	fl_out(pAC, IoC, 0, FP_CM_CSTATUS);
	if (status & (FP_EF | FP_VPPL)) {
		fl_out(pAC, IoC, 0, FP_CM_ARRAY);
		return ("Failure in Block Erase");
	}

	fl_out(pAC, IoC, 0, FP_CM_ARRAY);
	return (0);
}

/*****************************************************************************
 *
 * intel_flash_erase - Erases Intel-type flashes
 *
 * Description:
 *	This function erases Intel-type flashes.
 *
 * Returns:
 *	0	O.K., flash erased
 *	1	Error occured in block erase
 */
static int intel_flash_erase(
SK_AC	*pAC,
SK_IOC	IoC)
{
	int		i;
	char	*error;

	static struct {
		char	*name;
		long	start;
	} blocks[4] = {
		{ "Boot-Block",			0L		},
		{ "Parameter Block 1",	0x2000	},
		{ "Parameter Block 2",	0x3000	},
		{ "Main Block",			0x3000	},
	};
	for (i = 0; i < 4; i++) {
		bank_select(IoC, blocks[i].start / pAC->spi.max_faddr);
		error = fprom_erase(pAC, IoC, blocks[i].start % pAC->spi.max_faddr);
		if (error) {
			fl_err("error in block erase : %s\n", blocks[i].name);
			return (1);
		}
	}

	bank_select(IoC, 0);
	return (0);
}

/*****************************************************************************
 *
 * fl_29 - Sends a command to a type F29 flash
 *
 * Description:
 *	This function sends a command to a type F29 flash.
 *
 * Returns:
 *	Nothing
 */
static void	fl_29(
SK_AC	*pAC,
SK_IOC	IoC,
int		c)	/* command that shall be sent to the flash */
{
	fl_set(pAC, IoC, F29_5555, F29_AA);

	fl_set(pAC, IoC, F29_2AAA, F29_55);

	fl_set(pAC, IoC, F29_5555, (unsigned char)c);

	bank_select(IoC, 0);
}

/*****************************************************************************
 *
 * f29_flash_erase - Erases a type F29 flash
 *
 * Description:
 *	This function erases a type F29 flash. To be sure that only
 *	the specified sectors are erased, a sector count is computed
 *	out of pAC->spi.max_pages and pAC->spi.max_faddr
 *
 * Returns:
 *	0	Everything O.K.
 *	1	Cannot erase the chip
 */
static int f29_flash_erase(
SK_AC	*pAC,
SK_IOC	IoC)
{
	long	timeout;
	int	i;
	int	sec_count;

	if (verbose) {
		fl_print("erase FLASH ");
	}
	/* First compute the number of sectors to be erased */
	/* Note: max_faddr doesn't need to be equal to F29_SEC_SIZE */
	sec_count = (int) ((pAC->spi.max_pages * pAC->spi.max_faddr / F29_SEC_SIZE) & 0xffff);

	/*
	 * bug fix: write erase preamble once, than just give
	 * the sector addresses and the Sector Erase command (MA)
	 */
	fl_29(pAC, IoC, F29_ERASE);
	fl_set(pAC, IoC, F29_5555, F29_AA);
	fl_set(pAC, IoC, F29_2AAA, F29_55);

	/* Erase all sectors one by one */
	for (i = 0; i < sec_count; i++) {
		/* erase concurrently all sectors that are not protected */
		fl_set(pAC, IoC, (long) i * F29_SEC_SIZE, F29_SERASE);
	}

	/* The timeout need only be used after the last Sector erase */

	/*
	 * changed because of timeout problems in Genesis production
	 *
	for (timeout = 5000000; timeout && !(fl_in(pAC, IoC, 0) & 0x80); timeout--)
	 */
	i = 0;
	for (timeout = 20000000;;timeout--) {
		if ((fl_in(pAC, IoC, 0) & 0x80) != 0) {
			break;
		}
		if (timeout == 0) {
			if (wait_for_erase) {
				/* restart and wait for unlimited time */
				timeout = 20000000;
			}
			else {
				break;
			}
		}
		if ((timeout % 100000) == 0) {
			if (verbose) {
				fl_print(".");
			}
		}
	}
	if (!timeout) {
		fl_err("\nerror in chip erase (timeout)\n");
		return (1);
	}

	if (verbose) {
		fl_print(" finished\n");
	}
	return (0);
}

/*****************************************************************************
 *
 * flash_erase - Erases flash
 *
 * Description:
 *	This function is the main flash erase routine (called by erase_flash)
 *	It erases flashes using the appropriate flash erase algorithm.
 *
 * Returns:
 *	error code
 */
static int flash_erase(
SK_AC	*pAC,
SK_IOC	IoC)
{
#ifdef SK_DIAG
	int Rtv;
#endif

	switch (pAC->spi.fl_type) {
	case FT_AMD:
		return (amd_flash_erase(pAC, IoC));
	case FT_INTEL:
		return (intel_flash_erase(pAC, IoC));
	case FT_29:
		return (f29_flash_erase(pAC, IoC));
	case FT_SPI:
	case FT_SPI_Y2:
#ifndef SK_DIAG
		return (spi_flash_erase(pAC, IoC, 0, (unsigned long)
			pAC->spi.max_pages * pAC->spi.max_faddr));
#else	/* SK_DIAG */
		if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_EX &&
			pAC->spi.max_faddr == 0x1000) {
			Rtv = spi_flash_erase(pAC, IoC, 0, SPI_PIG_OFF);
			if (Rtv == 0 &&
				(pAC->spi.max_pages * pAC->spi.max_faddr) - SPI_CHIP_SIZE > 0) {
				return (spi_flash_erase(pAC, IoC, SPI_CHIP_SIZE, (unsigned long)
					(pAC->spi.max_pages * pAC->spi.max_faddr) - SPI_CHIP_SIZE));
			}
			return (Rtv);
		}
		else {
			return (spi_flash_erase(pAC, IoC, 0, (unsigned long)
				pAC->spi.max_pages * pAC->spi.max_faddr));
		}
#endif	/* SK_DIAG */
		break;
	}
	return (1);
}

/*****************************************************************************
 *
 * normal_flash_verify - Verifies the contents of the flash
 *
 * Description:
 *	This function compares the contents of the flash with the Data.
 *
 * Returns:
 *	0	Verify O.K.
 *	1	Verify error
 */
static int normal_flash_verify(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)	/* pointer to the DATA that shall be verified */
{
	long	bank;
	long	addr;
	int		x;
	int		byte;

#ifdef	FLASH_VPP_READ
	/*
	 * read access to flash
	 * only with vpp on
	 */
	apply_vpp(pAC, IoC);

	DELAY10mS();
	fl_out(pAC, IoC, 0, FC_RESET);
	DELAY10mS();
#endif

	for (bank = 0; bank < pAC->spi.max_pages; bank++) {
		bank_select(IoC, bank);
		for (addr = 0; addr < pAC->spi.max_faddr; addr++,data++) {
			byte = data ? *data : 0;
			if ((x = fl_in(pAC, IoC, addr)) != byte) {
				fl_err("*** data verify error at address %lx, is %x, "
					"should be %x\n", bank * pAC->spi.max_faddr + addr, x, byte);
#ifdef	FLASH_VPP_READ
				off_vpp(pAC, IoC);
#endif
				return (1);
			}
		}
	}
	bank_select(IoC, 0);

#ifdef	FLASH_VPP_READ
	off_vpp(pAC, IoC);
#endif
	return (0);
}

/*****************************************************************************
 *
 * flash_verify - Verifies flash contents
 *
 * Description:
 *	This function compares the flash contents with the Data.
 *
 * Returns:
 *	0	Verify O.K.
 *	1	Verify error
 */
int	flash_verify(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)
{
	if (pAC->spi.fl_type == FT_SPI || pAC->spi.fl_type == FT_SPI_Y2) {
		return (spi_flash_manage(pAC, IoC, data, 0, (unsigned long)
			pAC->spi.max_pages * pAC->spi.max_faddr, SPI_VERIFY));
	}

	return (normal_flash_verify(pAC, IoC, data));
}

/*****************************************************************************
 *
 * fprom_program - Programs a byte
 *
 * Description:
 *	This function programs a byte into a ???-type flash.
 *
 * Returns:
 *	0	byte successfully programmed
 *	pointer	to error message otherwise
 */
static char *fprom_program(
SK_AC	*pAC,
SK_IOC	IoC,
long	addr,	/* address where byte shall be programmed */
char	data)	/* character to be programmed */
{
	int		status;
	long	timeout;

	fl_out(pAC, IoC, addr, FP_CM_PROGRAM);
	fl_out(pAC, IoC, addr, data);

	TIMEOUT(IoC, TIMEOUT_PROGRAM);

	if (!timeout) {
		fl_out(pAC, IoC, 0, FP_CM_ARRAY);
		return ("WSMR PROGRAM timeout");
	}

	/* read status */
	fl_out(pAC, IoC, 0, FP_CM_RSTATUS);
	status = fl_in(pAC, IoC, 0);

	/* clear status */
	fl_out(pAC, IoC, 0, FP_CM_CSTATUS);
	if (status & (FP_PF | FP_VPPL)) {
		fl_out(pAC, IoC, 0, FP_CM_ARRAY);
		return ("Failure in Byte Program");
	}
	fl_out(pAC, IoC, 0, FP_CM_ARRAY);

	return (0);
}

/*****************************************************************************
 *
 * amd_flash_program - Programs AMD-type flashes
 *
 * Description:
 *	This function programs an AMD-type flash.
 *	If data == NULL, program all zeros.
 *
 * NOTE
 *	The length is given by pAC->spi.max_pages * pAC->spi.max_faddr.
 *
 * Returns:
 *	0	successfully programmed
 *	1	error occured
 */
static int amd_flash_program(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)	/* pointer to the data that shall be programmed */
{
	long	bank;
	long	addr;
	int		cycles;
	int		x;
	int		byte;
	int		max;
	int		retries;
	int		fl;

	fl = spl5();

	/* reset state machine */
	fl_out(pAC, IoC, 0, FC_RESET);
	fl_out(pAC, IoC, 0, FC_RESET);

	/* program and verify loop */
	cycles = 0;
	retries = 0;
	max = 0;
	for (bank = 0; bank < pAC->spi.max_pages; bank++) {
		bank_select(IoC, bank);
		for (addr = 0; addr < pAC->spi.max_faddr; ) {
			byte = data ? *data : 0;
			fl_out(pAC, IoC, 0, FC_SETUP_PROGRAM);
			/* program data: address is memory address */
			fl_out(pAC, IoC, addr, (char)byte);
			DELAY10uS();
			fl_out(pAC, IoC, 0, FC_PRGRAM_VERIFY);
			DELAY6uS();
			if ((x = fl_in(pAC, IoC, addr)) != byte) {
				if (cycles++ >= 25) {
					fl_out(pAC, IoC, 0, FC_READ);
					splx(fl);
					fl_err("*** program verify error at address %lx, is %x, "
						"should be %x\n", bank * pAC->spi.max_faddr + addr, x, byte);
					return (1);
				}
			}
			else {
				addr++;
				if (data)		/* incr. only if !0 */
					data++;
				retries += cycles;
				if (cycles > max)
					max = cycles;
				cycles = 0;
			}
		}
	}
	bank_select(IoC, 0);
	fl_out(pAC, IoC, 0, FC_READ);	/* clear command */
	pAC->spi.prog_retries += retries;
	if (max > pAC->spi.max_retries)
		pAC->spi.max_retries = max;
	splx(fl);
	return (0);
}

/*****************************************************************************
 *
 * intel_flash_program - Programs Intel-type flashes
 *
 * Description:
 *	This function programs an Intel-type flash.
 *
 * NOTE
 *	The length is given by pAC->spi.max_pages * pAC->spi.max_faddr.
 *
 * Returns:
 *	0	successfully programmed
 *	1	error occured
 */
static int intel_flash_program(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)	/* pointer to the data that shall be programmed */
{
	long	bank;
	long	addr;
	char	*error;

	for (bank = 0; bank < pAC->spi.max_pages; bank++) {
		bank_select(IoC, bank);
		for (addr = 0; addr < pAC->spi.max_faddr; addr++) {
			error = fprom_program(pAC, IoC, addr, *data);
			if (error) {
				fl_err("*** program verify error at address %lx, is %x, "
					"should be %x\n", bank * pAC->spi.max_faddr + addr,
					fl_in(pAC, IoC, addr), *data);
				return (1);
			}
			data++;
		}
	}
	bank_select(IoC, 0);
	return (0);
}

/*****************************************************************************
 *
 * f29_program - Programs a byte into F29-type flashes
 *
 * Description:
 *	This function programs a byte into an F29-type flash.
 *
 * Returns:
 *	0	successfully programmed
 *	pointer	to error message in case of an error
 */
static char *f29_program(
SK_AC			*pAC,
SK_IOC			IoC,
long 			a,	/* address where the byte shall be written */
unsigned char	c)	/* byte that shall be written */
{
	long	addr;
	long	timeout;

	addr	= a % pAC->spi.max_faddr;

	fl_29(pAC, IoC, F29_RESET);
	fl_29(pAC, IoC, F29_PROGRAM);
	fl_set(pAC, IoC, a, c);
	for (timeout = 500000; timeout && (fl_in(pAC, IoC, addr) != c); timeout--)
		;
	return (timeout ? 0 : "timeout during byte programming");
}

/*****************************************************************************
 *
 * f29_flash_program - Programs F29-type flashes
 *
 * Description:
 *	This function programs an F29-type flash.
 *
 * NOTE
 *	The length is given by pAC->spi.max_pages * pAC->spi.max_faddr.
 *
 * Returns:
 *	0	successfully programmed
 *	1	error occured
 */
static int f29_flash_program(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)	/* pointer to the data that shall be programmed */
{
	long	i;
	char	*error;
	long	end;

	end = pAC->spi.max_pages * pAC->spi.max_faddr;

	for (i = 0; i < end; i++) {

		error = f29_program(pAC, IoC, i, *data);

		if (error) {
			fl_err("*** program verify error at address %lx, is %x, "
				"should be %x\n", i, fl_in(pAC, IoC,
				(int) (i % pAC->spi.max_faddr)), *data);

			fl_29(pAC, IoC, F29_RESET);
			return (1);
		}
		data++;
	}
	fl_29(pAC, IoC, F29_RESET);
	return (0);
}

/*****************************************************************************
 *
 * flash_program - Programs a flash
 *
 * Description:
 *	This function programs an F29-type flash.
 *	If data == NULL
 *		If flash type is AMD, program all zeros
 *		else do nothing.
 *
 * NOTE
 *	The length is given by pAC->spi.max_pages * pAC->spi.max_faddr.
 *
 * Returns:
 *	0	successfully programmed
 *	1	error occured
 */
static int flash_program(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data)	/* pointer to the data that shall be programmed */
{
	switch (pAC->spi.fl_type) {
	case FT_AMD:
		return (amd_flash_program(pAC, IoC, data));
	case FT_INTEL:
		if (data == NULL) {
			return (0);
		}
		return (intel_flash_program(pAC, IoC, data));
	case FT_29:
		if (data == NULL) {
			return (0);
		}
		return (f29_flash_program(pAC, IoC, data));
	case FT_SPI:
	case FT_SPI_Y2:
		if (data == NULL) {
			return (0);
		}
		return (spi_flash_manage(pAC, IoC, data, 0, (unsigned long)
			pAC->spi.max_pages * pAC->spi.max_faddr, SPI_WRITE));
	}
	return (1);
}

/*****************************************************************************
 *
 * flash_check_f29 - Checks whether a flash is of type F29
 *
 * Description:
 *	This function checks whether a flash is of type F29.
 *	If so it sets pAC->spi.fl_type.
 *
 * Returns:
 *	Nothing
 */
static void flash_check_f29(
SK_AC	*pAC,
SK_IOC	IoC,
int		*pManId,
int		*pDevId)
{
	unsigned	i;
	int			fl_man;		/* flash manufacturer */
	int			fl_dev;		/* flash device ID */

	/* set-up for Am29 */
	fl_29(pAC, IoC, F29_SELECT);
	fl_man = fl_in(pAC, IoC, 0);	/* read manufacturer code */
	fl_dev = fl_get(pAC, IoC, 1L);	/* read device code */
	fl_29(pAC, IoC, F29_RESET);

#ifdef SK_DIAG
	c_print("\nFLASH found: man=%02X, dev=%02X\n",
		fl_man, fl_dev);
#endif /* SK_DIAG */

	for (i = 0; i < MAX_DEVS; i++) {
		if (dev_table[i].dev_id == FT_29 &&
			dev_table[i].man_code == fl_man &&
			dev_table[i].dev_code == fl_dev) {

			pAC->spi.fl_type = FT_29;
			if (pManId != NULL) {
				*pManId = fl_man;
			}
			if (pDevId != NULL) {
				*pDevId = fl_dev;
			}
			break;
		}
	}
}

/*****************************************************************************
 *
 * flash_man_code - Reads device and manufacturer code from flash
 *
 * Description:
 *	This function reads device and manufacturer code from flash.
 *
 * Returns:
 *	0	Device found and pAC->spi.fl_type set correctly
 *	1	Device could not be identified or found
 */
int	flash_man_code(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned	i;
	int			fl_man;		/* flash manufacturer */
	int			fl_dev;		/* flash device ID */

	if (pAC->spi.fl_type == FT_SPI || pAC->spi.fl_type == FT_SPI_Y2) {
		return (0);
	}

	/* reset state machine */
	bank_select(IoC, 0);

	/* check for Am29 */
	flash_check_f29(pAC, IoC, &fl_man, &fl_dev);

	if (pAC->spi.fl_type != FT_29) {

		apply_vpp(pAC, IoC);

		if (pAC->spi.fl_type != FT_INTEL) {
			fl_out(pAC, IoC, 0, FC_RESET);
			fl_out(pAC, IoC, 0, FC_RESET);
		}

		/* command auto-select */
		fl_out(pAC, IoC, 0, (char) (pAC->spi.fl_type == FT_INTEL ? FP_CM_IDENT : FC_AUTO));
		fl_man = fl_in(pAC, IoC, 0);	/* read manufacturer code */
		fl_dev = fl_get(pAC, IoC, 1L);	/* read device code */
		/* clear command */
		fl_out(pAC, IoC, 0, (char) (pAC->spi.fl_type == FT_INTEL ? FP_CM_ARRAY : FC_READ));

		off_vpp(pAC, IoC);
	}

	for (i = 0; i < MAX_DEVS; i++) {
		if (fl_man == dev_table[i].man_code &&
			fl_dev == dev_table[i].dev_code) {
			break;
		}
	}

	if (i == MAX_DEVS) {
		fl_err("E002: UNKNOWN device: manufacturer '%02X' device '%02X'\n",
			fl_man, fl_dev);
		return (1);
	}

	fl_print("FLASH Device: '%s' (%02X) '%s' (%02X)\n",
		dev_table[i].man_name, fl_man, dev_table[i].dev_name, fl_dev);

	return (0);
}

#if defined(MSDOS) && !defined(DJGPP)

/*****************************************************************************
 *
 * tsync - Waits for timer to advance
 *
 * Description:
 *	This function waits for the OS timer to advance.
 *
 * Returns:
 *	Nothing
 */
static void tsync(
SK_AC	*pAC)
{
	unsigned long	time;

	time = (unsigned long) SkOsGetTime(pAC);
	while ((unsigned long) SkOsGetTime(pAC) == time)
		;
}

#endif	/* MSDOS && !DJGPP */

/*****************************************************************************
 *
 * calibrate - Calibrates timing parameters
 *
 * Description:
 *	This function calibrates timing parameters:
 *		wait 1 clock tick and count loop delay
 *		calculate nsecs per loop
 *		calculate delay parameters for 10 usec loop
 *		verify parameters for 1 second
 *
 * Returns:
 *	0	O.K.
 *	1	error occured (can't calibrate clock)
 */
int calibrate(
SK_AC	*pAC)
{
#if defined(MSDOS) && !defined(DJGPP)
	unsigned long	start;
	unsigned long	ns;
	unsigned		i;
	unsigned		n;
	unsigned		us10;
	unsigned		ticks;

	if (pAC->spi.fl_type == FT_29) {
		return (0);
	}

	fl_print("Calibrating Timer ...\n");

	/* try to calibrate 3 times */
	for (n = 0; n < 3; n++) {
		tsync(pAC);
		start = (unsigned long) SkOsGetTime(pAC);
		for (i = 0; start == (unsigned long) SkOsGetTime(pAC); i++) {
			delay(1, 1000);
		}
		ns = 54945054;		/* 1/18.2 */
		ns /= i * 1000L;
		us10 = (unsigned) (20000/ns);	/* 10 min 25 max -> 20 */

		tsync(pAC);
		start = (unsigned long) SkOsGetTime(pAC);
		for (i = 0; i < 10; i++) {
			delay((int) 5000, us10);	/* 100 mS */
		}

		ticks = (unsigned) ((unsigned long) SkOsGetTime(pAC) - start);
		if (verbose) {
			fl_print("us10 = %u, ticks = %u\n",us10,ticks);
		}
		if (ticks >= 17 && ticks <= 24) {
			break;	/* complete calibration successfully */
		}
		if (mul_loop_cnt == 0) {
			break;	/* terminate, calibration impossible */
		}
		if (ticks > 24) {
			mul_loop_cnt = 0;
		}
		if (ticks < 17) {
			mul_loop_cnt += 50;
		}
	}

	if (ticks < 17 || ticks > 24) {
		fl_err("E003: can't calibrate clock, ticks = %u, us10 = %u\n",
			ticks, us10);
		return (1);
	}
	del_10us = us10;
	del_6us = (unsigned) (12000 / ns);			/* 6 min no max -> 12 */
	del_10ms_in = (unsigned) (100000L / ns);	/* 100 uS */
	del_10ms_out = 100;							/* 100 *100uS = 10mS */
	DELAY10uS();
#endif	/* MSDOS && !DJGPP */
	return (0);
}

/*****************************************************************************
 *
 * init_flash - Initializes flash parameter values
 *
 * Description:
 *	This function initializes flash parameter values.
 *
 * Returns:
 *	Nothing
 */
void init_flash(
SK_AC		*pAC,
SK_IOC		IoC,
unsigned	rombase,	/* base address of memory access to the flash */
long		pages,		/* number of pages that can be selected */
long		faddr)		/* page size (0..faddr-1) are directly */
						/* addressable in rombase-window */
{
	unsigned long flash_size;

#if (defined MSDOS) && (!defined linux)
	FP_SEG(pAC->spi.fprom) = rombase;
	FP_OFF(pAC->spi.fprom) = 0;
#endif /* MSDOS */

	pAC->spi.max_pages = pages;
	pAC->spi.max_faddr = faddr;

	flash_check_spi(pAC, IoC, &flash_size);
	if (pAC->spi.fl_type == FT_SPI || pAC->spi.fl_type == FT_SPI_Y2) {
		return;
	}

	flash_check_f29(pAC, IoC, NULL, NULL);
}

/*****************************************************************************
 *
 * erase_flash - Erases the whole flash contents
 *
 * Description:
 *	This function erases the whole flash contents.
 *
 * Returns:
 *	0	Flash is successfully erased
 *	10	Flash cannot be programmed with all zeros before erasing
 *	11	Flash cannot be erased
 */
int	erase_flash(
SK_AC	*pAC,
SK_IOC	IoC)
{
	apply_vpp(pAC, IoC);

	if (flash_program(pAC, IoC, NULL)) {
		off_vpp(pAC, IoC);
		return (10);
	}

	if (flash_erase(pAC, IoC)) {
		off_vpp(pAC, IoC);
		return (11);
	}
	off_vpp(pAC, IoC);

	return (0);
}

/*****************************************************************************
 *
 * program_flash - Programs the contents of the flash
 *
 * Description:
 *	This function programs the contents of the flash.
 *
 * Returns:
 *	0	Successfully programmed
 *	6	Flash cannot be programmed with all zeros
 *	7	Flash cannot be erased before programming
 *	8	Flash cannot be programmed
 */
int	program_flash(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*prom_data)	/* pointer to the programming area */
{
	apply_vpp(pAC, IoC);

	if (flash_program(pAC, IoC, NULL)) {
		off_vpp(pAC, IoC);
		return (6);
	}
	if (flash_erase(pAC, IoC)) {
		off_vpp(pAC, IoC);
		return (7);
	}

	fl_print("\nStarting FLASH programming ");

	if (flash_program(pAC, IoC, prom_data)) {
		off_vpp(pAC, IoC);
		return (8);
	}
	off_vpp(pAC, IoC);
	return (0);
}

/*****************************************************************************
 *
 * read_from_flash - Reads contents of the flash
 *
 * Description:
 *	This function reads contents of the flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int	read_from_flash(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* pointer to the data buffer */
unsigned long	offs,	/* start offset to read from */
unsigned long	size)	/* number of bytes to read */
{
	unsigned long	i;

	if (pAC->spi.fl_type == FT_SPI || pAC->spi.fl_type == FT_SPI_Y2) {
		return (spi_flash_manage(pAC, IoC, data, offs, size, SPI_READ));
	}

	for (i = offs; i < offs + size; i++) {
		*data = fl_get(pAC, IoC, i);
		data++;
	}
	return (0);
}

/*****************************************************************************
 *
 * bank_protected - Determines whether a particualar sector is protected
 *
 * Description:
 *	This function determines whether a particualar sector is protected.
 *
 * Returns:
 *	0	Unprotected or unknown protection algorithm
 *	1	Protected sector
 */
int	bank_protected(
SK_AC	*pAC,
SK_IOC	IoC,
long	addr)	/* address in the sector to be analyzed */
{
	int	protection;

	if (pAC->spi.fl_type != FT_29) {
		/* unknown detection algorithm for that type */
		/* -> default: this sector is not protected */
		return (0);
	}

	/* Autoselect function */
	fl_29(pAC, IoC, F29_SELECT);

	protection = fl_get(pAC, IoC, addr / F29_SEC_SIZE * F29_SEC_SIZE + 2L);
	/* protection now: 00: unprotected, 01: protected sector */

	/* Reset the Flash now! */
	fl_29(pAC, IoC, F29_RESET);

	return (protection);
}

/* End of File */
