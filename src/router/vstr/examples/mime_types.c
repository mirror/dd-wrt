
#include "mime_types.h"

#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

static Vstr_sect_node *mime_types__srch(Mime_types_data *mime,
                                        const Vstr_base *fname,
                                        size_t pos, size_t len)
{
  unsigned int num = 0;
  
  ASSERT(fname && len && mime->ents->num);

  num = mime->ents->num - 1; /* FIXME: off by one for addition */
  while (num)
  {
    size_t epos  = VSTR_SECTS_NUM(mime->ents, num)->pos;
    size_t elen  = VSTR_SECTS_NUM(mime->ents, num)->len;
    unsigned int ext_num = num;
    
    --num;
    --num;

    if (vstr_cmp_eq(fname, pos, len, mime->ent_data, epos, elen))
      return (VSTR_SECTS_NUM(mime->ents, ext_num));
  }

  return (NULL);
}

int mime_types_load_simple(Mime_types *pmime, const char *fname)
{
  Mime_types_data *mime = pmime->ref->ptr;
  size_t orig_ent_data_len   = mime->ent_data->len;
  unsigned int orig_ents_num = mime->ents->num;
  Vstr_base *data = NULL;
  Vstr_sects *lines = NULL;
  unsigned int num = 0;
  Vstr_sects *sects = NULL;
  size_t data_pos = 0;
  size_t data_len = 0;
  int saved_errno = ENOMEM;

  ASSERT(pmime && pmime->ref && (pmime->ref->ref == 1));
  
  if (!fname || !*fname)
    return (TRUE);
  
  if (!(data = vstr_make_base(NULL)))
    goto fail_data;

  if (!(lines = vstr_sects_make(128)))
    goto fail_sects_lines;

  if (!(sects = vstr_sects_make(4)))
    goto fail_sects_tmp;

  if (!vstr_sc_read_len_file(data, data_pos, fname, 0, 0, NULL))
    goto fail_read_file;

  data_len = data->len - data_pos++;
  vstr_split_cstr_buf(data, data_pos, data_len, "\n", lines, 0, 0);
  if (lines->malloc_bad)
    goto fail_split_file;

  while (++num <= lines->num)
  {
    size_t pos = VSTR_SECTS_NUM(lines, num)->pos;
    size_t len = VSTR_SECTS_NUM(lines, num)->len;
    Vstr_sect_node *sct = NULL;

    if (vstr_export_chr(data, pos) == '#')
      continue;

    sects->num = 0;
    vstr_split_cstr_chrs(data, pos, len, " \t", sects, 0, 0);
    if (sects->malloc_bad)
      goto fail_split_line;
    
    while (sects->num > 1)
    {
      Vstr_sect_node *sext = VSTR_SECTS_NUM(sects, sects->num);
      size_t spos = 0;
      size_t slen = 0;
      Vstr_sect_node *old_sext = NULL;
      
      if (!sct)
      {
        sct = VSTR_SECTS_NUM(sects, 1);
        spos = mime->ent_data->len + 1;
        vstr_add_vstr(mime->ent_data, mime->ent_data->len,
                      data, sct->pos, sct->len, VSTR_TYPE_ADD_DEF);
        sct->pos = spos;
      }
      vstr_sects_add(mime->ents, sct->pos, sct->len);

      if (vstr_export_chr(data, sext->pos) != '.')
      {
        vstr_add_cstr_buf(mime->ent_data, mime->ent_data->len, ".");
        spos = mime->ent_data->len;
        slen = sext->len + 1;
      }
      else
      { /* chop the . off and use everything else */
        sext->len--; sext->pos++;
        spos = mime->ent_data->len + 1;
        slen = sext->len;
      }
      
      vstr_add_vstr(mime->ent_data, mime->ent_data->len,
                    data, sext->pos, sext->len, VSTR_TYPE_ADD_DEF);

      /* replace old versions ... so we can match forwards */
      if ((old_sext = mime_types__srch(mime, mime->ent_data, spos, slen)))
        old_sext->len = 0;

      vstr_sects_add(mime->ents, spos, slen);
      sects->num--;
    }
  }

  if (mime->ent_data->conf->malloc_bad || mime->ents->malloc_bad)
    goto fail_end_malloc_check;

  vstr_sects_free(sects);
  vstr_sects_free(lines);
  vstr_free_base(data);

  return (TRUE);
  
 fail_end_malloc_check:
  vstr_sc_reduce(mime->ent_data, 1, mime->ent_data->len,
                 mime->ent_data->len - orig_ent_data_len);
  mime->ents->num = orig_ents_num;
 fail_split_line:
 fail_split_file:
  errno = ENOMEM;
 fail_read_file:
  saved_errno = errno;
  vstr_sects_free(lines);
 fail_sects_tmp:
  vstr_sects_free(sects);
 fail_sects_lines:
  vstr_free_base(data);
 fail_data:
  errno = saved_errno;
  return (FALSE);
}

static void mime_types__filedata_free(Vstr_ref *ref)
{
  
  Mime_types_data *mime = ref->ptr;
  
  vstr_free_base(mime->ent_data); mime->ent_data = NULL;
  vstr_sects_free(mime->ents);    mime->ents     = NULL;
  
  (*mime->pref_func)(ref);
}

int mime_types_init(Mime_types *pmime,
                    const Vstr_base *def_vs1, size_t def_pos, size_t def_len)
{
  Mime_types_data *mime = NULL;
  
  ASSERT(pmime);
  ASSERT(def_vs1);

  if (!(pmime->ref = vstr_ref_make_malloc(sizeof(Mime_types_data))))
    goto ref_malloc_fail;
  mime = pmime->ref->ptr;

  if (!(mime->ent_data = vstr_make_base(NULL)))
    goto ent_data_malloc_fail;
  
  if (!(mime->ents = vstr_sects_make(128)))
    goto ents_malloc_fail;

  /* make reference do the right thing... */
  mime->pref_func  = pmime->ref->func;
  pmime->ref->func = mime_types__filedata_free;
  
  pmime->def_type_vs1 = def_vs1;
  pmime->def_type_pos = def_pos;
  pmime->def_type_len = def_len;
  
  return (TRUE);

 ents_malloc_fail:
  vstr_free_base(mime->ent_data);
 ent_data_malloc_fail:
  vstr_ref_del(pmime->ref);
 ref_malloc_fail:
  return (FALSE);
}

int mime_types_match(const Mime_types *pmime,
                     const Vstr_base *fname, size_t pos, size_t len,
                     const Vstr_base **ret_vs1, size_t *ret_pos,size_t *ret_len)
{
  const Mime_types_data *mime = NULL;
  unsigned int num = 0;

  ASSERT(pmime && pmime->ref && (pmime->ref->ref >= 1));
  ASSERT(ret_vs1 && ret_pos && ret_len);

  mime = pmime->ref->ptr;
  while (num++ < mime->ents->num)
  {
    size_t ctpos = VSTR_SECTS_NUM(mime->ents, num)->pos;
    size_t ctlen = VSTR_SECTS_NUM(mime->ents, num)->len;
    size_t epos  = 0;
    size_t elen  = 0;

    ASSERT(num < mime->ents->num);
    ++num;
    epos  = VSTR_SECTS_NUM(mime->ents, num)->pos;
    elen  = VSTR_SECTS_NUM(mime->ents, num)->len;

    if (!elen || (elen > len))
      continue;

    if (vstr_cmp_eod_eq(fname, pos, len, mime->ent_data, epos, elen))
    {
      *ret_vs1 = mime->ent_data;
      *ret_pos = ctpos;
      *ret_len = ctlen;
      return (TRUE);
    }
  }

  *ret_vs1 = pmime->def_type_vs1;
  *ret_pos = pmime->def_type_pos;
  *ret_len = pmime->def_type_len;
  return (FALSE);
}

void mime_types_exit(Mime_types *pmime)
{
  ASSERT(pmime && pmime->ref);
  vstr_ref_del(pmime->ref);       pmime->ref = NULL;
}

void mime_types_combine_filedata(Mime_types *pdst, Mime_types *psrc)
{
  Vstr_ref *tmp = NULL;
  
  ASSERT(psrc && psrc->ref && (psrc->ref->ref >= 1));
  ASSERT(pdst && pdst->ref && (pdst->ref->ref == 1));

  tmp = pdst->ref;
  pdst->ref = vstr_ref_add(psrc->ref);
  vstr_ref_del(tmp);
}
