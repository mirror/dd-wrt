/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

struct field;

typedef struct flist {
	char			*name;
	const struct field	*fld;
	struct flist		*child;
	struct flist		*sibling;
	int			low;
	int			high;
	int			flags;
	int			offset;
} flist_t;

/*
 * Flags for flist
 */
#define	FL_OKLOW	1
#define	FL_OKHIGH	2

typedef enum tokty {
	TT_NAME, TT_NUM, TT_STRING, TT_LB, TT_RB, TT_DASH, TT_DOT, TT_END
} tokty_t;

typedef struct ftok {
	char	*tok;
	tokty_t	tokty;
} ftok_t;

extern void	flist_free(flist_t *fl);
extern flist_t	*flist_make(char *name);
extern int	flist_parse(const struct field *fields, flist_t *fl, void *obj,
			    int startoff);
extern void	flist_print(flist_t *fl);
extern flist_t	*flist_scan(char *name);
