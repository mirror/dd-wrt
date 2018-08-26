_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("sscanf");

#ifexists Double_Type
static variable eps = 1.0;
while (1 + eps/2.0 != 1)
  eps /= 2.0;

static define feqs (x, y)
{
   if (x == y)
     return 1;

   % (delta_diff)^2 = (delta_y)^ + (delta_x)^2
   % delta_y = eps * y
   % (delta_diff)^2 = eps*eps (y^2 + x^2)
   % |delta_diff| = eps * sqrt (y^2 + x^2) ~= eps * x *sqrt(2)
   variable diff = y - x;
   if (x < 0) x = -x;
   if (y < 0) y = -y;
   if (diff < 0) diff = -diff;
   variable tol = ((x + y) * eps);

   if (diff <= tol)
     return 1;
   vmessage ("diff = %e, abs(x)*eps = %e, error=%e",
	     diff, tol, diff/(x+y));
   return 1;
}

static variable Inf = 1e1000;
static define test_atof (x)
{
   variable y;
   variable str = sprintf ("%.64e", x);
   variable tstr;

   tstr = strup (strtrim (str));

   if (tstr == "INF")
     y = Inf;
   else if (tstr == "-INF")
     y = -Inf;
   else
     y = atof (str);

   !if (feqs (x,y))
     failed ("%e = atof(%e [%s]): diff = %e\n", y, x, tstr, y-x);
}

static define test_atof_main (n)
{

   loop (n)
     {
	variable a,b,c;
	a = 500 - random () * 1000;
	b = 400 - 800 * random ();
	ERROR_BLOCK
	  {
	     _clear_error ();
	     () = fprintf (stderr, "Floating point exception occured for %g * 10^%g\n",
			   a, b);
	  }
	if (1)
	  {
	     c = a * 10.0^b;
	     test_atof (c);
	  }
     }

   test_atof (random ());
}
test_atof_main (1000);
#endif				       %  Double_Type

define test_scanf (buf, format, xp, yp, n)
{
   variable nn, x, y;
   nn = sscanf (buf, format, &x, &y);
   if (n != nn)
     failed ("sscanf (%s, %s, &x, &y) ==> returned %d",
	     buf, format, nn);
   if (n >= 1)
     {
	if (x != xp)
	  {
#ifexists Double_Type
	     if ((typeof (x) == Double_Type)
		 or (typeof (x) == Float_Type))
	       {
		  if (1)
		    failed ("sscanf (%s, %s, &x, &y) ==> x = %e, diff=%e",
			    buf, format, x, x - xp);
	       }
	     else
#endif
	       failed ("sscanf (%s, %s, &x, &y) ==> x = %S",
		       buf, format, x);
	  }
     }

   if (n >= 2)
     {
	if (y != yp)
	  {
#ifexists Double_Type
	     if ((typeof (y) == Double_Type)
		 or (typeof (y) == Float_Type))
	       failed ("sscanf (%s, %s, &x, &y) ==> y = %e, diff=%e",
		       buf, format, y, y - yp);
	     else
#endif
	       failed ("sscanf (%s, %s, &x, &y) ==> y = %S",
		       buf, format, y);
	  }
     }
}

test_scanf (" -30,,XX ,,2,3", "%2hd%4s", -3, "0,,X", 2);
test_scanf ("1,2,3", "%d,%2s", 1, "2,", 2);
test_scanf ("1,2 ,3", "%d,%2s", 1, "2", 2);
test_scanf ("1,2 ,3", "%d,%20s", 1, "2", 2);
test_scanf ("1,,,,2,3", "%d,%20s", 1, ",,,2,3", 2);
test_scanf ("1,    ,,,2,3", "%d,%20s", 1, ",,,2,3", 2);
test_scanf ("-30.1,,,,2,3", "%d,%2s", -30, "", 1);
test_scanf (" -30,,XX ,,2,3", "%d%4s", -30, ",,XX", 2);
test_scanf (" -30,,XX ,,2,3", "%hd%4s", -30, ",,XX", 2);
test_scanf (" -30,,XX ,,2,3", "%1hd%4s", -3, "0,,X", 0);
#ifexists Double_Type
test_scanf (" +30.173e-2,,XX ,,2,3", "%lf,,%4s", 30.173e-2, "XX", 2);
test_scanf (" -30.1,,XX ,,2,3", "%lf,,%4s", -30.1, "XX", 2);
test_scanf (" +30.1,,XX ,,2,3", "%lf,,%4s", 30.1, "XX", 2);
test_scanf (" +30.,,XX ,,2,3", "%lf,,%4s", 30.0, "XX", 2);
test_scanf (" +30.173,,XX ,,2,3", "%lf,,%4s", 30.173, "XX", 2);
test_scanf (" +30.173e+2,,XX ,,2,3", "%lf,,%4s", 30.173e2, "XX", 2);
test_scanf (" +30.173e-03,,XX ,,2,3", "%lf,,%4s", 30.173e-3, "XX", 2);
test_scanf (" +30.173E-03,,XX ,,2,3", "%lf,,%4s", 30.173e-3, "XX", 2);
test_scanf ("+.E", "%lf%lf", 0, 0, 0);
test_scanf ("+0.E", "%lf%s", 0, "E", 2);
test_scanf ("-0.E", "%lf%s", 0, "E", 2);
test_scanf ("-0.E-", "%lf%s", 0, "E-", 2);
test_scanf ("-0.E+", "%lf%s", 0, "E+", 2);
test_scanf ("-0.E+X", "%lf%s", 0, "E+X", 2);
test_scanf ("-1.E+0X", "%lf%s", -1, "X", 2);
test_scanf ("-0+X", "%lf%s", 0, "+X", 2);
test_scanf ("0+X", "%lf%s", 0, "+X", 2);
test_scanf ("0.000000000000E00+X", "%lf%s", 0, "+X", 2);
test_scanf ("1.000000000000E000000001+X", "%lf%s", 10, "+X", 2);
#endif

test_scanf (" hello world", "%s%s", "hello", "world", 2);
test_scanf (" hello world", "%s%c", "hello", ' ', 2);
test_scanf (" hello world", "%s%2c", "hello", " w", 2);
test_scanf (" hello world", "%s%5c", "hello", " worl", 2);
test_scanf (" hello world", "%s%6c", "hello", " world", 2);
test_scanf (" hello world", "%s%7c", "hello", " world", 2);
test_scanf (" hello world", "%s%1000c", "hello", " world", 2);

test_scanf (" hello world", "%*s%c%1000c", ' ', "world", 2);

test_scanf ("abcdefghijk", "%[a-c]%s", "abc", "defghijk", 2);
test_scanf ("abcdefghijk", "%4[a-z]%s", "abcd", "efghijk", 2);
test_scanf ("ab[-]cdefghijk", "%4[]ab]%s", "ab", "[-]cdefghijk", 2);
test_scanf ("ab[-]cdefghijk", "%40[][ab-]%s", "ab[-]", "cdefghijk", 2);
test_scanf ("ab12345cdefghijk", "ab%[^1-9]%s", "", "12345cdefghijk", 2);
test_scanf ("ab12345cdefghijk", "ab%3[^4-5]%s", "123", "45cdefghijk", 2);

test_scanf ("\t\n", "%s %s", "", "", 0);
test_scanf ("", "%s", "", "", 0);

define test_default_format ()
{
   loop (1000)
     {
	variable x = (2.0 * (random ()-0.5))
	  * 10^(40*(random()-0.5));
	if (x != eval(string(x)))
	  {
	     () = fprintf (stderr, "double %%S format failed for %.17g ==> %S\n", x, x);
	  }
	x = typecast (x, Float_Type);

	if (x != eval(string(x)+"f"))
	  {
	     () = fprintf (stderr, "float %%S format failed for %.17g ==> %S\n", x, x);
	  }
     }
}
test_default_format ();

print ("Ok\n");

exit (0);

