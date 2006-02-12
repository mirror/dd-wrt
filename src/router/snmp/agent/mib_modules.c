/*
 * wrapper to call all the mib module initialization functions 
 */

#include <net-snmp/agent/mib_module_config.h>
#include <net-snmp/net-snmp-config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "m2m.h"

#include "mibgroup/struct.h"
#include <net-snmp/agent/mib_modules.h>
#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_iterator.h>
#include "mib_module_includes.h"
#ifdef USING_AGENTX_SUBAGENT_MODULE
#include "mibgroup/agentx/subagent.h"
#endif

struct module_init_list *initlist = NULL;
struct module_init_list *noinitlist = NULL;


void
add_to_init_list(char *module_list)
{
    struct module_init_list *newitem, **list;
    char           *cp;

    if (module_list == NULL) {
        return;
    } else {
        cp = (char *) module_list;
    }

    if (*cp == '-' || *cp == '!') {
        cp++;
        list = &noinitlist;
    } else {
        list = &initlist;
    }

    cp = strtok(cp, ", :");
    while (cp) {
        newitem = (struct module_init_list *) calloc(1, sizeof(*initlist));
        newitem->module_name = strdup(cp);
        newitem->next = *list;
        *list = newitem;
        cp = strtok(NULL, ", :");
    }
}

int
should_init(const char *module_name)
{
    struct module_init_list *listp;

    /*
     * a definitive list takes priority 
     */
    if (initlist) {
        listp = initlist;
        while (listp) {
            if (strcmp(listp->module_name, module_name) == 0) {
                DEBUGMSGTL(("mib_init", "initializing: %s\n",
                            module_name));
                return DO_INITIALIZE;
            }
            listp = listp->next;
        }
        DEBUGMSGTL(("mib_init", "skipping:     %s\n", module_name));
        return DONT_INITIALIZE;
    }

    /*
     * initialize it only if not on the bad list (bad module, no bone) 
     */
    if (noinitlist) {
        listp = noinitlist;
        while (listp) {
            if (strcmp(listp->module_name, module_name) == 0) {
                DEBUGMSGTL(("mib_init", "skipping:     %s\n",
                            module_name));
                return DONT_INITIALIZE;
            }
            listp = listp->next;
        }
    }
    DEBUGMSGTL(("mib_init", "initializing: %s\n", module_name));

    /*
     * initialize it 
     */
    return DO_INITIALIZE;
}

void
init_mib_modules(void)
{
#  include "mib_module_inits.h"
}
