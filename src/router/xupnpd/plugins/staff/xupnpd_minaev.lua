-- Copyright (C) 2011-2012 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- feed: archive
function minaev_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url='http://www.minaevlive.ru/'..feed..'/'
    local feed_name='minaev_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_data=http.download(feed_url)

    if feed_data then
        local dfd=io.open(tmp_m3u_path,'w+')
        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=minaev\n')

            local pattern=string.format('<h2>%%s*<a href="(/%s/%%w+/)">(.-)</a>%%s*</h2>',feed)

            for u,name in string.gmatch(feed_data,pattern) do
                local url=string.format('http://www.minaevlive.ru%s',u)
                dfd:write('#EXTINF:0,',name,'\n',url,'\n')
            end

            dfd:close()

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('MinaevLive feed \''..feed_name..'\' updated') end
                    rc=true
                end
            else
                util.unlink(tmp_m3u_path)
            end
        end

        feed_data=nil
    end

    return rc
end

function minaev_sendurl(minaev_url,range)
    local url=nil

    if plugin_sendurl_from_cache(minaev_url,range) then return end

    local clip_page=http.download(minaev_url)

    if clip_page then
--http://media.russia.ru/minaevlive/120/sd.mp4
--http://media.russia.ru/minaevlive/120/hd720p.mp4
        local u=string.match(clip_page,'.+<source src="(http://media.russia.ru/minaevlive/%w+/)%w+.mp4.+"')

        if u then
            url=u..'sd.mp4'
        end
    end

    if url then
        if cfg.debug>0 then print('MinaevLive Real URL: '..url) end

        plugin_sendurl(minaev_url,url,range)
    else
        if cfg.debug>0 then print('MinaevLive clip is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

plugins['minaev']={}
plugins.minaev.name="MinaevLive"
plugins.minaev.desc="archive"
plugins.minaev.sendurl=minaev_sendurl
plugins.minaev.updatefeed=minaev_updatefeed

--minaev_updatefeed('archive')
--minaev_sendurl('http://www.minaevlive.ru/archive/link5bd88f9b/','')
