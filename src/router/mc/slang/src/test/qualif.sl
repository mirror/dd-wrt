() = evalfile ("inc.sl");

testing_feature ("qualifiers");

private define simple_test_function ()
{
   variable q = __qualifiers ();
   return q.qualifier;
}

if (3 != simple_test_function (;qualifier=3))
  failed ("Simple qualifier test");

public define qualifier_function ()
{
   variable q = __qualifiers ();
   loop (10) q = __qualifiers ();

   return _NARGS, q;
}

public variable Arg_List, Qual_Struct, Func_Ref, Struct;
private define call_qualifier_function (f, args, q)
{
   Arg_List = args;
   Qual_Struct = q;

   loop (10) () = __qualifiers (;foo="bar");
   variable args_expr 
     = strjoin (array_map (String_Type, &sprintf, 
			   "Arg_List[%d]", [0:length(args)-1]), ",");
   variable fields = get_struct_field_names (q);
   variable q_expr 
     = strjoin (array_map (String_Type, &sprintf, 
			   "%s=Qual_Struct.%s", fields, fields), ",");

   variable expr = sprintf ("%s (%s ; %s);", f, args_expr, q_expr);
   return eval (expr);
}

private define test_qualifiers (args, q)
{
   variable n = length (args), n1, q1;

   Func_Ref = &qualifier_function;
   Struct = struct { method = &qualifier_function };

   variable fs = ["qualifier_function", "(@Func_Ref)", "Struct.method"];
   variable ns = [n, n, n+1];

   variable f, i;
   _for i (0, 2, 1)
     {
	f = fs[i]; n = ns[i];
	(n1, q1) = call_qualifier_function (f, args, q);
	if (n != n1)
	  failed ("Expecting _NARGS to be %d, found %d  -- f = %s", n, n1, f);
	if (not _eqs (q, q1))
	  failed ("Qualifiers to do match the expected values -- f = %s", f);
     }
}

test_qualifiers ({1, 2, 3}, struct {a=1, b=3, c="foo"});
test_qualifiers ({1, 2, test_qualifiers ({1,2}, struct{c=&cos})},
		 struct {a=1, b=3, c="foo"} 
		 ; foo=1, bar=length ([1:10];baz=3));

private define test_qualifiers ()
{
   variable q = __qualifiers ();
   variable n, q1;

   (n, q1) = qualifier_function (1,, ;; q);
   if (n != 3)
     failed ("function call did not handle implicit NULL arguments");
   if (not _eqs (q, q1))
     failed (";; form of qualifiers");
}

test_qualifiers ();
test_qualifiers (;);
test_qualifiers (;;);
test_qualifiers (;;struct {foo=7});

private define fun2 (x, y)
{
   return qualifier ("x", x), qualifier ("y", y);
}

private define fun1 (x, y)
{
   return fun2 (x, y;; __qualifiers ());
}

private define test_qualifier ()
{
   variable x, y, x0, y0, x1, y1;
   
   x0 = 1; y0 = 2; x1 = "one"; y1 = "two";

   (x, y) = fun1 (x0, y0; x=x1, y=y1);
   if ((x != x1) || (y != y1))
     failed ("qualifier intrinsic 1");

   (x, y) = fun1 (x0, y0; y=y1);
   if ((x != x0) || (y != y1))
     failed ("qualifier intrinsic 2");

   (x, y) = fun1 (x0, y0; x = x1);
   if ((x != x1) || (y != y0))
     failed ("qualifier intrinsic 3");
}
test_qualifier ();

private define test_qualifier_exists (name, exists)
{
   if (exists != qualifier_exists (name))
     failed ("qualifier_exists (%s) != %d", name, exists);
}
test_qualifier_exists ("foo", 0);
test_qualifier_exists ("foo", 1; goo, foo=7);
test_qualifier_exists ("goo", 1; goo, foo=7);
test_qualifier_exists ("foo", 1; goo, foo);
test_qualifier_exists ("boo", 0; goo, foo);
test_qualifier_exists ("foo", 0; food=7);
test_qualifier_exists ("foo", 0;; struct{food=7});
test_qualifier_exists ("foo", 1;; struct{food=7, foo});

print ("Ok\n");

exit (0);

