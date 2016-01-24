#include "tst-main.c"

static const char *rf = __FILE__;

/* test weird integer corner cases of spec. */

static void tst_host(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
}

static void tst_vstr(const char *fmt, ...)
{
	va_list ap;

        vstr_del(s1, 1, s1->len);
	va_start(ap, fmt);
	vstr_add_vsysfmt(s1, s1->len, fmt, ap);
	va_end(ap);
}

int tst(void)
{
  int ret = 0;
  int num = 0;
  
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.0d", 0);
  TST_B_TST(ret, 1, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.d", 0);
  TST_B_TST(ret, 2, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%I.*d", 0, 0);
  TST_B_TST(ret, 3, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.0x", 0);
  TST_B_TST(ret, 4, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.x", 0);
  TST_B_TST(ret, 5, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.*x", 0, 0);
  TST_B_TST(ret, 6, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#.*x", 0, 0);
  TST_B_TST(ret, 7, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.0o", 0);
  TST_B_TST(ret, 8, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.o", 0);
  TST_B_TST(ret, 9, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.*o", 0, 0);
  TST_B_TST(ret, 10, s1->len);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#.o", 0);
  TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "0"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#x", 1);
  TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "0x1"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#.x", 1);
  TST_B_TST(ret, 13, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "0x1"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#x", 0);
  TST_B_TST(ret, 14, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "0"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#8o", 1);
  TST_B_TST(ret, 15, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "      01"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#08o", 0);
  TST_B_TST(ret, 16, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "00000000"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#8x", 1);
  TST_B_TST(ret, 17, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "     0x1"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%#08x", 1);
  TST_B_TST(ret, 18, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "0x000001"));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%-*d", 4, 1);
  TST_B_TST(ret, 18, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "1   "));

  sprintf(buf, "%#.o", 0);
  if (!buf[0])
    return (EXIT_FAILED_OK); /* Solaris (2.8) gets this wrong at least... */
  
  num = 5;
  while (num--)
  {
    unsigned int mfail_count = 0;
    int spaces = 2;
    
    while (spaces < 10)
    {
      const char *fmt = NULL;
      
#define FMT(x) (fmt = ((num == 0xff) ? "<%" x "x>" : (num == 0xfe) ? "<%" x "X>" : (num == 0777) ? "<%" x "o>" : "<%" x "d>"))
      
      FMT("+#*");
      vstr_del(s3, 1, s3->len);
      
      mfail_count = 0;
      do
      {
        ASSERT(!s3->len);
        vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s3, s3->len, fmt, spaces, num));
      tst_mfail_num(0);
      sprintf(buf, fmt, spaces, num);

      TST_B_TST(ret, 19, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

      FMT("*");
      vstr_del(s3, 1, s3->len);
      
      mfail_count = 0;
      do
      {
        ASSERT(!s3->len);
        vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s3, s3->len, fmt, -spaces, num));
      tst_mfail_num(0);
      sprintf(buf, fmt, -spaces, num);

      TST_B_TST(ret, 19, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

      FMT(" #.*");
      vstr_del(s3, 1, s3->len);
      
      mfail_count = 0;
      do
      {
        ASSERT(!s3->len);
        vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s3, s3->len, fmt, spaces, num));
      tst_mfail_num(0);
      sprintf(buf, fmt, spaces, num);

      TST_B_TST(ret, 19, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

      ++spaces;
    }

    switch (num)
    {
      case    4: num =    -3; break;
      case   -4: num = 0x100; break;
      case 0xff:              break;
      case 0xfe: num = 01000; break;
      case 0777: num =     1; break;
      case    0:              break;
    }
  }
  
  {
    static const char fmts[][80] = {
     "<%o|%#o|%d|%x|%#x|%#d>",
     "<%0o|%0#o|%0d|%0x|%0#x|%0#d>",
     "<%.o|%#.o|%.d|%.x|%#.x|%#.d>",
     "<%0.o|%0#.o|%0.d|%0.x|%0#.x|%0#.d>",
     "<%.1o|%#.1o|%.1d|%.1x|%#.1x|%#.1d>",
     "<%0.1o|%0#.1o|%0.1d|%0.1x|%0#.1x|%0#.1d>",
     "<%8o|%#8o|%8d|%8x|%#8x|%#8d>",
     "<%08o|%#08o|%08d|%08x|%0#8x|%0#8d>",
     "<%8.o|%#8.o|%8.d|%8.x|%#8.x|%#8.d>",
     "<%08.o|%#08.o|%08.d|%08.x|%0#8.x|%0#8.d>",
     "<%4.1o|%#4.1o|%4.1d|%4.1x|%#4.1x|%#4.1d>",
     "<%4.8o|%#4.8o|%4.8d|%4.8x|%#4.8x|%#4.8d>",
     "<%8.4o|%#8.4o|%8.4d|%8.4x|%#8.4x|%#8.4d>",
     "<%04.1o|%0#4.1o|%04.1d|%04.1x|%0#4.1x|%0#4.1d>",
     "<%04.8o|%0#4.8o|%04.8d|%04.8x|%0#4.8x|%0#4.8d>",
     "<%08.4o|%0#8.4o|%08.4d|%08.4x|%0#8.4x|%0#8.4d>",
     "<%-o|%-#o|%-d|%-x|%-#x|%-#d>",
     "<%-0o|%-0#o|%-0d|%-0x|%-0#x|%-0#d>",
     "<%-.o|%-#.o|%-.d|%-.x|%-#.x|%-#.d>",
     "<%-0.o|%-0#.o|%-0.d|%-0.x|%-0#.x|%-0#.d>",
     "<%-.1o|%-#.1o|%-.1d|%-.1x|%-#.1x|%-#.1d>",
     "<%-0.1o|%-0#.1o|%-0.1d|%-0.1x|%-0#.1x|%-0#.1d>",
     "<%-8o|%-#8o|%-8d|%-8x|%-#8x|%-#8d>",
     "<%-08o|%-#08o|%-08d|%-08x|%-0#8x|%-0#8d>",
     "<%-8.o|%-#8.o|%-8.d|%-8.x|%-#8.x|%-#8.d>",
     "<%-08.o|%-#08.o|%-08.d|%-08.x|%-0#8.x|%-0#8.d>",
     "<%-4.1o|%-#4.1o|%-4.1d|%-4.1x|%-#4.1x|%-#4.1d>",
     "<%-4.8o|%-#4.8o|%-4.8d|%-4.8x|%-#4.8x|%-#4.8d>",
     "<%-8.4o|%-#8.4o|%-8.4d|%-8.4x|%-#8.4x|%-#8.4d>",
     "<%-04.1o|%-0#4.1o|%-04.1d|%-04.1x|%-0#4.1x|%-0#4.1d>",
     "<%-04.8o|%-0#4.8o|%-04.8d|%-04.8x|%-0#4.8x|%-0#4.8d>",
     "<%-08.4o|%-0#8.4o|%-08.4d|%-08.4x|%-0#8.4x|%-0#8.4d>",
     "<%+o|%+#o|%+d|%+x|%+#x|%+#d>",
     "<%+0o|%+0#o|%+0d|%+0x|%+0#x|%+0#d>",
     "<%+.o|%+#.o|%+.d|%+.x|%+#.x|%+#.d>",
     "<%+0.o|%+0#.o|%+0.d|%+0.x|%+0#.x|%+0#.d>",
     "<%+.1o|%+#.1o|%+.1d|%+.1x|%+#.1x|%+#.1d>",
     "<%+0.1o|%+0#.1o|%+0.1d|%+0.1x|%+0#.1x|%+0#.1d>",
     "<%+8o|%+#8o|%+8d|%+8x|%+#8x|%+#8d>",
     "<%+08o|%+#08o|%+08d|%+08x|%+0#8x|%+0#8d>",
     "<%+8.o|%+#8.o|%+8.d|%+8.x|%+#8.x|%+#8.d>",
     "<%+08.o|%+#08.o|%+08.d|%+08.x|%+0#8.x|%+0#8.d>",
     "<%+4.1o|%+#4.1o|%+4.1d|%+4.1x|%+#4.1x|%+#4.1d>",
     "<%+4.8o|%+#4.8o|%+4.8d|%+4.8x|%+#4.8x|%+#4.8d>",
     "<%+8.4o|%+#8.4o|%+8.4d|%+8.4x|%+#8.4x|%+#8.4d>",
     "<%+04.1o|%+0#4.1o|%+04.1d|%+04.1x|%+0#4.1x|%+0#4.1d>",
     "<%+04.8o|%+0#4.8o|%+04.8d|%+04.8x|%+0#4.8x|%+0#4.8d>",
     "<%+08.4o|%+0#8.4o|%+08.4d|%+08.4x|%+0#8.4x|%+0#8.4d>",
     "<% o|% #o|% d|% x|% #x|% #d>",
     "<% 0o|% 0#o|% 0d|% 0x|% 0#x|% 0#d>",
     "<% .o|% #.o|% .d|% .x|% #.x|% #.d>",
     "<% 0.o|% 0#.o|% 0.d|% 0.x|% 0#.x|% 0#.d>",
     "<% .1o|% #.1o|% .1d|% .1x|% #.1x|% #.1d>",
     "<% 0.1o|% 0#.1o|% 0.1d|% 0.1x|% 0#.1x|% 0#.1d>",
     "<% 8o|% #8o|% 8d|% 8x|% #8x|% #8d>",
     "<% 08o|% #08o|% 08d|% 08x|% 0#8x|% 0#8d>",
     "<% 8.o|% #8.o|% 8.d|% 8.x|% #8.x|% #8.d>",
     "<% 08.o|% #08.o|% 08.d|% 08.x|% 0#8.x|% 0#8.d>",
     "<% 4.1o|% #4.1o|% 4.1d|% 4.1x|% #4.1x|% #4.1d>",
     "<% 4.8o|% #4.8o|% 4.8d|% 4.8x|% #4.8x|% #4.8d>",
     "<% 8.4o|% #8.4o|% 8.4d|% 8.4x|% #8.4x|% #8.4d>",
     "<% 04.1o|% 0#4.1o|% 04.1d|% 04.1x|% 0#4.1x|% 0#4.1d>",
     "<% 04.8o|% 0#4.8o|% 04.8d|% 04.8x|% 0#4.8x|% 0#4.8d>",
     "<% 08.4o|% 0#8.4o|% 08.4d|% 08.4x|% 0#8.4x|% 0#8.4d>"
    };

#define TST(sym, fmt, val) \
	tst_ ## sym (fmt, \
	             (val), (val), (val), (val), (val), (val))

    unsigned int count = 0;

    while (count < 0x1002)
    {
      unsigned int scan = 0;

      while (scan < sizeof(fmts)/sizeof(fmts[0]))
      {
        TST(host, fmts[scan], count);
        TST(vstr, fmts[scan], count);

        TST_B_TST(ret, 30, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

        TST(host, fmts[scan], -(int)count);
        TST(vstr, fmts[scan], -(int)count);

        TST_B_TST(ret, 30, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

        ++scan;
      }

      switch (count)
      { case 0x0000: count = 0x0001; break;
        case 0x0001: count =     10; break;
        case     10: count = 0x0010; break;
        case 0x0010: count =    100; break;
        case    100: count = 0x0100; break;
        case 0x0100: count =   1000; break;
        case   1000: count = 0x1000; break;
        case 0x1000: count = 0x1001; break;
        case 0x1001: count = 0x10000001; break;
        case 0x10000001: count = 0x10000002; break;
        default: abort();
      }
    }
  }

  return (TST_B_RET(ret));
}
