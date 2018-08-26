\function{close}
\synopsis{Close an open file descriptor}
\usage{Int_Type close (FD_Type fd)}
\description
  The \ifun{close} function is used to close and open file descriptor
  created by the \ifun{open} function.  Upon success \0 is returned,
  otherwise the function returns \-1 and sets \ivar{errno} accordingly.
\seealso{open, fclose, read, write}
\done

\function{dup_fd}
\synopsis{Duplicate a file descriptor}
\usage{FD_Type dup_fd (FD_Type fd)}
\description
  The \ifun{dup_fd} function duplicates a specified file descriptor and
  returns the duplicate.  If the function fails, \NULL will be
  returned and \ivar{errno} set accordingly.
\notes
  This function is essentially a wrapper around the POSIX \cfun{dup}
  function.
\seealso{open, close}
\done

\function{_fileno}
\synopsis{Get the underlying integer file descriptor}
\usage{Int_Type fileno (File_Type|FD_Type fp)}
\description
  The \ifun{_fileno} function returns the underlying integer
  descriptor for a specified stdio \dtype{File_Type} or
  \dtype{FD_Type} object.  Upon failure it returns -1 and sets
  \ivar{errno} accordingly.
\seealso{fileno, fopen, open, fclose, close, dup_fd}
\done

\function{fileno}
\synopsis{Convert a stdio File_Type object to a FD_Type descriptor}
\usage{FD_Type fileno (File_Type fp)}
\description
  The \ifun{fileno} function returns the \dtype{FD_Type} descriptor
  associated with the stdio \dtype{File_Type} file pointer.  Upon failure,
  \NULL is returned.
\notes
  Closing the resulting file descriptor will have no effect.
\seealso{fopen, open, fclose, close, dup_fd, _fileno}
\done

\function{isatty}
\synopsis{Determine if an open file descriptor refers to a terminal}
\usage{Int_Type isatty (FD_Type or File_Type fd)}
\description
  This function returns \1 if the file descriptor \exmp{fd} refers to a
  terminal; otherwise it returns \0.  The object \exmp{fd} may either
  be a \dtype{File_Type} stdio descriptor or a lower-level \dtype{FD_Type}
  object.
\seealso{fopen, fclose, fileno}
\done

\function{lseek}
\synopsis{Reposition a file descriptor's file pointer}
\usage{Long_Type lseek (FD_Type fd, LLong_Type ofs, int mode)}
   The \ifun{lseek} function repositions the file pointer associated
   with the open file descriptor \exmp{fd} to the offset \exmp{ofs}
   according to the mode parameter.  Specifically, \exmp{mode} must be
   one of the values:
#v+
     SEEK_SET   Set the offset to ofs from the beginning of the file
     SEEK_CUR   Add ofs to the current offset
     SEEK_END   Add ofs to the current file size
#v-
   Upon error, \ifun{lseek} returns \-1 and sets \ivar{errno}.  If
   successful, it returns the new filepointer offset.
\notes
   Not all file descriptors are capable of supporting the seek
   operation, e.g., a descriptor associated with a pipe.

   By using \icon{SEEK_END} with a positive value of the \exmp{ofs}
   parameter, it is possible to position the file pointer beyond the
   current size of the file.
\seealso{fseek, ftell, open, close}
\done

\function{open}
\synopsis{Open a file}
\usage{FD_Type open (String_Type filename, Int_Type flags [,Int_Type mode])}
\description
  The \ifun{open} function attempts to open a file specified by the
  \exmp{filename} parameter according to the \exmp{flags} parameter,
  which must be one of the following values:
#v+
     O_RDONLY   (read-only)
     O_WRONLY   (write-only)
     O_RDWR     (read/write)
#v-
  In addition, \exmp{flags} may also be bitwise-or'd with any of the
  following:
#v+
     O_BINARY   (open the file in binary mode)
     O_TEXT     (open the file in text mode)
     O_CREAT    (create the file if it does not exist)
     O_EXCL     (fail if the file already exists)
     O_NOCTTY   (do not make the device the controlling terminal)
     O_TRUNC    (truncate the file if it exists)
     O_APPEND   (open the file in append mode)
     O_NONBLOCK (open the file in non-blocking mode)
#v-
   Some of these flags make sense only when combined with other flags.
   For example, if O_EXCL is used, then O_CREAT must also be
   specified, otherwise unpredictable behavior may result.

   If \icon{O_CREAT} is used for the \exmp{flags} parameter then the
   \exmp{mode} parameter must be present. \exmp{mode} specifies the
   permissions to use if a new file is created. The actual file
   permissions will be affected by the process's \exmp{umask} via
   \exmp{mode&~umask}.  The \exmp{mode} parameter's value is
   constructed via bitwise-or of the following values:
#v+
     S_IRWXU    (Owner has read/write/execute permission)
     S_IRUSR    (Owner has read permission)
     S_IWUSR    (Owner has write permission)
     S_IXUSR    (Owner has execute permission)
     S_IRWXG    (Group has read/write/execute permission)
     S_IRGRP    (Group has read permission)
     S_IWGRP    (Group has write permission)
     S_IXGRP    (Group has execute permission)
     S_IRWXO    (Others have read/write/execute permission)
     S_IROTH    (Others have read permission)
     S_IWOTH    (Others have write permission)
     S_IXOTH    (Others have execute permission)
#v-
   Upon success \ifun{open} returns a file descriptor object
   (\dtype{FD_Type}), otherwise \NULL is returned and \ivar{errno}
   is set.
\notes
   If you are not familiar with the \ifun{open} system call, then it
   is recommended that you use \ifun{fopen} instead and use the higher
   level stdio interface.
\seealso{fopen, close, read, write, stat_file}
\done

\function{read}
\synopsis{Read from an open file descriptor}
\usage{UInt_Type read (FD_Type fd, Ref_Type buf, UInt_Type num)}
\description
  The \ifun{read} function attempts to read at most \exmp{num} bytes
  into the variable indicated by \exmp{buf} from the open file
  descriptor \exmp{fd}.  It returns the number of bytes read, or \-1
  upon failure and sets \ivar{errno}.  The number of bytes
  read may be less than \exmp{num}, and will be zero if an attempt is
  made to read past the end of the file.
\notes
  \ifun{read} is a low-level function and may return \-1 for a variety
  of reasons.  For example, if non-blocking I/O has been specified for
  the open file descriptor and no data is available for reading then
  the function will return \-1 and set \ivar{errno} to \icon{EAGAIN}.
\seealso{fread, open, close, write}
\done

\function{write}
\synopsis{Write to an open file descriptor}
\usage{UInt_Type write (FD_Type fd, BString_Type buf)}
\description
   The \ifun{write} function attempts to write the bytes specified by
   the \exmp{buf} parameter to the open file descriptor \exmp{fd}.  It
   returns the number of bytes successfully written, or \-1 and sets
   \ivar{errno} upon failure.  The number of bytes written may be less
   than \exmp{length(buf)}.
\seealso{read, fwrite, open, close}
\done

