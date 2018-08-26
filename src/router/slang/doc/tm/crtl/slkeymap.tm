\function{SLkm_define_key}
\synopsis{Define a key in a keymap}
\usage{int SLkm_define_key (char *seq, FVOID_STAR f, SLKeyMap_List_Type *km)}
\description
  \var{SLkm_define_key} associates the key sequence \var{seq} with the
  function pointer \var{f} in the keymap specified by \var{km}.  Upon
  success, it returns zero, otherwise it returns a negative integer
  upon error.
\seealso{SLkm_define_keysym, SLang_define_key}
\done

\function{SLang_define_key}
\synopsis{Define a key in a keymap}
\usage{int SLang_define_key(char *seq, char *fun, SLKeyMap_List_Type *km)}
\description
  \var{SLang_define_key} associates the key sequence \var{seq} with
  the function whose name is \var{fun} in the keymap specified by
  \var{km}.
\seealso{SLkm_define_keysym, SLkm_define_key}
\done

\function{SLkm_define_keysym}
\synopsis{Define a keysym in a keymap}
\usage{int SLkm_define_keysym (seq, ks, km)}
#v+
      char *seq;
      unsigned int ks;
      SLKeyMap_List_Type *km;
#v-
\description
  \var{SLkm_define_keysym} associates the key sequence \var{seq} with
  the keysym \var{ks} in the keymap \var{km}.  Keysyms whose value is
  less than or equal to \exmp{0x1000} is reserved by the library and
  should not be used.
\seealso{SLkm_define_key, SLang_define_key}
\done

\function{SLang_undefine_key}
\synopsis{Undefined a key from a keymap}
\usage{void SLang_undefine_key(char *seq, SLKeyMap_List_Type *km);}
\description
  \var{SLang_undefine_key} removes the key sequence \var{seq} from the
  keymap \var{km}.
\seealso{SLang_define_key}
\done

\function{SLang_create_keymap}
\synopsis{Create a new keymap}
\usage{SLKeyMap_List_Type *SLang_create_keymap (name, km)}
#v+
     char *name;
     SLKeyMap_List_Type *km;
#v-
\description
  \var{SLang_create_keymap} creates a new keymap called \var{name} by
  copying the key definitions from the keymap \var{km}.  If \var{km}
  is \var{NULL}, the newly created keymap will be empty and it is up
  to the calling routine to initialize it via the
  \var{SLang_define_key} and \var{SLkm_define_keysym} functions.
  \var{SLang_create_keymap} returns a pointer to the new keymap, or
  \var{NULL} upon failure.
\seealso{SLang_define_key, SLkm_define_keysym}
\done

\function{SLang_do_key}
\synopsis{Read a keysequence and return its keymap entry}
\usage{SLang_Key_Type *SLang_do_key (kml, getkey)}
#v+
     SLKeyMap_List_Type *kml;
     int (*getkey)(void);
#v-
\description
  The \var{SLang_do_key} function reads characters using the function
  specified by the \var{getkey} function pointer and uses the
  key sequence to return the appropriate entry in the keymap specified
  by \var{kml}.

  \var{SLang_do_key} returns \var{NULL} if the key sequence is not
  defined by the keymap, otherwise it returns a pointer to an object
  of type \var{SLang_Key_Type}, which is defined in \exmp{slang.h} as
#v+
     #define SLANG_MAX_KEYMAP_KEY_SEQ 14
     typedef struct SLang_Key_Type
     {
       struct SLang_Key_Type *next;
       union
       {
          char *s;
	  FVOID_STAR f;
	  unsigned int keysym;
       }
       f;
       unsigned char type;	       /* type of function */
     #define SLKEY_F_INTERPRET  0x01
     #define SLKEY_F_INTRINSIC  0x02
     #define SLKEY_F_KEYSYM     0x03
       unsigned char str[SLANG_MAX_KEYMAP_KEY_SEQ + 1];/* key sequence */
     }
SLang_Key_Type;

#v-
  The \var{type} field specifies which field of the union \var{f}
  should be used.  If \var{type} is \var{SLKEY_F_INTERPRET}, then
  \var{f.s} is a string that should be passed to the interpreter for
  evaluation.  If \var{type} is \var{SLKEY_F_INTRINSIC}, then
  \var{f.f} refers to function that should be called.  Otherwise,
  \var{type} is \var{SLKEY_F_KEYSYM} and \var{f.keysym} represents the
  value of the keysym that is associated with the key sequence.
\seealso{SLkm_define_keysym, SLkm_define_key}
\done

\function{SLang_find_key_function}
\synopsis{Obtain a function pointer associated with a keymap}
\usage{FVOID_STAR SLang_find_key_function (fname, km);}
#v+
    char *fname;
    SLKeyMap_List_Type *km;
#v-
\description
  The \var{SLang_find_key_function} routine searches through the
  \var{SLKeymap_Function_Type} list of functions associated with the
  keymap \var{km} for the function with name \var{fname}.
  If a matching function is found, a pointer to the function will
  be returned, otherwise \var{SLang_find_key_function} will return
  \var{NULL}.
\seealso{SLang_create_keymap, SLang_find_keymap}
\done

\function{SLang_find_keymap}
\synopsis{Find a keymap}
\usage{SLKeyMap_List_Type *SLang_find_keymap (char *keymap_name);}
\description
  The \var{SLang_find_keymap} function searches through the list of
  keymaps looking for one whose name is \var{keymap_name}.  If a
  matching keymap is found, the function returns a pointer to the
  keymap.  It returns \var{NULL} if no such keymap exists.
\seealso{SLang_create_keymap, SLang_find_key_function}
\done

\function{SLang_process_keystring}
\synopsis{Un-escape a key-sequence}
\usage{char *SLang_process_keystring (char *kseq);}
\description
  The \var{SLang_process_keystring} function converts an escaped key
  sequence to its raw form by converting two-character combinations
  such as \var{^A} to the \em{single} character \exmp{Ctrl-A} (ASCII
  1).  In addition, if the key sequence contains constructs such as
  \exmp{^(XX)}, where \exmp{XX} represents a two-character termcap
  specifier, the termcap escape sequence will be looked up and
  substituted.

  Upon success, \var{SLang_process_keystring} returns a raw
  key-sequence whose first character represents the total length of
  the key-sequence, including the length specifier itself.  It returns
  \var{NULL} upon failure.
\example
  Consider the following examples:
#v+
     SLang_process_keystring ("^X^C");
     SLang_process_keystring ("^[[A");
#v-
  The first example will return a pointer to a buffer of three characters
  whose ASCII values are given by \exmp{\{3,24,3\}}.  Similarly, the
  second example will return a pointer to the four characters
  \exmp{\{4,27,91,65\}}.  Finally, the result of
#v+
     SLang_process_keystring ("^[^(ku)");
#v-
  will depend upon the termcap/terminfo capability \exmp{"ku"}, which
  represents the escape sequence associated with the terminal's UP
  arrow key.  For an ANSI terminal whose UP arrow produces
  \exmp{"ESC [ A"}, the result will be \exmp{5,27,27,91,65}.
\notes
  \var{SLang_process_keystring} returns a pointer to a static area
  that will be overwritten on subsequent calls.
\seealso{SLang_define_key, SLang_make_keystring}
\done

\function{SLang_make_keystring}
\synopsis{Make a printable key sequence}
\usage{char *SLang_make_keystring (unsigned char *ks);}
\description
  The \var{SLang_make_keystring} function takes a raw key sequence
  \var{ks} and converts it to a printable form by converting
  characters such as ASCII 1 (ctrl-A) to \exmp{^A}.  That is, it
  performs the opposite function of \var{SLang_process_keystring}.
\notes
  This function returns a pointer to a static area that will be
  overwritten on the next call to \var{SLang_make_keystring}.
\seealso{SLang_process_keystring}
\done

