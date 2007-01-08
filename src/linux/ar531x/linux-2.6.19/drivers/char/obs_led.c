/*
 * Copyright (c) AXE,Inc.
 *	Based on ibm_ocp_gpio.c
 */
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
 *	TODO: devfs
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
 */

#define OBSLED_VER	"0.01"
#define OBSLED_MINOR	171

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

/* General purpose i/o */

typedef struct gpio_regs {
	u32 or;
	u32 tcr;
	u32 pad[4];
	u32 odr;
	u32 ir;
} gpio_t;

struct miscdevice obsled_miscdev;
static gpio_t *gpiop;
static int is_open = 0;

#define GPIO_BIT(n)	(1 << (31 - (n)))
#define PAT_1	GPIO_BIT(12)
#define PAT_2	GPIO_BIT(13)
#define PAT_4	GPIO_BIT(14)

#define ocp_get_pm(type, num)	(OCP_CPM_NA)

#ifdef CONFIG_PM
static struct pm_dev *pm_gpio;

static int gpio_save_state(u32 state)
{
	return 0;
}

static int gpio_suspend(u32 state)
{
	mtdcr(DCRN_CPMFR, mfdcr(DCRN_CPMFR) | state);
	return 0;
}

static int gpio_resume(u32 state)
{
	mtdcr(DCRN_CPMFR, mfdcr(DCRN_CPMFR) & ~state);
	return 0;
}
#endif

static int obsled_out_reg(int led, u32 *or, u32 *tcr)
{
	__u32 data;
	__u32 mask = PAT_1 | PAT_2 | PAT_4;
	data = 0;
	if (led & 1) data |= PAT_1;
	if (led & 2) data |= PAT_2;
	if (led & 4) data |= PAT_4;

	*or = (*or & ~mask) | ((~data) & mask);
	eieio();
	*tcr = *tcr | mask;
	eieio();
	return 0;
}

int obsled_out_pat(int led)
{
#ifdef CONFIG_PM
	pm_access(pm_gpio);
#endif
	return obsled_out_reg(led, &gpiop->or, &gpiop->tcr);
}

static int obsled_open(struct inode *inode, struct file *file)
{
	if (is_open) return -EALREADY;
	is_open = 1;

	MOD_INC_USE_COUNT;

	return 0;
}

static int obsled_release(struct inode *inode, struct file *file)
{
	is_open = 0;

	MOD_DEC_USE_COUNT;

	return 0;
}

static ssize_t obsled_write(struct file *file, const char *buf, size_t count,
			    loff_t *ppos)
{
	int err, i, led;

	if (count <= 0) {
		return 0;
	}

	for (i = 0; i < count; i++) {
		err = get_user(led, buf + i);
		if (err) {
			return err;
		}
		if (led < '0' || '7' < led) {
			/* skip */
			continue;
		}
		err = obsled_out_pat(led);
		if (err) {
			return err;
		}
	}

	return count;
}

static struct file_operations obsled_fops = {
	owner:		THIS_MODULE,
	write:		obsled_write,
	open:		obsled_open,
	release:	obsled_release,
};

static int __init obsled_init(void)
{
	int i;
	struct ocp_device *gpio_dev;

	printk("OBS_LED driver version %s\n", OBSLED_VER);

	for (i = 0; i < ocp_get_num(OCP_FUNC_GPIO); i++) {
		gpio_dev = ocp_get_dev(OCP_FUNC_GPIO, i);
		if (!gpio_dev)
			break;

		obsled_miscdev.minor = OBSLED_MINOR;
		obsled_miscdev.name = gpio_dev->name;
		obsled_miscdev.fops = &obsled_fops;
		misc_register(&obsled_miscdev);

		gpiop = (struct gpio_regs *)ioremap(
				gpio_dev->paddr, sizeof(struct gpio_regs));

		mtdcr(DCRN_CPMFR,
			mfdcr(DCRN_CPMFR) & ~ocp_get_pm(OCP_FUNC_GPIO, i));

		printk("OBS_LED: GPIO #%d at 0x%lx\n",
			       	i, (unsigned long)gpiop);
	}

	return 0; 
}

static void __exit obsled_exit(void)
{
	int i;
	struct ocp_device *gpio_dev;

	for (i = 0; i < ocp_get_num(OCP_FUNC_GPIO); i++) {
		gpio_dev = ocp_get_dev(OCP_FUNC_GPIO, i);
		misc_deregister(&obsled_miscdev);
		mtdcr(DCRN_CPMFR,
			mfdcr(DCRN_CPMFR) | ocp_get_pm(OCP_FUNC_GPIO, i));
	}
}

module_init(obsled_init);
module_exit(obsled_exit);

/* EXPORT_SYMBOL(obsled_out_pat); */

MODULE_LICENSE("GPL");

#define GPIO_OR		((u32 *)0xef600700)
#define GPIO_TCR	((u32 *)0xef600704)

int obsled_out(int led)
{
	return obsled_out_reg(led, GPIO_OR, GPIO_TCR);
}

