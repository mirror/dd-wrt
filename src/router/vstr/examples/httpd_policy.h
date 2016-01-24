#ifndef HTTPD_POLICY_H
#define HTTPD_POLICY_H

#include "opt_policy.h"

#define HTTPD_POLICY__PATH_LIM_FULL 0
#define HTTPD_POLICY__PATH_LIM_BEG  1
#define HTTPD_POLICY__PATH_LIM_END  2
#define HTTPD_POLICY__PATH_LIM_EQ   3
#define HTTPD_POLICY__PATH_LIM_MASK 3
#define HTTPD_POLICY_PATH_LIM_NONE       0
#define HTTPD_POLICY_PATH_LIM_PATH_FULL (0x4  | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_PATH_BEG  (0x4  | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_PATH_END  (0x4  | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_PATH_EQ   (0x4  | HTTPD_POLICY__PATH_LIM_EQ)
#define HTTPD_POLICY_PATH_LIM_NAME_FULL (0x8  | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_NAME_BEG  (0x8  | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_NAME_END  (0x8  | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_NAME_EQ   (0x8  | HTTPD_POLICY__PATH_LIM_EQ)
#define HTTPD_POLICY_PATH_LIM_EXTN_FULL (0xc  | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_EXTN_BEG  (0xc  | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_EXTN_END  (0xc  | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_EXTN_EQ   (0xc  | HTTPD_POLICY__PATH_LIM_EQ)
#define HTTPD_POLICY_PATH_LIM_EXTS_FULL (0x14 | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_EXTS_BEG  (0x14 | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_EXTS_END  (0x14 | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_EXTS_EQ   (0x14 | HTTPD_POLICY__PATH_LIM_EQ)
#define HTTPD_POLICY_PATH_LIM_BWEN_FULL (0x18 | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_BWEN_BEG  (0x18 | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_BWEN_END  (0x18 | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_BWEN_EQ   (0x18 | HTTPD_POLICY__PATH_LIM_EQ)
#define HTTPD_POLICY_PATH_LIM_BWES_FULL (0x1c | HTTPD_POLICY__PATH_LIM_FULL)
#define HTTPD_POLICY_PATH_LIM_BWES_BEG  (0x1c | HTTPD_POLICY__PATH_LIM_BEG)
#define HTTPD_POLICY_PATH_LIM_BWES_END  (0x1c | HTTPD_POLICY__PATH_LIM_END)
#define HTTPD_POLICY_PATH_LIM_BWES_EQ   (0x1c | HTTPD_POLICY__PATH_LIM_EQ)

/* choosing (1024) as a offset ... kinda hackyish */
#define HTTPD_POLICY_REQ_PATH_BEG (1024 +  0)
#define HTTPD_POLICY_REQ_PATH_END (1024 +  1)
#define HTTPD_POLICY_REQ_PATH_EQ  (1024 +  2)
#define HTTPD_POLICY_REQ_NAME_BEG (1024 +  3)
#define HTTPD_POLICY_REQ_NAME_END (1024 +  4)
#define HTTPD_POLICY_REQ_NAME_EQ  (1024 +  5)
#define HTTPD_POLICY_REQ_BWEN_BEG (1024 +  6)
#define HTTPD_POLICY_REQ_BWEN_END (1024 +  7)
#define HTTPD_POLICY_REQ_BWEN_EQ  (1024 +  8)
#define HTTPD_POLICY_REQ_BWES_BEG (1024 +  9)
#define HTTPD_POLICY_REQ_BWES_END (1024 + 10)
#define HTTPD_POLICY_REQ_BWES_EQ  (1024 + 11)
#define HTTPD_POLICY_REQ_EXTN_BEG (1024 + 12)
#define HTTPD_POLICY_REQ_EXTN_END (1024 + 13)
#define HTTPD_POLICY_REQ_EXTN_EQ  (1024 + 14)
#define HTTPD_POLICY_REQ_EXTS_BEG (1024 + 15)
#define HTTPD_POLICY_REQ_EXTS_END (1024 + 16)
#define HTTPD_POLICY_REQ_EXTS_EQ  (1024 + 17)

typedef struct Httpd_policy_path
{
 Vstr_base *s1;
 void (*ref_func)(Vstr_ref *);
} Httpd_policy_path;

extern void httpd_policy_change_con(struct Con *, const Httpd_policy_opts *);
extern void httpd_policy_change_req(Httpd_req_data *,
                                    const Httpd_policy_opts *);
   
extern int httpd_policy_build_path(struct Con *, Httpd_req_data *,
                                   const Conf_parse *, Conf_token *,
                                   int *, int *);
extern int httpd_policy_path_make(struct Con *con, Httpd_req_data *req,
                                  Conf_parse *, Conf_token *, unsigned int,
                                  Vstr_ref **);

extern int httpd_policy_path_eq(const Vstr_base *,
                                const Vstr_base *, size_t *, size_t *);
extern int httpd_policy_path_beg_eq(const Vstr_base *,
                                    const Vstr_base *, size_t *, size_t *);
extern int httpd_policy_path_end_eq(const Vstr_base *,
                                    const Vstr_base *, size_t *, size_t *);

extern void httpd_policy_path_mod_name(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_path_mod_dirn(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_path_mod_extn(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_path_mod_exts(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_path_mod_bwen(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_path_mod_bwes(const Vstr_base *, size_t *, size_t *);

extern int  httpd_policy_path_lim_eq(const Vstr_base *, size_t *, size_t *,
                                     unsigned int, size_t, Vstr_ref *);

extern int httpd_policy_uri_eq(const Vstr_base *,
                               const Vstr_base *, size_t *, size_t *);
extern int httpd_policy_uri_beg_eq(const Vstr_base *,
                                   const Vstr_base *, size_t *, size_t *);
extern int httpd_policy_uri_end_eq(const Vstr_base *,
                                   const Vstr_base *, size_t *, size_t *);

extern void httpd_policy_uri_mod_name(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_uri_mod_dirn(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_uri_mod_extn(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_uri_mod_exts(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_uri_mod_bwen(const Vstr_base *, size_t *, size_t *);
extern void httpd_policy_uri_mod_bwes(const Vstr_base *, size_t *, size_t *);

extern int  httpd_policy_uri_lim_eq(const Vstr_base *, size_t *, size_t *,
                                    unsigned int, int, Vstr_ref *);

extern int httpd_policy_path_req2lim(unsigned int);

extern int httpd_policy_ipv4_make(struct Con *, Httpd_req_data *,
                                  Conf_parse *, Conf_token *,
                                  unsigned int, struct sockaddr *, int *);
extern int httpd_policy_ipv4_cidr_eq(struct Con *, Httpd_req_data *,
                                     Opt_policy_ipv4 *, struct sockaddr *);

extern void httpd_policy_exit(Httpd_policy_opts *);
extern int httpd_policy_init(Httpd_opts *, Httpd_policy_opts *);
extern Opt_serv_policy_opts *httpd_policy_make(Opt_serv_opts *);
extern int httpd_policy_copy(Opt_serv_policy_opts *,
                             const Opt_serv_policy_opts *);

#if !defined(HTTPD_POLICY_COMPILE_INLINE)
# ifdef VSTR_AUTOCONF_NDEBUG
#  define HTTPD_POLICY_COMPILE_INLINE 1
# else
#  define HTTPD_POLICY_COMPILE_INLINE 0
# endif
#endif

#if defined(VSTR_AUTOCONF_HAVE_INLINE) && HTTPD_POLICY_COMPILE_INLINE

#ifndef VSTR_AUTOCONF_NDEBUG
# define HTTPD_POLICY__ASSERT ASSERT
#else
# define HTTPD_POLICY__ASSERT(x)
#endif

#define HTTPD_POLICY__TRUE  1
#define HTTPD_POLICY__FALSE 0

extern inline void httpd_policy_change_con(struct Con *con,
                                           const Httpd_policy_opts *policy)
{
  con->use_sendfile      = policy->use_sendfile;
  con->use_posix_fadvise = policy->use_posix_fadvise;
  con->policy            = policy;
}

extern inline void httpd_policy_change_req(Httpd_req_data *req,
                                           const Httpd_policy_opts *policy)
{
  req->parse_accept          = policy->use_err_406;
  req->parse_accept_language = policy->use_err_406;
  req->allow_accept_encoding = policy->use_enc_content_replacement;
  if (req->vhost_prefix_len && !policy->use_vhosts_name)
  { /* NOTE: doesn't do chk_host properly */
    vstr_del(req->fname, 1, req->vhost_prefix_len);
    req->vhost_prefix_len = 0;
  }
  if (!req->chked_encoded_path)
  {
    req->chk_encoded_slash   = policy->chk_encoded_slash;
    req->chk_encoded_dot     = policy->chk_encoded_dot;
  }
  req->policy                = policy;
}

extern inline int httpd_policy_path_eq(const Vstr_base *s1,
                                       const Vstr_base *s2,
                                       size_t *p2, size_t *l2)
{
  return (vstr_cmp_eq(s1, 1, s1->len, s2, *p2, *l2));
}

/* if the s1 is equal to the begining of s2 */
extern inline int httpd_policy_path_beg_eq(const Vstr_base *s1,
                                           const Vstr_base *s2,
                                           size_t *p2, size_t *l2)
{
  if (*l2 > s1->len)
    *l2 = s1->len;
  return (vstr_cmp_eq(s1, 1, s1->len, s2, *p2, *l2));
}

/* if the s1 is equal to the end of s2 */
extern inline int httpd_policy_path_end_eq(const Vstr_base *s1,
                                           const Vstr_base *s2,
                                           size_t *p2, size_t *l2)
{
  if (*l2 > s1->len)
  {
    *p2 += (*l2 - s1->len);
    *l2 = s1->len;
  }
  return (vstr_cmp_eq(s1, 1, s1->len, s2, *p2, *l2));
}

extern inline void httpd_policy_path_mod_name(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = vstr_srch_chr_rev(s1, *pos, *len, '/');
  HTTPD_POLICY__ASSERT(srch);

  *len -= vstr_sc_posdiff(*pos, srch);
  *pos += vstr_sc_posdiff(*pos, srch);
}

extern inline void httpd_policy_path_mod_dirn(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = vstr_srch_chr_rev(s1, *pos, *len, '/');
  HTTPD_POLICY__ASSERT(srch);

  *len = vstr_sc_posdiff(*pos, srch);
}

extern inline void httpd_policy_path_mod_extn(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = 0;
  
  httpd_policy_path_mod_name(s1, pos, len);

  if ((srch = vstr_srch_chr_rev(s1, *pos, *len, '.')))
  { /* include '.' */
    *len -= srch - *pos;
    *pos = srch;
  }
  else
  { /* at point just after basename */
    *pos = vstr_sc_poslast(*pos, *len);
    *len = 0;
  }
}

extern inline void httpd_policy_path_mod_exts(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = 0;
  
  httpd_policy_path_mod_name(s1, pos, len);

  if ((srch = vstr_srch_chr_fwd(s1, *pos, *len, '.')))
  { /* include '.' */
    *len -= srch - *pos;
    *pos = srch;
  }
  else
  { /* at point just after basename */
    *pos = vstr_sc_poslast(*pos, *len);
    *len = 0;
  }
}

extern inline void httpd_policy_path_mod_bwen(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = 0;

  httpd_policy_path_mod_name(s1, pos, len);

  if ((srch = vstr_srch_chr_rev(s1, *pos, *len, '.')))
    *len = vstr_sc_posdiff(*pos, srch) - 1; /* don't include '.' */
}

extern inline void httpd_policy_path_mod_bwes(const Vstr_base *s1,
                                              size_t *pos, size_t *len)
{
  size_t srch = 0;

  httpd_policy_path_mod_name(s1, pos, len);

  if ((srch = vstr_srch_chr_fwd(s1, *pos, *len, '.')))
    *len = vstr_sc_posdiff(*pos, srch) - 1; /* don't include '.' */
}

extern inline int httpd_policy_path_lim_eq(const Vstr_base *s1,
                                           size_t *pos, size_t *len,
                                           unsigned int lim,
                                           size_t vhost_prefix_len,
                                           Vstr_ref *ref)
{
  const Httpd_policy_path *srch = NULL;

  if (lim == HTTPD_POLICY_PATH_LIM_NONE)
    return (HTTPD_POLICY__TRUE);
  
  *len -= vhost_prefix_len;
  *pos += vhost_prefix_len;
      
  switch (lim)
  {
    default: HTTPD_POLICY__ASSERT(HTTPD_POLICY__FALSE);

    case HTTPD_POLICY_PATH_LIM_PATH_FULL:
    case HTTPD_POLICY_PATH_LIM_PATH_BEG:
    case HTTPD_POLICY_PATH_LIM_PATH_END:
    case HTTPD_POLICY_PATH_LIM_PATH_EQ:
      break;
    
    case HTTPD_POLICY_PATH_LIM_NAME_FULL:
    case HTTPD_POLICY_PATH_LIM_NAME_BEG:
    case HTTPD_POLICY_PATH_LIM_NAME_END:
    case HTTPD_POLICY_PATH_LIM_NAME_EQ:
      httpd_policy_path_mod_name(s1, pos, len);
      break;
    
    case HTTPD_POLICY_PATH_LIM_EXTN_FULL:
    case HTTPD_POLICY_PATH_LIM_EXTN_BEG:
    case HTTPD_POLICY_PATH_LIM_EXTN_END:
    case HTTPD_POLICY_PATH_LIM_EXTN_EQ:
      httpd_policy_path_mod_extn(s1, pos, len);
      break;

    case HTTPD_POLICY_PATH_LIM_EXTS_FULL:
    case HTTPD_POLICY_PATH_LIM_EXTS_BEG:
    case HTTPD_POLICY_PATH_LIM_EXTS_END:
    case HTTPD_POLICY_PATH_LIM_EXTS_EQ:
      httpd_policy_path_mod_exts(s1, pos, len);
      break;

    case HTTPD_POLICY_PATH_LIM_BWEN_FULL:
    case HTTPD_POLICY_PATH_LIM_BWEN_BEG:
    case HTTPD_POLICY_PATH_LIM_BWEN_END:
    case HTTPD_POLICY_PATH_LIM_BWEN_EQ:
      httpd_policy_path_mod_bwen(s1, pos, len);
      break;
      
    case HTTPD_POLICY_PATH_LIM_BWES_FULL:
    case HTTPD_POLICY_PATH_LIM_BWES_BEG:
    case HTTPD_POLICY_PATH_LIM_BWES_END:
    case HTTPD_POLICY_PATH_LIM_BWES_EQ:
      httpd_policy_path_mod_bwes(s1, pos, len);
      break;
  }

  if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_FULL)
    return (HTTPD_POLICY__TRUE);
  
  HTTPD_POLICY__ASSERT(ref);
  srch = ref->ptr;
  
  if (0) { }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_BEG)
  {
    if (!httpd_policy_path_beg_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_END)
  {
    if (!httpd_policy_path_end_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_EQ)
  {
    if (!httpd_policy_path_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else
    HTTPD_POLICY__ASSERT(HTTPD_POLICY__FALSE);
    
  vstr_ref_del(ref); ref = NULL;
  return (HTTPD_POLICY__TRUE);
 path_no_match:
  vstr_ref_del(ref); ref = NULL;
  return (HTTPD_POLICY__FALSE);
}

static inline int httpd_policy__uri_eq(const Vstr_base *s1,
                                       const Vstr_base *s2,
                                       size_t p2, size_t l2, size_t *ret_len)
{
  size_t p1 = 1;
  size_t l1 = s1->len;
  
  while (l1 && (l2 >= l1))
  {
    size_t tmp = vstr_cspn_cstr_chrs_fwd(s2, p2, l1, "%");
    unsigned int val1 = 0;
    unsigned int val2 = 0;
    unsigned int num_flags = VSTR_FLAG02(PARSE_NUM_NO, BEG_ZERO, BEG_PM);
    
    if (tmp)
    {
      if (!vstr_cmp_eq(s1, p1, tmp, s2, p2, tmp))
        return (HTTPD_POLICY__FALSE);
      l1 -= tmp; p1 += tmp;
      l2 -= tmp; p2 += tmp;
      continue;
    }

    if (l2 < 3)
      return (HTTPD_POLICY__FALSE);
    HTTPD_POLICY__ASSERT(vstr_export_chr(s2, p2) == '%');

    val1 = (unsigned char)vstr_export_chr(s1, p1);
    val2 = vstr_parse_ushort(s2, p2 + 1, 2, 16 | num_flags, &tmp, NULL);

    if ((tmp != 2) || (val1 != val2))
      return (HTTPD_POLICY__FALSE);
    
    l1 -= 1; p1 += 1;
    l2 -= 3; p2 += 3;
  }

  *ret_len = l2;
  return (!l1);
}

extern inline int httpd_policy_uri_eq(const Vstr_base *s1,
                                      const Vstr_base *s2,
                                      size_t *p2, size_t *l2)
{
  size_t tmp = 0;
  return (httpd_policy__uri_eq(s1, s2, *p2, *l2, &tmp) && !tmp);
}

/* if the s1 is equal to the begining of s2 */
extern inline int httpd_policy_uri_beg_eq(const Vstr_base *s1,
                                          const Vstr_base *s2,
                                          size_t *p2, size_t *l2)
{
  size_t tmp = 0;

  if (!httpd_policy__uri_eq(s1, s2, *p2, *l2, &tmp))
    return (HTTPD_POLICY__FALSE);
  
  if (tmp) *l2 -= tmp;
  
  return (HTTPD_POLICY__TRUE);
}

/* if the s1 is equal to the end of s2 */
extern inline int httpd_policy_uri_end_eq(const Vstr_base *s1,
                                          const Vstr_base *s2,
                                          size_t *p2, size_t *l2)
{
  if (!vstr_srch_chr_fwd(s2, *p2, *l2, '%'))
  {
    if (*l2 > s1->len)
    {
      *p2 += (*l2 - s1->len);
      *l2 = s1->len;
    }
    return (vstr_cmp_eq(s1, 1, s1->len, s2, *p2, *l2));
  }

  if (*l2 > (s1->len * 3))
  {
    *p2 += (*l2 - (s1->len * 3));
    *l2 = (s1->len * 3);
  }

  while (*l2 >= s1->len)
  {
    size_t tmp = 0;
    if (httpd_policy__uri_eq(s1, s2, *p2, *l2, &tmp) && !tmp)
      return (HTTPD_POLICY__TRUE);
    *l2 -= 1; *p2 += 1;
  }
  
  return (HTTPD_POLICY__FALSE);
}

extern inline void httpd_policy_uri_mod_name(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = vstr_srch_chr_rev(s1, *pos, *len, '/');
  size_t srch2 = vstr_srch_case_cstr_buf_rev(s1, *pos, *len, "%2f");
  HTTPD_POLICY__ASSERT(srch1);

  if (srch1 < srch2)
    srch1 = srch2 + 2;
  
  *len -= vstr_sc_posdiff(*pos, srch1);
  *pos += vstr_sc_posdiff(*pos, srch1);
}

extern inline void httpd_policy_uri_mod_dirn(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = vstr_srch_chr_rev(s1, *pos, *len, '/');
  size_t srch2 = vstr_srch_case_cstr_buf_rev(s1, *pos, *len, "%2f");
  HTTPD_POLICY__ASSERT(srch1);

  if (srch1 < srch2)
    srch1 = srch2 + 2;
  
  *len = vstr_sc_posdiff(*pos, srch1);
}

extern inline void httpd_policy_uri_mod_extn(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = 0;
  size_t srch2 = 0;
  
  httpd_policy_uri_mod_name(s1, pos, len);

  srch1 = vstr_srch_chr_rev(s1, *pos, *len, '.');
  srch2 = vstr_srch_case_cstr_buf_rev(s1, *pos, *len, "%2e");
  
  if (srch1 < srch2)
    srch1 = srch2;

  if (srch1)
  { /* include '.' or "%2e" */
    *len -= srch1 - *pos;
    *pos = srch1;
  }
  else
  { /* at point just after basename */
    *pos = vstr_sc_poslast(*pos, *len);
    *len = 0;
  }
}

extern inline void httpd_policy_uri_mod_exts(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = 0;
  size_t srch2 = 0;
  
  httpd_policy_uri_mod_name(s1, pos, len);

  srch1 = vstr_srch_chr_fwd(s1, *pos, *len, '.');
  srch2 = vstr_srch_case_cstr_buf_fwd(s1, *pos, *len, "%2e");
  
  if (srch1 > srch2)
    srch1 = srch2;

  if (srch1)
  { /* include '.' or "%2e" */
    *len -= srch1 - *pos;
    *pos = srch1;
  }
  else
  { /* at point just after basename */
    *pos = vstr_sc_poslast(*pos, *len);
    *len = 0;
  }
}

extern inline void httpd_policy_uri_mod_bwen(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = 0;
  size_t srch2 = 0;
  unsigned int num = 1;

  httpd_policy_uri_mod_name(s1, pos, len);

  srch1 = vstr_srch_chr_rev(s1, *pos, *len, '.');
  srch2 = vstr_srch_case_cstr_buf_rev(s1, *pos, *len, "%2e");
  
  if (srch1 < srch2)
  {
    srch1 = srch2;
    num   = 3;
  }
  
  if (srch1)
    *len = vstr_sc_posdiff(*pos, srch1) - num; /* don't include '.' */
}

extern inline void httpd_policy_uri_mod_bwes(const Vstr_base *s1,
                                             size_t *pos, size_t *len)
{
  size_t srch1 = 0;
  size_t srch2 = 0;
  unsigned int num = 1;

  httpd_policy_uri_mod_name(s1, pos, len);

  srch1 = vstr_srch_chr_fwd(s1, *pos, *len, '.');
  srch2 = vstr_srch_case_cstr_buf_fwd(s1, *pos, *len, "%2e");
  
  if (srch1 > srch2)
  {
    srch1 = srch2;
    num   = 3;
  }
  
  if (srch1)
    *len = vstr_sc_posdiff(*pos, srch1) - num; /* don't include '.' */
}

extern inline int httpd_policy_uri_lim_eq(const Vstr_base *s1,
                                          size_t *pos, size_t *len,
                                          unsigned int lim, int slash_dot_safe,
                                          Vstr_ref *ref)
{
  const Httpd_policy_path *srch = NULL;
  
  switch (lim)
  {
    default: HTTPD_POLICY__ASSERT(HTTPD_POLICY__FALSE);
    case HTTPD_POLICY_PATH_LIM_NONE:
      HTTPD_POLICY__ASSERT(!ref);
      return (HTTPD_POLICY__TRUE);

    case HTTPD_POLICY_PATH_LIM_PATH_FULL:
    case HTTPD_POLICY_PATH_LIM_PATH_BEG:
    case HTTPD_POLICY_PATH_LIM_PATH_END:
    case HTTPD_POLICY_PATH_LIM_PATH_EQ:
      break;
    
    case HTTPD_POLICY_PATH_LIM_NAME_FULL:
    case HTTPD_POLICY_PATH_LIM_NAME_BEG:
    case HTTPD_POLICY_PATH_LIM_NAME_END:
    case HTTPD_POLICY_PATH_LIM_NAME_EQ:
      if (slash_dot_safe)
        httpd_policy_path_mod_name(s1, pos, len);
      else
        httpd_policy_uri_mod_name(s1, pos, len);
      break;
    
    case HTTPD_POLICY_PATH_LIM_EXTN_FULL:
    case HTTPD_POLICY_PATH_LIM_EXTN_BEG:
    case HTTPD_POLICY_PATH_LIM_EXTN_END:
    case HTTPD_POLICY_PATH_LIM_EXTN_EQ:
      if (slash_dot_safe)
        httpd_policy_path_mod_extn(s1, pos, len);
      else
        httpd_policy_uri_mod_extn(s1, pos, len);
      break;

    case HTTPD_POLICY_PATH_LIM_EXTS_FULL:
    case HTTPD_POLICY_PATH_LIM_EXTS_BEG:
    case HTTPD_POLICY_PATH_LIM_EXTS_END:
    case HTTPD_POLICY_PATH_LIM_EXTS_EQ:
      if (slash_dot_safe)
        httpd_policy_path_mod_exts(s1, pos, len);
      else
        httpd_policy_uri_mod_exts(s1, pos, len);
      break;

    case HTTPD_POLICY_PATH_LIM_BWEN_FULL:
    case HTTPD_POLICY_PATH_LIM_BWEN_BEG:
    case HTTPD_POLICY_PATH_LIM_BWEN_END:
    case HTTPD_POLICY_PATH_LIM_BWEN_EQ:
      if (slash_dot_safe)
        httpd_policy_path_mod_bwen(s1, pos, len);
      else
        httpd_policy_uri_mod_bwen(s1, pos, len);
      break;
      
    case HTTPD_POLICY_PATH_LIM_BWES_FULL:
    case HTTPD_POLICY_PATH_LIM_BWES_BEG:
    case HTTPD_POLICY_PATH_LIM_BWES_END:
    case HTTPD_POLICY_PATH_LIM_BWES_EQ:
      if (slash_dot_safe)
        httpd_policy_path_mod_bwes(s1, pos, len);
      else
        httpd_policy_uri_mod_bwes(s1, pos, len);
      break;
  }

  if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_FULL)
  {
    HTTPD_POLICY__ASSERT(!ref);
    return (HTTPD_POLICY__TRUE);
  }
  
  HTTPD_POLICY__ASSERT(ref);
  srch = ref->ptr;
  
  if (0) { }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_BEG)
  {
    if (!httpd_policy_uri_beg_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_END)
  {
    if (!httpd_policy_uri_end_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else if ((lim & HTTPD_POLICY__PATH_LIM_MASK) == HTTPD_POLICY__PATH_LIM_EQ)
  {
    if (!httpd_policy_uri_eq(srch->s1, s1, pos, len))
      goto path_no_match;
  }
  else
    HTTPD_POLICY__ASSERT(HTTPD_POLICY__FALSE);
    
  vstr_ref_del(ref); ref = NULL;
  return (HTTPD_POLICY__TRUE);
 path_no_match:
  vstr_ref_del(ref); ref = NULL;
  return (HTTPD_POLICY__FALSE);
}

extern inline int httpd_policy_path_req2lim(unsigned int type)
{
  unsigned int lim = HTTPD_POLICY_PATH_LIM_NONE;
  
  switch (type)
  {
    case HTTPD_POLICY_REQ_PATH_BEG: lim = HTTPD_POLICY_PATH_LIM_PATH_BEG; break;
    case HTTPD_POLICY_REQ_PATH_END: lim = HTTPD_POLICY_PATH_LIM_PATH_END; break;
    case HTTPD_POLICY_REQ_PATH_EQ:  lim = HTTPD_POLICY_PATH_LIM_PATH_EQ;  break;
      
    case HTTPD_POLICY_REQ_NAME_BEG: lim = HTTPD_POLICY_PATH_LIM_NAME_BEG; break;
    case HTTPD_POLICY_REQ_NAME_END: lim = HTTPD_POLICY_PATH_LIM_NAME_END; break;
    case HTTPD_POLICY_REQ_NAME_EQ:  lim = HTTPD_POLICY_PATH_LIM_NAME_EQ;  break;
      
    case HTTPD_POLICY_REQ_BWEN_BEG: lim = HTTPD_POLICY_PATH_LIM_BWEN_BEG; break;
    case HTTPD_POLICY_REQ_BWEN_END: lim = HTTPD_POLICY_PATH_LIM_BWEN_END; break;
    case HTTPD_POLICY_REQ_BWEN_EQ:  lim = HTTPD_POLICY_PATH_LIM_BWEN_EQ;  break;
      
    case HTTPD_POLICY_REQ_BWES_BEG: lim = HTTPD_POLICY_PATH_LIM_BWES_BEG; break;
    case HTTPD_POLICY_REQ_BWES_END: lim = HTTPD_POLICY_PATH_LIM_BWES_END; break;
    case HTTPD_POLICY_REQ_BWES_EQ:  lim = HTTPD_POLICY_PATH_LIM_BWES_EQ;  break;
      
    case HTTPD_POLICY_REQ_EXTN_BEG: lim = HTTPD_POLICY_PATH_LIM_EXTN_BEG; break;
    case HTTPD_POLICY_REQ_EXTN_END: lim = HTTPD_POLICY_PATH_LIM_EXTN_END; break;
    case HTTPD_POLICY_REQ_EXTN_EQ:  lim = HTTPD_POLICY_PATH_LIM_EXTN_EQ;  break;
      
    case HTTPD_POLICY_REQ_EXTS_BEG: lim = HTTPD_POLICY_PATH_LIM_EXTS_BEG; break;
    case HTTPD_POLICY_REQ_EXTS_END: lim = HTTPD_POLICY_PATH_LIM_EXTS_END; break;
    case HTTPD_POLICY_REQ_EXTS_EQ:  lim = HTTPD_POLICY_PATH_LIM_EXTS_EQ;  break;
      
    default:
      HTTPD_POLICY__ASSERT(HTTPD_POLICY__FALSE);
  }

  return (lim);
}

extern inline int httpd_policy_ipv4_make(struct Con *con, Httpd_req_data *req,
                                         Conf_parse *conf, Conf_token *token,
                                         unsigned int type,
                                         struct sockaddr *sa, int *matches)
{
  HTTPD_POLICY__ASSERT(con);
  
  if (sa == EVNT_SA(con->evnt))
  {
    if (req)
      req->vary_star = HTTPD_POLICY__TRUE;
    else
      con->vary_star = HTTPD_POLICY__TRUE;
  }

  return (opt_policy_ipv4_make(conf, token, type, sa, matches));
}

extern inline int httpd_policy_ipv4_cidr_eq(struct Con *con,Httpd_req_data *req,
                                            Opt_policy_ipv4 *ipv4,
                                            struct sockaddr *sa)
{
  HTTPD_POLICY__ASSERT(con);
  
  if (sa == EVNT_SA(con->evnt))
  {
    if (req)
      req->vary_star = HTTPD_POLICY__TRUE;
    else
      con->vary_star = HTTPD_POLICY__TRUE;
  }

  return (opt_policy_ipv4_cidr_eq(ipv4, sa));
}


#endif

#endif
