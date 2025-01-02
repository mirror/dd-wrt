/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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

 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

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
 * Module to support operations on bitmap of cores. Coremask can be used to
 * select a specific core, a group of cores, or all available cores, for
 * initialization and differentiation of roles within a single shared binary
 * executable image.
 *
 * The core numbers used in this file are the same value as what is found in
 * the COP0_EBASE register and the rdhwr 0 instruction.
 *
 * For the CN78XX and other multi-node environments the core numbers are not
 * contiguous.  The core numbers for the CN78XX are as follows:
 *
 * Node 0:	Cores 0 - 47
 * Node 1:	Cores 128 - 175
 * Node 2:	Cores 256 - 303
 * Node 3:	Cores 384 - 431
 *
 * The coremask environment generally tries to be node agnostic in order to
 * provide future compatibility if more cores are added to future processors
 * or more nodes are supported.
 *
 * <hr>$Revision: 156174 $<hr>
 *
 */

#ifndef __CVMX_COREMASK_H__
#define __CVMX_COREMASK_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#include "cvmx-asm.h"

typedef uint64_t cvmx_coremask_holder_t;	/* basic type to hold the
						   coremask bits */

/* bits per holder */
#define CVMX_COREMASK_HLDRSZ ((int)(sizeof(cvmx_coremask_holder_t) * 8))

/** Maximum allowed cores per node */
#define CVMX_COREMASK_MAX_CORES_PER_NODE (1 << CVMX_NODE_NO_SHIFT)

/** Maximum number of bits actually used in the coremask */
#define CVMX_MAX_USED_CORES_BMP	(1 << (CVMX_NODE_NO_SHIFT + CVMX_NODE_BITS))

/* the number of valid bits in and the mask of the most significant holder */
#define CVMX_COREMASK_MSHLDR_NBITS (CVMX_MIPS_MAX_CORES % CVMX_COREMASK_HLDRSZ)
#define CVMX_COREMASK_MSHLDR_MASK					\
	((CVMX_COREMASK_MSHLDR_NBITS) ?					\
	    (((cvmx_coremask_holder_t) 1 << CVMX_COREMASK_MSHLDR_NBITS) - 1) : \
	    ((cvmx_coremask_holder_t) -1))

/* cvmx_coremask_t's size in cvmx_coremask_holder_t */
#define CVMX_COREMASK_BMPSZ						\
	((int)(CVMX_MIPS_MAX_CORES / CVMX_COREMASK_HLDRSZ + 		\
		(CVMX_COREMASK_MSHLDR_NBITS != 0)))

#define CVMX_COREMASK_USED_BMPSZ	\
	(CVMX_MAX_USED_CORES_BMP / CVMX_COREMASK_HLDRSZ)

#define CVMX_COREMASK_BMP_NODE_CORE_IDX(node, core)	\
	((((node) << CVMX_NODE_NO_SHIFT) + (core)) / CVMX_COREMASK_HLDRSZ)
/**
 * Maximum available coremask.
 */
#define CVMX_COREMASK_MAX		\
	{{ 0x0000FFFFFFFFFFFF, 0,	\
	   0x0000FFFFFFFFFFFF, 0,	\
	   0x0000FFFFFFFFFFFF, 0,	\
	   0x0000FFFFFFFFFFFF, 0,	\
	   0, 0,			\
	   0, 0,			\
	   0, 0,			\
	   0, 0}}

/**
 * Empty coremask
 */
#define CVMX_COREMASK_EMPTY		\
	{{ 0, 0, 0, 0, 0, 0, 0, 0,	\
	   0, 0, 0, 0, 0, 0, 0, 0}}

/* cvmx_coremask_t */
struct cvmx_coremask {
	cvmx_coremask_holder_t coremask_bitmap[CVMX_COREMASK_BMPSZ];
};

typedef struct cvmx_coremask cvmx_coremask_t;

#if defined(CVMX_BUILD_FOR_LINUX_KERNEL)
# define strtoull simple_strtoull
# define strtoul simple_strtoul
# include <linux/string.h>
#elif defined(__U_BOOT__)
# define strtoull simple_strtoull
# define strtoul simple_strtoul
# include <linux/string.h>
# include "cvmx-platform.h"
# include "cvmx-access.h"
#else
# include <string.h>
# if !defined(__mips__)
#  define cvmx_get_core_num() 0
# else
#  include "cvmx-platform.h"
#  include "cvmx-access.h"
# endif
#endif

/**
 * Macro to iterate through all available cores in a coremask
 *
 * @param core - core variable to use to iterate
 * @param pcm - pointer to core mask
 *
 * Use this like a for statement
 */
#define cvmx_coremask_for_each_core(core, pcm)			\
	for ((core) = -1;					\
		(core) = cvmx_coremask_next_core((core), pcm),	\
		(core) >= 0; )

/**
 * Given a node and node mask, return the next available node.
 *
 * @param node		starting node number
 * @param node_mask	node mask to use to find the next node
 *
 * @return next node number or -1 if no more nodes are available
 */
static inline int cvmx_coremask_next_node(unsigned node, uint8_t node_mask)
{
	int next_offset;

	next_offset = __builtin_ffs(node_mask >> (node + 1));
	if (next_offset == 0)
		return -1;
	else
		return node + next_offset;
}

/**
 * Iterate through all nodes in a node mask
 *
 * @param node		node iterator variable
 * @param node_mask	mask to use for iterating
 *
 * Use this like a for statement
 */
#define cvmx_coremask_for_each_node(node, node_mask)		\
	for ((node) = __builtin_ffs(node_mask) - 1;		\
	     (node) >= 0 && (node) < CVMX_MAX_NODES;		\
	     (node) = cvmx_coremask_next_node(node, node_mask))

/**
 * Is ``core'' set in the coremask?
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * @return 1 if core is set and 0 if not.
 */
static inline int cvmx_coremask_is_core_set(const cvmx_coremask_t *pcm,
					    int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;

	return (pcm->coremask_bitmap[i] & ((cvmx_coremask_holder_t)1 << n)) != 0;
}

/**
 * Is ``current core'' set in the coremask?
 *
 * @param pcm is the pointer to the coremask.
 * @return 1 if core is set and 0 if not.
 */
static inline int cvmx_coremask_is_self_set(const cvmx_coremask_t *pcm)
{
	return cvmx_coremask_is_core_set(pcm, (int)cvmx_get_core_num());
}

/**
 * Is coremask empty?
 * @param pcm is the pointer to the coremask.
 * @return 1 if *pcm is empty (all zeros), 0 if not empty.
 */
static inline int cvmx_coremask_is_empty(const cvmx_coremask_t *pcm)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if (pcm->coremask_bitmap[i] != 0)
			return 0;

	return 1;
}

/**
 * Set ``core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * @return 0.
 */
static inline int cvmx_coremask_set_core(cvmx_coremask_t *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] |= ((cvmx_coremask_holder_t)1 << n);

	return 0;
}

#ifndef CVMX_BUILD_FOR_LINUX_HOST
/* when run on HOST cvmx_get_core_num() is not applicable,
 * so this func as well
 */
/**
 * Set ``current core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @return 0.
 */
static inline int cvmx_coremask_set_self(cvmx_coremask_t *pcm)
{
	return cvmx_coremask_set_core(pcm, (int)cvmx_get_core_num());
}
#endif

/**
 * Clear ``core'' from the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * @return 0.
 */
static inline int cvmx_coremask_clear_core(cvmx_coremask_t *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] &= ~((cvmx_coremask_holder_t)1 << n);

	return 0;
}

#ifndef CVMX_BUILD_FOR_LINUX_HOST
/* when run on HOST cvmx_get_core_num() is not applicable,
 * so this func as well
 */
/**
 * Clear ``current core'' from the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @return 0.
 */
static inline int cvmx_coremask_clear_self(cvmx_coremask_t *pcm)
{
	return cvmx_coremask_clear_core(pcm, cvmx_get_core_num());
}
#endif

/**
 * Toggle ``core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @param core
 * @return 0.
 */
static inline int cvmx_coremask_toggle_core(cvmx_coremask_t *pcm, int core)
{
	int n, i;

	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;
	pcm->coremask_bitmap[i] ^= ((cvmx_coremask_holder_t)1 << n);

	return 0;
}

/**
 * Toggle ``current core'' in the coremask.
 *
 * @param pcm is the pointer to the coremask.
 * @return 0.
 */
static inline int cvmx_coremask_toggle_self(cvmx_coremask_t *pcm)
{
	return cvmx_coremask_toggle_core(pcm, cvmx_get_core_num());
}

/**
 * Set the lower 64-bit of the coremask.
 * @param pcm	pointer to coremask
 * @param coremask_64	64-bit coremask to apply to the first node (0)
 */
static inline void cvmx_coremask_set64(cvmx_coremask_t *pcm,
				       uint64_t coremask_64)
{
	pcm->coremask_bitmap[0] = coremask_64;
}

/**
 * Set the 64-bit of the coremask for a particular node.
 * @param pcm	pointer to coremask
 * @param node	node to set
 * @param coremask_64	64-bit coremask to apply to the specified node
 */
static inline void cvmx_coremask_set64_node(cvmx_coremask_t *pcm,
					    uint8_t node,
					    uint64_t coremask_64)
{

	pcm->coremask_bitmap[CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0)] =
								coremask_64;
}

/**
 * Gets the lower 64-bits of the coremask
 *
 * @param[in] pcm - pointer to coremask
 * @return 64-bit coremask for the first node
 */
static inline uint64_t cvmx_coremask_get64(const cvmx_coremask_t *pcm)
{
	return pcm->coremask_bitmap[0];
}

/**
 * Gets the lower 64-bits of the coremask for the specified node
 *
 * @param[in] pcm - pointer to coremask
 * @param node - node to get coremask for
 * @return 64-bit coremask for the first node
 */
static inline uint64_t cvmx_coremask_get64_node(const cvmx_coremask_t *pcm,
						uint8_t node)
{
	return pcm->coremask_bitmap[CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0)];
}

/**
 * Gets the lower 32-bits of the coremask for compatibility
 *
 * @param[in] pcm - pointer to coremask
 * @return 32-bit coremask for the first node
 * @deprecated This function is to maintain compatibility with older
 *             SDK applications and may disappear at some point.
 * This function is not compatible with the CN78XX or any other
 * Octeon device with more than 32 cores.
 */
static inline uint32_t cvmx_coremask_get32(const cvmx_coremask_t *pcm)
{
	return pcm->coremask_bitmap[0] & 0xffffffff;
}

/*
 * cvmx_coremask_cmp() returns an integer less than, equal to, or
 * greater than zero if *pcm1 is found, respectively, to be less than,
 * to match, or be greater than *pcm2.
 */
static inline int cvmx_coremask_cmp(const cvmx_coremask_t *pcm1,
				    const cvmx_coremask_t *pcm2)
{
	int i;
	/* Start from highest node for arithemtically correct result */
	for ( i = CVMX_COREMASK_USED_BMPSZ-1; i >= 0 ; i-- )
		if( pcm1->coremask_bitmap[i] != pcm2->coremask_bitmap[i] )
			return (pcm1->coremask_bitmap[i] > pcm2->coremask_bitmap[i]) ? 1 : -1;

	return 0;
}

/*
 * cvmx_coremask_OPx(pcm1, pcm2[, pcm3]), where OPx can be
 * - and
 * - or
 * - xor
 * - not
 * ...
 * For binary operators, pcm3 <-- pcm1 OPX pcm2.
 * For unaries, pcm2 <-- OPx pcm1.
 */
#define CVMX_COREMASK_BINARY_DEFUN(binary_op, op)			\
	static inline int cvmx_coremask_##binary_op(			\
	    cvmx_coremask_t *pcm1,					\
	    const cvmx_coremask_t *pcm2,				\
	    const cvmx_coremask_t *pcm3)				\
	{								\
		int i;							\
									\
		for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)		\
			pcm1->coremask_bitmap[i] = 			\
			    pcm2->coremask_bitmap[i]			\
				op					\
			    pcm3->coremask_bitmap[i];			\
									\
		return 0;						\
	}

#define CVMX_COREMASK_UNARY_DEFUN(unary_op, op)				\
	static inline int cvmx_coremask_##unary_op(			\
	    cvmx_coremask_t *pcm1,					\
	    const cvmx_coremask_t *pcm2)				\
	{								\
		int i;							\
									\
		for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)		\
			pcm1->coremask_bitmap[i] =			\
			    op						\
			    pcm2->coremask_bitmap[i];			\
									\
		return 0;						\
	}

CVMX_COREMASK_BINARY_DEFUN(and, &)  /* cvmx_coremask_and(pcm1, pcm2, pcm3): pcm1 = pmc2 & pmc3 */
CVMX_COREMASK_BINARY_DEFUN(or, |)   /* cvmx_coremask_or(pcm1, pcm2, pcm3): pcm1 = pmc2 | pmc3  */
CVMX_COREMASK_BINARY_DEFUN(xor, ^)  /* cvmx_coremask_xor(pcm1, pcm2, pcm3): pcm1 = pmc2 ^ pmc3 */
CVMX_COREMASK_BINARY_DEFUN(maskoff, & ~) /* cvmx_coremask_maskoff(pcm1, pcm2, pcm3): pcm1 = pmc2 & ~pmc3 */
CVMX_COREMASK_UNARY_DEFUN(not, ~)   /* cvmx_coremask_not(pcm1, pcm2): pcm1 = ~pcm2       */
CVMX_COREMASK_UNARY_DEFUN(fill, -1 | )  /* cvmx_coremask_fill(pcm1, pcm2): pcm1 = -1      */
CVMX_COREMASK_UNARY_DEFUN(clear, 0 &) /* cvmx_coremask_clear(pcm1, pcm2): pcm1 = 0     */
CVMX_COREMASK_UNARY_DEFUN(dup, +)   /* cvmx_coremask_dup(pcm1, pcm2): pcm1 = pcm2       */

/*
 * Macros using the unary functions defined w/
 * CVMX_COREMASK_UNARY_DEFUN
 * - set *pcm to its complement
 * - set all bits in *pcm to 0
 * - set all (valid) bits in *pcm to 1
 */
#define cvmx_coremask_complement(pcm)	cvmx_coremask_not(pcm, pcm)
/* On clear, even clear the unused bits */
#define cvmx_coremask_clear_all(pcm)	do {*(pcm) = (cvmx_coremask_t)CVMX_COREMASK_EMPTY;} while (0)
#define cvmx_coremask_set_all(pcm)	cvmx_coremask_fill(pcm, NULL)

/*
 * convert a string of hex digits to cvmx_coremask_t
 *
 * @param pcm
 * @param hexstr can be
 * 	- "[1-9A-Fa-f][0-9A-Fa-f]*", or
 * 	- "-1" to set the bits for all the cores.
 * return
 * 	 0 for success,
 * 	-1 for string too long (i.e., hexstr takes more bits than
 * 	   CVMX_MIPS_MAX_CORES),
 * 	-2 for conversion problems from hex string to an unsigned
 * 	   long long, e.g., non-hex char in hexstr, and
 * 	-3 for hexstr starting with '0'.
 * NOTE:
 * 	This function clears the bitmask in *pcm before the conversion.
 */
extern int cvmx_coremask_str2bmp(cvmx_coremask_t *pcm, char *hexstr);

/*
 * convert a cvmx_coremask_t to a string of hex digits
 *
 * @param pcm
 * @param hexstr is "[1-9A-Fa-f][0-9A-Fa-f]*"
 *
 * return 0.
 */
extern int cvmx_coremask_bmp2str(const cvmx_coremask_t *pcm, char *hexstr);

/*
 * Returns the index of the lowest bit in a coremask holder.
 */
static inline int cvmx_coremask_lowest_bit(cvmx_coremask_holder_t h)
{
	return __builtin_ctzll(h);
}

/*
 * Returns the 0-based index of the highest bit in a coremask holder.
 */
static inline int cvmx_coremask_highest_bit(cvmx_coremask_holder_t h)
{
	return (64 - __builtin_clzll(h) - 1);
}

/**
 * Returns the last core within the coremask and -1 when the coremask
 * is empty.
 *
 * @param[in] pcm - pointer to coremask
 * @returns last core set in the coremask or -1 if all clear
 *
 */
static inline int cvmx_coremask_get_last_core(const cvmx_coremask_t *pcm)
{
	int i;
	int found = -1;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++) {
		if (pcm->coremask_bitmap[i])
			found = i;
	}

	if (found == -1)
		return -1;

	return found * CVMX_COREMASK_HLDRSZ +
	     cvmx_coremask_highest_bit(pcm->coremask_bitmap[found]);
}

/**
 * Returns the first core within the coremask and -1 when the coremask
 * is empty.
 *
 * @param[in] pcm - pointer to coremask
 * @returns first core set in the coremask or -1 if all clear
 *
 */
static inline int cvmx_coremask_get_first_core(const cvmx_coremask_t *pcm)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if (pcm->coremask_bitmap[i])
			break;

	if (i == CVMX_COREMASK_USED_BMPSZ)
		return -1;

	return i * CVMX_COREMASK_HLDRSZ +
	    cvmx_coremask_lowest_bit(pcm->coremask_bitmap[i]);
}

/**
 * Given a core and coremask, return the next available core in the coremask
 * or -1 if none are available.
 *
 * @param core - starting core to check (can be -1 for core 0)
 * @param pcm - pointer to coremask to check for the next core.
 *
 * @return next core following the core parameter or -1 if no more cores.
 */
static inline int cvmx_coremask_next_core(int core,
					  const cvmx_coremask_t *pcm)
{
	int n, i;
	core++;
	n = core % CVMX_COREMASK_HLDRSZ;
	i = core / CVMX_COREMASK_HLDRSZ;

	if (pcm->coremask_bitmap[i] != 0) {
		for (; n < CVMX_COREMASK_HLDRSZ; n++)
			if (pcm->coremask_bitmap[i] & (1ULL << n))
				return ((i * CVMX_COREMASK_HLDRSZ) + n);
	}
	for (i = i + 1; i < CVMX_COREMASK_USED_BMPSZ; i++) {
		if (pcm->coremask_bitmap[i] != 0)
			return (i * CVMX_COREMASK_HLDRSZ) +
				cvmx_coremask_lowest_bit(pcm->coremask_bitmap[i]);
	}
	return -1;
}

/**
 * Compute coremask for count cores starting with start_core.
 * Note that the coremask for multi-node processors may have
 * gaps.
 *
 * @param[out]  pcm        pointer to core mask data structure
 * @param	start_core starting code number
 * @param       count      number of cores
 *
 */
static inline void cvmx_coremask_set_cores(cvmx_coremask_t *pcm,
					   unsigned int start_core,
					   unsigned int count)
{
	int node;
	int core;	/** Current core in node */
	int cores_in_node;
	int i;
#ifndef __linux__
	assert(CVMX_MAX_CORES < CVMX_COREMASK_HLDRSZ);
#endif
	node = start_core >> CVMX_NODE_NO_SHIFT;
	core = start_core & ((1 << CVMX_NODE_NO_SHIFT) - 1);
#ifndef __linux__
	assert(core < CVMX_MAX_CORES);
#endif

	cvmx_coremask_clear_all(pcm);
	while (count > 0) {
		if (count + core > CVMX_MAX_CORES)
			cores_in_node = CVMX_MAX_CORES - core;
		else
			cores_in_node = count;

		i = CVMX_COREMASK_BMP_NODE_CORE_IDX(node, core);
		pcm->coremask_bitmap[i] = ((1ULL << cores_in_node) - 1) << core;
		count -= cores_in_node;
		core = 0;
		node++;
	}
}


/**
 * Makes a copy of a coremask
 *
 * @param[out] dest - pointer to destination coremask
 * @param[in]  src  - pointer to source coremask
 */
static inline void cvmx_coremask_copy(cvmx_coremask_t *dest,
				      const cvmx_coremask_t *src)
{
	memcpy(dest, src, sizeof(*dest));
}

/**
 * Test to see if the specified core is first core in coremask.
 *
 * @param[in]  pcm  pointer to the coremask to test against
 * @param[in]  core core to check
 *
 * @return  1 if the core is first core in the coremask, 0 otherwise
 *
 */
static inline int
cvmx_coremask_is_core_first_core(const cvmx_coremask_t *pcm,
				 unsigned int core)
{
	int n, i;
	n = core / CVMX_COREMASK_HLDRSZ;


	for (i = 0; i < n; i++)
		if (pcm->coremask_bitmap[i] != 0)
			return 0;
	/* From now on we only care about the core number within an entry */
	core &= (CVMX_COREMASK_HLDRSZ - 1);
	if (__builtin_ffsll(pcm->coremask_bitmap[n]) < (core + 1))
		return 0;
	return (__builtin_ffsll(pcm->coremask_bitmap[n]) == core + 1);
}

/*
 * NOTE:
 * cvmx_coremask_is_first_core() was retired due to improper usage.
 * For inquiring about the current core being the initializing
 * core for an application, use cvmx_is_init_core().
 * For simply inquring if the current core is numerically
 * lowest in a given mask, use :
 * 	cvmx_coremask_is_core_first_core( pcm, dvmx_get_core_num())
 */

/**
 * Returns the number of 1 bits set in a coremask
 *
 * @param[in] pcm - pointer to core mask
 *
 * @return number of bits set in the coremask
 */
static inline int cvmx_coremask_get_core_count(const cvmx_coremask_t *pcm)
{
	int i;
	int count = 0;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		count += __builtin_popcountll(pcm->coremask_bitmap[i]);

	return count;
}

/**
 * For multi-node systems, return the node a core belongs to.
 *
 * @param core - core number (0-1023)
 *
 * @return node number core belongs to
 */
static inline int cvmx_coremask_core_to_node(int core)
{
	return (core >> CVMX_NODE_NO_SHIFT) & CVMX_NODE_MASK;
}

/**
 * Given a core number on a multi-node system, return the core number for a
 * particular node.
 *
 * @param core - global core number
 *
 * @returns core number local to the node.
 */
static inline int cvmx_coremask_core_on_node(int core)
{
	return (core & ((1 << CVMX_NODE_NO_SHIFT) - 1));
}


/**
 * Returns if one coremask is a subset of another coremask
 *
 * @param main - main coremask to test
 * @param subset - subset coremask to test
 *
 * @return 0 if the subset contains cores not in the main coremask or 1 if
 *         the subset is fully contained in the main coremask.
 */
static inline int cvmx_coremask_is_subset(const cvmx_coremask_t *main,
					  const cvmx_coremask_t *subset)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if ((main->coremask_bitmap[i] & subset->coremask_bitmap[i]) !=
		    subset->coremask_bitmap[i])
			return 0;
	return 1;
}

/**
 * Returns if one coremask intersects another coremask
 *
 * @param c1 - main coremask to test
 * @param c2 - subset coremask to test
 *
 * @return 1 if coremask c1 intersects coremask c2, 0 if they are exclusive
 */
static inline int cvmx_coremask_intersects(const cvmx_coremask_t *c1,
					   const cvmx_coremask_t *c2)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_USED_BMPSZ; i++)
		if ((c1->coremask_bitmap[i] & c2->coremask_bitmap[i]) != 0)
			return 1;
	return 0;
}

/**
 * Masks a single node of a coremask
 *
 * @param pcm - coremask to mask [inout]
 * @param node       - node number to mask against
 */
static inline void cvmx_coremask_mask_node(cvmx_coremask_t *pcm, int node)
{
	int i;

	for (i = 0; i < CVMX_COREMASK_BMP_NODE_CORE_IDX(node, 0); i++)
		pcm->coremask_bitmap[i] = 0;
	for (i = CVMX_COREMASK_BMP_NODE_CORE_IDX(node + 1, 0);
	     i < CVMX_COREMASK_USED_BMPSZ; i++)
		pcm->coremask_bitmap[i] = 0;
}

/**
 * Wait (stall) until all cores in the given coremask has reached this point
 * in the program execution before proceeding.
 *
 * @param  pcm  the group of cores performing the barrier sync
 *
 */
extern void cvmx_coremask_barrier_sync(const cvmx_coremask_t *pcm);

/**
 * Prints out a coremask in the form of node X: 0x... 0x...
 *
 * @param[in] pcm - pointer to core mask
 *
 * @return nothing
 */
void cvmx_coremask_print(const cvmx_coremask_t *pcm);

static inline void cvmx_coremask_dprint(const cvmx_coremask_t *pcm)
{
#if defined(__U_BOOT__) && defined(DEBUG)
	cvmx_coremask_print(pcm);
#else
  (void)(pcm);
#endif
}

/**
 * Gets the 'avail_coremask' from the dedicated (BOOTINFO) named block
 * @param[out] coremask pointer where to copy avail_coremask
 * @return 0 on success, -1 otherwise
 */
extern int cvmx_get_avail_coremask(cvmx_coremask_t *coremask);


/**
 * Gets the 'hardware_coremask' from the dedicated (BOOTINFO) named block
 * @param[out] coremask pointer where to copy hardware_coremask
 * @return 0 on success, -1 otherwise
 */
extern  int cvmx_get_hardware_coremask(cvmx_coremask_t *coremask);


#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_COREMASK_H__ */
