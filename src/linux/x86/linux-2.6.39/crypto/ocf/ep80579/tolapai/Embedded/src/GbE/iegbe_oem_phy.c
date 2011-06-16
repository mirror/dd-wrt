/*****************************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98

  Contact Information:

  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226 

*****************************************************************************/
/**************************************************************************
 * @ingroup OEM_PHY_GENERAL
 *
 * @file oem_phy.c
 *
 * @description
 *   This module contains oem functions.
 *
 **************************************************************************/

#include "iegbe_oem_phy.h"
#include "iegbe.h"
#include "gcu_if.h"
/*
 * List of functions leveraged from the base iegbe driver.
 *
 * Ideally, it would have been nice to keep iegbe_oem_phy.c
 * minimally dependent on the iegbe. Any function taking
 * a struct iegbe_hw as a parameter can be implemented in
 * this file. It was chosen to reuse as much code as possible
 * to save time (isn't that always the case ;)
 */
extern int iegbe_up(struct iegbe_adapter *adapter);
extern void iegbe_down(struct iegbe_adapter *adapter);
extern void iegbe_reset(struct iegbe_adapter *adapter);
extern int iegbe_set_spd_dplx(struct iegbe_adapter *adapter, uint16_t spddplx);
extern int32_t iegbe_copper_link_autoneg(struct iegbe_hw *hw);
extern int32_t iegbe_phy_force_speed_duplex(struct iegbe_hw *hw);
extern int32_t iegbe_copper_link_postconfig(struct iegbe_hw *hw);
extern int32_t iegbe_oem_set_trans_gasket(struct iegbe_hw *hw);

/* forward declarations for static support functions */
static int32_t iegbe_oem_link_m88_setup(struct iegbe_hw *hw);
static int32_t iegbe_oem_set_phy_mode(struct iegbe_hw *hw);
static int32_t iegbe_oem_detect_phy(struct iegbe_hw *hw);

/**
 * iegbe_oem_setup_link
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * Performs OEM Transceiver specific link setup as part of the
 * global iegbe_setup_link() function.
 **/
int32_t
iegbe_oem_setup_link(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    /* 
     * see iegbe_setup_copper_link() as the primary example. Look at both
     * the M88 and IGP functions that are called for ideas, possibly for
     * power management.
     */

    int32_t ret_val;
    uint32_t ctrl;
    uint16_t i;
    uint16_t phy_data;

    DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }
    /* AFU: add test to exit out if improper phy type
     */
    /* relevent parts of iegbe_copper_link_preconfig */
    ctrl = E1000_READ_REG(hw, CTRL);
    ctrl |= E1000_CTRL_SLU;
    ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
    E1000_WRITE_REG(hw, CTRL, ctrl);
    
    /* this is required for *hw init */
    ret_val = iegbe_oem_detect_phy(hw); 
    if(ret_val) {
        return ret_val;
    }
    ret_val = iegbe_oem_set_phy_mode(hw);
    if(ret_val) {
        return ret_val;
    }

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_link_m88_setup(hw);
            if(ret_val) { 
                return ret_val; 
            }
        break; 
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
     }

     if(hw->autoneg) {
         ret_val = iegbe_copper_link_autoneg(hw);
         if(ret_val) { 
             return ret_val; 
     }
     } 
     else {
         DEBUGOUT("Forcing speed and duplex\n");
         ret_val = iegbe_phy_force_speed_duplex(hw);
     }
           
     /* 
      * Check link status. Wait up to 100 microseconds for link to become
      * valid.
      */
      for(i = 0; i < 0xa; i++) {
          ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_STATUS, &phy_data);
          if(ret_val) {
              DEBUGOUT("Unable to read register PHY_STATUS\n");
              return ret_val;
          }

          ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_STATUS, &phy_data);
          if(ret_val) {
              DEBUGOUT("Unable to read register PHY_STATUS\n");
              return ret_val;
          }

          hw->icp_xxxx_is_link_up = (phy_data & MII_SR_LINK_STATUS) != 0;

          if(phy_data & MII_SR_LINK_STATUS) {
              /* Config the MAC and PHY after link is up */
              ret_val = iegbe_copper_link_postconfig(hw);
              if(ret_val) {
                  return ret_val;
              }
              DEBUGOUT("Valid link established!!!\n");
              return E1000_SUCCESS;
          }
          usec_delay(0xa);
      }

      DEBUGOUT("Unable to establish link!!!\n");
      return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    DEBUGOUT("Invalid value for hw->media_type");
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_link_m88_setup
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * lifted from iegbe_copper_link_mgp_setup, pretty much
 * copied verbatim except replace iegbe_phy_reset with iegbe_phy_hw_reset
 **/
static int32_t
iegbe_oem_link_m88_setup(struct iegbe_hw *hw)
{
    int32_t ret_val;
    uint16_t phy_data = 0;

    DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }

    ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, 
                                              &phy_data);
    phy_data |= 0x00000008;
    ret_val = iegbe_oem_write_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, phy_data);

    /* phy_reset_disable is set in iegbe_oem_set_phy_mode */
    if(hw->phy_reset_disable) {
        return E1000_SUCCESS;
    }
    /* Enable CRS on TX. This must be set for half-duplex operation. */
    ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, &phy_data);
    if(ret_val) {
        DEBUGOUT("Unable to read M88E1000_PHY_SPEC_CTRL register\n");
        return ret_val;
    }

    phy_data &= ~M88E1000_PSCR_ASSERT_CRS_ON_TX;

    /* 
     * Options:
     *   MDI/MDI-X = 0 (default)
     *   0 - Auto for all speeds
     *   1 - MDI mode
     *   2 - MDI-X mode
     *   3 - Auto for 1000Base-T only (MDI-X for 10/100Base-T modes)
     */
    phy_data &= ~M88E1000_PSCR_AUTO_X_MODE;

    switch (hw->mdix) {
    case 0x1:
        phy_data |= M88E1000_PSCR_MDI_MANUAL_MODE;
    break;
    case 0x2:
        phy_data |= M88E1000_PSCR_MDIX_MANUAL_MODE;
    break;
    case 0x3:
        phy_data |= M88E1000_PSCR_AUTO_X_1000T;
    break;
    case 0:
    default:
        phy_data |= M88E1000_PSCR_AUTO_X_MODE;
    break;
    }

    /* 
     * Options:
     *   disable_polarity_correction = 0 (default)
     *       Automatic Correction for Reversed Cable Polarity
     *   0 - Disabled
     *   1 - Enabled
     */
    phy_data &= ~M88E1000_PSCR_POLARITY_REVERSAL;

    if(hw->disable_polarity_correction == 1) {
        phy_data |= M88E1000_PSCR_POLARITY_REVERSAL;
    }          
    ret_val = iegbe_oem_write_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, phy_data);
    if(ret_val) {
        DEBUGOUT("Unable to write M88E1000_PHY_SPEC_CTRL register\n");
        return ret_val;
    }

    /* 
     * Force TX_CLK in the Extended PHY Specific Control Register
     * to 25MHz clock.
     */
    ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_EXT_PHY_SPEC_CTRL, 
                                        &phy_data);
    if(ret_val) {
        DEBUGOUT("Unable to read M88E1000_EXT_PHY_SPEC_CTRL register\n");
        return ret_val;
    }

    /* 
     * For Truxton, it is necessary to add RGMII tx and rx
     * timing delay though the EXT_PHY_SPEC_CTRL register
     */
    phy_data |= M88E1000_EPSCR_TX_TIME_CTRL;
    phy_data |= M88E1000_EPSCR_RX_TIME_CTRL;

    if (hw->phy_revision < M88E1011_I_REV_4) {

        phy_data |= M88E1000_EPSCR_TX_CLK_25;
        /* Configure Master and Slave downshift values */
        phy_data &= ~(M88E1000_EPSCR_MASTER_DOWNSHIFT_MASK |
                      M88E1000_EPSCR_SLAVE_DOWNSHIFT_MASK);
        phy_data |= (M88E1000_EPSCR_MASTER_DOWNSHIFT_1X |
                     M88E1000_EPSCR_SLAVE_DOWNSHIFT_1X);
    }
    ret_val = iegbe_oem_write_phy_reg_ex(hw, M88E1000_EXT_PHY_SPEC_CTRL, 
                                         phy_data);
    if(ret_val) {
            DEBUGOUT("Unable to read M88E1000_EXT_PHY_SPEC_CTRL register\n");
        return ret_val;
    }
    

    /* SW Reset the PHY so all changes take effect */
    ret_val = iegbe_phy_hw_reset(hw);
    if(ret_val) {
        DEBUGOUT("Error Resetting the PHY\n");
        return ret_val;
    }

    return E1000_SUCCESS;
}

/**
 * iegbe_oem_force_mdi
 * @hw: iegbe_hw struct containing device specific information
 * @resetPhy: returns true if after calling this function the 
 *            PHY requires a reset
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * This is called from iegbe_phy_force_speed_duplex, which is
 * called from iegbe_oem_setup_link.
 **/
int32_t 
iegbe_oem_force_mdi(struct iegbe_hw *hw, int *resetPhy)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    int32_t ret_val;

   DEBUGFUNC1("%s",__func__);

    if(!hw || !resetPhy) {
        return -1;
    }

    /* 
     * a boolean to indicate if the phy needs to be reset
     * 
     * Make note that the M88 phy is what'll be used on Truxton
     * see iegbe_phy_force_speed_duplex, which does the following for M88
     */
      switch (hw->phy_id) {
          case M88E1000_I_PHY_ID:
          case M88E1141_E_PHY_ID:
              ret_val = iegbe_oem_read_phy_reg_ex(hw, 
                                                   M88E1000_PHY_SPEC_CTRL, 
                                                   &phy_data);
              if(ret_val) {
                  DEBUGOUT("Unable to read M88E1000_PHY_SPEC_CTRL register\n");
                  return ret_val;
               }
          
               /*
                * Clear Auto-Crossover to force MDI manually. M88E1000 requires 
                * MDI forced whenever speed are duplex are forced.
                */
          
              phy_data &= ~M88E1000_PSCR_AUTO_X_MODE;
          ret_val = iegbe_oem_write_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, 
                                                    phy_data);
              if(ret_val) {
                  DEBUGOUT("Unable to write M88E1000_PHY_SPEC_CTRL register\n");
                  return ret_val;
              }
              *resetPhy = TRUE;
          break;
          default:
              DEBUGOUT("Invalid PHY ID\n");
              return -E1000_ERR_PHY_TYPE;
      }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !resetPhy) {
        return -1;
    }

    *resetPhy = FALSE;
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_phy_reset_dsp
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * This is called from iegbe_phy_force_speed_duplex, which is
 * called from iegbe_oem_setup_link.
 **/
int32_t 
iegbe_oem_phy_reset_dsp(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }

    /*
     * Make note that the M88 phy is what'll be used on Truxton.
     *
     * See iegbe_phy_force_speed_duplex, which calls iegbe_phy_reset_dsp
     * for the M88 PHY. The code as written references registers 29 and 30,
     * which are reserved for the M88 used on Truxton, so this will be a
     * no-op.
     */
     switch (hw->phy_id) {
         case M88E1000_I_PHY_ID:
         case M88E1141_E_PHY_ID:
             DEBUGOUT("No DSP to reset on OEM PHY\n");
         break;
         default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
     }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_cleanup_after_phy_reset
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * This is called from iegbe_phy_force_speed_duplex, which is
 * called from iegbe_oem_setup_link.
 **/
int32_t 
iegbe_oem_cleanup_after_phy_reset(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    int32_t ret_val;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    } 

    /* 
     * Make note that the M88 phy is what'll be used on Truxton.
     * see iegbe_phy_force_speed_duplex, which does the following for M88
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            /*
             * Because we reset the PHY above, we need to re-force 
             * TX_CLK in the Extended PHY Specific Control Register to
             * 25MHz clock.  This value defaults back to a 2.5MHz clock
             * when the PHY is reset.
             */

             ret_val = iegbe_oem_read_phy_reg_ex(hw,
                                                 M88E1000_EXT_PHY_SPEC_CTRL, 
                                                 &phy_data);
             if(ret_val) {
                 DEBUGOUT("Unable to read M88E1000_EXT_SPEC_CTRL register\n");
                 return ret_val;
             }

             phy_data |= M88E1000_EPSCR_TX_CLK_25;
             ret_val = iegbe_oem_write_phy_reg_ex(hw, 
                                                   M88E1000_EXT_PHY_SPEC_CTRL, 
                                                   phy_data);
             if(ret_val) {
                 DEBUGOUT("Unable to write M88E1000_EXT_PHY_SPEC_CTRL " 
				          "register\n");
                 return ret_val;
             }

             /*
              * In addition, because of the s/w reset above, we need to enable
              * CRX on TX.  This must be set for both full and half duplex 
              * operation.
              */

              ret_val = iegbe_oem_read_phy_reg_ex(hw, 
                                                   M88E1000_PHY_SPEC_CTRL, 
                                                   &phy_data);
              if(ret_val) {
                  DEBUGOUT("Unable to read M88E1000_PHY_SPEC_CTRL register\n");
                  return ret_val;
              }

          phy_data &= ~M88E1000_PSCR_ASSERT_CRS_ON_TX;
          ret_val = iegbe_oem_write_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, 
                                                    phy_data);
              if(ret_val) {
                  DEBUGOUT("Unable to write M88E1000_PHY_SPEC_CTRL register\n");
                  return ret_val;
              }         
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_set_phy_mode
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * This is called from iegbe_oem_setup_link which is
 * called from iegbe_setup_link.
 **/
static int32_t 
iegbe_oem_set_phy_mode(struct iegbe_hw *hw)
{
    /*
     * it is unclear if it is necessary to set the phy mode. Right now only
     * one MAC 82545 Rev 3 does it, but the other MACs like tola do not.
     * Leave the functionality off for now until it is determined that Tolapai
     * needs it as well.
     */
#ifdef skip_set_mode
#undef skip_set_mode
#endif

#ifdef skip_set_mode
    int32_t ret_val;
    uint16_t eeprom_data;
#endif
   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }

    /*
     * iegbe_set_phy_mode specifically works for 82545 Rev 3 only,
     * since it is a 'loner' compared to the 82545, 82546, and
     * 82546 Rev 3, assume for now it is anomaly and don't repeat
     * for Truxton/Haxton.
     * Note that this is the approach taken in both the Windows and
     * FreeBSD drivers
     */
#ifndef skip_set_mode
    DEBUGOUT("No need to call oem_set_phy_mode on Truxton\n");
#else
    /* 
     * Make note that the M88 phy is what'll be used on Truxton.
     *
     * use iegbe_set_phy_mode as example
     */
    switch (hw->phy_id) {
         case M88E1000_I_PHY_ID:
         case M88E1141_E_PHY_ID:
             ret_val = iegbe_read_eeprom(hw, 
                                          EEPROM_PHY_CLASS_WORD, 
                                          1, 
                                          &eeprom_data);
              if(ret_val) {
                  return ret_val;
              }

              if((eeprom_data != EEPROM_RESERVED_WORD) && 
                  (eeprom_data & EEPROM_PHY_CLASS_A)) 
              {
                  ret_val = iegbe_oem_write_phy_reg_ex(hw, 
				                                    M88E1000_PHY_PAGE_SELECT, 
                                                    0x000B);
                  if(ret_val) {
                      DEBUGOUT("Unable to write to M88E1000_PHY_PAGE_SELECT "
					           "register on PHY\n");
                      return ret_val;
                  }

                  ret_val = iegbe_oem_write_phy_reg_ex(hw, 
                                                      M88E1000_PHY_GEN_CONTROL, 
                                                      0x8104);
                  if(ret_val) {
                      DEBUGOUT("Unable to write to M88E1000_PHY_GEN_CONTROL"
                               "register on PHY\n");
                      return ret_val;
                  }

                  hw->phy_reset_disable = FALSE;
              }
         break;
         default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }
#endif
    
    return E1000_SUCCESS;

}


/**
 * iegbe_oem_detect_phy
 * @hw: iegbe_hw struct containing device specific information
 *
 * Fills hw->phy_type, hw->phy_id and hw->phy_revision fields as well
 * as verifies that the PHY identified is one that is comprehended
 * by the driver.
 *
 * This borrows heavily from iegbe_detect_gig_phy
 **/
static int32_t 
iegbe_oem_detect_phy(struct iegbe_hw *hw)
{
    int32_t ret_val;
    uint16_t phy_id_high, phy_id_low;

    DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }
    hw->phy_type = iegbe_phy_oem;

    ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_ID1, &phy_id_high);
    if(ret_val) {
        DEBUGOUT("Unable to read PHY register PHY_ID1\n");
        return ret_val;
    }
    
    usec_delay(0x14);
    ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_ID2, &phy_id_low);
    if(ret_val) {
        DEBUGOUT("Unable to read PHY register PHY_ID2\n");
        return ret_val;
    }
    hw->phy_id = (uint32_t) ((phy_id_high << 0x10) + 
	                         (phy_id_low & PHY_REVISION_MASK));
    hw->phy_revision = (uint32_t) phy_id_low & ~PHY_REVISION_MASK;

    return E1000_SUCCESS;
}


/**
 * iegbe_oem_get_tipg
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns the value of the Inter Packet Gap (IPG) Transmit Time (IPGT) in the
 * Transmit IPG register appropriate for the given PHY. This field is only 10 
 * bits wide.
 *
 * In the original iegbe code, only the IPGT field varied between media types.
 * If the OEM phy requires setting IPG Receive Time 1 & 2 Registers, it would 
 * be required to modify the iegbe_config_tx() function to accomdate the change
 *
 **/
uint32_t 
iegbe_oem_get_tipg(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    uint32_t phy_num;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return DEFAULT_ICP_XXXX_TIPG_IPGT;
    }

    switch (hw->phy_id) {
         case M88E1000_I_PHY_ID:
         case M88E1141_E_PHY_ID:
             phy_num = DEFAULT_ICP_XXXX_TIPG_IPGT;
         break;
         default:
            DEBUGOUT("Invalid PHY ID\n");
            return DEFAULT_ICP_XXXX_TIPG_IPGT;
    }
    
    return phy_num;

#else /* ifdef EXTERNAL_MDIO */

    /* return the default value required by ICP_xxxx style MACS */
    DEBUGOUT("Invalid value for transceiver type, return default"
             " TIPG.IPGT value\n");
    return DEFAULT_ICP_XXXX_TIPG_IPGT;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_phy_is_copper
 * @hw: iegbe_hw struct containing device specific information
 *
 * Test for media type within the iegbe driver is common, so this is a simple 
 * test for copper PHYs. The ICP_XXXX family of controllers initially only 
 * supported copper interconnects (no TBI (ten bit interface) for Fiber 
 * existed). If future revs support either Fiber or an internal SERDES, it 
 * may become necessary to evaluate where this function is used to go beyond 
 * determining whether or not media type is just copper.
 *
 **/
int 
iegbe_oem_phy_is_copper(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    int isCopper = TRUE;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return isCopper;
    }

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            isCopper = TRUE;
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }
    
    return isCopper;

#else /* ifdef EXTERNAL_MDIO */

    /* 
     * caught between returning true or false. True allows it to
     * be entered into && statements w/o ill effect, but false
     * would make more sense 
     */
    DEBUGOUT("Invalid value for transceiver type, return FALSE\n");
    return FALSE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_get_phy_dev_number
 * @hw: iegbe_hw struct containing device specific information
 *
 * For ICP_XXXX family of devices, there are 3 MACs, each of which may 
 * have a different PHY (and indeed a different media interface). This 
 * function is used to indicate which of the MAC/PHY pairs we are interested 
 * in.
 * 
 **/
uint32_t 
iegbe_oem_get_phy_dev_number(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    /* 
     * for ICP_XXXX family of devices, the three network interfaces are 
     * differentiated by their PCI device number, where the three share
     * the same PCI bus
     */
    struct iegbe_adapter *adapter;
    uint32_t device_number;

    DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return 0;
    }

    adapter = (struct iegbe_adapter *) hw->back;
    device_number = PCI_SLOT(adapter->pdev->devfn);

	switch(device_number)
    {
      case ICP_XXXX_MAC_0: 
	      hw->phy_addr = 0x00;
	  break;
      case ICP_XXXX_MAC_1: 
	      hw->phy_addr = 0x01;
	  break;
      case ICP_XXXX_MAC_2: 
	      hw->phy_addr = 0x02;
	  break;
	  default:  hw->phy_addr = 0x00;
    }
     return hw->phy_addr;

#else /* ifdef EXTERNAL_MDIO */
    DEBUGOUT("Invalid value for transceiver type, return 0\n");
    return 0;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_mii_ioctl
 * @adapter: iegbe_hw struct containing device specific information
 * @flags: The saved adapter->stats_lock flags from the initiating spinlock
 * @ifr: interface request structure for socket ioctls
 * @cmd: the original IOCTL command that instigated the call chain.
 *
 * This function abstracts out the code necessary to service the
 * SIOCSMIIREG case within the iegbe_mii_ioctl() for oem PHYs. 
 * iegbe_mii_ioctl() was implemented for copper phy's only and this
 * function will only be called if iegbe_oem_phy_is_copper() returns true for
 * a given MAC. Note that iegbe_mii_ioctl() has a compile flag
 * and exists only if SIOCGMIIPHY is defined.
 *
 * NOTE: a spinlock is in effect for the duration of this call. It is
 *       imperative that a negative value be returned on any error, so
 *       the spinlock can be released properly.
 *     
 **/
int
iegbe_oem_mii_ioctl(struct iegbe_adapter *adapter, unsigned long flags,
                    struct ifreq *ifr, int cmd)
{
#ifdef EXTERNAL_MDIO
    
    struct mii_ioctl_data *data = if_mii(ifr);
    uint16_t mii_reg = data->val_in;
    uint16_t spddplx;
    int retval;

   DEBUGFUNC1("%s",__func__);

    if(!adapter || !ifr) {
        return -1;
    }
    switch (data->reg_num) {
        case PHY_CTRL:
            if(mii_reg & MII_CR_POWER_DOWN) {
                  break;
            }
            if(mii_reg & MII_CR_AUTO_NEG_EN) {
                adapter->hw.autoneg = 1;
                adapter->hw.autoneg_advertised = ICP_XXXX_AUTONEG_ADV_DEFAULT;
            } 
            else {
                if(mii_reg & 0x40) {
                    spddplx = SPEED_1000;
                }
                else if(mii_reg & 0x2000) {
                     spddplx = SPEED_100;
                }
                else {
                    spddplx = SPEED_10;
                }
                spddplx += (mii_reg & 0x100) ? FULL_DUPLEX : HALF_DUPLEX;
                retval = iegbe_set_spd_dplx(adapter, spddplx);
                if(retval) {
                    return retval;
                }
            }
            if(netif_running(adapter->netdev)) {
                iegbe_down(adapter);
                iegbe_up(adapter);
            } 
            else {
                iegbe_reset(adapter);
            }
        break;
        case M88E1000_PHY_SPEC_CTRL:
        case M88E1000_EXT_PHY_SPEC_CTRL:
            retval = iegbe_phy_reset(&adapter->hw);
            if(retval) {
                DEBUGOUT("Error resetting the PHY\n");
                return -EIO;
            }
        break;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -EOPNOTSUPP;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_fiber_live_in_suspend
 * @hw: iegbe_hw struct containing device specific information
 *
 * This is called within iegbe_suspend() to allow an action to be performed
 * on an oem phy before before the MAC goes into suspend. This is only called
 * if the STATUS.LU (link up) bit has been previous set.
 *
 * For ICP_XXXX, this is a no op
 **/
void iegbe_oem_fiber_live_in_suspend(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return;
    }
    return;

#else /* ifdef EXTERNAL_MDIO */

    return;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_get_phy_regs
 * @adapter iegbe_adapter struct containing device specific information
 * @data unsigned integer array of size data_len
 * @data_len number of elements in data
 *
 * This is called by iegbe_get_regs() in response to an ethtool request
 * to return the data of the controller. Most of the data returned is from
 * the MAC, but some data comes from the PHY, thus from this f().
 *
 * Note: The call to iegbe_get_regs() assumed an array of 24 elements
 *       where the last 11 are passed to this function. If the array
 *       that is passed to the calling function has its size or element
 *       defintions changed, this function becomes broken. 
 *
 **/
void iegbe_oem_get_phy_regs(struct iegbe_adapter *adapter, uint32_t *data, 
                            uint32_t data_len)
{
#define EXPECTED_ARRAY_LEN 11
    uint32_t corrected_len;

   DEBUGFUNC1("%s",__func__);

    if(!adapter || !data) {
        return;
    }

    /* This f(n) expects to have EXPECTED_ARRAY_LEN elements to initialize.
     * Use the corrected_length variable to make sure we don't exceed that
     * length
     */
    corrected_len = data_len>EXPECTED_ARRAY_LEN 
                    ? EXPECTED_ARRAY_LEN : data_len;
    memset(data, 0, corrected_len*sizeof(uint32_t));

#ifdef EXTERNAL_MDIO

    /* 
     * Fill data[] with...
     *
     * [0] = cable length
     * [1] = cable length
     * [2] = cable length
     * [3] = cable length
     * [4] = extended 10bt distance
     * [5] = cable polarity
     * [6] = cable polarity
     * [7] = polarity correction enabled
     * [8] = undefined
     * [9] = phy receive errors
     * [10] = mdix mode
     */
    switch (adapter->hw.phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            if(corrected_len > 0) {
                iegbe_oem_read_phy_reg_ex(&adapter->hw, 
                                          M88E1000_PHY_SPEC_STATUS, 
                                          (uint16_t *) &data[0]);
            }
          if(corrected_len > 0x1){
              data[0x1] = 0x0;  /* Dummy (to align w/ IGP phy reg dump) */
            }
          if(corrected_len > 0x2) {
              data[0x2] = 0x0;  /* Dummy (to align w/ IGP phy reg dump) */
            }
          if(corrected_len > 0x3) {
              data[0x3] = 0x0;  /* Dummy (to align w/ IGP phy reg dump) */
            }
          if(corrected_len > 0x4) {
              iegbe_oem_read_phy_reg_ex(&adapter->hw, M88E1000_PHY_SPEC_CTRL, 
                                 (uint16_t *) &data[0x4]);
            }
          if(corrected_len > 0x5) {
              data[0x5] = data[0x0];
            }
          if(corrected_len > 0x6) {
              data[0x6] = 0x0;  /* Dummy (to align w/ IGP phy reg dump) */
            }
          if(corrected_len > 0x7) {
              data[0x7] = data[0x4];
            }
            /* phy receive errors */
          if(corrected_len > 0x9) {
              data[0x9] = adapter->phy_stats.receive_errors;
            }
          if(corrected_len > 0xa) {
              data[0xa] = data[0x0];
            }
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return;
    }
#endif /* ifdef EXTERNAL_MDIO */

#undef EXPECTED_ARRAY_LEN
    return;
}


/**
 * iegbe_oem_phy_loopback
 * @adapter iegbe_adapter struct containing device specific information
 *
 * This is called from iegbe_set_phy_loopback in response from call from
 * ethtool to place the PHY into loopback mode.
 **/
int 
iegbe_oem_phy_loopback(struct iegbe_adapter *adapter)
{
#ifdef EXTERNAL_MDIO

    int ret_val;
    uint32_t ctrl_reg = 0;

   DEBUGFUNC1("%s",__func__);

    if(!adapter) {
        return -1;
    }

    /*
     * This borrows liberally from iegbe_integrated_phy_loopback().
     * iegbe_nonintegrated_phy_loopback() was also a point of reference
     * since it was similar. The biggest difference between the two
     * was that nonintegrated called iegbe_phy_reset_clk_and_crs(),
     * hopefully this won't matter as CRS required for half-duplex
     * operation and this is set to full duplex.
     * 
     * Make note that the M88 phy is what'll be used on Truxton
     * Loopback configuration is the same for each of the supported PHYs.
     */
    switch (adapter->hw.phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:

          adapter->hw.autoneg = FALSE;

          /* turn off Auto-MDI/MDIX */
          /*ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw, 
                                               M88E1000_PHY_SPEC_CTRL, 0x0808);
          if(ret_val)
          {
             DEBUGOUT("Unable to write to register M88E1000_PHY_SPEC_CTRL\n");
              return ret_val;
          }
         */
          /* reset to update Auto-MDI/MDIX */
        /*  ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw,
                  PHY_CTRL, 0x9140);
          if(ret_val)
          {
              DEBUGOUT("Unable to write to register PHY__CTRL\n");
              return ret_val;
          }
        */
          /* autoneg off */
          /*ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw,
                                             PHY_CTRL, 0x8140); */
          ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw, PHY_CTRL, 0xa100);
          if(ret_val) {
              DEBUGOUT("Unable to write to register PHY_CTRL\n");
              return ret_val;
          }
          
          
          /* force 1000, set loopback */
          /*ret_val = 
                 iegbe_oem_write_phy_reg_ex(&adapter->hw, PHY_CTRL, 0x4140); */
          ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw, PHY_CTRL, 0x6100);
          if(ret_val) {
              DEBUGOUT("Unable to write to register PHY_CTRL\n");
              return ret_val;
          }

          ctrl_reg = E1000_READ_REG(&adapter->hw, CTRL);
          ctrl_reg &= ~E1000_CTRL_SPD_SEL;   /* Clear the speed sel bits */
          ctrl_reg |= (E1000_CTRL_FRCSPD     /* Set the Force Speed Bit */
                       | E1000_CTRL_FRCDPX   /* Set the Force Duplex Bit */
                       | E1000_CTRL_SPD_100 /* Force Speed to 1000 */
                       | E1000_CTRL_FD);       /* Force Duplex to FULL */
                    /*   | E1000_CTRL_ILOS); */  /* Invert Loss of Signal */

          E1000_WRITE_REG(&adapter->hw, CTRL, ctrl_reg);

          /*
           * Write out to PHY registers 29 and 30 to disable the Receiver. 
           * This directly lifted from iegbe_phy_disable_receiver().
           * 
           * The code is currently commented out as for the M88 used in
           * Truxton, registers 29 and 30 are unutilized. Leave in, just
           * in case we are on the receiving end of an 'undocumented' 
           * feature
           */
          /* 
           * iegbe_oem_write_phy_reg_ex(&adapter->hw, 29, 0x001F);
           * iegbe_oem_write_phy_reg_ex(&adapter->hw, 30, 0x8FFC);
           * iegbe_oem_write_phy_reg_ex(&adapter->hw, 29, 0x001A);
           * iegbe_oem_write_phy_reg_ex(&adapter->hw, 30, 0x8FF0);
           */
          
          break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return 0;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */

}


/**
 * iegbe_oem_loopback_cleanup
 * @adapter iegbe_adapter struct containing device specific information
 *
 * This is called from iegbe_loopback_cleanup in response from call from
 * ethtool to place the PHY out of loopback mode. This handles the OEM
 * specific part of loopback cleanup.
 **/
void 
iegbe_oem_loopback_cleanup(struct iegbe_adapter *adapter)
{
#ifdef EXTERNAL_MDIO

    /* 
     * This borrows liberally from iegbe_loopback_cleanup(). 
     * making note that the M88 phy is what'll be used on Truxton
     * 
     * Loopback cleanup is the same for all supported PHYs.
     */
    int32_t ret_val;
    uint16_t phy_reg;

   DEBUGFUNC1("%s",__func__);

    if(!adapter) {
        return ;
    }

    switch (adapter->hw.phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
        default:
            adapter->hw.autoneg = TRUE;
        
            ret_val = iegbe_oem_read_phy_reg_ex(&adapter->hw, PHY_CTRL, 
                                                &phy_reg);
            if(ret_val) {
                DEBUGOUT("Unable to read to register PHY_CTRL\n");
                return;
            }
        
            if(phy_reg & MII_CR_LOOPBACK) {
                phy_reg &= ~MII_CR_LOOPBACK;
        
                ret_val = iegbe_oem_write_phy_reg_ex(&adapter->hw, PHY_CTRL, 
                                                     phy_reg);
                if(ret_val) {
                    DEBUGOUT("Unable to write to register PHY_CTRL\n");
                    return;
                }
        
                iegbe_phy_reset(&adapter->hw);
            }
    }
        
#endif /* ifdef EXTERNAL_MDIO */
    return;

}


/**
 * iegbe_oem_phy_speed_downgraded
 * @hw iegbe_hw struct containing device specific information
 * @isDowngraded returns with value > 0 if the link belonging to hw
 *               has been downshifted
 *
 * Called by iegbe_check_downshift(), checks the PHY to see if it running
 * at as speed slower than its maximum.
 **/
uint32_t 
iegbe_oem_phy_speed_downgraded(struct iegbe_hw *hw, uint16_t *isDowngraded)
{
#ifdef EXTERNAL_MDIO

    uint32_t ret_val;
    uint16_t phy_data;

   DEBUGFUNC1("%s",__func__);

    if(!hw || !isDowngraded) {
        return 1;
    }

    /*
     * borrow liberally from E1000_check_downshift iegbe_phy_m88 case.
     * Make note that the M88 phy is what'll be used on Truxton
     */

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS, 
                                                &phy_data);
          if(ret_val) {
                DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
                return ret_val;
            }
          
            *isDowngraded = (phy_data & M88E1000_PSSR_DOWNSHIFT) 
                             >> M88E1000_PSSR_DOWNSHIFT_SHIFT;
         
        break; 
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return 1;
    }

    return 0;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !isDowngraded) {
        return 1;
    }

    *isDowngraded = 0;
    return 0; 

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_check_polarity
 * @hw iegbe_hw struct containing device specific information
 * @isDowngraded returns with value > 0 if the link belonging to hw
 *               has its polarity shifted.
 *
 * Called by iegbe_check_downshift(), checks the PHY to see if it running
 * at as speed slower than its maximum.
 **/
int32_t 
iegbe_oem_check_polarity(struct iegbe_hw *hw, uint16_t *polarity)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;
    uint16_t phy_data;

   DEBUGFUNC1("%s",__func__);

    if(!hw || !polarity) {
        return -1;
    }

    /* 
     * borrow liberally from iegbe_check_polarity.
     * Make note that the M88 phy is what'll be used on Truxton
     */

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            /* return the Polarity bit in the Status register. */
            ret_val = iegbe_oem_read_phy_reg_ex(hw, 
                                                M88E1000_PHY_SPEC_STATUS, 
                                                &phy_data);
            if(ret_val) {
              DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
              return ret_val;
            }

            *polarity = (phy_data & M88E1000_PSSR_REV_POLARITY) 
                         >> M88E1000_PSSR_REV_POLARITY_SHIFT;
          
         break; 
         default:
              DEBUGOUT("Invalid PHY ID\n");
              return -E1000_ERR_PHY_TYPE;
    }
    return 0;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !polarity) {
        return -1;
    }

    *polarity = 0;
    return -1;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_phy_is_full_duplex
 * @hw iegbe_hw struct containing device specific information
 * @isFD a boolean returning true if phy is full duplex
 *
 * This is called as part of iegbe_config_mac_to_phy() to align
 * the MAC with the PHY. It turns out on ICP_XXXX, this is not
 * done automagically.
 **/
int32_t 
iegbe_oem_phy_is_full_duplex(struct iegbe_hw *hw, int *isFD)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    int32_t ret_val;

    DEBUGFUNC1("%s",__func__);

    if(!hw || !isFD) {
        return -1;
    }
    /* 
     * Make note that the M88 phy is what'll be used on Truxton
     * see iegbe_config_mac_to_phy
     */
        
      switch (hw->phy_id) {
          case M88E1000_I_PHY_ID:
          case M88E1141_E_PHY_ID:
             ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS,
                                    			  &phy_data);
             if(ret_val) {
                DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
                return ret_val;
             }
              *isFD = (phy_data & M88E1000_PSSR_DPLX) != 0;
          
           break;
           default:
               DEBUGOUT("Invalid PHY ID\n");
               return -E1000_ERR_PHY_TYPE;
      }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !isFD) {
        return -1;
    }
    *isFD = FALSE;
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}
/**
 * iegbe_oem_phy_is_speed_1000
 * @hw iegbe_hw struct containing device specific information
 * @is1000 a boolean returning true if phy is running at 1000
 *
 * This is called as part of iegbe_config_mac_to_phy() to align
 * the MAC with the PHY. It turns out on ICP_XXXX, this is not
 * done automagically.
 **/
int32_t 
iegbe_oem_phy_is_speed_1000(struct iegbe_hw *hw, int *is1000)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    int32_t ret_val;

    DEBUGFUNC1("%s",__func__);

    if(!hw || !is1000) {
        return -1;
    }
    /*
     * Make note that the M88 phy is what'll be used on Truxton.
     * see iegbe_config_mac_to_phy
     */

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS, 
			                                    &phy_data);
            if(ret_val) {
                DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
                return ret_val;
            }
            *is1000 = (phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_1000MBS;
         break;
         default:
             DEBUGOUT("Invalid PHY ID\n");
             return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !is1000) {
        return -1;
    }
    *is1000 = FALSE;
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

/**
 * iegbe_oem_phy_is_speed_100
 * @hw iegbe_hw struct containing device specific information
 * @is100 a boolean returning true if phy is running at 100
 *
 * This is called as part of iegbe_config_mac_to_phy() to align
 * the MAC with the PHY. It turns out on ICP_XXXX, this is not
 * done automagically.
 **/
int32_t
iegbe_oem_phy_is_speed_100(struct iegbe_hw *hw, int *is100)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    int32_t ret_val;

    DEBUGFUNC1("%s",__func__);

    if(!hw || !is100) {
        return -1;
    }
    /*
     * Make note that the M88 phy is what'll be used on Truxton
     * see iegbe_config_mac_to_phy
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_read_phy_reg_ex(hw, 
                                                M88E1000_PHY_SPEC_STATUS,
                                                &phy_data);
            if(ret_val) {
                DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
                return ret_val;
            }
            *is100 = (phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_100MBS;
         break;
         default:
             DEBUGOUT("Invalid PHY ID\n");
             return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !is100) {
        return -1;
    }
    *is100 = FALSE;
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

/**
 * iegbe_oem_phy_get_info
 * @hw struct iegbe_hw containing hardware specific data
 * @phy_info struct iegbe_phy_info that returned
 *
 * This is called by iegbe_phy_get_info to gather PHY specific
 * data. This is called for copper media based phys.
 **/
int32_t
iegbe_oem_phy_get_info(struct iegbe_hw *hw,
                       struct iegbe_phy_info *phy_info)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;
    uint16_t phy_data, polarity;

   DEBUGFUNC1("%s",__func__);

    if(!hw || !phy_info) {
        return -1;
    }

    /*
     * Make note that the M88 phy is what'll be used on Truxton
     * see iegbe_phy_m88_get_info
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
  /* The downshift status is checked only once, after link is
      * established and it stored in the hw->speed_downgraded parameter.*/
            phy_info->downshift = (iegbe_downshift)hw->speed_downgraded;
    
            ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_CTRL, 
                                                &phy_data);
            if(ret_val) {
                DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_CTRL\n");
                return ret_val;
            }

            phy_info->extended_10bt_distance = 
                (phy_data & M88E1000_PSCR_10BT_EXT_DIST_ENABLE) 
                 >> M88E1000_PSCR_10BT_EXT_DIST_ENABLE_SHIFT;
            phy_info->polarity_correction =
                (phy_data & M88E1000_PSCR_POLARITY_REVERSAL) 
                 >> M88E1000_PSCR_POLARITY_REVERSAL_SHIFT;

            /* Check polarity status */
            ret_val = iegbe_oem_check_polarity(hw, &polarity);
            if(ret_val) {
                return ret_val;
            }

            phy_info->cable_polarity = polarity;

            ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS, 
                                                &phy_data);
            if(ret_val) {
               DEBUGOUT("Unable to read register M88E1000_PHY_SPEC_STATUS\n");
               return ret_val;
            }

            phy_info->mdix_mode = (phy_data & M88E1000_PSSR_MDIX)
                                   >> M88E1000_PSSR_MDIX_SHIFT;

            if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_1000MBS) {
                /* Cable Length Estimation and Local/Remote Receiver Information
                 * are only valid at 1000 Mbps.
                 */
                phy_info->cable_length = 
                    (phy_data & M88E1000_PSSR_CABLE_LENGTH)
                     >> M88E1000_PSSR_CABLE_LENGTH_SHIFT;

                ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_1000T_STATUS, 
                                                    &phy_data);
                if(ret_val) {
                    DEBUGOUT("Unable to read register PHY_1000T_STATUS\n");
                    return ret_val;
                }

                phy_info->local_rx = (phy_data & SR_1000T_LOCAL_RX_STATUS) 
                                      >> SR_1000T_LOCAL_RX_STATUS_SHIFT;
    
                phy_info->remote_rx = (phy_data & SR_1000T_REMOTE_RX_STATUS) 
                                      >> SR_1000T_REMOTE_RX_STATUS_SHIFT;
            }
          
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

/**
 * iegbe_oem_phy_hw_reset
 * @hw struct iegbe_hw containing hardware specific data
 *
 * This function will perform a software initiated reset of
 * the PHY
 **/
int32_t 
iegbe_oem_phy_hw_reset(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;
    uint16_t phy_data;

    DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }
    /*
     * This code pretty much copies the default case from 
     * iegbe_phy_reset() as that is what is appropriate for
     * the M88 used in truxton. 
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_read_phy_reg_ex(hw, PHY_CTRL, &phy_data);
            if(ret_val) {
                DEBUGOUT("Unable to read register PHY_CTRL\n");
                return ret_val;
            }

            phy_data |= MII_CR_RESET;
            ret_val = iegbe_oem_write_phy_reg_ex(hw, PHY_CTRL, phy_data);
            if(ret_val) {
                DEBUGOUT("Unable to write register PHY_CTRL\n");
                return ret_val;
            }

            usec_delay(1);
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

/**
 * iegbe_oem_phy_init_script
 * @hw struct iegbe_hw containing hardware specific data
 *
 * This gets called in three places, after egbe_oem_phy_hw_reset()
 * to perform and post reset initialiation. Not all PHYs require
 * this, which is why it was split off as a seperate function.
 **/
void 
iegbe_oem_phy_init_script(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return;
    }

    /* call the GCU func that can do any phy specific init
     * functions after a reset
     * 
     * Make note that the M88 phy is what'll be used on Truxton
     *
     * The closest thing is in iegbe_phy_init_script, however this is 
     * for the IGP style of phy. This is probably a no-op for truxton
     * but may be needed by OEM's later on
     * 
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            DEBUGOUT("Nothing to do for OEM PHY Init");
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return;
    }

#endif /* ifdef EXTERNAL_MDIO */
    return;

}

/**
 * iegbe_oem_read_phy_reg_ex
 * @hw struct iegbe_hw containing hardware specific data
 * @reg_addr address location within the PHY register set
 * @phy_data returns the data read from reg_addr
 *
 * This encapsulates the interface call to the GCU for access
 * to the MDIO for the PHY.
 **/
int32_t
iegbe_oem_read_phy_reg_ex(struct iegbe_hw *hw,
                          uint32_t reg_addr,
                          uint16_t *phy_data)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;

    DEBUGFUNC1("%s",__func__);

    if(!hw || !phy_data) {
        return -1;
    }

    /* call the GCU func that will read the phy
     * 
     * Make note that the M88 phy is what'll be used on Truxton.
     *
     * The closest thing is in iegbe_read_phy_reg_ex.
     *
     * NOTE: this is 1 (of 2) functions that is truly dependant on the
     *       gcu module
     */
    
        ret_val = gcu_read_eth_phy(iegbe_oem_get_phy_dev_number(hw),
                                   reg_addr, phy_data);
        if(ret_val) {
            DEBUGOUT("Error reading GCU");
            return ret_val;
        }

      return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}
/**
 * iegbe_oem_set_trans_gasket
 * @hw: iegbe_hw struct containing device specific information
 *
 * Returns E1000_SUCCESS, negative E1000 error code on failure
 *
 * This is called from iegbe_config_mac_to_phy. Various supported 
 * Phys may require the RGMII/RMII Translation gasket be set to RMII.
 **/
int32_t 
iegbe_oem_set_trans_gasket(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO
   uint32_t ctrl_aux_reg = 0;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }

     switch (hw->phy_id) {
         case M88E1000_I_PHY_ID:
         case M88E1141_E_PHY_ID:
         /* Gasket set correctly for Marvell Phys, so nothing to do */
         break;
         /* Add your PHY_ID here if your device requires an RMII interface
         case YOUR_PHY_ID: 
             ctrl_aux_reg = E1000_READ_REG(hw, CTRL_AUX);
             ctrl_aux_reg |= E1000_CTRL_AUX_ICP_xxxx_MII_TGS; // Set the RGMII_RMII bit
         */
             E1000_WRITE_REG(hw, CTRL_AUX, ctrl_aux_reg);
         break;
         default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
     }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

/**
 * iegbe_oem_write_phy_reg_ex
 * @hw struct iegbe_hw containing hardware specific data
 * @reg_addr address location within the PHY register set
 * @phy_data data to be written to reg_addr
 *
 * This encapsulates the interface call to the GCU for access
 * to the MDIO for the PHY.
 **/
int32_t
iegbe_oem_write_phy_reg_ex(struct iegbe_hw *hw,
                           uint32_t reg_addr,
                           uint16_t phy_data)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }
    /* call the GCU func that will write to the phy
     * 
     * Make note that the M88 phy is what'll be used on Truxton.
     *
     * The closest thing is in iegbe_write_phy_reg_ex
     *
     * NOTE: this is 2 (of 2) functions that is truly dependant on the
     *       gcu module
     */
        ret_val = gcu_write_eth_phy(iegbe_oem_get_phy_dev_number(hw),
                                    reg_addr, phy_data);
        if(ret_val) {
            DEBUGOUT("Error writing to GCU");
            return ret_val;
        }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_phy_needs_reset_with_mac
 * @hw struct iegbe_hw hardware specific data
 *
 * iegbe_reset_hw is called to reset the MAC. If, for
 * some reason the PHY needs to be reset as well, this 
 * should return TRUE and then iegbe_oem_phy_hw_reset()
 * will be called.
 **/
int 
iegbe_oem_phy_needs_reset_with_mac(struct iegbe_hw *hw)
{
#ifdef EXTERNAL_MDIO

    int ret_val;

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return FALSE;
    }

    /* 
     * From the original iegbe driver, the M88
     * PHYs did not seem to need this reset, 
     * so returning FALSE.
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = FALSE;
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return FALSE;
    }

    return ret_val;

#else /* ifdef EXTERNAL_MDIO */

    return FALSE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_config_dsp_after_link_change
 * @hw struct iegbe_hw containing hardware specific data
 * @link_up allows different configurations based on whether
 *          not the link was up.
 *
 * This is called from iegbe_check_for_link, and allows for
 * tweaking of the PHY, for PHYs that support a DSP.
 *
 **/
int32_t 
iegbe_oem_config_dsp_after_link_change(struct iegbe_hw *hw,
                                       int link_up)
{
#ifdef EXTERNAL_MDIO

   DEBUGFUNC1("%s",__func__);

    if(!hw) {
        return -1;
    }

    /*
     * Make note that the M88 phy is what'll be used on Truxton,
     * but in the iegbe driver, it had no such func. This is a no-op
     * for M88, but may be useful for other phys
     *
     * use iegbe_config_dsp_after_link_change as example
     */
    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            DEBUGOUT("No DSP to configure on OEM PHY");
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_get_cable_length
 * @hw struct iegbe_hw containing hardware specific data
 * @min_length pointer to return the approx minimum length
 * @max_length pointer to return the approx maximum length
 *
 *
 **/
int32_t 
iegbe_oem_get_cable_length(struct iegbe_hw *hw,
                           uint16_t *min_length,
                           uint16_t *max_length)
{
#ifdef EXTERNAL_MDIO

    int32_t ret_val;
    uint16_t cable_length;
    uint16_t phy_data;

   DEBUGFUNC1("%s",__func__);
    
    if(!hw || !min_length || !max_length) {
        return -1;
    }

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            ret_val = iegbe_oem_read_phy_reg_ex(hw, 
                                                M88E1000_PHY_SPEC_STATUS,
                                                &phy_data);
            if(ret_val) {
                return ret_val;
            }

             cable_length = (phy_data & M88E1000_PSSR_CABLE_LENGTH)
                             >> M88E1000_PSSR_CABLE_LENGTH_SHIFT;

            /* Convert the enum value to ranged values */
            switch (cable_length) {
                case iegbe_cable_length_50:
                    *min_length = 0;
                    *max_length = iegbe_igp_cable_length_50;
                break;
                case iegbe_cable_length_50_80:
                    *min_length = iegbe_igp_cable_length_50;
                    *max_length = iegbe_igp_cable_length_80;
                break;
                case iegbe_cable_length_80_110:
                    *min_length = iegbe_igp_cable_length_80;
                    *max_length = iegbe_igp_cable_length_110;
                break;
                case iegbe_cable_length_110_140:
                    *min_length = iegbe_igp_cable_length_110;
                    *max_length = iegbe_igp_cable_length_140;
                break;
                case iegbe_cable_length_140:
                    *min_length = iegbe_igp_cable_length_140;
                    *max_length = iegbe_igp_cable_length_170;
                break;
                default:
                    return -E1000_ERR_PHY;
                break;
            }
        break;
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}


/**
 * iegbe_oem_phy_is_link_up
 * @hw iegbe_hw struct containing device specific information
 * @isUp a boolean returning true if link is up 
 *
 * This is called as part of iegbe_config_mac_to_phy() to align
 * the MAC with the PHY. It turns out on ICP_XXXX, this is not
 * done automagically.
 **/
int32_t 
iegbe_oem_phy_is_link_up(struct iegbe_hw *hw, int *isUp)
{
#ifdef EXTERNAL_MDIO

    uint16_t phy_data;
    uint16_t statusMask;
    int32_t ret_val;

    DEBUGFUNC1("%s",__func__);

    if(!hw || !isUp) {
        return -1;
    }
    /* 
     * Make note that the M88 phy is what'll be used on Truxton
     * see iegbe_config_mac_to_phy
     */

    switch (hw->phy_id) {
        case M88E1000_I_PHY_ID:
        case M88E1141_E_PHY_ID:
            iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS, &phy_data); 
            ret_val = iegbe_oem_read_phy_reg_ex(hw, M88E1000_PHY_SPEC_STATUS, 
			                                    &phy_data);
            statusMask = M88E1000_PSSR_LINK;
        break; 
        default:
            DEBUGOUT("Invalid PHY ID\n");
            return -E1000_ERR_PHY_TYPE;
    }
    if(ret_val) {
        DEBUGOUT("Unable to read PHY register\n");
        return ret_val;
    }

    *isUp = (phy_data & statusMask) != 0;

    return E1000_SUCCESS;

#else /* ifdef EXTERNAL_MDIO */

    if(!hw || !isFD) {
        return -1;
    }
    *isUp = FALSE;
    return -E1000_ERR_PHY_TYPE;

#endif /* ifdef EXTERNAL_MDIO */
}

