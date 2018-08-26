% This is also a good test to perform leak checking on.
_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("recursive function modifications");

variable X = "";

variable V1 = "define crash () { eval(V2); X += \"V1\"; }";
variable V2 = "define crash () { eval(V1); X += \"V2\"; }";

define crash ();

define crash ()
{
   eval (V1);
   crash ();
   if (X != "V1")
     failed ("V1");

   if (1)
     {
	eval (V2);
	crash ();
	if (X != "V1V2")
	  failed ("V1V2");

	if (1)
	  eval (V1);
	crash ();
	if (X != "V1V2V1")
	  failed ("V1V2V1");
     }
   X += "V0";
}

crash ();
if (X != "V1V2V1V0") failed ("V1V2V1V0 : ", + X);

print ("Ok\n");
exit (0);

