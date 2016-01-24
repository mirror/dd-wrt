#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#undef  VSTR_AUTOCONF_HAVE_INLINE
#define VSTR_AUTOCONF_HAVE_INLINE 1
#undef  CONF_COMPILE_INLINE
#define CONF_COMPILE_INLINE 1
#define extern /* nothing */
#define inline /* nothing */

#include "conf.h"
