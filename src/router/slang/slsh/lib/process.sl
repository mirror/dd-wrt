require ("fork");
require ("fcntl");

private variable OPEN_MAX = 512;
try
{
   require ("sysconf");
   OPEN_MAX = (@__get_reference ("sysconf"))("_SC_OPEN_MAX", 512);
}
catch ImportError;

#ifexists signal
signal (SIGPIPE, SIG_IGN);
#endif

private define parse_redir (redir)
{
   variable redir_info =
     [{"^>> ?\(.*\)"R, O_WRONLY|O_CREAT|O_APPEND},
      {"^> ?\(.*\)"R, O_WRONLY|O_TRUNC|O_CREAT},
      {"^<> ?\(.*\)"R, O_RDWR|O_CREAT},
      {"^< ?\(.*\)"R, O_RDONLY}
     ];
   variable other_flags = O_NOCTTY;

   foreach (redir_info)
     {
	variable ri = ();
	variable re = ri[0];
	ifnot (string_match (redir, re, 1))
	  continue;
	variable pos, len;
	(pos, len) = string_match_nth (1);
	return ri[1] | other_flags, redir[[pos:pos+len-1]];
     }
   return 0, redir;
}

private variable S_RWUGO = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;

% Look for structure fields of the form fpN and open the corresponding
% files.
private define open_redirect_files (q)
{
   variable redir_fds = FD_Type[0], redir_ifds = Int_Type[0];
   if (q == NULL)
     return redir_fds, redir_ifds;

   foreach (get_struct_field_names (q))
     {
	variable name = ();
	variable fd, ifd, value, defflags = 0, flags, file;
	if (1 != sscanf (name, "fd%d", &ifd))
	  {
	     if (name == "stdin") ifd = 0;
	     else if (name == "stdout") ifd = 1;
	     else if (name == "stderr") ifd = 2;
	     else continue;
	  }
	if (ifd == 0) defflags = O_RDONLY|O_NOCTTY;
	if ((ifd == 1) || (ifd == 2)) defflags = O_WRONLY|O_TRUNC|O_CREAT|O_NOCTTY;

	value = get_struct_field (q, name);
	if (typeof(value) == String_Type)
	  {
	     (flags, file) = parse_redir (value);
	     if (file == "")
	       throw InvalidParmError, "Invalid redirection: $value";

	     if (flags == 0) flags = defflags;

	     if (flags & O_CREAT)
	       fd = open (file, flags, S_RWUGO);
	     else
	       fd = open (file, flags);

	     if (fd == NULL)
	       throw OpenError, sprintf ("%s: %s", file, errno_string ());
	  }
	else if (typeof(value) == FD_Type)
	  {
	     fd = value;
	  }
	else if (typeof(value) == File_Type)
	  {
	     fd = fileno (value);
	  }
	else
	  {
	     fd = @FD_Type(value);
	  }

	if (fd == NULL)
	  throw OSError, "fd$ifd: "$ + errno_string();

	redir_fds = [redir_fds, fd];
	redir_ifds = [redir_ifds, ifd];
     }

   return (redir_fds, redir_ifds);
}

% parse dipN=M qualifiers
private define parse_dup_qualifiers (q)
{
   variable open_fds = FD_Type[0], wanted_ifds = Int_Type[0];

   if (q == NULL)
     return open_fds, wanted_ifds;

   foreach (get_struct_field_names (q))
     {
	variable name = ();
	variable fd, ifd, value;
	if ((1 != sscanf (name, "dup%d", &ifd))
	    || (name != sprintf ("dup%d", ifd)))
	  continue;

	value = get_struct_field (q, name);
	if (typeof (value) == File_Type)
	  fd = fileno (value);
	else if (typeof (value) == FD_Type)
	  fd = value;
	else
	  fd = @FD_Type(value);
	if (fd == NULL)
	  throw OSError, "fd$ifd: "$ + errno_string();

	open_fds = [open_fds, fd];
	wanted_ifds = [wanted_ifds, ifd];
     }
   return open_fds, wanted_ifds;
}

% Here, open_fds is an array of all (known) open FD_Type objects, and open_ifds
% is the corresponding array of integer descriptors.  Starting at the
% index idx_offset, dup2 the FD_Type objects onto the array of
% wanted_ifds.  If a wanted_ifd is associated with an open descriptor,
% then that will be duped to a new integer descriptor.
private define dup2_open_fds (wanted_ifds, open_ifds, open_fds, idx_offset)
{
   variable i, ifd, fd;
   _for i (0, length(wanted_ifds)-1, 1)
     {
	ifd = wanted_ifds[i];

	i += idx_offset;
	variable j = wherefirst (open_ifds == ifd);
	if (j != NULL)
	  {
	     if (j == i)
	       continue;
	     fd = dup_fd (open_fds[j]);
	     if (fd == NULL)
	       throw OSError, "dup_fd failed: " + errno_string ();
	     open_ifds[j] = _fileno(fd);
	     open_fds[j] = fd;
	  }

	if (-1 == dup2_fd (open_fds[i], ifd))
	  throw OSError, "dup2_fd failed: " + errno_string ();

	open_fds[i] = @FD_Type(ifd);
	open_ifds[i] = ifd;
     }
}

private define exec_child (argv, child_fds, required_child_ifds)
{
   variable i, j, fd, ifd;

   % The child pipe ends will need to be dup2'd to the corresponding
   % integers.  Care must be exercised to not stomp on pipe descriptors
   % that have the same values.
   % Note: The first on in the list is the traceback fd
   variable child_open_ifds = array_map (Int_Type, &_fileno, child_fds);
   dup2_open_fds (required_child_ifds, child_open_ifds, child_fds, 1);

   if (__qualifiers != NULL)
     {
	% Handle the fdN=foo qualifiers, e.g., fd0="file", fd1=3
	variable redir_fds, wanted_redir_ifds;
	(redir_fds, wanted_redir_ifds) = open_redirect_files (__qualifiers);

	variable ofs = length (child_open_ifds);
	child_fds = [child_fds, redir_fds];
	child_open_ifds = [child_open_ifds,
			   array_map (Int_Type, &_fileno, redir_fds)];
	redir_fds = NULL;	       % decrement ref-counts
	dup2_open_fds (wanted_redir_ifds, child_open_ifds, child_fds, ofs);

	% Now handle the dupN=M qualifiers.  Here, M must already be
	% open in the child, and N will be duped from it.  Note: M
	% could be inherited from the parent, and as such may not be
	% in the child_open_ifds list.
	variable fdMs, ifdMs, ifdNs;
	(fdMs, ifdNs) = parse_dup_qualifiers (__qualifiers);
	variable num_aliased = length (ifdNs);
	if (num_aliased)
	  {
	     ifdMs = array_map (Int_Type, &_fileno, fdMs);
	     % Note the padding.  This is because there are not yet open
	     % descriptors that correspond to the ifdNs
	     child_fds = [child_fds, fdMs, fdMs];
	     child_open_ifds = [child_open_ifds, ifdMs, Int_Type[num_aliased]-1];
	     dup2_open_fds (ifdNs, child_open_ifds, child_fds, length(child_fds)-num_aliased);
	  }
     }

   variable hook = qualifier ("pre_exec_hook");
   if (hook != NULL)
     {
	% Call the hook.  Pass it the list of open descriptors.  All others
	% will be closed.
	variable list = {};
	foreach ifd (child_open_ifds) list_append (list, ifd);
	(@hook)(list);
	child_open_ifds = list_to_array (list);
     }

   variable close_mask = Char_Type[OPEN_MAX+1];
   close_mask [[3:]] = 1;
   foreach ifd (child_open_ifds) close_mask[ifd] = 0;
   _for ifd (0, length(close_mask)-1, 1)
     {
	if (close_mask[ifd]) () = _close (ifd);
     }

   () = execvp (argv[0], argv);
   throw OSError, "exec failed: " + errno_string ();
}

private define wait_method ()
{
   variable options = 0, s;

   if (_NARGS == 2)
     options = ();
   s = ();

   if (s.pid == -1)
     return NULL;

   return waitpid (s.pid, options);
}

define new_process ()
{
   if (_NARGS != 1)
     {
	usage ("obj = new_process([pgm, args...] [;qualifiers])");
     }

   variable argv = ();
   if (typeof (argv) == List_Type)
     argv = list_to_array (argv);
   if (typeof (argv) != Array_Type)
     argv = [argv];

   variable read_ifds = qualifier("read", Int_Type[0]);
   variable write_ifds = qualifier("write", Int_Type[0]);

   if (typeof (read_ifds) == List_Type)
     read_ifds = list_to_array (read_ifds);
   if (typeof (write_ifds) == List_Type)
     write_ifds = list_to_array (write_ifds);

   variable numfds = length(read_ifds) + length(write_ifds);
   variable parent_fds = FD_Type[numfds+1];   %  +1 for traceback fd
   variable child_fds = FD_Type[numfds+1];
   variable modes = String_Type[numfds+1];

   variable ifd, r, w;

   % The read and write fds become pipes to the child and are returned
   % as structure fields.
   variable i = 0;

   % The 0th one is used to commmunicate error messages
   (parent_fds[i], child_fds[i]) = pipe (); i++;
   variable fd = child_fds[0];
   () = fcntl_setfd (fd, fcntl_getfd (fd) | FD_CLOEXEC);
   fd = NULL;			       %  remove reference to it.

   variable child_ifds = [read_ifds, write_ifds];
   variable struct_fields = {};
   foreach ifd (read_ifds)
     {
	list_append (struct_fields,"fd$ifd"$);
	list_append (struct_fields,"fp$ifd"$);
	modes[i] = "w";
	(child_fds[i], parent_fds[i]) = pipe (); i++;
     }

   foreach ifd (write_ifds)
     {
	list_append (struct_fields,"fd$ifd"$);
	list_append (struct_fields,"fp$ifd"$);
	modes[i] = "r";
	(parent_fds[i], child_fds[i]) = pipe (); i++;
     }

   variable pid = fork ();
   if (pid == 0)
     {
	variable e;
	try (e)
	  {
	     variable dir = qualifier ("dir");
	     if (dir != NULL)
	       {
		  if (-1 == chdir (dir))
		    throw OSError, "chdir: " + errno_string ();
	       }

	     % We do not need the parent descriptors, so close them.
	     parent_fds = NULL;
	     exec_child (argv, child_fds, child_ifds;; __qualifiers);
	  }
	catch AnyError:
	  {
	     fd = child_fds[0];
	     () = write (fd, sprintf ("%S:%S:%S\n", e.file, e.line, e.message));
	     () = write (fd, sprintf ("Traceback:\n%S\n", e.traceback));
	     fd = NULL;
	  }
	_exit (1);
     }
   variable other_struct_fields = ["pid", "wait"];
   child_fds = NULL;

   if (length (struct_fields) == 0)
     struct_fields = String_Type[0];
   else
     struct_fields = list_to_array (struct_fields);
   variable s = @Struct_Type([struct_fields, other_struct_fields]);

   _for i (0, length (child_ifds)-1, 1)
     {
	ifd = child_ifds[i];
	fd = parent_fds[i+1];	       %  parent_fds[0] used for errors
	set_struct_field (s, "fd$ifd"$, fd);
	variable fp = fdopen (fd, modes[i+1]);
	if (fp == NULL)
	  throw OpenError, "fdopen failed on child descriptor $ifd"$;
	set_struct_field (s, "fp$ifd"$, fp);
     }
   s.pid = pid;
   s.wait = &wait_method;
   variable errmsg = "";
   variable derrmsg;
   while (read (parent_fds[0], &derrmsg, 512) > 0)
     {
	errmsg += derrmsg;
     }
   if (errmsg != "")
     throw OSError, errmsg;

   return s;
}

