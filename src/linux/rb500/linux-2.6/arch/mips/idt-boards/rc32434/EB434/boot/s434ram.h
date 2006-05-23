/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   IDT EB434 DDR setup values.
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


#ifndef __S434RAM__
#define __S434RAM__
/******************************** D E F I N E S *******************************/

#define MB32      1
#define MB64      2
#define MB128     3

#define DEV_CTL_BASE        PHYS_TO_K1(0x18010000)  /* device controller regs */
#define DDR_CTL_BASE        PHYS_TO_K1(0x18018010)  /* DDR controller regs */

#define DEV0_BASE           0x1FC00000
#define DEV0_MASK           0xFFC00000
#define DEV1_BASE           0x1A000000
#define DEV1_MASK           0xFE000000
#define DEV2_BASE           0x19000000
#define DEV2_MASK           0xFF000000
#define DEV3_BASE           0x00000000
#define DEV3_MASK           0x00000000

#if MHZ == 100000000
#define DEV0_CTRL           0x04108324              /* 8-bit devices */
#define DEV0_TC             0x00000000
#define DEV1_CTRL           0x04108324              /* 8-bit devices */
#define DEV1_TC             0x00000000
#define DEV2_CTRL           0x04108324              /* 8-bit devices */
#define DEV2_TC             0x00000000
#define DEV3_CTRL           0x0FFFFFF4              /* 8-bit devices */
#define DEV3_TC             0x00001FFF

#elif MHZ == 133000000
#define DEV0_CTRL           0x04108324              /* 8-bit devices */
#define DEV0_TC             0x00000000
#define DEV1_CTRL           0x05208324              /* 8-bit devices */
#define DEV1_TC             0x00000000
#define DEV2_CTRL           0x04108324              /* 8-bit devices */
#define DEV2_TC             0x00000000
#define DEV3_CTRL           0x0FFFFFF4              /* 8-bit devices */
#define DEV3_TC             0x00001FFF

#elif MHZ == 150000000
#define DEV0_CTRL           0x04108324              /* 8-bit devices */
#define DEV0_TC             0x00000000
#define DEV1_CTRL           0x05208324              /* 8-bit devices */
#define DEV1_TC             0x00000000
#define DEV2_CTRL           0x04108324              /* 8-bit devices */
#define DEV2_TC             0x00000000
#define DEV3_CTRL           0x0FFFFFF4              /* 8-bit devices */
#define DEV3_TC             0x00001FFF


#elif MHZ == 175000000
#define DEV0_CTRL           0x04108324              /* 8-bit devices */
#define DEV0_TC             0x00000000
#define DEV1_CTRL           0x05208324              /* 8-bit devices */
#define DEV1_TC             0x00000000
#define DEV2_CTRL           0x04108324              /* 8-bit devices */
#define DEV2_TC             0x00000000
#define DEV3_CTRL           0x0FFFFFF4              /* 8-bit devices */
#define DEV3_TC             0x00001FFF

#elif MHZ == 200000000
#define DEV0_CTRL           0x05208324              /* 8-bit devices */
#define DEV0_TC             0x00000000
#define DEV1_CTRL           0x06308324              /* 8-bit devices */
#define DEV1_TC             0x00000000
#define DEV2_CTRL           0x05208324              /* 8-bit devices */
#define DEV2_TC             0x00000000
#define DEV3_CTRL           0x0FFFFFF4              /* 8-bit devices */
#define DEV3_TC             0x00001FFF
#endif

#define DATA_PATTERN        0xA5A5A5A5
#define RCOUNT              PHYS_TO_K1(0x18028030)

#if DRAMSZ == MB32

#define DDR_BASE_VAL          0x00000000
#define DDR_MASK_VAL          0xFE000000
#define DDR_ABASE_VAL         0x08000000
#define DDR_AMASK_VAL         0x00000000

#if MHZ == 100000000
#define DDRC_VAL_NORMAL       0x82184800
#define DDRC_VAL_AT_INIT      0x02184800
#define DDR_REF_CMP_VAL       0x0000030c
#elif MHZ == 133000000
#define DDRC_VAL_NORMAL       0x82984800
#define DDRC_VAL_AT_INIT      0x02984800
#define DDR_REF_CMP_VAL       0x0000040e
#elif MHZ == 150000000
#define DDRC_VAL_NORMAL       0x82984800
#define DDRC_VAL_AT_INIT      0x02984800
#define DDR_REF_CMP_VAL       0x00000492
#elif MHZ == 175000000
#define DDRC_VAL_NORMAL       0x82994800
#define DDRC_VAL_AT_INIT      0x02994800
#define DDR_REF_CMP_VAL       0x00000516
#elif MHZ == 200000000
#define DDRC_VAL_NORMAL       0x82994800
#define DDRC_VAL_AT_INIT      0x02994800
#define DDR_REF_CMP_VAL       0x00000618
#else
#warning illegal value for MHZ
#endif

#define DDR_REF_CMP_FAST      0x00000080

#define DDR_CUST_NOP          0x0000003F
#define DDR_CUST_PRECHARGE    0x00000033
#define DDR_CUST_REFRESH      0x00000027
#define DDR_LD_MODE_REG       0x00000023
#define DDR_LD_EMODE_REG      0x00000063

/* 
 * All generated addresses for DDR init during custom transactions are shifted
 * by two address lines - see spec for used DDR chip
 */
#define DDR_PRECHARGE_OFFSET  0x00001000  /* 0x0400 - 9-bit page*/
#define DDR_EMODE_VAL         0x00000000  /* 0x0000 */
#define DDR_DLL_RES_MODE_VAL  0x00000584  /* 0x0161 - Reset DLL, CL2.5 */
#define DDR_DLL_MODE_VAL      0x00000184  /* 0x0061 - CL2.5 */

#define DELAY_200USEC         25000       /* not exactly */

#else
#error "unrecognized dram size"
#endif

#endif
