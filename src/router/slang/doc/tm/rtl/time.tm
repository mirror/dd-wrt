\function{ctime}
\synopsis{Convert a calendar time to a string}
\usage{String_Type ctime(Long_Type secs)}
\description
  This function returns a string representation of the time as given
  by \exmp{secs} seconds since 00:00:00 UTC, Jan 1, 1970.
\seealso{time, strftime, _time, localtime, gmtime}
\done

\function{gmtime}
\synopsis{Break down a time in seconds to the GMT timezone}
\usage{Struct_Type gmtime (Long_Type secs)}
\description
  The \ifun{gmtime} function is exactly like \ifun{localtime} except
  that the values in the structure it returns are with respect to GMT
  instead of the local timezone.  See the documentation for
  \ifun{localtime} for more information.
\notes
  On systems that do not support the \ifun{gmtime} C library function,
  this function is the same as \ifun{localtime}.
\seealso{localtime, _time, mktime}
\done

\function{localtime}
\synopsis{Break down a time in seconds to the local timezone}
\usage{Struct_Type localtime (Long_Type secs)}
\description
  The \ifun{localtime} function takes a parameter \exmp{secs}
  representing the number of seconds since 00:00:00, January 1 1970
  UTC and returns a structure containing information about \exmp{secs}
  in the local timezone.  The structure contains the following
  \dtype{Int_Type} fields:

  \var{tm_sec} The number of seconds after the minute, normally
     in the range 0 to 59, but can be up to 61 to allow for
     leap seconds.

  \var{tm_min} The number of minutes after the hour, in the
     range 0 to 59.

  \var{tm_hour} The number of hours past midnight, in the range
     0 to 23.

  \var{tm_mday} The day of the month, in the range 1 to 31.

  \var{tm_mon} The number of months since January, in the range
     0 to 11.

  \var{tm_year} The number of years since 1900.

  \var{tm_wday} The number of days since Sunday, in the range 0
     to 6.

  \var{tm_yday} The number of days since January 1, in the
     range 0 to 365.

  \var{tm_isdst} A flag that indicates whether daylight saving
     time is in effect at the time described.  The value is
     positive if daylight saving time is in effect, zero if it
     is not, and negative if the information is not available.
\seealso{gmtime, _time, ctime, mktime}
\done

\function{mktime}
\synopsis{Convert a time-structure to seconds}
\usage{secs = mktime (Struct_Type tm)}
\description
  The \ifun{mktime} function is essentially the inverse of the
  \ifun{localtime} function.  See the documentation for that function
  for more details.
\seealso{localtime, gmtime, _time}
\done

\function{strftime}
\synopsis{Format a date and time string}
\usage{str = strftime (String_Type format [,Struct_Type tm])}
\description
  The \ifun{strftime} creates a date and time string according to a
  specified format.  If called with a single argument, the current
  local time will be used as the reference time.  If called with two
  arguments, the second argument specifies the reference time, and
  must be a structure with the same fields as the structure returned
  by the \ifun{localtime} function.

  The format string may be composed of one or more of the following
  format descriptors:
#v+
       %A      full weekday name (Monday)
       %a      abbreviated weekday name (Mon)
       %B      full month name (January)
       %b      abbreviated month name (Jan)
       %c      standard date and time representation
       %d      day-of-month (01-31)
       %H      hour (24 hour clock) (00-23)
       %I      hour (12 hour clock) (01-12)
       %j      day-of-year (001-366)
       %M      minute (00-59)
       %m      month (01-12)
       %p      local equivalent of AM or PM
       %S      second (00-59)
       %U      week-of-year, first day Sunday (00-53)
       %W      week-of-year, first day Monday (00-53)
       %w      weekday (0-6, Sunday is 0)
       %X      standard time representation
       %x      standard date representation
       %Y      year with century
       %y      year without century (00-99)
       %Z      timezone name
       %%      percent sign
#v-
 as well as any others provided by the C library.  The actual values
 represented by the format descriptors are locale-dependent.
\example
#v+
    message (strftime ("Today is %A, day %j of the year"));
    tm = localtime (0);
    message (strftime ("Unix time 0 was on a %A", tm));
#v-
\seealso{localtime, time}
\done

\function{_tic}
\synopsis{Reset the CPU timer}
\usage{_tic ()}
\description
  The \ifun{_tic} function resets the internal CPU timer.  The
 \ifun{_toc} may be used to read this timer.  See the documentation
 for the \ifun{_toc} function for more information.
\seealso{_toc, times, tic, toc}
\done

\function{tic}
\synopsis{Reset the interval timer}
\usage{void tic ()}
\description
  The \ifun{tic} function resets the internal interval timer.  The
 \ifun{toc} may be used to read the interval timer.
\example
  The tic/toc functions may be used to measure execution times.  For
 example, at the \slsh prompt, they may be used to measure the speed
 of a loop:
#v+
   slsh> tic; loop (500000); toc;
   0.06558
#v-
\notes
  On Unix, this timer makes use of the C library \cfun{gettimeofday}
  function.
\seealso{toc, times}
\done

\function{_time}
\synopsis{Get the current calendar time in seconds}
\usage{Long_Type _time ()}
\description
  The \ifun{_time} function returns the number of elapsed seconds since
  00:00:00 UTC, January 1, 1970.  A number of functions (\ifun{ctime},
  \ifun{gmtime}, \ifun{localtime}, etc.) are able to convert such a
  value to other representations.
\seealso{ctime, time, localtime, gmtime}
\done

\function{time}
\synopsis{Return the current date and time as a string}
\usage{String_Type time ()}
\description
  This function returns the current time as a string of the form:
#v+
    Sun Apr 21 13:34:17 1996
#v-
\seealso{strftime, ctime, message, substr}
\done

\function{timegm}
\synopsis{Convert a time structure for the GMT timezone to seconds}
\usage{Long_Type secs = timegm(Struct_Type tm)}
\description
  \ifun{timegm} is the inverse of the \ifun{gmtime} function.
\seealso{gmtime, mktime, localtime}
\done

\function{times}
\synopsis{Get process times}
\usage{Struct_Type times ()}
\description
  The \ifun{times} function returns a structure containing the
  following fields:
#v+
    tms_utime     (user time)
    tms_stime     (system time)
    tms_cutime    (user time of child processes)
    tms_cstime    (system time of child processes)
#v-
\notes
  Not all systems support this function.
\seealso{_tic, _toc, _time}
\done

\function{_toc}
\synopsis{Get the elapsed CPU time for the current process}
\usage{Double_Type _toc ()}
\description
  The \ifun{_toc} function returns the elapsed CPU time in seconds since
  the last call to \ifun{_tic}.  The CPU time is the amount of time the
  CPU spent running the code of the current process.
\notes
  This function may not be available on all systems.

  The implementation of this function is based upon the \ifun{times}
  system call.  The precision of the clock is system dependent and may
  not be very accurate for small time intervals.  For this reason, the
  tic/toc functions may be more useful for small time-intervals.
\seealso{_tic, _tic, _toc, times, _time}
\done

\function{toc}
\synopsis{Read the interval timer}
\usage{Double_Type toc ()}
\description
  The \ifun{toc} function returns the elapsed time in seconds since
  the last call to \ifun{tic}.  See the documentation for the
 \ifun{tic} function for more information.
\seealso{tic, _tic, _toc, times, _time}
\done

