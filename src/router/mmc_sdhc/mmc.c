/*==============================================================================
 * mmc.c - Linksys WRT54G/WRT54GS/WRT54GL hardware mod - MMHC/SDHC card driver
 * 
 * Version: 2.0.1
 *
 * Authors:
 *
 *   Madsuk/Rohde/Cyril CATTIAUX/Marc DENTY/rcichielo KRUSCH/Chris
 *
 * Description:
 *
 *   Rework of the 1.3.5 optimized driver posted on the opewnwrt forum.
 *   See the Release notes for a description of changes and new features.
 *
 * Usage:
 * 
 *   The /etc/init.d/mmc script is used to start and stop mmc services.
 *   This script sets the appropriate gpiomask for /proc/diag/gpiomask,
 *   loads the kernel module and mounts the first partition under /mmc. 
 *   Modify the script if you want different behaviour:
 *
 *      /etc/init.d/mmc start  - Start mmc services - mount card if detected
 *      /etc/init.d/mmc stop   - Unmount card and stop mmc services
 *      /etc/init.d/mmc status - Print status of mmc service and card details
 *
 * Module Parameters:
 *
 *   major  - Major number to be assigned to the mmc device (default 0 - assign dynamically).
 *
 *   cs     - GPIO line connect to the card CS (chip select) pin (default 7).
 *
 *   clk    - GPIO line connected to the card CLK (clock) pin (default 3).
 *
 *   din    - GPIO line connected to the card DI (data in) pin (default 2).
 *
 *   dout   - GPIO line connected to the card DO (data out) pin (default 4).
 *
 *   rahead - Maximum number of blocks kernel can read ahead (default 2).
 *
 *   maxsec - Maximum number of sectors that can be clustered into one request (default 32).
 *            Increasing this number can improve read and/or write throughput for large files.
 *            Keep it smaller if you expect frequent concurrent IO to the card (reading/writing
 *            of multiple files at the same time). Experiment with the setting to see what
 *            works best for your system.
 *          
 *   dbg    - Only valid if you load the debug version of the kernel module (default 0).
 *            Bit flags that specify what debugging to product to the system log: 
 *
 *            1  - Card initialization trace
 *            2  - Calls to module "open" function
 *            3  - Calls to module "release" function
 *            4  - Calls to module "check_change" function
 *            5  - Calls to module "revalidate" function
 *            6  - Calls to module "request" function 
 *            7  - Calls to module "ioctl" function
 *            8  - Print "busy wait" information
 *
 *  gpio_input    - Set the gpio register memory locations. 
 *  gpio_output     Allows defaults to be overridden when testing driver 
 *  gpio_enable     on other broadcom based devices.
 *  gpio_control
 *
 *
 * Release Notes:
 *
 *   Version 2.0.0 - Mar 9, 2008
 *
 *     - Rework of code base:
 *
 *         1) Rework of functions that must honour max clock frequency. These functions
 *            were generalized and condensed. Max clock frequency now managed through 2 
 *            global vars - no need to pass timing arguments.
 *
 *         2) Logging functions replaced/simplified by variadic macros.
 *
 *         3) Document and comment. Standardize layout, variables, style, etc. Split  
 *            card initialization function into separate source file.
 * 
 *     - Switch so module uses a dynamically assigned major number by default. Implement "major="
 *       module parameter to allow a specific major number to be assigned.
 * 
 *     - Implement module parameters "cs=", "clk=", "din=", "dout=" for specifying GPIO to card mapping.
 *       Alter read/write algorithms to be more efficient with mappings in variables.
 * 
 *     - Implement module parameters "gpio_input=", "gpio_output=", "gpio_enable=", "gpio_control=" for 
 *       specifying GPIO register addresses. May be useful if you want to try using this module on other
 *       broadcom based platforms where the gpio registers are located at different locations. 
 *
 *     - Debugging improvements. Implement "dbg=" module parameter to allow selective enabling of
 *       debugging output by function. Only available when module compiled with debugging (-DDEBUG)
 *
 *     - Initialize max_segments array so requests are clustered. "maxsec=" module parameter
 *       sets the maximum number of sectors that can be clustered per request (default is 32).
 *
 *     - Implement clustering support in the module request method. Improves speed by allowing more
 *       clusters to be read/written per single invocation of a multi block read/write command.
 *
 *     - Implement Support for high density (> 2GB) SDHC and MMHC cards.
 *
 *     - Implement /proc/mmc/status for obtaining information about the detected card.
 *
 *     - Maximum number of supported partitions reduced from 64 to 8 (memory use reduction).
 *
 *     - Build using buildroot-ng environment. Generate ipkg file for installation.
 *       With so little difference in speed, and only a 4k memory savings, compile debug enabled version
 *
 *   Version 2.0.1 - Feb 8, 2009
 *
 *     - Fix integer overflow when calculating size of SDHC or MMHC cards greater than 4GB. Testers have
 *       confirmed that 8GB and 16GB cards now functioning correctly.
 *
 * Supported Linksys devices:
 *
 *   Developed and tested on WRT54GL V1.1. Should work on the majority of WRT54G/GS/GL devices.
 *   Reported working on:
 *
 *     - WRT54GL V1.1
 *   
 * Supported mc/sd cards.
 *
 *   Tested using the folowing cards:
 *
 *     - Kingston MMC Plus - 1GB
 *     - PNY Technologies SD - 1GB
 *     - SanDisk Mini SD - 128MB
 *     - Lexar SDHC - 4GB
 *
 *   Module users have reported success with the following cards:
 *
 *     - Kingston SD - 2GB
 *     - Kingston microSD - 2GB
 *     - Toshiba SDHC class 6 - 4GB
 *     - Sandisk SDHC class 4 - 4GB
 *     - Lexar SDHC - 16GB
 *
 *   No MMHC card tested - suspect possible problems at initialization.
 *   If you test one, please provide feedback on the results.
 *
 *============================================================================*/
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>

// Block driver header setup - use variable for MAJOR_NR to allow dynamic assignment
// These must be defined before blk.h is included.
static unsigned int major = 0;				// Device major number (mod parm - dynamic if 0)
#define MAJOR_NR major					// Major num - map to major var to allow dynamic assignment
#define PSHIFT 3	                             	// Max 8 partitions
#define NUMDEV 1                         		// We only support a single device
#define DEVICE_NR(device) (MINOR(device) >> PSHIFT)	// Same device num for all partitions
#define DEVICE_NAME "mmc"				// Device name (messaging and in /dev).
#define DEVICE_NO_RANDOM				// No entropy to contribute

#include <linux/blk.h>


// Assign default values for modules parameters - can override by passing a module parameter.
#define READ_AHEAD 2					// Block read ahead max
#define DI  2						// Card DI to GPIO pin mapping
#define DO  4						// Card DO to GPIO pin mapping
#define CLK 3						// Card CLK to GPIP pin mapping
#define CS  7						// Card CS to GPIO pin mapping
#define MSEC 32						// Maximum sectors per request

#define VERSION "2.0.1"					// Module Version Number
#define DEBUG						// Compile in debugging. Module is very slightly
							// faster, and 4k smaller with no debugging

#ifdef DEBUG						// Defined via compiler argument
// Debug flag - bit symbols
#define DBG_INIT	(1 << 0)			// 1
#define DBG_OPEN	(1 << 1)			// 2 
#define DBG_RLSE	(1 << 2)			// 4
#define DBG_CHG		(1 << 3)			// 8
#define DBG_REVAL	(1 << 4)			// 16
#define DBG_REQ  	(1 << 5)			// 32
#define DBG_IOCTL	(1 << 6)			// 64
#define DBG_BUSY	(1 << 7)			// 128
#endif

// R1 response - bit symbols
#define R1_IDLE_STATE	(1 << 0)
#define R1_ERASE_RESET	(1 << 1)
#define R1_ILL_CMD	(1 << 2)
#define R1_CRC_ERR	(1 << 3)
#define R1_ERASE_ERR	(1 << 4)
#define R1_ADDR_ERR	(1 << 5)
#define R1_PARM_ERR	(1 << 6)


// Macros for logging via printk.
#define LOG_INFO(...)		printk(KERN_INFO "[INF] mmc: " __VA_ARGS__)
#define LOG_NOTICE(...)		printk(KERN_NOTICE "[NOT] mmc: " __VA_ARGS__)
#define LOG_WARN(...)		printk(KERN_WARNING "[WRN] mmc: " __VA_ARGS__)
#define LOG_ERR(...)		printk(KERN_ERR "[ERR] mmc: " __VA_ARGS__)
#ifdef DEBUG
#define LOG_DEBUG(flag, ...)	if (dbg & flag)	printk(KERN_INFO "[DBG] mmc: " __VA_ARGS__)
#else
#define LOG_DEBUG(flag, ...)
#endif


// Function prototypes
int mmc_open(struct inode *inode, struct file *filp);
int mmc_release(struct inode *inode, struct file *filp);
int mmc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int mmc_check_change(kdev_t);
int mmc_revalidate(kdev_t);
static void mmc_request(request_queue_t *);
static ssize_t mmc_proc_read(struct file *, char *, size_t, loff_t *);
int mmc_init(void);
void mmc_exit(void);


// Module global variable declarations and setup.
// Items marked with '(parm)' can be overridden via a module parameter.
//static unsigned int major;				// Defined earlier - device major number (parm)
static int rahead = READ_AHEAD;				// Number of device blocks to read ahead (parm)
static int din = DI;					// GPIO attached to card DI pin (parm)
static int dout = DO;					// GPIO attached to card DO pin (parm)
static int clk = CLK;					// GPIO attached to card CLK pin (parm)
static int cs = CS;					// GPIO attached to card CS pin (parm)
static int maxsec = MSEC;				// Maximum number of sectors per request

static volatile unsigned char *gpio_input = (unsigned char *)0xb8000060;
static volatile unsigned char *gpio_output = (unsigned char *)0xb8000064;
static volatile unsigned char *gpio_enable = (unsigned char *)0xb8000068;
static volatile unsigned char *gpio_control = (unsigned char *)0xb800006c;

#ifdef DEBUG
static int dbg = 0;					// Flag bits indicating what debugging to produce
#endif


// Set up module parameters
MODULE_PARM(major, "i");
MODULE_PARM_DESC(major, "Major device number to use");
MODULE_PARM (rahead, "i");
MODULE_PARM_DESC(rahead, "Device read ahead blocks");
MODULE_PARM (din, "i");
MODULE_PARM_DESC(din, "Gpio to assign to DI pin");
MODULE_PARM (dout, "i");
MODULE_PARM_DESC(dout, "Gpio to assign to DO pin");
MODULE_PARM (clk, "i");
MODULE_PARM_DESC(clk, "Gpio to assign to CLK pin");
MODULE_PARM (cs, "i");
MODULE_PARM_DESC(cs, "Gpio to assign to CS pin");
MODULE_PARM (maxsec, "i");
MODULE_PARM_DESC(maxsec, "Max sectors per request");
MODULE_PARM (gpio_input, "i");
MODULE_PARM_DESC(gpio_input, "Address of gpio input register");
MODULE_PARM (gpio_output, "i");
MODULE_PARM_DESC(gpio_output, "Address of gpio output register");
MODULE_PARM (gpio_enable, "i");
MODULE_PARM_DESC(gpio_enable, "Address of gpio enable register");
MODULE_PARM (gpio_control, "i");
MODULE_PARM_DESC(gpio_control, "Address of gpio control register");
#ifdef DEBUG
MODULE_PARM (dbg, "i");
MODULE_PARM_DESC(dbg, "Control debugging output");
#endif


// Block driver global arrays - indexed by major and minor numbers
static int mmc_sizes[NUMDEV << PSHIFT];			// size of each device in kb
static int mmc_blocksizes[NUMDEV << PSHIFT];		// size of block used by device
static int mmc_hardsectsizes[NUMDEV << PSHIFT];		// size of sectors on device
static int mmc_maxsect[NUMDEV << PSHIFT];		// max size of a single request
static struct hd_struct mmc_part[NUMDEV << PSHIFT];	// decoded device partition table


// Block device operations structure
static struct block_device_operations mmc_bdops = {
	open: 			mmc_open,
	release:		mmc_release,
	ioctl: 			mmc_ioctl,
//	check_media_change:	mmc_check_change,
//	revalidate:		mmc_revalidate,
};


// Generic disk support control structure
struct gendisk mmc_gendisk = {
    major:              0,              		// Major number assigned later
    major_name:         DEVICE_NAME,    		// Name of the major device
    minor_shift:        PSHIFT,         		// Shift to get device number
    nr_real:		NUMDEV,         		// Number of real devices
    max_p:              1 << PSHIFT,  			// Number of partitions
    part:		mmc_part,			// Pointer to partition global array
    sizes:		mmc_sizes,			// Pointer to sizes global array
    fops:               &mmc_bdops,     		// Block dev operations
};

// Methods for driving the spi bus
#include "spi.c"

// Methods for initializing the cards
#include "init.c"

static struct card_info *card = NULL;			// Card information structure (after init.c
                                                        // to avoid creation of init.h header file.)

// Support for /proc filesystem
static struct proc_dir_entry *mmcd;
static struct file_operations mmc_proc_fops = {
    read: mmc_proc_read,
    write: NULL
};

//=================================================================================================

/*-----------------------------------------------------------------------------
 * Module open function
 *-----------------------------------------------------------------------------*/
int mmc_open(struct inode *inode, struct file *filp) {
    (void)filp;						// Supress compiler warning
    (void)inode;					// Supress compiler warning

    if ((card->state & CARD_STATE_DET) == 0) {
        LOG_DEBUG(DBG_OPEN, "open: no device\n");
	return -ENODEV;
    }
    LOG_DEBUG(DBG_OPEN, "open: use_count: old=%d new=%d\n", MOD_IN_USE, MOD_IN_USE + 1);
    MOD_INC_USE_COUNT;

    return 0;
}


/*-----------------------------------------------------------------------------
 * Module release function
 *-----------------------------------------------------------------------------*/
int mmc_release(struct inode *inode, struct file *filp) {
    (void)filp;						// Supress compiler warning

    LOG_DEBUG(DBG_RLSE, "release: use_count: old=%d new=%d\n", MOD_IN_USE, MOD_IN_USE - 1);
    fsync_dev(inode->i_rdev);
    invalidate_buffers(inode->i_rdev);
    MOD_DEC_USE_COUNT;
    return 0;
}


/*-----------------------------------------------------------------------------
 * Module ioctl function
 *-----------------------------------------------------------------------------*/
int mmc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
    (void)filp;						// supress compiler warning

    LOG_DEBUG(DBG_IOCTL, "ioctl: cmd=%02x arg=%08lx\n", cmd, arg);

    if (!inode || !inode->i_rdev) return -EINVAL;

    switch(cmd) {
        case BLKGETSIZE:
	    return put_user(mmc_part[MINOR(inode->i_rdev)].nr_sects, (unsigned long *)arg);
        case BLKGETSIZE64:
	    return put_user((u64)mmc_part[MINOR(inode->i_rdev)].nr_sects, (u64 *) arg);
        case BLKRRPART:
	    if (!capable(CAP_SYS_ADMIN)) return -EACCES;
	    return mmc_revalidate(inode->i_rdev);
        case HDIO_GETGEO:
	    {
	    struct hd_geometry *loc, g;
	    loc = (struct hd_geometry *) arg;
	    if (!loc) return -EINVAL;
	    g.heads = 4;
	    g.sectors = 16;
	    g.cylinders = mmc_part[0].nr_sects / (4 * 16);
	    g.start = mmc_part[MINOR(inode->i_rdev)].start_sect;
	    return copy_to_user(loc, &g, sizeof(g)) ? -EFAULT : 0;
	    }
        default:
	    return blk_ioctl(inode->i_rdev, cmd, arg);
    }
}


/*-----------------------------------------------------------------------------
 * Module check change function
 *-----------------------------------------------------------------------------*/
int mmc_check_change(kdev_t i_rdev) {
    (void)i_rdev;					// Supress compiler warning

    LOG_DEBUG(DBG_CHG, "check_change: called\n");
    return 0;
}


/*-----------------------------------------------------------------------------
 * Module revalidate function
 *-----------------------------------------------------------------------------*/
int mmc_revalidate(kdev_t i_rdev) {
    int i, minor;

    if ((card->state & CARD_STATE_DET) == 0) {
        LOG_DEBUG(DBG_REVAL, "revalidate: no device\n");
	return -ENODEV;
    }
	
    LOG_DEBUG(DBG_REVAL, "revalidate: device=%d\n", DEVICE_NR(i_rdev));

    // Invalidate all partitions for device
    for (i = (1 << PSHIFT) - 1; i >= 0; i--) {
 	minor = (DEVICE_NR(i_rdev) << PSHIFT) + i;
	invalidate_device(MKDEV(MAJOR_NR, minor), 1);
	mmc_gendisk.part[minor].start_sect = 0;
	mmc_gendisk.part[minor].nr_sects = 0;
    }

    // Try to re-read partition table
    grok_partitions(&mmc_gendisk, DEVICE_NR(i_rdev), (1 << PSHIFT), mmc_part[0].nr_sects);

    return 0;
}


/*-----------------------------------------------------------------------------
 * Module request function
 *-----------------------------------------------------------------------------*/
static void mmc_request(request_queue_t *q) {
    unsigned int card_address;
    unsigned char *buffer_address;
    unsigned char first, last;
    unsigned int sector;
    unsigned int nr_sectors;
    unsigned int part_sector;
    struct request * req;
    struct buffer_head * buf_head;
    int cmd;
    int r, code;
    (void)q;								// Supress compiler warning

    while (1) {
	INIT_REQUEST;							// More requests on queue? Return if not...

        // Process all the sectors clustered in the current request
        first = 1;							// About to process first buffer in request.
        while (1) {
	    code = 1;							// Default return code is success

	    // Extract current request information
	    req = CURRENT;						// Pointer to current request
	    cmd = req->cmd;						// Command
	    sector = req->sector;					// Starting sector of request
	    nr_sectors = req->current_nr_sectors;			// Number of sectors in request
	    buffer_address = req->buffer;				// Buffer address
	    part_sector = mmc_part[MINOR(req->rq_dev)].start_sect;	// Starting sector of requested partition
	    buf_head = req->bh;						// Buffer head pointer
	    last = (buf_head->b_reqnext == NULL) ? 1 : 0;		// Determine if last buffer

	    // Calculate card address - byte addressing for standard density, block for high density.
	    card_address = (unsigned int)(part_sector + sector);	// Assume block addressing
	    if (!(card->type & CARD_TYPE_HC)) card_address <<= 9;	// Multiply by 512 if byte addressing

	    LOG_DEBUG(DBG_REQ, "Request: cmd=%02x sector=%08x nr_sectors=%08x minor=%02x start_sect=%08x card_address=%08x\n", cmd, sector, nr_sectors, MINOR(req->rq_dev), part_sector, card_address);
		    
	    if (!(card->state & CARD_STATE_DET)) {
		LOG_ERR("Request: IO request but no MM/SD card detected!\n");
		code = 0;
	    } else if ((part_sector + sector + nr_sectors) > mmc_part[0].nr_sects) {
		LOG_ERR("Request: Sector out of range!\n");
		code = 0;
	    } else if (cmd == READ) {
		spin_unlock_irq(&io_request_lock);
		r = spi_mmc_read_multi_o(card_address, buffer_address, nr_sectors, first, last);
		if (r != 0) {
		    LOG_ERR("Request: protocol error - cmd=read rc=%02x\n", r);
		    code = 0;
		}
		spin_lock_irq(&io_request_lock);

	    } else if (cmd == WRITE) {
		spin_unlock_irq(&io_request_lock);
		r = spi_mmc_write_multi_o(card_address, buffer_address, nr_sectors, first, last);
		if (r != 0) {
		    LOG_ERR("Request: protocol error - cmd=write rc=%02x\n", r);
		    code = 0;
		}
		spin_lock_irq(&io_request_lock);
	    } else {
		code = 0;			// Kernel passing an unknown cmd - no way!
	    }

	    // Buffer transfered. End the request and break out of loop if no more
	    // buffers clustered in the rquest.
	    if (end_that_request_first(req, code, DEVICE_NAME) == 0) break;

	    first = 0;
	}

        // All clusters in this request have been processed - tell the kernel.
	blkdev_dequeue_request(req);
	end_that_request_last(req);
    }
}


/*-----------------------------------------------------------------------------
 * Module /proc/mmc/status read function
 *-----------------------------------------------------------------------------*/
static ssize_t mmc_proc_read(struct file *file, char *buf, size_t count, loff_t *ppos) {
    (void)file;						// supress compiler warning
    char *page;
    int len = 0;

    if ((page = kmalloc(1024, GFP_KERNEL)) == NULL)
	return -ENOBUFS;

    // Following are used for mapping bit patterns to strings for printing
    char * typ_to_str[] = { "", "SD", "MM" };                             				// SD/MM
    char * cap_to_str[] = { "", "HC" };                             				// std/high capacity
    char * sdv_to_str[] = { "1.00 - 1.01", "1.10", "2.0" }; 					// SD version
    char * mmv_to_str[] = { "1.0 - 1.2", "1.4", "2.0 - 2.2", "3.0 - 3.3", "4.1 - 4.3" };        // MM version
    char * mon_to_str[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    // calc min/max voltage (times 10) from the OCR voltage window bits.
    unsigned int voltmin = 0;
    unsigned int voltmax = 0;
    unsigned int j;
    for (j = 4; j < 24 ; j++) {
	if ( (0x00000001 << j) & card->volt ) {
	    voltmax = j + 13;
            voltmin = (voltmin == 0) ? j + 12 : voltmin;
        }
    }

    len = sprintf(page, 
	"Card Type      : %s%s\n"
	"Spec Version   : %s\n"
	"Card Size      : %d MB\n"
	"Block Size     : 512 bytes\n"
	"Num. of Blocks : %d\n"
	"Voltage Range  : %d.%d-%d.%d\n"
	"Manufacture ID : %02x\n"
	"Application ID : %.2s\n"
	"Product Name   : %.6s\n"
	"Revision       : %d.%d\n"
	"Serial Number  : %08x\n"
	"Manu. Date     : %s %d\n",
	typ_to_str[card->type >> 4], cap_to_str[card->type & 0x0f],
	(card->type & CARD_TYPE_SD) ? sdv_to_str[card->version] : mmv_to_str[card->version],
	card->blocks / 2 / 1024,
	card->blocks,
	voltmin/10, voltmin%10, voltmax/10, voltmax%10,
	card->manid,
	card->appid,
	card->name,
	(card->rev >> 4), (card->rev & 0x0f),
	card->serial,
	mon_to_str[card->month],card->year
    );

    len += 1;
    if (*ppos < len) {
	len = min_t(int, len - *ppos, count);
	if (copy_to_user(buf, (page + *ppos), len)) {
	    kfree(page);
	    return -EFAULT;
	}
	*ppos += len;
    } else {
	len = 0;
    }

    kfree(page);
    return len;
}


/*-----------------------------------------------------------------------------
 * Module initialization function
 *-----------------------------------------------------------------------------*/
int mmc_init(void) {
    int i, r;
    static struct proc_dir_entry *p;

    // Log the module version and parameters
#ifdef DEBUG
    LOG_INFO("Version: " VERSION "  Parms: major=%d din=%d dout=%d clk=%d cs=%d maxsec=%d rahead=%d dbg=%04x\n", major, din, dout, clk, cs, maxsec, rahead, dbg);
#else
    LOG_INFO("Version: " VERSION "  Parms: major=%d din=%d dout=%d clk=%d cs=%d maxsec=%d rahead=%d\n", major, din, dout, clk, cs, maxsec, rahead);
#endif

    // Initialize spi module - set gpio pins used
    spi_init();

    // Try and initialize card - returns a card info structure on success.
    if ((r = init_card(&card))) {
	// Give it one more shot!
	if (card) kfree(card);
	if ((r = init_card(&card))) goto err1;
    }

    // Initialize the /proc entries for this module
    if (!(mmcd = proc_mkdir("mmc", NULL))) {
	LOG_ERR("Failure creating /proc/mmc\n");
	r = -EPROTO;
	goto err1;
    }
    if (!(p = create_proc_entry("status", S_IRUSR, mmcd))) {
	LOG_ERR("Failure creating /proc/mmc/status\n");
	r = -EPROTO;
	goto err1;
    }
    p->proc_fops = &mmc_proc_fops;

    // Register block device. Creates entry in /proc/devices and assigns device major num 
    r = register_blkdev(MAJOR_NR, DEVICE_NAME, &mmc_bdops);
    if (r < 0) {
	LOG_ERR("Failure requesting major %d - rc=%d\n", MAJOR_NR, r);
	goto err1;
    }
    if (MAJOR_NR == 0) {
	MAJOR_NR = r; 				// If requested major was 0, assign dynamic major num 
	LOG_INFO("Assigned dynamic major number %d\n", major);
    }
    mmc_gendisk.major = MAJOR_NR;	   	// Can now set major number in gendisk structure

    // initialize device structures
    memset(mmc_sizes, 0, sizeof(mmc_sizes));
    memset(mmc_part, 0, sizeof(mmc_part));
    for (i = 0; i < (1 << PSHIFT); i++) mmc_maxsect[i] = maxsec;
    mmc_sizes[0] = card->blocks / 2;		// number of 1024 byte blocks
    mmc_part[0].nr_sects = card->blocks;	// number of 512  byte sectors
    read_ahead[MAJOR_NR] = rahead;
    blk_size[MAJOR_NR] = mmc_sizes;
    blksize_size[MAJOR_NR] = mmc_blocksizes;
    hardsect_size[MAJOR_NR] = mmc_hardsectsizes;
    max_sectors[MAJOR_NR] = mmc_maxsect;

    // Initialize the block request queue
    blk_init_queue(BLK_DEFAULT_QUEUE(MAJOR_NR), mmc_request);

    // Put gendisk structure on the head of the list
    add_gendisk(&mmc_gendisk);

    // Register the disk - system will now read the partition table using mmc_request
    register_disk(&mmc_gendisk, MKDEV(MAJOR_NR,0), (1 << PSHIFT), &mmc_bdops, mmc_part[0].nr_sects);

    LOG_INFO("Module loaded\n");

    return 0; //success

    /* ----- Error handling ----- */
err1:
    if (card) kfree(card);
    LOG_INFO("Module unloaded due to error\n");
    return r;
}

/*-----------------------------------------------------------------------------
 * Module exit function
 *-----------------------------------------------------------------------------*/
void mmc_exit(void) {
    unregister_blkdev(MAJOR_NR, DEVICE_NAME);
    blk_cleanup_queue(BLK_DEFAULT_QUEUE(MAJOR_NR));
    if (card) kfree(card);
    remove_proc_entry("status", mmcd);
    remove_proc_entry("mmc", NULL);
    LOG_INFO("Module unloaded\n");
}


/*-----------------------------------------------------------------------------
 * Specify the functions to call for module initialization and exit
 *-----------------------------------------------------------------------------*/
module_init(mmc_init); 
module_exit(mmc_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("madsuk/Rohde/Cyril CATTIAUX/Marc DENTY/rcichielo KRUSCH/Chris");
MODULE_DESCRIPTION("MMHC/SDHC Card Block Driver - " VERSION);
MODULE_SUPPORTED_DEVICE("WRT54GL");
