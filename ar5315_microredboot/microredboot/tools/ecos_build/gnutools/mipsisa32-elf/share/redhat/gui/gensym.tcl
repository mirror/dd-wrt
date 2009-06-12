# gensym.tcl - Generate new symbols.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Internal counter used to provide new symbol names.
defvar GENSYM_counter 0

# Return a new "symbol".  This proc hopes that nobody else decides to
# use its prefix.
proc gensym {} {
  global GENSYM_counter
  return __gensym_symbol_[incr GENSYM_counter]
}
