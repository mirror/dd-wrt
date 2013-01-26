/*** BEGIN file-header ***/

typedef struct {
  guint64 value;
  const gchar *value_name;
  const gchar *value_nick;
} GFlags64Value;

/*** END file-header ***/

/*** BEGIN file-production ***/
/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/
static const GFlags64Value @enum_name@_values[] = {
/*** END value-header ***/
/*** BEGIN value-production ***/
    { @VALUENAME@, "@VALUENAME@", "@valuenick@" },
/*** END value-production ***/
/*** BEGIN value-tail ***/
    { 0, NULL, NULL }
};

/**
 * @enum_name@_build_string_from_mask:
 * @mask: bitmask of @EnumName@ values.
 *
 * Builds a string containing a comma-separated list of nicknames for
 * each #@EnumName@ in @mask.
 *
 * Returns: (transfer full): a string with the list of nicknames, or %NULL if none given. The returned value should be freed with g_free().
 */
gchar *
@enum_name@_build_string_from_mask (@EnumName@ mask)
{
    guint i;
    gboolean first = TRUE;
    GString *str = NULL;

    for (i = 0; @enum_name@_values[i].value_nick; i++) {
        /* We also look for exact matches */
        if (mask == @enum_name@_values[i].value) {
            if (str)
                g_string_free (str, TRUE);
            return g_strdup (@enum_name@_values[i].value_nick);
        }

        /* Build list with single-bit masks */
        if (mask & @enum_name@_values[i].value) {
            guint c;
            gulong number = @enum_name@_values[i].value;

            for (c = 0; number; c++)
                number &= number - 1;

            if (c == 1) {
                if (!str)
                    str = g_string_new ("");
                g_string_append_printf (str, "%s%s",
                                        first ? "" : ", ",
                                        @enum_name@_values[i].value_nick);
                if (first)
                    first = FALSE;
            }
        }
    }

    return (str ? g_string_free (str, FALSE) : NULL);
}

/*** END value-tail ***/

/*** BEGIN file-tail ***/
/*** END file-tail ***/
