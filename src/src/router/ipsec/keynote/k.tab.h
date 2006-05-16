#ifndef BISON_K_TAB_H
# define BISON_K_TAB_H

#ifndef YYSTYPE
typedef union {
    char   *string;
    double  doubval;
    int     intval;
    int     bool;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	NUM	257
# define	KOF	258
# define	FLOAT	259
# define	STRING	260
# define	VARIABLE	261
# define	TRUE	262
# define	FALSE	263
# define	OPENPAREN	264
# define	CLOSEPAREN	265
# define	EQQ	266
# define	COMMA	267
# define	ACTSTR	268
# define	LOCINI	269
# define	KEYPRE	270
# define	KNVERSION	271
# define	DOTT	272
# define	SIGNERKEY	273
# define	HINT	274
# define	OPENBLOCK	275
# define	CLOSEBLOCK	276
# define	SIGNATUREENTRY	277
# define	PRIVATEKEY	278
# define	SEMICOLON	279
# define	EQ	280
# define	NE	281
# define	LT	282
# define	GT	283
# define	LE	284
# define	GE	285
# define	REGEXP	286
# define	OR	287
# define	AND	288
# define	NOT	289
# define	PLUS	290
# define	MINUS	291
# define	MULT	292
# define	DIV	293
# define	MOD	294
# define	EXP	295
# define	UNARYMINUS	296
# define	DEREF	297
# define	OPENNUM	298
# define	OPENFLT	299


extern YYSTYPE knlval;

#endif /* not BISON_K_TAB_H */
