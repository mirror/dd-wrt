
#include "httpd.h"
#include "httpd_policy.h"

#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "mk.h"

#define HTTPD_CONF_MAIN_MATCH_CON 1
#define HTTPD_CONF_MAIN_MATCH_REQ 2

#define HTTPD_POLICY_CON_POLICY 3
#define HTTPD_POLICY_REQ_POLICY 4

#define HTTPD_POLICY_CLIENT_IPV4_CIDR_EQ 5
#define HTTPD_POLICY_SERVER_IPV4_CIDR_EQ 6

static int httpd__policy_connection_tst_d1(struct Con *,
                                           Conf_parse *, Conf_token *,
                                           int *);

static int httpd__policy_connection_tst_op_d1(Conf_parse *conf,
                                              Conf_token *token,
                                              int *matches, void *con)
{
  return (httpd__policy_connection_tst_d1(con, conf, token, matches));
}


static int httpd__policy_connection_tst_d1(struct Con *con,
                                           Conf_parse *conf, Conf_token *token,
                                           int *matches)
{
  int clist = FALSE;
  
  ASSERT(con);  
  ASSERT(matches);
  
  CONF_SC_TOGGLE_CLIST_VAR(clist);

  if (token->type >= CONF_TOKEN_TYPE_USER_BEG)
  {
    unsigned int type = token->type - CONF_TOKEN_TYPE_USER_BEG;
    unsigned int nxt = 0;
    Vstr_ref *ref = conf_token_get_user_value(conf, token, &nxt);
    
    switch (type)
    {
      case HTTPD_POLICY_CLIENT_IPV4_CIDR_EQ:
      {
        struct sockaddr *sa = CON_CEVNT_SA(con);
        *matches = httpd_policy_ipv4_cidr_eq(con, NULL, ref->ptr, sa);
      }
      break;
        
      case HTTPD_POLICY_SERVER_IPV4_CIDR_EQ:
      {
        struct sockaddr *sa = CON_SEVNT_SA(con);
        *matches = httpd_policy_ipv4_cidr_eq(con, NULL, ref->ptr, sa);
      }
      break;
        
      default:
        vstr_ref_del(ref);
        return (FALSE);
    }

    vstr_ref_del(ref);
    if (nxt)
      return (conf_parse_num_token(conf, token, nxt));
  }
  
  else if (OPT_SERV_SYM_EQ("policy-eq") || OPT_SERV_SYM_EQ("policy=="))
    return (opt_policy_name_eq(conf, token, con->policy->s, matches));
  else if (OPT_SERV_SYM_EQ("client-ipv4-cidr-eq") ||
           OPT_SERV_SYM_EQ("client-ipv4-cidr=="))
    return (httpd_policy_ipv4_make(con, NULL, conf, token,
                                   HTTPD_POLICY_CLIENT_IPV4_CIDR_EQ,
                                   CON_CEVNT_SA(con), matches));
  else if (OPT_SERV_SYM_EQ("server-ipv4-cidr-eq") ||
           OPT_SERV_SYM_EQ("server-ipv4-cidr=="))
    return (httpd_policy_ipv4_make(con, NULL, conf, token,
                                   HTTPD_POLICY_SERVER_IPV4_CIDR_EQ,
                                   CON_SEVNT_SA(con), matches));
  else if (OPT_SERV_SYM_EQ("server-ipv4-port-eq") ||
           OPT_SERV_SYM_EQ("server-ipv4-port=="))
  {
    struct sockaddr *sa   = CON_SEVNT_SA(con);
    unsigned int tst_port = 0;

    OPT_SERV_X_UINT(tst_port);

    if (sa->sa_family != AF_INET)
      *matches = FALSE;
    else
    {
      struct sockaddr_in *sin = CON_SEVNT_SA_IN4(con);
      *matches = tst_port == ntohs(sin->sin_port);
    }
  }
  
  else
    return (opt_serv_sc_tst(conf, token, matches,
                            httpd__policy_connection_tst_op_d1, con));

  return (TRUE);
}

static const Httpd_policy_opts *httpd__policy_build(struct Con *con,
                                                    Conf_parse *conf,
                                                    Conf_token *token,
                                                    unsigned int utype)
{
  Opt_serv_policy_opts *policy = NULL;
  Vstr_ref *ref = NULL;
  Conf_token save;

  save = *token;
  CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
  if (!(policy = opt_policy_find(con->policy->s->beg, conf, token)))
    return (NULL);
  
  ref = policy->ref;
  if (!conf_token_set_user_value(conf, &save, utype, ref, token->num))
    return (NULL);
  
  return ((const Httpd_policy_opts *)policy);
}

static int httpd__policy_connection_d1(struct Con *con,
                                       Conf_parse *conf, Conf_token *token)
{
  int clist = FALSE;
  
  CONF_SC_TOGGLE_CLIST_VAR(clist);

  if (token->type >= CONF_TOKEN_TYPE_USER_BEG)
  {
    unsigned int type = token->type - CONF_TOKEN_TYPE_USER_BEG;
    unsigned int nxt = 0;
    Vstr_ref *ref = conf_token_get_user_value(conf, token, &nxt);

    switch (type)
    {
      case HTTPD_POLICY_CON_POLICY:
        httpd_policy_change_con(con, ref->ptr);
        break;
        
      default:
        vstr_ref_del(ref);
        return (FALSE);
    }

    vstr_ref_del(ref);
    if (nxt)
      return (conf_parse_num_token(conf, token, nxt));
  }
  
  else if (OPT_SERV_SYM_EQ("<close>"))
  {
    evnt_close(con->evnt);
    return (TRUE);
  }
  else if (OPT_SERV_SYM_EQ("policy"))
  {
    const Httpd_policy_opts *policy = NULL;

    policy = httpd__policy_build(con, conf, token, HTTPD_POLICY_CON_POLICY);
    httpd_policy_change_con(con, policy);
  }
  else if (OPT_SERV_SYM_EQ("Vary:_*"))
    OPT_SERV_X_TOGGLE(con->vary_star);
  
  else
    return (FALSE);
  
  return (TRUE);
}

static int httpd__policy_connection_d0(struct Con *con,
                                       Conf_parse *conf, Conf_token *token)
{
  unsigned int depth = token->depth_num;
  int matches = TRUE;

  CONF_SC_PARSE_SLIST_DEPTH_TOKEN_RET(conf, token, depth, FALSE);
  ++depth;
  while (conf_token_list_num(token, depth))
  {
    CONF_SC_PARSE_DEPTH_TOKEN_RET(conf, token, depth, FALSE);

    if (!httpd__policy_connection_tst_d1(con, conf, token, &matches))
      return (FALSE);

    if (!matches)
      return (TRUE);
  }
  --depth;
  
  while (conf_token_list_num(token, depth))
  {
    CONF_SC_PARSE_DEPTH_TOKEN_RET(conf, token, depth, FALSE);
    
    if (!httpd__policy_connection_d1(con, conf, token))
      return (FALSE);
    if (con->evnt->flag_q_closed) /* don't do anything else */
      return (TRUE);
  }

  return (TRUE);
}

int httpd_policy_connection(struct Con *con,
                            Conf_parse *conf, const Conf_token *beg_token)
{
  Conf_token token[1];
  unsigned int num = 0;
  
  if (!beg_token->num) /* not been parsed */
    return (TRUE);
  
  *token = *beg_token;

  num = token->num;
  while (num)
  {
    if (!conf_parse_num_token(conf, token, num))
      goto conf_fail;
    
    assert(token->type == (CONF_TOKEN_TYPE_USER_BEG+HTTPD_CONF_MAIN_MATCH_CON));

    conf_token_get_user_value(conf, token, &num);
    
    if (!httpd__policy_connection_d0(con, conf, token))
      goto conf_fail;
  }

  vstr_del(conf->tmp, 1, conf->tmp->len);
  return (TRUE);

 conf_fail:
  vstr_del(conf->tmp, 1, conf->tmp->len);
  conf_parse_backtrace(conf->tmp, "<policy-connection>", conf, token);
  return (FALSE);
}

struct Httpd__policy_req_tst_data
{
 struct Con *con;
 Httpd_req_data *req;
};

static int httpd__policy_request_tst_d1(struct Con *, Httpd_req_data *,
                                        Conf_parse *, Conf_token *, int *);

static int httpd__policy_request_tst_op_d1(Conf_parse *conf, Conf_token *token,
                                           int *matches, void *passed_data)
{
  struct Httpd__policy_req_tst_data *data = passed_data;
  struct Con *con = data->con;
  Httpd_req_data *req = data->req;

  return (httpd__policy_request_tst_d1(con, req, conf, token, matches));
}

static int httpd__policy_request_tst_d1(struct Con *con,
                                        Httpd_req_data *req,
                                        Conf_parse *conf, Conf_token *token,
                                        int *matches)
{
  Vstr_base *http_data = con->evnt->io_r;
  int clist = FALSE;
  
  ASSERT(con && req);  
  ASSERT(matches);
  
  CONF_SC_TOGGLE_CLIST_VAR(clist);

  if (token->type >= CONF_TOKEN_TYPE_USER_BEG)
  {
    unsigned int type = token->type - CONF_TOKEN_TYPE_USER_BEG;
    unsigned int nxt = 0;
    Vstr_ref *ref = conf_token_get_user_value(conf, token, &nxt);
    
    switch (type)
    {
      case HTTPD_POLICY_REQ_PATH_BEG:
      case HTTPD_POLICY_REQ_PATH_END:
      case HTTPD_POLICY_REQ_PATH_EQ:
        
      case HTTPD_POLICY_REQ_NAME_BEG:
      case HTTPD_POLICY_REQ_NAME_END:
      case HTTPD_POLICY_REQ_NAME_EQ:
        
      case HTTPD_POLICY_REQ_BWEN_BEG:
      case HTTPD_POLICY_REQ_BWEN_END:
      case HTTPD_POLICY_REQ_BWEN_EQ:
        
      case HTTPD_POLICY_REQ_BWES_BEG:
      case HTTPD_POLICY_REQ_BWES_END:
      case HTTPD_POLICY_REQ_BWES_EQ:
        
      case HTTPD_POLICY_REQ_EXTN_BEG:
      case HTTPD_POLICY_REQ_EXTN_END:
      case HTTPD_POLICY_REQ_EXTN_EQ:
        
      case HTTPD_POLICY_REQ_EXTS_BEG:
      case HTTPD_POLICY_REQ_EXTS_END:
      case HTTPD_POLICY_REQ_EXTS_EQ:
      {
        size_t pos = 1;
        size_t len = req->fname->len;
        unsigned int lim = httpd_policy_path_req2lim(type);

        vstr_ref_add(ref);
        *matches = httpd_policy_path_lim_eq(req->fname, &pos, &len, lim,
                                            req->vhost_prefix_len, ref);
      }
      break;
      
      case HTTPD_POLICY_CLIENT_IPV4_CIDR_EQ:
      {
        struct sockaddr *sa = CON_CEVNT_SA(con);
        *matches = httpd_policy_ipv4_cidr_eq(con, req, ref->ptr, sa);
      }
      break;
        
      case HTTPD_POLICY_SERVER_IPV4_CIDR_EQ:
      {
        struct sockaddr *sa = CON_SEVNT_SA(con);
        *matches = httpd_policy_ipv4_cidr_eq(con, req, ref->ptr, sa);
      }
        
      default:
        vstr_ref_del(ref);
        return (FALSE);
    }

    vstr_ref_del(ref);
    if (nxt)
      return (conf_parse_num_token(conf, token, nxt));
  }

  else if (OPT_SERV_SYM_EQ("policy-eq") || OPT_SERV_SYM_EQ("policy=="))
    return (opt_policy_name_eq(conf, token, req->policy->s, matches));
  else if (OPT_SERV_SYM_EQ("client-ipv4-cidr-eq") ||
           OPT_SERV_SYM_EQ("client-ipv4-cidr=="))
    return (httpd_policy_ipv4_make(con, req, conf, token,
                                   HTTPD_POLICY_CLIENT_IPV4_CIDR_EQ,
                                   CON_CEVNT_SA(con), matches));
  else if (OPT_SERV_SYM_EQ("server-ipv4-cidr-eq") ||
           OPT_SERV_SYM_EQ("server-ipv4-cidr=="))
    return (httpd_policy_ipv4_make(con, req, conf, token,
                                   HTTPD_POLICY_SERVER_IPV4_CIDR_EQ,
                                   CON_SEVNT_SA(con), matches));
  else if (OPT_SERV_SYM_EQ("hostname-eq") || OPT_SERV_SYM_EQ("hostname=="))
  { /* doesn't do escaping because DNS is ASCII */
    Vstr_sect_node *h_h = req->http_hdrs->hdr_host;
    Vstr_base *d_h = req->policy->default_hostname;

    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    if (h_h->len)
      *matches = conf_token_cmp_case_val_eq(conf, token,
                                            http_data, h_h->pos, h_h->len);
    else
      *matches = conf_token_cmp_case_val_eq(conf, token,
                                            d_h, 1, d_h->len);
  }
  else if (OPT_SERV_SYM_EQ("user-agent-eq") || OPT_SERV_SYM_EQ("UA-eq") ||
           OPT_SERV_SYM_EQ("user-agent==") || OPT_SERV_SYM_EQ("UA=="))
  { /* doesn't do escaping because URLs are ASCII */
    Vstr_sect_node *h_ua = req->http_hdrs->hdr_ua;

    req->vary_ua = TRUE;
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = conf_token_cmp_case_val_eq(conf, token, http_data, 1, h_ua->len);
  }
  else if (OPT_SERV_SYM_EQ("user-agent-search-eq") ||
           OPT_SERV_SYM_EQ("user-agent-search==") ||
           OPT_SERV_SYM_EQ("user-agent-srch-eq") ||
           OPT_SERV_SYM_EQ("user-agent-srch==") ||
           OPT_SERV_SYM_EQ("UA-srch-eq") ||
           OPT_SERV_SYM_EQ("UA-srch=="))
  { /* doesn't do escaping because URLs are ASCII */
    Vstr_sect_node *h_ua = req->http_hdrs->hdr_ua;

    req->vary_ua = TRUE;
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = !!conf_token_srch_case_val(conf, token, http_data, 1, h_ua->len);
  }
  else if (OPT_SERV_SYM_EQ("referrer-eq") || OPT_SERV_SYM_EQ("referer-eq") ||
           OPT_SERV_SYM_EQ("referrer==")  || OPT_SERV_SYM_EQ("referer=="))
  { /* doesn't do escaping because URLs are ASCII */
    Vstr_sect_node *h_ref = req->http_hdrs->hdr_referer;

    req->vary_rf = TRUE;
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = conf_token_cmp_case_val_eq(conf, token,
                                          http_data, 1, h_ref->len);
  }
  else if (OPT_SERV_SYM_EQ("referrer-beg") || OPT_SERV_SYM_EQ("referer-beg"))
  { /* doesn't do escaping because URLs are ASCII */
    Vstr_sect_node *h_ref = req->http_hdrs->hdr_referer;

    req->vary_rf = TRUE;
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = conf_token_cmp_case_val_beg_eq(conf, token,
                                              http_data, 1, h_ref->len);
  }
  else if (OPT_SERV_SYM_EQ("referrer-search-eq") ||
           OPT_SERV_SYM_EQ("referrer-search==") ||
           OPT_SERV_SYM_EQ("referrer-srch-eq") ||
           OPT_SERV_SYM_EQ("referrer-srch==") ||
           OPT_SERV_SYM_EQ("referer-search-eq") ||
           OPT_SERV_SYM_EQ("referer-search==") ||
           OPT_SERV_SYM_EQ("referer-srch-eq") ||
           OPT_SERV_SYM_EQ("referer-srch=="))
  { /* doesn't do escaping because URLs are ASCII */
    Vstr_sect_node *h_ref = req->http_hdrs->hdr_referer;

    req->vary_rf = TRUE;
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = !!conf_token_srch_case_val(conf, token,
                                          http_data, 1, h_ref->len);
  }
  else if (OPT_SERV_SYM_EQ("http-0.9-eq") || OPT_SERV_SYM_EQ("http-0.9=="))
    *matches =  req->ver_0_9;
  else if (OPT_SERV_SYM_EQ("http-1.0-eq") || OPT_SERV_SYM_EQ("http-1.0=="))
    *matches = !req->ver_0_9 && !req->ver_1_1;
  else if (OPT_SERV_SYM_EQ("http-1.1-eq") || OPT_SERV_SYM_EQ("http-1.1=="))
    *matches = !req->ver_0_9 &&  req->ver_1_1;
  else if (OPT_SERV_SYM_EQ("method-eq") || OPT_SERV_SYM_EQ("method=="))
  { /* doesn't do escaping because methods are ASCII */
    Vstr_sect_node *meth = VSTR_SECTS_NUM(req->sects, 1);
    
    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    *matches = conf_token_cmp_case_val_eq(conf, token,
                                          http_data, meth->pos, meth->len);
  }
  else if (OPT_SERV_SYM_EQ("tm-dow-eq") || OPT_SERV_SYM_EQ("tm-dow=="))
  { /* so we can do msff :) */
    Opt_serv_opts *opts = req->policy->s->beg;
    Httpd_opts *hopts = (Httpd_opts *)opts;
    const struct tm *tm = date_gmtime(hopts->date, req->now);
    int tmp = 0;

    if (!tm) return (FALSE);

    /* vary_star ... or nothing ? */
    OPT_SERV_X_UINT(tmp);
    *matches = tmp == tm->tm_wday;
  }
  else
  { /* spend time doing path/name/extn/bnwe */
    Vstr_ref *ref = NULL;
    unsigned int type = HTTPD_POLICY_REQ_PATH_BEG;
    unsigned int lim  = 0;
    size_t pos = 1;
    size_t len = req->fname->len;
    
    if (0) { }
    else if (OPT_SERV_SYM_EQ("path-beg"))
      type = HTTPD_POLICY_REQ_PATH_BEG;
    else if (OPT_SERV_SYM_EQ("path-end"))
      type = HTTPD_POLICY_REQ_PATH_END;
    else if (OPT_SERV_SYM_EQ("path-eq") || OPT_SERV_SYM_EQ("path=="))
      type = HTTPD_POLICY_REQ_PATH_EQ;
    else if (OPT_SERV_SYM_EQ("basename-beg"))
      type = HTTPD_POLICY_REQ_NAME_BEG;
    else if (OPT_SERV_SYM_EQ("basename-end"))
      type = HTTPD_POLICY_REQ_NAME_END;
    else if (OPT_SERV_SYM_EQ("basename-eq") || OPT_SERV_SYM_EQ("basename=="))
      type = HTTPD_POLICY_REQ_NAME_EQ;
    else if (OPT_SERV_SYM_EQ("extension-beg"))
      type = HTTPD_POLICY_REQ_EXTN_BEG;
    else if (OPT_SERV_SYM_EQ("extension-end"))
      type = HTTPD_POLICY_REQ_EXTN_END;
    else if (OPT_SERV_SYM_EQ("extension-eq") || OPT_SERV_SYM_EQ("extension=="))
      type = HTTPD_POLICY_REQ_EXTN_EQ;
    else if (OPT_SERV_SYM_EQ("extensions-beg"))
      type = HTTPD_POLICY_REQ_EXTS_BEG;
    else if (OPT_SERV_SYM_EQ("extensions-end"))
      type = HTTPD_POLICY_REQ_EXTS_END;
    else if (OPT_SERV_SYM_EQ("extensions-eq") ||
             OPT_SERV_SYM_EQ("extensions=="))
      type = HTTPD_POLICY_REQ_EXTS_EQ;
    else if (OPT_SERV_SYM_EQ("basename-without-extension-beg"))
      type = HTTPD_POLICY_REQ_BWEN_BEG;
    else if (OPT_SERV_SYM_EQ("basename-without-extension-end"))
      type = HTTPD_POLICY_REQ_BWEN_END;
    else if (OPT_SERV_SYM_EQ("basename-without-extension-eq") ||
             OPT_SERV_SYM_EQ("basename-without-extension=="))
      type = HTTPD_POLICY_REQ_BWEN_EQ;
    else if (OPT_SERV_SYM_EQ("basename-without-extensions-beg"))
      type = HTTPD_POLICY_REQ_BWES_BEG;
    else if (OPT_SERV_SYM_EQ("basename-without-extensions-end"))
      type = HTTPD_POLICY_REQ_BWES_END;
    else if (OPT_SERV_SYM_EQ("basename-without-extensions-eq") ||
             OPT_SERV_SYM_EQ("basename-without-extensions=="))
      type = HTTPD_POLICY_REQ_BWES_EQ;
    else
    {
      struct Httpd__policy_req_tst_data data[1];

      data->con = con;
      data->req = req;
      return (opt_serv_sc_tst(conf, token, matches,
                              httpd__policy_request_tst_op_d1, data));
    }
    
    if (!httpd_policy_path_make(con, req, conf, token, type, &ref))
    {
      vstr_ref_del(ref);
      return (FALSE);
    }
    
    lim = httpd_policy_path_req2lim(type);
    *matches = httpd_policy_path_lim_eq(req->fname, &pos, &len, lim,
                                        req->vhost_prefix_len, ref);
  }

  return (TRUE);
}

static int httpd__policy_request_d1(struct Con *con, struct Httpd_req_data *req,
                                    Conf_parse *conf, Conf_token *token)
{
  int clist = FALSE;
  
  CONF_SC_TOGGLE_CLIST_VAR(clist);

  if (token->type >= CONF_TOKEN_TYPE_USER_BEG)
  {
    unsigned int type = token->type - CONF_TOKEN_TYPE_USER_BEG;
    unsigned int nxt = 0;
    Vstr_ref *ref = conf_token_get_user_value(conf, token, &nxt);
    
    switch (type)
    {
      case HTTPD_POLICY_CON_POLICY:
        httpd_policy_change_con(con, ref->ptr);
        break;
        
      case HTTPD_POLICY_REQ_POLICY:
        httpd_policy_change_req(req, ref->ptr);
        break;
        
      default:
        vstr_ref_del(ref);
        return (FALSE);
    }

    vstr_ref_del(ref);
    if (nxt)
      return (conf_parse_num_token(conf, token, nxt));
  }
  
  else if (OPT_SERV_SYM_EQ("<close>"))
  {
    evnt_close(con->evnt);
    return (TRUE);
  }
  else if (OPT_SERV_SYM_EQ("connection-policy"))
  {
    const Httpd_policy_opts *policy = NULL;

    policy = httpd__policy_build(con, conf, token, HTTPD_POLICY_CON_POLICY);
    httpd_policy_change_con(con, policy);
  }
  else if (OPT_SERV_SYM_EQ("policy"))
  {
    const Httpd_policy_opts *policy = NULL;

    policy = httpd__policy_build(con, conf, token, HTTPD_POLICY_REQ_POLICY);
    httpd_policy_change_req(req, policy);
  }
  else if (OPT_SERV_SYM_EQ("org.and.jhttpd-conf-req-1.0"))
  {
    Httpd_opts *opts = (Httpd_opts *)con->policy->s->beg;
    return (httpd_conf_req_d0(con, req, /* server beg time is "close engouh" */
                              opts->beg_time, conf, token));
  }
  else
    return (FALSE);
  
  return (TRUE);
}

static int httpd__policy_request_d0(struct Con *con, struct Httpd_req_data *req,
                                    Conf_parse *conf, Conf_token *token)
{
  unsigned int depth = token->depth_num;
  int matches = TRUE;

  CONF_SC_PARSE_SLIST_DEPTH_TOKEN_RET(conf, token, depth, FALSE);
  ++depth;
  while (conf_token_list_num(token, depth))
  {
    CONF_SC_PARSE_DEPTH_TOKEN_RET(conf, token, depth, FALSE);

    if (!httpd__policy_request_tst_d1(con, req, conf, token, &matches))
      return (FALSE);

    if (!matches)
      return (TRUE);
  }
  --depth;
  
  while (conf_token_list_num(token, depth))
  {
    CONF_SC_PARSE_DEPTH_TOKEN_RET(conf, token, depth, FALSE);
    
    if (!httpd__policy_request_d1(con, req, conf, token))
      return (FALSE);
    if (con->evnt->flag_q_closed) /* don't do anything else */
      return (TRUE);
  }

  return (TRUE);
}

int httpd_policy_request(struct Con *con, struct Httpd_req_data *req,
                         Conf_parse *conf, const Conf_token *beg_token)
{
  Conf_token token[1];
  unsigned int num = 0;
  
  if (!beg_token->num) /* not been parsed */
    return (TRUE);
  
  *token = *beg_token;

  num = token->num;
  while (num)
  {
    if (!conf_parse_num_token(conf, token, num))
      goto conf_fail;
    
    assert(token->type == (CONF_TOKEN_TYPE_USER_BEG+HTTPD_CONF_MAIN_MATCH_REQ));

    conf_token_get_user_value(conf, token, &num);

    if (!httpd__policy_request_d0(con, req, conf, token))
      goto conf_fail;    
  }

  vstr_del(conf->tmp, 1, conf->tmp->len);
  return (TRUE);

 conf_fail:
  vstr_del(conf->tmp, 1, conf->tmp->len);
  if (!req->user_return_error_code)
  {
    conf_parse_backtrace(conf->tmp, "<policy-request>", conf, token);
    HTTPD_ERR(req, 503);
  }
  return (FALSE);
}

static int httpd__conf_main_policy_http_d1(Httpd_policy_opts *opts,
                                           const Conf_parse *conf,
                                           Conf_token *token)
{
  int clist = FALSE;
  
  CONF_SC_MAKE_CLIST_BEG(policy_http_d1, clist);
  
  else if (OPT_SERV_SYM_EQ("authorization") || OPT_SERV_SYM_EQ("auth"))
  { /* token is output of: echo -n foo:bar | openssl enc -base64 */
    /* see it with: echo token | openssl enc -d -base64 && echo */
    unsigned int depth = token->depth_num;

    CONF_SC_PARSE_TOP_TOKEN_RET(conf, token, FALSE);
    if (!OPT_SERV_SYM_EQ("basic-encoded")) return (FALSE);
    CONF_SC_MAKE_CLIST_MID(depth, clist);

    else if (OPT_SERV_SYM_EQ("realm")) OPT_SERV_X_VSTR(opts->auth_realm);
    else if (OPT_SERV_SYM_EQ("token")) OPT_SERV_X_VSTR(opts->auth_token);
    
    CONF_SC_MAKE_CLIST_END();
  }
  else if (OPT_SERV_SYM_EQ("canonize-host"))
    OPT_SERV_X_TOGGLE(opts->use_canonize_host);
  else if (OPT_SERV_SYM_EQ("check-dot-directory") ||
           OPT_SERV_SYM_EQ("chk-dot-dir") ||
           OPT_SERV_SYM_EQ("chk-.-dir"))
    OPT_SERV_X_TOGGLE(opts->chk_dot_dir);
  else if (OPT_SERV_SYM_EQ("check-double-headr") ||
           OPT_SERV_SYM_EQ("chk-dbl-hdr") ||
           OPT_SERV_SYM_EQ("check-*2-hdr") ||
           OPT_SERV_SYM_EQ("check-*2-header"))
    OPT_SERV_X_TOGGLE(opts->use_x2_hdr_chk);
  else if (OPT_SERV_SYM_EQ("check-encoded-slash") ||
           OPT_SERV_SYM_EQ("chk-enc-/"))
    OPT_SERV_X_TOGGLE(opts->chk_encoded_slash);
  else if (OPT_SERV_SYM_EQ("check-encoded-dot") ||
           OPT_SERV_SYM_EQ("chk-enc-."))
    OPT_SERV_X_TOGGLE(opts->chk_encoded_dot);
  else if (OPT_SERV_SYM_EQ("check-host") ||
           OPT_SERV_SYM_EQ("chk-host"))
    OPT_SERV_X_TOGGLE(opts->use_host_err_chk);
  else if (OPT_SERV_SYM_EQ("error-406"))
    OPT_SERV_X_TOGGLE(opts->use_err_406);
  else if (OPT_SERV_SYM_EQ("error-host-400"))
    OPT_SERV_X_TOGGLE(opts->use_host_err_400);
  else if (OPT_SERV_SYM_EQ("encoded-content-replacement"))
    OPT_SERV_X_TOGGLE(opts->use_enc_content_replacement);
  else if (OPT_SERV_SYM_EQ("header-names-strict"))
    OPT_SERV_X_TOGGLE(opts->use_non_spc_hdrs);
  else if (OPT_SERV_SYM_EQ("keep-alive"))
    OPT_SERV_X_TOGGLE(opts->use_keep_alive);
  else if (OPT_SERV_SYM_EQ("keep-alive-1.0"))
    OPT_SERV_X_TOGGLE(opts->use_keep_alive_1_0);
  else if (OPT_SERV_SYM_EQ("range"))
    OPT_SERV_X_TOGGLE(opts->use_range);
  else if (OPT_SERV_SYM_EQ("range-1.0"))
    OPT_SERV_X_TOGGLE(opts->use_range_1_0);
  else if (OPT_SERV_SYM_EQ("trace-op") || OPT_SERV_SYM_EQ("trace-operation"))
    OPT_SERV_X_TOGGLE(opts->use_trace_op);
  else if (OPT_SERV_SYM_EQ("url-remove-fragment"))
    OPT_SERV_X_TOGGLE(opts->remove_url_frag);
  else if (OPT_SERV_SYM_EQ("url-remove-query"))
    OPT_SERV_X_TOGGLE(opts->remove_url_query);
  
  else if (OPT_SERV_SYM_EQ("limit"))
  {
    CONF_SC_MAKE_CLIST_BEG(limit, clist);
    
    else if (OPT_SERV_SYM_EQ("header-size") ||
             OPT_SERV_SYM_EQ("header-sz"))
      OPT_SERV_X_UINT(opts->max_header_sz);
    else if (OPT_SERV_SYM_EQ("requests"))
      OPT_SERV_X_UINT(opts->max_requests);
    else if (OPT_SERV_SYM_EQ("nodes"))
    {
      CONF_SC_MAKE_CLIST_BEG(nodes, clist);
        
      else if (OPT_SERV_SYM_EQ("Accept:"))
        OPT_SERV_X_UINT(opts->max_A_nodes);
      else if (OPT_SERV_SYM_EQ("Accept-Charset:"))
        OPT_SERV_X_UINT(opts->max_AC_nodes);
      else if (OPT_SERV_SYM_EQ("Accept-Encoding:"))
        OPT_SERV_X_UINT(opts->max_AE_nodes);
      else if (OPT_SERV_SYM_EQ("Accept-Language:"))
        OPT_SERV_X_UINT(opts->max_AL_nodes);
      
      else if (OPT_SERV_SYM_EQ("Connection:"))
        OPT_SERV_X_UINT(opts->max_etag_nodes);
      else if (OPT_SERV_SYM_EQ("ETag:"))
        OPT_SERV_X_UINT(opts->max_etag_nodes);      
      else if (OPT_SERV_SYM_EQ("Range:"))
        OPT_SERV_X_UINT(opts->max_range_nodes);
      
      CONF_SC_MAKE_CLIST_END();
    }
    
    CONF_SC_MAKE_CLIST_END();
  }
  
  CONF_SC_MAKE_CLIST_END();
  
  return (TRUE);
}

static int httpd__conf_main_policy_d1(Httpd_policy_opts *opts,
                                      Conf_parse *conf, Conf_token *token,
                                      int clist)
{
  if (0) { }
  
  else if (OPT_SERV_SYM_EQ("match-init"))
    OPT_SERV_SC_MATCH_INIT(opts->s->beg,
                           httpd__conf_main_policy_d1(opts, conf, token,
                                                      clist));
  
  else if (OPT_SERV_SYM_EQ("directory-filename"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->dir_filename));
  else if (OPT_SERV_SYM_EQ("document-root") ||
           OPT_SERV_SYM_EQ("doc-root"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->document_root));
  else if (OPT_SERV_SYM_EQ("unspecified-hostname"))
    OPT_SERV_X_VSTR(opts->default_hostname);
  else if (OPT_SERV_SYM_EQ("MIME/types-default-type"))
    OPT_SERV_X_VSTR(opts->mime_types_def_ct);
  else if (OPT_SERV_SYM_EQ("MIME/types-filename-main"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->mime_types_main));
  else if (OPT_SERV_SYM_EQ("MIME/types-filename-extra") ||
           OPT_SERV_SYM_EQ("MIME/types-filename-xtra"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->mime_types_xtra));
  else if (OPT_SERV_SYM_EQ("request-configuration-directory") ||
           OPT_SERV_SYM_EQ("req-conf-dir"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->req_conf_dir));
  else if (OPT_SERV_SYM_EQ("request-error-directory") ||
           OPT_SERV_SYM_EQ("req-err-dir"))
    return (opt_serv_sc_make_static_path(opts->s->beg, conf, token,
                                         opts->req_err_dir));
  else if (OPT_SERV_SYM_EQ("server-name"))
    OPT_SERV_X_VSTR(opts->server_name);

  else if (OPT_SERV_SYM_EQ("secure-directory-filename"))
    OPT_SERV_X_TOGGLE(opts->use_secure_dirs);
  else if (OPT_SERV_SYM_EQ("redirect-filename-directory"))
    OPT_SERV_X_TOGGLE(opts->use_friendly_dirs);
  else if (OPT_SERV_SYM_EQ("mmap"))
    OPT_SERV_X_TOGGLE(opts->use_mmap);
  else if (OPT_SERV_SYM_EQ("sendfile"))
    OPT_SERV_X_TOGGLE(opts->use_sendfile);
  else if (OPT_SERV_SYM_EQ("virtual-hosts") ||
           OPT_SERV_SYM_EQ("vhosts") ||
           OPT_SERV_SYM_EQ("virtual-hosts-name") ||
           OPT_SERV_SYM_EQ("vhosts-name"))
    OPT_SERV_X_TOGGLE(opts->use_vhosts_name);
  else if (OPT_SERV_SYM_EQ("public-only"))
    OPT_SERV_X_TOGGLE(opts->use_public_only);
  else if (OPT_SERV_SYM_EQ("posix-fadvise"))
    OPT_SERV_X_TOGGLE(opts->use_posix_fadvise);
  else if (OPT_SERV_SYM_EQ("tcp-cork"))
    OPT_SERV_X_TOGGLE(opts->use_tcp_cork);
  else if (OPT_SERV_SYM_EQ("allow-request-configuration"))
    OPT_SERV_X_TOGGLE(opts->use_req_conf);
  else if (OPT_SERV_SYM_EQ("allow-header-splitting"))
    OPT_SERV_X_TOGGLE(opts->allow_hdr_split);
  else if (OPT_SERV_SYM_EQ("allow-header-NIL"))
    OPT_SERV_X_TOGGLE(opts->allow_hdr_nil);
  else if (OPT_SERV_SYM_EQ("unspecified-hostname-append-port"))
    OPT_SERV_X_TOGGLE(opts->add_def_port);

  else if (OPT_SERV_SYM_EQ("limit"))
  {
    CONF_SC_MAKE_CLIST_BEG(limit, clist);
    
    else if (OPT_SERV_SYM_EQ("request-configuration-size") ||
             OPT_SERV_SYM_EQ("request-configuration-sz") ||
             OPT_SERV_SYM_EQ("req-conf-size") ||
             OPT_SERV_SYM_EQ("req-conf-sz"))
      OPT_SERV_X_UINT(opts->max_req_conf_sz);
    else if (OPT_SERV_SYM_EQ("nodes"))
    {
      CONF_SC_MAKE_CLIST_BEG(nodes, clist);
        
      else if (OPT_SERV_SYM_EQ("Accept:"))
        OPT_SERV_X_UINT(opts->max_neg_A_nodes);
      else if (OPT_SERV_SYM_EQ("Accept-Language:"))
        OPT_SERV_X_UINT(opts->max_neg_AL_nodes);

      CONF_SC_MAKE_CLIST_END();
    }
    
    CONF_SC_MAKE_CLIST_END();
  }

  else if (OPT_SERV_SYM_EQ("HTTP") || OPT_SERV_SYM_EQ("http"))
    return (httpd__conf_main_policy_http_d1(opts, conf, token));
  
  else
    return (FALSE);
  
  return (TRUE);
}

static int httpd__conf_main_policy(Httpd_opts *opts,
                                   Conf_parse *conf, Conf_token *token)
{
  Opt_serv_policy_opts *popts = NULL;
  unsigned int cur_depth = opt_policy_sc_conf_parse(opts->s, conf, token,
                                                    &popts);
  int clist = FALSE;
  
  if (!cur_depth)
    return (FALSE);
  
  CONF_SC_MAKE_CLIST_MID(cur_depth, clist);
  
  else if (httpd__conf_main_policy_d1((Httpd_policy_opts *)popts, conf, token,
                                      clist))
  { }
  
  CONF_SC_MAKE_CLIST_END();
  
  return (TRUE);
}

static int httpd__conf_main_d1(Httpd_opts *httpd_opts,
                               Conf_parse *conf, Conf_token *token, int clist)
{
  if (OPT_SERV_SYM_EQ("org.and.daemon-conf-1.0"))
  {
    if (!opt_serv_conf(httpd_opts->s, conf, token))
      return (FALSE);
  }
  
  else if (OPT_SERV_SYM_EQ("match-init"))
    OPT_SERV_SC_MATCH_INIT(httpd_opts->s,
                           httpd__conf_main_d1(httpd_opts, conf, token, clist));
  
  else if (OPT_SERV_SYM_EQ("policy"))
  {
    if (!httpd__conf_main_policy(httpd_opts, conf, token))
      return (FALSE);
  }
  
  else if (OPT_SERV_SYM_EQ("match-connection"))
  {
    if (!conf_token_set_user_value(conf, token,
                                   HTTPD_CONF_MAIN_MATCH_CON, NULL, 0))
      return (FALSE);
    
    if (!httpd_opts->match_connection->num)
      *httpd_opts->match_connection = *token;
    else /* already have one, add this to end... */
      conf_token_set_user_value(conf, httpd_opts->tmp_match_connection,
                                HTTPD_CONF_MAIN_MATCH_CON, NULL, token->num);
    *httpd_opts->tmp_match_connection = *token;
    
    conf_parse_end_token(conf, token, token->depth_num);
  }
  else if (OPT_SERV_SYM_EQ("match-request"))
  {
    if (!conf_token_set_user_value(conf, token,
                                   HTTPD_CONF_MAIN_MATCH_REQ, NULL, 0))
      return (FALSE);
    
    if (!httpd_opts->match_request->num)
      *httpd_opts->match_request = *token;
    else /* already have one, add this to end... */
      conf_token_set_user_value(conf, httpd_opts->tmp_match_request,
                                HTTPD_CONF_MAIN_MATCH_REQ, NULL, token->num);
    *httpd_opts->tmp_match_request = *token;
    
    conf_parse_end_token(conf, token, token->depth_num);
  }
  
  else
    return (FALSE);
  
  return (TRUE);
}

int httpd_conf_main(Httpd_opts *opts, Conf_parse *conf, Conf_token *token)
{
  unsigned int cur_depth = token->depth_num;
  int clist = FALSE;
  
  if (!OPT_SERV_SYM_EQ("org.and.jhttpd-conf-main-1.0"))
    return (FALSE);

  CONF_SC_MAKE_CLIST_MID(cur_depth, clist);

  else if (httpd__conf_main_d1(opts, conf, token, clist))
  { }
  
  CONF_SC_MAKE_CLIST_END();
  
  /* And they all live together ... dum dum */
  if (conf->data->conf->malloc_bad)
    return (FALSE);
  
  return (TRUE);
}

#define HTTPD_CONF__BEG_APP() do {                                      \
      ASSERT(!opts->conf_num || (conf->state == CONF_PARSE_STATE_END)); \
      prev_conf_num = conf->sects->num;                                 \
      /* reinit */                                                      \
      conf->state = CONF_PARSE_STATE_BEG;                               \
    } while (FALSE)

/* restore the previous parsing we've done and skip parsing it again */
#define HTTPD_CONF__END_APP() do {                                      \
      if (opts->conf_num)                                               \
      {                                                                 \
        conf_parse_token(conf, token);                                  \
        conf_parse_num_token(conf, token, prev_conf_num);               \
      }                                                                 \
    } while (FALSE)
    

int httpd_conf_main_parse_cstr(Vstr_base *out,
                               Httpd_opts *opts, const char *data)
{
  Conf_parse *conf    = opts->conf;
  Conf_token token[1] = {CONF_TOKEN_INIT};
  unsigned int prev_conf_num = 0;
  size_t pos = 1;
  size_t len = 0;

  ASSERT(opts && data);

  if (!conf && !(conf = conf_parse_make(NULL)))
    goto conf_malloc_fail;

  pos = conf->data->len + 1;
  if (!vstr_add_cstr_ptr(conf->data, conf->data->len,
                         "(org.and.jhttpd-conf-main-1.0 "))
    goto read_malloc_fail;
  if (!vstr_add_cstr_ptr(conf->data, conf->data->len, data))
    goto read_malloc_fail;
  if (!vstr_add_cstr_ptr(conf->data, conf->data->len,
                         ")"))
    goto read_malloc_fail;
  len = vstr_sc_posdiff(pos, conf->data->len);

  HTTPD_CONF__BEG_APP();
  
  if (!conf_parse_lex(conf, pos, len))
    goto conf_fail;

  HTTPD_CONF__END_APP();

  if (!conf_parse_token(conf, token))
    goto conf_fail;

  if ((token->type != CONF_TOKEN_TYPE_CLIST) || (token->depth_num != 1))
    goto conf_fail;
    
  if (!conf_parse_token(conf, token))
    goto conf_fail;

  ASSERT(OPT_SERV_SYM_EQ("org.and.jhttpd-conf-main-1.0"));
  
  if (!httpd_conf_main(opts, conf, token))
    goto conf_fail;

  if (token->num != conf->sects->num)
    goto conf_fail;

  opts->conf = conf;
  opts->conf_num++;
  
  return (TRUE);
  
 conf_fail:
  conf_parse_backtrace(out, data, conf, token);
 read_malloc_fail:
  conf_parse_free(conf);
 conf_malloc_fail:
  return (FALSE);
}

int httpd_conf_main_parse_file(Vstr_base *out,
                               Httpd_opts *opts, const char *fname)
{
  Conf_parse *conf    = opts->conf;
  Conf_token token[1] = {CONF_TOKEN_INIT};
  unsigned int prev_conf_num = 0;
  size_t pos = 1;
  size_t len = 0;
  
  ASSERT(opts && fname);

  if (!conf && !(conf = conf_parse_make(NULL)))
    goto conf_malloc_fail;

  pos = conf->data->len + 1;
  if (!vstr_sc_read_len_file(conf->data, conf->data->len, fname, 0, 0, NULL))
    goto read_malloc_fail;
  len = vstr_sc_posdiff(pos, conf->data->len);

  HTTPD_CONF__BEG_APP();
  
  if (!conf_parse_lex(conf, pos, len))
    goto conf_fail;

  HTTPD_CONF__END_APP();
  
  while (conf_parse_token(conf, token))
  {
    if ((token->type != CONF_TOKEN_TYPE_CLIST) || (token->depth_num != 1))
      goto conf_fail;
    
    if (!conf_parse_token(conf, token))
      goto conf_fail;
    
    if (OPT_SERV_SYM_EQ("org.and.daemon-conf-1.0"))
    {
      if (!opt_serv_conf(opts->s, conf, token))
        goto conf_fail;
    }
    else if (OPT_SERV_SYM_EQ("org.and.jhttpd-conf-main-1.0"))
    {
      if (!httpd_conf_main(opts, conf, token))
        goto conf_fail;
    }
    else
      goto conf_fail;
    
    opts->conf_num++;
  }

  opts->conf = conf;
  
  return (TRUE);
  
 conf_fail:
  conf_parse_backtrace(out, fname, conf, token);
  errno = 0;
 read_malloc_fail:
  if (errno && out) /* can't find config. file */
    vstr_add_fmt(out, out->len, "open(%s): %m", fname);
  
  conf_parse_free(conf);
 conf_malloc_fail:
  return (FALSE);
}

void httpd_conf_main_free(Httpd_opts *opts)
{
  Opt_serv_policy_opts *scan = opts->s->def_policy;

  while (scan)
  {
    Httpd_policy_opts *tmp = (Httpd_policy_opts *)scan;
    Opt_serv_policy_opts *scan_next = scan->next;
    
    mime_types_exit(tmp->mime_types);
    
    scan = scan_next;
  }
  
  opt_policy_sc_all_ref_del(opts->s);
  conf_parse_free(opts->conf); opts->conf = NULL;
  date_free(opts->date); opts->date = NULL;
  
  opt_serv_conf_free(opts->s);
}

int httpd_conf_main_init(Httpd_opts *httpd_opts)
{
  Httpd_policy_opts *opts = NULL;

  if (!opt_serv_conf_init(httpd_opts->s))
    goto opts_init_fail;
  
  opts = (Httpd_policy_opts *)httpd_opts->s->def_policy;
  
  vstr_add_cstr_ptr(opts->server_name,       0, HTTPD_CONF_DEF_SERVER_NAME);
  vstr_add_cstr_ptr(opts->dir_filename,      0, HTTPD_CONF_DEF_DIR_FILENAME);
  vstr_add_cstr_ptr(opts->mime_types_def_ct, 0, HTTPD_CONF_MIME_TYPE_DEF_CT);
  vstr_add_cstr_ptr(opts->mime_types_main,   0, HTTPD_CONF_MIME_TYPE_MAIN);
  vstr_add_cstr_ptr(opts->mime_types_xtra,   0, HTTPD_CONF_MIME_TYPE_XTRA);

  if (!(httpd_opts->date = date_make()))
    goto httpd_init_fail;
  
  if (opts->s->policy_name->conf->malloc_bad)
    goto httpd_init_fail;
  
  return (TRUE);

 httpd_init_fail:
  httpd_conf_main_free(httpd_opts);
 opts_init_fail:
  return (FALSE);
}
