/*
 * default_store.h: storage space for defaults 
 */

#include <net-snmp/net-snmp-config.h>
#include <sys/types.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/library/default_store.h>    /* for "internal" definitions */
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmp_api.h>

static const char * stores [NETSNMP_DS_MAX_IDS] = { "LIB", "APP", "TOK" };

typedef struct netsnmp_ds_read_config_s {
  u_char          type;
  char           *token;
  char           *ftype;
  int             storeid;
  int             which;
  struct netsnmp_ds_read_config_s *next;
} netsnmp_ds_read_config;

static netsnmp_ds_read_config *netsnmp_ds_configs = NULL;

static int   netsnmp_ds_integers[NETSNMP_DS_MAX_IDS][NETSNMP_DS_MAX_SUBIDS];
static char  netsnmp_ds_booleans[NETSNMP_DS_MAX_IDS][NETSNMP_DS_MAX_SUBIDS/8];
static char *netsnmp_ds_strings[NETSNMP_DS_MAX_IDS][NETSNMP_DS_MAX_SUBIDS];
static void *netsnmp_ds_voids[NETSNMP_DS_MAX_IDS][NETSNMP_DS_MAX_SUBIDS];

/*
 * Prototype definitions 
 */
void            netsnmp_ds_handle_config(const char *token, char *line);

int
netsnmp_ds_set_boolean(int storeid, int which, int value)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    DEBUGMSGTL(("netsnmp_ds_set_boolean", "Setting %s:%d = %d/%s\n",
                stores[storeid], which, value, ((value) ? "True" : "False")));

    if (value > 0) {
        netsnmp_ds_booleans[storeid][which/8] |= (1 << (which % 8));
    } else {
        netsnmp_ds_booleans[storeid][which/8] &= (0xff7f >> (7 - (which % 8)));
    }

    return SNMPERR_SUCCESS;
}

int
netsnmp_ds_toggle_boolean(int storeid, int which)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    if ((netsnmp_ds_booleans[storeid][which/8] & (1 << (which % 8))) == 0) {
        netsnmp_ds_booleans[storeid][which/8] |= (1 << (which % 8));
    } else {
        netsnmp_ds_booleans[storeid][which/8] &= (0xff7f >> (7 - (which % 8)));
    }

    DEBUGMSGTL(("netsnmp_ds_toggle_boolean", "Setting %s:%d = %d/%s\n",
                stores[storeid], which, netsnmp_ds_booleans[storeid][which/8],
                ((netsnmp_ds_booleans[storeid][which/8]) ? "True" : "False")));

    return SNMPERR_SUCCESS;
}

int
netsnmp_ds_get_boolean(int storeid, int which)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    return (netsnmp_ds_booleans[storeid][which/8] & (1 << (which % 8))) ? 1:0;
}

int
netsnmp_ds_set_int(int storeid, int which, int value)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    DEBUGMSGTL(("netsnmp_ds_set_int", "Setting %s:%d = %d\n",
                stores[storeid], which, value));

    netsnmp_ds_integers[storeid][which] = value;
    return SNMPERR_SUCCESS;
}

int
netsnmp_ds_get_int(int storeid, int which)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    return netsnmp_ds_integers[storeid][which];
}

int
netsnmp_ds_set_string(int storeid, int which, const char *value)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    DEBUGMSGTL(("netsnmp_ds_set_string", "Setting %s:%d = \"%s\"\n",
                stores[storeid], which, (value ? value : "(null)")));

    /*
     * is some silly person is calling us with our own pointer?
     */
    if (netsnmp_ds_strings[storeid][which] == value)
        return SNMPERR_SUCCESS;
    
    if (netsnmp_ds_strings[storeid][which] != NULL) {
        free(netsnmp_ds_strings[storeid][which]);
	netsnmp_ds_strings[storeid][which] = NULL;
    }

    if (value) {
        netsnmp_ds_strings[storeid][which] = strdup(value);
    } else {
        netsnmp_ds_strings[storeid][which] = NULL;
    }

    return SNMPERR_SUCCESS;
}

char *
netsnmp_ds_get_string(int storeid, int which)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return NULL;
    }

    return netsnmp_ds_strings[storeid][which];
}

int
netsnmp_ds_set_void(int storeid, int which, void *value)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return SNMPERR_GENERR;
    }

    DEBUGMSGTL(("netsnmp_ds_set_void", "Setting %s:%d = %x\n",
                stores[storeid], which, value));

    netsnmp_ds_voids[storeid][which] = value;

    return SNMPERR_SUCCESS;
}

void *
netsnmp_ds_get_void(int storeid, int which)
{
    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS) {
        return NULL;
    }

    return netsnmp_ds_voids[storeid][which];
}

void
netsnmp_ds_handle_config(const char *token, char *line)
{
    netsnmp_ds_read_config *drsp;
    char            buf[SNMP_MAXBUF];
    char           *value, *endptr;
    int             itmp;

    DEBUGMSGTL(("netsnmp_ds_handle_config", "handling %s\n", token));

    for (drsp = netsnmp_ds_configs;
         drsp != NULL && strcasecmp(token, drsp->token) != 0;
         drsp = drsp->next);

    if (drsp != NULL) {
        DEBUGMSGTL(("netsnmp_ds_handle_config",
                    "setting: token=%s, type=%d, id=%s, which=%d\n",
                    drsp->token, drsp->type, stores[drsp->storeid],
                    drsp->which));

        switch (drsp->type) {
        case ASN_BOOLEAN:
            value = strtok(line, " \t\n");
            if (strcasecmp(value, "yes") == 0 || 
		strcasecmp(value, "true") == 0) {
                itmp = 1;
            } else if (strcasecmp(value, "no") == 0 ||
		       strcasecmp(value, "false") == 0) {
                itmp = 0;
            } else {
                itmp = strtol(value, &endptr, 10);
                if (*endptr != 0 || itmp < 0 || itmp > 1) {
                    config_perror("Should be yes|no|true|false|0|1");
		}
            }
            netsnmp_ds_set_boolean(drsp->storeid, drsp->which, itmp);
            DEBUGMSGTL(("netsnmp_ds_handle_config", "bool: %d\n", itmp));
            break;

        case ASN_INTEGER:
            value = strtok(line, " \t\n");
            itmp = strtol(value, &endptr, 10);
            if (*endptr != 0) {
                config_perror("Bad integer value");
	    } else {
                netsnmp_ds_set_int(drsp->storeid, drsp->which, itmp);
	    }
            DEBUGMSGTL(("netsnmp_ds_handle_config", "int: %d\n", itmp));
            break;

        case ASN_OCTET_STR:
            if (*line == '"') {
                copy_nword(line, buf, sizeof(buf));
                netsnmp_ds_set_string(drsp->storeid, drsp->which, buf);
            } else {
                netsnmp_ds_set_string(drsp->storeid, drsp->which, line);
            }
            DEBUGMSGTL(("netsnmp_ds_handle_config", "string: %s\n", line));
            break;

        default:
            snmp_log(LOG_ERR, "netsnmp_ds_handle_config: type %d (0x%02x)\n",
                     drsp->type, drsp->type);
            break;
        }
    } else {
        snmp_log(LOG_ERR, "netsnmp_ds_handle_config: no registration for %s\n",
                 token);
    }
}


int
netsnmp_ds_register_config(u_char type, const char *ftype, const char *token,
			   int storeid, int which)
{
    netsnmp_ds_read_config *drsp;

    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS    || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS || token == NULL) {
        return SNMPERR_GENERR;
    }

    if (netsnmp_ds_configs == NULL) {
        netsnmp_ds_configs = SNMP_MALLOC_TYPEDEF(netsnmp_ds_read_config);
        drsp = netsnmp_ds_configs;
    } else {
        for (drsp = netsnmp_ds_configs; drsp->next != NULL; drsp = drsp->next);
        drsp->next = SNMP_MALLOC_TYPEDEF(netsnmp_ds_read_config);
        drsp = drsp->next;
    }

    drsp->type    = type;
    drsp->ftype   = strdup(ftype);
    drsp->token   = strdup(token);
    drsp->storeid = storeid;
    drsp->which   = which;

    switch (type) {
    case ASN_BOOLEAN:
        register_config_handler(ftype, token, netsnmp_ds_handle_config, NULL,
                                "(1|yes|true|0|no|false)");
        break;

    case ASN_INTEGER:
        register_config_handler(ftype, token, netsnmp_ds_handle_config, NULL,
                                "integerValue");
        break;

    case ASN_OCTET_STR:
        register_config_handler(ftype, token, netsnmp_ds_handle_config, NULL,
                                "string");
        break;

    }
    return SNMPERR_SUCCESS;
}

int
netsnmp_ds_register_premib(u_char type, const char *ftype, const char *token,
			   int storeid, int which)
{
    netsnmp_ds_read_config *drsp;

    if (storeid < 0 || storeid >= NETSNMP_DS_MAX_IDS    || 
	which   < 0 || which   >= NETSNMP_DS_MAX_SUBIDS || token == NULL) {
        return SNMPERR_GENERR;
    }

    if (netsnmp_ds_configs == NULL) {
        netsnmp_ds_configs = SNMP_MALLOC_TYPEDEF(netsnmp_ds_read_config);
        drsp = netsnmp_ds_configs;
    } else {
        for (drsp = netsnmp_ds_configs; drsp->next != NULL; drsp = drsp->next);
        drsp->next = SNMP_MALLOC_TYPEDEF(netsnmp_ds_read_config);
        drsp = drsp->next;
    }

    drsp->type    = type;
    drsp->ftype   = strdup(ftype);
    drsp->token   = strdup(token);
    drsp->storeid = storeid;
    drsp->which   = which;

    switch (type) {
    case ASN_BOOLEAN:
        register_prenetsnmp_mib_handler(ftype, token, netsnmp_ds_handle_config,
                                        NULL, "(1|yes|true|0|no|false)");
        break;

    case ASN_INTEGER:
        register_prenetsnmp_mib_handler(ftype, token, netsnmp_ds_handle_config,
                                        NULL, "integerValue");
        break;

    case ASN_OCTET_STR:
        register_prenetsnmp_mib_handler(ftype, token, netsnmp_ds_handle_config,
                                        NULL, "string");
        break;

    }
    return SNMPERR_SUCCESS;
}

void
netsnmp_ds_shutdown()
{
    netsnmp_ds_read_config *drsp;
    int             i, j;

    for (drsp = netsnmp_ds_configs; drsp; drsp = netsnmp_ds_configs) {
        netsnmp_ds_configs = drsp->next;

        unregister_config_handler(drsp->ftype, drsp->token);
	if (drsp->ftype != NULL) {
	    free(drsp->ftype);
	}
	if (drsp->token != NULL) {
	    free(drsp->token);
	}
        free(drsp);
    }

    for (i = 0; i < NETSNMP_DS_MAX_IDS; i++) {
        for (j = 0; j < NETSNMP_DS_MAX_SUBIDS; j++) {
            if (netsnmp_ds_strings[i][j] != NULL) {
                free(netsnmp_ds_strings[i][j]);
                netsnmp_ds_strings[i][j] = NULL;
            }
        }
    }
}
