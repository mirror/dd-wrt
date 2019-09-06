#ifndef __BACKPORT_THERMAL_H__
#define __BACKPORT_THERMAL_H__
#include_next <linux/thermal.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
#define thermal_zone_device_register(type, trips, mask, devdata, ops, tzp, passive_delay, polling_delay) \
	thermal_zone_device_register(type, trips, devdata, ops, 0, 0, passive_delay, polling_delay)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
#define thermal_zone_device_register(type, trips, mask, devdata, ops, tzp, passive_delay, polling_delay) \
	thermal_zone_device_register(type, trips, mask, devdata, ops, 0, 0, passive_delay, polling_delay)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
#define thermal_zone_device_register(type, trips, mask, devdata, ops, tzp, passive_delay, polling_delay) \
	thermal_zone_device_register(type, trips, mask, devdata, ops, passive_delay, polling_delay)
#endif /* < 3.8 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
#define thermal_notify_framework LINUX_BACKPORT(thermal_notify_framework)
static inline void thermal_notify_framework(struct thermal_zone_device *tz, int trip)
{
       thermal_zone_device_update(tz);
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define thermal_notify_framework(tz, trip) notify_thermal_framework(tz, trip)
#endif /* < 3.10 */

#endif /* __BACKPORT_THERMAL_H__ */
