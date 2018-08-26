#% -*- mode: tm; mode: fold;  -*-
#%{{{Macros

#i linuxdoc.tm
#i local.tm

#d function#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d datatype#1 \sect{<bf>$1</bf>\label{$1}}<descrip>

#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d qualifiers <tag> Qualifiers </tag>
#d methods <tag> Methods </tag>
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

#%}}}

\linuxdoc
\begin{\documentstyle}

\title SLSH Library Reference (version 2.2.0)
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

\chapter{Introduction}

#d doc_root_url http://www.jedsoft.org/slang/doc
#d ifun_doc_url \doc_root_url/html/slangfun.html

This document describes the functions that are part of the \slsh
library.  These functions are written in \slang and make use of
lower-level intrinsic functions that are described in the
\href{\ifun_doc_url}{Intrinsic Function Reference Manual}.
As the \slsh library functions make no use of \slsh intrinsics, they
may be used by any conforming \slang application.

Before a particular \slsh library function may be used, the file that
defines the function must first be loaded.  The recommended mechanism
for loading a file is through the use of the
\sfun{require} function, e.g.,
#v+
   require ("structfuns");
#v-
will make the functions defined in the file \file{structfuns.sl}
available to the interpreter.  The \sfun{require} function itself is
defined in the file \file{require.sl}, which a conforming application
will automatically load.

\chapter{The \tt{require} Function}
The functions in \file{require.sl} facilitate the loading of \slang
code.
#i ../../lib/tm/require.tm

\chapter{Command Line Parsing Functions}
These functions are defined in the \file{cmdopt.sl} file.
#i ../../lib/tm/cmdopt.tm

\chapter{Filename Globbing Functions}
These functions are defined \file{glob.sl}.
#i ../../lib/tm/glob.tm

\chapter{Reading Text-formated Data Files}
These functions are defined in \file{readascii.sl}.
#i ../../lib/tm/readascii.tm

\chapter{Structure Functions}
These functions are defined in \file{structfuns.sl}.
#i ../../lib/tm/structfuns.tm

\chapter{Array Functions}
\slang includes many intrinsic functions that operate on arrays.   The
additional functions described here are defined in
\file{arrayfuns.sl}.
#i ../../lib/tm/arrayfuns.tm

\chapter{Subprocess Functions}
These functions in \file{process.sl} facilitate the creation of
subprocesses and pipelines.
#i ../../lib/tm/process.tm

\chapter{Profiling Functions}
These functions are used by \bf{slprof} for profiling \slsh
applications.
#i ../../lib/tm/profile.tm

\chapter{Set Functions}
These functions manipulate arrays and lists as sets.
#i ../../lib/tm/setfuns.tm

\chapter{Miscellaneous Functions}
#i ../../lib/tm/print.tm

\end{\documentstyle}
