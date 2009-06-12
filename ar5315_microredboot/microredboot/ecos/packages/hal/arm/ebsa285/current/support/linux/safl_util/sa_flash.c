
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FLASH_SZ (4 * 1024 * 1024)
#define FLASH_BLOCK_SZ (256 * 1024)

volatile void *flash_base;
int  driver_fd;

/* 
 * sync PCI transactions
 */
static void
pci_sync(void)
{
    volatile unsigned int x;

    x = *(unsigned int *)flash_base;
}

static void
flash_write_mode(void)
{
    pci_sync();
    *(volatile int *)flash_base = 0x40404040;
}


static void
flash_normal_mode(void)
{
    pci_sync();
    *(volatile int *)flash_base = 0xffffffff;
}


/*
 *  Check to see if there is some flash at the location specified.
 */
static int
flash_verify(void) 
{
    volatile unsigned int *fp = (volatile unsigned int *)flash_base; 
    unsigned int mfg, id ;

    flash_normal_mode();

    /* read the manufacturer's id. */
    pci_sync();
    *fp = 0x90909090;
    mfg = *fp;

    if (mfg != 0x89898989) {
	flash_normal_mode();
	return 0;
    } 

    id = *(fp + 1) ;

    flash_normal_mode();

    if (id < 0xA1A1A1A1)
	return 0;

    return 1;
}


static unsigned int
flash_read_dword(int offset)
{
    /* swap initial 32 byte blocks when accessing flash from PCI */
    if (offset < 32)
	offset += 32;
    else if (offset < 64)
	offset -= 32;

    offset &= ~3;  /* dword alignment */

    return *(volatile unsigned int *)(flash_base + offset);
}


static int
flash_write_dword(int offset, unsigned int data) 
{ 
    volatile unsigned int *fp; 
    int status;

    /* swap initial 32 byte blocks when accessing flash from PCI */
    if (offset < 32)
	offset += 32;
    else if (offset < 64)
	offset -= 32;

    offset &= ~3;  /* dword alignment */

    fp = (volatile unsigned int *)(flash_base + offset) ;

    flash_write_mode();
  
    /* write the data */
    *fp = data;

    /* wait till done */
    do {
	pci_sync();
	*fp = 0x70707070;
	status = *fp;
    } while ((status & 0x80808080) != 0x80808080);
  
    *fp = 0x50505050; /* Clear status register */
    flash_normal_mode();

    if ( (status & 0x02020202) != 0) {
        fprintf(stderr,"WRITE LOCKED %08x :", status);
        return 0;
    }
    if ( (status & 0x10101010) != 0) {
        fprintf(stderr,"WRITE FAILURE %08x :", status);
        return 0;
    }
    return 1;
}


static int
flash_erase_block(int block) 
{
    volatile unsigned int *fp;
    int status;

    fp = (volatile unsigned int *)(flash_base + (block * FLASH_BLOCK_SZ));

    /* write delete block command followed by confirm */
    pci_sync();
    *fp = 0x20202020;
    pci_sync();
    *fp = 0xd0d0d0d0;

    /* wait till done */
    do {
	pci_sync();
	*fp = 0x70707070;
	status = *fp;
    } while ((status & 0x80808080) != 0x80808080);
  
    *fp = 0x50505050; /* Clear status register */
    flash_normal_mode();

    if ( (status & 0x02020202) != 0) {
        fprintf(stderr,"ERASE LOCKED %08x :", status);
        return 0;
    }
    if ( (status & 0x20202020) != 0) {
        fprintf(stderr,"ERASE FAILURE %08x :", status);
        return 0;
    }

    return 1;
}


int
main(int argc, char *argv[])
{
    int in_fd = STDIN_FILENO, i=0, got, offset, extra;
    int buf[256];
    int fw = 1, fv = 1, fr = 0, verbose = 0;
    char *name = NULL;
    int block = 0;

    if ( argc > 2 ) {
        if ( '-' == argv[1][0] && 'b' == argv[1][1] && 0 == argv[1][3] ) {
            char c = argv[1][2];
            if (      '0' <= c && c <= '9') block = c - '0';
            else if ( 'a' <= c && c <= 'f') block = c - 'a' + 10;
            else if ( 'A' <= c && c <= 'F') block = c - 'A' + 10;
            else argc = 1; /* get usage message below */
            argv++, argc--;
        }
    }

    switch (argc) {
    case 1:
	in_fd = STDIN_FILENO;
        fv = 0; /* Cannot rewind stdin, so do not verify */
	break;
    case 2:
        if ( '-' == argv[1][0] ) {
            if ( 'r' != argv[1][1] || 0 != argv[1][2]) goto usage;
            fr = 1;
            fw = fv = 0;
            break;
        }
        name = argv[1];
        in_fd = open(argv[1], O_RDONLY);
	if (in_fd < 0) {
	    fprintf(stderr, "Can't open %s", argv[1]);
	    perror(": ");
	    exit(1);
	}
	break;
    case 3:
        if ( '-' != argv[1][0] || 0 != argv[1][2]) goto usage;
        if (      'v' == argv[1][1] )              fw = 0;
        else if ( 'V' == argv[1][1] )              fw = 0, verbose = 1;
        else if ( 'w' == argv[1][1] )              fv = 0;
        else                                       goto usage;

        name = argv[2];
	in_fd = open(argv[2], O_RDONLY);
	if (in_fd < 0) {
	    fprintf(stderr, "Can't open %s", argv[2]);
	    perror(": ");
	    exit(1);
	}
	break;
    default:
    usage:
	fprintf(stderr, "Usage:          sa_flash [filename]\n");
	fprintf(stderr, "Block number:   sa_flash -bN [filename]\n");
	fprintf(stderr, "Write only:     sa_flash [-bN] -w    filename\n");
	fprintf(stderr, "Verify only:    sa_flash [-bN] -v|-V filename\n");
	fprintf(stderr, "Read to stdout: sa_flash [-bN] -r\n");
	exit(1);
    }

    driver_fd = open("/dev/safl", O_RDWR);
    if (driver_fd < 0) {
	perror("Can't open device: ");
	exit (1);
    }

    flash_base = mmap(NULL, FLASH_SZ, PROT_READ|PROT_WRITE,
		      MAP_SHARED, driver_fd, 0);

    if (flash_base == NULL) {
	perror("mmap failed: ");
	close(driver_fd);
	return 0;
    }

    if (!flash_verify()) {
	fprintf(stderr, "Couldn't find flash.\n");
	exit(1);
    }
    
    if ( fw ) {
        if ( ! flash_erase_block(block) ) {
            fprintf(stderr,"Erase error block %x\n", block);
            exit(1);
        }

        extra = 0;
        offset = block * FLASH_BLOCK_SZ;
        while ((got = read(in_fd, ((char *)buf) + extra, sizeof(buf) - extra)) > 0) {
            got += extra;

            extra = got & 3;
            got /= 4;
            for (i = 0; i < got; ++i, offset += 4)
                if ( ! flash_write_dword(offset, buf[i]) )
                    fprintf(stderr,"Write error offset %06x\n",offset);

            if (extra)
                buf[0] = buf[i];

            printf("*"); fflush(stdout);
        }
        if (extra)
            if ( ! flash_write_dword(offset, buf[i]) )
                fprintf(stderr,"Write error offset %06x\n",offset);
        printf("\n");
    }

    flash_normal_mode();

    if ( fv ) {
        int badwords = 0;
        int skipping = 0;
        close( in_fd );
	in_fd = open(name, O_RDONLY);
	if (in_fd < 0) {
	    fprintf(stderr, "Can't re-open %s", argv[2]);
	    perror(": ");
	    exit(1);
	}

        extra = 0;
        offset = block * FLASH_BLOCK_SZ;
        while ((got = read(in_fd, ((char *)buf) + extra, sizeof(buf) - extra)) > 0) {
            got += extra;

            extra = got & 3;
            got /= 4;
            for (i = 0; i < got; ++i, offset += 4) {
                int data = flash_read_dword(offset);
                if ( data != buf[i] ) {
                    badwords++;
                    if ( !skipping ) {
                        fprintf(stderr, "Bad data at offset %06x: %08x read %08x wanted\n",
                                offset, data, buf[i] );
                        if ( !verbose && badwords > 15 ) {
                            skipping = 1;
                            fprintf(stderr, "(Too many errors, skipping...)\n");
                        }
                    }
                }
            }
            if (extra)
                buf[0] = buf[i];

            printf("+"); fflush(stdout);
        }
        if (extra) {
            int data = flash_read_dword(offset);
            if ( data != buf[0] ) {
                fprintf(stderr, "End data at offset %06x: %08x read %08x wanted\n",
                        offset, data, buf[0] );
            }
        }
        printf("\n");
        if ( badwords )
            fprintf(stderr, "Bad data: %d bad words out of %d (end offset %06x)\n",
                    badwords, offset/4, offset );
    }

    flash_normal_mode();

    if ( fr ) {
        for ( offset = block * FLASH_BLOCK_SZ;
              offset < (block+1) * FLASH_BLOCK_SZ;
              offset += 4 ) {
            for ( i = 0; i < (sizeof(buf)/sizeof(int)); ++i, offset += 4 ) {
                buf[i] = flash_read_dword(offset);
            }
            if ( sizeof(buf) != write( STDOUT_FILENO, buf, sizeof(buf) ) ) {
                perror("Stdout write failed: ");
                exit(1);
            }
            fprintf(stderr,"r");
            fflush(stderr);
        }
        fprintf(stderr,"\n");
    }

    munmap((void *)flash_base, FLASH_SZ);
    close(driver_fd);
    return 0;
}




/*
 * Local variables:
 *  compile-command: "cc -g -O2 -Wall sa_flash.c -o sa_flash"
 * End:
 */
