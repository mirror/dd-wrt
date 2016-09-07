-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- skeleton

cfg.skel_option1=123
cfg.skel_option2='test'

function skel_updatefeed(feed,friendly_name)
    return true
end

function skel_sendurl(url,range)
end

plugins['skel']={}
plugins.skel.disabled=true      -- comment for enable
plugins.skel.name="Skeleton"
plugins.skel.desc="Skeleton description for UI"
plugins.skel.sendurl=skel_sendurl
plugins.skel.updatefeed=skel_updatefeed

plugins.skel.ui_config_vars=
{
    { "input",  "skel_option1", "int" },
    { "select", "skel_option2" }
}

plugins.skel.ui_actons=
{
    ['skel_ui']      = { 'xupnpd - skeleton ui action', skel_ui_action }        -- 'http://127.0.0.1:4044/ui/skel_ui' for call
}

-- now you can use ${skel_option1} and ${skel_option3} in UI HTML tamplates
plugins.skel.ui_vars=
{
    { "skel_option1", cfg.skel_option1 },
    { "skel_option3", function() return 'value' end }
}
