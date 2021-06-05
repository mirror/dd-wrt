#!/bin/sh
#
# Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.      
#       
# Permission to use, copy, modify, and/or distribute this software for any      
# purpose with or without fee is hereby granted, provided that the above      
# copyright notice and this permission notice appear in all copies.      
#       
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES      
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF      
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY      
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES      
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION      
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN      
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.      
#
# $Id: shared_ksyms.sh,v 1.2 2008-12-05 20:10:41 $
#

cat <<EOF
#include <linux/version.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#include <linux/config.h>
#endif
#include <typedefs.h>
#include <hndsoc.h>
#include <siutils.h>
#include "../../../brcm/arm/shared/siutils_priv.h"
#include <bcmutils.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <bcmotp.h>
#include <bcmsrom.h>
#include <bcmrobo.h>
#include <sbchipc.h>
#include <nicpci.h>
#include <pcie_core.h>
extern uint32 otp_msg_level;
int dbushost_initvars_flash(si_t *sih, osl_t *osh, char **base, uint len);
extern uint8 patch_pair;
extern bcm_rxcplid_list_t *g_rxcplid_list;
extern uint32 pktpools_max;
extern const di_fcn_t dma64proc;
typedef struct cubkout2vreg {
	uint16 cbuck_mv;
	int8  vreg_val;
} cubkout2vreg_t;

extern cubkout2vreg_t BCMATTACHDATA(cbuck2vreg_tbl)[];
extern void *g_si_pmutmr_lock_arg;
extern si_pmu_callback_t g_si_pmutmr_lock_cb;
extern si_pmu_callback_t g_si_pmutmr_unlock_cb;
extern void si_pmu_otp_pllcontrol(si_t *sih, osl_t *osh);
extern void si_pmu_otp_regcontrol(si_t *sih, osl_t *osh);
extern void si_pmu_otp_chipcontrol(si_t *sih, osl_t *osh);
extern uint32 si_pmu_def_alp_clock(si_t *sih, osl_t *osh);
extern bool si_pmu_update_pllcontrol(si_t *sih, osl_t *osh, uint32 xtal, bool update_required);
extern int si_pmu_wait_for_steady_state(si_t *sih, osl_t *osh, chipcregs_t *cc);
extern uint32 si_pmu_get_pmutime_diff(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 *prev);
extern bool si_pmu_wait_for_res_pending(si_t *sih, osl_t *osh, chipcregs_t *cc, uint usec,
	bool cond, uint32 *elapsed_time);
extern uint32 si_pmu_get_pmutimer(si_t *sih, osl_t *osh, pmuregs_t *pmu);

extern void set_hc595_core(si_t *sih);
extern void set_hc595_reset(void);
extern void set_hc595(uint32 pin, uint32 value);
extern int do_4360_pcie2_war;
extern void si_gci_chipctrl_overrides(osl_t *osh, si_t *sih, char *pvars);
extern uint8 arl_entry_g[];
struct b53_port;
struct b53_vlan;
extern int bcm_robo_config_vlan_fun(void *b53_robo, struct b53_vlan *vlans, int vlans_num, struct b53_port *b53_ports);
extern int bcm_robo_global_config(void *robo, struct b53_vlan *vlans, int vlans_num, struct b53_port *ports);
extern void bcm_robo_reset_switch(void *b53_robo);
extern bool bcm_robo_port_link(unsigned int port);
extern pdesc_t pdesc25[];
extern pdesc_t pdesc97[];

EOF

for file in $* ; do
    ${CROSS_COMPILE}gcc-nm $file | sed -ne 's/[0-9A-Fa-f]* [BDRT] \([^ ]*\)/EXPORT_SYMBOL(\1);/p'
done
