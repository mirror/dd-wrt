() = evalfile ("inc.sl");

testing_feature ("sorting");

private define test_simple_sort (n, dir)
{
   variable cmp_op = (dir > 0) ? &_op_lt : &_op_gt;
   variable ishift = [[1:n-1], 0];
   variable x, dx, bad;

   foreach ([Int_Type, Float_Type, Double_Type,
	     Short_Type, Char_Type, Long_Type,
	     UInt_Type, UChar_Type, ULong_Type,
#ifexists LLong_Type
	     LLong_Type, ULLong_Type
#endif
	    ])
     {
	variable type = ();
	loop (10)
	  {
	     x = typecast (1000*(urand (n)), type);
	     x = x[array_sort (x; dir=dir)];
	     dx = x[ishift] - 1.0*x;
	     dx[-1] = 0;
	     if (any((@cmp_op)(dx, 0)))
	       failed ("simple sort: %S", type);

	     % Already sorted
	     x = x[array_sort (x; dir=dir)];
	     dx = x[ishift] - 1.0*x;
	     dx[-1] = 0;
	     if (any((@cmp_op)(dx, 0)))
	       failed ("simple sort already sorted: %S", type);

	     % Reversed
	     array_reverse (x);
	     x = x[array_sort (x; dir=dir)];
	     dx = x[ishift] - 1.0*x;
	     dx[-1] = 0;
	     if (any((@cmp_op)(dx, 0)))
	       failed ("simple sort reversed: %S", type);
	  }
     }
}

define run_test_simple_sorts (method)
{
   set_default_sort_method (method);
   if (method != get_default_sort_method ())
     failed ("get/set_default_sort_method");

   test_simple_sort (1, 1);
   test_simple_sort (2, 1);
   test_simple_sort (3, 1);
   test_simple_sort (4, 1);
   test_simple_sort (5, 1);
   test_simple_sort (14, 1);
   test_simple_sort (15, 1);
   test_simple_sort (16, 1);
   test_simple_sort (17, 1);
   test_simple_sort (1023, 1);
   test_simple_sort (1024, 1);
   test_simple_sort (1025, 1);
   test_simple_sort (2, -1);
   test_simple_sort (3, -1);
   test_simple_sort (15, -1);
   test_simple_sort (1023, -1);
   test_simple_sort (1024, -1);
   test_simple_sort (1025, -1);
}
run_test_simple_sorts ("qsort");
run_test_simple_sorts ("msort");

private define opaque_sort_func (s, i, j)
{
   return sign (s.a[i] - s.a[j]);
}

private define cmp_fun (a, b)
{
   return sign (a-b);
}

private define test_sort (x, n, ans)
{
   variable i, a;

   a = x[array_sort (x;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(int x)");

   a = 1.0f*x;
   a = a[array_sort (a;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(float x)");

   a = 1.0*x;
   a = a[array_sort (a;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(double x)");

   a = x[array_sort (x, &cmp_fun;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(int x, &cmp_fun)");

   a = 1.0f*x;
   a = a[array_sort (a, &cmp_fun;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(float x, &cmp_fun)");

   a = 1.0*x;
   a = a[array_sort (a, &cmp_fun;; __qualifiers)];
   ifnot (_eqs (a, ans))
     failed ("array_sort(double x, &cmp_fun)");

   variable s = struct {a};
   s.a = 1.0*x;
   i = array_sort(s, &opaque_sort_func, n;; __qualifiers);
   a = s.a[i];
   ifnot (_eqs (a, ans))
     failed ("array_sort(struct, &opaque_sort_func, n)");
}

private define run_test_sort (method)
{
   set_default_sort_method (method);
   if (method != get_default_sort_method ())
     failed ("get/set_default_sort_method");

   variable A = [2, 3, 7, 9, 4, 1, 0, 8, 6, 5];
   test_sort (A, length(A), [0:length(A)-1]);
   test_sort ([2], 1, [2]);
   test_sort (A[[0:-1]], 0, A[[0:-1]]);

   test_sort (A, length(A), [length(A)-1:0:-1] ;dir=-1);
   test_sort ([2], 1, [2]; dir=-1);
   test_sort (A[[0:-1]], 0, A[[0:-1]]; dir=-1);
}

private define test_stability (method)
{
   variable a = [1, 2, 3, 3, 4, 5, 5, 6];
   variable i = [7, 5, 6, 4, 2, 3, 1, 0];

   variable j = array_sort (a; dir=-1, method=method);
   ifnot (_eqs (j, i))
     failed ("[%s]: descend sort was not stable", method);

   a = [6, 6, 5, 4, 4, 3, 2, 1, 1];
   i = [7, 8, 6, 5, 3, 4, 2, 0, 1];

   j = array_sort (a; dir=1, method=method);
   ifnot (_eqs (j, i))
     failed ("[%s]: ascend sort was not stable", method);

}
test_stability ("qsort");
test_stability ("msort");

print ("Ok\n");

exit (0);

