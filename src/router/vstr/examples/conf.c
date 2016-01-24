/*
 *  Copyright (C) 2005  James Antill
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
/* configuration file functions */

#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "conf.h"

#include <limits.h>

#include "mk.h"

#define CLEN VSTR__AT_COMPILE_STRLEN

#define VPREFIX(vstr, p, l, cstr)                                       \
    (((l) >= CLEN(cstr)) &&                                             \
     vstr_cmp_buf_eq(vstr, p, CLEN(cstr), cstr, CLEN(cstr)))

Conf_parse *conf_parse_make(Vstr_conf *conf)
{
  Conf_parse *ret   = MK(sizeof(Conf_parse));
  Vstr_sects *sects = vstr_sects_make(4);
  Vstr_base *s1     = vstr_make_base(conf);
  Vstr_base *s2     = vstr_make_base(conf);
  unsigned int *ptr = MK(sizeof(unsigned int) * 4);

  if (!ret || !s1 || !sects || !ptr)
  {
    F(ret);
    vstr_sects_free(sects);
    vstr_free_base(s1);
    vstr_free_base(s2);
    F(ptr);
    return (NULL);
  }

  ret->sects     = sects;
  ret->data      = s1;
  ret->tmp       = s2;
  ret->types_sz  = 4;
  ret->types_ptr = ptr;
  ret->uvals_sz  = 0;
  ret->uvals_num = 0;
  ret->uvals_ptr = NULL;
  ret->state     = CONF_PARSE_STATE_BEG;
  ret->depth     = 0;
  
  return (ret);
}

void conf_parse_free(Conf_parse *conf)
{
  if (!conf)
    return;
  
  F(conf->types_ptr);
  vstr_sects_free(conf->sects);
  vstr_free_base(conf->data);
  vstr_free_base(conf->tmp);

  while (conf->uvals_num)
    vstr_ref_del(conf->uvals_ptr[--conf->uvals_num].ref);
  F(conf->uvals_ptr);
  
  F(conf);  
}

static int conf__parse_type_add(Conf_parse *conf, unsigned int type)
{
  unsigned int *ptr = NULL;

  ASSERT(conf->types_sz <= conf->sects->sz);
  
  if (conf->types_sz == conf->sects->sz)
    goto fin_ok;

  if (!MV(conf->types_ptr, ptr, sizeof(unsigned int) * conf->sects->sz))
    return (FALSE);
  conf->types_sz  = conf->sects->sz;
  
 fin_ok:
  conf->types_ptr[conf->sects->num - 1] = type;
  
  return (TRUE);
}


static size_t conf__parse_ws(Conf_parse *conf, size_t pos, size_t len)
{
  conf->state = CONF_PARSE_STATE_CHOOSE;
  return (vstr_spn_cstr_chrs_fwd(conf->data, pos, len, " \t\v\r\n"));
}

/* if the last token is a line comment token ... comment out that line */
static size_t conf__parse_comment(Conf_parse *conf, size_t pos, size_t len)
{
  Vstr_sect_node *node = NULL;

  ASSERT(conf->sects);
  
  if (conf->sects->malloc_bad)
    return (0);

  ASSERT(conf->sects->num);
  
  node = VSTR_SECTS_NUM(conf->sects, conf->sects->num);
              
  if (node->len >= 1)
  {
    Vstr_base *data = conf->data;
    int byte = vstr_export_chr(data, node->pos);
    size_t p = node->pos;
    size_t l = node->len;
    
    if (((byte == ';') && (vstr_spn_cstr_chrs_fwd(data, p, l, ";") == l)) ||
        ((byte == '#') && (vstr_spn_cstr_chrs_fwd(data, p, l, "#") == l)))
    {
      vstr_sects_del(conf->sects, conf->sects->num);
      conf->state = CONF_PARSE_STATE_CHOOSE;
      return (vstr_cspn_cstr_chrs_fwd(conf->data, pos, len, "\n"));
    }
  }
  
  return (0);
}

static int conf__parse_beg_list(Conf_parse *conf, size_t pos,
                                unsigned int *list_nums)
{
  if (conf->depth >= CONF_PARSE_LIST_DEPTH_SZ)
    return (FALSE);
  
  vstr_sects_add(conf->sects, pos, 0);
  conf__parse_type_add(conf, CONF_TOKEN_TYPE_ERR);
  list_nums[conf->depth++] = conf->sects->num;
  
  conf->state = CONF_PARSE_STATE_CHOOSE;
  return (TRUE);
}

static int conf__parse_end_list(Conf_parse *conf, unsigned int *list_nums,
                                int byte)
{
  Vstr_sects *sects = conf->sects;
  unsigned int depth_beg_num = 0;
  Vstr_sect_node *node = NULL;
  
  if (!conf->depth)
    return (FALSE);

  depth_beg_num = list_nums[--conf->depth];
  node = VSTR_SECTS_NUM(sects, depth_beg_num);
  if (byte == ']')
  {
    conf->types_ptr[depth_beg_num - 1] = CONF_TOKEN_TYPE_SLIST;
    if (vstr_export_chr(conf->data, node->pos) != '[')
      return (FALSE);
  }
  
  if (byte == ')')
  {
    conf->types_ptr[depth_beg_num - 1] = CONF_TOKEN_TYPE_CLIST;
    if (vstr_export_chr(conf->data, node->pos) != '(')
      return (FALSE);
  }
  
  ASSERT(!node->len);
  node->len = sects->num - depth_beg_num;
  
  conf->state = CONF_PARSE_STATE_CHOOSE;
  return (TRUE);
}

static void conf__parse_beg_quote_d(Conf_parse *conf, size_t pos,
                                    unsigned int *list_nums)
{
  ASSERT(conf->depth <= CONF_PARSE_LIST_DEPTH_SZ);
            
  vstr_sects_add(conf->sects, pos, 0);
  conf__parse_type_add(conf, CONF_TOKEN_TYPE_QUOTE_D);
  list_nums[conf->depth++] = conf->sects->num;
  
  conf->state = CONF_PARSE_STATE_QUOTE_D_BEG;
}

static void conf__parse_beg_quote_s(Conf_parse *conf, size_t pos,
                                    unsigned int *list_nums)
{
  ASSERT(conf->depth <= CONF_PARSE_LIST_DEPTH_SZ);
            
  vstr_sects_add(conf->sects, pos, 0);
  conf__parse_type_add(conf, CONF_TOKEN_TYPE_QUOTE_S);
  list_nums[conf->depth++] = conf->sects->num;
  
  conf->state = CONF_PARSE_STATE_QUOTE_S_BEG;
}

static void conf__parse_beg_quote_xxx(Conf_parse *conf, unsigned int *list_nums,
                                      int three)
{
  Vstr_sects *sects = conf->sects;
  unsigned int depth_beg_num = 0;
  size_t mv_pos = 1;
  unsigned int state = 0;
  
  ASSERT(conf->depth);
  
  depth_beg_num = list_nums[conf->depth - 1];

  if (conf->state == CONF_PARSE_STATE_QUOTE_S_BEG)
    state = CONF_PARSE_STATE_QUOTE_S_END;
  if (conf->state == CONF_PARSE_STATE_QUOTE_D_BEG)
    state = CONF_PARSE_STATE_QUOTE_D_END;

  if (three)
  {
    mv_pos = 3;
    state += CONF_PARSE_STATE_QUOTE_X2XXX;
    conf->types_ptr[depth_beg_num - 1] += 2;
  }
  
  VSTR_SECTS_NUM(sects, depth_beg_num)->pos += mv_pos;
  
  conf->state = state;
}

static size_t conf__parse_beg_unquote(Conf_parse *conf, size_t pos, size_t len,
                                      unsigned int *list_nums)
{
  size_t plen = 1;
  int d   = FALSE;
  int xxx = FALSE;
  
  if (FALSE) { }
  else if (VPREFIX(conf->data, pos, len, "\"\"\""))
  {
    d   = TRUE;
    xxx = TRUE;
  }
  else if (VPREFIX(conf->data, pos, len, "\""))
  {
    d   = TRUE;
    xxx = FALSE;
  }
  else if (VPREFIX(conf->data, pos, len, "'''"))
  {
    d   = FALSE;
    xxx = TRUE;
  }
  else if (VPREFIX(conf->data, pos, len, "'"))
  {
    d   = FALSE;
    xxx = FALSE;
  }
  else
    return (0);

  if (d)
    conf__parse_beg_quote_d(conf, pos, list_nums);
  else
    conf__parse_beg_quote_s(conf, pos, list_nums);

  if (!xxx)
    conf__parse_beg_quote_xxx(conf, list_nums, FALSE);
  else
  {
    plen = 3;
    conf__parse_beg_quote_xxx(conf, list_nums, TRUE);
  }

  conf->state += CONF_PARSE_STATE_QUOTE2UNQUOTE_END;
  
  len -= plen; pos += plen;
  if (VPREFIX(conf->data, pos, len, "\\\n")) /* allow first line */
  {
    unsigned int depth_beg_num = list_nums[conf->depth - 1];

    VSTR_SECTS_NUM(conf->sects, depth_beg_num)->pos += CLEN("\\\n");
    plen                                            += CLEN("\\\n");
  }
  
  return (plen);
}

static void conf__parse_end_quote_xxx(Conf_parse *conf, size_t pos,
                                      unsigned int *list_nums)
{
  Vstr_sects *sects = conf->sects;
  unsigned int depth_beg_num = 0;
  size_t beg_pos = 0;
  
  ASSERT(conf->depth);

  depth_beg_num = list_nums[--conf->depth];
  beg_pos = VSTR_SECTS_NUM(sects, depth_beg_num)->pos;
  VSTR_SECTS_NUM(sects, depth_beg_num)->pos = beg_pos;
  VSTR_SECTS_NUM(sects, depth_beg_num)->len = pos - beg_pos;
  
  conf->state = CONF_PARSE_STATE_LIST_END_OR_WS;
}

static int conf__parse_esc_quote(Conf_parse *conf, unsigned int *list_nums,
                                 size_t pos, size_t len, unsigned int type)
{
  Vstr_sects *sects = conf->sects;
  unsigned int depth_beg_num = 0;

  ASSERT(conf->depth);

  if (len < 2)
    return (FALSE);
  
  depth_beg_num = list_nums[conf->depth - 1];
  if ((pos == VSTR_SECTS_NUM(sects, depth_beg_num)->pos) &&
      vstr_cmp_cstr_eq(conf->data, pos, CLEN("\\\n"), "\\\n"))
  {
    VSTR_SECTS_NUM(sects, depth_beg_num)->pos += CLEN("\\\n");
    return (TRUE);
  }
  
  if (conf->types_ptr[depth_beg_num - 1] == type)
    conf->types_ptr[depth_beg_num - 1] += CONF_TOKEN_TYPE_2ESC;

  return (TRUE);
}
#define CONF__SC_PARSE_ESC_QUOTE(x) do {                                \
      ASSERT(vstr_export_chr(data, pos) == '\\');                       \
                                                                        \
      if (!conf__parse_esc_quote(conf, list_nums, pos, len, x))         \
        return (CONF_PARSE_ERR);                                        \
      plen = 2;                                                         \
    } while (FALSE)

#define CONF__SC_PARSE_QUOTE_X_BEG(x) do {                           \
      if (!VPREFIX(data, pos, len, x))                               \
        conf__parse_beg_quote_xxx(conf, list_nums, FALSE);           \
      else                                                           \
      {                                                              \
        plen = 2;                                                    \
        conf__parse_beg_quote_xxx(conf, list_nums, TRUE);            \
      }                                                              \
    } while (FALSE)

#define CONF__SC_PARSE_QUOTE_XXX_END(x, y, z) do {                      \
      if (!(plen = vstr_cspn_cstr_chrs_fwd(data, pos, len, x)))         \
      {                                                                 \
        plen = 3;                                                       \
        if (VPREFIX(data, pos, len, y))                                 \
          conf__parse_end_quote_xxx(conf, pos, list_nums);              \
        else if (vstr_export_chr(data, pos) != '\\')                    \
          plen = 1;                                                     \
        else /* \x */                                                   \
          CONF__SC_PARSE_ESC_QUOTE(CONF_TOKEN_TYPE_QUOTE_ ## z);        \
      }                                                                 \
    } while (FALSE)

#define CONF__SC_PARSE_QUOTE_X_END(x, y, z) do {                        \
      if (!(plen = vstr_cspn_cstr_chrs_fwd(data, pos, len, x)))         \
      {                                                                 \
        plen = 1;                                                       \
        if (vstr_export_chr(data, pos) == y)                            \
          conf__parse_end_quote_xxx(conf, pos, list_nums);              \
        else /* \x */                                                   \
          CONF__SC_PARSE_ESC_QUOTE(CONF_TOKEN_TYPE_QUOTE_ ## z);        \
      }                                                                 \
    } while (FALSE)

#define CONF__SC_PARSE_UNQUOTE_END(x) do {                           \
      if (!(plen = vstr_srch_cstr_buf_fwd(data, pos, len, x)))       \
        plen = len;                                                  \
      else                                                           \
      {                                                              \
        conf__parse_end_quote_xxx(conf, plen, list_nums);            \
        plen -= pos;                                                 \
        plen += CLEN(x);                                             \
      }                                                              \
    } while (FALSE)

int conf_parse_lex(Conf_parse *conf, size_t pos, size_t len)
{
  unsigned int list_nums[CONF_PARSE_LIST_DEPTH_SZ + 1];
  Vstr_base *data = NULL;

  ASSERT(conf && conf->data && conf->sects &&
         conf->types_ptr && conf->types_sz);
  
  data = conf->data;

  if (conf->state != CONF_PARSE_STATE_BEG)
    return (CONF_PARSE_ERR);

  while (len)
  {
    size_t plen = 0; /* amount parsed this loop */
    
    switch (conf->state)
    {
      case CONF_PARSE_STATE_BEG: /* unix she-bang */
        conf->state = CONF_PARSE_STATE_CHOOSE;
        if (VPREFIX(data, pos, len, "#!"))
        {
          plen = vstr_cspn_cstr_chrs_fwd(data, pos, len, "\n");
          conf->state = CONF_PARSE_STATE_WS;
        }
        break;
        
      case CONF_PARSE_STATE_WS:
        if (!(plen = conf__parse_ws(conf, pos, len)))
          return (CONF_PARSE_ERR);
        break;
        
      case CONF_PARSE_STATE_LIST_END_OR_WS:
      {
        int byte = vstr_export_chr(data, pos);

        plen = 1;
        switch (byte)
        {
          case ' ':  /* whitespace */
          case '\t': /* whitespace */
          case '\v': /* whitespace */
          case '\r': /* whitespace */
          case '\n': /* whitespace */
            plen = conf__parse_ws(conf, pos, len);
            break;
            
          case ')':
          case ']':
            if (!conf__parse_end_list(conf, list_nums, byte))
              return (CONF_PARSE_ERR);
            break;
          default:
            return (CONF_PARSE_ERR);
        }
      }
      break;
        
      case CONF_PARSE_STATE_CHOOSE:
      {
        int byte = vstr_export_chr(data, pos);

        plen = 1;
        switch (byte)
        {
          case ' ':  /* whitespace */
          case '\t': /* whitespace */
          case '\v': /* whitespace */
          case '\r': /* whitespace */
          case '\n': /* whitespace */
            plen = conf__parse_ws(conf, pos, len);
            break;
            
          case ')':
          case ']':
            if (!conf__parse_end_list(conf, list_nums, byte))
              return (CONF_PARSE_ERR);
            break;

          case '(':
          case '[':
            if (!conf__parse_beg_list(conf, pos, list_nums))
              return (CONF_PARSE_ERR);
            break;

          case '"':
            conf__parse_beg_quote_d(conf, pos, list_nums);
            break;
          case '\'':
            conf__parse_beg_quote_s(conf, pos, list_nums);
            break;
            
          case '@': /* allow "raw" strings, for backslashes, @ == C# */
          case 'r': /* allow "raw" strings, for backslashes, r == python */
            if (len > 1)
            {
              char tmp = vstr_export_chr(data, pos + 1);
              if ((tmp == '"') || (tmp == '\''))
              {
                conf->state = CONF_PARSE_STATE_UNQUOTE_BEG;
                break;
              }
            }
            
          default:
            plen = vstr_cspn_cstr_chrs_fwd(data, pos, len, " \t\v\r\n\"'()[]");
            conf->state = CONF_PARSE_STATE_SYMBOL_END;
            vstr_sects_add(conf->sects, pos, plen);
            conf__parse_type_add(conf, CONF_TOKEN_TYPE_SYMBOL);
            break;            
        }
      }
      break;
        
      case CONF_PARSE_STATE_QUOTE_D_BEG:
        CONF__SC_PARSE_QUOTE_X_BEG("\"\"");
        break;
        
      case CONF_PARSE_STATE_QUOTE_S_BEG:
        CONF__SC_PARSE_QUOTE_X_BEG("''");
      break;
        
      case CONF_PARSE_STATE_UNQUOTE_BEG:
        if (!(plen = conf__parse_beg_unquote(conf, pos, len, list_nums)))
          return (CONF_PARSE_ERR);
        break;
        
      case CONF_PARSE_STATE_QUOTE_D_END:
        CONF__SC_PARSE_QUOTE_X_END("\"\\", '"', D);
        break;
        
      case CONF_PARSE_STATE_QUOTE_DDD_END:
        CONF__SC_PARSE_QUOTE_XXX_END("\"\\", "\"\"\"", DDD);
        break;
        
      case CONF_PARSE_STATE_QUOTE_S_END:
        CONF__SC_PARSE_QUOTE_X_END("'\\", '\'', S);
        break;
        
      case CONF_PARSE_STATE_QUOTE_SSS_END:
        CONF__SC_PARSE_QUOTE_XXX_END("'\\", "'''", SSS);
        break;
        
      case CONF_PARSE_STATE_UNQUOTE_DDD_END:
        CONF__SC_PARSE_UNQUOTE_END("\"\"\"");
        break;
        
      case CONF_PARSE_STATE_UNQUOTE_D_END:
        CONF__SC_PARSE_UNQUOTE_END("\"");
        break;
        
      case CONF_PARSE_STATE_UNQUOTE_SSS_END:
        CONF__SC_PARSE_UNQUOTE_END("'''");
        break;
        
      case CONF_PARSE_STATE_UNQUOTE_S_END:
        CONF__SC_PARSE_UNQUOTE_END("'");
        break;
        
      case CONF_PARSE_STATE_SYMBOL_END:
      {
        int byte = vstr_export_chr(data, pos);

        switch (byte)
        {
          case ' ':  /* whitespace */
          case '\t': /* whitespace */
          case '\v': /* whitespace */
          case '\r': /* whitespace */
          case '\n': /* whitespace */
            if (!(plen = conf__parse_comment(conf, pos, len)))
              plen = conf__parse_ws(conf, pos, len);
            break;
            
          case ')':
          case ']':
            if (!(plen = conf__parse_comment(conf, pos, len)))
            {
              plen = 1;
              if (!conf__parse_end_list(conf, list_nums, byte))
                return (CONF_PARSE_ERR);
            }
            break;

          case '(':
          case '[':
            if (!(plen = conf__parse_comment(conf, pos, len)))
            {
              plen = 1;
              if (!conf__parse_beg_list(conf, pos, list_nums))
                return (CONF_PARSE_ERR);
            }
            break;
            
          case '"':
          case '\'':
            return (CONF_PARSE_ERR);
          default:
            assert(FALSE);
            return (CONF_PARSE_ERR);
        }
      }
      break;
      
      default:
        assert(FALSE);
        return (CONF_PARSE_ERR);
    }

    len -= plen; pos += plen;
  }

  if (conf->sects->malloc_bad || conf->depth)
    return (CONF_PARSE_ERR);

  conf->state = CONF_PARSE_STATE_END;
  return (CONF_PARSE_FIN);
}
#undef CONF__SC_PARSE_ESC_QUOTE
#undef CONF__SC_PARSE_QUOTE_X_BEG
#undef CONF__SC_PARSE_QUOTE_XXX_END
#undef CONF__SC_PARSE_QUOTE_X_END
#undef CONF__SC_PARSE_UNQUOTE_END

Conf_token *conf_token_make(void)
{
  Conf_token dummy = CONF_TOKEN_INIT;
  Conf_token *ret = MK(sizeof(Conf_token));

  if (!ret)
    return (NULL);
  
  *ret = dummy;

  return (ret);
}

void conf_token_free(Conf_token *token)
{
  F(token);
}

static const char *conf__token_name_map[] = {
 "<** Error **>",
 "Circular bracket list",
 "Square bracket list",
 "Quoted string (double, RAW)",
 "Quoted string (double, with Escaping)",
 "Quoted string (3x double, RAW)",
 "Quoted string (3x double, with Escaping)",
 "Quoted string (single, RAW)",
 "Quoted string (single, with Escaping)",
 "Quoted string (3x single, RAW)",
 "Quoted string (3x single, with Escaping)",
 "Symbol"
};

const char *conf_token_name(const Conf_token *token)
{
  ASSERT(token);

  if (token->type > CONF_TOKEN_TYPE_SYMBOL)
    return ("User type");
  
  return (conf__token_name_map[token->type]);
}

int conf_sc_token_parse_uint(const Conf_parse *conf, Conf_token *token,
                             unsigned int *val)
{
  unsigned int num = conf_token_list_num(token, token->depth_num);
  int ern = CONF_SC_TYPE_RET_OK;
  const Vstr_sect_node *pv = NULL;
  unsigned int nflags = VSTR_FLAG02(PARSE_NUM, OVERFLOW, SEP);
  size_t len = 0;
  
  ASSERT(val);

  if (!num)
    return (CONF_SC_TYPE_RET_ERR_NOT_EXIST);
  conf_parse_token(conf, token);
  if (!(pv = conf_token_value(token)))
    return (CONF_SC_TYPE_RET_ERR_PARSE);

  *val = vstr_parse_uint(conf->data, pv->pos, pv->len, nflags, &len, NULL);
  if (len != pv->len)
    ern = CONF_SC_TYPE_RET_ERR_PARSE;
  
  return (ern);
}

int conf_sc_token_app_vstr(const Conf_parse *conf, Conf_token *token,
                           Vstr_base *s1,
                           const Vstr_base **a_s1, size_t *a_pos, size_t *a_len)
{
  unsigned int num = conf_token_list_num(token, token->depth_num);
  int ern = CONF_SC_TYPE_RET_OK;
  const Vstr_sect_node *pv = NULL;
  
  ASSERT(s1);

  if (!num)
    return (CONF_SC_TYPE_RET_ERR_NOT_EXIST);
  conf_parse_token(conf, token);
  if (!(pv = conf_token_value(token)))
    return (CONF_SC_TYPE_RET_ERR_PARSE);
  
  if (vstr_add_vstr(s1, s1->len, conf->data, pv->pos, pv->len,
                    VSTR_TYPE_SUB_BUF_REF))
  {
    *a_s1  = s1;
    *a_pos = (s1->len - pv->len) + 1;
    *a_len = pv->len;
  }
  
  return (ern);
}

int conf_sc_token_sub_vstr(const Conf_parse *conf, Conf_token *token,
                           Vstr_base *s1, size_t pos, size_t len)
{
  unsigned int num = conf_token_list_num(token, token->depth_num);
  int ern = CONF_SC_TYPE_RET_OK;
  const Vstr_sect_node *pv = NULL;
  
  ASSERT(s1);
  
  if (!num)
    return (CONF_SC_TYPE_RET_ERR_NOT_EXIST);
  conf_parse_token(conf, token);
  if (!(pv = conf_token_value(token)))
    return (CONF_SC_TYPE_RET_ERR_PARSE);

  vstr_sub_vstr(s1, pos, len, conf->data, pv->pos, pv->len,
                VSTR_TYPE_SUB_BUF_REF);
  
  return (ern);
}

#define SUB2(x, y, z) vstr_sub_cstr_buf(x, y, 2, z)
int conf_sc_conv_unesc(Vstr_base *s1, size_t pos, size_t len,
                       size_t *ret_len)
{
  size_t dummy_len;

  if (!ret_len) ret_len = &dummy_len;
  
  *ret_len = len;
  
  while (!s1->conf->malloc_bad && (len > 1))
  {
    size_t plen = vstr_cspn_cstr_chrs_fwd(s1, pos, len, "\\");

    if (!plen)
    {
      char chr = vstr_export_chr(s1, pos + 1);

      if (chr == '\n')
      {
        vstr_del(s1, pos, 2);
        len -= 2; *ret_len -= 2;
        continue;
      }
      else if (chr == 't') { SUB2(s1, pos, "\t"); --len; --*ret_len; }
      else if (chr == 'v') { SUB2(s1, pos, "\v"); --len; --*ret_len; }
      else if (chr == 'r') { SUB2(s1, pos, "\r"); --len; --*ret_len; }
      else if (chr == 'n') { SUB2(s1, pos, "\n"); --len; --*ret_len; }
      else if (chr == 'b') { SUB2(s1, pos, "\b"); --len; --*ret_len; }
      else if (chr == '0')
      { /* \0 == NIL \0x0 == NIL */
        unsigned char val = 0;
        unsigned int nflags = VSTR_FLAG02(PARSE_NUM, OVERFLOW, SEP);

        if (chr != '0') nflags |= 8;

        /* FIXME: ... parse uchar */
        val = vstr_parse_ushort(s1, pos + 1, len - 1, nflags, &plen, NULL);
        vstr_sub_rep_chr(s1, pos, plen + 1, val, 1);
        len      -= plen; /* byte == \ ... so not plen + 1 */
        *ret_len -= plen;
      }
      else
      { /* rm \ ... Eg. \" == " and \\ == \ */
        vstr_del(s1, pos, 1);
        --len; --*ret_len;
      }
      plen = 1;
    }
    
    len -= plen;
    pos += plen;
  }

  return (!s1->conf->malloc_bad);
}
#undef SUB

int conf_token_set_user_value(Conf_parse *conf, Conf_token *token,
                              unsigned int type,
                              Vstr_ref *uval, unsigned int nxt)
{
  Vstr_ref *oref = NULL;
  
  ASSERT(conf && token);
  ASSERT(type <= (UINT_MAX - CONF_TOKEN_TYPE_USER_BEG));
  
  /* must not be a CLIST or SLIST */
  if ((token->type == CONF_TOKEN_TYPE_CLIST) ||
      (token->type == CONF_TOKEN_TYPE_SLIST))
    return (FALSE);

  if (token->type >= CONF_TOKEN_TYPE_USER_BEG)
    oref = conf->uvals_ptr[token->u.uval_num].ref;
  else
  {
    if (conf->uvals_sz <= conf->uvals_num)
    {
      unsigned int num = (conf->uvals_sz << 1) + 1;
      Conf__uval_store *uvals  = NULL;

      if (num <= conf->uvals_sz)
        return (FALSE);
      
      if (!conf->uvals_sz &&
          !(conf->uvals_ptr = MK(sizeof(Conf__uval_store) * num)))
        return (FALSE);
      else if (!MV(conf->uvals_ptr, uvals, sizeof(Conf__uval_store) * num))
        return (FALSE);
      
      conf->uvals_sz  = num; 
    }
    VSTR_SECTS_NUM(conf->sects, token->num)->pos = conf->uvals_num;
    token->u.uval_num                            = conf->uvals_num;
    
    ++conf->uvals_num;
  }
  
  token->type = CONF_TOKEN_TYPE_USER_BEG + type;

  conf->types_ptr[token->num - 1]        = token->type;
  conf->uvals_ptr[token->u.uval_num].ref = NULL;
  conf->uvals_ptr[token->u.uval_num].nxt = nxt;
  if (uval)
    conf->uvals_ptr[token->u.uval_num].ref = vstr_ref_add(uval);
  
  vstr_ref_del(oref);
  
  return (TRUE);
}

void conf_parse_compress(Conf_parse *conf)
{ /* FIXME: remove all Vstr data that isn't referenced, fixup sections */
  (void)conf;
}

void conf_parse_backtrace(Vstr_base *out, const char *filename,
                          const Conf_parse *conf, const Conf_token *token)
{
  const Vstr_sect_node *val = NULL;

  if (!out)
    return;
  
  if (conf->state != CONF_PARSE_STATE_END)
  {
    vstr_add_fmt(out, out->len, "Syntax error in %s,  ", filename);
    vstr_add_fmt(out, out->len, "  State was: %d", conf->state);
    return;
  }

  /* walk backwards */
  if (token->type > CONF_TOKEN_TYPE_SYMBOL)
    vstr_add_fmt(out, out->len, "Failed parse on node %u (%s %d)",
                 token->num, conf_token_name(token),
                 token->type - CONF_TOKEN_TYPE_SYMBOL);
  else if (!(val = conf_token_value(token)))
    vstr_add_fmt(out, out->len, "Failed parse on node %u [%s]",
                 token->num, conf_token_name(token));
  else
  {
    Vstr_base *s1 = conf->data;
    
    vstr_add_fmt(out, out->len, "Failed parse on node %u <%s> = ",
                 token->num, conf_token_name(token));
    vstr_add_vstr(out, out->len, s1, val->pos, val->len, VSTR_TYPE_ADD_BUF_REF);
  }
}
