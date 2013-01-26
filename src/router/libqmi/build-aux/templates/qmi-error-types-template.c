/*** BEGIN file-header ***/
/*** END file-header ***/

/*** BEGIN file-production ***/
/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/
static const G@Type@Value @enum_name@_values[] = {
/*** END value-header ***/
/*** BEGIN value-production ***/
    { @VALUENAME@, "@VALUENAME@", "@valuenick@" },
/*** END value-production ***/
/*** BEGIN value-tail ***/
    { 0, NULL, NULL }
};

/* @enum_name@_quark() implemented in qmi-errors-quarks.c */

GType
@enum_name@_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        GType g_define_type_id =
            g_@type@_register_static (g_intern_static_string ("@EnumName@"),
                                      @enum_name@_values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/* Enum-specific method to get the value as a string.
 * We get the nick of the GEnumValue. Note that this will be
 * valid even if the GEnumClass is not referenced anywhere. */
const gchar *
@enum_name@_get_string (@EnumName@ val)
{
    guint i;

    for (i = 0; @enum_name@_values[i].value_nick; i++) {
        if (val == @enum_name@_values[i].value)
            return @enum_name@_values[i].value_nick;
    }

    return NULL;
}

/*** END value-tail ***/

/*** BEGIN file-tail ***/
/*** END file-tail ***/
