# include <stdio.h>
# include <string.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <net/if.h>

int main (int argc, char **argv) {
    int r, s;
    struct ifreq ifr;
    
    strcpy( ifr.ifr_name, argc > 1 ? argv[1] : "eth0" );

    s = socket( PF_INET, SOCK_DGRAM, 0 );
    if (s == -1) {
	perror("socket");
	return -1;
    }

    r = ioctl( s, SIOCGIFHWADDR, &ifr );
    if (r == -1) {
	perror("SIOCGIFHWADDR");
	return -1;
    }

    printf( "%02x:%02x:%02x:%02x:%02x:%02x\n",
	    ifr.ifr_hwaddr.sa_data[0] & 0xFF,
	    ifr.ifr_hwaddr.sa_data[1] & 0xFF,
	    ifr.ifr_hwaddr.sa_data[2] & 0xFF,
	    ifr.ifr_hwaddr.sa_data[3] & 0xFF,
	    ifr.ifr_hwaddr.sa_data[4] & 0xFF,
	    ifr.ifr_hwaddr.sa_data[5] & 0xFF);
	
    close(s);
    return 0;
}
