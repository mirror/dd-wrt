#ifndef CONF_H
#define CONF_H

#include <vstr.h>

#define CONF_PARSE_ERR 0
#define CONF_PARSE_FIN 1

#define CONF_PARSE_STATE_BEG              0
#define CONF_PARSE_STATE_WS               1
#define CONF_PARSE_STATE_LIST_END_OR_WS   2
#define CONF_PARSE_STATE_CHOOSE           3
#define CONF_PARSE_STATE_QUOTE_D_BEG      4
#define CONF_PARSE_STATE_QUOTE_S_BEG      5
#define CONF_PARSE_STATE_UNQUOTE_BEG      6
#define CONF_PARSE_STATE_QUOTE_D_END      7
#define CONF_PARSE_STATE_QUOTE_DDD_END    8
#define CONF_PARSE_STATE_QUOTE_S_END      9
#define CONF_PARSE_STATE_QUOTE_SSS_END   10
#define CONF_PARSE_STATE_UNQUOTE_D_END   11 /* unescaped quoted */
#define CONF_PARSE_STATE_UNQUOTE_DDD_END 12
#define CONF_PARSE_STATE_UNQUOTE_S_END   13
#define CONF_PARSE_STATE_UNQUOTE_SSS_END 14
#define CONF_PARSE_STATE_SYMBOL_END      15
#define CONF_PARSE_STATE_END             16

#define CONF_PARSE_STATE_QUOTE_X2XXX                                    \
    (CONF_PARSE_STATE_QUOTE_DDD_END - CONF_PARSE_STATE_QUOTE_D_END)
#define CONF_PARSE_STATE_QUOTE2UNQUOTE_END                              \
    (CONF_PARSE_STATE_UNQUOTE_D_END - CONF_PARSE_STATE_QUOTE_D_END)

#define CONF_PARSE_LIST_DEPTH_SZ 64

#define CONF_TOKEN_TYPE_ERR              0
#define CONF_TOKEN_TYPE_CLIST            1
#define CONF_TOKEN_TYPE_SLIST            2
#define CONF_TOKEN_TYPE_QUOTE_D          3
#define CONF_TOKEN_TYPE_QUOTE_ESC_D      4
#define CONF_TOKEN_TYPE_QUOTE_DDD        5
#define CONF_TOKEN_TYPE_QUOTE_ESC_DDD    6
#define CONF_TOKEN_TYPE_QUOTE_S          7
#define CONF_TOKEN_TYPE_QUOTE_ESC_S      8
#define CONF_TOKEN_TYPE_QUOTE_SSS        9
#define CONF_TOKEN_TYPE_QUOTE_ESC_SSS   10
#define CONF_TOKEN_TYPE_SYMBOL          11
#define CONF_TOKEN_TYPE_USER_BEG        12

#define CONF_TOKEN_TYPE_2ESC                                    \
    (CONF_TOKEN_TYPE_QUOTE_ESC_D - CONF_TOKEN_TYPE_QUOTE_D)

#define CONF_SC_TYPE_RET_OK 0
/* #define CONF_SC_TYPE_RET_ERR_TOO_MANY  1 */
#define CONF_SC_TYPE_RET_ERR_NO_MATCH  2
#define CONF_SC_TYPE_RET_ERR_NOT_EXIST 3
#define CONF_SC_TYPE_RET_ERR_PARSE     4

#define CONF_SC_PARSE_DEPTH_TOKEN_RET(c, t, d, ret) do {         \
      if (!conf_token_list_num(t, d))                            \
        return ret ;                                             \
      conf_parse_token(c, t);                                    \
      if (conf_token_at_depth(t) != (d))                         \
        return ret ;                                             \
    } while (0)

#define CONF_SC_PARSE_TYPE_DEPTH_TOKEN_RET(c, t, d, T, ret) do {        \
      CONF_SC_PARSE_DEPTH_TOKEN_RET(c, t, d, ret);                      \
      if (token->type != T)                                             \
        return ret ;                                                    \
    } while (0)

#define CONF_SC_PARSE_CLIST_DEPTH_TOKEN_RET(c, t, d, ret)       \
    CONF_SC_PARSE_TYPE_DEPTH_TOKEN_RET(c, t, d, CONF_TOKEN_TYPE_CLIST, ret)
#define CONF_SC_PARSE_SLIST_DEPTH_TOKEN_RET(c, t, d, ret)       \
    CONF_SC_PARSE_TYPE_DEPTH_TOKEN_RET(c, t, d, CONF_TOKEN_TYPE_SLIST, ret)

#define CONF_SC_PARSE_TOP_TOKEN_RET(c, t, ret) do {                     \
      unsigned int conf_sc__token_depth = (t)->depth_num;               \
      CONF_SC_PARSE_DEPTH_TOKEN_RET(c, t, conf_sc__token_depth, ret);   \
    } while (0)
#define CONF_SC_PARSE_CLIST_TOP_TOKEN_RET(c, t, ret) do {               \
      unsigned int conf_sc__token_depth = (t)->depth_num;               \
      CONF_SC_PARSE_CLIST_DEPTH_TOKEN_RET(c, t, conf_sc__token_depth, ret); \
    } while (0)
#define CONF_SC_PARSE_SLIST_TOP_TOKEN_RET(c, t, ret) do {               \
      unsigned int conf_sc__token_depth = (t)->depth_num;               \
      CONF_SC_PARSE_SLIST_DEPTH_TOKEN_RET(c, t, conf_sc__token_depth, ret); \
    } while (0)

#define CONF_SC_TOGGLE_CLIST_VAR(x)  do {               \
      if (token->type == CONF_TOKEN_TYPE_CLIST)         \
      {                                                 \
        (x) = 1;                                        \
        CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, 0);    \
      }                                                 \
      else                                              \
        (x) = 0;                                        \
    } while (0)

#define CONF_SC_MAKE_CLIST_MID(x, y)                                    \
    while (conf_token_list_num(token, x))                               \
    {                                                                   \
      CONF_SC_PARSE_DEPTH_TOKEN_RET(conf, token, x, 0);                 \
      CONF_SC_TOGGLE_CLIST_VAR(y);                                      \
                                                                        \
      if (0)

#define CONF_SC_MAKE_CLIST_BEG(x, y)                                    \
    unsigned int conf_sc__depth_ ## x = token->depth_num;               \
    CONF_SC_MAKE_CLIST_MID(conf_sc__depth_ ## x, y)

#define CONF_SC_MAKE_CLIST_END()                \
    else                                        \
      return (0);                               \
    }                                           \
    do { } while (0)


typedef struct Conf__uval_store
{
 Vstr_ref *ref;
 unsigned int nxt;
} Conf__uval_store;

typedef struct Conf_parse
{
 Vstr_sects *sects;
 Vstr_base *data;
 Vstr_base *tmp;
 unsigned int  types_sz;
 unsigned int *types_ptr;
 unsigned int      uvals_sz;
 unsigned int      uvals_num;
 Conf__uval_store *uvals_ptr;
 unsigned int state;
 unsigned int depth;
} Conf_parse;

typedef struct Conf_token
{
 union 
 {
  const Vstr_sect_node *node;
  unsigned int list_num;
  unsigned int uval_num;
 } u;
 
 unsigned int type;
 unsigned int num;
 unsigned int depth_num;
 unsigned int depth_nums[CONF_PARSE_LIST_DEPTH_SZ + 1];
} Conf_token;
#define CONF_TOKEN_INIT {{NULL}, CONF_TOKEN_TYPE_ERR, 0, 0, {0}}

extern Conf_parse *conf_parse_make(Vstr_conf *)
   VSTR__COMPILE_ATTR_MALLOC();
extern void        conf_parse_free(Conf_parse *);

extern int conf_parse_lex(Conf_parse *, size_t, size_t);

extern Conf_token *conf_token_make(void)
   VSTR__COMPILE_ATTR_MALLOC();
extern void        conf_token_free(Conf_token *);

extern int conf_parse_token(const Conf_parse *, Conf_token *);
extern int conf_parse_end_token(const Conf_parse *, Conf_token *, unsigned int);
extern int conf_parse_num_token(const Conf_parse *, Conf_token *, unsigned int);

extern unsigned int conf_token_at_depth(const Conf_token *);

extern const char *conf_token_name(const Conf_token *);
extern const Vstr_sect_node *conf_token_value(const Conf_token *);

extern Vstr_ref *conf_token_get_user_value(const Conf_parse *,
                                           const Conf_token *, unsigned int *);
extern int conf_token_set_user_value(Conf_parse *, Conf_token *,
                                     unsigned int, Vstr_ref *, unsigned int);

extern int conf_token_cmp_val_eq(const Conf_parse *, const Conf_token *,
                                 const Vstr_base *, size_t, size_t);
extern int conf_token_cmp_val_buf_eq(const Conf_parse *, const Conf_token *,
                                     const char *, size_t);
extern int conf_token_cmp_val_cstr_eq(const Conf_parse *, const Conf_token *,
                                      const char *);
extern int conf_token_cmp_val_beg_eq(const Conf_parse *, const Conf_token *,
                                     const Vstr_base *, size_t, size_t);
extern int conf_token_cmp_case_val_eq(const Conf_parse *,
                                      const Conf_token *,
                                      const Vstr_base *, size_t, size_t);
extern int conf_token_cmp_case_val_cstr_eq(const Conf_parse *,
                                           const Conf_token *,
                                           const char *);
extern int conf_token_cmp_case_val_beg_eq(const Conf_parse *,
                                          const Conf_token *,
                                          const Vstr_base *, size_t, size_t);
extern int conf_token_cmp_sym_eq(const Conf_parse *, const Conf_token *,
                                 const Vstr_base *, size_t, size_t);
extern int conf_token_cmp_sym_buf_eq(const Conf_parse *, const Conf_token *,
                                     const char *, size_t);
extern int conf_token_cmp_sym_cstr_eq(const Conf_parse *, const Conf_token *,
                                      const char *);
extern int conf_token_cmp_case_sym_cstr_eq(const Conf_parse *,
                                           const Conf_token *,
                                           const char *);

extern int conf_token_srch_case_val(const Conf_parse *, const Conf_token *,
                                    const Vstr_base *, size_t, size_t);

extern unsigned int conf_token_list_num(const Conf_token *, unsigned int);

extern int conf_sc_token_parse_toggle(const Conf_parse *, Conf_token *, int *);
extern int conf_sc_token_parse_uint(const Conf_parse *, Conf_token *,
                                    unsigned int *);
extern int conf_sc_token_app_vstr(const Conf_parse *, Conf_token *,
                                  Vstr_base *,
                                  const Vstr_base **, size_t *, size_t *);
extern int conf_sc_token_sub_vstr(const Conf_parse *, Conf_token *,
                                  Vstr_base *, size_t, size_t);
extern int conf_sc_conv_unesc(Vstr_base *, size_t, size_t, size_t *);

extern void conf_parse_compress(Conf_parse *);
extern void conf_parse_backtrace(Vstr_base *, const char *,
                                 const Conf_parse *, const Conf_token *);

#if !defined(CONF_COMPILE_INLINE)
# ifdef VSTR_AUTOCONF_NDEBUG
#  define CONF_COMPILE_INLINE 1
# else
#  define CONF_COMPILE_INLINE 0
# endif
#endif

#if defined(VSTR_AUTOCONF_HAVE_INLINE) && CONF_COMPILE_INLINE

#ifndef VSTR_AUTOCONF_NDEBUG
# define CONF__ASSERT ASSERT
#else
# define CONF__ASSERT(x)
#endif

#define CONF__FALSE  0
#define CONF__TRUE   1

extern inline int conf_parse_token(const Conf_parse *conf, Conf_token *token)
{
  CONF__ASSERT(conf && conf->sects && token);
  CONF__ASSERT(!conf->depth); /* finished lex */
  CONF__ASSERT(conf->state == CONF_PARSE_STATE_END); /* finished lex */

  if (!token->num)
    token->depth_nums[0] = conf->sects->num;
  
  if (token->num >= conf->sects->num)
    return (CONF__FALSE);
  ++token->num;
  
  while (token->depth_nums[token->depth_num] < token->num)
  {
    CONF__ASSERT(token->depth_nums[token->depth_num] == (token->num - 1));
    --token->depth_num;
  }

  token->type = conf->types_ptr[token->num - 1];
  if ((token->type >= CONF_TOKEN_TYPE_QUOTE_D) &&
      (token->type <= CONF_TOKEN_TYPE_SYMBOL))
    token->u.node = VSTR_SECTS_NUM(conf->sects, token->num);
  else if ((token->type == CONF_TOKEN_TYPE_CLIST) ||
           (token->type == CONF_TOKEN_TYPE_SLIST))
  {
    token->u.list_num = VSTR_SECTS_NUM(conf->sects, token->num)->len;
    token->depth_nums[++token->depth_num] = token->num + token->u.list_num;
  }
  else
    token->u.uval_num = VSTR_SECTS_NUM(conf->sects, token->num)->pos;
    
  return (CONF__TRUE);
}

extern inline int conf_parse_end_token(const Conf_parse *conf,
                                       Conf_token *token,
                                       unsigned int depth)
{
  if (depth > token->depth_num)
    return (CONF__FALSE);

  if (token->depth_nums[depth] >= token->num)
    token->num = token->depth_nums[depth] - 1;

  return (conf_parse_token(conf, token));
}

extern inline int conf_parse_num_token(const Conf_parse *conf,
                                       Conf_token *token,
                                       unsigned int num)
{
  if (token->num == num)
  { /* refresh info... */
    token->type = conf->types_ptr[token->num - 1];

    if ((token->type >= CONF_TOKEN_TYPE_QUOTE_D) &&
        (token->type <= CONF_TOKEN_TYPE_SYMBOL))
    { }
    else if ((token->type == CONF_TOKEN_TYPE_CLIST) ||
             (token->type == CONF_TOKEN_TYPE_SLIST))
      token->u.list_num = VSTR_SECTS_NUM(conf->sects, token->num)->len;
    else
      token->u.uval_num = VSTR_SECTS_NUM(conf->sects, token->num)->pos;
    
    return (CONF__TRUE);    
  }
  
  if (token->num > num)
    return (CONF__FALSE);
  
  while (token->num < num)
  {
    if (!conf_parse_token(conf, token))
      return (CONF__FALSE);
  }

  return (CONF__TRUE);
}

extern inline unsigned int conf_token_at_depth(const Conf_token *token)
{
  unsigned int depth = 0;
  
  CONF__ASSERT(token);

  depth = token->depth_num;
  if ((token->type == CONF_TOKEN_TYPE_CLIST) ||
      (token->type == CONF_TOKEN_TYPE_SLIST))
    --depth;

  return (depth);
}

extern inline const Vstr_sect_node *conf_token_value(const Conf_token *token)
{
  CONF__ASSERT(token);
  
  if ((token->type >= CONF_TOKEN_TYPE_QUOTE_D) &&
      (token->type <= CONF_TOKEN_TYPE_SYMBOL))
    return (token->u.node);
  
  return (NULL);
}

extern inline Vstr_ref *conf_token_get_user_value(const Conf_parse *conf,
                                                  const Conf_token *token,
                                                  unsigned int *nxt)
{
  unsigned int dummy;
  
  CONF__ASSERT(conf && token);

  if (!nxt) nxt = &dummy;
  *nxt = 0;
  
  if (token->type <= CONF_TOKEN_TYPE_SYMBOL)
    return (NULL);

  if (token->u.uval_num >= conf->uvals_sz)
    return (NULL);

  CONF__ASSERT(conf->uvals_sz);

  *nxt = conf->uvals_ptr[token->u.uval_num].nxt;
  if (!conf->uvals_ptr[token->u.uval_num].ref)
    return (NULL);
  
  return (vstr_ref_add(conf->uvals_ptr[token->u.uval_num].ref));
}

extern inline int conf_token_cmp_val_eq(const Conf_parse *conf,
                                        const Conf_token *token,
                                        const Vstr_base *s1,
                                        size_t pos, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && s1);
  
  if (!val) return (CONF__FALSE);
  
  return (vstr_cmp_eq(conf->data, val->pos, val->len, s1, pos, len));
}

extern inline int conf_token_cmp_val_buf_eq(const Conf_parse *conf,
                                            const Conf_token *token,
                                            const char *buf, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && buf);
  
  if (!val) return (CONF__FALSE);
  
  return (vstr_cmp_buf_eq(conf->data, val->pos, val->len, buf, len));
}

extern inline int conf_token_cmp_val_cstr_eq(const Conf_parse *conf,
                                             const Conf_token *token,
                                             const char *cstr)
{
  return (conf_token_cmp_val_buf_eq(conf, token, cstr, strlen(cstr)));
}

extern inline int conf_token_cmp_val_beg_eq(const Conf_parse *conf,
                                            const Conf_token *token,
                                            const Vstr_base *s1,
                                            size_t pos, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && s1);
  
  if (!val) return (CONF__FALSE);

  if (len > val->len)
    len = val->len;
  
  return (vstr_cmp_eq(conf->data, val->pos, val->len, s1, pos, len));
}

extern inline int conf_token_cmp_case_val_eq(const Conf_parse *conf,
                                             const Conf_token *token,
                                             const Vstr_base *s1,
                                             size_t pos, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && s1);
  
  if (!val) return (CONF__FALSE);
  
  return (vstr_cmp_case_eq(conf->data, val->pos, val->len, s1, pos, len));
}

extern inline int conf_token_cmp_case_val_cstr_eq(const Conf_parse *conf,
                                                  const Conf_token *token,
                                                  const char *cstr)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && cstr);
  
  if (!val) return (CONF__FALSE);
  
  return (vstr_cmp_case_cstr_eq(conf->data, val->pos, val->len, cstr));
}

extern inline int conf_token_cmp_case_val_beg_eq(const Conf_parse *conf,
                                                 const Conf_token *token,
                                                 const Vstr_base *s1,
                                                 size_t pos, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && s1);
  
  if (!val) return (CONF__FALSE);

  if (len > val->len)
    len = val->len;
  
  return (vstr_cmp_case_eq(conf->data, val->pos, val->len, s1, pos, len));
}

extern inline int conf_token_cmp_sym_eq(const Conf_parse *conf,
                                        const Conf_token *token,
                                        const Vstr_base *s1,
                                        size_t pos, size_t len)
{
  CONF__ASSERT(token);

  if (token->type == CONF_TOKEN_TYPE_SYMBOL)
    return (conf_token_cmp_val_eq(conf, token, s1, pos, len));
  return (CONF__FALSE);
}

extern inline int conf_token_cmp_sym_buf_eq(const Conf_parse *conf,
                                            const Conf_token *token,
                                            const char *buf, size_t len)
{
  CONF__ASSERT(token);
  
  if (token->type == CONF_TOKEN_TYPE_SYMBOL)
    return (conf_token_cmp_val_buf_eq(conf, token, buf, len));
  return (CONF__FALSE);
}

extern inline int conf_token_cmp_sym_cstr_eq(const Conf_parse *conf,
                                             const Conf_token *token,
                                             const char *cstr)
{
  CONF__ASSERT(token);
  
  if (token->type == CONF_TOKEN_TYPE_SYMBOL)
    return (conf_token_cmp_val_cstr_eq(conf, token, cstr));
  return (CONF__FALSE);
}

extern inline int conf_token_cmp_case_sym_cstr_eq(const Conf_parse *conf,
                                                  const Conf_token *token,
                                                  const char *cstr)
{
  CONF__ASSERT(token);

  if (token->type == CONF_TOKEN_TYPE_SYMBOL)
    return (conf_token_cmp_case_val_cstr_eq(conf, token, cstr));
  return (CONF__FALSE);
}

extern inline int conf_token_srch_case_val(const Conf_parse *conf,
                                           const Conf_token *token,
                                           const Vstr_base *s1,
                                           size_t pos, size_t len)
{
  const Vstr_sect_node *val = conf_token_value(token);
  
  CONF__ASSERT(conf && token && conf->data && s1);
  
  if (!val) return (CONF__FALSE);

  return (vstr_srch_case_vstr_fwd(conf->data, val->pos, val->len,
                                  s1, pos, len));
}

extern inline unsigned int conf_token_list_num(const Conf_token *token,
                                               unsigned int depth)
{
  CONF__ASSERT(token);
  
  if (depth > token->depth_num)
    return (CONF__FALSE);

  CONF__ASSERT(token->depth_nums[depth] >= token->num);
  
  return (token->depth_nums[depth] - token->num);
}

extern inline int conf_sc_token_parse_toggle(const Conf_parse *conf,
                                             Conf_token *token, int *val)
{
  unsigned int num = conf_token_list_num(token, token->depth_num);
  int ern = CONF_SC_TYPE_RET_OK;
  
  CONF__ASSERT(val);
  
  if (!num)
  {
    *val = !*val;
    return (ern);
  }

  conf_parse_token(conf, token);
  if (0) { }
  else if (conf_token_cmp_val_cstr_eq(conf, token, "on") ||
           conf_token_cmp_val_cstr_eq(conf, token, "true") ||
           conf_token_cmp_val_cstr_eq(conf, token, "yes") ||
           conf_token_cmp_val_cstr_eq(conf, token, "ON") ||
           conf_token_cmp_val_cstr_eq(conf, token, "TRUE") ||
           conf_token_cmp_val_cstr_eq(conf, token, "YES") ||
           conf_token_cmp_val_cstr_eq(conf, token, "1"))
    *val = CONF__TRUE;
  else if (conf_token_cmp_val_cstr_eq(conf, token, "off") ||
           conf_token_cmp_val_cstr_eq(conf, token, "false") ||
           conf_token_cmp_val_cstr_eq(conf, token, "no") ||
           conf_token_cmp_val_cstr_eq(conf, token, "OFF") ||
           conf_token_cmp_val_cstr_eq(conf, token, "FALSE") ||
           conf_token_cmp_val_cstr_eq(conf, token, "NO") ||
           conf_token_cmp_val_cstr_eq(conf, token, "0"))
    *val = CONF__FALSE;
  else
    ern = CONF_SC_TYPE_RET_ERR_NO_MATCH;

  return (ern);
}

#endif

#endif
