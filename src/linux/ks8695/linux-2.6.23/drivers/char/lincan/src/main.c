/* main.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <linux/init.h>
#include <linux/autoconf.h>

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/interrupt.h>

#include <linux/device.h>
static struct class *can_class;
static dev_t can_devt;

#if !defined (__GENKSYMS__) 
#if (defined (MODVERSIONS) && !defined(NOVER))
#include <linux/modversions.h>
/*#include "../include/main.ver"*/
#endif
#endif

/*#undef CONFIG_DEVFS_FS*/

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#include <linux/miscdevice.h>
#endif

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/modparms.h"
#include "../include/devcommon.h"
#include "../include/setup.h"
#include "../include/proc.h"
#include "../include/open.h"
#include "../include/close.h"
#include "../include/read.h"
#include "../include/select.h"
#include "../include/irq.h"
#include "../include/ioctl.h"
#include "../include/write.h"
#include "../include/finish.h"
#include "../include/fasync.h"

#ifdef CAN_WITH_RTL
#include <rtl_posixio.h>
#include "../include/can_iortl.h"
#endif /*CAN_WITH_RTL*/

can_spinlock_t canuser_manipulation_lock;

int major=CAN_MAJOR;
int minor[MAX_TOT_CHIPS]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int extended=0;
int pelican=0;
int baudrate[MAX_TOT_CHIPS];
char *hw[MAX_HW_CARDS]={NULL,};
int irq[MAX_IRQ]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
unsigned long io[MAX_HW_CARDS]={-1,-1,-1,-1,-1,-1,-1,-1};
long clockfreq[MAX_HW_CARDS];
int stdmask=0;
int extmask=0;
int mo15mask=0;
int processlocal=0;

unsigned int minor_specified;
unsigned int baudrate_specified;
unsigned int hw_specified;
unsigned int irq_specified;
unsigned int io_specified;
unsigned int clockfreq_specified;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
/* Module parameters, some must be supplied at module loading time */
MODULE_PARM(major,"1i");
/*MODULE_PARM(minor, "1-" __MODULE_STRING(MAX_TOT_CHIPS)"i");*/
MODULE_PARM(minor, "1-" __MODULE_STRING(MAX_TOT_CHIPS_STR)"i");
MODULE_PARM(extended,"1i");
MODULE_PARM(pelican,"1i");
MODULE_PARM(baudrate, "1-" __MODULE_STRING(MAX_TOT_CHIPS_STR)"i");
MODULE_PARM(hw, "1-" __MODULE_STRING(MAX_HW_CARDS)"s");
MODULE_PARM(irq, "1-" __MODULE_STRING(MAX_IRQ)"i");
MODULE_PARM(io, "1-" __MODULE_STRING(MAX_HW_CARDS)"i");
MODULE_PARM(clockfreq, "1-" __MODULE_STRING(MAX_HW_CARDS)"i");
MODULE_PARM(stdmask, "1i");
MODULE_PARM(extmask, "1i");
MODULE_PARM(mo15mask, "1i");
MODULE_PARM(processlocal, "1i");

#else /* LINUX_VERSION_CODE >= 2,6,12 */
module_param(major, int, 0);
module_param_array(minor, int, &minor_specified, 0);
module_param(extended, int, 0);
module_param(pelican, int, 0);
module_param_array(baudrate, int, &baudrate_specified, 0);
module_param_array(hw, charp, &hw_specified, 0);
module_param_array(irq, int, &irq_specified, 0);
module_param_array(io, ulong, &io_specified, 0);
module_param_array(clockfreq, long, &clockfreq_specified, 0);
module_param(stdmask, int, 0);
module_param(extmask, int, 0);
module_param(mo15mask, int, 0);
module_param(processlocal, int, 0);
#endif /* LINUX_VERSION_CODE >= 2,6,12 */

MODULE_PARM_DESC(major,"can be used to change default major [" __MODULE_STRING(CAN_MAJOR) "]");
MODULE_PARM_DESC(minor,"can be used to change default starting minor for each channel");
MODULE_PARM_DESC(extended,"enables automatic switching to extended format if ID>2047,"
			" selects extended frames reception for i82527");
MODULE_PARM_DESC(pelican,"unused parameter, PeliCAN used by default for sja1000p chips");
MODULE_PARM_DESC(baudrate,"baudrate for each channel in step of 1kHz");
MODULE_PARM_DESC(hw,"list of boards types to initialize - virtual,pip5,...");
MODULE_PARM_DESC(irq,"list of iterrupt signal numbers, most ISA has one per chip, no value for PCI or virtual");
MODULE_PARM_DESC(io,"IO address for each board, use 0 for PCI or virtual");
MODULE_PARM_DESC(clockfreq,"base board clock source frequency in step of 1kHz");
MODULE_PARM_DESC(stdmask,"default standard mask for i82527 chips");
MODULE_PARM_DESC(extmask,"default extended mask for i82527 chips");
MODULE_PARM_DESC(mo15mask,"mask for communication object 15 of i82527 chips");
MODULE_PARM_DESC(processlocal,"select postprocessing/loopback of transmitted messages - "
		"0 .. disabled, 1 .. can be enabled by FIFO filter, 2 .. enabled by default");

#ifdef CAN_WITH_RTL
int can_rtl_priority=-1;
MODULE_PARM(can_rtl_priority, "1i");
MODULE_PARM_DESC(can_rtl_priority,"select priority of chip worker thread");
#endif /*CAN_WITH_RTL*/

/* Other module attributes */
#ifdef MODULE_SUPPORTED_DEVICE
MODULE_SUPPORTED_DEVICE("can");
#endif
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#ifdef MODULE_DESCRIPTION
MODULE_DESCRIPTION("Universal Linux CAN-bus device driver");
#endif
#ifdef MODULE_AUTHOR
MODULE_AUTHOR("Pavel Pisa <pisa@cmp.felk.cvut.cz>, Arnaud Westenberg");
#endif

/* Global structures, used to describe the installed hardware. */
struct canhardware_t canhardware;
struct canhardware_t *hardware_p=&canhardware;
struct canchip_t *chips_p[MAX_TOT_CHIPS];
struct msgobj_t *objects_p[MAX_TOT_MSGOBJS];
#ifdef CONFIG_DEVFS_FS
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,0))
devfs_handle_t  devfs_handles[MAX_TOT_MSGOBJS];
#endif
#endif

/* Pointers to dynamically allocated memory are maintained in a linked list
 * to ease memory deallocation.
 */
struct mem_addr *mem_head=NULL;

struct file_operations can_fops=
{
 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0))
	owner:		THIS_MODULE,	
 #endif
	read:		can_read,
	write:		can_write,
	poll:		can_poll,
	ioctl:		can_ioctl,
	open:		can_open,
	release:	can_close,
  #ifdef CAN_ENABLE_KERN_FASYNC
	fasync:		can_fasync
  #endif /*CAN_ENABLE_KERN_FASYNC*/
};

EXPORT_SYMBOL(can_fops);


#ifdef CAN_WITH_RTL
struct rtl_file_operations can_fops_rtl = {
	llseek:		NULL,
	read:		can_read_rtl_posix,
	write:		can_write_rtl_posix,
	ioctl:		can_ioctl_rtl_posix,
	mmap:		NULL,
	open:		can_open_rtl_posix,
	release:	can_release_rtl_posix
};
#endif /*CAN_WITH_RTL*/


/*
 2.6 kernel attributes for sysfs
 
static ssize_t show_xxx(struct class_device *cdev, char *buf)
{
        return sprintf(buf, "xxxx\n");
}

static ssize_t store_xxx(struct class_device *cdev, const char * buf, size_t count)
{
}

static CLASS_DEVICE_ATTR(xxx, S_IRUGO, show_xxx, store_xxx/NULL);

ret = class_device_create_file(class_dev, class_device_attr_xxx);
if (ret)
	goto err_unregister;

*/

int parse_kernel_parameter(void);

int parse_kernel_parameter(void)
{
	int ret = 0;
	int i=0,j=0,irq_needed=0,irq_supplied=0,io_needed=0,io_supplied=0,minor_needed=0,minor_supplied=0;
	const struct boardtype_t *brp;

	if(strlen(CONFIG_LINCAN_HW) == 0)
	{
		CANMSG("You must supply your type of hardware, interrupt numbers and io address.\n");
		return -ENODEV;
	}else if(CONFIG_LINCAN_IO == 0)
	{
		CANMSG("You must supply your type of hardware, interrupt numbers and io address.\n");
		return -ENODEV;
	}
	else
	{
		hw[0] = kmalloc(strlen(CONFIG_LINCAN_HW)+1, GFP_KERNEL);
		if(hw[0] == NULL)
			return -EINVAL;
		strcpy(hw[0], CONFIG_LINCAN_HW);
		io[0] = CONFIG_LINCAN_IO;
	}
	
	if(CONFIG_LINCAN_IRQ != 0 && strcmp(CONFIG_LINCAN_HW, "pcan_pci") != 0)
		irq[0] = CONFIG_LINCAN_IRQ;

	while ( (hw[i] != NULL) && (i < MAX_HW_CARDS) ) {
		brp = boardtype_find(hw[i]);
		if(!brp) {
			CANMSG("Sorry, hardware \"%s\" is currently not supported.\n",hw[i]);
			return -EINVAL;
		}
		irq_needed += brp->irqnum;
		i++;
	}

	/* Check wether the supplied number of io addresses is correct. */
	io_needed=i;
	while ( (io[io_supplied] != -1) & (io_supplied<MAX_HW_CARDS) ) 
		io_supplied++;
	if (io_needed != io_supplied) {
		CANMSG("Invalid number of io addresses.\n");
		CANMSG("Supplied hardware needs %d io address(es).\n",io_needed);
		return -EINVAL;
	}

	/* Check wether the supplied number of irq's is correct. */
	while ( (irq[irq_supplied] != -1) & (irq_supplied<MAX_IRQ) )
		irq_supplied++;
	while ( (hw[j] != NULL) && (j<MAX_HW_CARDS) ) {
		if (!strcmp(hw[j],"template"))
			irq_needed = irq_supplied;
		j++;
	}
	if (irq_needed != irq_supplied) {
		CANMSG("Invalid number of interrupts.\n");
		CANMSG("Supplied harware needs %d irq number(s).\n",irq_needed);
		return -EINVAL;
	}

	/* In case minor numbers were assigned check wether the correct number
	 * of minor numbers was supplied.
	 */
	if (minor[0] != -1) {
		minor_needed=irq_needed;
		while ((minor[minor_supplied] != -1) & (minor_supplied<MAX_IRQ))
			minor_supplied++; 
		if (minor_supplied != minor_needed) {
			CANMSG("Invalid number of minor numbers.\n");
			CANMSG("Supplied hardware needs %d minor number(s).\n",minor_needed);
			return -EINVAL;
		}
	}


	return ret;
}

int init_module(void)
{
	int res=0,i=0, j;
	struct candevice_t *candev;
	struct canchip_t *chip;

	if (parse_kernel_parameter())
		return -EINVAL;

	can_spin_lock_init(&canuser_manipulation_lock);
	canqueue_kern_initialize();

	if (init_hw_struct())
		return -ENODEV;

	#ifdef CAN_DEBUG
		list_hw();
	#endif

	res=register_chrdev(major,DEVICE_NAME, &can_fops);
	if (res<0) {
		CANMSG("Error registering driver.\n");
		goto register_error;
	}

	res = alloc_chrdev_region(&can_devt, 0, 4, "can");
	if (res < 0)
		printk(KERN_ERR "%s: failed to allocate char dev region\n",
			__FILE__);

	#ifdef CAN_WITH_RTL
	can_spin_lock_init(&can_irq_manipulation_lock);
	canqueue_rtl_initialize();
	res=rtl_register_rtldev(major,DEVICE_NAME,&can_fops_rtl);
	if (res<0) {
		CANMSG("Error registering RT-Linux major number.\n");
		goto rtldev_error;
	}
	#endif /*CAN_WITH_RTL*/

	for (i=0; i<hardware_p->nr_boards; i++) {
		candev=hardware_p->candevice[i];
		if (candev->hwspecops->request_io(candev)) 
			goto request_io_error;
		candev->flags|=CANDEV_IO_RESERVED;
	}

	for (i=0; i<hardware_p->nr_boards; i++) {
		candev=hardware_p->candevice[i];
		if (candev->hwspecops->reset(candev)) 
			goto reset_error;
	}

	can_spin_lock_init(&hardware_p->rtr_lock);
	hardware_p->rtr_queue=NULL;

	for (i=0; i<hardware_p->nr_boards; i++) {
		candev=hardware_p->candevice[i];
		for(j=0; j<candev->nr_all_chips; j++) {
			if((chip=candev->chip[j])==NULL)
				continue;

			if(chip->chipspecops->attach_to_chip(chip)<0) {
				CANMSG("Initial attach to the chip HW failed\n");
				goto interrupt_error;
			}

			chip->flags |= CHIP_ATTACHED;
	
			if(can_chip_setup_irq(chip)<0) {
				CANMSG("Error to setup chip IRQ\n");
				goto interrupt_error;
			}
		}

		if (candev->flags & CANDEV_PROGRAMMABLE_IRQ)
			if (candev->hwspecops->program_irq(candev)){
				CANMSG("Error to program board interrupt\n");
				goto interrupt_error;
			}
	}

#ifdef CONFIG_PROC_FS
	if (can_init_procdir())
		goto proc_error;
#endif

	can_class=class_create(THIS_MODULE, "can");

#if defined(CONFIG_DEVFS_FS) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
        {
		int dev_minor;
		struct device *this_dev;

		can_class->dev_attrs = NULL;

        	for(i=0;i<MAX_TOT_MSGOBJS;i++) {
        		if(!objects_p[i]) continue;
           		dev_minor=objects_p[i]->minor;
			this_dev=device_create(can_class, NULL, MKDEV(major, dev_minor), "can%d", dev_minor);
			if(IS_ERR(this_dev)){
				CANMSG("problem to create device \"can%d\" in the class \"can\"\n", dev_minor);
			}else{
				//this_dev->class_data=objects_p[i];
				this_dev->driver_data=objects_p[i];
				//class_set_devdata(this_dev,objects_p[i]);
			}
		      #ifdef CONFIG_DEVFS_FS
			devfs_mk_cdev(MKDEV(major, dev_minor), S_IFCHR | S_IRUGO | S_IWUGO, "can%d", dev_minor);
		      #endif
             	}
        }
#endif
	return 0;

#ifdef CONFIG_PROC_FS
	proc_error: ;
		CANMSG("Error registering /proc entry.\n");
		goto memory_error; 
#endif

	interrupt_error: ;
		goto memory_error;

	reset_error: ;
		CANMSG("Error resetting device.\n");
		goto memory_error;

	request_io_error: ;
		CANMSG("Error to request IO resources for device.\n");
		goto memory_error;

	memory_error: ;
		canhardware_done(hardware_p);

		#ifdef CAN_WITH_RTL
		rtl_unregister_rtldev(major,DEVICE_NAME);
	rtldev_error:
		canqueue_rtl_done();
		#endif /*CAN_WITH_RTL*/

		unregister_chrdev(major,DEVICE_NAME);
		CANMSG("No CAN devices or driver setup error.\n");

	register_error:
		if ( can_del_mem_list() ) 
			CANMSG("Error deallocating memory\n");

		return -ENODEV;
}

void cleanup_module(void)
{
	int i=0;

#ifdef CONFIG_PROC_FS
	if (can_delete_procdir())
		CANMSG("Error unregistering /proc/can entry.\n"); 
#endif

#if defined(CONFIG_DEVFS_FS) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
       	for(i=0;i<MAX_TOT_MSGOBJS;i++) {
		int dev_minor;
       		if(!objects_p[i]) continue;
       		dev_minor=objects_p[i]->minor;
		if(minor>=0){
		    #ifdef CONFIG_DEVFS_FS
			devfs_remove("can%d", dev_minor);
		    #endif
			//class_device_destroy(can_class, MKDEV(major, dev_minor));
		}
       	}
#endif

	class_destroy(can_class);

	canhardware_done(hardware_p);

	#ifdef CAN_WITH_RTL
	rtl_unregister_rtldev(major,DEVICE_NAME);
	canqueue_rtl_done();
	#endif /*CAN_WITH_RTL*/

	if ( can_del_mem_list() ) 
		CANMSG("Error deallocating memory\n");

	unregister_chrdev(major,DEVICE_NAME);
}

module_init(init_module);
module_exit(cleanup_module);
