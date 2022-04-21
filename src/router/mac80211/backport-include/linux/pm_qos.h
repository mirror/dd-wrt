#ifndef _COMPAT_LINUX_PM_QOS_H
#define _COMPAT_LINUX_PM_QOS_H 1

#include <linux/version.h>
#include_next <linux/pm_qos.h>

#if LINUX_VERSION_IS_LESS(5,7,0)
static inline void cpu_latency_qos_add_request(struct pm_qos_request *req,
                                              s32 value)
{
	pm_qos_add_request(req, PM_QOS_CPU_DMA_LATENCY, value);
}

static inline void cpu_latency_qos_update_request(struct pm_qos_request *req,
                                                 s32 new_value)
{
	pm_qos_update_request(req, new_value);
}

static inline void cpu_latency_qos_remove_request(struct pm_qos_request *req)
{
	pm_qos_remove_request(req);
}

static inline bool cpu_latency_qos_request_active(struct pm_qos_request *req)
{
	return pm_qos_request_active(req);
}

static inline s32 cpu_latency_qos_limit(void)
{
	return pm_qos_request(PM_QOS_CPU_DMA_LATENCY);
}
#endif /* < 5.7 */

#endif	/* _COMPAT_LINUX_PM_QOS_H */
