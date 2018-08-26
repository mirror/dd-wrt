/* Floating point exceptions */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#include <float.h>
#include <math.h>

#ifdef HAVE_FLOATINGPOINT_H
# include <floatingpoint.h>
#endif

#ifdef HAVE_FENV_H
# include <fenv.h>
#endif

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

#include "slang.h"
#include "_slang.h"

#ifdef HAVE_FECLEAREXCEPT
# define CLEAR_FP_EXCEPTION	(void)feclearexcept(FE_ALL_EXCEPT);
# define GET_FP_EXCEPTION	fetestexcept(FE_ALL_EXCEPT)
# ifndef FE_DIVBYZERO
#  define FE_DIVBYZERO	0
# endif
# ifndef FE_INVALID
#  define FE_INVALID	0
# endif
# ifndef FE_OVERFLOW
#  define FE_OVERFLOW	0
# endif
# ifndef FE_UNDERFLOW
#  define FE_UNDERFLOW	0
# endif
# ifndef FE_INEXACT
#  define FE_INEXACT	0
# endif
# define SYS_FE_DIVBYZERO	FE_DIVBYZERO
# define SYS_FE_INVALID		FE_INVALID
# define SYS_FE_OVERFLOW	FE_OVERFLOW
# define SYS_FE_UNDERFLOW	FE_UNDERFLOW
# define SYS_FE_INEXACT		FE_INEXACT
#else
# ifdef HAVE_FPSETSTICKY
#  define CLEAR_FP_EXCEPTION	(void)fpsetsticky(0)
#  define GET_FP_EXCEPTION	fpgetsticky()
#  define SYS_FE_DIVBYZERO	FP_X_DZ
#  define SYS_FE_INVALID	FP_X_INV
#  define SYS_FE_OVERFLOW	FP_X_OFL
#  define SYS_FE_UNDERFLOW	FP_X_UFL
#  define SYS_FE_INEXACT	FP_X_IMP
# else
/* ??? */
#  define CLEAR_FP_EXCEPTION	(void)0
#  define GET_FP_EXCEPTION	(0)
#  define SYS_FE_DIVBYZERO	0
#  define SYS_FE_INVALID	0
#  define SYS_FE_OVERFLOW	0
#  define SYS_FE_UNDERFLOW	0
#  define SYS_FE_INEXACT	0
# endif
#endif

void SLfpu_clear_except_bits (void)
{
   CLEAR_FP_EXCEPTION;
}

unsigned int SLfpu_test_except_bits (unsigned int bits)
{
   unsigned int rbits;
   unsigned long sysbits;

   sysbits = GET_FP_EXCEPTION;

   rbits = 0;
   if (sysbits & SYS_FE_DIVBYZERO) rbits |= SL_FE_DIVBYZERO;
   if (sysbits & SYS_FE_INVALID) rbits |= SL_FE_INVALID;
   if (sysbits & SYS_FE_OVERFLOW) rbits |= SL_FE_OVERFLOW;
   if (sysbits & SYS_FE_UNDERFLOW) rbits |= SL_FE_UNDERFLOW;
   if (sysbits & SYS_FE_INEXACT) rbits |= SL_FE_INEXACT;

   return rbits & bits;
}
