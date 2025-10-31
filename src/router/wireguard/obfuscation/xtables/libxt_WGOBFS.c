/*
 * iptables WGOBFS target extension
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <xtables.h>
#include "xt_WGOBFS.h"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum {
        FLAGS_KEY    = 1 << 0,
        FLAGS_OBFS   = 1 << 1,
        FLAGS_UNOBFS = 1 << 2,
};

enum {
        OPT_KEY = 0,
        OPT_OBFS,
        OPT_UNOBFS
};

enum {
        MODE_OBFS = 0,
        MODE_UNOBFS
};

static const struct option wg_obfs_opts[] = {
        { .name = "key", .has_arg = true, .val = OPT_KEY },
        { .name = "obfs", .has_arg = false, .val = OPT_OBFS },
        { .name = "unobfs", .has_arg = false, .val = OPT_UNOBFS },
        {},
};

static void wg_obfs_help(void)
{
        printf("WGOBFS target options:\n"
               "    --key <string>\n"
               "    --obfs or --unobfs\n");
}

/* repeat a input string until it reaches @outlen */
static void expand_string(const char *s, int len, char *outbuf, int outlen)
{
        int i, j;

        i = 0;
        while (true) {
                for (j = 0; j < len; j++, i++) {
                        if (i == outlen)
                                return;

                        outbuf[i] = s[j];
                }
        }
}

static int wg_obfs_parse(int c, char **argv, int z1, unsigned int *flags,
                         const void *z2, struct xt_entry_target **tgt)
{
        struct xt_wg_obfs_info *info = (void *)(*tgt)->data;
        unsigned long len;
        const char *s = optarg;
        char chacha_key[XT_CHACHA_KEY_SIZE];

        switch (c) {
        case OPT_KEY:
                len = strlen(s);
                if (len > XT_WGOBFS_MAX_KEY_SIZE)
                        xtables_error(PARAMETER_PROBLEM,
                                      "WGOBFS: Max key size is %d",
                                      XT_WGOBFS_MAX_KEY_SIZE);

                strncpy(info->key, s, XT_WGOBFS_MAX_KEY_SIZE);
                *flags |= FLAGS_KEY;
                expand_string(s, len, chacha_key, XT_CHACHA_KEY_SIZE);
                memcpy(info->chacha_key, chacha_key, XT_CHACHA_KEY_SIZE);
                return true;
        case OPT_OBFS:
                info->mode = XT_MODE_OBFS;
                *flags |= FLAGS_OBFS;
                return true;
        case OPT_UNOBFS:
                info->mode = XT_MODE_UNOBFS;
                *flags |= FLAGS_UNOBFS;
                return true;
        }

        return false;
}

static void wg_obfs_check(unsigned int flags)
{
        if (!(flags & FLAGS_KEY))
                xtables_error(PARAMETER_PROBLEM, "WGOBFS: --key is required.");

        if (!((flags & FLAGS_OBFS) || (flags & FLAGS_UNOBFS)))
                xtables_error(PARAMETER_PROBLEM,
                              "WGOBFS: --obfs or --unobfs is required.");
}

/* invoke by `iptables -L` to show previously inserted rules */
static void wg_obfs_print(const void *z1, const struct xt_entry_target *tgt,
                          int z2)
{
        const struct xt_wg_obfs_info *info = (const void *)tgt->data;
        if (info->mode == XT_MODE_OBFS)
                printf(" --key %s --obfs", info->key);
        else if (info->mode == XT_MODE_UNOBFS)
                printf(" --key %s --unobfs", info->key);
}

/* for iptables-save to dump rules */
static void wg_obfs_save(const void *u, const struct xt_entry_target *tgt)
{
        const struct xt_wg_obfs_info *info = (const void *)tgt->data;
        if (info->mode == XT_MODE_OBFS)
                printf(" --key %s --obfs", info->key);
        else if (info->mode == XT_MODE_UNOBFS)
                printf(" --key %s --unobfs", info->key);
}

static struct xtables_target wg_obfs_reg[] = {
        {
                .version        = XTABLES_VERSION,
                .name           = "WGOBFS",
                .revision       = 0,
                .family         = NFPROTO_IPV4,
                .size           = XT_ALIGN(sizeof(struct xt_wg_obfs_info)),
                .userspacesize  = XT_ALIGN(sizeof(struct xt_wg_obfs_info)),
                .help           = wg_obfs_help,
                .parse          = wg_obfs_parse,
                .final_check    = wg_obfs_check,
                .print          = wg_obfs_print,
                .save           = wg_obfs_save,
                .extra_opts     = wg_obfs_opts,
        },
        {
                .version        = XTABLES_VERSION,
                .name           = "WGOBFS",
                .revision       = 0,
                .family         = NFPROTO_IPV6,
                .size           = XT_ALIGN(sizeof(struct xt_wg_obfs_info)),
                .userspacesize  = XT_ALIGN(sizeof(struct xt_wg_obfs_info)),
                .help           = wg_obfs_help,
                .parse          = wg_obfs_parse,
                .final_check    = wg_obfs_check,
                .print          = wg_obfs_print,
                .save           = wg_obfs_save,
                .extra_opts     = wg_obfs_opts,
        },
};

static __attribute__((constructor)) void wg_obfs_ldr(void)
{
        xtables_register_targets(wg_obfs_reg, ARRAY_SIZE(wg_obfs_reg));
}
