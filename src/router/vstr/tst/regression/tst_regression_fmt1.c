#include "tst-main.c"

static const char *rf = __FILE__;

static int tst_usr_ptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec,
                          unsigned int base, unsigned int flags)
{
  unsigned int *num = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t len = 0;
  int ret = FALSE;

  flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM;
  
  len = vstr_sc_conv_num_uint(buf, sizeof(buf), *num, "0123456789abcdef", base);

  if (!vstr_sc_fmt_cb_beg(st, &pos, spec, &len, flags))
    return (FALSE);
  
  if (spec->fmt_quote)
    ret = vstr_sc_add_grpbasenum_buf(st, pos, base, buf, len);
  else
    ret = vstr_add_buf(st, pos, buf, len);
                                                                                
  if (!ret || !vstr_sc_fmt_cb_end(st, pos, spec, len))
    return (FALSE);
                                                                                
  return (TRUE);

}

static int tst_usr_uptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  return (tst_usr_ptr_cb(st, pos, spec, 10, 0));
}

static int tst_usr_xptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  return (tst_usr_ptr_cb(st, pos, spec, 16, VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_L));
}

static int tst_usr_optr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  return (tst_usr_ptr_cb(st, pos, spec, 8, VSTR_FLAG_SC_FMT_CB_BEG_OBJ_OCTNUM));
}

static int tst_usr_bptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  return (tst_usr_ptr_cb(st, pos, spec, 2, VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_L));
}

int tst(void)
{
  Vstr_conf *conf = vstr_make_conf();
  Vstr_base *sx = NULL;
  int val = 4000;
  Vstr_ref *ref = NULL;
  
  ASSERT(conf);
  
  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);


  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_fmt_add(conf, "<uptr:%p>", tst_usr_uptr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(conf, "<optr:%p>", tst_usr_optr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(conf, "<xptr:%p>", tst_usr_xptr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(conf, "<bptr:%p>", tst_usr_bptr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);

  ref = vstr_ref_make_strdup("_");
  ASSERT(ref);

  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP,  2, ref, 1));
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP,  8, ref, 1));
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP, 16, ref, 1));

  vstr_ref_del(ref);
  ref = vstr_ref_make_strdup(",");
  ASSERT(ref);

  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP, 10, ref, 1));
  
  vstr_ref_del(ref);
  ref = vstr_ref_make_strdup("\4");
  ASSERT(ref);
  
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP,  2, ref));
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP, 16, ref));
  
  vstr_ref_del(ref);
  ref = vstr_ref_make_strdup("\3");
  ASSERT(ref);
  
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP,  8, ref));
  ASSERT(vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP, 10, ref));
  
  vstr_ref_del(ref);
  sx = vstr_make_base(conf);
  ASSERT(sx);
  
  vstr_free_conf(conf);

  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<uptr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "4,000"));
  
  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<xptr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "0xfa0"));
  
  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<optr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "07_640"));
  
  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<bptr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "0b1111_1010_0000"));

  val = 16;
  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<bptr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "0b1_0000"));
  
  val = 0x12345;
  vstr_del(sx, 1, sx->len);
  vstr_add_fmt(sx, 0, "$#'<xptr:%p>", (void *)&val);
  ASSERT(vstr_cmp_cstr_eq(sx, 1, sx->len, "0x1_2345"));
  
  vstr_free_base(sx);

  return (EXIT_SUCCESS);
}


