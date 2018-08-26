_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("path");

static define test_path_concat (a, b, c)
{
   variable d = path_concat (a, b);
   if (c != d)
     failed ("path_concat(%s,%s) --> %s, not %s", a,b,d,c);
}

static define test_path (path, dir, base, ext, dirbase, sansextname)
{
   if (dir != path_dirname (path))
     failed ("path_dirname " + path);

   if (base != path_basename (path))
     failed ("path_basename " + path);

   if (ext != path_extname (path))
     failed ("path_extname " + path);

   test_path_concat (dir, base, dirbase);

   if (sansextname != path_sans_extname (path))
     failed ("path_sans_extname(\""+ path+ "\")");
}

#ifdef UNIX
test_path ("etc/rc.d", "etc", "rc.d", ".d", "etc/rc.d", "etc/rc");
test_path ("etc", ".", "etc", "", "./etc", "etc");
test_path ("usr/etc/", "usr/etc", "", "", "usr/etc/", "usr/etc/");
test_path ("/", "/", "", "", "/", "/");
test_path (".", ".", ".", ".", "./.", "");
test_path ("/a./b", "/a.", "b", "", "/a./b", "/a./b");
test_path (".c", ".", ".c", ".c", "./.c", "");
%"/tmp/jedtest4775.7430/dev/foo"

#elifdef VMS

test_path_concat ("drive:[dir.dir]", "a/b.c", "drive:[dir.dir.a]b.c");
test_path_concat ("drive:", "/a/b/c.d", "drive:[a.b]c.d");
test_path_concat ("drive:", "a/b/c.d", "drive:[a.b]c.d");
test_path_concat ("drive:", "/a.b", "drive:a.b");

test_path_concat ("[dir.dir]", "a/b.c", "[dir.dir.a]b.c");
test_path_concat ("", "/a/b/c.d", "[a.b]c.d");
test_path_concat ("", "a/b/c.d", "[a.b]c.d");
test_path_concat ("", "/a.b", "a.b");

#else
message ("**** NOT IMPLEMENTED ****");
#endif
print ("Ok\n");

exit (0);

