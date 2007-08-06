/*
 * jffs.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_JFFS2
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>

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
