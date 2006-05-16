#ifndef BISON_Z_TAB_H
# define BISON_Z_TAB_H

#ifndef YYSTYPE
typedef union {
	struct s {
		char   *string;
	} s;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	STRING	257
# define	VSTRING	258
# define	EQ	259


extern YYSTYPE kvlval;

#endif /* not BISON_Z_TAB_H */
