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
 * File Name  : flash_api.c
 *
 * Description: This file contains the implementation of the wrapper functions
 *              for the flash device interface.
 ***************************************************************************/

/** Includes. */
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#endif

#include "bcmtypes.h"
#include "board.h"
#include "flash_api.h"


/** Defines. **/
#ifndef NULL
#define NULL 0
#endif

#if !defined(INC_CFI_FLASH_DRIVER)
#define INC_CFI_FLASH_DRIVER        1
#endif

#if !defined(INC_SPI_FLASH_DRIVER)
#if defined(BCM96338)
#define INC_SPI_FLASH_DRIVER        1
#else
#define INC_SPI_FLASH_DRIVER        0
#endif
#endif


/** Externs. **/
#if (INC_CFI_FLASH_DRIVER==1)
extern int cfi_flash_init(flash_device_info_t **flash_info);
#else
#define cfi_flash_init(x)           FLASH_API_ERROR
#endif

#if (INC_SPI_FLASH_DRIVER==1)
extern int spi_flash_init(flash_device_info_t **flash_info);
#else
#define spi_flash_init(x)           FLASH_API_ERROR
#endif


/** Variables. **/
flash_device_info_t *g_flash_info = NULL;
int g_include_cfi_flash_driver = INC_CFI_FLASH_DRIVER; 
int g_include_spi_flash_driver = INC_SPI_FLASH_DRIVER; 


/***************************************************************************
 * Function Name: flash_init
 * Description  : Initialize flash part.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int flash_init(void)
{
    int ret = FLASH_API_ERROR;
    flash_device_info_t *flash_info = NULL;
    unsigned short cfi_devid = 0xffff;
    unsigned short spi_devid = 0xffff;
    char *type = "";

    if( g_include_cfi_flash_driver )
    {
        ret = cfi_flash_init( &flash_info );
        if( ret == FLASH_API_OK )
        {
            g_flash_info = flash_info;
            type = "Parallel";
        }
        else
            if( flash_info )
                cfi_devid = flash_info->flash_device_id;
    }

    if( ret != FLASH_API_OK && g_include_spi_flash_driver )
    {
        ret = spi_flash_init( &flash_info );
        if( ret == FLASH_API_OK )
        {
            g_flash_info = flash_info;
            type = "Serial";
        }
        else
            if( flash_info )
                spi_devid = flash_info->flash_device_id;
    }

    if( ret == FLASH_API_OK )
    {
        /* Flash initialization OK. */
        printk( "%s flash device: name %s, id 0x%4.4x, size %dKB\n", type,
            g_flash_info->flash_device_name, g_flash_info->flash_device_id,
            flash_get_total_size() / 1024 );
    }
    else
    {
        /* Flash initialization error. */
        if( cfi_devid != 0xffff && cfi_devid != 0x0000 )
        {
            printk( "Parallel flash device id %4.4x is not supported.\n",
                cfi_devid );
        }
        else
            if( spi_devid != 0xffff && spi_devid != 0x0000 )
            {
                printk( "Serial flash device id %4.4x is not supported.\n",
                    spi_devid );
            }
            else
                printk( "Flash device is not found.\n" );
    }

    return( ret );
} /* flash_init */

/***************************************************************************
 * Function Name: flash_sector_erase_int
 * Description  : Erase the specfied flash sector.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int flash_sector_erase_int(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_sector_erase_int) (sector)
        : FLASH_API_ERROR );
} /* flash_sector_erase_int */

/***************************************************************************
 * Function Name: flash_read_buf
 * Description  : Reads from flash memory.
 * Returns      : number of bytes read or FLASH_API_ERROR
 ***************************************************************************/
int flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_read_buf) (sector, offset, buffer, numbytes)
        : FLASH_API_ERROR );
} /* flash_read_buf */

/***************************************************************************
 * Function Name: flash_write_buf
 * Description  : Writes to flash memory.
 * Returns      : number of bytes written or FLASH_API_ERROR
 ***************************************************************************/
int flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_write_buf) (sector, offset, buffer, numbytes)
        : FLASH_API_ERROR );
} /* flash_write_buf */

/***************************************************************************
 * Function Name: flash_get_numsectors
 * Description  : Returns the number of sectors in the flash device.
 * Returns      : Number of sectors in the flash device.
 ***************************************************************************/
int flash_get_numsectors(void)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_numsectors) ()
        : FLASH_API_ERROR );
} /* flash_get_numsectors */

/***************************************************************************
 * Function Name: flash_get_sector_size
 * Description  : Returns the number of bytes in the specfied flash sector.
 * Returns      : Number of bytes in the specfied flash sector.
 ***************************************************************************/
int flash_get_sector_size(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_sector_size) (sector)
        : FLASH_API_ERROR );
} /* flash_get_sector_size */

/***************************************************************************
 * Function Name: flash_get_memptr
 * Description  : Returns the base MIPS memory address for the specfied flash
 *                sector.
 * Returns      : Base MIPS memory address for the specfied flash sector.
 ***************************************************************************/
unsigned char *flash_get_memptr(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_memptr) (sector)
        : NULL );
} /* flash_get_memptr */

/***************************************************************************
 * Function Name: flash_get_blk
 * Description  : Returns the flash sector for the specfied MIPS address.
 * Returns      : Flash sector for the specfied MIPS address.
 ***************************************************************************/
int flash_get_blk(int addr)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_blk) (addr)
        : FLASH_API_ERROR );
} /* flash_get_blk */

/***************************************************************************
 * Function Name: flash_get_total_size
 * Description  : Returns the number of bytes in the flash device.
 * Returns      : Number of bytes in the flash device.
 ***************************************************************************/
int flash_get_total_size(void)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_total_size) ()
        : FLASH_API_ERROR );
} /* flash_get_total_size */

/***************************************************************************
 * Function Name: flash_get_total_memory_mapped_size
 * Description  : Returns the number of bytes in the flash device.
 * Returns      : Number of bytes in the flash device.
 ***************************************************************************/
int flash_get_total_memory_mapped_size(void)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_total_memory_mapped_size) ()
        : FLASH_API_ERROR );
} /* flash_get_total_memory_mapped_size */

