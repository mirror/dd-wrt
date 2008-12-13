/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2007 Apsis GmbH
 *
 * This file is part of Pound.
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information:
 * Apsis GmbH
 * P.O.Box
 * 8707 Uetikon am See
 * Switzerland
 * Tel: +41-44-920 4904
 * EMail: roseg@apsis.ch
 */
#define NO_EXTERNALS 1
#include    "pound.h"

static int  xml_out = 0;

static void
usage(const char *arg0)
{
    fprintf(stderr, "Usage: %s -c /control/socket [ -X ] cmd\n", arg0);
    fprintf(stderr, "\twhere cmd is one of:\n");
    fprintf(stderr, "\t-L n - enable listener n\n");
    fprintf(stderr, "\t-l n - disable listener n\n");
    fprintf(stderr, "\t-S n m - enable service m in service n (use -1 for global services)\n");
    fprintf(stderr, "\t-s n m - disable service m in service n (use -1 for global services)\n");
    fprintf(stderr, "\t-B n m r - enable back-end r in service m in listener n\n");
    fprintf(stderr, "\t-b n m r - disable back-end r in service m in listener n\n");
    fprintf(stderr, "\t-N n m k r - add a session with key k and back-end r in service m in listener n\n");
    fprintf(stderr, "\t-n n m k - remove a session with key k r in service m in listener n\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\tentering the command without arguments lists the current configuration.\n");
    fprintf(stderr, "\tthe -X flags results in XML output.\n");
    exit(1);
}

/*
 * Translate inet/inet6 address/port into a string
 */
static char *
prt_addr(const struct addrinfo *addr)
{
    static char res[64];
    char        buf[64];
    int         port;
    void        *src;

    memset(buf, 0, 64);
#ifdef  HAVE_INET_NTOP
    switch(addr->ai_family) {
    case AF_INET:
        src = (void *)&((struct sockaddr_in *)addr->ai_addr)->sin_addr.s_addr;
        port = ntohs(((struct sockaddr_in *)addr->ai_addr)->sin_port);
        if(inet_ntop(AF_INET, src, buf, 63) == NULL)
            strncpy(buf, "(UNKNOWN)", 63);
        break;
    case AF_INET6:
        src = (void *)&((struct sockaddr_in6 *)addr->ai_addr)->sin6_addr.s6_addr;
        port = ntohs(((struct sockaddr_in6 *)addr->ai_addr)->sin6_port);
        if(inet_ntop(AF_INET6, src, buf, 63) == NULL)
            strncpy(buf, "(UNKNOWN)", 63);
        break;
    case AF_UNIX:
        strncpy(buf, (char *)addr->ai_addr, 63);
        port = 0;
        break;
    default:
        strncpy(buf, "(UNKNOWN)", 63);
        port = 0;
        break;
    }
    snprintf(res, 63, "%s:%d", buf, port);
#else
#error "Pound needs inet_ntop()"
#endif
    return res;
}

static void
be_prt(const int sock)
{
    BACKEND be;
    struct  sockaddr_storage    a, h;
    int     n_be;

    n_be = 0;
    while(read(sock, (void *)&be, sizeof(BACKEND)) == sizeof(BACKEND)) {
        if(be.disabled < 0)
            break;
        read(sock, &a, be.addr.ai_addrlen);
        be.addr.ai_addr = (struct sockaddr *)&a;
        if(be.ha_addr.ai_addrlen > 0) {
            read(sock, &h, be.ha_addr.ai_addrlen);
            be.ha_addr.ai_addr = (struct sockaddr *)&h;
        }
        if(xml_out)
            printf("<backend index=\"%d\" address=\"%s\" average=\"%.3f\" priority=\"%d\"%s%s />\n", n_be++,
                prt_addr(&be.addr), be.t_average / 1000000, be.priority, be.alive? "": " DEAD",
                be.disabled? " DISABLED": "");
        else
            printf("    %3d. Backend %s %s (%d %.3f sec) %s\n", n_be++, prt_addr(&be.addr),
                be.disabled? "DISABLED": "active", be.priority, be.t_average / 1000000, be.alive? "alive": "DEAD");
    }
    return;
}

static void
sess_prt(const int sock)
{
    TABNODE     sess;
    int         n_be, n_sess, cont_len;
    char        buf[KEY_SIZE + 1], escaped[KEY_SIZE * 2 + 1];

    n_sess = 0;
    while(read(sock, (void *)&sess, sizeof(TABNODE)) == sizeof(TABNODE)) {
        if(sess.content == NULL)
            break;
        read(sock, &n_be, sizeof(n_be));
        read(sock, &cont_len, sizeof(cont_len));
        memset(buf, 0, KEY_SIZE + 1);
        /* cont_len is at most KEY_SIZE */
        read(sock, buf, cont_len);
        if(xml_out) {
            int     i, j;
            char    escaped[KEY_SIZE * 2 + 1];

            for(i = j = 0; buf[i]; i++)
                if(buf[i] == '"') {
                    escaped[j++] = '\\';
                    escaped[j++] = '"';
                } else
                    escaped[j++] = buf[i];
            escaped[j] = '\0';
            printf("<session index=\"%d\" key=\"%s\" backend=\"%d\" />\n", n_sess++, escaped, n_be);
        } else
            printf("    %3d. Session %s -> %d\n", n_sess++, buf, n_be);
    }
    return;
}

static void
svc_prt(const int sock)
{
    SERVICE     svc;
    int         n_svc;

    n_svc = 0;
    while(read(sock, (void *)&svc, sizeof(SERVICE)) == sizeof(SERVICE)) {
        if(svc.disabled < 0)
            break;
        if(xml_out) {
            if(svc.name[0])
                printf("<service index=\"%d\" name=\"%s\"%s>\n", n_svc++, svc.name, svc.disabled? " DISABLED": "");
            else
                printf("<service index=\"%d\"%s>\n", n_svc++, svc.disabled? " DISABLED": "");
        } else {
            if(svc.name[0])
                printf("  %3d. Service \"%s\" %s (%d)\n", n_svc++, svc.name, svc.disabled? "DISABLED": "active",
                    svc.tot_pri);
            else
                printf("  %3d. Service %s (%d)\n", n_svc++, svc.disabled? "DISABLED": "active", svc.tot_pri);
        }
        be_prt(sock);
        sess_prt(sock);
        if(xml_out)
            printf("</service>\n");
    }
    return;
}

static int
get_sock(const char *sock_name)
{
    struct sockaddr_un  ctrl;
    int                 res;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.sun_family = AF_UNIX;
    strncpy(ctrl.sun_path, sock_name, sizeof(ctrl.sun_path) - 1);
    if((res = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket create");
        exit(1);
    }
    if(connect(res, (struct sockaddr *)&ctrl, (socklen_t)sizeof(ctrl)) < 0) {
        perror("connect");
        exit(1);
    }
    return res;
}

main(const int argc, char **argv)
{
    CTRL_CMD    cmd;
    int         sock, n_lstn, n_svc, n_be, n_sess, i;
    char        *arg0, *sock_name, buf[KEY_SIZE + 1];
    int         c_opt, en_lst, de_lst, en_svc, de_svc, en_be, de_be, a_sess, d_sess, is_set;
    LISTENER    lstn;
    SERVICE     svc;
    BACKEND     be;
    TABNODE     sess;
    struct  sockaddr_storage    a;

    arg0 = *argv;
    sock_name = NULL;
    en_lst = de_lst = en_svc = de_svc = en_be = de_be = is_set = a_sess = d_sess = 0;
    memset(&cmd, 0, sizeof(cmd));
    opterr = 0;
    i = 0;
    while(!i && (c_opt = getopt(argc, argv, "c:LlSsBbNnX")) > 0)
        switch(c_opt) {
        case 'c':
            sock_name = optarg;
            break;
        case 'X':
            xml_out = 1;
            break;
        case 'L':
            if(is_set)
                usage(arg0);
            en_lst = is_set = 1;
            break;
        case 'l':
            if(is_set)
                usage(arg0);
            de_lst = is_set = 1;
            break;
        case 'S':
            if(is_set)
                usage(arg0);
            en_svc = is_set = 1;
            break;
        case 's':
            if(is_set)
                usage(arg0);
            de_svc = is_set = 1;
            break;
        case 'B':
            if(is_set)
                usage(arg0);
            en_be = is_set = 1;
            break;
        case 'b':
            if(is_set)
                usage(arg0);
            de_be = is_set = 1;
            break;
        case 'N':
            if(is_set)
                usage(arg0);
            a_sess = is_set = 1;
            break;
        case 'n':
            if(is_set)
                usage(arg0);
            d_sess = is_set = 1;
            break;
        default:
            if(optopt == '1') {
                optind--;
                i = 1;
            } else {
                fprintf(stderr, "bad flag -%c", optopt);
                usage(arg0);
            }
            break;
        }

    if(sock_name == NULL)
        usage(arg0);
    if(en_lst || de_lst) {
        if(optind != (argc - 1))
            usage(arg0);
        cmd.cmd = (en_lst? CTRL_EN_LSTN: CTRL_DE_LSTN);
        cmd.listener = atoi(argv[optind++]);
    }
    if(en_svc || de_svc) {
        if(optind != (argc - 2))
            usage(arg0);
        cmd.cmd = (en_svc? CTRL_EN_SVC: CTRL_DE_SVC);
        cmd.listener = atoi(argv[optind++]);
        cmd.service = atoi(argv[optind++]);
    }
    if(en_be || de_be) {
        if(optind != (argc - 3))
            usage(arg0);
        cmd.cmd = (en_be? CTRL_EN_BE: CTRL_DE_BE);
        cmd.listener = atoi(argv[optind++]);
        cmd.service = atoi(argv[optind++]);
        cmd.backend = atoi(argv[optind++]);
    }
    if(a_sess) {
        if(optind != (argc - 4))
            usage(arg0);
        cmd.cmd = CTRL_ADD_SESS;
        cmd.listener = atoi(argv[optind++]);
        cmd.service = atoi(argv[optind++]);
        memset(cmd.key, 0, KEY_SIZE + 1);
        strncpy(cmd.key, argv[optind++], KEY_SIZE);
        cmd.backend = atoi(argv[optind++]);
    }
    if(d_sess) {
        if(optind != (argc - 3))
            usage(arg0);
        cmd.cmd = CTRL_DEL_SESS;
        cmd.listener = atoi(argv[optind++]);
        cmd.service = atoi(argv[optind++]);
        strncpy(cmd.key, argv[optind++], KEY_SIZE);
    }
    if(!is_set) {
        if(optind != argc)
            usage(arg0);
        cmd.cmd = CTRL_LST;
    }

    sock = get_sock(sock_name);
    write(sock, &cmd, sizeof(cmd));

    if (!is_set) {
        n_lstn = 0;
        if(xml_out)
            printf("<pound>\n");
        while(read(sock, (void *)&lstn, sizeof(LISTENER)) == sizeof(LISTENER)) {
            if(lstn.disabled < 0)
                break;
            read(sock, &a, lstn.addr.ai_addrlen);
            lstn.addr.ai_addr = (struct sockaddr *)&a;
            if(xml_out)
                printf("<listener index=\"%d\" %s address=\"%s\"%s>\n", n_lstn++, lstn.ctx? "HTTPS": "HTTP",
                    prt_addr(&lstn.addr), lstn.disabled? " DISABLED": "");
            else
                printf("%3d. %s Listener %s %s\n", n_lstn++, lstn.ctx? "HTTPS" : "http",
                    prt_addr(&lstn.addr), lstn.disabled? "*D": "a");
            svc_prt(sock);
            if(xml_out)
                printf("</listener>\n");
        }
        if(!xml_out)
            printf(" -1. Global services\n");
        svc_prt(sock);
        if(xml_out)
            printf("</pound>\n");
    }
    return 0;
}
