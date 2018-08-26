\chapter{Preface} #%{{{

  \slang is an interpreted language that was designed from the start
  to be easily embedded into a program to provide it with a powerful
  extension language.  Examples of programs that use \slang as an
  extension language include the \jed text editor and the \slrn
  newsreader.  Although \slang does not exist as a separate
  application, it is distributed with a quite capable program called
  \slsh (``slang-shell'') that embeds the interpreter and allows one
  to execute \slang scripts, or simply experiment with \slang at an
  interactive prompt.  Many of the the examples in this document are
  presented in the context of one of the above applications.

  \slang is also a programmer's library that permits a programmer to
  develop sophisticated platform-independent software.  In addition to
  providing the \slang interpreter, the library provides
  facilities for screen management, keymaps, low-level terminal I/O,
  etc. \if{\strcmp{\DocTitle}{\slang-library-programmers-guide}}{However, this document is concerned only with the extension
  language and does not address these other features of the \slang
  library.  For information about the other components of the library,
  the reader is referred to the \bf{\slang-library-programmers-guide.}}

\sect{A Brief History of \slang} #%{{{

  I first began working on \slang sometime during the fall of 1992.
  At that time I was writing a text editor (\jed), which I wanted to
  endow with a macro language.  It occurred to me that an
  application-independent language that could be embedded into the
  editor would prove more useful because I could envision embedding it
  into other programs.  As a result, \slang was born.

  \slang was originally a stack language that supported a
  postscript-like syntax.  For that reason, I named it \slang, where
  the \em{S} was supposed to emphasize its stack-based nature.  About
  a year later, I began to work on a preparser that would allow one
  unfamiliar with stack based languages to make use of a more
  traditional infix syntax. Currently, the syntax of the language
  resembles C, nevertheless some postscript-like features still
  remain, e.g., the `\var{%}' character is still used as a comment
  delimiter.

#%}}}

\sect{Acknowledgements} #%{{{

   Since I first released \slang, I have received a lot feedback about
   the library and the language from many people.  This has given me
   the opportunity and pleasure to interact with a number of people to
   make the library portable and easy to use.  In particular, I would
   like to thank the following individuals:

     Luchesar Ionkov for his comments and criticisms of the syntax of
     the language.  He was the person who made me realize that the
     low-level byte-code engine should be totally type-independent.
     He also improved the tokenizer and preparser and impressed upon
     me that the language needed a grammar.

     Mark Olesen for his many patches to various aspects of the
     library and his support on AIX. He also contributed a lot to the
     pre-processing (\var{SLprep}) routines.

     John Burnell for the OS/2 port of the video and keyboard
     routines.  He also made value suggestions regarding the
     interpreter interface.

     Darrel Hankerson for cleaning up and unifying some of the code
     and the makefiles.

     Dominik Wujastyk who was always willing to test new releases of
     the library.

     Michael Elkins for his work on the curses emulation.

     Hunter Goatley, Andy Harper, Martin P.J. Zinser, and Jouk Jansen
     for their VMS support.

     Dave Sims and Chin Huang for Windows 95 and Windows NT support,
     and Dino Sangoi for the Windows DLL support.

  I am also grateful to many other people who send in bug-reports,
  bug-fixes, little enhancements, and suggestions, and so on.  Without
  such community involvement, \slang would not be as well-tested and
  stable as it is.  Finally, I would like to thank my wife for her
  support and understanding while I spent long weekend hours
  developing the library.

#%}}}

#%}}}
