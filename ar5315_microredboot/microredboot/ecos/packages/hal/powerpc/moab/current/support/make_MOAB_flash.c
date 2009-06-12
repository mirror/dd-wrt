//
// This program reads a binary image and packs it into 
// a proper format, suitable for burning into the boot
// FLASH device on the MOAB platform.
//
// CAUTION! - This program is very "hard-wired" to the
// current MOAB environment.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define BOOT_FLASH_SIZE 0x80000
unsigned char FLASH_image[BOOT_FLASH_SIZE];
unsigned char BRA_0xFFF80100[] = { 0x4B, 0xF8, 0x01, 0x02 };

int main(int argc, char *argv[])
{
    int raw_fd, packed_fd, raw_len;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <RAW-binary> <PACKED-binary>\n", argv[0]);
        exit(1);
    }

    if ((raw_fd = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Can't open RAW file '%s': %s\n", argv[1], strerror(errno));
        exit(2);
    }
    memset(FLASH_image, 0xFF, sizeof(FLASH_image));
    if ((raw_len = read(raw_fd, FLASH_image, sizeof(FLASH_image))) < 0) {
        fprintf(stderr, "Can't read RAW file '%s': %s\n", argv[1], strerror(errno));
        exit(2);
    }        
    memcpy(&FLASH_image[BOOT_FLASH_SIZE-4], BRA_0xFFF80100, sizeof(BRA_0xFFF80100));
    close(raw_fd);

    if ((packed_fd = open(argv[2], O_RDWR | O_CREAT, 0666)) < 0) {
        fprintf(stderr, "Can't create '%s': %s\n", argv[2], strerror(errno));
        exit(3);
    }
    if (write(packed_fd, FLASH_image, sizeof(FLASH_image)) != sizeof(FLASH_image)) {
        fprintf(stderr, "Can't write '%s': %s\n", argv[2], strerror(errno));
        exit(3);
    }
    close(packed_fd);
}
