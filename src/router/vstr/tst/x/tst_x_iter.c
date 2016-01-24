#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  unsigned int scan = 1;
  size_t pos = 1;
  
  while (scan <= 111)
    vstr_add_rep_chr(s1, s1->len, scan++, 1);
  vstr_add_non(s1, s1->len, 11);
  scan += 11;
  while (scan <= 255)
    vstr_add_rep_chr(s1, s1->len, scan++, 1);

  vstr_add_rep_chr(s1, s1->len, 0, 11);
  
  ASSERT(s1->len == 266);
  
  {  
    Vstr_iter iter[1];
    
    ASSERT(vstr_iter_fwd_beg(s1, 1, s1->len, iter));
    ASSERT(iter->num == 1);
    ASSERT(vstr_iter_len(iter) == 266);
    ASSERT(vstr_iter_fwd_nxt(iter));
    
    ASSERT(!vstr_iter_fwd_beg(s1, 1, 0, iter));
    ASSERT(!iter->node);
  }
  
  while (pos <= 266)
  {
    Vstr_iter iter[1];
    unsigned int ern;
    size_t len = vstr_sc_posdiff(pos, 266);
    
    ASSERT(vstr_iter_fwd_beg(s1, pos, len, iter));
    
    ASSERT(vstr_iter_pos(iter, pos, len) == pos);
    ASSERT(vstr_iter_len(iter)           == len);

    scan = pos;
    while (scan <= 266)
    {
      unsigned char val = 0;

      ASSERT(vstr_iter_pos(iter, pos, len) == scan);
      ASSERT(vstr_iter_len(iter)           == vstr_sc_posdiff(scan, 266));
      
      val = vstr_iter_fwd_chr(iter, &ern);

      ASSERT(ern != VSTR_TYPE_ITER_END);
      ASSERT((ern == VSTR_TYPE_ITER_NON) || (ern == VSTR_TYPE_ITER_DEF));
      
      if (ern != VSTR_TYPE_ITER_NON)
        ASSERT((val == scan) || ((val == 0) && (scan >= 256) && (scan <= 266)));
      else
        ASSERT((scan > 111) && (scan <= 123) && (val == 0));
      
      ++scan;
    }
    ASSERT(!vstr_iter_len(iter));

    ASSERT(!vstr_iter_fwd_chr(iter, &ern) && (ern == VSTR_TYPE_ITER_END));
    ASSERT(!vstr_iter_fwd_chr(iter, &ern) && (ern == VSTR_TYPE_ITER_END));
    ASSERT(!vstr_iter_fwd_chr(iter, &ern) && (ern == VSTR_TYPE_ITER_END));
    ASSERT(!vstr_iter_fwd_chr(iter, &ern) && (ern == VSTR_TYPE_ITER_END));
    ASSERT(!vstr_iter_len(iter));
    
    ++pos;
  }

  /* tst for fwd_buf/fwd_cstr */
  {
    Vstr_iter iter[1];
    unsigned int ern;
    
    ASSERT(vstr_iter_fwd_beg(s1, 1, s1->len, iter));
  
    ASSERT(vstr_iter_pos(iter, 1, s1->len) == 1);
    ASSERT(vstr_iter_len(iter)             == s1->len);
    ASSERT(vstr_iter_fwd_buf(iter, 1, NULL, 0, NULL) == 1);
    ASSERT(vstr_iter_pos(iter, 1, s1->len) == 2);
    ASSERT(vstr_iter_len(iter)             == (s1->len - 1));
    ASSERT(vstr_iter_fwd_buf(iter, 265, NULL, 0, NULL) == 265);
    ASSERT(!vstr_iter_len(iter));
  
    ASSERT(!vstr_iter_fwd_chr(iter, &ern) && (ern == VSTR_TYPE_ITER_END));

    memset(buf, 114, sizeof(buf));
    
    ASSERT(vstr_iter_fwd_beg(s1, 1, s1->len, iter));
    ASSERT(vstr_iter_fwd_buf(iter, 267, buf, sizeof(buf), &ern) == 266);
    ASSERT(ern == VSTR_TYPE_ITER_DEF);  
    ASSERT(vstr_iter_fwd_buf(iter, 263, buf, 1, &ern) == 0);
    ASSERT(ern == VSTR_TYPE_ITER_END);  
  
    ASSERT(vstr_iter_fwd_beg(s1, 1, s1->len, iter));
    scan = 0;
    while (scan < 111)
      ASSERT(buf[scan++] == vstr_iter_fwd_chr(iter, NULL));
    ASSERT(vstr_iter_fwd_buf(iter, 11, NULL, 0, NULL) == 11);
    while (scan < 122)
      ASSERT(buf[scan++] == 114);
    while (scan < 255)
      ASSERT(buf[scan++] == vstr_iter_fwd_chr(iter, NULL));
    while (scan < 266)
      ASSERT(buf[scan++] == 0);

    ASSERT(vstr_iter_fwd_beg(s1, 2, 110, iter));
    ASSERT(vstr_iter_fwd_cstr(iter, 263, buf, sizeof(buf), &ern) == 110);
    ASSERT(vstr_iter_fwd_beg(s1, 2, 110, iter));
    scan = 0;
    while (scan < 110)
      ASSERT(buf[scan++] == vstr_iter_fwd_chr(iter, NULL));
    ASSERT(buf[scan] == 0);
    
    ASSERT(vstr_iter_fwd_beg(s1, 4, 2, iter));
    ASSERT(vstr_iter_fwd_cstr(iter, 263, buf, 2, &ern) == 2);
    ASSERT(buf[0] == 4);
    ASSERT(buf[1] == 0);
  }
  
  return (EXIT_SUCCESS);
}

