// LED control
// added for LED control, Jerry
#include <sys/ioctl.h>		/* ioctl() */
#include <sys/fcntl.h>		/* For O_RDRW, O_CREAT */
#include <string.h>
#include <stdio.h>
#include <bcmnvram.h>
#define INTERVAL        400000
// end
/*****************************************
*  Static Variables for Internal Use Only:
*****************************************/
static int sintIOCTLDescriptor = -1;
static unsigned char ac01IFName[64];	//[IFNAMSIZ];

/***********************************************************************
*  GPIO Access Interface(IOCTL):
***********************************************************************/
/*****************************
*  Proprietary Data Structure:
*****************************/
typedef struct _GPIO_IOC_DATA
{
  int sintRegisterID;		/*0-4 */
  int sintGPIONumber;		/*0-7, -1 for all *///-1 will destroy console, etc.
  int sintGPIOPattern;		/*See below */
} GPIO_IOC_DATA;

/********************************
*  GPIO_IOC_DATA.sintGPIOPattern:
********************************/
#define GPIO_PATTERN_OUTPUT_HZ          (-1)	/*Change to Tristate */
#define GPIO_PATTERN_OUTPUT_LO          (0)	/*Output a Low Voltage */
#define GPIO_PATTERN_OUTPUT_HI          (1)	/*Output a High Voltage */

/*****************************************
*  IOCTL Command ID: Please do not modify!
*****************************************/
#define DEVICE_MAJOR_EXPERIMENTAL       63
#define IOCTL_SET_GPIO                  _IOW(DEVICE_MAJOR_EXPERIMENTAL, 0, GPIO_IOC_DATA)


static int
serrGPIOClose (void)
/********************************************
*  Return 0: Closed Successfully.
*     -1: I/F has not been opened for access.
********************************************/
{
  if (sintIOCTLDescriptor == -1)
    {
      return (-1);
    }
  close ( /*IX int */ sintIOCTLDescriptor);
  sintIOCTLDescriptor = -1;
  return (0);
}

static int
serrGPIOOpen ( /*IP*/ unsigned char *pc01DevName)
/*******************************************************
*  Open An Interface for Get/Set Operation:
*  Return 0: Interface is opened successfully for IOCTL.
*     -1: I/F is in use! Close before reopen.
*     -2: *pc01DevName interface does not exist.
*******************************************************/
{
  int serrCode, sintFile;

  if (sintIOCTLDescriptor != -1)
    {
      return (-1);
    }
  sintFile = open (pc01DevName, O_RDWR);
  if (sintFile == -1)
    {
      return (-2);
    }

  sintIOCTLDescriptor = sintFile;
  strncpy (ac01IFName, pc01DevName, 64);	//IFNAMSIZ
  return (0);
}

static int
serrGPIOIssue ( /*IP*/ int sintCommand,
	       /*IP*/ int sintGPIONumber, /*IP*/ int sintGPIOPattern)
/*******************************************************
*  Issue an IOCTL command:
*  Return 0: Interface is opened successfully for IOCTL.
*     -1: I/F has not been opened for access.
*     -2: *pc01DevName interface does not exist.
*******************************************************/
{
  int serrCode;
  GPIO_IOC_DATA struIOCtl;

  if (sintIOCTLDescriptor == -1)
    {
      return (-1);
    }
  struIOCtl.sintRegisterID = 0;
  struIOCtl.sintGPIONumber = sintGPIONumber;
  struIOCtl.sintGPIOPattern = sintGPIOPattern;
  /*printf("\n==>serrGPIOIssue: FD=%08x, CMD=%08x",
     sintIOCTLDescriptor, sintCommand); */
  serrCode = ioctl (sintIOCTLDescriptor, sintCommand, &struIOCtl);
  if (serrCode < 0)
   /***********************************************
   *  If no wireless name : no wireless extensions:
   ***********************************************/
    {
      return (serrCode);
    }
  return (0);
}

/***********************************************************************
*  LED Control Interface:
***********************************************************************/
#define LED_ON                          GPIO_PATTERN_OUTPUT_LO
#define LED_OFF                         GPIO_PATTERN_OUTPUT_HI

/************************************************************************
*  LED Hardware layout: Note GPIO1, GPIO2, GPIO6, GPIO7 shall not be used
************************************************************************/
#define BELKIN_CONNECTED_LED            0	//GPIO0
#define BELKIN_ACTIVITY_LED             3	//GPIO3
#define BROADXENT_LINK_LED             4	//GPIO4
#define BELKIN_POWER_LED                5	//GPIO5
#define SIEMENS_LINK_LED_4702             4	//GPIO4
#define SIEMENS_LINK_LED_4712             0	//GPIO0
#define BCM4712_CPUTYPE "0x4712"

int
serrTurnOnLED ( /*IP*/ int sintLEDName)
/****************************************
*  Issue an IOCTL command to turn on the
*  given LED sintLEDName.
*  Return >=0 if successful.
*    <0: Failed.
****************************************/
{
  int serrCode;

  serrCode = serrGPIOIssue ( /*IP int */ IOCTL_SET_GPIO,
			    /*IP int */ sintLEDName,
			    /*IP int */ LED_ON);
  return (serrCode);
}

int
serrTurnOffLED ( /*IP*/ int sintLEDName)
/*****************************************
*  Issue an IOCTL command to turn off the
*  given LED sintLEDName.
*  Return >=0 if successful.
*    <0: Failed.
*****************************************/
{
  int serrCode;

  serrCode = serrGPIOIssue ( /*IP int */ IOCTL_SET_GPIO,
			    /*IP int */ sintLEDName,
			    /*IP int */ LED_OFF);
  return (serrCode);
}

int
led_ctrl (int on)
{
  pid_t pid;
  int found = 0;
  char tmp[128], ifunit;
  int wan_gpio = 0;
   /*********************************
   *  Open the device file for IOCTL:
   *********************************/
  int serrCode, rc;
  while (1)
    {
      unsigned char ac01DeviceBCM47xxGPIO[] = "/dev/bcm47xxgpio";
      serrCode = serrGPIOOpen (ac01DeviceBCM47xxGPIO);
      if (serrCode)
	{
	  printf ("\nError in opening interface [%s]: Code %d\n",
		  ac01DeviceBCM47xxGPIO, serrCode);
	  //usleep(INTERVAL/10);
	  return -1;
	}
      else
	break;
    }
  if (!strcmp (nvram_safe_get ("cpu_type"), BCM4712_CPUTYPE))
    {
      wan_gpio = SIEMENS_LINK_LED_4712;
    }
  else				// 4702
    {
      wan_gpio = SIEMENS_LINK_LED_4702;
    }

  if (on)
    {
      if ((rc = serrTurnOnLED (wan_gpio)) < 0)
	printf ("led_ctl():Can not ioctl!\n");
      printf ("led_ctl(): WAN is  connected, turn LED2(GPIO %d) on!\n",
	      wan_gpio);
    }				// turn on ACT
  else
    {
      if ((rc = serrTurnOffLED (wan_gpio)) < 0)
	printf ("led_ctl():Can not ioctl!\n");
      printf ("led_ctl(): WAN is  disconnected, turn LED2(GPIO %d) off!\n",
	      wan_gpio);
    }				// turn on ACT
  serrGPIOClose ();
  return 0;
}				//  led_ctrl



int
powerled_ctrl (int on)
{
  pid_t pid;
  int found = 0;
  char tmp[128], ifunit;
  int power_gpio = BELKIN_POWER_LED;
   /*********************************
   *  Open the device file for IOCTL:
   *********************************/
  int serrCode, rc;
  while (1)
    {
      unsigned char ac01DeviceBCM47xxGPIO[] = "/dev/bcm47xxgpio";
      serrCode = serrGPIOOpen (ac01DeviceBCM47xxGPIO);
      if (serrCode)
	{
	  printf ("\nError in opening interface [%s]: Code %d\n",
		  ac01DeviceBCM47xxGPIO, serrCode);
	  //usleep(INTERVAL/10);
	  return -1;
	}
      else
	break;
    }

  if (on)
    {
      if ((rc = serrTurnOnLED (power_gpio)) < 0)
	printf ("powerled_ctl():Can not ioctl!\n");
      printf
	("powerled_ctl(): WAN is  connected, turn power LED(GPIO %d) on!\n",
	 power_gpio);
    }				// turn on ACT
  else
    {
      if ((rc = serrTurnOffLED (power_gpio)) < 0)
	printf ("powerled_ctl():Can not ioctl!\n");
      printf
	("powerled_ctl(): WAN is  disconnected, turn power LED(GPIO %d) off!\n",
	 power_gpio);
    }				// turn on ACT
  serrGPIOClose ();
  return 0;
}				//  powerled_ctrl

int
start_powerled_ctrl_1 (void)
{
  return powerled_ctrl (1);
}
