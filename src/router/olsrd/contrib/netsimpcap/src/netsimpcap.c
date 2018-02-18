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
#include <pcap.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
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
#include "network_tap.h"
#include "olsr_random.h"

int debugMode = 0;

int running;

pcap_t *devices[128];
int deviceFD[128];
int deviceCount;

int tapFD;

u_int32_t *connBC;
u_int32_t *connUni;

u_int8_t buffer[65536];
u_int8_t mac_bc[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * signalHandler
 *
 * This handler stops the mainloop when the programm is send a signal
 * to stop
 */
static void
signalHandler(int signo __attribute__ ((unused)))
{
  /*
   * Normally errno must be saved here and restored before returning but since
   * we do a simple assignment we don't need to do that in this signal handler.
   */
  running = 0;
}

/*
 * calculateConnections
 *
 * @param pointer to user defined connection matrix
 * @param number of elements in a matrix line
 * @param 1 if matrix contains drop-propabilities
 */
void
calculateConnections(float *connMatrix, int socketcount, int drop)
{

  /* calculating unicast/broadcast matrix */
  connUni = malloc(sizeof(u_int32_t) * socketcount * socketcount);
  memset(connUni, 0, sizeof(u_int32_t) * socketcount * socketcount);
  connBC = malloc(sizeof(u_int32_t) * socketcount * socketcount);
  memset(connBC, 0, sizeof(u_int32_t) * socketcount * socketcount);

  float broadcast, unicast;
  int i, j;
  for (j = 0; j < socketcount; j++) {
    for (i = 0; i < socketcount; i++) {
      float prop = connMatrix[GRID(i, j, socketcount)];
      if (drop) {
        prop = 1.0 - prop;
      }
      broadcast = prop;

      /* IEEE 802.11 do (normally) up to 4 retransmissions for unicast.
       * A unicast package is only lost if all of this 5 transmissions
       * are lost.
       */
      prop = 1 - prop;
      unicast = (1 - prop) * (1 + prop + prop * prop + prop * prop * prop + prop * prop * prop * prop);

      connBC[GRID(i, j, socketcount)] = (1 << 24) * broadcast;
      connUni[GRID(i, j, socketcount)] = (1 << 24) * unicast;
    }
  }

  if (debugMode) {
    printf("Connection matrix for unicast:\n");
    for (j = 0; j < socketcount; j++) {
      for (i = 0; i < socketcount; i++) {
        if (i > 0) {
          printf(" ");
        }

        printf("%1.2f", (float)connBC[GRID(i, j, socketcount)] / (float)(1 << 24));
      }
      printf("\n");
    }
    printf("\nConnectionmatrix for broadcast:\n");
    for (j = 0; j < socketcount; j++) {
      for (i = 0; i < socketcount; i++) {
        if (i > 0) {
          printf(" ");
        }

        printf("%1.2f", (float)connUni[GRID(i, j, socketcount)] / (float)(1 << 24));
      }
      printf("\n");
    }
  }
}

/*
 * tap_callback
 *
 * This function is called every times a package is received through the
 * (optional) tap device. It retransmits the package to all connected devices.
 */
void
tap_callback(void)
{
  int len, i;

  len = read(tapFD, buffer, sizeof(buffer));
  if (len > 0) {
    for (i = 0; i < deviceCount; i++) {
      pcap_inject(devices[i], buffer, len);
    }
  }
}

/*
 * capture_callback
 *
 * This function is called for every package received through libpcap on
 * one of the connected devices. It retransmit the package to the other
 * devices (loss propability is defined in connection matrix) and always
 * to the tap device (if active)
 *
 * See libpcap documentation for parameters
 */
void
capture_callback(u_char * args, const struct pcap_pkthdr *hdr, const u_char * packet)
{
  int *index = (int *)args;
  int unicast = memcmp(packet, mac_bc, 6) != 0;

  int i, len;

  len = hdr->len;
  memcpy(buffer, packet, len);

  if (tapFD != -1) {
    int send = 0;
    while (send < len) {
      int t = write(tapFD, &buffer[send], len - send);
      if (t == -1) {
        printf("Error while sending to tap device!\n");
        break;
      }
      send += t;
    }
  }

  for (i = 0; i < deviceCount; i++) {
    u_int32_t prop;
    if (unicast) {
      prop = connUni[GRID(*index, i, deviceCount)];
    } else {
      prop = connBC[GRID(*index, i, deviceCount)];
    }

    if (prop == 0 || prop < (olsr_random() % (1 << 24))) {
      continue;
    }

    pcap_inject(devices[i], buffer, len);
  }
}

int
main(int argc, char **argv)
{
  int i;
  int dropPropability = 0;
  char *connectionFile = NULL;
  int deviceIndex = -1;
  int hubMode = 0;

  MacAddress mac;
  char *tapname = NULL;

  if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    printf("%s: [-con <connectionfile>] [-hub] [-debug]" " [-tap <devname> <mac>] [-drop] -dev <dev1> <dev2> <dev3>...\n", argv[0]);
    return 0;
  }

  deviceCount = 0;
  tapFD = -1;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-con") == 0 && i < argc - 1) {
      connectionFile = argv[i + 1];
      i++;
    }
    if (strcmp(argv[i], "-drop") == 0) {
      dropPropability = 1;
    }
    if (strcmp(argv[i], "-dev") == 0) {
      deviceIndex = ++i;
      while (i < argc && argv[i][0] != '-') {
        i++;
        deviceCount++;
      }
      i--;                      /* to cancel the i++ in the for loop */
    }
    if (strcmp(argv[i], "-hub") == 0) {
      hubMode = 1;
    }
    if (strcmp(argv[i], "-tap") == 0 && i < argc - 2) {
      tapname = argv[++i];
      readMac(argv[++i], &mac);
    }
    if (strcmp(argv[i], "-debug") == 0) {
      debugMode = 1;
    }
  }

  if (hubMode == 1 && connectionFile != NULL) {
    printf("Error, you cannot set matrix file in hub mode.\n");
    return 1;
  }

  if (connectionFile == NULL && hubMode == 0) {
    printf("Error, netsim needs a matrix file for connections if not running in hub mode.\n");
    return 1;
  }

  if (deviceIndex < 0) {
    printf("Error, you must specify the devices the programm connects to.\n");
    return 1;
  }

  if (deviceCount < 2) {
    printf("Error, you need to bind at least two devices to the bridge.\n");
    return 1;
  }

  if (tapname) {
    tapFD = createTap(tapname, &mac);
    if (tapFD == -1) {
      printf("Error, cannot open tap device '%s'\n", tapname);
      return 1;
    }

  }
  running = 1;

  float *connMatrix = malloc(sizeof(float) * deviceCount * deviceCount);
  if (!connMatrix) {
    printf("Error, not enough memory for mac buffer!");
    if (tapFD != -1)
      closeTap(tapFD);
    return 1;
  }

  if (hubMode) {
    int x, y;

    /*
     * In hub mode the any-to-any loss factor is 1.0, which
     * means we can skip reading a matrix file.
     */
    for (y = 0; y < deviceCount; y++) {
      for (x = 0; x < deviceCount; x++) {
        if (x != y) {
          connMatrix[GRID(x, y, deviceCount)] = 1.0;
        }
      }
    }
  } else {
    if (readConnectionMatrix(connMatrix, connectionFile, deviceCount)) {
      printf("Error while reading matrix file\n");
      free(connMatrix);
      if (tapFD != -1)
        closeTap(tapFD);
      return 1;
    }
  }
  calculateConnections(connMatrix, deviceCount, dropPropability);
  free(connMatrix);

  char errbuf[PCAP_ERRBUF_SIZE];
  int maxDeviceFD = 0;

  if (tapFD != -1) {
    maxDeviceFD = tapFD;
  }
  for (i = 0; i < deviceCount; i++) {
    devices[i] = pcap_open_live(argv[i + deviceIndex], BUFSIZ, 0, -1, errbuf);
    deviceFD[i] = -1;
    if (devices[i] == NULL) {
      printf("Error, cannot open pcap for device '%s'.\n", argv[i + deviceIndex]);
      running = 0;
    } else {
      deviceFD[i] = pcap_fileno(devices[i]);
      if (deviceFD[i] > maxDeviceFD) {
        maxDeviceFD = deviceFD[i];
      }
    }
  }

  /* activate signal handling */
  signal(SIGABRT, &signalHandler);
  signal(SIGTERM, &signalHandler);
  signal(SIGQUIT, &signalHandler);
  signal(SIGINT, &signalHandler);

  while (running) {
    fd_set socketSet;
    int socketsReady;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    FD_ZERO(&socketSet);
    for (i = 0; i < deviceCount; i++) {
      FD_SET(deviceFD[i], &socketSet);
    }
    if (tapFD != -1) {
      FD_SET(tapFD, &socketSet);
    }

    socketsReady = select(maxDeviceFD + 1, &socketSet, (fd_set *) 0, (fd_set *) 0, &timeout);
    if (socketsReady <= 0) {
      break;
    }

    for (i = 0; i < deviceCount; i++) {
      if (FD_ISSET(deviceFD[i], &socketSet)) {
        int error = pcap_dispatch(devices[i], -1, capture_callback,
                                  (u_char *) & i);

        if (error == -1) {
          printf("Error during pcap_dispatch for device %s\n", argv[i + deviceIndex]);
          running = 0;
          break;
        }
      }
    }
    if (tapFD != -1 && FD_ISSET(tapFD, &socketSet)) {
      tap_callback();
    }
  }

  for (i = 0; i < deviceCount; i++) {
    if (devices[i] != NULL) {
      pcap_close(devices[i]);
    }
  }
  free(connUni);
  free(connBC);

  if (tapFD != -1)
    closeTap(tapFD);
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
