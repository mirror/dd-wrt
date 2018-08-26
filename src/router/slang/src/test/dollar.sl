$1="Dollar-1";
$2 = "%$1${}%"$;

_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("dollar strings");

if (string(_debug_info) != "$_debug_info"$)
  failed ("_debug_info, got:" + "$_debug_info"$);

static variable This_is_static = "hello";
public variable This_is_public = "hello";

public define test_hello_world ()
{
   variable world = "world";
   if ("hello world" != "$This_is_static $world"$)
     failed ("hello world 1");

   if ("$This_is_static $world"$ != _$("$This_is_static $world"))
     failed ("hello world 1a");

   if ("hello world" != "$This_is_public $world"$)
     failed ("hello world 2");

   putenv ("WORLD=world");
   if ("hello world" != "$This_is_public $WORLD"$)
     failed ("hello world 3");

   if ("$This_is_public $WORLD"$ != _$("$This_is_public $WORLD"))
     failed ("hello world 3a");

   if ("hello world" != "$This_is_public $UNDEFINED$WORLD"$)
     failed ("hello world 4");

   if ("hello world" != "${This_is_static} ${world}"$)
     failed ("hello world 5");

   if ("hello world" != "${This_is_public} ${world}"$)
     failed ("hello world 6");

   putenv ("WORLD=world");
   if ("hello world" != "${This_is_public} ${WORLD}"$)
     failed ("hello world 7");

   if ("hello world" != "${This_is_public} ${UNDEFINED}${WORLD}"$)
     failed ("hello world 8");
}
test_hello_world ();

static define test_uninited ()
{
   variable a, b, c, d;
   a = "a";
   b = 2;
   c = Integer_Type[10];

   variable x = 0;
   try
     {
	d = "a=$a, b=$b, c=$c, d=$d"$;
	x++;
     }
   catch VariableUninitializedError:
   if (x != 0)
     failed ("unitialized");

   d = "a=$a, b=$b, c=$c, d=$e"$;
   if (d != sprintf ("a=%S, b=%S, c=%S, d=",a,b,c))
     failed ("d=e");
}
test_uninited ();

define test_special ()
{
   variable n = _NARGS;
   _pop_n (_NARGS);

   if ("$n"$ != "${_NARGS}"$)
     failed ("${_NARGS}");

   if ("$n"$ != "$_NARGS"$)
     failed ("$_NARGS");
}
test_special (1,2,3);
test_special ();

if ("$$$$$$$$"$ != "$$$$")
  failed ("$$$$$$$$");
if ("$"$ != "$")
  failed ("$");
$1="Dollar-1";
if ("$1"$ != string($1))
  failed ("$1");
if ("$*"$ != "$*")
  failed ("$*");
if ("$*$"$ != "$*$")
  failed ("$*$");
$2 = "%$1${}%"$;
if ($2 != "%Dollar-1%")
  failed ("%s, found \"%s\"","%$1${}%", $2);

if (eval ("static variable foo=7;\"$1$This_is_static${foo}\"$")
    != string ($1)+"7")
  failed ("eval");

print ("Ok\n");
exit (0);

