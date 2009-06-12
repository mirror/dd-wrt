/*
 * MIPS EJTAG Target
 * Copyright (C) 2001 Padraig O Mathuna (padraigo@yahoo.com)
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _EJTAG_PRIMITIVES_H_
#define _EJTAG_PRIMITIVES_H_

/* ejtag primitives */
int mips_ejtag_init (unsigned long* dev_id, unsigned long*impl);
int mips_ejtag_instr(char instr);
int mips_ejtag_implementation();
int mips_ejtag_version();
int mips_ejtag_data(int data);
int mips_ejtag_write_w(unsigned int addr, unsigned int data);
int mips_ejtag_read_w(unsigned int addr);
int mips_ejtag_write_h(unsigned int addr, unsigned int data);
int mips_ejtag_read_h(unsigned int addr);
int mips_ejtag_write_b(unsigned int addr, unsigned int data);
int mips_ejtag_read_b(unsigned int addr);
int mips_ejtag_ctrl( unsigned ctrl);
int mips_ejtag_checkstatus();
int mips_ejtag_portwrite(unsigned char byte);
int mips_ejtag_portread();
void mips_ejtag_pracc( int data);
void mips_ejtag_jtagbrk();
void mips_ejtag_setDSU(int en);
void mips_ejtag_release();
void mips_ejtag_setreg( int regnum, int val);
int mips_ejtag_getreg( int regnum);
unsigned int mips_ejtag_returnWord(unsigned int word, unsigned int addr);
unsigned int mips_ejtag_forwardWord( unsigned int addr);
void mips_ejtag_setSingleStep( int dss);
int mips_ejtag_toggleEJTAGWrites();
void mips_ejtag_ctrldump();
void mips_ejtag_dumpRegisters();
void mips_ejtag_storev0v1(unsigned long v[]);
void mips_ejtag_restorev0v1(unsigned long v[]);
void mips_ejtag_flushICache();
void mips_ejtag_flushDCache();
void mips_ejtag_procWrite_w( unsigned long addr, unsigned long data);
unsigned long mips_ejtag_procRead_w( unsigned long addr);
int mips_ejtag_wait (unsigned int *pstat);
unsigned int mips_ejtag_pwacc();
void mips_ejtag_pracc_notdbg( int data);

int mips_ejtag_getcp0reg(int cp0_regnum);
void mips_ejtag_setcp0reg(int cp0_regnum, int val);

void mips_ejtag_flushICacheLine(unsigned long addr);
void mips_ejtag_flushDCacheLine(unsigned long addr);

#define DEBUG_FUNC_CALL  (1<<0)
#define DEBUG_REG_ACCESS (1<<1)
#define DEBUG_MEM_ACCESS (1<<2)
#define DEBUG_PRACC      (1<<3)
#define DEBUG_EXCEPTION  (1<<4)

extern int ejtag_debug ;

#endif /* _EJTAG_PRIMITIVES_H_ */
