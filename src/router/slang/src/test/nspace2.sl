#ifexists This_Namespace
if (This_Namespace == "NS3")
  {
     if (current_namespace () != "NS3")
       {
	  failed ("evalfile in NS3");
       }
  }
else
  implements (This_Namespace);
%vmessage ("Loading ...");
% From this point on, define and variable defaults to static
private variable NS = This_Namespace;
define func ()
{
   return NS;
}
#else
variable This_Namespace;
_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("more NameSpace routines");
This_Namespace = "NS1";
loop (3)
  () = evalfile (__FILE__);
This_Namespace = "NS2";
() = evalfile (__FILE__);

This_Namespace = "NS3";
() = evalfile (__FILE__, "NS3");

use_namespace ("NS1");
if (func () != "NS1")
  failed ("use_namespace 1, found %s", func());

use_namespace ("NS2");
if (func () != "NS2")
  failed ("use_namespace 2");

use_namespace ("Global");
if (is_defined ("func"))
  failed ("use_namespace Global");

if (NS1->func () != "NS1")
  failed ("NS1->func");
if (NS2->func () != "NS2")
  failed ("NS2->func");

if ("NS1" != eval ("func", "NS1"))
  failed ("eval in NS1");
if ("NS2" != eval ("func", "NS2"))
  failed ("eval in NS2");
if ("NS3" != eval ("func", "NS3"))
  failed ("eval in NS3");

if ("NS4" != eval ("current_namespace()", "NS4"))
  failed ("eval in NS4");

implements ("foo");
variable X = "foo";
implements ("bar");
variable X = "bar";

if (foo->X != "foo")
  failed ("foo");
if (bar->X != "bar")
  failed ("bar");

print ("Ok\n");
exit (0);
#endif
