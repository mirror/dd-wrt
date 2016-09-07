-- Copyright (C) 2011-2012 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- skeleton

profiles['Skeleton']=
{
    -- comment this for enable
    ['disabled']=true,

    -- device description
    ['desc']='Skeleton for example',

    -- function which identifies device by User-Agent HTTP header (must return true or false)
    ['match']=function(user_agent)
                if string.find(user_agent,'User-Agent of Device',1,true) then
                    return true
                else
                    return false
                end
            end,

    -- any option from 'cfg' namespace, cfg.dlna_headers for example; expect description in '-- options for profiles' of xupnpd_main.lua
    ['options']=
    {
        ['dev_desc_xml']='/dev.xml',                            -- UPnP Device Description XML (/dev.xml, /wmc.xml)
        ['upnp_container']='object.container',                  -- UPnP class for containers (object.container, object.container.storageFolder)
        ['upnp_artist']=false,                                  -- send <upnp:artist> / <upnp:actor> in SOAP response
        ['upnp_feature_list']='',                               -- X_GetFeatureList response body (XML)
        ['upnp_albumart']=0,                                    -- 0: <upnp:albumArtURI>direct url</upnp:albumArtURI>, 1: <res>direct url<res>, 2: <upnp:albumArtURI>local url</upnp:
        ['dlna_headers']=true,                                  -- send TransferMode.DLNA.ORG and ContentFeatures.DLNA.ORG in HTTP response
        ['dlna_extras']=true,                                   -- DLNA extras in headers and SOAP
        ['cfg.content_disp']=false,                             -- send Content-Disposition when streaming
        ['soap_length']=true,                                   -- send Content-Length in SOAP response
        ['cfg.wdtv']=false,                                     -- WDTV Live compatible mode
        ['cfg.sec_extras']=false                                -- Samsung extras
--      ...
    },

    -- replace mime={} or join with mime_types={}
    ['replace_mime_types']=true

    -- any exist in mime={} or new file type, expect xupnpd_mime.lua
    ['mime_types']=
    {
        ['avi']    = { upnp_type.video, upnp_class.video, 'video/avi',       upnp_proto.avi,   dlna_org_extras.divx5 },
        ['asf']    = { upnp_type.video, upnp_class.video, 'video/x-ms-asf',  upnp_proto.asf,   dlna_org_extras.asf_mpeg4_sp }
--      ...
    }
}

