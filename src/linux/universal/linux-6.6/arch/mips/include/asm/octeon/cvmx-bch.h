/***********************license start***************
 * Copyright (c) 2013  Cavium Inc. (support@cavium.com). All rights
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
 *
 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION
 * OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to the CN7XXX hardware BCH engine.
 *
 * <hr>$Revision: 79788 $<hr>
 */

#ifndef __CVMX_BCH_H__
#define __CVMX_BCH_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
# include <asm/octeon/octeon.h>
# include <asm/octeon/cvmx-bch-defs.h>
# include <asm/octeon/cvmx-fpa.h>
#elif defined(CVMX_BUILD_FOR_UBOOT)
# include <common.h>
# include <asm/arch/cvmx.h>
# include <asm/arch/cvmx-bch-defs.h>
# include <asm/arch/cvmx-fpa.h>
#else
# include "cvmx.h"
# include "cvmx-bch-defs.h"
# include "cvmx-fpa.h"
#endif
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_DRBELL CVMX_BCH_DRBELL_FUNC()
static inline uint64_t CVMX_BCH_DRBELL_FUNC(void)
{
	if (!OCTEON_IS_MODEL(OCTEON_CN70XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CN73XX) &&
	    !OCTEON_IS_MODEL(OCTEON_CNF75XX))
		cvmx_warn("CVMX_BCH_DRBELL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001710000000000ull);
}
#else
#define CVMX_BCH_DRBELL (CVMX_ADD_IO_SEG(0x0001710000000000ull))
#endif

union cvmx_bch_drbell {
	uint64_t u64;
	struct cvmx_bch_drbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_20_63	: 44;
		uint64_t cnt		: 20;	/**
		* Number of 64-bit words to add
		* to the BCH instruction
		* doorbell count.  BCH
		* instructions are from 4 to 36
		* words.
		*/
#else
		uint64_t cnt		: 20;	/**
		* Number of 64-bit words to add
		* to the BCH instruction
		* doorbell count.  BCH
		* instructions are from 4 to 36
		* words.
		*/
		uint64_t reserved_20_63	: 44;
#endif
	} s;
	struct cvmx_bch_drbell_s	cn70xx;
};
typedef union cvmx_bch_drbell cvmx_bch_drbell_t;

typedef enum {
	CVMX_BCH_INST_BLOCK_CORRECTION	= 0,	/** Perform block correction */
	CVMX_BCH_INST_ECC_GENERATION	= 1,	/** Generate ECC data */
} cvmx_bch_inst_type_t;

typedef union cvmx_bch_command {
	uint64_t u64[4];
	struct fields {
		/** CWORD Format */
		struct _cword {
#ifdef __BIG_ENDIAN_BITFIELD
			uint64_t	ecc_gen:1;	/** See cvmx_bch_inst_type_t */
			uint64_t	reserved1:24;	/** Must be zero */
			/**
			 * Maximum number of errors that can be corrected.  The number
			 * of parity bytes is equal to ((15 * ecc_level) + 7) / 8.
			 * ecc_level must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
			 */
			uint64_t	ecc_level:7;
			uint64_t	reserved2:20;	/** Must be zero */
			/** Size in bytes of data block.  Must be multiple of 2 */
			uint64_t	size:12;
#else
			uint64_t	size:12;
			uint64_t	reserved1:20;
			uint64_t	ecc_level:7;
			uint64_t	reserved2:24;
			uint64_t	ecc_gen:1;
#endif
		} cword;
		/** OWORD Format */
		struct _oword {
#ifdef __BIG_ENDIAN_BITFIELD
			uint64_t	reserved1:6;	/** Must be zero */
			/**
			 * BCH can modify any byte in any 128-byte cache line
			 * touched by L2/DRAM addresses oword.ptr through
			 * oword.ptr + cword.size - 1.  Setting this to 1 can
			 * improve hardware performance as some DRAM load
			 * operations can be avoided on L2 cache misses.
			 */
			uint64_t	fw:1;
			/**
			 * When set indicates that BCH should not allocate L2
			 * cache space for the parity correction data on L2
			 * misses.
			 */
			uint64_t	nc:1;
			uint64_t	reserved2:16;	/** Must be zero */
			/**
			 * Starting address of L2/DRAM buffer.  When ecc_gen is
			 * CVMX_BCH_INST_BLOCK_CORRECTION this contains the
			 * address where the corrected data shall be placed,
			 * otherwise this contains the address of the generated
			 * ecc data.
			 *
			 * ptr must be naturally-aligned on an 8-byte boundary.
			 */
			uint64_t	ptr:40;
#else
			uint64_t	ptr:40;
			uint64_t	reserved1:16;
			uint64_t	nc:1;
			uint64_t	fw:1;
			uint64_t	reserved2:6;
#endif
		} oword;
		/** IWORD Format */
		struct _iword {
#ifdef __BIG_ENDIAN_BITFIELD
			uint64_t	reserved1:7;	/** Must be zero */
			/**
			 * When set do not allocate L2 cache space for the input
			 * data on L2 cache misses.
			 */
			uint64_t	nc:1;
			uint64_t	reserved2:16;	/** Must be zero */
			/**
			 * Starting address of input data in L2/DRAM.
			 *
			 * When ecc_gen is CVMX_BCH_INST_BLOCK_CORRECTION this
			 * contains the address where the ecc data is located,
			 * otherwwise this contains the address of the data
			 * block.
			 *
			 * ptr must be naturally-aligned on an 8-byte boundary.
			 */
			uint64_t	ptr:40;
#else
			uint64_t	ptr:40;
			uint64_t	reserved1:16;
			uint64_t	nc:1;
			uint64_t	reserved2:7;
#endif
		} iword;
		/** RESP Format */
		struct _resp {
#ifdef __BIG_ENDIAN_BITFIELD
			uint64_t	reserved:24;	/** Must be zero */
			/**
			 * Pointer where two byte operation status will be
			 * written
			 */
			uint64_t	ptr:40;
#else
			uint64_t	ptr:40;
			uint64_t	reserved:24;
#endif
		} resp;
	} s;
} cvmx_bch_command_t;

/** Response from BCH instruction */
union cvmx_bch_response {
	uint16_t  u16;
	struct cvmx_bch_response_s {
#ifdef __BIG_ENDIAN_BITFIELD
		uint16_t	done:1;		/** Block is done */
		uint16_t	uncorrectable:1;/** Block could not be corrected */
		uint16_t	erased:1;	/** Block is erased */
		uint16_t	zero:6;		/** Always zero, ignore */
		uint16_t	num_errors:7;	/** Number of errors in block */
#else
		uint16_t	num_errors:7;	/** Number of errors in block */
		uint16_t	zero:6;		/** Always zero, ignore */
		uint16_t	erased:1;	/** Block is erased */
		uint16_t	uncorrectable:1;/** Block could not be corrected */
		uint16_t	done:1;		/** Block is done */
#endif
	} s;
};
typedef union cvmx_bch_response cvmx_bch_response_t;

/**
 * Sets up the BCH configuration data
 * (only FPA pool for now)
 */
typedef struct {
	cvmx_fpa_pool_config_t	command_queue_pool;	/** BCH FPA pool */
	int			aura;
} cvmx_bch_app_config_t;

extern CVMX_SHARED cvmx_bch_app_config_t bch_config;

/**
 * Gets the pool number used by BCH
 *
 * @return FPA pool number used
 */
static inline int64_t cvmx_fpa_get_bch_pool(void)
{
	return bch_config.command_queue_pool.pool_num;
}

/**
 * Gets the size of the buffer in BCH FPA pool
 *
 * @return FPA pool buffer size
 */
static inline uint64_t cvmx_fpa_get_bch_pool_block_size(void)
{
	return bch_config.command_queue_pool.buffer_size;
}

/**
 * Gets the buffer count of the BCH pool
 *
 * @return buffer count of BCH FPA pool
 */
static inline uint64_t cvmx_fpa_get_bch_pool_buffer_count(void)
{
	return bch_config.command_queue_pool.buffer_count;
}

/**
 * Ring the BCH doorbell telling it that new commands are
 * available.
 *
 * @param num_commands	Number of new commands
 */
static inline void cvmx_bch_write_doorbell(uint64_t num_commands)
{
	uint64_t num_words =
		num_commands * sizeof(cvmx_bch_command_t) / sizeof(uint64_t);
	CVMX_SYNCWS;
	cvmx_write_csr(CVMX_BCH_DRBELL, num_words);
}

/**
 * Sets the internal FPA pool data structure for bch pool.
 * @param pool	fpa pool number to use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buffers to allocate to pool
 */
void cvmx_bch_set_cmd_que_pool_config(int64_t pool, uint64_t buffer_size,
				      uint64_t buffer_count);

/**
 * Gets the FPA pool data from internal data structure
 * @param bch_pool pointer to the fpa data structure to copy data
 */
void cvmx_bch_get_cmd_que_pool_config(cvmx_fpa_pool_config_t *bch_pool);

/**
 * Initializes the BCH hardware before use.
 * @return 0 on success, -1 on failure
 */
int cvmx_bch_initialize(void);

/**
 * Shutdown and cleanup resources used by the BCH
 */
int cvmx_bch_shutdown(void);

/**
 * Given a data block calculate the ecc data and fill in the response
 *
 * @param[in] block	8-byte aligned pointer to data block to calculate ECC
 * @param block_size	Size of block in bytes, must be a multiple of two.
 * @param ecc_level	Number of errors that must be corrected.  The number of
 * 			parity bytes is equal to ((15 * ecc_level) + 7) / 8.
 * 			Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] ecc	8-byte aligned pointer to where ecc data should go
 * @param[in] response	pointer to where responses will be written.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_bch_encode(const void *block, uint16_t block_size,
		    uint8_t ecc_level, void *ecc,
		    volatile cvmx_bch_response_t *response);

/**
 * Given a data block and ecc data correct the data block
 *
 * @param[in] block_ecc_in	8-byte aligned pointer to data block with ECC
 *				data concatenated to the end to correct
 * @param block_size		Size of block in bytes, must be a multiple of
 *				two.
 * @param ecc_level		Number of errors that must be corrected.  The
 *				number of parity bytes is equal to
 *				((15 * ecc_level) + 7) / 8.
 * 				Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] block_out	8-byte aligned pointer to corrected data buffer.
 * 				This should not be the same as block_ecc_in.
 * @param[in] response		pointer to where responses will be written.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_bch_decode(const void *block_ecc_in, uint16_t block_size,
		    uint8_t ecc_level, volatile void *block_out,
		    volatile cvmx_bch_response_t *response);

#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_BCH_H__ */
