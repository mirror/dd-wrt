#include <gio/gio.h>
#include "dbus-common.h"
#include "properties.h"

struct _PropertiesPrivate
{
    GDBusProxy *proxy;
    
    // a{sa{sv}}
    GVariant *dict_interfaces;
    
    gchar *dbus_type;
    gchar *dbus_service_name;
    gchar *dbus_object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE (Properties, properties, G_TYPE_OBJECT)

enum
{
    PROP_0,

    PROP_DBUS_TYPE,
    PROP_DBUS_SERVICE_NAME,
    PROP_DBUS_OBJECT_PATH,

    N_PROPERTIES
};

/* Keep a pointer to the properties definition */
static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/*
 * Private method definitions
 */
static void _properties_create_gdbus_proxy(Properties *self, GError **error);

/*
 * Methods
 */
static void _properties_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    Properties *self = PROPERTIES(object);
    GError *error = NULL;

    switch (property_id)
    {
//    case PROP_EXAMPLE:
//        g_free(self->priv->example);
//        self->priv->example = g_value_dup_string(value);
//        g_print("example: %s\n", self->priv->example);
//        break;

    case PROP_DBUS_TYPE:
        if(self->priv->dbus_type)
            g_free(self->priv->dbus_type);
        self->priv->dbus_type = g_value_dup_string(value);
        // _properties_set_dbus_object_path(self, g_value_dup_string(value), &error);
        _properties_create_gdbus_proxy(self, &error);
        break;
        
    case PROP_DBUS_SERVICE_NAME:
        if(self->priv->dbus_service_name)
            g_free(self->priv->dbus_service_name);
        self->priv->dbus_service_name = g_value_dup_string(value);
        // _properties_set_dbus_object_path(self, g_value_dup_string(value), &error);
        _properties_create_gdbus_proxy(self, &error);
        break;
        
    case PROP_DBUS_OBJECT_PATH:
        if(self->priv->dbus_object_path)
            g_free(self->priv->dbus_object_path);
        self->priv->dbus_object_path = g_value_dup_string(value);
        // _properties_set_dbus_object_path(self, g_value_dup_string(value), &error);
        _properties_create_gdbus_proxy(self, &error);
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }

    g_assert(error == NULL);
}

static void _properties_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    Properties *self = PROPERTIES(object);

    switch (property_id)
    {
//    case PROP_EXAMPLE:
//        g_value_set_string(value, self->priv->example);
//        break;

    case PROP_DBUS_TYPE:
        g_value_set_string(value, properties_get_dbus_type(self));
        break;
        
    case PROP_DBUS_SERVICE_NAME:
        g_value_set_string(value, properties_get_dbus_service_name(self));
        break;
        
    case PROP_DBUS_OBJECT_PATH:
        g_value_set_string(value, properties_get_dbus_object_path(self));
        break;

    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void properties_dispose (GObject *gobject)
{
    Properties *self = PROPERTIES (gobject);

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
    if(self->priv->dict_interfaces != NULL)
        g_variant_unref(self->priv->dict_interfaces);


    /* Always chain up to the parent class; there is no need to check if
     * the parent class implements the dispose() virtual function: it is
     * always guaranteed to do so
     */
    G_OBJECT_CLASS (properties_parent_class)->dispose (gobject);
}

static void properties_finalize (GObject *gobject)
{
    Properties *self = PROPERTIES(gobject);

    // g_free(self->priv->a_string);

    /* Always chain up to the parent class; as with dispose(), finalize()
     * is guaranteed to exist on the parent's class virtual function table
     */
    G_OBJECT_CLASS (properties_parent_class)->finalize (gobject);
}

static void properties_class_init (PropertiesClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    
    gobject_class->get_property = _properties_get_property;
    gobject_class->set_property = _properties_set_property;
    
    /* object DBusType [readwrite, construct only] */
    obj_properties[PROP_DBUS_TYPE] =
    g_param_spec_string ("DBusType",
                         "dbus_type",
                         "Properties D-Bus connection type",
                         NULL /* default value */,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE);
    
    /* object DBusServiceName [readwrite, construct only] */
    obj_properties[PROP_DBUS_SERVICE_NAME] =
    g_param_spec_string ("DBusServiceName",
                         "dbus_service_name",
                         "Properties D-Bus service name",
                         NULL /* default value */,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE);
    
    /* object DBusObjectPath [readwrite, construct only] */
    obj_properties[PROP_DBUS_OBJECT_PATH] =
    g_param_spec_string ("DBusObjectPath",
                         "dbus_object_path",
                         "Properties D-Bus object path",
                         NULL /* default value */,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES,
                                       obj_properties);
}

static void properties_init (Properties *self)
{
    self->priv = properties_get_instance_private (self);
    self->priv->proxy = NULL;
    self->priv->dict_interfaces = NULL;
    self->priv->dbus_type = NULL;
    self->priv->dbus_service_name = NULL;
    self->priv->dbus_object_path = NULL;
}

static void _properties_create_gdbus_proxy(Properties *self, GError **error)
{
    if(self->priv->dbus_type && self->priv->dbus_service_name && self->priv->dbus_object_path)
    {
        if(g_ascii_strcasecmp(g_ascii_strdown(self->priv->dbus_type, -1), "system") == 0)
        {
            g_assert(system_conn != NULL);
            self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, self->priv->dbus_service_name, self->priv->dbus_object_path, PROPERTIES_DBUS_INTERFACE, NULL, error);
        }
        else if(g_ascii_strcasecmp(g_ascii_strdown(self->priv->dbus_type, -1), "session") == 0)
        {
            g_assert(session_conn != NULL);
            self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, self->priv->dbus_service_name, self->priv->dbus_object_path, PROPERTIES_DBUS_INTERFACE, NULL, error);
        }
        else
            g_error("Invalid DBus connection type: %s", self->priv->dbus_type);
    }
}

const gchar *properties_get_dbus_type(Properties *self)
{
    g_assert(PROPERTIES_IS(self));
    g_assert(self->priv->proxy != NULL);
    return self->priv->dbus_type;
}

const gchar *properties_get_dbus_service_name(Properties *self)
{
    g_assert(PROPERTIES_IS(self));
    g_assert(self->priv->proxy != NULL);
    return g_dbus_proxy_get_name(self->priv->proxy);
}

const gchar *properties_get_dbus_object_path(Properties *self)
{
    g_assert(PROPERTIES_IS(self));
    g_assert(self->priv->proxy != NULL);
    return g_dbus_proxy_get_object_path(self->priv->proxy);
}

GVariant *properties_get(Properties *self, const gchar *interface_name, const gchar *property_name, GError **error)
{
    g_assert(PROPERTIES_IS(self));
    GVariant *retVal = g_dbus_proxy_call_sync(self->priv->proxy, "Get", g_variant_new("(ss)", interface_name, property_name), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
    if (retVal == NULL)
        return NULL;
    retVal = g_variant_get_child_value(retVal, 0);
    retVal = g_variant_get_variant(retVal);
    return retVal;
}

void properties_set(Properties *self, const gchar *interface_name, const gchar *property_name, const GVariant *value, GError **error)
{
    g_assert(PROPERTIES_IS(self));
    g_dbus_proxy_call_sync(self->priv->proxy, "Set", g_variant_new("(ssv)", interface_name, property_name, value), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

GVariant *properties_get_all(Properties *self, const gchar *interface_name, GError **error)
{
    g_assert(PROPERTIES_IS(self));
    GVariant *retVal = g_dbus_proxy_call_sync(self->priv->proxy, "GetAll", g_variant_new("(s)", interface_name), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
    if (retVal == NULL)
        return NULL;
    retVal = g_variant_get_child_value(retVal, 0);
    return retVal;
}