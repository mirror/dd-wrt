import ("rand");

private define get_generator_args (nargs, num_parms, parmsp, rtp, nump,
				   usage_str)
{
   @rtp = NULL;
   @nump = NULL;
   if (nargs == num_parms)
     {
	@parmsp = __pop_list (num_parms);
	return;
     }

   if (nargs == num_parms + 1)
     {
	@nump = ();
	variable parms = __pop_list (num_parms);
	if (typeof (parms[0]) == Rand_Type)
	  {
	     @rtp = list_pop (parms);
	     list_append (parms, @nump);
	     @nump = NULL;
	  }
	@parmsp = parms;
	return;
     }

   if (nargs == num_parms + 2)
     {
	@nump = ();
	@parmsp = __pop_list (num_parms);
	variable rt = ();
	if (typeof (rt) == Rand_Type)
	  {
	     @rtp = rt;
	     return;
	  }
     }
   else _pop_n (nargs);

   usage (usage_str);
}

private define call_rand_func ()
{
   variable num = ();
   variable args = __pop_list (_NARGS-3);
   variable rt, func;
   (func, rt) = ();

   if (rt == NULL)
     {
	if (num == NULL)
	  return (@func) (__push_list(args));

	return (@func) (__push_list(args), num);
     }

   if (num == NULL)
     return (@func)(rt, __push_list(args));

   return (@func)(rt, __push_list(args), num);
}

define rand_flat ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 2, &parms, &rt, &num,
		       "r = rand_flat ([Rand_Type,] xmin, xmax [,num])");

   variable r = call_rand_func (&rand_uniform, rt, num);

   return parms[0] + (parms[1] - parms[0])*__tmp(r);
}

define rand_chisq ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 1, &parms, &rt, &num,
		       "r = rand_chisq ([Rand_Type,] nu [,num])");
   return 2.0 * call_rand_func (&rand_gamma, rt, 0.5*parms[0], 1.0, num);
}

define rand_fdist ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 2, &parms, &rt, &num,
		       "r = rand_fdist ([Rand_Type,] nu1, nu2 [,num])");
   variable nu1 = parms[0], nu2 = parms[1];

   return (call_rand_func (&rand_gamma, rt, 0.5*nu1, 1.0, num)/nu1)
     / (call_rand_func(&rand_gamma, rt, 0.5*nu2, 1.0, num)/nu2);
}

define rand_tdist ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 1, &parms, &rt, &num,
		       "r = rand_tdist ([Rand_Type,] nu, [,num])");
   variable nu = parms[0];
   return call_rand_func (&rand_gauss, rt, 1.0, num)
     / sqrt(call_rand_func(&rand_chisq, rt, nu, num)/nu);
}

define rand_int ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 2, &parms, &rt, &num,
		       "r = rand_int ([Rand_Type,] imin, imax [,num])");

   variable r = call_rand_func (&rand_uniform, rt, num);

   return nint(parms[0] + (parms[1] - parms[0])*__tmp(r));
}

define rand_exp ()
{
   variable parms, rt, num;

   get_generator_args (_NARGS, 1, &parms, &rt, &num,
		       "r = rand_exp ([Rand_Type,] beta [,num])");

   return (-parms[0]) * log (call_rand_func (&rand_uniform_pos, rt, num));
}

private define make_indices (a, d, i)
{
   _for (0, length(array_shape(a))-1, 1)
     {
	variable j = ();
	if (j == d)
	  i;
	else
	  [:];
     }
}

define rand_sample ()
{
   if (_NARGS < 2)
     {
	_pop_n (_NARGS);
	usage ("(B1 [,B2,...]) = rand_sample ([Rand_Type,] A1 [A2,...], num)");
     }

   variable num = ();
   variable arrays = __pop_list (_NARGS-1);
   variable rt = NULL;

   if (typeof (arrays[0]) == Rand_Type)
     rt = list_pop (arrays);

   variable n0 = NULL, dim0;
   variable a, indices;

   foreach a (arrays)
     {
	dim0 = array_shape (a)[0];
	if (n0 == NULL)
	  {
	     n0 = dim0;
	     continue;
	  }
	if (n0 != dim0)
	  throw TypeMismatchError, "The arrays passed to rand_sample must have the same leading dimension";
     }

   if (num > n0)
     num = n0;

   if (rt == NULL)
     indices = rand_permutation (n0);
   else
     indices = rand_permutation (rt, n0);
   if (num < n0)
     indices = indices[[0:num-1]];

   foreach a (arrays)
     a[make_indices(a, 0, indices)];
}

