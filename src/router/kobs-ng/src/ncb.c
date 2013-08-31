/*
 *  ncb.c - verify and encode NCB
 *
 *  Copyright (c) 2008 by Embedded Alley Solution Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "mtd.h"
#include "config.h"
#include "rom_nand_hamming_code_ecc.h"

static inline int even_number_of_1s(uint8_t byte)
{
	int even = 1;

        while (byte > 0) {
		even ^= (byte & 0x1);
		byte >>= 1;
	}
	return even;
}


#define BIT(v,n)	(((v) >> (n)) & 0x1)
#define B(n)		(BIT(d,n))
#define BSEQ(a1,a2,a3,a4,a5,a6,a7,a8) \
	(B(a1) ^ B(a2) ^ B(a3) ^ B(a4) ^ B(a5) ^ B(a6) ^ B(a7) ^ B(a8))

static uint8_t calculate_parity_22_16(uint16_t d)
{
	uint8_t p = 0;

	if (d == 0 || d == 0xFFFF)
		return 0;       /* optimization :) */

	p |= BSEQ(15, 12, 11,  8,  5,  4,  3,  2) << 0;
	p |= BSEQ(13, 12, 11, 10,  9,  7,  3,  1) << 1;
	p |= BSEQ(15, 14, 13, 11, 10,  9,  6,  5) << 2;
	p |= BSEQ(15, 14, 13,  8,  7,  6,  4,  0) << 3;
	p |= BSEQ(12,  9,  8,  7,  6,  2,  1,  0) << 4;
	p |= BSEQ(14, 10,  5,  4,  3,  2,  1,  0) << 5;
	return p;
}

static uint8_t calculate_parity_13_8(uint8_t d)
{
        uint8_t p = 0;

	p |= (B(6) ^ B(5) ^ B(3) ^ B(2))        << 0;
	p |= (B(7) ^ B(5) ^ B(4) ^ B(2) ^ B(1)) << 1;
	p |= (B(7) ^ B(6) ^ B(5) ^ B(1) ^ B(0)) << 2;
	p |= (B(7) ^ B(4) ^ B(3) ^ B(0))        << 3;
	p |= (B(6) ^ B(4) ^ B(3) ^ B(2) ^ B(1) ^ B(0)) << 4;
	return p;
}
#undef BIT
#undef B
#undef BSEQ

static int encode_hamming_code_22_16(void *source_block, size_t source_size,
			       void *target_block, size_t target_size)
{
	int i, j, bit_index;
	uint16_t *src;
	uint8_t *dst;
	uint8_t np;
	uint8_t ecc[NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES];
	uint8_t data[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES];

	memset(data, 0, sizeof(data));
	memcpy(data, source_block, source_size);

	src = (uint16_t *) data;
	dst = (uint8_t *) target_block;

	/* create THREE copies of source block */
	for (i = 0; i < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES; i++) {
		dst[i + NAND_HC_ECC_OFFSET_FIRST_DATA_COPY] =
		    dst[i + NAND_HC_ECC_OFFSET_SECOND_DATA_COPY] =
		    dst[i + NAND_HC_ECC_OFFSET_THIRD_DATA_COPY] =
		    ((uint8_t *) src)[i];
	}

	/* finally, point to the end of populated data */
	for (bit_index = j = i = 0;
	     j < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES / sizeof(uint16_t);
	     j++) {
		np = calculate_parity_22_16(src[j]);

		switch (bit_index) {

		case 0:
			ecc[i] = np & 0x3F;
			break;
		case 2:
			ecc[i++] |= (np & 0x03) << 6;
			ecc[i] = (np & 0x3C) >> 2;
			break;
		case 4:
			ecc[i++] |= (np & 0x0F) << 4;
			ecc[i] = (np & 0x30) >> 4;
			break;
		case 6:
			ecc[i++] |= (np & 0x3F) << 2;
			break;
		}
		bit_index = (bit_index + 2) % 8;
	}

	for (i = 0; i < NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES; i++) {
		dst[i + NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY] =
		    dst[i + NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY] =
		    dst[i + NAND_HC_ECC_OFFSET_THIRD_PARITY_COPY] = ecc[i];
	}

	return 0;
}


static int encode_hamming_code_13_8(void *source_block, size_t source_size,
			       void *target_block, size_t target_size)
{
	uint8_t ecc[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES];
	uint8_t data[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES];
	int i;

	memset(ecc, 0, sizeof(ecc));
	memset(data, 0, sizeof(data));
	memcpy(data, source_block, source_size);

	for (i = 0; i < source_size; i ++)
		ecc[i] = calculate_parity_13_8(data[i]);

	memcpy((uint8_t*)target_block + 12, data, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);
	memcpy((uint8_t*)target_block + 12 + NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES,
			ecc, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);

	return 0;
}

static int ncb_single_pair_check(uint8_t *n1, uint8_t *p1, uint8_t *n2, uint8_t *p2)
{
	return (memcmp(n1, n2, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES) == 0 &&
		memcmp(p1, p2, NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES) == 0);
}

static int ncb_triple_check(void *page)
{
	uint8_t *n1 = (uint8_t*)page + NAND_HC_ECC_OFFSET_FIRST_DATA_COPY,
		*n2 = (uint8_t*)page + NAND_HC_ECC_OFFSET_SECOND_DATA_COPY,
		*n3 = (uint8_t*)page + NAND_HC_ECC_OFFSET_THIRD_DATA_COPY,
		*p1 = (uint8_t*)page + NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY,
		*p2 = (uint8_t*)page + NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY,
		*p3 = (uint8_t*)page + NAND_HC_ECC_OFFSET_THIRD_PARITY_COPY;

	if (ncb_single_pair_check(n1, p1, n2, p2))
		return 1;
	if (ncb_single_pair_check(n2, p2, n3, p3))
		return 2;
	if (ncb_single_pair_check(n1, p1, n3, p3))
		return 1;
	return -1;
}

static int lookup_single_error_22_16(uint8_t syndrome)
{
	int i;
	uint8_t syndrome_table[] = {
		0x38, 0x32, 0x31, 0x23, 0x29, 0x25, 0x1C, 0x1A,
		0x19, 0x16, 0x26, 0x07, 0x13, 0x0E, 0x2C, 0x0D,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
	};

	for (i = 0; i < ARRAY_SIZE(syndrome_table); i ++)
		if (syndrome_table[i] == syndrome)
			return i;
	return -ENOENT;
}

static int lookup_single_error_13_8(uint8_t syndrome)
{
	int i;
	uint8_t syndrome_table[] = {
		0x1C, 0x16, 0x13, 0x19,
		0x1A, 0x07, 0x15, 0x0E,
		0x01, 0x02, 0x04, 0x08,
		0x10,
	};

	for (i = 0; i < ARRAY_SIZE(syndrome_table); i ++)
		if (syndrome_table[i] == syndrome)
			return i;
	return -ENOENT;
}

static inline BootBlockStruct_t *ncb_verify_hamming_22_16(void *page)
{
	int r;
	uint16_t* n1 = (uint16_t*)((uint8_t*)page + NAND_HC_ECC_OFFSET_FIRST_DATA_COPY),
		 *n2 = ((uint16_t*)(uint8_t*)page + NAND_HC_ECC_OFFSET_SECOND_DATA_COPY),
		 *n, *data;
	uint8_t  *p1 = (uint8_t*)page + NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY,
		 *p2 = (uint8_t*)page + NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY,
		 *parity, p, np, syndrome;
	int bit_index, i, j, bit_to_flip;

	r = ncb_triple_check(page);
	if (r < 0)
		return NULL;
	if (r == 1) {
		data = n = n1;
		parity = p1;
	}
	else if (r == 2) {
		data = n = n2;
		parity = p2;
	}
	else {
		fprintf(stderr, "internal error: %s, r = %d\n", __func__, r);
		return NULL;
	}

	for (bit_index = i = j = 0, r = 0;
	     i <  NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES / sizeof(uint16_t) && r == 0;
	     i ++, data++) {

		switch (bit_index) {

		case 0:
			p = parity[j] & 0x3F;
			break;
		case 2:
			p = (parity[j++] & 0xC0) >> 6;
			p |= (parity[j] & 0x0F) << 2;
			break;
		case 4:
			p = (parity[j++] & 0xF0) >> 4;
			p |= (parity[j] & 0x03) << 4;
			break;
		case 6:
			p = (parity[j++] & 0xFC) >> 2;
			break;
		default:
			fprintf(stderr, "internal error at %s:%d\n", __func__, __LINE__);
			exit(5);
		}
		bit_index = (bit_index + 2) % 8;

		np = calculate_parity_22_16(*data);
		syndrome = np ^ p;
		if (syndrome == 0) /* cool */ {
			continue;
		}

		if (even_number_of_1s(syndrome)) {
			r = i;
			break;
		}

		bit_to_flip = lookup_single_error_22_16(syndrome);
		if (bit_to_flip < 0) {
			r = i;
			break;
		}

		if (bit_to_flip < 16)
			*data ^= (1 << bit_to_flip);
	}
	return r == 0 ? (BootBlockStruct_t*)n : NULL;
}

static inline BootBlockStruct_t *ncb_verify_hamming_13_8(void *ncb_page)
{
	int i, bit_to_flip;
	uint8_t *data, *parity, np, syndrome;

	data = (uint8_t*)ncb_page + 12;
	parity = (uint8_t*)ncb_page + 12 + NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES;

	for (i = 0; i < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES; i ++, data++) {
		np = calculate_parity_13_8(*data);
		syndrome = np ^ parity[i];
		if (syndrome == 0)
			continue;

		if (even_number_of_1s(syndrome))
			return NULL;

		bit_to_flip = lookup_single_error_13_8(syndrome);
		if (bit_to_flip < 0)
			return NULL;

		if (bit_to_flip < 8)
			*data ^= (1 << bit_to_flip);
	}
	return (BootBlockStruct_t*)(ncb_page + 12);
}

/**
 * ncb_encrypt - Encrypt the NCB block, assuming that target system uses NCB
 * version 'version'
 *
 * ncb:     Points to valid BootBlockStruct_t structure
 * target:  Points to a buffer large enough to contain an entire NAND Flash page
 *          (both data and OOB).
 * size:    The size of an entire NAND Flash page (both data and OOB).
 * version: The version number of the NCB.
 */
int ncb_encrypt(BootBlockStruct_t *ncb, void *target, size_t size, int version)
{
	assert(version == 0 || version == 1 || version == 3);
	assert(size >= sizeof(BootBlockStruct_t));

	memset(target, size, 0);

	switch (version)
	{
	case 0:
		memcpy(target, ncb, sizeof(*ncb));
		return size;
	case 1:
		return encode_hamming_code_22_16(ncb, sizeof(*ncb), target, size);
	case 3:
		return encode_hamming_code_13_8(ncb, sizeof(*ncb), target, size);
	default:
		fprintf(stderr, "NCB version == %d? Something is wrong!\n", version);
		return -EINVAL;
	}
}

/**
 * fcb_encrypt - Encrypt the FCB block, assuming that target system uses NCB
 * version 'version'
 *
 * fcb:     Points to valid imx28_BootBlockStruct_t structure.
 * target:  Points to a buffer large enough to contain an entire NAND Flash page
 *          (both data and OOB).
 * size:    The size of an entire NAND Flash page (both data and OOB).
 * version: The version number of the NCB.
 *
 */
int fcb_encrypt(V1_ROM_BootBlockStruct_t *fcb, void *target, size_t size, int version)
{
	uint32_t  accumulator;
	uint8_t   *p;
	uint8_t   *q;

	//----------------------------------------------------------------------
	// Check for nonsense.
	//----------------------------------------------------------------------

	assert(version == 1);
	assert(size >= sizeof(V1_ROM_BootBlockStruct_t));

	//----------------------------------------------------------------------
	// Clear out the target.
	//----------------------------------------------------------------------

	memset(target, size, 0);

	//----------------------------------------------------------------------
	// Compute the checksum.
	//
	// Note that we're computing the checksum only over the FCB itself,
	// whereas it's actually supposed to reflect the entire 508 bytes
	// in the FCB page between the base of the of FCB and the base of the
	// ECC bytes. However, the entire space between the top of the FCB and
	// the base of the ECC bytes will be all zeros, so this is OK.
	//----------------------------------------------------------------------

	p = ((uint8_t *) fcb) + 4;
	q = (uint8_t *) (fcb + 1);

	accumulator = 0;

	for (; p < q; p++) {
		accumulator += *p;
	}

	accumulator ^= 0xffffffff;

	fcb->m_u32Checksum = accumulator;

	//----------------------------------------------------------------------
	// Compute the ECC bytes.
	//----------------------------------------------------------------------

	switch (version)
	{
	case 0:
		memcpy(target, fcb, sizeof(*fcb));
		return size;
	case 1:
		return encode_hamming_code_13_8(fcb, sizeof(*fcb), target, size);
	default:
		fprintf(stderr, "FCB version == %d? Something is wrong!\n", version);
		return -EINVAL;
	}
}

/**
 * ncb_get_version - parse the boot block ncb_candidate and on success store
 * the pointer to NCB to result
 *
 * returns version of found NCB, or -1 otherwise - try further scanning..
 */
int ncb_get_version(void *ncb_candidate, BootBlockStruct_t **result)
{
	BootBlockStruct_t *bbs, *p1, *p2, *p3;

	assert(result != NULL);

	*result = NULL;

	/* first of all, check version 3 of ncb */
	bbs = (BootBlockStruct_t*)((uint8_t*)ncb_candidate + 12);
	if (bbs->m_u32FingerPrint1 == NCB_FINGERPRINT1 &&
	    bbs->m_u32FingerPrint2 == NCB_FINGERPRINT2 &&
	    bbs->m_u32FingerPrint3 == NCB_FINGERPRINT3) {
		/* fingerprints match, so it is either v3 or corrupted NCB */
		*result = ncb_verify_hamming_13_8(ncb_candidate);
		if (*result)
			return 3;
		fprintf(stderr, "ncb_verify_hamming_13_8 failed!\n");
		return -1;
	}

	/*
	 * then, check if it is version 1 (yes, there was not NCBv2
	 *
	 * To match the v1, there should be at least two identical copies
	 * of NCB
	 */

	p1 = (BootBlockStruct_t*)ncb_candidate /* + NAND_HC_OFFSET_FIRST_DATA_COPY */;
	p2 = (BootBlockStruct_t*)((uint8_t*)ncb_candidate + NAND_HC_ECC_OFFSET_SECOND_DATA_COPY);
	p3 = (BootBlockStruct_t*)((uint8_t*)ncb_candidate + NAND_HC_ECC_OFFSET_THIRD_DATA_COPY);

	if (memcmp(p1, p2, sizeof(BootBlockStruct_t)) == 0 ||
	    memcmp(p1, p3, sizeof(BootBlockStruct_t)) == 0 ||
	    memcmp(p2, p3, sizeof(BootBlockStruct_t)) == 0) {
		/*
		 * we found at least two identical copies of NCB; verify
		 * against the ECC
		 */
		*result = ncb_verify_hamming_22_16(ncb_candidate);
		if (*result)
			return 1;
		return -1;
	}

	/*
	 * and finally, does it look like NCBv0 ?
	 */
	if (p1->m_u32FingerPrint1 == NCB_FINGERPRINT1 &&
	    p1->m_u32FingerPrint2 == NCB_FINGERPRINT2 &&
	    p1->m_u32FingerPrint3 == NCB_FINGERPRINT3) {
		*result = p1;
		return 0;
	}

	/*
	 * we did try.
	 */
	return -1;
}


