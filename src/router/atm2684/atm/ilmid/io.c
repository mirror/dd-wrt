/*
 * io.c - Ilmi input/output routines
 *
 * Written by Scott W. Shumate
 * 
 * Copyright (c) 1995-97 All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * I ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>
#include <atm.h>
#include <linux/atmdev.h>
#include "io.h"
#include "atmd.h"
#include "atmf_uni.h"

#define SNMP_VCI 16
#define COMPONENT "IO"
#define MAX_EXTRA_ADDRS 4 /* maximum number of additional addresses that can
			     be manually configured (after ilmid has
			     registered the "official" address) - HACK */

static short atm_itf = -1; /* bad value */


AsnOid *get_esi(int fd, int itf)
{
  static AsnOid *name;
  struct atmif_sioc req;
  unsigned char esi[ESI_LEN];
  int m, n, size;

  req.number = itf;
  req.arg = esi;
  req.length = ESI_LEN;

  if(ioctl(fd, ATM_GETESI, &req) < 0)
    diag(COMPONENT, DIAG_FATAL, "ioctl ATM_GETESI: %s", strerror(errno));
 
  /* save esi to atmfMySystemIdentifierValue */
  if (!atmfMySystemIdentifierValue.octs) {
     atmfMySystemIdentifierValue.octs = alloc(6);
     memcpy(atmfMySystemIdentifierValue.octs, esi, 6);
  }
 
  /* Convert hex string to Object ID BER */
  for(m = 0, size = 0; m < ESI_LEN; esi[m++] & 0x80 ? size += 2 : size++);
  size++;
  name = alloc_t(AsnOid);
  name->octs = alloc(size);
  name->octetLen = size;
  
  for(m = 0, n = 0; m < ESI_LEN; m++, n++)
    {
      if(esi[m] & 0x80)
	name->octs[n++] = '\201';
      name->octs[n] = esi[m] & 0x7F;
    }
  
  /* Add the SEL */
  name->octs[n] = '\0';

  return name;
}

void update_nsap(int itf, AsnOid *netprefix, AsnOid *esi)
{
  struct atmif_sioc req;
  struct sockaddr_atmsvc addr, ouraddr[MAX_EXTRA_ADDRS+1];
  char buffer[MAX_ATM_ADDR_LEN+1];
  int fd, m, n;

  addr.sas_family = AF_ATMSVC;
  addr.sas_addr.pub[0] = 0;
  
  /* Convert net prefix BER to hex */
  for(m = 0, n = 0; m < netprefix->octetLen; m++, n++)
    if(netprefix->octs[m] & 0x80)
      addr.sas_addr.prv[n] = netprefix->octs[++m] | 0x80;
    else
      addr.sas_addr.prv[n] = netprefix->octs[m];

  /* Convert esi BER to hex */
  for(m = 0; m < esi->octetLen; m++, n++)
    if(esi->octs[m] & 0x80)
      addr.sas_addr.prv[n] = esi->octs[++m] | 0x80;
    else
      addr.sas_addr.prv[n] = esi->octs[m];

  if ((fd = socket(AF_ATMSVC, SOCK_DGRAM, 0)) < 0)
    diag(COMPONENT, DIAG_FATAL, "socket: %s", strerror(errno));
  
  req.number = itf;
  req.arg = &ouraddr;
  req.length = sizeof(ouraddr);

  /* Try to get our address on that interface */

  if (ioctl(fd, ATM_GETADDR, &req) <0)
    diag(COMPONENT, DIAG_FATAL, "ioctl ATM_GETADDR: %s", strerror(errno));

  n = 0;
  if (req.length && atm_equal((struct sockaddr *) &addr,
   (struct sockaddr *) &ouraddr[0], ATM_ESA_LEN, 0)) {
    diag(COMPONENT, DIAG_INFO, "Primary ATM Address did not change");
    n = 1;
  }

  if ((!(m = req.length)) || (!n)) {
  	  req.number = itf;
  	  req.arg = NULL;
  	  req.length = 0;

  	  if (ioctl(fd, ATM_RSTADDR, &req) < 0)
    	    diag(COMPONENT, DIAG_FATAL, "ioctl ATM_RSTADDR: %s", strerror(errno));

	  req.number = itf;
	  req.arg = &addr;
	  req.length = sizeof(addr); 

	  if (ioctl(fd, ATM_ADDADDR, &req) < 0)
	    diag(COMPONENT, DIAG_FATAL, "ioctl ATM_ADDADDR: %s", strerror(errno));

	  atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &addr, A2T_PRETTY);

	  diag(COMPONENT, DIAG_INFO, "Primary ATM Address %s added local", buffer);
	
	  for (n = 0; n < m/sizeof(addr); n++) {

		  if(n > MAX_EXTRA_ADDRS-1) break; /* We have already registered "primary" NSAP */

		  req.number = itf;
		  req.arg = &ouraddr[n];
		  req.length = sizeof(*ouraddr); 
	  
		  atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &ouraddr[n], A2T_PRETTY);

		  if (ioctl(fd, ATM_ADDADDR, &req) < 0)
		    diag(COMPONENT, DIAG_ERROR, "ioctl ATM_ADDADDR: %s", strerror(errno));
	          else	
		    diag(COMPONENT, DIAG_INFO, "Extra   ATM Address %s added local", buffer);

	  }
  }	     	
  close(fd);

}

int wait_for_message(int fd, struct timeval *timeout)
{
  int numfds;
  fd_set fdvar;

  FD_ZERO(&fdvar);
  FD_SET(fd, &fdvar);

  if((numfds = select(fd + 1, &fdvar, 0, 0, timeout)) < 0)
    diag(COMPONENT, DIAG_FATAL, "select: %s", strerror(errno));

  return numfds;
}


int read_message(int fd, Message *message)
{
  SBuf buffer;
  char data[MAX_ILMI_MSG];
  AsnLen length;
  jmp_buf env;

  if ((int) (length = read(fd, data, MAX_ILMI_MSG)) < 0)
    diag(COMPONENT, DIAG_FATAL, "read: %s", strerror(errno));

  SBufInstallData(&buffer, data, length);
  if (setjmp(env) == 0)
    {
      BDecMessage(&buffer, message, &length, env);
    }
  else
    {
      diag(COMPONENT, DIAG_ERROR, "message decoding error");
      return -1;
    }

  diag(COMPONENT, DIAG_DEBUG, "SNMP message received:");
  if(get_verbosity(NULL) == DIAG_DEBUG)
    PrintMessage(get_logfile(), message, 0);
  
  if(message->version != VERSION_1)
    {
      diag(COMPONENT, DIAG_ERROR, "received message with wrong version number");
      return -1;
    }

  if(message->community.octetLen != 4 || 
     memcmp(message->community.octs, "ILMI", 4))
    {
      diag(COMPONENT, DIAG_ERROR, "received message with wrong community");
      return -1;
    }

  return 0;
}

int send_message(int fd, Message *message)
{
  SBuf buffer;
  AsnLen length;
  char data[MAX_ILMI_MSG];

  SBufInit(&buffer, data, MAX_ILMI_MSG);
  SBufResetInWriteRvsMode(&buffer);
  
  if(!(length = BEncMessage(&buffer, message)))
  {
    diag(COMPONENT, DIAG_ERROR, "message encoding error");
    return -1;
  }

  if(write(fd, SBufDataPtr(&buffer), length) != length)
    diag(COMPONENT, DIAG_FATAL, "write: %s", strerror(errno));

  diag(COMPONENT, DIAG_DEBUG, "SNMP message sent:");
  if(get_verbosity(NULL) == DIAG_DEBUG)
    PrintMessage(get_logfile(), message, 0);
  
  return 0;
}


int open_ilmi(int itf,const char *qos_spec)
{
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    int fd;

    if((fd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0)
      diag(COMPONENT, DIAG_FATAL, "socket: %s", strerror(errno));

    atm_itf = itf;
    memset(&qos, 0, sizeof(qos));
    qos.rxtp.max_sdu = MAX_ILMI_MSG;
    qos.txtp.max_sdu = MAX_ILMI_MSG;
    qos.aal = ATM_AAL5;
    if (!qos_spec) qos.rxtp.traffic_class = qos.txtp.traffic_class = ATM_UBR;
    else if (text2qos(qos_spec,&qos,T2Q_DEFAULTS) < 0)
	    diag(COMPONENT,DIAG_FATAL,"invalid qos: %s",qos_spec);
    if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
	diag(COMPONENT,DIAG_FATAL,"setsockopt SO_ATMQOS: %s",strerror(errno));

    memset(&addr, 0, sizeof(addr));
    addr.sap_family = AF_ATMPVC;
    addr.sap_addr.itf = itf;
    addr.sap_addr.vpi = 0;
    addr.sap_addr.vci = SNMP_VCI;

    if(bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
      diag(COMPONENT, DIAG_FATAL, "bind: %s", strerror(errno));

    return fd;
}


int get_ci_range(struct atm_cirange *ci)
{
    struct atmif_sioc req;
    int fd,error;

    fd = socket(PF_ATMPVC,SOCK_DGRAM,0);
    if (fd < 0) diag(COMPONENT,DIAG_FATAL,"socket: %s",strerror(errno));
    req.number = atm_itf;
    req.length = sizeof(*ci);
    req.arg = ci;
    error = ioctl(fd,ATM_GETCIRANGE,&req);
    (void) close(fd);
    if (error < 0) {
	diag(COMPONENT,DIAG_ERROR,"ioctl ATM_GETCIRANGE: %s",strerror(errno));
	return error;
    }
}
