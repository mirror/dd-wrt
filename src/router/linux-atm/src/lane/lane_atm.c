 /*
 * ATM connection functions
 *
 * $Id: lane_atm.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
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

/* Local includes */
#include "lane_atm.h"
#include "atmsap.h"
#include "load.h"
#include "dump.h"
#include "connect.h"
#include "mem.h"

/* Local protos */
static void atm_init0(void);
static void atm_init1(void);
static void atm_dump(void);

/* Static variables */
static const char *rcsid="$Id: lane_atm.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $";

/* Data */
#define QLEN 5

const Unit_t atm_unit = {
  "atm",
  &atm_init0,
  &atm_init1,
  &atm_dump,
  NULL
};

static void 
atm_dump(void)
{
  ;
}

static void 
atm_init0(void)
{
  ;
}

static void 
atm_init1(void)
{
  set_var_str(&atm_unit, "version", rcsid);

  Debug_unit(&atm_unit,"Initialized");
}

int 
atm_create_socket(unsigned char codepoint, const AtmAddr_t *our_addr)
{ 
  struct sockaddr_atmsvc server;
  struct atm_sap atmsap;
  struct atm_blli blli;
  struct atm_qos qos;
  int fd, ret;
  int len = sizeof(server);

  fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
  if (fd <0) {
    dump_error(&atm_unit,"socket");
    return -1;
  }

  memset(&server, 0, len);
  memset(&blli, 0, sizeof(blli));
  memset(&qos, 0, sizeof(qos));
  server.sas_family = AF_ATMSVC;
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
    dump_error(&atm_unit, "setsockopt(fd, SOL_ATM, SO_ATMQOS,...)");
    close(fd);
    return -1;
  }
  disp_sockaddr(&server, &blli);

  memset(&atmsap, 0, sizeof(struct atm_sap));
  atmsap.blli[0] = blli;
  if (setsockopt(fd,SOL_ATM,SO_ATMSAP,&atmsap,sizeof(atmsap)) < 0) {
    dump_error(&atm_unit, "setsockop(SO_ATMSAP)");
    (void) close(fd);
    return -1;
  }
 
  ret = bind(fd, (struct sockaddr *)&server, len);
  if (ret <0) {
    dump_error(&atm_unit, "bind");
    close(fd);
    return -1;
  }
  ret = listen(fd, QLEN);
  if (ret <0) {
    dump_error(&atm_unit, "listen");
    close(fd);
    return -1;
  }
  return fd;
}

int
atm_connect_back(const AtmAddr_t *our_addr, const Conn_t *conn,
		 unsigned char codepoint)
{
  struct sockaddr_atmsvc address;
  struct atm_sap atmsap;
  struct atm_blli blli;
  struct atm_qos qos;
  int fd, ret;
  socklen_t len = sizeof(address);
  
  fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
  if (fd <0) {
    dump_error(&atm_unit,"socket");
    return -1;
  }
  
  memset(&qos, 0, sizeof(qos));
  qos.aal = ATM_AAL5;
  qos.txtp.traffic_class = ATM_UBR;
  qos.txtp.max_sdu = 1516;
  qos.rxtp.traffic_class = ATM_UBR;
  qos.rxtp.max_sdu = 1516;

  if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0) {
    dump_error(&atm_unit, "setsockopt(fd, SOL_ATM, SO_ATMQOS,...)");
    close(fd);
    return -1;
  }

  memset(&address, 0, len);
  address.sas_family = AF_ATMSVC;
  memcpy(address.sas_addr.prv, our_addr, sizeof(AtmAddr_t));

  disp_sockaddr(&address, &blli);

  ret = bind(fd, (struct sockaddr *)&address, len);
  if (ret <0) {
    dump_error(&atm_unit, "bind");
    close(fd);
    return -1;
  }
  memset(&address, 0, len);
  ret = getpeername(conn->fd, (struct sockaddr*)&address, &len);
  if (ret < 0) {
    dump_error(&atm_unit, "getpeername");
    close(fd);
    return -1;
  }
  memset(&blli, 0, sizeof(blli));
  blli.l3_proto = ATM_L3_TR9577;
  blli.l3.tr9577.ipi = NLPID_IEEE802_1_SNAP;
  blli.l3.tr9577.snap[0] = 0x00;
  blli.l3.tr9577.snap[1] = 0xa0;
  blli.l3.tr9577.snap[2] = 0x3e;
  blli.l3.tr9577.snap[3] = 0x00;
  blli.l3.tr9577.snap[4] = codepoint;
  
  disp_sockaddr(&address, &blli);

  memset(&atmsap, 0, sizeof(struct atm_sap));
  atmsap.blli[0] = blli;
  if (setsockopt(fd,SOL_ATM,SO_ATMSAP,&atmsap,sizeof(atmsap)) < 0) {
    dump_error(&atm_unit, "setsockop(SO_ATMSAP)");
    (void) close(fd);
    return -1;
  }
  
  ret = connect(fd, (struct sockaddr*)&address,sizeof(struct sockaddr_atmsvc));
  if (ret < 0) {
    dump_error(&atm_unit, "connect");
    close(fd);
    return -1;
  }
  return fd;
}

