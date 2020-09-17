// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	bmapbta_flds[];
extern const struct field	bmapbta_hfld[];
extern const struct field	bmapbta_crc_flds[];
extern const struct field	bmapbta_crc_hfld[];
extern const struct field	bmapbta_key_flds[];
extern const struct field	bmapbta_rec_flds[];

extern const struct field	bmapbtd_flds[];
extern const struct field	bmapbtd_hfld[];
extern const struct field	bmapbtd_crc_flds[];
extern const struct field	bmapbtd_crc_hfld[];
extern const struct field	bmapbtd_key_flds[];
extern const struct field	bmapbtd_rec_flds[];

extern const struct field	inobt_flds[];
extern const struct field	inobt_hfld[];
extern const struct field	inobt_crc_flds[];
extern const struct field	inobt_spcrc_flds[];
extern const struct field	inobt_crc_hfld[];
extern const struct field	inobt_spcrc_hfld[];
extern const struct field	inobt_key_flds[];
extern const struct field	inobt_rec_flds[];
extern const struct field	inobt_sprec_flds[];

extern const struct field	bnobt_flds[];
extern const struct field	bnobt_hfld[];
extern const struct field	bnobt_crc_flds[];
extern const struct field	bnobt_crc_hfld[];
extern const struct field	bnobt_key_flds[];
extern const struct field	bnobt_rec_flds[];

extern const struct field	cntbt_flds[];
extern const struct field	cntbt_hfld[];
extern const struct field	cntbt_crc_flds[];
extern const struct field	cntbt_crc_hfld[];
extern const struct field	cntbt_key_flds[];
extern const struct field	cntbt_rec_flds[];

extern const struct field	rmapbt_crc_flds[];
extern const struct field	rmapbt_crc_hfld[];
extern const struct field	rmapbt_key_flds[];
extern const struct field	rmapbt_rec_flds[];

extern const struct field	refcbt_crc_flds[];
extern const struct field	refcbt_crc_hfld[];
extern const struct field	refcbt_key_flds[];
extern const struct field	refcbt_rec_flds[];

extern int	btblock_size(void *obj, int startoff, int idx);
