#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pcap.h>

#include "aircrack-ptw-lib.h"

#define KEYLIMIT (1000000)
#define BSSIDLEN (6)

typedef struct {
       u_char bssid[BSSIDLEN];
       uint8_t keyindex;
       PTW_attackstate * state;
} network;

const uint8_t beginpacket[] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06,
0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02 };

const uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void printKey(uint8_t * key, int keylen) {
        int i;
        printf("Found key with len %02d: ", keylen);
        for (i = 0; i < keylen; i++) {
                printf("%02X ", key[i]);
        }
        printf("\n");
}

int main(int argc, char**argv) {
       char errbuf[PCAP_ERRBUF_SIZE];
       pcap_t * descr;
       const u_char *packet;
       struct pcap_pkthdr * hdr;
       int bssoffset;
       int dstoffset;
       network * networktable = NULL;
       int currenttable;
       int numstates = 0;
       uint8_t key[PTW_KEYHSBYTES];
       uint8_t buf[PTW_n];
       uint8_t iv[3];
       uint8_t keystream[16];
       int k,i;
       
       printf("This is aircrack-ptw 1.0.0\nFor more informations see http://www.cdc.informatik.tu-darmstadt.de/aircrack-ptw/\n");

       if (argc != 2) {
             printf("usage: aircrack-ptw <capturefile>\n");
             return -1;
       }

       descr = pcap_open_offline(argv[1], errbuf);
       if (descr == NULL) {
             printf("could not open file\n");
             return -1;
       }
       if (pcap_datalink(descr) != DLT_IEEE802_11) {
             printf("sorry, unsupported data link layer\n");
	     return -1;
       }
       while(pcap_next_ex(descr, &hdr, &packet) != -2) {
             if (((hdr->len == 68) || (hdr->len == 86) ) && (packet[0] == 0x08)) {
                  // ARP
                  // Find the right state
                  if (( packet[1] & 0x03) == 0x01) {
                      bssoffset = 4;
                      dstoffset = 10;
                  } else {
                      bssoffset = 10;
                      dstoffset = 4;
                  }
                  currenttable = -1;
                  for (k = 0; k < numstates; k++) {
                      if ((memcmp(networktable[k].bssid, &packet[bssoffset], BSSIDLEN) == 0) && (networktable[k].keyindex == packet[27])) {
                         currenttable = k;
                      }
                  }
                  if (currenttable == -1) {
                      // Allocate a new table
                      printf("allocating a new table\n");
                      printf("bssid = %02X:%02X:%02X:%02X:%02X:%02X  keyindex=%01d\n", packet[bssoffset+0], packet[bssoffset+1], packet[bssoffset+2], packet[bssoffset+3], packet[bssoffset+4], packet[bssoffset+5], packet[27]);
                      numstates++;
                      networktable = realloc(networktable, numstates * sizeof(network));
                      networktable[numstates-1].state = PTW_newattackstate();
                      if(networktable[numstates-1].state == NULL) {
                         printf("could not allocate state\n");
                         exit(-1);
                      }
                      memcpy(networktable[numstates-1].bssid, &packet[bssoffset], BSSIDLEN);
                      networktable[numstates-1].keyindex = packet[27];
                      currenttable = numstates-1;
                  }
                  // Find IV
                  for (i = 0; i < PTW_IVBYTES; i++) {
                      iv[i] = packet[24+i];
                  }
                  for (i = 0; i < PTW_KSBYTES; i++) {
                      keystream[i] = packet[28+i] ^ beginpacket[i];
                  }
                  if (memcmp(broadcast,&packet[dstoffset], 6) == 0) {
                      // it is a request
                      // printf("found request\n");
                      keystream[PTW_KSBYTES-1] ^= 0x03;
                  }
                  for (i = 0; i < PTW_KSBYTES; i++) {
                      buf[i] = keystream[i] ^ packet[28+i];
                  }
                  PTW_addsession(networktable[currenttable].state, iv, keystream);
             }
       }
       for ( k = 0; k < numstates; k++) {
                printf("stats for bssid %02X:%02X:%02X:%02X:%02X:%02X  keyindex=%01d packets=%d\n",
                        networktable[k].bssid[0],
                        networktable[k].bssid[1],
                        networktable[k].bssid[2],
                        networktable[k].bssid[3],
                        networktable[k].bssid[4],
                        networktable[k].bssid[5],
                        networktable[k].keyindex,
                        networktable[k].state->packets_collected);
             if(PTW_computeKey(networktable[k].state, key, 13, KEYLIMIT) == 1) {
                  printKey(key, 13);
             }
             if(PTW_computeKey(networktable[k].state, key, 5, KEYLIMIT/10) == 1) {
                  printKey(key, 5);
             }
       }
       return 0;
}
