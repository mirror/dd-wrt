/*
 *  $Id: libnet_link_dlpi.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_dlpi.c - dlpi routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1993, 1994, 1995, 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This code contributed by Atanu Ghosh (atanu@cs.ucl.ac.uk),
 * University College London.
 */


#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_SYS_BUFMOD_H
#include <sys/bufmod.h>
#endif
#include <sys/dlpi.h>
#ifdef HAVE_HPUX9
#include <sys/socket.h>
#endif
#ifdef DL_HP_PPA_ACK_OBS
#include <sys/stat.h>
#endif
#include <sys/stream.h>
#if defined(HAVE_SOLARIS) && defined(HAVE_SYS_BUFMOD_H)
#include <sys/systeminfo.h>
#endif

#ifdef HAVE_HPUX9
#include <net/if.h>
#endif

#include <ctype.h>
#ifdef HAVE_HPUX9
#include <nlist.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include <unistd.h>

#include "../include/libnet.h"
#include "../include/bpf.h"

#include "../include/gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif

#ifndef DLPI_DEV_PREFIX
#define DLPI_DEV_PREFIX "/dev"
#endif

#define	MAXDLBUF 8192

/* Forwards */
static int dlattachreq(int, bpf_u_int32, char *);
static int dlbindack(int, char *, char *);
static int dlbindreq(int, bpf_u_int32, char *);
static int dlinfoack(int, char *, char *);
static int dlinforeq(int, char *);
static int dlokack(int, const char *, char *, char *);
static int recv_ack(int, int, const char *, char *, char *);
static int send_request(int, char *, int, char *, char *, int);
#ifdef HAVE_SYS_BUFMOD_H
static int strioctl(int, int, int, char *);
#endif
#ifdef HAVE_HPUX9
static int dlpi_kread(int, off_t, void *, u_int, char *);
#endif
#ifdef HAVE_DEV_DLPI
static int get_dlpi_ppa(int, const char *, int, char *);
#endif

/* XXX Needed by HP-UX (at least) */
static bpf_u_int32 ctlbuf[MAXDLBUF];


struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    register char *cp;
    char *eos;
    register struct libnet_link_int *l;
    register int ppa;
    register dl_info_ack_t *infop;
    bpf_u_int32 buf[MAXDLBUF];
    char dname[100];
#ifndef HAVE_DEV_DLPI
    char dname2[100];
#endif

    l = (struct libnet_link_int *)malloc(sizeof(*l));
    if (l == NULL)
    {
        strcpy(ebuf, ll_strerror(errno));
        return (NULL);
    }
    memset(l, 0, sizeof(*l));

    /*
     *  Determine device and ppa
     */
    cp = strpbrk(device, "0123456789");
    if (cp == NULL)
    {
        sprintf(ebuf, "%s missing unit number", device);
        goto bad;
    }
    ppa = strtol(cp, &eos, 10);
    if (*eos != '\0')
    {
        sprintf(ebuf, "%s bad unit number", device);
        goto bad;
    }

    if (*device == '/')
    {
        strcpy(dname, device);
    }
    else
    {
        sprintf(dname, "%s/%s", DLPI_DEV_PREFIX, device);
    }
#ifdef HAVE_DEV_DLPI
    /*
     *  Map network device to /dev/dlpi unit
     */
    cp = "/dev/dlpi";

    l->fd = open(cp, O_RDWR);
    if (l->fd == -1)
    {
        sprintf(ebuf, "%s: %s", cp, ll_strerror(errno));
        goto bad;
    }

    /*
     *  Map network interface to /dev/dlpi unit
     */
    ppa = get_dlpi_ppa(l->fd, dname, ppa, ebuf);
    if (ppa < 0)
    {
        goto bad;
    }
#else
    /*
     *  Try device without unit number
     */
    strcpy(dname2, dname);
    cp = strchr(dname, *cp);
    *cp = '\0';

    l->fd = open(dname, O_RDWR);
    if (l->fd == -1)
    {
        if (errno != ENOENT)
        {
            sprintf(ebuf, "%s: %s", dname, ll_strerror(errno));
            goto bad;
        }

        /*
         *  Try again with unit number
         */
        l->fd = open(dname2, O_RDWR);
        if (l->fd == -1)
        {
            sprintf(ebuf, "%s: %s", dname2, ll_strerror(errno));
            goto bad;
        }

        cp = dname2;
        while (*cp && !isdigit(*cp)) cp++;
        if (*cp) ppa = atoi(cp);
        else
        /*
         *  XXX Assume unit zero
         */
        ppa = 0;
    }
#endif
    /*
     *  Attach if "style 2" provider
     */
    if (dlinforeq(l->fd, ebuf) < 0 || dlinfoack(l->fd, (char *)buf, ebuf) < 0)
    {
        goto bad;
    }
    infop = &((union DL_primitives *)buf)->info_ack;
    if (infop->dl_provider_style == DL_STYLE2 && (dlattachreq(l->fd, ppa, ebuf)
        < 0 || dlokack(l->fd, "attach", (char *)buf, ebuf) < 0))
    {
        goto bad;
    }

    /*
     *  Bind HP-UX 9 and HP-UX 10.20
     */
#if defined(HAVE_HPUX9) || defined(HAVE_HPUX10_20) || defined(HAVE_SOLARIS)
    if (dlbindreq(l->fd, 0, ebuf) < 0 || dlbindack(l->fd, (char *)buf, ebuf) < 0)
    {
        goto bad;
    }
#endif

    /*
     *  Determine link type
     */
    if (dlinforeq(l->fd, ebuf) < 0 || dlinfoack(l->fd, (char *)buf, ebuf) < 0)
    {
        goto bad;
    }

    infop = &((union DL_primitives *)buf)->info_ack;
    switch (infop->dl_mac_type)
    {
        case DL_CSMACD:
        case DL_ETHER:
            l->linktype = DLT_EN10MB;
            break;
        case DL_FDDI:
            l->linktype = DLT_FDDI;
            break;
        default:
            sprintf(ebuf, "unknown mac type 0x%lu", infop->dl_mac_type);
            goto bad;
    }

#ifdef	DLIOCRAW
    /*
     *  This is a non standard SunOS hack to get the ethernet header.
     */
    if (strioctl(l->fd, DLIOCRAW, 0, NULL) < 0)
    {
        sprintf(ebuf, "DLIOCRAW: %s", ll_strerror(errno));
        goto bad;
    }
#endif

    return (l);
bad:
    free(l);
    return (NULL);
}


static int
send_request(int fd, char *ptr, int len, char *what, char *ebuf, int flags)
{
    struct strbuf ctl;

    ctl.maxlen = 0;
    ctl.len = len;
    ctl.buf = ptr;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0)
    {
        sprintf(ebuf, "send_request: putmsg \"%s\": %s", what, ll_strerror(errno));
        return (-1);
    }
    return (0);
}

static int
recv_ack(int fd, int size, const char *what, char *bufp, char *ebuf)
{
    union DL_primitives *dlp;
    struct strbuf ctl;
    int flags;

    ctl.maxlen = MAXDLBUF;
    ctl.len = 0;
    ctl.buf = bufp;

    flags = 0;
    if (getmsg(fd, &ctl, (struct strbuf*)NULL, &flags) < 0)
    {
        sprintf(ebuf, "recv_ack: %s getmsg: %s", what, ll_strerror(errno));
        return (-1);
    }

    dlp = (union DL_primitives *)ctl.buf;
    switch (dlp->dl_primitive)
    {
        case DL_INFO_ACK:
        case DL_PHYS_ADDR_ACK:
        case DL_BIND_ACK:
        case DL_OK_ACK:
#ifdef DL_HP_PPA_ACK
        case DL_HP_PPA_ACK:
#endif
        /*
         *  These are OK
         */
        break;

        case DL_ERROR_ACK:
            switch (dlp->error_ack.dl_errno)
            {
                case DL_BADPPA:
                    sprintf(ebuf, "recv_ack: %s bad ppa (device unit)", what);
                    break;
                case DL_SYSERR:
                    sprintf(ebuf, "recv_ack: %s: %s",
                        what, ll_strerror(dlp->error_ack.dl_unix_errno));
                    break;
                case DL_UNSUPPORTED:
                    sprintf(ebuf,
                        "recv_ack: %s: Service not supplied by provider", what);
                    break;
                default:
                    sprintf(ebuf, "recv_ack: %s error 0x%x", what,
                        (bpf_u_int32)dlp->error_ack.dl_errno);
                    break;
            }
            return (-1);

        default:
            sprintf(ebuf, "recv_ack: %s unexpected primitive ack 0x%x ",
                what, (bpf_u_int32)dlp->dl_primitive);
            return (-1);
    }

    if (ctl.len < size)
    {
        sprintf(ebuf, "recv_ack: %s ack too small (%d < %d)",
            what, ctl.len, size);
        return (-1);
    }
    return (ctl.len);
}

static int
dlpromiscoffreq(int fd, bpf_u_int32 level, char *ebuf)
{
    dl_promiscon_req_t req;

    req.dl_primitive = DL_PROMISCOFF_REQ;
    req.dl_level     = level;

    return (send_request(fd, (char *)&req, sizeof(req), "promiscoff", ebuf, 0));
}

static int
dlpromisconreq(int fd, bpf_u_int32 level, char *ebuf)
{
    dl_promiscon_req_t req;

    req.dl_primitive = DL_PROMISCON_REQ;
    req.dl_level     = level;

    return (send_request(fd, (char *)&req, sizeof(req), "promiscon", ebuf, 0));
}


static int
dlattachreq(int fd, bpf_u_int32 ppa, char *ebuf)
{
    dl_attach_req_t req;

    req.dl_primitive = DL_ATTACH_REQ;
    req.dl_ppa       = ppa;

    return (send_request(fd, (char *)&req, sizeof(req), "attach", ebuf, 0));
}

static int
dlbindreq(int fd, bpf_u_int32 sap, char *ebuf)
{

    dl_bind_req_t	req;

    memset((char *)&req, 0, sizeof(req));
    req.dl_primitive = DL_BIND_REQ;
#ifdef DL_HP_RAWDLS
    req.dl_max_conind = 1;  /* XXX magic number */
    /*
     *  22 is INSAP as per the HP-UX DLPI Programmer's Guide
     */
    req.dl_sap = 22;
    req.dl_service_mode = DL_HP_RAWDLS;
#else
    req.dl_sap = sap;
#ifdef DL_CLDLS
    req.dl_service_mode = DL_CLDLS;
#endif
#endif
    return (send_request(fd, (char *)&req, sizeof(req), "bind", ebuf, 0));
}


static int
dlbindack(int fd, char *bufp, char *ebuf)
{
    return (recv_ack(fd, DL_BIND_ACK_SIZE, "bind", bufp, ebuf));
}


static int
dlokack(int fd, const char *what, char *bufp, char *ebuf)
{
    return (recv_ack(fd, DL_OK_ACK_SIZE, what, bufp, ebuf));
}


static int
dlinforeq(int fd, char *ebuf)
{
    dl_info_req_t req;

    req.dl_primitive = DL_INFO_REQ;

    return (send_request(fd, (char *)&req, sizeof(req), "info", ebuf, RS_HIPRI));
}

static int
dlinfoack(int fd, char *bufp, char *ebuf)
{
    return (recv_ack(fd, DL_INFO_ACK_SIZE, "info", bufp, ebuf));
}


#ifdef HAVE_SYS_BUFMOD_H
static int
strioctl(int fd, int cmd, int len, char *dp)
{
    struct strioctl str;
    int rc;

    str.ic_cmd    = cmd;
    str.ic_timout = -1;
    str.ic_len    = len;
    str.ic_dp     = dp;
    
    rc = ioctl(fd, I_STR, &str);
    if (rc < 0)
    {
        return (rc);
    }
    else
    {
        return (str.ic_len);
    }
}
#endif


#ifdef DL_HP_PPA_ACK_OBS
/*
 * Under HP-UX 10, we can ask for the ppa
 */
static int
get_dlpi_ppa(register int fd, register const char *device, register int unit,
    register char *ebuf)
{
    register dl_hp_ppa_ack_t *ap;
    register dl_hp_ppa_info_t *ip;
    register int i;
    register u_long majdev;
    dl_hp_ppa_req_t	req;
    struct stat statbuf;
    bpf_u_int32 buf[MAXDLBUF];

    if (stat(device, &statbuf) < 0)
    {
        sprintf(ebuf, "stat: %s: %s", device, ll_strerror(errno));
        return (-1);
    }
    majdev = major(statbuf.st_rdev);

    memset((char *)&req, 0, sizeof(req));
    req.dl_primitive = DL_HP_PPA_REQ;

    memset((char *)buf, 0, sizeof(buf));
    if (send_request(fd, (char *)&req, sizeof(req), "hpppa", ebuf, 0) < 0 ||
        recv_ack(fd, DL_HP_PPA_ACK_SIZE, "hpppa", (char *)buf, ebuf) < 0)
    {
        return (-1);
    }

    ap = (dl_hp_ppa_ack_t *)buf;
    ip = (dl_hp_ppa_info_t *)((u_char *)ap + ap->dl_offset);

    for (i = 0; i < ap->dl_count; i++)
    {
        if (ip->dl_mjr_num == majdev && ip->dl_instance_num == unit)
        break;

        ip = (dl_hp_ppa_info_t *)((u_char *)ip + ip->dl_next_offset);
    }

    if (i == ap->dl_count)
    {
        sprintf(ebuf, "can't find PPA for %s", device);
        return (-1);
    }

    if (ip->dl_hdw_state == HDW_DEAD)
    {
        sprintf(ebuf, "%s: hardware state: DOWN\n", device);
        return (-1);
    }
    return ((int)ip->dl_ppa);
}
#endif

#ifdef HAVE_HPUX9
/*
 * Under HP-UX 9, there is no good way to determine the ppa.
 * So punt and read it from /dev/kmem.
 */
static struct nlist nl[] =
{
#define NL_IFNET 0
    { "ifnet" },
    { "" }
};

static char path_vmunix[] = "/hp-ux";

/*
 *  Determine ppa number that specifies ifname
 */
static int
get_dlpi_ppa(register int fd, register const char *ifname, register int unit,
    register char *ebuf)
{
    register const char *cp;
    register int kd;
    void *addr;
    struct ifnet ifnet;
    char if_name[sizeof(ifnet.if_name)], tifname[32];

    cp = strrchr(ifname, '/');
    if (cp != NULL)
    {
        ifname = cp + 1;
    }
    if (nlist(path_vmunix, &nl) < 0)
    {
        sprintf(ebuf, "nlist %s failed", path_vmunix);
        return (-1);
    }

    if (nl[NL_IFNET].n_value == 0)
    {
        sprintf(ebuf, "could't find %s kernel symbol", nl[NL_IFNET].n_name);
        return (-1);
    }

    kd = open("/dev/kmem", O_RDONLY);
    if (kd < 0)
    {
        sprintf(ebuf, "kmem open: %s", ll_strerror(errno));
        return (-1);
    }

    if (dlpi_kread(kd, nl[NL_IFNET].n_value, &addr, sizeof(addr), ebuf) < 0)
    {
        close(kd);
        return (-1);
    }
    for (; addr != NULL; addr = ifnet.if_next)
    {
        if (dlpi_kread(kd, (off_t)addr, &ifnet, sizeof(ifnet), ebuf) < 0 ||
            dlpi_kread(kd, (off_t)ifnet.if_name,
            if_name, sizeof(if_name), ebuf) < 0)
            {
                close(kd);
                return (-1);
            }
            sprintf(tifname, "%.*s%d",
                (int)sizeof(if_name), if_name, ifnet.if_unit);
            if (strcmp(tifname, ifname) == 0)
            {
                return (ifnet.if_index);
            }
    }

    sprintf(ebuf, "Can't find %s", ifname);
    return (-1);
}

static int
dlpi_kread(register int fd, register off_t addr,
    register void *buf, register u_int len, register char *ebuf)
{
    register int cc;

    if (lseek(fd, addr, SEEK_SET) < 0)
    {
        sprintf(ebuf, "lseek: %s", ll_strerror(errno));
        return (-1);
    }
    cc = read(fd, buf, len);
    if (cc < 0)
    {
        sprintf(ebuf, "read: %s", ll_strerror(errno));
        return (-1);
    }
    else if (cc != len)
    {
        sprintf(ebuf, "short read (%d != %d)", cc, len);
        return (-1);
    }
    return (cc);
}
#endif

/*#include <netinet/if_ether.h>*/
#define ETHERADDRL 6
struct  EnetHeaderInfo
{
    struct ether_addr   DestEtherAddr;
    u_short             EtherFrameType;
};


int
libnet_close_link_interface(struct libnet_link_int *l)
{
    return (close(l->fd));
}


struct EnetHeaderInfo ArpHeader =
{
    {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}}, ETHERTYPE_ARP
};


int
libnet_write_link_layer(struct libnet_link_int *l, const u_char *device,
            u_char *buf, int len)
{
    struct strbuf data, ctl;
    union DL_primitives *dlp;
    int c;
    struct EnetHeaderInfo *EnetHeaderInfoP;

    dlp = (union DL_primitives*) ctlbuf;
    dlp->unitdata_req.dl_primitive        = DL_UNITDATA_REQ;
    dlp->unitdata_req.dl_priority.dl_min  = 0;
    dlp->unitdata_req.dl_priority.dl_max  = 0;
    dlp->unitdata_req.dl_dest_addr_length = (sizeof(struct ether_addr) +
                                            sizeof(u_short));
    dlp->unitdata_req.dl_dest_addr_offset = DL_UNITDATA_REQ_SIZE;

    EnetHeaderInfoP = (struct EnetHeaderInfo *)(ctlbuf + DL_UNITDATA_REQ_SIZE);
    memcpy(EnetHeaderInfoP, (char *)&(ArpHeader), (sizeof(struct ether_addr) +
                                                  sizeof(u_short)));

    /* Send it */
    ctl.len = DL_UNITDATA_REQ_SIZE + sizeof (struct EnetHeaderInfo);
    ctl.buf = (char *)dlp;

    data.maxlen = len;
    data.len    = len;
    data.buf    = buf;

    c = putmsg(l->fd, NULL, &data, 0);
    if (c == -1)
    {
#if (__DEBUG)
        fprintf(stderr, "write_link_layer: (%s)\n", strerror(errno));
#endif
        return (-1);
    }
    return (len);
}


struct ether_addr *
libnet_get_hwaddr(struct libnet_link_int *l, const u_char *device, char *ebuf)
{
    char    buf[2048];
    union DL_primitives *dlp;
    struct ether_addr *eap;

    dlp = (union DL_primitives*) buf;

    dlp->physaddr_req.dl_primitive = DL_PHYS_ADDR_REQ;
    dlp->physaddr_req.dl_addr_type = DL_CURR_PHYS_ADDR;

    if (send_request(l->fd, (char *)dlp, DL_PHYS_ADDR_REQ_SIZE, "physaddr",
                    ebuf, 0) < 0)
    {
        sprintf(ebuf, "get_hwaddr %s", strerror(errno));
        return (NULL);
    }
    if (recv_ack(l->fd, DL_PHYS_ADDR_ACK_SIZE, "physaddr", (char *)dlp, ebuf)
        < 0)
    {
        sprintf(ebuf, "get_hwaddr %s", strerror(errno));
        return (NULL);
    }

    eap = (struct ether_addr *)
        ((char *) dlp + dlp->physaddr_ack.dl_addr_offset);
    return (eap);
}   

/* EOF */
