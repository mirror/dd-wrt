#ifndef _VSC_PHY_H
#define _VSC_PHY_H

#define VSC_MII_MODE_STATUS     0x1
#define AUTONEG_COMPLETE        (1 << 5)
#define LINK_UP                 (1 << 2)

#define VSC_AUX_CTRL_STATUS     0x1c
#define FDX                     (1 << 5)
#define SPEED_STATUS            (3 << 3)

void vsc_phy_setup(int unit);
int vsc_phy_is_up(int unit);
int vsc_phy_is_fdx(int unit);
int vsc_phy_speed(int unit);

#endif
