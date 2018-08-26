_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("NameSpace routines");

if (current_namespace () != "")
  failed ("current_namespace - 1");

implements ("NSpace");
% From this point on, define and variable defaults to static

if (current_namespace () != "NSpace")
  failed ("current_namespace - 2");

define static_function ()
{
   "static_function";
}
variable static_variable = "static_variable";

public define public_function ()
{
   "public_function";
}
public variable public_variable = "public_variable";

private define private_function ()
{
   "private_function";
}
private variable private_variable = "private_variable";

!if (is_defined ("Global->public_function")) failed ("public_function");
!if (is_defined ("Global->public_variable")) failed ("public_variable");
!if (is_defined ("public_function")) failed ("public_function");
!if (is_defined ("public_variable")) failed ("public_variable");
!if (is_defined ("NSpace->static_function")) failed ("static_function");
!if (is_defined ("NSpace->static_variable")) failed ("static_variable");
if (is_defined ("NSpace->private_function")) failed ("private_function");
if (is_defined ("NSpace->private_variable")) failed ("private_variable");

if (static_variable != NSpace->static_variable) failed ("static_variable test");
if (public_variable != Global->public_variable) failed ("public_variable test");
if (private_variable != "private_variable") failed ("private_variable test");

public variable This_Namespace;

This_Namespace = "NS1";
() = evalfile ("ns1.inc");
This_Namespace = "NS2";
() = evalfile ("ns2.inc");

use_namespace ("NS1");
if (func () != "NS1")
  failed ("use_namespace 1");
define func1 ()
{
   return "1";
}
use_namespace ("NS2");
if (func () != "NS2")
  failed ("use_namespace 2");
define func1 ()
{
   return "2";
}
use_namespace ("Global");
if (is_defined ("func"))
  failed ("use_namespace Global");
define func1 ()
{
   return "3";
}
!if (is_defined ("func1"))
  failed ("use_namespace Global: func1");

if (NS1->func () != "NS1")
  failed ("NS1->func");
if (NS2->func () != "NS2")
  failed ("NS2->func");
if (NS1->func1 () != "1")
  failed ("NS1->func1");
if (NS2->func1 () != "2")
  failed ("NS2->func1");
if (Global->func1 () != "3")
  failed ("Global->func1");

if (length (_get_namespaces ()) != 4)  %  Global, NS1, NS2, NSpace
  failed ("_get_namespaces: %S", _get_namespaces());

% Test multiple namespaces in the same file.  Implements works such that
% if there is a namespace associated with the file, a new one will be created.
% Otherwise, the existing one will be used.  Since there was one created above,
% foo0 defined below is part of it.
use_namespace ("NSpace");
static define foo0();
implements ("foo1");
static define foo1 ();
implements ("foo2");
static define foo2 ();
implements ("foo3");
static define foo3 ();
use_namespace ("NSpace");
if (current_namespace () != "NSpace")
  failed ("Current Namespace after multiple implements");

static define test_foos (foo)
{
   variable f = sprintf ("%s->%s", foo, foo);
   if (0 == is_defined (f))
     failed ("%s", f);
}
test_foos ("foo1");
test_foos ("foo2");
test_foos ("foo3");

if (0 == is_defined ("NSpace->foo0"))
  failed ("NSpace->foo0");

if (0 != is_defined ("NSpace->exit"))
  failed ("NSpace->exit is defined");
fake_import ("NSpace");
if (0 == is_defined ("NSpace->exit"))
  failed ("NSpace->exit is not defined");
if (0 == is_defined ("Global->exit"))
  failed ("Global->exit is not defined");
if (is_defined ("foo1->exit"))
  failed ("foo1->exit is defined");

implements ("space3");
public variable Testing_Line_Token;
() = evalfile ("ns1.inc", current_namespace ());
if ("space3" != func ())
  failed ("nspace3");

print ("Ok\n");

exit (0);

print ("Ok\n");

exit (0);

