define print (x)
{
   x = string (x);
   () = fputs (x, stdout);
   () = fflush (stdout);
}

define testing_feature (f)
{
   variable u = "", nl = "";
   if (_slang_utf8_ok) u = " [UTF-8 mode]";
   if (f[-1] == '\n')
     {
	f = strtrim (f);
	nl = "\n";
     }
   () = fprintf (stdout, "Testing %s%s ...%s", f, u, nl);
   () = fflush (stdout);
}

new_exception ("TestError", AnyError, "Test Error");

define failed ()
{
   variable s = __pop_args (_NARGS);
   s = sprintf (__push_args(s));
   %() = fprintf (stderr, "Failed: %s\n", s);
   throw TestError, sprintf ("Failed: %s [utf8=%d]\n", s, _slang_utf8_ok);
   exit (1);
}

private variable _Random_Seed = 123456789UL * _time ();
define random ()
{
   _Random_Seed = (_Random_Seed * 69069UL + 1013904243UL)&0xFFFFFFFFU;
   return _Random_Seed/4294967296.0;
}

define urand ()
{
   if (_NARGS == 0)
     return random ();
   variable n = ();
   variable x = Double_Type [n];
   _for (0, n-1, 1)
     {
	variable i = ();
	x[i] = random ();
     }
   return x;
}
