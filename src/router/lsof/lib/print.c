/*
 * print.c - common print support functions for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#include "common.h"
#include "lsof.h"

/*
 * Local definitions, structures and function prototypes
 */

/*
 * access_to_char() - convert enum lsof_file_access_mode to char
 */
char access_to_char(enum lsof_file_access_mode access) {
    switch (access) {
    default:
    case LSOF_FILE_ACCESS_NONE:
        return ' ';
    case LSOF_FILE_ACCESS_READ:
        return 'r';
    case LSOF_FILE_ACCESS_WRITE:
        return 'w';
    case LSOF_FILE_ACCESS_READ_WRITE:
        return 'u';
    }
}

/*
 * lock_to_char() - convert enum lsof_lock_mode to char
 */
char lock_to_char(enum lsof_lock_mode lock) {
    switch (lock) {
    default:
    case LSOF_LOCK_NONE:
        return ' ';
    case LSOF_LOCK_UNKNOWN:
        return 'U';
    case LSOF_LOCK_READ_PARTIAL:
        return 'r';
    case LSOF_LOCK_READ_FULL:
        return 'R';
    case LSOF_LOCK_WRITE_PARTIAL:
        return 'w';
    case LSOF_LOCK_WRITE_FULL:
        return 'W';
    case LSOF_LOCK_READ_WRITE:
        return 'u';
    case LSOF_LOCK_SOLARIS_NFS:
        return 'N';
    case LSOF_LOCK_SCO_PARTIAL:
        return 'x';
    case LSOF_LOCK_SCO_FULL:
        return 'X';
    }
}

/*
 * file_type_to_string() - convert enum lsof_file_type to string
 */
void file_type_to_string(enum lsof_file_type type,
                         uint32_t unknown_file_type_number, char *buf,
                         size_t buf_len) {
    switch (type) {
    default:
    case LSOF_FILE_UNKNOWN_RAW:
        (void)snpf(buf, buf_len, "%04o", (unknown_file_type_number & 0xfff));
        break;
    case LSOF_FILE_FIFO:
        (void)snpf(buf, buf_len, "FIFO");
        break;
    case LSOF_FILE_CHAR:
        (void)snpf(buf, buf_len, "CHR");
        break;
    case LSOF_FILE_DIR:
        (void)snpf(buf, buf_len, "DIR");
        break;
    case LSOF_FILE_BLOCK:
        (void)snpf(buf, buf_len, "BLK");
        break;
    case LSOF_FILE_REGULAR:
        (void)snpf(buf, buf_len, "REG");
        break;
    case LSOF_FILE_LINK:
        (void)snpf(buf, buf_len, "LINK");
        break;
    /* Use lower case for network-related files except IPv4/IPv6/ATALK for
     * compatibility */
    case LSOF_FILE_SOCKET:
        (void)snpf(buf, buf_len, "sock");
        break;
    case LSOF_FILE_IPV4:
        (void)snpf(buf, buf_len, "IPv4");
        break;
    case LSOF_FILE_IPV6:
        (void)snpf(buf, buf_len, "IPv6");
        break;
    case LSOF_FILE_AX25:
        (void)snpf(buf, buf_len, "ax25");
        break;
    case LSOF_FILE_INET:
        (void)snpf(buf, buf_len, "inet");
        break;
    case LSOF_FILE_LINK_LEVEL_ACCESS:
        (void)snpf(buf, buf_len, "lla");
        break;
    case LSOF_FILE_ROUTE:
        (void)snpf(buf, buf_len, "rte");
        break;
    case LSOF_FILE_UNIX:
        (void)snpf(buf, buf_len, "unix");
        break;
    case LSOF_FILE_X25:
        (void)snpf(buf, buf_len, "x.25");
        break;
    case LSOF_FILE_APPLETALK:
        (void)snpf(buf, buf_len, "ATALK");
        break;
    case LSOF_FILE_NET_DRIVER:
        (void)snpf(buf, buf_len, "ndrv");
        break;
    case LSOF_FILE_INTERNAL_KEY:
        (void)snpf(buf, buf_len, "key");
        break;
    case LSOF_FILE_SYSTEM:
        (void)snpf(buf, buf_len, "systm");
        break;
    case LSOF_FILE_PPP:
        (void)snpf(buf, buf_len, "ppp");
        break;
    case LSOF_FILE_IPX:
        (void)snpf(buf, buf_len, "ipx");
        break;
    case LSOF_FILE_RAW:
        (void)snpf(buf, buf_len, "raw");
        break;
    case LSOF_FILE_RAW6:
        (void)snpf(buf, buf_len, "raw6");
        break;
    case LSOF_FILE_NETLINK:
        (void)snpf(buf, buf_len, "netlink");
        break;
    case LSOF_FILE_PACKET:
        (void)snpf(buf, buf_len, "pack");
        break;
    case LSOF_FILE_ICMP:
        (void)snpf(buf, buf_len, "icmp");
        break;

    case LSOF_FILE_PROC_AS:
        (void)snpf(buf, buf_len, "PAS");
        break;
    case LSOF_FILE_PROC_AUXV:
        (void)snpf(buf, buf_len, "PAXV");
        break;
    case LSOF_FILE_PROC_CRED:
        (void)snpf(buf, buf_len, "PCRE");
        break;
    case LSOF_FILE_PROC_CTRL:
        (void)snpf(buf, buf_len, "PCTL");
        break;
    case LSOF_FILE_PROC_CUR_PROC:
        (void)snpf(buf, buf_len, "PCUR");
        break;
    case LSOF_FILE_PROC_CWD:
        (void)snpf(buf, buf_len, "PCWD");
        break;
    case LSOF_FILE_PROC_DIR:
        (void)snpf(buf, buf_len, "PDIR");
        break;
    case LSOF_FILE_PROC_EXEC_TYPE:
        (void)snpf(buf, buf_len, "PETY");
        break;
    case LSOF_FILE_PROC_FD:
        (void)snpf(buf, buf_len, "PFD");
        break;
    case LSOF_FILE_PROC_FD_DIR:
        (void)snpf(buf, buf_len, "PFDR");
        break;
    case LSOF_FILE_PROC_FILE:
        (void)snpf(buf, buf_len, "PFIL");
        break;
    case LSOF_FILE_PROC_FP_REGS:
        (void)snpf(buf, buf_len, "PFPR");
        break;
    case LSOF_FILE_PROC_PAGE_DATA:
        (void)snpf(buf, buf_len, "PGD");
        break;
    case LSOF_FILE_PROC_GROUP_NOTIFIER:
        (void)snpf(buf, buf_len, "PGID");
        break;
    case LSOF_FILE_PROC_LWP_CTL:
        (void)snpf(buf, buf_len, "PLC");
        break;
    case LSOF_FILE_PROC_LWP_DIR:
        (void)snpf(buf, buf_len, "PLDR");
        break;
    case LSOF_FILE_PROC_LDT:
        (void)snpf(buf, buf_len, "PLDT");
        break;
    case LSOF_FILE_PROC_LPS_INFO:
        (void)snpf(buf, buf_len, "PLPI");
        break;
    case LSOF_FILE_PROC_LSTATUS:
        (void)snpf(buf, buf_len, "PLST");
        break;
    case LSOF_FILE_PROC_LUSAGE:
        (void)snpf(buf, buf_len, "PLU");
        break;
    case LSOF_FILE_PROC_LWP_GWINDOWS:
        (void)snpf(buf, buf_len, "PLWG");
        break;
    case LSOF_FILE_PROC_LWP_SINFO:
        (void)snpf(buf, buf_len, "PLWI");
        break;
    case LSOF_FILE_PROC_LWP_STATUS:
        (void)snpf(buf, buf_len, "PLWS");
        break;
    case LSOF_FILE_PROC_LWP_USAGE:
        (void)snpf(buf, buf_len, "PLWU");
        break;
    case LSOF_FILE_PROC_LWP_XREGS:
        (void)snpf(buf, buf_len, "PLWX");
        break;
    case LSOF_FILE_PROC_MAP:
        (void)snpf(buf, buf_len, "PMAP");
        break;
    case LSOF_FILE_PROC_MAPS:
        (void)snpf(buf, buf_len, "PMPS");
        break;
    case LSOF_FILE_PROC_MEMORY:
        (void)snpf(buf, buf_len, "PMEM");
        break;
    case LSOF_FILE_PROC_PROC_NOTIFIER:
        (void)snpf(buf, buf_len, "PNTF");
        break;
    case LSOF_FILE_PROC_OBJ:
        (void)snpf(buf, buf_len, "POBJ");
        break;
    case LSOF_FILE_PROC_OBJ_DIR:
        (void)snpf(buf, buf_len, "PODR");
        break;
    case LSOF_FILE_PROC_OLD_LWP:
        (void)snpf(buf, buf_len, "POLP");
        break;
    case LSOF_FILE_PROC_OLD_PID:
        (void)snpf(buf, buf_len, "POPF");
        break;
    case LSOF_FILE_PROC_OLD_PAGE:
        (void)snpf(buf, buf_len, "POPG");
        break;
    case LSOF_FILE_PROC_REGS:
        (void)snpf(buf, buf_len, "PREG");
        break;
    case LSOF_FILE_PROC_RMAP:
        (void)snpf(buf, buf_len, "PRMP");
        break;
    case LSOF_FILE_PROC_ROOT:
        (void)snpf(buf, buf_len, "PRTD");
        break;
    case LSOF_FILE_PROC_SIGACT:
        (void)snpf(buf, buf_len, "PSGA");
        break;
    case LSOF_FILE_PROC_PSINFO:
        (void)snpf(buf, buf_len, "PSIN");
        break;
    case LSOF_FILE_PROC_STATUS:
        (void)snpf(buf, buf_len, "PSTA");
        break;
    case LSOF_FILE_PROC_USAGE:
        (void)snpf(buf, buf_len, "PUSG");
        break;
    case LSOF_FILE_PROC_WATCH:
        (void)snpf(buf, buf_len, "PW");
        break;
    case LSOF_FILE_PROC_XMAP:
        (void)snpf(buf, buf_len, "PXMP");
        break;

    /* Others */
    case LSOF_FILE_ANON_INODE:
        (void)snpf(buf, buf_len, "a_inode");
        break;
    case LSOF_FILE_DEL:
        (void)snpf(buf, buf_len, "DEL");
        break;
    case LSOF_FILE_DOOR:
        (void)snpf(buf, buf_len, "DOOR");
        break;
    case LSOF_FILE_KQUEUE:
        (void)snpf(buf, buf_len, "KQUEUE");
        break;
    case LSOF_FILE_FSEVENTS:
        (void)snpf(buf, buf_len, "FSEVENTS");
        break;
    case LSOF_FILE_EVENTFD:
        (void)snpf(buf, buf_len, "EVENTFD");
        break;
    case LSOF_FILE_PROCDESC:
        (void)snpf(buf, buf_len, "PROCDSC");
        break;
    case LSOF_FILE_MULTIPLEXED_BLOCK:
        (void)snpf(buf, buf_len, "MPB");
        break;
    case LSOF_FILE_MULTIPLEXED_CHAR:
        (void)snpf(buf, buf_len, "MPC");
        break;
    case LSOF_FILE_UNKNOWN_DELETED:
        (void)snpf(buf, buf_len, "UNKNdel");
        break;
    case LSOF_FILE_UNKNOWN_MEMORY:
        (void)snpf(buf, buf_len, "UNKNmem");
        break;
    case LSOF_FILE_UNKNOWN_FD:
        (void)snpf(buf, buf_len, "UNKNfd");
        break;
    case LSOF_FILE_UNKNOWN_CWD:
        (void)snpf(buf, buf_len, "UNKNcwd");
        break;
    case LSOF_FILE_UNKNOWN_ROOT_DIR:
        (void)snpf(buf, buf_len, "UNKNrtd");
        break;
    case LSOF_FILE_UNKNOWN_PROGRAM_TEXT:
        (void)snpf(buf, buf_len, "UNKNtxt");
        break;
    case LSOF_FILE_UNKNOWN:
        (void)snpf(buf, buf_len, "UNKN");
        break;
    case LSOF_FILE_UNKNOWN_STAT:
        (void)snpf(buf, buf_len, "unknown");
        break;
    case LSOF_FILE_PIPE:
        (void)snpf(buf, buf_len, "PIPE");
        break;
    case LSOF_FILE_PORT:
        (void)snpf(buf, buf_len, "PORT");
        break;
    case LSOF_FILE_POSIX_MQ:
        (void)snpf(buf, buf_len, "PSXMQ");
        break;
    case LSOF_FILE_POSIX_SEMA:
        (void)snpf(buf, buf_len, "PSXSEM");
        break;
    case LSOF_FILE_POSIX_SHM:
        (void)snpf(buf, buf_len, "PSXSHM");
        break;
    case LSOF_FILE_SHM:
        (void)snpf(buf, buf_len, "SHM");
        break;
    case LSOF_FILE_PTS:
        (void)snpf(buf, buf_len, "PTS");
        break;
    case LSOF_FILE_SHARED_MEM_TRANSPORT:
        (void)snpf(buf, buf_len, "SMT");
        break;
    case LSOF_FILE_STREAM:
        (void)snpf(buf, buf_len, "STR");
        break;
    case LSOF_FILE_STREAM_SOCKET:
        (void)snpf(buf, buf_len, "STSO");
        break;
    case LSOF_FILE_SCO_UNKNOWN:
        (void)snpf(buf, buf_len, "XNAM");
        break;
    case LSOF_FILE_SCO_SEMA:
        (void)snpf(buf, buf_len, "XSEM");
        break;
    case LSOF_FILE_SCO_SHARED:
        (void)snpf(buf, buf_len, "XSD");
        break;
    case LSOF_FILE_UNSUPPORTED:
        (void)snpf(buf, buf_len, "UNSP");
        break;

    /* vnode */
    case LSOF_FILE_VNODE_VNON:
        (void)snpf(buf, buf_len, "VNON");
        break;
    case LSOF_FILE_VNODE_VREG:
        (void)snpf(buf, buf_len, "VREG");
        break;
    case LSOF_FILE_VNODE_VDIR:
        (void)snpf(buf, buf_len, "VDIR");
        break;
    case LSOF_FILE_VNODE_VBLK:
        (void)snpf(buf, buf_len, "VBLK");
        break;
    case LSOF_FILE_VNODE_VCHR:
        (void)snpf(buf, buf_len, "VCHR");
        break;
    case LSOF_FILE_VNODE_VLNK:
        (void)snpf(buf, buf_len, "VLNK");
        break;
    case LSOF_FILE_VNODE_VSOCK:
        (void)snpf(buf, buf_len, "SOCK");
        break;
    case LSOF_FILE_VNODE_VBAD:
        (void)snpf(buf, buf_len, "VBAD");
        break;
    case LSOF_FILE_VNODE_VMPC:
        (void)snpf(buf, buf_len, "VMPC");
        break;
    case LSOF_FILE_VNODE_VFIFO:
        (void)snpf(buf, buf_len, "FIFO");
        break;
    case LSOF_FILE_VNODE_VDOOR:
        (void)snpf(buf, buf_len, "DOOR");
        break;
    case LSOF_FILE_VNODE_VPORT:
        (void)snpf(buf, buf_len, "PORT");
        break;
    case LSOF_FILE_VNODE_VUNNAMED:
        (void)snpf(buf, buf_len, "UNNM");
        break;
    }
}

/*
 * endnm() - locate end of Namech
 */
char *endnm(struct lsof_context *ctx, size_t *sz) /* returned remaining size */
{
    register char *s;
    register size_t tsz;

    for (s = Namech, tsz = Namechl; *s; s++, tsz--)
        ;
    *sz = tsz;
    return (s);
}

void __attribute__((weak))
usage(struct lsof_context *ctx, /* context */
      int err,                  /* it is called as part of error handlng? */
      int fh,                   /* ``-F ?'' status */
      int version)              /* ``-v'' status */
{
    // do nothing in liblsof
}

#if !defined(HASPRIVPRIPP)
/*
 * printiproto() - print Internet protocol name
 */

void printiproto(struct lsof_context *ctx, /* context */
                 int p)                    /* protocol number */
{
    int i;
    static int m = -1;
    char *s;

    switch (p) {

#    if defined(IPPROTO_TCP)
    case IPPROTO_TCP:
        s = "TCP";
        break;
#    endif /* defined(IPPROTO_TCP) */

#    if defined(IPPROTO_UDP)
    case IPPROTO_UDP:
        s = "UDP";
        break;
#    endif /* defined(IPPROTO_UDP) */

#    if defined(IPPROTO_IP)
#        if !defined(IPPROTO_HOPOPTS) || IPPROTO_IP != IPPROTO_HOPOPTS
    case IPPROTO_IP:
        s = "IP";
        break;
#        endif /* !defined(IPPROTO_HOPOPTS) || IPPROTO_IP!=IPPROTO_HOPOPTS */
#    endif     /* defined(IPPROTO_IP) */

#    if defined(IPPROTO_ICMP)
    case IPPROTO_ICMP:
        s = "ICMP";
        break;
#    endif /* defined(IPPROTO_ICMP) */

#    if defined(IPPROTO_ICMPV6)
    case IPPROTO_ICMPV6:
        s = "ICMPV6";
        break;
#    endif /* defined(IPPROTO_ICMPV6) */

#    if defined(IPPROTO_IGMP)
    case IPPROTO_IGMP:
        s = "IGMP";
        break;
#    endif /* defined(IPPROTO_IGMP) */

#    if defined(IPPROTO_GGP)
    case IPPROTO_GGP:
        s = "GGP";
        break;
#    endif /* defined(IPPROTO_GGP) */

#    if defined(IPPROTO_EGP)
    case IPPROTO_EGP:
        s = "EGP";
        break;
#    endif /* defined(IPPROTO_EGP) */

#    if defined(IPPROTO_PUP)
    case IPPROTO_PUP:
        s = "PUP";
        break;
#    endif /* defined(IPPROTO_PUP) */

#    if defined(IPPROTO_IDP)
    case IPPROTO_IDP:
        s = "IDP";
        break;
#    endif /* defined(IPPROTO_IDP) */

#    if defined(IPPROTO_ND)
    case IPPROTO_ND:
        s = "ND";
        break;
#    endif /* defined(IPPROTO_ND) */

#    if defined(IPPROTO_RAW)
    case IPPROTO_RAW:
        s = "RAW";
        break;
#    endif /* defined(IPPROTO_RAW) */

#    if defined(IPPROTO_HELLO)
    case IPPROTO_HELLO:
        s = "HELLO";
        break;
#    endif /* defined(IPPROTO_HELLO) */

#    if defined(IPPROTO_PXP)
    case IPPROTO_PXP:
        s = "PXP";
        break;
#    endif /* defined(IPPROTO_PXP) */

#    if defined(IPPROTO_RAWIP)
    case IPPROTO_RAWIP:
        s = "RAWIP";
        break;
#    endif /* defined(IPPROTO_RAWIP) */

#    if defined(IPPROTO_RAWIF)
    case IPPROTO_RAWIF:
        s = "RAWIF";
        break;
#    endif /* defined(IPPROTO_RAWIF) */

#    if defined(IPPROTO_HOPOPTS)
    case IPPROTO_HOPOPTS:
        s = "HOPOPTS";
        break;
#    endif /* defined(IPPROTO_HOPOPTS) */

#    if defined(IPPROTO_IPIP)
    case IPPROTO_IPIP:
        s = "IPIP";
        break;
#    endif /* defined(IPPROTO_IPIP) */

#    if defined(IPPROTO_ST)
    case IPPROTO_ST:
        s = "ST";
        break;
#    endif /* defined(IPPROTO_ST) */

#    if defined(IPPROTO_PIGP)
    case IPPROTO_PIGP:
        s = "PIGP";
        break;
#    endif /* defined(IPPROTO_PIGP) */

#    if defined(IPPROTO_RCCMON)
    case IPPROTO_RCCMON:
        s = "RCCMON";
        break;
#    endif /* defined(IPPROTO_RCCMON) */

#    if defined(IPPROTO_NVPII)
    case IPPROTO_NVPII:
        s = "NVPII";
        break;
#    endif /* defined(IPPROTO_NVPII) */

#    if defined(IPPROTO_ARGUS)
    case IPPROTO_ARGUS:
        s = "ARGUS";
        break;
#    endif /* defined(IPPROTO_ARGUS) */

#    if defined(IPPROTO_EMCON)
    case IPPROTO_EMCON:
        s = "EMCON";
        break;
#    endif /* defined(IPPROTO_EMCON) */

#    if defined(IPPROTO_XNET)
    case IPPROTO_XNET:
        s = "XNET";
        break;
#    endif /* defined(IPPROTO_XNET) */

#    if defined(IPPROTO_CHAOS)
    case IPPROTO_CHAOS:
        s = "CHAOS";
        break;
#    endif /* defined(IPPROTO_CHAOS) */

#    if defined(IPPROTO_MUX)
    case IPPROTO_MUX:
        s = "MUX";
        break;
#    endif /* defined(IPPROTO_MUX) */

#    if defined(IPPROTO_MEAS)
    case IPPROTO_MEAS:
        s = "MEAS";
        break;
#    endif /* defined(IPPROTO_MEAS) */

#    if defined(IPPROTO_HMP)
    case IPPROTO_HMP:
        s = "HMP";
        break;
#    endif /* defined(IPPROTO_HMP) */

#    if defined(IPPROTO_PRM)
    case IPPROTO_PRM:
        s = "PRM";
        break;
#    endif /* defined(IPPROTO_PRM) */

#    if defined(IPPROTO_TRUNK1)
    case IPPROTO_TRUNK1:
        s = "TRUNK1";
        break;
#    endif /* defined(IPPROTO_TRUNK1) */

#    if defined(IPPROTO_TRUNK2)
    case IPPROTO_TRUNK2:
        s = "TRUNK2";
        break;
#    endif /* defined(IPPROTO_TRUNK2) */

#    if defined(IPPROTO_LEAF1)
    case IPPROTO_LEAF1:
        s = "LEAF1";
        break;
#    endif /* defined(IPPROTO_LEAF1) */

#    if defined(IPPROTO_LEAF2)
    case IPPROTO_LEAF2:
        s = "LEAF2";
        break;
#    endif /* defined(IPPROTO_LEAF2) */

#    if defined(IPPROTO_RDP)
    case IPPROTO_RDP:
        s = "RDP";
        break;
#    endif /* defined(IPPROTO_RDP) */

#    if defined(IPPROTO_IRTP)
    case IPPROTO_IRTP:
        s = "IRTP";
        break;
#    endif /* defined(IPPROTO_IRTP) */

#    if defined(IPPROTO_TP)
    case IPPROTO_TP:
        s = "TP";
        break;
#    endif /* defined(IPPROTO_TP) */

#    if defined(IPPROTO_BLT)
    case IPPROTO_BLT:
        s = "BLT";
        break;
#    endif /* defined(IPPROTO_BLT) */

#    if defined(IPPROTO_NSP)
    case IPPROTO_NSP:
        s = "NSP";
        break;
#    endif /* defined(IPPROTO_NSP) */

#    if defined(IPPROTO_INP)
    case IPPROTO_INP:
        s = "INP";
        break;
#    endif /* defined(IPPROTO_INP) */

#    if defined(IPPROTO_SEP)
    case IPPROTO_SEP:
        s = "SEP";
        break;
#    endif /* defined(IPPROTO_SEP) */

#    if defined(IPPROTO_3PC)
    case IPPROTO_3PC:
        s = "3PC";
        break;
#    endif /* defined(IPPROTO_3PC) */

#    if defined(IPPROTO_IDPR)
    case IPPROTO_IDPR:
        s = "IDPR";
        break;
#    endif /* defined(IPPROTO_IDPR) */

#    if defined(IPPROTO_XTP)
    case IPPROTO_XTP:
        s = "XTP";
        break;
#    endif /* defined(IPPROTO_XTP) */

#    if defined(IPPROTO_DDP)
    case IPPROTO_DDP:
        s = "DDP";
        break;
#    endif /* defined(IPPROTO_DDP) */

#    if defined(IPPROTO_CMTP)
    case IPPROTO_CMTP:
        s = "CMTP";
        break;
#    endif /* defined(IPPROTO_CMTP) */

#    if defined(IPPROTO_TPXX)
    case IPPROTO_TPXX:
        s = "TPXX";
        break;
#    endif /* defined(IPPROTO_TPXX) */

#    if defined(IPPROTO_IL)
    case IPPROTO_IL:
        s = "IL";
        break;
#    endif /* defined(IPPROTO_IL) */

#    if defined(IPPROTO_IPV6)
    case IPPROTO_IPV6:
        s = "IPV6";
        break;
#    endif /* defined(IPPROTO_IPV6) */

#    if defined(IPPROTO_SDRP)
    case IPPROTO_SDRP:
        s = "SDRP";
        break;
#    endif /* defined(IPPROTO_SDRP) */

#    if defined(IPPROTO_ROUTING)
    case IPPROTO_ROUTING:
        s = "ROUTING";
        break;
#    endif /* defined(IPPROTO_ROUTING) */

#    if defined(IPPROTO_FRAGMENT)
    case IPPROTO_FRAGMENT:
        s = "FRAGMNT";
        break;
#    endif /* defined(IPPROTO_FRAGMENT) */

#    if defined(IPPROTO_IDRP)
    case IPPROTO_IDRP:
        s = "IDRP";
        break;
#    endif /* defined(IPPROTO_IDRP) */

#    if defined(IPPROTO_RSVP)
    case IPPROTO_RSVP:
        s = "RSVP";
        break;
#    endif /* defined(IPPROTO_RSVP) */

#    if defined(IPPROTO_GRE)
    case IPPROTO_GRE:
        s = "GRE";
        break;
#    endif /* defined(IPPROTO_GRE) */

#    if defined(IPPROTO_MHRP)
    case IPPROTO_MHRP:
        s = "MHRP";
        break;
#    endif /* defined(IPPROTO_MHRP) */

#    if defined(IPPROTO_BHA)
    case IPPROTO_BHA:
        s = "BHA";
        break;
#    endif /* defined(IPPROTO_BHA) */

#    if defined(IPPROTO_ESP)
    case IPPROTO_ESP:
        s = "ESP";
        break;
#    endif /* defined(IPPROTO_ESP) */

#    if defined(IPPROTO_AH)
    case IPPROTO_AH:
        s = "AH";
        break;
#    endif /* defined(IPPROTO_AH) */

#    if defined(IPPROTO_INLSP)
    case IPPROTO_INLSP:
        s = "INLSP";
        break;
#    endif /* defined(IPPROTO_INLSP) */

#    if defined(IPPROTO_SWIPE)
    case IPPROTO_SWIPE:
        s = "SWIPE";
        break;
#    endif /* defined(IPPROTO_SWIPE) */

#    if defined(IPPROTO_NHRP)
    case IPPROTO_NHRP:
        s = "NHRP";
        break;
#    endif /* defined(IPPROTO_NHRP) */

#    if defined(IPPROTO_NONE)
    case IPPROTO_NONE:
        s = "NONE";
        break;
#    endif /* defined(IPPROTO_NONE) */

#    if defined(IPPROTO_DSTOPTS)
    case IPPROTO_DSTOPTS:
        s = "DSTOPTS";
        break;
#    endif /* defined(IPPROTO_DSTOPTS) */

#    if defined(IPPROTO_AHIP)
    case IPPROTO_AHIP:
        s = "AHIP";
        break;
#    endif /* defined(IPPROTO_AHIP) */

#    if defined(IPPROTO_CFTP)
    case IPPROTO_CFTP:
        s = "CFTP";
        break;
#    endif /* defined(IPPROTO_CFTP) */

#    if defined(IPPROTO_SATEXPAK)
    case IPPROTO_SATEXPAK:
        s = "SATEXPK";
        break;
#    endif /* defined(IPPROTO_SATEXPAK) */

#    if defined(IPPROTO_KRYPTOLAN)
    case IPPROTO_KRYPTOLAN:
        s = "KRYPTOL";
        break;
#    endif /* defined(IPPROTO_KRYPTOLAN) */

#    if defined(IPPROTO_RVD)
    case IPPROTO_RVD:
        s = "RVD";
        break;
#    endif /* defined(IPPROTO_RVD) */

#    if defined(IPPROTO_IPPC)
    case IPPROTO_IPPC:
        s = "IPPC";
        break;
#    endif /* defined(IPPROTO_IPPC) */

#    if defined(IPPROTO_ADFS)
    case IPPROTO_ADFS:
        s = "ADFS";
        break;
#    endif /* defined(IPPROTO_ADFS) */

#    if defined(IPPROTO_SATMON)
    case IPPROTO_SATMON:
        s = "SATMON";
        break;
#    endif /* defined(IPPROTO_SATMON) */

#    if defined(IPPROTO_VISA)
    case IPPROTO_VISA:
        s = "VISA";
        break;
#    endif /* defined(IPPROTO_VISA) */

#    if defined(IPPROTO_IPCV)
    case IPPROTO_IPCV:
        s = "IPCV";
        break;
#    endif /* defined(IPPROTO_IPCV) */

#    if defined(IPPROTO_CPNX)
    case IPPROTO_CPNX:
        s = "CPNX";
        break;
#    endif /* defined(IPPROTO_CPNX) */

#    if defined(IPPROTO_CPHB)
    case IPPROTO_CPHB:
        s = "CPHB";
        break;
#    endif /* defined(IPPROTO_CPHB) */

#    if defined(IPPROTO_WSN)
    case IPPROTO_WSN:
        s = "WSN";
        break;
#    endif /* defined(IPPROTO_WSN) */

#    if defined(IPPROTO_PVP)
    case IPPROTO_PVP:
        s = "PVP";
        break;
#    endif /* defined(IPPROTO_PVP) */

#    if defined(IPPROTO_BRSATMON)
    case IPPROTO_BRSATMON:
        s = "BRSATMN";
        break;
#    endif /* defined(IPPROTO_BRSATMON) */

#    if defined(IPPROTO_WBMON)
    case IPPROTO_WBMON:
        s = "WBMON";
        break;
#    endif /* defined(IPPROTO_WBMON) */

#    if defined(IPPROTO_WBEXPAK)
    case IPPROTO_WBEXPAK:
        s = "WBEXPAK";
        break;
#    endif /* defined(IPPROTO_WBEXPAK) */

#    if defined(IPPROTO_EON)
    case IPPROTO_EON:
        s = "EON";
        break;
#    endif /* defined(IPPROTO_EON) */

#    if defined(IPPROTO_VMTP)
    case IPPROTO_VMTP:
        s = "VMTP";
        break;
#    endif /* defined(IPPROTO_VMTP) */

#    if defined(IPPROTO_SVMTP)
    case IPPROTO_SVMTP:
        s = "SVMTP";
        break;
#    endif /* defined(IPPROTO_SVMTP) */

#    if defined(IPPROTO_VINES)
    case IPPROTO_VINES:
        s = "VINES";
        break;
#    endif /* defined(IPPROTO_VINES) */

#    if defined(IPPROTO_TTP)
    case IPPROTO_TTP:
        s = "TTP";
        break;
#    endif /* defined(IPPROTO_TTP) */

#    if defined(IPPROTO_IGP)
    case IPPROTO_IGP:
        s = "IGP";
        break;
#    endif /* defined(IPPROTO_IGP) */

#    if defined(IPPROTO_DGP)
    case IPPROTO_DGP:
        s = "DGP";
        break;
#    endif /* defined(IPPROTO_DGP) */

#    if defined(IPPROTO_TCF)
    case IPPROTO_TCF:
        s = "TCF";
        break;
#    endif /* defined(IPPROTO_TCF) */

#    if defined(IPPROTO_IGRP)
    case IPPROTO_IGRP:
        s = "IGRP";
        break;
#    endif /* defined(IPPROTO_IGRP) */

#    if defined(IPPROTO_OSPFIGP)
    case IPPROTO_OSPFIGP:
        s = "OSPFIGP";
        break;
#    endif /* defined(IPPROTO_OSPFIGP) */

#    if defined(IPPROTO_SRPC)
    case IPPROTO_SRPC:
        s = "SRPC";
        break;
#    endif /* defined(IPPROTO_SRPC) */

#    if defined(IPPROTO_LARP)
    case IPPROTO_LARP:
        s = "LARP";
        break;
#    endif /* defined(IPPROTO_LARP) */

#    if defined(IPPROTO_MTP)
    case IPPROTO_MTP:
        s = "MTP";
        break;
#    endif /* defined(IPPROTO_MTP) */

#    if defined(IPPROTO_AX25)
    case IPPROTO_AX25:
        s = "AX25";
        break;
#    endif /* defined(IPPROTO_AX25) */

#    if defined(IPPROTO_IPEIP)
    case IPPROTO_IPEIP:
        s = "IPEIP";
        break;
#    endif /* defined(IPPROTO_IPEIP) */

#    if defined(IPPROTO_MICP)
    case IPPROTO_MICP:
        s = "MICP";
        break;
#    endif /* defined(IPPROTO_MICP) */

#    if defined(IPPROTO_SCCSP)
    case IPPROTO_SCCSP:
        s = "SCCSP";
        break;
#    endif /* defined(IPPROTO_SCCSP) */

#    if defined(IPPROTO_ETHERIP)
    case IPPROTO_ETHERIP:
        s = "ETHERIP";
        break;
#    endif /* defined(IPPROTO_ETHERIP) */

#    if defined(IPPROTO_ENCAP)
#        if !defined(IPPROTO_IPIP) || IPPROTO_IPIP != IPPROTO_ENCAP
    case IPPROTO_ENCAP:
        s = "ENCAP";
        break;
#        endif /* !defined(IPPROTO_IPIP) || IPPROTO_IPIP!=IPPROTO_ENCAP */
#    endif     /* defined(IPPROTO_ENCAP) */

#    if defined(IPPROTO_APES)
    case IPPROTO_APES:
        s = "APES";
        break;
#    endif /* defined(IPPROTO_APES) */

#    if defined(IPPROTO_GMTP)
    case IPPROTO_GMTP:
        s = "GMTP";
        break;
#    endif /* defined(IPPROTO_GMTP) */

#    if defined(IPPROTO_DIVERT)
    case IPPROTO_DIVERT:
        s = "DIVERT";
        break;
#    endif /* defined(IPPROTO_DIVERT) */

    default:
        s = (char *)NULL;
    }
    if (s)
        (void)snpf(Lf->iproto, sizeof(Lf->iproto), "%.*s", IPROTOL - 1, s);
    else {
        if (m < 0) {
            for (i = 0, m = 1; i < IPROTOL - 2; i++)
                m *= 10;
        }
        if (m > p)
            (void)snpf(Lf->iproto, sizeof(Lf->iproto), "%d?", p);
        else
            (void)snpf(Lf->iproto, sizeof(Lf->iproto), "*%d?", p % (m / 10));
    }
}
#endif /* !defined(HASPRIVPRIPP) */

/*
 * printunkaf() - print unknown address family
 */

void printunkaf(struct lsof_context *ctx, int fam, /* unknown address family */
                int ty) /* output type: 0 = terse; 1 = full */
{
    char *p, *s;

    p = "";
    switch (fam) {

#if defined(AF_UNSPEC)
    case AF_UNSPEC:
        s = "UNSPEC";
        break;
#endif /* defined(AF_UNSPEC) */

#if defined(AF_UNIX)
    case AF_UNIX:
        s = "UNIX";
        break;
#endif /* defined(AF_UNIX) */

#if defined(AF_INET)
    case AF_INET:
        s = "INET";
        break;
#endif /* defined(AF_INET) */

#if defined(AF_INET6)
    case AF_INET6:
        s = "INET6";
        break;
#endif /* defined(AF_INET6) */

#if defined(AF_IMPLINK)
    case AF_IMPLINK:
        s = "IMPLINK";
        break;
#endif /* defined(AF_IMPLINK) */

#if defined(AF_PUP)
    case AF_PUP:
        s = "PUP";
        break;
#endif /* defined(AF_PUP) */

#if defined(AF_CHAOS)
    case AF_CHAOS:
        s = "CHAOS";
        break;
#endif /* defined(AF_CHAOS) */

#if defined(AF_NS)
    case AF_NS:
        s = "NS";
        break;
#endif /* defined(AF_NS) */

#if defined(AF_ISO)
    case AF_ISO:
        s = "ISO";
        break;
#endif /* defined(AF_ISO) */

#if defined(AF_NBS)
#    if !defined(AF_ISO) || AF_NBS != AF_ISO
    case AF_NBS:
        s = "NBS";
        break;
#    endif /* !defined(AF_ISO) || AF_NBS!=AF_ISO */
#endif     /* defined(AF_NBS) */

#if defined(AF_ECMA)
    case AF_ECMA:
        s = "ECMA";
        break;
#endif /* defined(AF_ECMA) */

#if defined(AF_DATAKIT)
    case AF_DATAKIT:
        s = "DATAKIT";
        break;
#endif /* defined(AF_DATAKIT) */

#if defined(AF_CCITT)
    case AF_CCITT:
        s = "CCITT";
        break;
#endif /* defined(AF_CCITT) */

#if defined(AF_SNA)
    case AF_SNA:
        s = "SNA";
        break;
#endif /* defined(AF_SNA) */

#if defined(AF_DECnet)
    case AF_DECnet:
        s = "DECnet";
        break;
#endif /* defined(AF_DECnet) */

#if defined(AF_DLI)
    case AF_DLI:
        s = "DLI";
        break;
#endif /* defined(AF_DLI) */

#if defined(AF_LAT)
    case AF_LAT:
        s = "LAT";
        break;
#endif /* defined(AF_LAT) */

#if defined(AF_HYLINK)
    case AF_HYLINK:
        s = "HYLINK";
        break;
#endif /* defined(AF_HYLINK) */

#if defined(AF_APPLETALK)
    case AF_APPLETALK:
        s = "APPLETALK";
        break;
#endif /* defined(AF_APPLETALK) */

#if defined(AF_BSC)
    case AF_BSC:
        s = "BSC";
        break;
#endif /* defined(AF_BSC) */

#if defined(AF_DSS)
    case AF_DSS:
        s = "DSS";
        break;
#endif /* defined(AF_DSS) */

#if defined(AF_ROUTE)
    case AF_ROUTE:
        s = "ROUTE";
        break;
#endif /* defined(AF_ROUTE) */

#if defined(AF_RAW)
    case AF_RAW:
        s = "RAW";
        break;
#endif /* defined(AF_RAW) */

#if defined(AF_LINK)
    case AF_LINK:
        s = "LINK";
        break;
#endif /* defined(AF_LINK) */

#if defined(pseudo_AF_XTP)
    case pseudo_AF_XTP:
        p = "pseudo_";
        s = "XTP";
        break;
#endif /* defined(pseudo_AF_XTP) */

#if defined(AF_RMP)
    case AF_RMP:
        s = "RMP";
        break;
#endif /* defined(AF_RMP) */

#if defined(AF_COIP)
    case AF_COIP:
        s = "COIP";
        break;
#endif /* defined(AF_COIP) */

#if defined(AF_CNT)
    case AF_CNT:
        s = "CNT";
        break;
#endif /* defined(AF_CNT) */

#if defined(pseudo_AF_RTIP)
    case pseudo_AF_RTIP:
        p = "pseudo_";
        s = "RTIP";
        break;
#endif /* defined(pseudo_AF_RTIP) */

#if defined(AF_NETMAN)
    case AF_NETMAN:
        s = "NETMAN";
        break;
#endif /* defined(AF_NETMAN) */

#if defined(AF_INTF)
    case AF_INTF:
        s = "INTF";
        break;
#endif /* defined(AF_INTF) */

#if defined(AF_NETWARE)
    case AF_NETWARE:
        s = "NETWARE";
        break;
#endif /* defined(AF_NETWARE) */

#if defined(AF_NDD)
    case AF_NDD:
        s = "NDD";
        break;
#endif /* defined(AF_NDD) */

#if defined(AF_NIT)
#    if !defined(AF_ROUTE) || AF_ROUTE != AF_NIT
    case AF_NIT:
        s = "NIT";
        break;
#    endif /* !defined(AF_ROUTE) || AF_ROUTE!=AF_NIT */
#endif     /* defined(AF_NIT) */

#if defined(AF_802)
#    if !defined(AF_RAW) || AF_RAW != AF_802
    case AF_802:
        s = "802";
        break;
#    endif /* !defined(AF_RAW) || AF_RAW!=AF_802 */
#endif     /* defined(AF_802) */

#if defined(AF_X25)
    case AF_X25:
        s = "X25";
        break;
#endif /* defined(AF_X25) */

#if defined(AF_CTF)
    case AF_CTF:
        s = "CTF";
        break;
#endif /* defined(AF_CTF) */

#if defined(AF_WAN)
    case AF_WAN:
        s = "WAN";
        break;
#endif /* defined(AF_WAN) */

#if defined(AF_OSINET)
#    if defined(AF_INET) && AF_INET != AF_OSINET
    case AF_OSINET:
        s = "OSINET";
        break;
#    endif /* defined(AF_INET) && AF_INET!=AF_OSINET */
#endif     /* defined(AF_OSINET) */

#if defined(AF_GOSIP)
    case AF_GOSIP:
        s = "GOSIP";
        break;
#endif /* defined(AF_GOSIP) */

#if defined(AF_SDL)
    case AF_SDL:
        s = "SDL";
        break;
#endif /* defined(AF_SDL) */

#if defined(AF_IPX)
    case AF_IPX:
        s = "IPX";
        break;
#endif /* defined(AF_IPX) */

#if defined(AF_SIP)
    case AF_SIP:
        s = "SIP";
        break;
#endif /* defined(AF_SIP) */

#if defined(psuedo_AF_PIP)
    case psuedo_AF_PIP:
        p = "pseudo_";
        s = "PIP";
        break;
#endif /* defined(psuedo_AF_PIP) */

#if defined(AF_OTS)
    case AF_OTS:
        s = "OTS";
        break;
#endif /* defined(AF_OTS) */

#if defined(pseudo_AF_BLUE)
    case pseudo_AF_BLUE: /* packets for Blue box */
        p = "pseudo_";
        s = "BLUE";
        break;
#endif /* defined(pseudo_AF_BLUE) */

#if defined(AF_NDRV) /* network driver raw access */
    case AF_NDRV:
        s = "NDRV";
        break;
#endif /* defined(AF_NDRV) */

#if defined(AF_SYSTEM) /* kernel event messages */
    case AF_SYSTEM:
        s = "SYSTEM";
        break;
#endif /* defined(AF_SYSTEM) */

#if defined(AF_USER)
    case AF_USER:
        s = "USER";
        break;
#endif /* defined(AF_USER) */

#if defined(pseudo_AF_KEY)
    case pseudo_AF_KEY:
        p = "pseudo_";
        s = "KEY";
        break;
#endif /* defined(pseudo_AF_KEY) */

#if defined(AF_KEY) /* Security Association DB socket */
    case AF_KEY:
        s = "KEY";
        break;
#endif /* defined(AF_KEY) */

#if defined(AF_NCA) /* NCA socket */
    case AF_NCA:
        s = "NCA";
        break;
#endif /* defined(AF_NCA) */

#if defined(AF_POLICY) /* Security Policy DB socket */
    case AF_POLICY:
        s = "POLICY";
        break;
#endif /* defined(AF_POLICY) */

#if defined(AF_PPP) /* PPP socket */
    case AF_PPP:
        s = "PPP";
        break;
#endif /* defined(AF_PPP) */

    default:
        if (!ty)
            (void)snpf(Namech, Namechl, "%#x", fam);
        else
            (void)snpf(Namech, Namechl, "no further information on family %#x",
                       fam);
        return;
    }
    if (!ty)
        (void)snpf(Namech, Namechl, "%sAF_%s", p, s);
    else
        (void)snpf(Namech, Namechl, "no further information on %sAF_%s", p, s);
    return;
}
