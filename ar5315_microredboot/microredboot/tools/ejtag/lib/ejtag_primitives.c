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
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "ejtag.h"
#include "tm.h"
#include "remote-mips-ejtag.h"
#include "ejtag_primitives.h"

int jtagbrk ; // mips in jtag break 
int ejtag_debug ;
static unsigned long tmp_reg[3];

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_init(unsigned long* dev_id, unsigned long*impl)
{
    int  retval ;
    unsigned word ;

    retval=ejtag_switch(EJTAG_TAPRESET,0);  
    retval=ejtag_switch(EJTAG_INIT,0);  
    *dev_id = mips_ejtag_version();
    *impl = mips_ejtag_implementation();
    jtagbrk = 0 ;

    /* qd error check */
    if (*dev_id == 0xFFFFFFFF & *impl == 0xFFFFFFFF )
    {
        printf ("Check cable is connected !!!\n");
        return;
    }
    printf ("debug register (bits)        : %d\n", EJTAG_REGBITS(*impl));
    printf ("break channels               : %d\n", EJTAG_BRKCHAN(*impl) );
    printf ("instr addr break implemented : %c\n", EJTAG_IADRBRK(*impl) ? 'n':'y' );
    printf ("data addr break implemented  : %c\n", EJTAG_DADRBRK(*impl) ? 'n':'y' );
    printf ("proc bus break implemented   : %c\n", EJTAG_PBADRBRK(*impl) ? 'n':'y' );

    /* get target mips information */
    printf("manu: %03x part: %04x\n",EJTAG_MANUFID(*dev_id), EJTAG_PARTID(*dev_id));

    /* we're default bigendian */
    ejtag_switch(EJTAG_BIGENDIAN,1);  

// AJP commented out
#if 0
    /* enable access to DSU */
    mips_ejtag_setDSU(1) ;
    /* enable write access to EJTAG probe memory */
    mips_ejtag_toggleEJTAGWrites();
    if ( ejtag_debug & DEBUG_FUNC_CALL )
        mips_ejtag_dumpRegisters();
#endif
    return retval ;
}

/******************************************************************************
 * Routine     : mips_ejtag_implementation
 * Inputs      : ejtag file descriptor - fd
 * Outputs     : none
 * Returns     : outcome of ioctl
 * Description : 
 *  reads ejtag implementation register, this register contains information
 * regarding the capability of the ejtag/cpu
 *****************************************************************************/
int mips_ejtag_implementation()
{
    return ejtag_switch(EJTAG_IMPLEMENTATION,0);  
}

/******************************************************************************
 * Routine     : mips_ejtag_version
 * Inputs      : ejtag file descriptor - fd
 * Outputs     : none
 * Returns     : outcome of ioctl
 * Description : 
 *  reads ejtag version register, this register contains informations 
 * regarding the part manufacturer, the part identification and its revision
 * number
 *****************************************************************************/
int mips_ejtag_version()
{
    return ejtag_switch(EJTAG_VERSION,0);  
}

/******************************************************************************
 * Routine     : mips_ejtag_instr
 * Inputs      : ejtag file descriptor - fd
 *               5 bit ejtag instruction to execute - instr 
 * Outputs     : none
 * Returns     : outcome of ioctl
 * Description : 
 *  low level primitive that sends an instruction to the ejtag via the 
 * parallel bus
 *****************************************************************************/
int mips_ejtag_instr(char instr)
{
    return ejtag_switch(EJTAG_INSTR,instr);  
}

/******************************************************************************
 * Routine     : mips_ejtag_data
 * Inputs      : ejtag file descriptor - fd
 *               32 bit ejtag data 
 * Outputs     : none
 * Returns     : outcome of ioctl
 * Description : 
 *  low level primitive that sends data to the ejtag, could be the addr, data
 * ctrl, implementation or version registers depending upon which command was
 * sent prior to the call
 *****************************************************************************/
int mips_ejtag_data(int data)
{
    return ejtag_switch(EJTAG_DATA,data);  
}

/******************************************************************************
 * Routine     : mips_ejtag_write_w
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of word to write - addr
 *               data to write - data
 * Outputs     : None
 * Returns     : success/failure
 * Description : 
 *  write a word to a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_write_w(unsigned int addr, unsigned int data)
{
    int cmd[2];

    addr&=0x1fffffff ;
    cmd[0] = addr ;
    cmd[1] = data ;

    ejtag_switch(EJTAG_WRITE_WORD,(long)cmd);
    return 0;
}

/******************************************************************************
 * Routine     : mips_ejtag_read_w
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of word to read - addr
 * Outputs     : None
 * Returns     : value of word read
 * Description : 
 *  read a word from a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_read_w(unsigned int addr)
{
    int data ;

    addr&=0x1fffffff ;
    data=ejtag_switch(EJTAG_READ_WORD,(int)&addr);
    //printf("rd %08x %08x\n", addr,data);
    return data ;
}

/******************************************************************************
 * Routine     : mips_ejtag_write_h
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of halfword to write - addr
 *               data to write - data
 * Outputs     : None
 * Returns     : success/failure
 * Description : 
 *  write a halfword to a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_write_h(unsigned int addr, unsigned int data)
{
    int cmd[2];

    addr&=0x1fffffff ;

    cmd[0] = addr ;
    cmd[1] = data ;

    return ejtag_switch(EJTAG_WRITE_HWORD,(long)cmd);
}

/******************************************************************************
 * Routine     : mips_ejtag_read_h
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of halfword to read - addr
 * Outputs     : None
 * Returns     : value of word read
 * Description : 
 *  read a halfword from a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_read_h(unsigned int addr)
{

    addr&=0x1fffffff ;
    return ejtag_switch(EJTAG_READ_HWORD,(int)&addr);
}

/******************************************************************************
 * Routine     : mips_ejtag_write_b
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of byte to write - addr
 *               data to write - data
 * Outputs     : None
 * Returns     : success/failure
 * Description : 
 *  write a byte to a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_write_b(unsigned int addr, unsigned int data)
{
    int cmd[2];

    addr&=0x1fffffff ;
    cmd[0] = addr ;
    cmd[1] = data ;

    return ejtag_switch(EJTAG_WRITE_BYTE,(long)cmd);
}

/******************************************************************************
 * Routine     : mips_ejtag_read_b
 * Inputs      : ejtag file descriptor - fd
 *               CORE address of byte to read - addr
 * Outputs     : None
 * Returns     : value of byte read
 * Description : 
 *  read a byte from a specified memory location within the target using the 
 * ejtag's dma 
 *****************************************************************************/
int mips_ejtag_read_b(unsigned int addr)
{
    addr&=0x1fffffff ;
    return ejtag_switch(EJTAG_READ_BYTE,(int)&addr);
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_ctrl(unsigned int ctrl)
{
    mips_ejtag_instr(JTAG_CONTROL_IR);
    ctrl |= DEV;
    return mips_ejtag_data(ctrl);
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_checkstatus()
{
    return ejtag_switch(EJTAG_CHECKSTATUS,0);  
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_portwrite(unsigned char byte)
{
    return ejtag_switch(EJTAG_PORTWRITE,(long)byte);
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_portread()
{
    return ejtag_switch(EJTAG_PORTREAD,0);
}

unsigned int mips_ejtag_getaddr()
{
    int addr;
    mips_ejtag_instr(JTAG_ADDRESS_IR);
    addr =  mips_ejtag_data(0);
    mips_ejtag_data(addr);
    return addr;
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
void mips_ejtag_pracc(int data)
{
    unsigned addr ;
    unsigned ctrl ;
    unsigned long timeout=16;
    unsigned prnw;

    /* if mips is not in a jtag brk then create one ! */
    if  ( !jtagbrk )
        mips_ejtag_jtagbrk();

    /* wait for processor access */
    do {
        ctrl = mips_ejtag_checkstatus() ;
        //printf("  ctrl = 0x%08x\n",ctrl);
        timeout--;
    } while (!(ctrl&PRACC)&&timeout!=0);

    if ( timeout == 0 ) {
        fprintf(stderr,"pracc timeout\n");
        return ;
    }
  
    prnw = ctrl&PRNW ? 1:0;
    mips_ejtag_instr(JTAG_ADDRESS_IR);
    addr=mips_ejtag_data(0);

    if (ctrl&PRNW != 0 ) {
        fprintf(stderr,"pracc %08lx not a read pracc!\n", ctrl);
        return ;
    }

    mips_ejtag_instr(JTAG_DATA_IR);
    mips_ejtag_data(data);
    mips_ejtag_instr(JTAG_CONTROL_IR);
    /* ensure that w0 are 1 and w1's are 0 */
    ctrl &= 0x00FFFFFF ;
    ctrl &= ~(TIF|SYNC|DSTRT|PRRST|PERRST|JTAGBRK|PRACC) ;
    ctrl |= DEV ;
    ctrl |= PROBEN;
    mips_ejtag_data(ctrl);

    // printf(__FUNCTION__ "  %08x : %08x %d\n", addr,data, prnw);
}

void mips_ejtag_pracc_notdbg(int data)
{
    unsigned addr ;
    unsigned ctrl ;
    unsigned long timeout=16;
    unsigned prnw;

    /* wait for processor access */
    do {
        ctrl = mips_ejtag_checkstatus() ;
        timeout--;
    } while (!(ctrl&PRACC)&&timeout!=0);

    if ( timeout == 0 ) {
        fprintf(stderr,"pracc timeout\n");
        return ;
    }
  
    prnw = ctrl&PRNW ? 1:0;
    mips_ejtag_instr(JTAG_ADDRESS_IR);
    addr=mips_ejtag_data(0);

    printf("Processor read from %08x\n",addr);

    if (ctrl&PRNW != 0 ) {
        fprintf(stderr,"pracc %08lx not a read pracc!\n", ctrl);
        return ;
    }

    mips_ejtag_instr(JTAG_DATA_IR);
    mips_ejtag_data(data);
    mips_ejtag_instr(JTAG_CONTROL_IR);
    /* ensure that w0 are 1 and w1's are 0 */
    ctrl &= 0x00FFFFFF ;
    ctrl &= ~(TIF|SYNC|DSTRT|PRRST|PERRST|JTAGBRK|PRACC) ;
    ctrl |= DEV ;
    ctrl |= PROBEN;
    mips_ejtag_data(ctrl);

    // printf(__FUNCTION__ "  %08x : %08x %d\n", addr,data, prnw);
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
void mips_ejtag_jtagbrk()
{
    unsigned long cp0_debug;
    unsigned ctrl ;
    ctrl = mips_ejtag_checkstatus();
    /* ensure that w0 are 1 and w1's are 0 */
    ctrl &= 0x00FFFFFF ;
    ctrl &= ~(TIF|SYNC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ctrl |= PROBEN|PRACC ;
    ctrl |= DEV;

    mips_ejtag_instr(JTAG_CONTROL_IR);
    mips_ejtag_data(ctrl);

    ctrl |= DEV|JTAGBRK;
    mips_ejtag_instr(JTAG_CONTROL_IR);
    mips_ejtag_data(ctrl);

    /* jtag break done, SYNC on entrance to debug mode */
    jtagbrk=1 ;

    /* NOTE: though the spec claims MIPS_SYNC is required for some
     *       implementations when entering debug mode, it has caused 
     *       troubles in other places. It appears to be needed here.
     */
    mips_ejtag_pracc(MIPS_SYNC);
    mips_ejtag_pracc(SSNOP);

    /* turn off count in debug mode */
    // printf("Turning off counting while in debug mode\n");
    cp0_debug=mips_ejtag_getcp0reg(CP0_DEBUG);
    cp0_debug &= ~CP0_DEBUG_CNT_DM;
    mips_ejtag_setcp0reg(CP0_DEBUG,cp0_debug);

    /* print the count on the way in */
    // printf("******* Count: %08x *******\n",mips_ejtag_getcp0reg(CP0_COUNT1));

    /* turn off watchdogs */
    /* XXX: cannot do this for Cobra-class SoCs, so turn off for now. */
    //mips_ejtag_procWrite_w(0xbc00300c,0xffffffff);
    //mips_ejtag_procWrite_w(0xbc003008,0);
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
void mips_ejtag_setDSU(int en)
{
    unsigned ctrl ;
    unsigned long orig ;
    unsigned long v[3] ;

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("DSU is being turned %s\n", en?"on":"off");

    /* store original contents of v0 v1 */
    mips_ejtag_storev0v1(v);

    // these instructions will not be executed by the cpu from probe memory
    mips_ejtag_pracc(MFC0(V0,CP0_CONFIG));     // mfc0    $v0,$22      
    mips_ejtag_pracc(NOP);                     // nop                  
    mips_ejtag_pracc(LUI(V1,en?2:0));          // lui     $v1,2        
    mips_ejtag_pracc(OR(V0,V0,V1));            // or      $v0,$v0,$v1  
    mips_ejtag_pracc(MTC0(V0,CP0_CONFIG));     // mtc0    $v0,$22      

    /* restore original contents of v0 v1 */
    mips_ejtag_restorev0v1(v);

}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
void mips_ejtag_release()
{
    unsigned ctrl, val;

    /* print the count on the way out */
    // printf("******* Count: %08x *******\n",mips_ejtag_getcp0reg(CP0_COUNT1));
    
    /* clear wlan1 interrupts */
    //val = mips_ejtag_procRead_w( 0xb8500084);
    //mips_ejtag_procWrite_w(0xb8500084, val);
/* XXX: we want these below for vxworks */
#if 0
    val = mips_ejtag_procRead_w( 0xb8500080);
    mips_ejtag_procWrite_w(0xb8500080, val);
    mips_ejtag_procRead_w( 0xb8500080);
    mips_ejtag_procRead_w( 0xbc003010);
#endif

    mips_ejtag_pracc(MIPS_SYNC);
    mips_ejtag_pracc(SSNOP);
    
    mips_ejtag_pracc(DERET);   // 00000024: 4200001f     deret
    //mips_ejtag_pracc(NOP);   // 00000028: 00000000     nop        
    
    jtagbrk=0 ;
}

/******************************************************************************
 * Routine     : mips_ejtag_setcp0reg
 * Inputs      : fd - ejtag file descriptor
 *               regnum - see remote-mips-ejtag.h
 *               val - value to set register to
 * Outputs     : None
 * Returns     : None
 * Description : 
 *  sets a cp0 register in the mips to a specified value
 *****************************************************************************/
void mips_ejtag_setcp0reg(int cp0_regnum, int val)
{
    if ( cp0_regnum < CP0_INDEX || 
         (cp0_regnum >= CP0_Reserved25 && cp0_regnum <= CP0_Reserved30) ||
         cp0_regnum > CP0_DESAVE )
        return ; /* invalid register do nothing */
    /* save current v0 value */
    mips_ejtag_pracc(MTC0(V0_REGNUM,CP0_DESAVE));
    mips_ejtag_pracc(NOP);          // nop                  
    /* load v0 with value we want to set cp0 register to */ 
    mips_ejtag_pracc(LUI(V0_REGNUM,(val>>16)&0xFFFF));
    mips_ejtag_pracc(ORI(V0_REGNUM, V0_REGNUM,val&0xFFFF));
    /* store v0 into specified register */
    mips_ejtag_pracc(MTC0(V0_REGNUM,cp0_regnum));
    mips_ejtag_pracc(NOP);          // nop                  
    /* restore original v0 value */
    mips_ejtag_pracc(MFC0(V0_REGNUM,CP0_DESAVE));
    mips_ejtag_pracc(NOP);          // nop                  
}

/******************************************************************************
 * Routine     : mips_ejtag_setreg
 * Inputs      : fd - ejtag file descriptor
 *               regnum - see remote-mips-ejtag.h
 *               val - value to set register to
 * Outputs     : None
 * Returns     : None
 * Description : 
 *  sets a register in the mips to a specified value
 *****************************************************************************/
void mips_ejtag_setreg(int regnum, int val)
{
    if ( regnum < ZERO_REGNUM ||  regnum > NUM_REGS )
        return ; /* do nothing */

    if ( regnum >= ZERO_REGNUM && regnum <= RA_REGNUM ) { /* reg file */
        mips_ejtag_pracc(NOP);
        mips_ejtag_pracc(LUI(regnum, (val>>16)&0xffff));
        mips_ejtag_pracc(ORI(regnum, regnum,val&0xFFFF));
    } 
    else if ( regnum > RA_REGNUM && regnum < FP0_REGNUM ) { /* dbg affected regs */
        switch(regnum){
        case PS_REGNUM:
            mips_ejtag_setcp0reg(CP0_STATUS,val);
            break ;
        case HI_REGNUM:
        case LO_REGNUM:
            /* save current v0 value */
            mips_ejtag_pracc(MTC0(V0_REGNUM,CP0_DESAVE));
            mips_ejtag_pracc(NOP);          // nop                  
            /* load v0 with value we want to set cp0 register to */ 
            mips_ejtag_pracc(LUI(V0_REGNUM,(val>>16)&0xFFFF));
            mips_ejtag_pracc(ORI(V0_REGNUM, V0_REGNUM,val&0xFFFF));
            /* store v0 into specified register */
            if ( regnum == HI_REGNUM ) {
                mips_ejtag_pracc(MTHI(V0_REGNUM));
                mips_ejtag_pracc(NOP);          // nop                  
            } else {
                mips_ejtag_pracc(MTLO(V0_REGNUM));
                mips_ejtag_pracc(NOP);          // nop                  
            }
            /* restore original v0 value */
            mips_ejtag_pracc(MFC0(V0_REGNUM,CP0_DESAVE));
            mips_ejtag_pracc(NOP);          // nop                  
            break ;
        case BADVADDR_REGNUM:
            mips_ejtag_setcp0reg(CP0_BADVADDR,val);
            break ;
        case CAUSE_REGNUM:
            mips_ejtag_setcp0reg(CP0_CAUSE,val);
            break ;
        case PC_REGNUM:
            mips_ejtag_setcp0reg(CP0_DEPC,val);
            break ;
        }
    } else if ( regnum >= FP0_REGNUM && regnum <= FP_REGNUM ) { /* floating point */
        return ; /* do nothing */
    } else { /* embedded registers in cp0 */
        mips_ejtag_setcp0reg(regnum-FIRST_EMBED_REGNUM,val);
    }
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_getcp0reg(int cp0_regnum) 
{
    int a0, val, lregnum ;

    if ( cp0_regnum < CP0_INDEX || 
         (cp0_regnum >= CP0_Reserved25 && cp0_regnum <= CP0_Reserved30) ||
         cp0_regnum > CP0_DESAVE )
        return -1 ; /* invalid register do nothing */

    /* save current v0 value */
    mips_ejtag_pracc(MTC0(V0_REGNUM,CP0_DESAVE));
    mips_ejtag_pracc(NOP);          // nop                  
    /* load v0 with value we want to set cp0 register to */ 
    mips_ejtag_pracc(LUI(V0_REGNUM,(EJTAG_BASE>>16)&0xFFFF));
    /* save current a0 value */
    mips_ejtag_pracc(SW(A0_REGNUM,0,V0_REGNUM));
    mips_ejtag_pracc(MIPS_SYNC);
    a0=mips_ejtag_forwardWord(0xff200000);
    /* store cp0 register into a0 */
    mips_ejtag_pracc(MFC0(A0_REGNUM,cp0_regnum));
    mips_ejtag_pracc(NOP);          // nop                  
    /* save current cp0 value */
    mips_ejtag_pracc(SW(A0_REGNUM,0,V0_REGNUM));
    mips_ejtag_pracc(MIPS_SYNC);
    val=mips_ejtag_forwardWord(0xff200000);
    /* restore original a0 value */
    mips_ejtag_pracc(LW(A0_REGNUM,0,V0_REGNUM));
    mips_ejtag_returnWord(a0,0xff200000);
    /* restore original v0 value */
    mips_ejtag_pracc(MFC0(V0_REGNUM,CP0_DESAVE));
    mips_ejtag_pracc(NOP);          // nop                  
    return val;
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *****************************************************************************/
int mips_ejtag_getreg(int regnum)
{
    int a0, val, lregnum ;

    if ( regnum < ZERO_REGNUM ||  regnum > NUM_REGS )
        return 0 ; /* do nothing */

    if ( regnum >= ZERO_REGNUM && regnum <= RA_REGNUM ) { /* reg file */
        /* choose either v0 or a0 as transit register */
        lregnum = regnum!=V0_REGNUM ? V0_REGNUM : A0_REGNUM ;

        /* save current v0/a0 value */
        mips_ejtag_pracc(MTC0(lregnum,CP0_DESAVE));
        mips_ejtag_pracc(NOP);          // nop                  
        /* load v0 with value we want to set cp0 register to */ 
        mips_ejtag_pracc(LUI(lregnum,(EJTAG_BASE>>16)&0xFFFF));
        mips_ejtag_pracc(SW(regnum,0,lregnum));
        mips_ejtag_pracc(MIPS_SYNC);
        val=mips_ejtag_forwardWord(EJTAG_BASE);
        /* restore current v0/a0 value */
        mips_ejtag_pracc(MFC0(lregnum,CP0_DESAVE));
        mips_ejtag_pracc(NOP);          // nop                  
    } else if ( regnum > RA_REGNUM && regnum < FP0_REGNUM ) { /* debug affected regs */
        switch(regnum){
        case PS_REGNUM:
            val=mips_ejtag_getcp0reg(CP0_STATUS);
            break ;
        case HI_REGNUM:
        case LO_REGNUM:
            /* save current v0 value */
            mips_ejtag_pracc(MTC0(V0_REGNUM,CP0_DESAVE));
            mips_ejtag_pracc(NOP);          // nop                  
            /* load v0 with value we want to set cp0 register to */ 
            mips_ejtag_pracc(LUI(V0_REGNUM,(EJTAG_BASE>>16)&0xFFFF));
            /* save current a0 value */
            mips_ejtag_pracc(SW(A0_REGNUM,0,V0_REGNUM));
            mips_ejtag_pracc(MIPS_SYNC);
            a0=mips_ejtag_forwardWord(EJTAG_BASE);
            /* store hi/lo into a0 */
            if ( regnum == HI_REGNUM ) {
                mips_ejtag_pracc(MFHI(A0_REGNUM));
                mips_ejtag_pracc(NOP);          // nop                  
            } else {
                mips_ejtag_pracc(MFLO(A0_REGNUM));
                mips_ejtag_pracc(NOP);          // nop                  
            }
            /* save current hi/lo value */
            mips_ejtag_pracc(SW(A0_REGNUM,0,V0_REGNUM));
            mips_ejtag_pracc(MIPS_SYNC);
            val=mips_ejtag_forwardWord(EJTAG_BASE);
            /* restore original a0 value */
            mips_ejtag_pracc(LW(A0_REGNUM,0,V0_REGNUM));
            mips_ejtag_returnWord(a0,0xff200000);
            /* restore original v0 value */
            mips_ejtag_pracc(MFC0(V0_REGNUM,CP0_DESAVE));
            mips_ejtag_pracc(NOP);          // nop                  
            break ;
        case BADVADDR_REGNUM:
            val=mips_ejtag_getcp0reg(CP0_BADVADDR);
            break ;
        case CAUSE_REGNUM:
            val=mips_ejtag_getcp0reg(CP0_CAUSE);
            break ;
        case PC_REGNUM:
            val=mips_ejtag_getcp0reg(CP0_DEPC);
            break ;
        }
    } else if ( regnum >= FP0_REGNUM && regnum <= FP_REGNUM ) { /* floating point */
        val=0 ; /* do nothing */
    } else { /* embedded registers in cp0 */
        val=mips_ejtag_getcp0reg(regnum-FIRST_EMBED_REGNUM);
    }
    return val;
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *  write `word' into ejtag's data register so processor will read `word' on 
 *  next bus cycle
 *****************************************************************************/
unsigned int mips_ejtag_returnWord(unsigned int word,unsigned int addr)
{
    unsigned paddr ;
    unsigned ctrl ;
    unsigned long timeout = 16 ;

    /* wait for processor access */
    do {
        ctrl = mips_ejtag_checkstatus() ;
    } while (!(ctrl&PRACC));

    do {
        mips_ejtag_instr(JTAG_ADDRESS_IR);
        paddr=mips_ejtag_data(addr);
    
        if ( paddr == addr )
            break ;
        mips_ejtag_pracc(NOP);
    } while( --timeout );
  
    if ( !timeout ) {
        fprintf(stderr,"returnWord timeout\n");
        return 0 ;
    }
    
    if (ctrl&PRNW != 0 ) {
        fprintf(stderr,"returnWord %08lx not a read pracc!\n", ctrl);
        return ;
    }

    mips_ejtag_instr(JTAG_DATA_IR);
    mips_ejtag_data(word);

    if  (ejtag_debug & DEBUG_PRACC )
        printf("Write %08x : %08x\n", addr, word);

    /* clear the PrAcc bit to allow the ejtag to send data to cpu */
    ctrl=mips_ejtag_ctrl(ctrl&~PRACC);
    return ctrl ;
}

/******************************************************************************
 * Routine     : 
 * Inputs      : 
 * Outputs     : 
 * Returns     : 
 * Description : 
 *  read `word' from ejtag's data register that processor has written
 *****************************************************************************/
unsigned int mips_ejtag_forwardWord(unsigned int addr)
{
    unsigned paddr ;
    unsigned ctrl ;
    unsigned word ;
    unsigned long timeout = 16 ;

    /* wait for processor access */
    do {
        do {
            ctrl = mips_ejtag_checkstatus() ;
        } while (!(ctrl&PRACC));
      
        /* check what address the cpu's accessing */
        mips_ejtag_instr(JTAG_ADDRESS_IR);
        paddr=mips_ejtag_data(addr);
      
        if ( paddr == addr )
            break;
        mips_ejtag_pracc(NOP);
        //mips_ejtag_pracc(SSNOP); // AJP: SSNOP
        printf(" **********  addr = %08x, paddr = %08x\n",addr,paddr);
    } while(--timeout);
  
    if ( timeout == 0 ) {
        fprintf(stderr,"forwardWord timeout\n");
        return 0 ;
    }
  
    if ( ctrl&PRNW == 0 ) {
        fprintf(stderr,"forwardWord %08lx not a write pracc!\n", ctrl);
        return ;
    }
    mips_ejtag_instr(JTAG_DATA_IR);
    word=mips_ejtag_data(word);

    /* clear the PrAcc bit to allow the ejtag to send data to cpu */
    ctrl=mips_ejtag_ctrl(ctrl&~PRACC);
  
    if ( ejtag_debug & DEBUG_PRACC )
        printf("Read  %08x : %08x\n", addr, word);
    return word ;
}

unsigned int mips_ejtag_pwacc()
{
    unsigned paddr ;
    unsigned ctrl ;
    unsigned word ;
    unsigned long timeout = 16 ;

    /* wait for processor access */
    do {
        ctrl = mips_ejtag_checkstatus() ;
    } while (!(ctrl&PRACC));
  
    /* check what address the cpu's accessing */
    mips_ejtag_instr(JTAG_ADDRESS_IR);
    paddr=mips_ejtag_data(paddr);
    mips_ejtag_data(paddr);
    printf("processor word write to %08x\n",paddr);
    
    if ( ctrl&PRNW == 0 ) {
        fprintf(stderr,"forwardWord %08lx not a write pracc!\n", ctrl);
        return ;
    }
    mips_ejtag_instr(JTAG_DATA_IR);
    word=mips_ejtag_data(word);

    /* clear the PrAcc bit to allow the ejtag to send data to cpu */
    ctrl=mips_ejtag_ctrl(ctrl&~PRACC);
  
    return word ;
}


/******************************************************************************
 * Routine     : mips_ejtag_setSingleStep
 * Inputs      : ejtag file descriptor - fd
 *               whether to turn single stepping on/off - dss
 * Outputs     : 
 * Returns     : 
 * Description : 
 * set the single step enable flag in the cpu's cop0 debug
 * register to the user specified value.  
 *****************************************************************************/
void mips_ejtag_setSingleStep(int dss)
{
    unsigned long cp0_debug ;
  
    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("mips_ejtag_setSingleStep(%d)\n",dss);

    //cp0_debug=mips_ejtag_getcp0reg(CP0_DEBUG);
    cp0_debug=mips_ejtag_getcp0reg(23);
    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("cp0_debug %08x\n",cp0_debug);

    if ( dss ) /* turn on single stepping */
        cp0_debug |= CP0_DEBUG_SST_EN ;
    else /* turn off single stepping */
        cp0_debug &= ~CP0_DEBUG_SST_EN ;
    /* update the real register */
    //mips_ejtag_setcp0reg(CP0_DEBUG,cp0_debug);
    mips_ejtag_setcp0reg(23,cp0_debug);

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("cp0_debug %08x\n",cp0_debug);
}

/******************************************************************************
 * Routine     : mips_ejtag_wait
 * Inputs      : ejtag file descriptor - fd
 * Outputs     : 
 * Returns     : 
 * Description : 
 * wait for the remote CPU to trap
 *****************************************************************************/
int mips_ejtag_wait (unsigned int *pstat)
{
    unsigned ctrl ;
    unsigned long cp0_debug,cp0_status,cp0_depc ;

    if ( ejtag_debug & DEBUG_FUNC_CALL ) 
        printf("mips_wait called\n");

    do {
        ctrl = mips_ejtag_checkstatus() ;
        usleep(200000);
    } while (!(ctrl&PRACC));

    jtagbrk=1;    

    /* NOTE: issuing MIPS_SYNC on entrance to debug mode from the debug
     *       break instruction causes the processor execution pipeline to
     *       become corrupted (DEPC becomes unreliable). The instructions
     *       that caused trouble are below for reference: 
     *
     *           mips_ejtag_pracc(MIPS_SYNC);
     *       followed by either
     *           mips_ejtag_pracc(SSNOP);
     *       or
     *           mips_ejtag_pracc(NOP);
     */

    /* print the count on the way in */
    printf("******* Count: %08x *******\n",mips_ejtag_getcp0reg(CP0_COUNT1));
    

/* AJP: remove to determine if causing trouble */
#if 0
    cp0_debug=mips_ejtag_getreg(FIRST_EMBED_REGNUM+CP0_DEBUG);

    if ( ejtag_debug & DEBUG_REG_ACCESS ) 
        printf("mips_wait : cp0_debug %08x\n",cp0_debug);

    if ( cp0_debug & (CP0_DEBUG_BSSF |CP0_DEBUG_TLF  | 
                      CP0_DEBUG_OES  |CP0_DEBUG_UMS  |
                      CP0_DEBUG_NIS  )) {
        cp0_status=mips_ejtag_getreg(FIRST_EMBED_REGNUM+CP0_STATUS);
        if ( cp0_debug & CP0_DEBUG_OES ) {
            if ( cp0_status & CP0_Status_BEV_Mask )
                mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,0xbfc00180);
            else
                mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,0x80000080) ;
            mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEBUG,
                              cp0_debug&~CP0_DEBUG_OES) ;
        } else if ( cp0_debug & CP0_DEBUG_UMS ) {
            if ( cp0_status & CP0_Status_BEV_Mask )
                mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,0xbfc00100) ;
            else
                mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,0x80000000) ;
            mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEBUG,
                              cp0_debug&~CP0_DEBUG_UMS) ;
        } else if ( cp0_debug & CP0_DEBUG_NIS ) { 
            mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,0xbfc00000) ;
            mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEBUG,
                              cp0_debug&~CP0_DEBUG_NIS) ;
        }
    }
    if  (cp0_debug&CP0_DEBUG_SS_EXC) {
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : single step exception\n");
        if ( cp0_debug&CP0_DEBUG_DBD) {
            if ( ejtag_debug & DEBUG_EXCEPTION ) 
                printf("mips_wait : branch delay\n");
            cp0_depc=mips_ejtag_getreg(FIRST_EMBED_REGNUM+CP0_DEPC);
            cp0_depc+=0x4 ;
            mips_ejtag_setreg(FIRST_EMBED_REGNUM+CP0_DEPC,cp0_depc);
        }
        *pstat = TARGET_WAITKIND_STOPPED ;
    }
    if ( cp0_debug&CP0_DEBUG_BP_EXC) {
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : breakpoint exception\n");
        *pstat = TARGET_WAITKIND_STOPPED ;
    }
    if ( cp0_debug&CP0_DEBUG_DBL_EXC) {
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : data address break load exception\n");
        *pstat = TARGET_WAITKIND_STOPPED ;
    }
    if ( cp0_debug&CP0_DEBUG_DBS_EXC) {
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : data address break store exception\n");
        *pstat = TARGET_WAITKIND_STOPPED ;
    }
    if ( cp0_debug&CP0_DEBUG_DIB_EXC) {
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : instruction address break exception\n");
        *pstat = TARGET_WAITKIND_STOPPED ;
    }
    if ( cp0_debug&CP0_DEBUG_DINT_EXC) { 
        if ( ejtag_debug & DEBUG_EXCEPTION ) 
            printf("mips_wait : processor/bus break\n");
        *pstat = TARGET_WAITKIND_STOPPED ;
    }  
#endif

    return 0 ;
}

/******************************************************************************
 * Routine     : mips_ejtag_enableEJTAGWrites
 * Inputs      : ejtag file descriptor - fd
 * Outputs     : None
 * Returns     : success/failure
 * Description : 
 *  destroys both a0 and v0 to enable writing to ejtag probe
 *****************************************************************************/
int mips_ejtag_toggleEJTAGWrites()
{
    unsigned long orig ;
    unsigned long v[3] ;

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("Toggling EJTAG Writes\n");

    /* store original contents of v0 v1 */
    mips_ejtag_storev0v1(v);

    /* li v0, 0xff300000 */
    mips_ejtag_pracc(LUI(V0,(DSU_BASE>>16)&0xFFFF));
    /* lw v1, 0(v0) */
    mips_ejtag_pracc(LW(V1,0,V0));
    /* bleed datapath */
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(SW(V1,0,ZERO));
    mips_ejtag_pracc(MIPS_SYNC);
    /* bleed datapath */
    mips_ejtag_pracc(NOP);          // nop                  
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    if  (ejtag_debug & DEBUG_MEM_ACCESS ) 
        printf( "%08lx read from dsu ctrl\n", mips_ejtag_read_w(0));
    mips_ejtag_pracc(XORI(V1,V1,DSU_DCR_MP));
    mips_ejtag_pracc(SW(V1,0,ZERO));
    mips_ejtag_pracc(MIPS_SYNC);
    /* bleed datapath */
    mips_ejtag_pracc(NOP);          // nop                  
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    /* sw v1, 0(v0) */
    mips_ejtag_pracc(SW(V1,0,V0));
    mips_ejtag_pracc(MIPS_SYNC);
    /* bleed datapath */
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);          // nop                  
    mips_ejtag_pracc(SW(V1,0,ZERO));
    mips_ejtag_pracc(MIPS_SYNC);
    /* bleed datapath */
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);
    mips_ejtag_pracc(NOP);          // nop                  
    if  (ejtag_debug & DEBUG_MEM_ACCESS ) 
        printf( "%08lx stored in dsu ctrl\n", mips_ejtag_read_w(0));

    /* restore original contents of v0 v1 */
    mips_ejtag_restorev0v1(v);
}
 
/******************************************************************************
 * Routine     : mips_ejtag_ctrldump
 * Inputs      : fd - ejtag file descriptor
 * Outputs     : None
 * Returns     : None
 * Description : 
 * dump contents of the ejtag ctrl/status register with explanations
 *****************************************************************************/
void mips_ejtag_ctrldump()
{
    int ctrl ;
  
    ctrl = mips_ejtag_checkstatus();

    printf ("\nCtrl + : %x\n", ctrl);
    printf ("     |\n");
    printf ("     +-- DCLKEN    : %d\n", (ctrl>> 0)&0x1 );
    printf ("     +-- TOF       : %d\n", (ctrl>> 1)&0x1 );
    printf ("     +-- TIF       : %d\n", (ctrl>> 2)&0x1 );
    printf ("     +-- BrkSt     : %d\n", (ctrl>> 3)&0x1 );
    printf ("     +-- Dinc      : %d\n", (ctrl>> 4)&0x1 );
    printf ("     +-- Dlock     : %d\n", (ctrl>> 5)&0x1 );
    printf ("     +-- Dsz       : %d\n", (ctrl>> 7)&0x3 );
    printf ("     +-- Drwm      : %d\n", (ctrl>> 9)&0x1 );
    printf ("     +-- Derr      : %d\n", (ctrl>>10)&0x1 );
    printf ("     +-- Dstrt     : %d\n", (ctrl>>11)&0x1 );
    printf ("     +-- JtagBrk   : %d\n", (ctrl>>12)&0x1 );
    printf ("     +-- ProbEn    : %d\n", (ctrl>>15)&0x1 );
    printf ("     +-- PrRst     : %d\n", (ctrl>>16)&0x1 );
    printf ("     +-- DmaAcc    : %d\n", (ctrl>>17)&0x1 );
    printf ("     +-- PrAcc     : %d\n", (ctrl>>18)&0x1 );
    printf ("     +-- PRnW      : %d\n", (ctrl>>19)&0x1 );
    printf ("     +-- PerRst    : %d\n", (ctrl>>20)&0x1 );
    printf ("     +-- Run       : %d\n", (ctrl>>21)&0x1 );
    printf ("     +-- Doze      : %d\n", (ctrl>>22)&0x1 );
    printf ("     +-- Sync      : %d\n", (ctrl>>23)&0x1 );
    printf ("     +-- DsuRst    : %d\n", (ctrl>>24)&0x1 );
    printf ("     +-- Psz       : %d\n", (ctrl>>25)&0x3 );
    printf ("     +-- JtagInt   : %d\n", (ctrl>>27)&0x1 );
    printf ("     +-- JtagIntEn : %d\n", (ctrl>>28)&0x1 );
}

/******************************************************************************
 * Routine     : mips_ejtag_dumpRegisters
 * Inputs      : fd - ejtag file descriptor
 * Outputs     : 
 * Returns     : 
 * Description : Dump all 32 registers in regfile
 *****************************************************************************/
void mips_ejtag_dumpRegisters()
{
    unsigned long orig,reg ;
    int i ;

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("Registers being dumped\n");

    if ( ejtag_debug & DEBUG_FUNC_CALL ){
        orig=mips_ejtag_read_w(0);
        printf("0 read %08x\n", orig);
    }
    /* store value at memory location 0x0 */
    orig=mips_ejtag_read_w(0);

    for(i=0;i<32;i++) {
        /* save v0 register */
        mips_ejtag_pracc(SW(i,0,ZERO));
        mips_ejtag_pracc(MIPS_SYNC);
        /* bleed datapath */
        mips_ejtag_pracc(NOP);
        mips_ejtag_pracc(NOP);
        mips_ejtag_pracc(NOP);
        reg=mips_ejtag_read_w(0);
        if ( i%4 == 0 )
            putchar('\n');
        if ( ejtag_debug & DEBUG_FUNC_CALL )
            printf("r%2d:%08x ", i, reg);
    }
    putchar('\n');
}

/******************************************************************************
 * Routine     : mips_ejtag_storev0v1
 * Inputs      : fd - ejtag file descriptor
 *               v - pointer to array of 3 unsigned longs
 * Outputs     : 
 * Returns     : 
 * Description : safely store contents of v0,v1
 *****************************************************************************/
void mips_ejtag_storev0v1(unsigned long v[])
{
    /* store current v0 value in debug reg */
    mips_ejtag_pracc(MTC0(V0,CP0_DESAVE));
    mips_ejtag_pracc(NOP);    

    /* load v0 with EJTAG base address */ 
    mips_ejtag_pracc(LUI(V0,EJTAG_BASE>>16&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,EJTAG_BASE&0xffff));
  
    /* grab v1 */
    mips_ejtag_pracc(SW(V1,0,V0));
    mips_ejtag_pracc(MIPS_SYNC);
    v[1]=mips_ejtag_forwardWord(EJTAG_BASE);

    /* grab v0 */
    mips_ejtag_pracc(MFC0(V1,CP0_DESAVE));
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(SW(V1,0,V0));
    mips_ejtag_pracc(MIPS_SYNC);
    v[0]=mips_ejtag_forwardWord(EJTAG_BASE);

    /* bleed datapath (really need this?)*/
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          

    /* note - don't care about v[2] anymore, since didn't clobber
     *   memory location 0.
     */
}

/******************************************************************************
 * Routine     : mips_ejtag_restorev0v1
 * Inputs      : fd - ejtag file descriptor
 *               v - pointer to array of 3 unsigned longs
 * Outputs     : 
 * Returns     : 
 * Description : safely restore contents of v0,v1
 *****************************************************************************/
void mips_ejtag_restorev0v1(unsigned long v[])
{
/* see mips_ejtag_restorev0v1() for different storing schemes. */
    /* load v0 with EJTAG base address */ 
    mips_ejtag_pracc(LUI(V0,EJTAG_BASE>>16&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,EJTAG_BASE&0xffff));
  
    /* write v1 */
    mips_ejtag_pracc(LW(V1,0,V0));
    mips_ejtag_returnWord(v[1],EJTAG_BASE);
  
    /* write v0 */
    mips_ejtag_pracc(LW(V0,0,V0));
    mips_ejtag_returnWord(v[0],EJTAG_BASE);
    //mips_ejtag_pracc(MIPS_SYNC);
}

/******************************************************************************
 * Routine     : mips_ejtag_procWrite_w
 * Inputs      : fd - ejtag file descriptor
 *               addr - address to write word to
 *               data - word to write
 * Outputs     : None
 * Returns     : None
 * Description : get cpu to write word to address.
 *****************************************************************************/
void mips_ejtag_procWrite_w(unsigned long addr, unsigned long data)
{
    int offs, base ;
    unsigned long v[3];
    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("mips_ejtag_procWrite_w(%08x,%08x)\n",addr,data);

    mips_ejtag_storev0v1(v);

    /* load v0 with address */ 
    mips_ejtag_pracc(LUI(V0,(addr>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,addr&0xffff));

    /* load v1 with data */ 
    mips_ejtag_pracc(LUI(V1,(data>>16)&0xffff));
    mips_ejtag_pracc(ORI(V1,V1,data&0xffff));
  
    /* store v1 */
    mips_ejtag_pracc(SW(V1,0,V0));
    mips_ejtag_pracc(MIPS_SYNC);

    /* bleed datapath */
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          
  
    mips_ejtag_restorev0v1(v);
}

void mips_ejtag_procWrite_wfast(unsigned long addr, unsigned long data)
{
    int offs, base ;
    unsigned long v[3];
    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("mips_ejtag_procWrite_w(%08x,%08x)\n",addr,data);

    //mips_ejtag_storev0v1(v);

    /* load v0 with address */ 
    mips_ejtag_pracc(LUI(V0,(addr>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,addr&0xffff));

    /* load v1 with data */ 
    mips_ejtag_pracc(LUI(V1,(data>>16)&0xffff));
    mips_ejtag_pracc(ORI(V1,V1,data&0xffff));
  
    /* store v1 */
    mips_ejtag_pracc(SW(V1,0,V0));
    //mips_ejtag_pracc(MIPS_SYNC);

    /* bleed datapath */
    //mips_ejtag_pracc(NOP);          
    //mips_ejtag_pracc(NOP);          
    //mips_ejtag_pracc(NOP);          
  
    //mips_ejtag_restorev0v1(v);
}



/******************************************************************************
 * Routine     : mips_ejtag_procRead_w
 * Inputs      : fd - ejtag file descriptor
 *               addr - address to write word to
 * Outputs     : None
 * Returns     : None
 * Description : get cpu to write word to address.
 *****************************************************************************/
unsigned long mips_ejtag_procRead_w(unsigned long addr)
{
    unsigned long v[3], val;

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("mips_ejtag_readRead_w(%08x)\n",addr);

    mips_ejtag_storev0v1(v);

    /* load v0 with address */ 
    mips_ejtag_pracc(LUI(V0,(addr>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,addr&0xffff));
    /* load v1 */
    mips_ejtag_pracc(LW(V1,0,V0));
    /* bleed datapath */
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          
    mips_ejtag_pracc(NOP);          

    /* load v0 with data */ 
    mips_ejtag_pracc(LUI(V0,(EJTAG_BASE>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0,V0,EJTAG_BASE&0xffff));
  
    /* store v1 */
    mips_ejtag_pracc(SW(V1,0,V0));
    mips_ejtag_pracc(MIPS_SYNC);
    val=mips_ejtag_forwardWord(EJTAG_BASE);

    mips_ejtag_restorev0v1(v);

    return val ;
}
  

void mips_ejtag_flushICacheLine(unsigned long addr)
{
    unsigned long v[3];

    mips_ejtag_storev0v1(v);

    mips_ejtag_pracc(LUI(V0,(addr>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0, V0,(addr&0xffff)));
          
    mips_ejtag_pracc(CACHE((0x14 | ICACHE),0,V0));

    mips_ejtag_restorev0v1(v);
}

void mips_ejtag_flushDCacheLine(unsigned long addr)
{
    unsigned long v[3];

    mips_ejtag_storev0v1(v);

    mips_ejtag_pracc(LUI(V0,(addr>>16)&0xffff));
    mips_ejtag_pracc(ORI(V0, V0,(addr&0xffff)));
          
    mips_ejtag_pracc(CACHE((0x14 | DCACHE),0,V0));

    mips_ejtag_restorev0v1(v);
}


/******************************************************************************
 * Routine     : mips_ejtag_flushICache
 * Inputs      : fd - ejtag file descriptor
 * Outputs     : None
 * Returns     : None
 * Description : invalidate instruction cache 
 *****************************************************************************/
void mips_ejtag_flushICache()
{
    unsigned long v[3];
    int i ;

    mips_ejtag_storev0v1(v);

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("flushing ICache\n");
  
    mips_ejtag_pracc(LUI(V0,0x8000));          
    for(i=0;i<512;i++) {
        mips_ejtag_pracc(CACHE(ICACHE,0,V0));
        mips_ejtag_pracc(ADDI(V0,V0,0x10));          
    }

    mips_ejtag_restorev0v1(v);
}

/******************************************************************************
 * Routine     : mips_ejtag_flushDCache
 * Inputs      : fd - ejtag file descriptor
 * Outputs     : None
 * Returns     : None
 * Description : invalidate instruction cache 
 *****************************************************************************/
void mips_ejtag_flushDCache()
{
    unsigned long v[3];
    int i ;
  
    mips_ejtag_storev0v1(v);

    if ( ejtag_debug & DEBUG_FUNC_CALL )
        printf("flushing DCache\n");
  
    mips_ejtag_pracc(LUI(V0,0x8000));          
    for(i=0;i<512;i++) {
        mips_ejtag_pracc(CACHE(DCACHE,0,V0));
        mips_ejtag_pracc(ADDI(V0,V0,0x10));          
    }

    mips_ejtag_restorev0v1(v);
}
