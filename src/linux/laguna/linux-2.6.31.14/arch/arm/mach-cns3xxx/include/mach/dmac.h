/*******************************************************************************
 *
 *  arch/arm/mach-cns3xxx/dmac.h
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#ifndef _CNS3XXX_DMAC_H_
#define _CNS3XXX_DMAC_H_

#define MAX_DMA_CHANNELS	9
#define DMAC_PCM1_PERIPH_ID_0	4
#define DMAC_SPI_PERIPH_ID	8
#define DMAC_PCM_PERIPH_ID_0	9
#define CNS3XXX_DMAC_I2STX_PID	12
#define CNS3XXX_DMAC_I2SRX_PID	13

/* APIs */
int __init dmac_init(void);
extern int dmac_get_channel (int (*handler)(void*), void *handler_args);
extern int dmac_get_channel_ex(int channel, int (*handler) (void *), void *handler_args);
extern int dmac_release_channel(int chan);

extern int dmac_get_event (int chan, int ev);
extern int dmac_release_event (int chan, int ev);

extern uint32_t dmac_create_ctrlval (int src_inc, int s_bpb, int s_dt, int dst_inc, int d_bpb, int d_dt, int swap);
/* enum - reg ? 0 => PL330_FTC, 1 => PL330_CS, 2 => PL330_CPC, 3 => PL330_SA, 
 * 4 => PL330_DA, 5=>PL330_CC, 6 => PL330_LC0, 7 => PL330_LC1
 */
typedef enum {  PL330_FTC =0,
                PL330_CS,
                PL330_CPC,
                PL330_SA,
                PL330_DA,
                PL330_CC,
                PL330_LC0,
                PL330_LC1 
} chregs_t;

extern uint32_t DMAC_READ_CHREGS (int chan, chregs_t reg);

/* Instruction Set */

/******************************************************************************
 *
 * Instruction:  DMAEND
 * Description:
 *	| 7 6 5 4 | 3 2 1 0 |
 *	  0 0 0 0   0 0 0 0
 * Example:
 * 	DMAEND
 *	00
 ******************************************************************************/
int DMAC_DMAEND(int ch_num);

/******************************************************************************
 *
 * Instruction:  DMAFLUSHP
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <periph[4:0]   >  0  0  0    0  0  1  1    0  1  0  1
 * Example:
 * 	DMAFLUSHP P0
 *	35 00
 ******************************************************************************/
#define DMAFLUSHP_INSTR_SIZE    2
int DMAC_DMAFLUSHP(int ch_num, int periph);

/******************************************************************************
 *
 * Instruction:  DMAGO
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	   0  0  0  0    0 <cn[2:0]>   1  0  1  0    0  0 ns  0
 *
 *	| 47                                                 16 |
 *        <                     imm[31:0]                     >
 * Example:
 * 	DMAGO  C0, 0x40000000
 *	A0 00 00 00 00 40
 ******************************************************************************/
int DMAC_DMAGO(int ch_num);

/******************************************************************************
 *
 * Instruction:  DMALD
 * Description:
 *	| 7 6 5 4 |  3  2  1 0 |
 *	  0 0 0 0    0  1 bs x
 * Example:
 * 	DMALD
 *	04
 ******************************************************************************/
#define DMALD_INSTR_SIZE    1
#define DMALDB_INSTR_SIZE   1
#define DMALDS_INSTR_SIZE   1
int DMAC_DMALD(int ch_num);

int DMAC_DMALDB(int ch_num);

int DMAC_DMALDS(int ch_num);

/******************************************************************************
 *
 * Instruction:  DMALP
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <       iter[7:0]       >    0  0  1  0    0  0 lc  0
 * Example:
 * 	DMALP 8
 *	20 07
 ******************************************************************************/
#define DMALP_INSTR_SIZE    2
int DMAC_DMALP(int ch_num, int loop_reg_idx, int iter);

/******************************************************************************
 *
 * Instruction:  DMALPEND
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <  backwards_jump[7:0]  >    0  0  1 nf    1 lc bs  x
 * Example:
 * 	DMALPEND
 *	38 04
 ******************************************************************************/
#define DMALPEND_INSTR_SIZE     2
int DMAC_DMALPEND(int ch_num, int loop_reg_idx, int jump, int lpfe);

/******************************************************************************
 *
 * Instruction:  DMAMOV
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	   0  0  0  0    0 <rd[2:0]>   1  0  1  1    1  1  0  0
 *
 *	| 47                                                 16 |
 *        <                     imm[31:0]                     >
 *
 *      # CCR Description
 *      # [30:28]  Endian swap size
 *      # [27:25]  AWCACHE[3,1:0] value
 *      # [24:22]  AWPROT value
 *      # [21:18]  AWLEN value
 *      # [17:15]  AWSIZE value
 *      # [14]     AWBURST[0] value
 *                 0 - FIXED / 1 - INCR
 *      # [13:11]  ARCACHE[2:0] value
 *      # [10:8]   ARPROT value
 *      # [7:4]    ARLEN value
 *      # [3:1]    ARSIZE value
 *      # [0]      ARBURST[0] value
 *                 0 - FIXED / 1 - INCR
 * Example:
 * 	DMAMOV   CCR, SB1 SS32 DB1 DS32
 *	BC 01 05 40 01 00
 ******************************************************************************/

#define DMAMOV_INSTR_SIZE   6
/* ccr_sar_dar: 0 for SAR, 1, for CCR, 2 for DAR */
typedef enum { SAR = 0, CCR = 1, DAR = 2 } dmamov_arg_t;
int DMAC_DMAMOV(int ch_num, dmamov_arg_t ccr_sar_dar, uint32_t value);

#define DMAWMB_INSTR_SIZE  1
int DMAC_DMAWMB (int ch_num);

#define DMANOP_INSTR_SIZE 1
int DMAC_DMANOP (int ch_num);
/******************************************************************************
 *
 * Instruction:  DMAST
 * Description:
 *	| 7 6 5 4 |  3  2  1 0 |
 *	  0 0 0 0    1  0 bs x
 * Example:
 * 	DMAST
 *	08
 ******************************************************************************/
#define DMAST_INSTR_SIZE    1 /* 1 Byte */
int DMAC_DMAST(int ch_num);

#define DMASTB_INSTR_SIZE   1 /* 1 Byte */
int DMAC_DMASTB(int ch_num);

#define DMASTS_INSTR_SIZE   1 /* 1 Byte */
int DMAC_DMASTS(int ch_num);

/******************************************************************************
 *
 * Instruction:  DMASTZ
 * Description:
 *	| 7 6 5 4 |  3  2  1 0 |
 *	  0 0 0 0    1  1  0 0
 * Example:
 * 	DMASTZ
 *	08
 ******************************************************************************/
/* Not done */

/******************************************************************************
 *
 * Instruction:  DMAWFE
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <event_num[4:0]>  0  i  0    0  0  1  1    0  1  1  0
 * Example:
 * 	DMAWFE E0
 *	36 00
 ******************************************************************************/
int DMAC_WFE(int ch_num, int event);
#define DMAWFE_INSTR_SIZE  2

/******************************************************************************
 *
 * Instruction:  DMAWFP
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <  periph[4:0] >  0  0  0    0  0  1  1    0  0 bs  p
 * Example:
 * 	DMAWFP P0, periph
 *	31 00
 ******************************************************************************/
typedef enum { SINGLE = 0, BURST = 1, PERIPHERAL = 2} dmawfp_burst_type;
int DMAC_DMAWFP(int ch_num, int periph_id,dmawfp_burst_type b);
#define DMAWFP_INSTR_SIZE  2

/******************************************************************************
 *
 * Instruction:  DMAKILL
 * Description:
 *	| 7 6 5 4 | 3 2 1 0 |
 *	  0 0 0 0   0 0 0 1
 * Example:
 * 	DMAKILL
 *	01
 ******************************************************************************/

/******************************************************************************
 *
 * Instruction:  DMASEV
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <event_num[4:0]>  0  i  0    0  0  1  1    0  1  0  0
 * Example:
 * 	DMASEV E0
 *	34 00
 ******************************************************************************/
int DMAC_DMASEV(int ch_num, int event_num);
#define DMASEV_INSTR_SIZE   2

/******************************************************************************
 *
 * Instruction:  DMALDP<S|B>
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <  periph[4:0] >  0  0  0    0  0  1  0    0  1 bs  1
 * Example:
 * 	DMALDPS P0
 *	25 00
 ******************************************************************************/
int DMAC_DMALDP(int ch_num, int periph_id, int burst);
#define DMALDP_INSTR_SIZE 2

/******************************************************************************
 *
 * Instruction:  DMASTP<S|B>
 * Description:
 *	| 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0 |
 *	  <  periph[4:0] >  0  0  0    0  0  1  0    1  0 bs  1
 * Example:
 * 	DMASTPS P0
 *	29 00
 ******************************************************************************/
int DMAC_DMASTP(int ch_num, int periph_id, int burst);
#define DMASTP_INSTR_SIZE 2

#endif
