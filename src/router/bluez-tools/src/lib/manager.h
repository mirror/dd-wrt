#ifndef MANAGER_H
#define	MANAGER_H

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
#define MANAGER_TYPE                  (manager_get_type ())
#define MANAGER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MANAGER_TYPE, Manager))
#define MANAGER_IS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MANAGER_TYPE))
#define MANAGER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MANAGER_TYPE, ManagerClass))
#define MANAGER_IS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MANAGER_TYPE))
#define MANAGER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MANAGER_TYPE, ManagerClass))

#define MANAGER_DBUS_PATH       "/"
#define MANAGER_DBUS_INTERFACE  "org.freedesktop.DBus.ObjectManager"
    
    typedef struct _Manager Manager;
    typedef struct _ManagerPrivate ManagerPrivate;
    typedef struct _ManagerClass ManagerClass;

    struct _Manager {
        /* Parent instance structure */
        GObject parent_instance;

        /* instance members */
        ManagerPrivate *priv;
    };

    struct _ManagerClass {
        /* Parent class structure */
        GObjectClass parent_class;

        /* class members */
    };

    /* used by MANAGER_TYPE */
    GType manager_get_type(void);

    /*
     * Constructor
     */
   Manager *manager_new();
    
    /*
     * Method definitions.
     */
    GVariant *manager_get_managed_objects(Manager *self, GError **error);
    const gchar *manager_find_adapter(Manager *self, const gchar *pattern, GError **error);
    GPtrArray *manager_get_adapters(Manager *self);
    const gchar **manager_get_devices(Manager *self, const gchar *adapter_pattern);

#ifdef	__cplusplus
}
#endif

#endif	/* MANAGER_H */

