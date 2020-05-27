/******************************************************************************
 *
 * Name:    fwos.h
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
 ******************************************************************************/

#ifndef __SK_FWOS_H__
#define __SK_FWOS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SK_FW_HANDLE			struct file *

#define FW_RECOVER_DELAY_VALUE		100000	/* ms * 1000 */
#define FW_READ_DELAY_VALUE			20000	/* ms * 1000 */

extern SK_FW_HANDLE FwOsOpenFile(SK_AC *, char *);
extern int          FwOsCloseFile(SK_AC *, SK_FW_HANDLE);
extern SK_U32       FwOsGetFileSize(SK_AC *, SK_FW_HANDLE);
extern SK_U8        *FwOsAllocMemory(SK_AC *, size_t);
extern int          FwOsFreeMemory(SK_AC *, SK_U8 *);
extern int          FwOsFileRead(SK_AC *, SK_FW_HANDLE, SK_U8 *, SK_U32, SK_U32);
extern void         FwOsSleep(int);

/******************************************************************************
 *
 * Linux specific IOCTL defines and typedefs
 *
 ******************************************************************************/

/* Embedded SDK driver ioctl */
#define SK_IOCTL_GETLINKCAP  (SK_IOCTL_BASE + 14)
#define SK_AUTONEG_SUCCESS 0
#define SK_AUTONEG_FAIL    1
#define SK_AUTONEG_OFF     2
#define SK_AUTONEG_NOLINK  3

/* Firmware ioctl defines and commands */
#define SK_IOCTL_APPTOFW     (SK_IOCTL_BASE + 5)
#define SK_IOCTL_APPTODRV    (SK_IOCTL_BASE + 6)
#define SK_IOCTL_FW_SEND	1
#define SK_IOCTL_FW_RECEIVE	2
#define SK_IOCTL_COMMAND	5

typedef struct s_FWIOCTL SK_GE_FWIOCTL;
struct s_FWIOCTL {
	unsigned int	Command;
	unsigned int	Len;
	char  __user	*pData;
	SK_U32			ID;
};

/*******************************************************************************
 *
 * OS specific FW structure
 *
 ******************************************************************************/

typedef struct s_FwOs {
	char    *pFilePathName;
	SK_U32  FileSize;
	SK_U32  *pFileHandle;
	SK_U8   *pFileData;
#ifdef SK_USE_ISR_FW_ISR
	SK_U8   *pFwData;
#endif
	int     FileIndex;
} SK_FWOS;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SK_FWOS_H__ */

