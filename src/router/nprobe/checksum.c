#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "md5.h"
#include "md5.c"


/******************************** Local Data **********************************/

/* #define NPROBE_SEED     "Welcome to nProbe" */
#define MD5_HASH_SIZE   16

/*********************************** Code *************************************/

void strMD5binary(unsigned char *buf, int length, unsigned char *result) {
  const char *hex = "0123456789ABCDEF";
  char	     *r;
  int        i;

  /*
   *	Take the MD5 hash of the string argument.
   */
  md5_state_t state;
  md5_byte_t digest[16];

  md5_init(&state);
  md5_append(&state, (const md5_byte_t *)buf, length);
  md5_finish(&state, digest);


  /*
   *	Prepare the resulting hash string
   */
  for (i = 0, r = result; i < 16; i++) {
    *r++ = hex[digest[i] >> 4];
    *r++ = hex[digest[i] & 0xF];
  }

  /*
   *	Zero terminate the hash string
   */
  *r = '\0';
}

/* ******************************************** */

static char result[(MD5_HASH_SIZE * 2) + 1];

/*
  Return codes:
  0         : checksum OK
  otherwise : checksum error
*/

int checkMac(char *deviceName, char *checksum) {
  int fd;
  struct ifreq ifr;
  u_char *data;
  char macAddr[64];
  char  seed[20];
  int   i=0;

  seed[i++] = 'W';
  seed[i++] = 'e';
  seed[i++] = 'l';
  seed[i++] = 'c';
  seed[i++] = 'o';
  seed[i++] = 'm';
  seed[i++] = 'e';
  seed[i++] = ' ';
  seed[i++] = 't';
  seed[i++] = 'o';
  seed[i++] = ' ';
  seed[i++] = 'n';
  seed[i++] = 'P';
  seed[i++] = 'r';
  seed[i++] = 'o';
  seed[i++] = 'b';
  seed[i++] = 'e';
  seed[i++] = 0;

  if(checksum == NULL) return(1); /* Invalid checksum */

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    printf("Unable to get interface address\n");
    return(1);
  }
  memset(&ifr, 0, sizeof(ifr));
#ifdef linux
  /* XXX Work around Linux kernel bug */
  ifr.ifr_addr.sa_family = AF_INET;
#endif
  (void)strncpy(ifr.ifr_name, deviceName, sizeof(ifr.ifr_name));

  if(ioctl(fd, SIOCGIFHWADDR, (char *)&ifr) < 0) {
    printf("Unable to get interface address (errno=%d)\n", errno);
    return(1);
  }
  close(fd);

  data = (u_char*)ifr.ifr_ifru.ifru_hwaddr.sa_data;

  sprintf(macAddr, "%s%02X:%02X:%02X:%02X:%02X:%02X",
	  seed,
	  data[0], data[1], data[2], data[3], data[4], data[5]);

  strMD5binary(macAddr, strlen(macAddr), result);

#ifdef DEBUG
  printf("Mac=%s Hash=%s\n", &macAddr[strlen(NPROBE_SEED)], result);
#endif

  return(strcasecmp(result, checksum));
}

