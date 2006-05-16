#include <net-snmp/net-snmp-config.h>

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
#if HAVE_STRING_H
#include <string.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "struct.h"
#include "versioninfo.h"
#include "util_funcs.h"

void
init_versioninfo(void)
{

    /*
     * define the structure we're going to ask the agent to register our
     * information at 
     */
    struct variable2 extensible_version_variables[] = {
        {MIBINDEX, ASN_INTEGER, RONLY, var_extensible_version, 1,
         {MIBINDEX}},
        {VERTAG, ASN_OCTET_STR, RONLY, var_extensible_version, 1,
         {VERTAG}},
        {VERDATE, ASN_OCTET_STR, RONLY, var_extensible_version, 1,
         {VERDATE}},
        {VERCDATE, ASN_OCTET_STR, RONLY, var_extensible_version, 1,
         {VERCDATE}},
        {VERIDENT, ASN_OCTET_STR, RONLY, var_extensible_version, 1,
         {VERIDENT}},
        {VERCONFIG, ASN_OCTET_STR, RONLY, var_extensible_version, 1,
         {VERCONFIG}},
        {VERCLEARCACHE, ASN_INTEGER, RWRITE, var_extensible_version, 1,
         {VERCLEARCACHE}},
        {VERUPDATECONFIG, ASN_INTEGER, RWRITE, var_extensible_version, 1,
         {VERUPDATECONFIG}},
        {VERRESTARTAGENT, ASN_INTEGER, RWRITE, var_extensible_version, 1,
         {VERRESTARTAGENT}},
        {VERSAVEPERSISTENT, ASN_INTEGER, RWRITE, var_extensible_version, 1,
         {VERSAVEPERSISTENT}},
        {VERDEBUGGING, ASN_INTEGER, RWRITE, var_extensible_version, 1,
         {VERDEBUGGING}}
    };

    /*
     * Define the OID pointer to the top of the mib tree that we're
     * registering underneath 
     */
    oid             version_variables_oid[] =
        { UCDAVIS_MIB, VERSIONMIBNUM };

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("ucd-snmp/versioninfo", extensible_version_variables,
                 variable2, version_variables_oid);

}


u_char         *
var_extensible_version(struct variable *vp,
                       oid * name,
                       size_t * length,
                       int exact,
                       size_t * var_len, WriteMethod ** write_method)
{

    static long     long_ret;
    static char     errmsg[300];
    char           *cptr;
    time_t          curtime;
#ifdef CONFIGURE_OPTIONS
    static char     config_opts[] = CONFIGURE_OPTIONS;
#endif

    DEBUGMSGTL(("ucd-snmp/versioninfo", "var_extensible_version: "));
    DEBUGMSGOID(("ucd-snmp/versioninfo", name, *length));
    DEBUGMSG(("ucd-snmp/versioninfo", " %d\n", exact));

    if (header_generic(vp, name, length, exact, var_len, write_method))
        return (NULL);

    switch (vp->magic) {
    case MIBINDEX:
        long_ret = name[8];
        return ((u_char *) (&long_ret));
    case VERTAG:
        sprintf(errmsg, NetSnmpVersionInfo);
        *var_len = strlen(errmsg);
        return ((u_char *) errmsg);
    case VERDATE:
        sprintf(errmsg, "$Date: 2004/06/20 21:54:28 $");
        *var_len = strlen(errmsg);
        return ((u_char *) errmsg);
    case VERCDATE:
        curtime = time(NULL);
        cptr = ctime(&curtime);
        sprintf(errmsg, cptr);
        *var_len = strlen(errmsg) - 1;
        return ((u_char *) errmsg);
    case VERIDENT:
        sprintf(errmsg,
                "$Id: versioninfo.c,v 1.1.2.1 2004/06/20 21:54:28 nikki Exp $");
        *var_len = strlen(errmsg);
        return ((u_char *) errmsg);
    case VERCONFIG:
#ifdef CONFIGURE_OPTIONS
        *var_len = strlen(config_opts);
        if (*var_len > 1024)
            *var_len = 1024;    /* mib imposed restriction */
        return (u_char *) config_opts;
#else
        sprintf(errmsg, "");
        *var_len = strlen(errmsg);
        return ((u_char *) errmsg);
#endif
    case VERCLEARCACHE:
        *write_method = clear_cache;
        long_ret = 0;
        return ((u_char *) & long_ret);
    case VERUPDATECONFIG:
        *write_method = update_hook;
        long_ret = 0;
        return ((u_char *) & long_ret);
    case VERRESTARTAGENT:
        *write_method = restart_hook;
        long_ret = 0;
        return ((u_char *) & long_ret);
    case VERSAVEPERSISTENT:
        *write_method = save_persistent;
        long_ret = 0;
        return ((u_char *) & long_ret);
    case VERDEBUGGING:
        *write_method = debugging_hook;
        long_ret = snmp_get_do_debugging();
        return ((u_char *) & long_ret);
    }
    return NULL;
}

int
update_hook(int action,
            u_char * var_val,
            u_char var_val_type,
            size_t var_val_len,
            u_char * statP, oid * name, size_t name_len)
{
    long            tmp = 0;

    if (var_val_type != ASN_INTEGER) {
        snmp_log(LOG_ERR, "Wrong type != int\n");
        return SNMP_ERR_WRONGTYPE;
    }
    tmp = *((long *) var_val);
    if (tmp == 1 && action == COMMIT) {
        update_config();
    }
    return SNMP_ERR_NOERROR;
}

int
debugging_hook(int action,
               u_char * var_val,
               u_char var_val_type,
               size_t var_val_len,
               u_char * statP, oid * name, size_t name_len)
{
    long            tmp = 0;

    if (var_val_type != ASN_INTEGER) {
        DEBUGMSGTL(("versioninfo", "Wrong type != int\n"));
        return SNMP_ERR_WRONGTYPE;
    }
    tmp = *((long *) var_val);
    if (action == COMMIT) {
        snmp_set_do_debugging(tmp);
    }
    return SNMP_ERR_NOERROR;
}

int
save_persistent(int action,
               u_char * var_val,
               u_char var_val_type,
               size_t var_val_len,
               u_char * statP, oid * name, size_t name_len)
{
    long            tmp = 0;

    if (var_val_type != ASN_INTEGER) {
        DEBUGMSGTL(("versioninfo", "Wrong type != int\n"));
        return SNMP_ERR_WRONGTYPE;
    }
    tmp = *((long *) var_val);
    if (action == COMMIT) {
        snmp_store(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                         NETSNMP_DS_LIB_APPTYPE));
    }
    return SNMP_ERR_NOERROR;
}
