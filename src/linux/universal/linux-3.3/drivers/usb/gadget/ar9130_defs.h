/*
 * Atheros AR9130 USB Controller Defines
 */

#ifndef __LINUX_AR9130_DEFS_H
#define __LINUX_AR9130_DEFS_H

#include <linux/platform_device.h>
#if defined (CONFIG_MACH_AR7240) || defined (CONFIG_MACH_HORNET)
#include <asm/mach-ar7240/ar7240.h>
#else
#include <asm/mach-ar7100/ar7100.h>
#endif
#include <linux/proc_fs.h>
#include <linux/delay.h>

#include <linux/usb.h>
#include <linux/usb/otg.h>

/*
 * AR9130 USB Registers
 */
struct ar9130_usb  {
    __u32   usbcmd;             /* Command register */
    __u32   usbsts;             /* Status register */
    __u32   usbintr;            /* Interrupt enable */
    __u32   usbfrmindx;         /* Frame index */
    __u32   ctrldssegment;      /* 4G segment selector */
    __u32   devaddr;            /* Device Address */
    __u32   ep_list_addr;       /* Endpoint List Address */
    __u32   reserved0[9];
    __u32   configflag;         /* Configured Flag register */
    __u32   portscx[8];         /* Port Status/Control x, x = 1..8 */
    __u32   otgsc;
    __u32   usbmode;            /* USB Host/Device mode */
    __u32   ep_setup_stat;      /* Endpoint Setup Status */
    __u32   ep_prime;           /* Endpoint Initialize */
    __u32   ep_flush;           /* Endpoint De-initialize */
    __u32   ep_status;          /* Endpoint Status */
    __u32   ep_complete;        /* Endpoint Interrupt On Complete */
    __u32   ep_ctrlx[16];       /* Endpoint Control, where x = 0.. 15 */
};

struct ar9130_otg_vars {
   u8   A_BUS_DROP;
   u8   A_BUS_REQ;
   u8   B_BUS_REQ;
   u8   HOST_UP;
   u8   DEVICE_UP;
   u8   A_SRP_DET;
   u32  A_SRP_TMR;
   u8   A_DATA_PULSE_DET;
   u32  TB_SE0_SRP_TMR;
   u8   B_SE0_SRP;
   s32  OTG_INT_STATUS;
   u8   A_BUS_RESUME;
   u8   A_BUS_SUSPEND;
   u8   A_BUS_SUSPEND_REQ;
   u8   A_CONN;
   u8   B_BUS_RESUME;
   u8   B_BUS_SUSPEND;
   u8   B_CONN;
   u8   A_SET_B_HNP_EN;
   u8   B_SESS_REQ;
   u8   B_SRP_DONE;
   u8   B_HNP_ENABLE;
   u8   A_CONNECTED;
   u8   B_DISCONNECTED;
   u8   CHECK_SESSION;
   u8   A_SUSPEND_TIMER_ON;
   /* A device specific timers */
   u32  TA_WAIT_VRISE_TMR;
   u32  TA_WAIT_BCON_TMR;
   u32  TA_AIDL_BDIS_TMR;
   u32  TA_BIDL_ADIS_TMR;
   /* B device specific timers */
   u32  TB_DATA_PLS_TMR;
   u32  TB_SRP_INIT_TMR;
   u32  TB_SRP_FAIL_TMR;
   u32  TB_VBUS_PLS_TMR;
   u32  TB_ASE0_BRST_TMR;
   u32  TB_A_SUSPEND_TMR;
   u32  TB_VBUS_DSCHRG_TMR;
};

/*
 * OTG Driver Information
 */
struct ar9130_otg {
    struct otg_transceiver  otg;
    struct work_struct      work;
    struct ar9130_otg_vars  state;
    struct otg_reg __iomem  *otg_reg;
    struct device           *dev;
    spinlock_t              arlock;
    unsigned int            irqnum;
    struct ehci_caps __iomem *cap_reg;
    struct ar9130_usb __iomem *usb_reg;
    void __iomem            *reg_base;
    irqreturn_t (*udc_isr)(int, void *, struct pt_regs *r);
    void                    *udc;
    u64                     rsrc_start;
    u64                     rsrc_len;
    u32                     enabled_ints;
    u32                     bVRISE_TIMEDOUT;
	struct echi_hcd *ehci;	/* Temp Added to Test HNP */
};

/*
 * AR9130 Debug functions
 */
#define AR9130_DEBUG_FUNCTION               (0x00000001)
#define AR9130_DEBUG_INTERRUPT              (0x00000002)
#define AR9130_DEBUG_ENDPOINT               (0x00000004)
#define AR9130_DEBUG_PORTSTATUS             (0x00000008)
#define AR9130_DEBUG_DEVICE                 (0x00000010)
#define AR9130_DEBUG_MEMORY                 (0x00000020)
#define AR9130_DEBUG_QUEUEHEAD              (0x00000040)
#define AR9130_DEBUG_DTD                    (0x00000080)
#define AR9130_DEBUG_OTG                    (0x00000100)


#ifdef AR9130_USB_DEBUG
#define ar9130_debug(level, format, arg...)             \
do {                                                    \
    if (level & ar9130_debug_level) {                   \
        printk(format, ##arg);                          \
    }                                                   \
} while (0)
#else
#define ar9130_debug(level, format, arg...)             \
    do { (void)(level); } while (0)
#endif

#define ar9130_debug_fn(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_FUNCTION, format, ##arg)
#define ar9130_debug_ep(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_ENDPOINT, format, ##arg)
#define ar9130_debug_ps(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_PORTSTATUS, format, ##arg)
#define ar9130_debug_int(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_INTERRUPT, format, ##arg)
#define ar9130_debug_dev(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_DEVICE, format, ##arg)
#define ar9130_debug_mem(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_MEMORY, format, ##arg)
#define ar9130_debug_qh(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_QUEUEHEAD, format, ##arg)
#define ar9130_debug_dtd(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_DTD, format, ##arg)
#define ar9130_debug_otg(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_OTG, format, ##arg)

#define ar9130_error(format, arg...)                    \
        printk(format, ##arg)
#define ar9130_warn(format, arg...)                     \
        printk(format, ##arg)

/* Device/Host Capability Reg */
#define AR9130_NON_EHCI_DCCPARAMS           (0x124)

/* Device/Host Timer Reg */
#define AR9130_NON_EHCI_TIMER0LD            (0x80)
#define AR9130_NON_EHCI_TIMER0CTRL          (0x84)
#define AR9130_NON_EHCI_TIMER1LD            (0x88)
#define AR9130_NON_EHCI_TIMER1CTRL          (0x8C)

/* Device/Host Operational Reg (non-ehci) */
#define AR9130_EHCI_EXT_BURSTSZ             (0x160)
#define AR9130_EHCI_EXT_TTCTRL              (0x15C)
#define AR9130_EHCI_EXT_USBMODE             (0x1A8)
#define AR9130_EHCI_EXT_TXFILL              (0x164)
#define AR9130_EHCI_EXT_ULPI                (0x170)
#define AR9130_EHCI_EXT_OTGSC               (0x1A4)

#if defined (CONFIG_MACH_AR7240) || defined (CONFIG_MACH_HORNET)

#define AR9130_RESET_USB_HOST               (AR7240_RESET_USB_HOST)
#define AR9130_RESET_USB_PHY                (AR7240_RESET_USB_PHY)
#define AR9130_RESET_USBSUS_OVRIDE	    (AR7240_RESET_USBSUS_OVRIDE)
#define AR9130_USB_MODE                     (AR7240_USB_MODE)

#define AR9130_RESET                        (AR7240_RESET_BASE + 0x1C)
#define AR9130_USB_CONFIG                   (AR7240_USB_CONFIG_BASE + 0x4)
#define AR9130_USB_FLADJ_VAL                (AR7240_USB_CONFIG_BASE)

#define ar9130_reg_rmw_set(_reg,_mask)      ar7240_reg_rmw_set(_reg,_mask)
#define ar9130_reg_rmw_clear(_reg,_mask)    ar7240_reg_rmw_clear(_reg,_mask)
#define ar9130_reg_wr(_phys,_val)           ar7240_reg_wr(_phys,_val)
#define ar9130_reg_rd(_phys)                ar7240_reg_rd(_phys)

#else

#define AR9130_RESET_USB_HOST               (AR7100_RESET_USB_HOST)
#define AR9130_RESET_USB_PHY                (AR7100_RESET_USB_PHY)
#define AR9130_RESET_USBSUS_OVRIDE	    (AR7100_RESET_USBSUS_OVRIDE)

#define AR9130_RESET                        (AR7100_RESET_BASE + 0x1C)
#define AR9130_USB_CONFIG                   (AR7100_USB_CONFIG_BASE + 0x4)
#define AR9130_USB_FLADJ_VAL                (AR7100_USB_CONFIG_BASE)

#define ar9130_reg_rmw_set(_reg,_mask)      ar7100_reg_rmw_set(_reg,_mask)
#define ar9130_reg_rmw_clear(_reg,_mask)    ar7100_reg_rmw_clear(_reg,_mask)
#define ar9130_reg_wr(_phys,_val)           ar7100_reg_wr(_phys,_val)
#define ar9130_reg_rd(_phys)                ar7100_reg_rd(_phys)

#endif
/* Operational Modes */
    enum ath_hc_op_modes {
        AR9130_OP_MODE_MPH,
        AR9130_OP_MODE_DEV,
        AR9130_OP_MODE_SPH,
        AR9130_OP_MODE_OTG,
    };

/* Transceiver Modes */
enum ath_hc_trans {
    AR9130_TRANS_UTMI,
    AR9130_TRANS_ULPI,
    AR9130_TRANS_SERIAL,
};


/* OTG Status/Control Reigser Defines */
#define AR9130_OTGSC_IMASK      (0x5F000000)
#define AR9130_OTGSC_SMASK      (0x005F0000)
#define AR9130_OTGSC_DPIE       (0x40000000)    /* Data-line pulsing IE */
#define AR9130_OTGSC_1MSIE      (0x20000000)
#define AR9130_OTGSC_BSEIE      (0x10000000)    /* B-session end IE */
#define AR9130_OTGSC_BSVIE      (0x08000000)    /* B-session valid IE */
#define AR9130_OTGSC_ASVIE      (0x04000000)    /* A-session valid IE */
#define AR9130_OTGSC_AVVIE      (0x02000000)    /* A-V-bus valid IE */
#define AR9130_OTGSC_IDIE       (0x01000000)    /* OTG ID IE */
#define AR9130_OTGSC_DPIS       (0x00400000)    /* Data-line pulsing IS */
#define AR9130_OTGSC_1MSIS      (0x00200000)
#define AR9130_OTGSC_BSEIS      (0x00100000)    /* B-session end IS */
#define AR9130_OTGSC_BSVIS      (0x00080000)    /* B-session valid IS */
#define AR9130_OTGSC_ASVIS      (0x00040000)    /* A-session valid IS */
#define AR9130_OTGSC_AVVIS      (0x00020000)    /* A-Vbus valid IS */
#define AR9130_OTGSC_IDIS       (0x00010000)    /* OTG ID IS */

/* OTG status bit masks */
#define AR9130_OTGSC_DPS        (1 << 14)
#define AR9130_OTGSC_1MST       (1 << 13)
#define AR9130_OTGSC_BSE        (1 << 12)       /* B-session end */
#define AR9130_OTGSC_BSV        (1 << 11)       /* B-session valid */
#define AR9130_OTGSC_ASV        (1 << 10)       /* A-session valid */
#define AR9130_OTGSC_AVV        (1 << 9)        /* A-Vbus Valid */
#define AR9130_OTGSC_ID         (1 << 8)        /* OTG ID */

/* OTG control bit masks */
#define AR9130_OTGSC_CTL_BITS   (0x2F)
#define AR9130_OTGSC_HABA       (1 << 7)        /*hw assisted data pulse bits*/
#define AR9130_OTGSC_HADP       (1 << 6)        /*hw assisted data pulse bits*/
#define AR9130_OTGSC_IDPU       (1 << 5)
#define AR9130_OTGSC_BHEN       (1 << 5)        /* B Host Enable */
#define AR9130_OTGSC_DP         (1 << 4)
#define AR9130_OTGSC_OT         (1 << 3)
#define AR9130_OTGSC_HAAR       (1 << 2)
#define AR9130_OTGSC_VC         (1 << 1)
#define AR9130_OTGSC_VD         (1 << 0)

/* USB Mode register defines */
#define AR9130_USBMODE_VBPS     (1 << 5)        /* VBUS Power select */
#define AR9130_USBMODE_SDIS     (1 << 4)        /* Stream Disable */
#define AR9130_USBMODE_SLOM     (1 << 3)        /* Setup Locout Mode */
#define AR9130_USBMODE_BES      (1 << 2)        /* Big Endian Select */
#define AR9130_USBMODE_CM_IDLE  (0x00)          /* Combination Host/Device */
#define AR9130_USBMODE_CM_DEV   (0x02)          /* Device Only Mode */
#define AR9130_USBMODE_CM_HOST  (0x03)          /* Host only mode */

#define AR9130_SET_HOST_MODE    (AR9130_USBMODE_CM_HOST | AR9130_USBMODE_SDIS)
//#define AR9130_SET_DEV_MODE     (AR9130_USBMODE_CM_DEV | AR9130_USBMODE_SDIS)
#define AR9130_SET_DEV_MODE     (AR9130_USBMODE_CM_DEV )

void *ar9130_get_otg (void);
u8 usb_otg_set_status (u8 componant, u8 status);

#endif /* __LINUX_AR9130_DEFS_H */
