/*
 *  System MIB group implementation - system.c
 *
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <ctype.h>
#if HAVE_UTSNAME_H
#include <utsname.h>
#else
#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"
#include "system_mib.h"
#include "struct.h"
#include "sysORTable.h"


        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#define SYS_STRING_LEN	256
char            version_descr[SYS_STRING_LEN] = NETSNMP_VERS_DESC;
char            sysContact[SYS_STRING_LEN] = NETSNMP_SYS_CONTACT;
char            sysName[SYS_STRING_LEN] = NETSNMP_SYS_NAME;
char            sysLocation[SYS_STRING_LEN] = NETSNMP_SYS_LOC;
oid             sysObjectID[MAX_OID_LEN];
size_t          sysObjectIDLength;

extern oid      version_sysoid[];
extern int      version_sysoid_len;

char            oldversion_descr[SYS_STRING_LEN];
char            oldsysContact[SYS_STRING_LEN];
char            oldsysName[SYS_STRING_LEN];
char            oldsysLocation[SYS_STRING_LEN];

int             sysServices = 72;
int             sysServicesConfiged = 0;

extern oid      version_id[];
extern int      version_id_len;

static int      sysContactSet = 0, sysLocationSet = 0, sysNameSet = 0;

WriteMethod     writeSystem;
int             header_system(struct variable *, oid *, size_t *, int,
                              size_t *, WriteMethod **);

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
static void     windowsOSVersionString(char [], size_t);
#endif

        /*********************
	 *
	 *  snmpd.conf config parsing
	 *
	 *********************/

void
system_parse_config_sysdescr(const char *token, char *cptr)
{
    char            tmpbuf[1024];

    if (strlen(cptr) >= sizeof(version_descr)) {
        snprintf(tmpbuf,
                 sizeof(tmpbuf),
                 "sysdescr token too long (must be < %lu):\n\t%s",
                 (unsigned long)sizeof(version_descr),
                 cptr);
        config_perror(tmpbuf);
    } else if (strcmp(cptr, "\"\"") == 0) {
        version_descr[0] = '\0';
    } else {
        strcpy(version_descr, cptr);
    }
}

void
system_parse_config_sysloc(const char *token, char *cptr)
{
    char            tmpbuf[1024];

    if (strlen(cptr) >= sizeof(sysLocation)) {
        snprintf(tmpbuf, 1024,
                 "syslocation token too long (must be < %lu):\n\t%s",
                 (unsigned long)sizeof(sysLocation), cptr);
        config_perror(tmpbuf);
    }

    if (strcmp(token, "psyslocation") == 0) {
        if (sysLocationSet < 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- the sysLocation
             * is already configured read-only.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysLocation.0\n");
            return;
        } else {
            sysLocationSet++;
        }
    } else {
        if (sysLocationSet > 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- we already read a
             * persistent value of sysLocation, which we should ignore in
             * favour of this one.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysLocation.0\n");
            /*
             * Fall through and copy in this value.  
             */
        }
        sysLocationSet = -1;
    }

    if (strcmp(cptr, "\"\"") == 0) {
        sysLocation[0] = '\0';
    } else if (strlen(cptr) < sizeof(sysLocation)) {
        strcpy(sysLocation, cptr);
    }
}

void
system_parse_config_syscon(const char *token, char *cptr)
{
    char            tmpbuf[1024];

    if (strlen(cptr) >= sizeof(sysContact)) {
        snprintf(tmpbuf, 1024,
                 "syscontact token too long (must be < %lu):\n\t%s",
                 (unsigned long)sizeof(sysContact), cptr);
        config_perror(tmpbuf);
    }

    if (strcmp(token, "psyscontact") == 0) {
        if (sysContactSet < 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- the sysContact
             * is already configured read-only.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysContact.0\n");
            return;
        } else {
            sysContactSet++;
        }
    } else {
        if (sysContactSet > 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- we already read a
             * persistent value of sysContact, which we should ignore in favour
             * of this one.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysContact.0\n");
            /*
             * Fall through and copy in this value.  
             */
        }
        sysContactSet = -1;
    }

    if (strcmp(cptr, "\"\"") == 0) {
        sysContact[0] = '\0';
    } else if (strlen(cptr) < sizeof(sysContact)) {
        strcpy(sysContact, cptr);
    }
}

void
system_parse_config_sysname(const char *token, char *cptr)
{
    char            tmpbuf[1024];

    if (strlen(cptr) >= sizeof(sysName)) {
        snprintf(tmpbuf, 1024,
                 "sysname token too long (must be < %lu):\n\t%s",
                 (unsigned long)sizeof(sysName), cptr);
        config_perror(tmpbuf);
    }

    if (strcmp(token, "psysname") == 0) {
        if (sysNameSet < 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- the sysName
             * is already configured read-only.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysName.0\n");
            return;
        } else {
            sysNameSet++;
        }
    } else {
        if (sysNameSet > 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- we already read a
             * persistent value of sysName, which we should ignore in favour
             * of this one.  
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only sysName.0\n");
            /*
             * Fall through and copy in this value.  
             */
        }
        sysNameSet = -1;
    }

    if (strcmp(cptr, "\"\"") == 0) {
        sysName[0] = '\0';
    } else if (strlen(cptr) < sizeof(sysName)) {
        strcpy(sysName, cptr);
    }
}

void
system_parse_config_sysServices(const char *token, char *cptr)
{
    sysServices = atoi(cptr);
    sysServicesConfiged = 1;
}

void system_parse_config_sysObjectID(const char *token, char *cptr)
{
    char tmpbuf[1024];

    sysObjectIDLength = MAX_OID_LEN;
    if (!read_objid(cptr, sysObjectID, &sysObjectIDLength)) {
        snprintf(tmpbuf,
                 sizeof(tmpbuf),
                 "sysobjectid token not a parsable OID:\n\t%s",
                 cptr);
        config_perror(tmpbuf);
        memcpy(sysObjectID, version_sysoid, version_sysoid_len * sizeof(oid));
        sysObjectIDLength = version_sysoid_len;
    }
}


        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

/*
 * define the structure we're going to ask the agent to register our
 * information at 
 */
struct variable1 system_variables[] = {
    {VERSION_DESCR, ASN_OCTET_STR, RONLY, var_system, 1, {1}},
    {VERSIONID, ASN_OBJECT_ID, RONLY, var_system, 1, {2}},
    {UPTIME, ASN_TIMETICKS, RONLY, var_system, 1, {3}},
    {SYSCONTACT, ASN_OCTET_STR, RWRITE, var_system, 1, {4}},
    {SYSTEMNAME, ASN_OCTET_STR, RWRITE, var_system, 1, {5}},
    {SYSLOCATION, ASN_OCTET_STR, RWRITE, var_system, 1, {6}},
    {SYSSERVICES, ASN_INTEGER, RONLY, var_system, 1, {7}},
    {SYSORLASTCHANGE, ASN_TIMETICKS, RONLY, var_system, 1, {8}}
};
/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath 
 */
oid             system_variables_oid[] = { SNMP_OID_MIB2, 1 };
oid             system_module_oid[] = { SNMP_OID_SNMPMODULES, 1 };
int             system_module_oid_len =
    sizeof(system_module_oid) / sizeof(oid);
int             system_module_count = 0;

static int
system_store(int a, int b, void *c, void *d)
{
    char            line[SNMP_MAXBUF_SMALL];

    if (sysLocationSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psyslocation %s", sysLocation);
        snmpd_store_config(line);
    }
    if (sysContactSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psyscontact %s", sysContact);
        snmpd_store_config(line);
    }
    if (sysNameSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psysname %s", sysName);
        snmpd_store_config(line);
    }

    return 0;
}

void
init_system_mib(void)
{

#ifdef HAVE_UNAME
    struct utsname  utsName;

    uname(&utsName);
    snprintf(version_descr, sizeof(version_descr),
            "%s %s %s %s %s", utsName.sysname,
            utsName.nodename, utsName.release, utsName.version,
            utsName.machine);
    version_descr[ sizeof(version_descr)-1 ] = 0;
#else
#if HAVE_EXECV
    struct extensible extmp;

    /*
     * set default values of system stuff 
     */
    sprintf(extmp.command, "%s -a", UNAMEPROG);
    /*
     * setup defaults 
     */
    extmp.type = EXECPROC;
    extmp.next = NULL;
    exec_command(&extmp);
    strncpy(version_descr, extmp.output, sizeof(version_descr));
    version_descr[sizeof(version_descr) - 1] = 0;
    version_descr[strlen(version_descr) - 1] = 0;       /* chomp new line */
#else
#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
    windowsOSVersionString(version_descr, sizeof(version_descr));
#else
    strcpy(version_descr, "unknown");
#endif
#endif
#endif

#ifdef HAVE_GETHOSTNAME
    gethostname(sysName, sizeof(sysName));
#else
#ifdef HAVE_UNAME
    strncpy(sysName, utsName.nodename, sizeof(sysName));
#else
#if defined (HAVE_EXECV) && !defined (mingw32)
    sprintf(extmp.command, "%s -n", UNAMEPROG);
    /*
     * setup defaults 
     */
    extmp.type = EXECPROC;
    extmp.next = NULL;
    exec_command(&extmp);
    strncpy(sysName, extmp.output, sizeof(sysName));
    sysName[strlen(sysName) - 1] = 0;   /* chomp new line */
#else
    strcpy(sysName, "unknown");
#endif                          /* HAVE_EXECV */
#endif                          /* HAVE_UNAME */
#endif                          /* HAVE_GETHOSTNAME */

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
  {
    HKEY hKey;
    /* Default sysContact is the registered windows user */
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
       char registeredOwner[256] = "";
       DWORD registeredOwnerSz = 256;
       if (RegQueryValueEx(hKey, "RegisteredOwner", NULL, NULL, (LPBYTE)registeredOwner, &registeredOwnerSz) == ERROR_SUCCESS) {
          strcpy(sysContact, registeredOwner);
       }
       RegCloseKey(hKey);
    }
  }
#endif

    /* default sysObjectID */
    memcpy(sysObjectID, version_sysoid, version_sysoid_len * sizeof(oid));
    sysObjectIDLength = version_sysoid_len;

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/system", system_variables, variable1,
                 system_variables_oid);

    if (++system_module_count == 3)
        REGISTER_SYSOR_ENTRY(system_module_oid,
                             "The MIB module for SNMPv2 entities");

    sysContactSet = sysLocationSet = sysNameSet = 0;

    /*
     * register our config handlers 
     */
    snmpd_register_config_handler("sysdescr",
                                  system_parse_config_sysdescr, NULL,
                                  "description");
    snmpd_register_config_handler("syslocation",
                                  system_parse_config_sysloc, NULL,
                                  "location");
    snmpd_register_config_handler("syscontact", system_parse_config_syscon,
                                  NULL, "contact-name");
    snmpd_register_config_handler("sysname", system_parse_config_sysname,
                                  NULL, "node-name");
    snmpd_register_config_handler("psyslocation",
                                  system_parse_config_sysloc, NULL, NULL);
    snmpd_register_config_handler("psyscontact",
                                  system_parse_config_syscon, NULL, NULL);
    snmpd_register_config_handler("psysname", system_parse_config_sysname,
                                  NULL, NULL);
    snmpd_register_config_handler("sysservices",
                                  system_parse_config_sysServices, NULL,
                                  "NUMBER");
    snmpd_register_config_handler("sysobjectid",
                                  system_parse_config_sysObjectID, NULL,
                                  "OID");
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           system_store, NULL);

}


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

#ifdef USING_MIBII_SYSORTABLE_MODULE
extern struct timeval sysOR_lastchange;
#endif

u_char         *
var_system(struct variable *vp,
           oid * name,
           size_t * length,
           int exact, size_t * var_len, WriteMethod ** write_method)
{
    static u_long   ulret;

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic) {
    case VERSION_DESCR:
        *var_len = strlen(version_descr);
        return (u_char *) version_descr;
    case VERSIONID:
        *var_len = sysObjectIDLength * sizeof(sysObjectID[0]);
        return (u_char *)sysObjectID;
    case UPTIME:
        ulret = netsnmp_get_agent_uptime();
        return ((u_char *) & ulret);
    case SYSCONTACT:
        *var_len = strlen(sysContact);
        *write_method = writeSystem;
        return (u_char *) sysContact;
    case SYSTEMNAME:
        *var_len = strlen(sysName);
        *write_method = writeSystem;
        return (u_char *) sysName;
    case SYSLOCATION:
        *var_len = strlen(sysLocation);
        *write_method = writeSystem;
        return (u_char *) sysLocation;
    case SYSSERVICES:
#if NETSNMP_NO_DUMMY_VALUES
        if (!sysServicesConfiged)
            return NULL;
#endif
        long_return = sysServices;
        return (u_char *) & long_return;

#ifdef USING_MIBII_SYSORTABLE_MODULE
    case SYSORLASTCHANGE:
        ulret = netsnmp_timeval_uptime(&sysOR_lastchange);
        return ((u_char *) & ulret);
#endif

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_system\n",
                    vp->magic));
    }
    return NULL;
}



int
writeSystem(int action,
            u_char * var_val,
            u_char var_val_type,
            size_t var_val_len,
            u_char * statP, oid * name, size_t name_len)
{
    u_char         *cp;
    char           *buf = NULL, *oldbuf = NULL;
    int             count, *setvar = NULL;

    switch ((char) name[7]) {
    case VERSION_DESCR:
    case VERSIONID:
    case UPTIME:
        snmp_log(LOG_ERR, "Attempt to write to R/O OID\n");
        return SNMP_ERR_NOTWRITABLE;
    case SYSCONTACT:
        buf = sysContact;
        oldbuf = oldsysContact;
        setvar = &sysContactSet;
        break;
    case SYSTEMNAME:
        buf = sysName;
        oldbuf = oldsysName;
        setvar = &sysNameSet;
        break;
    case SYSLOCATION:
        buf = sysLocation;
        oldbuf = oldsysLocation;
        setvar = &sysLocationSet;
        break;
    case SYSSERVICES:
    case SYSORLASTCHANGE:
        snmp_log(LOG_ERR, "Attempt to write to R/O OID\n");
        return SNMP_ERR_NOTWRITABLE;
    default:
        return SNMP_ERR_GENERR; /* ??? */
    }

    switch (action) {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_OCTET_STR) {
            snmp_log(LOG_ERR, "not string\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(sysLocation) - 1) {
            snmp_log(LOG_ERR, "bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }

        for (cp = var_val, count = 0; count < (int) var_val_len;
             count++, cp++) {
            if (!isprint(*cp)) {
                snmp_log(LOG_ERR, "not print %x\n", *cp);
                return SNMP_ERR_WRONGVALUE;
            }
        }
        if (setvar != NULL && *setvar < 0) {
            /*
             * The object is set in a read-only configuration file.  
             */
            return SNMP_ERR_NOTWRITABLE;
        }
        break;

    case RESERVE2:             /* Allocate memory and similar resources */

        /*
         * Using static strings, so nothing needs to be done 
         */
        break;

    case ACTION:               /* Perform the SET action (if reversible) */

        /*
         * Save the old value, in case of UNDO 
         */
        strcpy(oldbuf, buf);
        memcpy(buf, var_val, var_val_len);
        buf[var_val_len] = 0;
        break;

    case UNDO:                 /* Reverse the SET action and free resources */

        strcpy(buf, oldbuf);
        oldbuf[0] = 0;
        break;

    case COMMIT:
        if (setvar != NULL) {
            *setvar = 1;
        }
        snmp_save_persistent(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
        (void) snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,
                                   SNMP_CALLBACK_STORE_DATA, NULL);
        snmp_clean_persistent(netsnmp_ds_get_string
                              (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));

    case FREE:                 /* Free any resources allocated */

        /*
         * No resources have been allocated, but "empty" the 'oldbuf' 
         */
        oldbuf[0] = 0;
        break;
    }
    return SNMP_ERR_NOERROR;
}                               /* end of writeSystem */

        /*********************
	 *
	 *  Internal implementation functions - None
	 *
	 *********************/

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
static void
windowsOSVersionString(char stringbuf[], size_t stringbuflen)
{
    /* copy OS version to string buffer in 'uname -a' format */
    OSVERSIONINFOEX osVersionInfo;
    BOOL gotOsVersionInfoEx;
    char windowsVersion[256] = "";
    char hostname[256] = "";
    char identifier[256] = "";
    DWORD identifierSz = 256;
    HKEY hKey;

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    gotOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osVersionInfo);
    if (gotOsVersionInfoEx == FALSE) {
       GetVersionEx((OSVERSIONINFO *)&osVersionInfo);
    }

    switch (osVersionInfo.dwPlatformId) {
       case VER_PLATFORM_WIN32_NT:
          if ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion == 2)) {
             strcat(windowsVersion, "Server 2003");
          } else if ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion == 1)) {
             strcat(windowsVersion, "XP");
          } else if ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion == 0)) {
             strcat(windowsVersion, "2000");
          } else if (osVersionInfo.dwMajorVersion <= 4) {
             strcat(windowsVersion, "NT");
          }
          if (gotOsVersionInfoEx == TRUE) {
             if (osVersionInfo.wProductType == VER_NT_WORKSTATION) {
                if (osVersionInfo.dwMajorVersion == 4) {
                   strcat(windowsVersion, " Workstation 4.0");
                } else if (osVersionInfo.wSuiteMask & VER_SUITE_PERSONAL) {
                   strcat(windowsVersion, " Home Edition");
                } else {
                   strcat(windowsVersion, " Professional");
                }
             } else if (osVersionInfo.wProductType == VER_NT_SERVER) {
                if ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion == 2)) {
                   if (osVersionInfo.wSuiteMask & VER_SUITE_DATACENTER) {
                      strcat(windowsVersion, " Datacenter Edition");
                   } else if (osVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
                      strcat(windowsVersion, " Enterprise Edition");
                   } else if (osVersionInfo.wSuiteMask == VER_SUITE_BLADE) {
                      strcat(windowsVersion, " Web Edition");
                   } else {
                      strcat(windowsVersion, " Standard Edition");
                   }
                } else if ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion == 0)) {
                   if (osVersionInfo.wSuiteMask & VER_SUITE_DATACENTER) {
                      strcat(windowsVersion, " Datacenter Server");
                   } else if (osVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
                      strcat(windowsVersion, " Advanced Server");
                   } else {
                      strcat(windowsVersion, " Server");
                   }
                } else {
                   if (osVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
                      strcat(windowsVersion, " Server 4.0, Enterprise Edition");
                   } else {
                      strcat(windowsVersion, " Server 4.0");
                   }
                }
             }
          } else {
             char productType[80];
             DWORD productTypeSz = 80;

             if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
                if (RegQueryValueEx(hKey, "ProductType", NULL, NULL, (LPBYTE) productType, &productTypeSz) == ERROR_SUCCESS) {
                   char versionStr[10];
                   if (strcmpi("WINNT", productType) == 0) {
                      strcat(windowsVersion, " Workstation");
                   } else if (strcmpi("LANMANNT", productType) == 0) {
                      strcat(windowsVersion, " Server");
                   } else if (strcmpi("SERVERNT", productType) == 0) {
                      strcat(windowsVersion, " Advanced Server");
                   }
                   sprintf(versionStr, " %d.%d", (int)osVersionInfo.dwMajorVersion, (int)osVersionInfo.dwMinorVersion);
                   strcat(windowsVersion, versionStr);
                }
                RegCloseKey(hKey);
             }
          }
          break;
       case VER_PLATFORM_WIN32_WINDOWS:
          if ((osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 90)) {
             strcat(windowsVersion, "ME");
          } else if ((osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 10)) {
             strcat(windowsVersion, "98");
             if (osVersionInfo.szCSDVersion[1] == 'A') {
                strcat(windowsVersion, " SE");
             }
          } else if ((osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 0)) {
             strcat(windowsVersion, "95");
             if ((osVersionInfo.szCSDVersion[1] == 'C') || (osVersionInfo.szCSDVersion[1] == 'B')) {
                strcat(windowsVersion, " OSR2");
             }
          }
          break;
    }

    gethostname(hostname, sizeof(hostname));

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
       RegQueryValueEx(hKey, "Identifier", NULL, NULL, (LPBYTE)&identifier, &identifierSz);
       RegCloseKey(hKey);
    }

    /* Output is made to look like results from uname -a */
    snprintf(stringbuf, stringbuflen,
            "Windows %s %d.%d.%d %s %s %s", hostname,
             (int)osVersionInfo.dwMajorVersion, (int)osVersionInfo.dwMinorVersion,
             (int)osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion,
             windowsVersion, identifier);
}
#endif /* WIN32 and HAVE_WIN32_PLATFORM_SDK or mingw32 */

