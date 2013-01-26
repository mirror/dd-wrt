/*** BEGIN file-header ***/

#include <glib-object.h>

G_BEGIN_DECLS
/*** END file-header ***/

/*** BEGIN file-production ***/

/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/

#define __@ENUMNAME@_IS_@TYPE@__

#if defined __@ENUMNAME@_IS_ENUM__
#error Only flags expected, @EnumName@ is an enumeration
#endif

gchar *@enum_name@_build_string_from_mask (@EnumName@ mask);

/*** END value-header ***/

/*** BEGIN file-tail ***/
G_END_DECLS

/*** END file-tail ***/
