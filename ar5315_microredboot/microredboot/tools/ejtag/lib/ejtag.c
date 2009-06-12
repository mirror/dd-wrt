/*
 * EJTAG parallel port driver
 *
 * Copyright (C) 2000 padraigo
 *  Based on ejtag.c
 *
 * MODIFIED: no longer driver. many fixes to work on common MIPS 4K.
 */



#include <stdlib.h>
#include <stdio.h>
#include <asm/io.h>
#include <unistd.h>
#include <ejtag.h>

struct ejtag_struct {
    unsigned long flags;
    unsigned int debug ;
    unsigned int instr ;
    unsigned int ctrl ;
};

static int reset = 0;
static int dbg=0;

struct ejtag_struct ejtag_state =
{
    flags           : 0,                    
    ctrl            : 0,                    
    instr           : 0 
};


/******************************************************************************
 ** routines dealing with low level ejtag functionality
 ******************************************************************************/
static inline void pp_write_data(unsigned char data);
static inline void pp_write_ctrl(unsigned char ctrl);
static inline int pp_read_stat();
static void ejt_reset();
static inline unsigned char ejt_tapmove(unsigned char tms, unsigned char tdo);
static void ejt_tapreset();
static unsigned int ejt_data(unsigned din);
static unsigned char ejt_instr(unsigned char din);
static unsigned int ejt_dmawrite_w(unsigned int addr,unsigned int data);
static unsigned int ejt_dmawrite_h(unsigned int addr,unsigned int data);
static unsigned int ejt_dmawrite_b(unsigned int addr,unsigned int data);
static unsigned int ejt_dmaread_w(unsigned int addr);
static unsigned int ejt_dmaread_h(unsigned int addr);
static unsigned int ejt_dmaread_b(unsigned int addr);
static unsigned int ejt_version();
static unsigned int ejt_implementation();
static unsigned int ejt_checkstatus();

#undef EJTAG_DEBUG
#undef EJTAG_READ_DEBUG


/* --- low-level parport interface routines -------------------------- */

extern unsigned long pp_base;

/* write data to the parallel port */
static void pp_write_data(unsigned char data)
{
    outb(data,pp_base + PP_DATA_OFF);
}

/* write ctrl to the parallel port */
static void pp_write_ctrl(unsigned char ctrl)
{
    outb(ctrl,pp_base + PP_CONTROL_OFF); 
}

/* read in from the busy line of the parallel port */
static int pp_read_stat()
{
    return inb(pp_base + PP_STATUS_OFF);
}

/* --- low-level EJTAG functions ----------------------------------- */

/* reset the tap controller */
static void ejt_reset()
{
    unsigned char dout ;

    dout = (unsigned char)TRSTN_BIT(0);
    pp_write_data(dout);
    dout = (unsigned char)TRSTN_BIT(1);
    pp_write_data(dout);
    ejtag_state.ctrl=0;
    ejtag_state.instr=0;
    ejtag_state.debug=0;
}

/* move the tap controller to another state */
static unsigned char ejt_tapmove(unsigned char tms, unsigned char tdo)
{
    unsigned char dout,din ;
    dout = (unsigned char) (TRSTN_BIT(1) | TMS_BIT(tms) | TDO_BIT(tdo) | TCK_BIT(0));
    pp_write_data(dout);
    dout = (unsigned char) (TRSTN_BIT(1) | TMS_BIT(tms) | TDO_BIT(tdo) | TCK_BIT(1));
    pp_write_data(dout);
    din=pp_read_stat()&PARPORT_STATUS_BUSY ? 1 : 0; // invert level shift
    dout = (unsigned char) (TRSTN_BIT(1) | TMS_BIT(tms) | TDO_BIT(tdo) | TCK_BIT(0));
    pp_write_data(dout);
    return din;
}

/* reset the tap controller */
static void ejt_tapreset()
{
    int i ;
    unsigned char dout ;
    for (i=0;i<5;i++)
    {
        dout = TRSTN_BIT(1) | TMS_BIT(1) | TDO_BIT(0) | TCK_BIT(0);
        pp_write_data(dout);
        dout = TRSTN_BIT(1) | TMS_BIT(1) | TDO_BIT(0) | TCK_BIT(1);
        pp_write_data(dout);
    }
}

/* scan data into the tap controller's data register */
static unsigned int ejt_data(unsigned din)
{
    int i ;
    unsigned dout ;
    unsigned char bit ;
    unsigned ctrl_tx ;

    ctrl_tx=din ;

    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(1,0);             /* enter select-dr-scan    */
    ejt_tapmove(0,0);             /* enter capture-dir       */
    ejt_tapmove(0,0);             /* enter shift-dr          */

    dout=0;
    for(i=0;i<31;i++)
    {
        bit=ejt_tapmove(0,din&0x1); /* enter shift-dr          */
        din=din>>1;
        dout=dout|((bit&0x1)<<i);
    }
    bit=ejt_tapmove(1,din&0x1);   /* enter shift-dr          */
    dout=dout|((bit&0x1)<<i);

    ejt_tapmove(1,0);             /* enter update-dr         */
    ejt_tapmove(0,0);             /* enter runtest-idle      */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */

    if ( ejtag_state.instr == JTAG_CONTROL_IR )
    {
        ejtag_state.ctrl = dout ;
    }

    return dout ;
}

/* scan data into the tap controller's instruction register */
static unsigned char ejt_instr(unsigned char din)
{
    unsigned dout ;
    unsigned char bit ;

    /* keep track of last instruction */

    ejtag_state.instr = din ;

    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(1,0);             /* enter select-dr-scan    */
    ejt_tapmove(1,0);             /* enter select-ir-scan    */
    ejt_tapmove(0,0);             /* enter capture-ir        */
    ejt_tapmove(0,0);             /* enter shift-ir (dummy)  */

    dout=0;
    bit=ejt_tapmove(0,din&0x1);   /* enter shift-ir */
    din=din>>1;
    dout=dout|((bit&0x1)<<0);
    bit=ejt_tapmove(0,din&0x1);   /* enter shift-ir */
    din=din>>1;
    dout=dout|((bit&0x1)<<1);
    bit=ejt_tapmove(0,din&0x1);   /* enter shift-ir */
    din=din>>1;
    dout=dout|((bit&0x1)<<2);
    bit=ejt_tapmove(0,din&0x1);   /* enter shift-ir */
    din=din>>1;
    dout=dout|((bit&0x1)<<3);
    bit=ejt_tapmove(1,din&0x1);   /* enter shift-ir */
    dout=dout|((bit&0x1)<<4);

    ejt_tapmove(1,0);             /* enter update-ir */
    ejt_tapmove(0,0);             /* enter runtest-idle */
    ejt_tapmove(0,0);             /* runtestidle             */
    ejt_tapmove(0,0);             /* runtestidle             */

    return dout ;
}

/* scan in the instructions to do a pi word write */
static unsigned int ejt_dmawrite_w(unsigned int addr,unsigned int data)
{
    /* scan in address */
    ejt_instr(JTAG_ADDRESS_IR);
    ejt_data(addr);

    /* scan in data */
    ejt_instr(JTAG_DATA_IR);
    ejt_data(data);

    /* set up ctrl reg to do a DMA word write, no checks for DSTRT since this interface
     * is very slow! 
     */

    /* clear old DMA size and any read/lock flags */
    ejtag_state.ctrl &= 0x00FFFE7F&~(DRWN|DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK);
    ejtag_state.ctrl |= PROBEN|DMAACC|DSTRT|JTS_WORD|TOF|PRACC ;
    ejt_instr(JTAG_CONTROL_IR);
    ejt_data(ejtag_state.ctrl);

    /* clear the DMAACC after write as per specification, could interfere with PRACC! */
    ejtag_state.ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_state.ctrl |= PROBEN|PRACC|JTS_WORD ;
    ejt_instr(JTAG_CONTROL_IR);
    ejt_data(ejtag_state.ctrl);
    ejtag_state.ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    return ejtag_state.ctrl;
}

/* scan in the instructions to do a pi halfword write */
static unsigned int ejt_dmawrite_h(unsigned int addr,unsigned int data)
{
    /* scan in address */
    ejt_instr(JTAG_ADDRESS_IR);
    ejt_data(addr);

    /* handle the bigendian/littleendian */
    data = (data&0x0000ffff) | (data<<16) ;

    /* scan in data */
    ejt_instr(JTAG_DATA_IR);
    ejt_data(data);

    /* set up ctrl reg to do a DMA word write, no checks for DSTRT since this interface
     * is very slow! 
     */

    /* clear old DMA size and any read/lock flags */
    ejtag_state.ctrl &= 0x00FFFE7F&~(DRWN|DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK);
    ejtag_state.ctrl |= PROBEN|DMAACC|DSTRT|JTS_HALFWORD|TOF|PRACC ;
    ejt_instr(JTAG_CONTROL_IR);
    ejt_data(ejtag_state.ctrl);

    /* clear the DMAACC after write as per specification, could interfere with PRACC! */
    ejtag_state.ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_state.ctrl |= PROBEN|PRACC|JTS_HALFWORD ;
    ejt_instr(JTAG_CONTROL_IR);
    ejt_data(ejtag_state.ctrl);
    ejtag_state.ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    return ejtag_state.ctrl;
}

#if 0 // AJP: fixme later since don't currently use
/* scan in the instructions to do a pi byte write */
static unsigned int ejt_dmawrite_b(int minor,unsigned int addr,unsigned int data)
{

    cli();
    /* scan in address */
    ejt_instr(minor,JTAG_ADDRESS_IR);
    ejt_data(minor,addr);

    /* handle the bigendian/littleendian */
    data = data & 0xff ;
    data = (data<<24)|(data<<16)|(data<<8)|data ;
    /* scan in data */
    ejt_instr(minor,JTAG_DATA_IR);
    ejt_data(minor,data);

    /* set up ctrl reg to do a DMA word write, no checks for DSTRT since this interface
     * is very slow! 
     */

    /* clear old DMA size and any read/lock flags */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DRWN|DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK);
    ejtag_table[minor].ctrl |= PROBEN|DMAACC|DSTRT|JTS_BYTE|TOF|PRACC ;
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmawrite_b with ctrl    %08x\n", minor, ejtag_table[minor].ctrl);
    ejt_instr(minor,JTAG_CONTROL_IR);
    ejt_data(minor,ejtag_table[minor].ctrl);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmawrite_b returns ctrl %08x\n", minor, ejtag_table[minor].ctrl);

    /* clear the DMAACC after write as per specification, could interfere with PRACC! */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|PRACC|JTS_BYTE ;
    ejt_instr(minor,JTAG_CONTROL_IR);
    ejt_data(minor,ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    sti();
    return ejtag_table[minor].ctrl;
}

/* scan in the instructions to do a pi word read */
static unsigned int ejt_dmaread_w(int minor,unsigned int addr)
{
    unsigned int data ;

    cli();
    /* scan in address */
    ejt_instr(minor,JTAG_ADDRESS_IR);
    ejt_data(minor,addr);

    /* set up ctrl reg to do a DMA word read, no checks for DSTRT since this interface
     * is very slow!
     */
    /* clear old DMA size,lock flag, proc & periph resets */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|DMAACC|DSTRT|JTS_WORD|DRWN|TOF|PRACC ;
    ejt_instr(minor,JTAG_CONTROL_IR);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread with ctrl    %08x\n", minor, ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl=ejt_data(minor,ejtag_table[minor].ctrl);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread returns ctrl %08x\n", minor, ejtag_table[minor].ctrl);

    /* no more transmissions */
    ejtag_table[minor].ctrl &= ~DSTRT;
  
    /* read back data */
    ejt_instr(minor,JTAG_DATA_IR);
    data = ejt_data(minor,0);

    /* clear DMACC */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|PRACC|JTS_WORD;
    ejt_instr(minor,JTAG_CONTROL_IR);
    ejt_data(minor,ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    sti();
    return data ;
}


/* scan in the instructions to do a pi word read */
static unsigned int ejt_dmaread_h(int minor,unsigned int addr)
{
    unsigned int data ;

    cli();
    /* scan in address */
    ejt_instr(minor,JTAG_ADDRESS_IR);
    ejt_data(minor,addr);

    /* set up ctrl reg to do a DMA word read, no checks for DSTRT since this interface
     * is very slow!
     */
    /* clear old DMA size,lock flag, proc & periph resets */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|DMAACC|DSTRT|JTS_HALFWORD|DRWN|TOF|PRACC ;
    ejt_instr(minor,JTAG_CONTROL_IR);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread with ctrl    %08x\n", minor, ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl=ejt_data(minor,ejtag_table[minor].ctrl);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread returns ctrl %08x\n", minor, ejtag_table[minor].ctrl);

    /* no more transmissions */
    ejtag_table[minor].ctrl &= ~DSTRT;
  
    /* read back data */
    ejt_instr(minor,JTAG_DATA_IR);
    data = ejt_data(minor,0);

    /* clear DMACC */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|PRACC|JTS_HALFWORD;
    ejt_instr(minor,JTAG_CONTROL_IR);
    ejt_data(minor,ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    sti();

    /* handle the bigendian/littleendian */
    if ( ejtag_table[minor].flags&EJTAG_BIGEND )
        if ( addr & 0x2 )
            data = (data&0x0000ffff) ;
        else
            data = (data>>16)&0xffff ;
    else
        if ( addr & 0x2 )
            data = (data>>16)&0xffff ;
        else
            data = (data&0x0000ffff) ;

    return data ;
}

/* scan in the instructions to do a pi word read */
static unsigned int ejt_dmaread_b(int minor,unsigned int addr)
{
    unsigned int  byte_loc ;
    unsigned int data ;

    cli();
    /* scan in address */
    ejt_instr(minor,JTAG_ADDRESS_IR);
    ejt_data(minor,addr);

    /* set up ctrl reg to do a DMA word read, no checks for DSTRT since this interface
     * is very slow!
     */
    /* clear old DMA size,lock flag, proc & periph resets */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|DMAACC|DSTRT|JTS_BYTE|DRWN|TOF|PRACC ;
    ejt_instr(minor,JTAG_CONTROL_IR);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread with ctrl    %08x\n", minor, ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl=ejt_data(minor,ejtag_table[minor].ctrl);
    if ( ejtag_table[minor].debug )
        printk(KERN_DEBUG "ejtag%d: dmaread returns ctrl %08x\n", minor, ejtag_table[minor].ctrl);

    /* no more transmissions */
    ejtag_table[minor].ctrl &= ~DSTRT;
  
    /* read back data */
    ejt_instr(minor,JTAG_DATA_IR);
    data = ejt_data(minor,0);

    /* clear DMACC */
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_table[minor].ctrl |= PROBEN|PRACC|JTS_BYTE;
    ejt_instr(minor,JTAG_CONTROL_IR);
    ejt_data(minor,ejtag_table[minor].ctrl);
    ejtag_table[minor].ctrl &= 0x00FFFE7F&~(DLOCK|TIF|SYNC|DMAACC|DRWN|DSTRT|PRRST|PERRST|JTAGBRK) ;
    sti();

    /* handle the bigendian/littleendian */
    /* convert offset to byte offset 0123 BE = ~ 3210 LE */
    byte_loc = (ejtag_table[minor].flags&EJTAG_BIGEND?~addr:addr)&0x3 ;
    data = (data>>(8*byte_loc))&0xff;

    return data ;
}
#endif

/* scan out the version register */
static unsigned int ejt_version()
{
    ejt_instr(IDCODE);
    return ejt_data(0);
}

/* scan out the implementation register */
static unsigned int ejt_implementation()
{
    ejt_instr(IMPCODE);
    return ejt_data(0);
}

/* scan out the implementation register */
static unsigned int ejt_checkstatus()
{
    unsigned ctrl_orig ;
    ctrl_orig=ejtag_state.ctrl ;
    /* ensure that w0 are 1 and w1's are 0 */
    ejtag_state.ctrl &= 0x00FFFFFF ;
    ejtag_state.ctrl &= ~(TIF|SYNC|DSTRT|PRRST|PERRST|JTAGBRK) ;
    ejtag_state.ctrl |= DEV|PROBEN|PRACC;
    ejt_instr(JTAG_CONTROL_IR);
    ejt_data(ejtag_state.ctrl);
    return ejtag_state.ctrl;
}

int ejtag_switch(unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    int preval ;

    switch ( cmd ) {
    case EJTAG_DEBUG_D3_1:
        pp_write_data(TCK_BIT(1));
        break;
    case EJTAG_DEBUG_D3_0:
        pp_write_data(TCK_BIT(0));
        break;
    case EJTAG_INIT:
        ejt_reset();
        ejt_tapmove(0,0); /* enter the run-testidle state */
        break ;
    case EJTAG_TAPRESET:
        ejt_tapreset();
        ejt_tapmove(0,0); /* enter the run-testidle state */
        break ;
    case EJTAG_TAPMOVE:
        retval= (int)ejt_tapmove(arg&TMS_BIT_MASK,arg&TDO_BIT_MASK);
        break;
    case EJTAG_DATA:
        retval= ejt_data((int)arg);
        break;
    case EJTAG_INSTR:
        retval= (int)ejt_instr((char)arg);
        break;
    case EJTAG_CTRL_REG:
        ejt_instr(JTAG_CONTROL_IR);
        retval= ejt_data((int)arg);
        break;
    case EJTAG_ADDR_REG:
        ejt_instr(JTAG_ADDRESS_IR);
        retval= ejt_data((int)arg);
        break;
    case EJTAG_DATA_REG:
        ejt_instr(JTAG_DATA_IR);
        retval= ejt_data((int)arg);
        break;
#if 0
    case EJTAG_WRITE_WORD:
        retval= ejt_dmawrite_w(minor,*(int*)arg, *((int*)arg+1));
        break;
    case EJTAG_READ_WORD:
        retval= ejt_dmaread_w(minor,*(int*)arg);
        break;
    case EJTAG_WRITE_HWORD:
        retval= ejt_dmawrite_h(minor,*(int*)arg, *((int*)arg+1));
        break;
    case EJTAG_READ_HWORD:
        retval= ejt_dmaread_h(*(int*)arg);
        break;
    case EJTAG_WRITE_BYTE:
        retval= ejt_dmawrite_b(*(int*)arg, *((int*)arg+1));
        break;
    case EJTAG_READ_BYTE:
        retval= ejt_dmaread_b((int*)arg);
        break;
#endif
    case EJTAG_IMPLEMENTATION:
        retval= ejt_implementation();
        break;
    case EJTAG_VERSION:
        retval= ejt_version();
        break;
    case EJTAG_CHECKSTATUS:
        preval= ejtag_state.ctrl ;
        retval= ejt_checkstatus();
        break;
    case EJTAG_PORTWRITE:
        retval= 0 ;
        pp_write_data(arg&0xFF);
        break;
    case EJTAG_PORTREAD:
        retval= pp_read_stat();
        break;
    case EJTAG_BIGENDIAN:
        if (arg)
            ejtag_state.flags |= EJTAG_BIGEND ;
        else
            ejtag_state.flags &= ~EJTAG_BIGEND ;
        break;
    case EJTAG_WRITE_DATAP:
        retval=0;
        pp_write_data(arg&0xff);
        break;
    case EJTAG_WRITE_CTRLP:
        retval=0;
        pp_write_ctrl(arg&0xff);
        break;
    case EJTAG_READ_STATP:
        retval= pp_read_stat();
        break;
    default:
        retval = -1;
    }
    return retval;
}

