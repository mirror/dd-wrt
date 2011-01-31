 /*
 * ATM connection functions
 *
 * $Id: atm_lecs.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <atm.h>

/* Local includes */
#include "atmsap.h"
#include "atm_lecs.h"

/* Data */
#define QLEN 5

int 
atm_create_socket(unsigned char codepoint, const unsigned char *our_addr)
{ 
  struct sockaddr_atmsvc server;
  struct atm_sap atmsap;
  struct atm_blli blli;
  struct atm_qos qos;
  int fd, ret;
  int len = sizeof(server);

  fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
  if (fd <0) {
    perror("socket(PF_ATMSVC, ...)");
    return -1;
  }

  memset(&server, 0, len);
  memset(&blli, 0, sizeof(blli));
  memset(&qos, 0, sizeof(qos));
  server.sas_family = AF_ATMSVC;
  if (our_addr)
    memcpy(server.sas_addr.prv, our_addr, ATM_ESA_LEN);
  qos.aal = ATM_AAL5;
  qos.txtp.traffic_class = ATM_UBR;
  qos.txtp.max_sdu = 1516;
  qos.rxtp.traffic_class = ATM_UBR;
  qos.rxtp.max_sdu = 1516;

  blli.l3_proto = ATM_L3_TR9577;
  blli.l3.tr9577.ipi = NLPID_IEEE802_1_SNAP;
  blli.l3.tr9577.snap[0] = 0x00;
  blli.l3.tr9577.snap[1] = 0xa0;
  blli.l3.tr9577.snap[2] = 0x3e;
  blli.l3.tr9577.snap[3] = 0x00;
  blli.l3.tr9577.snap[4] = codepoint;
  
  if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0) {
    perror("setsockopt(fd, SOL_ATM, SO_ATMQOS,...)");
    close(fd);
    return -1;
  }

  memset(&atmsap, 0, sizeof(struct atm_sap));
  atmsap.blli[0] = blli;
  if (setsockopt(fd,SOL_ATM,SO_ATMSAP,&atmsap,sizeof(atmsap)) < 0) {
    perror("setsockop(SO_ATMSAP)");
    (void) close(fd);
    return -1;
  }

  ret = bind(fd, (struct sockaddr *)&server, len);
  if (ret <0) {
    perror("bind(fd, ...)");
    close(fd);
    return -1;
  }
  ret = listen(fd, QLEN);
  if (ret <0) {
    perror("listen(fd, QLEN)");
    close(fd);
    return -1;
  }
  return fd;
}


