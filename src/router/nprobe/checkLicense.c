/*
 *  Copyright (C) 2005 Luca Deri <deri@ntop.org>
 *
 *  		       http://www.ntop.org/
 *
 */

#include <rpc/des_crypt.h> 
#include <stdio.h> 
#include <errno.h> 
#include <sys/types.h> 

#define INCLUDE_MD5
#include "checksum.c"


/* ******************************** */

u_char* bin2hex(u_char* binary, int binary_len, u_char *buffer) {
  int i = 0, len = 0;

  buffer[0] = '\0';

  for(i=0; i<binary_len; i++) {
    u_char tmpBuf[4];

    sprintf(tmpBuf, "%02X", binary[i] & 0xFF);
    buffer[len++] = tmpBuf[0];
    buffer[len++] = tmpBuf[1];
  }

  buffer[len] = 0;

  return(buffer);
}

/* ******************************** */

void help() {
  printf("checkLicense <interface> <license key [112 chars len]>\n");
  printf("\nExample:\n");
  printf("checkLicense eth0 E0D77EA94C3044821ADDC6DFD83E18C01ADDC6DFD83E18C01ADDC6DFD83E18C01ADDC6DFD83E18C00671B05CCF9D64E6EE6525D51DD5F272\n");
}

/* ******************************** */

int main(int argc, char* argv[]) {
  struct license_options license;
  u_char txt[256], seed[256], macAddr[32];
  u_char key[256], md5_res[64];
  u_char buf[2 * 256];
  int len, rc, i, fd;
  struct ifreq ifr;
  char *data;

  if(argc != 3) {
    help();
    return(0);
  }
    
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
  (void)strncpy(ifr.ifr_name, argv[1], sizeof(ifr.ifr_name));

  if(ioctl(fd, SIOCGIFHWADDR, (char *)&ifr) < 0) {
    printf("Unable to get interface address (errno=%d)\n", errno);
    return(1);
  }
  close(fd);

  data = (u_char*)ifr.ifr_ifru.ifru_hwaddr.sa_data;

  sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
	  data[0] & 0xFF, data[1] & 0xFF, data[2] & 0xFF, data[3] & 0xFF, data[4] & 0xFF, data[5] & 0xFF);

  if(strlen(argv[2]) != 112) {
    printf("ERROR: key is not 112 chars long [len=%d]\n", strlen(argv[2])); 
    return(0);
  }

  strMD5binary(argv[2], 80, md5_res);
  if(strncmp(md5_res, &argv[2][80], 32) != 0) {
    printf("ERROR: the key is invalid\n");
    return(0);
  }

  memset(seed, 0, sizeof(seed));
  i = fillSeed(seed);

  strcat(&seed[i], argv[1]);
  
  passwd2des(seed, key);

  errno = 0;
  len = 40;
  hex2bin(argv[2], 2*len, buf);
  rc = ecb_crypt(key, buf, len, DES_DECRYPT | DES_SW); 

  if(rc == 0) {
    char str[32];

    memcpy(&license, buf, sizeof(license));

    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", 
	    license.macAddress[0], license.macAddress[1], license.macAddress[2], 
	    license.macAddress[3], license.macAddress[4], license.macAddress[5]);

    if(strcmp(str, macAddr) != 0) {
      printf("ERROR: invalid licensee [mismatching mac][%s][%s]\n", str, macAddr);
    } else {
      printf("OK [");
      dump_license(&license);
      printf("]\n");
    }
  } else
    printf("ERROR: decoding error\n");

  return(0);
}
