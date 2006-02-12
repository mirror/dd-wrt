/***********************************************************************
*  LED control for Broadcom BCM47xx
*     Power LED : GPIO5
*     Connected LED : GPIO0
***********************************************************************/
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/pci.h>
#include <linux/devfs_fs_kernel.h> /*For devfs_xx()*/
#include <asm/uaccess.h> /* copy_from_user, etc */

/***********************************************************************
*  Proprietary Definition:
***********************************************************************/
/* Deal with CONFIG_MODVERSIONS */
#if CONFIG_MODVERSIONS==1
#define MODVERSIONS
#include <linux/modversions.h>
#endif

/*****************************
*  Proprietary Data Structure:
*****************************/
typedef struct _GPIO_IOC_DATA
{  int sintRegisterID; /*0-4*/
   int sintGPIONumber; /*0-7, -1 for all*/
   int sintGPIOPattern; /*See below*/
}  GPIO_IOC_DATA;

#define GPIO_PATTERN_OUTPUT_HZ          (-1) /*Change to Tristate*/
#define GPIO_PATTERN_OUTPUT_LO          (0) /*Output a Low Voltage*/
#define GPIO_PATTERN_OUTPUT_HI          (1) /*Output a High Voltage*/

#define DEVICE_MAJOR_EXPERIMENTAL       63
#define IOCTL_SET_GPIO                  _IOW(DEVICE_MAJOR_EXPERIMENTAL, 0, GPIO_IOC_DATA)
#define IOCTL_GET_GPIO                  _IOR(DEVICE_MAJOR_EXPERIMENTAL, 1, int)

/*******************************************
*  LED Definition:
*******************************************/
#define LED_DISABLE                     (-1)
#define LED_TURN_OFF                    0
#define LED_TURN_ON                     1
#define WLAN_LED_CONNECTED              (0x0001)
#define WLAN_LED_POWER                  (0x0020)
#define WLAN_LED_ACTIVITY               (0x0008)
#define GPIO_LED_REGISTER               0

int PowerLED= LED_TURN_OFF;
int ActLED= LED_TURN_OFF;
int ConnectedLED= LED_TURN_OFF;
int RegID= GPIO_LED_REGISTER;

/* Initialize the module */
static void *poidLEDVA= (void *)0x0000;

#include <sbextif.h>   /* The Silicon Backplan External Interface */


static unsigned int uintReadGPIO(void)
{  extifregs_t *ptruExtIF= (extifregs_t *)poidLEDVA;
   unsigned int uintValue;

   uintValue= readw(&ptruExtIF->gpioin);
   /*printk("\n==>uintReadGPIO: (%08x=>%08x) ",
      uintTarget, uintValue);*/
   return (uintValue);
}

static void voidDriveLEDs(unsigned int uintTarget,
   unsigned int uintPattern)
{  extifregs_t *ptruExtIF= (extifregs_t *)poidLEDVA;
   unsigned int uintValue;

   uintValue= readw(&ptruExtIF->gpio[RegID].out);
   uintPattern= (uintTarget & uintPattern)|(uintValue & ~uintTarget);
   /*printk("\n==>voidDriveLEDs: TGT=%08x PTN(%08x=>%08x) ",
      uintTarget, uintValue, uintPattern);*/
   writew(uintPattern, &ptruExtIF->gpio[RegID].out);
}

static void voidEnableLEDs(unsigned int uintTarget)
{  extifregs_t *ptruExtIF= (extifregs_t *)poidLEDVA;
   unsigned int uintValue;

   uintValue= readw(&ptruExtIF->gpio[RegID].outen);
   /*printk("\n==>voidEnableLEDs: Pattern=%08x EN_REG=%08x",
      uintTarget, uintValue);*/
   uintValue|= uintTarget;
   writew(uintValue, &ptruExtIF->gpio[RegID].outen);
}

static void voidDisableLEDs(unsigned int uintTarget)
{  extifregs_t *ptruExtIF= (extifregs_t *)poidLEDVA;
   unsigned int uintValue;

   uintValue= readw(&ptruExtIF->gpio[RegID].outen);
   /*printk("\n==>voidDisableLEDs: Pattern=%08x EN_REG=%08x",
      uintTarget, uintValue);*/
   uintValue&= ~uintTarget;
   writew(uintValue, &ptruExtIF->gpio[RegID].outen);
}

#if 1 //Gerald20030102, remarked.
static int sintLEDOpen(struct inode *inode, struct file *file)
{
   //printk("==>sintLEDOpen\n");
   return(0);
}

static int sintLEDClose(struct inode *inode, struct file *file)
{  //printk("==>sintLEDClose:");
	return(0);
}
#endif


static int sintLEDIoctl(/*IP*/ struct inode *ptruINode,
   /*IP*/ struct file *ptruFile,
   /*IP*/ unsigned int uintCommand,
   /*IP*/ unsigned long ulngArg)
{  unsigned int uintTarget, uintPattern;
   GPIO_IOC_DATA struIOCtlData;
   unsigned int uintRead;

   /* -lg- */
   //printk("===> sintLEDIoctl\n");
   switch (uintCommand)
   {
      case IOCTL_SET_GPIO:
         //printk("> IOCTL_SET_GPIO\n");
         /*printk("\n==>sintLEDIoctl: CMD=%08x, ARG=%08x ", uintCommand,
            (unsigned int)ulngArg);*/
         copy_from_user((char *)&struIOCtlData,
            (char *)ulngArg, /*Priprietary is always refered by pointer*/
            sizeof(struIOCtlData)); /*Copy uintCommand And uintLength*/
         /*printk("RegID=%d GPIO=%d PTN=%d ",
            struIOCtlData.sintRegisterID,
            struIOCtlData.sintGPIONumber,
            struIOCtlData.sintGPIOPattern);*/
         if (struIOCtlData.sintRegisterID<0 || struIOCtlData.sintRegisterID>4)
         {  RegID= 0;  }
         else
         {  RegID= struIOCtlData.sintRegisterID;  }
         if (struIOCtlData.sintGPIONumber>7)
         {  return(-1);  }
         if (struIOCtlData.sintGPIONumber==-1)
         {  uintTarget= (unsigned int)-1;  }
         else
         {  uintTarget= 1<<struIOCtlData.sintGPIONumber;  }
      
         if (struIOCtlData.sintGPIOPattern==GPIO_PATTERN_OUTPUT_HZ)
         {  voidDisableLEDs(/*unsigned int*/ uintTarget);  }
         else
         {  voidEnableLEDs(/*unsigned int*/ uintTarget);
            if (struIOCtlData.sintGPIOPattern==GPIO_PATTERN_OUTPUT_HI)
            {  uintPattern= (unsigned int)-1;  }
            else
            {  uintPattern= 0x00;  }
            voidDriveLEDs(/*unsigned int*/ uintTarget,
               /*unsigned int*/ uintPattern);
         }
         break;

      case IOCTL_GET_GPIO:
         //printk("> IOCTL_GET_GPIO\n");
         RegID= 0;
         uintRead= uintReadGPIO();
	 __put_user(uintRead, (int *)ulngArg);
	 break;

      default:
	 return -ENOTTY;
   }
   return(0);
}

static devfs_handle_t sintDevFSHandle= NULL;
static int sintDeviceMajorNumber= DEVICE_MAJOR_EXPERIMENTAL; //Experimental
static char ac01DeviceName[]= "bcm47xxgpio";
static struct file_operations struOperation=
{  owner: THIS_MODULE,
   ioctl: sintLEDIoctl,
   open: sintLEDOpen,
   release: sintLEDClose,
};

#define BCM_4702_GPIO_ADDRESS           0x18007000
#define BCM_4704_GPIO_ADDRESS           0x18000000

int init_module(void)
{  unsigned int uintPattern;
   int sintTargetLED= 0;
   unsigned int uintGPIO, uintSize;
   extern void *poidGetbcm947xx_sbh(void);
   extern unsigned int sb_chip(void *sbh);

   uintPattern= sb_chip(poidGetbcm947xx_sbh());
   if (uintPattern==0x4704 || uintPattern==0x4712 )
   {  uintGPIO= BCM_4704_GPIO_ADDRESS;
      uintSize= 0x0100;
   }
   else
   {  uintGPIO= BCM_4702_GPIO_ADDRESS;
      uintSize= 0x0100;
   }
   //printk("\nPlatform: %4x", uintPattern);

   poidLEDVA= ioremap(uintGPIO, uintSize); //Gerald20030716, for 4704. 0x18007000, 0x20);
   if (poidLEDVA==(void *)0)
   {  //printk("ioremap failed\n");
      return(-ENOMEM);
   }
   RegID= RegID % 5;
   /*printk("\nVirtual LEDBA= 0x%08x PowerLED=%d ConnectedLED=%d\n",
      (unsigned int)poidLEDVA, PowerLED, ConnectedLED);*/
   if (PowerLED!=-1)
   {  sintTargetLED|= WLAN_LED_POWER;  }
   else
   {  voidDisableLEDs(/*unsigned int*/ WLAN_LED_POWER);  }
   if (ConnectedLED!=-1)
   {  sintTargetLED|= WLAN_LED_CONNECTED;  }
   else
   {  voidDisableLEDs(/*unsigned int*/ WLAN_LED_CONNECTED);  }
   if (ActLED!=-1)
   {  sintTargetLED|= WLAN_LED_ACTIVITY;  }
   else
   {  voidDisableLEDs(/*unsigned int*/ WLAN_LED_ACTIVITY);  }
   if (sintTargetLED==0x00)
   {  return(0);  }

   uintPattern= 0;
   if (PowerLED==LED_TURN_OFF)
   {  uintPattern|= WLAN_LED_POWER;  } /*Turn it Off!*/
   if (ConnectedLED==LED_TURN_OFF)
   {  uintPattern|= WLAN_LED_CONNECTED;  } /*Turn it Off!*/
   if (ActLED==LED_TURN_OFF)
   {  uintPattern|= WLAN_LED_ACTIVITY;  } /*Turn it Off!*/

   voidEnableLEDs(/*unsigned int*/ sintTargetLED);
   voidDriveLEDs(/*unsigned int*/ sintTargetLED,
      /*unsigned int*/ uintPattern);

   /***************************************
   *  Gerald20030102, Adding IOCTL Support:
   ***************************************/
   {  int serrCode;

      serrCode= /*devfs_*/register_chrdev(/*IP*/ sintDeviceMajorNumber,
         /*IP*/ ac01DeviceName,
         /*IP*/ &struOperation);
      if (serrCode<0)
      {  printk("register_chrdev() Error: %d\n", serrCode);
         return(-EBUSY);
      }
      sintDevFSHandle= devfs_register(/*IP*/ NULL,
         /*IP*/ ac01DeviceName,
         /*IP*/ DEVFS_FL_DEFAULT,
         /*IP*/ sintDeviceMajorNumber,
         /*IP*/ 0,
         /*IP*/ S_IFCHR | S_IRUSR | S_IWUSR,
         /*IP*/ &struOperation,
         /*IP*/ NULL);
   }
   return(0);
}

void cleanup_module(void)
{
   //printk("===> cleanup_module\n");
   if (poidLEDVA!=0x0000)
   {  //printk("\nUnmapping 0x%08x\n", (unsigned int)poidLEDVA);
      iounmap(poidLEDVA);
      poidLEDVA= (void *)0x0000;
   }

   /***************************************
   *  Gerald20030102, Adding IOCTL Support:
   ***************************************/
   {  if (unregister_chrdev(/*IP*/ sintDeviceMajorNumber,
         /*IP*/ ac01DeviceName))
      {  printk ("unregister_chrdev failed\n");  }
      devfs_unregister(/*IP*/ sintDevFSHandle);
   }
}

MODULE_PARM(PowerLED, "i");
MODULE_PARM(ConnectedLED, "i");
MODULE_PARM(ActLED, "i");
MODULE_PARM(RegID, "i");
