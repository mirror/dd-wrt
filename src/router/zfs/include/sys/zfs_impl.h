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

#ifndef	_SYS_ZFS_IMPL_H
#define	_SYS_ZFS_IMPL_H

#ifdef	__cplusplus
extern "C" {
#endif

/* generic implementation backends */
typedef struct
{
	/* algorithm name */
	const char *name;

	/* return number of supported implementations */
	int (*get_impl_count)(void);

	/* return id of selected implementation */
	int (*get_impl_id)(void);

	/* return name of selected implementation */
	const char *(*get_impl_name)(void);

	/* setup id as fastest implementation */
	void (*set_impl_fastest)(uint32_t id);

	/* set implementation by id */
	void (*set_impl_id)(uint32_t id);

	/* set implementation by name */
	int (*set_impl_name)(const char *name);

	/* setup implementation */
	void (*setup_impl)(void);
} zfs_impl_t;

/* return some set of function pointer */
extern const zfs_impl_t *zfs_impl_get_ops(const char *algo);

extern const zfs_impl_t zfs_blake3_ops;
extern const zfs_impl_t zfs_sha256_ops;
extern const zfs_impl_t zfs_sha512_ops;

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_ZFS_IMPL_H */
