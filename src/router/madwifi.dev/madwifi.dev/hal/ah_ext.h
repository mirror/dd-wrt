/*
 * Hidden bonus features for the Atheros Hardware Abstraction Layer (HAL)
 * :)
 */
#ifndef __AH_EXT_H
#define __AH_EXT_H

#define HAL_EXT_CAP_BASE	64

enum {
	HAL_CAP_SUPERCH		= HAL_EXT_CAP_BASE + 0,
	HAL_CAP_POWERFIX	= HAL_EXT_CAP_BASE + 1,
	HAL_CAP_ANTGAIN		= HAL_EXT_CAP_BASE + 2,
	HAL_CAP_ANTGAINSUB	= HAL_EXT_CAP_BASE + 3,
	HAL_CAP_CHANSHIFT	= HAL_EXT_CAP_BASE + 4,
};

/* for channel privFlags */
#define CHANNEL_NOSCAN	0x10

#endif
