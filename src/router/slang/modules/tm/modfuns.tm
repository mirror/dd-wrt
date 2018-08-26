#% -*- mode: tm; mode: fold;  -*-
#%{{{Macros

#i linuxdoc.tm

#d function#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d datatype#1 \sect{<bf>$1</bf>\label{$1}}<descrip>

#d qualifier#2:3 <tt>$1</tt> : $2\ifarg{$3}{ (Default: <tt>$3</tt>)}<p>
#d method#2 <tt>$1</tt> : $2<p>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d methods <tag> Methods </tag>
#d description <tag> Description </tag>
#d qualifiers <tag> Qualifiers </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d r#1 \ref{$1}{$1}
#d done </descrip><p>
#d -1 <tt>-1</tt>
#d 0 <tt>0</tt>
#d 1 <tt>1</tt>
#d 2 <tt>2</tt>
#d 3 <tt>3</tt>
#d 4 <tt>4</tt>
#d 5 <tt>5</tt>
#d 6 <tt>6</tt>
#d 7 <tt>7</tt>
#d 8 <tt>8</tt>
#d 9 <tt>9</tt>
#d NULL <tt>NULL</tt>
#d documentstyle book

#d slang \bf{S-Lang}

#d exc#1 \tt{$1}
#d kw#1 \tt{$1}
#d exmp#1 \tt{$1}
#d var#1 \tt{$1}
#d ctype#1 \tt{$1}
#d cfun#1 \tt{$1}
#d ivar#1 \tt{$1}
#d ifun#1 \tt{$1}
#d icon#1 \tt{$1}
#d sfun#1 \tt{$1}
#d exfile#1 \tt{$1}
#d exns#1 \tt{$1}
#d exstr#1 \tt{"$1"}
#d module#1 \tt{$1}
#d dtype#1 \tt{$1}
#d file#1 \tt{$1}

#%}}}

\linuxdoc
\begin{\documentstyle}

\title S-Lang Module Reference (version 2.2)
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

\chapter{Introduction}

#d doc_root_url http://www.jedsoft.org/slang/doc
#d ifun_doc_url \doc_root_url/html/slangfun.html
#d moduleurl http://www.jedsoft.org/slang/modules/

This document describes the functions that are available via the
modules that are distributed with the \slang library.  A more complete
list of modules may be found at \href{\moduleurl}{\moduleurl}.

To utilize the functions in a model, the module must be loaded into
the interpreter.  This is most easily accomplished via the
\sfun{require} function.  For example, the \module{png} module may be
loaded using:
#v+
   require ("png");
#v-
Sometimes it is desireable to load the module's functions into a
separate namespace to avoid collisions, e.g.,
#v+
   require ("png", "PNG");
#v-
Then the \exmp{png_read} function may be called using
\exmp{PNG->png_read}.

\chapter{Perl Compatible Regular Expression Module}
This module provides an interface to the PCRE library.  It may be
loaded using \exmp{require("pcre")}.
#i pcrefuns.tm

\chapter{Oniguruma Regular Expression Module}
This module provides an interface to the Oniguruma regular expression
library.  Use \exmp{require("onig")} to load it.
#i onigfuns.tm

\chapter{Random Number Module}
The \module{rand} module provides a number of random number functions.
It may be loaded using \exmp{require("rand")}.
#i randfuns.tm

\chapter{PNG Module}
The \module{png} module includes a number of functions for dealing
with PNG images and colormaps.  Use \exmp{require("png")} to load it.
#i pngfuns.tm

\chapter{Fork Module}
The \module{fork} module contains several low-level functions that may
be used for subprocess creation.  Use \exmp{require("fork")} to load it.
#i forkfuns.tm

\chapter{Socket Module}
This module provides an interface to the POSIX socket functions.  Use
\exmp{require("socket")} to load it.
#i sockfuns.tm

\chapter{CSV Module}
This module allows a \slang script to read and write
comma-separated-value, tab-delimited files, etc.  Use
\exmp{require("csv")} to load it.
#i csvfuns.tm

\end{\documentstyle}
