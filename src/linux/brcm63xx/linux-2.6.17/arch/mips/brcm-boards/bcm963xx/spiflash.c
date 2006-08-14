/************************************************************************/
/*                                                                      */
/*  SPI Flash Memory Drivers                                            */
/*  File name: spiflash.c                                               */
/*  Revision:  1.0  1/27/2004                                           */
/*                                                                      */
/************************************************************************/                        

/** Includes. **/
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"       
#define printk  printf
#else       // linux
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <bcm_map_part.h>       
#endif

#include "bcmtypes.h"
#include "board.h"
#include "flash_api.h"


/** Defines. **/
#define OSL_DELAY(X)                        \
    do { { int i; for( i = 0; i < (X) * 500; i++ ) ; } } while(0)

#define MAX_RETRY           3

#define FAR

#ifndef NULL
#define NULL 0
#endif

#define MAX_MEMORY_MAPPED_SIZE      (1024 * 1024)

#define MAXSECTORS          1024    /* maximum number of sectors supported */

#define FLASH_PAGE_SIZE     256
#define SECTOR_SIZE_4K      (4 * 1024)
#define SECTOR_SIZE_32K     (32 * 1024)
#define SECTOR_SIZE_64K     (64 * 1024)
#define SST25VF020_SECTOR   64  /* 2 Mbit */
#define SST25VF040_SECTOR   128 /* 4 Mbit */
#define SST25VF080_SECTOR   256 /* 8 Mbit */
#define SST25VF016B_SECTOR  512 /* 16 Mbit */
#define AT25F512_SECTOR     2
#define AT25F2048_SECTOR    4
#define AMD25FL_SECTOR      4

/* use 60 instead 63 for aligned on word (4 bytes) boundaries */
#define MAX_READ            60
#define CMD_LEN_1           1
#define CMD_LEN_4           4

/* Standard Boolean declarations */
#define TRUE                1
#define FALSE               0

#define SPI_STATUS_OK               0
#define SPI_STATUS_INVALID_LEN      -1

/* Command codes for the flash_command routine */
#define FLASH_READ          0x03    /* read data from memory array */
#define FLASH_PROG          0x02    /* program data into memory array */
#define FLASH_WREN          0x06    /* set write enable latch */
#define FLASH_WRDI          0x04    /* reset write enable latch */
#define FLASH_RDSR          0x05    /* read status register */
#define FLASH_WRST          0x01    /* write status register */
#define FLASH_EWSR          0x50    /* enable write status */
#define FLASH_WORD_AAI      0xAD    /* auto address increment word program */
#define FLASH_AAI           0xAF    /* auto address increment program */

#define SST_FLASH_CERASE    0x60    /* erase all sectors in memory array */
#define SST_FLASH_SERASE    0x20    /* erase one sector in memroy array */
#define SST_FLASH_RDID      0x90    /* read manufacturer and product id */

#define ATMEL_FLASH_CERASE  0x62    /* erase all sectors in memory array */
#define ATMEL_FLASH_SERASE  0x52    /* erase one sector in memroy array */
#define ATMEL_FLASH_RDID    0x15    /* read manufacturer and product id */

#define AMD_FLASH_CERASE    0xC7    /* erase all sectors in memory array */
#define AMD_FLASH_SERASE    0xD8    /* erase one sector in memroy array */
#define AMD_FLASH_RDID      0xAB    /* read manufacturer and product id */

/* RDSR return status bit definition */
#define SR_WPEN             0x80
#define SR_BP2              0x10
#define SR_BP1              0x08
#define SR_BP0              0x04
#define SR_WEN              0x02
#define SR_RDY              0x01

/* Return codes from flash_status */
#define STATUS_READY        0       /* ready for action */
#define STATUS_BUSY         1       /* operation in progress */
#define STATUS_TIMEOUT      2       /* operation timed out */
#define STATUS_ERROR        3       /* unclassified but unhappy status */

/* Used to mask of bytes from word data */
#define HIGH_BYTE(a)        (a >> 8)
#define LOW_BYTE(a)         (a & 0xFF)

/* Define different type of flash */
#define FLASH_UNDEFINED     0
#define FLASH_SST           1
#define FLASH_ATMEL         2
#define FLASH_AMD           3

/* ATMEL's manufacturer ID */
#define ATMELPART           0x1F

/* A list of ATMEL device ID's - add others as needed */
#define ID_AT25F512         0x60
#define ID_AT25F2048        0x63

/* AMD's device ID */
#define AMD_S25FL002D       0x11

/* SST's manufacturer ID */
#define SSTPART             0xBF

/* A list of SST device ID's - add others as needed */
#define ID_SST25VF016B      0x41
#define ID_SST25VF020       0x43
#define ID_SST25VF040       0x44
#define ID_SST25VF080       0x80

#define SPI_MAKE_ID(A,B)    \
    (((unsigned short) (A) << 8) | ((unsigned short) B & 0xff))

#define SPI_FLASH_DEVICES                                   \
    {{SPI_MAKE_ID(ATMELPART, ID_AT25F512), "AT25F512"},     \
     {SPI_MAKE_ID(ATMELPART, ID_AT25F2048), "AT25F2048"},   \
     {SPI_MAKE_ID(AMD_S25FL002D, 0), "AMD_S25FL002D"},      \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF016B), "SST25VF016B"}, \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF020), "SST25VF020"},   \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF040), "SST25VF040"},   \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF080), "SST25VF080"},   \
     {0,""}                                                 \
    }


/** Structs. **/
/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {
    char *name;         /* "AT25F512", etc. */
    unsigned long addr; /* physical address, once translated */
    int nsect;          /* # of sectors */
    struct {
        long size;      /* # of bytes in this sector */
        long base;      /* offset from beginning of device */
    } sec[MAXSECTORS];  /* per-sector info */
};

struct flash_name_from_id {
    unsigned short fnfi_id;
    char fnfi_name[30];
};


/** Prototypes. **/
int spi_flash_init(flash_device_info_t **flash_info);
static int spi_read( unsigned char *msg_buf, int prependcnt, int nbytes );
static int spi_write( unsigned char *msg_buf, int nbytes );
static int spi_flash_sector_erase_int(unsigned short sector);
static int spi_flash_reset(void);
static int spi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes);
static int spi_flash_ub(unsigned short sector);
static int spi_flash_write_status(unsigned char status);
static int spi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes);
static int spi_flash_reset_ub(void);
static int spi_flash_get_numsectors(void);
static int spi_flash_get_sector_size(unsigned short sector);
static unsigned char *spi_get_flash_memptr(unsigned short sector);
static unsigned char *spi_flash_get_memptr(unsigned short sector);
static int spi_flash_write(unsigned short sector, int offset, unsigned char *buf,
    int nbytes,int ub);
static int spi_flash_status(void);
static unsigned short spi_flash_get_device_id(unsigned short sector);
static int spi_flash_get_blk(int addr);
static int spi_flash_get_total_size(void);
static int spi_flash_get_total_memory_mapped_size(void);


/** Variables. **/
static flash_device_info_t flash_spi_dev =
    {
        0xffff,
        "",
        spi_flash_sector_erase_int,
        spi_flash_read_buf,
        spi_flash_write_buf,
        spi_flash_get_numsectors,
        spi_flash_get_sector_size,
        spi_flash_get_memptr,
        spi_flash_get_blk,
        spi_flash_get_total_size,
        spi_flash_get_total_memory_mapped_size
    };

/*********************************************************************/
/* 'meminfo' should be a pointer, but most C compilers will not      */
/* allocate static storage for a pointer without calling             */
/* non-portable functions such as 'new'.  We also want to avoid      */
/* the overhead of passing this pointer for every driver call.       */
/* Systems with limited heap space will need to do this.             */
/*********************************************************************/
static struct flashinfo meminfo; /* Flash information structure */
static int totalSize = 0;
static int flashFamily = FLASH_UNDEFINED;
static int sstAaiWordProgram = FALSE;

static int spi_read( unsigned char *msg_buf, int prependcnt, int nbytes )
{
    int i;
    SPI->spiMsgCtl = (nbytes << SPI_BYTE_CNT_SHIFT |
                            HALF_DUPLEX_R << SPI_MSG_TYPE_SHIFT);
    
    for (i = 0; i < prependcnt; i++)
        SPI->spiMsgData[i] = msg_buf[i];

    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                    0 << SPI_CMD_DEVICE_ID_SHIFT | 
                    prependcnt << SPI_CMD_PREPEND_BYTE_CNT_SHIFT );


    while (!(SPI->spiIntStatus & SPI_INTR_CMD_DONE));

    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

    for(i = 0; i < nbytes; i++) {
        msg_buf[i] = SPI->spiRxDataFifo[i];
    }
    return SPI_STATUS_OK;
}

static int spi_write( unsigned char *msg_buf, int nbytes )
{
    int i;

    SPI->spiMsgCtl = (nbytes << SPI_BYTE_CNT_SHIFT |
                    HALF_DUPLEX_W << SPI_MSG_TYPE_SHIFT);

    for (i = 0; i < nbytes; i++)
        SPI->spiMsgData[i] = msg_buf[i];

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                    0 << SPI_CMD_DEVICE_ID_SHIFT | 
                    0 << SPI_CMD_PREPEND_BYTE_CNT_SHIFT );

    while (!(SPI->spiIntStatus & SPI_INTR_CMD_DONE));

    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

    return SPI_STATUS_OK;
}

/*********************************************************************/
/* Init_flash is used to build a sector table. This information is   */
/* translated from erase_block information to base:offset information*/
/* for each individual sector. This information is then stored       */
/* in the meminfo structure, and used throughout the driver to access*/
/* sector information.                                               */
/*                                                                   */
/* This is more efficient than deriving the sector base:offset       */
/* information every time the memory map switches (since on the      */
/* development platform can only map 64k at a time).  If the entire  */
/* flash memory array can be mapped in, then the addition static     */
/* allocation for the meminfo structure can be eliminated, but the   */
/* drivers will have to be re-written.                               */
/*                                                                   */
/* The meminfo struct occupies 44 bytes of heap space, depending     */
/* on the value of the define MAXSECTORS.  Adjust to suit            */
/* application                                                       */ 
/*********************************************************************/

int spi_flash_init(flash_device_info_t **flash_info)
{
    struct flash_name_from_id fnfi[] = SPI_FLASH_DEVICES;
    struct flash_name_from_id *fnfi_ptr;
    int i=0, count=0;
    int basecount=0L;
    unsigned short device_id;
    unsigned short blkEnables;
    int sectorsize = 0;
    int numsector = 0;

    *flash_info = &flash_spi_dev;

#if 0
    /* 
     * in case of flash corrupt, the following steps can erase the flash
     * 1. jumper USE_SPI_SLAVE to make SPI in slave mode
     * 2. start up JTAG debuger and remove the USE_SPI_SLAVE jumper 
     * 3. run the following code to erase the flash
     */
    flash_sector_erase_int(0);
    flash_sector_erase_int(1);
    printk("flash_init: erase all sectors\n");
    return FLASH_API_OK;
#endif
    blkEnables = PERF->blkEnables;
    if ((blkEnables & SPI_CLK_EN) == 0) {
        blkEnables |= SPI_CLK_EN;
        PERF->blkEnables = blkEnables;
    }

    flash_spi_dev.flash_device_id = device_id = spi_flash_get_device_id(0);

    if ((((char)(device_id >> 8)) == ATMELPART)) {
        flashFamily = FLASH_ATMEL;
        switch ((char)(device_id & 0x00ff)) {
            case ID_AT25F512:
                numsector = AT25F512_SECTOR;
                sectorsize = SECTOR_SIZE_32K;
                break;
            case ID_AT25F2048:
                numsector = AT25F2048_SECTOR;
                sectorsize = SECTOR_SIZE_64K;
                break;
            default:
                break;
        }
    }
    else if (((char)(device_id >> 8)) == (char)SSTPART) {
        flashFamily = FLASH_SST;
        sectorsize = SECTOR_SIZE_4K;
        switch ((unsigned char)(device_id & 0x00ff)) {
            case ID_SST25VF016B:
                numsector = SST25VF016B_SECTOR;
                sstAaiWordProgram = TRUE;
                break;
            case ID_SST25VF080:
                numsector = SST25VF080_SECTOR;
                break;
            case ID_SST25VF040:
                numsector = SST25VF040_SECTOR;
                break;
            case ID_SST25VF020:
            default:
                numsector = SST25VF020_SECTOR;
                break;
        }
    }
    else if (((char)(device_id >> 8)) == (char)AMD_S25FL002D) {
            flashFamily = FLASH_AMD;
            numsector = AMD25FL_SECTOR;
            sectorsize = SECTOR_SIZE_64K;
            device_id &= 0xff00;
    } 
    else {
            meminfo.addr = 0L;
            meminfo.nsect = 1;
            meminfo.sec[0].size = SECTOR_SIZE_4K;
            meminfo.sec[0].base = 0x00000;
            return FLASH_API_ERROR;
    }

    meminfo.addr = 0L;
    meminfo.nsect = numsector;
    for (i = 0; i < numsector; i++) {
        meminfo.sec[i].size = sectorsize;
        meminfo.sec[i].base = basecount;
        basecount += meminfo.sec[i].size;
        count++;
    }
    totalSize = meminfo.sec[count-1].base + meminfo.sec[count-1].size;

    for( fnfi_ptr = fnfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ ) {
        if( fnfi_ptr->fnfi_id == device_id ) {
            strcpy( flash_spi_dev.flash_device_name, fnfi_ptr->fnfi_name ); 
            break;
        }
    }

    return (FLASH_API_OK);
}

/*********************************************************************/
/* Flash_sector_erase_int() wait until the erase is completed before */
/* returning control to the calling function.  This can be used in   */
/* cases which require the program to hold until a sector is erased, */
/* without adding the wait check external to this function.          */
/*********************************************************************/

static int spi_flash_sector_erase_int(unsigned short sector)
{
    unsigned char buf[4];
    unsigned int addr;
    int rc;

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;

    /* set device to write enabled */
    spi_flash_reset();
    spi_flash_ub(sector);

    switch (flashFamily) {
        case FLASH_SST:
            buf[0] = SST_FLASH_SERASE;
            break;
        case FLASH_ATMEL:
            buf[0] = ATMEL_FLASH_SERASE;
            break;
        case FLASH_AMD:
            buf[0] = AMD_FLASH_SERASE;
            break;
    };


    /* erase the sector  */
    addr = (unsigned int) spi_get_flash_memptr(sector);
    buf[1] = (unsigned char)((addr & 0x00ff0000) >> 16);
    buf[2] = (unsigned char)((addr & 0x0000ff00) >> 8);
    buf[3] = (unsigned char)(addr & 0x000000ff);
    rc = spi_write(buf, sizeof(buf));

    /* check device is ready */
    if (rc == SPI_STATUS_OK) {
        while (spi_flash_status() != STATUS_READY) {}
    }

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_chip_erase_int() wait until the erase is completed before   */
/* returning control to the calling function.  This can be used in   */
/* cases which require the program to hold until a sector is erased, */
/* without adding the wait check external to this function.          */
/*********************************************************************/

#if 0 /* not used */
unsigned char spi_flash_chip_erase_int(void)
{
    unsigned char buf[4];
    int rc;

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;

    /* set device to write enabled */
    buf[0] = FLASH_WREN;
    rc = spi_write(buf, 1);

    /* check device is ready */
    if (rc == SPI_STATUS_OK) {
        do {
            buf[0] = FLASH_RDSR;
            rc = spi_read(buf, 1, 1);
            if (rc == SPI_STATUS_OK) {
                if (buf[0] & SR_WEN) {
                    break;
                }
            } else {
                break;
            }
        } while (1);
    }
    
    switch (flashFamily) {
        case FLASH_SST:
            buf[0] = SST_FLASH_CERASE;
            break;
        case FLASH_ATMEL:
            buf[0] = ATMEL_FLASH_CERASE;
            break;
        case FLASH_AMD:
            buf[0] = AMD_FLASH_CERASE;
            break;
    };
    /* erase the sector  */
    rc = spi_write(buf, 1);

    /* check device is ready */
    if (rc == SPI_STATUS_OK) {
        while (spi_flash_status() != STATUS_READY) {}
    }

    return(FLASH_API_OK);
}
#endif

/*********************************************************************/
/* flash_reset() will reset the flash device to reading array data.  */
/* It is good practice to call this function after autoselect        */
/* sequences had been performed.                                     */
/*********************************************************************/

static int spi_flash_reset(void)
{
    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;
    spi_flash_reset_ub();
    while (spi_flash_status() != STATUS_READY) { }
    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_read_buf() reads buffer of data from the specified          */
/* offset from the sector parameter.                                 */
/*********************************************************************/

static int spi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes)
{
    unsigned char buf[MAX_READ];
    unsigned int addr;
    int maxread;
    int idx;

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;

    spi_flash_reset();

    addr = (unsigned int) spi_get_flash_memptr(sector);
    addr += offset;
    idx = 0;
    while (numbytes) {
        maxread = (numbytes < sizeof(buf)) ? numbytes : sizeof(buf);
        buf[0] = FLASH_READ;
        buf[1] = (unsigned char)((addr & 0x00ff0000) >> 16);
        buf[2] = (unsigned char)((addr & 0x0000ff00) >> 8);
        buf[3] = (unsigned char)(addr & 0x000000ff);
        spi_read(buf, 4, maxread);
        while (spi_flash_status() != STATUS_READY) {}
        memcpy(buffer+idx, buf, maxread);
        idx += maxread;
        numbytes -= maxread;
        addr += maxread;
    }

    return (FLASH_API_OK);
}

/*********************************************************************/
/* flash_ub() places the flash into unlock bypass mode.  This        */
/* is REQUIRED to be called before any of the other unlock bypass    */
/* commands will become valid (most will be ignored without first    */
/* calling this function.                                            */
/*********************************************************************/

static int spi_flash_ub(unsigned short sector)
{
    unsigned char buf[4];
    int rc;

    do {
        buf[0] = FLASH_RDSR;
        rc = spi_read(buf, 1, 1);

        if (rc == SPI_STATUS_OK) {
            while (spi_flash_status() != STATUS_READY) {}
            if (buf[0] & (SR_BP2|SR_BP1|SR_BP0)) {
                spi_flash_write_status((unsigned char)~(SR_WPEN|SR_BP2|SR_BP1|SR_BP0));
            } else {
                break;
            }
        } else {
            break;
        }
    } while (1);

    /* set device to write enabled */
    buf[0] = FLASH_WREN;
    rc = spi_write(buf, 1);

    /* check device is ready */
    if (rc == SPI_STATUS_OK) {
        while (spi_flash_status() != STATUS_READY) {}
        do {
            buf[0] = FLASH_RDSR;
            rc = spi_read(buf, 1, 1);
            if (rc == SPI_STATUS_OK) {
                while (spi_flash_status() != STATUS_READY) {}
                if (buf[0] & SR_WEN) {
                    break;
                }
            } else {
                break;
            }
        } while (1);
    }

    return(FLASH_API_OK);
}

static int spi_flash_write_status(unsigned char status)
{
    unsigned char buf[4];
    int rc = SPI_STATUS_OK;

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;

    switch (flashFamily) {
        case FLASH_SST:
            buf[0] = FLASH_EWSR;
            rc = spi_write(buf, 1);
            break;
        default:
            break;
    }
    if (rc == SPI_STATUS_OK) {
        buf[0] = FLASH_WRST;
        buf[1] = (status & (SR_WPEN|SR_BP2|SR_BP1|SR_BP0));
        rc = spi_write(buf, 2);
        if (rc == SPI_STATUS_OK)
            while (spi_flash_status() != STATUS_READY) {}
    }

    return FLASH_API_OK;
}

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/

static int spi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes)
{
    int ret = FLASH_API_ERROR;

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;

    ret = spi_flash_write(sector, offset, buffer, numbytes, TRUE);

    if( ret == -1 )
        printk( "Flash write error.  Verify failed\n" );

    return( ret );
}

/*********************************************************************/
/* flash_reset_ub() is required to remove the flash from unlock      */
/* bypass mode.  This is important, as other flash commands will be  */
/* ignored while the flash is in unlock bypass mode.                 */
/*********************************************************************/

static int spi_flash_reset_ub(void)
{
    unsigned char buf[4];

    if (flashFamily == FLASH_UNDEFINED)
        return FLASH_API_ERROR;
    /* set device to write disabled */
    buf[0] = FLASH_WRDI;
    spi_write(buf, 1);
    while (spi_flash_status() != STATUS_READY) {}

    return(FLASH_API_OK);
}

/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/

static int spi_flash_get_numsectors(void)
{
    return meminfo.nsect;
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/

static int spi_flash_get_sector_size(unsigned short sector)
{
    return meminfo.sec[sector].size;
}

/*********************************************************************/
/* The purpose of get_flash_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer.                                  */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/

static unsigned char *spi_get_flash_memptr(unsigned short sector)
{
    unsigned char *memptr = (unsigned char*)
        (FLASH_BASE + meminfo.sec[sector].base);

    return (memptr);
}

static unsigned char *spi_flash_get_memptr(unsigned short sector)
{
    return( spi_get_flash_memptr(sector) );
}

/*********************************************************************/
/* Flash_write extends the functionality of flash_program() by       */
/* providing an faster way to program multiple data words, without   */
/* needing the function overhead of looping algorithms which         */
/* program word by word.  This function utilizes fast pointers       */
/* to quickly loop through bulk data.                                */
/*********************************************************************/
static int spi_flash_write(unsigned short sector, int offset, unsigned char *buf,
    int nbytes, int ub)
{
    unsigned char wbuf[64];
    unsigned int addr;
    unsigned int dst;
    unsigned char *pbuf;
    int cmdlen;
    int maxwrite;
    int pagelimit;

    addr = (unsigned int) spi_get_flash_memptr(sector);
    dst = addr + offset;

    pbuf = buf;
    switch (flashFamily) {
        case FLASH_SST:
            if( sstAaiWordProgram == FALSE )
            {
                /* Auto Address Increment one byte at a time. */
                spi_flash_ub(sector); /* enable write */
                wbuf[0] = FLASH_AAI;
                while (nbytes) {
                    if (pbuf != buf) {
                        wbuf[1] = *pbuf; 
                        cmdlen = CMD_LEN_1;
                    } else {
                        wbuf[1] = (unsigned char)((dst & 0x00ff0000) >> 16);
                        wbuf[2] = (unsigned char)((dst & 0x0000ff00) >> 8);
                        wbuf[3] = (unsigned char)(dst & 0x000000ff);
                        wbuf[4] = *pbuf;
                        cmdlen = CMD_LEN_4;
                    }
                    spi_write(wbuf, 1+cmdlen);
                    while (spi_flash_status() != STATUS_READY) {}
                    pbuf++; /* update address and count by one byte */
                    nbytes--;
                }
                spi_flash_reset_ub();
                while (spi_flash_status() != STATUS_READY) {}
            }
            else
            {
                /* Auto Address Increment one word (2 bytes) at a time. */
                spi_flash_ub(sector); /* enable write */
                wbuf[0] = FLASH_WORD_AAI;
                while (nbytes) {
                    if (pbuf != buf) {
                        wbuf[1] = *pbuf; 
                        wbuf[2] = *(pbuf + 1); 
                        cmdlen = 3;
                    } else {
                        wbuf[1] = (unsigned char)((dst & 0x00ff0000) >> 16);
                        wbuf[2] = (unsigned char)((dst & 0x0000ff00) >> 8);
                        wbuf[3] = (unsigned char)(dst & 0x000000ff);
                        wbuf[4] = *pbuf;
                        wbuf[5] = *(pbuf + 1); 
                        cmdlen = 6;
                    }
                    spi_write(wbuf, cmdlen);
                    while (spi_flash_status() != STATUS_READY) {}
                    pbuf += 2; /* update address and count by two bytes */
                    nbytes -= 2;
                }
                spi_flash_reset_ub();
                while (spi_flash_status() != STATUS_READY) {}
            }
            break;

        case FLASH_ATMEL:
            while (nbytes) {
                spi_flash_ub(sector); /* enable write */
                maxwrite = (nbytes < (sizeof(SPI->spiMsgData)-CMD_LEN_4))
                    ? nbytes : (sizeof(SPI->spiMsgData)-CMD_LEN_4);
                /* maxwrite is limit to page boundary */
                pagelimit = FLASH_PAGE_SIZE - (dst & 0x000000ff);
                maxwrite = (maxwrite < pagelimit) ? maxwrite : pagelimit;

                wbuf[0] = FLASH_PROG;
                wbuf[1] = (unsigned char)((dst & 0x00ff0000) >> 16);
                wbuf[2] = (unsigned char)((dst & 0x0000ff00) >> 8);
                wbuf[3] = (unsigned char)(dst & 0x000000ff);
                memcpy(&wbuf[4], pbuf, maxwrite);
                spi_write(wbuf, maxwrite+CMD_LEN_4);
                while (spi_flash_status() != STATUS_READY) {}
                pbuf += maxwrite;
                nbytes -= maxwrite;
                dst += maxwrite;
            }
            break;

        case FLASH_AMD:
            while (nbytes) {
                spi_flash_ub(sector); /* enable write */
                maxwrite = (nbytes < (sizeof(SPI->spiMsgData)-CMD_LEN_4))
                    ? nbytes : (sizeof(SPI->spiMsgData)-CMD_LEN_4);
                /* maxwrite is limit to page boundary */
                pagelimit = FLASH_PAGE_SIZE - (dst & 0x000000ff);
                maxwrite = (maxwrite < pagelimit) ? maxwrite : pagelimit;

                wbuf[0] = FLASH_PROG;
                wbuf[1] = (unsigned char)((dst & 0x00ff0000) >> 16);
                wbuf[2] = (unsigned char)((dst & 0x0000ff00) >> 8);
                wbuf[3] = (unsigned char)(dst & 0x000000ff);
                memcpy(&wbuf[4], pbuf, maxwrite);
                spi_write(wbuf, maxwrite+CMD_LEN_4);
                while (spi_flash_status() != STATUS_READY) {}
                pbuf += maxwrite;
                nbytes -= maxwrite;
                dst += maxwrite;
            }
            break;

        default:
            return 0;
    }

    return (pbuf-buf);
}

/*********************************************************************/
/* Flash_status return an appropriate status code                    */
/*********************************************************************/

static int spi_flash_status(void)
{
    unsigned char buf[4];
    int rc;
    int retry = 10;

    /* check device is ready */
    do {
        buf[0] = FLASH_RDSR;
        rc = spi_read(buf, 1, 1);
        if (rc == SPI_STATUS_OK) {
            if (!(buf[0] & SR_RDY)) {
                return STATUS_READY;
            }
        } else {
            return STATUS_ERROR;
        }
        OSL_DELAY(10);
    } while (retry--);

    return STATUS_TIMEOUT;
}

/*********************************************************************/
/* flash_get_device_id() return the device id of the component.      */
/*********************************************************************/

static unsigned short spi_flash_get_device_id(unsigned short sector)
{
    unsigned char buf[4];
    int prependcnt;
    int i;

    for (i = 0; i < MAX_RETRY; i++) {
        /* read product id command */
        buf[0] = SST_FLASH_RDID;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
        prependcnt = 4;
        spi_read(buf, prependcnt, sizeof(unsigned short));
        while (spi_flash_status() != STATUS_READY) {}

        if (buf[0] == SSTPART) {
            return( *(unsigned short *)&buf[0] );
        }
    }

    for (i = 0; i < MAX_RETRY; i++) {
        buf[0] = ATMEL_FLASH_RDID;
        prependcnt = 1;
        spi_read(buf, prependcnt, sizeof(unsigned short));
        while (spi_flash_status() != STATUS_READY) {}

        if (buf[0] == ATMELPART) {
            return( *(unsigned short *)&buf[0] );
        }
    }

    for (i = 0; i < MAX_RETRY; i++) {
        buf[0] = AMD_FLASH_RDID;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
        prependcnt = 4;
        spi_read(buf, prependcnt, sizeof(unsigned short));
        while (spi_flash_status() != STATUS_READY) {}

        if (buf[0] == AMD_S25FL002D) {
            return( *(unsigned short *)&buf[0] );
        }
    }

    /* return manufacturer code and device code */
    return( *(unsigned short *)&buf[0] );
}

/*********************************************************************/
/* The purpose of flash_get_blk() is to return a the block number */
/* for a given memory address.                                       */
/*********************************************************************/

static int spi_flash_get_blk(int addr)
{
    int blk_start, i;
    int last_blk = spi_flash_get_numsectors();
    int relative_addr = addr - (int) FLASH_BASE;

    for(blk_start=0, i=0; i < relative_addr && blk_start < last_blk; blk_start++)
        i += spi_flash_get_sector_size(blk_start);

    if( (unsigned int)i > (unsigned int)relative_addr ) {
        blk_start--;        // last blk, dec by 1
    } else {
        if( blk_start == last_blk )
        {
            printk("Address is too big.\n");
            blk_start = -1;
        }
    }

    return( blk_start );
}

/************************************************************************/
/* The purpose of flash_get_total_size() is to return the total size of */
/* the flash                                                            */
/************************************************************************/
static int spi_flash_get_total_size(void)
{
    return totalSize;
}

static int spi_flash_get_total_memory_mapped_size(void)
{
    return((totalSize < MAX_MEMORY_MAPPED_SIZE)
        ? totalSize : MAX_MEMORY_MAPPED_SIZE);
}

