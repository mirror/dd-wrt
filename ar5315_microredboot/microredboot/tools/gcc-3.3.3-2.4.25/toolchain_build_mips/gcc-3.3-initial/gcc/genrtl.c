/* Generated automatically by gengenrtl from rtl.def.  */

#include "config.h"
#include "system.h"
#include "obstack.h"
#include "rtl.h"
#include "ggc.h"

extern struct obstack *rtl_obstack;

rtx
gen_rtx_fmt_s (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;

  return rt;
}

rtx
gen_rtx_fmt_ee (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_ue (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_iss (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     const char *arg1;
     const char *arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;
  XSTR (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_is (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     const char *arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_i (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;

  return rt;
}

rtx
gen_rtx_fmt_isE (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     const char *arg1;
     rtvec arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;
  XVEC (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_iE (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtvec arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XVEC (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_Ess (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     rtvec arg0;
     const char *arg1;
     const char *arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XVEC (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;
  XSTR (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_sEss (code, mode, arg0, arg1, arg2, arg3)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     rtvec arg1;
     const char *arg2;
     const char *arg3;
{
  rtx rt;
  rt = ggc_alloc_rtx (4);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XVEC (rt, 1) = arg1;
  XSTR (rt, 2) = arg2;
  XSTR (rt, 3) = arg3;

  return rt;
}

rtx
gen_rtx_fmt_eE (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtvec arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XVEC (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_E (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     rtvec arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XVEC (rt, 0) = arg0;

  return rt;
}

rtx
gen_rtx_fmt_e (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;

  return rt;
}

rtx
gen_rtx_fmt_ss (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     const char *arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_sies (code, mode, arg0, arg1, arg2, arg3)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     int arg1;
     rtx arg2;
     const char *arg3;
{
  rtx rt;
  rt = ggc_alloc_rtx (4);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XINT (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XSTR (rt, 3) = arg3;

  return rt;
}

rtx
gen_rtx_fmt_sse (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     const char *arg1;
     rtx arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_sE (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     rtvec arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XVEC (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_iuuBteiee (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtx arg1;
     rtx arg2;
     struct basic_block_def *arg3;
     union tree_node *arg4;
     rtx arg5;
     int arg6;
     rtx arg7;
     rtx arg8;
{
  rtx rt;
  rt = ggc_alloc_rtx (9);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XBBDEF (rt, 3) = arg3;
  XTREE (rt, 4) = arg4;
  XEXP (rt, 5) = arg5;
  XINT (rt, 6) = arg6;
  XEXP (rt, 7) = arg7;
  XEXP (rt, 8) = arg8;

  return rt;
}

rtx
gen_rtx_fmt_iuuBteiee0 (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtx arg1;
     rtx arg2;
     struct basic_block_def *arg3;
     union tree_node *arg4;
     rtx arg5;
     int arg6;
     rtx arg7;
     rtx arg8;
{
  rtx rt;
  rt = ggc_alloc_rtx (10);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XBBDEF (rt, 3) = arg3;
  XTREE (rt, 4) = arg4;
  XEXP (rt, 5) = arg5;
  XINT (rt, 6) = arg6;
  XEXP (rt, 7) = arg7;
  XEXP (rt, 8) = arg8;
  X0EXP (rt, 9) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_iuuBteieee (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtx arg1;
     rtx arg2;
     struct basic_block_def *arg3;
     union tree_node *arg4;
     rtx arg5;
     int arg6;
     rtx arg7;
     rtx arg8;
     rtx arg9;
{
  rtx rt;
  rt = ggc_alloc_rtx (10);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XBBDEF (rt, 3) = arg3;
  XTREE (rt, 4) = arg4;
  XEXP (rt, 5) = arg5;
  XINT (rt, 6) = arg6;
  XEXP (rt, 7) = arg7;
  XEXP (rt, 8) = arg8;
  XEXP (rt, 9) = arg9;

  return rt;
}

rtx
gen_rtx_fmt_iuu000000 (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtx arg1;
     rtx arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (9);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  X0EXP (rt, 3) = NULL_RTX;
  X0EXP (rt, 4) = NULL_RTX;
  X0EXP (rt, 5) = NULL_RTX;
  X0EXP (rt, 6) = NULL_RTX;
  X0EXP (rt, 7) = NULL_RTX;
  X0EXP (rt, 8) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_iuuB00is (code, mode, arg0, arg1, arg2, arg3, arg4, arg5)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     rtx arg1;
     rtx arg2;
     struct basic_block_def *arg3;
     int arg4;
     const char *arg5;
{
  rtx rt;
  rt = ggc_alloc_rtx (8);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XBBDEF (rt, 3) = arg3;
  X0EXP (rt, 4) = NULL_RTX;
  X0EXP (rt, 5) = NULL_RTX;
  XINT (rt, 6) = arg4;
  XSTR (rt, 7) = arg5;

  return rt;
}

rtx
gen_rtx_fmt_ssiEEsi (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
     RTX_CODE code;
     enum machine_mode mode;
     const char *arg0;
     const char *arg1;
     int arg2;
     rtvec arg3;
     rtvec arg4;
     const char *arg5;
     int arg6;
{
  rtx rt;
  rt = ggc_alloc_rtx (7);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XSTR (rt, 0) = arg0;
  XSTR (rt, 1) = arg1;
  XINT (rt, 2) = arg2;
  XVEC (rt, 3) = arg3;
  XVEC (rt, 4) = arg4;
  XSTR (rt, 5) = arg5;
  XINT (rt, 6) = arg6;

  return rt;
}

rtx
gen_rtx_fmt_Ei (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtvec arg0;
     int arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XVEC (rt, 0) = arg0;
  XINT (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_eEee0 (code, mode, arg0, arg1, arg2, arg3)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtvec arg1;
     rtx arg2;
     rtx arg3;
{
  rtx rt;
  rt = ggc_alloc_rtx (5);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XVEC (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XEXP (rt, 3) = arg3;
  X0EXP (rt, 4) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_eee (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
     rtx arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_ (code, mode)
     RTX_CODE code;
     enum machine_mode mode;
{
  rtx rt;
  rt = ggc_alloc_rtx (0);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);

  return rt;
}

rtx
gen_rtx_fmt_w (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     HOST_WIDE_INT arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XWINT (rt, 0) = arg0;

  return rt;
}

rtx
gen_rtx_fmt_wwww (code, mode, arg0, arg1, arg2, arg3)
     RTX_CODE code;
     enum machine_mode mode;
     HOST_WIDE_INT arg0;
     HOST_WIDE_INT arg1;
     HOST_WIDE_INT arg2;
     HOST_WIDE_INT arg3;
{
  rtx rt;
  rt = ggc_alloc_rtx (4);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XWINT (rt, 0) = arg0;
  XWINT (rt, 1) = arg1;
  XWINT (rt, 2) = arg2;
  XWINT (rt, 3) = arg3;

  return rt;
}

rtx
gen_rtx_fmt_0 (code, mode)
     RTX_CODE code;
     enum machine_mode mode;
{
  rtx rt;
  rt = ggc_alloc_rtx (1);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  X0EXP (rt, 0) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_i0 (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  X0EXP (rt, 1) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_ei (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     int arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XINT (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_e0 (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  X0EXP (rt, 1) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_u00 (code, mode, arg0)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  X0EXP (rt, 1) = NULL_RTX;
  X0EXP (rt, 2) = NULL_RTX;

  return rt;
}

rtx
gen_rtx_fmt_eit (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     int arg1;
     union tree_node *arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XINT (rt, 1) = arg1;
  XTREE (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_eeeee (code, mode, arg0, arg1, arg2, arg3, arg4)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
     rtx arg2;
     rtx arg3;
     rtx arg4;
{
  rtx rt;
  rt = ggc_alloc_rtx (5);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XEXP (rt, 3) = arg3;
  XEXP (rt, 4) = arg4;

  return rt;
}

rtx
gen_rtx_fmt_Ee (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     rtvec arg0;
     rtx arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XVEC (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_uuEiiiiiibbii (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
     rtvec arg2;
     int arg3;
     int arg4;
     int arg5;
     int arg6;
     int arg7;
     int arg8;
     struct bitmap_head_def *arg9;
     struct bitmap_head_def *arg10;
     int arg11;
     int arg12;
{
  rtx rt;
  rt = ggc_alloc_rtx (13);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XVEC (rt, 2) = arg2;
  XINT (rt, 3) = arg3;
  XINT (rt, 4) = arg4;
  XINT (rt, 5) = arg5;
  XINT (rt, 6) = arg6;
  XINT (rt, 7) = arg7;
  XINT (rt, 8) = arg8;
  XBITMAP (rt, 9) = arg9;
  XBITMAP (rt, 10) = arg10;
  XINT (rt, 11) = arg11;
  XINT (rt, 12) = arg12;

  return rt;
}

rtx
gen_rtx_fmt_iiiiiiiitt (code, mode, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
     RTX_CODE code;
     enum machine_mode mode;
     int arg0;
     int arg1;
     int arg2;
     int arg3;
     int arg4;
     int arg5;
     int arg6;
     int arg7;
     union tree_node *arg8;
     union tree_node *arg9;
{
  rtx rt;
  rt = ggc_alloc_rtx (10);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XINT (rt, 0) = arg0;
  XINT (rt, 1) = arg1;
  XINT (rt, 2) = arg2;
  XINT (rt, 3) = arg3;
  XINT (rt, 4) = arg4;
  XINT (rt, 5) = arg5;
  XINT (rt, 6) = arg6;
  XINT (rt, 7) = arg7;
  XTREE (rt, 8) = arg8;
  XTREE (rt, 9) = arg9;

  return rt;
}

rtx
gen_rtx_fmt_eti (code, mode, arg0, arg1, arg2)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     union tree_node *arg1;
     int arg2;
{
  rtx rt;
  rt = ggc_alloc_rtx (3);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XTREE (rt, 1) = arg1;
  XINT (rt, 2) = arg2;

  return rt;
}

rtx
gen_rtx_fmt_bi (code, mode, arg0, arg1)
     RTX_CODE code;
     enum machine_mode mode;
     struct bitmap_head_def *arg0;
     int arg1;
{
  rtx rt;
  rt = ggc_alloc_rtx (2);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XBITMAP (rt, 0) = arg0;
  XINT (rt, 1) = arg1;

  return rt;
}

rtx
gen_rtx_fmt_uuuu (code, mode, arg0, arg1, arg2, arg3)
     RTX_CODE code;
     enum machine_mode mode;
     rtx arg0;
     rtx arg1;
     rtx arg2;
     rtx arg3;
{
  rtx rt;
  rt = ggc_alloc_rtx (4);
  memset (rt, 0, sizeof (struct rtx_def) - sizeof (rtunion));

  PUT_CODE (rt, code);
  PUT_MODE (rt, mode);
  XEXP (rt, 0) = arg0;
  XEXP (rt, 1) = arg1;
  XEXP (rt, 2) = arg2;
  XEXP (rt, 3) = arg3;

  return rt;
}

