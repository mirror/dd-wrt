
/* this parses and dumps a conf.c configuration ... */

#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_LIMIT 1
#define EX_UTILS_NO_USE_OPEN  1
#include "ex_utils.h"

#include "conf.h"

#include "mk.h"

MALLOC_CHECK_DECL();

int main(int argc, char *argv[])
{
  Vstr_base *out = ex_init(NULL);
  Vstr_base *s1  = NULL;
  Conf_parse *conf    = conf_parse_make(NULL);
  Conf_token token[1] = {CONF_TOKEN_INIT};
  int ret = 0;
  
  if (argc != 2)
    errx(EXIT_FAILURE, "args");

  if (!conf)
    errx(EXIT_FAILURE, "conf_parse_make()");
  
  s1 = conf->data;
  if (!vstr_sc_read_len_file(s1, 0, argv[1], 0, 0, NULL))
    errx(EXIT_FAILURE, "read(%s)", argv[1]);

  if (!conf_parse_lex(conf, 1, conf->data->len))
  {
    conf_parse_backtrace(out, argv[1], conf, token);
    vstr_add_cstr_ptr(out, out->len, "\n");
  }
  else
  while (conf_parse_token(conf, token))
  {
    const Vstr_sect_node *val = NULL;
    
    if (token->type > CONF_TOKEN_TYPE_SYMBOL)
    {
      vstr_add_rep_chr(out, out->len, ' ', token->depth_num << 1);
      vstr_add_fmt(out, out->len, "%u. (%s %d)\n",
                   token->num, conf_token_name(token),
                   token->type - CONF_TOKEN_TYPE_SYMBOL);
    }
    else if (!(val = conf_token_value(token)))
    {
      vstr_add_rep_chr(out, out->len, ' ', (token->depth_num - 1) << 1);
      vstr_add_fmt(out, out->len, "%u. [%s]\n",
                   token->num, conf_token_name(token));
    }
    else
    {
      vstr_add_rep_chr(out, out->len, ' ', token->depth_num << 1);
      vstr_add_fmt(out, out->len, "%u. <%s> = ",
                   token->num, conf_token_name(token));
      vstr_add_vstr(out, out->len, s1, val->pos, val->len, 0);
      vstr_add_cstr_buf(out, out->len, "\n");
    }
  }

  if (out->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "print");
  
  io_put_all(out, STDOUT_FILENO);

  conf_parse_free(conf);
  
  ret = ex_exit(out, NULL);

  MALLOC_CHECK_EMPTY();

  exit (ret);
}
