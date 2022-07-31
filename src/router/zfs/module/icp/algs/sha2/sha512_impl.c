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

#if defined(HAVE_AVX)
static boolean_t sha2_have_avx(void)
{
	return (kfpu_allowed() && zfs_avx_available());
}

extern void sha512_transform_avx(uint64_t s[8], const void *, size_t);
const sha512_ops_t sha512_avx_impl = {
	.is_supported = sha2_have_avx,
	.tf512 = sha512_transform_avx,
	.name = "avx"
};
#endif

#if defined(HAVE_AVX2)
static boolean_t sha2_have_avx2(void)
{
	return (kfpu_allowed() && zfs_avx2_available());
}

extern void sha512_transform_avx2(uint64_t s[8], const void *, size_t);
const sha512_ops_t sha512_avx2_impl = {
	.is_supported = sha2_have_avx2,
	.tf512 = sha512_transform_avx2,
	.name = "avx2"
};
#endif

#endif /* __x86_64 */

#if defined(__aarch64__)
static boolean_t sha512_have_armv8ce(void)
{
	return (kfpu_allowed() && zfs_sha512_armv8_available());
}

extern void sha512_block_armv8_ce(uint64_t s[8], const void *, size_t);
const sha512_ops_t sha512_armv8_impl = {
	.is_supported = sha512_have_armv8ce,
	.tf512 = sha512_block_armv8_ce,
	.name = "armv8-ce"
};
#endif /* __aarch64__ */

#if defined(__PPC64__)
extern void sha512_block_ppc(uint64_t s[8], const void *, size_t);
extern void sha512_block_p8(uint64_t s[8], const void *, size_t);
#endif /* __PPC64__ */

/* the two generic ones */
extern const sha512_ops_t sha512_generic_impl;

/* array with all sha512 implementations */
static const sha512_ops_t *const sha512_impls[] = {
	&sha512_generic_impl,
#if defined(__x86_64) && defined(HAVE_AVX)
	&sha512_avx_impl,
#endif
#if defined(__x86_64) && defined(HAVE_AVX2)
	&sha512_avx2_impl,
#endif
#if defined(__aarch64__)
	&sha512_armv8_impl,
#endif
#if defined(__PPC64__)
	&sha512_ppc_impl,
	&sha512_p8_impl,
#endif /* __PPC64__ */
};

/* use the generic implementation functions */
#define	IMPL_NAME		"SHA512"
#define	IMPL_OPS_T		sha512_ops_t
#define	IMPL_IMPLS		sha512_impls
#define	IMPL_GENERIC		sha512_generic_impl
#define	ZFS_IMPL_OPS		zfs_sha512_ops
#include <generic_impl.c>

/* return selected implementation */
const sha512_ops_t *
sha512_get_ops(void)
{
	return (&sha512_generic_impl);
}
