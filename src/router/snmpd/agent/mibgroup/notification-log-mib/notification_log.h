#ifndef NOTIFICATION_LOG_H
#define NOTIFICATION_LOG_H
#include <net-snmp/agent/agent_handler.h>

#ifdef __cplusplus
extern          "C" {
#endif
/*
 * function declarations 
 */
void            init_notification_log(void);
Netsnmp_Node_Handler nlmLogTable_handler;
Netsnmp_Node_Handler nlmLogVariableTable_handler;

void
                log_notification(netsnmp_pdu *pdu, netsnmp_transport *transport);

/*
 * column number definitions for table nlmLogTable 
 */
#define COLUMN_NLMLOGINDEX		1
#define COLUMN_NLMLOGTIME		2
#define COLUMN_NLMLOGDATEANDTIME		3
#define COLUMN_NLMLOGENGINEID		4
#define COLUMN_NLMLOGENGINETADDRESS		5
#define COLUMN_NLMLOGENGINETDOMAIN		6
#define COLUMN_NLMLOGCONTEXTENGINEID		7
#define COLUMN_NLMLOGCONTEXTNAME		8
#define COLUMN_NLMLOGNOTIFICATIONID		9

/*
 * column number definitions for table nlmLogVariableTable 
 */
#define COLUMN_NLMLOGVARIABLEINDEX		1
#define COLUMN_NLMLOGVARIABLEID		2
#define COLUMN_NLMLOGVARIABLEVALUETYPE		3
#define COLUMN_NLMLOGVARIABLECOUNTER32VAL		4
#define COLUMN_NLMLOGVARIABLEUNSIGNED32VAL		5
#define COLUMN_NLMLOGVARIABLETIMETICKSVAL		6
#define COLUMN_NLMLOGVARIABLEINTEGER32VAL		7
#define COLUMN_NLMLOGVARIABLEOCTETSTRINGVAL		8
#define COLUMN_NLMLOGVARIABLEIPADDRESSVAL		9
#define COLUMN_NLMLOGVARIABLEOIDVAL		10
#define COLUMN_NLMLOGVARIABLECOUNTER64VAL		11
#define COLUMN_NLMLOGVARIABLEOPAQUEVAL		12

#ifdef __cplusplus
}
#endif

#endif                          /* NOTIFICATION_LOG_H */
