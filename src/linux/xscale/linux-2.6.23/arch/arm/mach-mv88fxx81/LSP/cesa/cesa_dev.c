#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#include <cesa_dev.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
#include <linux/syscalls.h>
#endif

static int debug = 1;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Enable debug");

#ifdef CONFIG_MV_CESA_TEST

static int buf_size = 20000;
module_param(buf_size, int, 0);
MODULE_PARM_DESC(buf_size, "Size of each data buffer");

static int buf_num = 1;
module_param(buf_num, int, 0);
MODULE_PARM_DESC(buf_num, "Number of data buffers for each request");

extern void cesaTestStart(int bufNum, int bufSize);
extern void cesaTest(int iter, int reqSize, int checkMode);
extern void combiTest(int iter, int reqSize, int checkMode);
extern void cesaOneTest(int testIdx, int caseIdx, int iter, int reqSize, int checkMode);
extern void multiSizeTest(int idx, int checkMode, int iter, char* inputData);
extern void aesTest(int iter, int reqSize, int checkMode);
extern void desTest(int iter, int reqSize, int checkMode);
extern void tripleDesTest(int iter, int reqSize, int checkMode);
extern void mdTest(int iter, int reqSize, int checkMode);
extern void shaTest(int iter, int reqSize, int checkMode);

int run_cesa_test(CESA_TEST *cesa_test)
{
	switch(cesa_test->test){
		case(MULTI):
			combiTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);
			break;
		case(SIZE):
                        multiSizeTest(cesa_test->req_size, cesa_test->iter, cesa_test->checkmode, NULL);
			break;
		case(SINGLE):
			cesaOneTest(cesa_test->session_id, cesa_test->data_id, cesa_test->iter, 
					cesa_test->req_size, cesa_test->checkmode);
			break;
		case(AES):
			aesTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);		
			break;
		case(DES):
			desTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);
			break;
		case(TRI_DES):
			tripleDesTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);
			break;
		case(MD5):
                        mdTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);
                        break;
		case(SHA1):
                        shaTest(cesa_test->iter, cesa_test->req_size, cesa_test->checkmode);
                        break;
		default:
			dprintk("%s(unknown test 0x%x)\n", __FUNCTION__, cesa_test->test);
			return -EINVAL;
	}
	return 0;
}
#endif /* CONFIG_MV_CESA_TEST */

extern void    		mvCesaDebugSAD(int mode);
extern void    		mvCesaDebugCacheIdx(int idx);
extern void    		mvCesaDebugSA(short sid, int mode);
extern void    		mvCesaDebugQueue(int mode);
extern void    		mvCesaDebugChan(int chan, int mode);
extern void    		mvCesaDebugStatus(void);
extern void    		mvCesaDebugSram(int mode);
extern void    		cesaDebugReq(int req, int offset, int size);
extern int	   	    cesaDebugPrintSession(int idx);


int run_cesa_debug(CESA_DEBUG *cesa_debug)
{
	int error = 0;
	switch(cesa_debug->debug){
		case(STATUS):
			mvCesaDebugStatus();
			break;
		case(CHAN):
			mvCesaDebugChan(cesa_debug->index, cesa_debug->mode);
			break;
		case(QUEUE):
			mvCesaDebugQueue(cesa_debug->mode);
			break;
		case(SA):
			mvCesaDebugSA(cesa_debug->index, cesa_debug->mode);
			break;
		case(CACHE_IDX):
			mvCesaDebugCacheIdx(cesa_debug->index);
			break;
		case(SRAM):
			mvCesaDebugSram(cesa_debug->mode);
			break;
		case(SAD):
			mvCesaDebugSAD(cesa_debug->mode);
			break;

#ifdef CONFIG_MV_CESA_TEST
		case(TST_REQ):
			cesaDebugReq(cesa_debug->index, 0, cesa_debug->size);
			break;
		case(TST_SES):
			error = cesaDebugPrintSession(cesa_debug->index);
			break;
#endif /* CONFIG_MV_CESA_TEST */

		default:
			dprintk("%s(unknown debug 0x%x)\n", __FUNCTION__, cesa_debug->debug);
			error = EINVAL;
			break;

	}

	return(-error);
}


static int
cesadev_ioctl(
	struct inode *inode,
	struct file *filp,
	unsigned int cmd,
	unsigned long arg)
{	
	CESA_DEBUG cesa_debug;
	u32 error = 0;

	dprintk("%s: cmd=0x%x, CIOCDEBUG=0x%x, CIOCTEST=0x%x\n", 
                __FUNCTION__, cmd, CIOCDEBUG, CIOCTEST);

	switch (cmd) {
	case CIOCDEBUG:
		copy_from_user(&cesa_debug, (void*)arg, sizeof(CESA_DEBUG));
		dprintk("%s(CIOCDEBUG): debug %d index %d mode %d size %d\n", 
			__FUNCTION__, cesa_debug.debug, cesa_debug.index, cesa_debug.mode, cesa_debug.size);
		error = run_cesa_debug(&cesa_debug);
		break;

#ifdef CONFIG_MV_CESA_TEST
    case CIOCTEST:
		{
		CESA_TEST cesa_test;

		copy_from_user(&cesa_test, (void*)arg, sizeof(CESA_TEST));
		dprintk("%s(CIOCTEST): test %d iter %d req_size %d checkmode %d sess_id %d data_id %d \n", 
			__FUNCTION__, cesa_test.test, cesa_test.iter, cesa_test.req_size, cesa_test.checkmode,
			cesa_test.session_id, cesa_test.data_id );
		error = run_cesa_test(&cesa_test);
		}
		break;
#endif /* CONFIG_MV_CESA_TEST */

	default:
		dprintk("%s (unknown ioctl 0x%x)\n", __FUNCTION__, cmd);
		error = EINVAL;
		break;
	}
	return(-error);
}


static int
cesadev_open(struct inode *inode, struct file *filp)
{
	dprintk("%s()\n", __FUNCTION__);
	return(0);
}

static int
cesadev_release(struct inode *inode, struct file *filp)
{
	dprintk("%s()\n", __FUNCTION__);
	return(0);
}


static struct file_operations cesadev_fops = {
	.owner = THIS_MODULE,
	.open = cesadev_open,
	.release = cesadev_release,
	.ioctl = cesadev_ioctl,
};

static struct miscdevice cesadev = {
	.minor = CESADEV_MINOR,
	.name = "cesa",
	.fops = &cesadev_fops,
};

static int __init
cesadev_init(void)
{
	int rc;

#ifdef CONFIG_MV_CESA_TEST
    cesaTestStart(buf_num, buf_size);
#endif

	dprintk("%s(%p)\n", __FUNCTION__, cesadev_init);
	rc = misc_register(&cesadev);
	if (rc) {
		printk(KERN_ERR "cesadev: registration of /dev/cesadev failed\n");
		return(rc);
	}
	return(0);
}

static void __exit
cesadev_exit(void)
{
	dprintk("%s()\n", __FUNCTION__);
	misc_deregister(&cesadev);
}

module_init(cesadev_init);
module_exit(cesadev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ronen Shitrit");
MODULE_DESCRIPTION("Cesadev (user interface to CESA)");
