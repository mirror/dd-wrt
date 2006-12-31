/*
 * Debug packet dumper
 *
 * $Id: dump.c,v 1.16 1995/07/02 19:29:17 carnil Exp $
 *
 */

/* System includes */
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

#include <atm.h>

/* Local includes */
#include "atmsap.h"
#include "dump.h"
#include "lane.h"
#include "load.h"
#include "units.h"

/* Local function prototypes */
static void dump_init0(void);
static void dump_init1(void);
static void dump_dump(void);
static void dump_release(void);

/* Data */
#define BUFSIZE 256

static const char *rcsid = "$Id: dump.c,v 1.16 1995/07/02 19:29:17 carnil Exp $";

const Unit_t dump_unit = {
  "dump",
  &dump_init0,
  &dump_init1,
  &dump_dump,
  &dump_release
};

DumpType_t dump_type;
static DumpType_t dump_open;

static FILE *dump_filep;
static const char *dump_filename = "/tmp/lane.log";
static const char *dump_console = "/dev/console";

/* Functions */
static const char* dump_opcode_text(unsigned short opcode);
static const char* dump_status_text(unsigned short status);
static const char* dump_maxframe(unsigned char size);
static const char* dump_lantype(unsigned char lantype);
static void dump_atmtext(const AtmAddr_t addr);

/* Initialize local data */
static void
dump_init0(void)
{
  dump_filep = NULL;
  dump_open = dump_type;

  switch (dump_open) {
  case DT_NONE:
  case DT_CMN_ERR:
    break;
  case DT_STDERR:
    dump_filep = stderr;
    break;
  case DT_FILE:
    dump_filep = fopen(dump_filename, "a");
    if (dump_filep == NULL) {
      dump_open = DT_STDERR;
      dump_filep = stderr;
      dump_printf(EL_WARN, "Cannot open log file %s", dump_filename);
    }
    break;
  case DT_SYSLOG:
    openlog("lane", LOG_PID | LOG_CONS, LOG_DAEMON);
    break;
  case DT_CONSOLE:
    dump_filep = fopen(dump_console, "w");
    if (dump_filep == NULL) {
      dump_open = DT_STDERR;
      dump_filep = stderr;
      dump_printf(EL_WARN, "Cannot open console log");
    }
    break;
  }
  dump_printf(EL_DEBUG, "Log opened");
}

/* Initialization for data that needs other units */
static void
dump_init1(void)
{
  set_var_str(&dump_unit, "version", rcsid);
  Debug_unit(&dump_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
dump_dump(void)
{
  static const char *dumptypes [] = {
    "none", "standard error", "file", "syslog()", "cmn_err()", "/dev/console"
  };

  Debug_unit(&dump_unit, "Dumping to %s, filename: %s", dumptypes[dump_open],
	     dump_filename);
}

/* Release allocated memory, close files etc. */
static void
dump_release(void)
{
  int flush_ret;

  Debug_unit(&dump_unit, "Releasing unit");

  dump_printf(EL_DEBUG, "Closing log file");

  switch (dump_open) {
  case DT_NONE:
  case DT_CMN_ERR:
    break;
  case DT_STDERR:
    flush_ret = fflush(stderr);
    assert(flush_ret == 0);
    break;
  case DT_FILE:
    if (fclose(dump_filep) != 0) {
      dump_open = DT_STDERR;
      dump_filep = stderr;
      dump_printf(EL_WARN, "Cannot close log file %s", dump_filename);
    }
    break;
  case DT_SYSLOG:
    closelog();
    break;
  case DT_CONSOLE:
      if (fclose(dump_filep) != 0) {
      dump_open = DT_STDERR;
      dump_filep = stderr;
      dump_printf(EL_WARN, "Cannot close console log");
    }
    break;
  }
  dump_filep = NULL;
  dump_open = DT_NONE;
}

static const int el_to_log[] = {
  /* EL_CONT, EL_DEBUG, EL_NOTE, EL_WARN, EL_ERROR, EL_PANIC */
  LOG_DEBUG, LOG_DEBUG, LOG_NOTICE, LOG_WARNING, LOG_ERR, LOG_ALERT
};

static const char *el_prefix[] = {
  /* EL_CONT, EL_DEBUG, EL_NOTE, EL_WARN, EL_ERROR, EL_PANIC */
  "", "DEBUG: ", "NOTE: ", "WARNING: ", "ERROR: ", "PANIC: "
};

/* General printing function */
void
dump_printf(ErrorLevel_t level, const char *const format, ...)
{
  va_list args;
  int printf_ret, flush_ret;
  char buffer[BUFSIZE];

  assert(level <= EL_PANIC);

  switch (dump_open) {
  case DT_CMN_ERR:
  case DT_NONE:
    return;
  case DT_CONSOLE:
  case DT_STDERR:
  case DT_FILE:
    va_start(args, format);
    printf_ret = vsprintf(buffer, format, args);
    assert(printf_ret > 0);
    printf_ret = fprintf(dump_filep, "%s%s%s", el_prefix[level], buffer,
			 level != EL_CONT? "\n" : "");
    assert(printf_ret > 0);
    va_end(args);
    flush_ret = fflush(dump_filep);
    assert(flush_ret == 0);
    break;
  case DT_SYSLOG:
    va_start(args, format);
    printf_ret = vsprintf(buffer, format, args);
    assert(printf_ret > 0);
    syslog(el_to_log[level], "%s", buffer);
    va_end(args);
    break;
  }
}

/* Unit-specific debugging, can be enabled with variables */
void
Debug_unit(const Unit_t *unit, const char *const format, ...)
{
  va_list args;
  char buffer[2048];
  char buffer2[2048];
  int sprintf_ret;

#ifndef DEBUG
  if (get_var_bool(unit, "debug") == BL_TRUE)  {
#endif /* DEBUG */
    sprintf_ret = sprintf(buffer, "%s: %s", unit->name, format);
    assert(sprintf_ret > 0);

    va_start(args, format);
    sprintf_ret = vsprintf(buffer2, buffer, args);
    assert(sprintf_ret > 0);
    va_end(args);

    dump_printf(EL_DEBUG, "%s", buffer2);
#ifndef DEBUG
  }
#endif /* DEBUG */
}

void
dump_error(const Unit_t *unit, const char *msg)
{
  Debug_unit(unit, "%s: %s", msg, strerror(errno));
}


/* Dumping functions, LANE address */
void
dump_addr(const LaneDestination_t *addr)
{
  assert(addr != NULL);
  switch (ntohs(addr->tag)) {
  case LANE_DEST_NP:
    dump_printf(EL_CONT, "Not present");
    break;
    
  case LANE_DEST_MAC:
    dump_printf(EL_CONT, "MAC addr %2.2x.%2.2x.%2.2x.%2.2x.%2.2x.%2.2x",
		addr->a_r.mac_address[0],addr->a_r.mac_address[1],
		addr->a_r.mac_address[2],addr->a_r.mac_address[3],
		addr->a_r.mac_address[4],addr->a_r.mac_address[5]
		);
    break;
    
  case LANE_DEST_RD:
    dump_printf(EL_CONT, "Route Designator %4.4x", addr->a_r.route.designator);
    break;
  }
}

/* Dumping functions, ATM address */
void
dump_atmaddr(const AtmAddr_t *addr)
{
  int i;
  char buffer[ATM_ADDR_LEN*3+2];

  assert(addr != 0);

  for (i = 0; i<ATM_ADDR_LEN; i++){
    sprintf(&buffer[i*2], "%2.2x ", 0xff&(unsigned char)addr->addr[i]);
  }

  dump_printf(EL_DEBUG, "%s", buffer);
}

static void
dump_atmtext(const AtmAddr_t addr)
{
  int i;
  char buffer[ATM_ADDR_LEN*3+2];

  for (i=0;i<ATM_ADDR_LEN;i++) {
    sprintf(&buffer[i*2], "%2.2x ", 0xff&(unsigned char)addr.addr[i]);
  }
  dump_printf(EL_CONT, "%s", buffer);
}

/* Dumping functions, LANE control frame */
static const char*
dump_status_text(unsigned short status)
{
  switch (status) {
  case LE_STATUS_SUCCESS :  return "Success";
  case LE_STATUS_BAD_VERSION :  return "Version_Not_Supported";
  case LE_STATUS_BAD_REQ :  return "Invalid_Parameters";
  case LE_STATUS_DUPLICATE_REG :  return "Duplicate_LAN_Destination";
  case LE_STATUS_DUPLICATE_ADDR :  return "Duplicate_ATM_Address";
  case LE_STATUS_NO_RESOURCES :  return "Insufficient_Resources";
  case LE_STATUS_NO_ACCESS :  return "Access_Denied";
  case LE_STATUS_BAD_LECID :  return "Invalid_Request_ID";
  case LE_STATUS_BAD_DEST :  return "Invalid_LAN_Destination";
  case LE_STATUS_BAD_ADDR : return "Invalid_ATM_Address";
  case LE_STATUS_NO_CONFIG : return "No_Configuration";
  case LE_STATUS_CONFIG_ERROR : return "LE_Configuration_Error";
  case LE_STATUS_NO_INFO : return "Insufficient_Information";
  default : return "<Unknown Error Code>";
  }
}

static const char*
dump_opcode_text(unsigned short opcode)
{
  switch (opcode) {
  case LE_CONFIGURE_REQUEST : return "LE_CONFIG_REQUEST";
  case LE_CONFIGURE_RESPONSE : return "LE_CONFIG_RESPONSE";
  case LE_JOIN_REQUEST : return "LE_JOIN_REQUEST";
  case LE_JOIN_RESPONSE : return "LE_JOIN_RESPONSE";
  case READY_QUERY : return "READY_QUERY";
  case READY_IND : return "READY_INDICATION";
  case LE_REGISTER_REQUEST : return "LE_REGISTER_REQUEST";
  case LE_REGISTER_RESPONSE : return "LE_REGISTER_RESPONSE";
  case LE_UNREGISTER_REQUEST : return "LE_UNREGISTER_REQUEST";
  case LE_UNREGISTER_RESPONSE : return "LE_UNREGISTER_RESPONSE";
  case LE_ARP_REQUEST : return "LE_ARP_REQUEST";
  case LE_ARP_RESPONSE : return "LE_ARP_RESPONSE";
  case LE_FLUSH_REQUEST : return "LE_FLUSH_REQUEST";
  case LE_FLUSH_RESPONSE : return "LE_FLUSH_RESPONSE";
  case LE_NARP_REQUEST : return "LE_NARP_REQUEST";
  case LE_TOPOLOGY_REQUEST : return "LE_TOPOLOGY_REQUEST";
  default :     return "<Unknown Opcode>";
  }
}

static const char*
dump_lantype(unsigned char lantype)
{
  switch (lantype) {
  case LE_LAN_TYPE_UNSPECIFIED:
    return "Unspecified";
  case LE_LAN_TYPE_802_3:
    return "Ethernet/IEEE 802.3";
  case LE_LAN_TYPE_802_5:
    return "IEEE 802.5";
  default:
    return "<Unknown>";
  }
}

static const char*
dump_maxframe(unsigned char size)
{
  switch (size) {
  case LE_MAX_FRAME_UNSPECIFIED:
    return "Unspecified";
  case LE_MAX_FRAME_1516:
    return "1516";
  case LE_MAX_FRAME_4544:
    return "4544";
  case LE_MAX_FRAME_9234:
    return "9234";
  case LE_MAX_FRAME_18190:
    return "18190";
  default:
    return "<Unknown>";
  }
}

void
dump_control(const LaneControl_t *c)
{
  dump_printf(EL_DEBUG, "\tMarker:\t0x%4.4hx\n\tProtocol:\t0x%2.2x\n\tVersion:\t0x%2.2x",
	      ntohs(c->marker), c->protocol, c->version);
  dump_printf(EL_CONT, "\tOpcode:\t\t%s\n\tStatus:\t\t%s\n\tTransactionID:\t0x%8.8x\n\t"
	      "LECID:\t\t0x%4.4hx\n\tFlags:\t\t0x%4.4hx\n",
	      dump_opcode_text(ntohs(c->opcode)), 
	      dump_status_text(ntohs(c->status)),
	      ntohl(c->transaction_id), ntohs(c->lecid), ntohs(c->flags));
  switch(ntohs(c->opcode)) {
  case LE_CONFIGURE_REQUEST:
  case LE_CONFIGURE_RESPONSE:
  case LE_JOIN_REQUEST:
  case LE_JOIN_RESPONSE:
    dump_printf(EL_CONT,"\tSource LAN:\t");
    dump_addr(&c->source);
    dump_printf(EL_CONT,"\n\tSource ATM:\t");
    dump_atmtext(c->source_addr);
    dump_printf(EL_CONT,"\n\tLan type:\t%s\n",dump_lantype(c->lan_type));
    dump_printf(EL_CONT,"\tMax frame:\t%s\n",dump_maxframe(c->max_frame));
    dump_printf(EL_CONT,"\tELAN name size:\t%d\n",c->elan_name_size);
    if (c->elan_name_size>0) {
      dump_printf(EL_CONT,"\nELAN name:\t%s\n",c->elan_name);
    }
    break;
  case LE_ARP_REQUEST:
  case LE_ARP_RESPONSE:
    dump_printf(EL_CONT,"\tSource LAN:\t");
    dump_addr(&c->source);
    dump_printf(EL_CONT,"\n\tTarget LAN:\t");
    dump_addr(&c->target);
    dump_printf(EL_CONT,"\n\tSource ATM:\t");
    dump_atmtext(c->source_addr);
    dump_printf(EL_CONT,"\n\tTarget ATM:\t");
    dump_atmtext(c->target_addr);
    dump_printf(EL_CONT,"\n");
    break;
  case LE_FLUSH_REQUEST:
  case LE_FLUSH_RESPONSE:
    dump_printf(EL_CONT,"\tSource ATM:\t");
    dump_atmtext(c->source_addr);
    dump_printf(EL_CONT,"\n\tTarget ATM:\t");
    dump_atmtext(c->target_addr);
    dump_printf(EL_CONT,"\n");
    break;
  case LE_REGISTER_REQUEST:
  case LE_REGISTER_RESPONSE:
  case LE_UNREGISTER_REQUEST:
  case LE_UNREGISTER_RESPONSE:
    dump_printf(EL_CONT,"\tSource LAN:\t");
    dump_addr(&c->source);
    dump_printf(EL_CONT,"\n\tSource ATM:\t");
    dump_atmtext(c->source_addr);
    dump_printf(EL_CONT,"\n");
    break;
  }
}

void 
disp_sockaddr(struct sockaddr_atmsvc *addr, struct atm_blli *blli)
{
  int i;
  dump_printf(EL_DEBUG,"Socket_address");
  dump_printf(EL_CONT, "Sas_family:%d\n\tAddress:",addr->sas_family);
  for(i=0;i<20;i++) {
    dump_printf(EL_CONT,"%2.2x ",addr->sas_addr.prv[i]);
  }
  dump_printf(EL_CONT,"\nBlli:\n\t");
  dump_printf(EL_CONT,"l2_proto:%d\n\t",blli->l2_proto);
  dump_printf(EL_CONT,"l3_proto:%d\n\t\t",blli->l3_proto);
  dump_printf(EL_CONT,"ipi:%x\tsnap:",blli->l3.tr9577.ipi);
  for(i=0;i<5;i++) {
    dump_printf(EL_CONT,"%2.2x ",blli->l3.tr9577.snap[i]);
  }
  dump_printf(EL_CONT,"\n");
}
