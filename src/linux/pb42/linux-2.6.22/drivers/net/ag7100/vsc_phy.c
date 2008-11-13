#include <linux/kernel.h>
#include "ag7100.h"
#include "ag7100_phy.h"

typedef struct {
    int     is_enet_port;
    int     mac_unit;
    u32     phy_addr;
}vsc_phy_t;

vsc_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
     phy_addr    : 0x18},

    {is_enet_port: 1,
     mac_unit    : 1,
     phy_addr    : 0x10}
};


static vsc_phy_t *
vsc_phy_find(int unit)
{
    int i, found = 0;
    vsc_phy_t *phy;

    for(i = 0; i < sizeof(phy_info)/sizeof(vsc_phy_t); i++) {
        phy = &phy_info[i];
        if (phy->is_enet_port && (phy->mac_unit == unit)) 
            return phy;
    }
    return NULL;
}

void
vsc_phy_setup(int unit)
{
    vsc_phy_t *phy = vsc_phy_find(unit);

    if (!phy) {
        printk("\nNo phy found for unit %d\n", unit);
        return;
    }

    printk("unit %d phy addr %#x ", unit, phy->phy_addr);
    printk("reg0 %#x\n", ag7100_mii_read(0, phy->phy_addr, 0));
    ag7100_mii_write(0, phy->phy_addr, 0, 0x8000);
    ag7100_mii_write(0, phy->phy_addr, 0x1c, 0x4);
    /* enable rgmii skew compensation */
    ag7100_mii_write(0, phy->phy_addr, 0x17, 0x11000);
    //ag7100_mii_write(0, phy->phy_addr, 0, 0x2100);
    ag7100_mii_write(0, phy->phy_addr, 31, 0x2a30);
    ag7100_mii_write(0, phy->phy_addr, 8, 0x10);
    ag7100_mii_write(0, phy->phy_addr, 31, 0);
    ag7100_mii_write(0, phy->phy_addr, 0x12, 0x0008);
    

}

int
vsc_phy_is_up(int unit)
{
    int status;
    vsc_phy_t *phy = vsc_phy_find(unit);

    if (!phy) 
        return 0;

    status = ag7100_mii_read(0, phy->phy_addr, VSC_MII_MODE_STATUS);

    //if ((status & (AUTONEG_COMPLETE | LINK_UP)) == (AUTONEG_COMPLETE | LINK_UP))
    if ((status & (LINK_UP)) == (LINK_UP))
        return 1;

    return 0;
}

int
vsc_phy_is_fdx(int unit)
{
    int status;
    vsc_phy_t *phy = vsc_phy_find(unit);

    if (!phy) 
        return 0;

    status = ag7100_mii_read(0, phy->phy_addr, VSC_AUX_CTRL_STATUS);
    status = ((status & FDX) >> 5);

    return (status);
}

int
vsc_phy_speed(int unit)
{
    int status;
    vsc_phy_t *phy = vsc_phy_find(unit);

    if (!phy) 
        return 0;

    status = ag7100_mii_read(0, phy->phy_addr, VSC_AUX_CTRL_STATUS);
    status = ((status & SPEED_STATUS) >> 3);

    switch(status) {
        case 0:
            return AG7100_PHY_SPEED_10T;
        case 1:
            return AG7100_PHY_SPEED_100TX;
        case 2:
            return AG7100_PHY_SPEED_1000T;
        default:
            printk("Unkown speed read!\n");
    }
}
