/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   Board initialization code.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 rkt
 *
 * Initial release based on csu_idt.S from IDT/Sim (IDT bootloader)
 *
 * 
 *
 **************************************************************************
 */
	
#include <linux/config.h>
#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/offset.h>
#include <asm/cachectl.h>

#define MHZ CONFIG_IDT_BOARD_FREQ
		
#include "idthdr.h"
#include "iregdef.h"
#include "idtcpu.h"
#include "s434ram.h"
#include "s434.h"

#define IndexInvalidate_I       0x00

/*--------------------------------------------------------------
** prom entry point table
*-------------------------------------------------------------*/

FRAME(start,sp,0,ra)
  j idtstart        /* begin monitor from start       |00| */

idtstart:

  .set  noreorder
      mtc0  zero, C0_CAUSE
            nop
      li    v0, 0x0
      or    v0, (SR_CU0 | SR_BEV)
      mtc0  v0, C0_SR
            nop
      mfc0  v1, C0_CONFIG
            nop
      and   v1, ~(0x7)
      ori   v1, 0x3
      mtc0  v1, C0_CONFIG
            nop
            nop
            
/* ------------------- Disable WatchDog Timer ----------------------------- */
      li    t0, WTC_BASE
      sw    zero, 0x3C(t0) /* WTC */
/* ------ Alternate functions for GPIO pins --------------------------------*/
/* only UART0, UART1 and mem_addr */
      li    t0, GPIO_BASE
      li    t1, 0xf3
      sw    t1, 0x0(t0)

#if !defined(PCISAT)
/* ------------------- Assert PCI reset ----------------------------------- */
			li    t0, 0xb8080000
      lw    t1, (t0)
      andi  t2, t1, 0x1
      beqz  t2, 2f
            nop
      andi  t2, t1, 0x3fe
      sw    t2, (t0)
      li    t2, 0x1000
1:
      addi  t2, -1
      bnez  t2, 1b
            nop
2:
      ori   t2, t1, 0x1
      sw    t2, (t0)
      lui   t2, 0x2
rip:
      lw    t1, 4(t0)
      and   t1, t1, t2
      bnez  t1, rip
            nop
#endif

/* ------------------- Setup Device Controller ---------------------------- */
      li    t0, DEV_CTL_BASE      /* load 2 base address registers' base    */
      li    t1, DEV0_CTRL         /* device0 control parameter              */
      sw    t1, 0x8(t0)           /* set the control register  CS0          */
      li    t1, DEV0_TC           /* device0 timing config parameter        */
      sw    t1, 0xC(t0)
      li    t1, DEV1_BASE         /* set the device base register for CS1   */
      sw    t1, 0x10(t0)
      li    t1, DEV1_MASK         /* set the device mask register for CS1   */
      sw    t1, 0x14(t0) 
      li    t1, DEV1_CTRL         /* set the device control register for CS1*/
      sw    t1, 0x18(t0)
      li    t1, DEV1_TC           /* set the device timing register for CS1 */
      sw    t1, 0x1C(t0) 

      li    t1, DEV2_BASE         /* set the device base register for CS1   */
      sw    t1, 0x20(t0)
      li    t1, DEV2_MASK         /* set the device mask register for CS1   */
      sw    t1, 0x24(t0) 
      li    t1, DEV2_CTRL         /* set the device control register for CS1*/
      sw    t1, 0x28(t0)
      li    t1, DEV2_TC           /* set the device timing register for CS1 */
      sw    t1, 0x2C(t0) 

      li    t1, DEV3_BASE         /* set the device base register for CS1   */
      sw    t1, 0x30(t0)
      li    t1, DEV3_MASK         /* set the device mask register for CS1   */
      sw    t1, 0x34(t0) 
      li    t1, DEV3_CTRL         /* set the device control register for CS1*/
      sw    t1, 0x38(t0)
      li    t1, DEV3_TC           /* set the device timing register for CS1 */
      sw    t1, 0x3C(t0) 

      DISPLAY('I','D','D','R')
#if MEMCFG != SRAM_ONLY

/* ------------- INITIALIZE DDR SDRAM CONTROLLER ---------------------------*/

      li    t1, 0x0               /* Add 200 microseconds of delay */
      li    t2, DELAY_200USEC
1:
      add   t1, 1
      bne   t1, t2, 1b
            nop

/*-------------- Initialize DDR Base and Mask Registers --------------------*/

      li    t0, DDR_BASE

  /* Load the DDRC, reset  Refresh Enable */
      li    t1, DDRC_VAL_AT_INIT
      sw    t1, 0x10(t0)
      
      sw    zero, 0x4(t0)
      sw    zero, 0xc(t0)
      sw    zero, 0x18(t0)

  /* Store DDRBASE */
      li    t1, DDR_BASE_VAL
      sw    t1, 0x0(t0)

  /* Store DDRMASK */
      li    t1, DDR_MASK_VAL
      sw    t1, 0x4(t0)

  /* Store DDRABASE */
      li    t1, DDR_ABASE_VAL
      sw    t1, 0x14(t0)

  /* Load DDRAMASK to disable alternate Mapping */
      li    t1, DDR_AMASK_VAL
      sw    t1, 0x18(t0)

      li    t1, DDR_CUST_NOP      /* Write to DDR Custom transaction register */
      sw    t1, 0x20(t0)

      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Add 200 microseconds of delay */
      li    t1, 0x0
      li    t2, DELAY_200USEC
1:
      add   t1, 1
      bne   t1, t2, 1b
            nop
            
  /* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
      li    t1, DDR_CUST_PRECHARGE
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Generate A10 high to pre-charge both the banks */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_PRECHARGE_OFFSET | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
      li    t1, DDR_LD_EMODE_REG
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Generate EMODE register contents on A15-A2 */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_EMODE_VAL | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
      li    t1, DDR_LD_MODE_REG
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Generate Mode register contents on the address bus A15-A2  */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_DLL_RES_MODE_VAL | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Delay of  1.6 microseconds ~ 300 delay iteration value */
      li    t1, 0x0
      li    t2, 500
1:
      add   t1, 1
      bne   t1, t2, 1b
            nop

  /* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
      li    t1, DDR_CUST_PRECHARGE
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Generate A10 high to pre-charge both the banks */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_PRECHARGE_OFFSET | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Implements 9 cycles of Auto refresh allowing
     sufficient margin for stability*/
      li    t4, 9
      li    t3, 0
1:
      li    t1, DDR_CUST_REFRESH
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Read it back to flush CPU write buffers */
      lw    t1, 0x20(t0)

  /* Access DDR */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_BASE_VAL
      sw    t2, 0x0(t1)

      add   t3, 1
      bne   t3, t4, 1b
            nop

  /* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
      li    t1, DDR_LD_MODE_REG
      sw    t1, 0x20(t0)    /* Write to DDR Custom transaction register */

  /* Generate Mode Register contents on the address bus A12-A0 */
      li    t2, DATA_PATTERN
      li    t1, 0xA0000000 | DDR_DLL_MODE_VAL | DDR_BASE_VAL
      sw    t2, 0x0(t1)

  /* Initialize the refresh timer with fast refresh count */
      li    t0, RCOUNT
      li    t1, DDR_REF_CMP_FAST
      
  /* Set the RCOMPARE register */
      sw    t1, 0x4(t0)

  /* Enable the Refresh timer */
      li    t1, 0x1           /* CE set to enabled the  Refresh counter */
      sw    t1, 0x8(t0)

  /* Enable RE-refresh enable in the DDRC register */
      li    t0, DDR_BASE
      li    t1, DDRC_VAL_NORMAL
      sw    t1, 0x10(t0)

  /* Add 200 microseconds of delay */
      li    t1, 0x0
      li    t2, DELAY_200USEC
1:
      add   t1, 1
      bne   t1, t2, 1b
            nop

      li    t0, RCOUNT

  /* Disable the refresh counter before changing the compare value */
      sw    zero, 0x8(t0)

  /* Set the RCOMPARE register with value gotten above */
      li    t3, DDR_REF_CMP_VAL
      sw    t3, 0x4(t0)

  /* Enable the Refresh timer */
      li    t1, 0x1           /* CE set to enabled the  Refresh counter */
      sw    t1, 0x8(t0)

  /* Add 200 microseconds of delay */
      li    t1, 0x0
      li    t2, DELAY_200USEC
1:
      add   t1, 1
      bne   t1, t2, 1b
            nop

#endif
	li    t0, 0xa0000000
	li    t1, 0xa0100000
1:
	sw    zero, 0x00(t0)
	sw    zero, 0x04(t0)
	sw    zero, 0x08(t0)
	sw    zero, 0x0c(t0)
	addiu t0, 16
	nop
	blt   t0, t1, 1b
	nop
	nop
	nop
3:
	mfc0  t0, C0_SR
	nop
	nop
	and   t0, ~SR_BEV
	mtc0  t0, C0_SR
	nop
	nop

4:	
	
/* Jump to zImage startup */
	        
	la     k0, zstartup
	j      k0
	nop
	nop

ENDFRAME(start)
