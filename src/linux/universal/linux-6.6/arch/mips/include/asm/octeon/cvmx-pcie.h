/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
 * <hr>$Revision: 148290 $<hr>
 */

#ifndef __CVMX_PCIE_H__
#define __CVMX_PCIE_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#elif defined(__U_BOOT__)
#include <asm/arch/cvmx.h>
#else
#include <cvmx.h>
#endif

#define CVMX_PCIE_MAX_PORTS	4
#define CVMX_PCIE_PORTS		((OCTEON_IS_MODEL(OCTEON_CN78XX)	\
				  || OCTEON_IS_MODEL(OCTEON_CN73XX)) 	\
				 ? CVMX_PCIE_MAX_PORTS   		\
				 : (OCTEON_IS_MODEL(OCTEON_CN70XX) ? 3 : 2))

/*
 * The physical memory base mapped by BAR1.  256MB at the end of the
 * first 4GB.
 */
#define CVMX_PCIE_BAR1_PHYS_BASE ((1ull << 32) - (1ull << 28))
#define CVMX_PCIE_BAR1_PHYS_SIZE (1ull << 28)

/*
 * The RC base of BAR1.  gen1 has a 39-bit BAR2, gen2 has 41-bit BAR2,
 * place BAR1 so it is the same for both.
 */
#define CVMX_PCIE_BAR1_RC_BASE (1ull << 41)

typedef union {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD	/* A Linux compatible proxy for __BIG_ENDIAN */
	struct {
		uint64_t upper:2;	/* Normally 2 for XKPHYS */
		uint64_t reserved_49_61:13;	/* Must be zero */
		uint64_t io:1;	/* 1 for IO space access */
		uint64_t did:5;	/* PCIe DID = 3 */
		uint64_t subdid:3;	/* PCIe SubDID = 1 */
		uint64_t reserved_38_39:2;	/* Must be zero */
		uint64_t node:2;		/* Numa node number */
		uint64_t es:2;	/* Endian swap = 1 */
		uint64_t port:2;	/* PCIe port 0,1 */
		uint64_t reserved_29_31:3;	/* Must be zero */
		uint64_t ty:1;	/* Selects the type of the configuration request (0 = type 0, 1 = type 1). */
		uint64_t bus:8;	/* Target bus number sent in the ID in the request. */
		uint64_t dev:5;	/* Target device number sent in the ID in the request. Note that Dev must be
				   zero for type 0 configuration requests. */
		uint64_t func:3;	/* Target function number sent in the ID in the request. */
		uint64_t reg:12;	/* Selects a register in the configuration space of the target. */
	} config;
	struct {
		uint64_t upper:2;	/* Normally 2 for XKPHYS */
		uint64_t reserved_49_61:13;	/* Must be zero */
		uint64_t io:1;	/* 1 for IO space access */
		uint64_t did:5;	/* PCIe DID = 3 */
		uint64_t subdid:3;	/* PCIe SubDID = 2 */
		uint64_t reserved_38_39:2;	/* Must be zero */
		uint64_t node:2;		/* Numa node number */
		uint64_t es:2;	/* Endian swap = 1 */
		uint64_t port:2;	/* PCIe port 0,1 */
		uint64_t address:32;	/* PCIe IO address */
	} io;
	struct {
		uint64_t upper:2;	/* Normally 2 for XKPHYS */
		uint64_t reserved_49_61:13;	/* Must be zero */
		uint64_t io:1;	/* 1 for IO space access */
		uint64_t did:5;	/* PCIe DID = 3 */
		uint64_t subdid:3;	/* PCIe SubDID = 3-6 */
		uint64_t reserved_38_39:2;	/* Must be zero */
		uint64_t node:2;		/* Numa node number */
		uint64_t address:36;	/* PCIe Mem address */
	} mem;
#else
	struct {
		uint64_t reg:12;
		uint64_t func:3;
		uint64_t dev:5;
		uint64_t bus:8;
		uint64_t ty:1;
		uint64_t reserved_29_31:3;
		uint64_t port:2;
		uint64_t es:2;
		uint64_t node:2;
		uint64_t reserved_38_39:2;
		uint64_t subdid:3;
		uint64_t did:5;
		uint64_t io:1;
		uint64_t reserved_49_61:13;
		uint64_t upper:2;
	} config;
	struct {
		uint64_t address:32;
		uint64_t port:2;
		uint64_t es:2;
		uint64_t node:2;
		uint64_t reserved_38_39:2;
		uint64_t subdid:3;
		uint64_t did:5;
		uint64_t io:1;
		uint64_t reserved_49_61:13;
		uint64_t upper:2;
	} io;
	struct {
		uint64_t address:36;
		uint64_t node:2;
		uint64_t reserved_38_39:2;
		uint64_t subdid:3;
		uint64_t did:5;
		uint64_t io:1;
		uint64_t reserved_49_61:13;
		uint64_t upper:2;
	} mem;
#endif
} cvmx_pcie_address_t;

/**
 * Return the Core virtual base address for PCIe IO access. IOs are
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_io_base_address(int pcie_port);

/**
 * Size of the IO address region returned at address
 * cvmx_pcie_get_io_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return Size of the IO window
 */
uint64_t cvmx_pcie_get_io_size(int pcie_port);

/**
 * Return the Core virtual base address for PCIe MEM access. Memory is
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_mem_base_address(int pcie_port);

/**
 * Size of the Mem address region returned at address
 * cvmx_pcie_get_mem_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * @return Size of the Mem window
 */
uint64_t cvmx_pcie_get_mem_size(int pcie_port);

/**
 * Initialize a PCIe port for use in host(RC) mode. It doesn't enumerate the bus.
 *
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
int cvmx_pcie_rc_initialize(int pcie_port);

/**
 * Shutdown a PCIe port and put it in reset
 *
 * @param pcie_port PCIe port to shutdown
 *
 * @return Zero on success
 */
int cvmx_pcie_rc_shutdown(int pcie_port);

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
uint8_t cvmx_pcie_config_read8(int pcie_port, int bus, int dev, int fn, int reg);

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
uint16_t cvmx_pcie_config_read16(int pcie_port, int bus, int dev, int fn, int reg);

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
uint32_t cvmx_pcie_config_read32(int pcie_port, int bus, int dev, int fn, int reg);

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
void cvmx_pcie_config_write8(int pcie_port, int bus, int dev, int fn, int reg, uint8_t val);

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
void cvmx_pcie_config_write16(int pcie_port, int bus, int dev, int fn, int reg, uint16_t val);

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
void cvmx_pcie_config_write32(int pcie_port, int bus, int dev, int fn, int reg, uint32_t val);

/**
 * Read a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to read from
 * @param cfg_offset Address to read
 *
 * @return Value read
 */
uint32_t cvmx_pcie_cfgx_read(int pcie_port, uint32_t cfg_offset);
uint32_t cvmx_pcie_cfgx_read_node(int node, int pcie_port, uint32_t cfg_offset);

/**
 * Write a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to write to
 * @param cfg_offset Address to write
 * @param val        Value to write
 */
void cvmx_pcie_cfgx_write(int pcie_port, uint32_t cfg_offset, uint32_t val);
void cvmx_pcie_cfgx_write_node(int node, int pcie_port, uint32_t cfg_offset, uint32_t val);

/**
 * Write a 32bit value to the Octeon NPEI register space
 *
 * @param address Address to write to
 * @param val     Value to write
 */
static inline void cvmx_pcie_npei_write32(uint64_t address, uint32_t val)
{
	cvmx_write64_uint32(address ^ 4, val);
	cvmx_read64_uint32(address ^ 4);
}

/**
 * Read a 32bit value from the Octeon NPEI register space
 *
 * @param address Address to read
 * @return The result
 */ static inline uint32_t cvmx_pcie_npei_read32(uint64_t address)
{
	return cvmx_read64_uint32(address ^ 4);
}

/**
 * Initialize a PCIe port for use in target(EP) mode.
 *
 * @param pcie_port PCIe port to initialize
 *
 * @return Zero on success
 */
int cvmx_pcie_ep_initialize(int pcie_port);

/**
 * Wait for posted PCIe read/writes to reach the other side of
 * the internal PCIe switch. This will insure that core
 * read/writes are posted before anything after this function
 * is called. This may be necessary when writing to memory that
 * will later be read using the DMA/PKT engines.
 *
 * @param pcie_port PCIe port to wait for
 */
void cvmx_pcie_wait_for_pending(int pcie_port);

/**
 * Returns if a PCIe port is in host or target mode.
 *
 * @param pcie_port PCIe port number (PEM number)
 *
 * @return 0 if PCIe port is in target mode, !0 if in host mode.
 */
int cvmx_pcie_is_host_mode(int pcie_port);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif
