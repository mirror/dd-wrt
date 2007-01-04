/*
<:copyright-broadcom

 Copyright (c) 2005 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/
/***************************************************************************
 * File Name  : flash_api.h
 *
 * Description: This file contains definitions and prototypes for a public
 *              flash device interface and an internal flash device interface.
 ***************************************************************************/

#if !defined(_FLASH_API_H)
#define _FLASH_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Flash definitions. */
#define FLASH_API_OK                1
#define FLASH_API_ERROR             -1

/* Public Interface Prototypes. */
int flash_init(void);
int flash_sector_erase_int(unsigned short sector);
int flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_get_numsectors(void);
int flash_get_sector_size(unsigned short sector);
unsigned char *flash_get_memptr(unsigned short sector);
int flash_get_blk(int addr);
int flash_get_total_size(void);
int flash_get_total_memory_mapped_size(void);


/* Internal Flash Device Driver Information. */
typedef struct flash_device_info_s
{
    unsigned short flash_device_id;
    char flash_device_name[30];

    int (*fn_flash_sector_erase_int) (unsigned short sector);
    int (*fn_flash_read_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_write_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_get_numsectors) (void);
    int (*fn_flash_get_sector_size) (unsigned short sector);
    unsigned char * (*fn_flash_get_memptr) (unsigned short sector);
    int (*fn_flash_get_blk) (int addr);
    int (*fn_flash_get_total_size) (void);
    int (*fn_flash_get_total_memory_mapped_size) (void);
} flash_device_info_t;

#ifdef __cplusplus
}
#endif

#endif /* _FLASH_API_H */

