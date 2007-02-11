/*
 * watched.c
 * $Id: watched.c,v 1.1 2005/04/20 18:03:47 rstory Exp $
 *
 */
/**  @example watched.c
 *  These examples creates some scalar registrations that allows
 *  some simple variables to be accessed via SNMP.  In a more
 *  realistic example, it is likely that these variables would also be
 *  manipulated in other ways outside of SNMP gets/sets.
 */

/*
 * start by including the appropriate header files 
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

void init_watched_string(void);

void init_watched(void)
{
    init_watched_string();
}

void init_watched_string(void)
{
    /*
     * the storage for our string. It must be static or allocated.
     * we use static here for simplicity.
     */
    static char my_string[256] = "So long, and thanks for all the fish!";

    /*
     * the OID we want to register our string at.  This should be a
     * fully qualified instance.  In our case, it's a scalar at:
     * NET-SNMP-EXAMPLES-MIB::netSnmpExampleString.0  (note the trailing
     *  0 which is required for any instantiation of any scalar object) 
     */
    oid             my_registration_oid[] =
        { 1, 3, 6, 1, 4, 1, 8072, 2, 1, 3, 0 };

    /*
     * variables needed for registration
     */
    netsnmp_handler_registration *reginfo;
    netsnmp_watcher_info *watcher_info;
    int watcher_flags;

    /*
     * a debugging statement.  Run the agent with -Dexample_string_instance
     * to see the output of this debugging statement. 
     */
    DEBUGMSGTL(("example_string_instance",
                "Initalizing example string instance.  Default value = %s\n",
                my_string));

    /*
     * create the registration info for our string. If you want to
     *
     * If we wanted a callback when the value was retrieved or set
     * (even though the details of doing this are handled for you),
     * you could change the NULL pointer below to a valid handler
     * function.
     *
     * Change RWRITE to RONLY for a read-only string.
     */
    reginfo = netsnmp_create_handler_registration("my example string", NULL,
                                                  my_registration_oid,
                                                  OID_LENGTH(my_registration_oid),
                                                  HANDLER_CAN_RWRITE);
                                                  
    /*
     * the two options for a string watcher are:
     *   fixed size string (length never changes)
     *   variable size (length can be 0 - MAX, for some MAX)
     *
     * we'll use a variable length string.
     */
    watcher_flags = WATCHER_MAX_SIZE;

    /*
     * create the watcher info for our string, and set the max size.
     */
    watcher_info =
        netsnmp_create_watcher_info(my_string, strlen(my_string),
                                    ASN_OCTET_STR, watcher_flags);
    watcher_info->max_size = sizeof(my_string);

    /*
     * the line below registers our "my_string" variable above as
     * accessible and makes it writable. 
     * 
     * If we wanted a callback when the value was retrieved or set
     * (even though the details of doing this are handled for you),
     * you could change the NULL pointer below to a valid handler
     * function. 
     */
    netsnmp_register_watched_instance(reginfo, watcher_info);

    DEBUGMSGTL(("example_string_instance",
                "Done initalizing example string instance\n"));
}
