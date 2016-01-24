
static int app_fmt(Vstr_base *s1, const char *fmt, ... )
   VSTR__COMPILE_ATTR_FMT(2, 3);
static inline int app_fmt(Vstr_base *s1, const char *fmt, ... )
{
  va_list ap;
  int ret = FALSE;
  
  va_start(ap, fmt);
  ret = vstr_add_vfmt(s1, s1->len, fmt, ap);
  va_end(ap);

  return (ret);
}
static inline int app_buf(Vstr_base *s1, const void *buf, size_t len)
{ return (vstr_add_buf(s1, s1->len, buf, len)); }
static inline int app_cstr_buf(Vstr_base *s1, const char *buf)
{ return (vstr_add_cstr_buf(s1, s1->len, buf)); }
static inline int app_vstr(Vstr_base *s1, Vstr_base *s2, size_t pos, size_t len,
                           unsigned int flags)
{ return (vstr_add_vstr(s1, s1->len, s2, pos, len, flags)); }

static inline int app_b_uint8(Vstr_base *s1, unsigned int data)
{ return (vstr_add_rep_chr(s1, s1->len, data & 0xFF, 1)); }
static inline int app_b_uint16(Vstr_base *s1, unsigned int data)
{ return (vstr_sc_add_b_uint16(s1, s1->len, data)); }
static inline int app_b_uint32(Vstr_base *s1, unsigned int data)
{ return (vstr_sc_add_b_uint32(s1, s1->len, data)); }

static inline int sub_b_uint16(Vstr_base *s1, size_t pos, unsigned int data)
{ return (vstr_sc_sub_b_uint16(s1, pos, 2, data)); }

static inline unsigned int get_b_uint16(Vstr_base *s1, size_t pos)
{ return (vstr_sc_parse_b_uint16(s1, pos)); }
static inline unsigned int get_b_uint32(Vstr_base *s1, size_t pos)
{ return (vstr_sc_parse_b_uint32(s1, pos)); }

