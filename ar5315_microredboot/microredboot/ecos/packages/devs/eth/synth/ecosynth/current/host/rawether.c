//============================================================================
//
//     rawether.c
//
//     A utility program to perform low-level ethernet operations
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv, andrew.lunn@ascom.ch
// Date:        2002/08/07
// Version:     0.01
// Description:
//
// This program is fork'ed by the ethernet.tcl script running inside
// the synthetic target auxiliary. It is responsible for performing
// low-level ethernet I/O.
//
//####DESCRIPTIONEND####
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#ifdef HAVE_LINUX_IF_TUN_H
# include <linux/if_tun.h>
#endif

// The protocol between host and target is defined by a private
// target-side header.
#include "../src/protocol.h"

// Allow debug builds. Set this flag to 0, 1 or 2
#define DEBUG 0

// ----------------------------------------------------------------------------
// Statics.

// Are we using a real ethernet device or ethertap?
static int real_ether = 0;
static int ethertap   = 0;

// The six-byte MAC address, which must be returned to eCos
static unsigned char MAC[6];

// Does the driver support multicasting?
static int multicast_supported = 0;

// The file descriptor for incoming data ethernet packets.
// Used for select() together with fd 0 corresponding to ecosynth
static int ether_fd = -1;

// Is the interface up?
static int up = 0;

// Space for incoming and outgoing packets. In the case of rx_buffer
// there are an extra four bytes at the front for the protocol header.
#define MTU 1514
static unsigned char tx_buffer[MTU];
static unsigned char rx_buffer[MTU+4];

// Indirect to get to the actual implementation functions.
static void (*tx_fn)(unsigned char*, int);
static void (*rx_fn)(void);
static void (*start_fn)(int);
static void (*stop_fn)(void);
static void (*multicast_fn)(int);


// ----------------------------------------------------------------------------
// A utility buffer for messages.
#define MSG_SIZE 256
static unsigned char msg[MSG_SIZE];

// Report an error to ecosynth during initialization. This means a
// single byte 0, followed by a string.
static void
report_error(char* msg)
{
    write(1, "0", 1);
    write(1, msg, strlen(msg));
    close(1);
    exit(0);
}

// Report success to ecosynth. This means a byte 1 followed by
// the MAC address.
static void
report_success(void)
{
    write(1, "1", 1);
    memcpy(msg, MAC, 6);
    msg[6] = multicast_supported;
    write(1, msg, 7);
}


// ----------------------------------------------------------------------------
// Real ethernet. This involves creating a SOCK_RAW socket and binding it
// to the appropriate interface. Relevant documentation can be found in
// the man pages (packet(7) and netdevice(7)).

// The device name. Needed for various ioctl()'s.
static char real_devname[IFNAMSIZ];

// The interface index.
static int real_ifindex = -1;

// Transmit a single ethernet frame. The socket should be set up so a
// simple send() operation should do the trick. Errors such as EAGAIN,
// indicating that the network device is still busy, are ignored.
// Ethernet is not a reliable communication medium.
static void
real_handle_tx(unsigned char* buffer, int size)
{
    int result;

    result = send(ether_fd, buffer, size, MSG_DONTWAIT);
    if (result < 0) {
        // It appears that one retry is worthwhile, to clear pending
        // errors or something.
        result = send(ether_fd, buffer, size, MSG_DONTWAIT);
    }
#if (DEBUG > 0)
    fprintf(stderr, "rawether dbg: tx %d bytes -> %d\n", size, result);
#endif
#if (DEBUG > 1)
    fprintf(stderr, "    %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x %x:%x\n",
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
            buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11],
            buffer[12], buffer[13]);
#endif    
}

// Receive a single ethernet frame, using the static rxbuffer. If the
// interface is not currently up discard it. Otherwise forward it on
// to ecosynth.

static void
real_handle_rx(void)
{
    int             size;
    int             result;

    size = recv(ether_fd, rx_buffer + 4, MTU, MSG_TRUNC);
    
#if (DEBUG > 0)
    fprintf(stderr, "rawether dbg: rx returned %d, errno %s (%d)\n", size, strerror(errno), errno);
#endif
    
    if (size < 0) {
        return;     // Ignore errors, just go around the main loop again.
    }
    if ((size < 14) || (size > MTU)) {
        return;     // Invalid packet size. Discard the packet.
    }

#if (DEBUG > 1)
    fprintf(stderr, "    %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x %x:%x\n",
            rx_buffer[4], rx_buffer[5], rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9],
            rx_buffer[10], rx_buffer[11], rx_buffer[12], rx_buffer[13], rx_buffer[14], rx_buffer[15],
            rx_buffer[16], rx_buffer[17]);
#endif    

    if (!up) {
        // eCos is not currently expecting packets, so discard them.
        // This may not actually be necessary because the interface
        // is only up when eCos wants it to be up.
        return;
    }

    // It looks this packet should get forwarded to eCos.
    rx_buffer[0] = SYNTH_ETH_RX;
    rx_buffer[1] = 0;
    rx_buffer[2] = size & 0x00FF;
    rx_buffer[3] = (size >> 8) & 0x00FF;
    do {
        result = write(1, rx_buffer, 4 + size);
    } while ((-1 == result) && (EINTR == errno));

    if (result != (size + 4)) {
        fprintf(stderr, "rawether(%s): failed to send ethernet packet to I/O auxiliary, exiting.\n", real_devname);
        exit(1);
    }
}

// Utility to manipulate interface flags. This involves retrieving the
// current flags, or'ing in some bits, and'ing out others, and updating.
static void
real_update_ifflags(int set_bits, int clear_bits)
{
    struct ifreq    request;
    int             flags;
    
    strncpy(request.ifr_name, real_devname, IFNAMSIZ);
    if (ioctl(ether_fd, SIOCGIFFLAGS, &request) < 0) {
        fprintf(stderr, "rawether (%s): failed to get interface flags, exiting\n", real_devname);
        exit(1);
    }
    
    flags = request.ifr_flags;

    flags |= set_bits;
    flags &= ~clear_bits;

    if (flags == request.ifr_flags) {
        // Nothing is changing.
        return;
    }

    strncpy(request.ifr_name, real_devname, IFNAMSIZ);
    request.ifr_flags = flags;
    if (ioctl(ether_fd, SIOCSIFFLAGS, &request) < 0) {
        fprintf(stderr, "rawether (%s): failed to update interface flags, exiting\n", real_devname);
        exit(1);
    }
}


// Starting an interface. This involves bringing the interface up,
// and optionally setting promiscuous mode.
// NOTE: is UP really the right thing here? There is no IP address
// for this interface. In theory this should not matter because
// we have a bound socket which should receive all packets for
// this interface.

static void
real_handle_start(int promiscuous)
{
    if (promiscuous) {
        real_update_ifflags(IFF_UP | IFF_PROMISC, 0);
    } else {
        real_update_ifflags(IFF_UP, IFF_PROMISC);
    }
    up = 1;
}

// Stopping an interface means clearing the UP flag
static void
real_handle_stop(void)
{
    up = 0;
    real_update_ifflags(0, IFF_UP | IFF_PROMISC);
}

// Enabling/disabling multicast support.
static void
real_handle_multiall(int on)
{
    struct packet_mreq req;

    req.mr_ifindex  = real_ifindex;
    req.mr_type     = PACKET_MR_ALLMULTI;
    req.mr_alen     = 0;
    if (setsockopt(ether_fd, SOL_PACKET, on ? PACKET_ADD_MEMBERSHIP : PACKET_DROP_MEMBERSHIP, (void*)&req, sizeof(req)) < 0) {
        fprintf(stderr, "rawether (%s): failed to manipulate multicast-all flag, exiting\n", real_devname);
        exit(1);
    }
}

// When the application exists make sure that the interface goes down again.

static void
real_atexit(void)
{
    if (up) {
        real_update_ifflags(0, IFF_UP | IFF_PROMISC);
    }
}

static void
real_init(char* devname)
{
    struct sockaddr_ll  addr;
    struct ifreq        request;

    tx_fn           = &real_handle_tx;
    rx_fn           = &real_handle_rx;
    start_fn        = &real_handle_start;
    stop_fn         = &real_handle_stop;
    multicast_fn    = &real_handle_multiall;
    
    if (strlen(devname) >= IFNAMSIZ) {
        snprintf(msg, MSG_SIZE, "Invalid real network device name \"%s\", too long.\n", devname);
        report_error(msg);
    }
    strcpy(real_devname, devname);

    // All ioctl() operations need a socket. We might as well create the
    // raw socket immediately and use that.
    ether_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (ether_fd < 0) {
        snprintf(msg, MSG_SIZE, "Unable to create a raw socket for accessing network device\n"
                 "    Error %s (errno %d)\n", strerror(errno), errno);
        report_error(msg);
    }
    
    strncpy(request.ifr_name, real_devname, IFNAMSIZ);
    if (ioctl(ether_fd, SIOCGIFINDEX, &request) < 0) {
        snprintf(msg, MSG_SIZE, "Device %s does not correspond to a valid interface.\n"
                 "    Error %s (errno %d)\n", real_devname, strerror(errno), errno);
        report_error(msg);
    }
    real_ifindex = request.ifr_ifindex;

    // The interface exists. Now check that it is usable.
    strncpy(request.ifr_name, real_devname, IFNAMSIZ);
    if (ioctl(ether_fd, SIOCGIFFLAGS, &request) < 0) {
        snprintf(msg, MSG_SIZE, "Failed to get current interface flags for %s\n"
                 "    Error %s (errno %d)\n", real_devname, strerror(errno), errno);
        report_error(msg);
    }

    if (request.ifr_flags & (IFF_UP | IFF_RUNNING)) {
        snprintf(msg, MSG_SIZE, "Network device %s is already up and running.\n"
                 "    Exclusive access is required\n", real_devname);
        report_error(msg);
    }
    if (request.ifr_flags & IFF_LOOPBACK) {
        report_error("Loopback devices cannot be used for synthetic target ethernet emulation.\n");
    }
    if (request.ifr_flags & IFF_POINTOPOINT) {
        report_error("Point-to-point devices cannot be used for synthetic target ethernet emulation.\n");
    }
    if (request.ifr_flags & IFF_MULTICAST) {
        multicast_supported = 1;
    }
        
    // Make sure the interface is down. There is no point in receiving packets just yet.
    real_update_ifflags(0, IFF_UP | IFF_PROMISC);
    
    // The flags look ok. Now get hold of the hardware address.
    strncpy(request.ifr_name, real_devname, IFNAMSIZ);
    if (ioctl(ether_fd, SIOCGIFHWADDR, &request) < 0) {
        snprintf(msg, MSG_SIZE, "Failed to get hardware address for %s\n"
                 "    Error %s (errno %d)\n", real_devname, strerror(errno), errno);
        report_error(msg);
    }
    if (ARPHRD_ETHER != request.ifr_hwaddr.sa_family) {
        snprintf(msg, MSG_SIZE, "Device %s is not an ethernet device.\n", real_devname);
        report_error(msg);
    }
    memcpy(MAC, request.ifr_hwaddr.sa_data, 6);

    // The device is useable. Now just bind the socket to the appropriate address.
    addr.sll_family     = AF_PACKET;
    addr.sll_protocol   = htons(ETH_P_ALL);
    addr.sll_ifindex    = real_ifindex;
    if (bind(ether_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        snprintf(msg, MSG_SIZE, "Failed to bind socket for direct hardware address to %s\n"
                 "    Error %s (errno %d)\n", real_devname, strerror(errno), errno);
        report_error(msg);
    }

    // Make sure the interface gets shut down when rawether exits.
    atexit(real_atexit);

    // And that should be it.
}

// ----------------------------------------------------------------------------
// Ethertap device.
//
// See /usr/src/linux-2.x.y/Documentation/networking/tuntap.txt for more
// information on the tun/tap driver.
//
// Basically during initialization this code opens /dev/net/tun, then
// performs a TUNSETIFF ioctl() to initialize it. This causes a
// new network device tap?? to appear. Any ethernet frames written
// by the Linux kernel to this device can be read from the
// dev/net/tun file descriptor, and ethernet frames can be written to
// the same descriptor. The net effect is a virtual ethernet segment
// with one interface managed by the Linux kernel and another
// interface (or, theoretically, several) accessible via the file
// descriptor.
//
// The Linux kernel will invent a MAC address for its interface. An
// additional one is needed for eCos. This is either invented or
// specified in the target definition file.
//
// Old Linux kernels may not have the required support. This is detected
// by an autoconf test for <linux/if_tun.h>
#ifdef HAVE_LINUX_IF_TUN_H

static void
tap_handle_tx(unsigned char* buffer, int size)
{
    int result;

    result = write(ether_fd, buffer, size);
#if (DEBUG > 0)
    fprintf(stderr, "rawether dbg: tx %d bytes -> %d\n", size, result);
#endif
#if (DEBUG > 1)
    fprintf(stderr, "    %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x %x:%x\n",
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
            buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11],
            buffer[12], buffer[13]);
#endif    
}

// Receive a single packet from the socket. It is assumed that the
// tuntap code inside the kernel will preserve packet boundaries.
//
// For now it is assumed that all incoming packets are intended for
// eCos. That may not be accurate, and additional filtering in
// software might be appropriate. In promiscuous mode all packets
// should be accepted, obviously. Otherwise all broadcasts should
// be accepted, as should all messages intended for this specific
// interface's MAC address. Multicasts should be accepted only if
// enabled.
static void
tap_handle_rx(void)
{
    int             size;
    int             result;

    // select() has succeeded so this read() should never block.
    size = read(ether_fd, rx_buffer + 4, MTU);
#if (DEBUG > 0)
    fprintf(stderr, "rawether dbg: rx returned %d, errno %s (%d)\n", size, strerror(errno), errno);
#endif
    
    if (size < 0) {
        return;     // Ignore errors, just go around the main loop again.
    }
    if ((size < 14) || (size > MTU)) {
        return;     // Invalid packet size. Discard the packet.
    }

#if (DEBUG > 1)
    fprintf(stderr, "    %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x %x:%x\n",
            rx_buffer[4], rx_buffer[5], rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9],
            rx_buffer[10], rx_buffer[11], rx_buffer[12], rx_buffer[13], rx_buffer[14], rx_buffer[15],
            rx_buffer[16], rx_buffer[17]);
#endif    

    if (!up) {
        // eCos is not currently expecting packets, so discard them.
        return;
    }

    // It looks this packet should get forwarded to eCos.
    rx_buffer[0] = SYNTH_ETH_RX;
    rx_buffer[1] = 0;
    rx_buffer[2] = size & 0x00FF;
    rx_buffer[3] = (size >> 8) & 0x00FF;
    do {
        result = write(1, rx_buffer, 4 + size);
    } while ((-1 == result) && (EINTR == errno));

    if (result != (size + 4)) {
        fprintf(stderr, "rawether(%s): failed to send ethernet packet to I/O auxiliary, exiting.\n", real_devname);
        exit(1);
    }
}

// Nothing much can be done for start or stop. Just set the flag and
// let the rx and tx code discard packets when appropriate.
//
// For now the device is implicitly promiscuous and accepts all
// multicasts. Given the nature of a tap device it is unlikely that
// any packets will arrive which are not destined here.
// FIXME: this may have to change if bridging is enabled.
static void
tap_handle_start(int promiscuous)
{
    up = 1;
}

static void
tap_handle_stop(void)
{
    up = 0;
}

static void
tap_handle_multiall(int on)
{
}

static void
tap_init(int argc, char** argv)
{
    char* devname   = NULL;
    struct ifreq    ifr;
    int persistent  = 0;
    int have_mac    = 0;

    tx_fn           = &tap_handle_tx;
    rx_fn           = &tap_handle_rx;
    start_fn        = &tap_handle_start;
    stop_fn         = &tap_handle_stop;
    multicast_fn    = &tap_handle_multiall;
    
    // Which device? By default let the system pick one, but the user
    // can override this.
    if (0 != argc) {
        devname = argv[0];
    }
    
    // Work out the MAC address. By default a random one is generated,
    // but the user can specify one to avoid a source of randomness.
    // This MAC address is not actually needed by any of the code here,
    // but should be returned to eCos.
    if (2 <= argc) {
        unsigned int mac_data[6];   // sscanf() needs unsigned ints
	int result = sscanf(argv[1], "%x:%x:%x:%x:%x:%x",
			    &(mac_data[0]), &(mac_data[1]), &(mac_data[2]),
			    &(mac_data[3]), &(mac_data[4]), &(mac_data[5]));
	if (6 != result) {
	  if (strncmp(argv[1], "persistent", 10)) {
	    snprintf(msg, MSG_SIZE, "Invalid MAC address %s\n", argv[1]);
	    report_error(msg);
	  } 
	} else {
	  MAC[0] = mac_data[0];
	  MAC[1] = mac_data[1];
	  MAC[2] = mac_data[2];
	  MAC[3] = mac_data[3];
	  MAC[4] = mac_data[4];
	  MAC[5] = mac_data[5];
	  argv += 1;
	  argc -= 1;
	  have_mac = 1;
	}
    } 
    if ( 1 != have_mac) {
      srand(time(NULL));
      MAC[0] = 0;
      MAC[1] = 0x0FF;
      MAC[2] = rand() & 0x0FF;
      MAC[3] = rand() & 0x0FF;
      MAC[4] = rand() & 0x0FF;
      MAC[5] = rand() & 0x0FF;
    }

    // Should we make the TAP device persistent. When persistence is
    // enabled, the tap device is not removed when rawether
    // exits. This makes daemons happier. They can keep running
    // between invocations of rawether.
    if (2 <= argc ) {
      persistent = !strncmp(argv[1], "persistent", 10);
    }
    
    ether_fd = open("/dev/net/tun", O_RDWR);
    if (ether_fd < 0) {
        snprintf(msg, MSG_SIZE, "Failed to open /dev/net/tun, errno %s (%d)\n", strerror(errno), errno);
        report_error(msg);
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (NULL != devname) {
        strncpy(ifr.ifr_name, devname, IFNAMSIZ - 1);
    }
    if (ioctl(ether_fd, TUNSETIFF, (void*)&ifr) < 0) {
        snprintf(msg, MSG_SIZE, "Failed to initialize /dev/net/tun, errno %s (%d)\n", strerror(errno), errno);
        report_error(msg);
    }

    // Supporting multicasts is a no-op
    multicast_supported = 1;
    
    if (persistent) {
      if (ioctl(ether_fd, TUNSETPERSIST, 1) < 0) {
	snprintf(msg, MSG_SIZE, "Failed to set persistent flag, errno %s (%d)\n",
		 strerror(errno), errno);
	report_error(msg);
      }
    }
    // All done.
}
#else
static void
tap_init(int argc, char** argv)
{
    snprintf(msg, MSG_SIZE, "Ethertap support was not available when the host-side support was built\n");
    report_error(msg);
}
#endif  // HAVE_LINUX_IF_TUN_H

// ----------------------------------------------------------------------------
// Receive a single request from ecosynth. This consists of a four-byte
// header, optionally followed by a tx packet. EOF indicates that
// ecosynth has exited, so this process should just exit immediately
// as well. Any problems should be reported to stderr, followed by
// termination.
//
// Currently rawether is single-threaded. Theoretically this could
// cause a deadlock situation where the I/O auxiliary is trying to send
// rawether a request and is blocked on the write, while rawether is trying
// to send data to the I/O auxiliary. In practice the pipes should do
// enough buffering to avoid complications, especially since rawether
// gives priority to requests from the auxiliary.

static void
handle_ecosynth_request(void)
{
    unsigned char   req[4];
    int             result;
    int             code, arg, size;

    result = read(0, req, 4);
    if (result == 0) {
        // select() succeeded but no data. EOF. So exit
        exit(0);
    }
    if (result < 0) {
        // EINTR? EAGAIN? The latter should not happen since the pipe
        // has not been put into non-blocking mode.
        if ((EINTR == errno) || (EAGAIN == errno)) {
            return;
        } else {
            fprintf(stderr, "rawether: unexpected error reading request from ecosynth\n");
            fprintf(stderr, "    %s\n", strerror(errno));
            exit(1);
        }
    }
    if (result < 4) {
        fprintf(stderr, "rawether: unexpected error reading request from ecosynth\n    Expected 4 bytes, only received %d\n", result);
        exit(1);
    }

    code = req[0];
    arg  = req[1];
    size = req[2] + (req[3] << 8);

#if (DEBUG > 1)
    fprintf(stderr, "rawether dbg: request %x from auxiliary\n", code);
#endif    

    switch(code) {
      case SYNTH_ETH_TX:
        {
            if (size < 14) {
                fprintf(stderr, "rawether: attempt to send invalid ethernet packet of only %d bytes\n"
                        "Ethernet packets should be at least 14 bytes.\n", size);
                exit(1);
            }
            if (size > MTU) {
                fprintf(stderr, "rawether: attempt to send invalid ethernet packet of %d bytes\n"
                        "Only packets of up to %d bytes are supported.\n", size, MTU);
                exit(1);
            }
            do {
                result = read(0, tx_buffer, size);
            } while ((-1 == result) && (EINTR == errno));
            if (0 == result) {
                // EOF, at an inopportune moment
                exit(0);
            }
            if (result < size) {
                fprintf(stderr, "rawether: error reading ethernet packet from I/O auxiliary\n"
                        "Expected %d bytes but only read %d\n", size, result);
                exit(1);
            }

            (*tx_fn)(tx_buffer, size);
            break;
        }
      case SYNTH_ETH_START:
        {
            (*start_fn)(arg);
            break;
        }

      case SYNTH_ETH_STOP:
        {
            (*stop_fn)();
            break;
        }

      case SYNTH_ETH_MULTIALL:
        {
            (*multicast_fn)(arg);
            break;
        }

        // SYNTH_ETH_RX and SYNTH_ETH_GETPARAMS are handled inside ethernet.tcl
        
      default:
        fprintf(stderr, "rawether: protocol violation, received unknown request %d\n", code);
        exit(1);
    }
}

// The main loop. This waits for an event either from ecosynth or from
// the underlying ethernet device, using select. Requests from
// ecosynth are handled, and take priority to prevent the connecting
// pipe from filling up and ecosynth blocking. Incoming ethernet
// frames are forwarded to ecosynth.

static void
mainloop(void)
{
    fd_set          read_set;
    struct timeval  timeout;
    int result;

    for ( ; ; ) {
        FD_ZERO(&read_set);
        FD_SET(0, &read_set);
        FD_SET(ether_fd, &read_set);
        timeout.tv_sec  = 24 * 60 * 60;
        timeout.tv_usec = 0;

        result = select(ether_fd + 1, &read_set, NULL, NULL, &timeout);
        if (result <= 0) {
            continue;
        }

        if (FD_ISSET(0, &read_set)) {
            handle_ecosynth_request();
        } else if (FD_ISSET(ether_fd, &read_set)) {
            (*rx_fn)();
        }
    }
}

// ----------------------------------------------------------------------------

int
main(int argc, char**argv)
{
    // Ignore incoming ctrl-C's. We are in the same process group as the
    // eCos application which may sensibly be ctrl-C'd, but that should
    // result in the auxiliary detecting EOF and closing the pipe to
    // this process, which in turn causes this process to exit completely.
    signal(SIGINT, SIG_IGN);
    
    if (2 > argc ) {
        report_error("Expected at least one argument, \"real\" or \"ethertap\"\n");
    }
    if (0 == strcmp("real", argv[1])) {
        real_ether = 1;
        real_init(argv[2]);
    } else if (0 == strcmp("ethertap", argv[1])) {
        ethertap = 1;
        tap_init(argc - 2, argv + 2);
    } else {
        snprintf(msg, MSG_SIZE, "Invalid argument %s, expected \"real\" or \"ethertap\"\n", argv[1]);
        report_error(msg);
    }

    // If the device-specific initialization succeeded we must be set.
    report_success();

    mainloop();
    
    return 0;
}


