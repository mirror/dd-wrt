_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("provide and require");

() = evalfile ("../../slsh/lib/require.sl");

public variable ReqFoo_Num_Loads = 0;
require ("reqfoo", "foo");
if (ReqFoo_Num_Loads != 1)
  failed ("reqfoo did not load into foo");
if (ReqFoo_Namespace != "foo")
  failed ("ReqFoo_Namespace not foo");

require ("reqfoo", "bar");
if (ReqFoo_Num_Loads != 2)
  failed ("reqfoo did not load into bar");
if (ReqFoo_Namespace != "bar")
  failed ("ReqFoo_Namespace not bar");
if (0 == is_defined ("bar->reqfoo"))
  failed ("bar->reqfoo");

implements ("baz");
require ("reqfoo");
if (ReqFoo_Num_Loads != 3)
  failed ("reqfoo not loaded into baz");
if (ReqFoo_Namespace != "baz")
  failed ("ReqFoo_Namespace is not baz");

implements ("buz");
require ("reqfoo");
if (ReqFoo_Num_Loads != 4)
  failed ("reqfoo not loaded into buz");
if (ReqFoo_Namespace != "buz")
  failed ("ReqFoo_Namespace is not buz");

if ((0 == is_defined ("buz->reqfoo")) or (0 == is_defined ("baz->reqfoo")))
  failed ("reqfoo is not in buz and baz namespaces");

use_namespace ("Global");
require ("reqfoo");
if (ReqFoo_Num_Loads != 5)
  failed ("reqfoo did not load into global");
require ("reqfoo");
if (ReqFoo_Num_Loads != 5)
  failed ("reqfoo loaded again into global");

print ("Ok\n");

exit (0);

