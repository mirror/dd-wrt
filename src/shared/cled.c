
#include <typedefs.h>
//#include <bcm4710.h>
#include <sbextif.h>
#include <bcmnvram.h>

#define KSEG1ADDR(_a) ((unsigned long)(_a) | 0xA0000000)    
#define LED_DIAG        0x13       

#define BCM4710_EXTIF           0x1f000000      /* External Interface base address */
#define BCM4710_EUART           (BCM4710_EXTIF + 0x00800000)

int Cled(int led)
{
	        if(led==1)
                 *(volatile char *)(KSEG1ADDR(BCM4710_EUART)+LED_DIAG)=0xFF;
	        else
                 *(volatile char *)(KSEG1ADDR(BCM4710_EUART)+LED_DIAG);
}
