\function{path_basename}
\synopsis{Get the basename part of a filename}
\usage{String_Type path_basename (String_Type filename)}
\description
   The \ifun{path_basename} function returns the basename associated
   with the \exmp{filename} parameter.  The basename is the non-directory
   part of the filename, e.g., on unix \exmp{c} is the basename of
   \exmp{/a/b/c}.
\seealso{path_dirname, path_extname, path_concat, path_is_absolute}
\done

\function{path_basename_sans_extname}
\synopsis{Get the basename part of a filename but without the extension}
\usage{String_Type path_basename_sans_extname (String_Type path)}
\description
   The \ifun{path_basename_sans_extname} function returns the basename
   associated with the \exmp{filename} parameter, omitting the
   extension if present.  The basename is the non-directory part of
   the filename, e.g., on unix \exmp{c} is the basename of
   \exmp{/a/b/c}.
\seealso{path_dirname, path_basename, path_extname, path_concat, path_is_absolute}
\done

\function{path_concat}
\synopsis{Combine elements of a filename}
\usage{String_Type path_concat (String_Type dir, String_Type basename)}
\description
   The \ifun{path_concat} function combines the arguments \exmp{dir} and
   \exmp{basename} to produce a filename.  For example, on Unix if
   \exmp{dir} is \exmp{x/y} and \exmp{basename} is \exmp{z}, then the
   function will return \exmp{x/y/z}.
\seealso{path_dirname, path_basename, path_extname, path_is_absolute}
\done

\function{path_dirname}
\synopsis{Get the directory name part of a filename}
\usage{String_Type path_dirname (String_Type filename)}
\description
   The \ifun{path_dirname} function returns the directory name
   associated with a specified filename.
\notes
   On systems that include a drive specifier as part of the filename,
   the value returned by this function will also include the drive
   specifier.
\seealso{path_basename, path_extname, path_concat, path_is_absolute}
\done

\function{path_extname}
\synopsis{Return the extension part of a filename}
\usage{String_Type path_extname (String_Type filename)}
\description
   The \ifun{path_extname} function returns the extension portion of the
   specified filename.  If an extension is present, this function will
   also include the dot as part of the extension, e.g., if \exmp{filename}
   is \exmp{"file.c"}, then this function will return \exmp{".c"}.  If no
   extension is present, the function returns an empty string \exmp{""}.
\notes
   Under VMS, the file version number is not returned as part of the
   extension.
\seealso{path_sans_extname, path_dirname, path_basename, path_concat, path_is_absolute}
\done

\function{path_get_delimiter}
\synopsis{Get the value of a search-path delimiter}
\usage{Char_Type path_get_delimiter ()}
\description
  This function returns the value of the character used to delimit
  fields of a search-path.
\seealso{set_slang_load_path, get_slang_load_path}
\done

\function{path_is_absolute}
\synopsis{Determine whether or not a filename is absolute}
\usage{Int_Type path_is_absolute (String_Type filename)}
\description
   The \ifun{path_is_absolute} function will return non-zero is
   \exmp{filename} refers to an absolute filename, otherwise it returns zero.
\seealso{path_dirname, path_basename, path_extname, path_concat}
\done

\function{path_sans_extname}
\synopsis{Strip the extension from a filename}
\usage{String_Type path_sans_extname (String_Type filename)}
\description
  The \ifun{path_sans_extname} function removes the file name extension
  (including the dot) from the filename and returns the result.
\seealso{path_basename_sans_extname, path_extname, path_basename, path_dirname, path_concat}
\done

