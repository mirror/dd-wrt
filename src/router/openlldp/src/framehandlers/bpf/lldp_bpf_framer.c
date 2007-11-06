#ifdef BPF_FRAMER
#include <sys/types.h>
#include "lldp_bpf_framer.h"
#include "lldp_port.h"
#include "lldp_debug.h"

#include "bpflib.h"
#include <fcntl.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdint.h>
#include <stdbool.h>

// BPF includes:
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <sys/socket.h>
#include <net/if.h>
// End BPF includes

#include <netinet/in.h>
#include <net/if_dl.h>

#define XENOSOCK -1;

#define TRUE 1
#define FALSE 0

int lldp_read(struct lldp_port *lldp_port) {
    /* This is a really dirty hack to chop off the first 18 bytes of the received frame. */

    lldp_port->rx.recvsize = read(lldp_port->socket, lldp_port->rx.frame, lldp_port->mtu);

    char *frame_buffer = malloc(lldp_port->mtu);

    if(frame_buffer) {
        debug_printf(DEBUG_INT, "(%s) Raw BPF Frame: \n", lldp_port->if_name);

        debug_hex_dump(DEBUG_INT, lldp_port->rx.frame, lldp_port->rx.recvsize);

        // Copy the frame buffer, minus the first 18 bytes.
        memcpy(frame_buffer, &lldp_port->rx.frame[18], lldp_port->rx.recvsize - 18);

        // Now free the old buffer
        free(lldp_port->rx.frame);

        // Now assign the new buffer
        lldp_port->rx.frame = frame_buffer;
    } else {
        debug_printf(DEBUG_NORMAL, "Couldn't malloc!  Skipping frame to prevent leak...\n");
    }

    return(lldp_port->rx.recvsize);
}

int socketInitializeLLDP(struct lldp_port *lldp_port) {
    if(lldp_port->if_name == NULL) {
        debug_printf(DEBUG_NORMAL, "Got NULL interface in %s():%d\n", __FUNCTION__, __LINE__);
        exit(-1);
    }

    if(!lldp_port->if_index > 0) {
        debug_printf(DEBUG_NORMAL, "'%s' does not appear to be a valid interface name!\n", lldp_port->if_name);
        return XENOSOCK;
    }

    debug_printf(DEBUG_INT, "'%s' is index %d\n", lldp_port->if_name, lldp_port->if_index);

    lldp_port->socket = bpf_new();

    if(lldp_port->socket < 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        return XENOSOCK;
    }


    // This is necessary for Mac OS X at least... 
    if(bpf_set_immediate(lldp_port->socket, 1) > 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }

    struct bpf_insn progcodes[] = {
        BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 12), // inspect ethernet_frame_type
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0x88CC, 0, 1), // if EAPOL frame, continue with next instruction, else jump
        BPF_STMT(BPF_RET+BPF_K, (u_int)-1),
        BPF_STMT(BPF_RET+BPF_K, 0)
    };

    struct bpf_program prog = {
        4,
        (struct bpf_insn*) &progcodes
    };

    if(ioctl(lldp_port->socket, BIOCSETF, (u_int)&prog) < 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }

    if(bpf_setif(lldp_port->socket, lldp_port->if_name) < 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }

    // Do we even need this? - Don't do it on Apple platforms
#ifndef __APPLE__
    if(bpf_set_promiscuous(lldp_port->socket) > 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }
#endif // __APPLE__

    // Set the socket to be non-blocking
    if (fcntl(lldp_port->socket, F_SETFL, O_NONBLOCK) > 0)
    {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;      
    }

    // Get the size of the BPF buffer
    if (bpf_get_blen(lldp_port->socket, &lldp_port->mtu) < 0) {
        debug_printf(DEBUG_NORMAL, "[Error] (%d) : %s (%s:%d)\n", errno, strerror(errno), __FUNCTION__, __LINE__);
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }

    // Turn on bpf echo cancellation
    bpf_see_sent(lldp_port->socket, 1);

    _getmac(lldp_port->source_mac, lldp_port->if_name);

    _getip(lldp_port->source_ipaddr, lldp_port->if_name);

    debug_printf(DEBUG_NORMAL, "MTU is: %d\n", lldp_port->mtu);

    if(lldp_port->mtu > 0) {
        lldp_port->rx.frame = malloc(lldp_port->mtu);
        lldp_port->tx.frame = malloc(lldp_port->mtu);
    } else {
        debug_printf(DEBUG_NORMAL, "Frame buffer MTU is 0, ditching interface.\n");
        bpf_dispose(lldp_port->socket);
        return XENOSOCK;
    }

    return 0;
}

/***********************************************
 * Get the MAC address of an interface
 ***********************************************/
static int _getmac(char *dest, char *ifname) {

    struct ifaddrs *ifap;

    memset(dest, 0x0, 6);

    if (getifaddrs(&ifap) == 0) {
        struct ifaddrs *p;
        for (p = ifap; p; p = p->ifa_next) {
            if (p->ifa_addr->sa_family == AF_LINK && strcmp(p->ifa_name, ifname) == 0) {
                struct sockaddr_dl* sdp = (struct sockaddr_dl*) p->ifa_addr;		
                memcpy(dest, sdp->sdl_data + sdp->sdl_nlen, 6);
                printf("I think I saw a MAC address: %s: %02X:%02X:%02X:%02X:%02X:%02X\n",
                        p->ifa_name,
                        dest[0],
                        dest[1],
                        dest[2],
                        dest[3],
                        dest[4],
                        dest[5]);
                freeifaddrs(ifap);
                return TRUE;
            }
        }
        freeifaddrs(ifap);
    }

    return FALSE;
}

/***********************************************
 * Get the IP address of an interface
 ***********************************************/
static int _getip(char *dest, char *ifname) {

    struct ifaddrs *ifap;

    if (getifaddrs(&ifap) == 0) {
        struct ifaddrs *p;
        for (p = ifap; p; p = p->ifa_next) {
            if (p->ifa_addr->sa_family == AF_INET && strcmp(p->ifa_name, ifname) == 0) {
                memcpy(dest, &p->ifa_addr->sa_data[2], 4);
                printf("I think I saw an IP address: %s: %x:%x:%x:%x\n",
                        p->ifa_name,
                        dest[0],
                        dest[1],
                        dest[2],
                        dest[3]);
                freeifaddrs(ifap);
                return TRUE;
            }
        }
        freeifaddrs(ifap);
    }

    return FALSE;
}

// Dunno if there's a better way to handle this... maybe an OS specific event?
void refreshInterfaceData(struct lldp_port *lldp_port) {
  _getmac(lldp_port->source_mac, lldp_port->if_name);
  
  _getip(lldp_port->source_ipaddr, lldp_port->if_name);  
}

void socketCleanupLLDP(struct lldp_port *lldp_port)
{
    bpf_dispose(lldp_port->socket);
}

#endif /* BPF_FRAMER */
