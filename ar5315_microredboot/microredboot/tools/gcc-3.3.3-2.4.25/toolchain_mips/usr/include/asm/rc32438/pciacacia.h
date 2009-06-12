/* $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/pciacacia.h#1 $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef _PCIACACIA_H
#define _PCIACACIA_H


#define PCI_MSG_VirtualAddress	     0xB8088010
#define rc32438_pci ((volatile PCI_t) PCI0_VirtualAddress)
#define rc32438_pci_msg ((volatile PCIM_t) PCI_MSG_VirtualAddress)

#define PCIM_SHFT		0x6
#define PCIM_BIT_LEN		0x7
#define PCIM_H_EA		0x3
#define PCIM_H_IA_FIX		0x4
#define PCIM_H_IA_RR		0x5
#if 0
#define PCI_ADDR_START		0x13000000
#endif

#define PCI_ADDR_START		0x50000000

#define CPUTOPCI_MEM_WIN	0x02000000
#define CPUTOPCI_IO_WIN		0x00100000
#define PCILBA_SIZE_SHFT	2
#define PCILBA_SIZE_MASK	0x1F
#define SIZE_256MB		0x1C
#define SIZE_128MB		0x1B
#define SIZE_64MB               0x1A
#define SIZE_32MB		0x19
#define SIZE_16MB               0x18
#define SIZE_4MB		0x16
#define SIZE_2MB		0x15
#define SIZE_1MB		0x14
#define ACACIA_CONFIG0_ADDR	0x80000000
#define ACACIA_CONFIG1_ADDR	0x80000004
#define ACACIA_CONFIG2_ADDR	0x80000008
#define ACACIA_CONFIG3_ADDR	0x8000000C
#define ACACIA_CONFIG4_ADDR	0x80000010
#define ACACIA_CONFIG5_ADDR	0x80000014
#define ACACIA_CONFIG6_ADDR	0x80000018
#define ACACIA_CONFIG7_ADDR	0x8000001C
#define ACACIA_CONFIG8_ADDR	0x80000020
#define ACACIA_CONFIG9_ADDR	0x80000024
#define ACACIA_CONFIG10_ADDR	0x80000028
#define ACACIA_CONFIG11_ADDR	0x8000002C
#define ACACIA_CONFIG12_ADDR	0x80000030
#define ACACIA_CONFIG13_ADDR	0x80000034
#define ACACIA_CONFIG14_ADDR	0x80000038
#define ACACIA_CONFIG15_ADDR	0x8000003C
#define ACACIA_CONFIG16_ADDR	0x80000040
#define ACACIA_CONFIG17_ADDR	0x80000044
#define ACACIA_CONFIG18_ADDR	0x80000048
#define ACACIA_CONFIG19_ADDR	0x8000004C
#define ACACIA_CONFIG20_ADDR	0x80000050
#define ACACIA_CONFIG21_ADDR	0x80000054
#define ACACIA_CONFIG22_ADDR	0x80000058
#define ACACIA_CONFIG23_ADDR	0x8000005C
#define ACACIA_CONFIG24_ADDR	0x80000060
#define ACACIA_CONFIG25_ADDR	0x80000064
#define ACACIA_CMD 		(PCFG04_command_ioena_m | \
				 PCFG04_command_memena_m | \
				 PCFG04_command_bmena_m | \
				 PCFG04_command_mwinv_m | \
				 PCFG04_command_parena_m | \
				 PCFG04_command_serrena_m )

#define ACACIA_STAT		(PCFG04_status_mdpe_m | \
				 PCFG04_status_sta_m  | \
				 PCFG04_status_rta_m  | \
				 PCFG04_status_rma_m  | \
				 PCFG04_status_sse_m  | \
				 PCFG04_status_pe_m)

#define ACACIA_CNFG1		((ACACIA_STAT<<16)|ACACIA_CMD)

#define ACACIA_REVID		0
#define ACACIA_CLASS_CODE	0
#define ACACIA_CNFG2		((ACACIA_CLASS_CODE<<8) | \
				  ACACIA_REVID)

#define ACACIA_CACHE_LINE_SIZE	4
#define ACACIA_MASTER_LAT	0x3c
#define ACACIA_HEADER_TYPE	0
#define ACACIA_BIST		0

#define ACACIA_CNFG3 ((ACACIA_BIST << 24) | \
		      (ACACIA_HEADER_TYPE<<16) | \
		      (ACACIA_MASTER_LAT<<8) | \
		      ACACIA_CACHE_LINE_SIZE )

#define ACACIA_BAR0	0x00000008 /* 128 MB Memory */
#define ACACIA_BAR1	0x18800001 /* 1 MB IO */
#define ACACIA_BAR2	0x18000001 /* 2 MB IO window for Acacia
					internal Registers */
#define ACACIA_BAR3	0x48000008 /* Spare 128 MB Memory */

#define ACACIA_CNFG4	ACACIA_BAR0
#define ACACIA_CNFG5    ACACIA_BAR1
#define ACACIA_CNFG6 	ACACIA_BAR2
#define ACACIA_CNFG7	ACACIA_BAR3

#define ACACIA_SUBSYS_VENDOR_ID 0
#define ACACIA_SUBSYSTEM_ID	0
#define ACACIA_CNFG8		0
#define ACACIA_CNFG9		0
#define ACACIA_CNFG10		0
#define ACACIA_CNFG11 	((ACACIA_SUBSYS_VENDOR_ID<<16) | \
			  ACACIA_SUBSYSTEM_ID)
#define ACACIA_INT_LINE		1
#define ACACIA_INT_PIN		1
#define ACACIA_MIN_GNT		8
#define ACACIA_MAX_LAT		0x38
#define ACACIA_CNFG12		0
#define ACACIA_CNFG13 		0
#define ACACIA_CNFG14		0
#define ACACIA_CNFG15	((ACACIA_MAX_LAT<<24) | \
			 (ACACIA_MIN_GNT<<16) | \
			 (ACACIA_INT_PIN<<8)  | \
			  ACACIA_INT_LINE)
#define	ACACIA_RETRY_LIMIT	0x80
#define ACACIA_TRDY_LIMIT	0x80
#define ACACIA_CNFG16 ((ACACIA_RETRY_LIMIT<<8) | \
			ACACIA_TRDY_LIMIT)
#define PCI_PBAxC_R		0x0
#define PCI_PBAxC_RL		0x1
#define PCI_PBAxC_RM		0x2
#define SIZE_SHFT		2

#define ACACIA_PBA0C	( PCIPBAC_mrl_m | PCIPBAC_sb_m | \
			  ((PCI_PBAxC_RM &0x3) << PCIPBAC_mr_b) | \
			  PCIPBAC_pp_m | \
			  (SIZE_128MB<<SIZE_SHFT) | \
			   PCIPBAC_p_m)
#if 0

#define ACACIA_PBA0C	( PCIPBAC_sb_m | PCIPBAC_pp_m | \
			  ((PCI_PBAxC_R &0x3) << PCIPBAC_mr_b) | \
			  (SIZE_128MB<<SIZE_SHFT))
#endif
#define ACACIA_CNFG17	ACACIA_PBA0C
#define ACACIA_PBA0M	0x0
#define ACACIA_CNFG18	ACACIA_PBA0M

#define ACACIA_PBA1C	((SIZE_1MB<<SIZE_SHFT) | PCIPBAC_sb_m | \
			  PCIPBAC_msi_m)

#define ACACIA_CNFG19	ACACIA_PBA1C
#define ACACIA_PBA1M	0x0
#define ACACIA_CNFG20	ACACIA_PBA1M

#define ACACIA_PBA2C	((SIZE_2MB<<SIZE_SHFT) | PCIPBAC_sb_m | \
			  PCIPBAC_msi_m)

#define ACACIA_CNFG21	ACACIA_PBA2C
#define ACACIA_PBA2M	0x18000000
#define ACACIA_CNFG22	ACACIA_PBA2M
#define ACACIA_PBA3C	0
#define ACACIA_CNFG23	ACACIA_PBA3C
#define ACACIA_PBA3M	0
#define ACACIA_CNFG24	ACACIA_PBA3M



#define	PCITC_DTIMER_VAL	8
#define PCITC_RTIMER_VAL	0x10

					  			  
#endif /* _PCIACACIA_H */

















