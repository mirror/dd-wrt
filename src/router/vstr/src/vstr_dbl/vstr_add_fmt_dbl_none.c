/*
 *  Copyright (C) 2002  James Antill
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
/* This code does the %F, %f, %G, %g, %A, %a, %E, %e from *printf by always
 * treating the float as a zero, useful if you never need to do fp stuff */
/* Note that this file is #include'd */

#ifdef USE_RESTRICTED_HEADERS /* always use C locale */
# undef  SYS_LOC /* special version to "." is there */
# define SYS_LOC(x) "."
#endif

static int vstr__add_fmt_dbl(Vstr_base *base, size_t pos_diff,
                             struct Vstr__fmt_spec *spec)
{
  char sign = 0;
  int big = FALSE;
  unsigned int field_width = 0;
  unsigned int precision = 0;
  unsigned int have_g_or_G = FALSE;
  unsigned int have_dot = FALSE;
  const char *decimal_point_str = NULL;
  size_t decimal_point_len      = 0;
  
  decimal_point_str = vstr__loc_num_pnt_ptr(base->conf->loc, num_base);
  decimal_point_len = vstr__loc_num_pnt_len(base->conf->loc, num_base);
  
  switch (spec->fmt_code)
  {
    case 'a':
    case 'A':
      if (!(spec->flags & PREC_USR))
        precision = 0;
      break;

    case 'g':
    case 'G':
      have_g_or_G = TRUE;
    case 'e':
    case 'E':
    case 'f':
    case 'F':
      if (!(spec->flags & PREC_USR))
        precision = 6;
      break;

    default:
      assert(FALSE);
  }

  if (spec->flags & PREC_USR)
    precision = spec->precision;

 if (spec->field_width_usr)
   field_width = spec->field_width;

  if (spec->flags & LEFT)
    spec->flags &= ~ZEROPAD;

  if (spec->flags & SIGN)
  {
    if (spec->flags & NUM_IS_NEGATIVE)
      sign = '-';
    else
    {
      if (spec->flags & PLUS)
        sign = '+';
      else
        if (spec->flags & SPACE)
          sign = ' ';
        else
          ++field_width;
    }

    if (field_width) --field_width;
  }

  if (field_width) --field_width; /* size of number */

  if (field_width > precision)
    field_width -= precision;
  else
    field_width = 0;

  /* glibc %#.g is wrong ... it shouldn't print a '.' ... we are compat. with it
   *
   * **************************************************************
   *
   * C99 7.19.6.1 says...
   *
   * For the '#' flag
   *
   * For x (or X) conversion, a nonzero result has 0x (or 0X) prefixed to it.
   * For a, A, e, E, f, F, g, and G conversions, the result of converting a
   * floating-point number always contains a decimal-point character, even if
   * no digits follow it.
   *
   * But for 'g' convertaion
   *
   * Trailing zeros are removed from the fractional portion of the result
   * unless the # flag is specified; a decimal-point character appears only
   * if it is followed by a digit.
   *
   * **************************************************************
   *
   * Also there is this, which seems to confirm their opinion...
   * http://std.dkuug.dk/jtc1/sc22/wg14/www/docs/dr_233.htm
   *
   */
  if ((!have_g_or_G && (spec->flags & SPECIAL)) ||
      (!have_g_or_G && precision) ||
      ( have_g_or_G && (spec->flags & SPECIAL)))
      /* ( have_g_or_G && (spec->flags & SPECIAL) && precision)) */
    have_dot = TRUE;

  if (have_dot)
  {
    if (field_width > decimal_point_len)
      field_width -= decimal_point_len;
    else
      field_width = 0;
  }
  
  switch (spec->fmt_code)
  {
    case 'a':
    case 'A': /* "0x" ... "p+0"  == 5 extra */
      if (field_width) --field_width;
    case 'e':
    case 'E': /* "e+00" == 4 extra */
      if (field_width) --field_width;
      if (field_width) --field_width;
      if (field_width) --field_width;
      if (field_width) --field_width;
      break;

    case 'f':
    case 'F':
    case 'g':
    case 'G':
      break;

    default:
      assert(FALSE);
  }

  if (!(spec->flags & (ZEROPAD | LEFT)))
    if (field_width > 0)
    { /* right justify number, with spaces -- zeros done after sign/spacials */
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', field_width))
        goto failed_alloc;
      field_width = 0;
    }

  if (sign)
    if (!VSTR__FMT_ADD(base, &sign, 1))
      goto failed_alloc;

  if (spec->fmt_code == 'a')
    if (!VSTR__FMT_ADD(base, "0x", 2))
      goto failed_alloc;
  if (spec->fmt_code == 'A')
    if (!VSTR__FMT_ADD(base, "0X", 2))
      goto failed_alloc;

  if (!(spec->flags & LEFT))
    if (field_width > 0)
    {
      assert(spec->flags & ZEROPAD);
      if (!VSTR__FMT_ADD_REP_CHR(base, '0', field_width))
        goto failed_alloc;
      field_width = 0;
    }

  if (!VSTR__FMT_ADD_REP_CHR(base, '0', 1)) /* number */
    goto failed_alloc;

  if (have_dot)
    if (!VSTR__FMT_ADD(base, decimal_point_str, decimal_point_len))
      goto failed_alloc;

  switch (spec->fmt_code)
  {
    case 'g':
    case 'G':
      if (!(spec->flags & SPECIAL))
        precision = 0;
      else if (precision > 1)
        --precision;
    default:
      break;
  }

  if (precision && !VSTR__FMT_ADD_REP_CHR(base, '0', precision))
    goto failed_alloc;

  switch (spec->fmt_code)
  {
    case 'e':
    case 'E':
      if (!VSTR__FMT_ADD(base, &spec->fmt_code, 1))
        goto failed_alloc;

      if (!VSTR__FMT_ADD(base, "+", 1))
        goto failed_alloc;

      if (!VSTR__FMT_ADD_REP_CHR(base, '0', 2))
        goto failed_alloc;
      break;

    case 'G':
    case 'g': /* always using 0.0, so %f is always smaller for %g */
    case 'F':
    case 'f':
        break;

    case 'A':
      big = TRUE;
    case 'a':
      if (!VSTR__FMT_ADD(base, big ? "P" : "p", 1))
        goto failed_alloc;

      if (!VSTR__FMT_ADD(base, "+", 1))
        goto failed_alloc;

      if (!VSTR__FMT_ADD_REP_CHR(base, '0', 1))
        goto failed_alloc;
      break;

    default:
      assert(FALSE);
  }

  if (field_width > 0)
  {
    assert(spec->flags & LEFT);
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', field_width))
      goto failed_alloc;
    field_width = 0;
  }

  return (TRUE);

 failed_alloc:
  return (FALSE);
}
