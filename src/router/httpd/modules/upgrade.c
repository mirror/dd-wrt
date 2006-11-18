
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: upgrade.c,v 1.4 2005/11/30 11:53:42 seg Exp $
 */


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096
#define CODE_PATTERN_ERROR 9999
static int upgrade_ret;


void
//do_upgrade_cgi(char *url, FILE *stream)
do_upgrade_cgi (char *url, webs_t stream)	//jimmy, https, 8/6/2003
{
#ifndef ANTI_FLASH
  
  if (upgrade_ret) {
    do_ej ("Fail_u_s.asp", stream);
  } else {
    do_ej ("Success_u_s.asp", stream);
  }
  websDone (stream, 200);

  /* Reboot if successful */
  if (upgrade_ret == 0) {
  	sleep (3);
  	sys_reboot ();
  }
  
#else

  do_ej ("Fail_u_s.asp", stream);
  websDone (stream, 200);
  
#endif
}

int
//sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade (char *url, webs_t stream, int *total, int type)	//jimmy, https, 8/6/2003
{

#ifndef ANTI_FLASH
  char upload_fifo[] = "/tmp/uploadXXXXXX";
  FILE *fifo = NULL;
  char *write_argv[4];
  pid_t pid;
  char *buf = NULL;
  int count, ret = 0;
  long flags = -1;
  int size = BUFSIZ;
  int i = 0;
  {
    write_argv[0] = "write";
    write_argv[1] = upload_fifo;
    write_argv[2] = "linux";
    write_argv[3] = NULL;
  }

  if (url)
    return eval ("write", url, "linux");

  //diag_led(DIAG, START_LED);  // blink the diag led
  C_led (1);
#ifdef HAVE_HTTPS
  if (do_ssl)
    ACTION ("ACT_WEBS_UPGRADE");
  else
#endif
    ACTION ("ACT_WEB_UPGRADE");

  /* Feed write from a temporary FIFO */
  if (!mktemp (upload_fifo) ||
      mkfifo (upload_fifo, S_IRWXU) < 0 ||
      (ret = _eval (write_argv, NULL, 0, &pid)) ||
      !(fifo = fopen (upload_fifo, "w")))
    {
      if (!ret)
	ret = errno;
      goto err;
    }

  /* Set nonblock on the socket so we can timeout */
#ifdef HAVE_HTTPS
  if (!do_ssl)
    {
#endif
      if ((flags = fcntl (fileno (stream), F_GETFL)) < 0 ||
	  fcntl (fileno (stream), F_SETFL, flags | O_NONBLOCK) < 0)
	{
	  ret = errno;
	  goto err;
	}
#ifdef HAVE_HTTPS
    }
#endif

  /*
   ** The buffer must be at least as big as what the stream file is
   ** using so that it can read all the data that has been buffered
   ** in the stream file. Otherwise it would be out of sync with fn
   ** select specially at the end of the data stream in which case
   ** the select tells there is no more data available but there in
   ** fact is data buffered in the stream file's buffer. Since no
   ** one has changed the default stream file's buffer size, let's
   ** use the constant BUFSIZ until someone changes it.
   **/

  if (size < MIN_BUF_SIZE)
    size = MIN_BUF_SIZE;
  if ((buf = malloc (size)) == NULL)
    {
      ret = ENOMEM;
      goto err;
    }

  /* Pipe the rest to the FIFO */
  cprintf ("Upgrading\n");
  while (total && *total)
    {
#ifdef HAVE_HTTPS
      if (do_ssl)
	{
	  if (size > *total)
	    size = *total;
	  count = wfread (buf, 1, size, stream);
	}
      else
#endif
	{
	  if (waitfor (fileno (stream), 5) <= 0)
	    {
	      cprintf ("waitfor timeout 5 secs\n");
	      break;
	    }
	  count = safe_fread (buf, 1, size, stream);
	  if (!count && (ferror (stream) || feof (stream)))
	    break;
	}

      if (i == 0)
	{			// check code pattern, the first data must have code pattern
	  char ver[40];
	  long ver1, ver2, ver3;

	  snprintf (ver, sizeof (ver), "v%d.%d.%d", buf[11], buf[12],
		    buf[13]);
	  ver1 = convert_ver (ver);
	  ver2 = convert_ver (INTEL_FLASH_SUPPORT_VERSION_FROM);
	  ver3 = convert_ver (BCM4712_CHIP_SUPPORT_VERSION_FROM);

	  cprintf
	    ("upgrade_ver[%s] upgrade_ver[%ld] intel_ver[%ld] 4712_ver[%ld]\n",
	     ver, ver1, ver2, ver3);

	  if (memcmp (&buf[0], &CODE_PATTERN_WRT54G, 4)
	      && memcmp (&buf[0], &CODE_PATTERN_WRT54GS, 4)
	      && memcmp (&buf[0], &CODE_PATTERN_WRT300N, 4)
	      && memcmp (&buf[0], &CODE_PATTERN_WRTSL54GS, 4)
	      && memcmp (&buf[0], &CODE_PATTERN_WRT54GSV4, 4))
	    {
	      cprintf ("code pattern error!\n");
	      goto write_data;
	    }
	  if (type != 1 && check_flash ())
	    {
	      /*if (ver1 == -1 || ver2 == -1 || ver1 < ver2)
	         {
	         cprintf
	         ("The old firmware version cann't support intel flash\n");
	         cprintf
	         ("Can't downgrade to this old firmware version (%s), must be above %s(included)\n",
	         ver, INTEL_FLASH_SUPPORT_VERSION_FROM);
	         goto write_data;
	         } */
	    }

	  if (check_hw_type () == BCM4712_CHIP && ver1 < ver3)
	    {
	      cprintf
		("The old firmware version cann't support bcm4712 chipset\n");
	      cprintf
		("Cann't downgrade to this old firmware version (%s), must be above %s(included)\n",
		 ver, BCM4712_CHIP_SUPPORT_VERSION_FROM);
	      goto write_data;
	    }

	  cprintf ("code pattern correct!\n");
	  *total -= count;
	  safe_fwrite (&buf[sizeof (struct code_header)], 1,
		       count - sizeof (struct code_header), fifo);

	  i++;
	  continue;
	}
    write_data:
      *total -= count;
      safe_fwrite (buf, 1, count, fifo);
      //safe_fwrite(buf, 1, size, fifo);
      cprintf (".");
      i++;
    }
  fclose (fifo);
  fifo = NULL;

  /* Wait for write to terminate */
  waitpid (pid, &ret, 0);
  cprintf ("done\n");
#ifdef HAVE_HTTPS
  if (!do_ssl)
    {
#endif
      /* Reset nonblock on the socket */
      if (fcntl (fileno (stream), F_SETFL, flags) < 0)
	{
	  ret = errno;
	  goto err;
	}
#ifdef HAVE_HTTPS
    }
#endif

err:
  if (buf)
    free (buf);
  if (fifo)
    fclose (fifo);
  unlink (upload_fifo);

  //diag_led(DIAG, STOP_LED);
  C_led (0);
  ACTION ("ACT_IDLE");


  return ret;
#else
  return 0;
#endif
}



void
//do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
do_upgrade_post (char *url, webs_t stream, int len, char *boundary)	//jimmy, https, 8/6/2003
{
  killall("udhcpc",SIGKILL);

#ifndef ANTI_FLASH
  char buf[1024];
  int type = 0;

  upgrade_ret = EINVAL;

  /* Look for our part */
  while (len > 0) {
  	if (!wfgets (buf, MIN (len + 1, sizeof (buf)), stream))
  		return;
  	
  	len -= strlen (buf);
  	if (!strncasecmp (buf, "Content-Disposition:", 20)) {
  		if (strstr (buf, "name=\"erase\"")) {
  			while (len > 0 && strcmp (buf, "\n") && strcmp (buf, "\r\n")) {
  				if (!wfgets (buf, MIN (len + 1, sizeof (buf)), stream))
  					return;
  				
  				len -= strlen (buf);
  			}
  			if (!wfgets (buf, MIN (len + 1, sizeof (buf)), stream))
  				return;
	      len -= strlen (buf);
	      buf[1] = '\0';	// we only want the 1st digit
	      nvram_set ("sv_restore_defaults", buf);
	      nvram_commit ();
	    }
	    else if (strstr (buf, "name=\"file\""))	{		// upgrade image
	    	type = 0;
	      break;
	    }
	  }
	}

  /* Skip boundary and headers */
  while (len > 0) {
  	if (!wfgets (buf, MIN (len + 1, sizeof (buf)), stream))
  		return;
  	
  	len -= strlen (buf);
    if (!strcmp (buf, "\n") || !strcmp (buf, "\r\n"))
     	break;
    }
    
    upgrade_ret = sys_upgrade (NULL, stream, &len, type);
    
    /* Restore factory original settings if told to.
     * This will also cause a restore defaults on reboot
     * of a Sveasoft firmware.
     */
    if (nvram_match ("sv_restore_defaults", "1")) {
    	system2 ("/sbin/erase nvram");
    }
    
    /* Slurp anything remaining in the request */
    
    while (len--) {
#ifdef HAVE_HTTPS
			if (do_ssl) {
#ifdef HAVE_OPENSSL
				BIO_gets ((BIO *) stream, buf, 1);
#elif defined(HAVE_MATRIXSSL)
				matrixssl_gets (stream, buf, 1);
#endif
			}
			else {
				(void) fgetc (stream);
			}
#else
      (void) fgetc (stream);
#endif
    }
#endif
}
