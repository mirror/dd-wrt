/*
 * addrs_dlpi.c:
 *
 * Provides the get_addrs_dlpi() function for use on systems that require
 * the use of the System V STREAMS DataLink Programming Interface for
 * acquiring low-level ethernet information about interfaces.
 *
 * Like Solaris.
 *
 */

#include "config.h"

#ifdef HAVE_DLPI

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/dlpi.h>
#include <net/if.h>

#include "dlcommon.h"

extern char *split_dname(char *device, int *unitp);
extern char *strncpy2(char *dest, char *src, int n);
extern char *strncat2(char *dest, char *src, int n);

/*
 * This function identifies the IP address and ethernet address for the interface
 * specified
 *
 * This function returns -1 on catastrophic failure, or a bitwise OR of the
 * following values:
 * XXX: change this to perfom "best effort" identification of addresses.
 * Failure to find an address - for whatever reason - isn't fatal, just a
 * nuisance.
 *
 * 1 - Was able to get the ethernet address
 * 2 - Was able to get the IP address
 *
 * This function should return 3 if all information was found
 */

int
get_addrs_dlpi(char *interface, char if_hw_addr[], struct in_addr *if_ip_addr)
{
  int got_hw_addr = 0;
  int got_ip_addr = 0;

  int fd;
  long buf[MAXDLBUF];		/* long aligned */
  union DL_primitives *dlp;

  char *cp;
  int unit_num = 0;
  int sap = 0;

  char *devname = NULL;
  char *devname2 = NULL;
  char fulldevpath[256];

  struct ifreq ifr = {};

  /* -- */

  memset(if_hw_addr, 0, 6);

  // we want to be able to process either a fully qualified /dev/ge0
  // type interface definition, or just ge0.

  if (strncmp(interface, "/dev/", strlen("/dev/")) == 0) {
    devname = interface + strlen("/dev/");
  } else {
    devname = interface;
  }

  strncpy2(fulldevpath, "/dev/", sizeof(fulldevpath)-1);
  cp = strncat2(fulldevpath, interface, sizeof(fulldevpath));

  if (strlen(cp) != 0) {
    fprintf(stderr, "device name buffer overflow %s\n", fulldevpath);
    return -1;
  }

  fprintf(stderr,"interface: %s\n", devname);

  // on Solaris, even though we are wanting to talk to ethernet device
  // ge0, we have to open /dev/ge, then bind to unit 0.  Dupe our
  // full path, then identify and cut off the unit number

  devname2 = strdup(fulldevpath);

  cp = split_dname(devname2, &unit_num);

  if (cp == NULL) {
    free(devname2);
    goto get_ip_address;
  } else {
    *cp = '\0';			/* null terminate devname2 right before numeric extension */
  }

  // devname2 should now be something akin to /dev/ge.  Try to open
  // it, and if it fails, fall back to the full /dev/ge0.

  if ((fd = open(devname2, O_RDWR)) < 0) {
    if (errno != ENOENT) {
      fprintf(stderr, "Couldn't open %s\n", devname2);
      free(devname2);
      goto get_ip_address;
    } else {
      if ((fd = open(fulldevpath, O_RDWR)) < 0) {
	fprintf(stderr, "Couldn't open %s\n", fulldevpath);
	free(devname2);
	goto get_ip_address;
      }
    }
  }

  free(devname2);
  devname2 = NULL;

  /* Use the dlcommon functions to get access to the DLPI information for this
   * interface.  All of these functions exit() out on failure
   */

  dlp = (union DL_primitives*) buf;

  /*
   * DLPI attach to our low-level device
   */

  dlattachreq(fd, unit_num);
  dlokack(fd, buf);

  /*
   * DLPI bind
   */

  dlbindreq(fd, sap, 0, DL_CLDLS, 0, 0);
  dlbindack(fd, buf);

  /*
   * DLPI DL_INFO_REQ
   */

  dlinforeq(fd);
  dlinfoack(fd, buf);

  /* 
     printdlprim(dlp);  // uncomment this to dump out info from DLPI
  */

  if (dlp->info_ack.dl_addr_length + dlp->info_ack.dl_sap_length == 6) {
    memcpy(if_hw_addr, 
	   OFFADDR(dlp, dlp->info_ack.dl_addr_offset),
	   dlp->info_ack.dl_addr_length);
    got_hw_addr = 1;
  } else {
    fprintf(stderr, "Error, bad length for hardware interface %s -- %d\n", 
	    interface,
	    dlp->info_ack.dl_addr_length);
  }

  close(fd);

 get_ip_address:

  /* Get the IP address of the interface */

#ifdef SIOCGIFADDR

  fd = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */

  strncpy(ifr.ifr_name, interface, IFNAMSIZ);

  (*(struct sockaddr_in *) &ifr.ifr_addr).sin_family = AF_INET;

  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    fprintf(stderr, "Error getting IP address for interface: %s\n", "ge0"); 
    perror("ioctl(SIOCGIFADDR)");
  } else {
    memcpy(if_ip_addr, &((*(struct sockaddr_in *) &ifr.ifr_addr).sin_addr), sizeof(struct in_addr));
    got_ip_addr = 2;
  }
#else
  fprintf(stderr, "Cannot obtain IP address on this platform\n");
#endif

  close(fd);
  
  return got_hw_addr + got_ip_addr;
}

/*
 * Split a device name into a device type name and a unit number;
 * return the a pointer to the beginning of the unit number, which
 * is the end of the device type name, and set "*unitp" to the unit
 * number.
 *
 * Returns NULL on error, and fills "ebuf" with an error message.
 */
char *
split_dname(char *device, int *unitp)
{
  char *cp;
  char *eos;
  int unit;

  /* -- */

  /*
   * Look for a number at the end of the device name string.
   */

  cp = device + strlen(device) - 1;
  if (*cp < '0' || *cp > '9') {
    fprintf(stderr, "%s missing unit number", device);
    return (NULL);
  }
  
  /* Digits at end of string are unit number */
  while (cp-1 >= device && *(cp-1) >= '0' && *(cp-1) <= '9')
    cp--;
  
  unit = (int) strtol(cp, &eos, 10);
  if (*eos != '\0') {
    fprintf(stderr, "%s bad unit number", device);
    return (NULL);
  }
  *unitp = unit;
  return (cp);
}

/*------------------------------------------------------------------------------
                                                                      strncpy2()

strncpy2() is like strncpy(), except that strncpy2() will always
insure that the <dest> buffer is null terminated.  strncpy() will not
NULL terminate the destination buffer if the <src> string is <n>
characters long or longer, not counting the terminating NULL character.

      STRNCPY2() IS NOT A COMPATIBLE REPLACEMENT FOR STRNCPY()!!

There are two reasons to use strncpy2(). 

The first reason is to guarantee that <dest> buffer's bounds are not
violated.  In this case, <n> should be the size of the <dest> buffer
minus one.

i.e.,

char tempstring[MAXLINE];

strncpy2(tempstring, my_own_string, MAXLINE - 1);

The second reason is to copy a specific number of characters from
<src> to <dest>.  In this case, <n> should be the number of characters
you want to transfer, not including the terminating NULL character.

The following example copies "abc" into tempstring, and NULL
terminates it.

char tempstring[MAXLINE];

strncpy2(tempstring, "abcdef123", 3);

strncpy2() returns a pointer to the first character in <src> that was
not copied to <dest>.  If all of <src> was copied to <dest>,
strncpy2() will return a pointer to the NULL character terminating the
<src> string.

------------------------------------------------------------------------------*/
char *
strncpy2(char *dest, char *src, int n)
{
  int
    i = 0;

  char
    *char_ptr;

  /* -- */

  if ((!dest) || (!src))
    return(src);

  char_ptr = dest;

  while ((i++ < n) && *src)
    *char_ptr++ = *src++;

  *char_ptr = '\0';

  return(src);
}

/*------------------------------------------------------------------------------
                                                                      strncat2()

Similar to strncat except that <n> is the size of the <dest> buffer
(INCLUDING SPACE FOR THE TRAILING NULL CHAR), NOT the number of
characters to add to the buffer.

      STRNCAT2() IS NOT A COMPATIBLE REPLACEMENT FOR STRNCAT()!

strncat2() always guarantees that the <dest> will be null terminated, and that
the buffer limits will be honored.  strncat2() will not write even one
byte beyond the end of the <dest> buffer.

strncat2() concatenates up to <n-1> - strlen(<dest>) characters from
<src> to <dest>.

So if the <dest> buffer has a size of 20 bytes (including trailing NULL),
and <dest> contains a 19 character string, nothing will be done to
<dest>.

If the string in <dest> is longer than <n-1> characters upon entry to
strncat2(), <dest> will be truncated after the <n-1>th character.

strncat2() returns a pointer to the first character in the src buffer that
was not copied into dest.. so if strncat2() returns a non-zero character,
string truncation occurred in the concat operation.

------------------------------------------------------------------------------*/
char *
strncat2(char *dest, char *src, int n)
{
  int
    i = 0;

  char
    *dest_ptr,
    *src_ptr;

  /* -- */

  if (!dest || !src)
    return NULL;

  dest_ptr = dest;
  src_ptr = src;

  /* i = 0 */

  while ((i < (n-1)) && *dest_ptr)
    {
      i++;
      dest_ptr++;
    }

  /* i is the number of characters in dest before the concatenation
     operation.. a number between 0 and n-1 */

  while ((i++ < (n-1)) && *src_ptr)
    *dest_ptr++ = *src_ptr++;

  /* i is the number of characters in dest after the concatenation
     operation, or n if the concat operation got truncated.. a number
     between 0 and n 

     We need to check src_ptr here because i will be equal to n if
     <dest> was full before the concatenation operation started (which
     effectively causes instant truncation even if the <src> string is
     empty..

     We could just test src_ptr here, but that would report
     a string truncation if <src> was empty, which we don't
     necessarily want. */

  if ((i == n) && *src_ptr)
    {
      // we could log truncation here
    }

  *dest_ptr = '\0';

  /* should point to a non-empty substring only if the concatenation
     operation got truncated.

     If src_ptr points to an empty string, the operation always
     succeeded, either due to an empty <src> or because of
     sufficient room in <dest>. */
     
  return(src_ptr);
}

#endif /* HAVE_DLPI */
