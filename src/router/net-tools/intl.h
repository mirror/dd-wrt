/* Dummy header for libintl.h */

#if I18N
#include <locale.h>
#undef __OPTIMIZE__
#include <libintl.h>
#define _(String) gettext((String))
#define N_(String) (String)
#else
#define _(String) (String)
#define N_(String) (String)
#endif
