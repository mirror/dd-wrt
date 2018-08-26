\function{new_process}
\synopsis{Create a subprocess object}
\usage{Struct_Type new_process (String_Type argv[]; qualifiers)}
\description
  This function executes the program specified by the \exmp{argv}
  parameter in a subprocess.  If \exmp{argv} is an array, the first
  element (\exmp{argv[0]}) of the array gives the name of the program
  to be executed, and the remaining elements serve as arguments passed
  to the program.  The program returns a structure that may be used to
  interact with the process.  Upon error, an exception will be thrown.

  The calling program may interact with the subprocess by reading from
  or writing to the file descriptor fields of the structure returned
  by the \sfun{new_process} function.  The specific file descriptors
  are dictated via the \exmp{read}, \exmp{write}, and \exmp{dupN}
  qualifiers, as described in detail below.

  The function returns a structure containing zero or more fields of the form
  \exmp{fdN} where \exmp{N} is an integer derived from the qualifiers,
  e.g., \exmp{fd0} and \exmp{fd1} correspond to the child's stdin and
  stdout, respectively.  The structure also contains fields of the
  form \exmp{fpN} whose values are stdio \dtype{File_Type} objects
  obtained using \exmp{fdopen} with the correponding \exmp{fdN} value.

  Other important fields include \exmp{pid} whose value is
  the process-id of the newly created process.

  The status of the process may be checked or collected using the
  \sfun{wait} method.  It is very important to call this method to
  avoid the creation of zombie processes.

\qualifiers
  The following qualifiers are supported:
#v+
   read=fds
#v-
    fds is a list of integer file descriptors that are open for read
    access in the subprocess, and may be written to by the calling
    process using the fdN or fpN fields of the structure.
#v+
   write=fds
#v-
    fds is a list of integer file descriptors that are open for write
    access in the subprocess, and may be read to by the calling
    process using the fdN or fpN fields of the structure.
#v+
   stdin=filename
   stdout=filename
   stderr=filename
#v-
    These qualifiers allow the stdin, stdout, and stderr file
    descriptors in the subprocess to be redirected to a file.  Note:
    The filenames are interpreted relative to the value of the
    \exmp{dir} qualifier.
#v+
   fdN=string
#v-
    This qualifier will cause the integer file descriptor N to be open
    in the subprocess and redirected to the filename represented by
    the string, which is interpreted relative to the value of the \exmp{dir}
    qualifier.  The access mode is dictated by the first few
    characters of the string as described in more detail below.
#v+
   stdin=File_Type|FD_Type
   stdout=File_Type|FD_Type
   stderr=File_Type|FD_Type
   fdN=FD_Type|FD_Type
#v-
     If the stdin, stdout, stderr, or fdN qualifiers have File_Type or
     FD_Type values, then corresponding file descriptors in the
     subprocess will be dup'd to FD_Type or FP_Type file descriptor.
     This form of the qualifier may be used to setup pipelines.
#v+
   dupN=int
#v-
     The file descriptor corresponding to the integer N in the
     subprocess is created by duping the descriptor given by the
     integer value of the qualifier.  For example, dup2=1 would cause
     stderr (fd=2) in the subprocess to be redirected to stdout (fd=1).
#v+
   dir=string
#v-
     Change to the specified directory in the child process.  This
     will happen after the child process is started, but before any
     files have been opened.  Hence, files attached to \ivar{stdin},
     \ivar{stdout}, etc will be opened relative to this directory.
#v+
   pre_exec_hook=&func
#v-
     This qualifier will cause the function corresponding to func to
     be called prior to closing unused file descriptors and invoking
     the executable.  The function will be passed a list of integer
     valued of file descriptors that will be kept open.  Additional
     integers may be added to the list by the function.

  Note that the read and write qualifiers specify the nature of the
  file descriptors from the child process's view.  That is, those
  opened in the child process using the read qualifier, may be written
  to by the parent.  Similarly, those opened using the write qualifier
  may be read by the parent.

\methods
#v+
    Struct_Type .wait ( [ options ] )
#v-
  The \exmp{.wait} method may be used to collect the exist status of
  the process.  When called without arguments, it will cause the
  parent process to wait for the subprocess to exit and return its
  exit status in the form of a \ifun{waitpid} structure.  The optional
  \exmp{options} argument corresponds to the options argument of the
  \ifun{waitpid} function.  The most common is the WNOHANG option,
  which will cause the \exmp{.wait} method to return immediately if
  the process has not exited.

  If an error occurs, the function will return NULL and set
  \ivar{errno} accordingly.  Otherwise it will return a \exmp{waitpid}
  structure.  See the documentation for \exmp{waitpid} for more
  information.

\example
  In the following examples, \exmp{pgm} represents the program to be
  invoked in the subprocess.  For simplicity, no addition arguments are
  shown

  Create subprocess that inherits stdin, stdout, stderr from the caller:
#v+
   obj = new_process (pgm);
#v-

  Create a subprocess that inherits stdin, stdout, and writes stderr
  to a file:
#v+
   obj = new_process (pgm; stderr="/tmp/file");   % form 1
   obj = new_process (pgm; fd2=">/tmp/file");     % form 2
#v-

  Mimic popen(pgm, "r"):
#v+
   obj = new_process (pgm; write=1);   % Read from obj.fp1
#v-

  Mimic popen(pgm, "w"):
#v+
   obj = new_process (pgm; read=0);  % Write to obj.fp0
#v-

  Mimic popen("pgm 2>&1", "r"):
#v+
   obj = new_process (pgm; write=1, dup2=1);  % Read from fp1
#v-

  Send stdout to a file, read from the subprocess's stderr:
#v+
   obj = new_process (pgm; stdout="/tmp/file", write=2);
   % Read from obj.fp2
#v-

  Create a process with handles to its stdin, stdout, stderr
#v+
   obj = new_process (pgm; write={1,2}, read=0);
   % Use obj.fp0 for stdin, obj.fp1 for stdout, and obj.fp2 for stderr
#v-

  Create a process with a write handle to the process's fd=27 and a
  read handle to the process's stdout.
#v+
   obj = new_process (pgm; read=27, write=1);
   % write to fp27, read from fp1
#v-

  Create a pipeline: pgm1 | pgm2 > /tmp/log :
#v+
  obj1 = new_process (pgm1; write=1);
  obj2 = new_process (pgm2; stdin=obj1.fp1, stdout="/tmp/log");
#v-

  Create a pipeline with fd=27 from pgm1 redirected to stdin of pgm2:
#v+
  obj1 = new_process (pgm1; write=27);
  obj2 = new_process (pgm2; stdin=obj1.fp27);
#v-

  Create a pipeline with fd=27 from pgm1 redirected to fd=9 of pgm2:
#v+
  obj1 = new_process (pgm1; write=27);
  obj2 = new_process (pgm2; fp9=obj1.fp27);
#v-

  Mimic: pgm 2>&1 1>/dev/null
#v+
  obj = new_process (pgm; fp2=1, stdout="/dev/null");
#v-

  Mimic: pgm >/dev/null 2>&1
#v+
  obj = new_process (pgm; stdout="/dev/null", dup2=1);
#v-

  Append the output of pgm to /tmp/file.log:
#v+
   obj = new_process (pgm; stdout=">>/tmp/file.log");
#v-

\notes
  Care must be exercised when reading or writing to multiple file
  descriptors of a subprocess to avoid deadlock.  In such cases, the
  select module should be used, or the file descriptors could be put in
  non-blocking mode via the fcntl module.

  It is important to call the \exmp{.wait} method prevent the process
  from becoming a zombie and clogging the process table.
\seealso{popen, system}
\done
