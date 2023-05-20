/* module to include the various ucd-snmp specific extension modules. */
config_require(ucd-snmp/proc)
config_require(ucd-snmp/versioninfo)
config_require(ucd-snmp/pass)
config_require(ucd-snmp/pass_persist)
config_require(ucd-snmp/disk_hw)
config_require(ucd-snmp/loadave)
config_require(agent/extend)
config_require(ucd-snmp/errormib)
config_require(ucd-snmp/file)
#if defined(HAVE_DLFCN_H) && defined(HAVE_DLOPEN)
config_require(ucd-snmp/dlmod)
#endif
config_require(ucd-snmp/proxy)
#ifdef HAVE_REGEX_H
config_require(ucd-snmp/logmatch)
#endif
config_require(ucd-snmp/memory)
config_require(ucd-snmp/vmstat)
config_add_mib(UCD-SNMP-MIB)
config_add_mib(UCD-DEMO-MIB)
