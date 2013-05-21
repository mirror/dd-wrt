#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7100_phy.h"
#include "ag7100.h"
#include "athrmagpie_phy.h"
/*
 * we use first three registers to pass on any information about read/write to magpie
 */
/* actions */
#define MAGPIE_ACTION_READ 0x1
#define MAGPIE_ACTION_WRITE 0x2

/* register to use */
#define MAGPIE_REG_CMD      0x0
#define MAGPIE_REG_ADDR     0x2
#define MAGPIE_REG_VAL_LO   0x4
#define MAGPIE_REG_VAL_HI   0x6
extern void mdio_reg_write(int unit, int addr, uint16_t data);
extern uint16_t mdio_reg_read(int unit,int addr);

uint32_t
athrmagpie_phy_read(int reg_offset)
{
    uint32_t val=0;
    /* first write the register offset, and issue read command */
    mdio_reg_write (0, MAGPIE_REG_CMD, 0x0);
    mdio_reg_write (0, MAGPIE_REG_ADDR, reg_offset);
    mdio_reg_write (0, MAGPIE_REG_CMD, MAGPIE_ACTION_READ);
    while (mdio_reg_read(0, MAGPIE_REG_CMD) & MAGPIE_ACTION_READ) {
        udelay(10);
    }
    val = mdio_reg_read(0, MAGPIE_REG_VAL_LO);
    val |= (mdio_reg_read (0, MAGPIE_REG_VAL_HI) << 16) ;
    return val;

}

uint32_t 
athrmagpie_phy_write ( int reg_offset, uint32_t val)
{
    /* write register value, write value, then give command */
    mdio_reg_write (0, MAGPIE_REG_ADDR, reg_offset);
    mdio_reg_write (0, MAGPIE_REG_VAL_LO, val & 0x0000ffff);
    mdio_reg_write (0, MAGPIE_REG_VAL_HI, (val & 0xffff0000 ) >> 16);
    mdio_reg_write (0, MAGPIE_REG_CMD, MAGPIE_ACTION_WRITE );
   
    while (mdio_reg_read(0, MAGPIE_REG_CMD) & MAGPIE_ACTION_WRITE ) {
        udelay(10);
    } 
}

int
athrmagpie_phy_is_up(int unit)
{
    //    printk("%s: calling \n", __func__);
    return 1; /* always on */
}

int
athrmagpie_phy_speed(int unit)
{
  //  printk("%s: calling \n", __func__);
    uint32_t val;
    /* read MII control and decide the speed */
    //printk("inside 2 %s \n", __func__);
#if 0
    val = athrmagpie_phy_read(ATHR_MAGPIE_MII_CTRL);
#else
    val = 0x20;
#endif
    if ( (val & 0x30) == 0x10) return 1;
    else if ( ( val & 0x30) == 0x20)return 2; /* return 1000 always now */
}

int
athrmagpie_phy_is_fdx (int unit )
{
    // printk("%s: calling \n", __func__);
    return 1; /* say magpie is always full duplex */
}
void
athrmagpie_phy_setup (int unit)
{
    
    uint32_t val;

    printk("%s: calling \n", __func__);
    return ;
    val = athrmagpie_phy_read(ATHR_MAGPIE_MII_CTRL);
    printk("phy cmd cmd:%d addr: %x val: %x\n", 1, ATHR_MAGPIE_MII_CTRL, val);
    val = athrmagpie_phy_write(ATHR_MAGPIE_MII_CTRL, 0x15);
    val = athrmagpie_phy_read(ATHR_MAGPIE_MII_CTRL);
    printk("phy cmd cmd:%d addr: %x val: %x\n", 1, ATHR_MAGPIE_MII_CTRL, val);
    /* set 1000 mbps and full duplex */
    return; /* right now do nothing, every thing comes through gmac driver */
}
