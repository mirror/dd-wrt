/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   Some useful macros.
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
 * Initial release based on IDT/Sim (IDT bootloader)
 *
 * 
 *
 **************************************************************************
 */

#ifndef __S434__
#define __S434__
/******************************** D E F I N E S *******************************/

/*
** following defines simple and uniform to save and restore context
** when enrtering and leaving as assemblu language program when memory
** and registers are both premiunm.
*/
#define SAVE_CNTXT  \
  subu  sp, 64;     \
  sw    t0, 60(sp); \
  sw    t1, 56(sp); \
  sw    t2, 52(sp); \
  sw    t3, 48(sp); \
  sw    t4, 44(sp); \
  sw    t5, 40(sp); \
  sw    t6, 36(sp); \
  sw    t7, 32(sp); \
  sw    t8, 28(sp); \
  sw    t9, 24(sp); \
  sw    a0, 20(sp); \
  sw    a1, 16(sp); \
  sw    a2, 12(sp); \
  sw    a3,  8(sp); \
  sw    ra,  4(sp)

#define RSTR_CNTXT  \
  lw    t0, 60(sp); \
  lw    t1, 56(sp); \
  lw    t2, 52(sp); \
  lw    t3, 48(sp); \
  lw    t4, 44(sp); \
  lw    t5, 40(sp); \
  lw    t6, 36(sp); \
  lw    t7, 32(sp); \
  lw    t8, 28(sp); \
  lw    t9, 24(sp); \
  lw    a0, 20(sp); \
  lw    a1, 16(sp); \
  lw    a2, 12(sp); \
  lw    a3,  8(sp); \
  lw    ra,  4(sp); \
  add   sp, 64

/*
** Following define is to specify a maximum value for a software
** busy wait counter.
*/
/*
#define LP_CNT_100NS  1000      
#define LP_CNT_3S     1000000   
*/

/*
** Following are other common timer definitions.
*/
#define DDR_BASE           PHYS_TO_K1(0x18018000)
#define TIMER_BASE        PHYS_TO_K1(0x18028000)  
#define WTC_BASE          PHYS_TO_K1(0x18030000)
#define INTERRUPT_BASE    PHYS_TO_K1(0x18038000)
#define GPIO_BASE         PHYS_TO_K1(0x18050000)

#define TIMEOUT_COUNT     0x00000FFF
#define ENABLE_TIMER      0x1
#define DISABLE_TIMER     0x0
#define BIG_VALUE         0xFFFFFFFF

/*
** following few lines define a macro DISPLAY
** which is used to write a set of 4 characters
** onto the EB434 LED.
*/

#ifndef LED_BASE

#define LED_BASE    PHYS_TO_K1(0x19000000)
#define LED_DIGIT0  0x7
#define LED_DIGIT1  0x6
#define LED_DIGIT2  0x5
#define LED_DIGIT3  0x4
#define LED_CLEAR   0x0

#endif

#define DISPLAY(d0, d1, d2, d3)     \
        li    t6, LED_BASE                    ;\
        lb    t7, LED_CLEAR(t6)               ;\
              nop                             ;\
        li    t7, (d0) & 0xff                 ;\
        sb    t7, LED_DIGIT0(t6)              ;\
        li    t7, (d1) & 0xff                 ;\
        sb    t7, LED_DIGIT1(t6)              ;\
        li    t7, (d2) & 0xff                 ;\
        sb    t7, LED_DIGIT2(t6)              ;\
        li    t7, (d3) & 0xff                 ;\
        sb    t7, LED_DIGIT3(t6)

#define LEDCLEAR()              \
        li    t6, LED_BASE                    ;\
        lb    t7, LED_CLEAR(t6)               ;\
              nop

#define DESTRUCTIVE     1
#define NONDESTRUCTIVE  0

#endif
