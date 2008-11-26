/******************************************************************************
 *
 * Name:    skfops.c
 * Project: Gigabit Ethernet Adapters, Common Modules
 * Version: $Revision: 1.1.2.2 $
 * Date:    $Date: 2006/09/18 11:57:21 $
 * Purpose: Kernel mode file read functions.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  (C)Copyright 1998-2002 SysKonnect GmbH.
 *  (C)Copyright 2002-2003 Marvell.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This module is intended to handle all file read functions
 *
 * Include File Hierarchy:
 *
 *   "h/skdrv1st.h"
 *   "h/skdrv2nd.h"
 *
 ******************************************************************************/

/*
 Event queue and dispatcher
*/
#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
"$Header: /data/cvs/sweprojects/yukon2/lindrv/asf_linux/Attic/skfops.c,v 1.1.2.2 2006/09/18 11:57:21 mlindner Exp $" ;
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

/** 
 *  @brief This function opens/create a file in kernel mode.
 *  
 *  @param filename	Name of the file to be opened
 *  @param flags		File flags 
 *  @param mode		File permissions
 *  @return 		file pointer if successful or NULL if failed.
 */
static struct file * sk_fopen(const char * filename, 
	unsigned int flags,
	int mode)
{
	int		orgfsuid, orgfsgid;
	struct		file *	file_ret;

	/* Save uid and gid used for filesystem access.  */
	orgfsuid = current->fsuid;
	orgfsgid = current->fsgid;

	/* Set user and group to 0 (root) */
	current->fsuid = 0;
	current->fsgid = 0;
  
	/* Open the file in kernel mode */
	file_ret = filp_open(filename, flags, mode);
	
	/* Restore the uid and gid */
	current->fsuid = orgfsuid;
	current->fsgid = orgfsgid;

	/* Check if the file was opened successfully
	  and return the file pointer of it was.
	 */
	return ((IS_ERR(file_ret)) ? NULL : file_ret);
}



/** 
 *  @brief This function closes a file in kernel mode.
 *  
 *  @param file_ptr     File pointer 
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int sk_fclose(struct file * file_ptr)
{
	int	orgfsuid, orgfsgid;
	int	file_ret;

	if((NULL == file_ptr) || (IS_ERR(file_ptr)))
		return -ENOENT;
	
	/* Save uid and gid used for filesystem access.  */
	orgfsuid = current->fsuid;
	orgfsgid = current->fsgid;

	/* Set user and group to 0 (root) */
	current->fsuid = 0;
	current->fsgid = 0;
  
	/* Close the file in kernel mode (user_id = 0) */
	file_ret = filp_close(file_ptr, 0);
	
	/* Restore the uid and gid */
	current->fsuid = orgfsuid;
	current->fsgid = orgfsgid;

    return (file_ret);
}



/** 
 *  @brief This function reads data from files in kernel mode.
 *  
 *  @param file_ptr     File pointer
 *  @param buf		Buffers to read data into
 *  @param len		Length of buffer
 *  @return 		number of characters read	
 */
static int sk_fread(struct file * file_ptr, char * buf, int len)
{
	int				orgfsuid, orgfsgid;
	int				file_ret;
	mm_segment_t	orgfs;

	/* Check if the file pointer is valid */
	if((NULL == file_ptr) || (IS_ERR(file_ptr)))
		return -ENOENT;

	/* Check for a valid file read function */
	if(file_ptr->f_op->read == NULL)
		return  -ENOSYS;

	/* Check for access permissions */
	if(((file_ptr->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) == 0)
		return -EACCES;

	/* Check if there is a valid length */
	if(0 >= len)
		return -EINVAL;

	/* Save uid and gid used for filesystem access.  */
	orgfsuid = current->fsuid;
	orgfsgid = current->fsgid;

	/* Set user and group to 0 (root) */
	current->fsuid = 0;
	current->fsgid = 0;

	/* Save FS register and set FS register to kernel
	  space, needed for read and write to accept
	  buffer in kernel space.  */
	orgfs = get_fs();

	/* Set the FS register to KERNEL mode.  */
	set_fs(KERNEL_DS);

	/* Read the actual data from the file */
	file_ret = file_ptr->f_op->read(file_ptr, buf, len, &file_ptr->f_pos);

	/* Restore the FS register */
	set_fs(orgfs);

	/* Restore the uid and gid */
	current->fsuid = orgfsuid;
	current->fsgid = orgfsgid;

    return (file_ret);
}



/********************************************************
		Global Functions
********************************************************/

/** 
 *  @brief This function reads FW/Helper.
 *  
 *  @param name		File name
 *  @param addr		Pointer to buffer storing FW/Helper
 *  @param len     	Pointer to length of FW/Helper
 *  @return 		WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
SK_BOOL fw_read(	SK_AC *pAC,    /* Pointer to adapter context */
	char *name, SK_U8 **addr, SK_U32 *len )
{
	struct 	file *fp;
	SK_BOOL 	ret;
	SK_U8	*ptr;

	fp = sk_fopen(name, O_RDWR, 0 );

	if ( fp == NULL ) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("fw_read: Could not open file:%s\n", name));
		return SK_FALSE;
	}

	/*calculate file length*/
	*len = fp->f_dentry->d_inode->i_size - fp->f_pos;

	ptr = (SK_U8 *)kmalloc( *len, GFP_KERNEL );

	if ( ptr == NULL ) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("fw_read: vmalloc failure!\n"));
		return SK_FALSE;
	}
	if(sk_fread(fp, ptr,*len) > 0) {
		*addr = ptr;
		ret = SK_TRUE;
	} else {
		kfree(ptr);
		*addr = NULL;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("fw_read: failed to read file:%s\n", name));
		ret = SK_FALSE;
	}
     	
	sk_fclose( fp );
	return ret;
}


/** 
 *  @brief This function checks whether the file 'name' exists.
 *  
 *  @param name		File name
 *  @return 		SK_TRUE or SK_FALSE
 */
SK_BOOL fw_file_exists(	SK_AC *pAC,    /* Pointer to adapter context */
	char *name )
{
	struct 	file *fp;

	fp = sk_fopen(name, O_RDONLY | O_LARGEFILE, 0 );

	if ( fp == NULL ) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("fw_file_exists: Could not open file:%s\n", name));
		return SK_FALSE;
	}
	sk_fclose( fp );
	return SK_TRUE;
}

