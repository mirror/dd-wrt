#ifndef __LINUX_ATH_USB_OTG_H
#define __LINUX_ATH_USB_OTG_H

#include "../gadget/ath_defs.h"

#define USB_lock()          spin_lock_irqsave(&ath_usb->arlock, arflags)
#define USB_unlock()        spin_unlock_irqrestore(&ath_usb->arlock, arflags)

/* Informational Request/Set Types */
#define  USB_OTG_A_SET_B_HNP_ENABLE         (0x01)
#define  USB_OTG_B_HNP_ENABLE               (0x02)
#define  USB_OTG_A_BUS_REQ                  (0x03)
#define  USB_OTG_B_BUS_REQ                  (0x04)
#define  USB_OTG_A_VBUS_ON                  (0x05)
#define  USB_OTG_B_SRP_REQ                  (0x06)
#define  USB_OTG_B_HNP_REQ                  (0x07)
#define  USB_OTG_B_CONN                     (0x08)
#define  USB_OTG_B_BUS_SUSPEND              (0x09)
#define  USB_OTG_A_BUS_SUSPEND              (0x0A)
#define  USB_OTG_J_ATTACH                   (0x0B)
#define  USB_OTG_A_BUS_RESUME               (0x0C)
#define  USB_OTG_A_CONN                     (0x0D)
#define  USB_OTG_STATE                      (0x0E)
#define  USB_OTG_A_BUS_SUSPEND_REQ          (0x0F)
#define  USB_OTG_B_BUS_RESUME               (0x10)

/* no action require with this change */
#define  USB_OTG_A_BUS_DROP                 (0x11)
#define  USB_OTG_HOST_ACTIVE                (0x12)
#define  USB_OTG_DEVICE_ACTIVE              (0x13)

/* Entry for proc */
#define MASK_MODE			    (0x04)
#define MASK_USBID			    (0x08)
#define MASK_ASV			    (1 << 10)
#define MASK_BSV			    (1 << 11)
/*
**  A-DEVICE timing  constants
*/

/* Wait for VBUS Rise  */
#define TA_WAIT_VRISE       (102)    /* a_wait_vrise 100 ms, section: 6.6.5.1 */

/* Wait for B-Connect */
#define TA_WAIT_BCON  		(5002) 	/* a_wait_bcon > 1 sec, section: 6.6.5.2 
                                    ** This is only used to get out of A_WAIT_BCON state 
                                    ** state if there was no connection for 
                                    ** these many milliseconds 
                                    */

/* A-Idle to B-Disconnect */
/* It is necessary for this timer to be more than 750 ms because of a bug in OPT
test 5.4 in which B OPT disconnects after 750 ms instead of 75ms as stated in the
test description */
#define TA_AIDL_BDIS        (800)    /* a_suspend minimum 200 ms, section: 6.6.5.3 */

/* B-Idle to A-Disconnect */
#define TA_BIDL_ADIS       (12)     /* 3 to 200 ms */

/*
**  B-DEVICE timing constants.
*/

/* SE0 Time Before SRP */
#define TB_SE0_SRP          (5)       /* b_idle 2 ms 5.3.2 */

/* Data-Line Pulse Time*/
#define TB_DATA_PLS        (9)      /* b_srp_init 5 to 10 ms, section: 5.3.3 */

#define TB_DATA_PLS_MIN      (5)       /* b_srp_init 5 to 10 ms, section: 5.3.3 */
#define TB_DATA_PLS_MAX      (10)      /* b_srp_init 5 to 10 ms, section: 5.3.3 */

/* SRP Initiate Time  */
#define TB_SRP_INIT        (102)    /* b_srp_init 100 ms, section: 5.3.8 */

/* SRP Fail Time  */
#define TB_SRP_FAIL        (7002)    /* b_srp_init 5 to 30 sec,section: 6.8.2.2*/

/* VBus timer */
/* VBus timer */
#define TB_VBUS_PLS        (10)     /* sufficient time to keep Vbus pulsing asserted */ 

/* Discharge timer */
/* This time should be less than 10ms. It varies from system to
 system. ON ARCADE boards it has been seen that with Philips PHY 8ms
 is enough time for the VBUS to discharge */
#define TB_VBUS_DSCHRG		8		

/* A-SE0 to B-Reset  */
#define TB_ASE0_BRST        (6)       /* b_wait_acon 3.125, section: 6.8.2.4 */

/* A bus suspend timer before we can switch to B_WAIT_ACON */
#define TB_A_SUSPEND       (60)

#define TB_BUS_RESUME      (12)


/* Timer Macros */
#define TB_SRP_FAIL_TMR_ON(s,t)          ((s)->TB_SRP_FAIL_TMR = t)
#define TB_SRP_FAIL_TMR_OFF(s)           ((s)->TB_SRP_FAIL_TMR = -1)
#define TB_SRP_FAIL_TMR_EXPIRED(s)     ((s)->TB_SRP_FAIL_TMR ==  0)

#define TB_SRP_INIT_TMR_ON(s,t)         ((s)->TB_SRP_INIT_TMR = t)
#define TB_SRP_INIT_TMR_OFF(s)          ((s)->TB_SRP_INIT_TMR = -1)
#define TB_SRP_INIT_TMR_EXPIRED(s)       ((s)->TB_SRP_INIT_TMR == 0)

#define TB_DATA_PLS_TMR_ON(s,t)          ((s)->TB_DATA_PLS_TMR = t)
#define TB_DATA_PLS_TMR_OFF(s)          ((s)->TB_DATA_PLS_TMR = -1)
#define TB_DATA_PLS_TMR_EXPIRED(s)       ((s)->TB_DATA_PLS_TMR == 0)

#define TB_VBUS_PLS_TMR_ON(s,t) 			((s)->TB_VBUS_PLS_TMR = t)
#define TB_VBUS_PLS_TMR_OFF(s) 			((s)->TB_VBUS_PLS_TMR = -1)
#define TB_VBUS_PLS_TMR_EXPIRED(s) 		((s)->TB_VBUS_PLS_TMR == 0)

#define TB_ASE0_BRST_TMR_ON(s,t)        ((s)->TB_ASE0_BRST_TMR = t)
#define TB_ASE0_BRST_TMR_OFF(s)          ((s)->TB_ASE0_BRST_TMR = -1)
#define TB_ASE0_BRST_TMR_EXPIRED(s)    ((s)->TB_ASE0_BRST_TMR == 0)

#define TB_A_SUSPEND_TMR_ON(s,t)  		((s)->TB_A_SUSPEND_TMR = t)
#define TB_A_SUSPEND_TMR_OFF(s)    		((s)->TB_A_SUSPEND_TMR = -1)
#define TB_A_SUSPEND_TMR_EXPIRED(s) 	((s)->TB_A_SUSPEND_TMR == 0)

#define TB_VBUS_DSCHRG_TMR_ON(s,t)  		((s)->TB_VBUS_DSCHRG_TMR = t)
#define TB_VBUS_DSCHRG_TMR_OFF(s)    		((s)->TB_VBUS_DSCHRG_TMR = -1)
#define TB_VBUS_DSCHRG_TMR_EXPIRED(s) 	((s)->TB_VBUS_DSCHRG_TMR == 0)

#define TA_WAIT_VRISE_TMR_ON(s,t)        ((s)->TA_WAIT_VRISE_TMR = t)
#define TA_WAIT_VRISE_TMR_OFF(s)       ((s)->TA_WAIT_VRISE_TMR = -1)
#define TA_WAIT_VRISE_TMR_EXPIRED(s)    ((s)->TA_WAIT_VRISE_TMR == 0)

#define TA_WAIT_BCON_TMR_ON(s,t)        ((s)->TA_WAIT_BCON_TMR = t)
#define TA_WAIT_BCON_TMR_OFF(s)          ((s)->TA_WAIT_BCON_TMR = -1)
#define TA_WAIT_BCON_TMR_EXPIRED(s)    ((s)->TA_WAIT_BCON_TMR == 0)

#define TA_AIDL_BDIS_TMR_ON(s,t)        ((s)->TA_AIDL_BDIS_TMR = t)
#define TA_AIDL_BDIS_TMR_OFF(s)          ((s)->TA_AIDL_BDIS_TMR = -1)
#define TA_AIDL_BDIS_TMR_EXPIRED(s)    ((s)->TA_AIDL_BDIS_TMR == 0)

#define TA_BIDL_ADIS_TMR_ON(s,t)  		((s)->TA_BIDL_ADIS_TMR = t)
#define TA_BIDL_ADIS_TMR_OFF(s)    		((s)->TA_BIDL_ADIS_TMR = -1)
#define TA_BIDL_ADIS_TMR_EXPIRED(s) 	((s)->TA_BIDL_ADIS_TMR == 0)

#define TB_SE0_SRP_TMR_ON(s,t)           ((s)->TB_SE0_SRP_TMR = t)
#define TB_SE0_SRP_TMR_OFF(s)          ((s)->TB_SE0_SRP_TMR = -1)
#define CHECK_TB_SE0_SRP_TMR_OFF(s)    ((s)->TB_SE0_SRP_TMR == -1)
#define TB_SE0_SRP_TMR_EXPIRED(s)       ((s)->TB_SE0_SRP_TMR == 0)

#define A_SRP_TMR_ON(s,t)              ((s)->A_SRP_TMR = t)
#define A_SRP_TMR_OFF(s)                ((s)->A_SRP_TMR = -1)
#define A_SRP_TMR_EXPIRED(s)             ((s)->A_SRP_TMR == 0)


#undef TRUE
#define TRUE    1
#undef FALSE
#define FALSE   0

#define LOG_TIMER(str)	/*uartputs(str)*/

/* define the hardware specific registers */
#define START_TIMER(x)                                                      \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_1MSIE), &(x)->otgsc);                     \
      (ath_usb->enabled_ints |= ATH_USB_OTGSC_1MSIE);                         \
   }


#define STOP_TIMER(x)                                                       \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel ((temp & ~ATH_USB_OTGSC_1MSIE), &(x)->otgsc);                   \
      (ath_usb->enabled_ints &= ~ATH_USB_OTGSC_1MSIE);                        \
   }

/*
 * Interrupt status register signals
 */
#define ID_CHG(mask)          (mask & ATH_USB_OTGSC_IDIS)
#define SESS_VLD_CHG(mask)    (mask & (ATH_USB_OTGSC_ASVIS | ATH_USB_OTGSC_BSVIS))
#define A_VBUS_CHG(mask)      (mask & ATH_USB_OTGSC_AVVIS)


/*
 * status register signals
 */
#define SESS_VLD_FALSE(x)     \
   (!(readl(&(x)->otgsc) & (ATH_USB_OTGSC_BSV | ATH_USB_OTGSC_ASV)))
   
#define SESS_VLD(x)           \
   (readl(&(x)->otgsc) & (ATH_USB_OTGSC_BSV | ATH_USB_OTGSC_ASV))
   
#define B_SESS_END(x)         \
   (readl(&(x)->otgsc) & ATH_USB_OTGSC_BSE)

#define ID(x)                 \
   (readl(&(x)->otgsc) & ATH_USB_OTGSC_ID)
   
#define ID_FALSE(x)           \
   (!(readl(&(x)->otgsc) & ATH_USB_OTGSC_ID))
   
#define A_VBUS_VLD(x)         \
   (readl(&(x)->otgsc) & ATH_USB_OTGSC_AVV)

#define A_VBUS_VLD_FALSE(x)   \
   (!(readl(&(x)->otgsc) &  ATH_USB_OTGSC_AVV))
   
#define STABLE_J_SE0_LINE_STATE(x) (TRUE)
 
#define SET_STATE(stat, ordinal)		 ath_usb->otg.state = (stat)

/*
 * OTG control register signals
 */
/* discharge the VBUS */
#define VBUS_DSCHG_ON(x)                                                    \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_VD), &(x)->otgsc);                        \
   }
   
#define VBUS_DSCHG_OFF(x)                                                   \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp & ~ATH_USB_OTGSC_VD), &(x)->otgsc);                       \
   }

/* TURN ON/OFF VBUS charging */
#define VBUS_CHG_ON(x)                                                      \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_VC), &(x)->otgsc);                        \
   }
   
#define VBUS_CHG_OFF(x)                                                     \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp & ~ATH_USB_OTGSC_VC), &(x)->otgsc);                       \
   }

#define EHCI_PORTSCX_W1C_BITS       (0x2A)
#define VBUS_ON(x)                                                          \
   {                                                                        \
      u32  temp;                                                            \
      temp = readl(&(x)->portscx[0]);                                   \
      temp &= ~EHCI_PORTSCX_W1C_BITS;                                       \
      writel((temp | PORT_POWER), &(x)->portscx[0]);                    \
   }
   
#define VBUS_OFF(x)                                                         \
   {                                                                        \
      u32  temp;                                                            \
      temp = readl(&(x)->portscx[0]);                                   \
      temp &= ~EHCI_PORTSCX_W1C_BITS;                                       \
      writel ((temp & ~PORT_POWER), &(x)->portscx[0]);                  \
   }

#define DM_HIGH_ON(x)

#define DM_HIGH_OFF(x)

/***************************************************************
SG 06/21/2004
During SRP, software controls the duration of data pulse.
This control depends upon the system's interrupt latency
because in a system with large interrupt time, software
execution can take longer and data pulse may exceed
the max 10 mili second deadline. Latest hardware provides
a way that instead of setting D+ ON, we can just set the
HADP bit in hardware. This bit will set the D+ On and
after 7 mili second, it will turn off the DP.
***************************************************************/

#define DP_HIGH_ON(x)                                                       \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_HADP), &(x)->otgsc);                      \
   }
   
#define DP_HIGH_OFF(x)                                                      \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp & ~ATH_USB_OTGSC_HADP), &(x)->otgsc);                     \
   }

/*
#define DP_HIGH_ON(x)         \
   { \
      uint_32  temp; \
      temp = ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC & ~ATH_USB_OTGSC_INTERRUPT_STATUS_BITS_MASK); \
      ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC = (temp | ATH_USB_OTGSC_DP)); \
   }
   
#define DP_HIGH_OFF(x)        \
   { \
      uint_32  temp; \
      temp = ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC & ~ATH_USB_OTGSC_INTERRUPT_STATUS_BITS_MASK); \
      ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC = (temp & ~ATH_USB_OTGSC_DP)); \
   }
*/
/* EHCI RESERVERED BITS */
#define EHCI_PORTSCX_LINE_STATUS_BITS           (3 << 10)
#define EHCI_PORTSCX_LINE_STATUS_SE0            (0 << 0)
#define EHCI_PORTSCX_LINE_STATUS_KSTATE         (1 << 9)

#define SE0(x)                                                              \
   ((readl(&(x)->portscx[0]) & EHCI_PORTSCX_LINE_STATUS_BITS) ==        \
            EHCI_PORTSCX_LINE_STATUS_SE0)
      
#define JSTATE(x)    (TRUE)

#define KSTATE(x)                                                           \
   ((readl(&(x)->portscx[0]) & EHCI_PORTSCX_LINE_STATUS_BITS) ==        \
            EHCI_PORTSCX_LINE_STATUS_KSTATE)

/*hardware assistance can be used by setting the auto reset
bit in OTG register. This will help meet 1 ms reset
requirement under 5.4 OPT complaince test */
#define AUTO_RESET_ON(x)                                                    \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_HAAR), &(x)->otgsc);                      \
   }

#define AUTO_RESET_OFF(x)                                                   \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp & ~ATH_USB_OTGSC_HAAR), &(x)->otgsc);                     \
   }

#define IS_AUTO_RESET_ON(x,y)                                               \
   {                                                                        \
      u32 temp;                                                             \
      temp = readl(&(x)->otgsc);                                            \
      y = ((temp & ATH_USB_OTGSC_HAAR)? TRUE : FALSE);                       \
   }

/*hardware assistance can be used by setting the auto reset
bit in OTG register. This will help meet 1 ms reset
requirement under 5.4 OPT complaince test */
#define AUTO_HNP_ON(x)                                                      \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_HABA), &(x)->otgsc);                      \
   }

#define AUTO_HNP_OFF(x)                                                     \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp & ~ATH_USB_OTGSC_HABA), &(x)->otgsc);                     \
   }

#define IS_AUTO_HNP_ON(x,y)                                                 \
   {                                                                        \
      u32 temp;                                                             \
      temp = readl(&(x)->otgsc);                                            \
      y = ((temp & ATH_USB_OTGSC_HABA)? TRUE : FALSE);                       \
   }

#define  ATH_USB_MODE_DEV                 (0x00000002)

#define IS_DEVICE_MODE_ON(x,y)                                              \
   {                                                                        \
      u32 temp;                                                             \
      temp = (readl(&(x)->usbmode) & 0x3);                                  \
      y = ((temp == ATH_USB_MODE_DEV)? TRUE : FALSE);                        \
   }

/*
 * patch_3 is enabled to make software compatable with
 * revision 4.0 of hardware
 */
#ifdef PATCH_3
    /*
     * WEB20040409 below changed to reflect  ATH_USB_OTGSC_B_HOST_EN is no 
     * longer used. 
	      temp = ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC & ~ATH_USB_OTGSC_INTERRUPT_STATUS_BITS_MASK); \
	      ((x)->REGISTERS.OPERATIONAL_HOST_REGISTERS.OTGSC = (temp | ATH_USB_OTGSC_B_HOST_EN));
          */
	/* to this */ 
	#define PULL_UP_PULL_DOWN_HOST_CTRL(x)                                  \
	   {                                                                    \
	      u32  temp;                                                        \
	      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                \
	      writel ((temp),&(x)->otgsc);                                      \
	 	  }

#else
	#define PULL_UP_PULL_DOWN_HOST_CTRL(x)                                  \
	   {                                                                    \
	      u32  temp;                                                        \
	      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                \
	      writel((temp | ATH_USB_OTGSC_BHEN), &(x)->otgsc);                  \
	   }
#endif

#define PULL_UP_PULL_DOWN_DEVICE_CTRL(x)                                    \
   {                                                                        \
      u32  temp;                                                            \
      temp = (readl(&(x)->otgsc) & ~ATH_USB_OTGSC_SMASK);                    \
      writel((temp | ATH_USB_OTGSC_OT), &(x)->otgsc);                        \
   }

#define PULL_UP_PULL_DOWN_IDLE(x)

#define OTG_HOST_INIT(x, y)                                                 \
   ath_usb_host_resume((x));                                                 \
   PULL_UP_PULL_DOWN_HOST_CTRL(y);                                          \
   (x)->state.HOST_UP = TRUE;

#define OTG_DEVICE_INIT(x, y)                                               \
   if (!((x)->state.DEVICE_UP)) { \
      (x)->state.DEVICE_UP = TRUE; \
      printk ("device up\n"); \
      gadget_resume(ath_usb);  \
   } \
   if (((x)->otg.state) != OTG_STATE_B_IDLE) { \
      PULL_UP_PULL_DOWN_DEVICE_CTRL(y); \
   }

#define EHCI_IAA_JIFFIES        (HZ/100)        /* arbitrary; ~10 msec */
#define EHCI_IO_JIFFIES         (HZ/10)         /* io watchdog > irq_thresh */
#define EHCI_ASYNC_JIFFIES      (HZ/20)         /* async idle timeout */
#define EHCI_SHRINK_JIFFIES     (HZ/200)        /* async qh unlink delay */

#endif /* __LINUX_ATH_USB_OTG_H */
