/*
 *  Host Resources MIB - network device group implementation - hr_network.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef hpux11
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif

#include "host_res.h"
#include "mibII/interfaces.h"
#include "hr_network.h"


        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

void            Init_HR_Network(void);
int             Get_Next_HR_Network(void);
void            Save_HR_Network_Info(void);

const char     *describe_networkIF(int);
int             network_status(int);
int             network_errors(int);
int             header_hrnet(struct variable *, oid *, size_t *, int,
                             size_t *, WriteMethod **);

#define HRN_MONOTONICALLY_INCREASING

        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

#define	HRNET_IFINDEX		1

struct variable4 hrnet_variables[] = {
    {HRNET_IFINDEX, ASN_INTEGER, RONLY, var_hrnet, 2, {1, 1}}
};
oid             hrnet_variables_oid[] = { 1, 3, 6, 1, 2, 1, 25, 3, 4 };


void
init_hr_network(void)
{
    init_device[HRDEV_NETWORK] = Init_HR_Network;
    next_device[HRDEV_NETWORK] = Get_Next_HR_Network;
    save_device[HRDEV_NETWORK] = Save_HR_Network_Info;
#ifdef HRN_MONOTONICALLY_INCREASING
    dev_idx_inc[HRDEV_NETWORK] = 1;
#endif

    device_descr[HRDEV_NETWORK] = describe_networkIF;
    device_status[HRDEV_NETWORK] = network_status;
    device_errors[HRDEV_NETWORK] = network_errors;

    REGISTER_MIB("host/hr_network", hrnet_variables, variable4,
                 hrnet_variables_oid);
}

/*
 * header_hrnet(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */

int
header_hrnet(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
#define HRNET_ENTRY_NAME_LENGTH	11
    oid             newname[MAX_OID_LEN];
    int             net_idx;
    int             result;
    int             LowIndex = -1;

    DEBUGMSGTL(("host/hr_network", "var_hrnet: "));
    DEBUGMSGOID(("host/hr_network", name, *length));
    DEBUGMSG(("host/hr_network", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    /*
     * Find "next" net entry 
     */

    Init_HR_Network();
    for (;;) {
        net_idx = Get_Next_HR_Network();
        if (net_idx == -1)
            break;
        newname[HRNET_ENTRY_NAME_LENGTH] = net_idx;
        result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
        if (exact && (result == 0)) {
            LowIndex = net_idx;
            break;
        }
        if (!exact && (result < 0) &&
            (LowIndex == -1 || net_idx < LowIndex)) {
            LowIndex = net_idx;
#ifdef HRN_MONOTONICALLY_INCREASING
            break;
#endif
        }
    }

    if (LowIndex == -1) {
        DEBUGMSGTL(("host/hr_network", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[HRNET_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname,
           (vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("host/hr_network", "... get net stats "));
    DEBUGMSGOID(("host/hr_network", name, *length));
    DEBUGMSG(("host/hr_network", "\n"));
    return LowIndex;
}


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/


u_char         *
var_hrnet(struct variable * vp,
          oid * name,
          size_t * length,
          int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             net_idx;

    net_idx = header_hrnet(vp, name, length, exact, var_len, write_method);
    if (net_idx == MATCH_FAILED)
        return NULL;


    switch (vp->magic) {
    case HRNET_IFINDEX:
        long_return = net_idx & ((1 << HRDEV_TYPE_SHIFT) - 1);
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hrnet\n",
                    vp->magic));
    }
    return NULL;
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/


#ifndef solaris2
static short    HRN_index;
#endif
#ifdef hpux11
static char     HRN_name[MAX_PHYSADDR_LEN];
static nmapi_phystat HRN_ifnet;
#define M_Interface_Scan_Next(a, b, c, d)	Interface_Scan_Next(a, b, c)
#else                           /* hpux11 */
static char     HRN_name[16];
static struct ifnet HRN_ifnet;
#define M_Interface_Scan_Next(a, b, c, d)	Interface_Scan_Next(a, b, c, d)
#endif

#ifdef hpux11
static char     HRN_savedName[MAX_PHYSADDR_LEN];
#else
static char     HRN_savedName[16];
#endif
static u_short  HRN_savedFlags;
static int      HRN_savedErrors;


void
Init_HR_Network(void)
{
#ifndef solaris2
    Interface_Scan_Init();
#endif
}

int
Get_Next_HR_Network(void)
{
#ifndef solaris2
    if (M_Interface_Scan_Next(&HRN_index, HRN_name, &HRN_ifnet, NULL))
        return (HRDEV_NETWORK << HRDEV_TYPE_SHIFT) + HRN_index;
    else
#endif
        return -1;
}

void
Save_HR_Network_Info(void)
{
    strcpy(HRN_savedName, HRN_name);
#ifdef hpux11
    HRN_savedFlags = HRN_ifnet.if_entry.ifOper;
    HRN_savedErrors = HRN_ifnet.if_entry.ifInErrors +
        HRN_ifnet.if_entry.ifOutErrors;
#else                           /* hpux11 */
    HRN_savedFlags = HRN_ifnet.if_flags;
    HRN_savedErrors = HRN_ifnet.if_ierrors + HRN_ifnet.if_oerrors;
#endif                          /* hpux11 */
}


const char     *
describe_networkIF(int idx)
{
    static char     string[100];

    snprintf(string, sizeof(string)-1, "network interface %s", HRN_savedName);
    string[ sizeof(string)-1 ] = 0;
    return string;
}


int
network_status(int idx)
{
#ifdef hpux11
    if (HRN_savedFlags == LINK_UP)
#else
    if (HRN_savedFlags & IFF_UP)
#endif
        return 2;               /* running */
    else
        return 5;               /* down */
}

int
network_errors(int idx)
{
    return HRN_savedErrors;
}
