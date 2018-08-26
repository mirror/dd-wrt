_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("NaN and Inf");

if (isnan (0)) failed ("isnan(0)");
if (isnan (0.0)) failed ("isnan(0.0)");
if (isinf (0)) failed ("isinf(0)");
if (isinf (0.0)) failed ("isinf(0.0)");

if (0 == isnan (_NaN)) failed ("isnan (_NaN)");
if (0 == isnan (-_NaN)) failed ("isnan (-_NaN)");
if (isinf (_NaN)) failed ("isinf(_NaN)");
if (isinf (-_NaN)) failed ("isinf(-_NaN)");

if (isnan (_Inf)) failed ("isnan (_Inf)");
if (isnan (-_Inf)) failed ("isnan (-_Inf)");
if (0 == isinf (_Inf)) failed ("isinf(_Inf)");
if (0 == isinf (-_Inf)) failed ("isinf(-_Inf)");

private define test_sscanf (ret, str, format, x, y)
{
   variable x1, y1, ret1;

   ret1 = sscanf (str, format, &x1, &y1);
   if (ret1 != ret)
     failed ("sscanf (str=%s using format=%s failed: returned %d",
	     str, format, ret1);
   if (ret > 0)
     {
	if (isnan (x))
	  {
	     ifnot (isnan(x1))
	       failed ("sscanf (str=%s using format=%s arg1 failure: got %S",
		       str, format, x1);
	  }
	else if (isinf (x))
	  {
	     if ((isnan(x1))
		 || (x1 != x))
	       failed ("sscanf (str=%s using format=%s arg1 failure: got %S",
		       str, format, x1);
	  }
     }
   if (ret > 1)
     {
	if (isnan (y))
	  {
	     ifnot (isnan(y1))
	       failed ("sscanf (str=%s using format=%s arg2 failure: got %S",
		       str, format, y1);
	  }
	else if (isinf (y))
	  {
	     if ((isnan(y1))
		 || (y1 != y))
	       failed ("sscanf (str=%s using format=%s arg2 failure: got %S",
		       str, format, y1);
	  }
     }
}

test_sscanf (1, "Nany", "%lf", _NaN, NULL);
test_sscanf (2, "NanInf", "%lf%lf", _NaN, _Inf);
test_sscanf (2, "Nan -Inf", "%lf %lf", _NaN, -_Inf);
test_sscanf (2, "Nan(xxx) -Inf", "%lf %lf", _NaN, -_Inf);
test_sscanf (2, "Nan() -Inf", "%lf %lf", _NaN, -_Inf);
test_sscanf (1, "Nan( -Inf", "%lf %lf", _NaN, -_Inf);
test_sscanf (2, "-Nan() -Inf", "%lf %lf", _NaN, -_Inf);

print ("Ok\n");

exit (0);

