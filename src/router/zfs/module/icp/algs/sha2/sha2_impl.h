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

#ifndef	SHA2_IMPL_H
#define	SHA2_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/sha2.h>

/* transform function definition */
typedef void (*sha256_f)(uint32_t state[8], const void *data, size_t blks);
typedef void (*sha512_f)(uint64_t state[8], const void *data, size_t blks);

/* needed for checking valid implementations */
typedef boolean_t (*sha2_is_supported_f)(void);

typedef struct {
	const char *name;
	sha256_f transform;
	sha2_is_supported_f is_supported;
} sha256_ops_t;

typedef struct {
	const char *name;
	sha512_f transform;
	sha2_is_supported_f is_supported;
} sha512_ops_t;

extern const sha256_ops_t *sha256_get_ops(void);
extern const sha512_ops_t *sha512_get_ops(void);

#ifdef	__cplusplus
}
#endif

#endif /* _SHA2_IMPL_H */
