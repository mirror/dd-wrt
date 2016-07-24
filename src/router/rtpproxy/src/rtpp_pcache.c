#if defined(RTPP_DEBUG)
#include <assert.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_types.h"
#include "rtpp_hash_table.h"
#include "rtpp_pcache.h"

struct rtpp_pcache_obj_priv {
  struct rtpp_pcache_obj *real;
  struct rtpp_hash_table_obj *hash_table;
};

struct rtpp_pcache_obj_full {
  struct rtpp_pcache_obj pub;
  struct rtpp_pcache_obj_priv pvt;
};

struct rtpp_pcache_fd {
  off_t cpos;
  struct rtpp_hash_table_entry *hte;
};

static void rtpp_pcache_obj_dtor(struct rtpp_pcache_obj *);
static struct rtpp_pcache_fd *rtpp_pcache_obj_open(struct rtpp_pcache_obj *, const char *);
static int rtpp_pcache_obj_read(struct rtpp_pcache_obj *, struct rtpp_pcache_fd *, void *, size_t);
static void rtpp_pcache_obj_close(struct rtpp_pcache_obj *, struct rtpp_pcache_fd *);

struct rtpp_pcache_obj *
rtpp_pcache_ctor(void)
{
    struct rtpp_pcache_obj_full *fp;
    struct rtpp_pcache_obj *pub;
    struct rtpp_pcache_obj_priv *pvt;

    fp = malloc(sizeof(struct rtpp_pcache_obj_full));
    if (fp == NULL) {
        return (NULL);
    }
    memset(fp, '\0', sizeof(struct rtpp_pcache_obj_full));
    pub = &(fp->pub);
    pvt = &(fp->pvt);
    pvt->hash_table = rtpp_hash_table_ctor();
    if (pvt->hash_table == NULL) {
        free(fp);
        return (NULL);
    }
    pub->pvt = pvt;
    pub->open = &rtpp_pcache_obj_open;
    pub->read = &rtpp_pcache_obj_read;
    pub->close = &rtpp_pcache_obj_close;
    pub->dtor = &rtpp_pcache_obj_dtor;
#if defined(RTPP_DEBUG)
    assert((void *)fp == (void *)pub);
#endif
    return (pub);
}

struct rtpp_pcache_fd *
rtpp_pcache_obj_open(struct rtpp_pcache_obj *self, const char *fname)
{
    struct rtpp_pcache_fd *p_fd;
    struct rtpp_pcache_obj_priv *pvt;

    p_fd = malloc(sizeof(struct rtpp_pcache_fd));
    if (p_fd == NULL) {
        return (NULL);
    }
    memset(p_fd, '\0', sizeof(struct rtpp_pcache_fd));
    pvt = self->pvt;
    p_fd->hte = CALL_METHOD(pvt->hash_table, append, fname, p_fd);    
    return (p_fd);
}

static void
rtpp_pcache_obj_close(struct rtpp_pcache_obj *self, struct rtpp_pcache_fd *p_fd)
{

    CALL_METHOD(self->pvt->hash_table, remove_nc, p_fd->hte);
    free(p_fd);
}

static int
rtpp_pcache_obj_read(struct rtpp_pcache_obj *self, struct rtpp_pcache_fd *p_fd, void *buf, size_t len)
{

    p_fd->cpos += len;
    memset(buf, p_fd->cpos, len);
    return(len);
}

static void
rtpp_pcache_obj_dtor(struct rtpp_pcache_obj *self)
{

    free(self);
}
