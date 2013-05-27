/* mpn_sbpi1_div_qr_sec, mpn_sbpi1_div_r_sec -- Compute Q = floor(U / V), U = U
   mod V.  Side-channel silent under the assumption that the used instructions
   are side-channel silent.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2011, 2012 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your option)
any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
for more details.

You should have received a copy of the GNU Lesser General Public License along
with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#if OPERATION_sbpi1_div_qr_sec
/* Needs (dn + 1) + (nn - dn) + (nn - dn) = 2nn - dn + 1 limbs at tp. */
#define FNAME mpn_sbpi1_div_qr_sec
#define Q(q) q,
#define RETTYPE mp_limb_t
#endif
#if OPERATION_sbpi1_div_r_sec
/* Needs (dn + 1) limbs at tp.  */
#define FNAME mpn_sbpi1_div_r_sec
#define Q(q)
#define RETTYPE void
#endif

RETTYPE
FNAME (Q(mp_ptr qp)
       mp_ptr np, mp_size_t nn,
       mp_srcptr dp, mp_size_t dn,
       mp_limb_t dinv,
       mp_ptr tp)
{
  mp_limb_t nh, cy, q1h, q0h, dummy, h;
  mp_size_t i;
  mp_ptr hp;
#if OPERATION_sbpi1_div_qr_sec
  mp_limb_t qh;
  mp_ptr qlp, qhp;
#endif

  ASSERT (dn >= 1);
  ASSERT (nn >= dn);
  ASSERT ((dp[dn - 1] & GMP_NUMB_HIGHBIT) != 0);

  if (nn == dn)
    {
      cy = mpn_sub_n (np, np, dp, dn);
      mpn_addcnd_n (np, np, dp, dn, cy);
#if OPERATION_sbpi1_div_qr_sec
      return 1 - cy;
#else
      return;
#endif
    }

  /* Create a divisor copy shifted half a limb.  */
  hp = tp;					/* (dn + 1) limbs */
  cy = mpn_lshift (hp, dp, dn, GMP_NUMB_BITS / 2);
  hp[dn] = dp[dn - 1] >> GMP_NUMB_BITS / 2;

#if OPERATION_sbpi1_div_qr_sec
  qlp = tp + (dn + 1);				/* (nn - dn) limbs */
  qhp = tp + (nn + 1);				/* (nn - dn) limbs */
#endif

  np += nn;

  /* Main loop.  Develop one full limb per iteration, but do it in two steps in
     order to avoid conditionals.  Quotient bits will be either correct or
     underestimates.  When a quotient is underestimated, the next quotient will
     compensate, since quotients are to be added at consecutive weight distance
     GMP_NUMB_BITS/2.  We make two quotient arrays, each with GMP_NUMB_BITS/2+2
     bits per entry.  The arrays are added late after the loop.  Separate
     arrays avoid data-dependent carry propagation.  */
  nh = 0;
  for (i = nn - dn - 1; i >= 0; i--)
    {
      np--;

      nh = (nh << GMP_NUMB_BITS/2) + (np[0] >> GMP_NUMB_BITS/2);
      umul_ppmm (q1h, dummy, nh, dinv);
      q1h += nh;
#if OPERATION_sbpi1_div_qr_sec
      qhp[i] = q1h;
#endif
      cy = mpn_submul_1 (np - dn, hp, dn + 1, q1h);

      nh = np[0];
      umul_ppmm (q0h, dummy, nh, dinv);
      q0h += nh;
#if OPERATION_sbpi1_div_qr_sec
      qlp[i] = q0h;
#endif
      cy = mpn_submul_1 (np - dn, dp, dn, q0h);

      nh -= cy;
    }

  np[0] = nh;

  np -= dn;

  /* 1st adjustment depends on extra high remainder limb.  */
  h = np[dn];
#if OPERATION_sbpi1_div_qr_sec
  qlp[0] += h;
#endif
  h -= mpn_subcnd_n (np, np, dp, dn, h);

  /* 2nd adjustment depends on remainder/divisor comparision as well as whether
     extra remainder limb was nullified by previous subtract.  */
  cy = mpn_sub_n (np, np, dp, dn);
  cy = cy == h;				/* FIXME: might leak on some archs */
#if OPERATION_sbpi1_div_qr_sec
  qlp[0] += cy;
#endif
  mpn_addcnd_n (np, np, dp, dn, 1 - cy);

  /* Combine quotient halves into final quotient.  */
#if OPERATION_sbpi1_div_qr_sec
  qh = 0;
  if (nn - dn != 0)
    {
      qh = mpn_lshift (qhp, qhp, nn - dn, GMP_NUMB_BITS/2);
      qh += mpn_add_n (qp, qhp, qlp, nn - dn);
    }

  return qh;
#else
  return;
#endif
}
