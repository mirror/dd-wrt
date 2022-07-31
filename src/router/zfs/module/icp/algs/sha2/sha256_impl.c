/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or https://opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 2022 Tino Reichardt <milky-zfs@mcmilk.de>
 */

#include <sys/zfs_context.h>
#include <sys/zfs_impl.h>
#include <sys/sha2.h>
#include <sys/simd.h>

#include "sha2_impl.h"

#if defined(__x86_64)

#if defined(HAVE_SSSE3)
static boolean_t sha2_have_ssse3(void)
{
	return (kfpu_allowed() && zfs_ssse3_available());
}

extern void sha256_transform_ssse3(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_ssse3_impl = {
	.is_supported = sha2_have_ssse3,
	.tf256 = sha256_transform_ssse3,
	.name = "ssse3"
};
#endif

#if defined(HAVE_AVX)
static boolean_t sha2_have_avx(void)
{
	return (kfpu_allowed() && zfs_avx_available());
}

extern void sha256_transform_avx(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_avx_impl = {
	.is_supported = sha2_have_avx,
	.tf256 = sha256_transform_avx,
	.name = "avx"
};
#endif

#if defined(HAVE_AVX2)
static boolean_t sha2_have_avx2(void)
{
	return (kfpu_allowed() && zfs_avx2_available());
}

extern void sha256_transform_avx2(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_avx2_impl = {
	.is_supported = sha2_have_avx2,
	.tf256 = sha256_transform_avx2,
	.name = "avx2"
};
#endif

#if defined(HAVE_SHANI)
static boolean_t sha2_have_shani(void)
{
	return (kfpu_allowed() && zfs_shani_available());
}

extern void sha256_transform_shani(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_shani_impl = {
	.is_supported = sha2_have_shani,
	.tf256 = sha256_transform_shani,
	.name = "shani"
};
#endif


#endif /* __x86_64 */

#if defined(__aarch64__)
static boolean_t sha256_have_neon(void)
{
	return (kfpu_allowed() && zfs_neon_available());
}

extern void sha256_block_neon(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_neon_impl = {
	.is_supported = sha256_have_neon,
	.tf256 = sha256_block_neon,
	.name = "neon"
};

static boolean_t sha256_have_armv8ce(void)
{
	return (kfpu_allowed() && zfs_sha256_armv8_available());
}

extern void sha256_block_armv8_ce(uint32_t s[8], const void *, size_t);
const sha256_ops_t sha256_armv8_impl = {
	.is_supported = sha256_have_armv8ce,
	.tf256 = sha256_block_armv8_ce,
	.name = "armv8-ce"
};
#endif /* __aarch64__ */

#if defined(__PPC64__)
extern void sha256_block_ppc(uint32_t s[8], const void *, size_t);
extern void sha256_block_p8(uint32_t s[8], const void *, size_t);
#endif /* __PPC64__ */

/* the two generic ones */
extern const sha256_ops_t sha256_generic_impl;

/* array with all sha256 implementations */
static const sha256_ops_t *const sha256_impls[] = {
	&sha256_generic_impl,
#if defined(__x86_64) && defined(HAVE_SSSE3)
	&sha256_ssse3_impl,
#endif
#if defined(__x86_64) && defined(HAVE_AVX)
	&sha256_avx_impl,
#endif
#if defined(__x86_64) && defined(HAVE_AVX2)
	&sha256_avx2_impl,
#endif
#if defined(__x86_64) && defined(HAVE_SHANI)
	&sha256_shani_impl,
#endif
#if defined(__aarch64__)
	&sha256_neon_impl,
	&sha256_armv8_impl,
#endif
#if defined(__PPC64__)
	&sha256_ppc_impl,
	&sha256_p8_impl,
#endif /* __PPC64__ */
};

/* use the generic implementation functions */
#define	IMPL_NAME		"SHA256"
#define	IMPL_OPS_T		sha256_ops_t
#define	IMPL_IMPLS		sha256_impls
#define	IMPL_GENERIC		sha256_generic_impl
#define	ZFS_IMPL_OPS		zfs_sha256_ops
#include <generic_impl.c>

/* return selected implementation */
const sha256_ops_t *
sha256_get_ops(void)
{
	return (&sha256_generic_impl);
}
