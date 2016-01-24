#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  unsigned int scan = 1;
  
  ASSERT(vstr_sc_add_b_uint16(s1, s1->len, 0));
  ASSERT(vstr_sc_add_b_uint16(s1, s1->len,  1u <<  7));
  ASSERT(vstr_sc_add_b_uint16(s1, s1->len,  1u <<  8));
  ASSERT(vstr_sc_add_b_uint16(s1, s1->len, (1u << 16) - 1));

  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 0));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u <<  7));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u <<  8));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u << 15));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u << 16));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u << 23));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u << 24));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 1u << 30));
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 0u - 1u));
  /* 0x55AA6699 == 0b0101_0101_1010_1010_0110_0110_1001_1001 */
  ASSERT(vstr_sc_add_b_uint32(s1, s1->len, 0x55AA6699));

  ASSERT(0                == vstr_sc_parse_b_uint16(s1, scan)); scan += 2;
  ASSERT( (1u <<  7)      == vstr_sc_parse_b_uint16(s1, scan)); scan += 2;
  ASSERT( (1u <<  8)      == vstr_sc_parse_b_uint16(s1, scan)); scan += 2;
  ASSERT(((1u << 16) - 1) == vstr_sc_parse_b_uint16(s1, scan)); scan += 2;

  ASSERT(0                == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u <<  7)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u <<  8)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u << 15)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u << 16)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u << 23)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u << 24)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  ASSERT( (1u << 30)      == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;

  ASSERT(((1u << 16) - 1) == vstr_sc_parse_b_uint16(s1, scan));
  ASSERT(((1u << 16) - 1) == vstr_sc_parse_b_uint16(s1, scan + 2));
  
  ASSERT( (0u - 1u)       == vstr_sc_parse_b_uint32(s1, scan)); scan += 4;
  
  ASSERT(0x55AA           == vstr_sc_parse_b_uint16(s1, scan));
  ASSERT(0x6699           == vstr_sc_parse_b_uint16(s1, scan + 2));
  
  ASSERT(0x55AA6699       == vstr_sc_parse_b_uint32(s1, scan));
  
  ASSERT(vstr_sc_sub_b_uint16(s1, scan, 2, 0x05A0));

  ASSERT(0x05A06699       == vstr_sc_parse_b_uint32(s1, scan));

  ASSERT(vstr_sc_sub_b_uint32(s1, scan, 2, 0x669955AA));

  ASSERT(0x6699           == vstr_sc_parse_b_uint16(s1, scan));
  ASSERT(0x55AA           == vstr_sc_parse_b_uint16(s1, scan + 2));
  
  ASSERT(0x669955AA       == vstr_sc_parse_b_uint32(s1, scan));
  
  return (EXIT_SUCCESS);
}
