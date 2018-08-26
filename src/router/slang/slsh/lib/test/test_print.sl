require ("print");

define slsh_main ()
{
   variable x = [1:20:0.1];
   variable ref_x, file_x, fp_x;
   print (x, &ref_x);

   variable file = sprintf ("/tmp/test_print_%X_%d", _time(), getpid());
   print (x, file);
   variable fp = fopen (file, "r");
   () = fread_bytes (&file_x, 2*strlen (ref_x), fp);
   () = fclose (fp);

   fp = fopen (file, "wb");
   print (x, fp);
   () = fclose (fp);
   fp = fopen (file, "r");
   () = fread_bytes (&fp_x, 2*strlen (ref_x), fp);
   () = fclose (fp);

   () = remove (file);
   if ((ref_x != file_x) || (ref_x != fp_x))
     {
	() = fprintf (stderr, "Failed: print failed to produce identical results\n");
	exit (1);
     }
}

