#ifdef HAVE_JFFS2
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

void
stop_jffs2 (void)
{
  eval ("umount", "/jffs");
  eval ("rmmod", "jffs2");
}

void
start_jffs2 (void)
{
#ifdef HAVE_REGISTER
  char *rwpart = "mtd5";
#else
  char *rwpart = "mtd4";
#endif
  int itworked = 0;
  if (nvram_match ("sys_enable_jffs2", "1"))
    {
      if (nvram_match ("sys_clean_jffs2", "1"))
	{
	  nvram_set ("sys_clean_jffs2", "0");
	  nvram_commit ();
	  itworked = eval ("mtd", "erase", rwpart);
	  eval ("insmod", "crc32");
	  eval ("insmod", "jffs2");

#ifdef HAVE_REGISTER
	  itworked +=
	    mount ("/dev/mtdblock/5", "/jffs", "jffs2", MS_MGC_VAL, NULL);
#else
	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
#endif
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }

	}
      else
	{
	  itworked = eval ("mtd", "unlock", rwpart);
	  eval ("insmod", "crc32");
	  eval ("insmod", "jffs2");
#ifdef HAVE_REGISTER
	  itworked +=
	    mount ("/dev/mtdblock/5", "/jffs", "jffs2", MS_MGC_VAL, NULL);
#else
	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
#endif
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }

	}
    }

}
#endif


