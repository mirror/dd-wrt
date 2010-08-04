/* -*- linux-c -*- ------------------------------------------------------- *
 *
 *   Copyright 2002 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * raid6recov.c
 *
 * RAID-6 data recovery in dual failure mode.  In single failure mode,
 * use the RAID-5 algorithm (or, in the case of Q failure, just reconstruct
 * the syndrome.)
 */

#include <linux/raid/pq.h>

#ifdef CONFIG_CNS3XXX_RAID
#define        R6_RECOV_PD             1
#define        R6_RECOV_DD             2
#define        R6_RECOV_DQ             3
extern void do_cns_rdma_gfgen_pd_dd_dq(unsigned int src_no, unsigned int bytes,
				       void **bh_ptr, void *w1_dst,
				       void *w2_dst, int pd_dd_qd,
				       unsigned int w1_idx, unsigned int w2_idx,
				       unsigned int *src_idx);

/**
 * @disks: nr_disks
 * @bytes: len
 * @faila: 1st failed DD
 * @ptrs:  ptrs by order {d0, d1, ..., da, ..., dn, P, Q}
 *
 * Desc:
 *     new_read_ptrs = {d0, d1, ... dn, Q}
 *     dd1 = faila
 *     p_dst = P
 */
void raid6_datap_recov(int disks, size_t bytes, int faila, void **ptrs)
{
	int cnt = 0;
	int count = 0;
	void *p_dst, *q;
	void *dd1_dst;
	void *new_read_ptrs[disks - 2];
	unsigned int read_idx[disks - 2];

	q = ptrs[disks - 1];
	p_dst = ptrs[disks - 2];
	dd1_dst = ptrs[faila];

	while (cnt < disks) {
		if (cnt != faila && cnt != disks - 2) {
			new_read_ptrs[count] = ptrs[cnt];
			read_idx[count] = cnt;
			count++;
		}
		cnt++;
	}

	do_cns_rdma_gfgen_pd_dd_dq(disks - 2, bytes,
				   new_read_ptrs, p_dst, dd1_dst,
				   R6_RECOV_PD, disks - 1, faila + 1, read_idx);
}

/**
 * @disks: nr_disks
 * @bytes: len
 * @faila: 1st failed DD
 * @failb: 2nd failed DD
 * @ptrs:  ptrs by order {d0, d1, ..., da, ..., db, ..., dn, P, Q}
 *
 * Desc:
 *     new_read_ptrs = {d0, d1, ... dn, P, Q}
 *     dd1_dst = faila
 *     dd2_dst = failb
 */
void raid6_2data_recov(int disks, size_t bytes, int faila, int failb,
		       void **ptrs)
{

	int cnt = 0;
	int count = 0;
	void *p, *q;
	void *dd1_dst, *dd2_dst;
	void *new_read_ptrs[disks - 2];
	unsigned int read_idx[disks - 2];

	q = ptrs[disks - 1];
	p = ptrs[disks - 2];
	dd1_dst = ptrs[faila];
	dd2_dst = ptrs[failb];

	while (cnt < disks) {
		if (cnt != faila && cnt != failb) {
			new_read_ptrs[count] = ptrs[cnt];
			read_idx[count] = cnt;
			count++;
		}
		cnt++;
	}

	do_cns_rdma_gfgen_pd_dd_dq(disks - 2, bytes,
				   new_read_ptrs, dd1_dst, dd2_dst,
				   R6_RECOV_DD, faila + 1, failb + 1, read_idx);
}

/**
 * @disks: nr_disks
 * @bytes: len
 * @faila: 1st failed DD
 * @ptrs:  ptrs by order {d0, d1, ..., da, ..., dn, P, Q}
 *
 * Desc:
 *     new_read_ptrs = {d0, d1, ... dn, P}
 *     dd1 = faila
 *     q_dst = Q
 */
void raid6_dataq_recov(int disks, size_t bytes, int faila, void **ptrs)
{
	int cnt = 0;
	int count = 0;
	void *q_dst, *p;
	void *dd1_dst;
	void *new_read_ptrs[disks - 2];
	unsigned int read_idx[disks - 2];

	p = ptrs[disks - 2];
	q_dst = ptrs[disks - 1];
	dd1_dst = ptrs[faila];

	while (cnt < disks) {
		if (cnt != faila && cnt != disks - 1) {
			new_read_ptrs[count] = ptrs[cnt];
			read_idx[count] = cnt;
			count++;
		}
		cnt++;
	}

	do_cns_rdma_gfgen_pd_dd_dq(disks - 2, bytes,
				   new_read_ptrs, dd1_dst, q_dst,
				   R6_RECOV_DQ, faila + 1, disks, read_idx);
}

#else /* CONFIG_CNS3XXX_RAID

/* Recover two failed data blocks. */
void raid6_2data_recov(int disks, size_t bytes, int faila, int failb,
		       void **ptrs)
{
	u8 *p, *q, *dp, *dq;
	u8 px, qx, db;
	const u8 *pbmul;	/* P multiplier table for B data */
	const u8 *qmul;		/* Q multiplier table (for both) */

	p = (u8 *)ptrs[disks-2];
	q = (u8 *)ptrs[disks-1];

	/* Compute syndrome with zero for the missing data pages
	   Use the dead data pages as temporary storage for
	   delta p and delta q */
	dp = (u8 *)ptrs[faila];
	ptrs[faila] = (void *)raid6_empty_zero_page;
	ptrs[disks-2] = dp;
	dq = (u8 *)ptrs[failb];
	ptrs[failb] = (void *)raid6_empty_zero_page;
	ptrs[disks-1] = dq;

	raid6_call.gen_syndrome(disks, bytes, ptrs);

	/* Restore pointer table */
	ptrs[faila]   = dp;
	ptrs[failb]   = dq;
	ptrs[disks-2] = p;
	ptrs[disks-1] = q;

	/* Now, pick the proper data tables */
	pbmul = raid6_gfmul[raid6_gfexi[failb-faila]];
	qmul  = raid6_gfmul[raid6_gfinv[raid6_gfexp[faila]^raid6_gfexp[failb]]];

	/* Now do it... */
	while ( bytes-- ) {
		px    = *p ^ *dp;
		qx    = qmul[*q ^ *dq];
		*dq++ = db = pbmul[px] ^ qx; /* Reconstructed B */
		*dp++ = db ^ px; /* Reconstructed A */
		p++; q++;
	}
}
EXPORT_SYMBOL_GPL(raid6_2data_recov);

/* Recover failure of one data block plus the P block */
void raid6_datap_recov(int disks, size_t bytes, int faila, void **ptrs)
{
	u8 *p, *q, *dq;
	const u8 *qmul;		/* Q multiplier table */

	p = (u8 *)ptrs[disks-2];
	q = (u8 *)ptrs[disks-1];

	/* Compute syndrome with zero for the missing data page
	   Use the dead data page as temporary storage for delta q */
	dq = (u8 *)ptrs[faila];
	ptrs[faila] = (void *)raid6_empty_zero_page;
	ptrs[disks-1] = dq;

	raid6_call.gen_syndrome(disks, bytes, ptrs);

	/* Restore pointer table */
	ptrs[faila]   = dq;
	ptrs[disks-1] = q;

	/* Now, pick the proper data tables */
	qmul  = raid6_gfmul[raid6_gfinv[raid6_gfexp[faila]]];

	/* Now do it... */
	while ( bytes-- ) {
		*p++ ^= *dq = qmul[*q ^ *dq];
		q++; dq++;
	}
}
EXPORT_SYMBOL_GPL(raid6_datap_recov);
#endif /* CONFIG_CNS3XXX_RAID */

#ifndef __KERNEL__
/* Testing only */

/* Recover two failed blocks. */
void raid6_dual_recov(int disks, size_t bytes, int faila, int failb, void **ptrs)
{
	if ( faila > failb ) {
		int tmp = faila;
		faila = failb;
		failb = tmp;
	}

	if ( failb == disks-1 ) {
		if ( faila == disks-2 ) {
			/* P+Q failure.  Just rebuild the syndrome. */
			raid6_call.gen_syndrome(disks, bytes, ptrs);
		} else {
			/* data+Q failure.  Reconstruct data from P,
			   then rebuild syndrome. */
			/* NOT IMPLEMENTED - equivalent to RAID-5 */
		}
	} else {
		if ( failb == disks-2 ) {
			/* data+P failure. */
			raid6_datap_recov(disks, bytes, faila, ptrs);
		} else {
			/* data+data failure. */
			raid6_2data_recov(disks, bytes, faila, failb, ptrs);
		}
	}
}

#endif
