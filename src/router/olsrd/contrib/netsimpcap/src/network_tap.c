/*
 * NetsimPcap - a userspace network bridge with simulated packet loss
 *             Copyright 2008 H. Rogge (rogge@fgan.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <net/if_arp.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#include "debug.h"
#include "network_tap.h"
#include "kernel_tunnel.h"

char macBroadcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * This function translates a mac address from string
 * to binary format.
 *
 * @param pointer to source string
 * @param pointer to target mac adresee
 * @return 0 if successful, 1 for an error
 */
int
readMac(char *value, MacAddress * target)
{
  char buffer[13];
  int index = 0;

  memset(buffer, 0, sizeof(buffer));

  char c;
  while ((c = *value++)) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
      if (index > 11) {
        return 1;
      }
      buffer[index++] = c;
      break;
    default:
      break;
    }
  }

  if (index < 12) {
    return 1;
  }

  for (index = 5; index >= 0; index--) {
    buffer[index * 2 + 2] = 0;

    int value;
    sscanf(&buffer[index], "%x", &value);
    target->mac[index] = (char)value;
  }
  return 0;
}

/*
 * closeTap
 *
 * This function close a tap device
 *
 * @param file destriptor of the tap device
 */
void
closeTap(int fd)
{
  close(fd);
}

/*
 * createTap
 *
 * This function creates a tap device, sets a mac address
 * and the broadcast address and activates the device
 *
 * @param pointer to device name
 * @param pointer to mac address
 * @return file descriptor of tap device, -1 if an error
 * happened
 */
int
createTap(char *name, MacAddress * mac)
{
  static const char * deviceName = OS_TUNNEL_PATH;
  int etfd;
  struct ifreq ifreq;

  int ioctlSkfd;
  int ioctlres;

  etfd = open(deviceName, O_RDWR);
  if (etfd < 0) {
    printf("Cannot open tap device!\n");
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, name, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0';  /* Ensures null termination */

  /*
   * Specify the IFF_TAP flag for Ethernet packets.
   * Specify IFF_NO_PI for not receiving extra meta packet information.
   */
  ifreq.ifr_flags = IFF_TAP;
  ifreq.ifr_flags |= IFF_NO_PI;

  if (ioctl(etfd, TUNSETIFF, (void *)&ifreq) < 0) {
    close(etfd);
    printf("Cannot set tun device type!\n");
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, name, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0';  /* Ensures null termination */
  ifreq.ifr_addr.sa_family = AF_INET;

  ioctlSkfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (ioctlSkfd < 0) {
    close(etfd);
    printf("Cannot open configuration socket!\n");
    return -1;
  }

  /* Set hardware address */
  ifreq.ifr_addr.sa_family = ARPHRD_ETHER;
  memcpy(ifreq.ifr_addr.sa_data, mac, 6);
  ioctlres = ioctl(ioctlSkfd, SIOCSIFHWADDR, &ifreq);
  if (ioctlres >= 0) {
    /* Set hardware broadcast */
    memcpy(ifreq.ifr_addr.sa_data, macBroadcast, 6);
    ioctlres = ioctl(ioctlSkfd, SIOCSIFHWBROADCAST, &ifreq);
    if (ioctlres >= 0) {
      /* Bring EtherTunTap interface up (if not already) */
      ifreq.ifr_addr.sa_family = AF_INET;
      ioctlres = ioctl(ioctlSkfd, SIOCGIFFLAGS, &ifreq);
      if (ioctlres >= 0) {
        ifreq.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_BROADCAST);
        ioctlres = ioctl(ioctlSkfd, SIOCSIFFLAGS, &ifreq);
      }
    }
  }
  if (ioctlres < 0) {
    printf("Configuration of tun device failed! (%d %s)\n", errno, strerror(errno));
    close(etfd);
    close(ioctlSkfd);
    return -1;
  }

  /* Set the multicast flag on the interface */
  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, name, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0';  /* Ensures null termination */

  ioctlres = ioctl(ioctlSkfd, SIOCGIFFLAGS, &ifreq);
  if (ioctlres >= 0) {
    ifreq.ifr_flags |= IFF_MULTICAST;
    ioctlres = ioctl(ioctlSkfd, SIOCSIFFLAGS, &ifreq);
  }

  close(ioctlSkfd);
  return etfd;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
