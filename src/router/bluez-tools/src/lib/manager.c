#include <gio/gio.h>
#include "bluez-api.h"
#include "dbus-common.h"
#include "manager.h"

struct _ManagerPrivate
{
    GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(Manager, manager, G_TYPE_OBJECT)

static void manager_dispose (GObject *gobject)
{
    Manager *self = MANAGER (gobject);

    /* In dispose(), you are supposed to free all types referenced from this
     * object which might themselves hold a reference to self. Generally,
     * the most simple solution is to unref all members on which you own a 
     * reference.
     */

    /* dispose() might be called multiple times, so we must guard against
     * calling g_object_unref() on an invalid GObject by setting the member
     * NULL; g_clear_object() does this for us, atomically.
     */
    // g_clear_object (&self->priv->an_object);
    g_clear_object (&self->priv->proxy);


    /* Always chain up to the parent class; there is no need to check if
     * the parent class implements the dispose() virtual function: it is
     * always guaranteed to do so
     */
    G_OBJECT_CLASS (manager_parent_class)->dispose (gobject);
}

static void manager_finalize (GObject *gobject)
{
    Manager *self = MANAGER(gobject);

    // g_free(self->priv->a_string);

    /* Always chain up to the parent class; as with dispose(), finalize()
     * is guaranteed to exist on the parent's class virtual function table
     */
    G_OBJECT_CLASS (manager_parent_class)->finalize (gobject);
}

static void manager_class_init(ManagerClass *klass)
{
}

static void manager_init(Manager *self)
{
    self->priv = manager_get_instance_private(self);
    GError *error = NULL;

    g_assert(system_conn != NULL);
    self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, BLUEZ_DBUS_SERVICE_NAME, MANAGER_DBUS_PATH, MANAGER_DBUS_INTERFACE, NULL, &error);

    if (self->priv->proxy == NULL)
    {
        g_critical("%s", error->message);
    }
    g_assert(error == NULL);
}

Manager *manager_new()
{
    return g_object_new(MANAGER_TYPE, NULL);
}

GVariant *manager_get_managed_objects(Manager *self, GError **error)
{
    g_assert(MANAGER_IS(self));

    GVariant *retVal = NULL;
    retVal = g_dbus_proxy_call_sync(self->priv->proxy, "GetManagedObjects", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);

    if (retVal != NULL)
        retVal = g_variant_get_child_value(retVal, 0);

    return retVal;
}

const gchar *manager_find_adapter(Manager *self, const gchar *pattern, GError **error)
{
    g_assert(MANAGER_IS(self));

    GVariant *objects = NULL;
    objects = manager_get_managed_objects(self, error);
    if (objects == NULL)
        return NULL;

    const gchar *object_path;
    GVariant *ifaces_and_properties;
    GVariantIter i;

    gchar *pattern_lowercase = NULL;
    if (pattern != NULL)
    {
        pattern_lowercase = g_ascii_strdown(pattern, -1);
    }

    g_variant_iter_init(&i, objects);
    gboolean still_looking = TRUE;
    while (still_looking && g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties))
    {
        const gchar *interface_name;
        GVariantIter ii;
        GVariant* properties;
        g_variant_iter_init(&ii, ifaces_and_properties);
        while (g_variant_iter_next(&ii, "{&s@a{sv}}", &interface_name, &properties))
        {
            gchar *interface_name_lowercase = g_ascii_strdown(interface_name, -1);
            if (strstr(interface_name_lowercase, "adapter"))
            {
                g_free(interface_name_lowercase);

                if (!pattern_lowercase)
                {
                    still_looking = FALSE;
                    break;
                }

                gchar *object_base_name_original = g_path_get_basename(object_path);
                gchar *object_base_name = g_ascii_strdown(interface_name, -1);
                g_free(object_base_name_original);

                if (strstr(object_base_name, pattern_lowercase))
                {
                    still_looking = FALSE;
                    g_free(object_base_name);
                    break;
                }

                g_free(object_base_name);

                const gchar *address_original = g_variant_get_string(g_variant_lookup_value(properties, "Address", NULL), NULL);
                gchar *address = g_ascii_strdown(address_original, -1);

                if (strstr(address, pattern_lowercase))
                {
                    still_looking = FALSE;
                    g_free(address);
                    break;
                }
                g_free(address);
            }
            else
            {
                g_free(interface_name_lowercase);
            }

            g_variant_unref(properties);
        }

        g_variant_unref(ifaces_and_properties);
    }
    g_variant_unref(objects);
    g_free(pattern_lowercase);

    if (still_looking)
    {
        return NULL;
    }
    else
    {
        return object_path;
    }
}

GPtrArray *manager_get_adapters(Manager *self)
{
    g_assert(MANAGER_IS(self));

    GVariant *objects = NULL;
    GError *error = NULL;
    objects = manager_get_managed_objects(self, &error);
    if (objects == NULL)
    {
        g_critical("%s", error->message);
        g_error_free(error);
        return NULL;
    }

    GPtrArray *adapter_array = g_ptr_array_new();

    const gchar *object_path;
    GVariant *ifaces_and_properties;
    GVariantIter i;

    g_variant_iter_init(&i, objects);
    while (g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties))
    {
        const gchar *interface_name;
        GVariant *properties;
        GVariantIter ii;
        g_variant_iter_init(&ii, ifaces_and_properties);
        while (g_variant_iter_next(&ii, "{&s@a{sv}}", &interface_name, &properties))
        {
            char* interface_name_lowercase = g_ascii_strdown(interface_name, -1);
            if (strstr(interface_name_lowercase, "adapter"))
                g_ptr_array_add(adapter_array, (gpointer) g_strdup(object_path));

            g_free(interface_name_lowercase);
            g_variant_unref(properties);
        }
        g_variant_unref(ifaces_and_properties);
    }
    g_variant_unref(objects);

    return adapter_array;
}

const gchar **manager_get_devices(Manager *self, const gchar *adapter_pattern)
{
    g_assert(MANAGER_IS(self));

    GVariant *objects = NULL;
    GError *error = NULL;
    objects = manager_get_managed_objects(self, &error);
    if (objects == NULL)
    {
        g_critical("%s", error->message);
        g_error_free(error);
        return NULL;
    }
    
    GRegex *adapter_regex = g_regex_new(adapter_pattern, 0, 0, &error);
    if (adapter_regex == NULL)
    {
        g_critical("%s", error->message);
        g_error_free(error);
    }
    
    GPtrArray *device_array = g_ptr_array_new();
    
    const gchar *object_path;
    GVariant *ifaces_and_properties;
    GVariantIter i;
    
    g_variant_iter_init(&i, objects);
    while (g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties))
    {
        const gchar *interface_name;
        GVariant *properties;
        GVariantIter ii;
        g_variant_iter_init(&ii, ifaces_and_properties);
        while (g_variant_iter_next(&ii, "{&s@a{sv}}", &interface_name, &properties))
        {
            if (g_strcmp0(interface_name, "org.bluez.Device1") == 0)
            {
                const gchar *adapter_prop = g_variant_get_string(g_variant_lookup_value(properties, "Adapter", G_VARIANT_TYPE_OBJECT_PATH), NULL);
                if(g_regex_match(adapter_regex, adapter_prop, 0, NULL))
                    g_ptr_array_add(device_array, (gpointer) g_strdup(object_path));
            }
            g_variant_unref(properties);
        }
        g_variant_unref(ifaces_and_properties);
    }
    g_variant_unref(objects);

    g_regex_unref(adapter_regex);
    
    if(device_array->len > 0)
    {
        // Top it off with a NULL pointer
        g_ptr_array_add(device_array, (gpointer) NULL);
        return (const gchar**) g_ptr_array_free(device_array, FALSE);
    }
    else
        return NULL;
}