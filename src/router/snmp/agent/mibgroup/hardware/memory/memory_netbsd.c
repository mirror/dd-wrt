#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/hardware/memory.h>

#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/swap.h>

#if defined(HAVE_UVM_UVM_PARAM_H) && defined(HAVE_UVM_UVM_EXTERN_H)
#include <uvm/uvm_param.h>
#include <uvm/uvm_extern.h>
#elif defined(HAVE_VM_VM_PARAM_H) && defined(HAVE_VM_VM_EXTERN_H)
#include <vm/vm_param.h>
#include <vm/vm_extern.h>
#else
#error memory_netbsd1.c: Is this really a NetBSD system?
#endif

#ifdef SWAP_NSWAP
void swapinfo(long);
#endif

    /*
     * Load the latest memory usage statistics
     */
int netsnmp_mem_arch_load( netsnmp_cache *cache, void *magic ) {

    netsnmp_memory_info *mem;
    long           pagesize;

    struct uvmexp  uvmexp;
    int            uvmexp_size  = sizeof(uvmexp);
    int            uvmexp_mib[] = { CTL_VM, VM_UVMEXP };

    struct vmtotal total;
    size_t         total_size  = sizeof(total);
    int            total_mib[] = { CTL_VM, VM_METER };

    long            phys_mem;
    long            user_mem;
    size_t          mem_size  = sizeof(phys_mem);
    int             phys_mem_mib[] = { CTL_HW, HW_PHYSMEM };
    int             user_mem_mib[] = { CTL_HW, HW_USERMEM };

    /*
     * Retrieve the memory information from the underlying O/S...
     */
    sysctl(uvmexp_mib,   2, &uvmexp,   &uvmexp_size,   NULL, 0);
    sysctl(total_mib,    2, &total,    &total_size,    NULL, 0);
    sysctl(phys_mem_mib, 2, &phys_mem, &mem_size,      NULL, 0);
    sysctl(user_mem_mib, 2, &user_mem, &mem_size,      NULL, 0);
    pagesize = uvmexp.pagesize;

    /*
     * ... and save this in a standard form.
     */
    mem = netsnmp_memory_get_byIdx( NETSNMP_MEM_TYPE_PHYSMEM, 1 );
    if (!mem) {
        snmp_log_perror("No Physical Memory info entry");
    } else {
        if (!mem->descr)
             mem->descr = strdup("Physical memory");
        mem->units = pagesize;
        mem->size  = phys_mem/pagesize;
        mem->free  = total.t_free;
        mem->other = -1;
    }

    mem = netsnmp_memory_get_byIdx( NETSNMP_MEM_TYPE_USERMEM, 1 );
    if (!mem) {
        snmp_log_perror("No (user) Memory info entry");
    } else {
        if (!mem->descr)
             mem->descr = strdup("Real memory");
        mem->units = pagesize;
        mem->size  = user_mem/pagesize;
        mem->free  = uvmexp.free;
        mem->other = -1;
    }

#ifdef SWAP_NSWAP
    swapinfo(pagesize);
#endif
    mem = netsnmp_memory_get_byIdx( NETSNMP_MEM_TYPE_SWAP, 1 );
    if (!mem) {
        snmp_log_perror("No Swap info entry");
    } else {
        if (!mem->descr)
             mem->descr = strdup("Swap space");
        mem->units = pagesize;
        mem->size  = uvmexp.swpages;
        mem->free  = uvmexp.swpages - uvmexp.swpginuse;
        mem->other = -1;
    }

    return 0;
}


#ifdef SWAP_NSWAP
    /*
     * If there are multiple swap devices, record
     *   the statistics for each one individually.
     */
void
swapinfo(long pagesize)
{
    int             i, n;
    struct swapent *s;
    netsnmp_memory_info *mem;
    char buf[1024];

        /*
         * If there's only one swap device, don't bother
         */
    n = swapctl( SWAP_NSWAP, NULL, 0 );
    if ( n <= 1 )
        return;

    s = (struct swapent*)calloc(n, sizeof(struct swapent));
    swapctl( SWAP_STATS, s, n );

    for (i = 0; i < n; ++i) {
        mem = netsnmp_memory_get_byIdx( NETSNMP_MEM_TYPE_SWAP+1+i, 1 );
        if (!mem)
            continue;
        if (!mem->descr) {
         /* sprintf(buf, "swap #%d", s[i].se_dev); */
            sprintf(buf, "swap %s",  s[i].se_path);
            mem->descr = strdup( buf );
        }
        mem->units = pagesize;
        mem->size  = s[i].se_nblks;
        mem->free  = s[i].se_nblks - s[i].se_inuse;
        mem->other = -1;
    }
}
#endif
