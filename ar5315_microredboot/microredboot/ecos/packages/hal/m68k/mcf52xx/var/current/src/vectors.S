/*****************************************************************************
vectors.S -- mcf52xx exception vectors
*****************************************************************************/
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif /* CYGPKG_KERNEL */

/****************************************************************************/

        .file   "vectors.S"

/****************************************************************************/

/****************************************************************************/

/*      The mcf52xx core allows us to move the VBR to our RAM vector  table */
/* at cyg_hal_vsr_table provided the table is at a 1MB boundary.            */

/****************************************************************************/
/*      ROM vector table                                                    */

/*      Create the ROM vector table.   We  use  this  table  to  initialize */
/* cyg_hal_vsr_table which we point the VBR to.                             */

        .macro  hw_vsr name
        .long   hw_vsr_\name
        .endm

        .section ".romvec","ax"

        .globl  rom_vsr_table
rom_vsr_table:

        /*   0 - Initial SSP                                                */
        hw_vsr  stack

        /*   1 - Initial PC                                                 */
        hw_vsr  reset

        /*   2-24 - Default exception handlers                              */
        .rept   24-2+1
        hw_vsr  default
        .endr

        /*   25-31 - Autovector interrupts 1-7                              */
        .rept   31-25+1
        hw_vsr  autovec
        .endr

        /*   32-63 - Default exception handlers                             */
        .rept   63-32+1
        hw_vsr  default
        .endr

        /*   64-255 - User interrupt vectors                                */
        .rept   255-64+1
        hw_vsr  interrupt
        .endr

        .equ    rom_vsr_table_size, . - rom_vsr_table

/****************************************************************************/
/*      The default excetpion vector handler                                */

/*      The default handler  for  all  machine  exceptions.   We  save  the */
/* machine state and call the default C VSR handler.  This routine passes a */
/* pointer to the saved state to the  C VSR handler.  The stack pointer  in */
/* the saved state  points to the  byte following the  PC on the  exception */
/* stack (the  sp before  the exception).   The format/vector  word in  the */
/* exception stack contains the vector number.                              */

/*
void hal_default_exception_handler(CYG_WORD vector, HAL_SavedRegisters *regs);
*/

        .text
        .balign 4
hw_vsr_default:

        lea.l   -(16*4)(%sp),%sp            /*   Preserve the entire state. */
        movem.l %d0-%d7/%a0-%a6,(%sp)       /* Allocate   space   for   all */
        lea.l   ((16+1+1)*4)(%sp),%a0       /* registers   (including   the */
        move.l  %a0,(15*4)(%sp)             /* stack pointer).   Write  all */
                                            /* registers   to   the   stack */
                                            /* space.  Write  the  original */
                                            /* stack pointer  value to  the */
                                            /* stack.   The   format/vector */
                                            /* word, sr, and pc are already */
                                            /* on the stack.                */

        move.w  (16*4)(%sp),%d0             /*   Calculate    the    vector */
        and.l   #0x000003fc,%d0             /* number.   The  format/vector */
        lsr.l   #2,%d0                      /* word on  the stack  contains */
                                            /* the vector number.           */

        pea.l   (%sp)                       /*   Pass  a  pointer  to   the */
                                            /* saved state to the exception */
                                            /* handler.                     */

        move.l  %d0,-(%sp)                  /*   Push  the  vector   number */
                                            /* parameter.                   */

        .extern hal_default_exception_handler   /*   Call    the    default */
        jbsr    hal_default_exception_handler   /* exception   VSR.    This */
                                                /* routine     may     (and */
                                                /* probably  will)   modify */
                                                /* the exception context.   */

        addq.l  #2*4,%sp                    /*   Remove the  vector  number */
                                            /* and the  state pointer  from */
                                            /* the stack.                   */

                                            /*   Restore the state.   There */
                                            /* is a chance that a  debugger */
                                            /* changed the state (including */
                                            /* the   stack   pointer,   PC, */
                                            /* etc.).   We  must  be   very */
                                            /* careful to  restore the  new */
                                            /* state     without      first */
                                            /* overwriting  the  values  on */
                                            /* the stack.  We must copy the */
                                            /* format/vector word, SR,  and */
                                            /* PC to the new stack, but  we */
                                            /* must make sure that the  new */
                                            /* stack is not  in the  middle */
                                            /* of   our    current    stack */
                                            /* variables that we are using. */

        movem.l (%sp),%d0-%d7/%a0-%a4       /*   Restore   all    of    the */
                                            /* registers  that  we  do  not */
                                            /* need in the following  code. */
                                            /* We will  copy all  registers */
                                            /* that are  not restored  here */
                                            /* to  the  new  stack   before */
                                            /* restoring them.              */

        move.l  (15*4)(%sp),%a6             /*   Load the  address  of  the */
                                            /* new SP.                      */

        lea.l   (18*4)(%sp),%a5             /*   Get  a   pointer  to   the */
                                            /* location    following    the */
                                            /* exception context.           */

        cmp.l   %a5,%a6                     /*   Compare  the   new   stack */
        jcc     1f   /*jcc=jhs*/            /* address to the  end  of  the */
                                            /* exception   context.    This */
                                            /* will tell us the order  that */
                                            /* we   need   to   copy    the */
                                            /* exception  stack   and   the */
                                            /* remaining registers from the */
                                            /* exception context to the new */
                                            /* stack.    The    order    is */
                                            /* important because the  stack */
                                            /* frames might  (and  in  many */
                                            /* cases do) overlap.           */

                                            /*   The new SP  is at a  lower */
                                            /* address than the end of  the */
                                            /* exception   context.    Copy */
                                            /* from the  lowest address  to */
                                            /* the highest address.         */

        lea.l   -5*4(%a6),%a6               /*   Copy A5, A6, FVW, SR,  and */
        move.l  -5*4(%a5),(%a6)             /* PC from the old stack to the */
        move.l  -4*4(%a5),1*4(%a6)          /* new stack.  Note that we  do */
        move.l  -2*4(%a5),3*4(%a6)          /* not copy the SP location but */
        move.l  -1*4(%a5),4*4(%a6)          /* we leave a  space for it  on */
                                            /* the new stack.  If we do not */
                                            /* leave  space  for  the   SP, */
                                            /* there is  a  possibility  of */
                                            /* overwriting  some   of   our */
                                            /* data.  Note that we copy  in */
                                            /* increasing order.            */

        move.l  %a6,%sp                     /*   A6 points to  the  top  of */
        move.l  (%sp)+,%a5                  /* the  new   stack  with   our */
        move.l  (%sp)+,%a6                  /* registers  on  it.   Restore */
        addq.l  #1*4,%sp                    /* the remaining registers  and */
        rte                                 /* use the  exception stack  to */
                                            /* return.  Note  that we  also */
                                            /* remove the unused space left */
                                            /* for the SP.                  */

1:

        move.l  -(%a5),-(%a6)               /*   The new SP is at a  higher */
        move.l  -(%a5),-(%a6)               /* or the same  address as  the */
        subq.l  #4,%a5                      /* end   of    the    exception */
        move.l  -(%a5),-(%a6)               /* context.   Copy   from   the */
        move.l  -(%a5),-(%a6)               /* highest   address   to   the */
                                            /* lowest address.   Note  that */
                                            /* we do  not  copy  the  stack */
                                            /* pointer.   When  copying  in */
                                            /* this direction, there is  no */
                                            /* reason to  leave  space  for */
                                            /* the stack pointer.           */

        move.l  %a6,%sp                     /*   A6 points to  the  top  of */
        move.l  (%sp)+,%a5                  /* the  new   stack  with   our */
        move.l  (%sp)+,%a6                  /* registers  on  it.   Restore */
        rte                                 /* the remaining registers  and */
                                            /* use the  exception stack  to */
                                            /* return.                      */

/****************************************************************************/
/*      User interrupt vector handler                                       */

/*      Control is transferred here from a user interrupt vector (#64-255). */

/*      Before branching  to common  code, load  a value  to translate  the */
/* vector table offset to the ISR  table  offset.   The  ISR  vector  table */
/* contains  the  autovectors  (0-6)  followed  by  the  interrupt  vectors */
/* (7-198).                                                                 */

        .equ      int_pres_regs_sz,((2+3)*4)
        .macro    int_pres_regs
        lea.l     -int_pres_regs_sz(%sp),%sp
        movem.l   %d0-%d2/%a0-%a1,(%sp)
        .endm
        .macro    int_rest_regs
        movem.l   (%sp),%d0-%d2/%a0-%a1
        lea.l     int_pres_regs_sz(%sp),%sp
        .endm

        .text
        .balign 4
hw_vsr_interrupt:

        int_pres_regs                       /*   Preserve   all   registers */
                                            /* that this ISR routine  needs */
                                            /* to  preserve.   The  C  code */
                                            /* will  preserve   all   other */
                                            /* registers.                   */

        move.l #(-64+7)*4,%d0               /*   Adding this  value to  the */
                                            /* vector  table  offset   will */
                                            /* result in the  corresponding */
                                            /* offset into the ISR table.   */

        /*   Fall through to common code.                                   */

hw_vsr_int_common:                          /*   Common code.               */

        /*   d0.l: Contains a value to translate the vector table offset to */
        /* the ISR table offset.                                            */

        move.w  int_pres_regs_sz(%sp),%d1   /*   Calculate    the    vector */
        and.l   #0x000003fc,%d1             /* offset.   The  format/vector */
                                            /* word on  the stack  contains */
                                            /* the vector number.  Mask off */
                                            /* all unused  bits.   The  bit */
                                            /* position   of   the   vector */
                                            /* number   field   makes    it */
                                            /* automatically multiplied  by */
                                            /* four.                        */

        add.l   %d1,%d0                     /*   Calculate  the  ISR  table */
                                            /* offset.   Add   the   vector */
                                            /* table    offset    to    the */
                                            /* translation value.           */

        asr.l   #2,%d1                      /*   Calculate    the    vector */
                                            /* number  using   the   vector */
                                            /* table offset.                */

        /*   d0.l: Contains the offset into the ISR table.                  */

        /*   d1.l: Contains the vector number.                              */

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

        .extern cyg_scheduler_sched_lock        /*   Lock the scheduler  if */
        addq.l  #1,cyg_scheduler_sched_lock     /* we are using the kernel. */

#endif /* CYGFUN_HAL_COMMON_KERNEL_SUPPORT */

/*      We need to call the following routines.  The isr address, data, and */
/* intr are all from the  ISR table.  interrupt_end is  a C routine and  is */
/* only called if we are using  the  kernel.   regs  points  to  the  saved */
/* registers on the stack.  isr_ret is the return address from isr.  vector */
/* is the vector number.                                                    */

/*
static cyg_uint32 isr(CYG_ADDRWORD vector,
                      CYG_ADDRWORD data)

externC void interrupt_end(cyg_uint32 isr_ret,
                           Cyg_Interrupt *intr,
                           HAL_SavedRegisters  *regs)
*/

        pea     (%sp)                       /*   Push the regs pointer.     */

        .extern cyg_hal_interrupt_objects       /*   Push the  intr  object */
        lea     cyg_hal_interrupt_objects,%a0   /* pointer from the table.  */
        move.l  (%a0,%d0.l),-(%sp)

        .extern cyg_hal_interrupt_data          /*   Push  the  data  value */
        lea     cyg_hal_interrupt_data,%a0      /* from the table.          */
        move.l  (%a0,%d0.l),-(%sp)

        .extern cyg_hal_interrupt_handlers      /*   Get the address of the */
        lea     cyg_hal_interrupt_handlers,%a0  /* ISR from the table.      */
        move.l  (%a0,%d0.l),%a0

        move.l  %d1,-(%sp)                  /*   Push  the  vector   number */
                                            /* parameter.                   */

        jbsr   (%a0)                        /*   Call the ISR.              */

        addq.l  #4*1,%sp                    /*   Remove     the      vector */
                                            /* parameter from the stack.    */

        move.l  %d0,(%sp)                   /*   d0.l contains  the  return */
                                            /* value    from    the    ISR. */
                                            /* Overwrite the data parameter */
                                            /* with the ISR return value to */
                                            /* pass    as    a    parameter */
                                            /* (isr_ret) to  interrupt_end. */
                                            /* The  intr  object  and  regs */
                                            /* parameters are still on  the */
                                            /* stack.                       */

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

        /*   We only need to  call interrupt_end() when  there is a  kernel */
        /* present to do any tidying up.                                    */

        /*   The  interrupt_end  routine  will   call  the   DSRs  and   do */
        /* rescheduling when it  decrements the  scheduler lock  from 1  to */
        /* zero.  In this case, we  do not want  to have interrupts  masked */
        /* while the DSRs run.   Restore the  interrupt mask  to the  value */
        /* prior  to  this  interrupt.    Do  not  completely  unmask   all */
        /* interrupts because this interrupt may be a nested interrupt.  We */
        /* do not want to lower  the interrupt mask  on the lower  priority */
        /* interrupt.                                                       */

        move.w  (4*3)+int_pres_regs_sz+2(%sp),%d2
        move.w  %d2,%sr

        /*   If the interrupt mask was not previously zero, we want to make */
        /* sure that the DSRs to not run and no preemption occurs.  Add the */
        /* value of the previous interrupt mask to the scheduler lock.   If */
        /* the previous mask was  zero, the scheduler  lock will remain  at */
        /* one and the interrupt  end function will  decrement it to  zero. */
        /* Otherwise, we want  to prevent the  interrupt end function  from */
        /* unlocking the scheduler.  We do  this because there is a  chance */
        /* that someone had  interrupts masked with  the scheduler lock  at */
        /* zero.  If a  higher  priority  interrupt  occurs,  we  could  be */
        /* running DSRs and doing preemption with the interrupts masked!    */

        and.l   #0x0700,%d2
        lsr.l   #8,%d2
        add.l   %d2,cyg_scheduler_sched_lock

        .extern interrupt_end               /*   Call the  interrupt_end  C */
        jbsr    interrupt_end               /* routine.  This routine might */
                                            /* preempt    the     currently */
                                            /* running thread.              */

        /*   Now that interrupt end  is  complete,  subtract  the  previous */
        /* interrupt level back out of the scheduler lock.                  */

        sub.l   %d2,cyg_scheduler_sched_lock

#endif

        lea     (4*3)(%sp),%sp              /*   Remove the isr_ret,  intr, */
                                            /* and regs parameters from the */
                                            /* stack.                       */

        int_rest_regs                       /*   Restore   the    preserved */
                                            /* registers  for  the  current */
                                            /* thread.                      */

        rte                                 /*   Restore the SR and PC.     */

/****************************************************************************/
/*      Autovector interrupt vector handler.                                */

/*      Control is transferred here from an interrupt autovector  (#25-31). */

/*      Before branching  to common  code, load  a value  to translate  the */
/* vector table offset to the ISR  table  offset.   The  ISR  vector  table */
/* contains  the  autovectors  (0-6)  followed  by  the  interrupt  vectors */
/* (7-198).                                                                 */

        .text
        .balign 4
hw_vsr_autovec:

        int_pres_regs                       /*   Preserve   all   registers */
                                            /* that this ISR routine  needs */
                                            /* to  preserve.   The  C  code */
                                            /* will  preserve   all   other */
                                            /* registers.                   */

        move.l #(-25+0)*4,%d0               /*   Adding this  value to  the */
                                            /* vector  table  offset   will */
                                            /* result in the  corresponding */
                                            /* offset into the ISR table.   */

        jra     hw_vsr_int_common           /*   Branch into common code.   */

/****************************************************************************/
/*      hw_vsr_reset -- Hardware Reset Vector                               */

/*      We assume that most of the chip selects are configured by the  boot */
/* loader.                                                                  */

        .text
        .balign 4
        .globl  hw_vsr_reset
hw_vsr_reset:

        .globl  __exception_reset           /*   Define the entry point for */
__exception_reset:                          /* the linker.                  */

        move.w  #0x2700,%sr                 /*   Make   sure    that    all */
                                            /* interrupts are masked.       */

        lea     hw_vsr_stack,%sp            /*   Load   the    reset    and */
                                            /* interrupt stack pointer.     */

        lea     0,%fp                       /*   Set up  the initial  frame */
        link    %fp,#0                      /* pointer.                     */

        .extern hal_reset                   /*   Call  the  C  routine   to */
        jbsr    hal_reset                   /* complete the reset process.  */

9:      stop    #0x2000                     /*   If we return, stop.        */
        jra     9b

/****************************************************************************/
/*      Interrupt and reset stack                                           */

/*      WARNING:  Do  not  put  this  in  any  memory  section  that   gets */
/* initialized.  Doing so may cause the C code to initialize its own stack. */

        .section ".uninvar","aw",@nobits

        .balign 16
        .global hw_vsr_stack_bottom
hw_vsr_stack_bottom:
        .skip   0x2000
        .balign 16
        .global hw_vsr_stack
hw_vsr_stack:
        .skip   0x10

/****************************************************************************/

