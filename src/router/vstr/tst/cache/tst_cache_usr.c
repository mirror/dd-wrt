#include "tst-main.c"

static const char *rf = __FILE__;

struct tst_cache_chr
{
 size_t pos;
 size_t len;
 char srch;
 size_t found_at;
};

static unsigned int tst_cache_srch_pos = 0;

static enum {
 START,
 END,
 WHOLE,
 NONE
} tst_in_cache = NONE;

static int called_cache_cb = 0;

static void *tst_cache_cb(const Vstr_base *base __attribute__((unused)),
                           size_t pos, size_t len,
                           unsigned int type, void *passed_data)
{ /* this is a simple version, it could chop the cache when something changes */
  struct tst_cache_chr *data = passed_data;
  const size_t end_pos = vstr_sc_poslast(pos, len);
  const size_t data_end_pos = VSTR_SC_POSLAST(data->pos, data->len);

  --called_cache_cb;

  if (type == VSTR_TYPE_CACHE_FREE)
  {
    free(data);
    return (NULL);
  }

  if (!data->pos)
    return (data);

  if (data_end_pos < pos)
    return (data);

  if (type == VSTR_TYPE_CACHE_ADD)
  {
    if (data->pos > pos)
    {
      data->pos += len;
      if (data->found_at)
        data->found_at += len;
      return (data);
    }

    if (data_end_pos == pos)
      return (data);
  }

  if (type == VSTR_TYPE_CACHE_DEL)
  {
    if (data->pos < pos)
    {
      data->len = pos - data->pos;
      if (data->found_at > pos)
        data->found_at = 0;
      return (data);
    }
    if (data->pos > end_pos)
    {
      data->pos -= len;
      if (data->found_at)
        data->found_at -= len;
      return (data);
    }
    if (data_end_pos > end_pos)
    {
      if (data->found_at &&
          (data->found_at > len) && (data->pos <= (data->found_at - len)))
        data->found_at -= len;
      else
        data->found_at = 0;
      data->pos = pos;
      data->len = data_end_pos - end_pos;

      return (data);
    }
  }

  if (type == VSTR_TYPE_CACHE_SUB)
  {
    if (data->pos < pos)
    {
      data->len = pos - data->pos;
      if (data->found_at > pos)
        data->found_at = 0;
      return (data);
    }
    if (data->pos > end_pos)
      return (data);
    if (data_end_pos > end_pos)
    {
      if (data->found_at &&
          (data->found_at > len) && (data->pos <= (data->found_at - len)))
        /* do nothing */ ;
      else
        data->found_at = 0;
      data->pos = end_pos + 1;
      data->len = data_end_pos - end_pos;
      return (data);
    }
  }

  data->pos = 0;

  return (data);
}

static size_t xvstr_srch_chr_fwd(Vstr_base *base, size_t pos, size_t len,
                                 char srch)
{
  struct tst_cache_chr *data = vstr_cache_get(base, tst_cache_srch_pos);

  if (data)
  {
    const size_t end_pos = (pos + len - 1);

    /* if the cache applies */
    if (data->pos && (data->srch == srch) &&
        (!data->found_at ||
         ((data->found_at >= pos) && (data->found_at <= end_pos))))
    {
      const size_t data_end_pos = (data->pos + data->len - 1);
      size_t found_at = 0;

      /* if it covers everything... */
      if ((data->pos <= pos) && (data_end_pos >= end_pos))
      {
        assert(tst_in_cache == WHOLE);
        return (data->found_at);
      }

      /* if it covers some of the start of our search... */
      if ((data->pos <= pos) && (data_end_pos >= pos))
      {
        size_t uc_len = 0;

        assert(tst_in_cache == START);

        if (data->found_at)
          return (data->found_at);

        uc_len = end_pos - data_end_pos;
        data->len += uc_len;

        data->found_at = vstr_srch_chr_fwd(base, data_end_pos, uc_len, srch);

        return (data->found_at);
      }

      assert(tst_in_cache == END);

      /* it must cover some of the end of our search... */
      assert((data->pos <= end_pos) && (data_end_pos >= end_pos));
      found_at = vstr_srch_chr_fwd(base, pos, data->pos - pos, srch);
      if (found_at)
        data->found_at = found_at;

      data->len += data->pos - pos;
      data->pos = pos;

      return (data->found_at);
    }

    assert(tst_in_cache == NONE);

    data->pos = pos;
    data->len = len;
    data->srch = srch;

    data->found_at = vstr_srch_chr_fwd(base, pos, len, srch);

    return (data->found_at);
  }

  assert(tst_in_cache == NONE);

  if (!(data = malloc(sizeof(struct tst_cache_chr))))
    return (vstr_srch_chr_fwd(base, pos, len, srch));

  data->pos = pos;
  data->len = len;
  data->srch = srch;

  data->found_at = vstr_srch_chr_fwd(base, pos, len, srch);

  if (!vstr_cache_set(base, tst_cache_srch_pos, data))
  {
    size_t found_at = data->found_at;

    free(data);

    return (found_at);
  }

  return (data->found_at);
}

#define CB_NORM_CHECK() do { \
 if (called_cache_cb) return (100 + called_cache_cb); \
 } while (FALSE)

#define CB_SUB_CHECK() do { \
 if (called_cache_cb > 0) return (100 + called_cache_cb); \
 called_cache_cb = 0; \
 } while (FALSE)

int tst(void)
{
  tst_cache_srch_pos = vstr_cache_add(s1->conf, "/tst_usr/srch_fwd",
                                      tst_cache_cb);
  if (!tst_cache_srch_pos)
    die();

  /* make sure it's the same ... */
  if (vstr_cache_srch(s1->conf, "/tst_usr/srch_fwd") != tst_cache_srch_pos)
    return (99);

  VSTR_ADD_CSTR_BUF(s1, 0, "The tester!retset ehT");

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (1);

  tst_in_cache = WHOLE;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (2);

  ++called_cache_cb;
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd-");
  CB_NORM_CHECK();

  tst_in_cache = END;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("abcd-The tester!"))
    return (3);

  ++called_cache_cb;
  VSTR_SUB_CSTR_BUF(s1, 1, strlen("abcd-"), "xyz-");
  CB_SUB_CHECK();

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("xyz-The tester!"))
    return (4);

  ++called_cache_cb;
  vstr_del(s1, 1, 1);
  CB_NORM_CHECK();

  tst_in_cache = WHOLE;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("yz-The tester!"))
    return (5);

  ++called_cache_cb;
  vstr_del(s1, 1, strlen("yz-"));
  CB_NORM_CHECK();

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (6);

  ++called_cache_cb;
  VSTR_ADD_CSTR_BUF(s1, s1->len, "-abcd");
  CB_NORM_CHECK();

  tst_in_cache = START;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (7);

  ++called_cache_cb;
  VSTR_SUB_CSTR_BUF(s1, s1->len - strlen("abcd"), strlen("-abcd"),
                    "-xyz");
  CB_SUB_CHECK();

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (8);

  ++called_cache_cb;
  vstr_del(s1, s1->len - strlen("xyz"), strlen("-xyz"));
  CB_NORM_CHECK();

  tst_in_cache = WHOLE;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (9);

  ++called_cache_cb;
  vstr_del(s1, s1->len - strlen("he tester"), strlen("The tester"));
  CB_NORM_CHECK();

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (10);

  ++called_cache_cb;
  vstr_cache_cb_free(s1, tst_cache_srch_pos);
  CB_NORM_CHECK();

  tst_in_cache = NONE;

  if (xvstr_srch_chr_fwd(s1, 1, s1->len, '!') != strlen("The tester!"))
    return (11);

  if (vstr_cache_get(s1, 0))
    return (12);

  return (0);
}
