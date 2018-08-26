#% -*- mode: tm; mode: fold;  -*-
#%{{{Macros

#i linuxdoc.tm

#i local.tm

#d function#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d datatype#1 \sect{<bf>$1</bf>\label{$1}}<descrip>

#d qualifier#2:3 <tt>$1</tt> : $2\ifarg{$3}{ (Default: <tt>$3</tt>)}<p>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d altusage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d qualifiers <tag> Qualifiers </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#% d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d seealso#1 <tag> See Also </tag> \linuxdoc_list_to_ref{$1}
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

#d user-manual \bf{A Guide to the S-Lang Language}

#%}}}

\linuxdoc
\begin{\documentstyle}

\title \slang-intrinsic-function-reference (\docversion)
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

\chapter{Preface}
This document describes the intrinsic functions that are available to
any application that embeds the \slang interpreter.  In addition,
\slsh defines a number of useful functions that are also available to
conforming \slang applications.  Those functions are described in
\href{http://www.jedsoft.org/slang/docs/}{The SLSH Library Reference}.

\chapter{Data Types}
#i rtl/datatype.tm

\chapter{Array Functions}
#i rtl/array.tm

\chapter{Associative Array Functions}
#i rtl/assoc.tm

\chapter{Functions that Operate on Strings}
#i rtl/strops.tm

\chapter{Functions that Operate on Binary Strings}
#i rtl/bstr.tm

\chapter{Functions that Manipulate Structures}
#i rtl/struct.tm

\chapter{Functions that Create and Manipulate Lists}
#i rtl/list.tm

\chapter{Informational Functions}
#i rtl/info.tm

\chapter{Mathematical Functions}
#i rtl/math.tm

\chapter{Message and Error Functions}
#i rtl/message.tm

\chapter{Time and Date Functions}
#i rtl/time.tm

\chapter{Data-Type Conversion Functions}
#i rtl/type.tm

\chapter{Stdio File I/O Functions}
#i rtl/stdio.tm

\chapter{Low-level POSIX I/O functions}
#i rtl/posio.tm

\chapter{Directory Functions}
#i rtl/dir.tm

\chapter{Functions that Parse Filenames}
#i rtl/ospath.tm

\chapter{System Call Functions}
#i rtl/posix.tm

\chapter{Eval Functions}
#i rtl/eval.tm

\chapter{Qualifier Functions}
#i rtl/qualif.tm

\chapter{Module Functions}
#i rtl/import.tm

\chapter{Debugging Functions}
#i rtl/debug.tm

\chapter{Stack Functions}
#i rtl/stack.tm

\chapter{Functions that deal with the S-Lang readline interface}
#i rtl/rline.tm

\chapter{Miscellaneous Functions}
#i rtl/misc.tm

\end{\documentstyle}
