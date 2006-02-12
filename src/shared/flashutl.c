/*
 * flashutl.c - Flash Read/write/Erase routines
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <osl.h>

#define DECLARE_FLASHES
#include <sbutils.h>
#include <sbconfig.h>
#include <flash.h>
#include <sflash.h>
#include <flashutl.h>
#include <bcmnvram.h>
#include <bcmutils.h>

#define DPRINT(x) 

#define ERR2	0x30
#define DONE	0x80
#define WBUFSIZE 32
#define FLASH_TRIES 4000000 /* retry count */
#define CMD_ADDR ((unsigned long)0xFFFFFFFF)

/* 'which' param for block() */
#define BLOCK_BASE	0
#define BLOCK_LIM	1

#define FLASH_ADDR(off) ((unsigned long)flashutl_base + (off))

static chipcregs_t *cc;

/* Global vars */
char*		flashutl_base	= NULL;
flash_desc_t*	flashutl_desc	= NULL;
flash_cmds_t*	flashutl_cmd	= NULL;

static void		scmd(uint16 cmd, unsigned long off);
static void		cmd(uint16 cmd, unsigned long off);
static void		flash_reset(void);
static int		flash_poll(unsigned long off, uint16 data);
static unsigned long	block(unsigned long addr, int which);
static int	flash_erase(void);
static int	flash_eraseblk(unsigned long off);
static int	flash_write(unsigned long off, uint16 *src, uint nbytes);
static unsigned long	flash_block_base(unsigned long off);
static unsigned long	flash_block_lim(unsigned long off);
static chipcregs_t *cc;

/* Read the flash ID and set the globals */
int
sysFlashInit(char *flash_str)
{
	uint32 fltype = PFLASH;
	uint16 flash_vendid = 0;
	uint16 flash_devid = 0;
	uint16* flash = (uint16*)0xbfc00000;
	int idx;
	struct sflash *sflash;
	sb_t *sbh;

	/*
	 * Check for serial flash.
	 */
	sbh = sb_kattach();
	ASSERT(sbh);
	cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0);

	if (cc) {
		flash = (uint16*)0xbc000000;
		fltype = R_REG(&cc->capabilities) & CAP_FLASH_MASK;
		/* Select SFLASH ? */
		if (fltype == SFLASH_ST || fltype == SFLASH_AT) {
			sflash = sflash_init(cc);
			flashutl_cmd = &sflash_cmd_t;
			flashutl_desc = &sflash_desc;
			flashutl_desc->size = sflash->size;
			if (flash_str) 
				sprintf(flash_str, "SFLASH %d kB", sflash->size/1024);
			return(0);
		}
	}

	flashutl_base = (uint8*)flash;

	/* 
	 * Parallel flash support
	 *  Some flashes have different unlock addresses, try each it turn
	 */
	idx = sizeof(flash_cmds)/sizeof(flash_cmds_t) - 2;
	flashutl_cmd = &flash_cmds[idx--];
	while((fltype == PFLASH) && flashutl_cmd->type) {

		if (flashutl_cmd->read_id)
			cmd(flashutl_cmd->read_id, CMD_ADDR);

#ifdef MIPSEB
		flash_vendid = *(flash + 1);
		flash_devid = *flash;	
#else
		flash_vendid = *flash;
		flash_devid = *(flash + 1);
#endif

		/* Funky AMD */
		if ((flash_vendid == 1) && (flash_devid == 0x227e)) {
			/* Get real devid */
#ifdef MIPSEB
			flash_devid = *(flash+0xe);	
#else
			flash_devid = *(flash+0xf);
#endif
		}

		flashutl_desc = flashes;
		while (flashutl_desc->mfgid != 0 &&
			   !(flashutl_desc->mfgid == flash_vendid &&
			 flashutl_desc->devid == flash_devid)) {
			flashutl_desc++;
		}
		if (flashutl_desc->mfgid != 0)
			break;

		flashutl_cmd = &flash_cmds[idx--];
	}

	if (flashutl_desc->mfgid == 0) {
		flashutl_desc = NULL;
		flashutl_cmd = NULL;
	} else {
		flashutl_cmd = flash_cmds;
		while (flashutl_cmd->type != 0 && flashutl_cmd->type != flashutl_desc->type)
			flashutl_cmd++;
		if (flashutl_cmd->type == 0)
			flashutl_cmd = NULL;
	}

	if (flashutl_cmd != NULL) {
		flash_reset();
	}

	if (flashutl_desc == NULL) {
		if (flash_str)
			sprintf(flash_str, "UNKNOWN 0x%x 0x%x", flash_vendid, flash_devid);
		DPRINT(("Flash type UNKNOWN\n"));
		return 1;
	}
	
	if (flash_str)
		strcpy(flash_str, flashutl_desc->desc);
	DPRINT(("Flash type \"%s\"\n", flashutl_desc->desc));

	return 0;
}

static int
flash_erase()
{
	unsigned long size = flashutl_desc->size;
	unsigned long addr;
	int err = 0;
	
	for (addr = 0; addr < size; addr = block(addr, BLOCK_LIM)) {
		err = flash_eraseblk(addr);
		if (err) break;
	}
	
	return err;
}

static int
flash_eraseblk(unsigned long addr)
{
	unsigned long a;
	uint16 st;

	a = (unsigned long)addr;
	if (a >= flashutl_desc->size)
		return 1;
	
	a = block(a, BLOCK_BASE);

	/* Ensure blocks are unlocked (for intel chips)*/ 
	if (flashutl_cmd->type == BSC) {
		scmd((unsigned char)INTEL_UNLOCK1, a);
		scmd((unsigned char)INTEL_UNLOCK2, a);
	}

	if (flashutl_cmd->pre_erase)
		cmd(flashutl_cmd->pre_erase, CMD_ADDR);
	if (flashutl_cmd->erase_block)
		cmd(flashutl_cmd->erase_block, a);
	if (flashutl_cmd->confirm)
		scmd(flashutl_cmd->confirm, a);

	st = flash_poll(a, 0xffff);
	
	flash_reset();

	if (st) {
		DPRINT(("Erase of block 0x%08lx-0x%08lx failed\n",
			a, block((unsigned long)addr, BLOCK_LIM)));
		return st;
	}

	DPRINT(("Erase of block 0x%08lx-0x%08lx done", a, block((unsigned long)addr, BLOCK_LIM)));

	return 0;
}

static int
flash_write(unsigned long off, uint16 *src, uint nbytes)
{
	uint16* dest;
	uint16 st, data;
	uint i, len;

	ASSERT(flashutl_desc != NULL);

	if (off >= flashutl_desc->size)
		return 1;

	dest = (uint16*)FLASH_ADDR(off);
	st = 0;

	while (nbytes) {
		if ((flashutl_desc->type == SCS) &&
		    flashutl_cmd->write_buf &&
		    ((off & (WBUFSIZE - 1)) == 0)) {
			/* issue write command */
			if (flashutl_cmd->write_buf)
				cmd(flashutl_cmd->write_buf, off);
			if ((st = flash_poll(off, DONE)))
				continue;

			len = MIN(nbytes, WBUFSIZE);

#ifndef MIPSEB
			/* write (length - 1) */
			cmd((len / 2) - 1, off);

			/* write data */
			for (i = 0; i < len; i += 2, dest++, src++)
				*dest = *src;
#else
			/* 
			 * BCM4710 endianness is word consistent but
			 * byte/short scrambled. This write buffer
			 * mechanism appears to be sensitive to the
			 * order of the addresses hence we need to
			 * unscramble them. We may also need to pad
			 * the source with two bytes of 0xffff in case
			 * an odd number of shorts are presented.
			 */

			/* write (padded length - 1) */
			cmd((ROUNDUP(len, 4) / 2) - 1, off);

			/* write data (plus pad if necessary) */
			for (i = 0; i < ROUNDUP(len, 4); i += 4, dest += 2, src += 2) {
				*(dest + 1) = ((i + 2) < len) ? *(src + 1) : 0xffff;
				*dest = *src;
			}
#endif

			/* write confirm */
			if (flashutl_cmd->confirm)
				cmd(flashutl_cmd->confirm, off);

			if ((st = flash_poll(off, DONE)))
				break;
		} else {
			/* issue write command */
			if (flashutl_cmd->write_word)
				cmd(flashutl_cmd->write_word, CMD_ADDR);

			/* write data */
			len = MIN(nbytes, 2);
			data = *src++;
			*dest++ = data;

			/* poll for done */
			if ((st = flash_poll(off, data)))
				break;
		}

		nbytes -= len;
		off += len;
	}

	flash_reset();

	return st;
}

static unsigned long
flash_block_base(unsigned long off)
{
	return block(off, BLOCK_BASE);
}

static unsigned long
flash_block_lim(unsigned long off)
{
	return block(off, BLOCK_LIM);
}

/* Writes a single command to the flash. */
static void
scmd(uint16 cmd, unsigned long off)
{
	ASSERT(flashutl_base != NULL);
	
	/*  cmd |= cmd << 8; */

	*(uint16*)(flashutl_base + off) = cmd;
}

/* Writes a command to flash, performing an unlock if needed. */
static void
cmd(uint16 cmd, unsigned long off)
{
	int i;
	unlock_cmd_t *ul=NULL;
	unsigned long cmd_off;

	ASSERT(flashutl_cmd != NULL);

	switch (flashutl_cmd->type) {
	case AMD:
		ul = &unlock_cmd_amd;
		cmd_off = AMD_CMD;
		break;
	case SST:
		ul = &unlock_cmd_sst;
		cmd_off = SST_CMD;
		break;
	default:
		cmd_off = 0;
		break;
	}
	
	if (flashutl_cmd->need_unlock) {
		for (i = 0; i < UNLOCK_CMD_WORDS; i++)
			*(uint16*)(flashutl_base + ul->addr[i]) = ul->cmd[i];
	}
	
	/* cmd |= cmd << 8; */

	if (off == CMD_ADDR) 
		off = cmd_off;

#ifdef MIPSEB
	off ^= 2;
#endif
	
	*(uint16*)(flashutl_base + off) = cmd;
}

static void
flash_reset() 
{
	ASSERT(flashutl_desc != NULL);

	if (flashutl_cmd->clear_csr)
		scmd(flashutl_cmd->clear_csr, 0);
	if (flashutl_cmd->read_array)
		scmd(flashutl_cmd->read_array, 0);
}

static int
flash_poll(unsigned long off, uint16 data)
{
	volatile uint16* addr;
	int cnt = FLASH_TRIES;
	uint16 st;

	ASSERT(flashutl_desc != NULL);

	if (flashutl_desc->type == AMD || flashutl_desc->type == SST) {
		/* AMD style poll checkes the address being written */
		addr = (volatile uint16*)FLASH_ADDR(off);
		while ((st = *addr) != data && cnt != 0)
			cnt--;
		if (cnt == 0) {
			DPRINT(("flash_poll: timeout, read 0x%x, expected 0x%x\n", st, data));
			return -1;
		}
	} else {
		/* INTEL style poll is at second word of the block being written */
		addr = (volatile uint16*)FLASH_ADDR(block(off, BLOCK_BASE));
		addr++;
		while (((st = *addr) & DONE) == 0 && cnt != 0)
			cnt--;
		if (cnt == 0) {
			DPRINT(("flash_poll: timeout, error status = 0x%x\n", st));
			return -1;
		}
	}
	
	return 0;
}

static unsigned long
block(unsigned long addr, int which)
{
	unsigned long b, l, sb;
	uint* sblocks;
	int i;
	
	ASSERT(flashutl_desc != NULL);
	ASSERT(addr < (unsigned long)flashutl_desc->size);
	
	b = addr / flashutl_desc->bsize;
	/* check for an address a full size block */
	if (b >= flashutl_desc->ff && b <= flashutl_desc->lf) {
		if (which == BLOCK_LIM) b++;
		return (b * flashutl_desc->bsize);
	}

	/* search for the sub-block */
	if (flashutl_desc->ff == 0) {
		/* sub blocks are at the end of the flash */
		sb = flashutl_desc->bsize * (flashutl_desc->lf + 1);
	} else {
		/* sub blocks are at the start of the flash */
		sb = 0;
	}

	sblocks = flashutl_desc->subblocks;
	for (i = 0; i < flashutl_desc->nsub; i++) {
		b = sb + sblocks[i];
		l = sb + sblocks[i+1];
		if (addr >= b && addr < l) {
			if (which == BLOCK_BASE)
				return b;
			else
				return l;
		}
	}

	ASSERT(1);
	return 0;
}

void
nvWrite(unsigned short *data, unsigned int len)
{
	uint off = flashutl_desc->size - NVRAM_SPACE;
	sysFlashWrite(off, (uchar*)data, len);
}

int
sysFlashErase(uint off, unsigned int numbytes)
{
	unsigned long end = off + numbytes;
	int err = 0;
	
	if (flashutl_cmd->type == SFLASH) {
		err = sflash_commit(cc, off, numbytes, NULL);
	} else {
		ASSERT(!(off & 1));
		while (off < end) {
			err = flash_eraseblk(off);
			if (err)
				break;
			off = flash_block_lim(off);
		}
	}

	if (err)
		DPRINT(("Block erase at 0x%x failed\n", off));
	else
		DPRINT(("Done\n"));
	
	return !err;
}

int
sysFlashWrite(uint off, uchar *src, uint numbytes)
{
	int err;
	
	DPRINT(("Writing 0x%x bytes to flash off @0x%x ...\n", (unsigned int)numbytes, off));

	if (flashutl_cmd->type == SFLASH)
		err = sflash_commit(cc, off, numbytes, src);
	else {
		ASSERT(!(off & 1));
		if (!sysFlashErase(off, numbytes)) 
			return 0;
		err = flash_write(off, (uint16*)src, numbytes);
	}

	if (err) 
		DPRINT(("Flash write failed\n"));
	else
		DPRINT(("Flash write succeeded\n"));

	return !err;
}

int 
sysFlashRead(uint off, uchar *buf, uint numbytes)
{
	uint read, total_read=0;
	uint16 *src, *dst;

	if (flashutl_cmd->type == SFLASH) {
		while (numbytes) {
			read = sflash_read(cc, off, numbytes, buf);
			numbytes -= read;
			buf += read;
			off += read;
			total_read += read;
		}
	} else {
		ASSERT(!(off & 1));
		ASSERT(!(numbytes & 1));
		
		src = (uint16*)(flashutl_base + off);
		dst = (uint16*)buf;
		
		while(numbytes) {
			*dst++ = *src++;
			numbytes-=2;
			total_read+=2;
		}
	}

	return(total_read);
}
