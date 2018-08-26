_debug_info = 1; () = evalfile ("inc.sl");
testing_feature ("syntax");

private define check_version_string ()
{
   variable vers = _slang_version_string;
   if (0 == strncmp (vers, "pre", 3))
     vers = vers[[3:]];
   variable v = strtok (vers, "-.");
   variable major = integer(v[0]);
   variable minor = integer(v[1]);
   variable micro = integer(v[2]);

   v = ((major*100)+minor)*100 + micro;
   if (v != _slang_version)
     failed ("slang version %S does not match %S", _slang_version, _slang_version_string);
}
check_version_string ();

pop(1);  % This tests pop as a special keyword

#ifexists test_char_return	       %  defined by the sltest script
if (0x12 != test_char_return (0x12)) failed ("test_char_return");
if (0x1234h != test_short_return (0x1234h)) failed ("test_short_return");
if (0x1234 != test_int_return (0x1234)) failed ("test_int_return");
if (0x12345678L != test_long_return (0x12345678L)) failed ("test_long_return");
% if (1.2e34f != test_float_return (1.2e34f)) failed ("test_float_return");
#ifexists Double_Type
if (1.2e34 != test_double_return (1.2e34)) failed ("test_double_return");
#endif
#endif

static define static_xxx ()
{
   return "xxx";
}

private define private_yyy ()
{
   return "yyy";
}

public define public_zzz ()
{
   return "zzz";
}

if (is_defined ("static_xxx") or "xxx" != static_xxx ())
  failed ("static_xxx");
if (is_defined ("private_yyy") or "yyy" != private_yyy ())
  failed ("private_yyy");
if (not is_defined ("public_zzz") or "zzz" != public_zzz ())
  failed ("public_xxx");

variable XXX = 1;
static define xxx ()
{
   variable XXX = 2;
   if (XXX != 2) failed ("local variable XXX");
}

xxx ();
if (XXX != 1) failed ("global variable XXX");
if (1)
{
   if (orelse
	{0}
	{0}
	{0}
	{0}
       )
     failed ("orelse");
}

!if (orelse
     {0}
     {0}
     {0}
     {1}) failed ("not orelse");

_auto_declare = 1;
XXX_auto_declared = 1;

if (&XXX_auto_declared != __get_reference ("XXX_auto_declared"))
  failed ("__get_reference");

if (&XXX_auto_declared != __get_reference (string(&XXX_auto_declared)))
  failed ("__get_reference via string form");

if (0 == __is_initialized (&XXX_auto_declared))
  failed ("__is_initialized");
() = __tmp (XXX_auto_declared);
if (__is_initialized (&XXX_auto_declared))
  failed ("__is_initialized __tmp");
XXX_auto_declared = "xxx";
__uninitialize (&XXX_auto_declared);
if (__is_initialized (&XXX_auto_declared))
  failed ("__is_initialized __uninitialize");

static define test_uninitialize ()
{
   variable x;
   if (__is_initialized (&x))
     failed ("__is_initialized x");
   x = 3;
   !if (__is_initialized (&x))
     failed ("__is_initialized x=3");
   if (3 != __tmp (x))
     failed ("__tmp return value");
   if (__is_initialized (&x))
     failed ("__tmp x");
   x = 4;
   __uninitialize (&x);
   if (__is_initialized (&x))
     failed ("__uninitialize x");
}

test_uninitialize ();

static define check_args (n)
{
   if (n + 1 != _NARGS)
     failed ("check_args %d", n);
   _pop_n (_NARGS-1);
}

static define nitems (n)
{
   loop (n) 1;
}

static define check_iscallable (x, y)
{
   if (y != __is_callable (x))
     failed ("__is_callable %S", x);
}

check_iscallable (1, 0);
check_iscallable (&_NARGS, 0);
check_iscallable (&check_iscallable, 1);
check_iscallable (&sin, 1);
check_iscallable (NULL, 0);

static define check_isnumeric (x, y)
{
   if (y != __is_numeric (x))
     failed ("__is_numeric %S", x);
}

check_isnumeric ("foo", 0);
check_isnumeric ("0", 0);
check_isnumeric (0, 1);
check_isnumeric (PI, 2);
#ifexists Complex_Type
check_isnumeric (2i, 3);
#endif
check_args (1, 1);
check_args (1,2,2);
check_args (nitems(3), nitems(5), 8);
static variable X = [1:10];
check_args (nitems (3), check_args(nitems(4), 4, 5), 3);

static define check_no_args ()
{
   if (_NARGS != 0)
     failed ("check_no_args");
}

% This failed in previous versions because abs was not treated as a function
% call.
if (abs (1) > 0)
  check_no_args ();

define check_tmp_optim ()
{
   variable a = [1:10:1.0];
   variable b = a*0.0;
   if ((a[0] != 1.0) or (__is_same(a,b)))
     failed ("__tmp optimization: a[0] = %f", a[0]);
}

check_tmp_optim ();

define check_for ()
{
   variable i;

   variable s = 0;
   _for (0, 10, 1)
     {
	i = ();
	s += i;
     }
   variable s1 = 0;

   _for i (0, 10, 1)
     s1 += i;

   if ((s1 != s) or (s != 55))
     failed ("_for: s1=%S, s=%S", s1, s);
}

check_for ();

define check_foreach ()
{
   variable i;

   variable s = 0;
   foreach ([0:10])
     {
	i = ();
	s += i;
     }

   variable s1 = 0;
   foreach i ([0:10])
     s1 += i;

   if ((s1 != s) or (s != 55))
     failed ("foreach");
}
check_foreach ();

$1 = 0;
static define check_NULL_args ()
{
   if (_NARGS != $1)
     failed ("check_NULL_args: _NARGS=%d != %d", _NARGS, $1);
}
$1 = 0; check_NULL_args ();
$1 = 2; check_NULL_args (,);
$1 = 3; check_NULL_args (,,);
$1 = 3; check_NULL_args (1,,);
$1 = 3; check_NULL_args (1,2,);
$1 = 3; check_NULL_args (1,,3);
$1 = 4; check_NULL_args (1,,3,);

static define test_strings (a, b)
{
   if (a != b)
     failed ("test_strings: %S != %S");
}

test_strings ("\\", "\x5C");
test_strings ("\\"Q, "\x5C");
test_strings ("\x5C"R, "\\x5C"Q);
test_strings ("\\"Q, "\x5C"Q);
%test_strings ("\"R, "\\");   % "%);

static define test_optimized ()
{
   variable x=10,y,z;
   (x,y,z) = (x*x,2*x+1,x+1);
   if (x != 100)
     failed ("test_optimized: x");
   if (y != 21)
     failed ("test_optimized: y");
   if (z != 11)
     failed ("test_optimized: z");
}
test_optimized ();

private define test (expr)
{
   if (eval (expr))
     failed (expr);
}
test ("-1 != 0-1");
test ("-1h != 0h-1h");
test ("-1L != 0L-1L");
#ifexists Double_Type
test ("-11^2 != -121");
test ("-2.0^2 != -4");
#endif

#ifexists test_pop_mmt
try
{
   () = test_pop_mmt (1.0);
}
catch ApplicationError;

() = test_pop_mmt (stdin);
#endif

define test_ternary ()
{
   variable arg, ans;
   foreach arg (['A', 'B', 'C', 'D', 'Z'])
     {
	ans =
	  arg == 'A' ? "A" :
	  arg == 'B' ? "B" :
	  arg == 'C' ? "C" :
	  arg == 'D' ? "D" :
	  "Z";

	if (ans[0] != arg)
	  failed ("ternary expression");
     }
   foreach arg (['A', 'B', 'C', 'D', 'Z'])
     {
	ans =
	  arg == 'A' - 1 + 1 ? "A" :
	  arg == 'B' - 1 + 1 ? "B" :
	  arg == 'C' - 1 + 1 ? "C" :
	  arg == 'D' - 1 + 1 ? "D" :
	  "Z";

	if (ans[0] != arg)
	  failed ("ternary expression involving binary ops");
     }
   foreach arg (['A', 'B', 'C', 'D', 'Z'])
     {
	ans =
	  arg == char('A')[0] - 1 + 1 ? "A" :
	  arg == char('B')[0] - 1 + 1 ? "B" :
	  arg == char('C')[0] - 1 + 1 ? "C" :
	  arg == char('D')[0] - 1 + 1 ? "D" :
	  "Z";
	if (ans[0] != arg)
	  failed ("ternary expression involving array index");
     }
}
test_ternary ();

private variable P = "foo";
private define test_private_variable_ops ()
{
   % This test is for valgrind
   loop (5)
     {
	variable x;
	foreach x ({2, [1,2,3], "foobar"})
	  {
	     x = x;
	     P = x;
	     P += x;
	     P += (x+x);
	     P = P;
	     P += P;
	     P = __tmp(P);

	     x = x+x;
	     x = x;
	     P = x;
	     P += x;
	     P += (x+x);
	     P = P;
	     P += P;
	     P = __tmp(P);
	  }
     }
}
test_private_variable_ops ();

private define test_assignment_expressions ()
{
   variable a = [1,2,3];
   (a)[1] = -1;
   if ((a[1] != -1) || ((a)[1]!=-1))
     failed ("lvalue parse problem: (a)[1]=-1");

   (a)[1] += 10*(a)[1];
   if ((a[1] != -11)||(a[1]!=-11))
     failed ("lvalue parse problem: (a)[1] += 10*(a)[1]");

   (a+a)[1] = -1;
}
test_assignment_expressions ();
try
{
   () = evalfile("./longline.inc");
}
catch LimitExceededError;

private define test_binary (astr, b)
{
   variable a = eval (astr);
   if (a != b)
     {
	failed ("Binary string %S ==> %S != %S", astr, a, b);
     }
   if (b >= 0)
     {
	variable fmt = sprintf ("%%#.%dB", strbytelen (astr)-2);
	variable bstr = sprintf (fmt, b);
	if (bstr != astr)
	  {
	     failed ("%S != %S=sprintf(%S, %S)", astr, bstr, fmt, b);
	  }
     }
}
test_binary ("0b0", 0);
test_binary ("0b00", 0);
test_binary ("0b1", 1);
test_binary ("-0b1", -1);
test_binary ("0b01", 1);
test_binary ("0b0001", 1);
test_binary ("0b0011", 3);
test_binary ("0b11", 3);
test_binary ("0b111", 7);
test_binary ("0b1110", 14);
test_binary ("-0b1110", -14);

$1 = -0;
print ("Ok\n");

exit (0);

