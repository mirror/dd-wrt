/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/**
 * @defgroup fal_port_ctrl FAL_PORT_CONTROL
 * @{
 */
#ifndef _FAL_PORTCTRL_H_
#define _FAL_PORTCTRL_H_

#ifdef __cplusplus
extern "c" {
#endif

#include "common/sw.h"
#include "fal/fal_type.h"

    typedef enum {
        FAL_HALF_DUPLEX = 0,
        FAL_FULL_DUPLEX,
        FAL_DUPLEX_BUTT = 0xffff
    }
                      fal_port_duplex_t;

    typedef enum
    {
        FAL_SPEED_10    = 10,
        FAL_SPEED_100   = 100,
        FAL_SPEED_1000  = 1000,
        FAL_SPEED_10000 = 10000,
        FAL_SPEED_BUTT  = 0xffff,
    } fal_port_speed_t;

    typedef enum
    {
        FAL_CABLE_STATUS_NORMAL  = 0,
        FAL_CABLE_STATUS_SHORT   = 1,
        FAL_CABLE_STATUS_OPENED  = 2,
        FAL_CABLE_STATUS_INVALID = 3,
        FAL_CABLE_STATUS_BUTT    = 0xffff,
    } fal_cable_status_t;

#define FAL_ENABLE      1
#define FAL_DISABLE     0

//phy autoneg adv
#define FAL_PHY_ADV_10T_HD      0x01
#define FAL_PHY_ADV_10T_FD      0x02
#define FAL_PHY_ADV_100TX_HD    0x04
#define FAL_PHY_ADV_100TX_FD    0x08
//#define FAL_PHY_ADV_1000T_HD    0x100
#define FAL_PHY_ADV_1000T_FD    0x200
#define FAL_PHY_ADV_1000BX_HD    0x400
#define FAL_PHY_ADV_1000BX_FD    0x800

#define FAL_PHY_ADV_FE_SPEED_ALL   \
    (FAL_PHY_ADV_10T_HD | FAL_PHY_ADV_10T_FD | FAL_PHY_ADV_100TX_HD |\
     FAL_PHY_ADV_100TX_FD)

#define FAL_PHY_ADV_GE_SPEED_ALL   \
    (FAL_PHY_ADV_10T_HD | FAL_PHY_ADV_10T_FD | FAL_PHY_ADV_100TX_HD |\
     FAL_PHY_ADV_100TX_FD | FAL_PHY_ADV_1000T_FD)

#define FAL_PHY_ADV_BX_SPEED_ALL   \
    (FAL_PHY_ADV_1000BX_HD | FAL_PHY_ADV_1000BX_FD)

#define FAL_PHY_ADV_PAUSE       0x10
#define FAL_PHY_ADV_ASY_PAUSE   0x20
#define FAL_PHY_FE_ADV_ALL         \
    (FAL_PHY_ADV_FE_SPEED_ALL | FAL_PHY_ADV_PAUSE | FAL_PHY_ADV_ASY_PAUSE)
#define FAL_PHY_GE_ADV_ALL         \
    (FAL_PHY_ADV_GE_SPEED_ALL | FAL_PHY_ADV_PAUSE | FAL_PHY_ADV_ASY_PAUSE)

#define FAL_PHY_COMBO_ADV_ALL         \
    (FAL_PHY_ADV_BX_SPEED_ALL | FAL_PHY_ADV_GE_SPEED_ALL | FAL_PHY_ADV_PAUSE | FAL_PHY_ADV_ASY_PAUSE)

//phy capablity
#define FAL_PHY_AUTONEG_CAPS   0x01
#define FAL_PHY_100T2_HD_CAPS  0x02
#define FAL_PHY_100T2_FD_CAPS  0x04
#define FAL_PHY_10T_HD_CAPS    0x08
#define FAL_PHY_10T_FD_CAPS    0x10
#define FAL_PHY_100X_HD_CAPS   0x20
#define FAL_PHY_100X_FD_CAPS   0x40
#define FAL_PHY_100T4_CAPS     0x80
//#define FAL_PHY_1000T_HD_CAPS  0x100
#define FAL_PHY_1000T_FD_CAPS  0x200
//#define FAL_PHY_1000X_HD_CAPS  0x400
#define FAL_PHY_1000X_FD_CAPS  0x800

//phy partner capablity
#define FAL_PHY_PART_10T_HD   0x1
#define FAL_PHY_PART_10T_FD   0x2
#define FAL_PHY_PART_100TX_HD 0x4
#define FAL_PHY_PART_100TX_FD 0x8
//#define FAL_PHY_PART_1000T_HD 0x10
#define FAL_PHY_PART_1000T_FD 0x20

//phy interrupt flag
#define FAL_PHY_INTR_SPEED_CHANGE         0x1
#define FAL_PHY_INTR_DUPLEX_CHANGE        0x2
#define FAL_PHY_INTR_STATUS_UP_CHANGE     0x4
#define FAL_PHY_INTR_STATUS_DOWN_CHANGE   0x8
#define FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE   0x10
#define FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE   0x20
#define FAL_PHY_INTR_MEDIA_STATUS_CHANGE   0x40
#define FAL_PHY_INTR_WOL_STATUS   0x80
#define FAL_PHY_INTR_POE_STATUS   0x100

    typedef enum
    {
        FAL_NO_HEADER_EN = 0,
        FAL_ONLY_MANAGE_FRAME_EN,
        FAL_ALL_TYPE_FRAME_EN
    } fal_port_header_mode_t;

    typedef struct
    {
        a_uint16_t pair_a_status;
        a_uint16_t pair_b_status;
        a_uint16_t pair_c_status;
        a_uint16_t pair_d_status;
        a_uint32_t pair_a_len;
        a_uint32_t pair_b_len;
        a_uint32_t pair_c_len;
        a_uint32_t pair_d_len;
    } fal_port_cdt_t;

/*below is new add for malibu phy*/

/** Phy mdix mode */
typedef enum {
    PHY_MDIX_AUTO        = 0, /**< Auto MDI/MDIX */
    PHY_MDIX_MDI         = 1, /**< Fixed MDI */
    PHY_MDIX_MDIX        = 2  /**< Fixed MDIX */
} fal_port_mdix_mode_t;

/** Phy mdix status */
typedef enum {
    PHY_MDIX_STATUS_MDI        = 0, /**< Fixed MDI */
    PHY_MDIX_STATUS_MDIX       = 1  /**< Fixed MDIX */

} fal_port_mdix_status_t;

/** Phy master mode */
typedef enum {
    PHY_MASTER_MASTER    = 0,   /**< Phy manual MASTER configuration */
    PHY_MASTER_SLAVE     = 1,   /**< Phy manual SLAVE configuration */
    PHY_MASTER_AUTO      = 2    /**< Phy automatic MASTER/SLAVE configuration */
} fal_port_master_t;

/** Phy preferred medium type */
typedef enum {
    PHY_MEDIUM_COPPER    = 0,   /**< Copper */
    PHY_MEDIUM_FIBER     = 1,   /**< Fiber */

} fal_port_medium_t;

/** Phy pages */
typedef enum {
    PHY_SGBX_PAGES      = 0,    /**< sgbx pages */
    PHY_COPPER_PAGES    = 1     /**< copper pages */

} fal_port_reg_pages_t;


/** Phy preferred Fiber mode */
typedef enum {
    PHY_FIBER_100FX     = 0,    /**< 100FX fiber mode */
    PHY_FIBER_1000BX    = 1,    /**< 1000BX fiber mode */

} fal_port_fiber_mode_t;

/** Phy reset status */
typedef enum {
    PHY_RESET_DONE        = 0,  /**< Phy reset done */
    PHY_RESET_BUSY        = 1   /**< Phy still in reset process */
} fal_port_reset_status_t;

/** Phy auto-negotiation status */
typedef enum {
    PHY_AUTO_NEG_STATUS_BUSY   = 0, /**< Phy still in auto-negotiation process */
    PHY_AUTO_NEG_STATUS_DONE   = 1  /**< Phy auto-negotiation done */
} fal_port_auto_neg_status_t;


/** Phy interface mode */
	typedef enum {
		PHY_PSGMII_BASET = 0,
				/**< PSGMII mode */
		PHY_PSGMII_BX1000 = 1,
				/**< PSGMII BX1000 mode */
		PHY_PSGMII_FX100 = 2,
				/**< PSGMII FX100 mode */
		PHY_PSGMII_AMDET = 3,
				/**< PSGMII Auto mode */
		PHY_SGMII_BASET = 4,
				/**< SGMII mode */
	} fal_port_interface_mode_t;

/** Phy counter information */
typedef struct {
	a_uint32_t RxGoodFrame;
	a_uint32_t RxBadCRC;
	a_uint32_t TxGoodFrame;
	a_uint32_t TxBadCRC;
} fal_port_counter_info_t;

/*above is new add for malibu phy*/

    sw_error_t
    fal_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_duplex_t duplex);



    sw_error_t
    fal_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_duplex_t * pduplex);



    sw_error_t
    fal_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
                       fal_port_speed_t speed);



    sw_error_t
    fal_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
                       fal_port_speed_t * pspeed);



    sw_error_t
    fal_port_autoneg_status_get(a_uint32_t dev_id, fal_port_t port_id,
                                a_bool_t * status);



    sw_error_t
    fal_port_autoneg_enable(a_uint32_t dev_id, fal_port_t port_id);



    sw_error_t
    fal_port_autoneg_restart(a_uint32_t dev_id, fal_port_t port_id);



    sw_error_t
    fal_port_autoneg_adv_set(a_uint32_t dev_id, fal_port_t port_id,
                             a_uint32_t autoadv);



    sw_error_t
    fal_port_autoneg_adv_get(a_uint32_t dev_id, fal_port_t port_id,
                             a_uint32_t * autoadv);



    sw_error_t
    fal_port_hdr_status_set(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t enable);



    sw_error_t
    fal_port_hdr_status_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_bool_t * enable);



    sw_error_t
    fal_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);



    sw_error_t
    fal_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);


    sw_error_t
    fal_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
                                    a_bool_t enable);


    sw_error_t
    fal_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
                                    a_bool_t * enable);


    sw_error_t
    fal_port_powersave_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_powersave_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);


    sw_error_t
    fal_port_hibernate_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_hibernate_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);


    sw_error_t
    fal_port_cdt(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
                 fal_cable_status_t *cable_status, a_uint32_t *cable_len);


    sw_error_t
    fal_port_rxhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_header_mode_t mode);


    sw_error_t
    fal_port_rxhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_header_mode_t * mode);


    sw_error_t
    fal_port_txhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_header_mode_t mode);


    sw_error_t
    fal_port_txhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_port_header_mode_t * mode);


    sw_error_t
    fal_header_type_set(a_uint32_t dev_id, a_bool_t enable, a_uint32_t type);


    sw_error_t
    fal_header_type_get(a_uint32_t dev_id, a_bool_t * enable, a_uint32_t * type);


    sw_error_t
    fal_port_txmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_txmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_rxmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_rxmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_bp_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_bp_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_link_forcemode_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_link_forcemode_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);


    sw_error_t
    fal_port_link_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * status);

    sw_error_t
    fal_ports_link_status_get(a_uint32_t dev_id, a_uint32_t * status);

    sw_error_t
    fal_port_mac_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_mac_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable);

	sw_error_t
	fal_port_congestion_drop_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t queue_id, a_bool_t enable);


	sw_error_t
	fal_port_congestion_drop_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t queue_id, a_bool_t * enable);

	sw_error_t
	fal_ring_flow_ctrl_thres_set(a_uint32_t dev_id, a_uint32_t ring_id, a_uint8_t on_thres, a_uint8_t off_thres);


	sw_error_t
	fal_ring_flow_ctrl_thres_get(a_uint32_t dev_id, a_uint32_t ring_id, a_uint8_t *on_thres, a_uint8_t *off_thres);

    sw_error_t
    fal_port_8023az_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);


    sw_error_t
    fal_port_8023az_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);

    sw_error_t
    fal_port_mdix_set(a_uint32_t dev_id, fal_port_t port_id,
                       fal_port_mdix_mode_t mode);
    sw_error_t
    fal_port_mdix_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_mode_t * mode);
   sw_error_t
   fal_port_mdix_status_get(a_uint32_t dev_id, fal_port_t port_id,
                        fal_port_mdix_status_t * mode);
   sw_error_t
   fal_port_combo_prefer_medium_set(a_uint32_t dev_id, a_uint32_t port_id,
                          fal_port_medium_t medium);
   sw_error_t
   fal_port_combo_prefer_medium_get(a_uint32_t dev_id, a_uint32_t port_id,
                          fal_port_medium_t * medium);
  sw_error_t
  fal_port_combo_medium_status_get(a_uint32_t dev_id, a_uint32_t port_id,
                          fal_port_medium_t * medium);
  sw_error_t
  fal_port_combo_fiber_mode_set(a_uint32_t dev_id, a_uint32_t port_id,
                          fal_port_fiber_mode_t  mode);
  sw_error_t
  fal_port_combo_fiber_mode_get(a_uint32_t dev_id, a_uint32_t port_id,
                          fal_port_fiber_mode_t * mode);
      sw_error_t
    fal_port_local_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);

    sw_error_t
    fal_port_local_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);

         sw_error_t
    fal_port_remote_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);

    sw_error_t
    fal_port_remote_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);

     sw_error_t
	    fal_port_reset(a_uint32_t dev_id, fal_port_t port_id);

	 sw_error_t
	    fal_port_power_off(a_uint32_t dev_id, fal_port_t port_id);

	 sw_error_t
	    fal_port_power_on(a_uint32_t dev_id, fal_port_t port_id);

    sw_error_t
    fal_port_magic_frame_mac_set (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac);

   sw_error_t
   fal_port_magic_frame_mac_get (a_uint32_t dev_id, fal_port_t port_id,
				   fal_mac_addr_t * mac);

 sw_error_t
    fal_port_phy_id_get (a_uint32_t dev_id, fal_port_t port_id,
		      a_uint16_t * org_id, a_uint16_t * rev_id);

 sw_error_t
    fal_port_wol_status_set (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t enable);

 sw_error_t
    fal_port_wol_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      a_bool_t * enable);

 sw_error_t
    fal_port_interface_mode_set(a_uint32_t dev_id, fal_port_t port_id, fal_port_interface_mode_t mode);

 sw_error_t
    fal_port_interface_mode_get (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t * mode);
 sw_error_t
    fal_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t * mode);
 sw_error_t
	fal_port_counter_set(a_uint32_t dev_id, fal_port_t port_id,
			a_bool_t enable);
 
 sw_error_t
	fal_port_counter_get(a_uint32_t dev_id, fal_port_t port_id,
			a_bool_t * enable);
 sw_error_t
	fal_port_counter_show (a_uint32_t dev_id, fal_port_t port_id,
				  fal_port_counter_info_t * counter_info);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_PORTCTRL_H_ */
/**
 * @}
 */
