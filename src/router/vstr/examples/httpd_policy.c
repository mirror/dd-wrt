
#include "httpd.h"
#include "httpd_policy.h"

#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "mk.h"

static void httpd__policy_path_ref_free(Vstr_ref *ref)
{
  Httpd_policy_path *path = NULL;

  if (!ref)
    return;
  
  path = ref->ptr;
  vstr_free_base(path->s1);
  (*path->ref_func)(ref);
}

static int httpd__policy_path_init(Httpd_policy_path *path,
                                   Vstr_base *s1, size_t pos, size_t len, 
                                   Vstr_ref *ref)
{
  size_t tmp = 0;
  
  ASSERT(ref->ref == 1);
  
  if (!(s1 = vstr_dup_vstr(s1->conf, s1, pos, len, 0)))
    return (FALSE);
  path->s1 = s1;

  /* remove any double slashes... */
  while ((tmp = vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "//")))
    if (!vstr_del(s1, tmp, 1))
    {
      vstr_free_base(s1);
      return (FALSE);
    }
  
  path->ref_func = ref->func;
  
  ref->func = httpd__policy_path_ref_free;

  return (TRUE);
}

static int httpd_policy__build_parent_path(Vstr_base *s1, Vstr_base *s2)
{
  size_t pos = 0;
  size_t len = 0;

  ASSERT(s1 && s2);
  
  if (!s2->len)
    return (TRUE);
      
  if (s2->len == 1)
  {
    ASSERT(vstr_cmp_cstr_eq(s2, 1, s2->len, "/"));
    vstr_add_cstr_ptr(s1, s1->len, "/");
    return (TRUE);
  }
      
  if (!(pos = vstr_srch_chr_rev(s2, 1, s2->len - 1, '/')))
  {
    vstr_add_cstr_ptr(s1, s1->len, "./");
    return (TRUE);
  }
      
  len = vstr_sc_posdiff(1, pos);
  HTTPD_APP_REF_VSTR(s1, s2, 1, len);

  return (TRUE);
}

/* builds into conf->tmp */
int httpd_policy_build_path(struct Con *con, Httpd_req_data *req,
                            const Conf_parse *conf, Conf_token *token,
                            int *used_policy, int *used_req)
{
  unsigned int cur_depth = token->depth_num;
  int dummy_used_policy;
  int dummy_used_req;

  if (!used_policy) used_policy = &dummy_used_policy;
  if (!used_req)    used_req    = &dummy_used_req;
  *used_policy = *used_req = FALSE;
  
  vstr_del(conf->tmp, 1, conf->tmp->len);
  while (conf_token_list_num(token, cur_depth))
  {
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    
    if (0) { }
    
    else if (OPT_SERV_SYM_EQ("<basename>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_name(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-basename>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_name(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<basename-without-extension>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;

      *used_req = TRUE;
      httpd_policy_path_mod_bwen(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-basename-without-extension>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;

      *used_req = TRUE;
      httpd_policy_path_mod_bwen(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<basename-without-extensions>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;

      *used_req = TRUE;
      httpd_policy_path_mod_bwes(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-basename-without-extensions>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;

      *used_req = TRUE;
      httpd_policy_path_mod_bwes(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<directory-filename>"))
    {
      Vstr_base *s1 = req->policy->dir_filename;
      *used_policy = TRUE;
      HTTPD_APP_REF_ALLVSTR(conf->tmp, s1);
    }
    else if (OPT_SERV_SYM_EQ("<dirname>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_dirn(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-dirname>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_dirn(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<document-root>") ||
             OPT_SERV_SYM_EQ("<doc-root>"))
    {
      Vstr_base *s1 = req->policy->document_root;
      *used_policy = TRUE;
      HTTPD_APP_REF_ALLVSTR(conf->tmp, s1);
    }
    else if (OPT_SERV_SYM_EQ("<document-root/..>") ||
             OPT_SERV_SYM_EQ("<doc-root/..>"))
    {
      ASSERT(req->policy->document_root->len);
      if (!httpd_policy__build_parent_path(conf->tmp,
                                           req->policy->document_root))
        return (FALSE);
      *used_policy = TRUE;
    }
    else if (OPT_SERV_SYM_EQ("<extension>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;

      *used_req = TRUE;
      httpd_policy_path_mod_extn(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-extension>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_extn(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<extensions>"))
    {
      size_t pos = 1;
      size_t len = req->fname->len;

      *used_req = TRUE;
      httpd_policy_path_mod_exts(req->fname, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, req->fname, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<url-extensions>"))
    {
      size_t pos = req->path_pos;
      size_t len = req->path_len;
      
      *used_req = TRUE;
      httpd_policy_path_mod_exts(con->evnt->io_r, &pos, &len);
      HTTPD_APP_REF_VSTR(conf->tmp, con->evnt->io_r, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<hostname>"))
    {
      Vstr_base *http_data = con->evnt->io_r;
      Vstr_sect_node *h_h = req->http_hdrs->hdr_host;
      
      *used_req = TRUE;
      if (h_h->len)
        HTTPD_APP_REF_VSTR(conf->tmp, http_data, h_h->pos, h_h->len);
      else
        httpd_sc_add_default_hostname(con, req, conf->tmp, conf->tmp->len);
    }
    else if (OPT_SERV_SYM_EQ("<request-configuration-directory>") ||
             OPT_SERV_SYM_EQ("<req-conf-dir>"))
    {
      Vstr_base *s1 = req->policy->req_conf_dir;
      *used_policy = TRUE;
      HTTPD_APP_REF_ALLVSTR(conf->tmp, s1);
    }
    else if (OPT_SERV_SYM_EQ("<request-configuration-directory/..>") ||
             OPT_SERV_SYM_EQ("<req-conf-dir/..>"))
    {
      if (!httpd_policy__build_parent_path(conf->tmp,
                                           req->policy->req_conf_dir))
        return (FALSE);
      *used_policy = TRUE;
    }
    else if (OPT_SERV_SYM_EQ("<request-error-directory>") ||
             OPT_SERV_SYM_EQ("<req-err-dir>"))
    {
      Vstr_base *s1 = req->policy->req_err_dir;
      *used_policy = TRUE;
      HTTPD_APP_REF_ALLVSTR(conf->tmp, s1);
    }
    else if (OPT_SERV_SYM_EQ("<request-error-directory/..>") ||
             OPT_SERV_SYM_EQ("<req-err-dir/..>"))
    {
      if (!httpd_policy__build_parent_path(conf->tmp, req->policy->req_err_dir))
        return (FALSE);
      *used_policy = TRUE;
    }
    else if (OPT_SERV_SYM_EQ("<file-path>") || OPT_SERV_SYM_EQ("<path>"))
    {
      *used_req = TRUE;
      HTTPD_APP_REF_ALLVSTR(conf->tmp, req->fname);
    }
    else if (OPT_SERV_SYM_EQ("<url-path>"))
    {
      *used_req = TRUE;
      HTTPD_APP_REF_VSTR(conf->tmp,
                         con->evnt->io_r, req->path_pos, req->path_len);
    }
    else if (OPT_SERV_SYM_EQ("<content-type-extension>") ||
             OPT_SERV_SYM_EQ("<content-type-path>"))
    {
      const Vstr_base *s1 = req->ext_vary_a_vs1;
      size_t pos          = req->ext_vary_a_pos;
      size_t len          = req->ext_vary_a_len;
      
      *used_req = TRUE;
      if (s1 && len)
        HTTPD_APP_REF_VSTR(conf->tmp, s1, pos, len);
    }
    else if (OPT_SERV_SYM_EQ("<content-language-extension>") ||
             OPT_SERV_SYM_EQ("<content-language-path>"))
    {
      const Vstr_base *s1 = req->ext_vary_al_vs1;
      size_t pos          = req->ext_vary_al_pos;
      size_t len          = req->ext_vary_al_len;
      
      *used_req = TRUE;
      if (s1 && len)
        HTTPD_APP_REF_VSTR(conf->tmp, s1, pos, len);
    }
    else
    { /* unknown symbol or string */
      size_t pos = conf->tmp->len + 1;
      const Vstr_sect_node *pv = conf_token_value(token);
      
      if (!pv || !HTTPD_APP_REF_VSTR(conf->tmp, conf->data, pv->pos, pv->len))
        return (FALSE);
      
      OPT_SERV_X__ESC_VSTR(conf->tmp, pos, pv->len);
    }
  }

  /* we "didn't use the policy" if we only have one policy */
  if (!con->policy->s->beg->def_policy->next)
    *used_policy = FALSE;
  /* FIXME: if we are in here inside a group with a test for policy-eq
   * then we can act like we didn't use the policy.
   */
  
  return (TRUE);
}

int httpd_policy_path_make(struct Con *con, Httpd_req_data *req,
                           Conf_parse *conf, Conf_token *token,
                           unsigned int type, Vstr_ref **ret_ref)
{
  Conf_token save = *token;
  Vstr_ref *ref = NULL;
  int ret = FALSE;
  int used_pol = FALSE;
  int used_req = FALSE;
  int clist = FALSE;
  
  ASSERT(ret_ref);
  *ret_ref     = NULL;

  CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
  CONF_SC_TOGGLE_CLIST_VAR(clist);

  if (OPT_SERV_SYM_EQ("assign") || OPT_SERV_SYM_EQ("="))
  {
    if (!httpd_policy_build_path(con, req, conf, token, &used_pol, &used_req))
      return (FALSE);
  }
  else if (clist)
    return (FALSE);
  else
  {
    const Vstr_sect_node *pv = conf_token_value(token);
    Vstr_base *s1 = conf->tmp;
    if (!pv || !vstr_sub_vstr(s1, 1, s1->len, conf->data, pv->pos, pv->len,
                              VSTR_TYPE_SUB_BUF_REF))
      return (FALSE);
    OPT_SERV_X__ESC_VSTR(s1, 1, pv->len);    
  }
  
  if (!(ref = vstr_ref_make_malloc(sizeof(Httpd_policy_path))))
    return (FALSE);

  ret = httpd__policy_path_init(ref->ptr, conf->tmp, 1, conf->tmp->len, ref);
  if (ret && !(used_pol || used_req))
    ret = conf_token_set_user_value(conf, &save, type, ref, token->num);
  
  if (ret)
    *ret_ref = ref;

  return (ret);
}

void httpd_policy_exit(Httpd_policy_opts *opts)
{
  ASSERT(opts);
  
  vstr_free_base(opts->document_root);     opts->document_root      = NULL;
  vstr_free_base(opts->server_name);       opts->server_name        = NULL;
  vstr_free_base(opts->dir_filename);      opts->dir_filename       = NULL;
  vstr_free_base(opts->mime_types_def_ct); opts->mime_types_def_ct  = NULL;
  vstr_free_base(opts->mime_types_main);   opts->mime_types_main    = NULL;
  vstr_free_base(opts->mime_types_xtra);   opts->mime_types_xtra    = NULL;
  vstr_free_base(opts->default_hostname);  opts->default_hostname   = NULL;
  vstr_free_base(opts->req_conf_dir);      opts->req_conf_dir       = NULL;
  vstr_free_base(opts->req_err_dir);       opts->req_err_dir        = NULL;
  vstr_free_base(opts->auth_realm);        opts->auth_realm         = NULL;
  vstr_free_base(opts->auth_token);        opts->auth_token         = NULL;

  opt_policy_exit(opts->s);
}

int httpd_policy_init(Httpd_opts *beg, Httpd_policy_opts *opts)
{ 
  if (!opt_policy_init(beg->s, opts->s))
    return (FALSE);

  ASSERT(opts->s->beg == beg->s);
  
  /* do all, then check ... so we don't have to unwind */
  opts->document_root     = vstr_make_base(NULL);
  opts->server_name       = vstr_make_base(NULL);
  opts->dir_filename      = vstr_make_base(NULL);
  opts->mime_types_def_ct = vstr_make_base(NULL);
  opts->mime_types_main   = vstr_make_base(NULL);
  opts->mime_types_xtra   = vstr_make_base(NULL);
  opts->default_hostname  = vstr_make_base(NULL);
  opts->req_conf_dir      = vstr_make_base(NULL);
  opts->req_err_dir       = vstr_make_base(NULL);
  opts->auth_realm        = vstr_make_base(NULL);
  opts->auth_token        = vstr_make_base(NULL);
  
  if (!opts->document_root     ||
      !opts->server_name       ||
      !opts->dir_filename      ||
      !opts->mime_types_def_ct ||
      !opts->mime_types_main   ||
      !opts->mime_types_xtra   ||
      !opts->default_hostname  ||
      !opts->req_conf_dir      ||
      !opts->req_err_dir       ||
      !opts->auth_realm        ||
      !opts->auth_token        ||
      FALSE)
  {
    httpd_policy_exit(opts);
    return (FALSE);
  }

  opts->mime_types->ref = NULL;
  opts->mime_types->def_type_vs1 = NULL;
  opts->mime_types->def_type_pos = 0;
  opts->mime_types->def_type_len = 0;
  
  opts->use_mmap           = HTTPD_CONF_USE_MMAP;
  opts->use_sendfile       = HTTPD_CONF_USE_SENDFILE;
  opts->use_keep_alive     = HTTPD_CONF_USE_KEEPA;
  opts->use_keep_alive_1_0 = HTTPD_CONF_USE_KEEPA_1_0;
  opts->use_vhosts_name    = HTTPD_CONF_USE_VHOSTS_NAME;
  opts->use_range          = HTTPD_CONF_USE_RANGE;
  opts->use_range_1_0      = HTTPD_CONF_USE_RANGE_1_0;
  opts->use_public_only    = HTTPD_CONF_USE_PUBLIC_ONLY; /* 8th bitfield */
  opts->use_enc_content_replacement = HTTPD_CONF_USE_ENC_CONTENT_REPLACEMENT;

  opts->use_err_406        = HTTPD_CONF_USE_ERR_406;
  opts->use_canonize_host  = HTTPD_CONF_USE_CANONIZE_HOST;
  opts->use_host_err_400   = HTTPD_CONF_USE_HOST_ERR_400;
  opts->use_host_err_chk   = HTTPD_CONF_USE_HOST_ERR_CHK;
  opts->use_x2_hdr_chk     = HTTPD_CONF_USE_x2_HDR_CHK;
  opts->use_trace_op       = HTTPD_CONF_USE_TRACE_OP;
  opts->remove_url_frag    = HTTPD_CONF_USE_REMOVE_FRAG;
  opts->remove_url_query   = HTTPD_CONF_USE_REMOVE_QUERY;
  opts->use_secure_dirs    = HTTPD_CONF_USE_SECURE_DIRS;
  opts->use_friendly_dirs  = HTTPD_CONF_USE_FRIENDLY_DIRS;
  
  opts->use_posix_fadvise  = HTTPD_CONF_USE_POSIX_FADVISE;
  opts->use_tcp_cork       = HTTPD_CONF_USE_TCP_CORK;
  
  opts->use_req_conf       = HTTPD_CONF_USE_REQ_CONF;
  opts->allow_hdr_split    = HTTPD_CONF_USE_ALLOW_HDR_SPLIT; /* 16th bitfield */
  opts->allow_hdr_nil      = HTTPD_CONF_USE_ALLOW_HDR_NIL;
  
  opts->chk_dot_dir        = HTTPD_CONF_USE_CHK_DOT_DIR;
  
  opts->chk_encoded_slash  = HTTPD_CONF_USE_CHK_ENCODED_SLASH;
  opts->chk_encoded_dot    = HTTPD_CONF_USE_CHK_ENCODED_DOT;
  
  opts->add_def_port       = HTTPD_CONF_ADD_DEF_PORT;

  opts->use_non_spc_hdrs   = HTTPD_CONF_USE_NON_SPC_HDRS; /* 22nd bitfield */
  
  opts->max_header_sz      = HTTPD_CONF_INPUT_MAXSZ;

  opts->max_requests       = HTTPD_CONF_MAX_REQUESTS;

  opts->max_neg_A_nodes    = HTTPD_CONF_MAX_NEG_A_NODES;
  opts->max_neg_AL_nodes   = HTTPD_CONF_MAX_NEG_AL_NODES;
  
  opts->max_A_nodes        = HTTPD_CONF_MAX_A_NODES;
  opts->max_AC_nodes       = HTTPD_CONF_MAX_AC_NODES;
  opts->max_AE_nodes       = HTTPD_CONF_MAX_AE_NODES;
  opts->max_AL_nodes       = HTTPD_CONF_MAX_AL_NODES;
  
  opts->max_connection_nodes = HTTPD_CONF_MAX_CONNECTION_NODES;
  opts->max_etag_nodes     = HTTPD_CONF_MAX_ETAG_NODES;

  opts->max_range_nodes    = HTTPD_CONF_MAX_RANGE_NODES;
  
  opts->max_req_conf_sz    = HTTPD_CONF_REQ_CONF_MAXSZ;
  
  return (TRUE);
}

static void httpd_policy_free(Vstr_ref *ref)
{
  Httpd_policy_opts *opts = NULL;
  
  if (!ref)
    return;

  if ((opts = ref->ptr))
    httpd_policy_exit(opts);
  F(opts);
  free(ref);
}

Opt_serv_policy_opts *httpd_policy_make(Opt_serv_opts *beg)
{
  Opt_serv_policy_opts *opts = MK(sizeof(Httpd_policy_opts));
  Vstr_ref *ref = NULL;
  
  if (!opts)
    goto mk_opts_fail;

  if (!(ref = vstr_ref_make_ptr(opts, httpd_policy_free)))
    goto mk_ref_fail;
  opts->ref = ref;

  if (!httpd_policy_init((Httpd_opts *)beg, (Httpd_policy_opts *)opts))
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

#define HTTPD_POLICY_CP_VSTR(x)                               \
    vstr_sub_vstr(dst-> x , 1, dst-> x ->len, src-> x , 1, src-> x ->len, \
                  VSTR_TYPE_SUB_BUF_REF)
#define HTTPD_POLICY_CP_VAL(x)        \
    dst-> x = src-> x

int httpd_policy_copy(Opt_serv_policy_opts *sdst,
                      const Opt_serv_policy_opts *ssrc)
{
  Httpd_policy_opts *dst = (Httpd_policy_opts *)sdst;
  Httpd_policy_opts *src = (Httpd_policy_opts *)ssrc;

  if (!opt_policy_copy(sdst, ssrc))
    return (FALSE);
  
  HTTPD_POLICY_CP_VSTR(document_root);
  HTTPD_POLICY_CP_VSTR(server_name);
  HTTPD_POLICY_CP_VSTR(dir_filename);

  HTTPD_POLICY_CP_VSTR(mime_types_def_ct);
  HTTPD_POLICY_CP_VSTR(mime_types_main);
  HTTPD_POLICY_CP_VSTR(mime_types_xtra);
  HTTPD_POLICY_CP_VSTR(default_hostname);
  HTTPD_POLICY_CP_VSTR(req_conf_dir);
  HTTPD_POLICY_CP_VSTR(req_err_dir);

  HTTPD_POLICY_CP_VAL(use_mmap);
  HTTPD_POLICY_CP_VAL(use_sendfile);
  HTTPD_POLICY_CP_VAL(use_keep_alive);
  HTTPD_POLICY_CP_VAL(use_keep_alive_1_0);
  HTTPD_POLICY_CP_VAL(use_vhosts_name);
  HTTPD_POLICY_CP_VAL(use_range);
  HTTPD_POLICY_CP_VAL(use_range_1_0);
  HTTPD_POLICY_CP_VAL(use_public_only);
  HTTPD_POLICY_CP_VAL(use_enc_content_replacement);
  HTTPD_POLICY_CP_VAL(use_err_406);
  HTTPD_POLICY_CP_VAL(use_canonize_host);
  HTTPD_POLICY_CP_VAL(use_host_err_400);
  HTTPD_POLICY_CP_VAL(use_host_err_chk);
  HTTPD_POLICY_CP_VAL(use_x2_hdr_chk);
  HTTPD_POLICY_CP_VAL(use_trace_op);
  HTTPD_POLICY_CP_VAL(remove_url_frag);
  HTTPD_POLICY_CP_VAL(remove_url_query);
  HTTPD_POLICY_CP_VAL(use_secure_dirs);
  HTTPD_POLICY_CP_VAL(use_friendly_dirs);
  HTTPD_POLICY_CP_VAL(use_posix_fadvise);
  HTTPD_POLICY_CP_VAL(use_tcp_cork);
  HTTPD_POLICY_CP_VAL(use_req_conf);
  HTTPD_POLICY_CP_VAL(allow_hdr_split);
  HTTPD_POLICY_CP_VAL(allow_hdr_nil);
  HTTPD_POLICY_CP_VAL(chk_dot_dir);
  HTTPD_POLICY_CP_VAL(chk_encoded_slash);
  HTTPD_POLICY_CP_VAL(chk_encoded_dot);
  HTTPD_POLICY_CP_VAL(add_def_port);

  HTTPD_POLICY_CP_VAL(use_non_spc_hdrs);

  HTTPD_POLICY_CP_VAL(max_header_sz);
  HTTPD_POLICY_CP_VAL(max_requests);

  HTTPD_POLICY_CP_VAL(max_neg_A_nodes);
  HTTPD_POLICY_CP_VAL(max_neg_AL_nodes);
  
  HTTPD_POLICY_CP_VAL(max_AC_nodes);
  HTTPD_POLICY_CP_VAL(max_AE_nodes);
  HTTPD_POLICY_CP_VAL(max_AL_nodes);
  
  HTTPD_POLICY_CP_VAL(max_connection_nodes);
  HTTPD_POLICY_CP_VAL(max_etag_nodes);

  HTTPD_POLICY_CP_VAL(max_range_nodes);

  HTTPD_POLICY_CP_VAL(max_req_conf_sz);

  return (!dst->document_root->conf->malloc_bad);
}
