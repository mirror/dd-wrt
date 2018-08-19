_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("Binary Strings");

define test ()
{
   variable a = "\000A\000B\000C\000D";

   if (typeof (a) != BString_Type) failed ("typeof");

   if (bstrlen (a) != 8) failed ("bstrlen");
   
   if ((a[[0:7:2]] != "\000\000\000\000")
       or (a[[1:7:2]] != "ABCD")) failed ("array indexing");

   if (strlen (a) != 0) failed ("typecast");

   a += "XYZ";
   if (a[[8:]] != "XYZ") failed ("+= op");

   a = "XYZ" + a;
   if (a == "XYZ") failed ("== op");   
   if (strcmp (a, "XYZ")) failed ("failed strcmp");

   a = "XYZ"B;
   if (typeof (a) != BString_Type)
     failed ("B suffix on a binary string");

   loop (500)
     {
	variable aa = a + ""B;
	variable b = __tmp(a) + "X";
	if (b != aa + "X")
	  failed ("__tmp op");
	a = __tmp(b);
     }

   a = BString_Type[10]; a[*] = "XYZ"B;
   b = "XYZ";
   loop (100)
     {
	a += "XYZ"B;
	b += "XYZ";
	if (any (a != b))
	  failed ("Adding array of bstrings");
     }
   
   loop (1000)
     {
	a = "\000A\000B\000C\000D";
	a = "A\000B\000C\000";
     }
}
test ();

print ("Ok\n");
exit (0);
