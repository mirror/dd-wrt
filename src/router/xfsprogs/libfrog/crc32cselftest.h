// SPDX-License-Identifier: GPL-2.0
/*
 * Aug 8, 2011 Bob Pearson with help from Joakim Tjernlund and George Spelvin
 * cleaned up code to current version of sparse and added the slicing-by-8
 * algorithm to the closely similar existing slicing-by-4 algorithm.
 * Oct 15, 2000 Matt Domsch <Matt_Domsch@dell.com>
 * Nicer crc32 functions/docs submitted by linux@horizon.com.  Thanks!
 * Code was from the public domain, copyright abandoned.  Code was
 * subsequently included in the kernel, thus was re-licensed under the
 * GNU GPL v2.
 * Oct 12, 2000 Matt Domsch <Matt_Domsch@dell.com>
 * Same crc32 function was used in 5 other places in the kernel.
 * I made one version, and deleted the others.
 * There are various incantations of crc32().  Some use a seed of 0 or ~0.
 * Some xor at the end with ~0.  The generic crc32() function takes
 * seed as an argument, and doesn't xor at the end.  Then individual
 * users can do whatever they need.
 *   drivers/net/smc9194.c uses seed ~0, doesn't xor with ~0.
 *   fs/jffs2 uses seed 0, doesn't xor with ~0.
 *   fs/partitions/efi.c uses seed ~0, xor's with ~0.
 */

/* see: Documentation/crc32.txt for a description of algorithms */

/*
 * lifted from the 3.8-rc2 kernel source for xfsprogs. Killed CONFIG_X86
 * specific bits for just the generic algorithm. Also removed the big endian
 * version of the algorithm as XFS only uses the little endian CRC version to
 * match the hardware acceleration available on Intel CPUs.
 */

/* This is just the crc32 self test bits from crc32.c. */
#include "libfrog/randbytes.h"

#ifndef __LIBFROG_CRC32CSELFTEST_H__
#define __LIBFROG_CRC32CSELFTEST_H__

/* 100 test cases */
static struct crc_test {
	uint32_t crc;	/* random starting crc */
	uint32_t start;	/* random 6 bit offset in buf */
	uint32_t length;	/* random 11 bit length of test */
	uint32_t crc32c_le;	/* expected crc32c_le result */
} crc_tests[] =
{
	{0x674bf11d, 0x00000038, 0x00000542, 0xf6e93d6c},
	{0x35c672c6, 0x0000003a, 0x000001aa, 0x0fe92aca},
	{0x496da28e, 0x00000039, 0x000005af, 0x52e1ebb8},
	{0x09a9b90e, 0x00000027, 0x000001f8, 0x0798af9a},
	{0xdc97e5a9, 0x00000025, 0x000003b6, 0x18eb3152},
	{0x47c58900, 0x0000000a, 0x000000b9, 0xd00d08c7},
	{0x292561e8, 0x0000000c, 0x00000403, 0x8ba966bc},
	{0x415037f6, 0x00000003, 0x00000676, 0x11d694a2},
	{0x3466e707, 0x00000026, 0x00000042, 0x6ab3208d},
	{0xafd1281b, 0x00000023, 0x000002ee, 0xba4603c5},
	{0xd3857b18, 0x00000028, 0x000004a2, 0xe6071c6f},
	{0x1d825a8f, 0x0000002b, 0x0000050b, 0x179ec30a},
	{0x5033e3bc, 0x0000000b, 0x00000078, 0x0903beb8},
	{0x94f1fb5e, 0x0000000f, 0x000003a2, 0x6a7cb4fa},
	{0xc9a0fe14, 0x00000009, 0x00000473, 0xdb535801},
	{0x88a034b1, 0x0000001c, 0x000005ad, 0x92bed597},
	{0xf0f72239, 0x00000020, 0x0000026d, 0x192a3f1b},
	{0xcc20a5e3, 0x0000003b, 0x0000067a, 0xccbaec1a},
	{0xce589c95, 0x0000002b, 0x00000641, 0x7eabae4d},
	{0x78edc885, 0x00000035, 0x000005be, 0x28c72982},
	{0x9d40a377, 0x0000003b, 0x00000038, 0xc3cd4d18},
	{0x703d0e01, 0x0000003c, 0x000006f1, 0xbca8f0e7},
	{0x776bf505, 0x0000000f, 0x000005b2, 0x713f60b3},
	{0x4a3e7854, 0x00000027, 0x000004b8, 0xebd08fd5},
	{0x209172dd, 0x0000003b, 0x00000356, 0x64406c59},
	{0x3ba4cc5b, 0x0000002f, 0x00000203, 0x7421890e},
	{0xfc62f297, 0x00000000, 0x00000079, 0xe9347603},
	{0x64280b8b, 0x00000016, 0x000007ab, 0x1bef9060},
	{0x97dd724b, 0x00000033, 0x000007ad, 0x34720072},
	{0x61394b52, 0x00000035, 0x00000571, 0x48310f59},
	{0x29b4faff, 0x00000024, 0x0000006e, 0x783a4213},
	{0x29bfb1dc, 0x0000000b, 0x00000244, 0x9e8efd41},
	{0x86ae934b, 0x00000035, 0x00000104, 0xfc3d34a5},
	{0xc4c1024e, 0x0000002e, 0x000006b1, 0x17a52ae2},
	{0x3287a80a, 0x00000026, 0x00000496, 0x886d935a},
	{0xa4db423e, 0x00000023, 0x0000045d, 0xeaaeaeb2},
	{0x7a1078df, 0x00000015, 0x0000014a, 0x8e900a4b},
	{0x6048bd5b, 0x00000006, 0x0000006a, 0xd74662b1},
	{0xd8f9ea20, 0x0000003d, 0x00000277, 0xd26752ba},
	{0xea5ec3b4, 0x0000002a, 0x000004fe, 0x8b1fcd62},
	{0x2dfb005d, 0x00000016, 0x00000345, 0xf54342fe},
	{0x5a214ade, 0x00000020, 0x000005b6, 0x5b95b988},
	{0xf0ab9cca, 0x00000032, 0x00000515, 0x2e1176be},
	{0x91b444f9, 0x0000002e, 0x000007f8, 0x66120546},
	{0x1b5d2ddb, 0x0000002e, 0x0000012c, 0xf256a5cc},
	{0xd824d1bb, 0x0000003a, 0x000007b5, 0x4af1dd69},
	{0x0470180c, 0x00000034, 0x000001f0, 0x56f0a04a},
	{0xffaa3a3f, 0x00000036, 0x00000299, 0x74f6b6b2},
	{0x6406cfeb, 0x00000023, 0x00000600, 0x085951fd},
	{0xb24aaa38, 0x0000003e, 0x000004a1, 0xc65387eb},
	{0x58b2ab7c, 0x00000039, 0x000002b4, 0x1ca9257b},
	{0x3db85970, 0x00000006, 0x000002b6, 0xfd196d76},
	{0x857830c5, 0x00000003, 0x00000590, 0x5ef88339},
	{0xe1fcd978, 0x0000003e, 0x000007d8, 0x2c3714d9},
	{0xb982a768, 0x00000016, 0x000006e0, 0x58576548},
	{0x1d581ce8, 0x0000001e, 0x0000058b, 0xfd7c57de},
	{0x2456719b, 0x00000025, 0x00000503, 0xd5fedd59},
	{0xfae6d8f2, 0x00000000, 0x0000055d, 0x1cc3b17b},
	{0xcba828e3, 0x00000039, 0x000002ce, 0x270eed73},
	{0x13d25952, 0x0000000a, 0x0000072d, 0x91ecbb11},
	{0x0342be3f, 0x00000015, 0x00000599, 0x05ed8d0c},
	{0xeaa344e0, 0x00000014, 0x000004d8, 0x0b09ad5b},
	{0xbbb52021, 0x0000003b, 0x00000272, 0xf8d511fb},
	{0xb66384dc, 0x0000001d, 0x000007fc, 0x5ad832cc},
	{0x616c01b6, 0x00000022, 0x000002c8, 0x1214d196},
	{0xce2bdaad, 0x00000016, 0x0000062a, 0x5747218a},
	{0x00fe84d7, 0x00000005, 0x00000205, 0xde8f14de},
	{0xbebdcb4c, 0x00000006, 0x0000055d, 0x3563b7b9},
	{0xd8b1a02a, 0x00000010, 0x00000387, 0x071475d0},
	{0x3b96cad2, 0x00000036, 0x00000347, 0x54c79d60},
	{0xc94c1ed7, 0x00000005, 0x0000038b, 0x4c53eee6},
	{0x1aad454e, 0x00000025, 0x000002b2, 0x10137a3c},
	{0xa4fec9a6, 0x00000000, 0x000006d6, 0xaa9d6c73},
	{0x1bbe71e2, 0x0000001f, 0x000002fd, 0xb63d23e7},
	{0x4201c7e4, 0x00000002, 0x000002b7, 0x7f53e9cf},
	{0x23fddc96, 0x00000003, 0x00000627, 0x13c1cd83},
	{0xd82ba25c, 0x00000016, 0x0000063e, 0x49ff5867},
	{0x786f2032, 0x0000002d, 0x0000060f, 0x8467f211},
	{0xfebe4e1f, 0x0000002a, 0x000004f2, 0x3f9683b2},
	{0x1a6e0a39, 0x00000008, 0x00000672, 0x76a3f874},
	{0x56000ab8, 0x0000000e, 0x000000e5, 0x863b702f},
	{0x4717fe0c, 0x00000000, 0x000006ec, 0xdc6c58ff},
	{0xd5d5d68e, 0x0000003c, 0x000003a3, 0x0622cc95},
	{0xc25dd6c6, 0x00000024, 0x000006c0, 0xe85605cd},
	{0xe9b11300, 0x00000023, 0x00000683, 0x31da5f06},
	{0x95cd285e, 0x00000001, 0x00000047, 0xa1f2e784},
	{0xd9245a25, 0x0000001e, 0x000003a6, 0xb07cc616},
	{0x103279db, 0x00000006, 0x0000039b, 0xbf943b6c},
	{0x1cba3172, 0x00000027, 0x000001c8, 0x2c01af1c},
	{0x8f613739, 0x0000000c, 0x000001df, 0x0fe5f56d},
	{0x1c6aa90d, 0x0000001b, 0x0000053c, 0xf8943b2d},
	{0xaabe5b93, 0x0000003d, 0x00000715, 0xe4d89272},
	{0xf15dd038, 0x00000006, 0x000006db, 0x7c2f6bbb},
	{0x584dd49c, 0x00000020, 0x000007bc, 0xabbf388b},
	{0x5d8c9506, 0x00000020, 0x00000470, 0x1dca1f4e},
	{0xb80d17b0, 0x00000032, 0x00000346, 0x5c170e23},
	{0xdaf0592e, 0x00000023, 0x000007b0, 0xc0e9d672},
	{0x4793cc85, 0x0000000d, 0x00000706, 0xc18bdc86},
	{0x82ebf64e, 0x00000009, 0x000007c3, 0xa874fcdd},
	{0xb18a0319, 0x00000026, 0x000007db, 0x9dc0bb48},
};

/* Don't print anything to stdout. */
#define CRC32CTEST_QUIET	(1U << 0)

static int
crc32c_test(
	unsigned int	flags)
{
	int		i;
	int		errors = 0;
	int		bytes = 0;
	struct timeval	start, stop;
	uint64_t	usec;

	/* keep static to prevent cache warming code from
	 * getting eliminated by the compiler */
	static uint32_t	crc;

	/* pre-warm the cache */
	for (i = 0; i < 100; i++) {
		bytes += 2 * crc_tests[i].length;

		crc ^= crc32c_le(crc_tests[i].crc,
				randbytes_test_buf + crc_tests[i].start,
				crc_tests[i].length);
	}

	gettimeofday(&start, NULL);
	for (i = 0; i < 100; i++) {
		crc = crc32c_le(crc_tests[i].crc,
				randbytes_test_buf + crc_tests[i].start,
				crc_tests[i].length);
		if (crc != crc_tests[i].crc32c_le)
			errors++;
	}
	gettimeofday(&stop, NULL);

	usec = stop.tv_usec - start.tv_usec +
		1000000 * (stop.tv_sec - start.tv_sec);

	if (flags & CRC32CTEST_QUIET)
		return errors;

	if (errors)
		printf("crc32c: %d self tests failed\n", errors);
	else {
		printf("crc32c: tests passed, %d bytes in %" PRIu64 " usec\n",
			bytes, usec);
	}

	return errors;
}

#endif /* __LIBFROG_CRC32CSELFTEST_H__ */
