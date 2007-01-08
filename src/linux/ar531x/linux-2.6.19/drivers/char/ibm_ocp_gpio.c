/*
 * FILE NAME ibm_ocp_gpio.c
 *
 * BRIEF MODULE DESCRIPTION
 *  API for IBM PowerPC 4xx GPIO device.
 *  Driver for IBM PowerPC 4xx GPIO device.
 *
 *  Armin Kuster akuster@pacbell.net
 *  Sept, 2001
 *
 *  Orignial driver
 *  Author: MontaVista Software, Inc.  <source@mvista.com>
 *          Frank Rowand <frank_rowand@mvista.com>
 *          Debbie Chu   <debbie_chu@mvista.com>
 *
 * Copyright 2000,2001,2002 MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Version: 02/01/12 - Armin
 *			 converted to ocp and using ioremap
 *
 *	1.2 02/21/01 - Armin
 *		minor compiler warning fixes
 *
 *	1.3 02/22/01 - Armin
 *		added apm
 *
 *	1.4 05/07/02 - Armin/David Mueller
 *		coverted to core_ocp[];
 *
 *	1.5 05/25/02 - Armin
 *	 name change from *_driver to *_dev
 *
 *	1.6 06/04/02 - Matt Porter
 *	ioremap paddr. Comment as 4xx generic driver.
 *	Fix header to be userland safe and locate in
 *	an accessible area.  Add ioctl to configure
 *	multiplexed GPIO pins.
 *
 *	1.7 07/25/02 - Armin
 *	added CPM to enable/disable in init/exit
 *	
 *	TODO: - devfs (Armin)
 *	      - Move selection DCRN to "additional" infos instead
 *	        of #ifdefs. Sort out that pm_access crap.
 *
 *
 */

#define VUFX "07.25.02"

#include <linux/module.h>
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/pm.h>
#include <linux/ibm_ocp_gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/ocp.h>


typedef struct gpio_regs {
	u32 or;
	u32 tcr;
	u32 pad[4];
	u32 odr;
	u32 ir;
} gpio_t;

struct miscdevice ibm_gpio_miscdev;
static struct gpio_regs *gpiop;
static struct ocp_device *gpio_ocpdev;

#ifdef _CONFIG_PM_FIXME_
static struct pm_dev *pm_gpio;
#endif

static int
ibm_gpio_save_state(struct ocp_device *dev, u32 state)
{
	return 0;
}

static int
ibm_gpio_suspend(struct ocp_device *dev, pm_message_t state)
{
	ocp_force_power_off(gpio_ocpdev);
	return 0;
}

static int
ibm_gpio_resume(struct ocp_device *dev)
{
	ocp_force_power_on(gpio_ocpdev);
	return 0;
}

int
ibm_gpio_config(__u32 device, __u32 mask, __u32 data)
{
	u32 cfg_reg;

	if (device != 0)
		return -ENXIO;
#ifdef _CONFIG_PM_FIXME_
	pm_access(pm_gpio);
#endif

#ifdef CONFIG_40x
#ifdef DCRN_CHCR0
	/*
	 * PPC405 uses CPC0_CR0 to select multiplexed GPIO pins.
	 */
	cfg_reg = mfdcr(DCRN_CHCR0);
	cfg_reg = (cfg_reg & ~mask) | (data & mask);
	mtdcr(DCRN_CHCR0, cfg_reg);
#endif
#elif CONFIG_440
	/*
	 * PPC440 uses CPC0_GPIO to select multiplexed GPIO pins.
	 */
	cfg_reg = mfdcr(DCRN_CPC0_GPIO);
	cfg_reg = (cfg_reg & ~mask) | (data & mask);
	mtdcr(DCRN_CPC0_GPIO, cfg_reg);
#else
#error This driver is only supported on PPC40x and PPC440 CPUs
#endif

	return 0;
}

int
ibm_gpio_tristate(__u32 device, __u32 mask, __u32 data)
{
	if (device != 0)
		return -ENXIO;
#ifdef _CONFIG_PM_FIXME_
	pm_access(pm_gpio);
#endif
	gpiop->tcr = (gpiop->tcr & ~mask) | (data & mask);
	return 0;
}

int
ibm_gpio_open_drain(__u32 device, __u32 mask, __u32 data)
{
	if (device != 0)
		return -ENXIO;
#ifdef _CONFIG_PM_FIXME_
	pm_access(pm_gpio);
#endif
	gpiop->odr = (gpiop->odr & ~mask) | (data & mask);

	return 0;
}

int
ibm_gpio_in(__u32 device, __u32 mask, volatile __u32 * data)
{
	if (device != 0)
		return -ENXIO;
#ifdef _CONFIG_PM_FIXME_
	pm_access(pm_gpio);
#endif
	gpiop->tcr = gpiop->tcr & ~mask;
	eieio();

	/*
	   ** If the previous state was OUT, and gpiop->ir is read once, then the
	   ** data that was being OUTput will be read.  One way to get the right
	   ** data is to read gpiop->ir twice.
	 */

	*data = gpiop->ir;
	*data = gpiop->ir & mask;
	eieio();
	return 0;
}

int
ibm_gpio_out(__u32 device, __u32 mask, __u32 data)
{
	if (device != 0)
		return -ENXIO;
#ifdef _CONFIG_PM_FIXME_
	pm_access(pm_gpio);
#endif
	gpiop->or = (gpiop->or & ~mask) | (data & mask);
	eieio();
	gpiop->tcr = gpiop->tcr | mask;
	eieio();
	return 0;
}

static int
ibm_gpio_open(struct inode *inode, struct file *file)
{
//	MOD_INC_USE_COUNT;

	return 0;
}

static int
ibm_gpio_release(struct inode *inode, struct file *file)
{
//	MOD_DEC_USE_COUNT;

	return 0;
}

static int
ibm_gpio_ioctl(struct inode *inode, struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	static struct ibm_gpio_ioctl_data ioctl_data;
	int status;

	if (gpio_ocpdev == NULL)
		return -ENODEV;

	switch (cmd) {
	case IBMGPIO_IN:
		if (copy_from_user(&ioctl_data,
				   (struct ibm_gpio_ioctl_data *) arg,
				   sizeof (ioctl_data))) {
			return -EFAULT;
		}

		status = ibm_gpio_in(ioctl_data.device,
				     ioctl_data.mask, &ioctl_data.data);
		if (status != 0)
			return status;

		if (copy_to_user((struct ibm_gpio_ioctl_data *) arg,
				 &ioctl_data, sizeof (ioctl_data))) {
			return -EFAULT;
		}

		break;

	case IBMGPIO_OUT:
		if (copy_from_user(&ioctl_data,
				   (struct ibm_gpio_ioctl_data *) arg,
				   sizeof (ioctl_data))) {
			return -EFAULT;
		}

		return ibm_gpio_out(ioctl_data.device,
				    ioctl_data.mask, ioctl_data.data);

		break;

	case IBMGPIO_OPEN_DRAIN:
		if (copy_from_user(&ioctl_data,
				   (struct ibm_gpio_ioctl_data *) arg,
				   sizeof (ioctl_data))) {
			return -EFAULT;
		}

		return ibm_gpio_open_drain(ioctl_data.device,
					   ioctl_data.mask, ioctl_data.data);

		break;

	case IBMGPIO_TRISTATE:
		if (copy_from_user(&ioctl_data,
				   (struct ibm_gpio_ioctl_data *) arg,
				   sizeof (ioctl_data)))
			return -EFAULT;

		return ibm_gpio_tristate(ioctl_data.device,
					 ioctl_data.mask, ioctl_data.data);

		break;

	case IBMGPIO_CFG:
		if (copy_from_user(&ioctl_data,
				   (struct ibm_gpio_ioctl_data *) arg,
				   sizeof (ioctl_data)))
			return -EFAULT;

		return ibm_gpio_config(ioctl_data.device,
				ioctl_data.mask, ioctl_data.data);

		break;

	default:
		return -ENOIOCTLCMD;

	}
	return 0;
}

static struct file_operations ibm_gpio_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= ibm_gpio_ioctl,
	.open		= ibm_gpio_open,
	.release	= ibm_gpio_release,
};

static int
ibm_gpio_probe(struct ocp_device *ocpdev)
{
	int rc;
	
	/* We suppport only one instance */
	if (gpio_ocpdev != NULL)
		return -ENODEV;
	
	ibm_gpio_miscdev.minor = 185;	/*GPIO_MINOR; */
	ibm_gpio_miscdev.name = "ibm_gpio";
	ibm_gpio_miscdev.fops = &ibm_gpio_fops;
	rc = misc_register(&ibm_gpio_miscdev);	/*ibm_gpio_miscdev); */
	if (rc != 0)
		return rc;
	gpiop = (struct gpio_regs *)ioremap(ocpdev->def->paddr,
					sizeof(struct gpio_regs));
	ocp_force_power_on(ocpdev);
	printk("GPIO #%d at 0x%lx\n", ocpdev->def->index, (unsigned long) gpiop);
	gpio_ocpdev = ocpdev;

	return 0;
}

static void
ibm_gpio_remove(struct ocp_device *ocpdev)
{
	if (gpio_ocpdev != ocpdev)
		return;
	gpio_ocpdev = NULL;
	misc_deregister(&ibm_gpio_miscdev);
	ocp_force_power_off(ocpdev);
}

/* Structure for a device driver */
static struct ocp_device_id ibm_gpio_ids[] =
{
	{ .vendor = OCP_ANY_ID, .function = OCP_FUNC_GPIO },
	{ .vendor = OCP_VENDOR_INVALID }
};

static struct ocp_driver ibm_gpio_driver =
{
	.name 		= "gpio",
	.id_table	= ibm_gpio_ids,
	
	.probe		= ibm_gpio_probe,
	.remove		= ibm_gpio_remove,
	.suspend	= ibm_gpio_suspend,
	.resume		= ibm_gpio_resume,
//	.save_state	= ibm_gpio_save_state,
};

static int __init
ibm_gpio_init(void)
{	
	printk("IBM gpio driver version %s\n", VUFX);
	return ocp_register_driver(&ibm_gpio_driver);
}

static void __exit
ibm_gpio_exit(void)
{
	ocp_unregister_driver(&ibm_gpio_driver);
}

module_init(ibm_gpio_init);
module_exit(ibm_gpio_exit);

EXPORT_SYMBOL(ibm_gpio_tristate);
EXPORT_SYMBOL(ibm_gpio_open_drain);
EXPORT_SYMBOL(ibm_gpio_in);
EXPORT_SYMBOL(ibm_gpio_out);

MODULE_LICENSE("GPL");
