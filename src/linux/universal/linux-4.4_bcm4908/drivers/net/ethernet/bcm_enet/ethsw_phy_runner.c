/*
 <:copyright-BRCM:2011:DUAL/GPL:standard
 
    Copyright (c) 2011 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
 :>
*/

#include <bcm_map_part.h>
#include "bcmenet_common.h"
#include "bcmswaccess.h"
#include "ethsw_runner.h"
#include "ethsw_phy.h"
#include "phys_common_drv.h"

extern spinlock_t bcm_ethlock_phy_access;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
static int wait_mdio_op(int *reg_out)
{
    int i;
    volatile uint32 *sf2_mdio_command_reg     = (void *)(SF2_MDIO_COMMAND_REG);

#define MDIO_READ_BACK_RETRY 6
    for(i=0; i<MDIO_READ_BACK_RETRY; i++)
    {
        *reg_out = *sf2_mdio_command_reg;
        if ((*reg_out & SF2_MDIO_BUSY) == 0) break;
        udelay(20);
    }

    if (i == MDIO_READ_BACK_RETRY)
    {
        printk("****** Error: MDIO No Response!");
        return 0;
    }

    if ((*reg_out & SF2_MDIO_FAIL))
    {
        printk("****** Error: MDIO Access Returned FAIL!");
        return 0;
    }
    return 1;
}

static int mdio_reg_op(int phyId, int reg_in, u32 *data)
{
    int reg_out = 0, loop, op = reg_in & SF2_MDIO_CMD_M;
    volatile uint32 *sf2_mdio_command_reg     = (void *)(SF2_MDIO_COMMAND_REG);
    volatile uint32 cl22;

#define MDIO_READ_LOOP 2
    cl22 = (*(uint32 *)(SF2_MDIO_CONFIG_REG)) & SF2_MDIO_CONFIG_CLAUSE22;
    loop = (cl22 && (op & SF2_MDIO_CMD_C22_READ))? MDIO_READ_LOOP : 1;  /* twice read back to work around */
    for(; loop>0; loop--)   /* Multiple reads for reliability */
    {
        *sf2_mdio_command_reg = reg_in | SF2_MDIO_BUSY;
        if (!wait_mdio_op(&reg_out)) goto failed;
    }

    if (data) *data = reg_out & 0xffff;

    return 0;

failed:
    printk(" %s Access: Op: %s; PortAddr: 0x%02x; DevAddr: 0x%02x; Reg_in: 0x%08x; Reg_out: 0x%08x\n", 
            cl22? "CL22": "CL45", 
            (op & SF2_MDIO_CMD_C22_READ)? "Read": (op==SF2_MDIO_CMD_WRITE)? "Write": "Address",
            (reg_in & SF2_MDIO_PHY_PORT_ADDR_M) >> SF2_MDIO_PHY_PORT_ADDR_S,
            (reg_in & SF2_MDIO_PHY_REG_DEV_M) >> SF2_MDIO_PHY_REG_DEV_S, reg_in, reg_out);
    return -1;
}

/* Return: 0: Success, -1: Failed */
static int ethSwPhyWriteRegisterLock(int phy_id, u32 reg, u32 data, int ext, int locked)
{
    uint32_t reg_value = 0;
    volatile uint32 *sf2_mdio_config_reg      = (void *)(SF2_MDIO_CONFIG_REG);
    int rc;

    phy_id &= BCM_PHY_ID_M;

    if(ext == ETHCTL_FLAG_ACCESS_I2C_PHY)
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL) && defined(CONFIG_I2C)
        sfp_i2c_phy_write(reg, data);
#else
        printk("Error: I2C PHY is not supported\n");
#endif
        return -1;
    }

    if (ext == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE)
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
        ethsw_serdes_power_mode_set(phy_id, reg);
#else
        printk("Error: Serdes is not supported in this platform\n");
#endif
        return -1;
    }

    if (locked == 0)
    {
        spin_lock(&bcm_ethlock_phy_access);
        sf2_mdio_master_disable();
    }

#if defined(SERDES_2P5G_CAPABLE)
    if(reg >= 0x200000 || (ext & ETHCTL_FLAG_ACCESS_32BIT)) /* Clause 45 32bit register access */
    {
        uint32_t reg_hi, reg_lo, data_hi, data_lo;
        reg_lo = reg & 0xffff;
        reg_hi = (reg >> 16) & 0xffff;
        data_lo = data & 0xffff;
        data_hi = (data >> 16) & 0xffff;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_ADDR_LO, reg_lo, 0, 1))) goto failed;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_ADDR_HI, reg_hi, 0, 1))) goto failed;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_DATA_LO, data_lo, 0, 1))) goto failed;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_DATA_HI, data_hi, 0, 1))) goto failed;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_CTRL, CL45_DNLD_WRT_DWORD, 0, 1))) goto failed;
    }
    else 
#endif
    if (reg < 0x20)   /* Clause 22 */
    {
        reg_value = SF2_MDIO_CMD_WRITE | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            ((reg << SF2_MDIO_PHY_REG_DEV_S) & SF2_MDIO_PHY_REG_DEV_M) | data;

        if ((rc = mdio_reg_op(phy_id, reg_value, 0))) goto end;
    }
    else if (reg > 0xffff && reg < 0x200000) /* Clause 45 */
    {
        *sf2_mdio_config_reg &= ~SF2_MDIO_CONFIG_CLAUSE22;

        reg_value = SF2_MDIO_CMD_ADDR_C45 | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            (reg & (SF2_MDIO_PHY_REG_DEV_M | SF2_MDIO_PHY_ADDR_DATA_M));
        if ((rc = mdio_reg_op(phy_id, reg_value, 0))) goto failed;

        reg_value = SF2_MDIO_CMD_WRITE | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            (reg & SF2_MDIO_PHY_REG_DEV_M) | (data & SF2_MDIO_PHY_ADDR_DATA_M);
        if ((rc = mdio_reg_op(phy_id, reg_value, 0))) goto failed;

    }
    else    /* 0x20-0xffff should be handled by general expanded register access function */
    {
        printk("Error: Wrong address range: PHY ID: %x, reg: %x\n", phy_id, reg);
        rc = -1;
    }

end:
    *sf2_mdio_config_reg |= SF2_MDIO_CONFIG_CLAUSE22;
    if (locked == 0)
    {
        sf2_mdio_master_enable();
        spin_unlock(&bcm_ethlock_phy_access);
    }
    return rc;
failed:
    printk (" ***** Error: failed during access\n");
    goto end;
}

static int ethSwPhyReadRegisterLock(int phy_id, u32 reg, u32 *data, int ext, int locked)
{
    uint32_t reg_in = 0, reg_out = 0;
    volatile uint32 *sf2_mdio_config_reg      = (void *)(SF2_MDIO_CONFIG_REG);
    int rc = 0;

    phy_id &= BCM_PHY_ID_M;

    if(ext == ETHCTL_FLAG_ACCESS_I2C_PHY)
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL) && defined(CONFIG_I2C)
        sfp_i2c_phy_read(reg, &reg_out);
#else
        printk("Error: I2C PHY is not supported\n");
        rc = -1;
#endif
        goto endSfp;
    }

    if (ext == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE)
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
        ethsw_serdes_power_mode_get(phy_id, &reg_out);
#else
        printk("Error: Serdes is not supported in this platform\n");
#endif
        goto endSfp;
    }

    if (locked == 0)
    {
        spin_lock(&bcm_ethlock_phy_access);
        sf2_mdio_master_disable();
    }

#if defined(SERDES_2P5G_CAPABLE)
    if(reg >= 0x200000 || (ext & ETHCTL_FLAG_ACCESS_32BIT)) /* Clause 45 32bit register access */
    {
        uint32_t reg_lo, reg_hi, data_lo, data_hi;
        reg_lo = reg & 0xffff;
        reg_hi = (reg >> 16) & 0xffff;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_ADDR_LO, reg_lo, 0, 1))) goto end;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_ADDR_HI, reg_hi, 0, 1))) goto end;
        if ((rc = ethSwPhyWriteRegisterLock(phy_id, CL45_REG_DNLD_CTRL, CL45_DNLD_READ_DWORD, 0, 1))) goto end;
        if ((rc = ethSwPhyReadRegisterLock(phy_id, CL45_REG_DNLD_DATA_LO, &data_lo, 0, 1))) goto end;
        if ((rc = ethSwPhyReadRegisterLock(phy_id, CL45_REG_DNLD_DATA_HI, &data_hi, 0, 1))) goto end;
        reg_out = (data_hi << 16) | data_lo;
    }
    else 
#endif /* defined(SERDES_2P5G_CAPABLE) */
    if (reg < 0x20)   /* Clause 22 */
    {
        reg_in = SF2_MDIO_CMD_C22_READ | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            ((reg << SF2_MDIO_PHY_REG_DEV_S) & SF2_MDIO_PHY_REG_DEV_M);
        if((rc = mdio_reg_op(phy_id, reg_in, &reg_out))) goto end;
    }
    else if (reg > 0xffff && reg < 0x200000) /* Clause 45 */
    {
        *sf2_mdio_config_reg &= ~SF2_MDIO_CONFIG_CLAUSE22;
        reg_in = SF2_MDIO_CMD_ADDR_C45 | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            (reg & (SF2_MDIO_PHY_REG_DEV_M | SF2_MDIO_PHY_ADDR_DATA_M));
        if ((rc = mdio_reg_op(phy_id, reg_in, 0))) goto end;

        reg_in = SF2_MDIO_CMD_C45_READ | ((phy_id << SF2_MDIO_PHY_PORT_ADDR_S) & SF2_MDIO_PHY_PORT_ADDR_M)|
            (reg & SF2_MDIO_PHY_REG_DEV_M);
        if ((rc = mdio_reg_op(phy_id, reg_in, &reg_out))) goto end;
    }
    else    /* 0x20-0xffff should be handled by general expanded register access function */
    {
        printk("Error: Wrong address range: PHY ID: %x, reg: %x\n", phy_id, reg);
        rc = -1;
    }

end:
    *sf2_mdio_config_reg |= SF2_MDIO_CONFIG_CLAUSE22;
    if (locked == 0)
    {
        sf2_mdio_master_enable();
        spin_unlock(&bcm_ethlock_phy_access);
    }

endSfp:
    *data = reg_out;

    return rc;
}
#endif

int ethsw_phy_rw_reg32_chip(int phy_id, u32 reg, u32 *data, int ext_bit, int rd)
{
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
    return rd? (*data = phy_read_register(phy_id|(ext_bit? PHY_EXTERNAL:0), reg)):
        phy_write_register(phy_id|(ext_bit?PHY_EXTERNAL:0), reg, (u16)(*data));
#else
    return rd? ethSwPhyReadRegisterLock(phy_id, reg, data, ext_bit, 0): 
        ethSwPhyWriteRegisterLock(phy_id, reg, *data, ext_bit, 0);
#endif
}

void ethsw_ephy_shadow_rw(int phy_id, int bank, uint16 reg, uint16 *data, int write)
{
    _ethsw_ephy_shadow_rw(phy_id, bank, reg, data, write);
}


MODULE_LICENSE("GPL");

