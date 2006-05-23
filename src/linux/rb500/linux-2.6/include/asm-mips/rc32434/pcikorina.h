/* $Id: pciacacia.h,v 1.5 2001/05/01 10:09:17 carstenl Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef _PCIKORINA_H
#define _PCIKORINA_H


#define PCI_MSG_VirtualAddress	     0xB8088010
#define rc32434_pci ((volatile PCI_t) PCI0_VirtualAddress)
#define rc32434_pci_msg ((volatile PCIM_t) PCI_MSG_VirtualAddress)

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
#define KORINA_CONFIG0_ADDR	0x80000000
#define KORINA_CONFIG1_ADDR	0x80000004
#define KORINA_CONFIG2_ADDR	0x80000008
#define KORINA_CONFIG3_ADDR	0x8000000C
#define KORINA_CONFIG4_ADDR	0x80000010
#define KORINA_CONFIG5_ADDR	0x80000014
#define KORINA_CONFIG6_ADDR	0x80000018
#define KORINA_CONFIG7_ADDR	0x8000001C
#define KORINA_CONFIG8_ADDR	0x80000020
#define KORINA_CONFIG9_ADDR	0x80000024
#define KORINA_CONFIG10_ADDR	0x80000028
#define KORINA_CONFIG11_ADDR	0x8000002C
#define KORINA_CONFIG12_ADDR	0x80000030
#define KORINA_CONFIG13_ADDR	0x80000034
#define KORINA_CONFIG14_ADDR	0x80000038
#define KORINA_CONFIG15_ADDR	0x8000003C
#define KORINA_CONFIG16_ADDR	0x80000040
#define KORINA_CONFIG17_ADDR	0x80000044
#define KORINA_CONFIG18_ADDR	0x80000048
#define KORINA_CONFIG19_ADDR	0x8000004C
#define KORINA_CONFIG20_ADDR	0x80000050
#define KORINA_CONFIG21_ADDR	0x80000054
#define KORINA_CONFIG22_ADDR	0x80000058
#define KORINA_CONFIG23_ADDR	0x8000005C
#define KORINA_CONFIG24_ADDR	0x80000060
#define KORINA_CONFIG25_ADDR	0x80000064
#define KORINA_CMD 		(PCFG04_command_ioena_m | \
				 PCFG04_command_memena_m | \
				 PCFG04_command_bmena_m | \
				 PCFG04_command_mwinv_m | \
				 PCFG04_command_parena_m | \
				 PCFG04_command_serrena_m )

#define KORINA_STAT		(PCFG04_status_mdpe_m | \
				 PCFG04_status_sta_m  | \
				 PCFG04_status_rta_m  | \
				 PCFG04_status_rma_m  | \
				 PCFG04_status_sse_m  | \
				 PCFG04_status_pe_m)

#define KORINA_CNFG1		((KORINA_STAT<<16)|KORINA_CMD)

#define KORINA_REVID		0
#define KORINA_CLASS_CODE	0
#define KORINA_CNFG2		((KORINA_CLASS_CODE<<8) | \
				  KORINA_REVID)

#define KORINA_CACHE_LINE_SIZE	4
#define KORINA_MASTER_LAT	0x3c
#define KORINA_HEADER_TYPE	0
#define KORINA_BIST		0

#define KORINA_CNFG3 ((KORINA_BIST << 24) | \
		      (KORINA_HEADER_TYPE<<16) | \
		      (KORINA_MASTER_LAT<<8) | \
		      KORINA_CACHE_LINE_SIZE )

#define KORINA_BAR0	0x00000008 /* 128 MB Memory */
#define KORINA_BAR1	0x18800001 /* 1 MB IO */
#define KORINA_BAR2	0x18000001 /* 2 MB IO window for Acacia
					internal Registers */
#define KORINA_BAR3	0x48000008 /* Spare 128 MB Memory */

#define KORINA_CNFG4	KORINA_BAR0
#define KORINA_CNFG5    KORINA_BAR1
#define KORINA_CNFG6 	KORINA_BAR2
#define KORINA_CNFG7	KORINA_BAR3

#define KORINA_SUBSYS_VENDOR_ID 0
#define KORINA_SUBSYSTEM_ID	0
#define KORINA_CNFG8		0
#define KORINA_CNFG9		0
#define KORINA_CNFG10		0
#define KORINA_CNFG11 	((KORINA_SUBSYS_VENDOR_ID<<16) | \
			  KORINA_SUBSYSTEM_ID)
#define KORINA_INT_LINE		1
#define KORINA_INT_PIN		1
#define KORINA_MIN_GNT		8
#define KORINA_MAX_LAT		0x38
#define KORINA_CNFG12		0
#define KORINA_CNFG13 		0
#define KORINA_CNFG14		0
#define KORINA_CNFG15	((KORINA_MAX_LAT<<24) | \
			 (KORINA_MIN_GNT<<16) | \
			 (KORINA_INT_PIN<<8)  | \
			  KORINA_INT_LINE)
#define	KORINA_RETRY_LIMIT	0x80
#define KORINA_TRDY_LIMIT	0x80
#define KORINA_CNFG16 ((KORINA_RETRY_LIMIT<<8) | \
			KORINA_TRDY_LIMIT)
#define PCI_PBAxC_R		0x0
#define PCI_PBAxC_RL		0x1
#define PCI_PBAxC_RM		0x2
#define SIZE_SHFT		2

#ifdef __MIPSEB__
#define KORINA_PBA0C	( PCIPBAC_mrl_m  | PCIPBAC_sb_m | \
			  ((PCI_PBAxC_RM &0x3) << PCIPBAC_mr_b) | \
			  PCIPBAC_pp_m | \
			  (SIZE_32MB<<SIZE_SHFT) | \
			   PCIPBAC_p_m)
#else
#define KORINA_PBA0C	( PCIPBAC_mrl_m  | \
			  ((PCI_PBAxC_RM &0x3) << PCIPBAC_mr_b) | \
			  PCIPBAC_pp_m | \
			  (SIZE_32MB<<SIZE_SHFT) | \
			   PCIPBAC_p_m)
#endif

#if 0

#define KORINA_PBA0C	( PCIPBAC_sb_m | PCIPBAC_pp_m | \
			  ((PCI_PBAxC_R &0x3) << PCIPBAC_mr_b) | \
			  (SIZE_128MB<<SIZE_SHFT))
#endif
#define KORINA_CNFG17	KORINA_PBA0C
#define KORINA_PBA0M	0x0
#define KORINA_CNFG18	KORINA_PBA0M

#ifdef __MIPSEB__
#define KORINA_PBA1C	((SIZE_1MB<<SIZE_SHFT)  | PCIPBAC_sb_m | \
			  PCIPBAC_msi_m)
#else
#define KORINA_PBA1C	((SIZE_1MB<<SIZE_SHFT)  | \
			  PCIPBAC_msi_m)

#endif

#define KORINA_CNFG19	KORINA_PBA1C
#define KORINA_PBA1M	0x0
#define KORINA_CNFG20	KORINA_PBA1M

#ifdef __MIPSEB__
#define KORINA_PBA2C	((SIZE_2MB<<SIZE_SHFT)  | PCIPBAC_sb_m | \
			  PCIPBAC_msi_m)
#else
#define KORINA_PBA2C	((SIZE_2MB<<SIZE_SHFT) | \
			  PCIPBAC_msi_m)

#endif
#define KORINA_CNFG21	KORINA_PBA2C
#define KORINA_PBA2M	0x18000000
#define KORINA_CNFG22	KORINA_PBA2M
#define KORINA_PBA3C	0
#define KORINA_CNFG23	KORINA_PBA3C
#define KORINA_PBA3M	0
#define KORINA_CNFG24	KORINA_PBA3M



#define	PCITC_DTIMER_VAL	8
#define PCITC_RTIMER_VAL	0x10

					  			  
#endif /* _PCIKORINA_H */

















