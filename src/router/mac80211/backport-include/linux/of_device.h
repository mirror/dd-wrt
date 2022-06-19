#ifndef __BP_OF_DEVICE_H
#define __BP_OF_DEVICE_H
#include_next <linux/of_device.h>
#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(3,8,0)
#define of_property_read_u8_array LINUX_BACKPORT(of_property_read_u8_array)
#ifdef CONFIG_OF
extern int of_property_read_u8_array(const struct device_node *np,
			const char *propname, u8 *out_values, size_t sz);
#else
static inline int of_property_read_u8_array(const struct device_node *np,
			const char *propname, u8 *out_values, size_t sz)
{
	return -ENOSYS;
}
#endif /* CONFIG_OF */
#endif /* LINUX_VERSION_IS_LESS(3,8,0) */

#if LINUX_VERSION_IS_LESS(4,18,0)
static inline int backport_of_dma_configure(struct device *dev,
					    struct device_node *np,
					    bool force_dma)
{
#if LINUX_VERSION_IS_GEQ(4,15,0)
	dev->bus->force_dma = force_dma;
	return of_dma_configure(dev, np);
#elif LINUX_VERSION_IS_GEQ(4,12,0)
	return of_dma_configure(dev, np);
#else
	of_dma_configure(dev, np);
	return 0;
#endif
}
#define of_dma_configure LINUX_BACKPORT(of_dma_configure)
#endif /* < 4.18 */

#endif /* __BP_OF_DEVICE_H */
