/*
 * MTD utility functions
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: mtd.c,v 1.3 2005/11/30 11:54:21 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/mount.h>

#include <linux/mtd/mtd.h>

#include <trxhdr.h>
#include <crc.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <shutils.h>

#include <cy_conf.h>
#include <utils.h>

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif

/*
 * Open an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @param	flags	open() flags
 * @return	return value of open()
 */
int
mtd_open (const char *mtd, int flags)
{
  FILE *fp;
  char dev[PATH_MAX];
  int i;

  if ((fp = fopen ("/proc/mtd", "r")))
    {
      while (fgets (dev, sizeof (dev), fp))
	{
	  if (sscanf (dev, "mtd%d:", &i) && strstr (dev, mtd))
	    {
	      snprintf (dev, sizeof (dev), "/dev/mtd/%d", i);
	      fclose (fp);
	      return open (dev, flags);
	    }
	}
      fclose (fp);
    }

  return open (mtd, flags);
}

/*
 * Erase an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @return	0 on success and errno on failure
 */
int
mtd_erase (const char *mtd)
{
  int mtd_fd;
  mtd_info_t mtd_info;
  erase_info_t erase_info;
  /*char *et0;
     char *et1;

     et0=nvram_safe_get("et0macaddr");
     et1=nvram_safe_get("et1macaddr");
     et0=strdup(et0);
     et1=strdup(et1); */
  /* Open MTD device */
  if ((mtd_fd = mtd_open (mtd, O_RDWR)) < 0)
    {
      perror (mtd);
      return errno;
    }

  /* Get sector size */
  if (ioctl (mtd_fd, MEMGETINFO, &mtd_info) != 0)
    {
      perror (mtd);
      close (mtd_fd);
      return errno;
    }

  erase_info.length = mtd_info.erasesize;

  for (erase_info.start = 0;
       erase_info.start < mtd_info.size;
       erase_info.start += mtd_info.erasesize)
    {
      (void) ioctl (mtd_fd, MEMUNLOCK, &erase_info);
      if (ioctl (mtd_fd, MEMERASE, &erase_info) != 0)
	{
	  perror (mtd);
	  close (mtd_fd);
	  return errno;
	}
    }

  close (mtd_fd);
  /*nvram_set("et0macaddr",et0);
     nvram_set("et1macaddr",et1);
     nvram_commit();
     free(et0);
     free(et1); */

  return 0;
}

extern int http_get (const char *server, char *buf, size_t count,
		     off_t offset);

/*
 * Write a file to an MTD device
 * @param	path	file to write or a URL
 * @param	mtd	path to or partition name of MTD device 
 * @return	0 on success and errno on failure
 */
int
mtd_write (const char *path, const char *mtd)
{
  int mtd_fd = -1;
  mtd_info_t mtd_info;
  erase_info_t erase_info;

  struct sysinfo info;
  struct trx_header trx;
  unsigned long crc;

  FILE *fp;
  char *buf = NULL;
  long count, len, off;
  long sum = 0;			// for debug
  int ret = -1;


  /* Examine TRX header */
  if ((fp = fopen (path, "r")))
    count = safe_fread (&trx, 1, sizeof (struct trx_header), fp);
  else
    count = http_get (path, (char *) &trx, sizeof (struct trx_header), 0);
  if (count < sizeof (struct trx_header))
    {
      fprintf (stderr, "%s: File is too small (%ld bytes)\n", path, count);
      goto fail;
    }
#ifdef HAVE_MAGICBOX
  trx.magic = STORE32_LE (trx.magic);
  trx.len = STORE32_LE (trx.len);
  trx.crc32 = STORE32_LE (trx.crc32);
#endif
#ifdef HAVE_XSCALE
  trx.magic = STORE32_LE (trx.magic);
  trx.len = STORE32_LE (trx.len);
  trx.crc32 = STORE32_LE (trx.crc32);
#endif


  if (trx.magic != TRX_MAGIC || trx.len < sizeof (struct trx_header))
    {
      fprintf (stderr, "%s: Bad trx header\n", path);
      goto fail;
    }
  /* Open MTD device and get sector size */
  if ((mtd_fd = mtd_open (mtd, O_RDWR)) < 0 ||
      ioctl (mtd_fd, MEMGETINFO, &mtd_info) != 0 ||
      mtd_info.erasesize < sizeof (struct trx_header))
    {
      perror (mtd);
      goto fail;
    }
  if (mtd_info.size < trx.len)
    {
      fprintf (stderr, "Image too big for partition: %s\n", mtd);
      close (mtd_fd);
      return 0;
    }




  /* See if we have enough memory to store the whole file */
  sysinfo (&info);
  fprintf (stderr, "freeram=[%ld] bufferram=[%ld]\n", info.freeram,
	   info.bufferram);
  if (info.freeram >= (trx.len + 1 * 1024 * 1024))
    {
      fprintf (stderr, "The free memory is enough, writing image once.\n");
      /* Begin to write image after all image be downloaded by web upgrade.
       * In order to avoid upgrade fail if user unplug the ethernet cable during upgrading */
      //if(check_action() == ACT_WEBS_UPGRADE || check_action() == ACT_WEB_UPGRADE)
      erase_info.length = ROUNDUP (trx.len, mtd_info.erasesize);
      //else
      //      erase_info.length = mtd_info.erasesize; 
    }
  else
    {
      erase_info.length = mtd_info.erasesize;
      fprintf (stderr,
	       "The free memory is not enough, writing image per %d bytes.\n",
	       erase_info.length);
    }

  /* Allocate temporary buffer */
  if (!(buf = malloc (erase_info.length)))
    {
      perror ("malloc");
      goto fail;
    }

  /* Calculate CRC over header */
  crc = crc32 ((uint8 *) & trx.flag_version,
	       sizeof (struct trx_header) - OFFSETOF (struct trx_header,
						      flag_version),
	       CRC32_INIT_VALUE);

  if (STORE32_LE (trx.flag_version) & TRX_NO_HEADER)
    trx.len -= sizeof (struct trx_header);

  /* Write file or URL to MTD device */
  for (erase_info.start = 0; erase_info.start < trx.len;
       erase_info.start += count)
    {
      len = MIN (erase_info.length, trx.len - erase_info.start);
      if ((STORE32_LE (trx.flag_version) & TRX_NO_HEADER) || erase_info.start)
	count = off = 0;
      else
	{
	  count = off = sizeof (struct trx_header);
	  memcpy (buf, &trx, sizeof (struct trx_header));
	}
      if (fp)
	count += safe_fread (&buf[off], 1, len - off, fp);
      else
	count +=
	  http_get (path, &buf[off], len - off, erase_info.start + off);

      /* for debug */
      sum = sum + count;
      fprintf (stderr, "sum=[%ld]\n", sum);

      if (count < len)
	{
	  fprintf (stderr, "%s: Truncated file (actual %ld expect %ld)\n",
		   path, count - off, len - off);
	  goto fail;
	}
      /* Update CRC */
      crc = crc32 (&buf[off], count - off, crc);
      /* Check CRC before writing if possible */
      if (count == trx.len)
	{
	  if (crc != trx.crc32)
	    {
	      fprintf (stderr, "%s: Bad CRC\n", path);
	      goto fail;
	    }
	  else
	    {
	      fprintf (stderr, "%s: CRC OK\n", mtd);
	      fprintf (stderr,
		       "Writing image to flash, waiting a moment...\n");
	    }
	}
      /* Do it */
      (void) ioctl (mtd_fd, MEMUNLOCK, &erase_info);
      if (ioctl (mtd_fd, MEMERASE, &erase_info) != 0 ||
	  write (mtd_fd, buf, count) != count)
	{
	  perror (mtd);
	  goto fail;
	}
    }

  ret = 0;

fail:
  if (buf)
    {
      /* Dummy read to ensure chip(s) are out of lock/suspend state */
      (void) read (mtd_fd, buf, 2);
      free (buf);
    }

  if (mtd_fd >= 0)
    close (mtd_fd);
  if (fp)
    fclose (fp);
//    eval("fischecksum");
  return ret;
}


/*
 * Irving -  We need an unlock function in order to mount a r/w jffs2 partition
 * Unlock an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @return	0 on success and errno on failure
 */
int
mtd_unlock (const char *mtd)
{
  int mtd_fd;
  mtd_info_t mtd_info;
  erase_info_t lock_info;

  /* Open MTD device */
  if ((mtd_fd = mtd_open (mtd, O_RDWR)) < 0)
    {
      perror (mtd);
      return errno;
    }

  /* Get sector size */
  if (ioctl (mtd_fd, MEMGETINFO, &mtd_info) != 0)
    {
      perror (mtd);
      close (mtd_fd);
      return errno;
    }

  lock_info.start = 0;
  lock_info.length = mtd_info.size;

  if (ioctl (mtd_fd, MEMUNLOCK, &lock_info))
    {
      fprintf (stderr, "Could not unlock MTD device: %s\n", mtd);
      perror (mtd);
      close (mtd_fd);
      return errno;
    }

  close (mtd_fd);
  return 0;
}
