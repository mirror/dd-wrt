
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct FPST {
    struct FPST *children;
    const char  *key;
    uint16_t     idx;
    uint16_t     bitmap;
    uint32_t     val;
} FPST;

#define FPST_DEFINED 1
#include "fpst.h"

#ifdef __GNUC__
# define popcount(X) ((unsigned int) __builtin_popcount(X))
# define prefetch(X)  __builtin_prefetch(X)
#else
# define prefetch(X) (void)(X)

static inline unsigned int
popcount(uint32_t w)
{
    w -= (w >> 1) & 0x55555555;
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w = (w + (w >> 4)) & 0x0f0f0f0f;
    w = (w * 0x01010101) >> 24;
    return w;
}
#endif

static inline unsigned char
fpst_quadbit_at(const char *str, size_t i)
{
    unsigned char c;

    c = (unsigned char) str[i / 2];
    if ((i & 1U) == 0U) {
        c >>= 4;
    }
    return c & 0xf;
}

static inline int
fpst_bitmap_is_set(const FPST *t, size_t bit)
{
    return (t->bitmap & (((uint16_t) 1U) << bit)) != 0U;
}

static inline void
fpst_bitmap_set(FPST *t, size_t bit)
{
    t->bitmap |= (((uint16_t) 1U) << bit);
}

static inline size_t
fpst_actual_index(const FPST *t, size_t i)
{
    const uint16_t b = t->bitmap & ((((uint16_t) 1U) << i) - 1U);

    return (size_t) popcount((uint32_t) b);
}

static inline FPST *
fpst_child_get(FPST *t, size_t i)
{
    if (!fpst_bitmap_is_set(t, i)) {
        return NULL;
    }
    return &t->children[fpst_actual_index(t, i)];
}

static int
fpst_child_set(FPST *t, FPST *v, size_t i)
{
    FPST     *previous;
    FPST     *tmp;
    size_t    ri;
    size_t    rcount;
    size_t    count;

    if ((previous = fpst_child_get(t, i)) != NULL) {
        *previous = *v;
        return 0;
    }
    count = (size_t) popcount(t->bitmap) + 1U;
    if ((tmp = (FPST *) realloc(t->children,
                                count * (sizeof *t->children))) == NULL) {
        return -1;
    }
    t->children = tmp;
    ri = fpst_actual_index(t, i);
    if ((rcount = count - ri - 1U) > 0U) {
        memmove(&t->children[ri + 1U], &t->children[ri],
                rcount * (sizeof *t->children));
    }
    t->children[ri] = *v;
    fpst_bitmap_set(t, i);

    return 0;
}

FPST *
fpst_new(void)
{
    return NULL;
}

FPST *
fpst_insert(FPST *trie, const char *key, size_t len, uint32_t val)
{
    FPST         *new_node_p;
    FPST         *t;
    const char   *lk;
    FPST          new_node, saved_node;
    size_t        i;
    size_t        j;
    unsigned char c;
    unsigned char x;

    if (len >= 0x7fff) {
        return NULL;
    }
    if (trie == NULL) {
        if ((new_node_p = (FPST *) malloc(sizeof *new_node_p)) == NULL) {
            return NULL;
        }
        new_node_p->key = key;
        new_node_p->val = val;
        new_node_p->idx = 0U;
        new_node_p->bitmap = 0U;
        new_node_p->children = NULL;

        return new_node_p;
    }
    t = trie;
    j = 0U;
    for (;;) {
        lk = t->key;
        x = 0U;
        for (; j <= len; j++) {
            x = ((unsigned char) lk[j]) ^ ((unsigned char) key[j]);
            if (x != 0U) {
                break;
            }
        }
        if (j > len && lk[j - 1] == 0) {
            assert(key[j - 1] == 0);
            t->val = val;
            return trie;
        }
        i = j * 2;
        if ((x & 0xf0) == 0U) {
            i++;
        }
        if (t->bitmap == 0U) {
            /* keep index from the new key */
        } else if (i >= t->idx) {
            i = t->idx;
            j = i / 2;
        } else {
            saved_node = *t;
            t->key = key;
            t->val = val;
            t->idx = (uint16_t) i;
            t->bitmap = 0U;
            t->children = NULL;
            c = fpst_quadbit_at(lk, i);
            if (fpst_child_set(t, &saved_node, (size_t) c) != 0) {
                *t = saved_node;
                return NULL;
            }
            return trie;
        }
        prefetch(t->children);
        c = fpst_quadbit_at(key, i);
        if (!fpst_bitmap_is_set(t, c)) {
            break;
        }
        t = fpst_child_get(t, (size_t) c);
    }
    t->idx = (uint16_t) i;
    assert(!fpst_bitmap_is_set(t, c));
    new_node.key = key;
    new_node.val = val;
    new_node.idx = 0U;
    new_node.bitmap = 0U;
    new_node.children = NULL;
    if (fpst_child_set(t, &new_node, (size_t) c) != 0) {
        return NULL;
    }
    return trie;
}

FPST *
fpst_insert_str(FPST *trie, const char *key, uint32_t val)
{
    return fpst_insert(trie, key, strlen(key), val);
}

int
fpst_starts_with_existing_key(FPST *trie,
                              const char *str, size_t len,
                              const char **found_key_p,
                              uint32_t *found_val_p)
{
    FPST          *t;
    const char    *lk;
    size_t         i;
    size_t         j;
    unsigned char  c;
    int            ret = 0;

    if (trie == NULL) {
        return 0;
    }
    t = trie;
    j = 0U;
    for (;;) {
        lk = t->key;
        for (; j <= len; j++) {
            if (lk[j] != str[j]) {
                if (lk[j] == 0) {
                    *found_key_p = t->key;
                    *found_val_p = t->val;
                    ret = 1;
                }
                break;
            }
        }
        if (j > len) {
            *found_key_p = t->key;
            *found_val_p = t->val;
            ret = 1;
            break;
        }
        if (t->bitmap == 0U) {
            break;
        }
        i = t->idx;
        if (i > len * 2) {
            break;
        }
        if (j > i / 2) {
            j = i / 2;
        }
        prefetch(t->children);
        c = fpst_quadbit_at(str, i);
        if (!fpst_bitmap_is_set(t, c)) {
            if (fpst_bitmap_is_set(t, 0U)) {
                c = 0U;
            } else {
                break;
            }
        }
        t = fpst_child_get(t, (size_t) c);
    }
    return ret;
}

int
fpst_str_starts_with_existing_key(FPST *trie, const char *str,
                                  const char **found_key_p,
                                  uint32_t *found_val_p)
{
    return fpst_starts_with_existing_key(trie, str, strlen(str),
                                         found_key_p, found_val_p);
}

static void
fpst_free_node(FPST *t, FPST_FreeFn free_kv_fn)
{
    size_t count;
    size_t i;

    if (t->bitmap == 0U) {
        assert(t->children == NULL);
    }
    count = (size_t) popcount(t->bitmap);
    for (i = 0; i < count; i++) {
        fpst_free_node(&t->children[i], free_kv_fn);
    }
    free(t->children);
    t->bitmap = 0U;
    t->children = NULL;
    free_kv_fn(t->key, t->val);
    t->key = NULL;
}

int
fpst_has_key(FPST *trie, const char *key, size_t len, uint32_t *found_val_p)
{
    const char *found_key;
    int         ret;

    ret = fpst_starts_with_existing_key(trie, key, len,
                                        &found_key, found_val_p);
    if (ret > 0 && strlen(found_key) != len) {
        ret = 0;
    }
    return ret;
}

int
fpst_has_key_str(FPST *trie, const char *key, uint32_t *found_val_p)
{
    return fpst_has_key(trie, key, strlen(key), found_val_p);
}

void
fpst_free(FPST *trie, FPST_FreeFn free_kv_fn)
{
    if (trie == NULL) {
        return;
    }
    fpst_free_node(trie, free_kv_fn);
    free(trie);
}
