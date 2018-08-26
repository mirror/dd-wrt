_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("stdio routines");

define fopen_tmp_file (fileptr, mode)
{
   variable n;
   variable file, fp;
   variable fmt;

   @fileptr = NULL;

   fmt = "tmp-xxx.%03d";    % I need something that works on an 8+3 filesystem

   n = -1;
   while (n < 999)
     {
	n++;
	file = sprintf (fmt, n);
	if (NULL != stat_file (file))
	  continue;

	fp = fopen (file, mode);
	if (fp != NULL)
	  {
	     @fileptr = file;
	     return fp;
	  }
     }
   failed ("Unable to open a tmp file");
}

define run_tests (some_text, read_fun, write_fun, length_fun)
{
   variable file, fp;
   variable new_text, nbytes, len;
   variable pos;

   fp = fopen_tmp_file (&file, "wb");

   if (-1 == @write_fun (some_text, fp))
     failed (string (write_fun));

   if (-1 == fclose (fp))
     failed ("fclose");

   fp = fopen (file, "rb");
   if (fp == NULL) failed ("fopen existing");

   len = @length_fun (some_text);
   nbytes = @read_fun (&new_text, len, fp);

   if ((nbytes != len)
       or (some_text != new_text))
     failed (string (read_fun));

   if (-1 != @read_fun (&new_text, 1, fp))
     failed (string (read_fun) + " at EOF");

   if (0 == feof (fp)) failed ("feof");

   clearerr (fp);
   if (feof (fp)) failed ("clearerr");

   if (0 != fseek (fp, 0, SEEK_SET)) failed ("fseek");

   nbytes = @read_fun (&new_text, len, fp);

   if ((nbytes != len)
       or (some_text != new_text))
     failed (string (read_fun) + " after fseek");

   pos = ftell (fp);
   if (pos == -1) failed ("ftell at EOF");

   if (0 != fseek (fp, 0, SEEK_SET)) failed ("fseek");
   if (0 != ftell (fp)) failed ("ftell at BOF");
   if (0 != fseek (fp, pos, SEEK_CUR)) failed ("fseek to pos");

   if (pos != ftell (fp)) failed ("ftell after fseek to pos");

   if (feof (fp) != 0) failed ("feof after fseek to EOF");

   () = fseek (fp, 0, SEEK_SET);
   nbytes = fread (&new_text, Char_Type, 0, fp);
   if (nbytes != 0)
     failed ("fread for 0 bytes");

   nbytes = fread (&new_text, Char_Type, len + 100, fp);
   if (nbytes != len)
     failed ("fread for 100 extra bytes");

   if (-1 == fclose (fp)) failed ("fclose after tests");
   () = remove (file);
   if (stat_file (file) != NULL) failed ("remove");
}

static define do_fgets (addr, nbytes, fp)
{
   return fgets (addr, fp);
}

static define do_fread (addr, nbytes, fp)
{
   % return fread (addr, UChar_Type, nbytes, fp);
   return fread_bytes (addr, nbytes, fp);
}

run_tests ("ABCDEFG", &do_fgets, &fputs, &strlen);
run_tests ("A\000BC\000\n\n\n", &do_fread, &fwrite, &bstrlen);

define test_fread_fwrite (x)
{
   variable fp, file, str, n, m, y, type, ch;

   fp = fopen_tmp_file (&file, "w+b");

   type = _typeof(x);
   n = length (x);
   if ((type == String_Type) or (type == BString_Type))
     {
	%type = UChar_Type;
	n = bstrlen (x);
     }

   if (n != fwrite (x, fp))
     failed ("test_fread_fwrite: fwrite");

   if (-1 == fseek (fp, 0, SEEK_SET))
     failed ("test_fread_fwrite: fseek");

   if (n != fread (&y, type, n, fp))
     failed ("test_fread_fwrite: fread");

   if (length (where (y != x)))
     failed ("test_fread_fwrite: fread failed to return: " + string(x));

   if (-1 == fseek (fp, 0, SEEK_SET))
     failed ("test_fread_fwrite: fseek");

   if (type == UChar_Type)
     {
	y = 0;
	foreach (fp) using ("char")
	  {
	     ch = ();
	     if (ch != x[y])
	       failed ("foreach using char: %S != %S", ch, x[y]);
	     y++;
	  }
	if (y != n)
	  failed ("foreach using char 2");
     }

   () = fclose (fp);

   if (-1 == remove (file))
     failed ("remove:" + errno_string(errno));
   if (stat_file (file) != NULL) failed ("remove");
}

test_fread_fwrite ("");
test_fread_fwrite ("hello");
test_fread_fwrite ("hel\0\0lo");
test_fread_fwrite (Integer_Type[0]);
test_fread_fwrite ([1:10]);
#ifexists Double_Type
test_fread_fwrite (3.17);
test_fread_fwrite ([1:10:0.1]);
#endif
#ifexists Complex_Type
test_fread_fwrite (Complex_Type[50] + 3 + 2i);
test_fread_fwrite (2i+3);
test_fread_fwrite ([2i+3, 7i+1]);
#endif

static define test_fgetsputslines ()
{
   variable lines = array_map (String_Type, &string, [1:1000]);
   lines += "\n";
   variable file;
   variable fp = fopen_tmp_file (&file, "w");
   if (length (lines) != fputslines (lines, fp))
     failed ("fputslines");
   if (-1 == fclose (fp))
     failed ("fputslines;fclose");
   fp = fopen (file, "r");
   if (fp == NULL)
     failed ("fputslines...fopen");
   variable lines1 = fgetslines (fp);
   if (0 == _eqs (lines1, lines))
     failed ("fgetslines");
   ()=fclose (fp);
   if (-1 == remove (file))
     failed ("remove:" + errno_string(errno));
}
test_fgetsputslines ();

define test_read_write ()
{
   variable file;
   variable fp = fopen_tmp_file (&file, "w");
   variable fd = fileno (fp);

   variable str = "helloworld";
   variable n = write (fd, str);
   if (n != strlen (str))
     failed ("write(%s) returned %d", str, n);
   () = fclose (fp);
   fd = open (file, O_RDONLY);
   variable buf;
   n = read (fd, &buf, n);
   if (n != strlen (str))
     failed ("read returned %d bytes", n);
   if (buf != str)
     failed ("read returned %s not %s", buf, str);
   () = close (fd);
   () = remove (file);
}
test_read_write();

print ("Ok\n");

exit (0);
