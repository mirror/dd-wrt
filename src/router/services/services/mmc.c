#ifdef HAVE_MMC
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

void
start_mmc (void)
{
  if (nvram_match ("mmc_enable", "1"))
    {
      int res;
      int mmc_di = 0, mmc_do = 0, mmc_clk = 0, mmc_cs = 0;
      char dddi[16], dddo[16], ddclk[16], ddcs[16];
      
      if (nvram_match ("mmc_gpio", "1"))  //manual gpio asigments
      	{
     	  mmc_di = strtoul (nvram_safe_get ("mmc_di"), NULL, 0);
     	  mmc_do = strtoul (nvram_safe_get ("mmc_do"), NULL, 0);
     	  mmc_clk = strtoul (nvram_safe_get ("mmc_clk"), NULL, 0);
     	  mmc_cs = strtoul (nvram_safe_get ("mmc_cs"), NULL, 0);
 		}
 	  else  //auto gpio based on router brand/model
 	  	{
        switch (getRouterBrand ())
        	{
        	case ROUTER_WRT54G:
        		if (nvram_match ("boardtype", "0x0467")) //v4 or GL
        		  mmc_di = 2;
        		else	
        		  mmc_di = 5;
				mmc_do = 4;
				mmc_clk = 3;
				mmc_cs = 7;
               break;
        	case ROUTER_ASUS_WL500GD:
				mmc_di = 5;
				mmc_do = 4;
				mmc_clk = 1;
				mmc_cs = 7;
           	   break;
			case ROUTER_BUFFALO_WHRG54S:
				mmc_di = 5;
				mmc_do = 6;
				mmc_clk = 3;
				mmc_cs = 7;
           	   break;
			}
		}
        
     	  sprintf (dddi, "DDDI=0x%X", 1 << mmc_di);
     	  sprintf (dddo, "DDDO=0x%X", 1 << mmc_do);
     	  sprintf (ddclk, "DDCLK=0x%X", 1 << mmc_clk);     	  
     	  sprintf (ddcs, "DDCS0=0x%X", 1 << mmc_cs);
     	 
     	 if ((mmc_di + mmc_do + mmc_clk + mmc_cs) > 5)  //eval only if at least 0, 1, 2, 3
     	 	{     	  
			 syslog (LOG_DEBUG, "MMC: starting, %s, %s, %s, %s\n", dddi, dddo, ddclk, ddcs);
	     	 res = eval("insmod","mmc", dddi, dddo, ddclk, ddcs);  //eval("insmod","mmc", "DDDI=0x04", "DDDO=0x10", "DDCLK=0x08", "DDCS=0x80");
     	    }
 
     	     
/*      res = eval ("insmod", "mmc"); */     
      
/*    if (res)
	{
	eval("rmmod","mmc_wrt1");
        res = eval("insmod","mmc_wrt2");
	}
    if (res)
        {
	eval("rmmod","mmc_wrt2");
        res = eval("insmod","mmc_buf1");
	}
    if (res)
        {
	eval("rmmod","mmc_buf1");
        res = eval("insmod","mmc_buf2");
	}
    if (res)
        {
	eval("rmmod","mmc_buf2");
	}    */

      if (!res)
	{
	  //device detected
	  eval ("insmod", "ext2");
	  if (mount
	      ("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL, NULL))
	    {
	      //device not formated
	      eval ("/sbin/mke2fs", "-F", "-b", "1024",
		    "/dev/mmc/disc0/part1");
	      mount ("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL,
		     NULL);
	    }
	}
    }
}
#endif


        /*	For Asus: SD_CLK=1, SD_DO=4, SD_DI=5, SD_CS=7

        GPIO PIN 0 0x01
        GPIO PIN 1: 0x02
        GPIO PIN 2: 0x04
        GPIO PIN 3: 0x08
        GPIO PIN 4: 0x10
        GPIO PIN 5: 0x20
        GPIO PIN 6: 0x40
        GPIO PIN 7: 0x80
        GPIO PIN 8: 0x100
        GPIO PIN 9: 0x200
        GPIO PIN 10: 0x400
        GPIO PIN 11: 0x800
        GPIO PIN 12: 0x1000
        GPIO PIN 13: 0x2000
        GPIO PIN 14: 0x4000
        GPIO PIN 15: 0x8000
        */