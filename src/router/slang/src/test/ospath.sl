_debug_info = 1; () = evalfile ("inc.sl");

testing_feature ("ospath");

static define test_path (path, dir, base, ext, dirbase)
{
   if (dir != path_dirname (path))
     failed ("path_dirname " + path);

   if (base != path_basename (path))
     failed ("path_basename " + path);

   if (ext != path_extname (path))
     failed ("path_extname " + path);

   if (dirbase != path_concat (dir, base))
     failed ("path_concat(%s,%s)", dir, base);
}

#ifdef UNIX
test_path ("etc/rc.d", "etc", "rc.d", ".d", "etc/rc.d");
test_path ("etc", ".", "etc", "", "./etc");
test_path ("usr/etc/", "usr/etc", "", "", "usr/etc/");
test_path ("/", "/", "", "", "/");
test_path (".", ".", ".", ".", "./.");
test_path ("/a./b", "/a.", "b", "", "/a./b");
test_path (".c", ".", ".c", ".c", "./.c");
#elifndef VMS
test_path ("etc\\rc.d", "etc", "rc.d", ".d", "etc\\rc.d");
test_path ("etc", ".", "etc", "", ".\\etc");
test_path ("usr\\etc\\", "usr\\etc", "", "", "usr\\etc\\");
test_path ("\\", "\\", "", "", "\\");
test_path (".", ".", ".", ".", ".\\.");
test_path ("\\a.\\b", "\\a.", "b", "", "\\a.\\b");
test_path (".c", ".", ".c", ".c", ".\\.c");
#else
message ("**** NOT IMPLEMENTED ****");
#endif
print ("Ok\n");

exit (0);

