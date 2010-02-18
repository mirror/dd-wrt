prepend_to_slang_load_path (".");
set_import_module_path (".:" + get_import_module_path ());
require("rand.sl");

private variable CLOSED_UPPER = 1;
private variable CLOSED_LOWER = 2;

define test_generic (func, ret_type, parms, exp_mean, exp_variance,
		     min_r, max_r, interval_flags)
{
   variable a, b, r;

   if (typeof ((@func) (__push_list(parms))) != ret_type)
     () = fprintf (stderr, "${func} did not produce a ${ret_type}\n"$);
   variable num = 10000;

   a = (@func) (__push_list(parms), num);
   if ((length (a) != num)
       || (_typeof (a) != ret_type))
     () = fprintf (stderr, "${func}(${num}) did not produce a ${num} ${ret_type}\n"$);

   if (any(isnan (a)))
     {
	() = fprintf (stderr, "${func} produced NaN values\n"$);
     }

   if ((min_r != NULL) && (max_r != NULL))
     {
	variable range_error = 0;
	range_error += ((interval_flags & CLOSED_LOWER) && any (a < min_r));
	range_error += (((interval_flags & CLOSED_LOWER) == 0) && any (a <= min_r));
	range_error += ((interval_flags & CLOSED_UPPER) && any (a > max_r));
	range_error += (((interval_flags & CLOSED_UPPER) == 0) && any (a >= max_r));

	if (range_error)
	  () = fprintf (stderr, "${func} produced values outside the ${min_r}-${max_r} range\n"$);
     }

   if (exp_variance != NULL)
     {
	variable mean = sum (a)/num;
	variable w = 1.0/sqrt(num);
	variable exp_stddev = sqrt(exp_variance);
	variable mean_lo = exp_mean - 3*exp_stddev*w;
	variable mean_hi = exp_mean + 3*exp_stddev*w;
	
	ifnot (mean_lo <= mean <= mean_hi)
	  () = fprintf (stderr, "${func}'s mean ${mean} outside the expected range: ${mean_lo} - ${mean_hi}\n"$);

	variable stddev = sqrt(sum((a-mean)^2)/num);
	ifnot (feqs (stddev, exp_stddev, 0.1, 1e-4))
	  {
	     () = fprintf (stderr, "${func}'s stddev ${stddev} differs from expected value ${exp_stddev} (var=${exp_variance})\n"$);
	     %print (a);
	  }
     }

   b = Int_Type[3,2,1];
   a = (@func) (__push_list(parms), b);
   ifnot (all (array_shape (a) == array_shape(b)))
     () = fprintf (stderr, "${func}(a) failed to produce an array with the dimensions of a");
}

define test_rand_uniform ()
{
   test_generic (&rand_uniform, Double_Type, {}, 0.5, 1.0/12, 0, 1, CLOSED_LOWER);
}

define test_rand_uniform_pos ()
{
   test_generic (&rand_uniform_pos, Double_Type, {}, 0.5, 1.0/12, 0, 1, 0);
}

define test_rand_gauss ()
{
   foreach ([0, 0.5, 1.0, 10.0])
     {
	variable sigma = ();
	test_generic (&rand_gauss, Double_Type, {sigma}, 0, sigma^2, NULL, NULL, CLOSED_LOWER);
     }
}

define test_rand_poisson ()
{
   foreach ([0.1, 1, 10, 100])
     {
	variable lam = ();
	test_generic (&rand_poisson, UInt_Type, {lam}, lam, lam,
		      0, _Inf, CLOSED_LOWER);
     }
}

define test_rand_gamma ()
{
   foreach ([0.1, 0.5, 10, 20])
     {
	variable theta = ();
	foreach ([1, 2, 4, 16])
	  {
	     variable k = ();
	     test_generic (&rand_gamma, Double_Type, {k, theta}, 
			   k*theta, k*theta^2,
			   0, _Inf, CLOSED_LOWER);
	  }
     }
}

define test_rand_binomial()
{
   variable p, n;
   foreach p ([0, 0.25, 0.75, 1.0])
     {
	foreach n ([1, 2, 10, 20])
	  {
	     test_generic (&rand_binomial, UInt_Type, {p, n},
			   n*p, n*p*(1-p),
			   0, n, CLOSED_LOWER|CLOSED_UPPER);
	  }
     }
}

define test_rand_beta()
{
   variable a, b;
   foreach a ([0.1, 0.5, 1, 2, 8])
     {
	foreach b ([0.1, 0.5, 1, 4, 16])
	  {
	     test_generic (&rand_beta, Double_Type, {a, b},
			   a/(a+b), (a*b)/(a+b)^2/(a+b+1),
			   0, 1, CLOSED_LOWER|CLOSED_UPPER);
	  }
     }
}

define test_rand_cauchy ()
{
   variable gamma;
   foreach gamma ([0.01, 0.1, 1, 10, 100])
     {
	test_generic (&rand_cauchy, Double_Type, {gamma},
		      0.0, NULL,
		      -_Inf, _Inf, 0);
     }
}

define test_rand_geometric ()
{
   variable p;
   foreach p ([0.01, 0.2, 0.6, 1.0])
     {
	test_generic (&rand_geometric, UInt_Type, {p},
		      1/p, (1-p)/p^2, 1, _Inf, CLOSED_LOWER);
     }   
}

define test_rand_flat ()
{
   variable x0 = 2-0.5, x1 = 2+0.5;
   test_generic (&rand_flat, Double_Type, {x0, x1},
		 0.5*(x0+x1), (x1-x0)^2/12.0, 
		 x0, x1, CLOSED_LOWER);
}


define test_rand_fdist ()
{
   variable nu1, nu2;
   foreach nu1 ([1,2,5,20])
     {
	foreach nu2 ([1,3,12,20])
	  {
	     variable mean = NULL, variance = NULL;
	     if (nu2 > 4)
	       {
		  mean = nu2/(nu2-2.0);
		  variance = 2.0*mean^2*(nu1+nu2-2.0)/nu1/(nu2-4.0);
	       }
	     test_generic (&rand_fdist, Double_Type, {nu1, nu2},
			   mean, variance, 0, _Inf, CLOSED_LOWER);
	  }
     }
}

define test_rand_chisq ()
{
   variable nu;
   foreach nu ([1,2,5,20])
     {
	test_generic (&rand_chisq, Double_Type, {nu},
		      nu, 2.0*nu, 0, _Inf, CLOSED_LOWER);
     }
}

define test_rand_tdist ()
{
   variable nu;
   foreach nu ([0.5,1,2,5,20])
     {
	variable mean = 0.0, variance = NULL;
	if (nu > 2)
	  variance = nu/(nu-2.0);

	test_generic (&rand_tdist, Double_Type, {nu},
		      mean, variance, -_Inf, _Inf, 0);
     }
}

define test_rand_int ()
{
   variable x0 = 1, x1 = 10;
   test_generic (&rand_int, Int_Type, {x0, x1},
		 0.5*(x0+x1), ((x1-x0+1)^2-1)/12.0, x0, x1, 
		 CLOSED_LOWER|CLOSED_UPPER);
}

define test_rand_exp ()
{
   variable beta;
   foreach beta ([0.1, 0.5, 1, 2, 10, 20])
     {
	test_generic (&rand_exp, Double_Type, {beta},
		      beta, beta^2, 0, _Inf, CLOSED_LOWER);
     }
}

private define test_rand ()
{
   variable r = rand_new ();
   srand (r, 12345);
   srand (12345);
   
   variable expected_values =
     [746892674, 935820662, 3317285904, 3160065947, 888929593, 316432806, 
      3891177748, 66504584, 827237220, 2731412032, 105892519, 1105593792, 
      4257164826, 2953826281, 477842505, 3161051103, 654741546, 2422625584, 
      3232900523, 2360188805, 70104872, 2440288176, 1468162482, 3428658486, 
      1893960569, 3842583023, 4119673423, 2214061288, 1769001282, 3996933411, 
      3342430755, 201679192, 431385446, 2648942401, 2718561501, 1689889009, 
      403183793, 662574206, 2167963286, 2166423399, 112978312, 3881586706, 
      2111007051, 2589350153, 3519959692, 838415425, 1148338613, 1576844827, 
      1939688263, 1896225294, 2843247453, 179614524, 594767376, 4056452573, 
      2301108737, 844660562, 1079103954, 3239244907, 3213734172, 3924276547
     ];
   variable a = rand (expected_values);
   variable b = rand (r, a);

   if (any (a != expected_values))
     () = fprintf (stderr, "The generator failed to produce the expected values");

   if (any (a != b))
     () = fprintf (stderr, "Seeding of the new generator failed\n");
}

define test_rand_permutation ()
{
   loop (20)
     {
	variable p = 1+rand_permutation (10);
	if ((prod(p) != 3628800) || (sum(p) != (10*11)/2))
	  {
	     () = fprintf (stderr, "rand_permutation failed\n");
	  }
     }
}

private define check_sampled_array (rt, a, b, n)
{
   variable dims = array_shape (a);
   dims[0] = n;
   ifnot (_eqs (dims, array_shape(b)))
     () = fprintf (stderr, "rand_sample failed for %S array using rt=%S, n=%S: found %S\n", a, rt, n,b);
}

private define sample_and_check_array (rt, a, n)
{
   variable b;
   if (rt == NULL)
     b = rand_sample (a, n);
   else
     b = rand_sample (rt, a, n);
   
   check_sampled_array (rt, a, b, n);
}


define test_rand_sample ()
{
   variable a0 = [1:20];
   variable a1 = _reshape ([1:20*30], [20, 30]);
   variable a2 = _reshape ([1:20*30*4], [20, 30, 4]);
   
   variable dims, a, b, n, b0, b1, b2;
   variable rt = rand_new ();
   foreach n ([5, 1, 20, 0])
     {
	foreach a ({a0,a1,a2})
	  {
	     sample_and_check_array (NULL, a, n);
	     sample_and_check_array (rt, a, n);
	  }
	(b0, b1, b2) = rand_sample (a0, a1, a2, n);
	check_sampled_array (NULL, a0, b0, n);
	check_sampled_array (NULL, a1, b1, n);
	check_sampled_array (NULL, a2, b2, n);

	(b0, b1, b2) = rand_sample (rt, a0, a1, a2, n);
	check_sampled_array (rt, a0, b0, n);
	check_sampled_array (rt, a1, b1, n);
	check_sampled_array (rt, a2, b2, n);
     }
}

define slsh_main ()
{
   test_rand ();
   test_rand_permutation ();
   test_rand_sample ();

   test_rand_uniform ();
   test_rand_uniform_pos ();
   test_rand_gauss ();
   test_rand_poisson ();
   test_rand_gamma ();
   test_rand_binomial ();
   test_rand_beta ();
   test_rand_cauchy ();
   test_rand_geometric ();
   test_rand_flat ();
   test_rand_fdist ();
   test_rand_chisq ();
   test_rand_tdist ();
   test_rand_int ();
   test_rand_exp ();
}
