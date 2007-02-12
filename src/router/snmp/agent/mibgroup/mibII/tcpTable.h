/*
 *  TCP MIB group interface - tcp.h
 *
 */
#ifndef _MIBGROUP_TCPTABLE_H
#define _MIBGROUP_TCPTABLE_H

#ifdef linux
struct inpcb {
    struct inpcb   *inp_next;   /* pointers to other pcb's */
    struct in_addr  inp_faddr;  /* foreign host table entry */
    u_short         inp_fport;  /* foreign port */
    struct in_addr  inp_laddr;  /* local host table entry */
    u_short         inp_lport;  /* local port */
    int             inp_state;
    int             uid;        /* owner of the connection */
};
#endif

#ifdef hpux11
#include <sys/mib.h>
#endif

#ifndef solaris2
#if !defined(linux) && !defined(hpux11)
extern int      TCP_Count_Connections(void);
#endif
extern void     TCP_Scan_Init(void);
#ifdef hpux11
extern int      TCP_Scan_Next(mib_tcpConnEnt *);
#else
struct inpcb;
extern int      TCP_Scan_Next(int *, struct inpcb *);
#endif
#endif

config_arch_require(solaris2, kernel_sunos5)
config_require(mibII/tcp util_funcs)

     extern FindVarMethod var_tcpEntry;

#endif                          /* _MIBGROUP_TCPTABLE_H */
