/* 
 * raid6cns.c
 *
 * CNS3xxx xor & gen_syndrome functions
 *
 */

#ifdef	CONFIG_CNS3XXX_RAID

#include <linux/raid/pq.h>

extern void do_cns_rdma_gfgen(unsigned int src_no, unsigned int bytes, void **bh_ptr, 
				void *p_dst, void *q_dst);

/**
 * raid6_cnsraid_gen_syndrome - CNSRAID Syndrome Generate
 *
 * @disks: raid disks
 * @bytes: length
 * @ptrs:  already arranged stripe ptrs, 
 *         disk0=[0], diskNNN=[disks-3], 
 *         P/Q=[z0+1] & [z0+2], or, [disks-2], [disks-1]
 */
static void raid6_cnsraid_gen_syndrome(int disks, size_t bytes, void **ptrs)
{
	do_cns_rdma_gfgen(disks - 2, bytes, ptrs, ptrs[disks-2], ptrs[disks-1]);
}

const struct raid6_calls raid6_cns_raid = {
	raid6_cnsraid_gen_syndrome,	/* callback */
	NULL,						/* always valid */
	"CNS-RAID",					/* name */
	1							/* preferred: revise it to "0" to compare/compete with others algos */
};

EXPORT_SYMBOL(raid6_cns_raid);

#endif /* CONFIG_CNS3XXX_RAID */
