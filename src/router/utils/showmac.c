/*
 * Obtain and display ethernet mac addresses for ixp0 and ixp1
 * The 6byte addresses are stored sequentially in the pcf8549c-2 
 * device at address 0x100.  This device is a 512byte device, thus the 
 * i2c eeprom driver shows it as having 2 banks.  We use the i2c-proc 
 * filesystem to obtain the raw data
 *  
 */
#include <stdio.h>
#include <errno.h>

int main (int argc, char** argv) {
  FILE *file;
  int mac = -1;
  int adapter_nr = 0;
  char *filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom"; /* bank2=0x100 */

  if (argc > 1) {
    int i;
    for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
         switch (argv[i][1]) {
           case 'p':
             sscanf(argv[i], "-p%d", &mac); 
             break;
           default:
             printf("Unknown argument %s\n", argv[i]);
             break;
         }
      } else {
         filename = argv[i];
      }
    }
  }

  /* open proc device
   */
  if (!(file = fopen(filename, "r"))) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
    exit(1);
  }

  /* Read data from EEPROM
   */
  unsigned char buf[16];
  fread(&buf[0],16,1,file);
  /*if ( fscanf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
              &buf[0], &buf[1], &buf[2], &buf[3], 
              &buf[4], &buf[5], &buf[6], &buf[7], 
              &buf[8], &buf[9], &buf[10], &buf[11], 
              &buf[12], &buf[13], &buf[14], &buf[15]) != 16 ) {
    //fprintf(stderr, "Error: could not read %s (%d)\n", filename, errno);
    //fclose(file);
    //exit(1);
  }*/
  if (mac == 0) {
     fprintf(stderr,"%02x:%02x:%02x:%02x:%02x:%02x\n", buf[0], buf[1], buf[2], 
        buf[3], buf[4], buf[5]);
  } else if (mac == 1) {
     fprintf(stderr,"%02x:%02x:%02x:%02x:%02x:%02x\n", buf[6], buf[7], buf[8], 
        buf[9], buf[10], buf[11]);
  } else {
     fprintf(stderr,"ixp0: %02x:%02x:%02x:%02x:%02x:%02x\n", buf[0], buf[1], buf[2], 
        buf[3], buf[4], buf[5]);
     fprintf(stderr,"ixp1: %02x:%02x:%02x:%02x:%02x:%02x\n", buf[6], buf[7], buf[8], 
        buf[9], buf[10], buf[11]);
  }
  fclose(file);
  exit(0);
}
