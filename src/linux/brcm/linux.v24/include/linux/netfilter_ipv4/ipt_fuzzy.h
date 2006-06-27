#ifndef _IPT_FUZZY_H
#define _IPT_FUZZY_H

#include <linux/param.h>
#include <linux/types.h>

#define MAXFUZZYRATE 10000000
#define MINFUZZYRATE 3

struct ipt_fuzzy_info {
	u_int32_t minimum_rate;
	u_int32_t maximum_rate;
	u_int32_t packets_total;
	u_int32_t bytes_total;
	u_int32_t previous_time;
	u_int32_t present_time;
	u_int32_t mean_rate;
	u_int8_t acceptance_rate;
};

#endif /*_IPT_FUZZY_H*/
