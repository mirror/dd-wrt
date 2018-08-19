require ("./zlib.sl");

private variable Failed = 0;

private define silly_deflate (str)
{
   variable z = zlib_deflate_new ();
   variable x = "";
   foreach (str)
     {
	variable ch = ();
	x = x + _zlib_deflate (z.zobj, pack("C", ch), 0);
     }
   x = x + z.flush ();
   return x;
}

private define silly_inflate (zstr)
{
   variable z = zlib_inflate_new ();
   variable x = "";
   foreach (zstr)
     {
	variable ch = ();
	x = x + _zlib_inflate (z.zobj, pack("C", ch), 0);
     }
   x = x + z.flush ();
   return x;
}

define test_zlib (str0)
{
   variable zstr = zlib_deflate (str0);
   variable str1 = zlib_inflate (zstr);
   if (str1 != str0)
     {
	() = fprintf (stderr, "Failed to deflate/inflate %s\n", str0);
	Failed++;
	return;
     }
   variable zstr1 = silly_deflate (str0);
   if (zstr1 != zstr)
     {
	() = fprintf (stderr, "Failed to deflate %s via multiple calls", str0);
	Failed++;
	return;
     }
   str1 = silly_inflate (zstr1);
   if (str1 != str0)
     {
	() = fprintf (stderr, "Failed to inflate %s via multiple calls", str0);
	Failed++;
	return;
     }
}

define slsh_main ()
{
   test_zlib ("");
   test_zlib ("\0");
   test_zlib ("\0\0\0");
   test_zlib ("A");
   test_zlib ("AA");
   test_zlib ("AAA");
   test_zlib ("AAAAAAAA");
   test_zlib ("AAAAAAAABBBBBBB");
   test_zlib ("AAAAAAAABBBBBBBAAAAAAA");
   test_zlib ("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
   test_zlib ("ABCDEFGHIJKLMNOPQRSTUVWXYZ\0");
   test_zlib ("\0ABCDEFGHIJKLMNOPQRSTUVWXYZ\0");
   exit (Failed);
}
