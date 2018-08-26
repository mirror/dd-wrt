_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("POSIX I/O routines");

static define open_tmp_file (fileptr, flags, mode)
{
   variable n;
   variable file, fd;
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

	fd = open (file, flags, 0777);
	if (fd != NULL)
	  {
	     @fileptr = file;
	     return fd;
	  }
     }
   failed ("Unable to open a tmp file");
}

define run_tests (some_text)
{
   variable file, fd, fp1, fp2;
   variable new_text, nbytes, len;
   variable pos;

   fd = open_tmp_file (&file, O_WRONLY|O_BINARY|O_CREAT, 0777);

   if (-1 == write (fd, some_text))
     failed ("write");

   loop (5)
     {
	fp1 = fdopen (fd, "wb");
	fp2 = fdopen (fd, "wb");
	if ((fp1 == NULL) || (fp2 == NULL))
	  failed ("fdopen");

	if (isatty (fileno (fp1)))
	  failed ("isatty (fileno)");
     }

   if (-1 == close (fd))
     failed ("close");

   fd = open (file, O_RDONLY|O_BINARY);
   if (fd == NULL) failed ("fopen existing");

   len = bstrlen (some_text);
   nbytes = read (fd, &new_text, len);
   if (nbytes == -1)
     failed ("read");

   if ((nbytes != len)
       or (some_text != new_text))
     failed ("read");

   if (0 != read (fd, &new_text, 1))
     failed ("read at EOF");
   if (bstrlen (new_text))
     failed ("read at EOF");

   if (-1 == close (fd)) failed ("close after tests");
   variable st = stat_file (file);
   () = st.st_mode;  %  see if stat_file returned the right struct
   () = remove (file);
   if (stat_file (file) != NULL) failed ("remove");
}

run_tests ("ABCDEFG");
run_tests ("A\000BC\000\n\n\n");

variable fd = open ("/dev/tty", O_RDONLY);
if (fd != NULL)
{
   if (0 == isatty (fd))
     failed ("isatty");
}
fd = 0;

if (fileno (stdin) != fileno(stdin))
{
   failed ("fileno(stdin) not equal to itself");
}

if (fileno (stdin) == fileno(stdout))
{
   failed ("fileno(stdin) is equal to fileno(stdout)");
}

print ("Ok\n");
exit (0);
