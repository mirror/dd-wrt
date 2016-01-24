#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "opt_policy.h"

#include "mk.h"


void opt_policy_exit(Opt_serv_policy_opts *opts)
{
  ASSERT(opts);
  
  vstr_free_base(opts->policy_name); opts->policy_name = NULL;
  opts->beg = NULL;
}

int opt_policy_init(Opt_serv_opts *beg_opts, Opt_serv_policy_opts *opts)
{
  ASSERT(beg_opts);

  opts->policy_name = vstr_make_base(NULL);
  if (!opts->policy_name ||
      FALSE)
  {
    opt_policy_exit(opts);
    return (FALSE);
  }

  opts->idle_timeout    = OPT_SERV_CONF_DEF_IDLE_TIMEOUT;
  opts->max_connections = OPT_SERV_CONF_DEF_MAX_CONNECTIONS;
  
  opts->beg  = beg_opts;
  opts->next = NULL;
  
  return (TRUE);
}

static void opt_policy_free(Vstr_ref *ref)
{
  Opt_serv_policy_opts *opts = NULL;
  
  if (!ref)
    return;

  if ((opts = ref->ptr))
    opt_policy_exit(opts);
  F(opts);
  free(ref);
}

Opt_serv_policy_opts *opt_policy_make(Opt_serv_opts *beg)
{
  Opt_serv_policy_opts *opts = MK(sizeof(Opt_serv_policy_opts));
  Vstr_ref *ref = NULL;
  
  if (!opts)
    goto mk_opts_fail;

  if (!(ref = vstr_ref_make_ptr(opts, opt_policy_free)))
    goto mk_ref_fail;
  opts->ref = ref;
  
  if (!opt_policy_init(beg, opts))
    goto policy_init_fail;
  
  return (opts);

 policy_init_fail:
  ref->ptr = NULL;
  vstr_ref_del(ref);
 mk_ref_fail:
  F(opts);
 mk_opts_fail:
  return (NULL);
}

void opt_policy_add(Opt_serv_opts *beg, Opt_serv_policy_opts *opts)
{
  Opt_serv_policy_opts **ins = NULL;
  
  opts->next = beg->def_policy;
  ins = &opts->next; /* hacky, fix when def_policy is a real pointer */
  ASSERT(*ins);
  
  while ((ins = &(*ins)->next) && *ins)
  {
    Vstr_base *s1 = opts->policy_name;
    Vstr_base *s2 = (*ins)->policy_name;

    ASSERT(!(*ins)->next ||
           ((*ins)->policy_name->len < (*ins)->next->policy_name->len) ||
           (((*ins)->policy_name->len == (*ins)->next->policy_name->len) &&
            vstr_cmp((*ins)->policy_name, 1, (*ins)->policy_name->len,
                     (*ins)->next->policy_name, 1,
                     (*ins)->next->policy_name->len) < 0));
    
    if (s1->len > s2->len)
      continue;
    if (s1->len < s2->len)
      break;
    if (vstr_cmp(s1, 1, s1->len, s2, 1, s2->len) <= 0)
      break;
  }
  ASSERT(ins != &opts->next);
  opts->next = *ins;
  *ins = opts;
}

int opt_policy_ipv4_make(Conf_parse *conf, Conf_token *token,
                         unsigned int type, struct sockaddr *sa, int *matches)
{
  Conf_token save = *token;
  Vstr_ref *ref = NULL;
  Opt_policy_ipv4 *data = NULL;
  int ret = FALSE;
  
  CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
  
  if (!(ref = vstr_ref_make_malloc(sizeof(Opt_policy_ipv4))))
    return (FALSE);
  data = ref->ptr;
  
  ret = vstr_parse_ipv4(conf->data, token->u.node->pos, token->u.node->len,
                        data->ipv4, &data->cidr,
                        VSTR_FLAG05(PARSE_IPV4, CIDR, CIDR_FULL,
                                    NETMASK, NETMASK_FULL, ONLY), NULL, NULL);

  if (ret)
  {
    *matches = opt_policy_ipv4_cidr_eq(data, sa);
    
    ret = conf_token_set_user_value(conf, &save, type, ref, token->num);
  }
  
  vstr_ref_del(ref);

  return (!!ret);
}

int opt_policy_ipv4_cidr_eq(Opt_policy_ipv4 *data, struct sockaddr *sa)
{
  struct sockaddr_in *sa_in = NULL;
  uint32_t tst_addr_ipv4;
  unsigned char tst_ipv4[4];
  unsigned int scan = 0;
  unsigned int cidr = data->cidr;
  
  ASSERT(cidr <= 32);

  if (!sa || (sa->sa_family != AF_INET))
    return (FALSE);
  sa_in = (struct sockaddr_in *)sa;
    
  tst_addr_ipv4 = ntohl(sa_in->sin_addr.s_addr);
    
  tst_ipv4[3] = (tst_addr_ipv4 >>  0) & 0xFF;
  tst_ipv4[2] = (tst_addr_ipv4 >>  8) & 0xFF;
  tst_ipv4[1] = (tst_addr_ipv4 >> 16) & 0xFF;
  tst_ipv4[0] = (tst_addr_ipv4 >> 24) & 0xFF;

  scan = 0;
  while (cidr >= 8)
  {
    if (tst_ipv4[scan] != data->ipv4[scan])
      return (FALSE);
    
    ++scan;
    cidr -= 8;
  }
  ASSERT(!cidr || (scan < 4));
  
  if (cidr)
  { /* x/7 == (1 << 7) - 1 == 0b0111_1111 */
    unsigned int mask = ((1 << cidr) - 1) << (8 - cidr);
    if ((tst_ipv4[scan] & mask) != (data->ipv4[scan] & mask))
      return (FALSE);
  }

  return (TRUE);
}

Opt_serv_policy_opts *opt_policy_sc_conf_make(Opt_serv_opts *opts,
                                              const Conf_parse *conf,
                                              const Conf_token *token,
                                              const Vstr_sect_node *pv)
{
  Opt_serv_policy_opts *popts = opts->make_policy(opts);
  Vstr_base *name = NULL;

  ASSERT(conf && token && pv);
  
  if (!popts)
    goto init_failure;

  if (!(*opts->copy_policy)(popts, opts->def_policy))
    goto copy_failure;

  ASSERT(pv);

  name = popts->policy_name;
  if (!vstr_sub_vstr(name, 1, name->len, conf->data, pv->pos, pv->len,
                     VSTR_TYPE_SUB_BUF_REF))
    goto name_failure;
  if ((token->type == CONF_TOKEN_TYPE_QUOTE_ESC_D) ||
      (token->type == CONF_TOKEN_TYPE_QUOTE_ESC_DDD) ||
      (token->type == CONF_TOKEN_TYPE_QUOTE_ESC_S) ||
      (token->type == CONF_TOKEN_TYPE_QUOTE_ESC_SSS) ||
      FALSE)
    if (!conf_sc_conv_unesc(name, 1, pv->len, NULL))
      goto name_failure;
  
  opt_policy_add(opts, popts);
  
  return (popts);
  
 name_failure:
 copy_failure:
  vstr_ref_del(popts->ref);
 init_failure:
  return (NULL);
}

unsigned int opt_policy_sc_conf_parse(Opt_serv_opts *opts,
                                      const Conf_parse *conf, Conf_token *token,
                                      Opt_serv_policy_opts **ret_popts)
{
  Opt_serv_policy_opts *popts = NULL;
  unsigned int cur_depth = token->depth_num;
  Conf_token save;
  const Vstr_sect_node *pv = NULL;
  int created_now = FALSE;
  
  ASSERT(opts && opts->def_policy && ret_popts);

  *ret_popts = NULL;
  
  /* name first */
  CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
  
  if (!(pv = conf_token_value(token)))
    return (0);
      
  if (!(popts = opt_policy_find(opts, conf, token)))
  {
    if (!(popts = opt_policy_sc_conf_make(opts, conf, token, pv)))
      return (0);
    created_now = TRUE;
  }

  save = *token;
  if (!conf_parse_token(conf, token) || (token->depth_num < cur_depth))
    return (cur_depth);
  if (token->type != CONF_TOKEN_TYPE_SLIST)
    *token = save; /* restore ... */
  else
  { /* allow set of attributes */
    int clist = FALSE;
    
    CONF_SC_MAKE_CLIST_BEG(policy, clist);

    else if (OPT_SERV_SYM_EQ("inherit"))
    {
      const Opt_serv_policy_opts *frm_opts = NULL;
      CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, 0);
      if (!(frm_opts = opt_policy_find(opts, conf, token)))
        return (0);
      if (created_now && !(*opts->copy_policy)(popts, frm_opts))
        return (0);
    }
    else if (OPT_SERV_SYM_EQ("copy"))
    {
      const Opt_serv_policy_opts *frm_opts = NULL;
      CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, 0);
      if (!(frm_opts = opt_policy_find(opts, conf, token)))
        return (0);
      if (!(*opts->copy_policy)(popts, frm_opts))
        return (0);
    }
    
    CONF_SC_MAKE_CLIST_END();
  }

  *ret_popts = popts;
  return (cur_depth);
}

void opt_policy_sc_all_ref_del(Opt_serv_opts *opts)
{
  Opt_serv_policy_opts *scan = opts->def_policy;

  opts->def_policy = NULL;
  while (scan)
  {
    Opt_serv_policy_opts *scan_next = scan->next;
    
    vstr_ref_del(scan->ref);
    
    scan = scan_next;
  }
}

