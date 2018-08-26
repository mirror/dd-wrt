_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("argv processing");

define play_games_width_argv ()
{
   try
     {
	eval ("__argc = 1;");
	failed ("__argc read-only test");
     }
   catch ReadOnlyError;

   try
     {
	eval ("__argv = [\"a\", \"b\"];");
	failed ("__argv = ... test");
     }
   catch ForbiddenError;

   try
     {
	if (length (__argv))
	  eval ("__argv[0] = \"a\";");
     }
   catch ReadOnlyError: failed ("__argv[0] is read-only");

   loop (20)
     {
	variable x = __argv;
     }
}
play_games_width_argv ();

define test_argv (argv)
{
   __set_argc_argv (argv);
   variable argc = length (argv);
   if (__argc != argc)
     failed ("Expected __argc to be %d, found %d", argc, __argc);
   if (length (argv) != __argc)
     failed ("__argv does not have the expected length, found %d", length(__argv));
   if (not _eqs (__argv, argv))
     failed ("__argv did not have the expected value");

   play_games_width_argv ();
}
test_argv (String_Type[0]);
test_argv (["a0"]);
test_argv (["a0", "b0"]);
test_argv (["a0", "b0", "c0"]);

print ("Ok\n");

exit (0);

