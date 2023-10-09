/*
 * xt_WGOBFS kernel module
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/udp.h>
#include <net/ip.h>
#include "xt_WGOBFS.h"
#include "wg.h"
#include "chacha.h"

#define WG_HANDSHAKE_INIT       0x01
#define WG_HANDSHAKE_RESP       0x02
#define WG_COOKIE               0x03
#define WG_DATA                 0x04
#define OBFS_WG_HANDSHAKE_INIT  0x11
#define OBFS_WG_HANDSHAKE_RESP  0x12
#define WG_MIN_LEN              32
#define MIN_RND_LEN             4

enum chacha_output_lengths {
	MAX_RND_LEN = 32,
	MAX_RND_WORDS = MAX_RND_LEN / sizeof(u32),
	WG_COOKIE_WORDS = WG_COOKIE_LEN / sizeof(u32),
	ONE_WORD = 1,
	HEAD_OBFS_WORDS = 16 / sizeof(u32) + 1
};

struct obfs_buf {
        u8 chacha_in[CHACHA_INPUT_SIZE];
        u8 chacha_out[MAX_RND_LEN];
        u8 rnd[MAX_RND_LEN];
        u8 rnd_len;
};

/* get a pseudo-random string by hashing part of wg message */
static u8 get_prn_insert(u8 *buf, struct obfs_buf *ob, const u8 *k,
                         const u8 min_len, const u8 max_len)
{
        u8 r, i;
        u8 *counter = ob->chacha_in;
 
        r = 0;
        while (1) {
                (*counter)++;
                chacha_hash(ob->chacha_in, k, ob->rnd, MAX_RND_WORDS);
                for (i = 0; i < MAX_RND_LEN; i++) {
                        if (ob->rnd[i] >= min_len && ob->rnd[i] <= max_len) {
                                r = ob->rnd[i];
                                break;
                        }
                }

                if (r > 0)
                        break;
        }

        ob->rnd_len = r;
        return r;
}

/* Replace the all zeros mac2 with random bytes, then change the type field to
 * 0x11 or 0x12
 */
static void obfs_mac2(u8 *buf, const int data_len, struct obfs_buf *ob,
                      const u8 *k)
{
        u8 type;
        struct wg_message_handshake_initiation *hsi;
        struct wg_message_handshake_response *hsr;
        u32 *np;
        u8 *counter = ob->chacha_in;

        type = buf[0];
        if (type == WG_HANDSHAKE_INIT && data_len == 148) {
                hsi = (struct wg_message_handshake_initiation *) buf;
                /* highly unlikely the first 4 bytes of cookie are all zeros */
                np = (u32 *) hsi->macs.mac2;
                if (*np)
                        return;

                /* Write 128bits PRN to mac2 */
                (*counter)++;
                chacha_hash(ob->chacha_in, k, hsi->macs.mac2, WG_COOKIE_WORDS);

                /* mark the packet as need restore mac2 upon receiving */
                buf[0] |= 0x10;

        } else if (type == WG_HANDSHAKE_RESP && data_len == 92) {
                hsr = (struct wg_message_handshake_response *) buf;
                np = (u32 *) hsr->macs.mac2;
                if (*np)
                        return;

                (*counter)++;
                chacha_hash(ob->chacha_in, k, hsr->macs.mac2, WG_COOKIE_WORDS);
                buf[0] |= 0x10;
        }
}

static int random_drop_wg_keepalive(u8 *buf, const int len, const u8 *key)
{
        u8 type = *buf;
        u8 *counter = ob->chacha_in;

        if (type != WG_DATA || len != 32)
                return 0;

        /* assume the probability of a 1 byte PRN > 50 is 0.8 */
        (*counter)++;
        chacha_hash(ob->chacha_in, key, ob->chacha_out, ONE_WORD);

        if (ob->chacha_out[0] > 50)
                return 1;
        else
                return 0;
}

/* The WG packet is obfuscated by:
 *
 *   - Replace the all zeros mac2 field with pseudo-random bytes.
 *
 *   - Obfs the first 16 bytes of WG message.
 *
 *   - Change the length of WG message by padding a variable length random
 *     string at the end, such that:
 *
 *     Orig_WG_message B1 B2 ... Bn
 *     Bn stores length of the padding.
 *
 */
static void obfs_wg(u8 *buf, const int len, struct obfs_buf *ob, const u8 *key)
{
        u8 *b;
        u8 rnd_len;
        int i;

        obfs_mac2(buf, len, ob, key);
        rnd_len = ob->rnd_len;
        memcpy(buf + len, ob->rnd, rnd_len);

        /* Use PRN to XOR with the first 16 bytes of WG message. It has message
         * type, reserved field and counter. They look distinct.
         */
        chacha_hash(buf + 16, key, ob->chacha_out, HEAD_OBFS_WORDS);

        /* set the last byte of random as its length */
        buf[len + rnd_len - 1] = rnd_len ^ ob->chacha_out[16];
        b = buf;
        for (i = 0; i < 16; i++, b++)
                *b ^= ob->chacha_out[i];
}

/* make a skb writable, and if necessary, expand it */
static int prepare_skb_for_insert(struct sk_buff *skb, int ntail)
{
        int extra_len;

        extra_len = ntail - skb_tailroom(skb);
        if (extra_len > 0) {
                if (pskb_expand_head(skb, 0, extra_len, GFP_ATOMIC))
                        return -1;
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
        if (unlikely(skb_ensure_writable(skb, skb->len)))
#else
        if (unlikely(!skb_make_writable(skb, skb->len)))
#endif
                return -1;

        skb_put(skb, ntail);
        return 0;
}

static unsigned int xt_obfs(struct sk_buff *skb,
                            const struct xt_wg_obfs_info *info)
{
        struct obfs_buf ob;
        struct iphdr *iph;
        struct udphdr *udph;
        int wg_data_len, max_rnd_len;
        u8 rnd_len;
        u8 *buf_udp;

        udph = udp_hdr(skb);
        buf_udp = (u8 *) udph + sizeof(struct udphdr);
        wg_data_len = ntohs(udph->len) - sizeof(struct udphdr);

        /* Use 16th to 31st bytes of WG message as input of chacha.
         *
         * The 16th to 31st bytes is:
         *  - handshake initiation unencrypted_ephemeral (32 bytes starts at 8)
         *  - handshake response unencrypted_ephemeral (32 bytes starts at 12)
         *  - cookie nonce (24 bytes starts at 8)
         *  - data encrypted packet (var length starts at 16)
         *  - keepalive random poly1305 tag (16 bytes starts at 16)
         *
         *  Increment the first byte as counter to generate different PRN
         */
        memcpy(&(ob.chacha_in), buf_udp + 16, CHACHA_INPUT_SIZE);

        /* Later will use the unchange 16th to 31st bytes to gernerate a PRN,
         * which XOR with first 16 bytes of WG. Peer will need generate an
         * identical PRN to recover the original WG.
         * Other PRNs will be generated with incremented counter.
         */
        ob.chacha_in[0] += 42;

        if (random_drop_wg_keepalive(buf_udp, wg_data_len, &ob, info->chacha_key))
                return NF_DROP;

        /* Insert a long pseudo-random string if the WG packet is small, or a
         * short string if WG packet is big.
         */
        max_rnd_len = (wg_data_len > 200) ? 8 : MAX_RND_LEN;
        rnd_len = get_prn_insert(buf_udp, &ob, info->chacha_key,
                                 MIN_RND_LEN, max_rnd_len);
        ob.rnd_len = rnd_len;
        if (prepare_skb_for_insert(skb, rnd_len))
                return NF_DROP;

        udph = udp_hdr(skb);
        buf_udp = (u8 *) udph + sizeof(struct udphdr);
        obfs_wg(buf_udp, wg_data_len, &ob, info->chacha_key);

        /* packet with DiffServ 0x88 looks distinct? */
        iph = ip_hdr(skb);
        iph->tos = 0;

        /* recalculate ip header checksum */
        iph->tot_len = htons(ntohs(iph->tot_len) + rnd_len);
        iph->check = 0;
        ip_send_check(iph);

        /* CHECKSUM_PARTIAL: The driver is required to checksum the packet.
         * With CHECKSUM_PARTIAL, the udp packet has good checksum in VM, bad
         * checksum after leave VM. Set to CHECKSUM_NONE fixes the problem.
         */
        if (skb->ip_summed == CHECKSUM_PARTIAL)
                skb->ip_summed = CHECKSUM_NONE;

        /* recalculate udp header checksum */
        udph->len = htons(ntohs(udph->len) + rnd_len);
        udph->check = 0;
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
                                        ntohs(udph->len), IPPROTO_UDP,
                                        csum_partial((char *) udph,
                                                     ntohs(udph->len), 0));
        return XT_CONTINUE;
}

static void restore_mac2(u8 *buf)
{
        struct wg_message_handshake_initiation *hsi;
        struct wg_message_handshake_response *hsr;
        static u8 zero_mac2[WG_COOKIE_LEN];

        /* mac2 was all zeros before obfscation, reset it back to zeros */
        switch (buf[0]) {
        case OBFS_WG_HANDSHAKE_INIT:
                hsi = (struct wg_message_handshake_initiation *) buf;
                /* memcpy is faster than memset, 860 vs 847 Mbits/s */
                memcpy(hsi->macs.mac2, zero_mac2, WG_COOKIE_LEN);
                break;
        case OBFS_WG_HANDSHAKE_RESP:
                hsr = (struct wg_message_handshake_response *) buf;
                memcpy(hsr->macs.mac2, zero_mac2, WG_COOKIE_LEN);
                break;
        }

        buf[0] &= 0x0F;
}

static int restore_wg(u8 *buf, int len, const u8 *key)
{
        u8 buf_prn[MAX_RND_LEN];
        u8 *head;
        int i, rnd_len;

        /* Same as obfuscate, generate the same PRN from 16th to 31st bytes of
         * WG message. Need it for restoring the first 16 bytes of WG message.
         */
        chacha_hash(buf + 16, key, buf_prn, HEAD_OBFS_WORDS);

        /* Restore the length of random padding. It is stored in the last byte
         * of obfuscated WG.
         */
        buf[len - 1] ^= buf_prn[16];

        rnd_len = (int) buf[len - 1];
        if (rnd_len + WG_MIN_LEN > len)
                return -1;

        /* restore the first 16 bytes of WG packet */
        head = buf;
        for (i = 0; i < 16; i++, head++)
                *head ^= buf_prn[i];

        restore_mac2(buf);
        return rnd_len;
}

static unsigned int xt_unobfs(struct sk_buff *skb,
                              const struct xt_wg_obfs_info *info)
{
        struct iphdr *iph;
        struct udphdr *udph;
        u8 *buf_udp;
        int data_len;
        int rnd_len;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0)
        if (unlikely(skb_ensure_writable(skb, skb->len)))
#else
        if (unlikely(!skb_make_writable(skb, skb->len)))
#endif
                return NF_DROP;

        udph = udp_hdr(skb);
        buf_udp = (u8 *) udph + sizeof(struct udphdr);
        data_len = ntohs(udph->len) - sizeof(struct udphdr);
        /* random bytes insertion adds at least 4 bytes */
        if (data_len < MIN_RND_LEN)
                return NF_DROP;

        rnd_len = restore_wg(buf_udp, data_len, info->chacha_key);
        if (rnd_len < 0)
                return NF_DROP;

        skb->len -= rnd_len;
        skb->tail -= rnd_len;

        /* recalculate ip header checksum */
        iph = ip_hdr(skb);
        iph->tot_len = htons(ntohs(iph->tot_len) - rnd_len);
        iph->check = 0;
        ip_send_check(iph);

        /* recalculate udp header checksum */
        udph->len = htons(ntohs(udph->len) - rnd_len);
        udph->check = 0;
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
                                        ntohs(udph->len), IPPROTO_UDP,
                                        csum_partial((char *) udph,
                                                     ntohs(udph->len), 0));
        return XT_CONTINUE;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
static unsigned int
xt_wg_obfs_target(struct sk_buff *skb, const struct xt_action_param *par)
#else
static unsigned int
xt_wg_obfs_target(struct sk_buff *skb, const struct xt_target_param *par)
#endif
{
        const struct xt_wg_obfs_info *info = par->targinfo;
        struct iphdr *iph;

        iph = ip_hdr(skb);
        /* only work with UDP so far, may obfuscate UDP into TCP later */
        if (iph->protocol != IPPROTO_UDP)
                return XT_CONTINUE;

        if (info->mode == XT_MODE_OBFS)
                return xt_obfs(skb, info);
        else if (info->mode == XT_MODE_UNOBFS)
                return xt_unobfs(skb, info);

        return XT_CONTINUE;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
static int xt_wg_obfs_checkentry(const struct xt_tgchk_param *par)
{
        if (strcmp(par->table, "mangle")) {
                printk(KERN_WARNING
                       "WGOBFS: can only be called from mangle table\n");
                return -EINVAL;
        }

        return 0;
}
#else
static bool xt_wg_obfs_checkentry(const struct xt_tgchk_param *par)
{
        if (strcmp(par->table, "mangle")) {
                printk(KERN_WARNING
                       "WGOBFS: can only be called from mangle table\n");
                return false;
        }

        return true;
}
#endif

static struct xt_target xt_wg_obfs = {
        .name = "WGOBFS",
        .revision = 0,
        .family = NFPROTO_IPV4,
        .table = "mangle",
        .target = xt_wg_obfs_target,
        .targetsize = XT_ALIGN(sizeof(struct xt_wg_obfs_info)),
        .checkentry = xt_wg_obfs_checkentry,
        .me = THIS_MODULE,
};

static int __init wg_obfs_target_init(void)
{
        return xt_register_target(&xt_wg_obfs);
}

static void __exit wg_obfs_target_exit(void)
{
        xt_unregister_target(&xt_wg_obfs);
}

module_init(wg_obfs_target_init);
module_exit(wg_obfs_target_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Iptables obfuscation module for WireGuard");
MODULE_AUTHOR("Wei Chen <weichen302@gmail.com>");
MODULE_VERSION("0.5");
MODULE_ALIAS("xt_WGOBFS");
