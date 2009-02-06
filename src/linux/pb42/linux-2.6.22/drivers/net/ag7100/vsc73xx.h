/* vsc73xx.h 
 *
 * History:
 * Jan  4, 2007 wclewis initial
 * Jan 12, 2007 wclewis ready for checkin
 * May 24, 2007 Tag before BSP resturcture 
 */

#ifndef _VSC73XX_H
#define _VSC73XX_H

#include "ag7100.h"

int
vsc73xx_rd(int block, int subblock, int reg, unsigned int  *value);

int
vsc73xx_wr(int block, int subblock, int reg, unsigned int  value);

/* Extern for heathrow.c and other vitesse provided stuff  */
#define vtss_io_si_rd vsc73xx_rd_vsc7395
#define vtss_io_si_wr vsc73xx_wr_vsc7395

#define VSC73XX_GPIO_0          0x01
#define VSC73XX_GPIO_1          0x02
#define VSC73XX_GPIO_2          0x04
#define VSC73XX_GPIO_3          0x08

#define VSC73XX_GPIO_MASK       (VSC73XX_GPIO_0 | VSC73XX_GPIO_1 | VSC73XX_GPIO_2 | VSC73XX_GPIO_3 )

#define VSC73XX_MAC_CFG_WEXC_DIS	(1<<31)
#define VSC73XX_MAC_CFG_PORT_RST        (1<<29)
#define VSC73XX_MAC_CFG_TX_EN       	(1<<28)
#define VSC73XX_MAC_CFG_FDX	        (1<<18)
#define VSC73XX_MAC_CFG_GIGA_MODE       (1<<17)
#define VSC73XX_MAC_CFG_RX_EN           (1<<16)
#define VSC73XX_MAC_CFG_100_BASE_T      (1<<13)
#define VSC73XX_MAC_CFG_TX_IPG(y)       ((y&0x1f)<<6)
#define VSC73XX_MAC_CFG_MAC_RX_RST      (1<<5)
#define VSC73XX_MAC_CFG_MAC_TX_RST      (11<4)
#define VSC73XX_MAC_CFG_CLK_SEL(y)      ((y&0x3)<<0)

#define VSC73XX_MAC_CFG_HYDRA_MASK ( \
VSC73XX_MAC_CFG_WEXC_DIS   | \
VSC73XX_MAC_CFG_PORT_RST   | \
VSC73XX_MAC_CFG_TX_EN      | \
VSC73XX_MAC_CFG_FDX        | \
VSC73XX_MAC_CFG_GIGA_MODE  | \
VSC73XX_MAC_CFG_RX_EN      | \
VSC73XX_MAC_CFG_100_BASE_T | \
VSC73XX_MAC_CFG_TX_IPG(31) | \
VSC73XX_MAC_CFG_MAC_RX_RST | \
VSC73XX_MAC_CFG_MAC_TX_RST | \
VSC73XX_MAC_CFG_CLK_SEL(3)   \
)

#define VSC73XX_MAC_CFG_HYDRA ( \
VSC73XX_MAC_CFG_TX_EN      | \
VSC73XX_MAC_CFG_FDX        | \
VSC73XX_MAC_CFG_GIGA_MODE  | \
VSC73XX_MAC_CFG_RX_EN      | \
VSC73XX_MAC_CFG_TX_IPG(6)  | \
VSC73XX_MAC_CFG_CLK_SEL(3)   \
)

#define VSC73XX_MAC_CFG_AR9100 ( \
VSC73XX_MAC_CFG_TX_EN      | \
VSC73XX_MAC_CFG_FDX        | \
VSC73XX_MAC_CFG_GIGA_MODE  | \
VSC73XX_MAC_CFG_RX_EN      | \
VSC73XX_MAC_CFG_TX_IPG(6)  | \
4                            \
)

#define VSC73XX_MAC_CFG_CLK_RGMI_125MHZ 1
#define VSC73XX_MAC_CFG_CLK_RGMI_25MHZ  2
#define VSC73XX_MAC_CFG_CLK_RGMI_2_5MHZ 3

#define VSC73XX_CLOCK_DELAY       (3<<4|3)
#define VSC73XX_CLOCK_DELAY_MASK  (3<<4|3)

#define VSC73XX_ADVPORTM_IFG_PPM       (1<<7)
#define VSC73XX_ADVPORTM_EXC_COL_CONT  (1<<6)
#define VSC73XX_ADVPORTM_EXT_PORT      (1<<5)
#define VSC73XX_ADVPORTM_INV_GTX       (1<<4)
#define VSC73XX_ADVPORTM_ENA_GTX       (1<<3)
#define VSC73XX_ADVPORTM_DDR_MODE      (1<<2)
#define VSC73XX_ADVPORTM_IO_LOOPBACK   (1<<1)
#define VSC73XX_ADVPORTM_HOST_LOOPBACK (1<<0)

#define VSC73XX_ADVPORTM_HYDRA_MASK ( \
VSC73XX_ADVPORTM_IFG_PPM       | \
VSC73XX_ADVPORTM_EXC_COL_CONT  | \
VSC73XX_ADVPORTM_EXT_PORT      | \
VSC73XX_ADVPORTM_INV_GTX       | \
VSC73XX_ADVPORTM_ENA_GTX       | \
VSC73XX_ADVPORTM_DDR_MODE      | \
VSC73XX_ADVPORTM_IO_LOOPBACK   | \
VSC73XX_ADVPORTM_HOST_LOOPBACK   \
)

#define VSC73XX_ADVPORTM_HYDRA ( \
VSC73XX_ADVPORTM_EXT_PORT      | \
VSC73XX_ADVPORTM_ENA_GTX       | \
VSC73XX_ADVPORTM_DDR_MODE        \
)

/* Port 0..4, 6 */

void
vsc73xx_print_counts(int port);

#define VSC73XX_PORT_0      0
#define VSC73XX_PORT_1      1
#define VSC73XX_PORT_2      2
#define VSC73XX_PORT_3      3
#define VSC73XX_PORT_4      4

#define VSC73XX_PORT_MAC    6

/* ********************************** INTERFACE TO AG7100 ********************* */

int 
vsc73xx_setup(int unit);

int
vsc73xx_get_link_status(int unit, int *up, int *fdx, ag7100_phy_speed_t *speed, unsigned int *cfg);

int
vsc73xx_phy_print_link_status(int unit);

/* This command may be used to read/write to the PHY in the case where the
 * switch code is running on the host. The 8051 processor is not running.
 *
 * If 8051 processor on the switch is running the 'unmanaged' application than
 * it is unwise to use the vsc73xx_rw_phy() command since the switch 8051 code
 * is also using that interface.
 */
unsigned short
vsc73xx_rw_phy(int writeFlg, int unit, int phy_addr, int reg, uint16_t value);

#ifdef USE_TEST_CODE

void
vsc73xx_test_reset_line(void);

void
vsc73xx_test_reset_and_verify_chipid(void);

void
vsc73xx_test_gpio(void);

void
vsc73xx_test_load_and_reset_firmware(void);

void
vsc73xx_test_link_status(void);

#endif

#endif
