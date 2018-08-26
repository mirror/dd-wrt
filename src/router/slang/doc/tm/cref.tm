#i linuxdoc.tm

#i local.tm

#d chapter#1 <chapt>$1<p>
#d preface <preface>
#d tag#1 <tag>$1</tag>

#d function#1 \sect1{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect1{<bf>$1</bf>\label{$1}}<descrip>
#cd function#1 <p><bf>$1</bf>\label{$1}<p><descrip>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d documentstyle article
#d done </descrip><p>

\linuxdoc
\begin{\documentstyle}

\title The S-Lang C Library Reference
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

\sect{Functions dealing with UTF-8 encoded strings}
#i crtl/slutf8.tm

\sect{Character classification functions}
#i crtl/slwchar.tm

\sect{SLsearch interface Functions}
#i crtl/slsearch.tm

\sect{Screen Management (SLsmg) functions}
#i crtl/slsmg.tm

\sect{Functions that deal with the interpreter}
#i crtl/slinterp.tm

\sect{Library Initialization Functions}
#i crtl/slinit.tm

\sect{Miscellaneous Functions}
#i crtl/slmisc.tm

\sect{Error and Messaging Functions}
#i crtl/slerr.tm

\sect{String and Memory Allocation Functions}
#i crtl/slalloc.tm

\sect{Keyboard Input Functions}
#i crtl/sltty.tm

\sect{Keymap Functions}
#i crtl/slkeymap.tm

\sect{Undocumented Functions}
The following functions are not yet documented:
#i crtl/undoc.tm

\end{\documentstyle}
