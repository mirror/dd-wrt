/*
<:copyright-gpl
 Copyright 2002 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/
/*
 ***************************************************************************
 * File Name  : bcm63xx_flash.c
 *
 * Description: This file contains the flash device driver APIs for bcm63xx board. 
 *
 * Created on :  8/10/2002  seanl:  use cfiflash.c, cfliflash.h (AMD specific)
 *
 ***************************************************************************/


/* Includes. */
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <asm/uaccess.h>

#include <bcm_map_part.h>
#include <board.h>
#define  BCMTAG_EXE_USE
#include <bcmTag.h>
#include "cfiflash.h"
#include "boardparms.h"

//#define DEBUG_FLASH

static FLASH_ADDR_INFO fInfo;
static int flashInitialized = 0;

void *retriedKmalloc(size_t size)
{
	void *pBuf;
    int tryCount = 0;

    // try 1000 times before quit
    while (((pBuf = kmalloc(size, GFP_KERNEL)) == NULL) && (tryCount++ < 1000))
    {
		current->state   = TASK_INTERRUPTIBLE;
		schedule_timeout(HZ/10);
	}
    if (tryCount >= 1000)
        pBuf = NULL;
    else
	    memset(pBuf, 0, size);

    return pBuf;
}

void retriedKfree(void *pBuf)
{
	kfree(pBuf);
}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc) 
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

// get the nvram start addr
//
unsigned long get_nvram_start_addr(void)
{
    return ((unsigned long) 
        (flash_get_memptr(fInfo.flash_nvram_start_blk) + fInfo.flash_nvram_blk_offset));
}

// get the scratch_pad start addr
//
unsigned long get_scratch_pad_start_addr(void)
{
    return ((unsigned long) 
        (flash_get_memptr(fInfo.flash_scratch_pad_start_blk) + fInfo.flash_scratch_pad_blk_offset));
}



/*  *********************************************************************
    *  kerSysImageTagGet()
    *   Get the image tag
    *  Input parameters:
    *      none
    *  Return value:
    *      point to tag -- Found
    *      NULL -- failed
    ********************************************************************* */
PFILE_TAG kerSysImageTagGet(void)
{
    int i;
    int totalBlks = flash_get_numsectors();
    UINT32 crc;
    unsigned char *sectAddr;
    PFILE_TAG pTag;

#if defined(DEBUG_FLASH)
    printk("totalblks in tagGet=%d\n", totalBlks);
#endif

    // start from 2nd blk, assume 1st one is always CFE
    for (i = 1; i < totalBlks; i++)
    {
        sectAddr =  flash_get_memptr((byte) i);
        crc = CRC32_INIT_VALUE;
        crc = getCrc32(sectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
        pTag = (PFILE_TAG) sectAddr;

#if defined(DEBUG_FLASH)
        printk("Check Tag crc on blk [%d]\n", i);
#endif

        if (crc == (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
            return pTag;
    }

    return (PFILE_TAG) NULL;
}

// Initialize the flash and fill out the fInfo structure
void kerSysFlashInit( void )
{
    int i = 0;
    int totalBlks = 0;
    int totalSize = 0;
    int startAddr = 0;
    int usedBlkSize = 0;
    NVRAM_DATA nvramData;
    UINT32 crc = CRC32_INIT_VALUE, savedCrc;
    PFILE_TAG pTag = NULL;
    unsigned long kernelEndAddr = 0;
    unsigned long spAddr = 0;

    if (flashInitialized)
        return;

    flashInitialized = 1;
    flash_init();

    totalBlks = flash_get_numsectors();
    totalSize = flash_get_total_size();

    printk("Total Flash size: %dK with %d sectors\n", totalSize/1024, totalBlks);

    /* nvram is always at the end of flash */
    fInfo.flash_nvram_length = FLASH45_LENGTH_NVRAM;
    fInfo.flash_nvram_start_blk = 0;  /* always the first block */
    fInfo.flash_nvram_number_blk = 1; /*always fits in the first block */
    fInfo.flash_nvram_blk_offset = NVRAM_DATA_OFFSET;
 
    // check nvram CRC
    memcpy((char *)&nvramData, (char *)get_nvram_start_addr(), sizeof(NVRAM_DATA));
    savedCrc = nvramData.ulCheckSum;
    nvramData.ulCheckSum = 0;
    crc = getCrc32((char *)&nvramData, (UINT32) sizeof(NVRAM_DATA), crc);   

    BpSetBoardId( nvramData.szBoardId );

    fInfo.flash_persistent_length = NVRAM_PSI_DEFAULT;
    if (savedCrc != crc)
    {
        printk("***Board is not initialized****: Using the default PSI size: %d\n",
            fInfo.flash_persistent_length);
    }
    else
    {
        unsigned long ulPsiSize;
        if( BpGetPsiSize( &ulPsiSize ) == BP_SUCCESS )
            fInfo.flash_persistent_length = ulPsiSize;
        else
        {
            printk("***Board id is not set****: Using the default PSI size: %d\n",
                fInfo.flash_persistent_length);
        }
    }

    fInfo.flash_persistent_length *= ONEK;
    startAddr = totalSize - fInfo.flash_persistent_length;
    fInfo.flash_persistent_start_blk = flash_get_blk(startAddr+FLASH_BASE_ADDR_REG);
    fInfo.flash_persistent_number_blk = totalBlks - fInfo.flash_persistent_start_blk;
    // save abs SP address (Scratch Pad). it is before PSI 
    spAddr = startAddr - SP_MAX_LEN ;
    // find out the offset in the start_blk
    usedBlkSize = 0;
    for (i = fInfo.flash_persistent_start_blk; 
        i < (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk); i++)
    {
        usedBlkSize += flash_get_sector_size((byte) i);
    }
    fInfo.flash_persistent_blk_offset =  usedBlkSize - fInfo.flash_persistent_length;

    // get the info for sp
    if (!(pTag = kerSysImageTagGet()))
    {
        printk("Failed to read image tag from flash\n");
        return;
    }
    kernelEndAddr = (unsigned long) simple_strtoul(pTag->kernelAddress, NULL, 10) + \
        (unsigned long) simple_strtoul(pTag->kernelLen, NULL, 10);

    // make suer sp does not share kernel block
    fInfo.flash_scratch_pad_start_blk = flash_get_blk(spAddr+FLASH_BASE_ADDR_REG);
    if (fInfo.flash_scratch_pad_start_blk != flash_get_blk(kernelEndAddr))
    {
        fInfo.flash_scratch_pad_length = SP_MAX_LEN;
        if (fInfo.flash_persistent_start_blk == fInfo.flash_scratch_pad_start_blk)  // share blk
        {
#if 1 /* do not used scratch pad unless it's in its own sector */
            printk("Scratch pad is not used for this flash part.\n");  
            fInfo.flash_scratch_pad_length = 0;     // no sp
#else /* allow scratch pad to share a sector with another section such as PSI */
            fInfo.flash_scratch_pad_number_blk = 1;
            fInfo.flash_scratch_pad_blk_offset = fInfo.flash_persistent_blk_offset - fInfo.flash_scratch_pad_length;
#endif
        }
        else // on different blk
        {
            fInfo.flash_scratch_pad_number_blk = fInfo.flash_persistent_start_blk\
                - fInfo.flash_scratch_pad_start_blk;
            // find out the offset in the start_blk
            usedBlkSize = 0;
            for (i = fInfo.flash_scratch_pad_start_blk; 
                i < (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk); i++)
                usedBlkSize += flash_get_sector_size((byte) i);
                fInfo.flash_scratch_pad_blk_offset =  usedBlkSize - fInfo.flash_scratch_pad_length;
        }
    }
    else
    {
        printk("No flash for scratch pad!\n");  
        fInfo.flash_scratch_pad_length = 0;     // no sp
    }

#if defined(DEBUG_FLASH)
    printk("fInfo.flash_scratch_pad_start_blk = %d\n", fInfo.flash_scratch_pad_start_blk);
    printk("fInfo.flash_scratch_pad_number_blk = %d\n", fInfo.flash_scratch_pad_number_blk);
    printk("fInfo.flash_scratch_pad_length = 0x%x\n", fInfo.flash_scratch_pad_length);
    printk("fInfo.flash_scratch_pad_blk_offset = 0x%x\n", (unsigned int)fInfo.flash_scratch_pad_blk_offset);

    printk("fInfo.flash_nvram_start_blk = %d\n", fInfo.flash_nvram_start_blk);
    printk("fInfo.flash_nvram_blk_offset = 0x%x\n", (unsigned int)fInfo.flash_nvram_blk_offset);
    printk("fInfo.flash_nvram_number_blk = %d\n", fInfo.flash_nvram_number_blk);

    printk("psi startAddr = %x\n", startAddr+FLASH_BASE_ADDR_REG);
    printk("fInfo.flash_persistent_start_blk = %d\n", fInfo.flash_persistent_start_blk);
    printk("fInfo.flash_persistent_blk_offset = 0x%x\n", (unsigned int)fInfo.flash_persistent_blk_offset);
    printk("fInfo.flash_persistent_number_blk = %d\n", fInfo.flash_persistent_number_blk);
#endif

}



/***********************************************************************
 * Function Name: kerSysFlashAddrInfoGet
 * Description  : Fills in a structure with information about the NVRAM
 *                and persistent storage sections of flash memory.  
 *                Fro physmap.c to mount the fs vol.
 * Returns      : None.
 ***********************************************************************/
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info)
{
    pflash_addr_info->flash_nvram_blk_offset = fInfo.flash_nvram_blk_offset;
    pflash_addr_info->flash_nvram_length = fInfo.flash_nvram_length;
    pflash_addr_info->flash_nvram_number_blk = fInfo.flash_nvram_number_blk;
    pflash_addr_info->flash_nvram_start_blk = fInfo.flash_nvram_start_blk;
    pflash_addr_info->flash_persistent_blk_offset = fInfo.flash_persistent_blk_offset;
    pflash_addr_info->flash_persistent_length = fInfo.flash_persistent_length;
    pflash_addr_info->flash_persistent_number_blk = fInfo.flash_persistent_number_blk;
    pflash_addr_info->flash_persistent_start_blk = fInfo.flash_persistent_start_blk;
}


// get shared blks into *** pTempBuf *** which has to be released bye the caller!
// return: if pTempBuf != NULL, poits to the data with the dataSize of the buffer
// !NULL -- ok
// NULL  -- fail
static char *getSharedBlks(int start_blk, int end_blk)
{
    int i = 0;
    int usedBlkSize = 0;
    int sect_size = 0;
    char *pTempBuf = NULL;
    char *pBuf = NULL;

    for (i = start_blk; i < end_blk; i++)
        usedBlkSize += flash_get_sector_size((byte) i);

#if defined(DEBUG_FLASH)
    printk("usedBlkSize = %d\n", usedBlkSize);
#endif

    if ((pTempBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
        printk("failed to allocate memory with size: %d\n", usedBlkSize);
        return pTempBuf;
    }
    
    pBuf = pTempBuf;
    for (i = start_blk; i < end_blk; i++)
    {
        sect_size = flash_get_sector_size((byte) i);

#if defined(DEBUG_FLASH)
        printk("i = %d, sect_size = %d, end_blk = %d\n", i, sect_size, end_blk);
#endif
        flash_read_buf((byte)i, 0, pBuf, sect_size);
        pBuf += sect_size;
    }
    
    return pTempBuf;
}



// Set the pTempBuf to flash from start_blk to end_blk
// return:
// 0 -- ok
// -1 -- fail
static int setSharedBlks(int start_blk, int end_blk, char *pTempBuf)
{
    int i = 0;
    int sect_size = 0;
    int sts = 0;
    char *pBuf = pTempBuf;

    for (i = start_blk; i < end_blk; i++)
    {
        sect_size = flash_get_sector_size((byte) i);
        flash_sector_erase_int(i);
        if (flash_write_buf(i, 0, pBuf, sect_size) != sect_size)
        {
            printk("Error writing flash sector %d.", i);
            sts = -1;
            break;
        }
        pBuf += sect_size;
    }

    return sts;
}



/*******************************************************************************
 * NVRAM functions
 *******************************************************************************/

// get nvram data
// return:
//  0 - ok
//  -1 - fail
int kerSysNvRamGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (!flashInitialized)
        kerSysFlashInit();

    if (strLen > FLASH45_LENGTH_NVRAM)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_nvram_start_blk,
        (fInfo.flash_nvram_start_blk + fInfo.flash_nvram_number_blk))) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_nvram_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


// set nvram 
// return:
//  0 - ok
//  -1 - fail
int kerSysNvRamSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > FLASH45_LENGTH_NVRAM)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_nvram_start_blk,
        (fInfo.flash_nvram_start_blk + fInfo.flash_nvram_number_blk))) == NULL)
        return -1;

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_nvram_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_nvram_start_blk, 
        (fInfo.flash_nvram_number_blk + fInfo.flash_nvram_start_blk), pBuf) != 0)
        sts = -1;
    
    retriedKfree(pBuf);

    return sts;
}


/***********************************************************************
 * Function Name: kerSysEraseNvRam
 * Description  : Erase the NVRAM storage section of flash memory.
 * Returns      : 1 -- ok, 0 -- fail
 ***********************************************************************/
int kerSysEraseNvRam(void)
{
    int sts = 1;
    char *tempStorage = retriedKmalloc(FLASH45_LENGTH_NVRAM);
    
    // just write the whole buf with '0xff' to the flash
    if (!tempStorage)
        sts = 0;
    else
    {
        memset(tempStorage, 0xff, FLASH45_LENGTH_NVRAM);
        if (kerSysNvRamSet(tempStorage, FLASH45_LENGTH_NVRAM, 0) != 0)
            sts = 0;
        retriedKfree(tempStorage);
    }

    return sts;
}


/*******************************************************************************
 * PSI functions
 *******************************************************************************/
// get psi data
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_persistent_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk))) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_persistent_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


// set psi 
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_persistent_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk))) == NULL)
        return -1;

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_persistent_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_persistent_start_blk, 
        (fInfo.flash_persistent_number_blk + fInfo.flash_persistent_start_blk), pBuf) != 0)
        sts = -1;
    
    retriedKfree(pBuf);

    return sts;
}


// flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
int kerSysBcmImageSet( int flash_start_addr, char *string, int size)
{
    int sts;
    int sect_size;
    int blk_start;
    int i;
    char *pTempBuf = NULL;
    int whole_image = 0;

    blk_start = flash_get_blk(flash_start_addr);
    if( blk_start < 0 )
        return( -1 );

    if (flash_start_addr == FLASH_BASE && size > FLASH45_LENGTH_BOOT_ROM)
        whole_image = 1;

   /* write image to flash memory */
    do 
    {
        sect_size = flash_get_sector_size(blk_start);
// NOTE: for memory problem in multiple PVC configuration, temporary get rid of kmalloc this 64K for now.
//        if ((pTempBuf = (char *)retriedKmalloc(sect_size)) == NULL)
//        {
//            printk("Failed to allocate memory with size: %d.  Reset the router...\n", sect_size);
//            kerSysMipsSoftReset();     // reset the board right away.
//        }
        // for whole image, no check on psi
        if (!whole_image && blk_start == fInfo.flash_persistent_start_blk)  // share the blk with psi
        {
            if (size > (sect_size - fInfo.flash_persistent_length))
            {
                printk("Image is too big\n");
                break;          // image is too big. Can not overwrite to nvram
            }
            if ((pTempBuf = (char *)retriedKmalloc(sect_size)) == NULL)
            {
               printk("Failed to allocate memory with size: %d.  Reset the router...\n", sect_size);
               kerSysMipsSoftReset();     // reset the board right away.
            }
            flash_read_buf((byte)blk_start, 0, pTempBuf, sect_size);
            if (copy_from_user((void *)pTempBuf,(void *)string, size) != 0)
                break;  // failed ?
            flash_sector_erase_int(blk_start);     // erase blk before flash
            if (flash_write_buf(blk_start, 0, pTempBuf, sect_size) == sect_size) 
                size = 0;   // break out and say all is ok
            retriedKfree(pTempBuf);
            break;
        }
        
        flash_sector_erase_int(blk_start);     // erase blk before flash

        if (sect_size > size) 
        {
            if (size & 1) 
                size++;
            sect_size = size;
        }
        
        if ((i = flash_write_buf(blk_start, 0, string, sect_size)) != sect_size) {
            break;
        }
        blk_start++;
        string += sect_size;
        size -= sect_size; 
    } while (size > 0);

    if (whole_image)  
    {
        // If flashing a whole image, erase to end of flash.
        int total_blks = flash_get_numsectors();
        while( blk_start < total_blks )
        {
            flash_sector_erase_int(blk_start);
            blk_start++;
        }
    }
    if (pTempBuf)
        retriedKfree(pTempBuf);

    if( size == 0 ) 
        sts = 0;  // ok
    else  
        sts = blk_start;    // failed to flash this sector

    return sts;
}

/*******************************************************************************
 * SP functions
 *******************************************************************************/
// get sp data.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_HEADER pHead = NULL;
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    char *endPtr = NULL;
    char *spEndPtr = NULL;
    int sts = -1;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if (bufLen >= (fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) - sizeof(SP_TOKEN))) 
    {
        printk("Exceed scratch pad space by %d\n", bufLen  - fInfo.flash_scratch_pad_length \
            - sizeof(SP_HEADER) - sizeof(SP_TOKEN));
        return sts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk))) == NULL)
        return sts;

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    pHead = (PSP_HEADER) pBuf;
    if (memcmp(pHead->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scrap pad is not initialized.\n");
        return sts;
    }

    // search up to SPUsedLen for the token
    startPtr = pBuf + sizeof(SP_HEADER);
    endPtr = pBuf + pHead->SPUsedLen;
    spEndPtr = pBuf + SP_MAX_LEN;
    while (startPtr < endPtr && startPtr < spEndPtr)
    {
        pToken = (PSP_TOKEN) startPtr;
        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), bufLen);
            sts = 0;
            break;
        }
        // get next token
        startPtr += sizeof(SP_TOKEN) + pToken->tokenLen;
    }

    retriedKfree(pShareBuf);

    return sts;
}


// set sp.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    PSP_HEADER pHead = NULL;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    int sts = -1;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if (bufLen >= (fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) - sizeof(SP_TOKEN))) 
    {
        printk("Exceed scratch pad space by %d\n", bufLen  - fInfo.flash_scratch_pad_length \
            - sizeof(SP_HEADER) - sizeof(SP_TOKEN));
        return sts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk))) == NULL)
        return sts;

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
    pHead = (PSP_HEADER) pBuf;

    // form header info.  SPUsedLen later on...
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, TOKEN_NAME_LEN - 1);
    SPToken.tokenLen = bufLen;
    if (memcmp(pHead->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        // new sp, so just flash the token
        printk("No Scrap pad found.  Initialize scratch pad...\n");
        SPHead.SPUsedLen = sizeof(SP_HEADER) + sizeof(SP_TOKEN) + bufLen;
        memcpy(pBuf, (char *)&SPHead, sizeof(SP_HEADER));
        curPtr = pBuf + sizeof(SP_HEADER);
        memcpy(curPtr, (char *)&SPToken, sizeof(SP_TOKEN));
        curPtr += sizeof(SP_TOKEN);
        memcpy(curPtr, tokBuf, bufLen);
    }
    else  
    {
        // need search for the token, if exist with same size overwrite it. if sizes differ, 
        // move over the later token data over and put the new one at the end
        char *endPtr = pBuf + pHead->SPUsedLen;
        char *spEndPtr = pBuf + SP_MAX_LEN;
        curPtr = pBuf + sizeof(SP_HEADER);
        while (curPtr < endPtr && curPtr < spEndPtr)
        {
            pToken = (PSP_TOKEN) curPtr;
            if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
            {
                if (pToken->tokenLen == bufLen) // overwirte it
                {
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);
                    break;
                }
                else // move later data over and put the new token at the end
                {
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);  // ~~~
                    break;
                }
            }
            else // not same token ~~~
            {
            }
            // get next token
            curPtr += sizeof(SP_TOKEN) + pToken->tokenLen;
        } // end while
        SPHead.SPUsedLen = sizeof(SP_HEADER) + sizeof(SP_TOKEN) + bufLen; // ~~~
        if (SPHead.SPUsedLen > SP_MAX_LEN)
        {
            printk("No more Scratch pad space left! Over limit by %d bytes\n", SPHead.SPUsedLen - SP_MAX_LEN);
            return sts;
        }

    } // else if not new sp

    sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
        (fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk), pShareBuf);
    
    retriedKfree(pShareBuf);

    return sts;

    
}

int kerSysFlashSizeGet(void)
{
   return flash_get_total_size();
}

