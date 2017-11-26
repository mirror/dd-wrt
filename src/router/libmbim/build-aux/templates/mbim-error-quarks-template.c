/*** BEGIN file-header ***/

#include <gio/gio.h>

/*** END file-header ***/

/*** BEGIN file-production ***/
/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/

#define ERROR_PREFIX @ENUMNAME@_DBUS_PREFIX
static const GDBusErrorEntry @enum_name@_entries[] = {
/*** END value-header ***/

/*** BEGIN value-production ***/
    { @VALUENAME@, ERROR_PREFIX ".@valuenick@" },
/*** END value-production ***/

/*** BEGIN value-tail ***/
};
#undef ERROR_PREFIX

GQuark
@enum_name@_quark (void)
{
    static volatile gsize quark_volatile = 0;

    if (!quark_volatile)
        g_dbus_error_register_error_domain ("@enum_name@_quark",
                                            &quark_volatile,
                                            @enum_name@_entries,
                                            G_N_ELEMENTS (@enum_name@_entries));

    return (GQuark) quark_volatile;
}

/*** END value-tail ***/

/*** BEGIN file-tail ***/
/*** END file-tail ***/
