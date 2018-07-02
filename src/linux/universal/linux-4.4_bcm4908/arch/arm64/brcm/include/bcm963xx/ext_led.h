/***************************************************************************
***
***    Copyright 2007  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without the
***    written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.  
***    Co. Ltd. company private, proprietary, and trade secret property and 
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************/

#ifndef __EXT_LED_H__
#define __EXT_LED_H__

#define EXT_LED_MAJOR_NUM           252

#define DEV_EXT_LED                 "/dev/ext_led"
#define EXT_LED_IOCTL_NUM           'N'

#define EXT_CLOCK_ON                _IOWR(EXT_LED_IOCTL_NUM, 0, int *)
#define EXT_CLOCK_OFF               _IOWR(EXT_LED_IOCTL_NUM, 1, int *)
#define EXT_LED_SET                 _IOWR(EXT_LED_IOCTL_NUM, 2, int *)
#define EXT_CLR_SET                 _IOWR(EXT_LED_IOCTL_NUM, 3, int *)
#define EXT_LED_GET                 _IOWR(EXT_LED_IOCTL_NUM, 4, int *)
#define EXT_SET_ALL                 _IOWR(EXT_LED_IOCTL_NUM, 5, int *)
#define EXT_SET_MULTI_LED           _IOWR(EXT_LED_IOCTL_NUM, 6, int *)

struct gpio_exchange{
    int gpio_pin;
    int gpio_value;
};

/*
 * Following defines should be modified for different solution
 */

/* Ctrl mode mask, bit[11:8] */
#define GPIO_CTRL_MODE_NONE             (0x0000)        /* Direct mode */
#define GPIO_CTRL_MODE_CLK_DATA         (0x0100)        /* Clk/Data mode */
#define GPIO_CTRL_MODE(x)               ((x) & 0x0F00)

/* Active mode mask, bit[12] */
#define GPIO_ACTIVE_MODE_LOW            (0x0000)
#define GPIO_ACTIVE_MODE_HIGH           (0x1000)
#define GPIO_ACTIVE_MODE(x)             ((x) & 0x0F00)

/* support max 64 gpios or max 64 extended LED pins */
#define GPIO_MAX_PIN                    (64 - 1)        /* 64 bit */
#define GPIO_PIN(x)                     ((x) & 0x00FF)

/* Clk/Data extended ctrl mode */
#define GPIO_EXT_CTRL_DATA              (1)
#define GPIO_EXT_CTRL_CLK               (0)

/* Used for direct output control */
#define EXT_LED_STATUS_ALL_ON           (~0U)
#define EXT_LED_STATUS_ALL_OFF          (0U)

/* Extended LED max shift times */
#define EXT_LED_MAX_SHIFTS              (8 - 1)         /* 8 extended pins */

#ifndef U12L311T00_S1
/* Extended LED shift defines, not gpio pins, all active low */
#define GPIO_LED_GPHY_FE              (0 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_GPHY_GE              (1 | GPIO_CTRL_MODE_CLK_DATA)
//#define GPIO_USB1_ENABLE              (2 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_USB2_ENABLE              (3 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_XDSL1_LED                (4 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_XDSL1_FAIL_LED           (5 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_24G_N                (6 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_24G_G                (7 | GPIO_CTRL_MODE_CLK_DATA)
#endif
#endif

