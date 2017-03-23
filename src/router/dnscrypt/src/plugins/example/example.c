
#include <dnscrypt/plugin.h>

DCPLUGIN_MAIN(__FILE__);

const char *
dcplugin_description(DCPlugin * const dcplugin)
{
    return "A sample (useless) plugin for dnscrypt-proxy";
}

const char *
dcplugin_long_description(DCPlugin * const dcplugin)
{
    return
        "This is a sample plugin, that actually doesn't do anything.\n"
        "Really. That's the beauty of it.";
}

int
dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
{
    return 0;
}

int
dcplugin_destroy(DCPlugin * const dcplugin)
{
    return 0;
}

DCPluginSyncFilterResult
dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    return DCP_SYNC_FILTER_RESULT_OK;
}

DCPluginSyncFilterResult
dcplugin_sync_post_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    return DCP_SYNC_FILTER_RESULT_OK;
}
