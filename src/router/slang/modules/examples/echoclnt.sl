import ("socket");

private define perform_echo (s)
{
   if (-1 == write (s, "Hello"))
     () = fprintf (stderr, "client: write failed: %s\n", errno_string (errno));

   variable buf;
   if (-1 == read (s, &buf, 1024))
     () = fprintf (stderr, "client: read failed %s\n", errno_string (errno));
   else
     () = fprintf (stdout, "client: Read %d bytes: %s\n", bstrlen (buf), buf);

   if (-1 == close (s))
     () = fprintf (stderr, "client: close failed: %s\n", errno_string (errno));
}

private define unix_client ()
{
   variable s = socket (PF_UNIX, SOCK_STREAM, 0);
   connect (s, "/tmp/foo.sock");
   perform_echo (s);
}

private define inet_client ()
{
   variable s = socket (PF_INET, SOCK_STREAM, 0);
   variable buf;

   connect (s, "aluche.mit.edu", 31000);
   perform_echo (s);
}

define slsh_main ()
{
   %unix_client ();
   inet_client ();
}

