/*
 * Debug packet dumper
 *
 * $Id: dump.h,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */
#ifndef LANE_DUMP_H
#define LANE_DUMP_H

/* System includes needed for types */
#include <atm.h>

/* Local includes needed for types */
#include "atmsap.h"
#include "units.h"
#include "lane.h"

/* Type definitions */
/* Output destinations: no output, standard error, file, syslog() */
typedef enum {
  DT_NONE, DT_STDERR, DT_FILE, DT_SYSLOG, DT_CMN_ERR, DT_CONSOLE
} DumpType_t;

/* 
 * Message type: continuation of another message, debug, notification only,
 * warning, error, panic (causes abort())
 */
typedef enum {
  EL_CONT, EL_DEBUG, EL_NOTE, EL_WARN, EL_ERROR, EL_PANIC
} ErrorLevel_t;


/* Global function prototypes */
void dump_printf(ErrorLevel_t level, const char *const format, ...);
void Debug_unit(const Unit_t *unit, const char *const format, ...);
void dump_error(const Unit_t *unit, const char *msg);

void dump_addr(const LaneDestination_t *addr);
void dump_atmaddr(const AtmAddr_t *addr);
void dump_control(const LaneControl_t *c);
void disp_sockaddr(struct sockaddr_atmsvc *addr, struct atm_blli *blli);

/* Global data */
extern const Unit_t dump_unit;
extern DumpType_t dump_type;

#endif
