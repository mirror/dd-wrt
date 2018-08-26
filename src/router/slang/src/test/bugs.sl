_debug_info = 1; () = evalfile ("inc.sl");
print ("Known Bugs or not yet implemented features:\n");
implements ("bugs");

static define f()
{
   _pop_n (_NARGS);
   return _NARGS;
}

static variable A = [&f];
if (3 != (@A[0])(1,2,3))
  vmessage ("\tArray indexing/_NARGS interaction still present: %d",
	    (@A[0])(1,2,3));

static define func1(p1, p2) { return _NARGS; }
static variable A = [ &func1, &func1 ];
define test(which) { (@A[which])(which, which);}
if (2 != test(0))
  vmessage ("\tArray indexing/deref/_NARGS interaction present");
A = &f;
if (3 != (@A)(4,5,6))
  vmessage ("\tderef/_NARGS interaction present");

static variable s = struct
{
   method
};
s.method = &f;
if (3 != (@s.method)(4,5,6))
  vmessage ("\tUnexpected struct method/_NARGS interaction");
if (4 != s.method(7,8,9))
  vmessage ("\tUnexpected struct method call/_NARGS interaction");

if (1)
{
   $1 = 0;
   ERROR_BLOCK
     {
	$1 = 1;
	_clear_error();
     }
   static variable A = Assoc_Type[Int_Type];
   A["foo"] = [1:10];
   if ($1 == 0)
     vmessage ("\tAssoc_Type/Array conflict present");
   %else
   %  vmessage ("\tIgnore above error.");
}

$1 = [1L,1]; $1 = [1,1L];

$1 = Double_Type[2,2];
$1[*] = PI;
if (length (where ($1 == PI)) != length ($1))
  vmessage ("\tA[*] where A is multidimensional");

% This _used_ to cause a subtle leak
typedef struct {a} Foo_Type;
()=[Struct_Type[1],@Foo_Type];

#iffalse
% This "broken-feature" is nolonger supported.
$1 = [1:10];
if (10 != length ($1[[0:-1]]))
  vmessage ("\tSubscripting a 1d array with negative indices");
$1 = _reshape ($1, [5,2]);
if (10 != length ($1[[0:-1]]))
  vmessage ("\tSubscripting a 2d array with negative indices");
#endif

if (1)
{
   $1 = 0;
   ERROR_BLOCK
     {
	_clear_error();
	vmessage ("case Array_Type bug present");
     }
   switch ($1)
     {
      case "hello":
	failed ("case");
     }
     {
      case [1:10]:
	failed ("case");
     }
}

static define foo ()
{
   return "foo";
}
static define bar ()
{
   return "bar";
}
static variable A = [&foo, &bar];
if ("bar" != (@A[1])())
  vmessage ("Dereferencing a array element bug");

A = Struct_Type[2];
static variable S = struct
{
   addr
};
A[0] = S;
A[1] = @S;

A[0].addr = &$1;
A[1].addr = &$2;

try
{
   eval ("@A[0].addr = PI", "bugs");
   eval ("@(A[0].addr) = PI", "bugs");
}
catch NotImplementedError:
{
   vmessage ("\tDereferencing an array of struct fields");
}

static define test_xxx (xxx)
{
   variable x=0, y=0;
   loop (3)
     {
	x++;
	try eval(sprintf ("if (1) %s;", xxx));
	catch SyntaxError;
	y++;
     }
   return 10*y+x;
}

if (test_xxx ("break") != 33)
  vmessage ("\tbreak at top-level");

if (test_xxx ("continue") != 33)
  vmessage ("\tcontinue at top-level");

if (test_xxx ("return") != 33)
  vmessage ("\treturn at top-level");

static define test_function_name ()
{
   return eval ("_function_name()");
}
if ("" != test_function_name ())
  vmessage ("\t_function_name at top-level");

try
{
   $1 = Int_Type[10];
   eval("() = &$1[0];");
   %vmessage ("**** Expecting &X[0] to generate an error!!!");
}
catch NotImplementedError: vmessage ("\t&s[0] not supported");
try
{
   $1 = struct {foo};
   eval("() = &$1.foo;");
   %vmessage ("**** Expecting &s.foo to generate an error!!!");
}
catch NotImplementedError: vmessage ("\t&s.foo not supported");

try
{
   eval ("()++;--();++();");
   vmessage ("**** ++() not being caught by the parser!!!");
}
catch SyntaxError;

private define foo (i, x)
{
   if (x != NULL)
     throw TypeMismatchError;
   return int (i);
}

define test_array_map_bug ()
{
   try
     {
	() = array_map (Int_Type, &foo, [1:10], NULL);
     }
   catch AnyError:
     {
	vmessage ("\tarray_map bug present with NULL present");
     }
}
test_array_map_bug ();

define test_range_array_bug ()
{
   variable a = {1h, 1, 1L, 1LL};
   foreach (a)
     {
	variable i = ();
	if (_typeof ([i:i]) == typeof (i))
	  continue;
	vmessage ("\tRange arrays of Short, Long, LongLong types converted to Int_Type");
	return;
     }
}
test_range_array_bug ();

define test_overzealous_tmp_opt_bug ()
{
   variable a = Array_Type[1];
   a[0] = [1];
   () = a + 1;
   if (a[0][0] != 1)
     vmessage ("\tOver-zealous __tmp optimization array bug present");
}
test_overzealous_tmp_opt_bug ();

define test_array_index_bug ()
{
   variable x = Double_Type[7];
   try
     {
	x[[1:7]] = 0.0;
     }
   catch IndexError: return;
   throw InternalError, "test_array_index_bug: this bug has returned.";
}
test_array_index_bug ();

exit (0);

