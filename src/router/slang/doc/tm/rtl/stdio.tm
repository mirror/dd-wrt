\function{clearerr}
\synopsis{Clear the error of a file stream}
\usage{clearerr (File_Type fp}
\description
  The \ifun{clearerr} function clears the error and end-of-file flags
  associated with the open file stream \exmp{fp}.
\seealso{ferror, feof, fopen}
\done

\function{fclose}
\synopsis{Close a file}
\usage{Integer_Type fclose (File_Type fp)}
\description
  The \ifun{fclose} function may be used to close an open file pointer
  \exmp{fp}.  Upon success it returns zero, and upon failure it sets
  \ivar{errno} and returns \exmp{-1}.  Failure usually indicates a that
  the file system is full or that \exmp{fp} does not refer to an open file.
\notes
  Many C programmers call \ifun{fclose} without checking the return
  value.  The \slang language requires the programmer to explicitly
  handle any value returned by a function.  The simplest way to
  handle the return value from \ifun{fclose} is to call it via:
#v+
     () = fclose (fp);
#v-
\seealso{fopen, fgets, fflush, pclose, errno}
\done

\function{fdopen}
\synopsis{Convert a FD_Type file descriptor to a stdio File_Type object}
\usage{File_Type fdopen (FD_Type, String_Type mode)}
\description
   The \ifun{fdopen} function creates and returns a stdio
   \dtype{File_Type} object from the open \dtype{FD_Type}
   descriptor \exmp{fd}.  The \exmp{mode} parameter corresponds to the
   \exmp{mode} parameter of the \ifun{fopen} function and must be
   consistent with the mode of the descriptor \exmp{fd}.  The function
   returns \NULL upon failure and sets \ivar{errno}.
\notes
   Since the stdio \dtype{File_Type} object created by this function
   is derived from the \dtype{FD_Type} descriptor, the \dtype{FD_Type}
   is regarded as more fundamental than the \dtype{File_Type} object.
   This means that the descriptor must be in scope while the
   \dtype{File_Type} object is used.  In particular, if the descriptor
   goes out of scope, the descriptor will get closed causing I/O to the
   \dtype{File_Type} object to fail, e.g.,
#v+
     fd = open ("/path/to/file", O_RDONLY);
     fp = fdopen (fd);
     fd = 0;     % This will cause the FD_Type descriptor to go out of
                 % scope.  Any I/O on fp will now fail.
#v-

   Calling the \ifun{fclose} function on the \dtype{File_Type} object
   will cause the underlying descriptor to close.

   Any stdio \dtype{File_Type} object created by the \ifun{fdopen}
   function will remain associated with the \dtype{FD_Type} descriptor,
   unless the object is explicitly removed via \ifun{fclose}.  This
   means that code such as
#v+
      fd = open (...);
      loop (50)
        {
           fp = fdopen (fd, ...);
              .
              .
        }
#v-
   will result in 50 \dtype{File_Type} objects attached to \exmp{fd}
   after the loop has terminated.
\seealso{fileno, fopen, open, close, fclose, dup_fd}
\done

\function{feof}
\synopsis{Get the end-of-file status}
\usage{Integer_Type feof (File_Type fp)}
\description
  This function may be used to determine the state of the end-of-file
  indicator of the open file descriptor \exmp{fp}.  It returns zero
  if the indicator is not set, or non-zero if it is.  The end-of-file
  indicator may be cleared by the \ifun{clearerr} function.
\seealso{ferror, clearerr, fopen}
\done

\function{ferror}
\synopsis{Determine the error status of an open file descriptor}
\usage{Integer_Type ferror (File_Type fp)}
\description
  This function may be used to determine the state of the error
  indicator of the open file descriptor \exmp{fp}.  It returns zero
  if the indicator is not set, or non-zero if it is.  The error
  indicator may be cleared by the \ifun{clearerr} function.
\seealso{feof, clearerr, fopen}
\done

\function{fflush}
\synopsis{Flush an output stream}
\usage{Integer_Type fflush (File_Type fp)}
\description
  The \ifun{fflush} function may be used to update the stdio \em{output}
  stream specified by \exmp{fp}.  It returns 0 upon success, or
  -1 upon failure and sets \ivar{errno} accordingly.  In
  particular, this function will fail if \exmp{fp} does not represent
  an open output stream, or if \exmp{fp} is associated with a disk file and
  there is insufficient disk space.
\example
  This example illustrates how to use the \ifun{fflush} function
  without regard to the return value:
#v+
    () = fputs ("Enter value> ", stdout);
    () = fflush (stdout);
#v-
\seealso{fopen, fclose}
\done

\function{fgets}
\synopsis{Read a line from a file}
\usage{Integer_Type fgets (SLang_Ref_Type ref, File_Type fp)}
\description
  \ifun{fgets} reads a line from the open file specified by \exmp{fp}
  and places the characters in the variable whose reference is
  specified by \exmp{ref}.
  It returns -1 if \exmp{fp} is not associated with an open file
  or an attempt was made to read at the end the file; otherwise, it
  returns the number of characters read.
\example
  The following example returns the lines of a file via a linked list:
#v+
    define read_file (file)
    {
       variable buf, fp, root, tail;
       variable list_type = struct { text, next };

       root = NULL;

       fp = fopen(file, "r");
       if (fp == NULL)
         throw OpenError, "fopen failed to open $file for reading"$;
       while (-1 != fgets (&buf, fp))
         {
            if (root == NULL)
              {
                 root = @list_type;
                 tail = root;
              }
            else
              {
                 tail.next = @list_type;
                 tail = tail.next;
              }
            tail.text = buf;
            tail.next = NULL;
         }
       () = fclose (fp);
       return root;
    }
#v-
\seealso{fgetslines, fopen, fclose, fputs, fread, error}
\done

\function{fgetslines}
\synopsis{Read lines as an array from an open file}
\usage{String_Type[] fgetslines (File_Type fp [,Int_Type num])}
\description
  The \ifun{fgetslines} function reads lines a specified number of
  lines as an array of strings from the file associated with the
  file pointer \exmp{fp}.  If the number of lines to be read is left
  unspecified, the function will return the rest of the lines in the
  file.  If the file is empty, an empty string array will be returned.
  The function returns \NULL upon error.
\example
  The following function returns the number of lines in a file:
#v+
    define count_lines_in_file (file)
    {
       variable fp, lines;

       fp = fopen (file, "r");
       if (fp == NULL)
         return -1;

       lines = fgetslines (fp);
       if (lines == NULL)
         return -1;

       return length (lines);
    }
#v-
  Note that the file was implicitly closed when the variable \exmp{fp}
  goes out of scope (in the case, when the function returns).
\seealso{fgets, fread, fopen, fputslines}
\done

\function{fopen}
\synopsis{Open a file}
\usage{File_Type fopen (String_Type f, String_Type m)}
\description
  The \ifun{fopen} function opens a file \exmp{f} according to the mode
  string \exmp{m}.  Allowed values for \exmp{m} are:
#v+
     "r"    Read only
     "w"    Write only
     "a"    Append
     "r+"   Reading and writing at the beginning of the file.
     "w+"   Reading and writing.  The file is created if it does not
              exist; otherwise, it is truncated.
     "a+"   Reading and writing at the end of the file.  The file is created
              if it does not already exist.
#v-
  In addition, the mode string can also include the letter \exmp{'b'}
  as the last character to indicate that the file is to be opened in
  binary mode.

  Upon success, \ifun{fopen} returns a \dtype{File_Type} object which is
  meant to be used by other operations that require an open file
  pointer.  Upon failure, the function returns \NULL.
\example
  The following function opens a file in append mode and writes a
  string to it:
#v+
    define append_string_to_file (str, file)
    {
       variable fp = fopen (file, "a");
       if (fp == NULL)
         throw OpenError, "$file could not be opened"$;
       () = fputs (str, fp);
       () = fclose (fp);
    }
#v-
  Note that the return values from \ifun{fputs} and \ifun{fclose} were
  ignored.
\notes
  There is no need to explicitly close a file opened with \ifun{fopen}.
  If the returned \dtype{File_Type} object goes out of scope, the
  interpreter will automatically close the file.  However, explicitly
  closing a file with \ifun{fclose} and checking its return value is
  recommended.
\seealso{fclose, fgets, fputs, popen}
\done

\function{fprintf}
\synopsis{Create and write a formatted string to a file}
\usage{Int_Type fprintf (File_Type fp, String_Type fmt, ...)}
\description
  \ifun{fprintf} formats the objects specified by the variable argument
  list according to the format \exmp{fmt} and write the result to the
  open file pointer \exmp{fp}.

  The format string obeys the same syntax and semantics as the
  \ifun{sprintf} format string.  See the description of the
  \ifun{sprintf} function for more information.

  \ifun{fprintf} returns the number of bytes written to the file,
  or -1 upon error.
\seealso{fputs, printf, fwrite, message}
\done

\function{fputs}
\synopsis{Write a string to an open stream}
\usage{Integer_Type fputs (String_Type s, File_Type fp)}
\description
  The \ifun{fputs} function writes the string \exmp{s} to the open file
  pointer \exmp{fp}. It returns -1 upon failure and sets \ivar{errno},
  otherwise it returns the length of the string.
\example
  The following function opens a file in append mode and uses the
  \ifun{fputs} function to write to it.
#v+
    define append_string_to_file (str, file)
    {
       variable fp;
       fp = fopen (file, "a");
       if (fp == NULL)
         throw OpenError, "Unable to open $file"$;
       if ((-1 == fputs (s, fp))
           or (-1 == fclose (fp)))
         throw WriteError, "Error writing to $file"$;
    }
#v-
\notes
  One must not disregard the return value from the \ifun{fputs}
  function.  Doing so may lead to a stack overflow error.

  To write an object that contains embedded null characters, use the
  \ifun{fwrite} function.
\seealso{fclose, fopen, fgets, fwrite}
\done

\function{fputslines}
\synopsis{Write an array of strings to an open file}
\usage{Int_Type fputslines (String_Type[]a, File_Type fp)}
\description
  The \ifun{fputslines} function writes an array of strings to the
  specified file pointer.  It returns the number of elements
  successfully written.  Any \NULL elements in the array will be
  skipped.
\example
#v+
    if (length (lines) != fputslines (lines, fp))
      throw WriteError;
#v-
\seealso{fputs, fgetslines, fopen}
\done

\function{fread}
\synopsis{Read binary data from a file}
\usage{UInt_Type fread (Ref_Type b, DataType_Type t, UInt_Type n, File_Type fp)}
\description
  The \ifun{fread} function may be used to read \exmp{n} objects of type
  \exmp{t} from an open file pointer \exmp{fp}.  Upon success, it
  returns the number of objects read from the file and places the
  objects in variable specified by \exmp{b}.  Upon error or
  end-of-file, it returns -1 and sets \ivar{errno} accordingly.

  If more than one object is read from the file, those objects will be
  placed in an array of the appropriate size.
\example
  The following example illustrates how to read 50 integers from a file:
#v+
     define read_50_ints_from_a_file (file)
     {
        variable fp, n, buf;

        fp = fopen (file, "rb");
        if (fp == NULL)
          throw OpenError;
        n = fread (&buf, Int_Type, 50, fp);
        if (n == -1)
          throw ReadError, "fread failed";
        () = fclose (fp);
        return buf;
     }
#v-
\notes
  Use the \ifun{pack} and \ifun{unpack} functions to read data with a
  specific byte-ordering.

  The \ifun{fread_bytes} function may be used to read a specified number of
  bytes in the form of a binary string (\exmp{BString_Type}).

  If an attempt is made to read at the end of a file, the function
  will return -1.  To distinguish this condition from a system error,
  the \ifun{feof} function should be used.  This distinction is
  particularly important when reading from a socket or pipe.
\seealso{fread_bytes, fwrite, fgets, feof, ferror, fopen, pack, unpack}
\done

\function{fread_bytes}
\synopsis{Read bytes from a file as a binary-string}
\usage{UInt_Type fread_bytes (Ref_Type b, UInt_Type n, File_Type fp)}
\description
  The \ifun{fread_bytes} function may be used to read \exmp{n} bytes
  from from an open file pointer \exmp{fp}.  Upon success, it returns
  the number of bytes read from the file and assigns to the variable
  attached to the reference \exmp{b} a binary string formed from the
  bytes read.  Upon error or end of file, the function returns
  -1 and sets \ivar{errno} accordingly.
\notes
  Use the \ifun{pack} and \ifun{unpack} functions to read data with a
  specific byte-ordering.
\seealso{fread, fwrite, fgets, fopen, pack, unpack}
\done

\function{fseek}
\synopsis{Reposition a stdio stream}
\usage{Integer_Type fseek (File_Type fp, LLong_Type ofs, Integer_Type whence}
\description
  The \ifun{fseek} function may be used to reposition the file position
  pointer associated with the open file stream \exmp{fp}. Specifically,
  it moves the pointer \exmp{ofs} bytes relative to the position
  indicated by \exmp{whence}.  If \exmp{whence} is set to one of the symbolic
  constants \icon{SEEK_SET}, \icon{SEEK_CUR}, or \icon{SEEK_END}, the
  offset is relative to the start of the file, the current position
  indicator, or end-of-file, respectively.

  The function returns 0 upon success, or -1 upon failure and sets
  \ivar{errno} accordingly.
\example
    define rewind (fp)
    {
       if (0 == fseek (fp, 0, SEEK_SET)) return;
       vmessage ("rewind failed, reason: %s", errno_string (errno));
    }
\seealso{ftell, fopen}
\done

\function{ftell}
\synopsis{Obtain the current position in an open stream}
\usage{LLong_Type ftell (File_Type fp)}
\description
  The ftell function may be used to obtain the current position in the
  stream associated with the open file pointer \exmp{fp}.  It returns
  the position of the pointer measured in bytes from the beginning of
  the file.  Upon error, it returns \exmp{-1} and sets \ivar{errno}
  accordingly.
\seealso{fseek, fopen}
\done

\function{fwrite}
\synopsis{Write binary data to a file}
\usage{UInt_Type fwrite (b, File_Type fp)}
\description
  The \ifun{fwrite} function may be used to write the object represented by
  \exmp{b} to an open file.  If \exmp{b} is a string or an array, the
  function will attempt to write all elements of the object to the
  file.  It returns the number of elements successfully written,
  otherwise it returns \-1 upon error and sets \ivar{errno}
  accordingly.
\example
  The following example illustrates how to write an integer array to a
  file.  In this example, \exmp{fp} is an open file descriptor:
#v+
     variable a = [1:50];     % 50 element integer array
     if (50 != fwrite (a, fp))
       throw WriteError;
#v-
  Here is how to write the array one element at a time:
#v+
     variable ai, a = [1:50];

     foreach ai (a)
       {
          if (1 != fwrite(ai, fp))
            throw WriteError;
       }
#v-
\notes
  Not all data types may be supported the \ifun{fwrite} function.  It
  is supported by all vector, scalar, and string objects.
\seealso{fread, fputs, fopen, pack, unpack}
\done

\function{pclose}
\synopsis{Close a process pipe}
\usage{Integer_Type pclose (File_Type fp)}
\description
  The \ifun{pclose} function waits for the process associated with
  \exmp{fp} to exit and the returns the exit status of the command.
\seealso{pclose, fclose}
\done

\function{popen}
\synopsis{Open a pipe to a process}
\usage{File_Type popen (String_Type cmd, String_Type mode)}
\description
  The \ifun{popen} function executes a process specified by \exmp{cmd}
  and opens a unidirectional pipe to the newly created process.  The
  \exmp{mode} indicates whether or not the pipe is open for reading
  or writing.  Specifically, if \exmp{mode} is \exmp{"r"}, then the
  pipe is opened for reading, or if \exmp{mode} is \exmp{"w"}, then the
  pipe will be open for writing.

  Upon success, a \dtype{File_Type} pointer will be returned, otherwise
  the function failed and \NULL will be returned.
\notes
  This function is not available on all systems.

 The \module{process} module's \ifun{new_process} function provides a
 much more secure and powerful interface to process I/O.
\seealso{new_process, pclose, fopen}
\done

\function{printf}
\synopsis{Create and write a formatted string to stdout}
\usage{Int_Type printf (String_Type fmt, ...)}
\description
  \ifun{printf} formats the objects specified by the variable argument
  list according to the format \exmp{fmt} and write the result to
  \ivar{stdout}.  This function is equivalent to \exmp{fprintf} used
  with the \ivar{stdout} file pointer.  See \exmp{fprintf} for more
  information.

  \exmp{printf} returns the number of bytes written or -1 upon error.
\notes
  Many C programmers do not check the return status of the
  \exmp{printf} C library function.  Make sure that if you do not care
  about whether or not the function succeeds, then code it as in the
  following example:
#v+
     () = printf ("%s laid %d eggs\n", chicken_name, num_egg);
#v-
\seealso{fputs, fprintf, fwrite, message}
\done

