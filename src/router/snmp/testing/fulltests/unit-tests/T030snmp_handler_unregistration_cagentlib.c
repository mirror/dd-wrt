/*
 * HEADER Testing SNMP handler registration and unregistration.
 * See also https://github.com/net-snmp/net-snmp/issues/983.
 */

static const oid Oid[] = { 1, 3, 6, 1, 4, 1, 8072, 2, 4, 1, 1, 2, 0 };
netsnmp_handler_registration *handler;
netsnmp_watcher_info *watcher;
int res;

init_snmp("snmp");
handler = netsnmp_create_handler_registration("handler", NULL, Oid,
                                              OID_LENGTH(Oid), HANDLER_CAN_RWRITE);
OK(handler != NULL, "Handler registration");
if (!handler)
    goto shutdown;

handler->contextName = strdup("contextName");
watcher = netsnmp_create_watcher_info(NULL, 0, ASN_INTEGER, WATCHER_FIXED_SIZE);
OK(watcher, "Watcher creation");
if (!watcher)
    goto unregister;
res = netsnmp_register_watched_instance2(handler, watcher);
OK(res == MIB_REGISTERED_OK, "Register watched instance");

unregister:
netsnmp_unregister_handler(handler);

shutdown:
snmp_shutdown("snmp");
