#ifndef PROPERTIES_H
#define	PROPERTIES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>
/*
 * Potentially, include other headers on which this header depends.
 */

/*
 * Type macros.
 */
#define PROPERTIES_TYPE                  (properties_get_type ())
#define PROPERTIES(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PROPERTIES_TYPE, Properties))
#define PROPERTIES_IS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PROPERTIES_TYPE))
#define PROPERTIES_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PROPERTIES_TYPE, PropertiesClass))
#define PROPERTIES_IS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PROPERTIES_TYPE))
#define PROPERTIES_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PROPERTIES_TYPE, PropertiesClass))

#define PROPERTIES_DBUS_INTERFACE  "org.freedesktop.DBus.Properties"
    
typedef struct _Properties Properties;
typedef struct _PropertiesPrivate PropertiesPrivate;
typedef struct _PropertiesClass PropertiesClass;

struct _Properties {
    /* Parent instance structure */
    GObject parent_instance;

    /* instance members */
    PropertiesPrivate *priv;
};

struct _PropertiesClass {
    /* Parent class structure */
    GObjectClass parent_class;

    /* class members */
};

/* used by PROPERTIES_TYPE */
GType properties_get_type(void);

/*
 * Method definitions.
 */
const gchar *properties_get_dbus_type(Properties *self);
const gchar *properties_get_dbus_service_name(Properties *self);
const gchar *properties_get_dbus_object_path(Properties *self);
GVariant *properties_get(Properties *self, const gchar *interface_name, const gchar *property_name, GError **error);
void properties_set(Properties *self, const gchar *interface_name, const gchar *property_name, const GVariant *value, GError **error);
GVariant *properties_get_all(Properties *self, const gchar *interface_name, GError **error);

#ifdef	__cplusplus
}
#endif

#endif	/* PROPERTIES_H */