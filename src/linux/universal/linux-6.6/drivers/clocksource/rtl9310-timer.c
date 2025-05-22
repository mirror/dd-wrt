#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/delay.h>

#include <asm/time.h>
#include <asm/string.h>

#include "mach-rtl83xx.h"

#define TIMER_NAME_SIZE 15

#define RTL9310_LXR_MHZ 200
#define DIVISOR_RTL9310 100
#define RTL9310_TC_BASE 0xB8003200
#define RTL9310_TC_REG_OFFSET (0x10)
#define RTL9310MP_TC0DATA (RTL9310_TC_BASE)
#define RTL9310MP_TC0CTL (RTL9310_TC_BASE + 0x08)
#define RTL9310MP_TC0INT (RTL9310_TC_BASE + 0xC)

#define RTL9310MP_TCEN (1 << 28)
#define RTL9310MP_TCMODE_TIMER (1 << 24)
#define RTL9310MP_TCIP (1 << 16)
#define RTL9310MP_TCIE (1 << 20)

#ifndef reg32
#define reg32(reg) (*(volatile unsigned int *)(reg))
#endif
#define RTL9310_GET_INTR_ID0 0

int rtl9310_cpuExtTimerIRQ_get(u32 cpu, u32 *irq_num)
{
	struct device_node *dn;
	char name[32];
	u32 v;

	if (irq_num == NULL)
		return -1;

	dn = of_find_compatible_node(NULL, NULL, name);
	if (!dn) {
		sw_w32(1, RTL931X_RST_GLB_CTRL);
		v = sw_r32(RTL931X_RST_GLB_CTRL);
		sw_w32(0x101, RTL931X_RST_GLB_CTRL);
		msleep(15);
		sw_w32(v, RTL931X_RST_GLB_CTRL);
		msleep(15);
		sw_w32(0x101, RTL931X_RST_GLB_CTRL);
	}
	*irq_num = irq_of_parse_and_map(dn, 0);
	return 0;
}

int rtl9310_cpuExtTimerID_get(u32 cpu, u32 *timer_id)
{
	int cpu_num = 0;

#ifdef CONFIG_SMP
	cpu_num = 2;
#else
	cpu_num = 1;
#endif
	*timer_id = cpu + 2;

	return 0;
}

DEFINE_PER_CPU(struct clock_event_device, ext_clockevent_device);

static int ext_timer_set_next_event(unsigned long delta,
				    struct clock_event_device *evt)
{
	return -EINVAL;
}

static void ext_timer_event_handler(struct clock_event_device *dev)
{
}

void rtl9310_cpuExtTimer_ack(u32 cpu)
{
	u32 offset, timer_id, v;
	int ret;
	if (cpu >= 4)
		return;
	ret = rtl9310_cpuExtTimerID_get(cpu, &timer_id);

	offset = (timer_id * RTL9310_TC_REG_OFFSET);

	reg32(RTL9310MP_TC0INT + offset) |= RTL9310MP_TCIP;
}

irqreturn_t cevt_extTimer_ack(int irq, void *dev_id)
{
	struct clock_event_device *cd = this_cpu_ptr(&ext_clockevent_device);
	unsigned int cpu = smp_processor_id();

	rtl9310_cpuExtTimer_ack(cpu);
	cd->event_handler(cd);

	return IRQ_HANDLED;
}

void rtl9310_cpuExtTimer_init(void)
{
	int cpu_idx, timer_id, offset, cpu_num;
	int TCDATA = 0;
	int TCCTL = 0;
	int ret;

#ifdef CONFIG_SMP
	cpu_num = 2;
#else
	cpu_num = 1;
#endif

	TCDATA = ((RTL9310_LXR_MHZ * 1000000) / ((int)DIVISOR_RTL9310 * HZ));

	for (cpu_idx = 0; cpu_idx < cpu_num; cpu_idx++) {
		ret = rtl9310_cpuExtTimerID_get(cpu_idx, &timer_id);
		if (ret) {
			return;
		}
		offset = (timer_id * RTL9310_TC_REG_OFFSET);
		if (reg32(RTL9310MP_TC0INT + offset) & RTL9310MP_TCIP) {
			reg32(RTL9310MP_TC0INT + offset) |= RTL9310MP_TCIP;
		}
		/* disable timer before setting CDBR */
		reg32(RTL9310MP_TC0CTL + offset) =
			0; /* disable timer before setting CDBR */
		reg32(RTL9310MP_TC0DATA + offset) = TCDATA;
	}

	/* Enable timer for all CPU at one time. Let the count of all timer is near */
	TCCTL = RTL9310MP_TCEN | RTL9310MP_TCMODE_TIMER | DIVISOR_RTL9310;

	for (cpu_idx = 0; cpu_idx < cpu_num; cpu_idx++) {
		ret = rtl9310_cpuExtTimerID_get(cpu_idx, &timer_id);
		if (ret) {
			return;
		}
		offset = (timer_id * RTL9310_TC_REG_OFFSET);
		reg32(RTL9310MP_TC0CTL + offset) = TCCTL;
	}
}

void rtl9310_cpuExtTimerIntr_enable(u32 cpu, bool enable)
{
	int ret;
	u32 timer_id;

	ret = rtl9310_cpuExtTimerID_get(cpu, &timer_id);

	if (enable)
		reg32((RTL9310MP_TC0INT) +
		      (timer_id * RTL9310_TC_REG_OFFSET)) |= RTL9310MP_TCIE;
	else
		reg32((RTL9310MP_TC0INT) +
		      (timer_id * RTL9310_TC_REG_OFFSET)) &= ~(RTL9310MP_TCIE);
}

int cevt_extTimerIntr_setup(int cpu)
{
	int ret;
	unsigned int tc_irq;
	char *timer_name;
	unsigned int timer_id;
	struct irqaction *ext_irqaction;
	u32 v;

	ext_irqaction = kmalloc(sizeof(struct irqaction), GFP_NOWAIT);
	if (ext_irqaction == NULL) {
		return -1;
	}
	memset(ext_irqaction, 0, sizeof(struct irqaction));

	timer_name = kmalloc(TIMER_NAME_SIZE, GFP_NOWAIT);
	if (timer_name == NULL) {
		return -1;
	}
	memset(timer_name, 0, TIMER_NAME_SIZE);

	ret = rtl9310_cpuExtTimerID_get(cpu, &timer_id);
	if (ret) {
		return -1;
	}

	ret = sprintf(timer_name, "timer_%d", timer_id);
	if (ret == 0) {
		return -1;
	}

	ret = rtl9310_cpuExtTimerIRQ_get(cpu, &tc_irq);
	if (ret) {
		return -1;
	}

	ext_irqaction->handler = cevt_extTimer_ack;
	ext_irqaction->flags = IRQF_TIMER;
	ext_irqaction->name = timer_name;

	if (setup_irq(tc_irq, ext_irqaction)) {
		sw_w32(1, RTL931X_RST_GLB_CTRL);
		v = sw_r32(RTL931X_RST_GLB_CTRL);
		sw_w32(0x101, RTL931X_RST_GLB_CTRL);
		msleep(15);
		sw_w32(v, RTL931X_RST_GLB_CTRL);
		msleep(15);
		sw_w32(0x101, RTL931X_RST_GLB_CTRL);
		return ret;
	}

	return 0;
}

int cevt_clockevent_init(struct clock_event_device *cd)
{
	unsigned int cpu = smp_processor_id();
	int ret = 0;
	char *clockevent_name;
	unsigned int tc_irq;
	clockevent_name = kmalloc(TIMER_NAME_SIZE, GFP_NOWAIT);
	if (clockevent_name == NULL) {
		return -1;
	}
	memset(clockevent_name, 0, TIMER_NAME_SIZE);
	ret = sprintf(clockevent_name, "timer_%d", cpu);
	if (ret == 0) {
		return -1;
	}

	ret = rtl9310_cpuExtTimerIRQ_get(cpu, &tc_irq);
	if (ret) {
		return -1;
	}

	cd->name = clockevent_name;
	cd->features = CLOCK_EVT_FEAT_PERIODIC;
	cd->event_handler = ext_timer_event_handler;
	cd->set_next_event = ext_timer_set_next_event;
	cd->rating = 100;
	cd->irq = tc_irq;
	cd->cpumask = cpumask_of(cpu);

	clockevents_register_device(cd);

	ret = cevt_extTimerIntr_setup(cpu);

	return ret;
}

static void ext_clockevent_exit(struct clock_event_device *cd)
{
	disable_irq(cd->irq);
}

static int ext_clockevent_init_notify(unsigned int cpu)
{
	int ret;
	unsigned int tc_irq;
	u32 v;
	/* init per cpu clockevent device*/
	ret = cevt_clockevent_init(this_cpu_ptr(&ext_clockevent_device));

	return 0;
}

static int ext_clockevent_exit_notify(unsigned int cpu)
{
	ext_clockevent_exit(this_cpu_ptr(&ext_clockevent_device));
	return 0;
}

static int __init rtk_cevt_timer_init(struct device_node *node)
{
	unsigned int cpu = smp_processor_id();
	int ret;

	rtl9310_cpuExtTimer_init();
	rtl9310_cpuExtTimerIntr_enable(cpu, true);

	ret = cpuhp_setup_state(CPUHP_AP_MIPS_GIC_TIMER_STARTING,
				"clockevents/mips/gic/timer:starting",
				ext_clockevent_init_notify,
				ext_clockevent_exit_notify);

	if (ret < 0)
		pr_info("[%s] Unable to register CPU notifier\n", __FUNCTION__);
	return 0;
}

TIMER_OF_DECLARE(ext_timer, "rtk,cevt-ext", rtk_cevt_timer_init);
