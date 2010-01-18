/* boards_list.c
 * Linux CAN-bus device driver.
 * Written for new CAN driver version by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"

extern int template_register(struct hwspecops_t *hwspecops);
extern int virtual_register(struct hwspecops_t *hwspecops);
extern int oscar_register(struct hwspecops_t *hwspecops);
extern int pip_register(struct hwspecops_t *hwspecops);
extern int m437_register(struct hwspecops_t *hwspecops);
extern int smartcan_register(struct hwspecops_t *hwspecops);
extern int pccanf_register(struct hwspecops_t *hwspecops);
extern int pccand_register(struct hwspecops_t *hwspecops);
extern int pccanq_register(struct hwspecops_t *hwspecops);
extern int kv_pcican_register(struct hwspecops_t *hwspecops);
extern int ems_cpcpci_register(struct hwspecops_t *hwspecops);
extern int nsi_register(struct hwspecops_t *hwspecops);
extern int cc104_register(struct hwspecops_t *hwspecops);
extern int pci03_register(struct hwspecops_t *hwspecops);
extern int pcm3680_register(struct hwspecops_t *hwspecops);
extern int aim104_register(struct hwspecops_t *hwspecops);
extern int pcccan_register(struct hwspecops_t *hwspecops);
extern int ssv_register(struct hwspecops_t *hwspecops);
extern int bfadcan_register(struct hwspecops_t *hwspecops);
extern int pikronisa_register(struct hwspecops_t *hwspecops);
extern int gensja1000io_register(struct hwspecops_t *hwspecops);
extern int pimx1_register(struct hwspecops_t *hwspecops);
extern int msmcan_register(struct hwspecops_t *hwspecops);
extern int unican_register(struct hwspecops_t *hwspecops);
extern int unican_pci_register(struct hwspecops_t *hwspecops);
extern int unican_vme_register(struct hwspecops_t *hwspecops);
extern int ipci165_register(struct hwspecops_t *hwspecops);
extern int pcan_dongle_register(struct hwspecops_t *hwspecops);
extern int eb8245_register(struct hwspecops_t *hwspecops);
extern int adlink7841_register(struct hwspecops_t *hwspecops);
extern int tscan1_register(struct hwspecops_t *hwspecops);
extern int ts7kv_register(struct hwspecops_t *hwspecops);
extern int ns_dev_register(struct hwspecops_t *hwspecops);
extern int hms30c7202_register(struct hwspecops_t *hwspecops);
extern int nsi_canpci_register(struct hwspecops_t *hwspecops);
extern int pcan_pci_register(struct hwspecops_t *hwspecops);
extern int esdpci200_register(struct hwspecops_t *hwspecops);
extern int sh7760_register(struct hwspecops_t *hwspecops);
extern int vscan_register(struct hwspecops_t *hwspecops);

const struct boardtype_t can_boardtypes[]={
    #ifdef CONFIG_OC_LINCAN_CARD_template
	{"template", template_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_virtual
	{"virtual", virtual_register, 0},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_oscar
	{"oscar", oscar_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pip
	{"pip5", pip_register, 1},
	{"pip6", pip_register, 1},
	{"pip7", pip_register, 1},
	{"pip8", pip_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_smartcan
	{"smartcan", smartcan_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_nsi
	{"nsican", nsi_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_cc_can104
	{"cc104", cc104_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_aim104
	{"aim104", aim104_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pc_i03
	{"pc-i03", pci03_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pcm3680
	{"pcm3680", pcm3680_register, 2},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pccan
	{"pccan-f", pccanf_register, 1},
	{"pccan-s", pccanf_register, 1},
	{"pccan-d", pccand_register, 2},
	{"pccan-q", pccanq_register, 4},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_kv_pcican)&&defined(CAN_ENABLE_PCI_SUPPORT)
	{"pcican-s", kv_pcican_register, 0},
	{"pcican-d", kv_pcican_register, 0},
	{"pcican-q", kv_pcican_register, 0},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_ems_cpcpci)&&defined(CAN_ENABLE_PCI_SUPPORT)
	{"ems_cpcpci", ems_cpcpci_register, 0},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_m437
	{"m437", m437_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pcccan
	{"pcccan", pcccan_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_ssv
	{"ssv",	ssv_register, 2},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_bfadcan
	{"bfadcan", bfadcan_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pikronisa
	{"pikronisa", pikronisa_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_gensja1000io
	{"gensja1000io", gensja1000io_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_pimx1
	{"pimx1", pimx1_register, 0},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_msmcan
	{"msmcan", msmcan_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_unican
	{"unican", unican_register, 1},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_ipci165
	{"ipci165", ipci165_register, 0},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_unican)&&defined(CAN_ENABLE_PCI_SUPPORT)
	{"unican-pci", unican_pci_register, 0},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_unican)&&defined(CAN_ENABLE_VME_SUPPORT)
	{"unican-vme", unican_vme_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_pcan_dongle)
	{"pcan_dongle", pcan_dongle_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_eb8245)
	{"eb8245", eb8245_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_adlink7841)
	{"adlink7841", adlink7841_register, 0},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_esdpci200)
	{"esdpci200", esdpci200_register, 0},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_tscan1)
	{"tscan1", tscan1_register, 1},
	{"ts7kv",  ts7kv_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_ns_dev_can)
	{"ns_dev", ns_dev_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_sh7760)
	{"sh7760", sh7760_register, 2},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_hms30c7202_can)
	{"hms30c7202", hms30c7202_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_nsi_canpci)&&defined(CAN_ENABLE_PCI_SUPPORT)
	{"nsicanpci", nsi_canpci_register, 1},
    #endif
    #if defined(CONFIG_OC_LINCAN_CARD_pcan_pci)&&defined(CAN_ENABLE_PCI_SUPPORT)
	{"pcan_pci", pcan_pci_register, 0},
    #endif
    #ifdef CONFIG_OC_LINCAN_CARD_vscan
	{"vscan", vscan_register, 1},
    #endif
	{NULL}
};

const struct boardtype_t* boardtype_find(const char *str)
{
	const struct boardtype_t *brp;

	for(brp=can_boardtypes;brp->boardtype;brp++) {
		if(!strcmp(str,brp->boardtype))
			return brp;
	}

	return NULL;
}
