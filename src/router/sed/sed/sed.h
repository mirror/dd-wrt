/*  GNU SED, a batch stream editor.
    Copyright (C) 1989-2022 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <https://www.gnu.org/licenses/>. */

#include <config.h>
#include "basicdefs.h"
#include "dfa.h"
#include "localeinfo.h"
#include "regex.h"
#include <stdio.h>
#include "unlocked-io.h"

#include "utils.h"

/* Struct vector is used to describe a compiled sed program. */
struct vector {
  struct sed_cmd *v;	/* a dynamically allocated array */
  size_t v_allocated;	/* ... number of slots allocated */
  size_t v_length;	/* ... number of slots in use */
};

/* This structure tracks files used by sed so that they may all be
   closed cleanly at normal program termination.  A flag is kept that tells
   if a missing newline was encountered, so that it is added on the
   next line and the two lines are not concatenated.  */
struct output {
  char *name;
  bool missing_newline;
  FILE *fp;
  struct output *link;
};

struct text_buf {
  char *text;
  size_t text_length;
};

struct regex {
  regex_t pattern;
  int flags;
  size_t sz;
  struct dfa *dfa;
  bool begline;
  bool endline;
  char re[1];
};

struct readcmd {
  char *fname;
  bool append; /* true: append (default); false: prepend (gnu extension) */
};

enum replacement_types {
  REPL_ASIS = 0,
  REPL_UPPERCASE = 1,
  REPL_LOWERCASE = 2,
  REPL_UPPERCASE_FIRST = 4,
  REPL_LOWERCASE_FIRST = 8,
  REPL_MODIFIERS = REPL_UPPERCASE_FIRST | REPL_LOWERCASE_FIRST,

  /* These are given to aid in debugging */
  REPL_UPPERCASE_UPPERCASE = REPL_UPPERCASE_FIRST | REPL_UPPERCASE,
  REPL_UPPERCASE_LOWERCASE = REPL_UPPERCASE_FIRST | REPL_LOWERCASE,
  REPL_LOWERCASE_UPPERCASE = REPL_LOWERCASE_FIRST | REPL_UPPERCASE,
  REPL_LOWERCASE_LOWERCASE = REPL_LOWERCASE_FIRST | REPL_LOWERCASE
};

enum text_types {
  TEXT_BUFFER,
  TEXT_REPLACEMENT,
  TEXT_REGEX
};

enum posixicity_types {
  POSIXLY_EXTENDED,	/* with GNU extensions */
  POSIXLY_CORRECT,	/* with POSIX-compatible GNU extensions */
  POSIXLY_BASIC		/* pedantically POSIX */
};

enum addr_state {
  RANGE_INACTIVE,	/* never been active */
  RANGE_ACTIVE,		/* between first and second address */
  RANGE_CLOSED		/* like RANGE_INACTIVE, but range has ended once */
};

enum addr_types {
  ADDR_IS_NULL,		/* null address */
  ADDR_IS_REGEX,	/* a.addr_regex is valid */
  ADDR_IS_NUM,		/* a.addr_number is valid */
  ADDR_IS_NUM_MOD,	/* a.addr_number is valid, addr_step is modulo */
  ADDR_IS_STEP,		/* address is +N (only valid for addr2) */
  ADDR_IS_STEP_MOD,	/* address is ~N (only valid for addr2) */
  ADDR_IS_LAST		/* address is $ */
};

struct addr {
  enum addr_types addr_type;
  countT addr_number;
  countT addr_step;
  struct regex *addr_regex;
};


struct replacement {
  char *prefix;
  size_t prefix_length;
  int subst_id;
  enum replacement_types repl_type;
  struct replacement *next;
};

struct subst {
  struct regex *regx;
  struct replacement *replacement;
  countT numb;		/* if >0, only substitute for match number "numb" */
  struct output *outf;	/* 'w' option given */
  unsigned global : 1;	/* 'g' option given */
  unsigned print : 2;	/* 'p' option given (before/after eval) */
  unsigned eval : 1;	/* 'e' option given */
  unsigned max_id : 4;  /* maximum backreference on the RHS */
#ifdef lint
  char* replacement_buffer;
#endif
};




struct sed_cmd {
  struct addr *a1;	/* save space: usually is NULL */
  struct addr *a2;

  /* See description the enum, above.  */
  enum addr_state range_state;

  /* Non-zero if command is to be applied to non-matches. */
  char addr_bang;

  /* The actual command character. */
  char cmd;

  /* auxiliary data for various commands */
  union {
    /* This structure is used for a, i, and c commands. */
    struct text_buf cmd_txt;

    /* This is used for the l, q and Q commands. */
    int int_arg;

    /* This is used for the {}, b, and t commands. */
    countT jump_index;

    /* This is used for the r command. */
    struct readcmd readcmd;

    /* This is used for the hairy s command. */
    struct subst *cmd_subst;

    /* This is used for the w command. */
    struct output *outf;

    /* This is used for the R command.
       (despite the struct name, it is used for both in and out files). */
    struct output *inf;

    /* This is used for the y command. */
    unsigned char *translate;
    char **translatemb;

    /* This is used for the ':' command (debug only).  */
    char* label_name;
  } x;
};


_Noreturn void bad_prog (const char *why);
size_t normalize_text (char *text, size_t len, enum text_types buftype);
struct vector *compile_string (struct vector *, char *str, size_t len);
struct vector *compile_file (struct vector *, const char *cmdfile);
void check_final_program (struct vector *);
void rewind_read_files (void);
void finish_program (struct vector *);

struct regex *compile_regex (struct buffer *b, int flags, int needed_sub);
int match_regex (struct regex *regex,
                 char *buf, size_t buflen, size_t buf_start_offset,
                 struct re_registers *regarray, int regsize);
#ifdef lint
void release_regex (struct regex *);
#endif

void
debug_print_command (const struct vector *program, const struct sed_cmd *sc);
void
debug_print_program (const struct vector *program);
void
debug_print_char (char c);

int process_files (struct vector *, char **argv);

int main (int, char **);

extern struct localeinfo localeinfo;

extern int extended_regexp_flags;

/* one-byte buffer delimiter */
extern char buffer_delimiter;

/* If set, fflush(stdout) on every line output,
   and turn off stream buffering on inputs.  */
extern bool unbuffered;

/* If set, don't write out the line unless explicitly told to. */
extern bool no_default_output;

/* If set, reset line counts on every new file. */
extern bool separate_files;

/* If set, follow symlinks when invoked with -i option */
extern bool follow_symlinks;

/* Do we need to be pedantically POSIX compliant? */
extern enum posixicity_types posixicity;

/* How long should the `l' command's output line be? */
extern countT lcmd_out_line_len;

/* How do we edit files in-place? (we don't if NULL) */
extern char *in_place_extension;

/* The mode to use to read and write files, either "rt"/"w" or "rb"/"wb".  */
extern char const *read_mode;
extern char const *write_mode;

/* Should we use EREs? */
extern bool use_extended_syntax_p;

/* Declarations for multibyte character sets.  */
extern int mb_cur_max;
extern bool is_utf8;

/* If set, operate in 'sandbox' mode - disable e/r/w commands */
extern bool sandbox;

/* If set, print debugging information.  */
extern bool debug;

#define MBRTOWC(pwc, s, n, ps) \
  (mb_cur_max == 1 ? \
   (*(pwc) = btowc (*(unsigned char *) (s)), 1) : \
   mbrtowc ((pwc), (s), (n), (ps)))

#define WCRTOMB(s, wc, ps) \
  (mb_cur_max == 1 ? \
   (*(s) = wctob ((wint_t) (wc)), 1) : \
   wcrtomb ((s), (wc), (ps)))

#define MBSINIT(s) \
  (mb_cur_max == 1 ? 1 : mbsinit ((s)))

#define MBRLEN(s, n, ps) \
  (mb_cur_max == 1 ? 1 : mbrtowc (NULL, s, n, ps))

#define IS_MB_CHAR(ch, ps)                \
  (mb_cur_max == 1 ? 0 : is_mb_char (ch, ps))

extern int is_mb_char (int ch, mbstate_t *ps);
extern void initialize_mbcs (void);

/* Use this to suppress gcc's '...may be used before initialized' warnings. */
#ifdef lint
# define IF_LINT(Code) Code
#else
# define IF_LINT(Code) /* empty */
#endif

#ifndef FALLTHROUGH
# if __GNUC__ < 7
#  define FALLTHROUGH ((void) 0)
# else
#  define FALLTHROUGH __attribute__ ((__fallthrough__))
# endif
#endif
