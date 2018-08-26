#ifnexists Testing_Autoload
public variable Testing_Autoload = 1;
() = evalfile ("./inc.sl");
testing_feature ("autoload");

public variable Num_Foo_Loads = 0;
public variable Num_Foo_Args = 0;
public variable Num = 0;

autoload ("foo", __FILE__);	       %  load foo into the global namespace
if (0 == is_defined ("foo"))
  failed ("autoload foo 1");

autoload ("ns2->foo", __FILE__);	       %  load foo into ns2

implements ("ns1");
# iffalse
% I want to add private namespaces before I allow this to be implicitly
% be loaded into the current namespace
autoload ("foo", __FILE__);
# else
autoload ("ns1->foo", __FILE__);
# endif
% Now foo should be define in ns1, as well as in the global
if (0 == is_defined ("ns1->foo"))
  failed ("autoload ns1->foo");

$1 = Global->foo (1,2,3);
if (Num_Foo_Loads != 1)
  failed ("Failed to load foo into the global namespace: %d", Num_Foo_Loads);
if (Num_Foo_Args != 3)
  failed ("_NARGS not 3 in autoloaded function");

%if ($1 == "Global")
%  failed ("Global namespace was used for private objects");

() = ns1->foo (1,2,3,4);
if (Num_Foo_Loads != 2)
  failed ("Failed to load foo into ns1");
if (Num_Foo_Args != 4)
  failed ("_NARGS not 4 in autoloaded function");

() = ns2->foo ();
if (Num_Foo_Loads != 3)
  failed ("Failed to load foo into ns2");
if (Num_Foo_Args != 0)
  failed ("_NARGS not 0 in autoloaded function");

() = evalfile (__FILE__, "ns3");
if (0 == is_defined ("ns3->foo"))
  failed ("ns3->foo is not defined");

if ("ns3" != ns3->foo())
  failed ("ns3->foo returned %s", ns3->foo ());
if ("ns2" != ns2->foo())
  failed ("ns2->foo returned %s", ns2->foo ());
if ("ns1" != ns1->foo())
  failed ("ns1->foo returned %s", ns1->foo ());

ns3->set_private_num (3);
ns2->set_private_num (2);
ns1->set_private_num (1);
Global->Num = 0;

if ((ns3->get_private_num () != 3)
    or (ns2->get_private_num () != 2)
    or (ns1->get_private_num () != 1))
  failed ("private namepaces were modified");

if (is_defined ("private_foo")
    or is_defined ("ns1->private_foo")
    or is_defined ("ns2->private_foo")
    or is_defined ("ns3->private_foo"))
  failed ("private_foo was defined in a non-private namespace");

print ("Ok\n");
exit (0);
#else
private variable My_Namespace = current_namespace ();
private variable Num = 1;
private define private_foo ()
{
   if (Num != 1)
     failed ("Private variable Num_Loads has been modified");
   return My_Namespace;
}

define get_private_num ()
{
   return Num;
}

define set_private_num (n)
{
   Num = n;
}

Num_Foo_Loads++;
define foo ()
{
   _pop_n (_NARGS);
   Num_Foo_Args = _NARGS;
   return private_foo ();
}
if (current_namespace () != foo (1,2,3,4,5))
  failed ("current_namespace: %s, Expected %s", current_namespace(), My_Namespace);
#endif
