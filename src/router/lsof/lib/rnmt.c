/*
 * Copyright (C) 2022  Tobias Nygren <tnn@NetBSD.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "common.h"
#include "machine.h"

#if defined(HASNCACHE) && defined(USE_LIB_RNMT)
#    include <sys/rbtree.h>
#    include <sys/vnode_impl.h>
#    include <err.h>

/*
 * rnmt.c - read NetBSD=>10-style red-black tree kernel name cache
 */

static int lnc_compare_nodes(void *, const void *, const void *);
static int lnc_compare_key(void *, const void *, const void *);

static rb_tree_t lnc_rbtree;

/* local name cache entry */
struct lnc {
    struct rb_node lnc_tree;      /* red-black tree */
    KA_T lnc_vp;                  /* vnode address */
    const struct lnc *lnc_plnc;   /* parent lnc address */
    int lnc_nlen;                 /* name length */
    char lnc_name[NCHNAMLEN + 1]; /* name */
};

static const rb_tree_ops_t lnc_rbtree_ops = {
    .rbto_compare_nodes = lnc_compare_nodes,
    .rbto_compare_key = lnc_compare_key,
    .rbto_node_offset = offsetof(struct lnc, lnc_tree),
    .rbto_context = NULL};

static int lnc_compare_nodes(void *context, const void *node1,
                             const void *node2) {
    const struct lnc *lnc1 = node1;
    const struct lnc *lnc2 = node2;

    if (lnc1->lnc_vp < lnc2->lnc_vp) {
        return -1;
    }
    if (lnc1->lnc_vp > lnc2->lnc_vp) {
        return 1;
    }

    return 0;
}

static int lnc_compare_key(void *context, const void *node, const void *key) {
    const struct lnc *lnc = node;
    const KA_T vp = (KA_T)key;

    if (lnc->lnc_vp < vp) {
        return -1;
    }
    if (lnc->lnc_vp > vp) {
        return 1;
    }

    return 0;
}

static struct lnc *ncache_enter_local(KA_T vp, const struct lnc *plnc,
                                      const struct namecache *nc) {
    struct lnc *lnc;

    lnc = malloc(sizeof(*lnc));
    if (!lnc) {
        errx(1, "can't allocate local name cache entry\n");
    }
    lnc->lnc_vp = vp;
    lnc->lnc_plnc = plnc;
    lnc->lnc_nlen = nc->nc_nlen;
    memcpy(lnc->lnc_name, nc->nc_name, lnc->lnc_nlen);
    lnc->lnc_name[lnc->lnc_nlen] = 0;

    rb_tree_insert_node(&lnc_rbtree, lnc);

    return lnc;
}

static int sanity_check_vnode_impl(const struct vnode_impl *vi) {
    if (vi->vi_vnode.v_type >= VBAD)
        return -1;

    return 0;
}

static int sanity_check_namecache(const struct namecache *nc) {
    if (nc->nc_vp == NULL)
        return -1;

    if (nc->nc_nlen > NCHNAMLEN)
        return -1;

    if (nc->nc_nlen == 1 && nc->nc_name[0] == '.')
        return -1;

    if (nc->nc_nlen == 2 && nc->nc_name[0] == '.' && nc->nc_name[1] == '.')
        return -1;

    return 0;
}

static void ncache_walk(struct lsof_context *ctx, KA_T ncp,
                        const struct lnc *plnc) {
    struct l_nch *lc;
    static struct vnode_impl vi;
    static struct namecache nc;
    struct lnc *lnc;
    KA_T vp;
    KA_T left, right;

    if (kread(ctx, ncp, (char *)&nc, sizeof(nc))) {
        return;
    }
    vp = (KA_T)nc.nc_vp;
    if (kread(ctx, vp, (char *)&vi, sizeof(vi))) {
        vi.vi_vnode.v_type = VBAD;
    }
    left = (KA_T)nc.nc_tree.rb_nodes[0];
    right = (KA_T)nc.nc_tree.rb_nodes[1];
    if (sanity_check_vnode_impl(&vi) == 0 && sanity_check_namecache(&nc) == 0) {
        lnc = ncache_enter_local(vp, plnc, &nc);
        if (vi.vi_vnode.v_type == VDIR && vi.vi_nc_tree.rbt_root != NULL) {
            ncache_walk((KA_T)vi.vi_nc_tree.rbt_root, lnc);
        }
    }
    if (left)
        ncache_walk(left, plnc);
    if (right)
        ncache_walk(right, plnc);
}

void ncache_load(struct lsof_context *ctx) {
    KA_T rootvnode_addr;
    struct vnode_impl vi;

    rootvnode_addr = (KA_T)0;
    if (get_Nl_value("rootvnode", (struct drive_Nl *)NULL, &rootvnode_addr) <
            0 ||
        !rootvnode_addr ||
        kread(ctx, (KA_T)rootvnode_addr, (char *)&rootvnode_addr,
              sizeof(rootvnode_addr)) ||
        kread(ctx, (KA_T)rootvnode_addr, (char *)&vi, sizeof(vi))) {
        errx(1, "can't read rootvnode\n");
    }

    rb_tree_init(&lnc_rbtree, &lnc_rbtree_ops);
    ncache_walk((KA_T)vi.vi_nc_tree.rbt_root, 0);
}

static void build_path(char **buf, size_t *remaining, const struct lnc *lnc) {
    size_t len;

    if (lnc == NULL)
        return;

    build_path(buf, remaining, lnc->lnc_plnc);
    if (remaining == 0) {
        return;
    }
    if (lnc->lnc_plnc != NULL) {
        **buf = '/';
        (*buf)++;
        remaining--;
    }
    len = lnc->lnc_nlen;
    if (*remaining < len)
        len = *remaining;
    memcpy(*buf, lnc->lnc_name, len);
    *remaining -= len;
    *buf += len;
}

char *ncache_lookup(char *buf, int blen, int *fp) {
    const struct lnc *lnc;
    char *p;
    size_t remaining;

    *fp = 0;
    lnc = rb_tree_find_node(&lnc_rbtree, (void *)Lf->na);
    if (lnc != NULL) {
        p = buf;
        remaining = blen;
        build_path(&p, &remaining, lnc);
        if (remaining == 0) {
            buf[blen - 1] = 0;
        } else {
            *p = 0;
        }
        *fp = 1;
        return buf;
    }

    return NULL;
}
#endif
