/*
<:copyright-BRCM:2015:GPL/GPL:standard

   Copyright (c) 2015 Broadcom
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpuidle.h>
#include <linux/export.h>
#include <asm/cpuidle.h>

#include <bcm_map_part.h>
#include "board.h"

// available cpu idle state indices
// (see bcm_arm_idle_driver[] below)
enum bcm_cpuidle_states {
	bcs_wfi_ddr,
	bcs_last
};

static void set_bcm_cpuidle_enter(void);

// Enable WFI by default to get the same behavior as when cpuidle is compiled out
static unsigned int wfi_enabled = 1;

void set_cpu_arm_wait(int enable)
{
	wfi_enabled = enable;
	set_bcm_cpuidle_enter();
	printk("wait instruction: %sabled\n", enable ? "en" : "dis");
}
EXPORT_SYMBOL(set_cpu_arm_wait);

int get_cpu_arm_wait(void)
{
	return (wfi_enabled);
}
EXPORT_SYMBOL(get_cpu_arm_wait);

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
static unsigned int self_refresh_enabled = 0;
static PWRMNGT_DDR_SR_CTRL ddrSrCtrl = {.word = 0};
static volatile PWRMNGT_DDR_SR_CTRL *pDdrSrCtrl = &ddrSrCtrl;

void BcmPwrMngtSetDRAMSelfRefresh(unsigned int enable)
{
	self_refresh_enabled = enable;
	set_bcm_cpuidle_enter();
	printk("DDR Self Refresh pwrsaving is %sabled\n", self_refresh_enabled ? "en" : "dis");
}
EXPORT_SYMBOL(BcmPwrMngtSetDRAMSelfRefresh);

int BcmPwrMngtGetDRAMSelfRefresh(void)
{
	return (self_refresh_enabled);
}
EXPORT_SYMBOL(BcmPwrMngtGetDRAMSelfRefresh);

void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr)
{
	int cpu;

	// Initialize to busy status
	if (NULL != pDdrSr) {
		pDdrSrCtrl = pDdrSr;
	}
	else {
		pDdrSrCtrl = &ddrSrCtrl;
	}

	pDdrSrCtrl->word = 0;
	for_each_possible_cpu(cpu) {
		pDdrSrCtrl->host |= 1<<cpu;
	}
}
EXPORT_SYMBOL(BcmPwrMngtRegisterLmemAddr);

static int bcm_arm_cpuidle_wfi_sr_enter(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index)
{
	// Assume cpu does not change through this function
	const int cpu_mask = 1 << raw_smp_processor_id();

	// On 63138, it was found that accessing the memory assigned by the DSL
	// driver to pDdrSrCtrl is very slow. A local shadow copy ddrSrCtrl is
	// first updated and used before deciding to access pDdrSrCtrl.
	// On non-DSL chips, the shadow and the pointer are referring to the same memory
	ddrSrCtrl.host &= ~cpu_mask;

	dsb(ish);
	if (!ddrSrCtrl.host) {
		// Let the PHY MIPS know that all cores on the host will be executing wfi
		pDdrSrCtrl->host = 0;
		// Ensure the PHY MIPS is not active so we can enter SR
		if (!pDdrSrCtrl->phy) {
			uint32 dram_cfg = MEMC->CHN_TIM_DRAM_CFG | DRAM_CFG_DRAMSLEEP;
			MEMC->CHN_TIM_DRAM_CFG = dram_cfg;
		}
	}
	wfi();
	// The first CPU to come out of WFI needs to check and set its mask bit
	if (!ddrSrCtrl.host) {
		pDdrSrCtrl->host |= cpu_mask;
	}
	ddrSrCtrl.host |= cpu_mask;
	return index;
}
#endif

static int bcm_arm_cpuidle_wfi_enter(struct cpuidle_device *dev,
		struct cpuidle_driver *drv, int index)
{
	cpu_do_idle();
	return index;
}

static int bcm_arm_cpuidle_nop_enter(struct cpuidle_device *dev,
		struct cpuidle_driver *drv, int index)
{
	return index;
}

static struct cpuidle_driver bcm_arm_idle_driver = {
	.name			= "bcm_idle",
	.owner			= THIS_MODULE,
	.states[bcs_wfi_ddr]		= {
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
		// .enter is changed to bcm_arm_cpuidle_wfi_sr_enter later when sr is enabled
		.enter			= bcm_arm_cpuidle_wfi_enter,
		.name			= "WFI_DDR_SR",
		.desc			= "WFI & DDR Self-Refresh",
		.power_usage		= 900,
#else
		.enter			= bcm_arm_cpuidle_wfi_enter,
		.name			= "WFI",
		.desc			= "Wait For Interrupt",
		.power_usage		= 1000,
#endif
		.exit_latency		= 0,
		.target_residency	= 0,
	},

	.state_count = bcs_last,
};

// This function assumes that writing the enter function pointer is atomic
static void set_bcm_cpuidle_enter(void)
{
	if (wfi_enabled) {
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
		if (self_refresh_enabled) {
			bcm_arm_idle_driver.states[bcs_wfi_ddr].enter = bcm_arm_cpuidle_wfi_sr_enter;
		}
		else
#endif
		{
			bcm_arm_idle_driver.states[bcs_wfi_ddr].enter = bcm_arm_cpuidle_wfi_enter;			
		}
	}
	else {
		bcm_arm_idle_driver.states[bcs_wfi_ddr].enter = bcm_arm_cpuidle_nop_enter;			
	}
}

static int __init bcm_arm_cpuidle_init(void)
{
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
	MEMC->PhyControl.IDLE_PAD_CONTROL = 0xe;
	MEMC->PhyControl.IDLE_PAD_EN0 = 0x2df;
	MEMC->PhyControl.IDLE_PAD_EN1 = 0x3fffff;
	MEMC->PhyByteLane0Control.IDLE_PAD_CTRL = 0xfffe;
	MEMC->PhyByteLane1Control.IDLE_PAD_CTRL = 0xfffe;
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM968360)
	// For future use, Self-Refresh is not currently released on these platforms
	MEMC->PhyByteLane2Control.IDLE_PAD_CTRL = 0xfffe;
	MEMC->PhyByteLane3Control.IDLE_PAD_CTRL = 0xfffe;
#endif
#endif
	return cpuidle_register(&bcm_arm_idle_driver, NULL);
}
late_initcall(bcm_arm_cpuidle_init);
