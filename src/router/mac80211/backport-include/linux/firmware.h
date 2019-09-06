#ifndef __BACKPORT_LINUX_FIRMWARE_H
#define __BACKPORT_LINUX_FIRMWARE_H
#include_next <linux/firmware.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
#define request_firmware_direct(fw, name, device) request_firmware(fw, name, device)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
#define firmware_request_nowarn LINUX_BACKPORT(firmware_request_nowarn)
int firmware_request_nowarn(const struct firmware **firmware, const char *name,
			    struct device *device);
#endif

#endif /* __BACKPORT_LINUX_FIRMWARE_H */
