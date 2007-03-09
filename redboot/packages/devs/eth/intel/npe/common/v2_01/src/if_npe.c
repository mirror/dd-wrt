//==========================================================================
//
//      if_npe.c
//
//	Intel NPE ethernet driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright 2002-2003 Intel Corporation All Rights Reserved.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2003-03-20
// Purpose:      
// Description:  hardware driver for Intel Network Processors.
// Notes:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>
#include <pkgconf/devs_eth_intel_npe.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <IxOsal.h>
#include <IxEthAcc.h>
#include <IxEthDB.h>
#include <IxNpeDl.h>
#include <IxQMgr.h>
#include <IxNpeMh.h>
#include <ix_ossl.h>
#include <IxFeatureCtrl.h>

#include <npe_info.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#include <redboot.h>
#include <flash_config.h>
#endif

#define NPE_NUM_PORTS 3

static IxQMgrDispatcherFuncPtr qDispatcherFunc;
static int npe_exists[NPE_NUM_PORTS];
static int npe_used[NPE_NUM_PORTS];
static BOOL phy_exists[IXP425_ETH_ACC_MII_MAX_ADDR];
static BOOL phy_scanned[IXP425_ETH_ACC_MII_MAX_ADDR];
static const char *npe_names[NPE_NUM_PORTS] = {
    "NPE-B", "NPE-C", "NPE-A"
};

// Convert IX_ETH_PORT_n to IX_NPEMH_NPEID_NPEx
static inline int __eth_to_npe(int eth_id)
{
    switch(eth_id) {
    case IX_ETH_PORT_1:	return IX_NPEMH_NPEID_NPEB;
    case IX_ETH_PORT_2:	return IX_NPEMH_NPEID_NPEC;
    case IX_ETH_PORT_3:	return IX_NPEMH_NPEID_NPEA;
    }
    return 0;
}

// Poll the CSR machinery.
static inline void __npe_poll(int eth_id)
{
    ixNpeMhMessagesReceive(__eth_to_npe(eth_id));
    (*qDispatcherFunc)(IX_QMGR_QUELOW_GROUP);
}

static inline int __npe_poll_timed(int eth_id, int *flagp, unsigned ms)
{
    do {
	__npe_poll(eth_id);
	if (ms) {
	    CYGACC_CALL_IF_DELAY_US(1000);
	    --ms;
	}
    } while (!*flagp && ms);

    return *flagp;
}

#define NPE_INT_BITS ((1 << CYGNUM_HAL_INTERRUPT_NPEB) | \
                      (1 << CYGNUM_HAL_INTERRUPT_NPEC) | \
                      (1 << CYGNUM_HAL_INTERRUPT_QM1))

// Can't use SMII without NPE-B
#if CYGINT_DEVS_ETH_INTEL_NPEB_SMII
#define CYGSEM_NPE_SMII
#else
#if CYGINT_DEVS_ETH_INTEL_NPEA_SMII
#error Cannot use SMII for NPE-A without SMII for NPE-B
#endif
#if CYGINT_DEVS_ETH_INTEL_NPEC_SMII
#error Cannot use SMII for NPE-C without SMII for NPE-B
#endif
#endif

static int  npe_csr_load(void);
static void npe_csr_unload(void);
static int  phy_present(int phyno);
static int  link_check(unsigned int phyNo);
#if CYGINT_DEVS_ETH_INTEL_NPE_PHY_DISCOVERY
static int  check_phy_association(int portno, int phyno);
static void set_phy_association(int portno, int phyno);
#endif
#ifdef CYGSEM_NPE_SMII
static int  npe_enable_smii_mode(cyg_uint32 mask);
static int  reset_npes(cyg_uint32 mask);
static void unreset_npes(cyg_uint32 mask);
#endif

#include CYGDAT_DEVS_ETH_INTEL_NPE_INL

#ifdef CYGSEM_INTEL_NPE_USE_ETH0
#define __NUM_ETH0_PORTS 1
static struct npe npe_eth0_priv_data = { 
    eth_id: CYGNUM_ETH0_ETH_ID,
    phy_no: CYGNUM_ETH0_PHY_NO,
#if defined(CYGDAT_ETH0_DEFAULT_ESA)
    mac_address: CYGDAT_ETH0_DEFAULT_ESA
#endif
};
#else
#define __NUM_ETH0_PORTS 0
#endif

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
#define __NUM_ETH1_PORTS 1
static struct npe npe_eth1_priv_data = { 
    eth_id: CYGNUM_ETH1_ETH_ID,
    phy_no: CYGNUM_ETH1_PHY_NO,
#if defined(CYGDAT_ETH1_DEFAULT_ESA)
    mac_address: CYGDAT_ETH1_DEFAULT_ESA
#endif
};
#else
#define __NUM_ETH1_PORTS 0
#endif

#ifdef CYGSEM_INTEL_NPE_USE_ETH2
#define __NUM_ETH2_PORTS 1
static struct npe npe_eth2_priv_data = { 
    eth_id: CYGNUM_ETH2_ETH_ID,
    phy_no: CYGNUM_ETH2_PHY_NO,
#if defined(CYGDAT_ETH2_DEFAULT_ESA)
    mac_address: CYGDAT_ETH2_DEFAULT_ESA
#endif
};
#else
#define __NUM_ETH2_PORTS 0
#endif

#ifdef CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_UTOPIA_FLAG_VAR
RedBoot_config_option("Utopia on NPE-A",
                      utopia,
                      ALWAYS_ENABLED, true,
                      CONFIG_BOOL,
                      CYGDAT_DEVS_ETH_INTEL_NPE_DEFAULT_UTOPIA_FLAG
    );
#endif // CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_UTOPIA_FLAG_VAR
#ifdef CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA_VARS
#ifdef CYGSEM_INTEL_NPE_USE_ETH0
RedBoot_config_option("Network hardware address [MAC] for NPE eth0",
                      npe_eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, npe_eth0_priv_data.mac_address
    );
#endif //CYGSEM_INTEL_NPE_USE_ETH0

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
RedBoot_config_option("Network hardware address [MAC] for NPE eth1",
                      npe_eth1_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, npe_eth1_priv_data.mac_address
    );
#endif // CYGSEM_INTEL_NPE_USE_ETH1

#ifdef CYGSEM_INTEL_NPE_USE_ETH2
RedBoot_config_option("Network hardware address [MAC] for NPE eth2",
                      npe_eth2_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, npe_eth2_priv_data.mac_address
    );
#endif // CYGSEM_INTEL_NPE_USE_ETH1
#endif // CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA_VARS

#define MAX_PORTS (__NUM_ETH0_PORTS + __NUM_ETH1_PORTS + __NUM_ETH2_PORTS)

#ifdef CYGPKG_REDBOOT
#define ACTIVE_PORTS 1
#else
#define ACTIVE_PORTS MAX_PORTS
#endif

#define NPE_PKT_SIZE 1600

#define NPE_MBUF_POOL_SIZE                               \
	((CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS + \
 	 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) * \
	sizeof(IX_OSAL_MBUF) * ACTIVE_PORTS)

#define NPE_PKT_POOL_SIZE                                \
	((CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS + \
 	 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) * \
	NPE_PKT_SIZE * ACTIVE_PORTS)

#define NPE_MEM_POOL_SIZE (NPE_MBUF_POOL_SIZE + NPE_PKT_POOL_SIZE)


// A little extra so we can align to cacheline.
static cyg_uint8 npe_alloc_pool[NPE_MEM_POOL_SIZE + HAL_DCACHE_LINE_SIZE - 1];
static cyg_uint8 *npe_alloc_end;
static cyg_uint8 *npe_alloc_free;

static void*
npe_alloc(int size)
{
    void *p = NULL;

    size = (size + (HAL_DCACHE_LINE_SIZE-1)) & ~(HAL_DCACHE_LINE_SIZE-1);
    if ((npe_alloc_free + size) < npe_alloc_end) {
        p = npe_alloc_free;
        npe_alloc_free += size;
    }
    return p;
}


// Not interrupt safe!
static void
mbuf_enqueue(IX_OSAL_MBUF **q, IX_OSAL_MBUF *new)
{
    IX_OSAL_MBUF *m = *q;

    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(new) = NULL;

    if (m) {
        while(IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m))
            m = IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m);
        IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m) = new;
    } else
        *q = new;
}

// Not interrupt safe!
static IX_OSAL_MBUF *
mbuf_dequeue(IX_OSAL_MBUF **q)
{
    IX_OSAL_MBUF *m = *q;
    if (m)
	*q = IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m);
    return m;
}

static void
reset_tx_mbufs(struct npe* p_npe)
{
    IX_OSAL_MBUF *m;
    int i;

    p_npe->txQHead = NULL;

    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS; i++) {
	m = &p_npe->tx_mbufs[i];

	memset(m, 0, sizeof(*m));

	IX_OSAL_MBUF_MDATA(m) = (void *)&p_npe->tx_pkts[i * NPE_PKT_SIZE];
#ifdef __LITTLE_ENDIAN
	IX_OSAL_MBUF_MDATA(m) = (void *)(((unsigned)IX_OSAL_MBUF_MDATA(m)) | SDRAM_DC_BASE);
#endif
	IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
	mbuf_enqueue(&p_npe->txQHead, m);
    }
}

static void
reset_rx_mbufs(struct npe* p_npe)
{
    IX_OSAL_MBUF *m;
    int i;

    p_npe->rxQHead = NULL;

    HAL_DCACHE_INVALIDATE(p_npe->rx_pkts, NPE_PKT_SIZE *
			  CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
 
    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS; i++) {
	m = &p_npe->rx_mbufs[i];

	memset(m, 0, sizeof(*m));

	IX_OSAL_MBUF_MDATA(m) = (void *)&p_npe->rx_pkts[i * NPE_PKT_SIZE];
#ifdef __LITTLE_ENDIAN
	IX_OSAL_MBUF_MDATA(m) = (void *)(((unsigned)IX_OSAL_MBUF_MDATA(m)) | SDRAM_DC_BASE);
#endif
	IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;

	if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("ixEthAccPortRxFreeReplenish failed for port %d\n",
			p_npe->eth_id);
#endif
	    break;
	}
    }
}

static void
init_rx_mbufs(struct npe* p_npe)
{
    p_npe->rxQHead = NULL;

    p_npe->rx_pkts = npe_alloc(NPE_PKT_SIZE *
			       CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
    if (p_npe->rx_pkts == NULL) {
#ifdef DEBUG
	diag_printf("alloc of packets failed.\n");
#endif
	return;
    }

    p_npe->rx_mbufs = (IX_OSAL_MBUF *)npe_alloc(sizeof(IX_OSAL_MBUF) *
					 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
    if (p_npe->rx_mbufs == NULL) {
#ifdef DEBUG
	diag_printf("alloc of mbufs failed.\n");
#endif
	return;
    }

    reset_rx_mbufs(p_npe);
}


static void
init_tx_mbufs(struct npe* p_npe)
{
    p_npe->tx_pkts = npe_alloc(NPE_PKT_SIZE *
			       CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
    if (p_npe->tx_pkts == NULL) {
#ifdef DEBUG
	diag_printf("alloc of packets failed.\n");
#endif
	return;
    }

    p_npe->tx_mbufs = (IX_OSAL_MBUF *)npe_alloc(sizeof(IX_OSAL_MBUF) *
					 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
    if (p_npe->tx_mbufs == NULL) {
#ifdef DEBUG
	diag_printf("alloc of mbufs failed.\n");
#endif
	return;
    }

    reset_tx_mbufs(p_npe);
}

// Returns non-zero if link is up.
static int
link_check(unsigned int phyNo)
{
    BOOL fullDuplex, linkUp, speed, autoneg;

    ixEthAccMiiLinkStatus(phyNo, &linkUp, &speed, &fullDuplex, &autoneg);
    if (linkUp == FALSE) {
	int retry = 20; /* 2 seconds */
#ifdef DEBUG
	diag_printf("Wait for PHY %u to be ready ...", phyNo);
#endif
	while (linkUp == FALSE && retry-- > 0) {
	    CYGACC_CALL_IF_DELAY_US((cyg_int32)100000);
	    ixEthAccMiiLinkStatus(phyNo, &linkUp, &speed, &fullDuplex, &autoneg);
	}
	if (linkUp == FALSE) 
	    return 0;
    }
    return 1;
}

// Returns non-zero if given PHY is present.
static int
phy_present(int phyno)
{
    if (phyno < 0 || phyno >= IXP425_ETH_ACC_MII_MAX_ADDR)
	return 0;

    if (!phy_scanned[phyno]) {
	if (ixEthMiiPhyPresent(phyno, &phy_exists[phyno]) != IX_ETH_ACC_SUCCESS)
	    return 0;
	phy_scanned[phyno] = 1;
    }
    return  phy_exists[phyno];
}

#if defined(CYGSEM_NPE_SMII)
static int
reset_npes(cyg_uint32 mask)
{
    cyg_uint32 timeout;
    cyg_uint32 resetMask = 0;

    if (mask & (1 << IX_ETH_PORT_1))
	resetMask |= (1 << IX_FEATURECTRL_NPEB);
    if (mask & (1 << IX_ETH_PORT_2))
	resetMask |= (1 << IX_FEATURECTRL_NPEC);
    if (mask & (1 << IX_ETH_PORT_3))
	resetMask |= (1 << IX_FEATURECTRL_NPEA);

    ixFeatureCtrlWrite(ixFeatureCtrlRead() | resetMask);

    // wait for up to 5 seconds
    for (timeout = 5000; timeout; timeout--) {
	CYGACC_CALL_IF_DELAY_US(1000);
	if ((ixFeatureCtrlRead() & resetMask) == resetMask)
	    return 1;
    }
    return 0;
}

static void
unreset_npes(cyg_uint32 mask)
{
    cyg_uint32 resetMask = 0;

    if (mask & (1 << IX_ETH_PORT_1))
	resetMask |= (1 << IX_FEATURECTRL_NPEB);
    if (mask & (1 << IX_ETH_PORT_2))
	resetMask |= (1 << IX_FEATURECTRL_NPEC);
    if (mask & (1 << IX_ETH_PORT_3))
	resetMask |= (1 << IX_FEATURECTRL_NPEA);

    ixFeatureCtrlWrite(ixFeatureCtrlRead() & ~resetMask);
}

static int
npe_enable_smii_mode(cyg_uint32 mask)
{
    cyg_uint32 smiiMask = 0;

    // Disable NPE-B Ethernet 1-3 Coprocessors
    ixFeatureCtrlWrite(ixFeatureCtrlRead() | (1 << IX_FEATURECTRL_NPEB_ETH));

    if (!reset_npes(mask))
	return 0;

    *IXP46X_EXP_SMII_RCOMP_CSR |= (1 << 16);

    *IXP46X_EXP_SMIIDLL = CYGDAT_NPE_SMII_DLL_SETTING;

    if (mask & (1 << IX_ETH_PORT_1))
	smiiMask |= EXP_CNFG1_NPEB_SMII;
    if (mask & (1 << IX_ETH_PORT_2))
	smiiMask |= EXP_CNFG1_NPEC_SMII;
    if (mask & (1 << IX_ETH_PORT_3))
	smiiMask |= EXP_CNFG1_NPEA_SMII;

    *IXP425_EXP_CNFG1 |= smiiMask;

    CYGACC_CALL_IF_DELAY_US(50);

    if ((*IXP425_EXP_CNFG1 & smiiMask) != smiiMask)
	return 0;

    unreset_npes(mask);

    // delay to allow lines to settle
    CYGACC_CALL_IF_DELAY_US(20000);

    return 1;
}

#if !defined(CYGHAL_NPE_SMII_INIT)
static int
do_enable_smii_mode(void)
{
    int mask = 0;

#if CYGINT_DEVS_ETH_INTEL_NPEB_SMII
    mask |= (1 << IX_ETH_PORT_1);
#endif
#if CYGINT_DEVS_ETH_INTEL_NPEC_SMII
    if (npe_used[IX_ETH_PORT_2] && npe_exists[IX_ETH_PORT_2])
	mask |= (1 << IX_ETH_PORT_2);
#endif
#if CYGINT_DEVS_ETH_INTEL_NPEA_SMII
    if (npe_used[IX_ETH_PORT_3] && npe_exists[IX_ETH_PORT_3])
	mask |= (1 << IX_ETH_PORT_3);
#endif

    return npe_enable_smii_mode(mask);
}
#endif  // !defined(CYGHAL_NPE_SMII_INIT)
#endif  // CYGSEM_NPE_SMII

// Returns non-zero if NPE-A is being used for UTOPIA
static int
utopia_enabled(void)
{
#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X) {
#ifdef CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_UTOPIA_FLAG
	bool ok, val;

	ok = false;
	ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,
					 "utopia", &val, CONFIG_BOOL);
	return (ok && val);
#else
#ifdef CYGDAT_DEVS_ETH_INTEL_NPE_UTOPIA_FLAG
	return CYGDAT_DEVS_ETH_INTEL_NPE_UTOPIA_FLAG;
#else
#error No method for getting UTOPIA flag.
#endif
#endif // CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_UTOPIA_FLAG
    }
#endif // CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x

    // npe_a is used for UTOPIA
    return 1;
}

#if CYGINT_DEVS_ETH_INTEL_NPE_PHY_DISCOVERY
/*
 * Some platforms may have jumper configurations which change the connection
 * between NPE engines and PHYs. This routine tries to determine if the given
 * NPE port is connected to the given PHY. It accomplishes this by putting
 * the PHY into loopback mode and seeing if the NPE port can send itself a
 * packet through that loopback.
 */
static int loop_txdone, loop_rxdone, loop_rxmatch;

static void
loop_tx_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m)
{
#ifdef DEBUG
    diag_printf("loop_tx_callback\n");
#endif
    loop_txdone = 1;
}

static void
loop_rx_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m, IxEthAccPortId portid)
{
    char *pkt = IX_OSAL_MBUF_MDATA(m);

    loop_rxdone = 1;

    if (!strcmp(pkt+14, "Hello"))
	loop_rxmatch = 1;

#ifdef DEBUG
    diag_printf("loop_rx_callback: found[%d]\n", loop_rxmatch);
#endif
    HAL_DCACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(m), NPE_PKT_SIZE);
}

static int
check_phy_association(int portno, int phyno)
{
    IX_OSAL_MBUF  *rxmb, *txmb;
    cyg_uint8 *rxpkt, *txpkt, *p;
    IxEthAccMacAddr saved_mac;
    int i;

    if (portno < 0 || portno >= NPE_NUM_PORTS || !npe_exists[portno])
	return 0;

    if (!phy_present(phyno))
	return 0;

    ixEthAccPortUnicastMacAddressGet(portno, &saved_mac);

    ixEthMiiPhyReset(phyno);
    ixEthMiiPhyConfig(phyno, 1, 1, 0);
    ixEthMiiPhyLoopbackEnable(phyno);
    ixEthAccPortPromiscuousModeSet(portno);

    // carve out a chunk of free memory for temporary use
    p = npe_alloc_free;
    rxpkt = p; p += NPE_PKT_SIZE;
    txpkt = p; p += NPE_PKT_SIZE;
#ifdef __LITTLE_ENDIAN
    rxpkt = (void *)((unsigned)rxpkt | SDRAM_DC_BASE);
    txpkt = (void *)((unsigned)txpkt | SDRAM_DC_BASE);
#endif
    rxmb = (IX_OSAL_MBUF *)p; p += sizeof(IX_OSAL_MBUF);
    txmb = (IX_OSAL_MBUF *)p;

    // initialize rx mbuf and give it to eth port
    HAL_DCACHE_INVALIDATE(rxpkt, NPE_PKT_SIZE);
    memset(rxmb, 0, sizeof(*rxmb));
    IX_OSAL_MBUF_MDATA(rxmb) = (void *)rxpkt;
    IX_OSAL_MBUF_MLEN(rxmb) = IX_OSAL_MBUF_PKT_LEN(rxmb) = NPE_PKT_SIZE;

    ixEthAccPortRxFreeReplenish(portno, rxmb);

    // register callbacks and enable eth port
    ixEthAccPortRxCallbackRegister(portno, loop_rx_callback, 0);
    ixEthAccPortTxDoneCallbackRegister(portno, loop_tx_callback, 0);
    ixEthAccPortEnable(portno);

    // clear flags
    loop_txdone = loop_rxdone = loop_rxmatch = 0;
    
#ifdef DEBUG
    diag_printf("check_phy_association: transmitting on port[%d] phy[%d]\n", portno, phyno);
#endif

    // transmit a packet
    memset(txmb, 0, sizeof(*txmb));
    IX_OSAL_MBUF_MDATA(txmb) = (void *)txpkt;
    memset(txpkt, 0, NPE_PKT_SIZE);
    txpkt[12] = 0x08;
    strcpy(txpkt + 14, "Hello");
    IX_OSAL_MBUF_MLEN(txmb) = IX_OSAL_MBUF_PKT_LEN(txmb) = 100;
    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(txmb) = NULL;
    HAL_DCACHE_FLUSH(IX_OSAL_MBUF_MDATA(txmb), IX_OSAL_MBUF_MLEN(txmb));
    ixEthAccPortTxFrameSubmit(portno, txmb, IX_ETH_ACC_TX_DEFAULT_PRIORITY);

    // wait for txdone
    if (!__npe_poll_timed(portno, &loop_txdone, 100)) {
#ifdef DEBUG
	diag_printf("no txdone: eth[%d] phy[%d]\n", portno, phyno);
#endif
    } else if (!__npe_poll_timed(portno, &loop_rxmatch, 100)) {
#ifdef DEBUG
	diag_printf("no rxmatch: eth[%d] phy[%d] rxdone[%d]\n", portno, phyno, loop_rxdone);
#endif
    }

    // put eth port back the way it was
    ixEthAccPortPromiscuousModeClear(portno);
    ixEthAccPortDisable(portno);

    // Delay to give time for recovery of mbufs
    for (i = 0; i < 100 && (!loop_txdone || !loop_rxdone); i++) {
	__npe_poll(portno);
	CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
    }

    ixEthMiiPhyLoopbackDisable(phyno);
    ixEthAccPortUnicastMacAddressSet(portno, &saved_mac);

    // return non-zero if packet looped back
    return loop_rxmatch;
}

static void
set_phy_association(int portno, int phyno)
{
#ifdef CYGSEM_INTEL_NPE_USE_ETH0
    if (npe_eth0_priv_data.eth_id == portno) {
	npe_eth0_priv_data.phy_no = phyno;
	return;
    }
#endif
#ifdef CYGSEM_INTEL_NPE_USE_ETH1
    if (npe_eth1_priv_data.eth_id == portno) {
	npe_eth1_priv_data.phy_no = phyno;
	return;
    }
#endif
#ifdef CYGSEM_INTEL_NPE_USE_ETH2
    if (npe_eth2_priv_data.eth_id == portno) {
	npe_eth2_priv_data.phy_no = phyno;
	return;
    }
#endif
}
#endif // CYGINT_DEVS_ETH_INTEL_NPE_PHY_DISCOVERY

// Initialize given PHY for given port.
// Returns non-zero if successful.
static int
phy_init(int portno, int phyno)
{
    BOOL speed, linkUp, fullDuplex, autoneg;

    ixEthMiiPhyReset(phyno);

    ixEthMiiPhyConfig(phyno, TRUE, TRUE, TRUE);

    // wait until the link is up before setting the MAC duplex
    // mode, the PHY duplex mode may change after autonegotiation 
#ifndef SKIP_NPE_LINK_CHECK
    link_check(phyno);
#endif

    ixEthAccMiiLinkStatus(phyno, &linkUp, &speed, &fullDuplex, &autoneg);

    /* Set the MAC duplex mode */
    if (ixEthAccPortDuplexModeSet(portno, fullDuplex ? IX_ETH_ACC_FULL_DUPLEX :
				  IX_ETH_ACC_HALF_DUPLEX) != IX_ETH_ACC_SUCCESS) {
	diag_printf("PortDuplexModeSet:  failed!\n");
	return 0;
    }

#ifdef DEBUG
    diag_printf("\nPHY %d configuration:\n", phyno);
    ixEthAccMiiShow(phyno);
#endif

    return 1;
}


// ethAcc RX callback
static void
npe_rx_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m, IxEthAccPortId portid)
{
    struct npe* p_npe = (struct npe *)cbTag;
    struct eth_drv_sc *sc;

    sc = p_npe->sc;

    if (IX_OSAL_MBUF_MLEN(m) > 0) {
	mbuf_enqueue(&p_npe->rxQHead, m);

	(sc->funs->eth_drv->recv)(sc, IX_OSAL_MBUF_MLEN(m));
    }

    // Now return mbuf to NPE
    IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m) = NULL;
    IX_OSAL_MBUF_FLAGS(m) = 0;
    
    HAL_DCACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(m), NPE_PKT_SIZE);
 
    if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_rx_callback: Error returning mbuf.\n");
#endif
    }
}

// callback which is used by ethAcc to recover RX buffers when stopping
static void
npe_rx_stop_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m, IxEthAccPortId portid)
{
}


// ethAcc TX callback
static void
npe_tx_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m)
{
    struct npe* p_npe = (struct npe *)cbTag;
    struct eth_drv_sc *sc;

    sc = p_npe->sc;

    (sc->funs->eth_drv->tx_done)(sc, (unsigned long)IX_OSAL_MBUF_PRIV(m), 1);
    
    IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m) = NULL;
    IX_OSAL_MBUF_FLAGS(m) = 0;

    mbuf_enqueue(&p_npe->txQHead, m);
}


// callback which is used by ethAcc to recover TX buffers when stopping
static void
npe_tx_stop_callback(cyg_uint32 cbTag, IX_OSAL_MBUF *m)
{
}

static int
npe_set_mac_address(struct npe *p_npe)
{
    int mac_ok = false;
    IxEthAccMacAddr  npeMac;

    // Set MAC address
#if defined(CYGPKG_DEVS_ETH_INTEL_NPE_REDBOOT_HOLDS_ESA)
    {
	char *cfgname = NULL;
#if defined(CYGSEM_INTEL_NPE_USE_ETH0)
	if (p_npe == &npe_eth0_priv_data)
	    cfgname = "npe_eth0_esa";
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH1) 
	if (p_npe == &npe_eth1_priv_data)
	    cfgname = "npe_eth1_esa";
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH2) 
	if (p_npe == &npe_eth2_priv_data)
	    cfgname = "npe_eth2_esa";
#endif
        mac_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET, cfgname,
					     p_npe->mac_address, CONFIG_ESA);
    }
#elif defined(CYGHAL_GET_NPE_ESA)
    CYGHAL_GET_NPE_ESA(p_npe->eth_id, p_npe->mac_address, mac_ok);
#else
#error No mechanism to get MAC address
#endif

    if (!mac_ok) {
#ifdef DEBUG
	diag_printf("Error getting MAC address for %s.\n", npe_names[p_npe->eth_id]);
#endif
	return 0;
    }

    npeMac.macAddress[0] = p_npe->mac_address[0];
    npeMac.macAddress[1] = p_npe->mac_address[1];
    npeMac.macAddress[2] = p_npe->mac_address[2];
    npeMac.macAddress[3] = p_npe->mac_address[3];
    npeMac.macAddress[4] = p_npe->mac_address[4];
    npeMac.macAddress[5] = p_npe->mac_address[5];

    if (ixEthAccPortUnicastMacAddressSet(p_npe->eth_id, &npeMac) != IX_ETH_ACC_SUCCESS) {
	diag_printf("Error setting unicast address! %02x:%02x:%02x:%02x:%02x:%02x\n",
		    npeMac.macAddress[0], npeMac.macAddress[1],
		    npeMac.macAddress[2], npeMac.macAddress[3],
		    npeMac.macAddress[4], npeMac.macAddress[5]);
	return 0;
    }
    return 1;
}


// Boot-time CSR library initialization.
static int
npe_csr_load(void)
{
    int i;

    if (ixQMgrInit() != IX_SUCCESS) {
	diag_printf("Error initialising queue manager!\n");
	return 0;
    }

    ixQMgrDispatcherLoopGet(&qDispatcherFunc);

    if(ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES) != IX_SUCCESS) {
	diag_printf("Error initialising NPE Message handler!\n");
	return 0;
    }

    if (npe_used[IX_ETH_PORT_1] && npe_exists[IX_ETH_PORT_1] &&
	ixNpeDlNpeInitAndStart(IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS) != IX_SUCCESS) {
	diag_printf("Error downloading firmware to NPE-B!\n");
	return 0;
    }

    if (npe_used[IX_ETH_PORT_2] && npe_exists[IX_ETH_PORT_2] &&
	ixNpeDlNpeInitAndStart(IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS) != IX_SUCCESS) {
	diag_printf("Error downloading firmware to NPE-C!\n");
	return 0;
    }

    if (npe_used[IX_ETH_PORT_3] && npe_exists[IX_ETH_PORT_3] &&
	ixNpeDlNpeInitAndStart(IX_NPEDL_NPEIMAGE_NPEA_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS) != IX_SUCCESS) {
	diag_printf("Error downloading firmware to NPE-A!\n");
	return 0;
    }

#ifdef CYGPKG_REDBOOT
    // don't need this for redboot
    ixFeatureCtrlSwConfigurationWrite(IX_FEATURECTRL_ETH_LEARNING, FALSE);
#endif

    if (ixEthAccInit() != IX_ETH_ACC_SUCCESS) {
	diag_printf("Error initialising Ethernet access driver!\n");
	return 0;
    }

    for (i = 0; i < IX_ETH_ACC_NUMBER_OF_PORTS; i++) {
	if (!npe_used[i] || !npe_exists[i])
	    continue;
	if (ixEthAccPortInit(i) != IX_ETH_ACC_SUCCESS) {
	    diag_printf("Error initialising Ethernet port%d!\n", i);
	}
	if (ixEthAccTxSchedulingDisciplineSet(i, FIFO_NO_PRIORITY) != IX_ETH_ACC_SUCCESS) {
	    diag_printf("Error setting scheduling discipline for port %d.\n", i);
	}
	if (ixEthAccPortRxFrameAppendFCSDisable(i) != IX_ETH_ACC_SUCCESS) {
	    diag_printf("Error disabling RX FCS for port %d.\n", i);
	}
	if (ixEthAccPortTxFrameAppendFCSEnable(i) != IX_ETH_ACC_SUCCESS) {
	    diag_printf("Error enabling TX FCS for port %d.\n", i);
	}
    }

#if defined(CYGSEM_INTEL_NPE_USE_ETH0)
    if (npe_exists[npe_eth0_priv_data.eth_id])
        npe_set_mac_address(&npe_eth0_priv_data);
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH1)
    if (npe_exists[npe_eth1_priv_data.eth_id])
	npe_set_mac_address(&npe_eth1_priv_data);
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH2)
    if (npe_exists[npe_eth2_priv_data.eth_id])
	npe_set_mac_address(&npe_eth2_priv_data);
#endif

    memset(phy_scanned, 0, sizeof(phy_scanned));

    return 1;
}

// Uninitialize CSR library.
static void
npe_csr_unload(void)
{
    ixEthAccUnload();
    ixEthDBUnload();
    ixNpeMhUnload();
    ixQMgrUnload();
}

// ------------------------------------------------------------------------
//
//  API Function : npe_init
//
// ------------------------------------------------------------------------
static bool
npe_init(struct cyg_netdevtab_entry * ndp)
{
    static int initialized = 0;
    struct eth_drv_sc *sc;
    struct npe *p_npe;
    int i;

    sc = (struct eth_drv_sc *)ndp->device_instance;
    p_npe = (struct npe *)sc->driver_private;

    p_npe->sc = sc;

    if (!initialized) {
	// One time initialization common to all ports
	initialized = 1;

	if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X) {
	    switch (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK) {
	    case IX_FEATURE_CTRL_SILICON_TYPE_B0:
		/*
		 * If it is B0 Silicon, we only enable port when its corresponding  
		 * Eth Coprocessor is available.
		 */
		if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
		    IX_FEATURE_CTRL_COMPONENT_ENABLED)
		    npe_exists[IX_ETH_PORT_1] = TRUE;

		if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) == 
		    IX_FEATURE_CTRL_COMPONENT_ENABLED)
		    npe_exists[IX_ETH_PORT_2] = TRUE;
		break;
	    case IX_FEATURE_CTRL_SILICON_TYPE_A0:
		/*
		 * If it is A0 Silicon, we enable both as both Eth Coprocessors 
		 * are available. 
		 */ 
		npe_exists[IX_ETH_PORT_1] = TRUE;
		npe_exists[IX_ETH_PORT_2] = TRUE;
		break;
	    }
	} else if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X) {
	    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) == IX_FEATURE_CTRL_COMPONENT_ENABLED)
		npe_exists[IX_ETH_PORT_1] = TRUE;

	    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) == IX_FEATURE_CTRL_COMPONENT_ENABLED)
		npe_exists[IX_ETH_PORT_2] = TRUE;

	    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) == IX_FEATURE_CTRL_COMPONENT_ENABLED)
		npe_exists[IX_ETH_PORT_3] = utopia_enabled() ? FALSE : TRUE;
	}

#if defined(CYGSEM_INTEL_NPE_USE_ETH0)
	npe_used[CYGNUM_ETH0_ETH_ID] = 1;
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH1)
	npe_used[CYGNUM_ETH1_ETH_ID] = 1;
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH2) 
	npe_used[CYGNUM_ETH2_ETH_ID] = 1;
#endif

	// If we need NPE-A for ethernet, turn off Utopia
	if (npe_used[IX_ETH_PORT_3] && npe_exists[IX_ETH_PORT_3])
	    ixFeatureCtrlWrite(ixFeatureCtrlRead() | (1 << IX_FEATURECTRL_UTOPIA));

	npe_alloc_end = npe_alloc_pool + sizeof(npe_alloc_pool);
	npe_alloc_free = (cyg_uint8 *)(((unsigned)npe_alloc_pool + HAL_DCACHE_LINE_SIZE - 1)
				       & ~(HAL_DCACHE_LINE_SIZE - 1));

#if defined(CYGSEM_NPE_SMII)
#if !defined(CYGHAL_NPE_SMII_INIT)
#define CYGHAL_NPE_SMII_INIT() do_enable_smii_mode()
#endif
	if (!CYGHAL_NPE_SMII_INIT()) {
	    diag_printf("NPE SMII initialization failed!\n");
	    return 0;
	}
#endif

	if (!npe_csr_load())
	    return 0;

	diag_printf("\n");
    }

    if (!npe_exists[p_npe->eth_id])
	return 0;

    diag_printf("Trying %s...", npe_names[p_npe->eth_id]);

#if CYGINT_DEVS_ETH_INTEL_NPE_PHY_DISCOVERY
    // This tries to find which phy (could be more than one) is
    // connected to this port.
    if (p_npe->phy_no == NPE_PHY_UNKNOWN) {
	for (i = 0; i < IXP425_ETH_ACC_MII_MAX_ADDR; i++) {
	    if (check_phy_association(p_npe->eth_id, i)) {
		set_phy_association(p_npe->eth_id, i);
		break;
	    }
	}
    }
#endif

    if (p_npe->phy_no == NPE_PHY_UNKNOWN || !phy_present(p_npe->phy_no)) {
	diag_printf("no PHY found\n");
	return 0;
    }

#ifdef DEBUG
    diag_printf("phy for %s found!\n", npe_names[p_npe->eth_id]);
#endif

    if (!phy_init(p_npe->eth_id, p_npe->phy_no)) {
	diag_printf("PHY init error!\n");
	return 0;
    }

    // initialize mbuf pool
    init_rx_mbufs(p_npe);
    init_tx_mbufs(p_npe);

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
	diag_printf("can't register RX callback!\n");
	return 0;
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
	diag_printf("can't register TX callback!\n");
	return 0;
    }

    if (ixEthAccPortEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
	diag_printf("can't enable port!\n");
	return 0;
    }

//    ixEthDBFilteringPortMaximumFrameSizeSet(p_npe->eth_id, NPE_PKT_SIZE);

    p_npe->active = 1;

    diag_printf("success. Using %s with PHY %d.\n",
		npe_names[p_npe->eth_id], p_npe->phy_no);

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, &(p_npe->mac_address[0]) );

    return 1;
}

// ------------------------------------------------------------------------
//
//  API Function : npe_start
//
// ------------------------------------------------------------------------
static void 
npe_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
#ifndef CYGPKG_REDBOOT
    struct npe *p_npe = (struct npe *)sc->driver_private;

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering rx callback!\n");
#endif
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering tx callback!\n");
#endif
    }

    if (ixEthAccPortEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_start: Error disabling %s!\n", npe_names[p_npe->eth_id]);
#endif
    }

    p_npe->active = 1;
#endif
}

// ------------------------------------------------------------------------
//
//  API Function : npe_stop
//
// ------------------------------------------------------------------------
static void
npe_stop( struct eth_drv_sc *sc )
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    int i;

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_stop_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering rx callback!\n");
#endif
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_stop_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering tx callback!\n");
#endif
    }

    if (ixEthAccPortDisable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_stop: Error disabling NPEB!\n");
#endif
    }

    // Delay to give time for recovery of mbufs
    for (i = 0; i < 100; i++) {
	CYGACC_CALL_IF_DELAY_US((cyg_int32)10000);
	__npe_poll(p_npe->eth_id);
    }

// For RedBoot only, we are probably launching Linux or other OS that
// needs a clean slate for its NPE library.	
#ifdef CYGPKG_REDBOOT
    for (i = 0; i < IX_ETH_ACC_NUMBER_OF_PORTS; i++) {
	if (npe_used[i] && npe_exists[i])
	    if (ixNpeDlNpeStopAndReset(__eth_to_npe(i)) != IX_SUCCESS)
		diag_printf ("Failed to stop and reset NPE B.\n");
    }
#endif
    p_npe->active = 0;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_recv
//
// ------------------------------------------------------------------------
static void 
npe_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    struct eth_drv_sg *sg = sg_list;
    IX_OSAL_MBUF *m;
    cyg_uint8 *src;
    unsigned len;

    m = mbuf_dequeue(&p_npe->rxQHead);
    src = IX_OSAL_MBUF_MDATA(m);

    while (sg < &sg_list[sg_len]) {
            
        len = sg->len;

        if (len < 0 || sg->buf == 0)
            return;

        if (len > IX_OSAL_MBUF_MLEN(m))
            len = IX_OSAL_MBUF_MLEN(m);

        memcpy((char *)sg->buf, src, len);
        src += len;
        IX_OSAL_MBUF_MLEN(m) -= len;

	++sg;
    }
}

// ------------------------------------------------------------------------
//
//  API Function : npe_can_send
//
// ------------------------------------------------------------------------
static int 
npe_can_send(struct eth_drv_sc *sc)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;

    return p_npe->active && p_npe->txQHead != NULL;
}

// ------------------------------------------------------------------------
//
//  API Function : npe_send
//
// ------------------------------------------------------------------------
static void 
npe_send(struct eth_drv_sc *sc,
	 struct eth_drv_sg *sg_list, int sg_len,
	 int total_len, unsigned long key)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    struct eth_drv_sg *sg = sg_list;
    cyg_uint8 *dest;
    int len, err;
    IX_OSAL_MBUF *m;

    m = mbuf_dequeue(&p_npe->txQHead);

    dest = IX_OSAL_MBUF_MDATA(m);
    if ((unsigned)total_len > IX_OSAL_MBUF_MLEN(m))
	total_len = IX_OSAL_MBUF_MLEN(m);

    IX_OSAL_MBUF_PKT_LEN(m) = IX_OSAL_MBUF_MLEN(m) = total_len;
    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m) = NULL;
    IX_OSAL_MBUF_PRIV(m) = (void *)key;

    while (sg < &sg_list[sg_len] && total_len > 0) {

	len = sg->len;
	if (len > total_len)
	    len = total_len;

	memcpy(dest, (char *)sg->buf, len);

	dest += len;
	total_len -= len;

	++sg;
    }

    HAL_DCACHE_FLUSH(IX_OSAL_MBUF_MDATA(m), IX_OSAL_MBUF_MLEN(m));

    if ((err = ixEthAccPortTxFrameSubmit(p_npe->eth_id, m, IX_ETH_ACC_TX_DEFAULT_PRIORITY))
       != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_send: Can't submit frame. err[%d]\n", err);
#endif
	mbuf_enqueue(&p_npe->txQHead, m);
	return;
    }
}

// ------------------------------------------------------------------------
//
//  API Function : npe_deliver
//
// ------------------------------------------------------------------------
static void
npe_deliver(struct eth_drv_sc *sc)
{
}

// ------------------------------------------------------------------------
//
//  API Function : npe_poll
//
// ------------------------------------------------------------------------
static void
npe_poll(struct eth_drv_sc *sc)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    cyg_uint32 ints;

    ints = *IXP425_INTR_EN;
    *IXP425_INTR_EN = ints & ~NPE_INT_BITS;

    __npe_poll(p_npe->eth_id);

    *IXP425_INTR_EN = ints;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_int_vector
//
// ------------------------------------------------------------------------
static int
npe_int_vector(struct eth_drv_sc *sc)
{
    struct npe *p_npe;
    p_npe = (struct npe *)sc->driver_private;

    if (p_npe->eth_id == IX_ETH_PORT_1)
	return CYGNUM_HAL_INTERRUPT_NPEB;
    else if (p_npe->eth_id == IX_ETH_PORT_2)
	return CYGNUM_HAL_INTERRUPT_NPEC;
    else if (p_npe->eth_id == IX_ETH_PORT_3)
	return CYGNUM_HAL_INTERRUPT_NPEA;
    return -1;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_ioctl
//
// ------------------------------------------------------------------------
static int
npe_ioctl(struct eth_drv_sc *sc, unsigned long key,
	  void *data, int data_length)
{
    return -1;
}


#ifdef CYGSEM_INTEL_NPE_USE_ETH0
ETH_DRV_SC(npe_sc0,
           &npe_eth0_priv_data,
           CYGDAT_NPE_ETH0_NAME,
           npe_start,
           npe_stop,
           npe_ioctl,
           npe_can_send,
           npe_send,
           npe_recv,
           npe_deliver,
           npe_poll,
           npe_int_vector);

NETDEVTAB_ENTRY(npe_netdev0, 
                "npe_" CYGDAT_NPE_ETH0_NAME,
                npe_init, 
                &npe_sc0);
#endif // CYGSEM_INTEL_NPE_USE_ETH0

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
ETH_DRV_SC(npe_sc1,
           &npe_eth1_priv_data,
           CYGDAT_NPE_ETH1_NAME,
           npe_start,
           npe_stop,
           npe_ioctl,
           npe_can_send,
           npe_send,
           npe_recv,
           npe_deliver,
           npe_poll,
           npe_int_vector);

NETDEVTAB_ENTRY(npe_netdev1, 
                "npe_" CYGDAT_NPE_ETH1_NAME,
                npe_init, 
                &npe_sc1);
#endif // CYGSEM_INTEL_NPE_USE_ETH1

#ifdef CYGSEM_INTEL_NPE_USE_ETH2
ETH_DRV_SC(npe_sc2,
           &npe_eth2_priv_data,
           CYGDAT_NPE_ETH2_NAME,
           npe_start,
           npe_stop,
           npe_ioctl,
           npe_can_send,
           npe_send,
           npe_recv,
           npe_deliver,
           npe_poll,
           npe_int_vector);

NETDEVTAB_ENTRY(npe_netdev2, 
                "npe_" CYGDAT_NPE_ETH2_NAME,
                npe_init, 
                &npe_sc2);
#endif // CYGSEM_INTEL_NPE_USE_ETH1

// ------------------------------------------------------------------------
// EOF if_npe.c
