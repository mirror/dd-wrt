#ifndef _IP6T_NTH_H
#define _IP6T_NTH_H

#include <linux/param.h>
#include <linux/types.h>

#ifndef IP6T_NTH_NUM_COUNTERS
#define IP6T_NTH_NUM_COUNTERS 16
#endif

struct ip6t_nth_info {
	u_int8_t every;
	u_int8_t not;
	u_int8_t startat;
	u_int8_t counter;
	u_int8_t packet;
};

#endif /*_IP6T_NTH_H*/
