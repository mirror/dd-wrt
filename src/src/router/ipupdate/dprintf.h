#ifndef _DPRINTF_H
#define _DPRINTF_H

extern int options;
#ifndef OPT_DEBUG
#  define OPT_DEBUG       0x0001
#endif

#ifdef DEBUG
#define dprintf(x) if( options & OPT_DEBUG ) \
{ \
  fprintf(stderr, "%s,%d: ", __FILE__, __LINE__); \
    fprintf x; \
}
#else
#  define dprintf(x)
#endif

#endif
