/*
 * @par 
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/

/* Linux initialization */
#ifdef __linux

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/version.h>

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

/* Non-Linux related headers */
#include <IxFeatureCtrl.h>
#include <IxNpeMh.h>

/******************************************************
 * Include File Operations functions and declarations *
 * if Microcode is to be read in from a file          *
 ******************************************************/
#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE

/* #defines, variables and forward declarations for file operations */
#define DEV_IXNPE_MAJOR_NUMBER 241
#define DEV_IXNPE_MINOR_NUMBER 0
#define DEV_IXNPE_DEVFS_NAME "IxNpe"

typedef struct 
{
    UINT32* data;
    UINT32 size;
    struct IxNpeMicrocodeSegment* next;
} IxNpeMicrocodeSegment;

UINT32* ixNpeMicrocode_binaryArray;
PRIVATE IxNpeMicrocodeSegment* ixNpeDlSegmentedListHead = NULL;
PRIVATE IxNpeMicrocodeSegment* ixNpeDlSegmentedListTail = NULL;
PRIVATE UINT32 ixNpeDlMicrocodePageOrder, ixNpeDlTotalBytesReadIn = 0;

int ixNpe_dev_open (struct inode *inode, struct file *file);
int ixNpe_dev_release (struct inode *inode, struct file *file);
ssize_t ixNpe_dev_write (struct file *file, const char *buf, size_t byteCount, loff_t *offset);

struct file_operations ixNpe_dev_fops = {
    owner:    THIS_MODULE,
    
    open:     ixNpe_dev_open,
    release:  ixNpe_dev_release,
    
    write:    ixNpe_dev_write
};

#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */


/* Module parameters */
static int livelock_dispatcher = 0;/* default: don't use livelock dispatcher*/

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
MODULE_VERSION("2.1.1");
#endif
//module_param(livelock_dispatcher, int, 0);
//MODULE_PARM_DESC(livelock_dispatcher, "If non-zero, use the livelock prevention qmgr dispatcher");

/* Init and cleanup functions for module */
static int __init ixp400_sw_init_module(void)
{
    /* Add initialization code here */
#ifdef DEBUG
    printk("\n\n"__FUNCTION__": addr=%p\n\n", ixp400_sw_init_module);
#endif

    /*
     * If livelock prevention dispatcher is requested - enable in FeatureCtrl
     */
    if (livelock_dispatcher != 0)
    {
        ixFeatureCtrlSwConfigurationWrite (IX_FEATURECTRL_ORIGB0_DISPATCHER,
                                           IX_FEATURE_CTRL_SWCONFIG_DISABLED);
    }

#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE
    /* Register driver for /dev/ixNpe */
    if (register_chrdev(DEV_IXNPE_MAJOR_NUMBER, "ixNpe", &ixNpe_dev_fops))
	printk (KERN_ERR "Failed to register driver for /dev/ixNpe\n");
#ifdef CONFIG_DEVFS_FS
    devfs_mk_cdev(MKDEV(DEV_IXNPE_MAJOR_NUMBER, DEV_IXNPE_MAJOR_NUMBER),
    	S_IFCHR | S_IWUGO, DEV_IXNPE_DEVFS_NAME);
#endif /* CONFIG_DEVFS_FS */
#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */


    printk(KERN_DEBUG "ixp400: Module init.\n");
    return 0;
}

static void __exit ixp400_sw_cleanup_module(void)
{
    /* Add cleanup code here */
    ixNpeMhUnload();

#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE
    /* Free memory Microcode images are in and unregister driver for ixNpe */
    free_pages((UINT32)ixNpeMicrocode_binaryArray, ixNpeDlMicrocodePageOrder);
    unregister_chrdev(DEV_IXNPE_MAJOR_NUMBER, "ixNpe");
#ifdef CONFIG_DEVFS_FS
    devfs_remove(DEV_IXNPE_DEVFS_NAME);
#endif /* CONFIG_DEVFS_FS */
#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */

    printk(KERN_DEBUG "ixp400: Module uninit.\n");
}

module_init(ixp400_sw_init_module);
module_exit(ixp400_sw_cleanup_module);


/******************************************
 * Include File Operations functions if   *
 * Microcode is to be read in from a file *
 ******************************************/
#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE

/* File Operations support for /dev/ixNpe */
int ixNpe_dev_open (struct inode *inode, struct file *file)
{
    /* Test to see if we've read in Microcode already */
    if (ixNpeMicrocode_binaryArray != NULL)
    {
	printk (KERN_ERR "ixp400:  Microcode is already in memory.\n");
	return -EEXIST;
    }

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
    try_module_get(THIS_MODULE);
#else    
    MOD_INC_USE_COUNT;   /* Increment use count to prevent premature rmmod-ing */
#endif
    ixNpeDlSegmentedListHead = NULL;  /* Reset linked list which keeps track of Microcode fragments */
    ixNpeDlSegmentedListTail = NULL;
    ixNpeDlTotalBytesReadIn = 0;
    return 0;
}

int ixNpe_dev_release (struct inode *inode, struct file *file)
{
    IxNpeMicrocodeSegment* temp = NULL;
    UINT32* currentPosition = NULL;
    UINT32 pagesNeeded, x;
    
    /* If we got data, allocate pages and copy piecemeal *
     * data into permanent memory as single block        */
    if (ixNpeDlTotalBytesReadIn > 0)
    {
        /* Calculate how many pages needed */
	pagesNeeded = ixNpeDlTotalBytesReadIn/PAGE_SIZE;

	if (ixNpeDlTotalBytesReadIn%PAGE_SIZE)
	{
	    pagesNeeded++;        /* Allow for lack of floating point division */
	}
	
	/* Shift right to find what order is needed for __get_free_pages */
	for (x = pagesNeeded; x > 0; x >>= 1)
	{
	    ixNpeDlMicrocodePageOrder++;
	}

	/* __get_free_pages returns a pointer cast as an unsigned long */
	ixNpeMicrocode_binaryArray = (UINT32 *)(__get_free_pages(GFP_KERNEL, ixNpeDlMicrocodePageOrder));
	if (ixNpeMicrocode_binaryArray == NULL)
	{
	    printk (KERN_ERR "ixp400:  Error, couldn't get enough memory to store Microcode in (need %d pages).\n", pagesNeeded);
	}
	else
	{
	    /* Copy each segment of data into a single image */
	    temp = ixNpeDlSegmentedListHead;
	    currentPosition = ixNpeMicrocode_binaryArray;
	    
	    while (temp != NULL)
	    {
		ixOsalMemCopy(currentPosition, temp->data, temp->size);
		
		/* increment pointers */
		currentPosition = (UINT32 *)((UINT32)currentPosition + temp->size);
		temp = (IxNpeMicrocodeSegment *) temp->next;       /* cast is for fussy compiler */
	    }
	}
    }
    
    /* Free the temporary list memory and decrement use count */
    while (ixNpeDlSegmentedListHead->next != NULL)
    {
	/* Move the head pointer down the list and free old element */
	temp = ixNpeDlSegmentedListHead;
	ixNpeDlSegmentedListHead = (IxNpeMicrocodeSegment *) ixNpeDlSegmentedListHead->next;
	
	kfree (temp->data);
	kfree (temp);
    }
#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif

    return 0;
}

ssize_t ixNpe_dev_write (struct file *file, const char *buf, size_t byteCount, loff_t *offset)
{
    /* Malloc memory to hold this segment of the Microcode */
    UINT32* currentData = kmalloc (byteCount, GFP_KERNEL);
    IxNpeMicrocodeSegment* currentSegment = kmalloc (sizeof(IxNpeMicrocodeSegment), GFP_KERNEL);

    /* Copy data from user space */
    if (copy_from_user(currentData, buf, byteCount))
    {
	/* Copy Failed:  Free memory just kmalloced */
	kfree (currentData);
	kfree (currentSegment);
	
	/* Reset total byte count to prevent page allocation in ixNpe_dev_release() */
	ixNpeDlTotalBytesReadIn = 0;
	return -EFAULT;
    }

    /* Copy was successful so update struct */
    currentSegment->data = currentData;
    currentSegment->size = byteCount;
    currentSegment->next = NULL;	
    ixNpeDlTotalBytesReadIn += byteCount;

    /* Add to linked list of microcode segments */
    if (ixNpeDlSegmentedListHead == NULL)
    {
	ixNpeDlSegmentedListHead = currentSegment;
	ixNpeDlSegmentedListTail = currentSegment;
    }
    else
    {
	ixNpeDlSegmentedListTail->next = (struct IxNpeMicrocodeSegment *)currentSegment;  /* cast is for fussy compiler */
	ixNpeDlSegmentedListTail = currentSegment;
    }

    /* Return the number of bytes we wrote */
    return byteCount;
}

#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */

#else

#error Non-Linux compilation

#endif /* #ifdef __linux */
