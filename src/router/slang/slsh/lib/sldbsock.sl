% This file implements a socket-based wrapper around sldbcore for
% debugging a separate process.  Programs that wish to use this for
% debugging should something like
%
%    autoload ("sldb", "sldbcore");
%
% in a startup file.  Then instead of using `require` or `evalfile` to
% load a file, use sldb("name-of-file") to load the file to be
% debugged.  Then in another window, run
%
%    sldb --pid <pid-of-process-to-be-debugged>
%
% Do not byte-compile this file.  It contains both the client and
% server code.
require ("sldbcore");
require ("socket");

% Simple line-based protocol:

private variable SLDB_SOCKET_PREFIX	= "/tmp/.sldb_";

private variable OK_CMD_RECEIVED	= 200;
private variable OK_CONT_XFER		= 201;
private variable OK_XFER_RECEIVED	= 202;
private variable ERR_UNKNOWN_CMD	= 301;

private variable Debug_Mode = 0;

private define debug_output ()
{
   variable args = __pop_args (_NARGS);
   if (Debug_Mode)
     {
	variable fp = stderr;
	() = fprintf (fp, __push_args (args));
	() = fflush (fp);
     }
}

private define make_socket_name (pid)
{
   return "$SLDB_SOCKET_PREFIX$pid"$;
}

private define do_write (fp, data)
{
   variable nbytes = fwrite (data, fp);
   if ((bstrlen (data) != nbytes)
       or (-1 == fflush (fp)))
     throw WriteError;

   debug_output ("** WROTE: %s\n", data);
}

private define do_fgets (fp)
{
   variable buf;
   if (-1 == fgets (&buf, fp))
     throw ReadError;

   debug_output ("** READ: %s\n", buf);
   return buf;
}

private define receive_response (fp)
{
   variable line = do_fgets (fp);
   return atoi (line);
}

private define send_response (fp, rsp, txt)
{
   rsp = sprintf ("%03d %s\n", rsp, txt);
   debug_output ("Sending Response:<%s>\n", rsp);
   do_write (fp, rsp);
}

private define send_cmd (fp, cmd)
{
   debug_output ("Sending CMD:<%s>\n", cmd);
   do_write (fp, cmd);
   return receive_response (fp);
}

private define receive_data (fp)
{
   variable data = "";
   forever
     {
	variable line = do_fgets (fp);
	if (line[0] == '.')
	  {
	     if (line == ".\n")
	       break;

	     line = substr (line, 2, -1);
	  }
	data = strcat (data, line);
     }
   debug_output ("** Received:<%s>\n", data);

   send_response (fp, OK_XFER_RECEIVED, "Ok transfer received");
   return data;
}

private define send_data (fp, data)
{
   debug_output ("** Sending:<%s>\n", data);

   variable lines = strchop (data, '\n', 0);
   variable num = length (lines);
   if (data[-1] == '\n')
     num--;

   _for (0, num-1, 1)
     {
	variable i = ();
	variable line = lines[i];
	if (line[0] == '.')
	  line = strcat (".", line, "\n");
	else
	  line = strcat (line, "\n");

	do_write (fp, line);
     }
   do_write (fp, ".\n");
   return receive_response (fp);
}

%

#ifnexists __SLDB_CLIENT__
% Server Routines

private variable Server_Socket_fd = NULL;
private variable Server_Socket_fp = NULL;

private define vmessage_method ()
{
   variable args = __pop_args (_NARGS);
   variable buf = sprintf (__push_args (args));
   debug_output ("Entering vmessage_method to send <%s>\r\n", buf);
   variable fp = Server_Socket_fp;

   variable status = send_cmd (fp, "MESSAGE\n");
   if (status != OK_CONT_XFER)
     throw IOError, "Client returned $status to MESSAGE command";

   status = send_data (fp, buf);
   if (status != OK_XFER_RECEIVED)
     throw IOError, "Expected OK_XFER_RECEIVED, got $status"$;

   debug_output ("Leaving vmessage_method\r\n");
}

private define open_file_at_linenum (file, linenum)
{
   if (path_extname (file) == ".slc")
     file = path_sans_extname (file) + ".sl";

   variable fp = fopen (file, "r");
   if (fp == NULL)
     {
	vmessage_method ("Unable to open %s\n", file);
	return NULL;
     }
   if (linenum == 1)
     return fp;

   foreach (fp) using ("line")
     {
	variable line = ();
	linenum--;
	if (linenum == 1)
	  break;
     }
   return fp;
}

private define list_method (file, linemin, linemax)
{
   variable n = linemax - linemin + 1;
   foreach (open_file_at_linenum (file, linemin))
     {
	variable line = ();
	vmessage_method ("%d\t%s", linemin, line);
	linemin++;
	if (linemin > linemax)
	  break;
     }
}

private define read_input_method (prompt, default_cmd)
{
   variable fp = Server_Socket_fp;

   do
     {
	variable status = send_cmd (fp, "INPUT\n");
	if (status != OK_CONT_XFER)
	  throw IOError, "Expected OK_CONT_XFER, got $status"$;

	status = send_data (fp, prompt);

	if (status != OK_XFER_RECEIVED)
	  throw IOError, "Expected OK_XFER_RECEIVED, got $status"$;

	variable input = receive_data (fp);
	if (input == "\n")
	  input = default_cmd;
     }
   while ((input == "") or (input == NULL));
   return input;
}

private define quit_method ()
{
   variable fp = Server_Socket_fp;
   () = send_cmd (fp, "QUIT\n");
   exit (0);
}

private define exit_method ()
{
   variable fp = Server_Socket_fp;
   () = send_cmd (fp, "EXIT\n");
}

private define initialize_server ()
{
   if (Server_Socket_fd != NULL)
     return;

   variable s = socket (PF_UNIX, SOCK_STREAM, 0);
   variable name = make_socket_name (getpid ());
   bind (s, name);
   () = chmod (name, S_IRUSR|S_IWUSR);
   listen (s, 1);
   Server_Socket_fd = accept (s);
   Server_Socket_fp = fdopen (Server_Socket_fd, "r+");
   if (Server_Socket_fd == NULL)
     throw OpenError, errno_string (errno);

   vmessage_method ("Welcome to sldb");
}

define sldb_initialize ()
{
   initialize_server ();
   variable m = sldb_methods ();
   m.list = &list_method;
   m.vmessage = &vmessage_method;
   m.read_input = &read_input_method;
   m.quit = &quit_method;
   m.pprint = NULL;
   m.exit = &exit_method;
}
#endif				       % ! __SLDB_CLIENT__

#ifexists __SLDB_CLIENT__
% Client Routines

private define read_command (s)
{
   variable line = strtrim (do_fgets (s));
   return line;
}

private define handle_message_cmd (s)
{
   send_response (s, OK_CONT_XFER, "Ok continue transfer");
   variable msg = receive_data (s);

   () = fputs (msg, stdout);
   () = fflush (stdout);
}

private define handle_input_cmd (s)
{
   send_response (s, OK_CONT_XFER, "Ok continue transfer");
   variable prompt = receive_data (s);

   prompt = strtrim (prompt, "\n");
   forever
     {
	try
	  {
	     variable line = slsh_readline (prompt);
	  }
	catch UserBreakError: continue;
	if (line != NULL)
	  break;
     }
   send_data (s, line);
}

private define handle_unknown_cmd (s)
{
   send_response (s, ERR_UNKNOWN_CMD, "Error Unknown Command");
}

define sldbsock_attach (pid)
{
   variable name = make_socket_name (pid);
   variable s = socket (PF_UNIX, SOCK_STREAM, 0);
   variable e;
   try (e)
     {
	connect (s, name);
     }
   catch SocketError:
     {
	() = fprintf (stderr, "Unable to connect to socket %s\n", name);
	() = fprintf (stderr, "Is the remote process ready to be debugged?\n");
	return -1;
     }

   variable fp = fdopen (s, "r+");
   forever
     {
	variable cmd = read_command (fp);
	if (cmd == "MESSAGE")
	  {
	     handle_message_cmd (fp);
	     continue;
	  }
	if (cmd == "INPUT")
	  {
	     handle_input_cmd (fp);
	     continue;
	  }
	if (cmd == "QUIT")
	  {
	     send_response (fp, OK_CMD_RECEIVED, "Ok Cmd Received");
	     return 0;
	  }
	if (cmd == "EXIT")
	  {
	     send_response (fp, OK_CMD_RECEIVED, "Ok Cmd Received");
	     return 0;
	  }
	handle_unknown_cmd (fp, cmd);
     }
}

#endif

provide ("sldbsock");
