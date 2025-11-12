/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2016 by Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

static uint16_t parse_family(char *str, const char *option)
{
        if (strcmp(str, "ip") == 0)
                return NFPROTO_IPV4;
        else if (strcmp(str, "ip6") == 0)
                return NFPROTO_IPV6;
        else if (strcmp(str, "inet") == 0)
                return NFPROTO_INET;
        else {
                fprintf(stderr, "Unknown %s: ip, ip6, inet\n", option);
                exit(EXIT_FAILURE);
        }
}

static uint8_t parse_l4proto(char *str)
{
        if (strcmp(str, "udp") == 0)
                return IPPROTO_UDP;
        else if (strcmp(str, "tcp") == 0)
                return IPPROTO_TCP;
        else {
                fprintf(stderr, "Unknown l4proto: tcp, udp\n");
                exit(EXIT_FAILURE);
        }
        return IPPROTO_TCP;
}

static struct nftnl_obj *ct_helper_add_parse(int argc, char *argv[])
{
        struct nftnl_obj *t;
        uint16_t family, l3proto;
        uint8_t l4proto;

        t = nftnl_obj_alloc();
        if (t == NULL) {
                perror("OOM");
                return NULL;
        }

        family = parse_family(argv[1], "family");
        nftnl_obj_set_u32(t, NFTNL_OBJ_FAMILY, family);
        nftnl_obj_set_u32(t, NFTNL_OBJ_TYPE, NFT_OBJECT_CT_HELPER);
        nftnl_obj_set_str(t, NFTNL_OBJ_TABLE, argv[2]);
        nftnl_obj_set_str(t, NFTNL_OBJ_NAME, argv[3]);

        nftnl_obj_set_str(t, NFTNL_OBJ_CT_HELPER_NAME, argv[4]);
        l4proto = parse_l4proto(argv[5]);
        nftnl_obj_set_u8(t, NFTNL_OBJ_CT_HELPER_L4PROTO, l4proto);
        if (argc == 7) {
                l3proto = parse_family(argv[6], "l3proto");
                nftnl_obj_set_u16(t, NFTNL_OBJ_CT_HELPER_L3PROTO, l3proto);
        }

        return t;
}

int main(int argc, char *argv[])
{
        struct nftnl_obj *t;
        uint32_t seq, obj_seq, family, portid;
        struct mnl_nlmsg_batch *batch;
        char buf[MNL_SOCKET_BUFFER_SIZE];
        struct nlmsghdr *nlh;
        struct mnl_socket *nl;
        int ret;

        if (argc < 6) {
                fprintf(stderr, "%s <family> <table> <name> <type> <l4proto> [l3proto]\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        t = ct_helper_add_parse(argc, argv);
        if (t == NULL)
                exit(EXIT_FAILURE);

        seq = time(NULL);
        batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

        nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
        mnl_nlmsg_batch_next(batch);

        obj_seq = seq;
        family = nftnl_obj_get_u32(t, NFTNL_OBJ_FAMILY);
        nlh = nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
                                    NFT_MSG_NEWOBJ, family, NLM_F_CREATE | NLM_F_ACK, seq++);

        nftnl_obj_nlmsg_build_payload(nlh, t);
        nftnl_obj_free(t);
        mnl_nlmsg_batch_next(batch);

        nftnl_batch_end(mnl_nlmsg_batch_current(batch), seq++);
        mnl_nlmsg_batch_next(batch);

        nl = mnl_socket_open(NETLINK_NETFILTER);
        if (nl == NULL) {
                perror("mnl_socket_open");
                exit(EXIT_FAILURE);
        }

        if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
                perror("mnl_socket_bind");
                exit(EXIT_FAILURE);
        }
        portid = mnl_socket_get_portid(nl);

        if (mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
                              mnl_nlmsg_batch_size(batch)) < 0) {
                perror("mnl_socket_send");
                exit(EXIT_FAILURE);
        }

        mnl_nlmsg_batch_stop(batch);

        ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
        while (ret > 0) {
                ret = mnl_cb_run(buf, ret, obj_seq, portid, NULL, NULL);
                if (ret <= 0) {
                        break;
                }
                ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
        }
        if (ret == -1) {
                perror("error");
                exit(EXIT_FAILURE);
        }
        mnl_socket_close(nl);

        return EXIT_SUCCESS;
}
