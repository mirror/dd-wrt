/*
 *   perfstat() interface
 *     e.g. AIX
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/hardware/cpu.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <libperfstat.h>

void _cpu_copy_stats( netsnmp_cpu_info *cpu );

    /*
     * Initialise the list of CPUs on the system
     *   (including descriptions)
     */
void init_cpu_perfstat( void ) {
    int                   i;
    perfstat_id_t         name;
    perfstat_cpu_t       *cs2;
    netsnmp_cpu_info     *cpu = netsnmp_cpu_get_byIdx( -1, 1 );
    strcpy(cpu->name, "Overall CPU statistics");

    cpu_num = perfstat_cpu( NULL, NULL, sizeof(perfstat_cpu_t), 0 );
    cs2 = malloc( cpu_num*sizeof(perfstat_cpu_t));
 
    strcpy( name.name, "");
    if (perfstat_cpu(&name, cs2, sizeof(perfstat_cpu_t), cpu_num) > 0) {
        for ( i = 0; i < cpu_num; i++ ) {
            cpu = netsnmp_cpu_get_byIdx( i, 1 );
            sprintf( cpu->name, cs2[i].name);
        }
    }
    free(cs2);
}


    /*
     * Load the latest CPU usage statistics
     */
int netsnmp_cpu_arch_load( netsnmp_cache *cache, void *magic ) {
    int                     i,n;
    perfstat_id_t           name;
    perfstat_cpu_total_t    cs;
    perfstat_cpu_t         *cs2;
    perfstat_memory_total_t ms;
    netsnmp_cpu_info       *cpu = netsnmp_cpu_get_byIdx( -1, 0 );

    if (perfstat_cpu_total((perfstat_id_t *)NULL, &cs,
                     sizeof(perfstat_cpu_total_t), 1) > 0) {

        /* Returns 'u_longlong_t' statistics */
        cpu->user_ticks = (unsigned long)cs.user;
        cpu->sys_ticks  = (unsigned long)cs.sys;
        cpu->wait_ticks = (unsigned long)cs.wait;
        cpu->idle_ticks = (unsigned long)cs.idle;
        /* intrpt_ticks, sirq_ticks, nice_ticks unused */
    
        /*
         * Interrupt/Context Switch statistics
         *   XXX - Do these really belong here ?
         */
        cpu->pageIn       = (unsigned long)cs.sysread;
        cpu->pageOut      = (unsigned long)cs.syswrite;
        cpu->nInterrupts  = (unsigned long)cs.devintrs + cs.softintrs;
        cpu->nCtxSwitches = (unsigned long)cs.pswitch;
    }

    if (perfstat_memory_total((perfstat_id_t *)NULL, &ms,
                     sizeof(perfstat_memory_total_t), 1) > 0) {
        cpu->swapIn  = (unsigned long)ms.pgspins;
        cpu->swapOut = (unsigned long)ms.pgspouts;
    }


    /*
     * Per-CPU statistics
     */
    n   = cs.ncpus;   /* XXX - Compare against cpu_num */
    cs2 = malloc( n*sizeof(perfstat_cpu_t));
    strcpy( name.name, "");
    if (perfstat_cpu(&name, cs2, sizeof(perfstat_cpu_t), n) > 0) {
        for ( i = 0; i < n; i++ ) {
            cpu = netsnmp_cpu_get_byIdx( i, 0 );
            cpu->user_ticks = (unsigned long)cs2[i].user;
            cpu->sys_ticks  = (unsigned long)cs2[i].sys;
            cpu->wait_ticks = (unsigned long)cs2[i].wait;
            cpu->idle_ticks = (unsigned long)cs2[i].idle;
            cpu->pageIn     = (unsigned long)cs2[i].sysread;
            cpu->pageOut    = (unsigned long)cs2[i].syswrite;
            cpu->nCtxSwitches = (unsigned long)cs2[i].pswitch;
            /* Interrupt stats only apply overall, not per-CPU */
        }
    } else {
        _cpu_copy_stats( cpu );
    }
    free(cs2);

    return 0;
}
