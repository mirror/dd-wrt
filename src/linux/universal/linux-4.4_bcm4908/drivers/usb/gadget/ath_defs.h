/*
 * Atheros USB Controller Defines
 */

#ifndef __LINUX_ATH_DEFS_H
#define __LINUX_ATH_DEFS_H

#include <linux/platform_device.h>
#include <asm/mach-ar7240/ar7240.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>

#include <linux/usb.h>
#include <linux/usb/otg.h>

/*
 * USB Registers
 */
struct ath_usb {
	__u32 	usbcmd,		/* Command register */
		usbsts,		/* Status register */
		usbintr,	/* Interrupt enable */
		usbfrmindx,	/* Frame index */
		ctrldssegment,	/* 4G segment selector */
		devaddr,	/* Device Address */
		ep_list_addr,	/* Endpoint List Address */
		reserved,
		burst_size,	/* Setting the burst size*/
		tx_filltuning,
		tx_ttfilltuning,
		reserved0[5],
		configflag,	/* Configured Flag register */
		portscx[8],	/* Port Status/Control x, x = 1..8 */
		otgsc,
		usbmode,	/* USB Host/Device mode */
		ep_setup_stat,	/* Endpoint Setup Status */
		ep_prime,	/* Endpoint Initialize */
		ep_flush,	/* Endpoint De-initialize */
		ep_status,	/* Endpoint Status */
		ep_complete,	/* Endpoint Interrupt On Complete */
		ep_ctrlx[16];	/* Endpoint Control, where x = 0.. 15 */
};

struct ath_otg_vars {
	u8	A_BUS_DROP,
		A_BUS_REQ,
		B_BUS_REQ,
		HOST_UP,
		DEVICE_UP,
		A_SRP_DET;
	u32	A_SRP_TMR;
	u8	A_DATA_PULSE_DET;
	u32	TB_SE0_SRP_TMR;
	u8	B_SE0_SRP;
	s32	OTG_INT_STATUS;
	u8	A_BUS_RESUME,
		A_BUS_SUSPEND,
		A_BUS_SUSPEND_REQ,
		A_CONN,
		B_BUS_RESUME,
		B_BUS_SUSPEND,
		B_CONN,
		A_SET_B_HNP_EN,
		B_SESS_REQ,
		B_SRP_DONE,
		B_HNP_ENABLE,
		A_CONNECTED,
		B_DISCONNECTED,
		CHECK_SESSION,
		A_SUSPEND_TIMER_ON;
	/* A device specific timers */
	u32	TA_WAIT_VRISE_TMR,
		TA_WAIT_BCON_TMR,
		TA_AIDL_BDIS_TMR,
		TA_BIDL_ADIS_TMR,
	/* B device specific timers */
		TB_DATA_PLS_TMR,
		TB_SRP_INIT_TMR,
		TB_SRP_FAIL_TMR,
		TB_VBUS_PLS_TMR,
		TB_ASE0_BRST_TMR,
		TB_A_SUSPEND_TMR,
		TB_VBUS_DSCHRG_TMR;
};

/*
 * OTG Driver Information
 */
struct ath_otg {
	struct usb_phy			otg;
	struct work_struct		work;
	struct ath_otg_vars		state;
	struct otg_reg __iomem		*otg_reg;
	struct device			*dev;
	spinlock_t			arlock;
	unsigned int			irqnum;
	struct ehci_caps __iomem	*cap_reg;
	struct ath_usb __iomem		*usb_reg;
	void __iomem			*reg_base;
	irqreturn_t (*udc_isr)(int, void *, struct pt_regs *r);
	void				*udc;
	u64				rsrc_start;
	u64				rsrc_len;
	u32				enabled_ints;
	u32				bVRISE_TIMEDOUT;
	struct echi_hcd			*ehci;	/* Temp Added to Test HNP */
};

/*
 * Atheros USB Debug functions
 */
#define ATH_USB_DEBUG_FUNCTION		(0x00000001)
#define ATH_USB_DEBUG_INTERRUPT		(0x00000002)
#define ATH_USB_DEBUG_ENDPOINT		(0x00000004)
#define ATH_USB_DEBUG_PORTSTATUS	(0x00000008)
#define ATH_USB_DEBUG_DEVICE		(0x00000010)
#define ATH_USB_DEBUG_MEMORY		(0x00000020)
#define ATH_USB_DEBUG_QUEUEHEAD		(0x00000040)
#define ATH_USB_DEBUG_DTD		(0x00000080)
#define ATH_USB_DEBUG_OTG		(0x00000100)


#ifdef ATH_USB_DEBUG
#define ath_usb_debug(level, format, arg...)	\
do {						\
	if (level & ath_usb_debug_level) {	\
		printk(format, ##arg);		\
	}					\
} while (0)
#else
#define ath_usb_debug(level, format, arg...)	\
	do { (void)(level); } while (0)
#endif

#define ath_usb_debug_fn(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_FUNCTION, format, ##arg)
#define ath_usb_debug_ep(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_ENDPOINT, format, ##arg)
#define ath_usb_debug_ps(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_PORTSTATUS, format, ##arg)
#define ath_usb_debug_int(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_INTERRUPT, format, ##arg)
#define ath_usb_debug_dev(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_DEVICE, format, ##arg)
#define ath_usb_debug_mem(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_MEMORY, format, ##arg)
#define ath_usb_debug_qh(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_QUEUEHEAD, format, ##arg)
#define ath_usb_debug_dtd(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_DTD, format, ##arg)
#define ath_usb_debug_otg(format, arg...)	\
	ath_usb_debug(ATH_USB_DEBUG_OTG, format, ##arg)

#define ath_usb_error		printk
#define ath_usb_warn		printk

/* Device/Host Capability Reg */
#define ATH_USB_NON_EHCI_DCCPARAMS		(0x124)

/* Device/Host Timer Reg */
#define ATH_USB_NON_EHCI_TIMER0LD		(0x80)
#define ATH_USB_NON_EHCI_TIMER0CTRL		(0x84)
#define ATH_USB_NON_EHCI_TIMER1LD		(0x88)
#define ATH_USB_NON_EHCI_TIMER1CTRL		(0x8C)

/* Device/Host Operational Reg (non-ehci) */
#define ATH_USB_EHCI_EXT_BURSTSZ		(0x160)
#define ATH_USB_EHCI_EXT_TTCTRL			(0x15C)
#define ATH_USB_EHCI_EXT_USBMODE		(0x1A8)
#define ATH_USB_EHCI_EXT_TXFILL			(0x164)
#define ATH_USB_EHCI_EXT_ULPI			(0x170)
#define ATH_USB_EHCI_EXT_OTGSC			(0x1A4)

#define ATH_USB_RESET_USB_HOST			(ATH_RESET_USB_HOST)
#define ATH_USB_RESET_USB_PHY			(ATH_RESET_USB_PHY)
#define ATH_USB_RESET_USBSUS_OVRIDE		(ATH_RESET_USBSUS_OVRIDE)
#define ATH_USB_USB_MODE			(ATH_USB_MODE)
#define AR9130_USB_MODE                     (AR7240_USB_MODE)

#define ATH_USB_RESET				(ATH_RESET_BASE + 0x1C)
#define ATH_USB_USB_CONFIG			(ATH_USB_CONFIG_BASE + 0x4)
#define ATH_USB_USB_FLADJ_VAL			(ATH_USB_CONFIG_BASE)

#define ath_usb_suspend_mode	1
#if ath_usb_suspend_mode
#define ATH_USB_PWRCTL				(ATH_USB_CONFIG_BASE + 0x0)
#define ATH_USB_DEV_SUSPEND_CTRL		(ATH_USB_CONFIG_BASE + 0x8)
#define ATH_USB_SUSPEND_RESUME_CNTR		(ATH_USB_CONFIG_BASE + 0xc)
#else
#define ATH_USB_PWRCTL				(ATH_USB_EHCI_BASE + 0x0)
#define ATH_USB_DEV_SUSPEND_CTRL		(ATH_USB_EHCI_BASE + 0x8)
#define ATH_USB_SUSPEND_RESUME_CNTR		(ATH_USB_EHCI_BASE + 0xc)
#endif


#define ath_usb_reg_rmw_set(_reg,_mask)      ar7240_reg_rmw_set(_reg,_mask)
#define ath_usb_reg_rmw_clear(_reg,_mask)    ar7240_reg_rmw_clear(_reg,_mask)
#define ath_usb_reg_wr(_phys,_val)           ar7240_reg_wr(_phys,_val)
#define ath_usb_reg_rd(_phys)                ar7240_reg_rd(_phys)


/* Operational Modes */
enum ath_hc_op_modes {
	ATH_USB_OP_MODE_MPH,
	ATH_USB_OP_MODE_DEV,
	ATH_USB_OP_MODE_SPH,
	ATH_USB_OP_MODE_OTG,
};

/* Transceiver Modes */
enum ath_hc_trans {
	ATH_USB_TRANS_UTMI,
	ATH_USB_TRANS_ULPI,
	ATH_USB_TRANS_SERIAL,
};


/* OTG Status/Control Reigser Defines */
#define ATH_USB_OTGSC_IMASK	(0x5F000000)
#define ATH_USB_OTGSC_SMASK	(0x005F0000)
#define ATH_USB_OTGSC_DPIE	(0x40000000)	/* Data-line pulsing IE */
#define ATH_USB_OTGSC_1MSIE	(0x20000000)
#define ATH_USB_OTGSC_BSEIE	(0x10000000)	/* B-session end IE */
#define ATH_USB_OTGSC_BSVIE	(0x08000000)	/* B-session valid IE */
#define ATH_USB_OTGSC_ASVIE	(0x04000000)	/* A-session valid IE */
#define ATH_USB_OTGSC_AVVIE	(0x02000000)	/* A-V-bus valid IE */
#define ATH_USB_OTGSC_IDIE	(0x01000000)	/* OTG ID IE */
#define ATH_USB_OTGSC_DPIS	(0x00400000)	/* Data-line pulsing IS */
#define ATH_USB_OTGSC_1MSIS	(0x00200000)
#define ATH_USB_OTGSC_BSEIS	(0x00100000)	/* B-session end IS */
#define ATH_USB_OTGSC_BSVIS	(0x00080000)	/* B-session valid IS */
#define ATH_USB_OTGSC_ASVIS	(0x00040000)	/* A-session valid IS */
#define ATH_USB_OTGSC_AVVIS	(0x00020000)	/* A-Vbus valid IS */
#define ATH_USB_OTGSC_IDIS	(0x00010000)	/* OTG ID IS */

/* OTG status bit masks */
#define ATH_USB_OTGSC_DPS	(1 << 14)
#define ATH_USB_OTGSC_1MST	(1 << 13)
#define ATH_USB_OTGSC_BSE	(1 << 12)	/* B-session end */
#define ATH_USB_OTGSC_BSV	(1 << 11)	/* B-session valid */
#define ATH_USB_OTGSC_ASV	(1 << 10)	/* A-session valid */
#define ATH_USB_OTGSC_AVV	(1 << 9)	/* A-Vbus Valid */
#define ATH_USB_OTGSC_ID	(1 << 8)	/* OTG ID */

/* OTG control bit masks */
#define ATH_USB_OTGSC_CTL_BITS	(0x2F)
#define ATH_USB_OTGSC_HABA	(1 << 7)	/*hw assisted data pulse bits*/
#define ATH_USB_OTGSC_HADP	(1 << 6)	/*hw assisted data pulse bits*/
#define ATH_USB_OTGSC_IDPU	(1 << 5)
#define ATH_USB_OTGSC_BHEN	(1 << 5)	/* B Host Enable */
#define ATH_USB_OTGSC_DP	(1 << 4)
#define ATH_USB_OTGSC_OT	(1 << 3)
#define ATH_USB_OTGSC_HAAR	(1 << 2)
#define ATH_USB_OTGSC_VC	(1 << 1)
#define ATH_USB_OTGSC_VD	(1 << 0)

/* USB Mode register defines */
#define ATH_USB_MODE_VBPS	(1 << 5)	/* VBUS Power select */
#define ATH_USB_MODE_SDIS	(1 << 4)	/* Stream Disable */
#define ATH_USB_MODE_SLOM	(1 << 3)	/* Setup Locout Mode */
#define ATH_USB_MODE_BES	(1 << 2)	/* Big Endian Select */
#define ATH_USB_MODE_CM_IDLE	(0x00)	/* Combination Host/Device */
#define ATH_USB_MODE_CM_DEV	(0x02)	/* Device Only Mode */
#define ATH_USB_MODE_CM_HOST	(0x03)	/* Host only mode */

#define ATH_USB_SET_HOST_MODE	(ATH_USB_MODE_CM_HOST | ATH_USB_MODE_SDIS)
//#define ATH_USB_SET_DEV_MODE	(ATH_USB_MODE_CM_DEV | ATH_USB_MODE_SDIS)
#define ATH_USB_SET_DEV_MODE	(ATH_USB_MODE_CM_DEV )

void *ath_usb_get_otg (void);
u8 usb_otg_set_status (u8 componant, u8 status);

#endif /* __LINUX_ATH_DEFS_H */
