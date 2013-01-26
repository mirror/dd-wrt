/*** BEGIN file-header ***/

#include <glib-object.h>

G_BEGIN_DECLS
/*** END file-header ***/

/*** BEGIN file-production ***/

/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/
GType @enum_name@_get_type (void) G_GNUC_CONST;
#define @ENUMPREFIX@_TYPE_@ENUMSHORT@ (@enum_name@_get_type ())

/* Define type-specific symbols */
#define __@ENUMNAME@_IS_@TYPE@__

#if defined __@ENUMNAME@_IS_ENUM__
const gchar *@enum_name@_get_string (@EnumName@ val);
#endif

#if defined __@ENUMNAME@_IS_FLAGS__
gchar *@enum_name@_build_string_from_mask (@EnumName@ mask);
#endif

/*** END value-header ***/

/*** BEGIN file-tail ***/
G_END_DECLS

/*** END file-tail ***/
