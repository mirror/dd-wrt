// SPDX-License-Identifier: GPL-2.0
/*
 * Use slice-by-8, which is the fastest variant.
 *
 * Calculate checksum 8 bytes at a time with a clever slicing algorithm.
 * This is the fastest algorithm, but comes with a 8KiB lookup table.
 * Most modern processors have enough cache to hold this table without
 * thrashing the cache.
 *
 * The Linux kernel uses this as the default implementation "unless you
 * have a good reason not to".  The reason why Kconfig urges you to pick
 * SLICEBY8 is because people challenged the assertion that we should
 * always use slice by 8, so Darrick wrote a crc microbenchmark utility
 * and ran it on as many machines as he could get his hands on to show
 * that sb8 was the fastest.
 *
 * Every 64-bit machine (and most of the 32-bit ones too) saw the best
 * results with sb8.  Any machine with more than 4K of cache saw better
 * results.  The spreadsheet still exists today[1]; note that
 * 'crc32-kern-le' corresponds to the slice by 4 algorithm which is the
 * default unless CRC_LE_BITS is defined explicitly.
 *
 * FWIW, there are a handful of board defconfigs in the kernel that
 * don't pick sliceby8.  These are all embedded 32-bit mips/ppc systems
 * with very small cache sizes which experience cache thrashing with the
 * slice by 8 algorithm, and therefore chose to pick defaults that are
 * saner for their particular board configuration.  For nearly all of
 * XFS' perceived userbase (which we assume are 32 and 64-bit machines
 * with sufficiently large CPU cache and largeish storage devices) slice
 * by 8 is the right choice.
 *
 * [1] https://goo.gl/0LSzsG ("crc32c_bench")
 */
#define CRC_LE_BITS 64

/*
 * This is the CRC32c polynomial, as outlined by Castagnoli.
 * x^32+x^28+x^27+x^26+x^25+x^23+x^22+x^20+x^19+x^18+x^14+x^13+x^11+x^10+x^9+
 * x^8+x^6+x^0
 */
#define CRC32C_POLY_LE 0x82F63B78

/*
 * Little-endian CRC computation.  Used with serial bit streams sent
 * lsbit-first.  Be sure to use cpu_to_le32() to append the computed CRC.
 */
#if CRC_LE_BITS > 64 || CRC_LE_BITS < 1 || CRC_LE_BITS == 16 || \
	CRC_LE_BITS & CRC_LE_BITS-1
# error "CRC_LE_BITS must be one of {1, 2, 4, 8, 32, 64}"
#endif
