/* $Id: teles3.c,v 2.19 2001/09/24 13:22:57 kai Exp $
 *
 * low level stuff for Teles 16.3 & PNP isdn cards
 *
 * Author       Karsten Keil
 * Copyright    by Karsten Keil      <keil@isdn4linux.de>
 * 
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 * Thanks to    Jan den Ouden
 *              Fritz Elfert
 *              Beat Doebeli
 *
 */
#define __NO_VERSION__
#include <linux/init.h>
#include "hisax.h"
#include "isac.h"
#include "hscx.h"
#include "isdnl1.h"

extern const char *CardType[];
const char *teles3_revision = "$Revision: 2.19 $";

#define byteout(addr,val) outb(val,addr)
#define bytein(addr) inb(addr)

static inline u_char
readreg(unsigned int adr, u_char off)
{
	return (bytein(adr + off));
}

static inline void
writereg(unsigned int adr, u_char off, u_char data)
{
	byteout(adr + off, data);
}


static inline void
read_fifo(unsigned int adr, u_char * data, int size)
{
	insb(adr, data, size);
}

static void
write_fifo(unsigned int adr, u_char * data, int size)
{
	outsb(adr, data, size);
}

/* Interface functions */

static u_char
ReadISAC(struct IsdnCardState *cs, u_char offset)
{
	return (readreg(cs->hw.teles3.isac, offset));
}

static void
WriteISAC(struct IsdnCardState *cs, u_char offset, u_char value)
{
	writereg(cs->hw.teles3.isac, offset, value);
}

static void
ReadISACfifo(struct IsdnCardState *cs, u_char * data, int size)
{
	read_fifo(cs->hw.teles3.isacfifo, data, size);
}

static void
WriteISACfifo(struct IsdnCardState *cs, u_char * data, int size)
{
	write_fifo(cs->hw.teles3.isacfifo, data, size);
}

static u_char
ReadHSCX(struct IsdnCardState *cs, int hscx, u_char offset)
{
	return (readreg(cs->hw.teles3.hscx[hscx], offset));
}

static void
WriteHSCX(struct IsdnCardState *cs, int hscx, u_char offset, u_char value)
{
	writereg(cs->hw.teles3.hscx[hscx], offset, value);
}

/*
 * fast interrupt HSCX stuff goes here
 */

#define READHSCX(cs, nr, reg) readreg(cs->hw.teles3.hscx[nr], reg)
#define WRITEHSCX(cs, nr, reg, data) writereg(cs->hw.teles3.hscx[nr], reg, data)
#define READHSCXFIFO(cs, nr, ptr, cnt) read_fifo(cs->hw.teles3.hscxfifo[nr], ptr, cnt)
#define WRITEHSCXFIFO(cs, nr, ptr, cnt) write_fifo(cs->hw.teles3.hscxfifo[nr], ptr, cnt)

#include "hscx_irq.c"

static void
teles3_interrupt(int intno, void *dev_id, struct pt_regs *regs)
{
#define MAXCOUNT 5
	struct IsdnCardState *cs = dev_id;
	u_char val;
	int count = 0;

	if (!cs) {
		printk(KERN_WARNING "Teles: Spurious interrupt!\n");
		return;
	}
	val = readreg(cs->hw.teles3.hscx[1], HSCX_ISTA);
      Start_HSCX:
	if (val)
		hscx_int_main(cs, val);
	val = readreg(cs->hw.teles3.isac, ISAC_ISTA);
      Start_ISAC:
	if (val)
		isac_interrupt(cs, val);
	count++;
	val = readreg(cs->hw.teles3.hscx[1], HSCX_ISTA);
	if (val && count < MAXCOUNT) {
		if (cs->debug & L1_DEB_HSCX)
			debugl1(cs, "HSCX IntStat after IntRoutine");
		goto Start_HSCX;
	}
	val = readreg(cs->hw.teles3.isac, ISAC_ISTA);
	if (val && count < MAXCOUNT) {
		if (cs->debug & L1_DEB_ISAC)
			debugl1(cs, "ISAC IntStat after IntRoutine");
		goto Start_ISAC;
	}
	if (count >= MAXCOUNT)
		printk(KERN_WARNING "Teles3: more than %d loops in teles3_interrupt\n", count);
	writereg(cs->hw.teles3.hscx[0], HSCX_MASK, 0xFF);
	writereg(cs->hw.teles3.hscx[1], HSCX_MASK, 0xFF);
	writereg(cs->hw.teles3.isac, ISAC_MASK, 0xFF);
	writereg(cs->hw.teles3.isac, ISAC_MASK, 0x0);
	writereg(cs->hw.teles3.hscx[0], HSCX_MASK, 0x0);
	writereg(cs->hw.teles3.hscx[1], HSCX_MASK, 0x0);
}

inline static void
release_ioregs(struct IsdnCardState *cs, int mask)
{
	if (mask & 1)
		release_region(cs->hw.teles3.isac + 32, 32);
	if (mask & 2)
		release_region(cs->hw.teles3.hscx[0] + 32, 32);
	if (mask & 4)
		release_region(cs->hw.teles3.hscx[1] + 32, 32);
}

void
release_io_teles3(struct IsdnCardState *cs)
{
	if (cs->typ == ISDN_CTYPE_TELESPCMCIA) {
		release_region(cs->hw.teles3.hscx[1], 96);
	} else {
		if (cs->hw.teles3.cfg_reg) {
			if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
				release_region(cs->hw.teles3.cfg_reg, 1);
			} else {
				release_region(cs->hw.teles3.cfg_reg, 8);
			}
		}
		release_ioregs(cs, 0x7);
	}
}

static int
reset_teles3(struct IsdnCardState *cs)
{
	long flags;
	u_char irqcfg;

	if (cs->typ != ISDN_CTYPE_TELESPCMCIA) {
		if ((cs->hw.teles3.cfg_reg) && (cs->typ != ISDN_CTYPE_COMPAQ_ISA)) {
			switch (cs->irq) {
				case 2:
				case 9:
					irqcfg = 0x00;
					break;
				case 3:
					irqcfg = 0x02;
					break;
				case 4:
					irqcfg = 0x04;
					break;
				case 5:
					irqcfg = 0x06;
					break;
				case 10:
					irqcfg = 0x08;
					break;
				case 11:
					irqcfg = 0x0A;
					break;
				case 12:
					irqcfg = 0x0C;
					break;
				case 15:
					irqcfg = 0x0E;
					break;
				default:
					return(1);
			}
			save_flags(flags);
			byteout(cs->hw.teles3.cfg_reg + 4, irqcfg);
			sti();
			HZDELAY(HZ / 10 + 1);
			byteout(cs->hw.teles3.cfg_reg + 4, irqcfg | 1);
			HZDELAY(HZ / 10 + 1);
			restore_flags(flags);
		} else if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
			save_flags(flags);
			byteout(cs->hw.teles3.cfg_reg, 0xff);
			HZDELAY(2);
			byteout(cs->hw.teles3.cfg_reg, 0x00);
			HZDELAY(2);
			restore_flags(flags);
		} else {
			/* Reset off for 16.3 PnP , thanks to Georg Acher */
			save_flags(flags);
			byteout(cs->hw.teles3.isac + 0x3c, 0);
			HZDELAY(2);
			byteout(cs->hw.teles3.isac + 0x3c, 1);
			HZDELAY(2);
			restore_flags(flags);
		}
	}
	return(0);
}

static int
Teles_card_msg(struct IsdnCardState *cs, int mt, void *arg)
{
	switch (mt) {
		case CARD_RESET:
			reset_teles3(cs);
			return(0);
		case CARD_RELEASE:
			release_io_teles3(cs);
			return(0);
		case CARD_INIT:
			inithscxisac(cs, 3);
			return(0);
		case CARD_TEST:
			return(0);
	}
	return(0);
}

int __devinit
setup_teles3(struct IsdnCard *card)
{
	u_char val;
	struct IsdnCardState *cs = card->cs;
	char tmp[64];

	strcpy(tmp, teles3_revision);
	printk(KERN_INFO "HiSax: Teles IO driver Rev. %s\n", HiSax_getrev(tmp));
	if ((cs->typ != ISDN_CTYPE_16_3) && (cs->typ != ISDN_CTYPE_PNP)
	    && (cs->typ != ISDN_CTYPE_TELESPCMCIA) && (cs->typ != ISDN_CTYPE_COMPAQ_ISA))
		return (0);

	if (cs->typ == ISDN_CTYPE_16_3) {
		cs->hw.teles3.cfg_reg = card->para[1];
		switch (cs->hw.teles3.cfg_reg) {
			case 0x180:
			case 0x280:
			case 0x380:
				cs->hw.teles3.cfg_reg |= 0xc00;
				break;
		}
		cs->hw.teles3.isac = cs->hw.teles3.cfg_reg - 0x420;
		cs->hw.teles3.hscx[0] = cs->hw.teles3.cfg_reg - 0xc20;
		cs->hw.teles3.hscx[1] = cs->hw.teles3.cfg_reg - 0x820;
	} else if (cs->typ == ISDN_CTYPE_TELESPCMCIA) {
		cs->hw.teles3.cfg_reg = 0;
		cs->hw.teles3.hscx[0] = card->para[1] - 0x20;
		cs->hw.teles3.hscx[1] = card->para[1];
		cs->hw.teles3.isac = card->para[1] + 0x20;
	} else if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
		cs->hw.teles3.cfg_reg = card->para[3];
		cs->hw.teles3.isac = card->para[2] - 32;
		cs->hw.teles3.hscx[0] = card->para[1] - 32;
		cs->hw.teles3.hscx[1] = card->para[1];
	} else {	/* PNP */
		cs->hw.teles3.cfg_reg = 0;
		cs->hw.teles3.isac = card->para[1] - 32;
		cs->hw.teles3.hscx[0] = card->para[2] - 32;
		cs->hw.teles3.hscx[1] = card->para[2];
	}
	cs->irq = card->para[0];
	cs->hw.teles3.isacfifo = cs->hw.teles3.isac + 0x3e;
	cs->hw.teles3.hscxfifo[0] = cs->hw.teles3.hscx[0] + 0x3e;
	cs->hw.teles3.hscxfifo[1] = cs->hw.teles3.hscx[1] + 0x3e;
	if (cs->typ == ISDN_CTYPE_TELESPCMCIA) {
		if (check_region((cs->hw.teles3.hscx[1]), 96 )) {
			printk(KERN_WARNING
			       "HiSax: %s ports %x-%x already in use\n",
			       CardType[cs->typ],
			       cs->hw.teles3.hscx[1],
			       cs->hw.teles3.hscx[1] + 96);
			return (0);
		} else
			request_region(cs->hw.teles3.hscx[1], 96, "HiSax Teles PCMCIA");
	} else {
		if (cs->hw.teles3.cfg_reg) {
			if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
				if (check_region((cs->hw.teles3.cfg_reg), 1)) {
					printk(KERN_WARNING
						"HiSax: %s config port %x already in use\n",
						CardType[card->typ],
						cs->hw.teles3.cfg_reg);
					return (0);
				} else
					request_region(cs->hw.teles3.cfg_reg, 1, "teles3 cfg");
			} else {
				if (check_region((cs->hw.teles3.cfg_reg), 8)) {
					printk(KERN_WARNING
					       "HiSax: %s config port %x-%x already in use\n",
					       CardType[card->typ],
					       cs->hw.teles3.cfg_reg,
						cs->hw.teles3.cfg_reg + 8);
					return (0);
				} else
					request_region(cs->hw.teles3.cfg_reg, 8, "teles3 cfg");
			}
		}
		if (check_region((cs->hw.teles3.isac + 32), 32)) {
			printk(KERN_WARNING
			   "HiSax: %s isac ports %x-%x already in use\n",
			       CardType[cs->typ],
			       cs->hw.teles3.isac + 32,
			       cs->hw.teles3.isac + 64);
			if (cs->hw.teles3.cfg_reg) {
				if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
					release_region(cs->hw.teles3.cfg_reg, 1);
				} else {
					release_region(cs->hw.teles3.cfg_reg, 8);
				}
			}
			return (0);
		} else
			request_region(cs->hw.teles3.isac + 32, 32, "HiSax isac");
		if (check_region((cs->hw.teles3.hscx[0] + 32), 32)) {
			printk(KERN_WARNING
			 "HiSax: %s hscx A ports %x-%x already in use\n",
			       CardType[cs->typ],
			       cs->hw.teles3.hscx[0] + 32,
			       cs->hw.teles3.hscx[0] + 64);
			if (cs->hw.teles3.cfg_reg) {
				if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
					release_region(cs->hw.teles3.cfg_reg, 1);
				} else {
					release_region(cs->hw.teles3.cfg_reg, 8);
				}
			}
			release_ioregs(cs, 1);
			return (0);
		} else
			request_region(cs->hw.teles3.hscx[0] + 32, 32, "HiSax hscx A");
		if (check_region((cs->hw.teles3.hscx[1] + 32), 32)) {
			printk(KERN_WARNING
			 "HiSax: %s hscx B ports %x-%x already in use\n",
			       CardType[cs->typ],
			       cs->hw.teles3.hscx[1] + 32,
			       cs->hw.teles3.hscx[1] + 64);
			if (cs->hw.teles3.cfg_reg) {
				if (cs->typ == ISDN_CTYPE_COMPAQ_ISA) {
					release_region(cs->hw.teles3.cfg_reg, 1);
				} else {
					release_region(cs->hw.teles3.cfg_reg, 8);
				}
			}
			release_ioregs(cs, 3);
			return (0);
		} else
			request_region(cs->hw.teles3.hscx[1] + 32, 32, "HiSax hscx B");
	}
	if ((cs->hw.teles3.cfg_reg) && (cs->typ != ISDN_CTYPE_COMPAQ_ISA)) {
		if ((val = bytein(cs->hw.teles3.cfg_reg + 0)) != 0x51) {
			printk(KERN_WARNING "Teles: 16.3 Byte at %x is %x\n",
			       cs->hw.teles3.cfg_reg + 0, val);
			release_io_teles3(cs);
			return (0);
		}
		if ((val = bytein(cs->hw.teles3.cfg_reg + 1)) != 0x93) {
			printk(KERN_WARNING "Teles: 16.3 Byte at %x is %x\n",
			       cs->hw.teles3.cfg_reg + 1, val);
			release_io_teles3(cs);
			return (0);
		}
		val = bytein(cs->hw.teles3.cfg_reg + 2);/* 0x1e=without AB
							 * 0x1f=with AB
							 * 0x1c 16.3 ???
							 * 0x39 16.3 1.1
							 * 0x38 16.3 1.3
							 * 0x46 16.3 with AB + Video (Teles-Vision)
							 */
		if (val != 0x46 && val != 0x39 && val != 0x38 && val != 0x1c && val != 0x1e && val != 0x1f) {
			printk(KERN_WARNING "Teles: 16.3 Byte at %x is %x\n",
			       cs->hw.teles3.cfg_reg + 2, val);
			release_io_teles3(cs);
			return (0);
		}
	}
	printk(KERN_INFO
	       "HiSax: %s config irq:%d isac:0x%X  cfg:0x%X\n",
	       CardType[cs->typ], cs->irq,
	       cs->hw.teles3.isac + 32, cs->hw.teles3.cfg_reg);
	printk(KERN_INFO
	       "HiSax: hscx A:0x%X  hscx B:0x%X\n",
	       cs->hw.teles3.hscx[0] + 32, cs->hw.teles3.hscx[1] + 32);

	if (reset_teles3(cs)) {
		printk(KERN_WARNING "Teles3: wrong IRQ\n");
		release_io_teles3(cs);
		return (0);
	}
	cs->readisac = &ReadISAC;
	cs->writeisac = &WriteISAC;
	cs->readisacfifo = &ReadISACfifo;
	cs->writeisacfifo = &WriteISACfifo;
	cs->BC_Read_Reg = &ReadHSCX;
	cs->BC_Write_Reg = &WriteHSCX;
	cs->BC_Send_Data = &hscx_fill_fifo;
	cs->cardmsg = &Teles_card_msg;
	cs->irq_func = &teles3_interrupt;
	ISACVersion(cs, "Teles3:");
	if (HscxVersion(cs, "Teles3:")) {
		printk(KERN_WARNING
		       "Teles3: wrong HSCX versions check IO address\n");
		release_io_teles3(cs);
		return (0);
	}
	return (1);
}
