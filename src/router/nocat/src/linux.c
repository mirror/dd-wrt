# include <glib.h>
# include <unistd.h>
# include <stdio.h>
# include <errno.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/sendfile.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <net/if.h>
# include <netinet/in.h>

# include "http.h"
# include "firewall.h"
# include "util.h"

ssize_t http_sendfile ( http_request *h, int in_fd ) {
    int out_fd = g_io_channel_unix_get_fd( h->sock );
    struct stat s;
    ssize_t r;
    off_t len = 0;

    g_assert( in_fd  > 0 );
    g_assert( out_fd > 0 );

    r = fstat( in_fd, &s );
    if (r == -1) {
	g_warning("http_sendfile stat: %m");
	return -1;
    }

    r = sendfile( out_fd, in_fd, &len, s.st_size );
    if (r == -1) {
	g_warning("http_sendfile send: %m");
	return -1;
    }

    return r;
}

gchar *peer_arp( peer *p ) {
    gchar ip[16], hw[18];
    FILE *arp;

    g_assert( p != NULL );

    arp = fopen( "/proc/net/arp", "r" );
    if ( arp == NULL )
	g_error( "Can't open /proc/net/arp: %m" );
   
    fscanf(arp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s"); // Skip first line 
    while (fscanf( arp, "%15s %*s %*s %17s %*s %*s\n", ip, hw ) != EOF)  {
	if ( strncmp( p->ip, ip, sizeof(p->ip) ) == 0 ) {
	    g_strncpy( p->hw, hw, sizeof(p->hw) );
	    break;
	}
    }

    fclose( arp );
    return ( p->hw[0] != '\0' ? p->hw : NULL );
}

gchar *get_mac_address (const gchar *dev) {
    int r, s;
    struct ifreq ifr;
    gchar *dest = g_new0( gchar, 13 );
    gchar *hwaddr;
    
    strcpy( ifr.ifr_name, dev );

    s = socket( PF_INET, SOCK_DGRAM, 0 );
    if (s == -1) {
	g_warning("get_mac_address socket: %m");
	g_free(dest);
	return NULL;
    }

    r = ioctl( s, SIOCGIFHWADDR, &ifr );
    if (r == -1) {
	g_warning("SIOCGIFHWADDR: %m");
	g_free(dest);
	close(s);
	return NULL;
    }

    hwaddr = ifr.ifr_hwaddr.sa_data;
    g_snprintf( dest, 13, "%02X%02X%02X%02X%02X%02X", 
	hwaddr[0] & 0xFF,
	hwaddr[1] & 0xFF,
	hwaddr[2] & 0xFF,
	hwaddr[3] & 0xFF,
	hwaddr[4] & 0xFF,
	hwaddr[5] & 0xFF);
	
    close(s);
    return dest;
}


gchar *get_network_address (const gchar *dev) {
    int r, s;
    struct ifreq ifaddr, ifnetmask;
    gchar *dest = g_new0( gchar, 33 );
    gchar *addr, *mask;

    s = socket( PF_INET, SOCK_DGRAM, 0 );
    if (s == -1) {
	g_warning("get_network_address socket: %m");
	g_free(dest);
	return NULL;
    }

    strncpy( ifaddr.ifr_name, dev, IFNAMSIZ );
    r = ioctl( s, SIOCGIFADDR, &ifaddr );
    if (r == -1) {
	g_warning("SIOCGIFADDR: %m");
	g_free(dest);
	close(s);
	return NULL;
    }

    strncpy( ifnetmask.ifr_name, dev, IFNAMSIZ );
    r = ioctl( s, SIOCGIFNETMASK, &ifnetmask );
    if (r == -1) {
	g_warning("SIOCGIFNETMASK: %m");
	g_free(dest);
	close(s);
	return NULL;
    }
  
    addr = (char *) &(((struct sockaddr_in *)&ifaddr.ifr_addr)->sin_addr);
    mask = (char *) &(((struct sockaddr_in *)&ifnetmask.ifr_netmask)->sin_addr);
    
    g_snprintf( dest, 32, "%d.%d.%d.%d/%d.%d.%d.%d",
		addr[0] & mask[0] & 0xFF,
		addr[1] & mask[1] & 0xFF,
		addr[2] & mask[2] & 0xFF,
		addr[3] & mask[3] & 0xFF,
		mask[0] & 0xFF,
		mask[1] & 0xFF,
		mask[2] & 0xFF,
		mask[3] & 0xFF ); 
	
    close(s);
    return dest;
}

gchar *detect_network_device ( const gchar *exclude ) {
    gchar dev[7], dest[9];
    gchar *out = NULL;
    FILE *route;

    route = fopen( "/proc/net/route", "r" );
    if ( route == NULL )
	g_error( "Can't open /proc/net/route: %m" );
    
    // Skip first line 
    fscanf(route, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s"); 

    while (fscanf( route, 
	"%6s %8s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n", dev, dest ) != EOF) {
	if ( exclude ? 
		(strncmp( dev, exclude, 6 ) != 0) :   // not the whatever it is
		(strcmp( dest, "00000000" ) == 0) ) { // default route
	    out = g_strdup(dev);
	    break;
	}
    }

    fclose( route );
    return out;
}
