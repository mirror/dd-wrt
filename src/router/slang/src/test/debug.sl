_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("debugger support");
#ifnexists _set_frame_variable
print ("not implemented");
#stop
#endif

static define debug ()
{
   variable depth = 0;
   variable info;

   do
     {
	depth++;
	info = _get_frame_info (depth);
     }
   while (info.function != "fun0");
   _set_frame_variable (depth, "fun0_3", 3);
   if (_get_frame_variable (depth, "fun0_3") != 3)
     failed ("_get/set_frame_variable fun0_3");
}

static define fun2()
{
   if (_get_frame_depth () != 4)
     failed ("fun2 not at depth 4");
   variable foo = "zero";

   variable frame_info = _get_frame_info (0);
   if (length (frame_info.locals) != 2)
     failed ("length of frame_info");
   if (frame_info.function != "fun2")
     failed ("fun2: frame_info.function=%S", frame_info.function);
   if (frame_info.locals[1] != "frame_info")
     failed ("fun2: frame_info[1]=%S", frame_info.locals[1]);
   _set_frame_variable (0, "foo", 0);
   if (foo != 0)
     foo ("fun2: _set_frame_variable");
   if (0 != _get_frame_variable (0, "foo"))
     foo ("fun2: _get_frame_variable");
   debug();
}

static define fun1()
{
   if (_get_frame_depth () != 3)
     failed ("fun1 not at depth 3");
   fun2 ();
}
static define fun0()
{
   variable depth = _get_frame_depth ();
   if (_get_frame_depth () != 2)
     failed ("fun0 at depth %d, not at depth 2", depth);

   variable fun0_1 = "one";
   variable fun0_2 = "two";
   variable fun0_3 = "three";

   fun1 ();
   if (fun0_3 != 3)
     failed ("to set fun0_3");
}

fun0 ();

print ("Ok\n");

exit (0);

