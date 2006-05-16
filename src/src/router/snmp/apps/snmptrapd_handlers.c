#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <net-snmp/config_api.h>
#include <net-snmp/output_api.h>
#include <net-snmp/mib_api.h>
#include <net-snmp/utilities.h>

struct traphandle {
    char           *exec;
    oid             trap[MAX_OID_LEN];
    size_t          traplen;
    struct traphandle *next;
};

struct traphandle *traphandlers = 0;
struct traphandle *defaulthandler = 0;

/*
 * handles parsing .conf lines of: 
 */
/*
 * traphandle OID EXEC           
 */

char           *
snmptrapd_get_traphandler(oid * name, size_t namelen)
{
    struct traphandle **ttmp;
    DEBUGMSGTL(("snmptrapd:traphandler", "looking for trap handler for "));
    DEBUGMSGOID(("snmptrapd:traphandler", name, namelen));
    DEBUGMSG(("snmptrapd:traphandler", "...\n"));
    for (ttmp = &traphandlers;
         *ttmp != NULL
         && snmp_oid_compare((*ttmp)->trap, (*ttmp)->traplen, name,
                             namelen); ttmp = &((*ttmp)->next));
    if (*ttmp == NULL) {
        if (defaulthandler) {
            DEBUGMSGTL(("snmptrapd:traphandler",
                        "  None found, Using the default handler.\n"));
            return defaulthandler->exec;
        }
        DEBUGMSGTL(("snmptrapd:traphandler", "  Didn't find one.\n"));
        return NULL;
    }
    DEBUGMSGTL(("snmptrapd:traphandler", "  Found it!\n"));
    return (*ttmp)->exec;
}

void
snmptrapd_traphandle(const char *token, char *line)
{
    struct traphandle **ttmp;
    char            buf[STRINGMAX];
    char           *cptr;
    int             doingdefault = 0;

    /*
     * find the current one, if it exists 
     */
    if (strncmp(line, "default", 7) == 0) {
        ttmp = &defaulthandler;
        doingdefault = 1;
    } else {
        for (ttmp = &traphandlers; *ttmp != NULL; ttmp = &((*ttmp)->next));
    }

    if (*ttmp == NULL) {
        /*
         * it doesn't, so allocate a new one. 
         */
        *ttmp = (struct traphandle *) malloc(sizeof(struct traphandle));
        if (!*ttmp) {
            config_perror("malloc failed");
            return;
        }
        memset(*ttmp, 0, sizeof(struct traphandle));
    } else {
        if ((*ttmp)->exec)
            free((*ttmp)->exec);
    }
    cptr = copy_nword(line, buf, sizeof(buf));
    if (!doingdefault) {
        (*ttmp)->traplen = MAX_OID_LEN;
        if (!read_objid(buf, (*ttmp)->trap, &((*ttmp)->traplen))) {
            char            buf1[STRINGMAX];
            snprintf(buf1,  sizeof(buf1),
                    "Bad trap OID in traphandle directive: %s", buf);
            buf1[ sizeof(buf1)-1 ] = 0;
            config_perror(buf1);
            return;
        }
    }

    (*ttmp)->exec = strdup(cptr);
    DEBUGMSGTL(("read_config:traphandler", "registered handler for: "));
    if (doingdefault) {
        DEBUGMSG(("read_config:traphandler", "default"));
    } else {
        DEBUGMSGOID(("read_config:traphandler", (*ttmp)->trap,
                     (*ttmp)->traplen));
    }
    DEBUGMSG(("read_config:traphandler", "\n"));
}
