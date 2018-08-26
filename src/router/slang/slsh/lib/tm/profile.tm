\function{profile_on}
\synopsis{Enable code generation to support profiling hooks}
\usage{profile_on ( [line_by_line] )}
\description
  This function will turn cause the interpreter to generate code to
  call hooks that are used to profile code.  The \exmp{profile_on}
  function does not establish those hooks, rather it causes code to be
  generated to support such hooks.

  With no optional argument, only code to support function call hooks
  will be generated.  To enable support for line-by-line profile
  hooks, a value of 1 should be passed to this function.
\seealso{profile_off, profile_begin, profile_end, profile_calibrate, profile_report}}
\done

\function{profile_off}
\synopsis{Turn off code generation for profiling hooks}
\usage{profile_off ()}
\description
  This function turns off the generation of code to support profiling
  hooks.
\seealso{profile_on, profile_begin, profile_end, profile_calibrate, profile_report}}
\done

\function{profile_begin}
\synopsis{Establish profiling hooks and reset the profiler}
\usage{profile_begin ( [line_by_line] )}
\description
  This function establishes the profiling hooks and resets or
  initializes the profiler state.  By default, only hooks to gather
  function call information are created.  To enable the gathering of
  line-by-line information, a value of 1 should be passed to this
  function.
\seealso{profile_end, profile_on, profile_off, profile_calibrate, profile_report}}
\done

\function{profile_end}
\synopsis{Remove profiling hooks and turn off profiler code generation}
\usage{profile_end ()}
\description
  The \sfun{profile_end} function turns off the code generation to
  support profiling hooks and removes any profiling hooks that are in
  place.
\seealso{profile_begin, profile_on, profile_off, profile_calibrate, profile_report}
\done

\function{profile_calibrate}
\synopsis{Calibrate the profiler}
\usage{profile_calibrate ([ N ])}
\description
 This function may be used to ``calibrate'' the profiler.  As the
 performance of the profiler and the interpreter varies with a
 platform and load-dependent manner, this function should be called
 prior to enabling the profiler.  It tries to determine the average
 amount of overhead per statement executed and function call by
 executing a series of statements and functions many times to
 determine statistically accurate values.  The optional parameter
 \exmp{N} may be used to control the amount of code executed
 for the calibration process.  If no value is provided, the \exmp{N}
 will default to 1000.  The higher the value, the more accurate the
 calibration will be and the longer the calibration process will take.
\seealso{profile_begin, profile_end, profile_on, profile_off, profile_report}
\done

\function{profile_report}
\synopsis{Generate the profile report}
\usage{profile_report (file)}
\description
  The function may be used to format the profile report and write it
  to the specified file.  If the \exmp{file} parameter represents a
  \dtype{File_Type} file descriptor, then the report will be written
  the the descriptor.
\seealso{profile_begin, profile_end, profile_on, profile_off, profile_calibrate}
\done
