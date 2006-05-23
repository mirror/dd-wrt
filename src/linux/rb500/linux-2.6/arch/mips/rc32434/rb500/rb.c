#include <linux/module.h> /* module to be loadable (include kernel version) */
#include <linux/miscdevice.h> /* misc_register() */
#include <linux/kernel.h>   /* printk() */
#include <linux/types.h>    /* size_t */
#include <asm/uaccess.h>    /* copy_from_user() */
#include <linux/pci.h>
#include <asm/rc32434/rb.h>

#define RBMIPS_MINOR 245

#define POKE_MIPS_IOCTL 1

static volatile unsigned char *devCtl3Base = 0;
static unsigned char latchU5State = 0;
static struct semaphore lock;
static spinlock_t clu5Lock = SPIN_LOCK_UNLOCKED;

void set434Reg(unsigned regOffs, unsigned bit, unsigned len, unsigned val) {
    unsigned flags, data;
    unsigned i = 0;
    spin_lock_irqsave(&clu5Lock, flags);
    data = *(volatile unsigned *) (IDT434_REG_BASE + regOffs);
    for (i = 0; i != len; ++i) {
	if (val & (1 << i)) data |= (1 << (i + bit));
	else data &= ~(1 << (i + bit));
    }
    *(volatile unsigned *) (IDT434_REG_BASE + regOffs) = data;
    spin_unlock_irqrestore(&clu5Lock, flags);
}

void changeLatchU5(unsigned char orMask, unsigned char nandMask) {
    unsigned flags;
    spin_lock_irqsave(&clu5Lock, flags);
    if (!devCtl3Base) {
        devCtl3Base = (volatile unsigned char *)
	        KSEG1ADDR(*(volatile unsigned *) KSEG1ADDR(0x18010030));
    }
    latchU5State = (latchU5State | orMask) & ~nandMask;
    *devCtl3Base = latchU5State;
    spin_unlock_irqrestore(&clu5Lock, flags);
}

static int rbmips_open(struct inode *inode, struct file *filp)
{
//     MOD_INC_USE_COUNT;
    return 0;
}

static int rbmips_release(struct inode *inode, struct file *filp)
{
//     MOD_DEC_USE_COUNT;
    return 0;
}

static int rbmips_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    default: return -ENOTTY;
    case POKE_MIPS_IOCTL: {
	if (down_interruptible(&lock)) return -ERESTARTSYS;
	up(&lock);
	return 0;
    }
    }
}

struct file_operations rbmips_fops = {
    owner:	THIS_MODULE,
    open:       rbmips_open,
    release:    rbmips_release,
    ioctl:      rbmips_ioctl,
};

static struct miscdevice rbmips_miscdev = {
    RBMIPS_MINOR,
    "rbmips",
    &rbmips_fops
};

int init_module(void)
{
    {
	int result = misc_register(&rbmips_miscdev);
	if (result < 0) {
	    printk(KERN_WARNING "rbmips: failed to register char device\n");
	    return result;
	}
    }
    sema_init(&lock, 1);
    changeLatchU5(LO_WPX | LO_FOFF, LO_ULED);
    printk(KERN_INFO "rbmips module loaded\n");
    return 0;
}

void cleanup_module(void)
{
    misc_deregister(&rbmips_miscdev);
    printk(KERN_INFO "rbmips module removed\n");
}

EXPORT_SYMBOL(set434Reg);
EXPORT_SYMBOL(changeLatchU5);
