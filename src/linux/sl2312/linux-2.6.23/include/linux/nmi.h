/*
 *  linux/include/linux/nmi.h
 */
#ifndef LINUX_NMI_H
#define LINUX_NMI_H

#include <linux/sched.h>
#include <asm/irq.h>
#include <linux/io.h>
#include <asm/hardware.h>

/**
 * touch_nmi_watchdog - restart NMI watchdog timeout.
 * 
 * If the architecture supports the NMI watchdog, touch_nmi_watchdog()
 * may be used to reset the timeout - for code which intentionally
 * disables interrupts for a long time. This call is stateless.
 */
#ifdef ARCH_HAS_NMI_WATCHDOG
#include <asm/nmi.h>
extern void touch_nmi_watchdog(void);
extern void acpi_nmi_disable(void);
extern void acpi_nmi_enable(void);
#else
static inline void touch_nmi_watchdog(void)
{
#ifdef CONFIG_MACH_WBD222
        int val;

        val = __raw_readl(IO_ADDRESS(GEMINI_GPIO_BASE(0)) + 0x00);
        if (val & 1 << 18)
                val &= ~(1 << 18);
        else
                val |= 1 << 18;

        __raw_writel(val, IO_ADDRESS(GEMINI_GPIO_BASE(0)) + 0x00);
#endif
	touch_softlockup_watchdog();
}
static inline void acpi_nmi_disable(void) { }
static inline void acpi_nmi_enable(void) { }
#endif

#ifndef trigger_all_cpu_backtrace
#define trigger_all_cpu_backtrace() do { } while (0)
#endif

#endif
