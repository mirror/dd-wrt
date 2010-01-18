#ifndef SL2312_FLASH_NAND_H
#define SL2312_FLASH_NAND_H

#include <linux/wait.h>
#include <linux/spinlock.h>

/*Add function*/
static void nand_read_id(int chip_no,unsigned char *id);



#define	NFLASH_WiDTH8              0x00000000
#define	NFLASH_WiDTH16             0x00000400
#define	NFLASH_WiDTH32             0x00000800
#define NFLASH_CHIP0_EN            0x00000000  // 16th bit = 0
#define NFLASH_CHIP1_EN            0x00010000  // 16th bit = 1
#define	NFLASH_DIRECT              0x00004000
#define	NFLASH_INDIRECT            0x00000000


#define	DWIDTH             NFLASH_WiDTH8


#endif /* SL2312_FLASH_NAND_H */
