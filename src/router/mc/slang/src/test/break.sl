_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("break and continue N");

define test_break_n ()
{
   variable n1 = 3, n2 = 5, n3 = 7;
   variable n1_loops = 0;
   variable n2_loops = 0;
   variable n3_loops = 0;
   loop (n1)
     {
	loop (n2)
	  {
	     loop (n3)
	       {
		  break 2;
		  n3_loops++;
	       }
	     n2_loops++;
	  }
	n1_loops++;
     }

   if ((n1_loops != n1) || (n2_loops) || (n3_loops))
     failed ("break 2");
}

define test_cont_n ()
{
   variable n1 = 3, n2 = 5, n3 = 7;
   variable n1_loops = 0;
   variable n2_loops = 0;
   variable n2_then_loops = 0;
   variable n3_loops = 0;
   variable n3_then_loops = 0;
   variable n3_finally_loops = 0;
   loop (n1)
     {
	loop (n2)
	  {
	     test_break_n ();
	     loop (n3)
	       {
		  try
		    {
		       continue 2;
		    }
		  finally
		    {
		       n3_finally_loops++;
		    }
		  n3_loops++;
	       }
	     then
	       {
		  n3_then_loops++;
	       }
	     n2_loops++;
	  }
	then
	  {
	     n2_then_loops++;
	  }
	n1_loops++;
     }

   if (n3_finally_loops != (n1*n2))
     failed ("continue 2 with finally");
   if ((n1_loops != n1) || (n2_loops) || (n3_loops))
     failed ("continue 2");
   if (n3_then_loops || (n2_then_loops != n1))
     failed ("continue 2 with then");
}

test_break_n ();
test_cont_n  ();

loop (2)
{
   loop (2)
     continue 4;
}

$1 = 0;
loop (2)
{
   $1++;
}
if ($1 != 2)
  failed ("Use of continue N outside a function: val=%d", $1);

print ("Ok\n");

exit (0);

