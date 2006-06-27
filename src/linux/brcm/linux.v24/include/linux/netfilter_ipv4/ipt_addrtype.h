#ifndef _IPT_ADDRTYPE_H
#define _IPT_ADDRTYPE_H

struct ipt_addrtype_info {
	u_int16_t	source;		/* source-type mask */
	u_int16_t	dest;		/* dest-type mask */
	int		invert_source;
	int		invert_dest;
};

#endif
