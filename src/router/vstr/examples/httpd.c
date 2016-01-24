/*
 *  Copyright (C) 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* main HTTPD APIs, only really implements server portions */

#define EX_UTILS_NO_USE_INIT  1
#define EX_UTILS_NO_USE_EXIT  1
#define EX_UTILS_NO_USE_LIMIT 1
#define EX_UTILS_NO_USE_BLOCK 1
#define EX_UTILS_NO_USE_GET   1
#define EX_UTILS_NO_USE_PUT   1
#define EX_UTILS_USE_NONBLOCKING_OPEN 1
#define EX_UTILS_RET_FAIL     1
#include "ex_utils.h"

#include "mk.h"

#include "vlg.h"

#define HTTPD_HAVE_GLOBAL_OPTS 1
#include "httpd.h"
#include "httpd_policy.h"

#ifndef POSIX_FADV_SEQUENTIAL
# define posix_fadvise64(x1, x2, x3, x4) (errno = ENOSYS, -1)
#endif

#ifdef VSTR_AUTOCONF_NDEBUG
# define HTTP_CONF_MMAP_LIMIT_MIN (16 * 1024) /* a couple of pages */
# define HTTP_CONF_SAFE_PRINT_REQ TRUE
#else
# define HTTP_CONF_MMAP_LIMIT_MIN 8 /* debug... */
# define HTTP_CONF_SAFE_PRINT_REQ FALSE
#endif
#define HTTP_CONF_MMAP_LIMIT_MAX (50 * 1024 * 1024)

#define CLEN VSTR__AT_COMPILE_STRLEN

/* is the cstr a prefix of the vstr */
#define VPREFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))
/* is the cstr a suffix of the vstr */
#define VSUFFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_eod_buf_eq(vstr, p, l, cstr, CLEN(cstr)))

/* is the cstr a prefix of the vstr, no case */
#define VIPREFIX(vstr, p, l, cstr)                                      \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_case_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))

/* for simplicity */
#define VEQ(vstr, p, l, cstr)  vstr_cmp_cstr_eq(vstr, p, l, cstr)
#define VIEQ(vstr, p, l, cstr) vstr_cmp_case_cstr_eq(vstr, p, l, cstr)

#ifndef SWAP_TYPE
#define SWAP_TYPE(x, y, type) do {              \
      type internal_local_tmp = (x);            \
      (x) = (y);                                \
      (y) = internal_local_tmp;                 \
    } while (FALSE)
#endif

#define HTTP__HDR_SET(req, h, p, l) do {               \
      (req)-> http_hdrs -> hdr_ ## h ->pos = (p);          \
      (req)-> http_hdrs -> hdr_ ## h ->len = (l);          \
    } while (FALSE)
#define HTTP__HDR_MULTI_SET(req, h, p, l) do {         \
      (req)-> http_hdrs -> multi -> hdr_ ## h ->pos = (p); \
      (req)-> http_hdrs -> multi -> hdr_ ## h ->len = (l); \
    } while (FALSE)

#define HTTP__XTRA_HDR_INIT(x) do {             \
      req-> x ## _vs1 = NULL;                   \
      req-> x ## _pos = 0;                      \
      req-> x ## _len = 0;                      \
    } while (FALSE)

#define HTTP__XTRA_HDR_PARAMS(req, x)                            \
    (req)-> x ## _vs1, (req)-> x ## _pos, (req)-> x ## _len


HTTPD_CONF_MAIN_DECL_OPTS(httpd_opts);

static Vlg *vlg = NULL;


void httpd_init(Vlg *passed_vlg)
{
  ASSERT(passed_vlg && !vlg);
  vlg = passed_vlg;
}

void httpd_exit(void)
{
  ASSERT(vlg);
  vlg = NULL;
}

static void http__clear_hdrs(struct Httpd_req_data *req)
{
  Vstr_base *tmp = req->http_hdrs->multi->combiner_store;

  ASSERT(tmp);
  
  HTTP__HDR_SET(req, ua,                  0, 0);
  HTTP__HDR_SET(req, referer,             0, 0);

  HTTP__HDR_SET(req, expect,              0, 0);
  HTTP__HDR_SET(req, host,                0, 0);
  HTTP__HDR_SET(req, if_modified_since,   0, 0);
  HTTP__HDR_SET(req, if_range,            0, 0);
  HTTP__HDR_SET(req, if_unmodified_since, 0, 0);
  HTTP__HDR_SET(req, authorization,       0, 0);

  vstr_del(tmp, 1, tmp->len);
  HTTP__HDR_MULTI_SET(req, accept,          0, 0);
  HTTP__HDR_MULTI_SET(req, accept_charset,  0, 0);
  HTTP__HDR_MULTI_SET(req, accept_encoding, 0, 0);
  HTTP__HDR_MULTI_SET(req, accept_language, 0, 0);
  HTTP__HDR_MULTI_SET(req, connection,      0, 0);
  HTTP__HDR_MULTI_SET(req, if_match,        0, 0);
  HTTP__HDR_MULTI_SET(req, if_none_match,   0, 0);
  HTTP__HDR_MULTI_SET(req, range,           0, 0);
}

static void http__clear_xtra(struct Httpd_req_data *req)
{
  if (req->xtra_content)
    vstr_del(req->xtra_content, 1, req->xtra_content->len);

  HTTP__XTRA_HDR_INIT(content_type);
  HTTP__XTRA_HDR_INIT(content_disposition);
  HTTP__XTRA_HDR_INIT(content_language);
  HTTP__XTRA_HDR_INIT(content_location);
  HTTP__XTRA_HDR_INIT(content_md5);
  HTTP__XTRA_HDR_INIT(gzip_content_md5);
  HTTP__XTRA_HDR_INIT(bzip2_content_md5);
  HTTP__XTRA_HDR_INIT(cache_control);
  HTTP__XTRA_HDR_INIT(etag);
  HTTP__XTRA_HDR_INIT(gzip_etag);
  HTTP__XTRA_HDR_INIT(bzip2_etag);
  HTTP__XTRA_HDR_INIT(expires);
  HTTP__XTRA_HDR_INIT(link);
  HTTP__XTRA_HDR_INIT(p3p);
  HTTP__XTRA_HDR_INIT(ext_vary_a);
  HTTP__XTRA_HDR_INIT(ext_vary_ac);
  HTTP__XTRA_HDR_INIT(ext_vary_al);
}

Httpd_req_data *http_req_make(struct Con *con)
{
  static Httpd_req_data real_req[1];
  Httpd_req_data *req = real_req;
  const Httpd_policy_opts *policy = NULL;
  
  ASSERT(!req->using_req);

  if (!req->done_once)
  {
    Vstr_conf *conf = NULL;

    if (con)
      conf = con->evnt->io_w->conf;
      
    if (!(req->fname = vstr_make_base(conf)) ||
        !(req->http_hdrs->multi->combiner_store = vstr_make_base(conf)) ||
        !(req->sects = vstr_sects_make(8)))
      return (NULL);
    
    req->f_mmap       = NULL;
    req->xtra_content = NULL;
  }

  http__clear_hdrs(req);
  
  http__clear_xtra(req);
  
  req->http_hdrs->multi->comb = con ? con->evnt->io_r : NULL;

  vstr_del(req->fname, 1, req->fname->len);

  req->now = evnt_sc_time();
  
  req->len = 0;

  req->path_pos = 0;
  req->path_len = 0;

  req->error_code = 0;
  req->error_line = "";
  req->error_msg  = "";
  req->error_len  = 0;

  req->sects->num = 0;
  /* f_stat */
  if (con)
    req->orig_io_w_len = con->evnt->io_w->len;

  /* NOTE: These should probably be allocated at init time, depending on the
   * option flags given */
  ASSERT(!req->f_mmap || !req->f_mmap->len);
  if (req->f_mmap)
    vstr_del(req->f_mmap, 1, req->f_mmap->len);
  ASSERT(!req->xtra_content || !req->xtra_content->len);
  if (req->xtra_content)
    vstr_del(req->xtra_content, 1, req->xtra_content->len);
  
  req->vhost_prefix_len = 0;
  
  req->sects->malloc_bad = FALSE;

  req->content_encoding_gzip     = FALSE;
  req->content_encoding_bzip2    = FALSE;
  req->content_encoding_identity = TRUE;

  req->output_keep_alive_hdr = FALSE;

  req->user_return_error_code = FALSE;

  req->vary_star = con ? con->vary_star : FALSE;
  req->vary_a    = FALSE;
  req->vary_ac   = FALSE;
  req->vary_ae   = FALSE;
  req->vary_al   = FALSE;
  req->vary_rf   = FALSE;
  req->vary_ua   = FALSE;

  req->direct_uri         = FALSE;
  req->direct_filename    = FALSE;
  req->skip_document_root = FALSE;
  
  req->ver_0_9    = FALSE;
  req->ver_1_1    = FALSE;
  req->head_op    = FALSE;

  req->chked_encoded_path = FALSE;
  
  req->neg_content_type_done = FALSE;
  req->neg_content_lang_done = FALSE;

  req->conf_secure_dirs   = FALSE;
  req->conf_friendly_dirs = FALSE;
  
  req->done_once  = TRUE;
  req->using_req  = TRUE;

  req->malloc_bad = FALSE;

  if (con)
    policy = con->policy;
  else
    policy = (Httpd_policy_opts *)httpd_opts->s->def_policy;  
  httpd_policy_change_req(req, policy);
  
  return (req);
}

void http_req_free(Httpd_req_data *req)
{
  if (!req) /* for if/when move to malloc/free */
    return;

  ASSERT(req->done_once && req->using_req);

  /* we do vstr deletes here to return the nodes back to the pool */
  vstr_del(req->fname, 1, req->fname->len);
  ASSERT(!req->http_hdrs->multi->combiner_store->len);
  if (req->f_mmap)
    vstr_del(req->f_mmap, 1, req->f_mmap->len);

  http__clear_xtra(req);
  
  req->http_hdrs->multi->comb = NULL;

  req->using_req = FALSE;
}

void http_req_exit(void)
{
  Httpd_req_data *req = http_req_make(NULL);
  struct Http_hdrs__multi *tmp = NULL;
  
  if (!req)
    return;

  tmp = req->http_hdrs->multi;
  
  vstr_free_base(req->fname);          req->fname          = NULL;
  vstr_free_base(tmp->combiner_store); tmp->combiner_store = NULL;
  vstr_free_base(req->f_mmap);         req->f_mmap         = NULL;
  vstr_free_base(req->xtra_content);   req->xtra_content   = NULL;
  vstr_sects_free(req->sects);         req->sects          = NULL;
  
  req->done_once = FALSE;
  req->using_req = FALSE;
}


/* HTTP crack -- Implied linear whitespace between tokens, note that it
 * is *LWS == *([CRLF] 1*(SP | HT)) */
static void http__skip_lws(const Vstr_base *s1, size_t *pos, size_t *len)
{
  size_t lws__len = 0;
  
  ASSERT(s1 && pos && len);
  
  while (TRUE)
  {
    if (VPREFIX(s1, *pos, *len, HTTP_EOL))
    {
      *len -= CLEN(HTTP_EOL); *pos += CLEN(HTTP_EOL);
    }
    else if (lws__len)
      break;
    
    if (!(lws__len = vstr_spn_cstr_chrs_fwd(s1, *pos, *len, HTTP_LWS)))
      break;
    *len -= lws__len;
    *pos += lws__len;
  }
}

/* prints out headers in human friedly way for log files */
#define PCUR (pos + (base->len - orig_len))
static int http_app_vstr_escape(Vstr_base *base, size_t pos,
                                Vstr_base *sf, size_t sf_pos, size_t sf_len)
{
  unsigned int sf_flags = VSTR_TYPE_ADD_BUF_PTR;
  Vstr_iter iter[1];
  size_t orig_len = base->len;
  int saved_malloc_bad = FALSE;
  size_t norm_chr = 0;

  if (!sf_len) /* hack around !sf_pos */
    return (TRUE);
  
  if (!vstr_iter_fwd_beg(sf, sf_pos, sf_len, iter))
    return (FALSE);

  saved_malloc_bad = base->conf->malloc_bad;
  base->conf->malloc_bad = FALSE;
  while (sf_len)
  { /* assumes ASCII */
    char scan = vstr_iter_fwd_chr(iter, NULL);

    if ((scan >= ' ') && (scan <= '~') && (scan != '"') && (scan != '\\'))
      ++norm_chr;
    else
    {
      vstr_add_vstr(base, PCUR, sf, sf_pos, norm_chr, sf_flags);
      sf_pos += norm_chr;
      norm_chr = 0;
      
      if (0) {}
      else if (scan == '"')  vstr_add_cstr_buf(base, PCUR, "\\\"");
      else if (scan == '\\') vstr_add_cstr_buf(base, PCUR, "\\\\");
      else if (scan == '\t') vstr_add_cstr_buf(base, PCUR, "\\t");
      else if (scan == '\v') vstr_add_cstr_buf(base, PCUR, "\\v");
      else if (scan == '\r') vstr_add_cstr_buf(base, PCUR, "\\r");
      else if (scan == '\n') vstr_add_cstr_buf(base, PCUR, "\\n");
      else if (scan == '\b') vstr_add_cstr_buf(base, PCUR, "\\b");
      else
        vstr_add_sysfmt(base, PCUR, "\\x%02hhx", scan);
      ++sf_pos;
    }
    
    --sf_len;
  }

  vstr_add_vstr(base, PCUR, sf, sf_pos, norm_chr, sf_flags);
  
  if (base->conf->malloc_bad)
    return (FALSE);
  
  base->conf->malloc_bad = saved_malloc_bad;
  return (TRUE);
}
#undef PCUR

static int http__fmt__add_vstr_add_vstr(Vstr_base *base, size_t pos,
                                        Vstr_fmt_spec *spec)
{
  Vstr_base *sf = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_pos = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);
  size_t sf_len = VSTR_FMT_CB_ARG_VAL(spec, size_t, 2);
  
  return (http_app_vstr_escape(base, pos, sf, sf_pos, sf_len));
}
int http_fmt_add_vstr_add_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, http__fmt__add_vstr_add_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}
static int http__fmt__add_vstr_add_sect_vstr(Vstr_base *base, size_t pos,
                                             Vstr_fmt_spec *spec)
{
  Vstr_base *sf     = VSTR_FMT_CB_ARG_PTR(spec, 0);
  Vstr_sects *sects = VSTR_FMT_CB_ARG_PTR(spec, 1);
  unsigned int num  = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 2);
  size_t sf_pos     = VSTR_SECTS_NUM(sects, num)->pos;
  size_t sf_len     = VSTR_SECTS_NUM(sects, num)->len;
  
  return (http_app_vstr_escape(base, pos, sf, sf_pos, sf_len));
}
int http_fmt_add_vstr_add_sect_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, http__fmt__add_vstr_add_sect_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static void http__app_hdr_hdr(Vstr_base *out, const char *hdr)
{
  vstr_add_cstr_buf(out, out->len, hdr);
  vstr_add_cstr_buf(out, out->len, ": ");  
}
static void http__app_hdr_eol(Vstr_base *out)
{
  vstr_add_cstr_buf(out, out->len, HTTP_EOL);
}

/* use single node */
#define HTTP_APP_HDR_CONST_CSTR(o, h, c)                        \
    vstr_add_cstr_ptr(out, (out)->len, h ": " c HTTP_EOL)
static void http_app_hdr_cstr(Vstr_base *out, const char *hdr, const char *data)
{
  http__app_hdr_hdr(out, hdr);
  vstr_add_cstr_buf(out, out->len, data);
  http__app_hdr_eol(out);
}

static void http_app_hdr_vstr(Vstr_base *out, const char *hdr,
                              const Vstr_base *s1, size_t vpos, size_t vlen,
                              unsigned int type)
{
  http__app_hdr_hdr(out, hdr);
  vstr_add_vstr(out, out->len, s1, vpos, vlen, type);
  http__app_hdr_eol(out);
}

static void http_app_hdr_vstr_def(Vstr_base *out, const char *hdr,
                                  const Vstr_base *s1, size_t vpos, size_t vlen)
{
  http__app_hdr_hdr(out, hdr);
  vstr_add_vstr(out, out->len, s1, vpos, vlen, VSTR_TYPE_ADD_DEF);
  http__app_hdr_eol(out);
}

static void http_app_hdr_conf_vstr(Vstr_base *out, const char *hdr,
                                   const Vstr_base *s1)
{
  http__app_hdr_hdr(out, hdr);
  vstr_add_vstr(out, out->len, s1, 1, s1->len, VSTR_TYPE_ADD_DEF);
  http__app_hdr_eol(out);
}

static void http_app_hdr_fmt(Vstr_base *out, const char *hdr,
                             const char *fmt, ...)
   VSTR__COMPILE_ATTR_FMT(3, 4);
static void http_app_hdr_fmt(Vstr_base *out, const char *hdr,
                             const char *fmt, ...)
{
  va_list ap;

  http__app_hdr_hdr(out, hdr);
  
  va_start(ap, fmt);
  vstr_add_vfmt(out, out->len, fmt, ap);
  va_end(ap);

  http__app_hdr_eol(out);
}

static void http_app_hdr_uintmax(Vstr_base *out, const char *hdr,
                                 VSTR_AUTOCONF_uintmax_t data)
{
  http__app_hdr_hdr(out, hdr);
  vstr_add_fmt(out, out->len, "%ju", data);
  http__app_hdr_eol(out);
}

#define HTTP__VARY_ADD(x, y) do {                                       \
      ASSERT((x) < (sizeof(varies_ptr) / sizeof(varies_ptr[0])));       \
                                                                        \
      varies_ptr[(x)] = (y);                                            \
      varies_len[(x)] = CLEN(y);                                        \
      ++(x);                                                            \
    } while (FALSE)

void http_app_def_hdrs(struct Con *con, struct Httpd_req_data *req,
                       unsigned int http_ret_code,
                       const char *http_ret_line, time_t mtime,
                       const char *custom_content_type,
                       int use_range,
                       VSTR_AUTOCONF_uintmax_t content_length)
{
  Vstr_base *out = con->evnt->io_w;
  Date_store *ds = httpd_opts->date;
  
  if (use_range)
    use_range = req->policy->use_range;
  
  vstr_add_fmt(out, out->len, "%s %03u %s" HTTP_EOL,
               "HTTP/1.1", http_ret_code, http_ret_line);
  http_app_hdr_cstr(out, "Date", date_rfc1123(ds, req->now));
  http_app_hdr_conf_vstr(out, "Server", req->policy->server_name);

  if (mtime)
  { /* if mtime in future, chop it #14.29 
     * for cache validation we don't cmp last-modified == now either */
    if (difftime(req->now, mtime) <= 0)
      mtime = req->now;
    http_app_hdr_cstr(out, "Last-Modified", date_rfc1123(ds, mtime));
  }
  
  switch (con->keep_alive)
  {
    case HTTP_NON_KEEP_ALIVE:
      HTTP_APP_HDR_CONST_CSTR(out, "Connection", "close");
      break;
      
    case HTTP_1_0_KEEP_ALIVE:
      HTTP_APP_HDR_CONST_CSTR(out, "Connection", "Keep-Alive");
      /* FALLTHROUGH */
    case HTTP_1_1_KEEP_ALIVE: /* max=xxx ? */
      if (req->output_keep_alive_hdr)
        http_app_hdr_fmt(out, "Keep-Alive",
                         "%s=%u", "timeout", req->policy->s->idle_timeout);
      
      ASSERT_NO_SWITCH_DEF();
  }

  switch (http_ret_code)
  {
    case 200: /* OK */
      /* case 206: */ /* OK - partial -- needed? */
    case 301: case 302: case 303: case 307: /* Redirects */
    case 304: /* Not modified */
    case 406: /* Not accept - a */
      /* case 410: */ /* Gone - like 404 or 301 ? */
    case 412: /* Not accept - precondition */
    case 413: /* Too large */
    case 416: /* Bad range */
    case 417: /* Not accept - expect contained something */
      if (use_range)
        HTTP_APP_HDR_CONST_CSTR(out, "Accept-Ranges", "bytes");
  }
  
  if (req->vary_star)
    HTTP_APP_HDR_CONST_CSTR(out, "Vary", "*");
  else if (req->vary_a || req->vary_ac || req->vary_ae || req->vary_al ||
           req->vary_rf || req->vary_ua)
  {
    const char *varies_ptr[6];
    size_t      varies_len[6];
    unsigned int num = 0;
    
    if (req->vary_ua) HTTP__VARY_ADD(num, "User-Agent");
    if (req->vary_rf) HTTP__VARY_ADD(num, "Referer");
    if (req->vary_al) HTTP__VARY_ADD(num, "Accept-Language");
    if (req->vary_ae) HTTP__VARY_ADD(num, "Accept-Encoding");
    if (req->vary_ac) HTTP__VARY_ADD(num, "Accept-Charset");
    if (req->vary_a)  HTTP__VARY_ADD(num, "Accept");

    ASSERT(num && (num <= 5));
    
    http__app_hdr_hdr(out, "Vary");
    while (num-- > 1)
    {
      vstr_add_buf(out, out->len, varies_ptr[num], varies_len[num]);
      vstr_add_cstr_buf(out, out->len, ",");
    }
    ASSERT(num == 0);
    
    vstr_add_buf(out, out->len, varies_ptr[0], varies_len[0]);
    http__app_hdr_eol(out);
  }

  if (con->use_mpbr)
  {
    ASSERT(con->mpbr_ct && !con->mpbr_ct->len);
    if (req->content_type_vs1 && req->content_type_len)
      http_app_hdr_vstr_def(con->mpbr_ct, "Content-Type",
                            HTTP__XTRA_HDR_PARAMS(req, content_type));
    http_app_hdr_cstr(out, "Content-Type",
                      "multipart/byteranges; boundary=SEP");
  }
  else if (req->content_type_vs1 && req->content_type_len)
    http_app_hdr_vstr_def(out, "Content-Type",
                          HTTP__XTRA_HDR_PARAMS(req, content_type));
  else if (custom_content_type) /* possible we don't send one */
    http_app_hdr_cstr(out, "Content-Type", custom_content_type);
  
  if (req->content_encoding_bzip2)
      HTTP_APP_HDR_CONST_CSTR(out, "Content-Encoding", "bzip2");
  else if (req->content_encoding_gzip)
  {
    if (req->content_encoding_xgzip)
      HTTP_APP_HDR_CONST_CSTR(out, "Content-Encoding", "x-gzip");
    else
      HTTP_APP_HDR_CONST_CSTR(out, "Content-Encoding", "gzip");
  }
  
  if (!con->use_mpbr)
    http_app_hdr_uintmax(out, "Content-Length", content_length);
}
#undef HTTP__VARY_ADD

static void http_app_end_hdrs(Vstr_base *out)
{ /* apache contains a workaround for buggy Netscape 2.x, 3.x and 4.0beta */
  http__app_hdr_eol(out);
}

static void http_vlg_def(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  Vstr_sect_node *h_h  = req->http_hdrs->hdr_host;
  Vstr_sect_node *h_ua = req->http_hdrs->hdr_ua;
  Vstr_sect_node *h_r  = req->http_hdrs->hdr_referer;

  vlg_info(vlg, (" host[\"$<http-esc.vstr:%p%zu%zu>\"]"
                 " UA[\"$<http-esc.vstr:%p%zu%zu>\"]"
                 " ref[\"$<http-esc.vstr:%p%zu%zu>\"]"),
           data, h_h->pos, h_h->len,    
           data, h_ua->pos, h_ua->len,
           data, h_r->pos, h_r->len);

  if (req->ver_0_9)
    vlg_info(vlg, " ver[\"HTTP/0.9]\"");
  else
    vlg_info(vlg, " ver[\"$<vstr.sect:%p%p%u>\"]", data, req->sects, 3);

  vlg_info(vlg, ": $<http-esc.vstr:%p%zu%zu>\n",
           data, req->path_pos, req->path_len);
}

static struct File_sect *httpd__fd_next(struct Con *con)
{
  struct File_sect *fs = NULL;
  
  ASSERT(con->fs &&
         (con->fs_off <= con->fs_num) && (con->fs_num <= con->fs_sz));
  
  if (++con->fs_off >= con->fs_num)
  {
    fs = &con->fs[con->fs_off - 1];
    fs->len = 0;
    if (fs->fd != -1)
      close(fs->fd);
    fs->fd = -1;

    fs = con->fs;
    fs->fd = -1;
    fs->len = 0;
    
    con->fs_off = 0;
    con->fs_num = 0;

    con->use_mpbr = FALSE;
    if (con->mpbr_ct)
      vstr_del(con->mpbr_ct, 1, con->mpbr_ct->len);

    return (NULL);
  }

  /* only allow multipart/byterange atm. where all fd's are the same */
  ASSERT(con->fs[con->fs_off - 1].fd == con->fs[con->fs_off].fd);
  
  fs = &con->fs[con->fs_off];
  if (con->use_posix_fadvise)
    posix_fadvise64(fs->fd, fs->off, fs->len, POSIX_FADV_SEQUENTIAL);
  
  return (fs);
}

void httpd_fin_fd_close(struct Con *con)
{
  con->use_posix_fadvise = FALSE;
  while (httpd__fd_next(con))
  { /* do nothing */ }
}

static int http_fin_req(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *out = con->evnt->io_w;
  
  ASSERT(!out->conf->malloc_bad);
  http__clear_hdrs(req);

  /* Note: Not resetting con->parsed_method_ver_1_0,
   * if it's non-0.9 now it should continue to be */
  
  if (!con->keep_alive) /* all input is here */
  {
    evnt_wait_cntl_del(con->evnt, POLLIN);
    req->len = con->evnt->io_r->len; /* delete it all */
  }
  
  vstr_del(con->evnt->io_r, 1, req->len);
  
  evnt_put_pkt(con->evnt);

  if (req->policy->use_tcp_cork)
    evnt_fd_set_cork(con->evnt, TRUE);

  http_req_free(req);

  return (httpd_serv_send(con));
}

static int http_fin_fd_req(struct Con *con, Httpd_req_data *req)
{
  ASSERT(con->fs && (con->fs_off < con->fs_num) && (con->fs_num <= con->fs_sz));
  ASSERT(!con->fs_off);
  
  if (req->head_op || con->use_mmap || !con->fs->len)
    httpd_fin_fd_close(con);
  else if ((con->use_posix_fadvise = req->policy->use_posix_fadvise))
  {
    struct File_sect *fs = con->fs;
    posix_fadvise64(fs->fd, fs->off, fs->len, POSIX_FADV_SEQUENTIAL);
  }
  
  return (http_fin_req(con, req));
}

static int http_con_cleanup(struct Con *con, Httpd_req_data *req)
{
  con->evnt->io_r->conf->malloc_bad = FALSE;
  con->evnt->io_w->conf->malloc_bad = FALSE;
  
  http__clear_hdrs(req);
  http_req_free(req);
    
  return (FALSE);
}

static int http_con_close_cleanup(struct Con *con, Httpd_req_data *req)
{
  httpd_fin_fd_close(con);
  return (http_con_cleanup(con, req));
}

/* try to use gzip content-encoding on entity */
static int http__try_encoded_content(struct Con *con, Httpd_req_data *req,
                                     const struct stat64 *req_f_stat,
                                     Vstr_base *fname,
                                     const char *zip_ext, size_t zip_len)
{
  const char *fname_cstr = NULL;
  int fd = -1;
  int ret = FALSE;
  
  vstr_add_cstr_ptr(fname, fname->len, zip_ext);
  fname_cstr = vstr_export_cstr_ptr(fname, 1, fname->len);
  
  if (fname->conf->malloc_bad)
    vlg_warn(vlg, "Failed to export cstr for '%s'\n", zip_ext);
  else if ((fd = io_open_nonblock(fname_cstr)) == -1)
    vstr_sc_reduce(fname, 1, fname->len, zip_len);
  else
  {
    struct stat64 f_stat[1];
    
    if (fstat64(fd, f_stat) == -1)
      vlg_warn(vlg, "fstat: %m\n");
    else if ((req->policy->use_public_only && !(f_stat->st_mode & S_IROTH)) ||
             (S_ISDIR(f_stat->st_mode)) || (!S_ISREG(f_stat->st_mode)) ||
             (req_f_stat->st_mtime >  f_stat->st_mtime) ||
             !f_stat->st_size || /* zero sized compressed files aren't valid */
             (req_f_stat->st_size  <= f_stat->st_size))
    { /* ignore the encoded version */ }
    else
    {
      ASSERT(con->fs && !con->fs_num && !con->fs_off);
      
      /* swap, close the old fd (later) and use the new */
      SWAP_TYPE(con->fs->fd, fd, int);
      
      ASSERT(con->fs->len == (VSTR_AUTOCONF_uintmax_t)req_f_stat->st_size);
      /* _only_ copy the new size over, mtime etc. is from the original file */
      con->fs->len = req->f_stat->st_size = f_stat->st_size;
      req->encoded_mtime = f_stat->st_mtime;
      ret = TRUE;
    }
    close(fd);
  }

  return (ret);
}  

#define HTTP__PARSE_CHK_RET_OK() do {                   \
      HTTP_SKIP_LWS(data, pos, len);                    \
                                                        \
      if (!len ||                                       \
          VPREFIX(data, pos, len, ",") ||               \
          (allow_more && VPREFIX(data, pos, len, ";"))) \
      {                                                 \
        *passed_pos = pos;                              \
        *passed_len = len;                              \
                                                        \
        return (TRUE);                                  \
      }                                                 \
    } while (FALSE)

/* What is the quality parameter, value between 0 and 1000 inclusive.
 * returns TRUE on success, FALSE on failure. */
static int http_parse_quality(Vstr_base *data,
                              size_t *passed_pos, size_t *passed_len,
                              int allow_more, unsigned int *val)
{
  size_t pos = *passed_pos;
  size_t len = *passed_len;
  
  ASSERT(val);
  
  *val = 1000;
  
  HTTP_SKIP_LWS(data, pos, len);

  *passed_pos = pos;
  *passed_len = len;
  
  if (!len || VPREFIX(data, pos, len, ","))
    return (TRUE);
  else if (VPREFIX(data, pos, len, ";"))
  {
    int lead_zero = FALSE;
    unsigned int num_len = 0;
    unsigned int parse_flags = VSTR_FLAG02(PARSE_NUM, NO_BEG_PM, NO_NEGATIVE);
    
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);
    
    if (!VPREFIX(data, pos, len, "q"))
      return (!!allow_more);
    
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);
    
    if (!VPREFIX(data, pos, len, "="))
      return (!!allow_more);
    
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);

    /* if it's 0[.00?0?] TRUE, 0[.\d\d?\d?] or 1[.00?0?] is FALSE */
    if (!(lead_zero = VPREFIX(data, pos, len, "0")) &&
        !VPREFIX(data, pos, len, "1"))
      return (FALSE);
    *val = (!lead_zero) * 1000;
    
    len -= 1; pos += 1;
    
    HTTP__PARSE_CHK_RET_OK();

    if (!VPREFIX(data, pos, len, "."))
      return (FALSE);
    
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);

    *val += vstr_parse_uint(data, pos, len, 10 | parse_flags, &num_len, NULL);
    if (!num_len || (num_len > 3) || (*val > 1000))
      return (FALSE);
    if (num_len < 3) *val *= 10;
    if (num_len < 2) *val *= 10;
    ASSERT(*val <= 1000);
    
    len -= num_len; pos += num_len;
    
    HTTP__PARSE_CHK_RET_OK();
  }
  
  return (FALSE);
}
#undef HTTP__PARSE_CHK_RET_OK

static int http_parse_accept_encoding(struct Httpd_req_data *req)
{
  Vstr_base *data = req->http_hdrs->multi->comb;
  size_t pos = 0;
  size_t len = 0;
  unsigned int num = 0;
  unsigned int gzip_val     = 1001;
  unsigned int bzip2_val    = 1001;
  unsigned int identity_val = 1001;
  unsigned int star_val     = 1001;
  unsigned int dummy_val    = 1001;
  
  pos = req->http_hdrs->multi->hdr_accept_encoding->pos;
  len = req->http_hdrs->multi->hdr_accept_encoding->len;

  if (!req->policy->use_err_406 && !req->allow_accept_encoding)
    return (FALSE);
  req->vary_ae = TRUE;
  
  if (!len)
    return (FALSE);
  
  req->content_encoding_xgzip = FALSE;
  
  while (len)
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len,
                                         HTTP_EOL HTTP_LWS ";,");

    ++num;
    
    if (0) { }
    else if (VEQ(data, pos, tmp, "identity"))
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &identity_val))
        return (FALSE);
    }
    else if (req->allow_accept_encoding && VEQ(data, pos, tmp, "gzip"))
    {
      len -= tmp; pos += tmp;
      req->content_encoding_xgzip = FALSE;
      if (!http_parse_quality(data, &pos, &len, FALSE, &gzip_val))
        return (FALSE);
    }
    else if (req->allow_accept_encoding && VEQ(data, pos, tmp, "bzip2"))
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &bzip2_val))
        return (FALSE);
    }
    else if (req->allow_accept_encoding && VEQ(data, pos, tmp, "x-gzip"))
    {
      len -= tmp; pos += tmp;
      req->content_encoding_xgzip = TRUE;
      if (!http_parse_quality(data, &pos, &len, FALSE, &gzip_val))
        return (FALSE);
      gzip_val = 1000; /* ignore quality on x-gzip - just parse for errors */
    }
    else if (VEQ(data, pos, tmp, "*"))
    { /* "*;q=0,gzip" means TRUE ... and "*;q=1.0,gzip;q=0" means FALSE */
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &star_val))
        return (FALSE);
    }
    else
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &dummy_val))
        return (FALSE);
    }
    
    if (!len)
      break;
    assert(VPREFIX(data, pos, len, ","));
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);

    if (req->policy->max_AE_nodes && (num >= req->policy->max_AE_nodes))
      return (FALSE);
  }

  if (!req->allow_accept_encoding)
  {
    gzip_val  = 0;
    bzip2_val = 0;
  }
  
  if (gzip_val     == 1001) gzip_val     = star_val;
  if (bzip2_val    == 1001) bzip2_val    = star_val;
  if (identity_val == 1001) identity_val = star_val;

  if (gzip_val     == 1001) gzip_val     = 0;
  if (bzip2_val    == 1001) bzip2_val    = 0;
  if (identity_val == 1001) identity_val = 1;

  if (!identity_val)
    req->content_encoding_identity = FALSE;
  
  if ((identity_val > gzip_val) && (identity_val > bzip2_val))
    return (FALSE);

  if (gzip_val <= bzip2_val)
  { /* currently bzip2 is "preferred" so this works well always */
    req->content_encoding_gzip  = !!gzip_val;
    req->content_encoding_bzip2 = !!bzip2_val;
  }
  else
  { /* this doesn't "work well" if both are ok, and
     * only a *.bz2 file on disk. Maybe carry the quality values? */
    ASSERT(gzip_val);
    req->content_encoding_gzip  = TRUE;
    req->content_encoding_bzip2 = FALSE;
  }
  
  return (req->content_encoding_gzip || req->content_encoding_bzip2);
}

static void httpd__try_fd_encoding(struct Con *con, Httpd_req_data *req,
                                   const struct stat64 *fs, Vstr_base *fname)
{ /* Might normally add "!req->head_op && ..." but
   * http://www.w3.org/TR/chips/#gl6 says that's bad */
  if (http_parse_accept_encoding(req))
  {
    if ( req->content_encoding_bzip2 &&
         !http__try_encoded_content(con, req, fs, fname, ".bz2", CLEN(".bz2")))
      req->content_encoding_bzip2 = FALSE;
    if (!req->content_encoding_bzip2 && req->content_encoding_gzip &&
        !http__try_encoded_content(con, req, fs, fname, ".gz", CLEN(".gz")))
      req->content_encoding_gzip = FALSE;
  }
}

static void httpd__disable_sendfile(void)
{
  Opt_serv_policy_opts *scan = httpd_opts->s->def_policy;
  
  while (scan)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    tmp->use_sendfile = FALSE;
    scan = scan->next;
  }
}

static void httpd__disable_mmap(void)
{
  Opt_serv_policy_opts *scan = httpd_opts->s->def_policy;
  
  while (scan)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    tmp->use_mmap = FALSE;
    scan = scan->next;
  }
}

static void httpd_serv_call_mmap(struct Con *con, struct Httpd_req_data *req,
                                 struct File_sect *fs)
{
  static long pagesz = 0;
  Vstr_base *data = con->evnt->io_r;
  VSTR_AUTOCONF_uintmax_t mmoff = fs->off;
  VSTR_AUTOCONF_uintmax_t mmlen = fs->len;
  
  ASSERT(!req->f_mmap || !req->f_mmap->len);
  ASSERT(!con->use_mmap);
  ASSERT(!req->head_op);

  if (con->use_sendfile)
    return;

  if (con->fs_num > 1)
    return;
  
  if (!pagesz)
    pagesz = sysconf(_SC_PAGESIZE);
  if (pagesz == -1)
    httpd__disable_mmap();

  if (!req->policy->use_mmap ||
      (mmlen < HTTP_CONF_MMAP_LIMIT_MIN) || (mmlen > HTTP_CONF_MMAP_LIMIT_MAX))
    return;
  
  /* mmap offset needs to be aligned - so tweak offset before and after */
  mmoff /= pagesz;
  mmoff *= pagesz;
  ASSERT(mmoff <= fs->off);
  mmlen += fs->off - mmoff;

  if (!req->f_mmap && !(req->f_mmap = vstr_make_base(data->conf)))
    VLG_WARN_RET_VOID((vlg, /* fall back to read */
                       "failed to allocate mmap Vstr.\n"));
  ASSERT(!req->f_mmap->len);
  
  if (!vstr_sc_mmap_fd(req->f_mmap, 0, fs->fd, mmoff, mmlen, NULL))
  {
    if (errno == ENOSYS) /* also logs it */
      httpd__disable_mmap();

    VLG_WARN_RET_VOID((vlg, /* fall back to read */
                       "mmap($<http-esc.vstr:%p%zu%zu>,"
                       "(%ju,%ju)->(%ju,%ju)): %m\n",
                       req->fname, (size_t)1, req->fname->len,
                       fs->off, fs->len, mmoff, mmlen));
  }
  
  con->use_mmap = TRUE;  
  vstr_del(req->f_mmap, 1, fs->off - mmoff); /* remove alignment */
    
  ASSERT(req->f_mmap->len == fs->len);
}

static int httpd__serv_call_seek(struct Con *con, struct File_sect *fs)
{
  if (con->use_mmap || con->use_sendfile)
    return (TRUE);

  if (fs->off && fs->len && (lseek64(fs->fd, fs->off, SEEK_SET) == -1))
    return (FALSE);
  
  return (TRUE);
}

static void httpd_serv_call_seek(struct Con *con, struct Httpd_req_data *req,
                                 struct File_sect *fs,
                                 unsigned int *http_ret_code,
                                 const char ** http_ret_line)
{
  ASSERT(!req->head_op);

  if (!httpd__serv_call_seek(con, fs))
  { /* this should be impossible for normal files AFAIK */
    vlg_warn(vlg, "lseek($<http-esc.vstr:%p%zu%zu>,off=%ju): %m\n",
             req->fname, (size_t)1, req->fname->len, fs->off);
    /* opts->use_range - turn off? */
    req->http_hdrs->multi->hdr_range->pos = 0;
    *http_ret_code = 200;
    *http_ret_line = "OK - Range Failed";
  }
}

static int http__conf_req(struct Con *con, Httpd_req_data *req)
{
  Conf_parse *conf = NULL;
  
  if (!(conf = conf_parse_make(NULL)))
    return (FALSE);
  
  if (!httpd_conf_req_parse_file(conf, con, req))
  {
    Vstr_base *s1 = req->policy->s->policy_name;
    Vstr_base *s2 = conf->tmp;

    if (!req->user_return_error_code)
      vlg_info(vlg, "CONF-REQ-ERR from[$<sa:%p>]: policy $<vstr.all:%p>"
               " backtrace: $<vstr.all:%p>\n", CON_CEVNT_SA(con), s1, s2);
    conf_parse_free(conf);
    return (TRUE);
  }
  conf_parse_free(conf);
  
  if (req->direct_uri)
  {
    Vstr_base *s1 = req->policy->s->policy_name;
    vlg_info(vlg, "CONF-REQ-ERR from[$<sa:%p>]: policy $<vstr.all:%p>"
             " Has URI.\n", CON_CEVNT_SA(con), s1);
    HTTPD_ERR_RET(req, 503, TRUE);
  }
  
  return (TRUE);
}

static void http_app_hdrs_url(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *out = con->evnt->io_w;
  
  if (req->cache_control_vs1)
    http_app_hdr_vstr_def(out, "Cache-Control",
                          HTTP__XTRA_HDR_PARAMS(req, cache_control));
  if (req->expires_vs1 && (req->expires_time > req->f_stat->st_mtime))
    http_app_hdr_vstr_def(out, "Expires",
                          HTTP__XTRA_HDR_PARAMS(req, expires));
  if (req->p3p_vs1)
    http_app_hdr_vstr_def(out, "P3P",
                          HTTP__XTRA_HDR_PARAMS(req, p3p));
}

static void http_app_hdrs_file(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *out = con->evnt->io_w;
  time_t mtime     = req->f_stat->st_mtime;
  time_t enc_mtime = req->encoded_mtime;
  
  if (req->content_disposition_vs1)
    http_app_hdr_vstr_def(out, "Content-Disposition",
                          HTTP__XTRA_HDR_PARAMS(req, content_disposition));
  if (req->content_language_vs1)
    http_app_hdr_vstr_def(out, "Content-Language",
                          HTTP__XTRA_HDR_PARAMS(req, content_language));
  if (req->content_encoding_bzip2)
  {
    if (req->bzip2_content_md5_vs1 && (req->content_md5_time > enc_mtime))
      http_app_hdr_vstr_def(out, "Content-MD5",
                            HTTP__XTRA_HDR_PARAMS(req, bzip2_content_md5));
  }
  else if (req->content_encoding_gzip)
  {
    if (req->gzip_content_md5_vs1 && (req->content_md5_time > enc_mtime))
      http_app_hdr_vstr_def(out, "Content-MD5",
                            HTTP__XTRA_HDR_PARAMS(req, gzip_content_md5));
  }
  else if (req->content_md5_vs1 && (req->content_md5_time > mtime))
    http_app_hdr_vstr_def(out, "Content-MD5",
                          HTTP__XTRA_HDR_PARAMS(req, content_md5));
  
  if (req->content_encoding_bzip2)
  {
    if (req->bzip2_etag_vs1 && (req->etag_time > enc_mtime))
      http_app_hdr_vstr_def(out, "ETag",
                            HTTP__XTRA_HDR_PARAMS(req, bzip2_etag));
  }
  else if (req->content_encoding_gzip)
  {
    if (req->gzip_etag_vs1 && (req->etag_time > enc_mtime))
      http_app_hdr_vstr_def(out, "ETag",
                            HTTP__XTRA_HDR_PARAMS(req, gzip_etag));
  }
  else if (req->etag_vs1 && (req->etag_time > mtime))
    http_app_hdr_vstr_def(out, "ETag", HTTP__XTRA_HDR_PARAMS(req, etag));
  
  if (req->link_vs1)
    http_app_hdr_vstr_def(out, "Link", HTTP__XTRA_HDR_PARAMS(req, link));
}

static void http_app_hdrs_mpbr(struct Con *con, struct File_sect *fs)
{
  Vstr_base *out = con->evnt->io_w;
  VSTR_AUTOCONF_uintmax_t range_beg;
  VSTR_AUTOCONF_uintmax_t range_end;    

  ASSERT(fs && (fs->fd != -1));
  
  range_beg = fs->off;
  range_end = range_beg + fs->len - 1;

  vstr_add_cstr_ptr(out, out->len, "--SEP" HTTP_EOL);
  HTTPD_APP_REF_ALLVSTR(out, con->mpbr_ct);
  http_app_hdr_fmt(out, "Content-Range",
                   "%s %ju-%ju/%ju", "bytes", range_beg, range_end,
                   con->mpbr_fs_len);
  http_app_end_hdrs(out);
}

static void http_prepend_doc_root(Vstr_base *fname, Httpd_req_data *req)
{
  Vstr_base *dir = req->policy->document_root;
  
  ASSERT((dir->len   >= 1) && vstr_cmp_cstr_eq(dir,   dir->len, 1, "/"));
  ASSERT((fname->len >= 1) && vstr_cmp_cstr_eq(fname,        1, 1, "/"));
  vstr_add_vstr(fname, 0, dir, 1, dir->len - 1, VSTR_TYPE_ADD_BUF_REF);
}

static void http_app_err_file(struct Con *con, Httpd_req_data *req,
                              Vstr_base *fname, size_t *vhost_prefix_len)
{
  Vstr_base *dir = NULL;

  ASSERT(con && req);
  ASSERT(vhost_prefix_len && !*vhost_prefix_len);

  dir = req->policy->req_err_dir;
  ASSERT((dir->len   >= 1) && vstr_cmp_cstr_eq(dir,          1, 1, "/"));
  ASSERT((dir->len   >= 1) && vstr_cmp_cstr_eq(dir,   dir->len, 1, "/"));
  HTTPD_APP_REF_ALLVSTR(fname, dir);
  
  vstr_add_fmt(fname, fname->len, "%u", req->error_code);
  
  if (req->policy->use_vhosts_name)
  {
    Vstr_base *data = con->evnt->io_r;
    Vstr_sect_node *h_h = req->http_hdrs->hdr_host;
    size_t orig_len = fname->len;
    
    if (!h_h->len)
      httpd_sc_add_default_hostname(con, req, fname, 0);
    else if (vstr_add_vstr(fname, 0, data, /* add as buf's, for lowercase op */
                           h_h->pos, h_h->len, VSTR_TYPE_ADD_DEF))
      vstr_conv_lowercase(fname, 1, h_h->len);
    vstr_add_cstr_ptr(fname, 0, "/");

    *vhost_prefix_len = (fname->len - orig_len);
  }
}

static int http_fin_err_req(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *out = con->evnt->io_w;
  int use_cust_err_msg = FALSE;

  ASSERT(req->error_code);
  
  req->content_encoding_gzip  = FALSE;
  req->content_encoding_bzip2 = FALSE;
  
  if ((req->error_code == 400) || (req->error_code == 405) ||
      (req->error_code == 413) ||
      (req->error_code == 500) || (req->error_code == 501))
    con->keep_alive = HTTP_NON_KEEP_ALIVE;
  
  ASSERT(con->fs && !con->fs_num);
  
  vlg_info(vlg, "ERREQ from[$<sa:%p>] err[%03u %s]",
           CON_CEVNT_SA(con), req->error_code, req->error_line);
  if (req->sects->num >= 2)
    http_vlg_def(con, req);
  else
    vlg_info(vlg, "%s", "\n");

  if (req->malloc_bad)
  { /* delete all input to give more room */
    ASSERT(req->error_code == 500);
    vstr_del(con->evnt->io_r, 1, con->evnt->io_r->len);
  }
  else if (((req->error_code == 400) ||
            (req->error_code == 403) ||
            (req->error_code == 404) ||
            (req->error_code == 410) ||
            (req->error_code == 500) ||
            (req->error_code == 503)) && req->policy->req_err_dir->len)
  { /* custom err message */
    Vstr_base *fname = vstr_make_base(req->fname->conf);
    size_t vhost_prefix_len = 0;
    const char *fname_cstr = NULL;
    struct stat64 f_stat[1];

    if (!fname)
      goto fail_custom_err;

    /*
  req->vary_star = con ? con->vary_star : FALSE;
  req->vary_a    = FALSE;
  req->vary_ac   = FALSE;
  req->vary_ae   = FALSE;
  req->vary_al   = FALSE;
  req->vary_rf   = FALSE;
  req->vary_ua   = FALSE;
    */
    
    if (!req->user_return_error_code)
    {
      req->cache_control_vs1 = NULL;
      req->expires_vs1       = NULL;
      req->p3p_vs1           = NULL;
    }

    if (req->user_return_error_code && req->direct_filename)
    {
      HTTPD_APP_REF_ALLVSTR(fname, req->fname);
      if (!req->skip_document_root)
        http_prepend_doc_root(fname, req);
    }
    else if (!req->policy->req_conf_dir)
    {
      http_app_err_file(con, req, fname, &vhost_prefix_len);
      http_prepend_doc_root(fname, req);
    }
    else
    {
      int conf_ret = FALSE;
      unsigned int code = req->error_code;
      unsigned int ncode = 0;
      
      http_app_err_file(con, req, fname, &vhost_prefix_len);

      req->content_type_vs1 = NULL;
      req->error_code = 0;
      req->skip_document_root = FALSE;
      
      SWAP_TYPE(fname, req->fname, Vstr_base *);
      SWAP_TYPE(vhost_prefix_len, req->vhost_prefix_len, size_t);
      conf_ret = http__conf_req(con, req);
      SWAP_TYPE(fname, req->fname, Vstr_base *);
      SWAP_TYPE(vhost_prefix_len, req->vhost_prefix_len, size_t);

      ncode = req->error_code;
      switch (code)
      {
        case 400: HTTPD_ERR(req, 400); break;
        case 403: HTTPD_ERR(req, 403); break;
        case 404: HTTPD_ERR(req, 404); break;
        case 410: HTTPD_ERR(req, 410); break;
        case 500: HTTPD_ERR(req, 500); break;
        case 503: HTTPD_ERR(req, 503);
          ASSERT_NO_SWITCH_DEF();
      }
      if (!conf_ret)
        goto fail_custom_err;
      if (ncode)
        goto fail_custom_err; /* don't allow remapping errors -- loops */
      
      if (!req->skip_document_root)
        http_prepend_doc_root(fname, req);
    }
    fname_cstr = vstr_export_cstr_ptr(fname, 1, fname->len);
    
    if (fname->conf->malloc_bad)
      goto fail_custom_err;
    ASSERT(con->fs && (con->fs->fd == -1));
    if ((con->fs->fd = io_open_nonblock(fname_cstr)) == -1)
      goto fail_custom_err;

    if (fstat64(con->fs->fd, f_stat) == -1)
    {
      httpd_fin_fd_close(con);
      goto fail_custom_err;
    }
    if (req->policy->use_public_only && !(f_stat->st_mode & S_IROTH))
    {
      httpd_fin_fd_close(con);
      goto fail_custom_err;
    }
    if (!S_ISREG(f_stat->st_mode))
    {
      httpd_fin_fd_close(con);
      goto fail_custom_err;
    }
    con->fs_off = 0;
    con->fs_num = 1;
    con->fs->off = 0;
    con->fs->len = f_stat->st_size;
    
    if (!req->ver_0_9)
      httpd__try_fd_encoding(con, req, f_stat, fname);
    
    con->use_mmap = FALSE;
    if (!req->head_op)
      httpd_serv_call_mmap(con, req, con->fs);

    req->error_len   = con->fs->len;
    use_cust_err_msg = TRUE;
    
   fail_custom_err:
    fname->conf->malloc_bad = FALSE;
    vstr_free_base(fname);
  }

  if (!use_cust_err_msg)
    req->content_type_vs1 = NULL;

  if (!req->ver_0_9)
  { /* use_range is dealt with inside */
    http_app_def_hdrs(con, req, req->error_code, req->error_line,
                      httpd_opts->beg_time, "text/html", TRUE, req->error_len);
    
    if (req->error_code == 416)
      http_app_hdr_fmt(out, "Content-Range", "%s */%ju", "bytes",
                          (VSTR_AUTOCONF_uintmax_t)req->f_stat->st_size);

    if (req->error_code == 401)
      http_app_hdr_fmt(out, "WWW-Authenticate",
                       "Basic realm=\"$<vstr.all:%p>\"",
                       req->policy->auth_realm);
    
    if ((req->error_code == 405) || (req->error_code == 501))
      HTTP_APP_HDR_CONST_CSTR(out, "Allow", "GET, HEAD, OPTIONS, TRACE");
    
    if ((req->error_code == 301) || (req->error_code == 302) ||
        (req->error_code == 303) || (req->error_code == 307))
    { /* make sure we haven't screwed up and allowed response splitting */
      Vstr_base *tmp = req->fname;
      
      ASSERT(!vstr_srch_cstr_chrs_fwd(tmp, 1, tmp->len, HTTP_EOL));
      http_app_hdr_vstr(out, "Location",
                           tmp, 1, tmp->len, VSTR_TYPE_ADD_ALL_BUF);
    }
    
    if (req->user_return_error_code || use_cust_err_msg)
      http_app_hdrs_url(con, req);
    if (use_cust_err_msg)
      http_app_hdrs_file(con, req);
    
    http_app_end_hdrs(out);
  }

  if (!req->head_op)
  {
    Vstr_base *loc = req->fname;

    switch (req->error_code)
    {
      case 301: vstr_add_fmt(out, out->len, CONF_MSG_FMT_301,
                             CONF_MSG__FMT_301_BEG,
                             loc, (size_t)1, loc->len, VSTR_TYPE_ADD_ALL_BUF,
                             CONF_MSG__FMT_30x_END); break;
      case 302: vstr_add_fmt(out, out->len, CONF_MSG_FMT_302,
                             CONF_MSG__FMT_302_BEG,
                             loc, (size_t)1, loc->len, VSTR_TYPE_ADD_ALL_BUF,
                             CONF_MSG__FMT_30x_END); break;
      case 303: vstr_add_fmt(out, out->len, CONF_MSG_FMT_303,
                             CONF_MSG__FMT_303_BEG,
                             loc, (size_t)1, loc->len, VSTR_TYPE_ADD_ALL_BUF,
                             CONF_MSG__FMT_30x_END); break;
      case 307: vstr_add_fmt(out, out->len, CONF_MSG_FMT_307,
                             CONF_MSG__FMT_307_BEG,
                             loc, (size_t)1, loc->len, VSTR_TYPE_ADD_ALL_BUF,
                             CONF_MSG__FMT_30x_END); break;
      default:
        if (use_cust_err_msg)
        {
          if (con->use_mmap && !vstr_mov(con->evnt->io_w, con->evnt->io_w->len,
                                         req->f_mmap, 1, req->f_mmap->len))
            return (http_con_close_cleanup(con, req));

          vlg_dbg3(vlg, "ERROR CUSTOM-404 REPLY:\n$<vstr.all:%p>\n", out);
          
          if (out->conf->malloc_bad)
            return (http_con_close_cleanup(con, req));
  
          return (http_fin_fd_req(con, req));
        }

        /* default internal error message */
        assert(req->error_len < SIZE_MAX);
        vstr_add_ptr(out, out->len, req->error_msg, req->error_len);
    }
  }
  
  vlg_dbg3(vlg, "ERROR REPLY:\n$<vstr.all:%p>\n", out);

  if (out->conf->malloc_bad)
    return (http_con_cleanup(con, req));
  
  return (http_fin_req(con, req));
}

static int http_fin_errmem_req(struct Con *con, struct Httpd_req_data *req)
{ /* try sending a 500 as the last msg */
  Vstr_base *out = con->evnt->io_w;
  
  /* remove anything we can to free space */
  vstr_sc_reduce(out, 1, out->len, out->len - req->orig_io_w_len);
  
  con->evnt->io_r->conf->malloc_bad = FALSE;
  con->evnt->io_w->conf->malloc_bad = FALSE;
  req->malloc_bad = TRUE;
  
  HTTPD_ERR_RET(req, 500, http_fin_err_req(con, req));
}

static int http_fin_err_close_req(struct Con *con, Httpd_req_data *req)
{
  httpd_fin_fd_close(con);
  return (http_fin_err_req(con, req));
}

int httpd_sc_add_default_hostname(struct Con *con,
                                  Httpd_req_data *req,
                                  Vstr_base *lfn, size_t pos)
{
  const Httpd_policy_opts *opts = req->policy;
  const Vstr_base *d_h = opts->default_hostname;
  Acpt_data *acpt_data = con->acpt_sa_ref->ptr;
  struct sockaddr_in *sinv4 = ACPT_SA_IN4(acpt_data);
  int ret = FALSE;
  
  ret = vstr_add_vstr(lfn, pos, d_h, 1, d_h->len, VSTR_TYPE_ADD_DEF);

  ASSERT(sinv4->sin_family == AF_INET);
  
  if (ret && req->policy->add_def_port && (ntohs(sinv4->sin_port) != 80))
    ret = vstr_add_fmt(lfn, pos + d_h->len, ":%hu", ntohs(sinv4->sin_port));

  return (ret);
}

static void httpd_sc_add_default_filename(Httpd_req_data *req, Vstr_base *fname)
{
  if (vstr_export_chr(fname, fname->len) == '/')
    HTTPD_APP_REF_ALLVSTR(fname, req->policy->dir_filename);
}                                  

/* turn //foo//bar// into /foo/bar/ */
int httpd_canon_path(Vstr_base *s1)
{
  size_t tmp = 0;
  
  while ((tmp = vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "//")))
  {
    if (!vstr_del(s1, tmp, 1))
      return (FALSE);
  }

  return (TRUE);
}

int httpd_canon_dir_path(Vstr_base *s1)
{
  if (!httpd_canon_path(s1))
    return (FALSE);

  if (s1->len && (vstr_export_chr(s1, s1->len) != '/'))
    return (vstr_add_cstr_ptr(s1, s1->len, "/"));

  return (TRUE);
}

int httpd_canon_abs_dir_path(Vstr_base *s1)
{
  if (!httpd_canon_dir_path(s1))
    return (FALSE);

  if (s1->len && (vstr_export_chr(s1, 1) != '/'))
    return (vstr_add_cstr_ptr(s1, 0, "/"));
  
  return (TRUE);
}
  
void httpd_req_absolute_uri(struct Con *con, Httpd_req_data *req,
                            Vstr_base *lfn, size_t pos, size_t len)
{
  Vstr_base *data = con->evnt->io_r;
  Vstr_sect_node *h_h = req->http_hdrs->hdr_host;
  size_t apos = pos - 1;
  size_t alen = lfn->len;
  int has_schema   = TRUE;
  int has_abs_path = TRUE;
  int has_data     = TRUE;
  unsigned int prev_num = 0;
  
  if (!VPREFIX(lfn, pos, len, "http://"))
  {
    has_schema = FALSE;
    if (!VPREFIX(lfn, pos, len, "/")) /* relative pathname */
    {
      if (VPREFIX(lfn, pos, len, "./"))
      {
        has_data = TRUE;
        vstr_del(lfn, pos, CLEN("./")); len -= CLEN("./");
        alen = lfn->len;
      }
      else
      {
        while (VPREFIX(lfn, pos, len, "../"))
        {
          ++prev_num;
          vstr_del(lfn, pos, CLEN("../")); len -= CLEN("../");
        }
        if (prev_num)
          alen = lfn->len;
        else
          has_data = !!lfn->len;
      }
      
      has_abs_path = FALSE;
    }
  }

  if (!has_schema)
  {
    vstr_add_cstr_buf(lfn, apos, "http://");
    apos += lfn->len - alen;
    alen = lfn->len;
    if (!h_h->len)
      httpd_sc_add_default_hostname(con, req, lfn, apos);
    else
      vstr_add_vstr(lfn, apos,
                    data, h_h->pos, h_h->len, VSTR_TYPE_ADD_ALL_BUF);
    apos += lfn->len - alen;
  }
    
  if (!has_abs_path)
  {
    size_t path_len = req->path_len;

    if (has_data || prev_num)
    {
      path_len -= vstr_cspn_cstr_chrs_rev(data, req->path_pos, path_len, "/");
      
      while (path_len && prev_num--)
      {
        path_len -= vstr_spn_cstr_chrs_rev(data,  req->path_pos, path_len, "/");
        path_len -= vstr_cspn_cstr_chrs_rev(data, req->path_pos, path_len, "/");
      }
      if (!path_len) path_len = 1; /* make sure there is a / at the end */
    }

    vstr_add_vstr(lfn, apos, data, req->path_pos, path_len, VSTR_TYPE_ADD_DEF);
  }
}

/* doing http://www.example.com/foo/bar where bar is a dir is bad
   because all relative links will be relative to foo, not bar.
   Also note that location must be "http://www.example.com/foo/bar/" or maybe
   "http:/foo/bar/" (but we don't use the later anymore)
*/
static int http_req_chk_dir(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *fname = req->fname;
  
  /* fname == what was just passed to open() */
  ASSERT(fname->len);

  if (req->policy->use_secure_dirs && !req->conf_secure_dirs)
  { /* check if file exists before redirecting without leaking info. */
    const char *fname_cstr = NULL;
    struct stat64 d_stat[1];
  
    vstr_add_cstr_buf(fname, fname->len, "/");
    HTTPD_APP_REF_ALLVSTR(fname, req->policy->dir_filename);
    
    fname_cstr = vstr_export_cstr_ptr(fname, 1, fname->len);
    if (fname->conf->malloc_bad)
      return (http_fin_errmem_req(con, req));

    if ((stat64(fname_cstr, d_stat) == -1) || !S_ISREG(d_stat->st_mode))
      HTTPD_ERR_RET(req, 404, http_fin_err_req(con, req));
  }
  
  vstr_del(fname, 1, fname->len);
  httpd_req_absolute_uri(con, req, fname, 1, fname->len);
  
  /* we got:       http://foo/bar/
   * and so tried: http://foo/bar/index.html
   *
   * but foo/bar/index.html is a directory (fun), so redirect to:
   *               http://foo/bar/index.html/
   */
  if (fname->len && (vstr_export_chr(fname, fname->len) == '/'))
    HTTPD_APP_REF_ALLVSTR(fname, req->policy->dir_filename);
  
  vstr_add_cstr_buf(fname, fname->len, "/");
  
  HTTPD_ERR_301(req);
  
  if (fname->conf->malloc_bad)
    return (http_fin_errmem_req(con, req));
  
  return (http_fin_err_req(con, req));
}

/* doing http://www.example.com/foo/bar/ when url is really
   http://www.example.com/foo/bar is a very simple mistake, so we almost
   certainly don't want a 404 */
static int http_req_chk_file(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *fname = req->fname;
  size_t len = 0;
  
  /* fname == what was just passed to open() */
  ASSERT(fname->len);

  if (!req->policy->use_friendly_dirs)
    HTTPD_ERR_RET(req, 404, http_fin_err_req(con, req));
  else if (!req->conf_friendly_dirs)
  { /* check if file exists before redirecting without leaking info. */
    const char *fname_cstr = NULL;
    struct stat64 d_stat[1];
    len = vstr_cspn_cstr_chrs_rev(fname, 1, fname->len, "/") + 1;

    /* must be a filename, can't be toplevel */
    if ((len <= 1) || (len >= (fname->len - req->vhost_prefix_len)))
      HTTPD_ERR_RET(req, 404, http_fin_err_req(con, req));

    vstr_sc_reduce(fname, 1, fname->len, len);
    
    fname_cstr = vstr_export_cstr_ptr(fname, 1, fname->len);
    if (fname->conf->malloc_bad)
      return (http_fin_errmem_req(con, req));    
    if ((stat64(fname_cstr, d_stat) == -1) && !S_ISREG(d_stat->st_mode))
      HTTPD_ERR_RET(req, 404, http_fin_err_req(con, req));
  }
  
  vstr_sub_cstr_ptr(fname, 1, fname->len, "./");
  httpd_req_absolute_uri(con, req, fname, 1, fname->len);
  assert(VSUFFIX(fname, 1, fname->len, "/"));
  vstr_sc_reduce(fname, 1, fname->len, strlen("/"));

  HTTPD_ERR_301(req);
  
  if (fname->conf->malloc_bad)
    return (http_fin_errmem_req(con, req));
  
  return (http_fin_err_req(con, req));
}

static void http_req_split_method(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *s1 = con->evnt->io_r;
  size_t pos = 1;
  size_t len = req->len;
  size_t el = 0;
  size_t skip_len = 0;
  unsigned int orig_num = req->sects->num;
  
  el = vstr_srch_cstr_buf_fwd(s1, pos, len, HTTP_EOL);
  ASSERT(el >= pos);
  len = el - pos; /* only parse the first line */

  /* split request */
  if (!(el = vstr_srch_cstr_chrs_fwd(s1, pos, len, HTTP_LWS)))
    return;
  vstr_sects_add(req->sects, pos, el - pos);
  len -= (el - pos); pos += (el - pos);

  /* just skip whitespace on method call... */
  if ((skip_len = vstr_spn_cstr_chrs_fwd(s1, pos, len, HTTP_LWS)))
  { len -= skip_len; pos += skip_len; }

  if (!len)
  {
    req->sects->num = orig_num;
    return;
  }
  
  if (!(el = vstr_srch_cstr_chrs_fwd(s1, pos, len, HTTP_LWS)))
  {
    vstr_sects_add(req->sects, pos, len);
    len = 0;
  }
  else
  {
    vstr_sects_add(req->sects, pos, el - pos);
    len -= (el - pos); pos += (el - pos);
    
    /* just skip whitespace on method call... */
    if ((skip_len = vstr_spn_cstr_chrs_fwd(s1, pos, len, HTTP_LWS)))
    { len -= skip_len; pos += skip_len; }
  }

  if (len)
    vstr_sects_add(req->sects, pos, len);
  else
    req->ver_0_9 = TRUE;
}

static void http_req_split_hdrs(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *s1 = con->evnt->io_r;
  size_t pos = 1;
  size_t len = req->len;
  size_t el = 0;
  size_t hpos = 0;

  ASSERT(req->sects->num >= 3);  
    
  /* skip first line */
  el = (VSTR_SECTS_NUM(req->sects, req->sects->num)->pos +
        VSTR_SECTS_NUM(req->sects, req->sects->num)->len);
  
  assert(VEQ(s1, el, CLEN(HTTP_EOL), HTTP_EOL));
  len -= (el - pos) + CLEN(HTTP_EOL); pos += (el - pos) + CLEN(HTTP_EOL);
  
  if (VPREFIX(s1, pos, len, HTTP_EOL))
    return; /* end of headers */

  ASSERT(vstr_srch_cstr_buf_fwd(s1, pos, len, HTTP_END_OF_REQUEST));
  /* split headers */
  hpos = pos;
  while ((el = vstr_srch_cstr_buf_fwd(s1, pos, len, HTTP_EOL)) != pos)
  {
    char chr = 0;
    
    len -= (el - pos) + CLEN(HTTP_EOL); pos += (el - pos) + CLEN(HTTP_EOL);

    chr = vstr_export_chr(s1, pos);
    if (chr == ' ' || chr == '\t') /* header continues to next line */
      continue;

    vstr_sects_add(req->sects, hpos, el - hpos);
    
    hpos = pos;
  }
}

/* return the length of a quoted string (must be >= 2), or 0 on syntax error */
static size_t http__len_quoted_string(const Vstr_base *data,
                                      size_t pos, size_t len)
{
  size_t orig_pos = pos;
  
  if (!VPREFIX(data, pos, len, "\""))
    return (0);
  
  len -= 1; pos += 1;

  while (TRUE)
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len, "\"\\");
    
    len -= tmp; pos += tmp;
    if (!len)
      return (0);
    
    if (vstr_export_chr(data, pos) == '"')
      return (vstr_sc_posdiff(orig_pos, pos));
    
    ASSERT(vstr_export_chr(data, pos) == '\\');
    if (len < 3) /* must be at least <\X"> */
      return (0);
    len -= 2; pos += 2;
  }

  assert_ret(FALSE, 0);
}

/* skip a quoted string, or fail on syntax error */
static int http__skip_quoted_string(const Vstr_base *data,
                                    size_t *pos, size_t *len)
{
  size_t qlen = http__len_quoted_string(data, *pos, *len);

  assert(VPREFIX(data, *pos, *len, "\""));
  
  if (!qlen)
    return (FALSE);
  
  *len -= qlen; *pos += qlen;

  HTTP_SKIP_LWS(data, *pos, *len);
  return (TRUE);
}

/* match non-week entity tags in both strings, return true if any match
 * only allow non-weak entity tags if allow_weak = FALSE */
static int httpd_match_etags(struct Httpd_req_data *req,
                             const Vstr_base *hdr, size_t hpos, size_t hlen,
                             const Vstr_base *vs1, size_t epos, size_t elen,
                             int allow_weak)
{
  int need_comma = FALSE;

  ASSERT(hdr);
  
  if (!vs1)
    return (FALSE);
  
  while (hlen)
  {
    int weak = FALSE;
    size_t htlen = 0;

    if (vstr_export_chr(hdr, hpos) == ',')
    {
      hlen -= 1; hpos += 1;
      HTTP_SKIP_LWS(hdr, hpos, hlen);
      need_comma = FALSE;
      continue;
    }
    else if (need_comma)
      return (FALSE);
    
    if (VPREFIX(hdr, hpos, hlen, "W/"))
    {
      weak = TRUE;
      hlen -= CLEN("W/"); hpos += CLEN("W/");
    }
    if (!(htlen = http__len_quoted_string(hdr, hpos, hlen)))
      return (FALSE);

    if (allow_weak || !weak)
    {
      size_t orig_epos = epos;
      size_t orig_elen = elen;
      unsigned int num = 0;
      
      while (elen)
      {
        size_t etlen = 0;

        if (vstr_export_chr(vs1, epos) == ',')
        {
          elen -= 1; epos += 1;
          HTTP_SKIP_LWS(vs1, epos, elen);
          need_comma = FALSE;
          continue;
        }
        else if (need_comma)
          return (FALSE);

        ++num;
        
        if (!VPREFIX(vs1, epos, elen, "W/"))
          weak = FALSE;
        else
        {
          weak = TRUE;
          elen -= CLEN("W/"); epos += CLEN("W/");
        }
        if (!(etlen = http__len_quoted_string(vs1, epos, elen)))
          return (FALSE);
        
        if ((allow_weak || !weak) &&
            vstr_cmp_eq(hdr, hpos, htlen, vs1, epos, etlen))
          return (TRUE);
        
        elen -= etlen; epos += etlen;
        HTTP_SKIP_LWS(vs1, epos, elen);
        need_comma = TRUE;
        
        if (req->policy->max_etag_nodes && (num >= req->policy->max_etag_nodes))
          return (FALSE);
      }

      epos = orig_epos;
      elen = orig_elen;
    }

    hlen -= htlen; hpos += htlen;
    HTTP_SKIP_LWS(hdr, hpos, hlen);
    need_comma = TRUE;
  }

  return (FALSE);
}

#define HTTPD__HD_EQ(x)                                 \
    VEQ(hdrs, h_ ## x ->pos,  h_ ## x ->len,  date)

/* gets here if the GET/HEAD response is ok, we test for caching etc. using the
 * if-* headers */
/* FALSE = 412 Precondition Failed */
static int http_response_ok(struct Con *con, struct Httpd_req_data *req,
                            unsigned int *http_ret_code,
                            const char ** http_ret_line)
{
  const Vstr_base *hdrs = con->evnt->io_r;
  time_t mtime = req->f_stat->st_mtime;
  Vstr_sect_node *h_ims  = req->http_hdrs->hdr_if_modified_since;
  Vstr_sect_node *h_ir   = req->http_hdrs->hdr_if_range;
  Vstr_sect_node *h_iums = req->http_hdrs->hdr_if_unmodified_since;
  Vstr_sect_node *h_r    = req->http_hdrs->multi->hdr_range;
  Vstr_base *comb = req->http_hdrs->multi->comb;
  Vstr_sect_node *h_im   = req->http_hdrs->multi->hdr_if_match;
  Vstr_sect_node *h_inm  = req->http_hdrs->multi->hdr_if_none_match;
  int h_ir_tst      = FALSE;
  int h_iums_tst    = FALSE;
  int req_if_range  = FALSE;
  int cached_output = FALSE;
  const char *date = NULL;
  
  if (req->ver_1_1 && h_iums->pos)
    h_iums_tst = TRUE;

  if (req->ver_1_1 && h_ir->pos && h_r->pos)
    h_ir_tst = TRUE;
  
  /* assumes time doesn't go backwards ... From rfc2616:
   *
   Note: When handling an If-Modified-Since header field, some
   servers will use an exact date comparison function, rather than a
   less-than function, for deciding whether to send a 304 (Not
   Modified) response. To get best results when sending an If-
   Modified-Since header field for cache validation, clients are
   advised to use the exact date string received in a previous Last-
   Modified header field whenever possible.
  */
  if (difftime(req->now, mtime) > 0)
  { /* if mtime in future, or now ... don't allow checking */
    Date_store *ds = httpd_opts->date;

    date = date_rfc1123(ds, mtime);
    if (h_ims->pos && !cached_output && HTTPD__HD_EQ(ims))
      cached_output = TRUE;
    if (h_iums_tst &&                   HTTPD__HD_EQ(iums))
      return (FALSE);
    if (h_ir_tst   && !req_if_range  && HTTPD__HD_EQ(ir))
      req_if_range = TRUE;
  
    date = date_rfc850(ds, mtime);
    if (h_ims->pos && !cached_output && HTTPD__HD_EQ(ims))
      cached_output = TRUE;
    if (h_iums_tst &&                   HTTPD__HD_EQ(iums))
      return (FALSE);
    if (h_ir_tst   && !req_if_range  && HTTPD__HD_EQ(ir))
      req_if_range = TRUE;
  
    date = date_asctime(ds, mtime);
    if (h_ims->pos && !cached_output && HTTPD__HD_EQ(ims))
      cached_output = TRUE;
    if (h_iums_tst &&                   HTTPD__HD_EQ(iums))
      return (FALSE);
    if (h_ir_tst   && !req_if_range  && HTTPD__HD_EQ(ir))
      req_if_range = TRUE;
  }

  if (req->ver_1_1)
  {
    const Vstr_base *vs1 = NULL;
    size_t pos = 0;
    size_t len = 0;

    if (req->content_encoding_bzip2) /* point to any entity tags we have */
    {
      if (req->bzip2_etag_vs1 && (req->etag_time > mtime))
      {
        vs1 = req->bzip2_etag_vs1;
        pos = req->bzip2_etag_pos;
        len = req->bzip2_etag_len;
      }
    }
    else if (req->content_encoding_gzip)
    {
      if (req->gzip_etag_vs1 && (req->etag_time > mtime))
      {
        vs1 = req->gzip_etag_vs1;
        pos = req->gzip_etag_pos;
        len = req->gzip_etag_len;
      }
    }
    else if (req->etag_vs1 && (req->etag_time > mtime))
    {
      vs1 = req->etag_vs1;
      pos = req->etag_pos;
      len = req->etag_len;
    }

    if (h_ir_tst   && !req_if_range &&
        httpd_match_etags(req,
                          hdrs, h_ir->pos, h_ir->len, vs1, pos, len, FALSE))
      req_if_range = TRUE;
  
    if (h_ir_tst && !req_if_range)
      h_r->pos = 0;

    /* #13.3.3 says don't trust weak for "complex" queries, ie. byteranges */
    if (h_inm->pos && (VEQ(hdrs, h_inm->pos, h_inm->len, "*") ||
                       httpd_match_etags(req, comb, h_inm->pos, h_inm->len,
                                         vs1, pos, len, !h_r->pos)))
      cached_output = TRUE;
    
    if (h_im->pos  && !(VEQ(hdrs, h_im->pos, h_im->len, "*") ||
                        httpd_match_etags(req, comb, h_im->pos, h_im->len,
                                          vs1, pos, len, !h_r->pos)))
      return (FALSE);
  }
  else if (h_ir_tst && !req_if_range)
    h_r->pos = 0;
  
  if (cached_output)
  {
    req->head_op = TRUE;
    *http_ret_code = 304;
    *http_ret_line = "Not Modified";
  }
  else if (h_r->pos)
  {
    *http_ret_code = 206;
    *http_ret_line = "Partial content";
  }

  return (TRUE);
}

static void http__multi_hdr_fixup(Vstr_sect_node *hdr_ignore,
                                  Vstr_sect_node *hdr, size_t pos, size_t len)
{
  if (hdr == hdr_ignore)
    return;
  
  if (hdr->pos <= pos)
    return;

  hdr->pos += len;
}

static int http__multi_hdr_cp(Vstr_base *comb,
                              Vstr_base *data, Vstr_sect_node *hdr)
{
  size_t pos = comb->len + 1;

  if (!hdr->len)
    return (TRUE);
  
  if (!vstr_add_vstr(comb, comb->len,
                     data, hdr->pos, hdr->len, VSTR_TYPE_ADD_BUF_PTR))
    return (FALSE);

  hdr->pos = pos;
  
  return (TRUE);
}

static int http__app_multi_hdr(Vstr_base *data, struct Http_hdrs *hdrs,
                               Vstr_sect_node *hdr, size_t pos, size_t len)
{
  Vstr_base *comb = hdrs->multi->comb;
  
  ASSERT(comb);
  
  ASSERT((hdr == hdrs->multi->hdr_accept) ||
         (hdr == hdrs->multi->hdr_accept_charset) ||
         (hdr == hdrs->multi->hdr_accept_encoding) ||
         (hdr == hdrs->multi->hdr_accept_language) ||
         (hdr == hdrs->multi->hdr_connection) ||
         (hdr == hdrs->multi->hdr_if_match) ||
         (hdr == hdrs->multi->hdr_if_none_match) ||
         (hdr == hdrs->multi->hdr_range));

  ASSERT((comb == data) || (comb == hdrs->multi->combiner_store));
  
  if ((data == comb) && !hdr->pos)
  { /* Do the fast thing... */
    hdr->pos = pos;
    hdr->len = len;
    return (TRUE);
  }

  if (data == comb)
  { /* OK, so we have a crap request and need to JOIN multiple headers... */
    comb = hdrs->multi->comb = hdrs->multi->combiner_store;
  
    if (!http__multi_hdr_cp(comb, data, hdrs->multi->hdr_accept) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_accept_charset) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_accept_encoding) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_accept_language) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_connection) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_if_match) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_if_none_match) ||
        !http__multi_hdr_cp(comb, data, hdrs->multi->hdr_range) ||
        FALSE)
      return (FALSE);
  }
  
  if (!hdr->pos)
  {
    hdr->pos = comb->len + 1;
    hdr->len = len;
    return (vstr_add_vstr(comb, comb->len,
                          data, pos, len, VSTR_TYPE_ADD_BUF_PTR));
  }

  /* reverses the order, but that doesn't matter */
  if (!vstr_add_cstr_ptr(comb, hdr->pos - 1, ","))
    return (FALSE);
  if (!vstr_add_vstr(comb, hdr->pos - 1,
                     data, pos, len, VSTR_TYPE_ADD_BUF_PTR))
    return (FALSE);
  hdr->len += ++len;

  /* now need to "move" any hdrs after this one */
  pos = hdr->pos - 1;
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_accept,          pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_accept_charset,  pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_accept_encoding, pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_accept_language, pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_connection,      pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_if_match,        pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_if_none_match,   pos, len);
  http__multi_hdr_fixup(hdr, hdrs->multi->hdr_range,           pos, len);
    
  return (TRUE);
}

/* viprefix, with local knowledge */
static int http__hdr_eq(struct Con *con, size_t pos, size_t len,
                        const char *hdr, size_t hdr_len, size_t *hdr_val_pos)
{
  Vstr_base *data = con->evnt->io_r;

  ASSERT(CLEN(hdr) == hdr_len);
  ASSERT(hdr[hdr_len - 1] == ':');
  
  if (!con->policy->use_non_spc_hdrs)
    --hdr_len;
  
  if ((len < hdr_len) ||
      !vstr_cmp_case_buf_eq(data, pos, hdr_len, hdr, hdr_len))
    return (FALSE);
  len -= hdr_len; pos += hdr_len;

  if (!con->policy->use_non_spc_hdrs)
  {
    HTTP_SKIP_LWS(data, pos, len);
    if (!len)
      return (FALSE);
  
    if (vstr_export_chr(data, pos) != ':')
      return (FALSE);
    --len; ++pos;
  }
  
  *hdr_val_pos = pos;
  
  return (TRUE);
}

/* remove LWS from front and end... what a craptastic std. */
static void http__hdr_fixup(Vstr_base *data, size_t *pos, size_t *len,
                            size_t hdr_val_pos)
{
  size_t tmp = 0;
  
  *len -= hdr_val_pos - *pos; *pos += hdr_val_pos - *pos;
  HTTP_SKIP_LWS(data, *pos, *len);

  /* hand coding for a HTTP_SKIP_LWS() going backwards... */
  while ((tmp = vstr_spn_cstr_chrs_rev(data, *pos, *len, HTTP_LWS)))
  {
    *len -= tmp;
  
    if (VSUFFIX(data, *pos, *len, HTTP_EOL))
      *len -= CLEN(HTTP_EOL);
  }  
}

/* for single headers, multiple ones aren't allowed ...
 * we can do last one wins, or just error (erroring is more secure, see
 * HTTP Request smuggling) */
#define HDR__EQ(x) http__hdr_eq(con, pos, len, x ":", CLEN(x ":"), &hdr_val_pos)
#define HDR__SET(h) do {                                                \
      if (req->policy->use_x2_hdr_chk && http_hdrs-> hdr_ ## h ->pos)   \
        HTTPD_ERR_RET(req, 400, FALSE);                                 \
      http__hdr_fixup(data, &pos, &len, hdr_val_pos);                   \
      http_hdrs-> hdr_ ## h ->pos = pos;                                \
      http_hdrs-> hdr_ ## h ->len = len;                                \
    } while (FALSE)
#define HDR__MULTI_SET(h) do {                                          \
      http__hdr_fixup(data, &pos, &len, hdr_val_pos);                   \
      if (!http__app_multi_hdr(data, http_hdrs,                         \
                               http_hdrs->multi-> hdr_ ## h, pos, len)) \
      {                                                                 \
        req->malloc_bad = TRUE;                                         \
        HTTPD_ERR_RET(req, 500, FALSE);                                 \
      }                                                                 \
    } while (FALSE)

static int http__parse_hdrs(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  struct Http_hdrs *http_hdrs = req->http_hdrs;
  unsigned int num = 3; /* skip "method URI version" */
  int got_content_length = FALSE;
  int got_transfer_encoding = FALSE;
  
  while (++num <= req->sects->num)
  {
    size_t pos = VSTR_SECTS_NUM(req->sects, num)->pos;
    size_t len = VSTR_SECTS_NUM(req->sects, num)->len;
    size_t hdr_val_pos = 0;

    if (0) { /* nothing */ }
    /* nothing headers ... use for logging only */
    else if (HDR__EQ("User-Agent"))          HDR__SET(ua);
    else if (HDR__EQ("Referer"))             HDR__SET(referer);

    else if (HDR__EQ("Expect"))              HDR__SET(expect);
    else if (HDR__EQ("Host"))                HDR__SET(host);
    else if (HDR__EQ("If-Modified-Since"))   HDR__SET(if_modified_since);
    else if (HDR__EQ("If-Range"))            HDR__SET(if_range);
    else if (HDR__EQ("If-Unmodified-Since")) HDR__SET(if_unmodified_since);
    else if (HDR__EQ("Authorization"))       HDR__SET(authorization);

    /* allow continuations over multiple headers... *sigh* */
    else if (HDR__EQ("Accept"))              HDR__MULTI_SET(accept);
    else if (HDR__EQ("Accept-Charset"))      HDR__MULTI_SET(accept_charset);
    else if (HDR__EQ("Accept-Encoding"))     HDR__MULTI_SET(accept_encoding);
    else if (HDR__EQ("Accept-Language"))     HDR__MULTI_SET(accept_language);
    else if (HDR__EQ("Connection"))          HDR__MULTI_SET(connection);
    else if (HDR__EQ("If-Match"))            HDR__MULTI_SET(if_match);
    else if (HDR__EQ("If-None-Match"))       HDR__MULTI_SET(if_none_match);
    else if (HDR__EQ("Range"))               HDR__MULTI_SET(range);

    /* allow a 0 (zero) length content-length, some clients do send these */
    else if (HDR__EQ("Content-Length"))
    {
      unsigned int num_flags = 10 | (VSTR_FLAG_PARSE_NUM_NO_BEG_PM |
                                     VSTR_FLAG_PARSE_NUM_OVERFLOW);
      size_t num_len = 0;
      
      if (req->policy->use_x2_hdr_chk && got_content_length)
        HTTPD_ERR_RET(req, 400, FALSE);
      got_content_length = TRUE;
      http__hdr_fixup(data, &pos, &len, hdr_val_pos);

      if (vstr_parse_uint(data, pos, len, num_flags, &num_len, NULL))
        HTTPD_ERR_RET(req, 413, FALSE);
      if (num_len != len)
        HTTPD_ERR_RET(req, 400, FALSE);
    }
    
    /* in theory ,,identity;foo=bar;baz="zoom",, is ok ... who cares */
    else if (HDR__EQ("Transfer-Encoding"))
    {
      if (req->policy->use_x2_hdr_chk && got_transfer_encoding)
        HTTPD_ERR_RET(req, 400, FALSE);
      got_transfer_encoding = TRUE;
      http__hdr_fixup(data, &pos, &len, hdr_val_pos);

      if (!VEQ(data, pos, len, "identity")) /* 3.6 says 501 */
        HTTPD_ERR_RET(req, 501, FALSE);
    }
    else
    {
      size_t tmp = 0;

      /* all headers _must_ contain a ':' */
      tmp = vstr_srch_chr_fwd(data, pos, len, ':');
      if (!tmp)
        HTTPD_ERR_RET(req, 400, FALSE);

      /* make sure unknown header is whitespace "valid" */
      tmp = vstr_sc_posdiff(pos, tmp);
      if (req->policy->use_non_spc_hdrs &&
          (vstr_cspn_cstr_chrs_fwd(data, pos, tmp, HTTP_LWS) != tmp))
        HTTPD_ERR_RET(req, 400, FALSE);
    }
  }

  return (TRUE);
}
#undef HDR__EQ
#undef HDR__SET
#undef HDR__MUTLI_SET

/* might have been able to do it with string matches, but getting...
 * "HTTP/1.1" = OK
 * "HTTP/1.10" = OK
 * "HTTP/1.10000000000000" = BAD
 * ...seemed not as easy. It also seems like you have to accept...
 * "HTTP / 01 . 01" as "HTTP/1.1"
 */
static int http_req_parse_version(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  size_t op_pos = VSTR_SECTS_NUM(req->sects, 3)->pos;
  size_t op_len = VSTR_SECTS_NUM(req->sects, 3)->len;
  unsigned int major = 0;
  unsigned int minor = 0;
  size_t num_len = 0;
  unsigned int num_flags = 10 | (VSTR_FLAG_PARSE_NUM_NO_BEG_PM |
                                 VSTR_FLAG_PARSE_NUM_OVERFLOW);
  
  if (!VPREFIX(data, op_pos, op_len, "HTTP"))
    HTTPD_ERR_RET(req, 400, FALSE);

  op_len -= CLEN("HTTP"); op_pos += CLEN("HTTP");
  HTTP_SKIP_LWS(data, op_pos, op_len);
  
  if (!VPREFIX(data, op_pos, op_len, "/"))
    HTTPD_ERR_RET(req, 400, FALSE);
  op_len -= CLEN("/"); op_pos += CLEN("/");
  HTTP_SKIP_LWS(data, op_pos, op_len);
  
  major = vstr_parse_uint(data, op_pos, op_len, num_flags, &num_len, NULL);
  op_len -= num_len; op_pos += num_len;
  HTTP_SKIP_LWS(data, op_pos, op_len);
  
  if (!num_len || !VPREFIX(data, op_pos, op_len, "."))
    HTTPD_ERR_RET(req, 400, FALSE);

  op_len -= CLEN("."); op_pos += CLEN(".");
  HTTP_SKIP_LWS(data, op_pos, op_len);
  
  minor = vstr_parse_uint(data, op_pos, op_len, num_flags, &num_len, NULL);
  op_len -= num_len; op_pos += num_len;
  HTTP_SKIP_LWS(data, op_pos, op_len);
  
  if (!num_len || op_len)
    HTTPD_ERR_RET(req, 400, FALSE);
  
  if (0) { } /* not allowing HTTP/0.9 here */
  else if ((major == 1) && (minor >= 1))
    req->ver_1_1 = TRUE;
  else if ((major == 1) && (minor == 0))
  { /* do nothing */ }
  else
    HTTPD_ERR_RET(req, 505, FALSE);
        
  return (TRUE);
}

#define HDR__CON_1_0_FIXUP(name, h)                                     \
    else if (VIEQ(data, pos, tmp, name))                                \
      do {                                                              \
        req -> http_hdrs -> hdr_ ## h ->pos = 0;                        \
        req -> http_hdrs -> hdr_ ## h ->len = 0;                        \
      } while (FALSE)
#define HDR__CON_1_0_MULTI_FIXUP(name, h)                               \
    else if (VIEQ(data, pos, tmp, name))                                \
      do {                                                              \
        req -> http_hdrs -> multi -> hdr_ ## h ->pos = 0;               \
        req -> http_hdrs -> multi -> hdr_ ## h ->len = 0;               \
      } while (FALSE)
static void http__parse_connection(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *data = req->http_hdrs->multi->comb;
  size_t pos = 0;
  size_t len = 0;
  unsigned int num = 0;
  
  pos = req->http_hdrs->multi->hdr_connection->pos;
  len = req->http_hdrs->multi->hdr_connection->len;

  if (req->ver_1_1)
    con->keep_alive = HTTP_1_1_KEEP_ALIVE;

  if (!len)
    return;

  while (len)
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len,
                                         HTTP_EOL HTTP_LWS ",");

    ++num;
    if (req->ver_1_1)
    { /* this is all we have to do for HTTP/1.1 ... proxies understand it */
      if (VIEQ(data, pos, tmp, "close"))
        con->keep_alive = HTTP_NON_KEEP_ALIVE;
    }
    else if (VIEQ(data, pos, tmp, "keep-alive"))
    {
      if (req->policy->use_keep_alive_1_0)
        con->keep_alive = HTTP_1_0_KEEP_ALIVE;
    }
    /* now fixup connection headers for HTTP/1.0 proxies */
    HDR__CON_1_0_FIXUP("User-Agent",          ua);
    HDR__CON_1_0_FIXUP("Referer",             referer);
    HDR__CON_1_0_FIXUP("Expect",              expect);
    HDR__CON_1_0_FIXUP("Host",                host);
    HDR__CON_1_0_FIXUP("If-Modified-Since",   if_modified_since);
    HDR__CON_1_0_FIXUP("If-Range",            if_range);
    HDR__CON_1_0_FIXUP("If-Unmodified-Since", if_unmodified_since);
    HDR__CON_1_0_FIXUP("Authorization",       authorization);
    
    HDR__CON_1_0_MULTI_FIXUP("Accept",          accept);
    HDR__CON_1_0_MULTI_FIXUP("Accept-Charset",  accept_charset);
    HDR__CON_1_0_MULTI_FIXUP("Accept-Encoding", accept_encoding);
    HDR__CON_1_0_MULTI_FIXUP("Accept-Language", accept_language);
    HDR__CON_1_0_MULTI_FIXUP("If-Match",        if_match);
    HDR__CON_1_0_MULTI_FIXUP("If-None-Match",   if_none_match);
    HDR__CON_1_0_MULTI_FIXUP("Range",           range);

    /* skip to end, or after next ',' */
    tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len, ",");    
    len -= tmp; pos += tmp;
    if (!len)
      break;
    
    assert(VPREFIX(data, pos, len, ","));
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);

    if (req->policy->max_connection_nodes &&
        (num >= req->policy->max_connection_nodes))
      return;
  }
}
#undef HDR__CON_1_0_FIXUP
#undef HDR__CON_1_0_MULTI_FIXUP
                                  
/* parse >= 1.0 things like, version and headers */
static int http__parse_1_x(struct Con *con, struct Httpd_req_data *req)
{
  ASSERT(!req->ver_0_9);
  
  if (!http_req_parse_version(con, req))
    return (FALSE);
  
  if (!http__parse_hdrs(con, req))
    return (FALSE);

  if (req->policy->max_requests &&
      (req->policy->max_requests <= con->evnt->acct.req_got) && req->ver_1_1)
    return (TRUE);

  if (!req->policy->use_keep_alive && req->ver_1_1)
    return (TRUE);

  http__parse_connection(con, req);
  
  if (req->policy->max_requests &&
      (req->policy->max_requests <= con->evnt->acct.req_got))
    con->keep_alive = HTTP_NON_KEEP_ALIVE;

  return (TRUE);
}

/* because we only parse for a combined CRLF, and some proxies/clients parse for
 * either ... make sure we don't have embedded singles which could cause
 * response splitting */
static int http__chk_single_crlf(Vstr_base *data, size_t pos, size_t len)
{
  if (vstr_srch_chr_fwd(data, pos, len, '\r') ||
      vstr_srch_chr_fwd(data, pos, len, '\n'))
    return (TRUE);

  return (FALSE);
}

/* convert a http://abcd/foo into /foo with host=abcd ...
 * also do sanity checking on the URI and host for valid characters */
static int http_parse_host(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  size_t op_pos = req->path_pos;
  size_t op_len = req->path_len;
  
  /* check for absolute URIs */
  if (VIPREFIX(data, op_pos, op_len, "http://"))
  { /* ok, be forward compatible */
    size_t tmp = CLEN("http://");
    
    op_len -= tmp; op_pos += tmp;
    tmp = vstr_srch_chr_fwd(data, op_pos, op_len, '/');
    if (!tmp)
    {
      HTTP__HDR_SET(req, host, op_pos, op_len);
      op_len = 1;
      --op_pos;
    }
    else
    { /* found end of host ... */
      size_t host_len = tmp - op_pos;
      
      HTTP__HDR_SET(req, host, op_pos, host_len);
      op_len -= host_len; op_pos += host_len;
    }
    assert(VPREFIX(data, op_pos, op_len, "/"));
  }

  /* HTTP/1.1 requires a host -- allow blank hostnames */
  if (req->ver_1_1 && !req->http_hdrs->hdr_host->pos)
    return (FALSE);
  
  if (req->http_hdrs->hdr_host->len)
  { /* check host looks valid ... header must exist, but can be empty */
    size_t pos = req->http_hdrs->hdr_host->pos;
    size_t len = req->http_hdrs->hdr_host->len;
    size_t tmp = 0;

    /* leaving out most checks for ".." or invalid chars in hostnames etc.
       as the default filename checks should catch them
     */

    /*  Check for Host header with extra / ...
     * Ie. only allow a single directory name.
     *  We could just leave this (it's not a security check, /../ is checked
     * for at filepath time), but I feel like being anal and this way there
     * aren't multiple urls to a single path. */
    if (vstr_srch_chr_fwd(data, pos, len, '/'))
      return (FALSE);

    if (http__chk_single_crlf(data, pos, len))
      return (FALSE);

    if ((tmp = vstr_srch_chr_fwd(data, pos, len, ':')))
    { /* NOTE: not sure if we have to 400 if the port doesn't match
       * or if it's an "invalid" port number (Ie. == 0 || > 65535) */
      len -= tmp - pos; pos = tmp;

      /* if it's port 80, pretend it's not there */
      if (VEQ(data, pos, len, ":80") || VEQ(data, pos, len, ":"))
        req->http_hdrs->hdr_host->len -= len;
      else
      {
        len -= 1; pos += 1; /* skip the ':' */
        if (vstr_spn_cstr_chrs_fwd(data, pos, len, "0123456789") != len)
          return (FALSE);
      }
    }
  }

  if (http__chk_single_crlf(data, op_pos, op_len))
    return (FALSE);
  
  /* uri#fragment ... craptastic clients pass this and assume it is ignored */
  if (req->policy->remove_url_frag)
    op_len = vstr_cspn_cstr_chrs_fwd(data, op_pos, op_len, "#");
  /* uri?foo ... This is "ok" to pass, however if you move dynamic
   * resources to static ones you need to do this */
  if (req->policy->remove_url_query)
    op_len = vstr_cspn_cstr_chrs_fwd(data, op_pos, op_len, "?");
  
  req->path_pos = op_pos;
  req->path_len = op_len;
  
  return (TRUE);
}

static void http__parse_skip_blanks(Vstr_base *data,
                                    size_t *passed_pos, size_t *passed_len)
{
  size_t pos = *passed_pos;
  size_t len = *passed_len;
  
  HTTP_SKIP_LWS(data, pos, len);
  while (VPREFIX(data, pos, len, ",")) /* http crack */
  {
    len -= CLEN(","); pos += CLEN(",");
    HTTP_SKIP_LWS(data, pos, len);
  }

  *passed_pos = pos;
  *passed_len = len;
}

static int httpd__file_sect_add(struct Con *con, Httpd_req_data *req,
                                VSTR_AUTOCONF_uintmax_t range_beg,
                                VSTR_AUTOCONF_uintmax_t range_end, size_t len)
{
  struct File_sect *fs = NULL;
    
  ASSERT(con->fs && (con->fs_sz >= 1));
  if (!con->fs_num)
  {
    ASSERT((con->fs == con->fs_store) || (con->fs_sz > 1));
    ASSERT(!con->use_mpbr);

    goto file_sect_add;
  }

  con->use_mpbr = TRUE;
  
  if (con->fs == con->fs_store)
  {
    ASSERT(con->fs_num == 1);

    if (!(con->mpbr_ct = vstr_make_base(con->evnt->io_w->conf)))
      return (FALSE);
    
    if (!(fs = MK(sizeof(struct File_sect) * 16)))
      return (FALSE);
    con->fs    = fs;
    con->fs_sz = 16;
    
    con->fs->fd  = con->fs_store->fd;
    con->fs->off = con->fs_store->off;
    con->fs->len = con->fs_store->len;
    ++fs;
  }
  else if (con->fs_num >= con->fs_sz)
  {
    unsigned int num = (con->fs_sz << 1) + 1;
    
    ASSERT(con->fs_num == con->fs_sz);
  
    if (!MV(con->fs, fs, sizeof(struct File_sect) * num))
      return (FALSE);
    con->fs_sz = num;
  }

 file_sect_add:
  fs = con->fs + con->fs_num++;

  fs->fd  = con->fs->fd; /* copy to each one */
  fs->off = range_beg;
  fs->len = (range_end - range_beg) + 1;

  return (!len || (req->policy->max_range_nodes > con->fs_num));
}

/* Allow...
   bytes=NUM-NUM
   bytes=-NUM
   bytes=NUM-
   ...and due to LWS, http crapola parsing, even...
   bytes = , , NUM - NUM , ,
   ...allowing ability to disable multiple ranges at once, due to
   multipart/byteranges being too much crack, I think this is stds. compliant.
 */
static int http_parse_range(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *data     = req->http_hdrs->multi->comb;
  Vstr_sect_node *h_r = req->http_hdrs->multi->hdr_range;
  size_t pos = h_r->pos;
  size_t len = h_r->len;
  VSTR_AUTOCONF_uintmax_t fsize = req->f_stat->st_size;
  unsigned int num_flags = 10 | (VSTR_FLAG_PARSE_NUM_NO_BEG_PM |
                                 VSTR_FLAG_PARSE_NUM_OVERFLOW);
  size_t num_len = 0;
  
  if (!VPREFIX(data, pos, len, "bytes"))
    return (0);
  len -= CLEN("bytes"); pos += CLEN("bytes");

  HTTP_SKIP_LWS(data, pos, len);
  
  if (!VPREFIX(data, pos, len, "="))
    return (0);
  len -= CLEN("="); pos += CLEN("=");

  http__parse_skip_blanks(data, &pos, &len);
  
  while (len)
  {
    VSTR_AUTOCONF_uintmax_t range_beg = 0;
    VSTR_AUTOCONF_uintmax_t range_end = 0;
    
    if (VPREFIX(data, pos, len, "-"))
    { /* num bytes at end */
      VSTR_AUTOCONF_uintmax_t tmp = 0;

      len -= CLEN("-"); pos += CLEN("-");
      HTTP_SKIP_LWS(data, pos, len);

      tmp = vstr_parse_uintmax(data, pos, len, num_flags, &num_len, NULL);
      len -= num_len; pos += num_len;
      if (!num_len)
        return (0);

      if (!tmp)
        return (416);
    
      if (tmp >= fsize)
        return (0);
    
      range_beg = fsize - tmp;
      range_end = fsize - 1;
    }
    else
    { /* offset - [end] */
      range_beg = vstr_parse_uintmax(data, pos, len, num_flags, &num_len, NULL);
      len -= num_len; pos += num_len;
      HTTP_SKIP_LWS(data, pos, len);
    
      if (!VPREFIX(data, pos, len, "-"))
        return (0);
      len -= CLEN("-"); pos += CLEN("-");
      HTTP_SKIP_LWS(data, pos, len);

      if (!len || VPREFIX(data, pos, len, ","))
        range_end = fsize - 1;
      else
      {
        range_end = vstr_parse_uintmax(data, pos, len, num_flags, &num_len, 0);
        len -= num_len; pos += num_len;
        if (!num_len)
          return (0);
      
        if (range_end >= fsize)
          range_end = fsize - 1;
      }
    
      if ((range_beg >= fsize) || (range_beg > range_end))
        return (416);
    
      if ((range_beg == 0) && 
          (range_end == (fsize - 1)))
        return (0);
    }
  
    http__parse_skip_blanks(data, &pos, &len);

    if (!httpd__file_sect_add(con, req, range_beg, range_end, len))
      return (0); /* after all that, ignore if there is more than one range */
  }

  return (200);
}

static void httpd_serv_file_sects_none(struct Con *con, Httpd_req_data *req)
{
  con->use_mpbr = FALSE;
  con->fs_num = 1;
  con->fs->off = 0;
  con->fs->len = req->f_stat->st_size;
}
                                      
static void httpd_serv_call_file_init(struct Con *con, Httpd_req_data *req,
                                      unsigned int *http_ret_code,
                                      const char ** http_ret_line)
{
  ASSERT(req);
  
  con->use_mmap = FALSE;
  if (!req->head_op)
  {
    httpd_serv_call_mmap(con, req, con->fs);
    httpd_serv_call_seek(con, req, con->fs, http_ret_code, http_ret_line);
  }
}

static int http_req_1_x(struct Con *con, Httpd_req_data *req,
                        unsigned int *http_ret_code,
                        const char **http_ret_line)
{
  Vstr_base *out = con->evnt->io_w;
  Vstr_sect_node *h_r = req->http_hdrs->multi->hdr_range;
  time_t mtime = -1;
  
  if (req->ver_1_1 && req->http_hdrs->hdr_expect->len)
    /* I'm pretty sure we can ignore 100-continue, as no request will
     * have a body */
    HTTPD_ERR_RET(req, 417, FALSE);
          
  httpd__try_fd_encoding(con, req, req->f_stat, req->fname);
  
  if (req->policy->use_err_406 &&
      !req->content_encoding_identity &&
      !req->content_encoding_bzip2 && !req->content_encoding_gzip)
    HTTPD_ERR_RET(req, 406, FALSE);

  if (h_r->pos)
  {
    int ret_code = 0;

    if (!(req->policy->use_range &&
	  (req->ver_1_1 || req->policy->use_range_1_0)))
      h_r->pos = 0;
    else if (!(ret_code = http_parse_range(con, req)))
      h_r->pos = 0;
    ASSERT(!ret_code || (ret_code == 200) || (ret_code == 416));
    if (ret_code == 416)
    {
      if (!req->http_hdrs->hdr_if_range->pos)
        HTTPD_ERR_RET(req, 416, FALSE);
      h_r->pos = 0;
    }
  }
  
  if (!http_response_ok(con, req, http_ret_code, http_ret_line))
    HTTPD_ERR_RET(req, 412, FALSE);

  if (!h_r->pos)
    httpd_serv_file_sects_none(con, req);
  
  httpd_serv_call_file_init(con, req, http_ret_code, http_ret_line);

  ASSERT(con->fs && (con->fs_off < con->fs_num) && (con->fs_num <= con->fs_sz));
  ASSERT(!con->fs_off);

  mtime = req->f_stat->st_mtime;
  http_app_def_hdrs(con, req, *http_ret_code, *http_ret_line,
                    mtime, NULL, TRUE, con->fs->len);
  if (h_r->pos && !con->use_mpbr)
    http_app_hdr_fmt(out, "Content-Range", "%s %ju-%ju/%ju", "bytes",
                     con->fs->off, con->fs->off + (con->fs->len - 1),
                     (VSTR_AUTOCONF_uintmax_t)req->f_stat->st_size);
  if (req->content_location_vs1)
    http_app_hdr_vstr_def(out, "Content-Location",
                          HTTP__XTRA_HDR_PARAMS(req, content_location));
  http_app_hdrs_url(con, req);
  http_app_hdrs_file(con, req);  
  
  http_app_end_hdrs(out);

  if (req->head_op)
    con->use_mpbr = FALSE;
  else if (h_r->pos && con->use_mpbr)
  {
    con->mpbr_fs_len = req->f_stat->st_size;
    http_app_hdrs_mpbr(con, con->fs);
  }
  
  return (TRUE);
}

/* skip, or fail on syntax error */
static int http__skip_parameters(Vstr_base *data, size_t *pos, size_t *len)
{
  while (*len && (vstr_export_chr(data, *pos) != ','))
  { /* skip parameters */
    size_t tmp = 0;
    
    if (vstr_export_chr(data, *pos) != ';')
      return (FALSE); /* syntax error */
    
    *len -= 1; *pos += 1;
    HTTP_SKIP_LWS(data, *pos, *len);
    tmp = vstr_cspn_cstr_chrs_fwd(data, *pos, *len, ";,=");
    *len -= tmp; *pos += tmp;
    if (!*len)
      break;
    
    switch (vstr_export_chr(data, *pos))
    {
      case ';': break;
      case ',': break;
        
      case '=': /* skip parameter value */
        *len -= 1; *pos += 1;
        HTTP_SKIP_LWS(data, *pos, *len);
        if (!*len)
          return (FALSE); /* syntax error */
        if (vstr_export_chr(data, *pos) == '"')
        {
          if (!http__skip_quoted_string(data, pos, len))
            return (FALSE); /* syntax error */
        }
        else
        {
          tmp = vstr_cspn_cstr_chrs_fwd(data, *pos, *len, ";,");
          *len -= tmp; *pos += tmp;
        }
        break;
    }
  }

  return (TRUE);
}

/* returns quality of the passed content-type in the "Accept:" header,
 * if it isn't there or we get a syntax error we return 1001 for not avilable */
unsigned int http_parse_accept(Httpd_req_data *req,
                               const Vstr_base *ct_vs1,
                               size_t ct_pos, size_t ct_len)
{
  Vstr_base *data = req->http_hdrs->multi->comb;
  size_t pos = 0;
  size_t len = 0;
  unsigned int num = 0;
  unsigned int quality = 1001;
  unsigned int dummy   = 1001;
  int done_sub_type = FALSE;
  size_t ct_sub_len = 0;
  
  pos = req->http_hdrs->multi->hdr_accept->pos;
  len = req->http_hdrs->multi->hdr_accept->len;
  
  if (!len) /* no accept == accept all */
    return (1000);

  ASSERT(ct_vs1);
    
  if (!(ct_sub_len = vstr_srch_chr_fwd(ct_vs1, ct_pos, ct_len, '/')))
  { /* it's too weird, blank it */
    if (ct_vs1 == req->content_type_vs1)
      req->content_type_vs1 = NULL;
    return (1);
  }
  ct_sub_len = vstr_sc_posdiff(ct_pos, ct_sub_len);
  
  while (len)
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len,
                                         HTTP_EOL HTTP_LWS ";,");

    ++num;
    
    if (0) { }
    else if (vstr_cmp_eq(data, pos, tmp, ct_vs1, ct_pos, ct_len))
    { /* full match */
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, TRUE, &quality))
        return (1);
      return (quality);
    }
    else if ((tmp == (ct_sub_len + 1)) &&
             vstr_cmp_eq(data, pos, ct_sub_len, ct_vs1, ct_pos, ct_sub_len) &&
             (vstr_export_chr(data, vstr_sc_poslast(pos, tmp)) == '*'))
    { /* sub match */
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, TRUE, &quality))
        return (1);
      done_sub_type = TRUE;
    }
    else if (!done_sub_type && VEQ(data, pos, tmp, "*/*"))
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, TRUE, &quality))
        return (1);
    }
    else
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, TRUE, &dummy))
        return (1);
    }

    if (!http__skip_parameters(data, &pos, &len))
      return (1);
    if (!len)
      break;
    
    assert(VPREFIX(data, pos, len, ","));
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);
    
    if (req->policy->max_A_nodes && (num >= req->policy->max_A_nodes))
      return (1);
  }

  ASSERT(quality <= 1001);
  if (quality == 1001)
    return (0);
  
  return (quality);
}

static int http__cmp_lang_eq(const Vstr_base *s1, size_t p1, size_t l1,
                             const Vstr_base *s2, size_t p2, size_t l2)
{
  if (l1 == l2)
    return (FALSE);
  
  if (l1 > l2)
    return ((vstr_export_chr(s1, p1 + l2) == '-') &&
            vstr_cmp_case_eq(s1, p1, l2, s2, p2, l2));

  return ((vstr_export_chr(s2, p2 + l1) == '-') &&
          vstr_cmp_case_eq(s1, p1, l1, s2, p2, l1));
}

unsigned int http_parse_accept_language(Httpd_req_data *req,
                                        const Vstr_base *ct_vs1,
                                        size_t ct_pos, size_t ct_len)
{
  Vstr_base *data = req->http_hdrs->multi->comb;
  size_t pos = 0;
  size_t len = 0;
  unsigned int num = 0;
  unsigned int quality = 1001;
  unsigned int dummy   = 1001;
  size_t done_sub_type = 0;
  
  pos = req->http_hdrs->multi->hdr_accept_language->pos;
  len = req->http_hdrs->multi->hdr_accept_language->len;
  
  if (!len) /* no accept-language == accept all */
    return (1000);

  ASSERT(ct_vs1);
  while (len)
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(data, pos, len,
                                         HTTP_EOL HTTP_LWS ";,");

    ++num;
    
    if (0) { }
    else if (vstr_cmp_case_eq(data, pos, tmp, ct_vs1, ct_pos, ct_len))
    { /* full match */
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &quality))
        return (1);
      return (quality);
    }
    else if ((tmp >= done_sub_type) &&
             http__cmp_lang_eq(data, pos, tmp, ct_vs1, ct_pos, ct_len))
    { /* sub match - can be x-y-A;q=0, x-y-B, x-y
         rfc2616#14.4
         The language quality factor assigned to a language-tag by the
         Accept-Language field is the quality value of the longest
         language-range in the field that matches the language-tag.
      */
      unsigned int sub_type_qual = 0;
      
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &sub_type_qual))
        return (1);
      
      ASSERT(sub_type_qual <= 1000);
      if ((tmp > done_sub_type) || (sub_type_qual > quality))
        quality = sub_type_qual;
      
      done_sub_type = tmp;
    }
    else if (!done_sub_type && VEQ(data, pos, tmp, "*"))
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &quality))
        return (1);
    }
    else
    {
      len -= tmp; pos += tmp;
      if (!http_parse_quality(data, &pos, &len, FALSE, &dummy))
        return (1);
    }
    
    if (!len)
      break;
    
    assert(VPREFIX(data, pos, len, ","));
    len -= 1; pos += 1;
    HTTP_SKIP_LWS(data, pos, len);
    
    if (req->policy->max_AL_nodes && (num >= req->policy->max_AL_nodes))
      return (1);
  }

  ASSERT(quality <= 1001);
  if (quality == 1001)
    return (0);
  
  return (quality);
}

/* Lookup content type for filename, If this lookup "fails" it still returns
 * the default content-type. So we just have to determine if we want to use
 * it or not. Can also return "content-types" like /404/ which returns a 404
 * error for the request */
int http_req_content_type(Httpd_req_data *req)
{
  const Vstr_base *vs1 = NULL;
  size_t     pos = 0;
  size_t     len = 0;

  if (req->content_type_vs1) /* manually set */
    return (TRUE);

  mime_types_match(req->policy->mime_types,
                   req->fname, 1, req->fname->len, &vs1, &pos, &len);
  if (!len)
  {
    req->parse_accept = FALSE;
    return (TRUE);
  }
  
  if ((vstr_export_chr(vs1, pos) == '/') && (len > 2) &&
      (vstr_export_chr(vs1, vstr_sc_poslast(pos, len)) == '/'))
  {
    size_t num_len = 1;

    len -= 2;
    ++pos;
    req->user_return_error_code = TRUE;
    req->direct_filename = FALSE;
    switch (vstr_parse_uint(vs1, pos, len, 0, &num_len, NULL))
    {
      case 400: if (num_len == len) HTTPD_ERR_RET(req, 400, FALSE);
      case 403: if (num_len == len) HTTPD_ERR_RET(req, 403, FALSE);
      case 404: if (num_len == len) HTTPD_ERR_RET(req, 404, FALSE);
      case 410: if (num_len == len) HTTPD_ERR_RET(req, 410, FALSE);
      case 500: if (num_len == len) HTTPD_ERR_RET(req, 500, FALSE);
      case 503: if (num_len == len) HTTPD_ERR_RET(req, 503, FALSE);
        
      default: /* just ignore any other content */
        req->user_return_error_code = FALSE;
        return (TRUE);
    }
  }

  req->content_type_vs1 = vs1;
  req->content_type_pos = pos;
  req->content_type_len = len;

  return (TRUE);
}

static int http__policy_req(struct Con *con, Httpd_req_data *req)
{
  if (!httpd_policy_request(con, req,
                            httpd_opts->conf, httpd_opts->match_request))
  {
    Vstr_base *s1 = httpd_opts->conf->tmp;
    if (!req->user_return_error_code)
      vlg_info(vlg, "CONF-MATCH-REQ-ERR from[$<sa:%p>]:"
               " backtrace: $<vstr.all:%p>\n", CON_CEVNT_SA(con), s1);
    return (TRUE);
  }
  
  if (con->evnt->flag_q_closed)
  {
    Vstr_base *s1 = req->policy->s->policy_name;
    
    vlg_info(vlg, "BLOCKED from[$<sa:%p>]: policy $<vstr.all:%p>\n",
             CON_CEVNT_SA(con), s1);
    return (FALSE);
  }

  if (req->direct_uri)
  {
    Vstr_base *s1 = req->policy->s->policy_name;
    vlg_info(vlg, "CONF-MATCH-REQ-ERR from[$<sa:%p>]: policy $<vstr.all:%p>"
             " Has URI.\n", CON_CEVNT_SA(con), s1);
    HTTPD_ERR_RET(req, 503, TRUE);
  }
  
  if (req->policy->auth_token->len) /* they need rfc2617 auth */
  {
    Vstr_base *data = con->evnt->io_r;
    size_t pos = req->http_hdrs->hdr_authorization->pos;
    size_t len = req->http_hdrs->hdr_authorization->len;
    Vstr_base *auth_token = req->policy->auth_token;
    int auth_ok = TRUE;
    
    if (!VIPREFIX(data, pos, len, "Basic"))
      auth_ok = FALSE;
    else
    {
      len -= CLEN("Basic"); pos += CLEN("Basic");
      HTTP_SKIP_LWS(data, pos, len);
      if (!vstr_cmp_eq(data, pos, len, auth_token, 1, auth_token->len))
        auth_ok = FALSE;
    }
    
    if (!auth_ok)
    {
      req->user_return_error_code = FALSE;
      HTTPD_ERR(req, 401);
    }
  }
  
  return (TRUE);
}

int http_req_op_get(struct Con *con, Httpd_req_data *req)
{ /* GET or HEAD ops */
  Vstr_base *data = con->evnt->io_r;
  Vstr_base *out  = con->evnt->io_w;
  Vstr_base *fname = req->fname;
  const char *fname_cstr = NULL;
  unsigned int http_ret_code = 200;
  const char * http_ret_line = "OK";
  
  if (fname->conf->malloc_bad)
    goto malloc_err;

  assert(VPREFIX(fname, 1, fname->len, "/"));

  /* final act of vengance, before policy */
  if (vstr_srch_cstr_buf_fwd(data, req->path_pos, req->path_len, "//"))
  { /* in theory we can skip this if there is no policy */
    vstr_sub_vstr(fname, 1, fname->len, data, req->path_pos, req->path_len, 0);
    if (!httpd_canon_path(fname))
      goto malloc_err;
    httpd_req_absolute_uri(con, req, fname, 1, fname->len);
    HTTPD_ERR_301(req);
  
    return (http_fin_err_req(con, req));
  }
  
  if (!http__policy_req(con, req))
    return (FALSE);
  if (req->error_code)
    return (http_fin_err_req(con, req));

  httpd_sc_add_default_filename(req, fname);
  
  if (!http__conf_req(con, req))
    return (FALSE);
  if (req->error_code)
    return (http_fin_err_req(con, req));
    
  if (!http_req_content_type(req))
    return (http_fin_err_req(con, req));

  /* don't change vary_a/vary_al just because of 406 */
  if (req->parse_accept)
  {
    if (!http_parse_accept(req, HTTP__XTRA_HDR_PARAMS(req, content_type)))
      HTTPD_ERR_RET(req, 406, http_fin_err_req(con, req));
  }
  if (!req->content_language_vs1 || !req->content_language_len)
    req->parse_accept_language = FALSE;
  if (req->parse_accept_language)
  {
    if (!http_parse_accept_language(req,
                                    HTTP__XTRA_HDR_PARAMS(req,
                                                          content_language)))
      HTTPD_ERR_RET(req, 406, http_fin_err_req(con, req));
  }
  
  /* Add the document root now, this must be at least . so
   * "///foo" becomes ".///foo" ... this is done now
   * so nothing has to deal with document_root values */
  if (!req->skip_document_root)
    http_prepend_doc_root(fname, req);
  
  fname_cstr = vstr_export_cstr_ptr(fname, 1, fname->len);
  
  if (fname->conf->malloc_bad)
    goto malloc_err;

  ASSERT(con->fs && !con->fs_num && !con->fs_off && (con->fs->fd == -1));
  if ((con->fs->fd = io_open_nonblock(fname_cstr)) == -1)
  {
    if (0) { }
    else if (req->direct_filename && (errno == EISDIR)) /* don't allow */
      HTTPD_ERR(req, 404);
    else if (errno == EISDIR)
      return (http_req_chk_dir(con, req));
    else if (((errno == ENOENT) && req->conf_friendly_dirs) ||
             (errno == ENOTDIR) || /* part of path was not a dir */
             (errno == ENAMETOOLONG) || /* 414 ? */
             FALSE)
      return (http_req_chk_file(con, req));
    else if (errno == EACCES)
      HTTPD_ERR(req, 403);
    else if ((errno == ENOENT) ||
             (errno == ENODEV) || /* device file, with no driver */
             (errno == ENXIO) || /* device file, with no driver */
             (errno == ELOOP) || /* symlinks */
             FALSE)
      HTTPD_ERR(req, 404);
    else
      HTTPD_ERR(req, 500);
    
    return (http_fin_err_req(con, req));
  }
  if (fstat64(con->fs->fd, req->f_stat) == -1)
    HTTPD_ERR_RET(req, 500, http_fin_err_close_req(con, req));
  if (req->policy->use_public_only && !(req->f_stat->st_mode & S_IROTH))
    HTTPD_ERR_RET(req, 403, http_fin_err_close_req(con, req));
  
  if (S_ISDIR(req->f_stat->st_mode))
  {
    if (req->direct_filename) /* don't allow */
      HTTPD_ERR_RET(req, 404, http_fin_err_close_req(con, req));
    httpd_fin_fd_close(con);
    return (http_req_chk_dir(con, req));
  }
  if (!S_ISREG(req->f_stat->st_mode))
    HTTPD_ERR_RET(req, 403, http_fin_err_close_req(con, req));
  
  con->fs->len = req->f_stat->st_size;
  
  if (req->ver_0_9)
  {
    httpd_serv_file_sects_none(con, req);
    httpd_serv_call_file_init(con, req, &http_ret_code, &http_ret_line);
    http_ret_line = "OK - HTTP/0.9";
  }
  else if (!http_req_1_x(con, req, &http_ret_code, &http_ret_line))
    return (http_fin_err_close_req(con, req));
  
  if (out->conf->malloc_bad)
    goto malloc_close_err;
  
  vlg_dbg3(vlg, "REPLY:\n$<vstr.all:%p>\n", out);
  
  if (con->use_mmap && !vstr_mov(con->evnt->io_w, con->evnt->io_w->len,
                                 req->f_mmap, 1, req->f_mmap->len))
    goto malloc_close_err;

  /* req->head_op is set for 304 returns */
  vlg_info(vlg, "REQ $<vstr.sect:%p%p%u> from[$<sa:%p>] ret[%03u %s]"
           " sz[${BKMG.ju:%ju}:%ju]", data, req->sects, 1U, CON_CEVNT_SA(con),
           http_ret_code, http_ret_line, con->fs->len, con->fs->len);
  http_vlg_def(con, req);
  
  return (http_fin_fd_req(con, req));
  
 malloc_close_err:
  httpd_fin_fd_close(con);
 malloc_err:
  VLG_WARNNOMEM_RET(http_fin_errmem_req(con, req), (vlg, "op_get(): %m\n"));
}

int http_req_op_opts(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *out = con->evnt->io_w;
  Vstr_base *fname = req->fname;
  VSTR_AUTOCONF_uintmax_t tmp = 0;

  if (fname->conf->malloc_bad)
    goto malloc_err;
  
  assert(VPREFIX(fname, 1, fname->len, "/") ||
         !req->policy->use_vhosts_name ||
         !req->policy->use_host_err_chk ||
	 !req->policy->use_host_err_400 ||
         VEQ(con->evnt->io_r, req->path_pos, req->path_len, "*"));
  
  /* apache doesn't test for 404's here ... which seems weird */
  
  http_app_def_hdrs(con, req, 200, "OK", 0, NULL, TRUE, 0);
  HTTP_APP_HDR_CONST_CSTR(out, "Allow", "GET, HEAD, OPTIONS, TRACE");
  http_app_end_hdrs(out);
  if (out->conf->malloc_bad)
    goto malloc_err;
  
  vlg_info(vlg, "REQ %s from[$<sa:%p>] ret[%03u %s] sz[${BKMG.ju:%ju}:%ju]",
           "OPTIONS", CON_CEVNT_SA(con), 200, "OK", tmp, tmp);
  http_vlg_def(con, req);
  
  return (http_fin_req(con, req));
  
 malloc_err:
  VLG_WARNNOMEM_RET(http_fin_errmem_req(con, req), (vlg, "op_opts(): %m\n"));
}

int http_req_op_trace(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  Vstr_base *out  = con->evnt->io_w;
  VSTR_AUTOCONF_uintmax_t tmp = 0;
      
  http_app_def_hdrs(con, req, 200, "OK", req->now,
                    "message/http", FALSE, req->len);
  http_app_end_hdrs(out);
  vstr_add_vstr(out, out->len, data, 1, req->len, VSTR_TYPE_ADD_DEF);
  if (out->conf->malloc_bad)
    VLG_WARNNOMEM_RET(http_fin_errmem_req(con, req), (vlg, "op_trace(): %m\n"));

  tmp = req->len;
  vlg_info(vlg, "REQ %s from[$<sa:%p>] ret[%03u %s] sz[${BKMG.ju:%ju}:%ju]",
           "TRACE", CON_CEVNT_SA(con), 200, "OK", tmp, tmp);
  http_vlg_def(con, req);
      
  return (http_fin_req(con, req));
}

/* characters that are valid in a part of a URL _and_ in a file basename ...
 * without encoding */
#define HTTPD__VALID_CSTR_CHRS_URL_FILENAME \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"            \
    "abcdefghijklmnopqrstuvwxyz"            \
    "0123456789"                            \
    ":.-_~"
int httpd_valid_url_filename(Vstr_base *s1, size_t pos, size_t len)
{
  const char *const cstr = HTTPD__VALID_CSTR_CHRS_URL_FILENAME;
  return (vstr_spn_cstr_chrs_fwd(s1, pos, len, cstr) == s1->len);
}

int httpd_init_default_hostname(Opt_serv_policy_opts *sopts)
{
  Httpd_policy_opts *popts = (Httpd_policy_opts *)sopts;
  Vstr_base *nhn = popts->default_hostname;
  Vstr_base *chn = NULL;

  if (!httpd_valid_url_filename(nhn, 1, nhn->len))
    vstr_del(nhn, 1, nhn->len);

  if (nhn->len)
    return (TRUE);
  
  if (sopts != sopts->beg->def_policy)
    chn = ((Httpd_policy_opts *)sopts->beg->def_policy)->default_hostname;
  
  if (chn)
    HTTPD_APP_REF_ALLVSTR(nhn, chn);
  else
  {
    opt_serv_sc_append_hostname(nhn, 0);
    vstr_conv_lowercase(nhn, 1, nhn->len);
  }

  return (!nhn->conf->malloc_bad);
}

static int httpd__chk_vhost(const Httpd_policy_opts *popts,
                            Vstr_base *lfn, size_t pos, size_t len)
{
  const char *vhost = NULL;
  struct stat64 v_stat[1];
  const Vstr_base *def_hname = popts->default_hostname;
  int ret = -1;
  
  ASSERT(pos);

  if (!popts->use_host_err_chk)
    return (TRUE);
  
  if (vstr_cmp_eq(lfn, pos, len, def_hname, 1, def_hname->len))
    return (TRUE); /* don't do lots of work for nothing */

  vstr_add_vstr(lfn, pos - 1, 
		popts->document_root, 1, popts->document_root->len,
                VSTR_TYPE_ADD_BUF_PTR);
  len += popts->document_root->len;
  
  if (lfn->conf->malloc_bad || !(vhost = vstr_export_cstr_ptr(lfn, pos, len)))
    return (TRUE); /* dealt with as errmem_req() later */
  
  ret = stat64(vhost, v_stat);
  vstr_del(lfn, pos, popts->document_root->len);

  if (ret == -1)
    return (FALSE);

  if (!S_ISDIR(v_stat->st_mode))
    return (FALSE);
  
  return (TRUE);
}

static int httpd_serv_add_vhost(struct Con *con, struct Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  Vstr_base *fname = req->fname;
  Vstr_sect_node *h_h = req->http_hdrs->hdr_host;
  size_t h_h_pos = h_h->pos;
  size_t h_h_len = h_h->len;
  size_t orig_len = 0;
  
  if (!req->policy->use_vhosts_name)
    return (TRUE);

  if (h_h_len && req->policy->use_canonize_host)
  {
    size_t dots = 0;
    
    if (VIPREFIX(data, h_h_pos, h_h_len, "www."))
    { h_h_len -= CLEN("www."); h_h_pos += CLEN("www."); }
    
    dots = vstr_spn_cstr_chrs_rev(data, h_h_pos, h_h_len, ".");
    h_h_len -= dots;
  }
  h_h->pos = h_h_pos;
  h_h->len = h_h_len;
    
  orig_len = fname->len;
  if (!h_h_len)
    httpd_sc_add_default_hostname(con, req, fname, 0);
  else if (vstr_add_vstr(fname, 0, data, /* add as buf's, for lowercase op */
                         h_h_pos, h_h_len, VSTR_TYPE_ADD_DEF))
  {
    vstr_conv_lowercase(fname, 1, h_h_len);
    
    if (!httpd__chk_vhost(req->policy, fname, 1, h_h_len))
    {
      if (req->policy->use_host_err_400)
        HTTPD_ERR_RET(req, 400, FALSE); /* rfc2616 5.2 */
      else
      { /* what everything else does ... *sigh* */
        if (fname->conf->malloc_bad)
          return (TRUE);

        h_h->len = 0;
        vstr_del(fname, 1, h_h_len);
        httpd_sc_add_default_hostname(con, req, fname, 0);
      }  
    }
  }
  vstr_add_cstr_ptr(fname, 0, "/");

  req->vhost_prefix_len = (fname->len - orig_len);

  return (TRUE);
}

/* Decode url-path,
   check url-path for a bunch of problems,
   if vhosts is on add vhost prefix,
   Note we don't do dir_filename additions yet */
static int http_req_make_path(struct Con *con, Httpd_req_data *req)
{
  Vstr_base *data = con->evnt->io_r;
  Vstr_base *fname = req->fname;
  
  ASSERT(!fname->len);

  assert(VPREFIX(data, req->path_pos, req->path_len, "/") ||
         VEQ(data, req->path_pos, req->path_len, "*"));

  if (req->chk_encoded_slash &&
      vstr_srch_case_cstr_buf_fwd(data, req->path_pos, req->path_len, "%2f"))
    HTTPD_ERR_RET(req, 403, FALSE);
  if (req->chk_encoded_dot &&
      vstr_srch_case_cstr_buf_fwd(data, req->path_pos, req->path_len, "%2e"))
    HTTPD_ERR_RET(req, 403, FALSE);
  req->chked_encoded_path = TRUE;

  vstr_add_vstr(fname, 0,
                data, req->path_pos, req->path_len, VSTR_TYPE_ADD_BUF_PTR);
  vstr_conv_decode_uri(fname, 1, fname->len);

  if (fname->conf->malloc_bad) /* dealt with as errmem_req() later */
    return (TRUE);

  /* NOTE: need to split function here so we can more efficently alter the
   * path. */
  if (!httpd_serv_add_vhost(con, req))
    return (FALSE);
  
  /* check posix path ... including hostname, for NIL and path escapes */
  if (vstr_srch_chr_fwd(fname, 1, fname->len, 0))
    HTTPD_ERR_RET(req, 403, FALSE);

  /* web servers don't have relative paths, so /./ and /../ aren't "special" */
  if (vstr_srch_cstr_buf_fwd(fname, 1, fname->len, "/../") ||
      VSUFFIX(req->fname, 1, req->fname->len, "/.."))
    HTTPD_ERR_RET(req, 400, FALSE);
  if (req->policy->chk_dot_dir &&
      (vstr_srch_cstr_buf_fwd(fname, 1, fname->len, "/./") ||
       VSUFFIX(req->fname, 1, req->fname->len, "/.")))
    HTTPD_ERR_RET(req, 400, FALSE);

  ASSERT(fname->len);
  assert(VPREFIX(fname, 1, fname->len, "/") ||
         VEQ(fname, 1, fname->len, "*") ||
         fname->conf->malloc_bad);

  if (fname->conf->malloc_bad)
    return (TRUE);
  
  return (TRUE);
}

static int http_parse_wait_io_r(struct Con *con)
{
  if (con->evnt->io_r_shutdown)
    return (!!con->evnt->io_w->len);

  evnt_wait_cntl_add(con->evnt, POLLIN);
  evnt_fd_set_cork(con->evnt, FALSE);
  
  return (TRUE);
}

static int httpd_serv__parse_no_req(struct Con *con, struct Httpd_req_data *req)
{
  if (req->policy->max_header_sz &&
      (con->evnt->io_r->len > req->policy->max_header_sz))
    HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));

  http_req_free(req);
  
  return (http_parse_wait_io_r(con));
}

/* http spec says ignore leading LWS ... *sigh* */
static int http__parse_req_all(struct Con *con, struct Httpd_req_data *req,
                               const char *eol, int *ern)
{
  Vstr_base *data = con->evnt->io_r;
  
  ASSERT(eol && ern);
  
  *ern = FALSE;
  
  if (!(req->len = vstr_srch_cstr_buf_fwd(data, 1, data->len, eol)))
      goto no_req;
  
  if (req->len == 1)
  { /* should use vstr_del(data, 1, vstr_spn_cstr_buf_fwd(..., HTTP_EOL)); */
    while (VPREFIX(data, 1, data->len, HTTP_EOL))
      vstr_del(data, 1, CLEN(HTTP_EOL));
    
    if (!(req->len = vstr_srch_cstr_buf_fwd(data, 1, data->len, eol)))
      goto no_req;
    
    ASSERT(req->len > 1);
  }
  
  req->len += CLEN(eol) - 1; /* add rest of EOL */

  return (TRUE);

 no_req:
  *ern = httpd_serv__parse_no_req(con, req);
  return (FALSE);
}

static int http_parse_req(struct Con *con)
{
  Vstr_base *data = con->evnt->io_r;
  struct Httpd_req_data *req = NULL;
  int ern_req_all = FALSE;

  ASSERT(con->fs && !con->fs_num);

  if (!data->len)
    return (http_parse_wait_io_r(con));
  
  if (!(req = http_req_make(con)))
    return (FALSE);

  if (con->parsed_method_ver_1_0) /* wait for all the headers */
  {
    if (!http__parse_req_all(con, req, HTTP_END_OF_REQUEST, &ern_req_all))
      return (ern_req_all);
  }
  else
  {
    if (!http__parse_req_all(con, req, HTTP_EOL,            &ern_req_all))
      return (ern_req_all);
  }

  con->keep_alive = HTTP_NON_KEEP_ALIVE;
  http_req_split_method(con, req);
  if (req->sects->malloc_bad)
    VLG_WARNNOMEM_RET(http_fin_errmem_req(con, req), (vlg, "split: %m\n"));
  else if (req->sects->num < 2)
    HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));
  else
  {
    size_t op_pos = 0;
    size_t op_len = 0;

    if (req->ver_0_9)
      vlg_dbg1(vlg, "Method(0.9):"
               " $<http-esc.vstr.sect:%p%p%u> $<http-esc.vstr.sect:%p%p%u>\n",
               con->evnt->io_r, req->sects, 1U,
               con->evnt->io_r, req->sects, 2U);
    else
    { /* need to get all headers */
      if (!con->parsed_method_ver_1_0)
      {
        con->parsed_method_ver_1_0 = TRUE;
        if (!http__parse_req_all(con, req, HTTP_END_OF_REQUEST, &ern_req_all))
          return (ern_req_all);
      }
      
      vlg_dbg1(vlg, "Method(1.x):"
               " $<http-esc.vstr.sect:%p%p%u> $<http-esc.vstr.sect:%p%p%u>"
               " $<http-esc.vstr.sect:%p%p%u>\n", data, req->sects, 1U,
               data, req->sects, 2U, data, req->sects, 3U);
      http_req_split_hdrs(con, req);
    }
    evnt_got_pkt(con->evnt);

    if (HTTP_CONF_SAFE_PRINT_REQ)
      vlg_dbg3(vlg, "REQ:\n$<vstr.hexdump:%p%zu%zu>",
               data, (size_t)1, req->len);
    else
      vlg_dbg3(vlg, "REQ:\n$<vstr:%p%zu%zu>", data, (size_t)1, req->len);
    
    assert(((req->sects->num >= 3) && !req->ver_0_9) || (req->sects->num == 2));
    
    op_pos        = VSTR_SECTS_NUM(req->sects, 1)->pos;
    op_len        = VSTR_SECTS_NUM(req->sects, 1)->len;
    req->path_pos = VSTR_SECTS_NUM(req->sects, 2)->pos;
    req->path_len = VSTR_SECTS_NUM(req->sects, 2)->len;

    if (!req->ver_0_9 && !http__parse_1_x(con, req))
    {
      if (req->error_code == 500)
        return (http_fin_errmem_req(con, req));
      return (http_fin_err_req(con, req));
    }
    
    if (!http_parse_host(con, req))
      HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));

    if (0) { }
    else if (VEQ(data, op_pos, op_len, "GET"))
    {
      if (!VPREFIX(data, req->path_pos, req->path_len, "/"))
        HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));
      
      if (!http_req_make_path(con, req))
        return (http_fin_err_req(con, req));

      return (http_req_op_get(con, req));
    }
    else if (req->ver_0_9) /* 400 or 501? - apache does 400 */
      HTTPD_ERR_RET(req, 501, http_fin_err_req(con, req));      
    else if (VEQ(data, op_pos, op_len, "HEAD"))
    {
      req->head_op = TRUE; /* not sure where this should go here */
      
      if (!VPREFIX(data, req->path_pos, req->path_len, "/"))
        HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));
      
      if (!http_req_make_path(con, req))
        return (http_fin_err_req(con, req));

      return (http_req_op_get(con, req));
    }
    else if (VEQ(data, op_pos, op_len, "OPTIONS"))
    {
      if (!VPREFIX(data, req->path_pos, req->path_len, "/") &&
          !VEQ(data, req->path_pos, req->path_len, "*"))
        HTTPD_ERR_RET(req, 400, http_fin_err_req(con, req));

      /* Speed hack: Don't even call make_path if it's "OPTIONS * ..."
       * and we don't need to check the Host header */
      if (req->policy->use_vhosts_name &&
          req->policy->use_host_err_chk && req->policy->use_host_err_400 &&
          !VEQ(data, req->path_pos, req->path_len, "*") &&
          !http_req_make_path(con, req))
        return (http_fin_err_req(con, req));

      return (http_req_op_opts(con, req));
    }
    else if (req->policy->use_trace_op && VEQ(data, op_pos, op_len, "TRACE"))
      return (http_req_op_trace(con, req));
    else if (VEQ(data, op_pos, op_len, "TRACE") ||
             VEQ(data, op_pos, op_len, "POST") ||
             VEQ(data, op_pos, op_len, "PUT") ||
             VEQ(data, op_pos, op_len, "DELETE") ||
             VEQ(data, op_pos, op_len, "CONNECT") ||
             FALSE) /* we know about these ... but don't allow them */
      HTTPD_ERR_RET(req, 405, http_fin_err_req(con, req));
    else
      HTTPD_ERR_RET(req, 501, http_fin_err_req(con, req));
  }
  ASSERT_NOT_REACHED();
}

static int httpd__serv_send_err(struct Con *con, const char *msg)
{
  if (errno != EPIPE)
    vlg_warn(vlg, "send(%s): %m\n", msg);
  else
    vlg_dbg2(vlg, "send(%s): SIGPIPE $<sa:%p>\n", msg, CON_CEVNT_SA(con));

  return (FALSE);
}

static int httpd_serv_q_send(struct Con *con)
{
  vlg_dbg2(vlg, "http Q send $<sa:%p>\n", CON_CEVNT_SA(con));
  if (!evnt_send_add(con->evnt, TRUE, HTTPD_CONF_MAX_WAIT_SEND))
    return (httpd__serv_send_err(con, "Q"));
      
  /* queued */
  return (TRUE);
}

static int httpd__serv_fin_send(struct Con *con)
{
  if (con->keep_alive)
  { /* need to try immediately, as we might have already got the next req */
    if (!con->evnt->io_r_shutdown)
      evnt_wait_cntl_add(con->evnt, POLLIN);
    return (http_parse_req(con));
  }

  vlg_dbg2(vlg, "shutdown_w = %p\n", con->evnt);
  return (evnt_shutdown_w(con->evnt));
}

/* NOTE: lim is just a hint ... we can send more */
static int httpd__serv_send_lim(struct Con *con, const char *emsg,
                                unsigned int lim, int *cont)
{
  Vstr_base *out = con->evnt->io_w;
  
  *cont = FALSE;
  
  while (out->len >= lim)
  {
    if (!con->io_limit_num--) return (httpd_serv_q_send(con));
    
    if (!evnt_send(con->evnt))
      return (httpd__serv_send_err(con, emsg));
  }

  *cont = TRUE;
  return (TRUE);
}

int httpd_serv_send(struct Con *con)
{
  Vstr_base *out = con->evnt->io_w;
  int cont = FALSE;
  int ret = FALSE;
  struct File_sect *fs = NULL;
  
  ASSERT(!out->conf->malloc_bad);
    
  if (!con->fs_num)
  {
    ASSERT(!con->fs_off);
    
    ret = httpd__serv_send_lim(con, "end", 1, &cont);
    if (!cont)
      return (ret);
    
    return (httpd__serv_fin_send(con));
  }

  ASSERT(con->fs && (con->fs_off < con->fs_num) && (con->fs_num <= con->fs_sz));
  fs = &con->fs[con->fs_off];
  ASSERT((fs->fd != -1) && fs->len);
  
  if (con->use_sendfile)
  {
    unsigned int ern = 0;

    ret = httpd__serv_send_lim(con, "sendfile", 1, &cont);
    if (!cont)
      return (ret);
    
    while (fs->len)
    {
      if (!con->io_limit_num--) return (httpd_serv_q_send(con));
      
      if (!evnt_sendfile(con->evnt, fs->fd, &fs->off, &fs->len, &ern))
      {
        if (ern == VSTR_TYPE_SC_READ_FD_ERR_EOF)
          goto file_eof_end;
        
        if (errno == EPIPE)
        {
          vlg_dbg2(vlg, "sendfile: SIGPIPE $<sa:%p>\n", CON_CEVNT_SA(con));
          return (FALSE);
        }

        if (errno == ENOSYS)
          httpd__disable_sendfile();
        vlg_warn(vlg, "sendfile: %m\n");
        
        if (lseek64(fs->fd, fs->off, SEEK_SET) == -1)
          VLG_WARN_RET(FALSE,
                       (vlg, "lseek(<sendfile>,off=%ju): %m\n", fs->off));

        con->use_sendfile = FALSE;
        return (httpd_serv_send(con)); /* recurse */
      }
    }
    
    goto file_end;
  }

  while (fs->len)
  {
    ret = httpd__serv_send_lim(con, "max", EX_MAX_W_DATA_INCORE, &cont);
    if (!cont)
      return (ret);
    
    switch (evnt_sc_read_send(con->evnt, fs->fd, &fs->len))
    {
      case EVNT_IO_OK:
        ASSERT_NO_SWITCH_DEF();
        
      case EVNT_IO_READ_ERR:
        vlg_warn(vlg, "read: %m\n");
        out->conf->malloc_bad = FALSE;
        
      case EVNT_IO_READ_FIN: goto file_end;
      case EVNT_IO_READ_EOF: goto file_eof_end;
        
      case EVNT_IO_SEND_ERR:
        return (httpd__serv_send_err(con, "io_get"));
    }
  }

  ASSERT_NOT_REACHED();

 file_end:
  ASSERT(!fs->len);
  if (con->use_mpbr) /* multipart/byterange */
  {
    ASSERT(con->mpbr_ct);
    ASSERT(con->mpbr_fs_len);

    if (!(fs = httpd__fd_next(con)))
    {
      vstr_add_cstr_ptr(out, out->len, "--SEP--" HTTP_EOL);
      return (httpd_serv_send(con)); /* restart with a read, or finish */
    }
    con->use_mmap = FALSE;
    if (!httpd__serv_call_seek(con, fs))
      VLG_WARN_RET(FALSE, (vlg, "lseek(<mpbr>,off=%ju): %m\n", fs->off));

    http_app_hdrs_mpbr(con, fs);
    return (httpd_serv_send(con)); /* start outputting next file section */
  }
  
 file_eof_end:
  if (fs->len) /* something bad happened, just kill the connection */
    con->keep_alive = HTTP_NON_KEEP_ALIVE;
  
  httpd_fin_fd_close(con);
  return (httpd_serv_send(con)); /* restart with a read, or finish */
}

int httpd_serv_recv(struct Con *con)
{
  unsigned int ern = 0;
  int ret = 0;
  Vstr_base *data = con->evnt->io_r;

  ASSERT(!con->evnt->io_r_shutdown);
  
  if (!con->io_limit_num--)
    return (TRUE);
  
  if (!(ret = evnt_recv(con->evnt, &ern)))
  {
    if (ern != VSTR_TYPE_SC_READ_FD_ERR_EOF)
    {
      vlg_dbg2(vlg, "RECV ERR from[$<sa:%p>]: %u\n", CON_CEVNT_SA(con), ern);
      goto con_cleanup;
    }
    if (!evnt_shutdown_r(con->evnt, TRUE))
      goto con_cleanup;
  }

  if (con->fs_num) /* may need to stop input, until we can deal with next req */
  {
    ASSERT(con->keep_alive || con->parsed_method_ver_1_0);
    
    if (con->policy->max_header_sz && (data->len > con->policy->max_header_sz))
      evnt_wait_cntl_del(con->evnt, POLLIN);

    return (TRUE);
  }
  
  if (http_parse_req(con))
    return (TRUE);
  
 con_cleanup:
  con->evnt->io_r->conf->malloc_bad = FALSE;
  con->evnt->io_w->conf->malloc_bad = FALSE;
    
  return (FALSE);
}

int httpd_con_init(struct Con *con, struct Acpt_listener *acpt_listener)
{
  int ret = TRUE;

  con->mpbr_ct = NULL;
  
  con->fs      = con->fs_store;
  con->fs->len = 0;
  con->fs->fd  = -1;
  con->fs_off  = 0;
  con->fs_num  = 0;
  con->fs_sz   = 1;

  con->vary_star   = FALSE;
  con->keep_alive  = HTTP_NON_KEEP_ALIVE;
  con->acpt_sa_ref = vstr_ref_add(acpt_listener->ref);
  con->use_mpbr    = FALSE;
  con->use_mmap    = FALSE;

  con->parsed_method_ver_1_0 = FALSE;

  httpd_policy_change_con(con, (Httpd_policy_opts *)httpd_opts->s->def_policy);

  if (!httpd_policy_connection(con,
                               httpd_opts->conf, httpd_opts->match_connection))
  {
    Vstr_base *s1 = con->policy->s->policy_name;
    Vstr_base *s2 = httpd_opts->conf->tmp;

    vlg_info(vlg, "CONF-MAIN-ERR from[$<sa:%p>]: policy $<vstr.all:%p>"
             " backtrace: $<vstr.all:%p>\n", CON_CEVNT_SA(con), s1, s2);
    ret = FALSE;
  }
  else if (con->evnt->flag_q_closed)
  {
    Vstr_base *s1 = con->policy->s->policy_name;
    vlg_info(vlg, "BLOCKED from[$<sa:%p>]: policy $<vstr.all:%p>\n",
             CON_CEVNT_SA(con), s1);
    ret = FALSE;
  }
  
  return (ret);
}
