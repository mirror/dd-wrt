#ifndef _ATHRS16_PHY_H
#define _ATHRS16_PHY_H

#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)

#ifndef BOOL
#define BOOL    int
#endif

#define RTL_8309G_PHY_CTRL_REG			0x00
#define RTL_8309G_PHY_SPEC_STATUS		0x01
#define RTL_8309G_PHY_ID1			0x02
#define RTL_8309G_PHY_ID2			0x03
#define RTL_8309G_PHY_AUTO_NEG			0x04

#define RTL_8309G_PHY_AUTO_NEG_VALUE    0x05e1
#define RTL_8309G_STATUS_LINK_PASS		0x0004
#define RTL_8309G_PHY_CTRL_VALE			0x3100
#define RTL_AUTO_NEG_CHECK				0x0020
#define RTL_CTRL_SOFTWARE_RESET			0x8000

#define RTL_AUTO_NEG_DONE(phy_control)  (((phy_control) & (RTL_AUTO_NEG_CHECK)) == 0)
#define RTL_AUTO_LINK_UP(phy_control)   (((phy_control) & (RTL_AUTO_LINK_CHECK)) == 0)
#define RTL_RESET_DONE(phy_control)     (((phy_control) & (RTL_CTRL_SOFTWARE_RESET)) == 0)

#define RTL_AUTO_LINK_CHECK				0x0004

BOOL rtl8309g_phy_is_link_alive(int phyUnit);
BOOL rtl8309g_phy_setup(int ethUnit);
void rtl8309g_reg_init(int ethUnit);
int rtl8309g_phy_is_fdx(int ethUnit);
int rtl8309g_phy_speed(int ethUnit);
int rtl8309g_phy_is_up(int ethUnit);

#endif
