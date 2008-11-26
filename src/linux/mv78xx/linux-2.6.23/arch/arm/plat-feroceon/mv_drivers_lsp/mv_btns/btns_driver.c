/*******************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include "btns_dev.h"
#include "gpp/mvGpp.h"
#include "btns_driver.h"


/* MACROS */
#define GPP_GROUP(gpp) 	gpp/32
#define GPP_ID(gpp)   	gpp%32
#define GPP_BIT(gpp)	0x1 << GPP_ID(gpp)

/* waiting Q */
wait_queue_head_t btns_waitq;

/*
 * common debug for all
 */
#undef DEBUG

#ifdef DEBUG
#define dprintk   printk
#else
#define dprintk(a...)
#endif

/* At GPP initialization, this strucure is filled with what       
 * operation will be monitored for each button (Push and/or          
 * Release) */ 
BTN_OP 	btn_op_cfg[CONFIG_MV_GPP_MAX_PINS] = {BTN_NO_OP};

/* At GPP initialization, this strucure is filled with what       
 * operation will be monitored for each button (Push and/or          
 * Release) */ 
u32 	gpp_default_val_cfg[CONFIG_MV_GPP_MAX_PINS] = {-1};

/* This structures monitors how many time each button was 	  
 * Push/Released since the last time it was sampled */ 
BTN		btns_status[CONFIG_MV_GPP_MAX_PINS];

u32		is_opend = 0;
u32		gpp_changed = 0;
u32		gpp_changed_id = -1;


/*
 * Get GPP real value....
 * When Gpp is input:
 * Since reading GPP_DATA_IN_REG return the GPP real value after considering the active polarity
 * active low = 1, active high = 0
 * we will use: val^0 -> val, val^1->not(val)
 * when output don't consider the active polarity
 */
static unsigned int 
mv_gpp_value_real_get(unsigned int gpp_group, unsigned int mask)
{
        unsigned int temp, in_out, gpp_val;
        /* in ->1,  out -> 0 */
        in_out = MV_REG_READ(GPP_DATA_OUT_EN_REG(gpp_group)) & mask;

	gpp_val = MV_REG_READ(GPP_DATA_IN_REG(gpp_group)) & mask;

        /* outputs values */
        temp = (gpp_val & ~in_out);

        /* input */
        temp |= (( gpp_val ^ MV_REG_READ(GPP_DATA_IN_POL_REG(gpp_group)) ) & in_out) & mask;

        return temp;
}


static irqreturn_t
mv_btns_handler(int irq , void *dev_id)
{
	u32 gpp = (u32) irq - IRQ_GPP_START;
	u32 gpp_level, gppVal;
	BTN_OP btn_op;
	
	/* get gpp real val */
	gppVal = mv_gpp_value_real_get(GPP_GROUP(gpp), GPP_BIT(gpp));

	dprintk("Gpp value was changed: ");
        if (btn_op_cfg[gpp] != BTN_NO_OP) {
                dprintk("gpp %d has changed. now it's %x \n",gpp,
				mv_gpp_value_real_get(GPP_GROUP(gpp), GPP_BIT(gpp)) );
		
		/* Count button Pushes/Releases
		 * mv_gpp_value_real_get == gpp_default_val_cfg[gpp] --> Button push, 
		 * else --> Button release
		 */
		btn_op = (gppVal == gpp_default_val_cfg[gpp]) ? BTN_PUSH : BTN_RELEASE;

		if(btn_op == BTN_RELEASE)
		{
			dprintk("button (of gpp %d) was released \n",gpp);

			btns_status[gpp].btn_release_cntr++;
		} else {
			dprintk("button (of gpp %d) was pressed \n",gpp);
			
			btns_status[gpp].btn_push_cntr++;
		}
		
                /* change polarity */
                gpp_level = MV_REG_READ(GPP_DATA_IN_POL_REG(GPP_GROUP(gpp)));
                gpp_level = gpp_level^GPP_BIT(gpp);
                MV_REG_WRITE(GPP_DATA_IN_POL_REG(GPP_GROUP(gpp)), gpp_level);

		/* Check if current botton operation should be monitored */
		if(btn_op_cfg[gpp] == btn_op || btn_op_cfg[gpp] == BTN_CHANGE)
		{
			gpp_changed = 1;
			gpp_changed_id = gpp;
			wake_up_interruptible(&btns_waitq);
		}
	}

	return IRQ_HANDLED;
}

static int
btnsdev_ioctl(
        struct inode *inode,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
	unsigned int btn_id;
	BTN btn_sts;
	BTN_PTR user_btn_sts_ptr;
	unsigned int error = 0;
	int i;	

        dprintk("%s()\n", __FUNCTION__);

        switch (cmd) {
        case CIOCWAIT_P:
		/* Haim - Is the condition here correct? */
            	error = wait_event_interruptible(btns_waitq, gpp_changed);
		/* Reset Wait Q condition */
		gpp_changed = 0;

		if(error < 0)
			dprintk("%s(CIOCWAIT_P) - got interrupted\n", __FUNCTION__);
		
		/* Set information for user*/
		btn_sts.btn_id = gpp_changed_id;
		btn_sts.btn_push_cntr   =btns_status[gpp_changed_id].btn_push_cntr;
		btn_sts.btn_release_cntr=btns_status[gpp_changed_id].btn_release_cntr;
		
		dprintk("Button ID %d was pressed %d and released %d\n",gpp_changed_id,
			btns_status[gpp_changed_id].btn_push_cntr,btns_status[gpp_changed_id].btn_release_cntr);

		user_btn_sts_ptr = &(((BTNS_STS_PTR)arg)->btns[0]);
		if ( copy_to_user((void*)user_btn_sts_ptr, &btn_sts,  sizeof(BTN)) ) {
                        dprintk("%s(CIOCWAIT_P) - bad copy\n", __FUNCTION__);
                        error = EFAULT;
                }

		/* Reset changed button operations counters*/
		btns_status[gpp_changed_id].btn_push_cntr = 0;
		btns_status[gpp_changed_id].btn_release_cntr = 0;

                break;
	case CIOCNOWAIT_P:
		/* Eventhough we don't monitor for a button status change, we need to 
 			reset the indication of a change in case it happend */
		gpp_changed = 0;

		dprintk("There are %d buttons to be checked\n", ((BTNS_STS_PTR)arg)->btns_number);

		/* Set information for user*/
		for (i=0; i<((BTNS_STS_PTR)arg)->btns_number; i++)
		{
			btn_id = ((BTNS_STS_PTR)arg)->btns[i].btn_id;

			/* initialize temp strucure which will be copied to user */
			btn_sts.btn_id = btn_id;
			btn_sts.btn_push_cntr = btns_status[btn_id].btn_push_cntr;
			btn_sts.btn_release_cntr = btns_status[btn_id].btn_release_cntr;

			/* Reset button's operations counters*/
			btns_status[btn_id].btn_push_cntr = 0;
			btns_status[btn_id].btn_release_cntr = 0;

			/* Copy temp structure to user */
			user_btn_sts_ptr = &(((BTNS_STS_PTR)arg)->btns[i]);

			if ( copy_to_user((void*)user_btn_sts_ptr, &btn_sts,  sizeof(BTN)) ) {
				dprintk("%s(CIOCNOWAIT_P) - bad copy\n", __FUNCTION__);
				error = EFAULT;
			}
		}
		
		break;
        default:
                dprintk("%s(unknown ioctl 0x%x)\n", __FUNCTION__, cmd);
                error = EINVAL;
                break;
        }
        return(-error);
}

/*
 * btn_gpp_init
 * initialize on button's GPP and registers its IRQ
 *
 */
static int  
btn_gpp_init(unsigned int gpp, unsigned int default_gpp_val, BTN_OP btn_op, char* btn_name)
{
	unsigned int pol;
	/* Set Polarity bit */
	pol = MV_REG_READ(GPP_DATA_IN_POL_REG(GPP_GROUP(gpp)));
	pol |= GPP_BIT(gpp);
	MV_REG_WRITE(GPP_DATA_IN_POL_REG(GPP_GROUP(gpp)), pol);
	
	/* Set which button operation should be monitored */
	btn_op_cfg[gpp] = btn_op;

	/* Set GPP default value*/
	gpp_default_val_cfg[gpp] = default_gpp_val;

	/* Register IRQ */
	if( request_irq( IRQ_GPP_START + gpp, mv_btns_handler,
		IRQF_DISABLED, btn_name, NULL ) ) 
	{
		printk( KERN_ERR "btnsdev:  can't get irq for button %s (GPP %d)\n",btn_name,gpp );
		return -1;
	}

	return 0;
}

static int
btnsdev_open(struct inode *inode, struct file *filp)
{
        dprintk("%s()\n", __FUNCTION__);

	if(!is_opend) {
		is_opend = 1;
		
		/* Reset button operations counters*/
		memset(&btns_status,0,CONFIG_MV_GPP_MAX_PINS);
	}

        return(0);
}

static int
btnsdev_release(struct inode *inode, struct file *filp)
{
        dprintk("%s()\n", __FUNCTION__);
        return(0);
}


static struct file_operations btnsdev_fops = {
        .open = btnsdev_open,
        .release = btnsdev_release,
        .ioctl = btnsdev_ioctl,
};

static struct miscdevice btnsdev = {
        .minor = BTNSDEV_MINOR,
        .name = "btns",
        .fops = &btnsdev_fops,
};


static int 
btns_probe(struct platform_device *pdev)
{
        struct btns_platform_data *btns_data = pdev->dev.platform_data;
        int ret, i;
	
	dprintk("%s\n", __FUNCTION__);
	printk(KERN_NOTICE "MV Buttons Driver Load\n");

        for (i = 0; i < btns_data->btns_num; i++) {
		ret = btn_gpp_init(btns_data->btns_data_arr[i].gpp_id, btns_data->btns_data_arr[i].default_gpp_val, 
					btns_data->btns_data_arr[i].btn_op, btns_data->btns_data_arr[i].btn_name);

		if (ret != 0) {
			return ret;
		}
        }

        return 0;
}


static struct platform_driver btns_driver = {
	.probe          = btns_probe,
	.driver  = {
        	.name		= MV_BTNS_NAME,
	},
};


static int __init
btnsdev_init(void)
{
	int rc;

	dprintk("%s\n", __FUNCTION__);

	/* Initialize Wait Q*/
	init_waitqueue_head(&btns_waitq);
	
	/* Register btns device */
	if (misc_register(&btnsdev)) 
        {
            printk(KERN_ERR "btnsdev: registration of /dev/btnsdev failed\n");
            return -1;
        }

	/* Register platform driver*/
	rc = platform_driver_register(&btns_driver);
	if (rc) {
		printk(KERN_ERR "btnsdev: registration of platform driver failed\n");
		return rc;
	}

        return 0;
}

static void __exit
btnsdev_exit(void)
{
	dprintk("%s() should never be called.\n", __FUNCTION__);
}

module_init(btnsdev_init);
module_exit(btnsdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ronen Shitrit & Haim Boot");
MODULE_DESCRIPTION("PH: Buttons press handling.");


