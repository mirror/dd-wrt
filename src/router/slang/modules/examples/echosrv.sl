import ("socket");

private define perform_echo (s1)
{
   variable buf;
   if (-1 == read (s1, &buf, 1024))
     () = fprintf (stderr, "server: read failed: %s", errno_string (errno));
   else
     () = fprintf (stdout, "server read %d bytes: %s\n", bstrlen (buf), buf);

   if (-1 == write (s1, buf))
     () = fprintf (stderr, "server: write failed: %s", errno_string(errno));
}

private define unix_server ()
{
   variable s = socket (PF_UNIX, SOCK_STREAM, 0);
   bind (s, "/tmp/foo.sock");
   listen (s, 1);
   variable s1 = accept (s);
   perform_echo (s1);
}

private define inet_server ()
{
   variable s = socket (PF_INET, SOCK_STREAM, 0);
   bind (s, "aluche", 31000);
   listen (s, 1);
   variable host, port;
   variable s1 = accept (s, &host, &port);
   vmessage ("accepted connection from %s:%d", host, port);
   perform_echo (s1);
}

define slsh_main ()
{
   %unix_server ();
   inet_server ();
}
