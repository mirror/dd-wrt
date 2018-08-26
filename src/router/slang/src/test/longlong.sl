_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("long long");

#ifnexists LLong_Type
print ("Not implemented\n");
exit (0);
#stop
#endif

static define test_add (a, b, c)
{
   if (0 == __is_same (a+b, c))
     failed ("%S[%S] + %S[%S] != %S[%S]",
	     a, typeof(a), b, typeof(b), c, typeof(c));
}

static define test_sub (a, b, c)
{
   if (0 == __is_same (a-b, c))
     failed ("%S[%S] - %S[%S] != %S[%S]",
	     a, typeof(a), b, typeof(b), c, typeof(c));
}

static define test_mul (a, b, c)
{
   if (0 == __is_same (a*b, c))
     failed ("%S[%S] * %S[%S] != %S[%S]",
	     a, typeof(a), b, typeof(b), c, typeof(c));
}

static define test_div (a, b, c)
{
   if (0 == __is_same (a/b, c))
     failed ("%S[%S] / %S[%S] != %S[%S]",
	     a, typeof(a), b, typeof(b), c, typeof(c));
}

test_add (1LL, 2LL, 3LL);
test_add (1LL, 2ULL, 3ULL);
test_add (1ULL, 2ULL, 3ULL);
test_add (1ULL, 2LL, 3ULL);

test_add (123456789000000LL, 1h, 123456789000001LL);
test_add (2147483647, 100LL, 2147483747LL);

static variable X, Y;

X = 1LL;
Y = X; Y++;
test_add (X, 1, Y);
test_sub (X, -1, Y);

X = 1LL;
Y = X; Y--;
test_add (X, -1, Y);
test_sub (X, 1, Y);

X = 1LL;
Y = X; Y += 1;
test_add (X, 1, Y);
test_sub (X, -1, Y);

% test of atoll
%
#ifnexists atoll
failed ("atoll() is not defined\n");
#endif
X = atoll ("1");
if (LLong_Type != typeof(X))
  failed ("atoll(1) did not return a LLong_Type");
if (1LL != X)
  failed ("atoll(1) did not return 1LL");

private define test_sprintf (expected, fmt, val)
{
   if (expected != sprintf (fmt, val))
     failed ("sprintf failed for %S", val);
}

test_sprintf ("1234", "%lld", 1234);
test_sprintf ("1234", "%lld", 1234L);
test_sprintf ("1234", "%lld", 1234LL);
test_sprintf ("1234", "%lld", 1234ULL);
test_sprintf ("FFFF", "%llX", 0xFFFFULL);
test_sprintf ("FFFF", "%llX", 0xFFFF);
test_sprintf ("1234", "%llu", 1234);
test_sprintf ("1234", "%llu", 1234L);

print ("Ok\n");

exit (0);

