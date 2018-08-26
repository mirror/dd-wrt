\function{_featurep}
\synopsis{Test whether or not a feature is present}
\usage{Int_Type _featurep (String_Type feature [,String_Type namespace])}
\description
  The \sfun{_featurep} function returns a non-zero value if the specified
  feature is present.  Otherwise, it returns 0 to indicate that the feature
  has not been loaded.
\seealso{require, provide}
\done

\function{provide}
\synopsis{Declare that a specified feature is available}
\usage{provide (String_Type feature [,String_Type namespace])}
\description
 The \sfun{provide} function may be used to declare that a "feature" has
 been loaded into the specified namespace.  If the namespace argument is not
 present, the current namespace will be used.
 See the documentation for \sfun{require} for more information.
\seealso{require, _featurep}
\done

\function{require}
\synopsis{Make sure a feature is present, and load it if not}
\usage{require (feature [,namespace [,file]])}
#v+
   String_Type feature, namespace, file;
#v-
\description
  The \sfun{require} function ensures that a specified "feature" is present.
  If the feature is not present, the \sfun{require} function will attempt to
  load the feature from a file.  If the \exmp{namespace} argument is present
  and non-NULL, the specified namespace will be used.  The default is to use
  the current non-anonymous namespace. If called with three arguments, the
  feature will be loaded from the file specified by the third argument
  if it does not already exist in the namespace.  Otherwise, the feature
  will be loaded from a file given by the name of the feature, with
  ".sl" appended.

  If after loading the file, if the feature is not present,
  a warning message will be issued.
\example
#v+
    require ("histogram");
    require ("histogram", "foo");
    require ("histogram", "foo", "/home/bob/hist.sl");
    require ("histogram", ,"/home/bob/hist.sl");
#v-
\notes
  "feature" is an abstract quantity that is undefined here.

  A popular use of the \sfun{require} function is to ensure that a specified
  file has already been loaded.  In this case, the feature is the
  filename itself.  The advantage of using this mechanism over using
  \ifun{evalfile} is that if the file has already been loaded, \sfun{require}
  will not re-load it.
\seealso{provide, _featurep, evalfile}
\done
