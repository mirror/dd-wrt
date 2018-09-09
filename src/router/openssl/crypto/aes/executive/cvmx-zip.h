/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * Header file for the zip (deflate) block
 *
 * <hr>$Revision: 91634 $<hr>
 */

#ifndef __CVMX_ZIP_H__
#define __CVMX_ZIP_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t unused:5;
		uint64_t full_block_write:1;
		uint64_t no_l2_alloc:1;
		uint64_t little_endian:1;
		uint64_t length:16;
		uint64_t ptr:40;
#else
		uint64_t ptr:40;
		uint64_t length:16;
		uint64_t little_endian:1;
		uint64_t no_l2_alloc:1;
		uint64_t full_block_write:1;
		uint64_t unused:5;
#endif
	} s;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t length:16;
		uint64_t full_block_write:1;
		uint64_t no_l2_alloc:1;
		uint64_t little_endian:1;
		uint64_t unused:3;
		uint64_t ptr:42;
#else
		uint64_t ptr:42;
		uint64_t unused:3;
		uint64_t little_endian:1;
		uint64_t no_l2_alloc:1;
		uint64_t full_block_write:1;
		uint64_t length:16;
#endif
	} zip3;
} cvmx_zip_ptr_t;
#define CVMX_ZIP_PTR_MAX_LEN    ((1 << 16) - 1)

typedef enum {
	CVMX_ZIP_COMPLETION_NOTDONE = 0,
	CVMX_ZIP_COMPLETION_SUCCESS = 1,
	CVMX_ZIP_COMPLETION_OTRUNC = 2,
	CVMX_ZIP_COMPLETION_STOP = 3,
	CVMX_ZIP_COMPLETION_ITRUNC = 4,
	CVMX_ZIP_COMPLETION_RBLOCK = 5,
	CVMX_ZIP_COMPLETION_NLEN = 6,
	CVMX_ZIP_COMPLETION_BADCODE = 7,
	CVMX_ZIP_COMPLETION_BADCODE2 = 8,
	CVMX_ZIP_COMPLETION_ZERO_LEN = 9,
	CVMX_ZIP_COMPLETION_PARITY = 10,
	CVMX_ZIP_COMPLETION_FATAL = 11
} cvmx_zip_completion_code_t;

typedef union {
	uint64_t u64[3];
	struct {

		// WORD 0
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t crc32:32;
		uint64_t adler:32;
#else
		uint64_t adler:32;
		uint64_t crc32:32;
#endif

		// WORD 1
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t totalbyteswritten:32;
		uint64_t totalbytesread:32;
#else
		uint64_t totalbytesread:32;
		uint64_t totalbyteswritten:32;
#endif

		// WORD 2
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t totalbitsprocessed:32;	// decompression only
		uint64_t unused20:5;
		uint64_t exnum:3;	// compression only
		uint64_t unused21:1;
		uint64_t exbits:7;	// compression only
		uint64_t unused22:7;
		uint64_t eof:1;	// decompression only
		cvmx_zip_completion_code_t completioncode:8;	// If polling, SW should set this to zero and wait for non-zero
#else
		cvmx_zip_completion_code_t completioncode:8;	// If polling, SW should set this to zero and wait for non-zero
		uint64_t eof:1;	// decompression only
		uint64_t unused22:7;
		uint64_t exbits:7;	// compression only
		uint64_t unused21:1;
		uint64_t exnum:3;	// compression only
		uint64_t unused20:5;
		uint64_t totalbitsprocessed:32;	// decompression only
#endif
	} s;
} cvmx_zip_result_t;

typedef union {
	uint64_t u64[8];
	struct {

		// WORD 0
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t unused00:8;
		uint64_t totaloutputlength:24;
		uint64_t unused01:5;
		uint64_t exnum:3;
		uint64_t unused02:1;
		uint64_t exbits:7;
		uint64_t unused03:4;
		uint64_t flush:1;
		uint64_t speed:2;
		uint64_t forcefixed:1;
		uint64_t forcedynamic:1;
		uint64_t eof:1;
		uint64_t bof:1;
		uint64_t compress:1;
		uint64_t unused04:1;
		uint64_t dscatter:1;
		uint64_t dgather:1;
		uint64_t hgather:1;
#else
		uint64_t hgather:1;
		uint64_t dgather:1;
		uint64_t dscatter:1;
		uint64_t unused04:1;
		uint64_t compress:1;
		uint64_t bof:1;
		uint64_t eof:1;
		uint64_t forcedynamic:1;
		uint64_t forcefixed:1;
		uint64_t speed:2;
		uint64_t flush:1;
		uint64_t unused03:4;
		uint64_t exbits:7;
		uint64_t unused02:1;
		uint64_t exnum:3;
		uint64_t unused01:5;
		uint64_t totaloutputlength:24;
		uint64_t unused00:8;
#endif

		// WORD 1
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t historylength:16;
		uint64_t unused10:16;
		uint64_t adler32:32;
#else
		uint64_t adler32:32;
		uint64_t unused10:16;
		uint64_t historylength:16;
#endif

		// WORD 2
		cvmx_zip_ptr_t ctx_ptr;

		// WORD 3
		cvmx_zip_ptr_t hist_ptr;

		// WORD 4
		cvmx_zip_ptr_t in_ptr;

		// WORD 5
		cvmx_zip_ptr_t out_ptr;

		// WORD 6
		cvmx_zip_ptr_t result_ptr;

		// WORD 7
		cvmx_zip_ptr_t wq_ptr;

	} s;
} cvmx_zip_command_t;

typedef struct
{
	cvmx_fpa_pool_config_t zip_pool;
	int		       aura;
}cvmx_zip_config_t;

extern CVMX_SHARED cvmx_zip_config_t zip_config;

static inline int64_t cvmx_fpa_get_zip_pool(void)
{
	return (zip_config.zip_pool.pool_num);
}

static inline uint64_t cvmx_fpa_get_zip_pool_block_size(void)
{
	return (zip_config.zip_pool.buffer_size);
}

static inline uint64_t cvmx_fpa_get_zip_aura(void)
{
	return (zip_config.aura);
}

/**
 * Sets the internal FPA pool data structure for ZIP pool.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 */

void cvmx_zip_set_fpa_pool_config(int64_t pool, uint64_t buffer_size,
					   uint64_t buffer_count);

/**
 * Gets the internal FPA pool data structure for zip pool.
 * @param zip_pool pointer to timer fpa pool structure where data gets copied
 */
void cvmx_zip_get_fpa_pool_config(cvmx_fpa_pool_config_t *zip_pool);

/**
 * Initialize the ZIP block
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_initialize(void);

/**
 * Initialize the ZIP QUEUE buffer
 *
 * @param queue : ZIP instruction queue
 * @param zcoremask : ZIP coremask to use for this queue
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_initialize(int queue, int zcoremask);

/**
 * @INTERNAL
 * Initialize the ZIP queue buffer and zip block.
 *
 * @param queue : ZIP instruction queue
 * @param zcoremask : ZIP coremask to use for this queue
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip3_init(int queue, int zcoremask);

/**
 * Shutdown the ZIP block. ZIP must be idle when
 * this function is called.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_shutdown(void);

/**
 * Shutdown the ZIP block for a queue. ZIP must be idle when
 * this function is called.
 *
 * @param queue   Zip instruction queue of the command
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_shutdown(int queue);

/**
 * Submit a command to the ZIP block
 *
 * @param command Zip command to submit
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_submit(cvmx_zip_command_t * command);

/**
 * Submit a command to the ZIP block
 *
 * @param command Zip command to submit
 * @param queue   Zip instruction queue of the command
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_submit(cvmx_zip_command_t * command, int queue);

/* CSR typedefs have been moved to cvmx-zip-defs.h */

static inline void cvmx_zptr_set_ptr(cvmx_zip_ptr_t *zptr, uint64_t ptr)
{
	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		zptr->zip3.ptr = ptr;
	else
		zptr->s.ptr = ptr;
}

static inline uint64_t cvmx_zptr_get_ptr(cvmx_zip_ptr_t *zptr)
{
	uint64_t	ptr;

	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		ptr = zptr->zip3.ptr;
	else
		ptr = zptr->s.ptr;

	return ptr;
}

static inline void cvmx_zptr_set_len(cvmx_zip_ptr_t *zptr, int len)
{
	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		zptr->zip3.length = len;
	else
		zptr->s.length = len;
}

static inline int cvmx_zptr_get_len(cvmx_zip_ptr_t *zptr)
{
	int	len;

	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		len = zptr->zip3.length;
	else
		len = zptr->s.length;

	return len;
}

static inline void cvmx_zptr_set_fbw(cvmx_zip_ptr_t *zptr, int fbw)
{
	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		zptr->zip3.full_block_write = fbw;
	else
		zptr->s.full_block_write = fbw;
}

static inline int cvmx_zptr_get_fbw(cvmx_zip_ptr_t *zptr)
{
	int	fbw;

	if (octeon_has_feature(OCTEON_FEATURE_ZIP3))
		fbw = zptr->zip3.full_block_write;
	else
		fbw = zptr->s.full_block_write;

	return fbw;
}


#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/*INDENT-ON* */
#endif
#endif				/* __CVMX_ZIP_H__ */
