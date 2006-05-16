/** allows overriding of a given oid with a new type and value */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"

typedef struct override_data_s {
    int             type;
    void           *value;
    size_t          value_len;
} override_data;

/** @todo: (optionally) save values persistently when configured for
 *  read-write */
int
override_handler(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{

    override_data  *data = handler->myvoid;
    if (!data) {
        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
        return SNMP_ERR_NOERROR;
    }

    switch (reqinfo->mode) {
    case MODE_GET:
        DEBUGMSGTL(("override", "overriding oid "));
        DEBUGMSGOID(("override", requests->requestvb->name,
                     requests->requestvb->name_length));
        DEBUGMSG(("override", "\n"));
        snmp_set_var_typed_value(requests->requestvb, data->type,
                                 (u_char *) data->value, data->value_len);
        break;

    default:
        snmp_log(LOG_ERR, "unsupported mode in override handler");
        break;
    }
    return SNMP_ERR_NOERROR;
}

#define MALLOC_OR_DIE(x) \
  thedata->value = malloc(x); \
  thedata->value_len = x; \
  if (!thedata->value) { \
      free(thedata); \
      config_perror("memory allocation failure"); \
      return; \
  }

void
netsnmp_parse_override(const char *token, char *line)
{
    char           *cp;
    char            buf[SNMP_MAXBUF];
    int             readwrite = 0;
    oid             oidbuf[MAX_OID_LEN];
    size_t          oidbuf_len = sizeof(oidbuf);
    int             type;
    override_data  *thedata;
    netsnmp_handler_registration *the_reg;

    cp = copy_nword(line, buf, sizeof(buf) - 1);
    if (strcmp(buf, "-rw") == 0) {
        readwrite = 1;
        cp = copy_nword(cp, buf, sizeof(buf) - 1);
    }

    if (!cp) {
        config_perror("no oid specified");
        return;
    }

    if (!snmp_parse_oid(buf, oidbuf, &oidbuf_len)) {
        config_perror("illegal oid");
        return;
    }
    cp = copy_nword(cp, buf, sizeof(buf) - 1);

    if (!cp && strcmp(buf, "null") != 0) {
        config_perror("no variable value specified");
        return;
    }

    type = se_find_value_in_slist("asntypes", buf);
    if (!type) {
        config_perror("unknown type specified");
        return;
    }

    if (cp)
        copy_nword(cp, buf, sizeof(buf) - 1);
    else
        buf[0] = 0;

    thedata = SNMP_MALLOC_TYPEDEF(override_data);
    thedata->type = type;
    if (!thedata) {
        config_perror("memory allocation failure");
        return;
    }

    switch (type) {
    case ASN_INTEGER:
        MALLOC_OR_DIE(sizeof(long));
        *((long *) thedata->value) = strtol(buf, NULL, 0);
        break;

    case ASN_COUNTER:
    case ASN_UNSIGNED:
        MALLOC_OR_DIE(sizeof(u_long));
        *((u_long *) thedata->value) = strtoul(buf, NULL, 0);
        break;

    case ASN_OCTET_STR:
    case ASN_BIT_STR:
        if (buf[0] == '0' && buf[1] == 'x') {
            /*
             * hex 
             */
            thedata->value_len =
                hex_to_binary2(buf + 2, strlen(buf) - 2,
                               (char **) &thedata->value);
        } else {
            thedata->value = strdup(buf);
            thedata->value_len = strlen(buf);
        }
        break;

    case ASN_OBJECT_ID:
        read_config_read_objid(buf, (oid **) & thedata->value,
                               &thedata->value_len);
        break;

    case ASN_NULL:
        thedata->value_len = 0;
        break;

    default:
        config_perror("illegal/unsupported type specified");
        return;
    }

    if (!thedata->value && thedata->type != ASN_NULL) {
        config_perror("memory allocation failure");
        free(thedata);
        return;
    }

    the_reg = SNMP_MALLOC_TYPEDEF(netsnmp_handler_registration);
    if (!the_reg) {
        config_perror("memory allocation failure");
        free(thedata);
        return;
    }

    the_reg->priority = 255;
    the_reg->modes = (readwrite) ? HANDLER_CAN_RWRITE : HANDLER_CAN_RONLY;
    the_reg->handler =
        netsnmp_create_handler("override", override_handler);
    memdup((u_char **) & the_reg->rootoid, (const u_char *) oidbuf,
           oidbuf_len * sizeof(oid));
    the_reg->rootoid_len = oidbuf_len;
    if (!the_reg->rootoid) {
        config_perror("memory allocation failure");
        free(thedata);
        return;
    }
    the_reg->handler->myvoid = thedata;

    if (netsnmp_register_instance(the_reg)) {
        config_perror("oid registration failed within the agent");
        SNMP_FREE(thedata->value);
        free(thedata);
        return;
    }
}


void
init_override(void)
{

    snmpd_register_config_handler("override", netsnmp_parse_override, NULL,     /* XXX: free func */
                                  "[-rw] mibnode type value");
}
