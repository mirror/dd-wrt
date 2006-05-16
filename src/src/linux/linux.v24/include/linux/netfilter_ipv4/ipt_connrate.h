#ifndef _IPT_CONNRATE_H
#define _IPT_CONNRATE_H

struct ipt_connrate_info
{
	/* Per connection transfer rate, in bytes per second. If
	   'from' is smaller or equal to 'to', rate is matched to be
	   inside the inclusive range [from,to], otherwise rate is
	   matched to be outside the inclusive range [to,from]. */
	u_int32_t from, to;
};
#endif
