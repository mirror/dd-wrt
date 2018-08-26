require("cmdopt.sl");

private define test_args (opts, args, expected_iend, s, val_list, should_error)
{
   try
     {
	variable iend = cmdopt_process (opts, args, 0);
	if (should_error)
	  throw RunTimeError;

	variable names = get_struct_field_names (s);
	_for (0, length(val_list)-1, 1)
	  {
	     variable i = ();
	     variable name = names[i];
	     variable val0 = get_struct_field (s, name);
	     variable val1 = val_list[i];

	     ifnot (_eqs (val0, val1))
	       throw RunTimeError, "Expecteding $name=$val1, found $val0"$;
	     i++;
	  }
	if (iend != expected_iend)
	  throw RunTimeError, "iend=${expected_iend}, found $iend"$;
     }
   catch AnyError:
     {
	if (should_error == 0)
	  throw;
     }
}

private define test_list_args (opts, args, expected_iend, listp, val_list, should_error)
{
   try
     {
	variable iend = opts.process (args, 0);
	if (should_error)
	  throw RunTimeError;

	variable list = @listp;
	if (iend != expected_iend)
	  throw RunTimeError, "iend=${expected_iend}, found $iend"$;

	if (length (list) != length (val_list))
	  throw RunTimeError, "list not of expected size";
	ifnot (_eqs (list, val_list))
	  throw RunTimeError, "list does not have expected items";
     }
   catch AnyError:
     {
	if (should_error == 0)
	  throw;
     }
}

private define callback_opt (value, ref)
{
   @ref = value;
}

private define callback_inc (ref)
{
   @ref += 1;
}

define slsh_main ()
{
   variable opts = cmdopt_new (NULL);
   variable s = struct
     {
	foo = 1,
	verb = 0,
	opt = NULL,
	a = 0,
	f = 1.0,
	l = 0,
	g, h,
	list,
	b = 0,
     };
   cmdopt_add (opts, "f|foo", &s.foo; type="int");
   cmdopt_add (opts, "v|verb", &s.verb; inc);
   cmdopt_add (opts, "opt", &s.opt; optional="opt-default", type="string");
   cmdopt_add (opts, "a|al", &s.a);
   cmdopt_add (opts, "float", &s.f; type="float");
   cmdopt_add (opts, "l|lil", &s.l; type="int", optional=3);

   variable args;

   set_struct_fields (s, 1,0,NULL,0,1.0,0);
   args = ["--foo", "7", "-vvv", "-al", "foo"];
   test_args (opts, args, 4, s, {7, 3, NULL, 1, 1.0, 3}, 0);

   set_struct_fields (s, 1,0,NULL,0,1.0,0);
   args = ["--foo=2", "-v", "-al", "--opt", "foo"];
   test_args (opts, args, 4, s, {2, 1, "opt-default", 1, 1.0, 3}, 0);

   set_struct_fields (s, 1,0,NULL,0,1.0,0);
   args = ["-vv", "-alf", "-1", "foo"];
   test_args (opts, args, 3, s, {-1, 2, NULL, 1, 1.0, 3}, 0);

   set_struct_fields (s, 1, 0, "opt", -1, PI, -1);
   args = ["-vv", "-f24", "-al", "--float", "-1", "-v", "--", "foo"];
   test_args (opts, args, 7, s, {24, 3, "opt", 1, -1.0, 3}, 0);

   set_struct_fields (s, 1, 0, "opt", -1, PI, -1);
   args = ["-vv", "-f-24", "-al", "--float", "-1", "-v", "-", "foo"];
   test_args (opts, args, 6, s, {-24, 3, "opt", 1, -1.0, 3}, 0);

   set_struct_fields (s, 1, 0, "opt", -1, PI, -1);
   args = ["-vv", "-f36", "-al", "--float", "-1", "-v"];
   test_args (opts, args, 6, s, {36, 3, "opt", 1, -1.0, 3}, 0);

   cmdopt_add (opts, "g|ginc", &callback_inc, &s.g; inc);
   cmdopt_add (opts, "h|hfun", &callback_opt, &s.h; type="int");

   set_struct_fields (s, 1, 0, "opt", -1, PI, -1, 0, 1);
   args = ["-vv", "-f35", "-g", "-al", "--float", "-1", "-h3", "-v"];
   test_args (opts, args, 8, s, {35, 3, "opt", 1, -1.0, 3, 1, 3}, 0);

   opts.add ("list", &s.list; type="string", optional="foo", append);
   args = ["--list", "--list=3", "--list=bar"];
   test_list_args (opts, args, 3, &s.list, {"foo", "3", "bar"}, 0);

   s = struct {bitmap = 1, flags=0x4|0x2};
   opts = cmdopt_new (NULL);
   opts.add ("a|b8000", &s.bitmap; bor=0x8000);
   opts.add ("b|b4000", &s.bitmap; bor=0x4000);
   opts.add ("f", &s.flags; bor=1, band=~0x2);
   args = ["-a", "-f", "--b4000"];
   test_args (opts, args, 3, s, {0x8000|0x4000|1, 0x4|0x1}, 0);
}
