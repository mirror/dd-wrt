private variable Script_Version_String = "0.1.1";

require ("cmdopt");
require ("glob");

private define convert_path (path)
{
   return strtrans (path, "/", "\\");
}

private define mkdir_p (dir);
private define mkdir_p (dir)
{
   dir = convert_path (dir);

   if ((-1 == mkdir (dir)) && (errno != EEXIST))
     {
	variable parent = path_dirname (dir);
	if ((-1 == mkdir_p (parent))
	    || (-1 == mkdir (dir)))
	  {
	     () = fprintf (stderr, "Failed to create %s: %s\n",
			   dir, errno_string ());
	     exit (1);
	  }
     }
   return 0;
}

private define run_cmd (cmd)
{
   () = system (cmd);
}

private define install_file (file, dir)
{
   () = fprintf (stdout, "Installing %s in %s\n", file, dir);
   dir = convert_path (dir);
   file = convert_path (file);

   run_cmd ("copy $file $dir"$);
}

private define install_files (pat, dir)
{
   foreach (glob (pat))
     {
	variable file = ();
	install_file (file, dir);
     }
}

private define install_libslang (root, objdir)
{
   variable libdir = "$root/lib"$;
   variable bindir = "$root/bin"$;
   variable incdir = "$root/include"$;

   () = mkdir_p (libdir);
   () = mkdir_p (incdir);
   () = mkdir_p (bindir);

   install_file ("src/slang.h", incdir);
   install_file ("src/$objdir/libslang.a"$, libdir);
   install_file ("src/$objdir/libslang.dll"$, bindir);
}

private define install_slang_doc (root)
{
   variable dir = "$root/share/doc/slang/v2"$;
   () = mkdir_p (dir);
   install_file ("changes.txt", dir);
   install_file ("COPYING", dir);
   install_file ("doc/slangdoc.html", dir);
   install_file ("doc/text/*.txt", dir);
}

private define install_slsh (prefix, confdir, objdir)
{
   variable dir = "$prefix/bin"$;
   () = mkdir_p (dir);
   install_file ("slsh/$objdir/slsh.exe"$, dir);

   dir = "$prefix/share/slsh"$;
   () = mkdir_p (dir);
   install_files ("slsh/lib/*.sl", dir);

   dir = "$prefix/share/slsh/help"$;
   () = mkdir_p (dir);
   install_files ("slsh/lib/help/*.hlp", dir);

   dir = "$prefix/share/slsh/rline"$;
   () = mkdir_p (dir);
   install_files ("slsh/lib/rline/*", dir);

   dir = "$prefix/share/slsh/scripts"$;
   () = mkdir_p (dir);
   install_files ("slsh/scripts/*", dir);

   dir = "$prefix/share/slsh/local-packages"$;
   () = mkdir_p (dir);
   dir = "$prefix/share/slsh/local-packages/help"$;
   () = mkdir_p (dir);

   dir = "$confdir"$;
   () = mkdir_p (dir);
   install_file ("slsh/etc/slsh.rc", dir);
}

private define install_modules (prefix)
{
   variable dir = "$prefix/lib/slang/v2/modules"$;
   () = mkdir_p (dir);
   install_files ("modules/*.dll", dir);

   dir = "$prefix/share/slsh"$;
   () = mkdir_p (dir);
   install_files ("modules/*.sl", dir);

   dir = "$prefix/share/slsh/cmaps"$;
   () = mkdir_p (dir);
   install_files ("modules/cmaps/*", dir);
}

private define exit_version ()
{
   () = fprintf (stdout, "Version: %S\n", Script_Version_String);
   exit (0);
}

private define exit_usage ()
{
   variable fp = stderr;
   () = fprintf (fp, "Usage: %s [options] install\n", __argv[0]);
   variable opts =
     [
      "Options:\n",
      " -v|--version               Print version\n",
      " -h|--help                  This message\n",
      " --prefix=/install/prefix   Default is /usr\n",
      " --distdir=/path            Default is blank\n",
     ];
   foreach (opts)
     {
	variable opt = ();
	() = fputs (opt, fp);
     }
   exit (1);
}

define slsh_main ()
{
   variable c = cmdopt_new ();
   variable destdir = "";
   variable prefix = "/usr";

   c.add("h|help", &exit_usage);
   c.add("v|version", &exit_version);
   c.add("destdir", &destdir; type="str");
   c.add("prefix", &prefix; type="str");
   variable i = c.process (__argv, 1);

   if ((i + 1 != __argc) || (__argv[i] != "install"))
     exit_usage ();

   % See the comment in makefile.m32 for the motivation behind the
   % sillyness involving X-.
   if (0==strncmp (destdir, "X-", 2))
     destdir = substr (destdir, 3, -1);

   () = fprintf (stdout, "Using destdir=%s, prefix=%s\n", destdir, prefix);

   variable root = strcat (destdir, prefix);

   variable confdir = "/etc";
   if (prefix != "/usr")
     confdir = path_concat (prefix, "etc");
   confdir = strcat (destdir, confdir);

   variable objdir = "gw32objs";
   install_libslang (root, objdir);
   install_slang_doc (root);
   install_slsh (root, confdir, objdir);
   install_modules (root);
}

