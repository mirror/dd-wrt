#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <typedefs.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <bcmnvram.h>

static unsigned char 	buf[WLC_IOCTL_MAXLEN];

char *wlmac = NULL;

static int
dump_srom(void)
{
	srom_rw_t       *s1;
	int ret, i, j;
	unsigned short *words = (unsigned short *)&buf[8];

	s1 = (srom_rw_t *)buf;

        s1->byteoff = 0;
        s1->nbytes = 128;

	ret = wl_ioctl(wlmac, WLC_GET_SROM,buf, sizeof(buf));

	if(ret != 0)
		return -1;

	printf("Srom contents:\n");
	for(i=0 ; i<8 ; i++) {
		printf("  srom[%03d]:  ", (i*8) );
		for(j=0;j<8;j++)
			printf("0x%04x  ", words[(i*8)+j]);
		printf("\n");
	}
	printf("SPROM version:\n");
	printf("SubSystem Vendor ID: 0x%04X\n", words[3]);
	printf("SubSystem Device ID: 0x%04X\n", words[2]);

	return 0;
}

static int
write_srom(char *data)
{
	int offset;
	char tmp[10];
	int ret;
	unsigned short value;
	unsigned char aa[254];
	unsigned char bb[2];

	srom_rw_t       *s1;

	ret = sscanf(data, "%d.%s", &offset, tmp);

	if(ret != 2) {
		printf("Invalid parameter! (%d)\n", ret);
		return -1;
	}

	value = strtoul(tmp, NULL, 16);

	printf("Write 0x%04X to offset %03d\n", value, offset);

	s1 = (srom_rw_t *)aa;

	bb[0] = value & 0x00ff;
	bb[1] = value >> 8 ; 

	memcpy(s1->buf, bb, 2);

        s1->byteoff = offset*2;
        s1->nbytes = 2;

	ret = wl_ioctl(wlmac, WLC_SET_SROM, aa, sizeof(aa));
		
	return 0;
}

static void
usage(char *cmd) 
{	
	cprintf("Usage: %s [OPTIONS]\n"
		  "\t-d\n"
		  "\t\tDump srom\n"
		  "\t-w wordoffset.value\n"
		  "\t\tWrite srom\n"
		  "\n\tExample:\n"
		  "\n\t%s -w 2.0x0058	(Device ID)"
		  "\n\t%s -w 3.0x1737	(Vendor ID)"
		  "\n\t%s -w 36.0x0090	(MAC)"
		  "\n\t%s -w 37.0x4c99	(MAC)"
		  "\n\t%s -w 38.0x0001	(MAC)\n"
		  , cmd, cmd, cmd, cmd, cmd, cmd);
}

int 
main(int argc, char * argv[])
{
	int c;

	wlmac = nvram_safe_get("wl0_ifname");

	if(!strcmp(wlmac, "")) {
		printf("Cann't find any wireless card!\n");
		return -1;
	}

	while ((c = getopt(argc, argv, "dw:h")) != -1)
                switch (c) {
			case 'd':
				dump_srom();
				exit(0);
				break;
			case 'w':
				write_srom(optarg);
				exit(0);
				break;
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}

	usage(argv[0]);

	return 0;
}

