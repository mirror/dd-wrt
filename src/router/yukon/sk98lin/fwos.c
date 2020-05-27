/******************************************************************************
 *
 * Name:    fwos.c
 * Project: Gigabit Ethernet Adapters, Common Modules
 * Version: $Revision: #5 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: Kernel mode file read functions.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 *****************************************************************************/

#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
"$Header: //Release/Yukon_1G/Shared/fw_interface/V1/linux/fwos.c#5 $" ;
#endif

#include "h/sktypes.h"
#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/fcntl.h>

/********************************************************
		Local Variables
********************************************************/

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

/********************************************************
		Global Functions
********************************************************/

/**
 *  @brief This function opens the FW image file and stores
 *         the data in an allocated buffer.
 *
 *  @param pAC           Pointer to adapter context
 *  @param pFilePathName Pointer to full pathname of FW image file
 *  @param FileHandle    FileHandle of the FW image file
 *  @return              FileHandle
 */
SK_FW_HANDLE FwOsOpenFile(
SK_AC *pAC,
char  *pFilePathName
)
{
	struct file	*filp = NULL;
	struct inode	*inode = NULL;
	loff_t		size;


	/* Open the file in kernel mode */
	printk("Open File %s\n", FW_FILE_PATHNAME);
	filp = filp_open(FW_FILE_PATHNAME, O_RDONLY | O_LARGEFILE, 0);

	/* Check if the file was opened successfully
	   and return the file pointer of it was.
	*/
	if (IS_ERR(filp)) {
		printk("sk98lin: Firmware image NOT found.\n");
		filp = NULL;
		return filp;
	}

	inode = filp->f_dentry->d_inode;
	size = i_size_read(inode->i_mapping->host);

	if (size <= 0) {
		printk("sk98lin: Firmware image found but not readable.\n");
		filp = NULL;
		return filp;
	}

	printk("sk98lin: Using flash image from binary file. Size: %d\n", (int) size);
	return filp;

} /* FwOsOpenFile */

/**
 *  @brief This function closes the FW image file.
 *
 *  @param pAC        Pointer to adapter context
 *  @param FileHandle FileHandle of the FW image file
 *  @return           SK_TRUE or SK_FALSE
 */
int FwOsCloseFile(
SK_AC 			*pAC,
SK_FW_HANDLE	pFileHandle)
{
	filp_close(pFileHandle, current->files);
	return SK_TRUE;
} /* FwOsCloseFile */

/**
 *  @brief This function returns the size of the FW image file.
 *
 *  @param pAC      Pointer to adapter context
 *  @return         Size of FW image file
 */
SK_U32 FwOsGetFileSize(SK_AC *pAC, SK_FW_HANDLE FileHandle)
{
	struct inode	*inode = NULL;
	loff_t		size;

	inode = FileHandle->f_dentry->d_inode;
	size = i_size_read(inode->i_mapping->host);

	return size;
} /* FwOsGetFileSize */

/**
 *  @brief This function allocates kernel memory.
 *
 *  @param pAC      Pointer to adapter context
 *  @return         Pointer to allocated memory
 */
SK_U8 *FwOsAllocMemory(SK_AC *pAC, size_t Size)
{
	SK_U8 *pMemory;

	pMemory = kmalloc(Size, GFP_KERNEL);

	return pMemory;
} /* FwOsAllocMemory */

/**
 *  @brief This function frees previously allocated memory.
 *
 *  @param pAC      Pointer to adapter context
 *  @return         SK_TRUE or SK_FALSE
 */
int FwOsFreeMemory(SK_AC *pAC, SK_U8 *pMemory)
{
	if (pMemory != NULL) {
		kfree(pMemory);
	}

	return SK_TRUE;
} /* FwOsFreeMemory */

/**
 *  @brief This function reads data from the FW image file.
 *
 *  @param pAC        Pointer to adapter context
 *  @param FileHandle Pointer to adapter context
 *  @param pData      Pointer to data buffer
 *  @param Offset     Start offset in flash eprom for operation
 *  @param Length     Length in flash eprom
 *  @return           SK_TRUE or SK_FALSE
 */
int FwOsFileRead(
SK_AC  			*pAC,
SK_FW_HANDLE 		FileHandle,
SK_U8			*pData,
SK_U32			Offset,
SK_U32			Length)
{
	int	readret;
	loff_t	origpos;
	mm_segment_t	old_fs;
	origpos = FileHandle->f_pos;
	FileHandle->f_pos += Offset;

	old_fs=get_fs();
	set_fs(get_ds());
	readret = FileHandle->f_op->read(FileHandle, pData, Length, &FileHandle->f_pos);
	set_fs(old_fs);

	FileHandle->f_pos = origpos;
	return SK_TRUE;
} /* FwOsFileRead */

/**
 *  @brief This function frees previously allocated memory.
 *
 *  @param Delay    Time to sleep
 *  @return         SK_TRUE or SK_FALSE
 */
void FwOsSleep(int Delay)
{
	/* Time to sleep in milliseconds */
	msleep((unsigned int)(Delay/1000));
} /* FwOsSleep */

