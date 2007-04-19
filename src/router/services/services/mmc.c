#ifdef HAVE_MMC
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>

void
start_mmc (void)
{
  if (nvram_match ("mmc_enable", "1"))
    {
      int res = eval ("insmod", "mmc");
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
