\function{access}
\synopsis{Check to see if a file is accessible}
\usage{Int_Type access (String_Type pathname, Int_Type mode)}
\description
 This functions checks to see if the current process has access to the
 specified pathname.  The \exmp{mode} parameter determines the type of
 desired access.  Its value is given by the bitwise-or of one or more
 of the following constants:
#v+
    R_OK   Check for read permission
    W_OK   Check for write permission
    X_OK   Check for execute permission
    F_OK   Check for existence
#v-

 The function will return 0 if process has the requested access
 permissions to the file, otherwise it will return -1 and set
 \ivar{errno} accordingly.

 Access to a file depend not only upon the file itself, but also upon
 the permissions of each of the directories in the pathname.  The
 checks are done using the real user and group ids of the process, and
 not using the effective ids.
\seealso{stat_file}
\done

\function{chdir}
\synopsis{Change the current working directory}
\usage{Int_Type chdir (String_Type dir)}
\description
  The \ifun{chdir} function may be used to change the current working
  directory to the directory specified by \exmp{dir}.  Upon success it
  returns zero.  Upon failure it returns \exmp{-1} and sets
  \ivar{errno} accordingly.
\seealso{mkdir, stat_file}
\done

\function{chmod}
\synopsis{Change the mode of a file}
\usage{Int_Type chmod (String_Type file, Int_Type mode)}
\description
  The \ifun{chmod} function changes the permissions of the specified
  file to those given by \exmp{mode}.  It returns \exmp{0} upon
  success, or \exmp{-1} upon failure setting \ivar{errno} accordingly.

  See the system specific documentation for the C library
  function \ifun{chmod} for a discussion of the \exmp{mode} parameter.
\seealso{chown, stat_file}
\done

\function{chown}
\synopsis{Change the owner of a file}
\usage{Int_Type chown (String_Type file, Int_Type uid, Int_Type gid)}
\description
  The \ifun{chown} function is used to change the user-id and group-id of
  \exmp{file} to \exmp{uid} and \exmp{gid}, respectively.  It returns
  0 upon success and -1 upon failure, with \ivar{errno}
  set accordingly.
\notes
  On most systems, only the superuser can change the ownership of a
  file.

  Some systems do not support this function.
\seealso{chmod, stat_file}
\done

\function{getcwd}
\synopsis{Get the current working directory}
\usage{String_Type getcwd ()}
\description
  The \ifun{getcwd} function returns the absolute pathname of the
  current working directory.  If an error occurs or it cannot
  determine the working directory, it returns \NULL and sets
  \ivar{errno} accordingly.
\notes
  Under Unix, OS/2, and MSDOS, the pathname returned by this function
  includes the trailing slash character.  It may also include
  the drive specifier for systems where that is meaningful.
\seealso{mkdir, chdir, errno}
\done

\function{hardlink}
\synopsis{Create a hard-link}
\usage{Int_Type hardlink (String_Type oldpath, String_Type newpath)}
\description
  The \ifun{hardlink} function creates a hard-link called
  \exmp{newpath} to the existing file \exmp{oldpath}.  If the link was
  successfully created, the function will return 0.  Upon error, the
  function returns -1 and sets \ivar{errno} accordingly.
\notes
  Not all systems support the concept of a hard-link.
\seealso{symlink}
\done

\function{listdir}
\synopsis{Get a list of the files in a directory}
\usage{String_Type[] listdir (String_Type dir)}
\description
  The \ifun{listdir} function returns the directory listing of all the
  files in the specified directory \exmp{dir} as an array of strings.
  It does not return the special files \exmp{".."} and \exmp{"."} as
  part of the list.
\seealso{stat_file, stat_is, length}
\done

\function{lstat_file}
\synopsis{Get information about a symbolic link}
\usage{Struct_Type lstat_file (String_Type file)}
\description
  The \ifun{lstat_file} function behaves identically to \ifun{stat_file}
  but if \exmp{file} is a symbolic link, \ifun{lstat_file} returns
  information about the link itself, and not the file that it
  references.

  See the documentation for \ifun{stat_file} for more information.
\notes
  On systems that do not support symbolic links, there is no
  difference between this function and the \ifun{stat_file} function.
\seealso{stat_file, stat_is, stat_mode_to_string, readlink}
\done

\function{mkdir}
\synopsis{Create a new directory}
\usage{Int_Type mkdir (String_Type dir [,Int_Type mode])}
\description
  The \ifun{mkdir} function creates a directory whose name is specified
  by the \exmp{dir} parameter with permissions given by the optional
  \exmp{mode} parameter.  Upon success \ifun{mkdir} returns 0, or it
  returns \exmp{-1} upon failure setting \ivar{errno} accordingly.  In
  particular, if the directory already exists, the function will fail
  and set errno to \icon{EEXIST}.
\example
  The following function creates a new directory, if it does not
  already exist (indicated by \exmp{errno==EEXIST}).
#v+
     define my_mkdir (dir)
     {
        if (0 == mkdir (dir)) return;
        if (errno == EEXIST) return;
        throw IOError,
           sprintf ("mkdir %s failed: %s", dir, errno_string (errno));
     }
#v-
\notes
  The \exmp{mode} parameter may not be meaningful on all systems.  On
  systems where it is meaningful, the actual permissions on the newly
  created directory are modified by the process's umask.
\seealso{rmdir, getcwd, chdir, fopen, errno}
\done

\function{readlink}
\synopsis{String_Type readlink (String_Type path)}
\usage{Get the value of a symbolic link}
\description
  The \ifun{readlink} function returns the value of a symbolic link.
  Upon failure, \NULL is returned and \ivar{errno} set accordingly.
\notes
  Not all systems support this function.
\seealso{symlink, lstat_file, stat_file, stat_is}
\done

\function{remove}
\synopsis{Delete a file}
\usage{Int_Type remove (String_Type file)}
\description
  The \ifun{remove} function deletes a file.  It returns 0 upon
  success, or -1 upon error and sets \ivar{errno} accordingly.
\seealso{rename, rmdir}
\done

\function{rename}
\synopsis{Rename a file}
\usage{Int_Type rename (String_Type old, String_Type new)}
\description
  The \ifun{rename} function renames a file from \exmp{old} to \exmp{new}
  moving it between directories if necessary.  This function may fail
  if the directories are not on the same file system.  It returns
  0 upon success, or -1 upon error and sets \ivar{errno} accordingly.
\seealso{remove, errno}
\done

\function{rmdir}
\synopsis{Remove a directory}
\usage{Int_Type rmdir (String_Type dir)}
\description
  The \ifun{rmdir} function deletes the specified directory.  It returns
  0 upon success or -1 upon error and sets \ivar{errno} accordingly.
\notes
  The directory must be empty before it can be removed.
\seealso{rename, remove, mkdir}
\done

\function{stat_file}
\synopsis{Get information about a file}
\usage{Struct_Type stat_file (String_Type file)}
\description
  The \ifun{stat_file} function returns information about \exmp{file}
  through the use of the system \cfun{stat} call.  If the stat call
  fails, the function returns \NULL and sets errno accordingly.
  If it is successful, it returns a stat structure with the following
  integer-value fields:
#v+
    st_dev
    st_ino
    st_mode
    st_nlink
    st_uid
    st_gid
    st_rdev
    st_size
    st_atime
    st_mtime
    st_ctime
#v-
  See the C library documentation of \cfun{stat} for a discussion of the
  meanings of these fields.
\example
  The following example shows how the \ifun{stat_file} function may be
  used to get the size of a file:
#v+
     define file_size (file)
     {
        variable st;
        st = stat_file(file);
        if (st == NULL)
          throw IOError, "Unable to stat $file"$;
        return st.st_size;
     }
#v-
\seealso{lstat_file, stat_is, stat_mode_to_string, utime}
\done

\function{stat_is}
\synopsis{Parse the st_mode field of a stat structure}
\usage{Char_Type stat_is (String_Type type, Int_Type st_mode)}
\description
  The \ifun{stat_is} function returns a boolean value according to
  whether or not the \exmp{st_mode} parameter is of the specified type.
  Specifically, \exmp{type} must be one of the strings:
#v+
     "sock"     (socket)
     "fifo"     (fifo)
     "blk"      (block device)
     "chr"      (character device)
     "reg"      (regular file)
     "lnk"      (link)
     "dir"      (dir)
#v-
  It returns a non-zero value if \exmp{st_mode} corresponds to
  \exmp{type}.
\example
  The following example illustrates how to use the \ifun{stat_is}
  function to determine whether or not a file is a directory:
#v+
     define is_directory (file)
     {
        variable st;

        st = stat_file (file);
        if (st == NULL) return 0;
        return stat_is ("dir", st.st_mode);
     }
#v-
\seealso{stat_file, lstat_file, stat_mode_to_string}
\done

\function{stat_mode_to_string}
\synopsis{Format the file type code and access permission bits as a string}
\usage{String_Type stat_mode_to_string (Int_Type st_mode)}
\description
  The \ifun{stat_mode_to_string} function returns a 10 characters string
  that indicates the type and permissions of a file as represented by
  the \var{st_mode} parameter.  The returned string consists of the following
  characters:
#v+
     "s"      (socket)
     "p"      (fifo)
     "b"      (block device)
     "c"      (character device)
     "-"      (regular file)
     "l"      (link)
     "d"      (dir)
#v-
  The access permission bit is one of the following characters:
#v+
     "s"      (set-user-id)
     "w"      (writable)
     "x"      (executable)
     "r"      (readable)
#v-
\notes
  This function is an \slsh intrinsic.  As such, it is not part of
  \slang proper.
\seealso{stat_file, lstat_file, stat_is}
\done

\function{symlink}
\synopsis{Create a symbolic link}
\usage{status = symlink (String_Type oldpath, String_Type new_path)}
\description
  The \ifun{symlink} function may be used to create a symbolic link
  named \exmp{new_path} for  \exmp{oldpath}.  If successful, the function
  returns 0, otherwise it returns -1 and sets \ivar{errno} appropriately.
\notes
  This function is not supported on all systems and even if supported,
  not all file systems support the concept of a symbolic link.
\seealso{readlink, hardlink}
\done

\function{utime}
\synopsis{Change a file's last access and modification times}
\usage{Int_Type utime(String_Type file, Double_Type actime, Double_Type modtime)}
\description
 This function may be used to change the last access (actime) and last
 modification (modtime) times associated with the specified file.  If
 sucessful, the function returns 0; otherwise it returns -1 and sets
 \ivar{errno} accordingly.
\notes
 The \ifun{utime} function will call the C library \cfun{utimes}
 function if available, which permits microsecond accuracy.
 Otherwise, it will truncate the time arguments to integers and call
 the \cfun{utime} function.
\seealso{stat_file}
\done
