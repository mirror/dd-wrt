/***********************license start***************
 * Copyright (c) 2003-2015  Cavium, Inc. <support@cavium.com>.  All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to PCIe as a host(RC) or target(EP)
 *
 * <hr>$Revision: 170015 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-ciu-defs.h>
#include <asm/octeon/cvmx-dtx-defs.h>
#include <asm/octeon/cvmx-dpi-defs.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#include <asm/octeon/cvmx-mio-defs.h>
#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-npei-defs.h>
#include <asm/octeon/cvmx-pci-defs.h>
#include <asm/octeon/cvmx-pcieepx-defs.h>
#include <asm/octeon/cvmx-pciercx-defs.h>
#include <asm/octeon/cvmx-pemx-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-pescx-defs.h>
#include <asm/octeon/cvmx-rst-defs.h>
#include <asm/octeon/cvmx-sli-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-helper-jtag.h>
#include <linux/of.h>
#include <linux/moduleparam.h>

#ifdef CONFIG_CAVIUM_DECODE_RSL
#include <asm/octeon/cvmx-error.h>
#endif
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-errata.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-pcie.h>
#include <asm/octeon/cvmx-sysinfo.h>
#include <asm/octeon/cvmx-swap.h>
#include <asm/octeon/cvmx-wqe.h>
#else
#include "cvmx.h"
#include "cvmx-csr-db.h"
#include "cvmx-pcie.h"
#include "cvmx-sysinfo.h"
#include "cvmx-swap.h"
#include "cvmx-wqe.h"
#include "cvmx-error.h"
#include "cvmx-helper-errata.h"
#include "cvmx-qlm.h"
#include "cvmx-bootmem.h"
#include "cvmx-rst-defs.h"
#include "octeon_mem_map.h"
#ifdef __U_BOOT__
# include <libfdt.h>
# include <asm/arch/cvmx-helper-fdt.h>
#else
# include "libfdt/libfdt.h"
# include "libfdt/cvmx-helper-fdt.h"
#endif
#endif

#define MRRS_CN5XXX 0		/* 128 byte Max Read Request Size */
#define MPS_CN5XXX  0		/* 128 byte Max Packet Size (Limit of most PCs) */
#define MRRS_CN6XXX 3		/* 1024 byte Max Read Request Size */
#define MPS_CN6XXX  0		/* 128 byte Max Packet Size (Limit of most PCs) */

/* Endian swap mode. */
#ifdef __BIG_ENDIAN_BITFIELD	/* A Linux compatible proxy for __BIG_ENDIAN */
#define _CVMX_PCIE_ES 1
#else
#define _CVMX_PCIE_ES 0
#endif

#define CVMX_READ_CSR(addr)		cvmx_read_csr_node(node,addr)
#define CVMX_WRITE_CSR(addr,val)	cvmx_write_csr_node(node,addr,val)
#define CVMX_PCIE_CFGX_READ(p,addr)	cvmx_pcie_cfgx_read_node(node,p,addr)
#define CVMX_PCIE_CFGX_WRITE(p,addr,val)	cvmx_pcie_cfgx_write_node(node,p,addr,val)

bool pcie_link_initialized[CVMX_MAX_NODES][CVMX_PCIE_MAX_PORTS];

int cvmx_primary_pcie_bus_number = 1;

/**
 * Return the Core virtual base address for PCIe IO access. IOs are
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_io_base_address(int pcie_port)
{
	cvmx_pcie_address_t pcie_addr;
	pcie_addr.u64 = 0;
	pcie_addr.io.upper = 0;
	pcie_addr.io.io = 1;
	pcie_addr.io.did = 3;
	pcie_addr.io.subdid = 2;
	pcie_addr.io.node = (pcie_port >> 4) & 0x3;
	pcie_addr.io.es = _CVMX_PCIE_ES;
	pcie_addr.io.port = (pcie_port & 0x3);
	return pcie_addr.u64;
}

/**
 * Size of the IO address region returned at address
 * cvmx_pcie_get_io_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return Size of the IO window
 */
uint64_t cvmx_pcie_get_io_size(int pcie_port)
{
	return 1ull << 32;
}

/**
 * Return the Core virtual base address for PCIe MEM access. Memory is
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_mem_base_address(int pcie_port)
{
	cvmx_pcie_address_t pcie_addr;
	pcie_addr.u64 = 0;
	pcie_addr.mem.upper = 0;
	pcie_addr.mem.io = 1;
	pcie_addr.mem.did = 3;
	pcie_addr.mem.subdid = 3 + (pcie_port & 0x3);
	pcie_addr.mem.node = (pcie_port >> 4) & 0x3;
	return pcie_addr.u64;
}

/**
 * Size of the Mem address region returned at address
 * cvmx_pcie_get_mem_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return Size of the Mem window
 */
uint64_t cvmx_pcie_get_mem_size(int pcie_port)
{
	return 1ull << 36;
}

/**
 * @INTERNAL
 * Return the QLM number for the PCIE port.
 *
 * @param  pcie_port  QLM number to return for.
 *
 * @return QLM number.
 */
static int __cvmx_pcie_get_qlm(int node, int pcie_port)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_pemx_qlm_t pem_qlm;
		cvmx_gserx_cfg_t gserx_cfg;
		switch (pcie_port) {
		case 0: /* PEM0 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1: /* PEM1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(0));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 1; /* PEM1 is on QLM 1 */
			else
				return -1; /* PEM1 is disabled */
		case 2: /* PEM2 */
			pem_qlm.u64 = CVMX_READ_CSR(CVMX_PEMX_QLM(2));
			if (pem_qlm.cn73xx.pemdlmsel == 1) {
				gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(5));
				if (gserx_cfg.s.pcie)
					return 5;  /* PEM2 is on DLM5 */
				else
					return -1; /* PEM2 is disabled */
			}
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(2));
			if (gserx_cfg.s.pcie)
				return 2; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		case 3: /* PEM3 */
			pem_qlm.u64 = CVMX_READ_CSR(CVMX_PEMX_QLM(3));
			if (pem_qlm.cn73xx.pemdlmsel == 1) {
				gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(6));
				if (gserx_cfg.s.pcie)
					return 6;  /* PEM2 is on DLM5 */
				else
					return -1; /* PEM2 is disabled */
			}
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(2));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(3));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 3; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		default:
			cvmx_dprintf("Invalid %d PCIe port\n", pcie_port);
			return -2;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_gserx_cfg_t gserx_cfg;
		switch (pcie_port) {
		case 0:
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1: /* PEM1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(0));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 1; /* PEM1 is on QLM 1 */
			else
				return -1; /* PEM1 is disabled */
		case 2: /* PEM2 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(2));
			if (gserx_cfg.s.pcie)
				return 2; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		case 3: /* PEM3 */
			{
				cvmx_gserx_cfg_t gser4_cfg;
				pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(2));
				gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(3));
				gser4_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(4));
				if (pem_cfg.cn78xx.lanes8) {
					if (gser4_cfg.s.pcie)
						return 4;  /* PEM3 is on QLM4 */
					else
						return -1; /* PEM3 is disabled */
				} else {
					if (gserx_cfg.s.pcie)
						return 3; /* PEM3 is on QLM3 */
					else if (gser4_cfg.s.pcie)
						return 4; /* PEM3 is on QLM4 */
					else
						return -1; /* PEM3 is disabled */
				}
			}
		default:
			cvmx_dprintf("Invalid %d PCIe port\n", pcie_port);
			return -1;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		enum cvmx_qlm_mode mode1 = cvmx_qlm_get_mode(1);
		enum cvmx_qlm_mode mode2 = cvmx_qlm_get_mode(2);
		switch (pcie_port) {
		case 0: /* PCIe0 can be DLM1 with 1, 2 or 4 lanes */
			if (mode1 == CVMX_QLM_MODE_PCIE || /* Using DLM 1-2 */
			    mode1 == CVMX_QLM_MODE_PCIE_1X2 || /* Using DLM 1 */
			    mode1 == CVMX_QLM_MODE_PCIE_2X1 || /* Using DLM 1, lane 0 */
			    mode1 == CVMX_QLM_MODE_PCIE_1X1) /* Using DLM 1, lane0, lane 1 not used */
				return 1;
			else
				return -1;
		case 1: /* PCIe1 can be DLM1 1 lane(1), DLM2 1 lane(0) or 2 lanes(0-1) */
			if (mode1 == CVMX_QLM_MODE_PCIE_2X1)
				return 1;
			else if (mode2 == CVMX_QLM_MODE_PCIE_1X2)
				return 2;
			else if (mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return 2;
			else
				return -1;
		case 2: /* PCIe2 can be DLM2 1 lanes(1) */
			if (mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return 2;
			else
				return -1;
		default: /* Only three PEM blocks */
			return -1;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_gserx_cfg_t gserx_cfg;
		switch (pcie_port) {
		case 0:  /* PEM0 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1: /* PEM1 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (gserx_cfg.s.pcie)
				return 1;  /* PEM1 is on DLM1 */
			else
				return -1;  /* PEM1 is disabled */
		default:
			return -1;
		}
	}
	return -1;
}

/**
 * @INTERNAL
 * Initialize the RC config space CSRs
 *
 * @param node      node
 * @param pcie_port PCIe port to initialize
 */
static void __cvmx_pcie_rc_initialize_config_space(int node, int pcie_port)
{
	/* Max Payload Size (PCIE*_CFG030[MPS]) */
	/* Max Read Request Size (PCIE*_CFG030[MRRS]) */
	/* Relaxed-order, no-snoop enables (PCIE*_CFG030[RO_EN,NS_EN] */
	/* Error Message Enables (PCIE*_CFG030[CE_EN,NFE_EN,FE_EN,UR_EN]) */
	{
		cvmx_pciercx_cfg030_t pciercx_cfg030;
		pciercx_cfg030.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG030(pcie_port));
		if (OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
			pciercx_cfg030.s.mps = MPS_CN5XXX;
			pciercx_cfg030.s.mrrs = MRRS_CN5XXX;
		} else {
			pciercx_cfg030.s.mps = MPS_CN6XXX;
			pciercx_cfg030.s.mrrs = MRRS_CN6XXX;
		}
		pciercx_cfg030.s.ro_en = 1;	/* Enable relaxed order processing. This will allow devices to affect read response ordering */
		pciercx_cfg030.s.ns_en = 1;	/* Enable no snoop processing. Not used by Octeon */
		pciercx_cfg030.s.ce_en = 1;	/* Correctable error reporting enable. */
		pciercx_cfg030.s.nfe_en = 1;	/* Non-fatal error reporting enable. */
		pciercx_cfg030.s.fe_en = 1;	/* Fatal error reporting enable. */
		pciercx_cfg030.s.ur_en = 1;	/* Unsupported request reporting enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG030(pcie_port),
				     pciercx_cfg030.u32);
	}

	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		/* Max Payload Size (NPEI_CTL_STATUS2[MPS]) must match PCIE*_CFG030[MPS] */
		/* Max Read Request Size (NPEI_CTL_STATUS2[MRRS]) must not exceed PCIE*_CFG030[MRRS] */
		cvmx_npei_ctl_status2_t npei_ctl_status2;
		npei_ctl_status2.u64 = CVMX_READ_CSR(CVMX_PEXP_NPEI_CTL_STATUS2);
		npei_ctl_status2.s.mps = MPS_CN5XXX;	/* Max payload size = 128 bytes for best Octeon DMA performance */
		npei_ctl_status2.s.mrrs = MRRS_CN5XXX;	/* Max read request size = 128 bytes for best Octeon DMA performance */
		if (pcie_port)
			npei_ctl_status2.s.c1_b1_s = 3;	/* Port1 BAR1 Size 256MB */
		else
			npei_ctl_status2.s.c0_b1_s = 3;	/* Port0 BAR1 Size 256MB */

		CVMX_WRITE_CSR(CVMX_PEXP_NPEI_CTL_STATUS2, npei_ctl_status2.u64);
	} else {
		/* Max Payload Size (DPI_SLI_PRTX_CFG[MPS]) must match PCIE*_CFG030[MPS] */
		/* Max Read Request Size (DPI_SLI_PRTX_CFG[MRRS]) must not exceed PCIE*_CFG030[MRRS] */
		cvmx_dpi_sli_prtx_cfg_t prt_cfg;
		cvmx_sli_s2m_portx_ctl_t sli_s2m_portx_ctl;
		prt_cfg.u64 = CVMX_READ_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port));
		prt_cfg.s.mps = MPS_CN6XXX;
		prt_cfg.s.mrrs = MRRS_CN6XXX;
		/* Max outstanding load request. */
		prt_cfg.s.molr = 32;
		CVMX_WRITE_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port), prt_cfg.u64);

		sli_s2m_portx_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port));
		if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)
		      || OCTEON_IS_MODEL(OCTEON_CN73XX)
		      || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
			sli_s2m_portx_ctl.cn61xx.mrrs = MRRS_CN6XXX;
		CVMX_WRITE_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port), sli_s2m_portx_ctl.u64);
	}

	/* ECRC Generation (PCIE*_CFG070[GE,CE]) */
	{
		cvmx_pciercx_cfg070_t pciercx_cfg070;
		pciercx_cfg070.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG070(pcie_port));
		pciercx_cfg070.s.ge = 1;	/* ECRC generation enable. */
		pciercx_cfg070.s.ce = 1;	/* ECRC check enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG070(pcie_port),
				     pciercx_cfg070.u32);
	}

	/* Access Enables (PCIE*_CFG001[MSAE,ME]) */
	/* ME and MSAE should always be set. */
	/* Interrupt Disable (PCIE*_CFG001[I_DIS]) */
	/* System Error Message Enable (PCIE*_CFG001[SEE]) */
	{
		cvmx_pciercx_cfg001_t pciercx_cfg001;
		pciercx_cfg001.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG001(pcie_port));
		pciercx_cfg001.s.msae = 1;	/* Memory space enable. */
		pciercx_cfg001.s.me = 1;	/* Bus master enable. */
		pciercx_cfg001.s.i_dis = 1;	/* INTx assertion disable. */
		pciercx_cfg001.s.see = 1;	/* SERR# enable */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG001(pcie_port),
				     pciercx_cfg001.u32);
	}

	/* Advanced Error Recovery Message Enables */
	/* (PCIE*_CFG066,PCIE*_CFG067,PCIE*_CFG069) */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG066(pcie_port), 0);
	/* Use CVMX_PCIERCX_CFG067 hardware default */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG069(pcie_port), 0);

	/* Active State Power Management (PCIE*_CFG032[ASLPC]) */
	{
		cvmx_pciercx_cfg032_t pciercx_cfg032;
		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG032(pcie_port));
		pciercx_cfg032.s.aslpc = 0;	/* Active state Link PM control. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG032(pcie_port),
				     pciercx_cfg032.u32);
	}

	/* Link Width Mode (PCIERCn_CFG452[LME]) - Set during
	 * cvmx_pcie_rc_initialize_link()
	 */
	/* Primary Bus Number (PCIERCn_CFG006[PBNUM]) */
	{
		/* We set the primary bus number to 1 so IDT bridges are happy.
		 * They don't like zero
		 */
		cvmx_pciercx_cfg006_t pciercx_cfg006;
		pciercx_cfg006.u32 = 0;
		pciercx_cfg006.s.pbnum = cvmx_primary_pcie_bus_number;
		pciercx_cfg006.s.sbnum = cvmx_primary_pcie_bus_number;
		pciercx_cfg006.s.subbnum = cvmx_primary_pcie_bus_number;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG006(pcie_port),
				     pciercx_cfg006.u32);
	}

	/* Memory-mapped I/O BAR (PCIERCn_CFG008) */
	/* Most applications should disable the memory-mapped I/O BAR by */
	/* setting PCIERCn_CFG008[ML_ADDR] < PCIERCn_CFG008[MB_ADDR] */
	{
		cvmx_pciercx_cfg008_t pciercx_cfg008;
		pciercx_cfg008.u32 = 0;
		pciercx_cfg008.s.mb_addr = 0x100;
		pciercx_cfg008.s.ml_addr = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG008(pcie_port),
				     pciercx_cfg008.u32);
	}

	/* Prefetchable BAR (PCIERCn_CFG009,PCIERCn_CFG010,PCIERCn_CFG011) */
	/* Most applications should disable the prefetchable BAR by setting */
	/* PCIERCn_CFG011[UMEM_LIMIT],PCIERCn_CFG009[LMEM_LIMIT] < */
	/* PCIERCn_CFG010[UMEM_BASE],PCIERCn_CFG009[LMEM_BASE] */
	{
		cvmx_pciercx_cfg009_t pciercx_cfg009;
		cvmx_pciercx_cfg010_t pciercx_cfg010;
		cvmx_pciercx_cfg011_t pciercx_cfg011;
		pciercx_cfg009.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG009(pcie_port));
		pciercx_cfg010.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG010(pcie_port));
		pciercx_cfg011.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG011(pcie_port));
		pciercx_cfg009.s.lmem_base = 0x100;
		pciercx_cfg009.s.lmem_limit = 0;
		pciercx_cfg010.s.umem_base = 0x100;
		pciercx_cfg011.s.umem_limit = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG009(pcie_port),
				     pciercx_cfg009.u32);
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG010(pcie_port),
				     pciercx_cfg010.u32);
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG011(pcie_port),
				     pciercx_cfg011.u32);
	}

	/* System Error Interrupt Enables (PCIERCn_CFG035[SECEE,SEFEE,SENFEE]) */
	/* PME Interrupt Enables (PCIERCn_CFG035[PMEIE]) */
	{
		cvmx_pciercx_cfg035_t pciercx_cfg035;
		pciercx_cfg035.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG035(pcie_port));
		pciercx_cfg035.s.secee = 1;	/* System error on correctable error enable. */
		pciercx_cfg035.s.sefee = 1;	/* System error on fatal error enable. */
		pciercx_cfg035.s.senfee = 1;	/* System error on non-fatal error enable. */
		pciercx_cfg035.s.pmeie = 1;	/* PME interrupt enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG035(pcie_port),
				     pciercx_cfg035.u32);
	}

	/* Advanced Error Recovery Interrupt Enables */
	/* (PCIERCn_CFG075[CERE,NFERE,FERE]) */
	{
		cvmx_pciercx_cfg075_t pciercx_cfg075;
		pciercx_cfg075.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG075(pcie_port));
		pciercx_cfg075.s.cere = 1;	/* Correctable error reporting enable. */
		pciercx_cfg075.s.nfere = 1;	/* Non-fatal error reporting enable. */
		pciercx_cfg075.s.fere = 1;	/* Fatal error reporting enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG075(pcie_port),
				     pciercx_cfg075.u32);
	}

	/* HP Interrupt Enables (PCIERCn_CFG034[HPINT_EN], */
	/* PCIERCn_CFG034[DLLS_EN,CCINT_EN]) */
	{
		cvmx_pciercx_cfg034_t pciercx_cfg034;
		pciercx_cfg034.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG034(pcie_port));
		pciercx_cfg034.s.hpint_en = 1;	/* Hot-plug interrupt enable. */
		pciercx_cfg034.s.dlls_en = 1;	/* Data Link Layer state changed enable */
		pciercx_cfg034.s.ccint_en = 1;	/* Command completed interrupt enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG034(pcie_port),
				     pciercx_cfg034.u32);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)
	    || OCTEON_IS_MODEL(OCTEON_CN73XX)
	    || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		int qlm = __cvmx_pcie_get_qlm(node, pcie_port);
		int speed = cvmx_qlm_get_gbaud_mhz(qlm);
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_pciercx_cfg031_t cfg031;
		cvmx_pciercx_cfg040_t cfg040;
		cvmx_pciercx_cfg452_t cfg452;
		cvmx_pciercx_cfg089_t cfg089;
		cvmx_pciercx_cfg090_t cfg090;
		cvmx_pciercx_cfg091_t cfg091;
		cvmx_pciercx_cfg092_t cfg092;
		cvmx_pciercx_cfg554_t cfg554;

		/* Make sure the PEM agrees with GSERX about the speed
  		   its going to try */
		switch(speed) {
		case 2500: /* Gen1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 0;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 1;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port),
						cfg040.u32);
			break;
		case 5000: /* Gen2 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 1;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 2;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port),
						cfg040.u32);
			break;
		case 8000: /* Gen3 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 2;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 3;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port),
						cfg040.u32);
			break;
		default:
			break;
		}

		/* Link Width Mode (PCIERCn_CFG452[LME]) */
		pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
		cfg452.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG452(pcie_port));
		if (qlm >= 5)
			cfg452.s.lme = 0x3;
		else
			cfg452.s.lme = (pem_cfg.cn78xx.lanes8) ? 0xf : 0x7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG452(pcie_port), cfg452.u32);

		/* Errata PEM-25990 - Disable ASLPMS */
		cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG031(pcie_port));
		cfg031.s.aslpms = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port), cfg031.u32);

		/* CFG554.PRV default changed from 16'h7ff to 16'h593. */
		cfg554.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG554(pcie_port));
		cfg554.s.prv = 0x593;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG554(pcie_port), cfg554.u32);
		/* Errata PEM-26189 - Disable the 2ms timer on all chips */
		cfg554.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG554(pcie_port));
		cfg554.s.p23td = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG554(pcie_port), cfg554.u32);

		/* Errata PEM-21178 - Change the CFG[089-092] LxUTP & LxDTP defaults. */
		cfg089.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG089(pcie_port));
		cfg089.s.l1ddtp = 7;
		cfg089.s.l1utp = 7;
		cfg089.s.l0dtp = 7;
		cfg089.s.l0utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG089(pcie_port), cfg089.u32);
		cfg090.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG090(pcie_port));
		cfg090.s.l3dtp = 7;
		cfg090.s.l3utp = 7;
		cfg090.s.l2dtp = 7;
		cfg090.s.l2utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG090(pcie_port), cfg090.u32);
		cfg091.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG091(pcie_port));
		cfg091.s.l5dtp = 7;
		cfg091.s.l5utp = 7;
		cfg091.s.l4dtp = 7;
		cfg091.s.l4utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG091(pcie_port), cfg091.u32);
		cfg092.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG092(pcie_port));
		cfg092.s.l7dtp = 7;
		cfg092.s.l7utp = 7;
		cfg092.s.l6dtp = 7;
		cfg092.s.l6utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG092(pcie_port), cfg092.u32);
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
			cvmx_pciercx_cfg548_t cfg548;
			/* Disable phase 2 and phase 3 equalization */
			cfg548.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG548(pcie_port));
			cfg548.s.ep2p3d = 1;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG548(pcie_port), cfg548.u32);
		}
	}
}

/**
 * @INTERNAL
 * Initialize a host mode PCIe gen 1 link. This function takes a PCIe
 * port from reset to a link up state. Software can then begin
 * configuring the rest of the link.
 *
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
static int __cvmx_pcie_rc_initialize_link_gen1(int pcie_port)
{
	uint64_t start_cycle;
	cvmx_pescx_ctl_status_t pescx_ctl_status;
	cvmx_pciercx_cfg452_t pciercx_cfg452;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg448_t pciercx_cfg448;

	/* Set the lane width */
	pciercx_cfg452.u32 = cvmx_pcie_cfgx_read(pcie_port,
						 CVMX_PCIERCX_CFG452(pcie_port));
	pescx_ctl_status.u64 = cvmx_read_csr(CVMX_PESCX_CTL_STATUS(pcie_port));
	if (pescx_ctl_status.s.qlm_cfg == 0) {
		/* We're in 8 lane (56XX) or 4 lane (54XX) mode */
		pciercx_cfg452.s.lme = 0xf;
	} else {
		/* We're in 4 lane (56XX) or 2 lane (52XX) mode */
		pciercx_cfg452.s.lme = 0x7;
	}
	cvmx_pcie_cfgx_write(pcie_port, CVMX_PCIERCX_CFG452(pcie_port),
			     pciercx_cfg452.u32);

	/* CN52XX pass 1.x has an errata where length mismatches on UR responses
	 * can cause bus errors on 64bit memory reads. Turning off length error
	 * checking fixes this
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X)) {
		cvmx_pciercx_cfg455_t pciercx_cfg455;
		pciercx_cfg455.u32 = cvmx_pcie_cfgx_read(pcie_port,
							 CVMX_PCIERCX_CFG455(pcie_port));
		pciercx_cfg455.s.m_cpl_len_err = 1;
		cvmx_pcie_cfgx_write(pcie_port, CVMX_PCIERCX_CFG455(pcie_port),
				     pciercx_cfg455.u32);
	}

	/* Lane swap needs to be manually enabled for CN52XX */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX) && (pcie_port == 1)) {
		pescx_ctl_status.s.lane_swp = 1;
		cvmx_write_csr(CVMX_PESCX_CTL_STATUS(pcie_port),
			       pescx_ctl_status.u64);
	}

	/* Bring up the link */
	pescx_ctl_status.u64 = cvmx_read_csr(CVMX_PESCX_CTL_STATUS(pcie_port));
	pescx_ctl_status.s.lnk_enb = 1;
	cvmx_write_csr(CVMX_PESCX_CTL_STATUS(pcie_port), pescx_ctl_status.u64);

	/* CN52XX pass 1.0: Due to a bug in 2nd order CDR, it needs to be
	 * disabled
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_0))
		__cvmx_helper_errata_qlm_disable_2nd_order_cdr(0);

	/* Wait for the link to come up */
	start_cycle = cvmx_get_cycle();
	do {
		if (cvmx_get_cycle() - start_cycle > 2 * cvmx_clock_get_rate(CVMX_CLOCK_CORE)) {
			cvmx_printf("PCIe: Port %d link timeout\n", pcie_port);
			return -1;
		}
		cvmx_wait(10000);
		pciercx_cfg032.u32 = cvmx_pcie_cfgx_read(pcie_port,
							 CVMX_PCIERCX_CFG032(pcie_port));
		/* Errata (PEM-31375) PEM RSL accesses to PCLK registers can
		   timeout during speed change. Check for a temporary hardware
		   timeout, and retry if one happens */
		if (pciercx_cfg032.u32 == 0xffffffff)
			continue;
	} while ((pciercx_cfg032.s.dlla == 0) || (pciercx_cfg032.s.lt == 1));

	/* Clear all pending errors */
	cvmx_write_csr(CVMX_PEXP_NPEI_INT_SUM,
		       cvmx_read_csr(CVMX_PEXP_NPEI_INT_SUM));

	/* Update the Replay Time Limit. Empirically, some PCIe devices take a
	   little longer to respond than expected under load. As a workaround for
	   this we configure the Replay Time Limit to the value expected for a 512
	   byte MPS instead of our actual 256 byte MPS. The numbers below are
	   directly from the PCIe spec table 3-4 */
	pciercx_cfg448.u32 = cvmx_pcie_cfgx_read(pcie_port,
						 CVMX_PCIERCX_CFG448(pcie_port));
	switch (pciercx_cfg032.s.nlw) {
	case 1:		/* 1 lane */
		pciercx_cfg448.s.rtl = 1677;
		break;
	case 2:		/* 2 lanes */
		pciercx_cfg448.s.rtl = 867;
		break;
	case 4:		/* 4 lanes */
		pciercx_cfg448.s.rtl = 462;
		break;
	case 8:		/* 8 lanes */
		pciercx_cfg448.s.rtl = 258;
		break;
	}
	cvmx_pcie_cfgx_write(pcie_port, CVMX_PCIERCX_CFG448(pcie_port),
			     pciercx_cfg448.u32);

	return 0;
}

static void __cvmx_increment_ba(cvmx_sli_mem_access_subidx_t * pmas)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		pmas->cn68xx.ba++;
	else
		pmas->cn63xx.ba++;
}

/*
 * milliseconds to retry PCIe cfg-space access:
 * Value 32(unscaled) was recommended in HRM, but may be too small for
 * some PCIe devices. This 200mS default should cover most devices,
 * but can be extended by bootparam cvmx-pcie.cfg_timeout, or reduced
 * to speed boot if it is known that no devices need so much time.
 */
static int cfg_timeout = 200;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
module_param(cfg_timeout, int, 0644);
MODULE_PARM_DESC(cfg_timeout, "PCIe config-space i/o timeout in mS,"
	" to accomodate slow-to-start devices");
#endif
static int cfg_retries(void)
{
	static int cfg_ticks = -1;

	if (cfg_ticks < 0) {
		uint64_t nS = cfg_timeout * 1000000;
		const int ceiling = 0xffff;

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		cfg_ticks = nS / (octeon_get_io_clock_rate() >> 16);
#else
		cfg_ticks = nS / (cvmx_clock_get_rate(CVMX_CLOCK_SCLK) >> 16);
#endif

		if (cfg_ticks > ceiling)
			cfg_ticks = ceiling;
	}
	return cfg_ticks;
}

/**
 * Initialize a PCIe gen 1 port for use in host(RC) mode. It doesn't enumerate
 * the bus.
 *
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
static int __cvmx_pcie_rc_initialize_gen1(int pcie_port)
{
	int i;
	int base;
	uint64_t addr_swizzle;
	cvmx_ciu_soft_prst_t ciu_soft_prst;
	cvmx_pescx_bist_status_t pescx_bist_status;
	cvmx_pescx_bist_status2_t pescx_bist_status2;
	cvmx_npei_ctl_status_t npei_ctl_status;
	cvmx_npei_mem_access_ctl_t npei_mem_access_ctl;
	cvmx_npei_mem_access_subidx_t mem_access_subid;
	cvmx_npei_dbg_data_t npei_dbg_data;
	cvmx_pescx_ctl_status2_t pescx_ctl_status2;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_npei_bar1_indexx_t bar1_index;

	if (pcie_port >= CVMX_PCIE_PORTS) {
		//cvmx_dprintf("Invalid PCIe%d port\n", pcie_port);
		return -1;
	}

retry:
	/* Make sure we aren't trying to setup a target mode interface in host
	 * mode
	 */
	npei_ctl_status.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_CTL_STATUS);
	if ((pcie_port == 0) && !npei_ctl_status.s.host_mode) {
		cvmx_printf("PCIe: Port %d in endpoint mode\n", pcie_port);
		return -1;
	}

	/* Make sure a CN52XX isn't trying to bring up port 1 when it is
	 * disabled
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX)) {
		npei_dbg_data.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
		if ((pcie_port == 1) && npei_dbg_data.cn52xx.qlm0_link_width) {
			cvmx_printf("PCIe: ERROR: cvmx_pcie_rc_initialize() called on port1, but port1 is disabled\n");
			return -1;
		}
	}

	/* PCIe switch arbitration mode. '0' == fixed priority NPEI, PCIe0,
	 * then PCIe1. '1' == round robin.
	 */
	npei_ctl_status.s.arb = 1;
	npei_ctl_status.s.cfg_rtry = cfg_retries();
	/* CN52XX pass1.x has an errata where P0_NTAGS and P1_NTAGS don't
	 * reset
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X)) {
		npei_ctl_status.s.p0_ntags = 0x20;
		npei_ctl_status.s.p1_ntags = 0x20;
	}
	cvmx_write_csr(CVMX_PEXP_NPEI_CTL_STATUS, npei_ctl_status.u64);

	/* Bring the PCIe out of reset */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_EBH5200) {
		/* The EBH5200 board swapped the PCIe reset lines on the board.
		 * As a workaround for this bug, we bring both PCIe ports out
		 * of reset at the same time instead of on separate calls.  So
		 * for port 0, we bring both out of reset and do nothing on
		 * port 1
		 */
		if (pcie_port == 0) {
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST);
			/* After a chip reset the PCIe will also be in reset.
			 * If it isn't, most likely someone is trying to init
			 * it again without a proper PCIe reset
			 */
			if (ciu_soft_prst.s.soft_prst == 0) {
				/* Reset the ports */
				ciu_soft_prst.s.soft_prst = 1;
				cvmx_write_csr(CVMX_CIU_SOFT_PRST,
					       ciu_soft_prst.u64);
				ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST1);
				ciu_soft_prst.s.soft_prst = 1;
				cvmx_write_csr(CVMX_CIU_SOFT_PRST1,
					       ciu_soft_prst.u64);
				/* Wait until pcie resets the ports. */
				cvmx_wait_usec(2000);
			}
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST1);
			ciu_soft_prst.s.soft_prst = 0;
			cvmx_write_csr(CVMX_CIU_SOFT_PRST1, ciu_soft_prst.u64);
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST);
			ciu_soft_prst.s.soft_prst = 0;
			cvmx_write_csr(CVMX_CIU_SOFT_PRST, ciu_soft_prst.u64);
		}
	} else {
		/* The normal case: The PCIe ports are completely separate and
		 * can be brought out of reset independently
		 */
		if (pcie_port)
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST1);
		else
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST);
		/* After a chip reset the PCIe will also be in reset.  If it
		 * isn't, most likely someone is trying to init it again
		 * without a proper PCIe reset
		 */
		if (ciu_soft_prst.s.soft_prst == 0) {
			/* Reset the port */
			ciu_soft_prst.s.soft_prst = 1;
			if (pcie_port)
				cvmx_write_csr(CVMX_CIU_SOFT_PRST1,
					       ciu_soft_prst.u64);
			else
				cvmx_write_csr(CVMX_CIU_SOFT_PRST,
					       ciu_soft_prst.u64);
			/* Wait until pcie resets the ports. */
			cvmx_wait_usec(2000);
		}
		if (pcie_port) {
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST1);
			ciu_soft_prst.s.soft_prst = 0;
			cvmx_write_csr(CVMX_CIU_SOFT_PRST1, ciu_soft_prst.u64);
		} else {
			ciu_soft_prst.u64 = cvmx_read_csr(CVMX_CIU_SOFT_PRST);
			ciu_soft_prst.s.soft_prst = 0;
			cvmx_write_csr(CVMX_CIU_SOFT_PRST, ciu_soft_prst.u64);
		}
	}

	/* Wait for PCIe reset to complete. Due to errata PCIE-700, we don't
	 * poll PESCX_CTL_STATUS2[PCIERST], but simply wait a fixed number of
	 * cycles
	 */
	cvmx_wait(400000);

	/* PESCX_BIST_STATUS2[PCLK_RUN] was missing on pass 1 of CN56XX and
	 * CN52XX, so we only probe it on newer chips
	 */
	if (!OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_X) &&
	    !OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X)) {
		/* Clear PCLK_RUN so we can check if the clock is running */
		pescx_ctl_status2.u64 = cvmx_read_csr(CVMX_PESCX_CTL_STATUS2(pcie_port));
		pescx_ctl_status2.s.pclk_run = 1;
		cvmx_write_csr(CVMX_PESCX_CTL_STATUS2(pcie_port),
			       pescx_ctl_status2.u64);
		/* Now that we cleared PCLK_RUN, wait for it to be set again
		 * telling us the clock is running
		 */
		if (CVMX_WAIT_FOR_FIELD64(CVMX_PESCX_CTL_STATUS2(pcie_port),
					  cvmx_pescx_ctl_status2_t, pclk_run,
					  ==, 1, 10000)) {
			cvmx_printf("PCIe: Port %d isn't clocked, skipping.\n",
				    pcie_port);
			return -1;
		}
	}

	/* Check and make sure PCIe came out of reset. If it doesn't the board
	 * probably hasn't wired the clocks up and the interface should be
	 * skipped
	 */
	pescx_ctl_status2.u64 = cvmx_read_csr(CVMX_PESCX_CTL_STATUS2(pcie_port));
	if (pescx_ctl_status2.s.pcierst) {
		cvmx_printf("PCIe: Port %d stuck in reset, skipping.\n",
			    pcie_port);
		return -1;
	}

	/* Check BIST2 status. If any bits are set skip this interface.  This
	 * is an attempt to catch PCIE-813 on pass 1 parts
	 */
	pescx_bist_status2.u64 = cvmx_read_csr(CVMX_PESCX_BIST_STATUS2(pcie_port));
	if (pescx_bist_status2.u64) {
		cvmx_printf("PCIe: Port %d BIST2 failed. Most likely this port isn't hooked up, skipping.\n",
			    pcie_port);
		return -1;
	}

	/* Check BIST status */
	pescx_bist_status.u64 = cvmx_read_csr(CVMX_PESCX_BIST_STATUS(pcie_port));
	if (pescx_bist_status.u64)
		cvmx_printf("PCIe: BIST FAILED for port %d (0x%016llx)\n",
			    pcie_port, CAST64(pescx_bist_status.u64));

	/* Initialize the config space CSRs */
	__cvmx_pcie_rc_initialize_config_space(0, pcie_port);

	/* Bring the link up */
	if (__cvmx_pcie_rc_initialize_link_gen1(pcie_port)) {
		cvmx_printf("PCIe: Failed to initialize port %d, probably the slot is empty\n",
			    pcie_port);
		return -1;
	}

	/* Store merge control (NPEI_MEM_ACCESS_CTL[TIMER,MAX_WORD]) */
	npei_mem_access_ctl.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_MEM_ACCESS_CTL);
	npei_mem_access_ctl.s.max_word = 0;	/* Allow 16 words to combine */
	npei_mem_access_ctl.s.timer = 127;	/* Wait up to 127 cycles for more data */
	cvmx_write_csr(CVMX_PEXP_NPEI_MEM_ACCESS_CTL, npei_mem_access_ctl.u64);

	/* Setup Mem access SubDIDs */
	mem_access_subid.u64 = 0;
	mem_access_subid.s.port = pcie_port;	/* Port the request is sent to. */
	mem_access_subid.s.nmerge = 1;	/* Due to an errata on pass 1 chips, no merging is allowed. */
	mem_access_subid.s.esr = 1;	/* Endian-swap for Reads. */
	mem_access_subid.s.esw = 1;	/* Endian-swap for Writes. */
	mem_access_subid.s.nsr = 0;	/* Enable Snooping for Reads. Octeon doesn't care, but devices might want this more conservative setting */
	mem_access_subid.s.nsw = 0;	/* Enable Snoop for Writes. */
	mem_access_subid.s.ror = 0;	/* Disable Relaxed Ordering for Reads. */
	mem_access_subid.s.row = 0;	/* Disable Relaxed Ordering for Writes. */
	mem_access_subid.s.ba = 0;	/* PCIe Adddress Bits <63:34>. */

	/* Setup mem access 12-15 for port 0, 16-19 for port 1, supplying 36
	 * bits of address space
	 */
	for (i = 12 + pcie_port * 4; i < 16 + pcie_port * 4; i++) {
		cvmx_write_csr(CVMX_PEXP_NPEI_MEM_ACCESS_SUBIDX(i),
			       mem_access_subid.u64);
		mem_access_subid.s.ba += 1;	/* Set each SUBID to extend the addressable range */
	}

	/* Disable the peer to peer forwarding register. This must be setup
	   by the OS after it enumerates the bus and assigns addresses to the
	   PCIe busses */
	for (i = 0; i < 4; i++) {
		cvmx_write_csr(CVMX_PESCX_P2P_BARX_START(i, pcie_port), -1);
		cvmx_write_csr(CVMX_PESCX_P2P_BARX_END(i, pcie_port), -1);
	}

	/* Set Octeon's BAR0 to decode 0-16KB. It overlaps with Bar2 */
	cvmx_write_csr(CVMX_PESCX_P2N_BAR0_START(pcie_port), 0);

	/* BAR1 follows BAR2 with a gap so it has the same address as for gen2. */
	cvmx_write_csr(CVMX_PESCX_P2N_BAR1_START(pcie_port),
		       CVMX_PCIE_BAR1_RC_BASE);

	bar1_index.u32 = 0;
	bar1_index.s.addr_idx = (CVMX_PCIE_BAR1_PHYS_BASE >> 22);
	bar1_index.s.ca = 1;	/* Not Cached */
	bar1_index.s.end_swp = 1;	/* Endian Swap mode */
	bar1_index.s.addr_v = 1;	/* Valid entry */

	base = pcie_port ? 16 : 0;

	/* Big endian swizzle for 32-bit PEXP_NCB register. */
#ifdef __MIPSEB__
	addr_swizzle = 4;
#else
	addr_swizzle = 0;
#endif
	for (i = 0; i < 16; i++) {
		cvmx_write64_uint32((CVMX_PEXP_NPEI_BAR1_INDEXX(base) ^ addr_swizzle), bar1_index.u32);
		base++;
		/* 256MB / 16 >> 22 == 4 */
		bar1_index.s.addr_idx += (((1ull << 28) / 16ull) >> 22);
	}

	/* Set Octeon's BAR2 to decode 0-2^39. Bar0 and Bar1 take precedence
	   where they overlap. It also overlaps with the device addresses, so
	   make sure the peer to peer forwarding is set right */
	cvmx_write_csr(CVMX_PESCX_P2N_BAR2_START(pcie_port), 0);

	/* Setup BAR2 attributes */
	/* Relaxed Ordering (NPEI_CTL_PORTn[PTLP_RO,CTLP_RO, WAIT_COM]) */
	/* ­ PTLP_RO,CTLP_RO should normally be set (except for debug). */
	/* ­ WAIT_COM=0 will likely work for all applications. */
	/* Load completion relaxed ordering (NPEI_CTL_PORTn[WAITL_COM]) */
	if (pcie_port) {
		cvmx_npei_ctl_port1_t npei_ctl_port;
		npei_ctl_port.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_CTL_PORT1);
		npei_ctl_port.s.bar2_enb = 1;
		npei_ctl_port.s.bar2_esx = 1;
		npei_ctl_port.s.bar2_cax = 0;
		npei_ctl_port.s.ptlp_ro = 1;
		npei_ctl_port.s.ctlp_ro = 1;
		npei_ctl_port.s.wait_com = 0;
		npei_ctl_port.s.waitl_com = 0;
		cvmx_write_csr(CVMX_PEXP_NPEI_CTL_PORT1, npei_ctl_port.u64);
	} else {
		cvmx_npei_ctl_port0_t npei_ctl_port;
		npei_ctl_port.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_CTL_PORT0);
		npei_ctl_port.s.bar2_enb = 1;
		npei_ctl_port.s.bar2_esx = 1;
		npei_ctl_port.s.bar2_cax = 0;
		npei_ctl_port.s.ptlp_ro = 1;
		npei_ctl_port.s.ctlp_ro = 1;
		npei_ctl_port.s.wait_com = 0;
		npei_ctl_port.s.waitl_com = 0;
		cvmx_write_csr(CVMX_PEXP_NPEI_CTL_PORT0, npei_ctl_port.u64);
	}

	/* Both pass 1 and pass 2 of CN52XX and CN56XX have an errata that causes
	 * TLP ordering to not be preserved after multiple PCIe port resets. This
	 * code detects this fault and corrects it by aligning the TLP counters
	 * properly. Another link reset is then performed. See PCIE-13340
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN56XX_PASS2_X) ||
	    OCTEON_IS_MODEL(OCTEON_CN52XX_PASS2_X) ||
	    OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_X) ||
	    OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X)) {
		cvmx_npei_dbg_data_t dbg_data;
		int old_in_fif_p_count;
		int in_fif_p_count;
		int out_p_count;
		int in_p_offset = (OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X) ||
				   OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_X)) ? 4 : 1;
		int i;

		/* Choose a write address of 1MB. It should be harmless as all
		 * bars haven't been setup
		 */
		uint64_t write_address = (cvmx_pcie_get_mem_base_address(pcie_port) + 0x100000) | (1ull << 63);

		/* Make sure at least in_p_offset have been executed before we
		 * try and read in_fif_p_count
		 */
		i = in_p_offset;
		while (i--) {
			cvmx_write64_uint32(write_address, 0);
			cvmx_wait(10000);
		}

		/* Read the IN_FIF_P_COUNT from the debug select.
		 * IN_FIF_P_COUNT can be unstable sometimes so read it twice
		 * with a write between the reads.  This way we can tell the
		 * value is good as it will increment by one due to the write
		 */
		cvmx_write_csr(CVMX_PEXP_NPEI_DBG_SELECT,
			       (pcie_port) ? 0xd7fc : 0xcffc);
		cvmx_read_csr(CVMX_PEXP_NPEI_DBG_SELECT);
		do {
			dbg_data.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
			old_in_fif_p_count = dbg_data.s.data & 0xff;
			cvmx_write64_uint32(write_address, 0);
			cvmx_wait(10000);
			dbg_data.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
			in_fif_p_count = dbg_data.s.data & 0xff;
		} while (in_fif_p_count != ((old_in_fif_p_count + 1) & 0xff));

		/* Update in_fif_p_count for it's offset with respect to
		 * out_p_count
		 */
		in_fif_p_count = (in_fif_p_count + in_p_offset) & 0xff;

		/* Read the OUT_P_COUNT from the debug select */
		cvmx_write_csr(CVMX_PEXP_NPEI_DBG_SELECT,
			       (pcie_port) ? 0xd00f : 0xc80f);
		cvmx_read_csr(CVMX_PEXP_NPEI_DBG_SELECT);
		dbg_data.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
		out_p_count = (dbg_data.s.data >> 1) & 0xff;

		/* Check that the two counters are aligned */
		if (out_p_count != in_fif_p_count) {
			cvmx_printf("PCIe: Port %d aligning TLP counters as workaround to maintain ordering\n",
				    pcie_port);
			while (in_fif_p_count != 0) {
				cvmx_write64_uint32(write_address, 0);
				cvmx_wait(10000);
				in_fif_p_count = (in_fif_p_count + 1) & 0xff;
			}
			/* The EBH5200 board swapped the PCIe reset lines on
			 * the board.  This means we must bring both links down
			 * and up, which will cause the PCIe0 to need alignment
			 * again. Lots of messages will be displayed, but
			 * everything should work
			 */
			if ((cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_EBH5200)
			    && (pcie_port == 1))
				cvmx_pcie_rc_initialize(0);
			/* Rety bringing this port up */
			goto retry;
		}
	}

	/* Display the link status */
	pciercx_cfg032.u32 = cvmx_pcie_cfgx_read(pcie_port,
						 CVMX_PCIERCX_CFG032(pcie_port));
	cvmx_printf("PCIe: Port %d link active, %d lanes, speed gen%d \n",
		    pcie_port, pciercx_cfg032.s.nlw, pciercx_cfg032.s.ls);

	pcie_link_initialized[0][pcie_port] = true;
	return 0;
}

/**
 * @INTERNAL
 * Enable/Disable PEMX_PEMON.pemon based on the direction.
 *
 * @param node      node
 * @param pcie_port PCIe port
 * @param direction 0 to disable, 1 to enable
 */
static void __cvmx_pcie_config_pemon(int node, int pcie_port, bool direction)
{
	cvmx_pemx_on_t pemon;

	pemon.u64 = CVMX_READ_CSR(CVMX_PEMX_ON(pcie_port));
	pemon.s.pemon = direction;
	CVMX_WRITE_CSR(CVMX_PEMX_ON(pcie_port), pemon.u64);
	pemon.u64 = CVMX_READ_CSR(CVMX_PEMX_ON(pcie_port));
}

/**
 * @INTERNAL
 * De-assert GSER_PHY.phy_reset for a given qlm
 *
 * @param node       node
 * @param qlm        qlm for a given PCIe port
 */
static void __cvmx_pcie_gser_phy_config(int node, int pcie_port, int qlm)
{
	cvmx_pemx_cfg_t pem_cfg;
	cvmx_gserx_phy_ctl_t ctrl;
	int has_8lanes = 0;
	int is_gen3 = 0;

	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));

	/* Assert the reset */
	ctrl.s.phy_reset = 1;
	CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm), ctrl.u64);
	pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
	cvmx_wait_usec(10);

	has_8lanes = pem_cfg.cn78xx.lanes8;
	is_gen3 = pem_cfg.cn78xx.md >= 2;

	if (has_8lanes) {
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
		ctrl.s.phy_reset = 1;
		CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm + 1), ctrl.u64);
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
	}
	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));
	cvmx_wait_usec(10);

	/* Deassert the reset */
	ctrl.s.phy_reset = 0;
	CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm), ctrl.u64);
	pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
	cvmx_wait_usec(500);

	if (has_8lanes) {
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
		ctrl.s.phy_reset = 0;
		CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm + 1), ctrl.u64);
	}
	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));
	cvmx_wait_usec(500);

	/* Apply some erratas after PHY reset, only applies to PCIe GEN3 */
	if (is_gen3) {
		int i;
		int high_qlm = has_8lanes ? qlm + 1 : qlm;

		/* Apply workaround for Errata GSER-26150 */
		if (OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_0)) {
			for (i = qlm; i < high_qlm; i++) {
				cvmx_gserx_glbl_pll_cfg_3_t pll_cfg_3;
				cvmx_gserx_glbl_misc_config_1_t misc_config_1;
				/* Update PLL parameters */
				/* Step 1: Set GSER()_GLBL_PLL_CFG_3[PLL_VCTRL_SEL_LCVCO_VAL] = 0x2, and
				   GSER()_GLBL_PLL_CFG_3[PCS_SDS_PLL_VCO_AMP] = 0 */
				pll_cfg_3.u64 = CVMX_READ_CSR(CVMX_GSERX_GLBL_PLL_CFG_3(i));
				pll_cfg_3.s.pcs_sds_pll_vco_amp = 0;
				pll_cfg_3.s.pll_vctrl_sel_lcvco_val = 2;
				CVMX_WRITE_CSR(CVMX_GSERX_GLBL_PLL_CFG_3(i), pll_cfg_3.u64);

				/* Step 2: Set GSER()_GLBL_MISC_CONFIG_1[PCS_SDS_TRIM_CHP_REG] = 0x2. */
				misc_config_1.u64 = CVMX_READ_CSR(CVMX_GSERX_GLBL_MISC_CONFIG_1(i));
				misc_config_1.s.pcs_sds_trim_chp_reg = 2;
				CVMX_WRITE_CSR(CVMX_GSERX_GLBL_MISC_CONFIG_1(i), misc_config_1.u64);
			}
		}

		/* Apply workaround for Errata GSER-25992 */
		if (OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_X)
		    || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			for (i = qlm; i < high_qlm; i++)
				cvmx_qlm_gser_errata_25992(node, i);
		}
	}
}

/**
 * @INTERNAL
 * Initialize a host mode PCIe gen 2 link. This function takes a PCIe
 * port from reset to a link up state. Software can then begin
 * configuring the rest of the link.
 *
 * @param node	    node
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
static int __cvmx_pcie_rc_initialize_link_gen2(int node, int pcie_port)
{
	uint64_t start_cycle;
	int try_gen3;

	cvmx_pemx_ctl_status_t pem_ctl_status;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg031_t pciercx_cfg031;
	cvmx_pciercx_cfg040_t pciercx_cfg040;
	cvmx_pciercx_cfg448_t pciercx_cfg448;

	if (OCTEON_IS_OCTEON3()) {
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_ON(pcie_port),
					       cvmx_pemx_on_t, pemoor, ==, 1,
					       100000)) {
			cvmx_printf("%d:PCIe: Port %d PEM not on, skipping\n", node, pcie_port);
			return -1;
		}
	}

	/* Remember if the link should be Gen3. This is needed for the CN78XX
	   pass 1.x workaround below */
	pciercx_cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG031(pcie_port));
	try_gen3 = (pciercx_cfg031.s.mls == 3);

	/* Errata (GSER-21178) PCIe gen3 doesn't work */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) && try_gen3) {
		/* Force Gen1 for initial link bringup. We'll fix it later */
		pciercx_cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG031(pcie_port));
		pciercx_cfg031.s.mls = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port), pciercx_cfg031.u32);
		pciercx_cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
		pciercx_cfg040.s.tls = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port), pciercx_cfg040.u32);
	}

	/* Bring up the link */
	pem_ctl_status.u64 = CVMX_READ_CSR(CVMX_PEMX_CTL_STATUS(pcie_port));
	pem_ctl_status.s.lnk_enb = 1;
	CVMX_WRITE_CSR(CVMX_PEMX_CTL_STATUS(pcie_port), pem_ctl_status.u64);

	/* Wait for the link to come up */
	start_cycle = cvmx_get_cycle();
	do {
		if (cvmx_get_cycle() - start_cycle > cvmx_clock_get_rate(CVMX_CLOCK_CORE)) {
			return -1;
		}
		cvmx_wait_usec(1000);
		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG032(pcie_port));
	} while ((pciercx_cfg032.s.dlla == 0) || (pciercx_cfg032.s.lt == 1));

	/* Errata (GSER-21178) PCIe gen3 doesn't work, continued */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) && try_gen3) {
		cvmx_pciercx_cfg031_t cfg031;
		cvmx_pciercx_cfg040_t cfg040;
		cvmx_pciercx_cfg515_t cfg515;
		cvmx_pemx_cfg_t pem_cfg;
		int low_qlm, high_qlm;
		int qlm, lane;

		/* Enable gen3 speed selection */
		cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG031(pcie_port));
		cfg031.s.mls = 3;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port), cfg031.u32);
		cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
		cfg040.s.tls = 3;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port), cfg040.u32);
		/* Force a demand speed change */
		cfg515.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG515(pcie_port));
		cfg515.s.dsc = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG515(pcie_port), cfg515.u32);
		cvmx_wait_usec(500);

		/* Wait up to 10ms for the link speed change to complete */
		start_cycle = cvmx_get_cycle();
		do {
			if (cvmx_get_cycle() - start_cycle > cvmx_clock_get_rate(CVMX_CLOCK_CORE))
				return -1;
			cvmx_wait(10000);
			pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG032(pcie_port));
		} while (pciercx_cfg032.s.ls != 3);

		pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
		low_qlm = pcie_port;
		high_qlm = (pem_cfg.cn78xx.lanes8) ? low_qlm+1 : low_qlm;

		/* Toggle cfg_rx_dll_locken_ovvrd_en and rx_resetn_ovrrd_en across
		   all QM lanes in use */
		for (qlm = low_qlm; qlm <= high_qlm; qlm++) {
			for (lane = 0; lane < 4; lane++) {
				cvmx_gserx_lanex_rx_misc_ovrrd_t misc_ovrrd;
				cvmx_gserx_lanex_pwr_ctrl_t pwr_ctrl;

				misc_ovrrd.u64 = CVMX_READ_CSR(CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm));
				misc_ovrrd.s.cfg_rx_dll_locken_ovrrd_en = 1;
				CVMX_WRITE_CSR(CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm), misc_ovrrd.u64);
				pwr_ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_LANEX_PWR_CTRL(lane, qlm));
				pwr_ctrl.s.rx_resetn_ovrrd_en = 1;
				CVMX_WRITE_CSR(CVMX_GSERX_LANEX_PWR_CTRL(lane, qlm), pwr_ctrl.u64);
			}
		}
		for (qlm = low_qlm; qlm <= high_qlm; qlm++) {
			for (lane = 0; lane < 4; lane++) {
				cvmx_gserx_lanex_rx_misc_ovrrd_t misc_ovrrd;
				cvmx_gserx_lanex_pwr_ctrl_t pwr_ctrl;

				misc_ovrrd.u64 = CVMX_READ_CSR(CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm));
				misc_ovrrd.s.cfg_rx_dll_locken_ovrrd_en = 0;
				CVMX_WRITE_CSR(CVMX_GSERX_LANEX_RX_MISC_OVRRD(lane, qlm), misc_ovrrd.u64);
				pwr_ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_LANEX_PWR_CTRL(lane, qlm));
				pwr_ctrl.s.rx_resetn_ovrrd_en = 0;
				CVMX_WRITE_CSR(CVMX_GSERX_LANEX_PWR_CTRL(lane, qlm), pwr_ctrl.u64);
			}
		}

		/* Wait for the link to come up (hopefully Gen3) and link
		   training to be complete */
		start_cycle = cvmx_clock_get_count(CVMX_CLOCK_CORE);
		do {
			if (cvmx_clock_get_count(CVMX_CLOCK_CORE) - start_cycle > cvmx_clock_get_rate(CVMX_CLOCK_CORE))
				return -1;
			cvmx_wait_usec(1000);
			pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
		} while ((pciercx_cfg032.s.dlla == 0) || (pciercx_cfg032.s.lt == 1));
	}

	/* Update the Replay Time Limit.  Empirically, some PCIe devices take a
	 * little longer to respond than expected under load. As a workaround
	 * for this we configure the Replay Time Limit to the value expected
	 * for a 512 byte MPS instead of our actual 256 byte MPS. The numbers
	 * below are directly from the PCIe spec table 3-4
	 */
	pciercx_cfg448.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG448(pcie_port));
	switch (pciercx_cfg032.s.nlw) {
	case 1:		/* 1 lane */
		pciercx_cfg448.s.rtl = 1677;
		break;
	case 2:		/* 2 lanes */
		pciercx_cfg448.s.rtl = 867;
		break;
	case 4:		/* 4 lanes */
		pciercx_cfg448.s.rtl = 462;
		break;
	case 8:		/* 8 lanes */
		pciercx_cfg448.s.rtl = 258;
		break;
	}
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG448(pcie_port),
			     pciercx_cfg448.u32);

	return 0;
}

/**
 * Initialize a PCIe gen 2 port for use in host(RC) mode. It doesn't enumerate
 * the bus.
 *
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
static int __cvmx_pcie_rc_initialize_gen2(int pcie_port)
{
	int i;
	cvmx_ciu_soft_prst_t ciu_soft_prst;
	cvmx_mio_rst_ctlx_t mio_rst_ctl;
	cvmx_pemx_bar_ctl_t pemx_bar_ctl;
	cvmx_pemx_ctl_status_t pemx_ctl_status;
	cvmx_pemx_bist_status_t pemx_bist_status;
	cvmx_pemx_bist_status2_t pemx_bist_status2;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg515_t pciercx_cfg515;
	cvmx_sli_ctl_portx_t sli_ctl_portx;
	cvmx_sli_mem_access_ctl_t sli_mem_access_ctl;
	cvmx_sli_mem_access_subidx_t mem_access_subid;
	cvmx_pemx_bar1_indexx_t bar1_index;
	uint64_t ciu_soft_prst_reg, ciu_soft_prst_reg_alt, rst_ctl_reg;
	int ep_mode;
	int qlm;
	int node = (pcie_port >> 4) & 0x3;
	int connected_pcie_reset = -1;
	int requires_pem_reset = 0;
	enum cvmx_qlm_mode mode = CVMX_QLM_MODE_DISABLED;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	static void *fdt_addr = 0;
#endif
	pcie_port &= 0x3;
	qlm = __cvmx_pcie_get_qlm(node, pcie_port);

	if (pcie_port >= CVMX_PCIE_PORTS) {
		//cvmx_dprintf("Invalid PCIe%d port\n", pcie_port);
		return -1;
	}

	/* Make sure this interface is PCIe */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		if (cvmx_qlm_get_dlm_mode(1, pcie_port) == CVMX_QLM_MODE_DISABLED) {
			cvmx_printf("PCIe: Port %d not in PCIe mode, skipping\n",
				    pcie_port);
			return -1;
		}
		mode = cvmx_qlm_get_mode(qlm);
	} else if (octeon_has_feature(OCTEON_FEATURE_PCIE)) {
		/* Requires reading the MIO_QLMX_CFG register to figure
		   out the port type. */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			qlm = 3 - (pcie_port * 2);
		else if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
			cvmx_mio_qlmx_cfg_t qlm_cfg;
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(1));
			if (qlm_cfg.s.qlm_cfg == 1)
				qlm = 1;
			else
				qlm = pcie_port;
		} else if (OCTEON_IS_MODEL(OCTEON_CN66XX)
			   || OCTEON_IS_MODEL(OCTEON_CN63XX))
			qlm = pcie_port;

		/* PCIe is allowed only in QLM1, 1 PCIe port in x2 or
		 * 2 PCIe ports in x1
		 */
		else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
			qlm = 1;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			mode = cvmx_qlm_get_mode_cn78xx(node, qlm);
		else
			mode = cvmx_qlm_get_mode(qlm);
		if (mode == CVMX_QLM_MODE_SRIO_1X4 ||
		    mode == CVMX_QLM_MODE_SRIO_2X2 ||
		    mode == CVMX_QLM_MODE_SRIO_4X1) {
			cvmx_printf("%d:PCIe: Port %d is SRIO, skipping.\n",
				    node, pcie_port);
			return -1;
		} else if (mode == CVMX_QLM_MODE_SGMII) {
			cvmx_printf("%d:PCIe: Port %d is SGMII, skipping.\n",
				    node, pcie_port);
			return -1;
		} else if (mode == CVMX_QLM_MODE_XAUI ||
			   mode == CVMX_QLM_MODE_RXAUI) {
			cvmx_printf("%d:PCIe: Port %d is XAUI, skipping.\n",
				    node, pcie_port);
			return -1;
		} else if (mode == CVMX_QLM_MODE_ILK) {
			cvmx_printf("%d:PCIe: Port %d is ILK, skipping.\n",
				    node, pcie_port);
			return -1;
		} else if (mode != CVMX_QLM_MODE_PCIE &&
			   mode != CVMX_QLM_MODE_PCIE_1X8 &&
			   mode != CVMX_QLM_MODE_PCIE_1X2 &&
			   mode != CVMX_QLM_MODE_PCIE_2X1 &&
			   mode != CVMX_QLM_MODE_PCIE_1X1) {
			cvmx_printf("%d:PCIe: Port %d is unknown, skipping.\n",
				    node, pcie_port);
			return -1;
		}
	}
#if 0
	/* This code is so that the PCIe analyzer is able to see 63XX traffic */
	cvmx_dprintf("PCIE : init for pcie analyzer.\n");
	cvmx_helper_qlm_jtag_init();
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 85);
	cvmx_helper_qlm_jtag_shift(pcie_port, 1, 1);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 300 - 86);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 85);
	cvmx_helper_qlm_jtag_shift(pcie_port, 1, 1);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 300 - 86);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 85);
	cvmx_helper_qlm_jtag_shift(pcie_port, 1, 1);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 300 - 86);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 85);
	cvmx_helper_qlm_jtag_shift(pcie_port, 1, 1);
	cvmx_helper_qlm_jtag_shift_zeros(pcie_port, 300 - 86);
	cvmx_helper_qlm_jtag_update(pcie_port);
#endif

	/* Make sure we aren't trying to setup a target mode interface in host
	 * mode
	 */
	if (OCTEON_IS_OCTEON3()) {
		ciu_soft_prst_reg = CVMX_RST_SOFT_PRSTX(pcie_port);
		ciu_soft_prst_reg_alt = CVMX_RST_SOFT_PRSTX(pcie_port ^ 1);
		rst_ctl_reg = CVMX_RST_CTLX(pcie_port);
		if (!OCTEON_IS_MODEL(OCTEON_CN70XX))
			requires_pem_reset = 1;
	} else {
		ciu_soft_prst_reg = (pcie_port) ?
				 CVMX_CIU_SOFT_PRST1 : CVMX_CIU_SOFT_PRST;
		ciu_soft_prst_reg_alt = (pcie_port ^ 1) ?
				 CVMX_CIU_SOFT_PRST1 : CVMX_CIU_SOFT_PRST;
		rst_ctl_reg = CVMX_MIO_RST_CTLX(pcie_port);
	}

	mio_rst_ctl.u64 = CVMX_READ_CSR(rst_ctl_reg);
	ep_mode = ((OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		    OCTEON_IS_MODEL(OCTEON_CNF71XX))
		? (mio_rst_ctl.s.prtmode != 1) : (!mio_rst_ctl.s.host_mode));

	if (OCTEON_IS_MODEL(OCTEON_CN70XX) && pcie_port) {
		cvmx_pemx_cfg_t pemx_cfg;
		pemx_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(0));
		if ((pemx_cfg.s.md & 3) == 2) {
			cvmx_printf("PCIe: Port %d in 1x4 mode.\n", pcie_port);
			return -1;
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		CVMX_WRITE_CSR(CVMX_DTX_PEMX_SELX(0, pcie_port), 0x17);
		CVMX_WRITE_CSR(CVMX_DTX_PEMX_SELX(1, pcie_port), 0);
	}

	if (ep_mode) {
		cvmx_printf("%d:PCIe: Port %d in endpoint mode.\n", node, pcie_port);
		return -1;
	}

#if 0
	/* Enable this code to force PCIe link for x1 lane card. */
	__cvmx_qlm_pcie_cfg_rxd_set_tweak(qlm, 0);
#endif

#if 0
	/* Enable this code to force PCIe link for x2 lane card. */
	__cvmx_qlm_pcie_cfg_rxd_set_tweak(qlm, 0);
	__cvmx_qlm_pcie_cfg_rxd_set_tweak(qlm, 1);
#endif

#if 0
	/* Enable this code to force PCIe link for x4 lane card. */
	__cvmx_qlm_pcie_cfg_rxd_set_tweak(qlm, -1);
#endif

	/* CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis be
	 * programmed
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		if (pcie_port) {
			cvmx_ciu_qlm1_t ciu_qlm;
			ciu_qlm.u64 = cvmx_read_csr(CVMX_CIU_QLM1);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			cvmx_write_csr(CVMX_CIU_QLM1, ciu_qlm.u64);
		} else {
			cvmx_ciu_qlm0_t ciu_qlm;
			ciu_qlm.u64 = cvmx_read_csr(CVMX_CIU_QLM0);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			cvmx_write_csr(CVMX_CIU_QLM0, ciu_qlm.u64);
		}
	}

	/* On some boards, notably the SFF6100 board, the PCIe reset lines
	 * are miswired in PCIe 2x1 mode.  In 2x1 mode, the PRST line of
	 * QLM0 should go to PCIe PEM 0 and the PRST line of QLM1 should
	 * go to PEM 1.
	 *
	 * On the SFF board, the PRST line of QLM1 is wired to both PEM0 and
	 * PEM1.
	 */
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Note that fdr_addr is 'static' */
	if (fdt_addr == 0 && mode == CVMX_QLM_MODE_PCIE_2X1)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);
	if (fdt_addr && mode == CVMX_QLM_MODE_PCIE_2X1) {
		int offset;

		offset = fdt_path_offset(fdt_addr, "/soc");
		if (offset >= 0) {
			connected_pcie_reset =
				cvmx_fdt_get_int(fdt_addr, offset,
						 "cavium,connected-pcie-reset-2x1",
						 -1);
		}
	}
#else
	if (mode == CVMX_QLM_MODE_PCIE_2X1) {
		struct device_node *node = of_find_node_by_path("/soc@0");
		if (node) {
			if (of_property_read_u32(node,
						 "cavium,connected-pcie-reset-2x1",
						 &connected_pcie_reset) != 0)
				connected_pcie_reset = -1;
		}
	}
#endif
	/* On SFF6100 board, if QLM1 is configured as 2X1 DLM mode,
	   the board has mis-wired PCIe reset line, initialize PCIe1
	   followed by PCIe0. */
	switch (connected_pcie_reset) {
	case 0:
	case 1:
		if ((pcie_port != connected_pcie_reset) &&
		    (mode == CVMX_QLM_MODE_PCIE_2X1)) {
			ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
			if (ciu_soft_prst.s.soft_prst == 0) {
				/* Reset the port */
				ciu_soft_prst.s.soft_prst = 1;
				CVMX_WRITE_CSR(ciu_soft_prst_reg,
					       ciu_soft_prst.u64);
				CVMX_WRITE_CSR(ciu_soft_prst_reg_alt,
					       ciu_soft_prst.u64);
				/* Wait until pcie resets the ports. */
				cvmx_wait_usec(2000);

			}
		}
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
		ciu_soft_prst.s.soft_prst = 0;
		CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg_alt);
		ciu_soft_prst.s.soft_prst = 0;
		CVMX_WRITE_CSR(ciu_soft_prst_reg_alt, ciu_soft_prst.u64);
		break;
	case -1:
	default:
		/* Bring the PCIe out of reset */
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
		/* After a chip reset the PCIe will also be in reset. If it
		 * isn't, most likely someone is trying to init it again
		 * without a proper PCIe reset.
		 */
		if (ciu_soft_prst.s.soft_prst == 0) {
			/* Disable the MAC controller before resetting */
			if (requires_pem_reset) {
				__cvmx_pcie_config_pemon(node, pcie_port, 0);
			}

			/* Reset the port */
			ciu_soft_prst.s.soft_prst = 1;
			CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);

			/* Read to make sure write happens */
			ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);

			/* Keep PERST asserted for 2 ms */
			cvmx_wait_usec(2000);

			if (requires_pem_reset) {
				/* Reset GSER_PHY to put in a clean state */
				__cvmx_pcie_gser_phy_config(node, pcie_port, qlm);
				requires_pem_reset = 2;

				/* Enable MAC controller before taking pcie out of reset */
				__cvmx_pcie_config_pemon(node, pcie_port, 1);
			}
		}

		/* Deassert PERST */
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
		ciu_soft_prst.s.soft_prst = 0;
		CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);

		/* Check if PLLs are locked after GSER_PHY reset. */
		if (requires_pem_reset == 2) {
			cvmx_pemx_cfg_t pemx_cfg;
			pemx_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(pcie_port));
			if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm),
				   cvmx_gserx_qlm_stat_t, rst_rdy, ==, 1, 10000)) {
				cvmx_warn("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
				return -1;
			}
			if (pemx_cfg.cn78xx.lanes8
			    && (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm + 1),
				cvmx_gserx_qlm_stat_t, rst_rdy, ==, 1, 10000))) {
				cvmx_warn("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm+1);
				return -1;
			}
		}
	}

	/* Wait 1ms for PCIe reset to complete */
	cvmx_wait_usec(1000);

	/* Set MPLL multiplier as per Errata 20669. */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		int qlm = __cvmx_pcie_get_qlm(0, pcie_port);
		int old_mult;
		uint64_t meas_refclock = cvmx_qlm_measure_clock(qlm);
		if (meas_refclock > 99000000 && meas_refclock < 101000000)
			old_mult = 35;
		else if (meas_refclock > 124000000 && meas_refclock < 126000000)
			old_mult = 56;
		else if (meas_refclock > 156000000 && meas_refclock < 156500000)
			old_mult = 45;
		else {
			cvmx_dprintf("%s: Invalid reference clock for qlm %d\n",
				     __func__, qlm);
			return -1;
		}
		__cvmx_qlm_set_mult(qlm, 2500, old_mult);
		/* Adjust mplls for both dlms when configured as pcie 1x4 */
		if ((mode == CVMX_QLM_MODE_PCIE) && (pcie_port == 0))
			__cvmx_qlm_set_mult(qlm + 1, 2500, old_mult);
	}

	/* Check and make sure PCIe came out of reset. If it doesn't the board
	   probably hasn't wired the clocks up and the interface should be
	   skipped */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, rst_ctl_reg, cvmx_mio_rst_ctlx_t,
				  rst_done, ==, 1, 10000)) {
		cvmx_printf("%d:PCIe: Port %d stuck in reset, skipping.\n",
			    node, pcie_port);
		return -1;
	}

	/* Check BIST status */
	pemx_bist_status.u64 = CVMX_READ_CSR(CVMX_PEMX_BIST_STATUS(pcie_port));
	if (pemx_bist_status.u64)
		cvmx_printf("%d:PCIe: BIST FAILED for port %d (0x%016llx)\n",
			    node, pcie_port, CAST64(pemx_bist_status.u64));
	/* BIST_STATUS2 is not present on some of Octeon3 models. */
	if (OCTEON_IS_OCTEON3() && !OCTEON_IS_MODEL(OCTEON_CN70XX))
		pemx_bist_status2.u64 = 0;
	else
		pemx_bist_status2.u64 = CVMX_READ_CSR(CVMX_PEMX_BIST_STATUS2(pcie_port));
	/* Errata PCIE-14766 may cause the lower 6 bits to be randomly set on CN63XXp1 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X))
		pemx_bist_status2.u64 &= ~0x3full;
	if (pemx_bist_status2.u64)
		cvmx_printf("%d:PCIe: BIST2 FAILED for port %d (0x%016llx)\n",
			    node, pcie_port, CAST64(pemx_bist_status2.u64));

	/* Initialize the config space CSRs */
	__cvmx_pcie_rc_initialize_config_space(node, pcie_port);

	/* Enable gen2 speed selection */
	pciercx_cfg515.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG515(pcie_port));
	pciercx_cfg515.s.dsc = 1;
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG515(pcie_port),
			     pciercx_cfg515.u32);

	/* Bring the link up */
	if (__cvmx_pcie_rc_initialize_link_gen2(node, pcie_port)) {
		/* Some gen1 devices don't handle the gen 2 training correctly.
		 * Disable gen2 and try again with only gen1
		 */
		if (OCTEON_IS_OCTEON3() && !OCTEON_IS_MODEL(OCTEON_CN70XX)) {
			cvmx_printf("%d:PCIe: Link timeout on port %d, probably the slot is empty\n",
				    node, pcie_port);
			return -1;
		} else {
			cvmx_pciercx_cfg031_t pciercx_cfg031;
			pciercx_cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG031(pcie_port));
			pciercx_cfg031.s.mls = 1;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port),
				     		pciercx_cfg031.u32);
			if (__cvmx_pcie_rc_initialize_link_gen2(node, pcie_port)) {
				cvmx_printf("PCIe: Link timeout on port %d, probably the slot is empty\n",
					    pcie_port);
				return -1;
			}
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)
	    || OCTEON_IS_MODEL(OCTEON_CN73XX)
	    || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_pciercx_cfg038_t pciercx_cfg038;
		cvmx_pciercx_cfg548_t cfg548;

		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
					CVMX_PCIERCX_CFG032(pcie_port));
		/* Errata PEM-28816: Link retrain initiated at GEN1 can cause PCIE
		   link to hang. For Gen1 links we must disable equalization */
		if (pciercx_cfg032.s.ls == 1) {
#if 0
			cvmx_dprintf("%d:%d:PCIe: Disabling equalization for GEN1 Link\n",
					node, pcie_port);
#endif
			cfg548.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
					CVMX_PCIERCX_CFG548(pcie_port));
			cfg548.s.ed = 1;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG548(pcie_port),
					cfg548.u32);
		}

		/* Errata PCIE-29440: Atomic operations to work properly */
		pciercx_cfg038.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
					CVMX_PCIERCX_CFG038(pcie_port));
		pciercx_cfg038.s.atom_op_eb = 0;
		pciercx_cfg038.s.atom_op = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG038(pcie_port),
					pciercx_cfg038.u32);

		/* Errata PCIE-29566 PEM Link Hangs after going into L1 */
		cfg548.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG548(pcie_port));
		cfg548.s.grizdnc = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG548(pcie_port), cfg548.u32);
	}

	/* Store merge control (SLI_MEM_ACCESS_CTL[TIMER,MAX_WORD]) */
	sli_mem_access_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_MEM_ACCESS_CTL);
	sli_mem_access_ctl.s.max_word = 0;	/* Allow 16 words to combine */
	sli_mem_access_ctl.s.timer = 127;	/* Wait up to 127 cycles for more data */
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_CTL, sli_mem_access_ctl.u64);

	/* Setup Mem access SubDIDs */
	mem_access_subid.u64 = 0;
	mem_access_subid.s.port = pcie_port;	/* Port the request is sent to. */
	mem_access_subid.s.nmerge = 0;	/* Allow merging as it works on CN6XXX. */
	mem_access_subid.s.esr = _CVMX_PCIE_ES;	/* Endian-swap for Reads. */
	mem_access_subid.s.esw = _CVMX_PCIE_ES;	/* Endian-swap for Writes. */
	mem_access_subid.s.wtype = 0;	/* "No snoop" and "Relaxed ordering" are not set */
	mem_access_subid.s.rtype = 0;	/* "No snoop" and "Relaxed ordering" are not set */
	/* PCIe Adddress Bits <63:34>. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		mem_access_subid.cn68xx.ba = 0;
	else
		mem_access_subid.cn63xx.ba = 0;

	/* Setup mem access 12-15 for port 0, 16-19 for port 1, supplying 36
	 * bits of address space
	 */
	for (i = 12 + pcie_port * 4; i < 16 + pcie_port * 4; i++) {
		CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_SUBIDX(i),
			       mem_access_subid.u64);
		/* Set each SUBID to extend the addressable range */
		__cvmx_increment_ba(&mem_access_subid);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN63XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN66XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN68XX) ||
	    (OCTEON_IS_OCTEON3() && !OCTEON_IS_MODEL(OCTEON_CN70XX))) {
		/* Disable the peer to peer forwarding register. This must be
		 * setup by the OS after it enumerates the bus and assigns
		 * addresses to the PCIe busses
		 */
		for (i = 0; i < 4; i++) {
			CVMX_WRITE_CSR(CVMX_PEMX_P2P_BARX_START(i, pcie_port), -1);
			CVMX_WRITE_CSR(CVMX_PEMX_P2P_BARX_END(i, pcie_port), -1);
		}
	}

	/* Set Octeon's BAR0 to decode 0-16KB. It overlaps with Bar2 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR0_START(pcie_port), 0);

	/* Set Octeon's BAR2 to decode 0-2^41. Bar0 and Bar1 take precedence
	 * where they overlap. It also overlaps with the device addresses, so
	 * make sure the peer to peer forwarding is set right
	 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR2_START(pcie_port), 0);

	/* Setup BAR2 attributes */
	/* Relaxed Ordering (NPEI_CTL_PORTn[PTLP_RO,CTLP_RO, WAIT_COM]) */
	/* ­ PTLP_RO,CTLP_RO should normally be set (except for debug). */
	/* ­ WAIT_COM=0 will likely work for all applications. */
	/* Load completion relaxed ordering (NPEI_CTL_PORTn[WAITL_COM]) */
	pemx_bar_ctl.u64 = CVMX_READ_CSR(CVMX_PEMX_BAR_CTL(pcie_port));
	pemx_bar_ctl.s.bar1_siz = 3;	/* 256MB BAR1 */
	pemx_bar_ctl.s.bar2_enb = 1;
	pemx_bar_ctl.s.bar2_esx = _CVMX_PCIE_ES;
	pemx_bar_ctl.s.bar2_cax = 0;
	CVMX_WRITE_CSR(CVMX_PEMX_BAR_CTL(pcie_port), pemx_bar_ctl.u64);
	sli_ctl_portx.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_CTL_PORTX(pcie_port));
	sli_ctl_portx.s.ptlp_ro = 1;
	sli_ctl_portx.s.ctlp_ro = 1;
	sli_ctl_portx.s.wait_com = 0;
	sli_ctl_portx.s.waitl_com = 0;
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_CTL_PORTX(pcie_port), sli_ctl_portx.u64);

	/* BAR1 follows BAR2 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR1_START(pcie_port),
		       CVMX_PCIE_BAR1_RC_BASE);

	bar1_index.u64 = 0;
	bar1_index.s.addr_idx = (CVMX_PCIE_BAR1_PHYS_BASE >> 22);
	bar1_index.s.ca = 1;	/* Not Cached */
	bar1_index.s.end_swp = _CVMX_PCIE_ES;	/* Endian Swap mode */
	bar1_index.s.addr_v = 1;	/* Valid entry */

	for (i = 0; i < 16; i++) {
		CVMX_WRITE_CSR(CVMX_PEMX_BAR1_INDEXX(i, pcie_port),
			       bar1_index.u64);
		/* 256MB / 16 >> 22 == 4 */
		bar1_index.s.addr_idx += (((1ull << 28) / 16ull) >> 22);
	}

	/* Wait for 200ms */
	pemx_ctl_status.u64 = CVMX_READ_CSR(CVMX_PEMX_CTL_STATUS(pcie_port));
	pemx_ctl_status.cn63xx.cfg_rtry = cfg_retries();
	CVMX_WRITE_CSR(CVMX_PEMX_CTL_STATUS(pcie_port), pemx_ctl_status.u64);

	/* Here is the second part of the config retry changes. Wait for 700ms
	   after setting up the link before continuing. PCIe says the devices
	   may need up to 900ms to come up. 700ms plus 200ms from above gives 
	   us a total of 900ms */
	cvmx_wait_usec(700000);

	/* Display the link status */
	pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG032(pcie_port));
	cvmx_printf("PCIe: Port %d link active, %d lanes, speed gen%d \n",
		    pcie_port, pciercx_cfg032.s.nlw, pciercx_cfg032.s.ls);

	pcie_link_initialized[node][pcie_port] = true;
	return 0;
}

/**
 * Initialize a PCIe port for use in host(RC) mode. It doesn't enumerate the bus.
 *
 * @param pcie_port PCIe port to initialize for a node
 *
 * @return Zero on success
 */
int cvmx_pcie_rc_initialize(int pcie_port)
{
	int result;
	if (octeon_has_feature(OCTEON_FEATURE_NPEI))
		result = __cvmx_pcie_rc_initialize_gen1(pcie_port);
	else
		result = __cvmx_pcie_rc_initialize_gen2(pcie_port);

#if !defined(CVMX_BUILD_FOR_LINUX_KERNEL) || defined(CONFIG_CAVIUM_DECODE_RSL)
	if (result == 0)
		cvmx_error_enable_group(CVMX_ERROR_GROUP_PCI, pcie_port);
#endif
	return result;
}
EXPORT_SYMBOL(cvmx_pcie_rc_initialize);

/**
 * Shutdown a PCIe port and put it in reset
 *
 * @param pcie_port PCIe port to shutdown for a node
 *
 * @return Zero on success
 */
int cvmx_pcie_rc_shutdown(int pcie_port)
{
	uint64_t ciu_soft_prst_reg;
	cvmx_ciu_soft_prst_t ciu_soft_prst;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
#if !defined(CVMX_BUILD_FOR_LINUX_KERNEL) || defined(CONFIG_CAVIUM_DECODE_RSL)
	cvmx_error_disable_group(CVMX_ERROR_GROUP_PCI, pcie_port);
#endif
	/* Wait for all pending operations to complete */
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		if (CVMX_WAIT_FOR_FIELD64(CVMX_PESCX_CPL_LUT_VALID(pcie_port),
			cvmx_pescx_cpl_lut_valid_t, tag, ==, 0, 2000))
			cvmx_dprintf("PCIe: Port %d shutdown timeout\n",
				     pcie_port);
	} else {
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_CPL_LUT_VALID(pcie_port),
					  cvmx_pemx_cpl_lut_valid_t, tag, ==,
					  0, 2000))
			cvmx_dprintf("PCIe: Port %d shutdown timeout\n",
				     pcie_port);
	}
	if (OCTEON_IS_OCTEON3()) {
		ciu_soft_prst_reg = CVMX_RST_SOFT_PRSTX(pcie_port);
	} else {
		ciu_soft_prst_reg = (pcie_port) ?
				    CVMX_CIU_SOFT_PRST1 : CVMX_CIU_SOFT_PRST;
	}

	/* Force reset */
	ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
	ciu_soft_prst.s.soft_prst = 1;
	CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);

	return 0;
}

/**
 * @INTERNAL
 * Build a PCIe config space request address for a device
 *
 * @param node	    node
 * @param port	    PCIe port (relative to the node) to access
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * @return 64bit Octeon IO address
 */
static uint64_t __cvmx_pcie_build_config_addr(int node, int port, int bus,
					      int dev, int fn, int reg)
{
	cvmx_pcie_address_t pcie_addr;
	cvmx_pciercx_cfg006_t pciercx_cfg006;

	pciercx_cfg006.u32 = cvmx_pcie_cfgx_read_node(node, port,
						      CVMX_PCIERCX_CFG006(port));
	if ((bus <= pciercx_cfg006.s.pbnum) && (dev != 0))
		return 0;

	pcie_addr.u64 = 0;
	pcie_addr.config.upper = 2;
	pcie_addr.config.io = 1;
	pcie_addr.config.did = 3;
	pcie_addr.config.subdid = 1;
	pcie_addr.config.node = node;
	pcie_addr.config.es = _CVMX_PCIE_ES;
	pcie_addr.config.port = port;
	/* Always use config type 0 */
	if (pciercx_cfg006.s.pbnum == 0)
		pcie_addr.config.ty = (bus > pciercx_cfg006.s.pbnum + 1);
	else
		pcie_addr.config.ty = (bus > pciercx_cfg006.s.pbnum);
	pcie_addr.config.bus = bus;
	pcie_addr.config.dev = dev;
	pcie_addr.config.func = fn;
	pcie_addr.config.reg = reg;
	return pcie_addr.u64;
}

/**
 * Read 8bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * @return Result of the read
 */
uint8_t cvmx_pcie_config_read8(int pcie_port, int bus, int dev, int fn, int reg)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address)
		return cvmx_read64_uint8(address);
	else
		return 0xff;
}

/**
 * Read 16bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * @return Result of the read
 */
uint16_t cvmx_pcie_config_read16(int pcie_port, int bus, int dev,
				 int fn, int reg)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address)
		return cvmx_le16_to_cpu(cvmx_read64_uint16(address));
	else
		return 0xffff;
}

/**
 * Read 32bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * @return Result of the read
 */
uint32_t cvmx_pcie_config_read32(int pcie_port, int bus, int dev,
				 int fn, int reg)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address && pcie_link_initialized[node][pcie_port])
		return cvmx_le32_to_cpu(cvmx_read64_uint32(address));
	else
		return 0xffffffff;
}

/**
 * Write 8bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write8(int pcie_port, int bus, int dev, int fn,
			     int reg, uint8_t val)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address)
		cvmx_write64_uint8(address, val);
}

/**
 * Write 16bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write16(int pcie_port, int bus, int dev, int fn,
			      int reg, uint16_t val)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address)
		cvmx_write64_uint16(address, cvmx_cpu_to_le16(val));
}

/**
 * Write 32bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write32(int pcie_port, int bus, int dev, int fn,
			      int reg, uint32_t val)
{
	uint64_t address;
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev,
							 fn, reg);
	if (address)
		cvmx_write64_uint32(address, cvmx_cpu_to_le32(val));
}

/**
 * Read a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to read from
 * @param cfg_offset Address to read
 *
 * @return Value read
 */
uint32_t cvmx_pcie_cfgx_read(int pcie_port, uint32_t cfg_offset)
{
	return cvmx_pcie_cfgx_read_node(0, pcie_port, cfg_offset);
}

uint32_t cvmx_pcie_cfgx_read_node(int node, int pcie_port, uint32_t cfg_offset)
{
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_pescx_cfg_rd_t pescx_cfg_rd;
		pescx_cfg_rd.u64 = 0;
		pescx_cfg_rd.s.addr = cfg_offset;
		cvmx_write_csr(CVMX_PESCX_CFG_RD(pcie_port), pescx_cfg_rd.u64);
		pescx_cfg_rd.u64 = cvmx_read_csr(CVMX_PESCX_CFG_RD(pcie_port));
		return pescx_cfg_rd.s.data;
	} else {
		cvmx_pemx_cfg_rd_t pemx_cfg_rd;
		pemx_cfg_rd.u64 = 0;
		pemx_cfg_rd.s.addr = cfg_offset;
		CVMX_WRITE_CSR(CVMX_PEMX_CFG_RD(pcie_port), pemx_cfg_rd.u64);
		pemx_cfg_rd.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG_RD(pcie_port));
		return pemx_cfg_rd.s.data;
	}
}

/**
 * Write a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to write to
 * @param cfg_offset Address to write
 * @param val        Value to write
 */
void cvmx_pcie_cfgx_write(int pcie_port, uint32_t cfg_offset, uint32_t val)
{
	cvmx_pcie_cfgx_write_node(0, pcie_port, cfg_offset, val);
}

void cvmx_pcie_cfgx_write_node(int node, int pcie_port, uint32_t cfg_offset, uint32_t val)
{
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_pescx_cfg_wr_t pescx_cfg_wr;
		pescx_cfg_wr.u64 = 0;
		pescx_cfg_wr.s.addr = cfg_offset;
		pescx_cfg_wr.s.data = val;
		cvmx_write_csr(CVMX_PESCX_CFG_WR(pcie_port), pescx_cfg_wr.u64);
	} else {
		cvmx_pemx_cfg_wr_t pemx_cfg_wr;
		pemx_cfg_wr.u64 = 0;
		pemx_cfg_wr.s.addr = cfg_offset;
		pemx_cfg_wr.s.data = val;
		CVMX_WRITE_CSR(CVMX_PEMX_CFG_WR(pcie_port), pemx_cfg_wr.u64);
	}
}

extern int cvmx_pcie_is_host_mode(int pcie_port);

/**
 * Initialize a PCIe port for use in target(EP) mode.
 *
 * @param pcie_port PCIe port to initialize for a node
 *
 * @return Zero on success
 */
int cvmx_pcie_ep_initialize(int pcie_port)
{
	int node = (pcie_port >> 4) & 0x3;
	if (cvmx_pcie_is_host_mode(pcie_port))
		return -1;

	pcie_port &= 0x3;

	/* CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis be
	 * programmed
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		if (pcie_port) {
			cvmx_ciu_qlm1_t ciu_qlm;
			ciu_qlm.u64 = cvmx_read_csr(CVMX_CIU_QLM1);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			cvmx_write_csr(CVMX_CIU_QLM1, ciu_qlm.u64);
		} else {
			cvmx_ciu_qlm0_t ciu_qlm;
			ciu_qlm.u64 = cvmx_read_csr(CVMX_CIU_QLM0);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			cvmx_write_csr(CVMX_CIU_QLM0, ciu_qlm.u64);
		}
	}

	/* Enable bus master and memory */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIEEPX_CFG001(pcie_port), 0x6);

	/* Max Payload Size (PCIE*_CFG030[MPS]) */
	/* Max Read Request Size (PCIE*_CFG030[MRRS]) */
	/* Relaxed-order, no-snoop enables (PCIE*_CFG030[RO_EN,NS_EN] */
	/* Error Message Enables (PCIE*_CFG030[CE_EN,NFE_EN,FE_EN,UR_EN]) */
	{
		cvmx_pcieepx_cfg030_t pcieepx_cfg030;
		pcieepx_cfg030.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIEEPX_CFG030(pcie_port));
		if (OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
			pcieepx_cfg030.s.mps = MPS_CN5XXX;
			pcieepx_cfg030.s.mrrs = MRRS_CN5XXX;
		} else {
			pcieepx_cfg030.s.mps = MPS_CN6XXX;
			pcieepx_cfg030.s.mrrs = MRRS_CN6XXX;
		}
		pcieepx_cfg030.s.ro_en = 1;	/* Enable relaxed ordering. */
		pcieepx_cfg030.s.ns_en = 1;	/* Enable no snoop. */
		pcieepx_cfg030.s.ce_en = 1;	/* Correctable error reporting enable. */
		pcieepx_cfg030.s.nfe_en = 1;	/* Non-fatal error reporting enable. */
		pcieepx_cfg030.s.fe_en = 1;	/* Fatal error reporting enable. */
		pcieepx_cfg030.s.ur_en = 1;	/* Unsupported request reporting enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIEEPX_CFG030(pcie_port),
				     pcieepx_cfg030.u32);
	}

	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		/* Max Payload Size (NPEI_CTL_STATUS2[MPS]) must match
		 * PCIE*_CFG030[MPS]
		 */
		/* Max Read Request Size (NPEI_CTL_STATUS2[MRRS]) must not
		 * exceed PCIE*_CFG030[MRRS]
		 */
		cvmx_npei_ctl_status2_t npei_ctl_status2;
		npei_ctl_status2.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_CTL_STATUS2);
		npei_ctl_status2.s.mps = MPS_CN5XXX;	/* Max payload size = 128 bytes (Limit of most PCs) */
		npei_ctl_status2.s.mrrs = MRRS_CN5XXX;	/* Max read request size = 128 bytes for best Octeon DMA performance */
		cvmx_write_csr(CVMX_PEXP_NPEI_CTL_STATUS2, npei_ctl_status2.u64);
	} else {
		/* Max Payload Size (DPI_SLI_PRTX_CFG[MPS]) must match
		 * PCIE*_CFG030[MPS]
		 */
		/* Max Read Request Size (DPI_SLI_PRTX_CFG[MRRS]) must not
		 * exceed PCIE*_CFG030[MRRS]
		 */
		cvmx_dpi_sli_prtx_cfg_t prt_cfg;
		cvmx_sli_s2m_portx_ctl_t sli_s2m_portx_ctl;
		prt_cfg.u64 = CVMX_READ_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port));
		prt_cfg.s.mps = MPS_CN6XXX;
		prt_cfg.s.mrrs = MRRS_CN6XXX;
		/* Max outstanding load request. */
		prt_cfg.s.molr = 32;
		CVMX_WRITE_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port), prt_cfg.u64);

		sli_s2m_portx_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port));
		if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)
		      || OCTEON_IS_MODEL(OCTEON_CN73XX)
		      || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
			sli_s2m_portx_ctl.cn61xx.mrrs = MRRS_CN6XXX;
		CVMX_WRITE_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port), sli_s2m_portx_ctl.u64);
	}

	/* Setup Mem access SubDID 12 to access Host memory */
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_npei_mem_access_subidx_t mem_access_subid;
		mem_access_subid.u64 = 0;
		mem_access_subid.s.port = pcie_port;	/* Port the request is sent to. */
		mem_access_subid.s.nmerge = 1;	/* Merging is not allowed in this window. */
		mem_access_subid.s.esr = 0;	/* Endian-swap for Reads. */
		mem_access_subid.s.esw = 0;	/* Endian-swap for Writes. */
		mem_access_subid.s.nsr = 0;	/* Enable Snooping for Reads. Octeon doesn't care, but devices might want this more conservative setting */
		mem_access_subid.s.nsw = 0;	/* Enable Snoop for Writes. */
		mem_access_subid.s.ror = 0;	/* Disable Relaxed Ordering for Reads. */
		mem_access_subid.s.row = 0;	/* Disable Relaxed Ordering for Writes. */
		mem_access_subid.s.ba = 0;	/* PCIe Adddress Bits <63:34>. */
		cvmx_write_csr(CVMX_PEXP_NPEI_MEM_ACCESS_SUBIDX(12), mem_access_subid.u64);
	} else {
		cvmx_sli_mem_access_subidx_t mem_access_subid;
		mem_access_subid.u64 = 0;
		mem_access_subid.s.port = pcie_port;	/* Port the request is sent to. */
		mem_access_subid.s.nmerge = 0;	/* Merging is allowed in this window. */
		mem_access_subid.s.esr = 0;	/* Endian-swap for Reads. */
		mem_access_subid.s.esw = 0;	/* Endian-swap for Writes. */
		mem_access_subid.s.wtype = 0;	/* "No snoop" and "Relaxed ordering" are not set */
		mem_access_subid.s.rtype = 0;	/* "No snoop" and "Relaxed ordering" are not set */
		/* PCIe Adddress Bits <63:34>. */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			mem_access_subid.cn68xx.ba = 0;
		else
			mem_access_subid.cn63xx.ba = 0;
		CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_SUBIDX(12 + pcie_port * 4), mem_access_subid.u64);
	}
	return 0;
}

/**
 * Wait for posted PCIe read/writes to reach the other side of
 * the internal PCIe switch. This will insure that core
 * read/writes are posted before anything after this function
 * is called. This may be necessary when writing to memory that
 * will later be read using the DMA/PKT engines.
 *
 * @param pcie_port PCIe port to wait for
 */
void cvmx_pcie_wait_for_pending(int pcie_port)
{
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_npei_data_out_cnt_t npei_data_out_cnt;
		int a;
		int b;
		int c;

		/* See section 9.8, PCIe Core-initiated Requests, in the manual
		 * for a description of how this code works
		 */
		npei_data_out_cnt.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DATA_OUT_CNT);
		if (pcie_port) {
			if (!npei_data_out_cnt.s.p1_fcnt)
				return;
			a = npei_data_out_cnt.s.p1_ucnt;
			b = (a + npei_data_out_cnt.s.p1_fcnt - 1) & 0xffff;
		} else {
			if (!npei_data_out_cnt.s.p0_fcnt)
				return;
			a = npei_data_out_cnt.s.p0_ucnt;
			b = (a + npei_data_out_cnt.s.p0_fcnt - 1) & 0xffff;
		}

		while (1) {
			npei_data_out_cnt.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DATA_OUT_CNT);
			c = (pcie_port) ? npei_data_out_cnt.s.p1_ucnt : npei_data_out_cnt.s.p0_ucnt;
			if (a <= b) {
				if ((c < a) || (c > b))
					return;
			} else {
				if ((c > b) && (c < a))
					return;
			}
		}
	} else {
		cvmx_sli_data_out_cnt_t sli_data_out_cnt;
		int a;
		int b;
		int c;

		sli_data_out_cnt.u64 = cvmx_read_csr(CVMX_PEXP_SLI_DATA_OUT_CNT);
		if (pcie_port) {
			if (!sli_data_out_cnt.s.p1_fcnt)
				return;
			a = sli_data_out_cnt.s.p1_ucnt;
			b = (a + sli_data_out_cnt.s.p1_fcnt - 1) & 0xffff;
		} else {
			if (!sli_data_out_cnt.s.p0_fcnt)
				return;
			a = sli_data_out_cnt.s.p0_ucnt;
			b = (a + sli_data_out_cnt.s.p0_fcnt - 1) & 0xffff;
		}

		while (1) {
			sli_data_out_cnt.u64 = cvmx_read_csr(CVMX_PEXP_SLI_DATA_OUT_CNT);
			c = (pcie_port) ? sli_data_out_cnt.s.p1_ucnt : sli_data_out_cnt.s.p0_ucnt;
			if (a <= b) {
				if ((c < a) || (c > b))
					return;
			} else {
				if ((c > b) && (c < a))
					return;
			}
		}
	}
}

/**
 * Returns if a PCIe port is in host or target mode.
 *
 * @param pcie_port PCIe port number (PEM number)
 *
 * @return 0 if PCIe port is in target mode, !0 if in host mode.
 */
int cvmx_pcie_is_host_mode(int pcie_port)
{
	int node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)
	    || OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_pemx_strap_t strap;
		strap.u64 = CVMX_READ_CSR(CVMX_PEMX_STRAP(pcie_port));
		return (strap.cn78xx.pimode == 3);
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_rst_ctlx_t rst_ctl;
		rst_ctl.u64 = cvmx_read_csr(CVMX_RST_CTLX(pcie_port));
		return !!rst_ctl.s.host_mode;
	} else if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_npei_ctl_status_t npei_ctl_status;
		npei_ctl_status.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_CTL_STATUS);
		return !!npei_ctl_status.s.host_mode;
	} else {
		cvmx_mio_rst_ctlx_t mio_rst_ctl;

		mio_rst_ctl.u64 = cvmx_read_csr(CVMX_MIO_RST_CTLX(pcie_port));
		if (OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		    OCTEON_IS_MODEL(OCTEON_CNF71XX))
			return mio_rst_ctl.s.prtmode != 0;
		else
			return !!mio_rst_ctl.s.host_mode;
	}
}
