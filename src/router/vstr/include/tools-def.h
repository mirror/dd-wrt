#ifndef TOOLS_DEF_H
#define TOOLS_DEF_H

#if defined(HAVE_CHAR_BITFLAG) /* && defined (USE_CHAR_BITFLAG) */
typedef unsigned char bitflag;
#else
/* ANSI says that only int is a supported bitflag type...
 * Might be quicker doing this anyway,
 * although it might take up to much more room */
typedef unsigned int bitflag;
#endif

#endif
