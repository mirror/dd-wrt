#ifndef RALINK_MTD_H
#define RALINK_MTD_H

/* this header is included by UBOOT only */

#if defined (__UBOOT__)
#include <linux/mtd/mtd.h>

#if defined (CFG_ENV_IS_IN_NAND)
#include "drivers/mtd/ralink/ralink_nand.h"
#endif

#if defined (CFG_ENV_IS_IN_SPI)
#include "drivers/mtd/ralink/ralink_spi.h"
#endif


#endif //__UBOOT__

#endif
