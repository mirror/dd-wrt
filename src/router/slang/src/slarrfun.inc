/* -*- mode: C -*- */

/* Some "inline" functions for generic scalar types */

#if SLANG_HAS_FLOAT
# ifndef IS_INFINITY
#  ifdef HAVE_INF
#   define IS_INFINITY(x) isinf(x)
#  else
#   define IS_INFINITY(x) _pSLmath_isinf(x)
#  endif
# endif
#endif

#ifdef TRANSPOSE_2D_ARRAY
static SLang_Array_Type *TRANSPOSE_2D_ARRAY (SLang_Array_Type *at, SLang_Array_Type *bt)
{
   GENERIC_TYPE *a_data, *b_data;
   SLindex_Type nr, nc, i;

   nr = at->dims[0];
   nc = at->dims[1];

   a_data = (GENERIC_TYPE *) at->data;
   b_data = (GENERIC_TYPE *) bt->data;

   for (i = 0; i < nr; i++)
     {
	GENERIC_TYPE *offset = b_data + i;
	int j;
	for (j = 0; j < nc; j++)
	  {
	     *offset = *a_data++;
	     offset += nr;
	  }
     }
   return bt;
}
#undef TRANSPOSE_2D_ARRAY
#endif

#ifdef INNERPROD_FUNCTION

static void INNERPROD_FUNCTION
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   GENERIC_TYPE_A *a;
   GENERIC_TYPE_B *b;
   GENERIC_TYPE_C *c;
   unsigned int kmin;
   unsigned int block = Inner_Prod_Block_Size;

   block *= sizeof (double)/sizeof(GENERIC_TYPE_B);

   c = (GENERIC_TYPE_C *) ct->data;
   b = (GENERIC_TYPE_B *) bt->data;
   a = (GENERIC_TYPE_A *) at->data;
#if 1
   for (kmin = 0; kmin < inner_loops; kmin += block)
     {
	unsigned int jmin;
	unsigned int kmax = kmin + block;
	if (kmax > inner_loops) kmax = inner_loops;

	for (jmin = 0; jmin < b_loops; jmin += block)
	  {
	     unsigned int i;
	     unsigned int jmax = jmin + block;
	     if (jmax > b_loops) jmax = b_loops;

	     for (i = 0; i < a_loops; i++)
	       {
		  GENERIC_TYPE_A *aa = a + i * a_stride;
		  GENERIC_TYPE_C *cc = c + i * b_loops;
		  unsigned int k;

		  for (k = kmin; k < kmax; k++)
		    {
		       double x = (double) aa[k];

		       if (x != 0.0)
			 {
			    unsigned int j;
			    GENERIC_TYPE_B *bb = b + b_inc*k;

			    j = jmin;
			    if (j + 8 < jmax)
			      {
				 unsigned int jmax1 = jmax - 8;
				 while (j < jmax1)
				   {
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				      cc[j] += x * bb[j]; j++;
				   }
			      }
			    while (j < jmax)
			      {
				 cc[j] += x * bb[j]; j++;
			      }
			 }
		    }
	       }
	  }
     }
#else
   while (a_loops--)
     {
	GENERIC_TYPE_B *bb;
	unsigned int j;

	bb = b;

	for (j = 0; j < inner_loops; j++)
	  {
	     double x = (double) a[j];

	     if (x != 0.0)
	       {
		  unsigned int k;

		  for (k = 0; k < b_loops; k++)
		    c[k] += x * bb[k];
	       }
	     bb += b_inc;
	  }
	c += b_loops;
	a += a_stride;
     }
#endif
}
#undef INNERPROD_FUNCTION

#undef GENERIC_TYPE_A
#undef GENERIC_TYPE_B
#undef GENERIC_TYPE_C
#endif

#ifdef INNERPROD_COMPLEX_A
static void INNERPROD_COMPLEX_A
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   double *a;
   GENERIC_TYPE *b;
   double *c;

   c = (double *) ct->data;
   b = (GENERIC_TYPE *) bt->data;
   a = (double *) at->data;

   a_stride *= 2;

   while (a_loops--)
     {
	GENERIC_TYPE *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;

	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     double *aa;
	     GENERIC_TYPE *bbb;

	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += aa[0] * (double)bbb[0];
		  imag_sum += aa[1] * (double)bbb[0];
		  aa += 2;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb++;
	  }

	a += a_stride;
     }
}

static void INNERPROD_A_COMPLEX
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   GENERIC_TYPE *a;
   double *b;
   double *c;

   c = (double *) ct->data;
   b = (double *) bt->data;
   a = (GENERIC_TYPE *) at->data;

   b_inc *= 2;

   while (a_loops--)
     {
	double *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;

	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     GENERIC_TYPE *aa;
	     double *bbb;

	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += (double)aa[0] * bbb[0];
		  imag_sum += (double)aa[0] * bbb[1];
		  aa += 1;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb += 2;
	  }

	a += a_stride;
     }
}

#undef INNERPROD_A_COMPLEX
#undef INNERPROD_COMPLEX_A
#endif				       /* INNERPROD_COMPLEX_A */

#ifdef INNERPROD_COMPLEX_COMPLEX
static void INNERPROD_COMPLEX_COMPLEX
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   double *a;
   double *b;
   double *c;

   c = (double *) ct->data;
   b = (double *) bt->data;
   a = (double *) at->data;

   a_stride *= 2;
   b_inc *= 2;

   while (a_loops--)
     {
	double *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;

	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     double *aa;
	     double *bbb;

	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += aa[0]*bbb[0] - aa[1]*bbb[1];
		  imag_sum += aa[0]*bbb[1] + aa[1]*bbb[0];
		  aa += 2;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb += 2;
	  }

	a += a_stride;
     }
}
#undef INNERPROD_COMPLEX_COMPLEX
#endif

#ifdef SUM_FUNCTION
#if SLANG_HAS_FLOAT
static int SUM_FUNCTION (VOID_STAR xp, unsigned int inc, unsigned int num, VOID_STAR yp)
{
   GENERIC_TYPE *x, *xmax;
   double sum, sumerr;

   x = (GENERIC_TYPE *) xp;
   xmax = x + num;

   sumerr = 0.0;
   sum = 0.0;
   while (x < xmax)
     {
	double v = *x - sumerr;
	double new_sum = sum + v;
	sumerr = (new_sum - sum) - v;
	sum = new_sum;
	x += inc;
     }
   *(SUM_RESULT_TYPE *)yp = (SUM_RESULT_TYPE) sum;
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef SUM_FUNCTION
#endif

#ifdef SUMSQ_FUNCTION
#if SLANG_HAS_FLOAT
static int SUMSQ_FUNCTION (VOID_STAR xp, unsigned int inc, unsigned int num, VOID_STAR yp)
{
   GENERIC_TYPE *x, *xmax;
   double sum, sumerr;

   sum = 0.0;
   sumerr = 0.0;

   x = (GENERIC_TYPE *) xp;
   xmax = x + num;
   while (x < xmax)
     {
	double v = (double)(*x) * (double)(*x) - sumerr;
	double new_sum = sum + v;
	sumerr = (new_sum - sum) - v;
	sum = new_sum;
	x += inc;
     }
   *(SUM_RESULT_TYPE *)yp = (SUM_RESULT_TYPE) sum;
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef SUMSQ_FUNCTION
#endif
#undef SUM_RESULT_TYPE

#ifdef MIN_FUNCTION
static int
MIN_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR sp)
{
   unsigned int n, n0;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *)ip;

   if (-1 == check_for_empty_array ("min", num))
     return -1;

# ifdef IS_NAN_FUNCTION
   n0 = 0;
   do
     {
	m = i[n0];
	n0 += inc;
     }
   while (IS_NAN_FUNCTION(m) && (n0 < num));
# else
   m = i[0];
   n0 = inc;
# endif

   for (n = n0; n < num; n += inc)
     if (m > i[n]) m = i[n];

   *(GENERIC_TYPE *)sp = m;
   return 0;
}
#undef MIN_FUNCTION
#endif

#ifdef MINABS_FUNCTION
static int
MINABS_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR sp)
{
   unsigned int n, n0;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *)ip;

   if (-1 == check_for_empty_array ("minabs", num))
     return -1;

# ifdef IS_NAN_FUNCTION
   n0 = 0;
   do
     {
	m = ABS_FUNCTION(i[n0]);
	n0 += inc;
     }
   while (IS_NAN_FUNCTION(m) && (n0 < num));
# else
   m = ABS_FUNCTION(i[0]);
   n0 = inc;
# endif

   for (n = n0; n < num; n += inc)
     if (m > ABS_FUNCTION(i[n])) m = ABS_FUNCTION(i[n]);

   *(GENERIC_TYPE *)sp = m;
   return 0;
}
#undef MINABS_FUNCTION
#endif

#ifdef MAX_FUNCTION
static int
MAX_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR s)
{
   unsigned int n, n0;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *) ip;

   if (-1 == check_for_empty_array ("max", num))
     return -1;

# ifdef IS_NAN_FUNCTION
   n0 = 0;
   do
     {
	m = i[n0];
	n0 += inc;
     }
   while (IS_NAN_FUNCTION(m) && (n0 < num));
# else
   m = i[0];
   n0 = inc;
# endif

   for (n = n0; n < num; n += inc)
     if (m < i[n]) m = i[n];

   *(GENERIC_TYPE *)s = m;
   return 0;
}
#undef MAX_FUNCTION
#endif

#ifdef MAXABS_FUNCTION
static int
MAXABS_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR s)
{
   unsigned int n, n0;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *) ip;

   if (-1 == check_for_empty_array ("maxabs", num))
     return -1;

# ifdef IS_NAN_FUNCTION
   n0 = 0;
   do
     {
	m = ABS_FUNCTION(i[n0]);
	n0 += inc;
     }
   while (IS_NAN_FUNCTION(m) && (n0 < num));
# else
   m = ABS_FUNCTION(i[0]);
   n0 = inc;
# endif

   for (n = n0; n < num; n += inc)
     if (m < ABS_FUNCTION(i[n])) m = ABS_FUNCTION(i[n]);

   *(GENERIC_TYPE *)s = m;
   return 0;
}
#undef MAXABS_FUNCTION
#endif

#ifdef ANY_FUNCTION
static int
ANY_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR s)
{
   unsigned int n;
   GENERIC_TYPE *i = (GENERIC_TYPE *) ip;

   for (n = 0; n < num; n += inc)
     if (i[n] != 0)
       {
#ifdef IS_NAN_FUNCTION
	  if (IS_NAN_FUNCTION(i[n]))
	    continue;
#endif
	  *(char *)s = 1;
	  return 0;
       }

   *(char *)s = 0;
   return 0;
}
#undef ANY_FUNCTION
#endif

#ifdef ALL_FUNCTION
static int
ALL_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR s)
{
   unsigned int n;
   GENERIC_TYPE *i = (GENERIC_TYPE *) ip;

   if (num == 0)
     {
	*(char *)s = 0;
	return 0;
     }
   for (n = 0; n < num; n += inc)
     {
	if (i[n] == (GENERIC_TYPE)0)
	  {
	     *(char *)s = 0;
	     return 0;
	  }
#ifdef IS_NAN_FUNCTION
	/* I really do not want to call this for all numbers, nor do I know
	 * what makes most sense.  Doing nothing means that all(_NaN) is 1.
	 * Such an interpretation is consistent with using
	 *   length(x) == length(where (x))
	 */
#endif
     }

   *(char *)s = 1;
   return 0;
}
#undef ALL_FUNCTION
#endif

#ifdef CUMSUM_FUNCTION
#ifdef SLANG_HAS_FLOAT
static int
CUMSUM_FUNCTION (SLtype xtype, VOID_STAR xp, unsigned int inc,
		 unsigned int num,
		 SLtype ytype, VOID_STAR yp, VOID_STAR clientdata)
{
   GENERIC_TYPE *x, *xmax;
   CUMSUM_RESULT_TYPE *y;
   double c;
   double cerr;

   (void) xtype;
   (void) ytype;
   (void) clientdata;

   x = (GENERIC_TYPE *) xp;
   y = (CUMSUM_RESULT_TYPE *) yp;
   xmax = x + num;

   c = 0.0;
   cerr = 0.0;
   while (x < xmax)
     {
	double d = (double) *x - cerr;
	double c1 = c + d;
	cerr = (c1 - c) - d;
	c = c1;
	*y = (CUMSUM_RESULT_TYPE) c;
	x += inc;
	y += inc;
     }
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef CUMSUM_FUNCTION
#undef CUMSUM_RESULT_TYPE
#endif

#ifdef PROD_FUNCTION
#if SLANG_HAS_FLOAT
static int PROD_FUNCTION (VOID_STAR xp, unsigned int inc, unsigned int num, VOID_STAR yp)
{
   GENERIC_TYPE *x, *xmax;
   double prod;

   prod = 1.0;

   x = (GENERIC_TYPE *) xp;
   xmax = x + num;
   while (x < xmax)
     {
	prod *= (double) *x;
	x += inc;
     }
   *(PROD_RESULT_TYPE *)yp = (PROD_RESULT_TYPE) (prod);
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef PROD_FUNCTION
#undef PROD_RESULT_TYPE
#endif

#ifdef GENERIC_TYPE
# undef GENERIC_TYPE
#endif

#ifdef IS_NAN_FUNCTION
# undef IS_NAN_FUNCTION
#endif

#ifdef ABS_FUNCTION
# undef ABS_FUNCTION
#endif
