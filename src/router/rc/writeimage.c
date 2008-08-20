#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <linux/mtd/mtd.h>

#include <sys/sysinfo.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <mtd.h>

#include <cyutils.h>
#include <cymac.h>
#include <utils.h>
#include <cy_conf.h>
#include <bcmutils.h>

#define WRITE_IMAGE "/tmp/write_image"

#define PMON_SIZE 256 * 1024
#define ROUNDUP(x, y) ((((unsigned int)(x)+((y)-1))/(y))*(y))
#define OFFSET(p, field) ((unsigned int)(&((p)->field)) - (unsigned int)(p))

extern int mtd_erase_sector (const char *mtd, unsigned long start,
			     unsigned long end);


/* Write pmon from user space, and erease first 128K of kernel
 * Must check EST header.
 * Restore lan mac to new boot.
 * The boot filename must be boot.bin
 */

#if 0
int
write_boot (const char *path, const char *mtd)
{
  int mtd_fd = -1;
  mtd_info_t mtd_info;
  erase_info_t erase_info;

  struct sysinfo info;

  struct boot_header boot;

  FILE *fp;
  char *buf = NULL;
  long count = 0, len, off, ret_count;
  int ret = -1;
  char *lan_mac;
  int i;
  int MAC_START_ADDRESS;

  printf ("Write_boot(%s,%s) doing....\n", path, mtd);

  if (check_now_boot () == PMON_BOOT)
    {
      printf ("Now boot is PMON\n");
      MAC_START_ADDRESS = PMON_MAC_START_ADDRESS;
    }
  else
    {
      printf ("Now boot is CFE\n");
      MAC_START_ADDRESS = CFE_MAC_START_ADDRESS;
    }

  FILE *fp_w;
  fp_w = fopen (WRITE_IMAGE, "w");
  fclose (fp_w);

  memset (&boot, 0, sizeof (struct boot_header));

  /* Examine EST header */
  if ((fp = fopen (path, "r")))
    count = fread (&boot, 1, sizeof (struct boot_header), fp);
  if (count < 0)
    {
      perror (path);
      ret++;
      goto fail;
    }
  printf ("Read header size = [%ld]\n", (unsigned long) count);

  if (memcmp (&boot.magic[0], &BOOT_PATTERN, 3))
    {
      printf ("Bad boot header!\n");
      ret++;
      goto fail;
    }

  /* Open MTD device and get sector size */
  if ((mtd_fd = mtd_open (mtd, O_RDWR)) < 0)
    {
      perror (mtd);
      ret++;
      goto fail;
    }
  if (ioctl (mtd_fd, MEMGETINFO, &mtd_info) != 0 ||
      mtd_info.erasesize < sizeof (struct boot_header))
    {
      perror (mtd);
      ret++;
      goto fail;
    }

  /* See if we have enough memory to store the whole file */
  sysinfo (&info);
  if ((info.freeram + info.bufferram) >= PMON_SIZE)
    erase_info.length = ROUNDUP (PMON_SIZE, mtd_info.erasesize);
  else
    erase_info.length = mtd_info.erasesize;

  /* Allocate temporary buffer */
  if (!(buf = malloc (PMON_SIZE)))
    {
      perror ("malloc");
      ret++;
      goto fail;
    }
  //memset(&buf,0,sizeof(buf));

  /* Write file or URL to MTD device */
  for (erase_info.start = 0; erase_info.start < PMON_SIZE;
       erase_info.start += count)
    {
      len = MIN (erase_info.length, PMON_SIZE - erase_info.start);
      if (erase_info.start == 0)
	{
	  count = sizeof (struct boot_header);
	  off = 0;
	  //memcpy(buf, &boot, sizeof(struct boot_header));
	}
      else
	count = off = 0;

      if (fp)
	count += fread (&buf[off], 1, len - off, fp);

      printf ("Read data size = [%ld]\n",
	      count - sizeof (struct boot_header));

      if (count < 0)
	{
	  perror (path);
	  ret++;
	  goto fail;
	}
      if (count < len)
	{
	  ret++;
	  goto fail;
	}

      /* update mac to new boot */
      lan_mac = nvram_get ("et0macaddr");
      printf ("Restore mac [%s] to new boot, the location is 0x%x\n", lan_mac,
	      MAC_START_ADDRESS);

      for (i = 0; i < 17; i++)
	buf[MAC_START_ADDRESS + i] = *(lan_mac + i);
      buf[MAC_START_ADDRESS + 17] = '\0';

      printf ("Restore eou key to new boot, the location is 0x%x\n",
	      CFE_EOU_KEY_START_ADDRESS);
      if (nvram_invmatch ("eou_device_id", ""))
	{
	  memcpy (&buf[CFE_EOU_KEY_START_ADDRESS],
		  nvram_safe_get ("eou_device_id"), 8);
	  memcpy (&buf[CFE_EOU_KEY_START_ADDRESS + 8],
		  nvram_safe_get ("eou_private_key"), 256);
	  memcpy (&buf[CFE_EOU_KEY_START_ADDRESS + 8 + 256],
		  nvram_safe_get ("eou_public_key"), 258);
	}

      /* Do it */
      printf ("Erasing %s...\n", mtd);
      (void) ioctl (mtd_fd, MEMUNLOCK, &erase_info);
      if (ioctl (mtd_fd, MEMERASE, &erase_info) != 0)
	{
	  perror (mtd);
	  ret++;
	  goto fail;
	}

      printf ("Writing data to %s ...\n", mtd);
      ret_count = write (mtd_fd, buf, count - sizeof (struct boot_header));	//skip the header size
      if (ret_count != count - sizeof (struct boot_header))
	{
	  perror (mtd);
	  ret++;
	  goto fail;
	}
    }

  if (strcmp (path, "/bin/boot.bin"))
    {				// from tftp update boot.bin
      nvram_set ("boot_ver", "");	// The boot ver set to null, so we can update old boot.
      nvram_commit ();
      printf ("Erasing first 128K of kernel\n");	// Force to update code.bin, when update boot.bin.
      /* erase first 128k of kernel */
      if (mtd_erase_sector ("linux", 0, 128 * 1024) != 0)
	{
	  ret++;
	  goto fail;
	}
    }
  else				// from code.bin update boot.bin
    ;;

  ret = 0;

  printf ("Done\n");

fail:
  if (buf)
    free (buf);
  if (mtd_fd >= 0)
    close (mtd_fd);
  if (fp)
    fclose (fp);

  unlink (WRITE_IMAGE);
  return ret;
}

/* PMON default nvram address is from 0x400 to 0x2400,
 * We use 0x2000 to store 8's mac.
 * Please confirm the address 0x2000 to 0x2400 is full 0xFF.
 * Because the user space cann't directly access flash value, 
 * so i use kernel to write mac.
 * The mac filename must be mac.bin.
 *
 * mac.bin format :
 * new format:
 * byte 0 - 5  :  mac
 * old format: (no use)
 * byte 0      :  increment count
 * byte 1 - 4  :  ip address
 * byte 5      :  start position
 * byte 6 - 11 :  start mac
 * byte 12 - 15:  no used
 * byte 16 - 21:  end mac
 */

int
write_mac (const char *path)
{
  char string[] = "string=xx:xx:xx:xx:xx:xx";
  unsigned char buf[80];
  unsigned char mac[18];
  FILE *fp;
  int count = 0;

  cprintf ("Write mac start...\n");

  if ((fp = fopen (path, "r")))
    count = fread (buf, sizeof (char), 80, fp);
  fclose (fp);

  cprintf ("%s size=[%d]\n", path, count);

  if (count < 0)
    {
      perror (path);
      return -1;
    }

  buf[0] = buf[0] & 0xFE;	//  *unmask the 0th bit (valid mac addr should leave this bit as 0)

  sprintf (mac, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2],
	   buf[3], buf[4], buf[5]);
  cprintf ("Download mac=[%s]\n", mac);

  snprintf (string, sizeof (string), "string=%s", mac);


//      write_wl_mac(mac); /* barry add for WRT54G v1.1 single board, write wl mac to EEPROM ! */

  cprintf ("Write mac done...\n");

  return 0;
}

int
write_eou_key (const char *path)
{
  char string[1024];
  char buf[1024];
  FILE *fp;
  int count = 0;
  char device_id[8 + 1];
  char private_key[256 + 1];
  char public_key[258 + 1];
  char *str = NULL;
  int ret = 0;

  cprintf ("write_eou_key(): Write eou key start...\n");

  if ((fp = fopen (path, "r")))
    {
      count = fread (buf, sizeof (char), sizeof (buf), fp);
      fclose (fp);
    }
  else
    {
      cprintf ("Cann't open %s\n", path);
      return -1;
    }


  cprintf ("%s size=[%d]\n", path, count);

  if (count < 0)
    {
      perror (path);
      return -1;
    }

  bzero (string, sizeof (string));

  if (!(str = strstr (buf, "DEVICE_ID=")))
    {
      cprintf ("Cann't find DEVICE_ID\n");
      return -1;
    }
  else
    {
      memcpy (device_id, str + strlen ("DEVICE_ID="), 8);
      device_id[8] = '\0';
      cprintf ("device_id=[%s]\n", device_id);
      printASC (device_id, 8);
    }

  if (!(str = strstr (buf, "PRIVATE_KEY=")))
    {
      cprintf ("Cann't find PRIVATE_KEY\n");
      return -1;
    }
  else
    {
      memcpy (private_key, str + strlen ("PRIVATE_KEY="), 256);
      private_key[256] = '\0';
      cprintf ("private_key =\n");
      printASC (private_key, 256);
    }

  if (!(str = strstr (buf, "PUBLIC_KEY=")))
    {
      cprintf ("Cann't find PUBLIC_KEY\n");
      return -1;
    }
  else
    {
      memcpy (public_key, str + strlen ("PUBLIC_KEY="), 258);
      public_key[258] = '\0';
      cprintf ("public_key =\n");
      printASC (public_key, 258);
    }

  snprintf (string, sizeof (string), "string=%s%s%s\0", device_id,
	    private_key, public_key);

//  eval ("insmod", "writemac", "flag=set_eou_key", string);
//  eval ("rmmod", "writemac");

  cprintf ("write_eou_key(): Write eou key done...\n");

  return 0;
}

/*
 * Erase an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @param	start	start address to erase
 * @param	end	end address to erase
 * @return	0 on success and errno on failure
 */
int
mtd_erase_sector (const char *mtd, unsigned long start, unsigned long end)
{
  int mtd_fd;
  mtd_info_t mtd_info;
  erase_info_t erase_info;

  printf ("mtd_erase_sector(%s,%ld,%ld) doing", mtd, start, end);

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

  for (erase_info.start = start;
       erase_info.start < end; erase_info.start += mtd_info.erasesize)
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
  return 0;
}
#endif
