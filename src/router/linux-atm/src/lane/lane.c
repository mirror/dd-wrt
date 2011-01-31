/*
 * Lan Emulation Server
 *
 * $Id: lane.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

/* Local includes */
#include "units.h"
#include "load.h"
#include "dump.h"
#include "mem.h"
#include "connect.h"
#include "events.h"
#include "timers.h"

/* Type definitions */

/* Local function prototypes */
static void main_init1(void);
static void main_release(void);
static void parse_args(int argc, char **argv);
static void usage(void);
static int dump_handler(const Event_t *event, void *funcdata);
static int exit_handler(const Event_t *event, void *funcdata);

/* Data */
static const char *rcsid = "$Id: lane.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $";
const Unit_t main_unit = {
  "main",
  NULL,
  main_init1,
  NULL,
  main_release
};

static const char *progname;
char *var_file = NULL;

/* Functions */

/* Initialization for data that needs other units */
static void
main_init1(void)
{
  set_var_str(&main_unit, "version", rcsid);
  add_event_handler(CE_DUMP, &dump_handler, "dump_handler", NULL);
  add_event_handler(CE_EXIT, &exit_handler, "exit_handler", NULL);
  Debug_unit(&main_unit, "Initialized.");
}

static void 
main_release(void)
{
  if (var_file) {
    mem_free(&main_unit, var_file);
    var_file = NULL;
  }
}

/* Main loop */
int
main(int argc, char **argv)
{
  short do_restart = 0;
  const Event_t *event;
  const Unit_t **units;

  /* Debugging facility */
  dump_type = DT_STDERR;

  while (1) {
    /* Call phase 0 initializers */
    Debug_unit(&main_unit, "Calling phase 0 initializers");
    FOR_ALL_UNITS(units) {
      if ((*units)->init0 != NULL) {
	Debug_unit(&main_unit, "Initializing %s", (*units)->name);
	(*((*units)->init0))();
      }
    }

    /* Get flags from command line */
    parse_args(argc, argv);

    /* Call phase 1 initializers */
    Debug_unit(&main_unit, "Calling phase 1 initializers");
    FOR_ALL_UNITS(units) {
      if ((*units)->init1 != NULL) {
	Debug_unit(&main_unit, "Initializing %s", (*units)->name);
	(*((*units)->init1))();
      }
    }

    do_restart = 0;

    while (!do_restart) {
      event = event_get_next();
      if (dispatch_handlers(event) == 1) {
	continue;
      }
      switch (event->type) {
      case CE_RESTART:
	do_restart = 1;
	break;
      case CE_EXIT:
	break;
      case CE_DUMP:
	break;
      case CE_SVC_OPEN:
	break;
      case CE_SVC_CLOSE:
	break;
      case CE_DATA:
	break;
      case CE_TIMER:
	break;
      }
      mem_free(&main_unit, event);
    }
    /* Restart */
    Debug_unit(&main_unit, "Releasing %d units", num_units);
    FOR_ALL_UNITS_REV(units) {
      if ((*units)->release != NULL) {
	Debug_unit(&main_unit, "Releasing %s", (*units)->name);
	(*((*units)->release))();
      }
    }
  }
  return 0;
}

/* Process CE_DUMP events */
static int
dump_handler(const Event_t *event, void *funcdata)
{
  const Unit_t **units;

  mem_free(&main_unit, event);
  FOR_ALL_UNITS(units) {
    if ((*units)->dump != NULL) {
      Debug_unit(&main_unit, "Dumping %s", (*units)->name);
      (*((*units)->dump))();
    }
  }
  return 1;
}

/* Process CE_EXIT events */
static int
exit_handler(const Event_t *event, void *funcdata)
{
  const Unit_t **units;

  mem_free(&main_unit, event);
  Debug_unit(&main_unit, "Releasing %d units", num_units);
  FOR_ALL_UNITS_REV(units) {
    if ((*units)->release != NULL) {
      Debug_unit(&main_unit, "Releasing %s", (*units)->name);
      (*((*units)->release))();
    }
  }
  exit(0);
}

static void
parse_args(int argc, char **argv)
{
  int i = 0;
  const Unit_t *unit, **units;
  
  progname = argv[0];
  while(i!=-1) {
    i = getopt(argc, argv, "d:m:f:");
    switch(i) {
    case 'd':
      if (strcmp(optarg, "all") == 0) {
	FOR_ALL_UNITS(units) {
	  set_var_bool(*units, "debug", BL_TRUE);
	}
      } else {
	unit = find_unit(optarg);
	if (unit) {
	  set_var_bool(unit, "debug", BL_TRUE);
	} else {
	  dump_printf(EL_ERROR, "Unknown module name: %s", optarg);
	  usage();
	}
      }
      break;
    case 'm':
      if (strcmp(optarg, "all") == 0) {
	FOR_ALL_UNITS(units) {
	  set_var_bool(*units, "memdebug", BL_TRUE);
	}
      } else {
	unit = find_unit(optarg);
	if (unit) {
	  set_var_bool(unit, "memdebug", BL_TRUE);
	} else {
	  dump_printf(EL_ERROR, "Unknown module name: %s", optarg);
	  usage();
	}
      }
      break;
    case 'f':
      var_file = mem_alloc(&main_unit, strlen(optarg)+1);
      strcpy(var_file, optarg);
      dump_printf(EL_NOTE, "Configuration file: %s\n", var_file);
      break;
    case -1:
      break;
    default:
      usage();
      return;
    }
  }
  if (argc != optind) usage();
  if (!var_file) {
    var_file = mem_alloc(&main_unit, strlen(DEFAULT_CFG_FILE)+1);
    strcpy(var_file, DEFAULT_CFG_FILE);
    dump_printf(EL_NOTE, "Configuration file: %s\n", var_file);
  }    
}

static void
usage(void)
{
    dump_printf(EL_ERROR, "Usage:");
    dump_printf(EL_ERROR, "%s [-d module]... [-m module]...[-f conf_file]", progname);
    exit(1);
}

