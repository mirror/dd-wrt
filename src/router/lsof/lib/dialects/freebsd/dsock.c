/*
 * dsock.c - FreeBSD socket processing functions for lsof
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

#include <sys/socket.h>
#ifndef lint
static char copyright[] =
    "@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
#endif

#include "common.h"

#if defined(HASIPv6)

/*
 * IPv6_2_IPv4()  -- macro to define the address of an IPv4 address contained
 *		     in an IPv6 address
 */

#    define IPv6_2_IPv4(v6) (((uint8_t *)((struct in6_addr *)v6)->s6_addr) + 12)

#    if defined(HAS_NO_6PORT)
/*
 * If the in_pcb structure no longer has the KAME accommodations of
 * in6p_[fl]port, redefine them to inp_[fl]port.
 */

#        define in6p_fport inp_fport
#        define in6p_lport inp_lport
#    endif /* defined(HAS_NO_6PORT) */

#    if defined(HAS_NO_6PPCB)
/*
 * If the in_pcb structure no longer has the KAME accommodation of in6p_pcb,
 * redefine it to inp_ppcb.
 */

#        define in6p_ppcb inp_ppcb
#    endif /* defined(HAS_NO_6PPCB) */
#endif     /* defined(HASIPv6) */

#if defined(HAS_SB_CCC)
#    define SOCK_CC sb_ccc
#else /* !defined(HAS_SB_CCC) */
#    define SOCK_CC sb_cc
#endif /* defined(HAS_SB_CCC) */

#if __FreeBSD_version >= 1200026
#    define XTCPCB_SO xt_inp.xi_socket
#    define XTCPCB_SO_PCB xt_inp.xi_socket.so_pcb
#else /* __FreeBSD_version < 1200026 */
#    define XTCPCB_SO xt_socket
#    define XTCPCB_SO_PCB xt_socket.so_pcb
#endif /* __FreeBSD_version >= 1200026 */

/*
 * Local function prototypes
 */

static int ckstate(struct lsof_context *ctx, struct xtcpcb *pcb, int fam);

static int cmp_xunpcb_sock_pcb(const void *a, const void *b) {
    const struct xunpcb *pcb1 = (const struct xunpcb *)a;
    const struct xunpcb *pcb2 = (const struct xunpcb *)b;

    if (pcb1->xu_socket.so_pcb < pcb2->xu_socket.so_pcb)
        return -1;
    else if (pcb1->xu_socket.so_pcb > pcb2->xu_socket.so_pcb)
        return 1;
    else
        return 0;
}

static int cmp_xtcpcb_sock_pcb(const void *a, const void *b) {
    const struct xtcpcb *pcb1 = (const struct xtcpcb *)a;
    const struct xtcpcb *pcb2 = (const struct xtcpcb *)b;

    if (pcb1->XTCPCB_SO_PCB < pcb2->XTCPCB_SO_PCB)
        return -1;
    else if (pcb1->XTCPCB_SO_PCB > pcb2->XTCPCB_SO_PCB)
        return 1;
    else
        return 0;
}

static int cmp_xinpcb_sock_pcb(const void *a, const void *b) {
    const struct xinpcb *pcb1 = (const struct xinpcb *)a;
    const struct xinpcb *pcb2 = (const struct xinpcb *)b;

    if (pcb1->xi_socket.so_pcb < pcb2->xi_socket.so_pcb)
        return -1;
    else if (pcb1->xi_socket.so_pcb > pcb2->xi_socket.so_pcb)
        return 1;
    else
        return 0;
}

static int get_unix_pcbs(const char *name, struct xunpcb **pcbs,
                         size_t *n_pcbs) {
    size_t len;
    char *buffer = NULL;
    struct xunpgen *ug = (struct xunpgen *)buffer;
    int count = 0;
    int ret = 1;

    if (sysctlbyname(name, NULL, &len, NULL, 0))
        goto end;
    if ((buffer = malloc(len)) == NULL)
        goto end;
    if (sysctlbyname(name, buffer, &len, NULL, 0))
        goto end;
    ug = (struct xunpgen *)buffer;
    for (ug = (struct xunpgen *)((char *)ug + ug->xug_len);
         ug->xug_len == sizeof(struct xunpcb);
         ug = (struct xunpgen *)((char *)ug + ug->xug_len)) {
        ++count;
    }
    memmove(buffer, buffer + ((struct xunpgen *)buffer)->xug_len,
            count * sizeof(struct xunpcb));
    qsort(buffer, count, sizeof(struct xunpcb), cmp_xunpcb_sock_pcb);
    ret = 0;

end:
    *pcbs = (struct xunpcb *)buffer;
    *n_pcbs = count;
    if (ret)
        free(buffer);
    return ret;
}

static int get_tcp_pcbs(struct xtcpcb **pcbs, size_t *n_pcbs) {
    size_t len;
    char *buffer = NULL;
    struct xinpgen *ig = (struct xinpgen *)buffer;
    int count = 0;
    int ret = 1;

    if (sysctlbyname("net.inet.tcp.pcblist", NULL, &len, NULL, 0))
        goto end;
    if ((buffer = malloc(len)) == NULL)
        goto end;
    if (sysctlbyname("net.inet.tcp.pcblist", buffer, &len, NULL, 0))
        goto end;
    ig = (struct xinpgen *)buffer;
    for (ig = (struct xinpgen *)((char *)ig + ig->xig_len);
         ig->xig_len == sizeof(struct xtcpcb);
         ig = (struct xinpgen *)((char *)ig + ig->xig_len)) {
        ++count;
    }
    memmove(buffer, buffer + ((struct xinpgen *)buffer)->xig_len,
            count * sizeof(struct xtcpcb));
    qsort(buffer, count, sizeof(struct xtcpcb), cmp_xtcpcb_sock_pcb);
    ret = 0;

end:
    *pcbs = (struct xtcpcb *)buffer;
    *n_pcbs = count;
    if (ret)
        free(buffer);
    return ret;
}

static int get_udp_pcbs(struct xinpcb **pcbs, size_t *n_pcbs) {
    size_t len;
    char *buffer = NULL;
    struct xinpgen *ig = (struct xinpgen *)buffer;
    int count = 0;
    int ret = 1;

    if (sysctlbyname("net.inet.udp.pcblist", NULL, &len, NULL, 0))
        goto end;
    if ((buffer = malloc(len)) == NULL)
        goto end;
    if (sysctlbyname("net.inet.udp.pcblist", buffer, &len, NULL, 0))
        goto end;
    ig = (struct xinpgen *)buffer;
    for (ig = (struct xinpgen *)((char *)ig + ig->xig_len);
         ig->xig_len == sizeof(struct xinpcb);
         ig = (struct xinpgen *)((char *)ig + ig->xig_len)) {
        ++count;
    }
    memmove(buffer, buffer + ((struct xinpgen *)buffer)->xig_len,
            count * sizeof(struct xinpcb));
    qsort(buffer, count, sizeof(struct xinpcb), cmp_xinpcb_sock_pcb);
    ret = 0;

end:
    *pcbs = (struct xinpcb *)buffer;
    *n_pcbs = count;
    if (ret)
        free(buffer);
    return 0;
}

struct pcb_lists *read_pcb_lists(void) {
    struct pcb_lists *pcbs;
    int succeeded = 0;

    pcbs = calloc(1, sizeof(struct pcb_lists));
    if (!pcbs)
        goto end;

    if (get_unix_pcbs("net.local.stream.pcblist", &pcbs->un_stream_pcbs,
                      &pcbs->n_un_stream_pcbs))
        goto end;
    if (get_unix_pcbs("net.local.dgram.pcblist", &pcbs->un_dgram_pcbs,
                      &pcbs->n_un_dgram_pcbs))
        goto end;
    if (get_unix_pcbs("net.local.seqpacket.pcblist", &pcbs->un_seqpacket_pcbs,
                      &pcbs->n_un_seqpacket_pcbs))
        goto end;
    if (get_tcp_pcbs(&pcbs->tcp_pcbs, &pcbs->n_tcp_pcbs))
        goto end;
    if (get_udp_pcbs(&pcbs->udp_pcbs, &pcbs->n_udp_pcbs))
        goto end;
    succeeded = 1;

end:
    if (!succeeded) {
        free_pcb_lists(pcbs);
        return NULL;
    }
    return pcbs;
}

void free_pcb_lists(struct pcb_lists *pcbs) {
    if (pcbs) {
        free(pcbs->un_stream_pcbs);
        free(pcbs->un_dgram_pcbs);
        free(pcbs->un_seqpacket_pcbs);
        free(pcbs->tcp_pcbs);
        free(pcbs->udp_pcbs);
    }
    free(pcbs);
}

/*
 * ckstate() -- read TCP control block and check TCP state for inclusion
 *		or exclusion
 * return: -1 == no TCP CB available
 *	    0 == TCP DB available; continue processing file
 *	    1 == stop processing file
 */

static int ckstate(struct lsof_context *ctx, struct xtcpcb *pcb, int fam) {
#if __FreeBSD_version >= 1200026
    int tsnx;

    if (TcpStXn || TcpStIn) {

        /*
         * If there are TCP state inclusions or exclusions, check them.
         */
        if ((tsnx = (int)pcb->t_state + TcpStOff) >= TcpNstates)
            return (0);
        if (TcpStXn) {
            if (TcpStX[tsnx]) {
                Lf->sf &= ~SELNET;
                Lf->sf |= SELEXCLF;
                return (1);
            }
        }
        if (TcpStIn) {
            if (TcpStI[tsnx]) {
                TcpStI[tsnx] = 2;
                Lf->sf |= SELNET;
            } else {
                Lf->sf &= ~SELNET;
                Lf->sf |= SELEXCLF;
                return (1);
            }
        }
    }
    if (!(Lf->sf & SELNET) && !TcpStIn) {

        /*
         * See if this TCP file should be selected.
         */
        if (Fnet) {
            if (!FnetTy || ((FnetTy == 4) && (fam == AF_INET)) ||
                ((FnetTy == 6) && (fam == AF_INET6))

            ) {
                Lf->sf |= SELNET;
            }
        }
    }
    return (0);
#else  /* __FreeBSD_version < 1200026 */
    return (-1);
#endif /* __FreeBSD_version >= 1200026 */
}

static void find_pcb_and_xsocket(struct pcb_lists *pcbs, int domain, int type,
                                 uint64_t pcb_addr, void **pcb,
                                 struct xsocket **xsocket) {
    if (!pcbs)
        return;
    if (domain == PF_INET || domain == PF_INET6) {
        if (type == SOCK_STREAM) {
            struct xtcpcb key, *result;
            key.XTCPCB_SO_PCB = pcb_addr;
            result = bsearch(&key, pcbs->tcp_pcbs, pcbs->n_tcp_pcbs,
                             sizeof(*pcbs->tcp_pcbs), cmp_xtcpcb_sock_pcb);
            if (result) {
                *xsocket = &result->XTCPCB_SO;
                *pcb = result;
            }
        } else if (type == SOCK_DGRAM) {
            struct xinpcb key, *result;
            key.xi_socket.so_pcb = pcb_addr;
            result = bsearch(&key, pcbs->udp_pcbs, pcbs->n_udp_pcbs,
                             sizeof(*pcbs->udp_pcbs), cmp_xinpcb_sock_pcb);
            if (result) {
                *xsocket = &result->xi_socket;
                *pcb = result;
            }
        }
    } else if (domain == PF_UNIX) {
        struct xunpcb key, *result = NULL;
        key.xu_socket.so_pcb = pcb_addr;
        if (type == SOCK_STREAM) {
            result =
                bsearch(&key, pcbs->un_stream_pcbs, pcbs->n_un_stream_pcbs,
                        sizeof(*pcbs->un_stream_pcbs), cmp_xunpcb_sock_pcb);
        } else if (type == SOCK_DGRAM) {
            result = bsearch(&key, pcbs->un_dgram_pcbs, pcbs->n_un_dgram_pcbs,
                             sizeof(*pcbs->un_dgram_pcbs), cmp_xunpcb_sock_pcb);
        } else if (type == SOCK_SEQPACKET) {
            result = bsearch(
                &key, pcbs->un_seqpacket_pcbs, pcbs->n_un_seqpacket_pcbs,
                sizeof(*pcbs->un_seqpacket_pcbs), cmp_xunpcb_sock_pcb);
        }
        if (result) {
            *xsocket = &result->xu_socket;
            *pcb = result;
        }
    }
}

/*
 * process_socket() - process socket
 */

void process_socket(struct lsof_context *ctx, struct kinfo_file *kf,
                    struct pcb_lists *pcbs) {
    unsigned char *fa = (unsigned char *)NULL;
    int fam;
    int fp, lp;
    unsigned char *la = (unsigned char *)NULL;
    void *pcb = NULL;
    struct xsocket *s = NULL;
    int ts = -1;
    struct sockaddr_un *ua = NULL;

    int unl;

    Lf->type = LSOF_FILE_SOCKET;
    Lf->inp_ty = 2;
    /*
     * Read the socket, protocol, and domain structures.
     */
    find_pcb_and_xsocket(pcbs, kf->kf_sock_domain, kf->kf_sock_type,
                         kf->kf_un.kf_sock.kf_sock_pcb, &pcb, &s);
/*
 * Save size information.
 */
#if defined(HAS_KF_SOCK_SENDQ)
    if (Lf->access == LSOF_FILE_ACCESS_READ)
        Lf->sz = (SZOFFTYPE)kf->kf_un.kf_sock.kf_sock_recvq;
    else if (Lf->access == LSOF_FILE_ACCESS_WRITE)
        Lf->sz = (SZOFFTYPE)kf->kf_un.kf_sock.kf_sock_sendq;
    else
        Lf->sz = (SZOFFTYPE)kf->kf_un.kf_sock.kf_sock_recvq +
                 (SZOFFTYPE)kf->kf_un.kf_sock.kf_sock_sendq;
    Lf->sz_def = 1;

#    if defined(HASTCPTPIQ)
    Lf->lts.rq = kf->kf_un.kf_sock.kf_sock_recvq;
    Lf->lts.sq = kf->kf_un.kf_sock.kf_sock_sendq;
    Lf->lts.rqs = Lf->lts.sqs = 1;
#    endif /* defined(HASTCPTPIQ) */
#endif     /* defined(HAS_KF_SOCK_SENDQ) */

    if (s) {
        Lf->lts.ltm = (unsigned int)s->so_linger;
        Lf->lts.opt = (unsigned int)s->so_options;

        if (s->so_options & SO_ACCEPTCONN) {
            Lf->lts.pqlen = (unsigned int)s->so_incqlen;
            Lf->lts.qlen = (unsigned int)s->so_qlen;
            Lf->lts.qlim = (unsigned int)s->so_qlimit;
        } else {
            Lf->lts.rbsz = (unsigned long)s->so_rcv.sb_mbmax;
            Lf->lts.sbsz = (unsigned long)s->so_snd.sb_mbmax;
        }
        Lf->lts.pqlens = Lf->lts.qlens = Lf->lts.qlims = Lf->lts.rbszs =
            Lf->lts.sbszs = (unsigned char)1;

        Lf->lts.ss = (unsigned int)s->so_state;
    }
    Lf->lts.sbs_rcv = kf->kf_un.kf_sock.kf_sock_rcv_sb_state;
    Lf->lts.sbs_snd = kf->kf_un.kf_sock.kf_sock_snd_sb_state;

    /*
     * Process socket by the associated domain family.
     */
    switch ((fam = kf->kf_sock_domain)) {
        /*
         * Process an Internet domain socket.
         */
    case AF_INET:
    case AF_INET6:

        if (Fnet) {
            if (!FnetTy || ((FnetTy == 4) && (fam == AF_INET)) ||
                ((FnetTy == 6) && (fam == AF_INET6))) {
                if (!TcpStIn && !UdpStIn)
                    Lf->sf |= SELNET;
            }
        }
        printiproto(ctx, kf->kf_sock_protocol);

        Lf->type = (fam == AF_INET) ? LSOF_FILE_IPV4 : LSOF_FILE_IPV6;

        if (fam == AF_INET6) {
            struct sockaddr_in6 *local_addr6, *foreign_addr6;
            local_addr6 = (struct sockaddr_in6 *)&kf->kf_sa_local;
            foreign_addr6 = (struct sockaddr_in6 *)&kf->kf_sa_peer;
            /*
             * Read IPv6 protocol control block.
             */
            if (!pcb) {
                (void)snpf(Namech, Namechl, "can't read in6pcb at %s",
                           print_kptr((KA_T)kf->kf_un.kf_sock.kf_sock_pcb,
                                      (char *)NULL, 0));
                enter_nm(ctx, Namech);
                return;
            }
            /*
             * Save IPv6 address information.
             */
            if (kf->kf_sock_protocol == IPPROTO_TCP) {
                if ((ts = ckstate(ctx, (struct xtcpcb *)pcb, fam)) == 1) {
                    return;
                }
            }
            enter_dev_ch(ctx,
                         print_kptr((KA_T)(kf->kf_un.kf_sock.kf_sock_inpcb
                                               ? kf->kf_un.kf_sock.kf_sock_inpcb
                                               : kf->kf_un.kf_sock.kf_sock_pcb),
                                    (char *)NULL, 0));
            la = (unsigned char *)&local_addr6->sin6_addr;
            lp = (int)ntohs(local_addr6->sin6_port);
            if (!IN6_IS_ADDR_UNSPECIFIED(&foreign_addr6->sin6_addr) ||
                foreign_addr6->sin6_port) {
                fa = (unsigned char *)&foreign_addr6->sin6_addr;
                fp = (int)ntohs(foreign_addr6->sin6_port);
            }
        } else {

            /*
             * Read Ipv4 protocol control block.
             */
            struct sockaddr_in *local_addr =
                (struct sockaddr_in *)&kf->kf_sa_local;
            struct sockaddr_in *foreign_addr =
                (struct sockaddr_in *)&kf->kf_sa_peer;
            if (!pcb) {
                (void)snpf(
                    Namech, Namechl, "no PCB%s%s",
                    (kf->kf_un.kf_sock.kf_sock_snd_sb_state & SBS_CANTSENDMORE)
                        ? ", CANTSENDMORE"
                        : "",
                    (kf->kf_un.kf_sock.kf_sock_rcv_sb_state & SBS_CANTRCVMORE)
                        ? ", CANTRCVMORE"
                        : "");
                enter_nm(ctx, Namech);
                return;
            }
            if (kf->kf_sock_protocol == IPPROTO_TCP) {
                if ((ts = ckstate(ctx, (struct xtcpcb *)pcb, fam)) == 1)
                    return;
            }
            enter_dev_ch(ctx,
                         print_kptr((KA_T)(kf->kf_un.kf_sock.kf_sock_inpcb
                                               ? kf->kf_un.kf_sock.kf_sock_inpcb
                                               : kf->kf_un.kf_sock.kf_sock_pcb),
                                    (char *)NULL, 0));
            lp = (int)ntohs(local_addr->sin_port);
            la = (unsigned char *)&local_addr->sin_addr;

            /*
             * Save IPv4 address information.
             */
            if (foreign_addr->sin_addr.s_addr != INADDR_ANY ||
                foreign_addr->sin_port) {
                fp = (int)ntohs(foreign_addr->sin_port);
                fa = (unsigned char *)&foreign_addr->sin_addr;
            }
        }

        if ((fam == AF_INET6) &&
            ((la && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)la)) ||
             ((fa && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)fa))))) {

            /*
             * Adjust for IPv4 addresses mapped in IPv6 addresses.
             */
            if (la)
                la = (unsigned char *)IPv6_2_IPv4(la);
            if (fa)
                fa = (unsigned char *)IPv6_2_IPv4(fa);
            fam = AF_INET;
        }

        /*
         * Enter local and remote addresses by address family.
         */
        if (fa || la)
            (void)ent_inaddr(ctx, la, lp, fa, fp, fam);
        if (ts == 0) {
            struct xtcpcb *tcp_pcb = (struct xtcpcb *)pcb;
            Lf->lts.type = 0;
#if __FreeBSD_version >= 1200026
            Lf->lts.state.i = (int)tcp_pcb->t_state;
#endif

#if defined(HASTCPOPT)
#    if defined(HAS_XTCPCB_TMAXSEG)
            Lf->lts.mss = (unsigned long)tcp_pcb->t_maxseg;
            Lf->lts.msss = (unsigned char)1;
#    endif /* defined(HAS_XTCPCB_TMAXSEG) */
#    if __FreeBSD_version >= 1200026
            Lf->lts.topt = (unsigned int)tcp_pcb->t_flags;
#    endif
#endif /* defined(HASTCPOPT) */
        }
        break;
        /*
         * Process a ROUTE domain socket.
         */
    case AF_ROUTE:
        Lf->type = LSOF_FILE_ROUTE;
        if (s && s->so_pcb)
            enter_dev_ch(ctx, print_kptr((KA_T)(s->so_pcb), (char *)NULL, 0));
        else
            (void)snpf(Namech, Namechl, "no protocol control block");
        break;
        /*
         * Process a Unix domain socket.
         */
    case AF_UNIX: {
        struct xunpcb *unix_pcb = (struct xunpcb *)pcb;
        if (Funix)
            Lf->sf |= SELUNX;
        Lf->type = LSOF_FILE_UNIX;
        /*
         * Read Unix protocol control block and the Unix address structure.
         */

        if (unix_pcb)
            enter_dev_ch(
                ctx, print_kptr(unix_pcb->xu_socket.xso_so, (char *)NULL, 0));
        else {
            (void)snpf(
                Namech, Namechl, "can't read unpcb at %s",
                print_kptr(kf->kf_un.kf_sock.kf_sock_pcb, (char *)NULL, 0));
            break;
        }
        ua = (struct sockaddr_un *)&kf->kf_sa_local;

        if (ua->sun_path[0]) {

            unl = ua->sun_len - offsetof(struct sockaddr_un, sun_path);
            if ((unl < 0) || (unl >= sizeof(ua->sun_path)))
                unl = sizeof(ua->sun_path) - 1;
            ua->sun_path[unl] = '\0';

            if (ua->sun_path[0] && Sfile && is_file_named(ctx, ua->sun_path, 0))
                Lf->sf |= SELNM;
            if (ua->sun_path[0] && !Namech[0])
                (void)snpf(Namech, Namechl, "%s", ua->sun_path);
        } else if (kf->kf_un.kf_sock.kf_sock_unpconn) {
            struct xsocket *peer_socket = NULL;
            void *peer_pcb;
            find_pcb_and_xsocket(pcbs, kf->kf_sock_domain, kf->kf_sock_type,
                                 kf->kf_un.kf_sock.kf_sock_unpconn, &peer_pcb,
                                 &peer_socket);
            if (!peer_socket)
                (void)snpf(Namech, Namechl, "can't read unp_conn at %s",
                           print_kptr((KA_T)kf->kf_un.kf_sock.kf_sock_unpconn,
                                      (char *)NULL, 0));
            else
                (void)snpf(
                    Namech, Namechl, "->%s",
                    print_kptr((KA_T)peer_socket->xso_so, (char *)NULL, 0));
        } else
            (void)snpf(Namech, Namechl, "->(none)");
        break;
    }
    default:
        printunkaf(ctx, fam, 1);
    }
    if (Namech[0])
        enter_nm(ctx, Namech);
}
