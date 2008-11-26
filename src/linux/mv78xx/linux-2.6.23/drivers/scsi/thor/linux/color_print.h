#ifndef __COLOR_PRINT_H__
#define __COLOR_PRINT_H__
/*
 *  add fun to your (debug) output
 *  A.C.
 *  Sep 26th, 2006
 *
 *  color number verified in rxvt
 */


#define	   FG_BLACK     30
#define	   FG_RED       31
#define	   FG_GREEN     32
#define	   FG_YELLOW    33
#define	   FG_BLUE      34
#define	   FG_MAGENTA   35
#define	   FG_CYAN      36
#define	   FG_WHITE     37

#define	   BG_BLACK     40
#define	   BG_RED       41
#define	   BG_GREEN     42
#define	   BG_YELLOW    43
#define	   BG_BLUE      44
#define	   BG_MAGENTA   45
#define	   BG_CYAN      46
#define	   BG_WHITE     47

#define __TERM_COLOR(fg, bg) "\e["#fg";"#bg"m"

#define TERM_COLOR(f,b) __TERM_COLOR(f,b)
 
/* this varies , how do I get to know the default setting? */
/* OK, default to set all attr off */
#define DEFAULT_THEME "\e[0m"

#ifdef __COLOR_DEBUG__
#define __SET_THEME(fg, bg, x) TERM_COLOR(FG_##fg,BG_##bg) x DEFAULT_THEME
#else  /*  __COLOR_DEBUG__ */
#define __SET_THEME(fg, bg, x) x
#endif /*  __COLOR_DEBUG__ */

/* short for X_ON_BLACK */
#define RED(x)      __SET_THEME(RED, BLACK, x)
#define GREEN(x)    __SET_THEME(GREEN, BLACK, x)
#define BLUE(x)     __SET_THEME(BLUE, BLACK, x)
#define MAGENTA(x)  __SET_THEME(MAGENTA, BLACK, x)
#define CYAN(x)     __SET_THEME(CYAN, BLACK, x)
#define WHITE(x)    __SET_THEME(WHITE, BLACK, x)
#define YELLOW(x)   __SET_THEME(YELLOW, BLACK, x)

#endif /* __COLOR_PRINT_H__ */
