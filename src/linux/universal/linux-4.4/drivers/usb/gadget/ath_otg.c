#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/usb_ch9.h>
#include <linux/usb_gadget.h>
#include <linux/usb.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>

#include <asm/irq.h>
#include <asm/io.h>

#include "../core/hcd.h"
#include "../host/ehci.h"
#define ATH_USB_DEBUG
#include "../gadget/ath_usb_otg.h"
#include "../gadget/ath_usb_udc.h"

#define ATH_USB_HOST_REG_OFFSET	(0x1A4)
#define ATH_USB_CAP_REG_OFFSET	(0x100)
#define ATH_USB_DRIVER_NAME	"ath_usb_otg"

#ifdef ATH_USB_DEBUG
static int ath_usb_debug_level = (
//	ATH_USB_DEBUG_FUNCTION		|
//	ATH_USB_DEBUG_INTERRUPT		|
//	ATH_USB_DEBUG_ENDPOINT		|
//	ATH_USB_DEBUG_PORTSTATUS	|
//	ATH_USB_DEBUG_DEVICE		|
//	ATH_USB_DEBUG_MEMORY		|
//	ATH_USB_DEBUG_QUEUEHEAD		|
//	ATH_USB_DEBUG_DTD		|
//	ATH_USB_DEBUG_OTG		|
	0x0
);
#endif

static struct ath_usb_otg *ath_usb_otg = NULL;
static int ath_usb_start_hnp(struct otg_transceiver *dev);
static int ath_usb_start_srp(struct otg_transceiver *dev);
static int ath_usb_set_power(struct otg_transceiver *dev, unsigned mA);
static int ath_usb_host_suspend(struct ath_usb_otg *ath_usb);
static int ath_usb_host_resume(struct ath_usb_otg *ath_usb);
static void _usb_otg_state_machine (struct ath_usb_otg *ath_usb);
static int gadget_resume(struct ath_usb_otg *ath_usb);
static int gadget_suspend(struct ath_usb_otg *ath_usb);
void _usb_otg_process_b_idle (struct ath_usb_otg *ath_usb);
void _usb_otg_process_a_idle (struct ath_usb_otg *ath_usb);
void _usb_otg_process_a_wait_vfall(struct ath_usb_otg *ath_usb);
void _usb_otg_process_b_device_session_valid (struct ath_usb_otg *ath_usb);
void _usb_otg_process_a_wait_vrise(struct ath_usb_otg *ath_usb);
void _usb_otg_reset_state_machine (struct ath_usb_otg *ath_usb);

void *ath_usb_get_otg (void)
{
    if (!ath_usb_otg) {
        return (NULL);
    }
    return (ath_usb_otg);
}

static int enable_vbus_draw (struct ath_usb_otg *ath_usb, int value)
{
    /* Nothing to do */
    return (0);
}

static int power_down (struct ath_usb_otg *ath_usb)
{
    return (0);
}

static void ath_usb_reset_otg_state (struct ath_usb_otg *ath_usb)
{
    struct otg_transceiver *otg = &ath_usb->otg;

    otg->state      = OTG_STATE_A_IDLE;
    otg->default_a  = 0;
    otg->host       = NULL;
    otg->gadget     = NULL;
    otg->port_status= 0;
    otg->port_change= 0;
    _usb_otg_reset_state_machine (ath_usb);
    ath_usb->state.HOST_UP = 0;
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_reset_state_machine
* Returned Value : TRUE or FALSE
* Comments       :
*     Reset the state machine to its orginal state.
*END*--------------------------------------------------------------------*/
void  _usb_otg_reset_state_machine (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars  *s_ptr;

    s_ptr = &ath_usb->state;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_reset_state_machine");
#endif

    s_ptr->OTG_INT_STATUS = 0;
    s_ptr->A_BUS_RESUME = 0;
    s_ptr->A_BUS_SUSPEND = 0;
    s_ptr->A_BUS_SUSPEND_REQ = 0;
    s_ptr->A_CONN = 0;
    s_ptr->B_BUS_RESUME = 0;
    s_ptr->B_BUS_SUSPEND = 0;
    s_ptr->B_CONN = 0;
    s_ptr->A_SET_B_HNP_EN = 0;
    s_ptr->B_SESS_REQ = 0;
    s_ptr->B_SRP_DONE = 0;
    s_ptr->B_HNP_ENABLE = 0;
    s_ptr->A_CONNECTED = 0;
    s_ptr->B_DISCONNECTED = 0;
    s_ptr->CHECK_SESSION = TRUE;
    s_ptr->A_SUSPEND_TIMER_ON = 0;

    s_ptr->B_BUS_REQ = 0;
    s_ptr->A_BUS_REQ = 1;
    s_ptr->A_BUS_DROP = 0;

    TB_SE0_SRP_TMR_OFF(s_ptr);
    A_SRP_TMR_OFF(s_ptr);

    TA_WAIT_VRISE_TMR_OFF(s_ptr);
    TA_WAIT_BCON_TMR_OFF(s_ptr);
    TA_AIDL_BDIS_TMR_OFF(s_ptr);
    TA_BIDL_ADIS_TMR_OFF(s_ptr);

    TB_DATA_PLS_TMR_OFF(s_ptr);
    TB_SRP_INIT_TMR_OFF(s_ptr);
    TB_SRP_FAIL_TMR_OFF(s_ptr);
    TB_VBUS_PLS_TMR_OFF(s_ptr);
    TB_ASE0_BRST_TMR_OFF(s_ptr);
    TB_A_SUSPEND_TMR_OFF(s_ptr);
    TB_VBUS_DSCHRG_TMR_OFF(s_ptr);

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_reset_state_machine, SUCCESSFUL");
#endif

}
/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_exceptions
* Returned Value : TRUE or FALSE
* Comments       :
*     process the  exception signals and switch  state and return TRUE.
*END*--------------------------------------------------------------------*/
u8  _usb_otg_process_exceptions (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars      *s_ptr;
    struct ath_usb_usb __iomem   *usb_reg;
    u8                          previous_state;
    u8                          state;
    u32                         otg_int_status;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_exceptions");
#endif

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;
    state = ath_usb->otg.state;
    otg_int_status = s_ptr->OTG_INT_STATUS;

    /* Check exception for A device */
    previous_state = state;

    if ((state < OTG_STATE_B_IDLE) || (state > OTG_STATE_B_HOST)) {
        /*
         ** Exceptions are as follows.
         ** 1. Transition to B_IDLE when id = TRUE.
         ** 2. Transition to A_WAIT_VFALL when V_BUS is voltage is fallen below
         ** 3. 4 VDC or not able to set the V_BUS voltage.
         ** 4. If a_wait_b_con_timout  is set.
         ** 5. If  a_aidl_bdis_tmout  is set.
         */

        /*      if(m1 || m2 || m3 || m4 || m5) */

        if ((ID_CHG(otg_int_status) && ID(usb_reg))||
                ((A_VBUS_CHG(otg_int_status) && A_VBUS_VLD_FALSE(usb_reg)) ) ||
                (SESS_VLD_CHG(otg_int_status) && SESS_VLD_FALSE(usb_reg)) ||
                (TA_WAIT_VRISE_TMR_EXPIRED(s_ptr)) ||
                (TA_WAIT_BCON_TMR_EXPIRED(s_ptr)) ||
                (TA_AIDL_BDIS_TMR_EXPIRED(s_ptr)))

        {
#ifndef PERIPHERAL_ONLY
            if ((state == OTG_STATE_A_IDLE) && (ID_CHG(otg_int_status) && ID(usb_reg)))
            {
                SET_STATE(OTG_STATE_B_IDLE, "F ");
                s_ptr->B_BUS_REQ = FALSE;
                _usb_otg_process_b_idle(ath_usb);
            }
            /* change made after I2C code merge the line containing && (state != A_WAIT_VRISE)
               should work for I2C and non-I2C hardware.
               */
            else if ((state != OTG_STATE_A_IDLE) && (state != OTG_STATE_A_WAIT_VFALL))

            {
#if 0
                if (TA_WAIT_VRISE_TMR_EXPIRED(s_ptr))
                    uartputs("\nTA_WAIT_VRISE_TMR_EXPIRED(s_ptr) == TRUE");
                else
                    uartputs("\nTA_WAIT_VRISE_TMR_EXPIRED(s_ptr) == FALSE");
                if (A_VBUS_CHG(otg_int_status))
                    uartputs("\nA_VBUS_CHG(otg_int_status) == TRUE");
                else
                    uartputs("\nA_VBUS_CHG(otg_int_status) == FALSE");
                if (A_VBUS_VLD_FALSE(usb_reg))
                    uartputs("\nA_VBUS_VLD_FALSE() == TRUE");
                else
                    uartputs("\nA_VBUS_VLD_FALSE() == FALSE");
#endif

                if ((TA_WAIT_VRISE_TMR_EXPIRED(s_ptr)) ||
                        (A_VBUS_CHG(otg_int_status) && A_VBUS_VLD_FALSE(usb_reg)))
                {
                    // TODO ath_usb_service(ath_usb, USB_OTG_OVER_CURRENT);
                }

#ifdef HNP_HARDWARE_ASSISTANCE
                /*set the HABA bit off in OTGSC to disable hardware assitance */
                AUTO_HNP_OFF(usb_reg);
#endif

                SET_STATE(OTG_STATE_A_WAIT_VFALL, "s ");

                if (TA_WAIT_BCON_TMR_EXPIRED(s_ptr))
                {
                    TA_WAIT_BCON_TMR_OFF(s_ptr);
                    // TODO ath_usb_service(ath_usb, USB_OTG_NO_CONNECTION);
                }

                VBUS_CHG_OFF(usb_reg);
                VBUS_OFF(usb_reg);
                _usb_otg_process_a_wait_vfall(ath_usb);
            }

#endif
        }
    } else {
        /*
         ** Process B- device exceptions
         */
        if ((ID_CHG(otg_int_status) && ID_FALSE(usb_reg)) ||
                (SESS_VLD_CHG(otg_int_status) && SESS_VLD_FALSE(usb_reg) && ath_usb->otg.state != OTG_STATE_B_SRP_INIT))
        {
            VBUS_CHG_OFF(usb_reg);
            VBUS_OFF(usb_reg);
            s_ptr->B_BUS_REQ = FALSE;
            if (ID(usb_reg) ) {
                /************************************************************
                  SG 8/18/2003.
                  When software is about to move to B_IDLE state, it should
                  generate an HNP failure message if it is coming from
                  B_WAIT_ACON state. This is necessary to pass OPT on
                  test 5.9 because on some OTG PHY cards, VBUS can drop
                  faster (before B_ASE0_BRST_TMOUT timer expires) and
                  we can get a session valid interrupt. This interrupt
                  has caused the software to come here and it should
                  post the message to application saying that HNP has
                  failed.
                 ************************************************************/
                if ((state == OTG_STATE_B_WAIT_ACON) && s_ptr->B_HNP_ENABLE)
                {
                    // TODO ath_usb_service(ath_usb, USB_OTG_HNP_FAILED);
                }

                /*if we are not running a suspend timer and waiting for HNP
                  to finish, we should move to B_IDLE state */
                //if(!((state == B_PERIPHERAL) && s_ptr->A_SUSPEND_TIMER_ON))
                {
                    /* proceed to B_IDLE state */

                    SET_STATE(OTG_STATE_B_IDLE, "G ");
                    PULL_UP_PULL_DOWN_IDLE(usb_reg);
                    _usb_otg_process_b_idle(ath_usb);

                }
            } else {
#ifndef PERIPHERAL_ONLY
                SET_STATE(OTG_STATE_A_IDLE, "H ");
                /* A unique case where we need to set this to TRUE */
                s_ptr->A_BUS_REQ = TRUE;
                _usb_otg_process_a_idle(ath_usb);
#endif
            }
        } /* endif */
    }
    if (ath_usb->otg.state != previous_state){
#ifdef _OTG_DEBUG_
        DEBUG_LOG_TRACE("_usb_otg_process_exceptions, SUCCESSFUL state changed");
#endif
        return (TRUE);
    } else {
#ifdef _OTG_DEBUG_
        DEBUG_LOG_TRACE("_usb_otg_process_exceptions, SUCCESSFUL no state change");
#endif
        return (FALSE);
    }
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_b_idle
* Returned Value : None
* Comments       :
*     Process B_IDLE state.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_b_idle (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars   *s_ptr;
    struct ath_usb_usb __iomem *usb_reg;
    s32                      srp_tmr;
    u8                       se0_srp;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_idle");
#endif

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;

    srp_tmr = s_ptr->TB_SE0_SRP_TMR;
    se0_srp = s_ptr->B_SE0_SRP;
    _usb_otg_reset_state_machine(ath_usb);
    s_ptr->TB_SE0_SRP_TMR = srp_tmr;
    s_ptr->B_SE0_SRP = se0_srp;

    if (s_ptr->HOST_UP) {
        s_ptr->HOST_UP = FALSE;
        ath_usb_host_suspend(ath_usb);
        PULL_UP_PULL_DOWN_IDLE(usb_reg);
    }

    // Alagu 04/11/06 OTG_DEVICE_INIT(ath_usb, usb_reg);
    if (s_ptr->DEVICE_UP) {
        s_ptr->DEVICE_UP = FALSE;
        gadget_suspend(ath_usb);
        PULL_UP_PULL_DOWN_IDLE(usb_reg);
    }

    /*
     ** When a B-device detects that the voltage on VBUS is greater
     ** than the B-Device Session Valid threshold (VSESS_VLD),
     ** then the B-device shall consider a session to be in progress.
     ** After the VBUS voltage crosses this threshold, the B-device
     ** shall assert either the D+ or D- data-line within 100 ms.
     */
    if (SESS_VLD(usb_reg)) {
        _usb_otg_process_b_device_session_valid(ath_usb);
    } else if (s_ptr->B_BUS_REQ || s_ptr->B_SESS_REQ) {
        /*  ********* Starting  SRP *****************************************
         ** When the B-device detects that VBUS has gone below its Session End
         ** threshold and detects that both D+ and D- have been low (SE0) for
         ** at least 2 ms (TB_SE0_SRP min), then any previous session on the
         ** A-device is over and a new session may start. The B-device may
         ** initiate the SRP any time the initial conditions of Section 5.3.2
         ** are met.
         */
        /* activate the timer only if we want to do SRP */
        if ((s_ptr->B_BUS_REQ || s_ptr->B_SESS_REQ) &&
                (CHECK_TB_SE0_SRP_TMR_OFF(s_ptr)))
        {
            TB_SE0_SRP_TMR_ON(s_ptr, TB_SE0_SRP);
        }

        if ((s_ptr->B_BUS_REQ || s_ptr->B_SESS_REQ) && (B_SESS_END(usb_reg)) &&
                (s_ptr->B_SE0_SRP))
        {
            TB_SE0_SRP_TMR_OFF(s_ptr);
            s_ptr->B_SE0_SRP = FALSE;
            /*
             ** initial conditions are met as described above and then turns
             ** on its data line pull-up resistor (either D+ or D-) for a
             ** period of 5 ms to 10 ms (TB_DATA_PLS). A dual-role B-device is
             ** only allowed to initiate SRP at full-speed, and thus shall
             ** only pull up D+. The duration of such a data line pulse is
             ** sufficient to allow the A-device to reject spurious voltage
             ** transients on the data lines.
             */
            DP_HIGH_ON(usb_reg);
            TB_SRP_FAIL_TMR_ON(s_ptr, TB_SRP_FAIL);
            TB_SRP_INIT_TMR_ON(s_ptr, TB_SRP_INIT);
            TB_DATA_PLS_TMR_ON(s_ptr, TB_DATA_PLS);
            SET_STATE(OTG_STATE_B_SRP_INIT, "t ");
            s_ptr->B_SRP_DONE = FALSE;
            // TODO ath_usb_service(ath_usb, USB_OTG_SRP_ACTIVE);
        } /* Endf */
    }

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_idle, SUCCESSFUL");
#endif
}

#ifndef PERIPHERAL_ONLY
/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_a_idle
* Returned Value : None
* Comments       :
*     Process B_IDLE state.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_a_idle (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars      *s_ptr;
    struct ath_usb_usb __iomem   *usb_reg;
    u8                          a_srp_det;
    s32                         a_srp_tmr;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_a_idle");
#endif
    ath_usb_debug_otg("_usb_otg_process_a_idle\n");

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;

    a_srp_det = s_ptr->A_SRP_DET;
    a_srp_tmr = s_ptr->A_SRP_TMR;
    _usb_otg_reset_state_machine(ath_usb);
    s_ptr->A_SRP_DET = a_srp_det;
    s_ptr->A_SRP_TMR = a_srp_tmr;

    if (ID(usb_reg)) {
        SET_STATE(OTG_STATE_B_IDLE, "J ");
        _usb_otg_process_b_idle(ath_usb);
#ifdef _OTG_DEBUG_
        DEBUG_LOG_TRACE("_usb_otg_process_a_idle, SUCCESSFUL (state to B_IDLE)");
#endif
        return;
    }

    if (s_ptr->DEVICE_UP) {
        s_ptr->DEVICE_UP = FALSE;
        gadget_suspend(ath_usb);
        PULL_UP_PULL_DOWN_IDLE(usb_reg);
    }

    if (!s_ptr->HOST_UP) {
        OTG_HOST_INIT(ath_usb, usb_reg);
    }

    if ((!s_ptr->A_BUS_REQ) && (s_ptr->A_SRP_TMR == -1) &&
            (s_ptr->A_DATA_PULSE_DET))
    {
        A_SRP_TMR_ON(s_ptr, TB_DATA_PLS_MAX);
    }

    /* ************  SRP detection ****************************
     ** The A-device continuously monitors VBUS as long as power is
     ** available on the A-device. An A-device that is designed to
     ** detect the VBUS pulsing method will detect that VBUS has
     ** gone above the A-device Session Valid threshold (VA_SESS_VLD)
     ** and generate an indication that SRP has been detected.
     */
    if ((!s_ptr->A_BUS_DROP) && (SESS_VLD(usb_reg) || s_ptr->A_SRP_DET ||
                s_ptr->A_BUS_REQ))
    {
        s_ptr->A_SRP_DET = FALSE;
        SET_STATE(OTG_STATE_A_WAIT_VRISE, "u ");
        VBUS_ON(usb_reg);
        TA_WAIT_VRISE_TMR_ON(s_ptr, TA_WAIT_VRISE);
        _usb_otg_process_a_wait_vrise(ath_usb);
    }

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_a_idle, SUCCESSFUL");
#endif
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_a_wait_vrise
* Returned Value : None
* Comments       :
*     Process A_WAIT_VRISE state.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_a_wait_vrise(struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars   *s_ptr;
    struct ath_usb_usb __iomem *usb_reg;

   #ifdef _OTG_DEBUG_
      DEBUG_LOG_TRACE("_usb_otg_process_a_wait_vrise");
   #endif
      ath_usb_debug_otg("_usb_otg_process_a_wait_vrise\n");

   s_ptr = &ath_usb->state;
   usb_reg = ath_usb->usb_reg;

   if (A_VBUS_VLD(usb_reg)) {
      TA_WAIT_VRISE_TMR_OFF(s_ptr);
      /* switch to  a_wait_bcon */
      ath_usb->bVRISE_TIMEDOUT = 0;
      SET_STATE(OTG_STATE_A_WAIT_BCON, "v ");
#ifndef TA_WAIT_BCON_TMR_FOR_EVER
      TA_WAIT_BCON_TMR_ON(s_ptr, TA_WAIT_BCON);
#endif
   } else {
       if (TA_WAIT_VRISE_TMR_EXPIRED(s_ptr))
       {
           ath_usb->bVRISE_TIMEDOUT = 1;
           SET_STATE(OTG_STATE_A_WAIT_BCON, "v2 ");
           TA_WAIT_BCON_TMR_ON(s_ptr, TA_WAIT_BCON);
       }
   }


   #ifdef _OTG_DEBUG_
      DEBUG_LOG_TRACE("_usb_otg_process_a_wait_vrise, SUCCESSFUL");
   #endif
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_a_wait_vfall
* Returned Value : None
* Comments       :
*     Process A_WAIT_VFALL state.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_a_wait_vfall(struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars       *s_ptr;
    struct ath_usb_usb __iomem    *usb_reg;
    u32                          hw_signal;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_a_wait_vfall");
#endif
    ath_usb_debug_otg("_usb_otg_process_a_wait_vfall\n");

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;
    hw_signal = s_ptr->OTG_INT_STATUS;

    /* Turn this timer off if it is still on from A_peripheral state */
    TA_BIDL_ADIS_TMR_OFF(s_ptr);

    if (
            (/*SESS_VLD_CHG(hw_signal) && */SESS_VLD_FALSE(usb_reg) /*&& (!s_ptr->B_CONN)*/) ||
            ID(usb_reg) ||
            s_ptr->A_BUS_REQ
       )
    {
        SET_STATE(OTG_STATE_A_IDLE, "K ");
        VBUS_CHG_OFF(usb_reg);
        VBUS_OFF(usb_reg);
        s_ptr->A_BUS_REQ = FALSE;
        _usb_otg_process_a_idle(ath_usb);
    }

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_a_wait_vfall, SUCCESSFUL");
#endif
}
#endif

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_b_device_session_valid
* Returned Value : None
* Comments       :
*     Process B device session valid.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_b_device_session_valid (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars  *s_ptr;
    struct ath_usb_usb __iomem *usb_reg;

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_device_session_valid");
#endif
    ath_usb_debug_otg("_usb_otg_process_b_device_session_valid\n");

    if (ath_usb->otg.state == OTG_STATE_B_SRP_INIT) {
        TB_SRP_FAIL_TMR_OFF(s_ptr);
        TB_SRP_INIT_TMR_OFF(s_ptr);
        TB_DATA_PLS_TMR_OFF(s_ptr);
        TB_SE0_SRP_TMR_OFF(s_ptr);
        TB_VBUS_DSCHRG_TMR_OFF(s_ptr);
    }

    DP_HIGH_OFF(usb_reg);
    VBUS_CHG_OFF(usb_reg);
    VBUS_DSCHG_OFF(usb_reg);

    SET_STATE(OTG_STATE_B_PERIPHERAL, "w ");

    OTG_DEVICE_INIT(ath_usb, usb_reg);

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_device_session_valid, SUCCESSFUL, state to B_PERIPHERAL");
#endif
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_b_srp_init
* Returned Value : None
* Comments       :
*     Process OTG_STATE_B_SRP_INIT state.
*
*END*--------------------------------------------------------------------*/
void _usb_otg_process_b_srp_init (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars  *s_ptr;
    struct ath_usb_usb __iomem  *usb_reg;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_srp_init");
#endif
    ath_usb_debug_otg("_usb_otg_process_b_srp_init\n");

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;
    /*usb_otg_ptr->DO_SRP = FALSE;*/

    /*
     ** When a B-device detects that the voltage on VBUS is greater than the
     ** B-Device Session Valid threshold (VSESS_VLD), then the B-device
     ** shall consider a session to be in progress. After the VBUS voltage
     ** crosses this threshold, the B-device shall assert either the D+ or
     ** D- data-line within 100 ms(TB_SVLD_BCON max).
     */
    if ((SESS_VLD(usb_reg)) && (s_ptr->CHECK_SESSION)) {
        _usb_otg_process_b_device_session_valid(ath_usb);
    } else {
        /* check_srp_activity */
        if (TB_DATA_PLS_TMR_EXPIRED(s_ptr))
        {
            TB_DATA_PLS_TMR_OFF(s_ptr);
            DP_HIGH_OFF(usb_reg);
            /* An A-device is only required to respond to one of the two
             ** SRP signaling methods. A B-device shall use both methods
             ** when initiating SRP to insure that the A-device responds.
             */
            s_ptr->CHECK_SESSION = FALSE;
            VBUS_CHG_ON(usb_reg);
            /* Turn on the timer so as to keep VBUS pusling ON for 8 ms */
            TB_VBUS_PLS_TMR_ON(s_ptr, TB_VBUS_PLS);
        }

        if (TB_VBUS_PLS_TMR_EXPIRED(s_ptr)) {
            /* Turn off the VBus pulsing timer */
            TB_VBUS_PLS_TMR_OFF(s_ptr);
            /* Stop VBus pulsing */
            VBUS_CHG_OFF(usb_reg);
#if 0
            s_ptr->CHECK_SESSION = TRUE;
            /* If session is valid then proceed */
            if (SESS_VLD(usb_reg)) {
                _usb_otg_process_b_device_session_valid(ath_usb);
            }
#endif
            VBUS_DSCHG_ON(usb_reg);
            TB_VBUS_DSCHRG_TMR_ON(s_ptr, TB_VBUS_DSCHRG);
        }

        if (TB_VBUS_DSCHRG_TMR_EXPIRED(s_ptr)) {
            VBUS_DSCHG_OFF(usb_reg);
            TB_VBUS_DSCHRG_TMR_OFF(s_ptr);
            s_ptr->CHECK_SESSION = TRUE;
            /* If session is valid then proceed */
            if (SESS_VLD(usb_reg)) {
                _usb_otg_process_b_device_session_valid(ath_usb);
            }
        }

        if (TB_SRP_INIT_TMR_EXPIRED(s_ptr))
        {
            TB_SRP_INIT_TMR_OFF(s_ptr);
            VBUS_CHG_OFF(usb_reg);
        }

        /*
         ** The error call back have the following requirement:
         ** After initiating SRP, the B-device is required to wait at least
         ** 5 seconds (TB_TB_SRP_FAIL min) for the A-device to respond,
         ** before informing the user that the communication attempt
         ** has failed.
         **
         */
        if (TB_SRP_FAIL_TMR_EXPIRED(s_ptr)) {
            TB_SRP_FAIL_TMR_OFF(s_ptr);
            s_ptr->B_SRP_DONE = FALSE;
            s_ptr->B_BUS_REQ = FALSE; /*we should set this to false to avoid looping with B_IDLE */
            SET_STATE(OTG_STATE_B_IDLE, "M ");
            DP_HIGH_OFF(usb_reg);
            VBUS_CHG_OFF(usb_reg);
            // TODO ath_usb_service(ath_usb, USB_OTG_SRP_FAIL);
            _usb_otg_process_b_idle(ath_usb);
        }
    }

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_b_srp_init, SUCCESSFUL");
#endif
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_process_timers
* Returned Value : TRUE/ FALSE
* Comments       :
*     If any timers expired, the state machine will be executed and returns
*     TRUE, otherwise it will return FALSE.
*
*END*--------------------------------------------------------------------*/
u8 _usb_otg_process_timers (struct ath_usb_otg *ath_usb)
{ /* Body */
    struct ath_usb_otg_vars      *s_ptr;
    struct ath_usb_usb __iomem   *usb_reg;
    u8                          timer_expired = FALSE;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_timers");
#endif

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;

    /* Process all the A timers */
    if (s_ptr->TA_WAIT_VRISE_TMR > 0) {
        s_ptr->TA_WAIT_VRISE_TMR--;
        if (TA_WAIT_VRISE_TMR_EXPIRED(s_ptr)) {
            timer_expired = TRUE;
            LOG_TIMER("\nTA_WAIT_VRISE_TMR_EXPIRED ");
        }
    }

    if (s_ptr->TA_WAIT_BCON_TMR > 0) {
        s_ptr->TA_WAIT_BCON_TMR--;
        if (TA_WAIT_BCON_TMR_EXPIRED(s_ptr)) {
            timer_expired = TRUE;
            LOG_TIMER("\nTA_WAIT_BCON_TMR_EXPIRED ");
        }
    }

    if (s_ptr->TA_AIDL_BDIS_TMR > 0) {
        s_ptr->TA_AIDL_BDIS_TMR--;
        if (TA_AIDL_BDIS_TMR_EXPIRED(s_ptr)) {
            timer_expired = TRUE;
            LOG_TIMER("\nTA_AIDL_BDIS_TMR_EXPIRED ");
        }
    }

    if (s_ptr->TA_BIDL_ADIS_TMR > 0) {
        s_ptr->TA_BIDL_ADIS_TMR--;
        if (TA_BIDL_ADIS_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTA_BIDL_ADIS_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    /*
     ** Process all the debouncing timers first
     */
    if (s_ptr->TB_SE0_SRP_TMR > 0) {

        if (SE0(usb_reg) &&
                (STABLE_J_SE0_LINE_STATE(usb_reg)))
        {
            /* signal is constant for 1 ms, so decrement counter */
            s_ptr->TB_SE0_SRP_TMR--;
        } else {
            /* random behaviour; Reset the counter */
            TB_SE0_SRP_TMR_ON(s_ptr, TB_SE0_SRP);
        }

        if (TB_SE0_SRP_TMR_EXPIRED(s_ptr)) {
            s_ptr->B_SE0_SRP = TRUE;
            LOG_TIMER("\nTB_SE0_SRP_TMR_EXPIRED ");
            timer_expired = TRUE;
        }

    }

    /* process all the B timers */
    if (s_ptr->TB_DATA_PLS_TMR > 0) {
        s_ptr->TB_DATA_PLS_TMR--;
        if (TB_DATA_PLS_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_DATA_PLS_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    if (s_ptr->TB_VBUS_PLS_TMR > 0) {
        /* VBus pulsing timer */
        s_ptr->TB_VBUS_PLS_TMR--;
        if (TB_VBUS_PLS_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_VBUS_PLS_TMR_EXPIRED ");
            timer_expired = TRUE;
#if 0
            s_ptr->CHECK_SESSION = TRUE;
#endif
        }
    }

    if (s_ptr->TB_SRP_INIT_TMR > 0) {
        s_ptr->TB_SRP_INIT_TMR--;
        if (TB_SRP_INIT_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_SRP_INIT_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    if (s_ptr->TB_VBUS_DSCHRG_TMR > 0) {
        s_ptr->TB_VBUS_DSCHRG_TMR--;
        if (TB_VBUS_DSCHRG_TMR_EXPIRED(s_ptr)) {
            timer_expired = TRUE;
            /* Test GR */
            /*s_ptr->CHECK_SESSION = TRUE;*/
            LOG_TIMER("\nTB_VBUS_DSCHRG_TMR_EXPIRED ");
        }

    }
    if (s_ptr->TB_SRP_FAIL_TMR > 0) {
        s_ptr->TB_SRP_FAIL_TMR--;
        if (TB_SRP_FAIL_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_SRP_FAIL_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    if (s_ptr->TB_ASE0_BRST_TMR > 0) {
        s_ptr->TB_ASE0_BRST_TMR--;
        if (TB_ASE0_BRST_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_ASE0_BRST_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    if (s_ptr->TB_A_SUSPEND_TMR > 0) {
        s_ptr->TB_A_SUSPEND_TMR--;
        if (TB_A_SUSPEND_TMR_EXPIRED(s_ptr)) {
            LOG_TIMER("\nTB_A_SUSPEND_TMR_EXPIRED ");
            timer_expired = TRUE;
        }
    }

    if (s_ptr->A_SRP_TMR > 0) {
        s_ptr->A_SRP_TMR = -1;
        s_ptr->A_DATA_PULSE_DET = FALSE;
        timer_expired = TRUE;
        s_ptr->A_SRP_DET = TRUE;
    }

    if (timer_expired) {
        _usb_otg_state_machine(ath_usb);
    }

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_process_timers, SUCCESSFUL");
#endif

    return timer_expired;
} /* EndBody */

static void ath_usb_reset_controller (struct ath_usb_otg *ath_usb)
{
    u32 otgsc;
    struct ath_usb_usb __iomem *usb_reg;

    usb_reg = ath_usb->usb_reg;
    if (!usb_reg) {
        ath_usb_error("reset controller otg not initialized\n");
        return;
    }

    /* Clear OTG status/control interrupts */
    otgsc = readl(&usb_reg->otgsc);
    writel (otgsc, &usb_reg->otgsc);

    /* Mask all OTG Interrupts */
    writel (otgsc & ~ATH_USB_OTGSC_IMASK, &usb_reg->otgsc);

    /* Clear all OTG Interrupts */
    writel (otgsc | ATH_USB_OTGSC_SMASK, &usb_reg->otgsc);

    /* Place controller in Idle mode for dual host/device operation */
    writel (ATH_USB_USBMODE_SDIS | ATH_USB_USBMODE_CM_IDLE, &usb_reg->usbmode);
    ath_usb_debug_dev("Placing OTG controller in idle mode\n");
}

static int ath_usb_otg_enable (struct ath_usb_otg *ath_usb)
{
    struct otg_transceiver *otg = &ath_usb->otg;
    struct ath_usb_usb __iomem *usb_reg = ath_usb->usb_reg;

    /*
     * The identification (id) input is FALSE when a Mini-A plug is inserted
     * in the device's Mini-AB receptacle. Otherwise, this input is TRUE.
     */
    if (ID_FALSE(usb_reg)) {
        otg->state = OTG_STATE_A_IDLE;
    } else {
        otg->state = OTG_STATE_UNDEFINED;
    }

    ath_usb_debug_dev("Enabling OTG interrupts\n");
    ath_usb->enabled_ints = (ATH_USB_OTGSC_DPIE | ATH_USB_OTGSC_BSEIE |
            ATH_USB_OTGSC_BSVIE | ATH_USB_OTGSC_ASVIE | ATH_USB_OTGSC_AVVIE |
            ATH_USB_OTGSC_IDIE | ATH_USB_OTGSC_IDPU);
    writel (ath_usb->enabled_ints, &usb_reg->otgsc);
    _usb_otg_state_machine(ath_usb);

    return (0);
}


#undef NO_HOST_SUSPEND
static int ath_usb_host_suspend(struct ath_usb_otg *ath_usb)
{
printk ("host_suspend\n");
#ifdef NO_HOST_SUSPEND
    return 0;
#else
    struct usb_hcd *hcd = NULL;

    writel (ATH_USB_USBMODE_SDIS | ATH_USB_USBMODE_CM_IDLE, &ath_usb->usb_reg->usbmode);
    if (!ath_usb->otg.host)
        return -ENODEV;

    /* Currently ASSUMES only the OTG port matters;
     * other ports could be active...
     */
    hcd = container_of(ath_usb->otg.host, struct usb_hcd, self);
    return hcd->driver->bus_suspend(hcd);
#endif
}

static int ath_usb_host_resume(struct ath_usb_otg *ath_usb)
{
    printk ("host_resume\n");
#ifdef NO_HOST_SUSPEND
    return 0;
#else
    struct usb_hcd *hcd = NULL;

    writel (ATH_USB_SET_HOST_MODE, &ath_usb->usb_reg->usbmode);
    if (!ath_usb->otg.host)
        return -ENODEV;

    hcd = container_of(ath_usb->otg.host, struct usb_hcd, self);
    return hcd->driver->bus_resume(hcd);
#endif
}

static int gadget_resume(struct ath_usb_otg *ath_usb)
{
    if (!ath_usb->otg.gadget)
        return -ENODEV;
    ath_usb->otg.gadget->b_hnp_enable = 0;
    ath_usb->otg.gadget->a_hnp_support = 0;
    ath_usb->otg.gadget->a_alt_hnp_support = 0;
    writel ((ATH_USB_SET_DEV_MODE | ATH_USB_USBMODE_SLOM), &ath_usb->usb_reg->usbmode);
    return usb_gadget_vbus_connect(ath_usb->otg.gadget);
}

static int gadget_suspend(struct ath_usb_otg *ath_usb)
{
    if (!ath_usb->otg.gadget)
        return -ENODEV;
    ath_usb->otg.gadget->b_hnp_enable = 0;
    ath_usb->otg.gadget->a_hnp_support = 0;
    ath_usb->otg.gadget->a_alt_hnp_support = 0;
    writel (ATH_USB_USBMODE_SDIS | ATH_USB_USBMODE_CM_IDLE, &ath_usb->usb_reg->usbmode);
    return usb_gadget_vbus_disconnect(ath_usb->otg.gadget);
}

irqreturn_t ath_usb_otg_irq (int irq, void *__ath_usb, struct pt_regs *pr)
{
    struct ath_usb_otg *ath_usb = __ath_usb;
    u32 otg_status, otgsc, timer_expired;
    u32 port_control;
    struct ath_usb_otg_vars *s_ptr;
    struct ath_usb_usb __iomem *usb_reg;
    struct otg_transceiver *otg;

    s_ptr = &ath_usb->state;
    usb_reg = ath_usb->usb_reg;
    otg   = &ath_usb->otg;
    timer_expired = FALSE;

    /* Get the status of enabled interrupts */
    otg_status = readl(&usb_reg->otgsc) & (ath_usb->enabled_ints >> 8);

    do {
        /* Clear all enabled interrupts */
        otgsc = readl (&usb_reg->otgsc);
        writel ((otgsc | otg_status), &usb_reg->otgsc);

        /* If we have OTG interrupt call the state machine */
        if (otg_status) {
            s_ptr->OTG_INT_STATUS = otg_status;

            if ((otg_status & ATH_USB_OTGSC_DPIS) && (!s_ptr->A_DATA_PULSE_DET))
            {
                s_ptr->A_DATA_PULSE_DET = TRUE;
            }

            /*
             * If any timers expired  it will process the event and
             * any other signals at that time. We don't need to call
             * the state machine if this is the case.
             */
            if (otg_status & ATH_USB_OTGSC_1MSIS) {
                timer_expired = _usb_otg_process_timers(ath_usb);
                otg_status &= ~ATH_USB_OTGSC_1MSIS;
            }

            if (otg_status && (!timer_expired)) {
                _usb_otg_state_machine(ath_usb);
            } else {
                timer_expired = FALSE;
            }
            s_ptr->OTG_INT_STATUS = 0;
        }

        /* A_DEVICE */
        if (s_ptr->HOST_UP) {
            /* Check data line pulsing by B device */
            port_control = readl(&ath_usb->usb_reg->portscx[0]);

            if (s_ptr->A_SRP_TMR == -1) {
                if ((otg->state == OTG_STATE_B_WAIT_ACON) &&
                        ((port_control & PORT_CONNECT)&&
                         (port_control & PORT_CSC)))
                {
                    s_ptr->A_CONNECTED = TRUE;
                }
                if (otg->host) {
                    struct usb_hcd *hcd = NULL;
                    hcd = container_of(otg->host, struct usb_hcd, self);
                    hcd->driver->irq(hcd,pr);
                    set_bit (HCD_FLAG_SAW_IRQ, &hcd->flags);
                }
            }
        }

        /* B_DEVICE */
        if (s_ptr->DEVICE_UP){
            if ((otg->state == OTG_STATE_B_IDLE) &&
                    (readl(&ath_usb->usb_reg->usbsts) & STS_RST))
            {
                ;
            }
            if (otg->gadget && ath_usb->udc_isr) {
                (ath_usb->udc_isr)(irq, ath_usb->udc, pr);
            }

        }

        /* check the interrupt again */
        /* Get the status of enabled interrupts */
        otg_status = readl(&usb_reg->otgsc) & (ath_usb->enabled_ints >> 8);
    } while(otg_status);

    return (IRQ_HANDLED);
}

/* add or disable the host device+driver */
static int
ath_usb_set_host(struct otg_transceiver *otg, struct usb_bus *host)
{
    struct ath_usb_otg *ath_usb = container_of(otg, struct ath_usb_otg, otg);

    if (!otg || ath_usb != ath_usb_otg) {
        return -ENODEV;
    }

    if (!host) {
        power_down(ath_usb);
        ath_usb->otg.host = 0;
        return 0;
    }

#ifdef CONFIG_USB_OTG
    ath_usb->otg.host = host;
    dev_dbg(ath_usb->dev, "registered host\n");
    ath_usb_host_suspend(ath_usb);
#if 0
    if (ath_usb->otg.gadget)
        return ath_usb_otg_enable(ath_usb);
# else
        return ath_usb_otg_enable(ath_usb);
#endif
    return 0;
#else
    dev_dbg(ath_usb->dev, "host sessions not allowed\n");
    return -EINVAL;
#endif
}

static int
ath_usb_set_peripheral(struct otg_transceiver *otg, struct usb_gadget *gadget)
{
    struct ath_usb_otg *ath_usb = container_of(otg, struct ath_usb_otg, otg);

    if (!otg || ath_usb != ath_usb_otg)
        return -ENODEV;

    if (!gadget) {
        if (!ath_usb->otg.default_a)
            enable_vbus_draw(ath_usb, 0);
#if 0
        usb_gadget_vbus_disconnect(ath_usb->otg.gadget);
#else
        gadget_suspend(ath_usb);
#endif
        ath_usb->otg.gadget = 0;
        power_down(ath_usb);
        return 0;
    }

#ifdef CONFIG_USB_OTG
    ath_usb->otg.gadget = gadget;
    dev_dbg(ath_usb->dev, "registered gadget\n");
    /* gadget driver may be suspended until vbus_connect () */
#if 0
    if (ath_usb->otg.host)
        return ath_usb_otg_enable(ath_usb);
#else
        return ath_usb_otg_enable(ath_usb);
#endif
    return 0;

#elif !defined(CONFIG_USB_EHCI_HCD) && !defined(CONFIG_USB_EHCI_HCD_MODULE)
    ath_usb->otg.gadget = gadget;

    /* TODO power_up(isp); */
    ath_usb->otg.state = OTG_STATE_B_IDLE;

    /* TODO Enable SESS_VLD and VBUS_VLD interrupts */
    dev_info(ath_usb->dev, "B-Peripheral sessions ok\n");

    /* If this has a Mini-AB connector, this mode is highly
     * nonstandard ... but can be handy for testing, so long
     * as you don't plug a Mini-A cable into the jack.
     */
    if (isp1301_get_u8(isp, ISP1301_INTERRUPT_SOURCE) & INTR_VBUS_VLD)
        b_peripheral(isp);

    return 0;

#else
    dev_dbg(ath_usb->dev, "peripheral sessions not allowed\n");
    return -EINVAL;
#endif
}

static void
ath_usb_otg_work (void *data)
{
}

static int ath_usb_otg_setup(void)
{
    struct ath_usb_otg *ath_usb;

    if (ath_usb_otg) {
        printk ("device already initialized\n");
        return (-EBUSY);
    }

    ath_usb = kzalloc(sizeof(struct ath_usb_otg), GFP_KERNEL);
    if (!ath_usb) {
        return (-ENOMEM);
    }

    INIT_WORK(&ath_usb->work, ath_usb_otg_work, ath_usb);

    /* Reset OTG State machine */
    ath_usb_reset_otg_state (ath_usb);

    ath_usb->otg.label = AR_DRIVER_NAME;

    ath_usb->otg.set_host = ath_usb_set_host,
    ath_usb->otg.set_peripheral = ath_usb_set_peripheral,
    ath_usb->otg.set_power = ath_usb_set_power,
    ath_usb->otg.start_srp = ath_usb_start_srp,
    ath_usb->otg.start_hnp = ath_usb_start_hnp,

    ath_usb_otg = ath_usb;

    return 0;
}

static int ath_usb_otg_shutdown(void)
{
    struct ath_usb_otg *ath_usb;

    if (!ath_usb_otg) {
        return 0;
    }

    ath_usb = ath_usb_otg;
    flush_scheduled_work();
    kfree(ath_usb_otg);
    ath_usb_otg = 0;

    return 0;
}

static int ath_usb_otg_remove(struct device *dev)
{
    if (!ath_usb_otg) {
        return 0;
    }

    free_irq(ath_usb_otg->irqnum, ath_usb_otg);
	/*shekar - 6th Dec06*/
	release_mem_region(ath_usb_otg->rsrc_start, ath_usb_otg->rsrc_len);
    ath_usb_reset_controller (ath_usb_otg);
    ath_usb_otg_shutdown();

    return 0;
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : _usb_otg_set_status
* Returned Value : USB_OK or error code
* Comments       :
*     Set the value of a specified OTG parameter.
*
*END*--------------------------------------------------------------------*/
u8 _usb_otg_set_status (struct ath_usb_otg *ath_usb, u8 component, u8 status)
{
    u8                    state_change;
    struct ath_usb_otg_vars *s_ptr;
    unsigned long arflags;

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_set_status");
#endif

    s_ptr = &ath_usb->state;

    if (component < USB_OTG_A_BUS_DROP) {
        state_change = TRUE;
    } else {
        state_change = FALSE;
    }
    USB_lock();
    switch (component) {

#ifndef PERIPHERAL_ONLY
        case USB_OTG_A_SET_B_HNP_ENABLE:
            s_ptr->A_SET_B_HNP_EN = status;
            break;

        case USB_OTG_A_BUS_DROP:
            s_ptr->A_BUS_DROP = status;
            break;

        case USB_OTG_A_BUS_REQ:
            s_ptr->A_BUS_REQ = status;
            break;

        case USB_OTG_A_BUS_SUSPEND_REQ:
            s_ptr->A_BUS_SUSPEND_REQ = status;
            break;

        case USB_OTG_A_BUS_SUSPEND:
            s_ptr->A_BUS_SUSPEND = status;
            break;

        case USB_OTG_A_CONN:
            s_ptr->A_CONN = status;
            break;

        case USB_OTG_HOST_ACTIVE:
            s_ptr->HOST_UP = status;
            break;

        case USB_OTG_A_VBUS_ON:
            /*
             ** The A-device turn ON V_BUS:
             ** if the A-device application is not wanting to drop
             ** the bus (a_bus_drop = FALSE), and if the A-device Application
             ** is requesting the bus (a_bus_req = TRUE), or SRP is detected
             ** on the bus (a_srp_det = TRUE).
             */
            s_ptr->A_BUS_DROP = FALSE;
            s_ptr->A_BUS_REQ = TRUE;
            break;

        case USB_OTG_A_BUS_RESUME:
            s_ptr->A_BUS_RESUME = status;
            break;
#endif

        case USB_OTG_B_HNP_ENABLE:
            s_ptr->B_HNP_ENABLE = status;
            break;

        case USB_OTG_B_BUS_REQ:
            s_ptr->B_BUS_REQ = status;
            break;

        case USB_OTG_B_CONN:
#if 0
            uartputs("\nsetting USB_OTG_B_CONN=");
            if (status)
                uartputs("TRUE");
            else
                uartputs("FALSE");
#endif

	    printk ("B_CONN Status %d\n", status);
            s_ptr->B_CONN = status;
            break;

        case USB_OTG_B_BUS_SUSPEND:
            s_ptr->B_BUS_SUSPEND = status;
            break;

        case USB_OTG_B_BUS_RESUME:
            s_ptr->B_BUS_RESUME = status;
            break;

        case USB_OTG_DEVICE_ACTIVE:
            s_ptr->DEVICE_UP = status;
            break;

        case USB_OTG_B_SRP_REQ:
            if (ath_usb->otg.state != OTG_STATE_B_IDLE) {
                USB_unlock();
#ifdef _OTG_DEBUG_
                DEBUG_LOG_TRACE("_usb_otg_set_status, invalid state");
#endif
                return  -EINVAL;
            }
            /*
             ** set the user condition to start SRP. When we call the state
             ** machine  SRP req will be activated if preconditions are met
             */
#ifdef PERIPHERAL_ONLY
            s_ptr->B_SESS_REQ = TRUE;
#endif
            s_ptr->B_BUS_REQ = TRUE;
            break;

        default:
            USB_unlock();

#ifdef _OTG_DEBUG_
            DEBUG_LOG_TRACE("_usb_otg_set_status, bad status");
#endif

            return -EINVAL;
    } /* Endswitch */

    if (state_change) {
        _usb_otg_state_machine(ath_usb);
    }
    USB_unlock();

#ifdef _OTG_DEBUG_
    DEBUG_LOG_TRACE("_usb_otg_set_status, SUCCESSFUL");
#endif

    return (0);
}


u8 usb_otg_set_status (u8 component, u8 status)
{
	struct ath_usb_otg *ath_usb = ath_usb_otg;
	if (ath_usb_otg) {
		return (_usb_otg_set_status (ath_usb, component, status));
	}
	return (0);
}

static void  _usb_otg_state_machine (struct ath_usb_otg *ath_usb)
{
    struct ath_usb_otg_vars  *s_ptr;
    struct ath_usb_usb __iomem  *usb_reg;
    u32                     state;
    u32                     hw_signal;
    static u32              recursive = 0xffffffff;
    static u8               bTriedToRecurse = FALSE;

    /*
     * We prevent recursion to make sure that all state transition actions occur
     * in the proper sequence,but we do call _usb_otg_state_machine again at the
     * end of the function if recursion was attempted
     */
    recursive++;
    if (recursive) {
        bTriedToRecurse = TRUE;
        goto done;
    }

    s_ptr     = &ath_usb->state;
    usb_reg   = ath_usb->usb_reg;
    state     = ath_usb->otg.state;
    hw_signal = s_ptr->OTG_INT_STATUS;

#ifdef PERIPHERAL_ONLY
    switch (state) {
        case OTG_STATE_B_IDLE:
            _usb_otg_process_b_idle(ath_usb);
            break;
        case OTG_STATE_B_SRP_INIT:
            _usb_otg_process_b_srp_init(ath_usb);
            break;
        case OTG_STATE_B_PERIPHERAL:
            if (SESS_VLD_FALSE(usb_reg)) {
                SET_STATE(OTG_STATE_B_IDLE, "C ");
                s_ptr->B_BUS_REQ = FALSE;
                VBUS_CHG_OFF(usb_reg);
                VBUS_DSCHG_ON(usb_reg);
            }
            break;
        default:
            break;
    } /* Endswitch */
#else
    /*
     * process exception if  not start state
     */
    if (state) {
        if (_usb_otg_process_exceptions(ath_usb) ){
            if (state != ath_usb->otg.state) {
                goto done;  /* state is changed so return */
            }
        }
    }

    switch (state) {
        case OTG_STATE_UNDEFINED:
            if(ID(usb_reg)) {
                SET_STATE(OTG_STATE_B_IDLE, "D ");
                _usb_otg_process_b_idle(ath_usb);
                break;
            } else {
                /* ID is false when A device is identified */
                SET_STATE(OTG_STATE_A_IDLE, "E ");
            }
            /* Fall through the A_IDLE state */
        case OTG_STATE_A_IDLE:
            _usb_otg_process_a_idle(ath_usb);
            break;

        case OTG_STATE_A_WAIT_VRISE:
            _usb_otg_process_a_wait_vrise(ath_usb);
            break;

        case OTG_STATE_A_WAIT_BCON:
            /*
             * After waiting long enough to insure that the D+ line cannot
             * be high due to the residual effect of the A-device pull-up,
             * (see Section 5.1.9), the A-device sees that the D+ line
             * is high (and D- low) indicating that the B-device is
             * signaling a connect and is ready to respond as a Peripheral.
             * At this point, the A-device becomes Host and asserts bus
             * reset to start using the bus.
             */
            if (s_ptr->B_CONN) {
                s_ptr->B_BUS_SUSPEND = FALSE;
                SET_STATE(OTG_STATE_A_HOST, "d ");

                /* Turn Off A_WAIT_BCON timer since we are exiting the state */
                TA_WAIT_BCON_TMR_OFF(s_ptr);
                /*
                 * If the B device is not supported by A device
                 * a _bus_req = FALSE will set by the application
                 * during initial communication with the device
                 */
            } else if (TA_WAIT_BCON_TMR_EXPIRED(s_ptr)) {
                TA_WAIT_BCON_TMR_OFF(s_ptr);
                SET_STATE(OTG_STATE_A_WAIT_VFALL, "e ");
                _usb_otg_process_a_wait_vfall(ath_usb);
            }
            /* Turn off timer if it is still on from the A-peripheral state */
            TA_BIDL_ADIS_TMR_OFF(s_ptr);
            break;

        case OTG_STATE_A_HOST:
            /*
             * If mini B plug is removed RESET (DETACH) interrupt will
             * happen on A-Device.The DETACH will processed by the host
             * ISR  and set b_conn to  FALSE
             */
            if (!s_ptr->B_CONN) {
                /* switch to  a_wait_bcon */
                SET_STATE(OTG_STATE_A_WAIT_BCON, "f ");

                /* Turn on A_WAIT_BCON timer since we are in A_WAIT_BCON state */
                TA_WAIT_BCON_TMR_ON(s_ptr, TA_WAIT_BCON);
                break;
            }

            /************   Starting HNP **************************************
             * When the A-device is in a_host state and has set the dual-role
             * B-device's HNP enable bit (b_hnp_enable = TRUE ie, a_suspend_req
             * = TRUE), A-device shall place the connection to B-device into
             * Suspend when it is finished using the bus.  We can go to the
             * suspend state also if user want to power down.
             */
            if (!s_ptr->A_BUS_REQ || s_ptr->A_BUS_SUSPEND_REQ) {
                SET_STATE(OTG_STATE_A_SUSPEND, "g ");
                s_ptr->A_BUS_REQ = FALSE;
                s_ptr->A_SET_B_HNP_EN = TRUE;
                TA_AIDL_BDIS_TMR_ON(s_ptr, TA_AIDL_BDIS);

#ifdef HNP_HARDWARE_ASSISTANCE
                /***********************************************************
                  Set the HABA bit in OTGSC to enable hardware assitance
                  for HNP. Please see the latest hardware revision manual and
                  documentation of OTGSC register to develop and understanding
                  for the functionality of the bit being set by the code here.
                 ***********************************************************/
                AUTO_HNP_ON(usb_reg);
#endif

                /* CALL the Host API function to suspend the bus */
// TODO                 _usb_host_bus_control(usb_host_state_struct_ptr, USB_SUSPEND_SOF);
// TODO                suspend_done = TRUE;
            }
            break;

        case OTG_STATE_A_SUSPEND:
        printk ("OTG_STATE_A_SUSPEND\n");
            /*
             * b_conn = FALSE if a RESET interrupt happens and it will set by
             * the application; a_set_b_hnp_en is false as default and set to
             * if the request is not stalled
             */
            if (!s_ptr->B_CONN && !s_ptr->A_SET_B_HNP_EN) {
                TA_AIDL_BDIS_TMR_OFF(s_ptr);
                s_ptr->B_BUS_RESUME = FALSE;
#ifdef HNP_HARDWARE_ASSISTANCE
                /*set the HABA bit in OTGSC to enable hardware assitance */
                AUTO_HNP_OFF(usb_reg);
#endif
                /* switch to  a_wait_bcon */
                SET_STATE(OTG_STATE_A_WAIT_BCON, "h ");
                TA_WAIT_BCON_TMR_ON(s_ptr, TA_WAIT_BCON);
                break;
            }

            /*
             * If the B-device disconnects after the bus has been suspended,
             * then this is an indication that the B-device is attempting
             * to become Host. When the A-device detects the disconnect
             * from the B-device, it shall turn on its D+ pull-up resistor
             * within 3 ms (TA_BDIS_ACON max) to acknowledge
             * the request from the B-device.
             */
            if ((!s_ptr->B_CONN || s_ptr->B_DISCONNECTED) &&
                    (s_ptr->A_SET_B_HNP_EN))
            {
                PULL_UP_PULL_DOWN_IDLE(usb_reg);
                if (s_ptr->HOST_UP) {
                    s_ptr->HOST_UP = FALSE;
                    ath_usb_host_suspend(ath_usb);
                }
                OTG_DEVICE_INIT(ath_usb, usb_reg);
                SET_STATE(OTG_STATE_A_PERIPHERAL, "j ");
                TA_AIDL_BDIS_TMR_OFF(s_ptr);
                s_ptr->A_SET_B_HNP_EN = FALSE ;
                s_ptr->A_BUS_SUSPEND_REQ = FALSE;
                s_ptr->B_DISCONNECTED = FALSE;
                break;
            }

            if (s_ptr->A_BUS_REQ || s_ptr->B_BUS_RESUME) {
#ifdef HNP_HARDWARE_ASSISTANCE
                /*set the HABA bit in OTGSC to enable hardware assitance */
                AUTO_HNP_OFF(usb_reg);
#endif
                SET_STATE(OTG_STATE_A_HOST, "k ");
                s_ptr->B_BUS_RESUME = FALSE;
                TA_AIDL_BDIS_TMR_OFF(s_ptr);
                s_ptr->A_BUS_SUSPEND_REQ = FALSE;
                s_ptr->A_BUS_REQ = TRUE;

            }
            break;

        case OTG_STATE_A_PERIPHERAL:
            /*
             * A-device detects lack of bus activity for more than 3 ms
             * (TA_BIDL_ADIS min) and turns off its D+ pull-up.Alternatively,
             * if the A-device has no further need to communicate with the
             * B-device, the A-device may turn off VBUS and end the session.
             * Sleep INT will be processed by the  device ISR b_bus_suspend
             * variable will be set by the application
             * The following will be done when this happens:
             *
             *   - disconnect its pull up
             *   - allow time for the data line to discharge
             *   - check if the B-device has connected its pull up
             */

            if (s_ptr->B_BUS_SUSPEND) {
                if (s_ptr->DEVICE_UP) {
                    s_ptr->DEVICE_UP = FALSE;
                    gadget_suspend(ath_usb);
                    PULL_UP_PULL_DOWN_IDLE(usb_reg);
                }
                if (!s_ptr->HOST_UP) {
                    OTG_HOST_INIT(ath_usb, usb_reg);
                }
                /* switch to  a_wait_bcon */
                SET_STATE(OTG_STATE_A_WAIT_BCON, "m ");
#ifndef  TA_WAIT_BCON_TMR_FOR_EVER
                TA_WAIT_BCON_TMR_ON(s_ptr, TA_WAIT_BCON);
                s_ptr->B_BUS_SUSPEND = FALSE;
#endif
            }
            break;

        case OTG_STATE_A_WAIT_VFALL:
            _usb_otg_process_a_wait_vfall(ath_usb);
            break;

        case OTG_STATE_B_IDLE:
            _usb_otg_process_b_idle(ath_usb);
            break;

        case OTG_STATE_B_SRP_INIT:
            _usb_otg_process_b_srp_init(ath_usb);
            break;

        case OTG_STATE_B_PERIPHERAL:

            /* a_bus_suspend (SLEEP_INT) will be processed by the DEVICE ISR */
            /*
             * B-device detects that bus is idle for more than 3 ms
             * (TB_AIDL_BDIS min) and begins HNP by turning off pull-up on D+.
             * This allows the bus to discharge to the SE0 state.
             * If the bus was operating in HS mode, the B-device will first
             * enter the full-speed mode and turn on its D+ pull-up resistor
             * for at least TB_FS_BDIS min before turning off its pull up
             * to start the HNP sequence.
             * The following will be done when this happens:
             *
             *   disconnect its pull up
             *   allow time for the data line to discharge
             *   check if the A-device has connected its pull up
             */

            if (s_ptr->B_HNP_ENABLE && s_ptr->B_BUS_REQ &&
                    s_ptr->A_BUS_SUSPEND && (!s_ptr->A_SUSPEND_TIMER_ON))
            {

                TB_A_SUSPEND_TMR_ON(s_ptr, TB_A_SUSPEND);
                s_ptr->A_SUSPEND_TIMER_ON = TRUE;
                /*ensure that A_BUS_RESUME is false so that we look for
                  a fresh resume on the bus */
                s_ptr->A_BUS_RESUME = FALSE;
            }

            /*
             * when in suspended state in B_PERIPHERAL and host drives a resume
             * we should recover back to B_PERIPHERAL state
             */
            if((s_ptr->A_BUS_RESUME) && s_ptr->A_SUSPEND_TIMER_ON) {
                TB_A_SUSPEND_TMR_OFF(s_ptr);
                AUTO_RESET_OFF(usb_reg);
                s_ptr->A_BUS_SUSPEND = FALSE;

            }

            if (s_ptr->A_SUSPEND_TIMER_ON) {

                /*****************************************************
                  To meet the 1ms reset requirement for OPT test 5.4
                  , latest hardware revisions provide the autoreset bit
                  When this bit is set, hardware will drive a reset on
                  the bus, the moment device connects. This will ensure
                  that reset is hardware driver in a timely manner,
                  rather than driver by software when a 'connect'
                  interrupt happens. The bit has been added to meet
                  the OTP timing requirement for customers have large
                  interrupt latencies in their design

                  SG 06/18/2004
                 *****************************************************/
                AUTO_RESET_ON(usb_reg);

                /* 7ms timer to let the bus discharge to SE0 state */
                if (TB_A_SUSPEND_TMR_EXPIRED(s_ptr)) {
                    s_ptr->A_SUSPEND_TIMER_ON = FALSE;
                    TB_A_SUSPEND_TMR_OFF(s_ptr);

                    s_ptr->A_BUS_SUSPEND = FALSE;

                    if (s_ptr->DEVICE_UP) {
                        s_ptr->DEVICE_UP = FALSE;
                        gadget_suspend(ath_usb);
                        PULL_UP_PULL_DOWN_IDLE(usb_reg);
                    }

                    if (!s_ptr->HOST_UP) {
                        OTG_HOST_INIT(ath_usb, usb_reg);
                        /* No VBUS_ON */
                    }
                    TB_ASE0_BRST_TMR_ON(s_ptr, TB_ASE0_BRST);
                    SET_STATE(OTG_STATE_B_WAIT_ACON, "n ");
                }
            }
            break;

        case OTG_STATE_B_WAIT_ACON:
            /*
             * After waiting long enough to insure that the D+ line
             * cannot be high due to the residual effect of the B-device
             * pull-up,(see Section 5.1.9), the B-device sees that the
             * D+ line is high and D- low, (i.e. J state). This indicates
             * that the A-device has recognized the HNP request from the
             * B-device. At this point, the B-device becomes Host and
             * asserts bus reset to start using the bus. The B-device
             * must assert the bus reset (SE0) within 1 ms (TB_ACON_BSE0 max)
             * of the time that the A-device turns on its
             * pull-up.
             */
            /* ATTACH_INT will be processed by the host ISR */
            if (s_ptr->A_CONN) {
                s_ptr->A_CONNECTED = FALSE;
                s_ptr->A_BUS_SUSPEND = FALSE;
                s_ptr->B_HNP_ENABLE = FALSE;
                TB_ASE0_BRST_TMR_OFF(s_ptr);
                SET_STATE(OTG_STATE_B_HOST, "p ");
                break;
            }

            /*
             * While waiting in the b_wait_acon state, the B-device
             * may detect a K state on the bus. This indicates that the
             * A-device is signaling a resume condition and is retaining
             * control of the bus. In this case, the B-device will return
             * to the b_peripheral state.
             *
             * a_bus_resume will be set by the host ISR when K state is
             * detected  if (a_bus_resume || b_ase0_brst_tmr)
             */
            if (s_ptr->A_BUS_RESUME || (TB_ASE0_BRST_TMR_EXPIRED(s_ptr) &&
                        (!s_ptr->A_CONNECTED)))
            {
                s_ptr->A_BUS_RESUME = FALSE;
                if (s_ptr->HOST_UP) {
                    s_ptr->HOST_UP = FALSE;
                    ath_usb_host_suspend(ath_usb);
                    PULL_UP_PULL_DOWN_IDLE(usb_reg);
                }
                /***********************************************************
                  OPT test 5.8 and 5.9 require that we must display a message
                  when a HNP fails. This event should allow the application
                  to know  that HNP failed with Host.
                 ***********************************************************/
                if((TB_ASE0_BRST_TMR_EXPIRED(s_ptr)) && s_ptr->B_HNP_ENABLE)
                {
// TODO                    ath_usb_service(ath_usb, USB_OTG_HNP_FAILED);
                }

                TB_ASE0_BRST_TMR_OFF(s_ptr);
                OTG_DEVICE_INIT(ath_usb, usb_reg);
                SET_STATE(OTG_STATE_B_PERIPHERAL, "q ");
                s_ptr->A_BUS_SUSPEND = FALSE;
                s_ptr->B_HNP_ENABLE = FALSE;
                break;
            }

            if (TB_ASE0_BRST_TMR_EXPIRED(s_ptr)) {
                TB_ASE0_BRST_TMR_OFF(s_ptr);
            }
            break;

        case OTG_STATE_B_HOST:
            /*
             * If the B-device at any time detects more than 3.125 ms of SE0
             * (TB_ASE0_BRST min), then this is an indication that the
             * A-device is remaining Host and is resetting the bus. In this
             * case the B-device shall return to the b_peripheral state
             * and start to process the bus reset before TB_ASE0_BRST max.
             */
            if (!s_ptr->B_BUS_REQ || !s_ptr->A_CONN) {
                PULL_UP_PULL_DOWN_IDLE(usb_reg);
                if (s_ptr->HOST_UP) {
                    s_ptr->HOST_UP = FALSE;
                    ath_usb_host_suspend(ath_usb);
                }

                OTG_DEVICE_INIT(ath_usb, usb_reg);

                SET_STATE(OTG_STATE_B_PERIPHERAL, "r ");
            }
            break;
        default:
            break;
    }
#endif  /* PERIPHERAL-ONLY */

    if ((s_ptr->TA_WAIT_VRISE_TMR > 0) ||
            (s_ptr->TA_WAIT_BCON_TMR > 0)  ||
            (s_ptr->TA_AIDL_BDIS_TMR > 0)  ||
            (s_ptr->TA_BIDL_ADIS_TMR > 0)  ||
            (s_ptr->TB_DATA_PLS_TMR > 0)   ||
            (s_ptr->TB_SRP_INIT_TMR > 0)   ||
            (s_ptr->TB_SRP_FAIL_TMR > 0)   ||
            (s_ptr->TB_VBUS_PLS_TMR > 0)   ||
            (s_ptr->TB_ASE0_BRST_TMR > 0)  ||
            (s_ptr->TB_SE0_SRP_TMR > 0)    ||
            (s_ptr->TB_VBUS_DSCHRG_TMR > 0)  ||
            (s_ptr->TB_A_SUSPEND_TMR > 0)  ||
            (s_ptr->A_SRP_TMR > 0))
    {
        START_TIMER(usb_reg);
    } else {
        STOP_TIMER(usb_reg);
    }

done:
    recursive--;

    /*
     * If we are at the top level, and we had tried to recurse, we clear the
     * recurse flag and we call _usb_otg_state_machine again.
     */
    if (recursive == 0xffffffff) {
        if (bTriedToRecurse) {
            bTriedToRecurse = FALSE;
            _usb_otg_state_machine(ath_usb);  /* fake recursion */
        }
    }

    return;
}

static int
ath_usb_set_power(struct otg_transceiver *dev, unsigned mA)
{
    if (!ath_usb_otg)
        return -ENODEV;
    if (dev->state == OTG_STATE_B_PERIPHERAL)
        enable_vbus_draw(ath_usb_otg, mA);
    return 0;
}


static int ath_usb_start_srp(struct otg_transceiver *dev)
{
    return 0;
}

static int
ath_usb_start_hnp(struct otg_transceiver *dev)
{
#ifdef CONFIG_USB_OTG
    struct ath_usb_otg *ath_usb = container_of(dev, struct ath_usb_otg, otg);
	printk("Start HNP \n");
    if (!dev || ath_usb != ath_usb_otg)
        return -ENODEV;
    if (ath_usb->otg.default_a && (ath_usb->otg.host == NULL
                || !ath_usb->otg.host->b_hnp_enable))
        return -ENOTCONN;
    if (!ath_usb->otg.default_a && (ath_usb->otg.gadget == NULL
                || !ath_usb->otg.gadget->b_hnp_enable))
        return -ENOTCONN;

    /*
     * We want hardware to manage most HNP protocol timings.
     * So do this part as early as possible...
     */
    _usb_otg_set_status(ath_usb, USB_OTG_A_SET_B_HNP_ENABLE, TRUE);
    _usb_otg_set_status(ath_usb, USB_OTG_A_BUS_SUSPEND_REQ, TRUE);
    _usb_otg_set_status(ath_usb, USB_OTG_A_BUS_REQ, FALSE);

    return 0;
#else
    /* srp-only */
    return -EINVAL;
#endif
}

/* Proc Interface to Start HNP */
static int ath_usb_rd_status(char *page,char **start,off_t off,
							int count,int *eof,void *data)
{
	struct ath_usb_otg *ath_usb = (struct ath_usb_otg *)data;
	char *seq = page;
	int length;

	if(!ath_usb || !ath_usb->usb_reg){
		printk("Returned with !ath_usb \n");
		return -EINVAL;
	}
	__u32 otgsc =0;
	otgsc = readl(&ath_usb->usb_reg->otgsc);
#if 0
	seq += sprintf(seq,"Driver : %s \n",AR_DRIVER_NAME);
	seq += sprintf(seq,"OTG Device Running at %s Mode \n\n",
				   (readl(&ath_usb->usb_reg->portscx[0]) & MASK_MODE) ? "Device":"Host");
#endif
	seq += sprintf(seq,"OTG Status: \n");
	seq += sprintf(seq,"OTG - %s\n",(otgsc & MASK_USBID) ? "B Device":"A Device");

	seq += sprintf(seq,"OTG - %s\n",(otgsc & MASK_ASV) ? "A Session Valid":
							    ((otgsc & MASK_BSV) ? "B Session Valid":"Invalid State"));
	length = seq - page - off;

	if(length < count){
		*eof = 1;
		if(length <= 0){
			return 0;
		}
	}else{
		length = count;
	}

	*start = page + off;
	return length;
}

static int ath_usb_wr_startHNP(struct file *file,const char *buf,
							  unsigned long count,void *data)
{
	char start;
	struct ath_usb_otg *ath_usb = (struct ath_usb_otg *)data;
	struct otg_transceiver *transceiver = &ath_usb->otg;
	unsigned long flags;

	if(!ath_usb){
		return -EINVAL;
	}

	if(copy_from_user(&start,buf,count)){
		return -EINVAL;
	}

	if(start == 1){
		ath_usb_start_hnp(transceiver);
		local_irq_save(flags);
		transceiver->state = OTG_STATE_A_SUSPEND;
		local_irq_restore(flags);
	}
	return count;
}

int ath_usb_create_proc(struct ath_usb_otg *otg)
{
	struct proc_dir_entry	*usbroot = NULL;
	struct proc_dir_entry	*rdStatus;
	struct proc_dir_entry	*start_hnp;

	usbroot = proc_mkdir("otg",0);

	if(!usbroot){
		return -1;
	}
	usbroot->owner = THIS_MODULE;

	/* Read OTG Status */
	rdStatus = create_proc_entry("status",S_IFREG|S_IRUGO|S_IWUSR,usbroot);
	if(!rdStatus){
		return -1;
	}

	rdStatus->nlink =1;
	rdStatus->read_proc = ath_usb_rd_status;
	rdStatus->data = (void *)otg;
	rdStatus->owner	= THIS_MODULE;

	/* Entry to Start HNP */
	start_hnp = create_proc_entry("startHNP",S_IFREG|S_IRUGO|S_IWUSR,usbroot);
	if(!start_hnp){
		return -1;
	}
	start_hnp->nlink =1;
	start_hnp->write_proc = ath_usb_wr_startHNP;
	start_hnp->data = (void *)otg;
	start_hnp->owner = THIS_MODULE;

	return(0);
}

static int
ath_usb_otg_probe(struct device *dev)
{
    struct platform_device  *pdev   = to_platform_device(dev);
    struct ath_usb_otg *ath_usb;
    int retval;

    ath_usb_debug_dev("\nprobing ath_usb otg...\n");

    retval = ath_usb_otg_setup();
    if (retval < 0) {
        return (retval);
    }

    if (pdev->resource[1].flags != IORESOURCE_IRQ) {
        ath_usb_error ("resource[1] is not IORESOURCE_IRQ");
        retval = -ENOMEM;
    }

    ath_usb = ath_usb_otg;
    ath_usb->rsrc_start = pdev->resource[0].start;
    ath_usb->rsrc_len   = pdev->resource[0].end - pdev->resource[0].start + 1;

    if (!request_mem_region(ath_usb->rsrc_start, ath_usb->rsrc_len,
                AR_DRIVER_NAME))
    {
        ath_usb_error("request_mem_region failed");
        retval = -EBUSY;
        goto err1;
    }

    ath_usb->reg_base = ioremap(ath_usb->rsrc_start, ath_usb->rsrc_len);
    if (!ath_usb->reg_base) {
        ath_usb_error("ioremap failed");
        retval = -ENOMEM;
        goto err2;
    }
    ath_usb_debug_dev("otg->reg_base is %p\n", ath_usb->reg_base);
#if 0
    /* Device Initialization - start */
    ath_usb_reg_rmw_clear(ATH_USB_RESET,ATH_USB_RESET_USB_HOST);
    ath_usb_reg_rmw_set(ATH_USB_RESET,ATH_USB_RESET_USB_PHY);  //PHY RESET

    /* Clear Host Mode */
    ath_usb_reg_rmw_clear(ATH_USB_CONFIG,(1 << 2));
    ath_usb_debug_dev("Usb Config Reg %x\n",ath_usb_reg_rd(ATH_USB_CONFIG));
    mdelay(10);

    ath_usb_reg_rmw_clear(ATH_USB_RESET,ATH_USB_RESET_USB_PHY);//PHY CLEAR RESET
    ath_usb_debug_dev("ATH_USB_RESET %x \n",ath_usb_reg_rd(ATH_USB_RESET));
    mdelay(10);
#endif


    ath_usb_reg_rmw_clear(ATH_USB_RESET,ATH_USB_RESET_USB_PHY); //CLEAR PHY RESET

    ath_usb_reg_rmw_clear(ATH_USB_RESET,ATH_USB_RESET_USB_HOST); //CLEAR HOST RESET

    ath_usb->cap_reg = ath_usb->reg_base + ATH_USB_CAP_REG_OFFSET;
    ath_usb->usb_reg = ath_usb->reg_base + ATH_USB_CAP_REG_OFFSET +
        HC_LENGTH(readl(&ath_usb->cap_reg->hc_capbase));
#if 0
    /* Setting 16-bit mode */
    ath_usb_reg_rmw_set(&ath_usb->usb_reg->portscx[0],(1 <<28));
    ath_usb_debug_dev("PORT_STATUS[0] %p = %x\n", &ath_usb->usb_reg->portscx[0], readl(&ath_usb->usb_reg->portscx[0]));
    mdelay(10);

#endif

    ath_usb->irqnum = pdev->resource[1].start;
    if ((retval = request_irq (ath_usb->irqnum, &ath_usb_otg_irq,
                    (SA_INTERRUPT | SA_SHIRQ), "ath_usb-irq", ath_usb) != 0))
    {
        dev_err(ath_usb->dev, "request interrupt %d failed\n", ath_usb->irqnum);
        goto err3;
    }

    spin_lock_init(&ath_usb->arlock);
    ath_usb->otg.dev = dev;
    ath_usb->dev = dev;
    ath_usb_reset_controller(ath_usb);

	/*Create Proc File Sys */
	if(ath_usb_create_proc(ath_usb_otg) < 0){
		ath_usb_debug_dev("Failed to Create Procfs \n");
	}

    // TODO remove after OTG Testing
    // ath_usb_otg_enable(ath_usb);
    return (0);

err3:
    iounmap(ath_usb->usb_reg);
err2:
    release_mem_region(ath_usb->rsrc_start, ath_usb->rsrc_len);
err1:
    ath_usb_otg_shutdown();
    return retval;
}





static struct device_driver ath_usb_otg_driver = {
    .name       =   "ath-ehci",
    .bus        =   &platform_bus_type,
    .probe      =   ath_usb_otg_probe,
    .remove     =   ath_usb_otg_remove,
};

static int __init ath_usb_otg_init(void)
{
    return (driver_register(&ath_usb_otg_driver));
}
module_init(ath_usb_otg_init);

static void __exit ath_usb_otg_exit(void)
{
    driver_unregister(&ath_usb_otg_driver);
}
module_exit(ath_usb_otg_exit);
MODULE_LICENSE("GPL");
EXPORT_SYMBOL(ath_usb_get_otg);
EXPORT_SYMBOL(usb_otg_set_status);
