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
 * Interface to the hardware Free Pool Allocator.
 *
 * <hr>$Revision: 90198 $<hr>
 *
 */

#ifndef __CVMX_FPA_H__
#define __CVMX_FPA_H__

#include "cvmx-scratch.h"
#include "cvmx.h"
//#include "cvmx-spinlock.h"


#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include "cvmx-fpa-defs.h"
#endif

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define CVMX_FPA_NUM_POOLS      8
#define CVMX_FPA3_NUM_POOLS	64
#define CVMX_FPA_AURA_NUM       1024
#define CVMX_FPA_MIN_BLOCK_SIZE 128
#define CVMX_FPA_ALIGNMENT      128
#define CVMX_FPA_POOL_NAME_LEN  16
#define CVMX_FPA_AURA_NAME_LEN  16

/**
 * Structure describing the data format used for stores to the FPA.
 */
typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t scraddr:8;	/**
					 * the (64-bit word) location in
					 * scratchpad to write to (if len != 0)
					 */
		uint64_t len:8;		/**
					 * the number of words in the response
					 * (0 => no response)
					 */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t addr:40;	/**
					 * the address that will appear in the
					 * first tick on the NCB bus
					 */
#else
		uint64_t addr:40;	/**
					 * the address that will appear in the
					 * first tick on the NCB bus
					 */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t len:8;		/**
					 * the number of words in the response
					 * (0 => no response)
					 */
		uint64_t scraddr:8;	/**
					 * the (64-bit word) location in
					 * scratchpad to write to (if len != 0)
					 */
#endif
	} s;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t scraddr:8;     /**
					 * the (64-bit word) location in
					 * scratchpad to write to (if len != 0)
					 */
		uint64_t len:8;		/**
					 * length of return in workds, must be
					 * one.  If the pool has no availale pts
					 * in the selected pool then the ptr
					 * returned for the IOBDMA operation are
					 * 0s, indicating that the pool does not
					 * have an adequate number of ptrs to
					 * satisfy the IOBDMA.
					 */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t node:4;	/** OCI node number */
		uint64_t red:1;		/** Perform RED on allocation */
		uint64_t reserved2:9;   /** Reserved */
		uint64_t aura:10;	/** Aura number */
		uint64_t reserved3:16;	/** Reserved */
#else
		uint64_t reserved3:16;	/** Reserved */
		uint64_t aura:10;	/** Aura number */
		uint64_t reserved2:9;   /** Reserved */
		uint64_t red:1;		/** Perform RED on allocation */
		uint64_t node:4;	/** OCI node number */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t len:8;		/**
					 * length of return in workds, must be
					 * one.  If the pool has no availale pts
					 * in the selected pool then the ptr
					 * returned for the IOBDMA operation are
					 * 0s, indicating that the pool does not
					 * have an adequate number of ptrs to
					 * satisfy the IOBDMA.
					 */
		uint64_t scraddr:8;	/**
					 * the (64-bit word) location in
					 * scratchpad to write to (if len != 0)
					 */
#endif
	} cn78xx;
} cvmx_fpa_iobdma_data_t;

/**
 * Struct describing load allocate operation addresses for FPA pool.
 */
typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved1:15;
		uint64_t io:1;		/** Indicates I/O space */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t node:4;	/** OCI node number */
		uint64_t red:1;		/** Perform RED on allocation */
		uint64_t reserved2:9;   /** Reserved */
		uint64_t aura:10;	/** Aura number */
		uint64_t reserved3:16;	/** Reserved */
#else
		uint64_t reserved3:16;	/** Reserved */
		uint64_t aura:10;	/** Aura number */
		uint64_t reserved2:9;   /** Reserved */
		uint64_t red:1;		/** Perform RED on allocation */
		uint64_t node:4;	/** OCI node number */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t io:1;		/** Indicates I/O space */
		uint64_t reserved1:15;

#endif
	} cn78xx;
} cvmx_fpa_load_data_t;

/**
 * Struct describing store free operation addresses from FPA pool.
 */
typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved1:15;
		uint64_t io:1;		/** Indicates I/O space */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t node:4;	/** OCI node number */
		uint64_t reserved2:10;  /** Reserved */
		uint64_t aura:10;	/** Aura number */
		uint64_t fabs:1;	/** free absolute */
		uint64_t reserved3:3;	/** Reserved */
		uint64_t dwb_count:9;	/**
					 * Number of cache lines for which the
					 * hardware should try to execute
					 * 'don't-write-back' commands.
					 */
		uint64_t reserved4:3;	/** Reserved */
#else
		uint64_t reserved4:3;	/** Reserved */
		uint64_t dwb_count:9;	/**
					 * Number of cache lines for which the
					 * hardware should try to execute
					 * 'don't-write-back' commands.
					 */
		uint64_t reserved3:3;	/** Reserved */
		uint64_t fabs:1;	/** free absolute */
		uint64_t aura:10;	/** Aura number */
		uint64_t reserved2:10;  /** Reserved */
		uint64_t node:4;	/** OCI node number */
		uint64_t did:8;		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		uint64_t io:1;		/** Indicates I/O space */
		uint64_t reserved1:15;
#endif
	} cn78xx;
} cvmx_fpa_store_addr_t;

enum fpa_pool_alignment {
	FPA_NATURAL_ALIGNMENT,
	FPA_OFFSET_ALIGNMENT,
	FPA_OPAQUE_ALIGNMENT
};

/**
 * Structure describing the current state of a FPA pool.
 */
typedef struct {
	char name[CVMX_FPA_POOL_NAME_LEN];	/** FPA Pool Name */
	uint64_t size;				/** Size of each block */
	void *base;				/**
						 * The base memory address of
						 * whole block
						 */
	uint64_t stack_base;			/**
						 * Base address of stack of FPA
						 * pool
						 */
	uint64_t starting_element_count;	/**
						 * The number of elements in the
						 * pool at creation
						 */
	uint64_t max_buffer_cnt;		/**
						 * Maximum amount of buffers
						 * that can be held in this
						 * FPA pool
						 */
} cvmx_fpa_pool_info_t;


/**
 * Structure which contains information on auras.
 */
typedef struct {
	char name[CVMX_FPA_AURA_NAME_LEN];
	int pool_num;
} cvmx_fpa_aura_info_t;

/**
 * Structure to store FPA pool configuration parameters.
 */
typedef struct cvmx_fpa_pool_config
{
	int64_t pool_num;
	uint64_t buffer_size;
	uint64_t buffer_count;
}cvmx_fpa_pool_config_t;

/**
 * Current state of all the pools. Use access functions
 * instead of using it directly.
 */
extern CVMX_SHARED cvmx_fpa_pool_info_t
cvmx_fpa_pool_info[CVMX_MAX_NODES][CVMX_FPA3_NUM_POOLS];

/* CSR typedefs have been moved to cvmx-fpa-defs.h */

/**
 * Get a new block from the FPA Aura
 *
 * @param node  - node number
 * @param aura  - aura to get the block from
 * @return pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_alloc_aura(int node, int aura)
{
	uint64_t address;
	cvmx_fpa_load_data_t load_addr;

	load_addr.u64 = 0;
	load_addr.cn78xx.io = 1;
	load_addr.cn78xx.did = 0x29;    /* Device ID. Indicates FPA. */
	load_addr.cn78xx.node = node;   /* OCI node number */
	load_addr.cn78xx.red = 0;       /* Perform RED on allocation.
					 * FIXME to use config option
					 */
	load_addr.cn78xx.aura = aura;   /* Aura number */

	address = cvmx_read_csr((CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS,
					      load_addr.u64)));
	if (!address)
		return NULL;
	return cvmx_phys_to_ptr(address);
}

/**
 * Free a pointer back to the aura.
 *
 * @param node   node number
 * @param aura   aura number
 * @param ptr    physical address of block to free.
 * @param num_cache_lines Cache lines to invalidate
 */
static inline void cvmx_fpa_free_aura(void *ptr, uint64_t node, int aura,
				      uint64_t num_cache_lines)
{
	cvmx_fpa_store_addr_t newptr;
	cvmx_addr_t newdata;

	newdata.u64 = cvmx_ptr_to_phys(ptr);

	newptr.u64 = 2ull<<62;
	newptr.cn78xx.io = 1;
	newptr.cn78xx.did = 0x29;    /* Device id, indicates FPA */
	newptr.cn78xx.node = node;   /* OCI node number. */
	newptr.cn78xx.aura = aura;   /* Aura number */
	newptr.cn78xx.fabs = 0;	/* Free absolute. FIXME to use config option */
	newptr.cn78xx.dwb_count = num_cache_lines;

	/*cvmx_dprintf("aura = %d ptr_to_phys(ptr) = 0x%llx newptr.u64 = 0x%llx"
		     " ptr = %p \n", ptr, aura, (ULL) newptr.u64
		     (ULL) cvmx_ptr_to_phys(ptr)); */
	/* Make sure that any previous writes to memory go out before we free
	   this buffer. This also serves as a barrier to prevent GCC from
	   reordering operations to after the free. */
	CVMX_SYNCWS;
        cvmx_write_io(newptr.u64, newdata.u64);
}

/**
 * Return the name of the pool
 *
 * @param pool   Pool to get the name of
 * @return The name
 */
static inline const char *cvmx_fpa_get_name(uint64_t pool)
{
	return cvmx_fpa_pool_info[0][pool].name;
}

/**
 * Return the base of the pool
 *
 * @param pool   Pool to get the base of
 * @return The base
 */ static inline void *cvmx_fpa_get_base(uint64_t pool)
{
	return cvmx_fpa_pool_info[0][pool].base;
}

/**
 * Check if a pointer belongs to an FPA pool. Return non-zero
 * if the supplied pointer is inside the memory controlled by
 * an FPA pool.
 *
 * @param pool   Pool to check
 * @param ptr    Pointer to check
 * @return Non-zero if pointer is in the pool. Zero if not
 */
static inline int cvmx_fpa_is_member(uint64_t pool, void *ptr)
{
	return ((ptr >= cvmx_fpa_pool_info[0][pool].base) &&
		((char *)ptr < ((char *)(cvmx_fpa_pool_info[0][pool].base))
		 + (cvmx_fpa_pool_info[0][pool].size
		    * cvmx_fpa_pool_info[0][pool].starting_element_count)));
}

/**
 * Enable the FPA for use. Must be performed after any CSR
 * configuration but before any other FPA functions.
 */
static inline void cvmx_fpa_enable(void)
{
	cvmx_fpa_ctl_status_t status;

	if(!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		status.u64 = cvmx_read_csr(CVMX_FPA_CTL_STATUS);
		if (status.s.enb) {
			/*
		 	* CN68XXP1 should not reset the FPA (doing so may break
			* the SSO, so we may end up enabling it more than once.
			* Just return and don't spew messages.
		 	*/
			return;
		}

		status.u64 = 0;
		status.s.enb = 1;
		cvmx_write_csr(CVMX_FPA_CTL_STATUS, status.u64);
	}
}

/**
 * Reset FPA to disable. Make sure buffers from all FPA pools are freed
 * before disabling FPA.
 */
static inline void cvmx_fpa_disable(void)
{
	cvmx_fpa_ctl_status_t status;

	status.u64 = cvmx_read_csr(CVMX_FPA_CTL_STATUS);
	status.s.reset = 1;
	cvmx_write_csr(CVMX_FPA_CTL_STATUS, status.u64);
}

/**
 * Get a new block from the FPA
 *
 * @param pool   Pool to get the block from
 * @return Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_alloc(uint64_t pool)
{
	uint64_t address;

	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		/* We use the pool as the aura */
		return cvmx_fpa_alloc_aura(0, (int)pool);
	}

	for (;;) {
		address = cvmx_read_csr(CVMX_ADDR_DID(CVMX_FULL_DID(CVMX_OCT_DID_FPA,
								    pool)));
		if (cvmx_likely(address)) {
			return cvmx_phys_to_ptr(address);
		} else {
			/* If pointers are available, continuously retry.  */
			if (cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool)) > 0)
				cvmx_wait(50);
			else
				return NULL;
		}
	}
}

/**
 * Asynchronously get a new block from the FPA
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param pool      Pool to get the block from
 */
static inline void cvmx_fpa_async_alloc(uint64_t scr_addr, uint64_t pool)
{
	cvmx_fpa_iobdma_data_t data;

	/* Hardware only uses 64 bit aligned locations, so convert from byte
	 * address to 64-bit index
	 */
	data.s.scraddr = scr_addr >> 3;
	data.s.len = 1;
	data.s.did = CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool);
	data.s.addr = 0;
	cvmx_send_single(data.u64);
}

/**
 * Retrieve the result of cvmx_fpa_async_alloc
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param pool Pool the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * @return Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_async_alloc_finish(uint64_t scr_addr, uint64_t pool)
{
	uint64_t address;

	CVMX_SYNCIOBDMA;

	address = cvmx_scratch_read64(scr_addr);
	if (cvmx_likely(address))
		return cvmx_phys_to_ptr(address);
	else
		return cvmx_fpa_alloc(pool);
}

/**
 * Free a block allocated with a FPA pool.
 * Does NOT provide memory ordering in cases where the memory block was
 * modified by the core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free_nosync(void *ptr, uint64_t pool,
					uint64_t num_cache_lines)
{
	cvmx_addr_t newptr;
	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace =
		CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Prevent GCC from reordering around free */
	asm volatile ("":::"memory");
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}

/**
 * Free a block allocated with a FPA pool.  Provides required memory
 * ordering in cases where memory block was modified by core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free(void *ptr, uint64_t pool,
				 uint64_t num_cache_lines)
{
	cvmx_addr_t newptr;

	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		/* We use the pool as the aura */
		cvmx_fpa_free_aura(ptr, 0, (int)pool, num_cache_lines);
		return;
	}

	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace =
		CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Make sure that any previous writes to memory go out before we free
	 * this buffer.  This also serves as a barrier to prevent GCC from
	 * reordering operations to after the free.
	 */
	CVMX_SYNCWS;
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}


static inline void __cvmx_fpa_aura_cfg(int node, int aura, int pool,
				       int buffers_cnt, int ptr_dis)
{
       cvmx_fpa_aurax_cfg_t aura_cfg;
       uint64_t pool64 = pool;

       aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_FPA_AURAX_CFG(aura));
       aura_cfg.s.ptr_dis = ptr_dis;
        /* Configure CVMX_FPA_AURAX_CNT_LEVELS, CVMX_FPA_AURAX_POOL_LEVELS  */
       cvmx_write_csr_node(node, CVMX_FPA_AURAX_CFG(aura), aura_cfg.u64);
       cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT(aura), buffers_cnt);
       cvmx_write_csr_node(node, CVMX_FPA_AURAX_POOL(aura), pool64);

       /* TODO : config back pressure, RED */
}

/**
 * This function sets up QOS related parameter for specified Aura.
 *
 * @param node       node number.
 * @param aura       aura to configure.
 * @param ena_red       enable RED based on [DROP] and [PASS] levels
                        1: enable 0:disable
 * @param pass_thresh   pass threshold for RED.
 * @param drop_thresh   drop threshold for RED
 * @param ena_bp        enable backpressure based on [BP] level.
                        1:enable 0:disable
 * @param bp_thresh     backpressure threshold.
 *
 */
static inline void cvmx_fpa_setup_aura_qos(int node, int aura, bool ena_red,
					   uint64_t pass_thresh,
					   uint64_t drop_thresh,
					   bool ena_bp,uint64_t bp_thresh)
{
	uint64_t shift=0;
	uint64_t shift_thresh;
	cvmx_fpa_aurax_cnt_levels_t aura_level;

	shift_thresh = bp_thresh > drop_thresh ? bp_thresh:drop_thresh;

	while ( (shift_thresh & (uint64_t)(~(0xff)))) {
		shift_thresh = shift_thresh >> 1;
		shift++;
	};

	aura_level.u64 = cvmx_read_csr_node(node,CVMX_FPA_AURAX_CNT_LEVELS(aura));
	aura_level.s.pass = pass_thresh >> shift;
	aura_level.s.drop = drop_thresh >> shift;
	aura_level.s.bp = bp_thresh >> shift;
	aura_level.s.shift = shift;
	aura_level.s.red_ena = ena_red;
	aura_level.s.bp_ena = ena_bp;
	cvmx_write_csr_node(node,CVMX_FPA_AURAX_CNT_LEVELS(aura),aura_level.u64);
}

/**
 * Setup a FPA pool to control a new block of memory.
 * This can only be called once per pool. Make sure proper
 * locking enforces this.
 *
 * @param pool       Pool to initialize
 *                   0 <= pool < 8
 * @param name       Constant character string to name this pool.
 *                   String is not copied.
 * @param buffer     Pointer to the block of memory to use. This must be
 *                   accessable by all processors and external hardware.
 * @param block_size Size for each block controlled by the FPA
 * @param num_blocks Number of blocks
 *
 * @return 0 on Success,
 *         -1 on failure
 */
extern int cvmx_fpa_setup_pool(uint64_t pool, const char *name, void *buffer,
			       uint64_t block_size, uint64_t num_blocks);

/**
 * Shutdown a Memory pool and validate that it had all of
 * the buffers originally placed in it. This should only be
 * called by one processor after all hardware has finished
 * using the pool. Most like you will want to have called
 * cvmx_helper_shutdown_packet_io_global() before this
 * function to make sure all FPA buffers are out of the packet
 * IO hardware.
 *
 * @param pool   Pool to shutdown
 *
 * @return Zero on success
 *         - Positive is count of missing buffers
 *         - Negative is too many buffers or corrupted pointers
 */
extern uint64_t cvmx_fpa_shutdown_pool(uint64_t pool);

/**
 * Get the size of blocks controlled by the pool
 * This is resolved to a constant at compile time.
 *
 * @param pool   Pool to access
 * @return Size of the block in bytes
 */
extern uint64_t cvmx_fpa_get_block_size(uint64_t pool);

/**
 * Initialize FPA block global configuration.
 */
int cvmx_fpa_global_initialize(void);

/**
 * Initialize global configuration for FPA block for specified node.
 */
int cvmx_fpa_global_init_node(int node);

/**
 * Allocate or reserve  the specified fpa pool.
 *
 * @param pool	  FPA pool to allocate/reserve. If -1 it
 *                finds an empty pool to allocate.
 * @return        Alloctaed pool number or -1 if fails to allocate
                  the pool
 */
int cvmx_fpa_alloc_pool(int pool);

/**
 * Free the specified fpa pool.
 * @param pool	   Pool to free
 * @return         0 for success -1 failure
 */
int cvmx_fpa_release_pool(int pool);

/**
 * This call will initialise the stack of the specified pool. Only the stack
 * memory which is the memory that holds the buffer pointers is allocated.
 * For now assume that natural alignment is used. When using natural alignment
 * the hardware needs to be initialised with the buffer size and hence it is
 * specified at the time of pool initialisation. The value will be buffered and
 * used later during cvmx_fpa_aura_init() to allocate buffers.
 * Before invoking this call the application should already have ownership of
 * the pool, the ownership is obtained when pool is one of the values in the
 * pools_allocated array after invocation of cvmx_allocate_fpa_pool()
 * The linux device driver chooses to not use this call as it would initialise
 * the FPA pool with kernel memory as opposed to using bootmem.
 * @param node     - specifies the node of FPA pool.
 * @parma pool     - Specifies the FPA pool number.
 * @param name     - Specifies the FPA pool name.
 * @param mem_node - specifies the node from which the memory for the stack
 *                   is allocated.
 * @param max_buffer_cnt - specifies the maximum buffers that FPA pool can hold.
 * @parm  align          - specifies the alignment type.
 * @param buffer_sz      - Only when the alignment is natural this field is used
 *                         to specify the size of each buffer in the FPA .
 *
 */
int cvmx_fpa_pool_stack_init(int node, int pool, const char *name, int mem_node,
			     int max_buffer_cnt, enum fpa_pool_alignment align,
			     int buffer_sz);

/**
 * This call will allocated buffers_cnt number of buffers from  the bootmemory
 * of bootmem_node and populate the aura specified with the allocated buffers.
 * The size of the buffers is obtained from the buffer_sz used to initialise the
 * stack of the FPA pool associated with the aura. This also means that the aura
 * has already been mapped to an FPA pool. More parameters is possible to
 * specify RED and buffer level parameters. This is another that would not be
 * used by the Linux device driver, the driver would populate the buffers
 * in the pool using it's own allocation mechanism.
 * @param node     - specifies the node of aura to be initialized
 * @parma aura     - specifies the aura to be initalized.
 * @param name     - specifies the name of aura to be initalized.
 * @param mem_node - specifies the node from which the memory for the buffers
 *                   is allocated.
 * @param buffers  - Block of memory to use for the aura buffers. If NULL,
 *                   aura memory is allocated.
 * @param ptr_dis - Need to look into this more but is on the lines of of
 *		    whether the hardware checks double frees.
 */
int cvmx_fpa_aura_init(int node, int aura, const char *name, int mem_node,
		       void *buffers, int buffers_cnt, int ptr_dis);
int cvmx_fpa_config_red_params(int node, int qos_avg_en, int red_lvl_dly,
			       int avg_dly);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * This will map auras specified in the auras_index[] array to specified
 * FPA pool_index.
 * The array is assumed to contain count number of entries.
 * @param count is the number of entries in the auras_index array.
 * @pool_index is the index of the fpa pool.
 * @return 0 on success
 */
int cvmx_fpa_assign_auras(int node, int auras_index[], int count,
			  int pool_index);

static inline int cvmx_fpa_assign_aura(int node, int aura, int pool_index)
{
	int auras[1];

	auras[0] = aura;
	return cvmx_fpa_assign_auras(node, auras, 1, pool_index);
}
#endif

int cvmx_fpa_allocate_auras(int node, int auras_allocated[], int count);
int cvmx_fpa_free_auras(int node, int *pools_allocated, int count);
/**
 * This will allocate count number of FPA pools on the specified node to the
 * calling application. These pools will be for exclusive use of the application
 * until they are freed.
 * @param pools_allocated is an array of length count allocated by
 *			  the application before invoking the
 *			  cvmx_allocate_fpa_pool call.  On return it will
 *			  contain the index numbers of the pools allocated.
 * @return 0 on success and -1 on failure.
 */
int cvmx_fpa_allocate_fpa_pools(int node, int pools_allocated[], int count);
#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /*  __CVM_FPA_H__ */
