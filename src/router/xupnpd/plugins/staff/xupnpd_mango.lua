-- Playlist example
-- #EXTM3U name="Mango" type=mp4 plugin=mango
-- #EXTINF:0,Test
-- http://XX.XX.XX.XX:YYYY/mango_Premium/mp4:oosaravelli_1280.mp4/chunklist.m3u8?wowzasessionid=????????????


function mango_updatefeed(feed,friendly_name)
    return false
end

-- send '\r\n' before data
function mango_sendurl(mango_url,range)

    local urls=mango_get_video_urls(mango_url)

    local url_base=get_url_base(mango_url)

    if urls==nil then
        if cfg.debug>0 then print('mango clip '..mango_id..' is not found') end

        plugin_sendfile('www/corrupted.mp4')
    else

        for i,url in ipairs(urls) do

            local full_url=url_base..url

            if cfg.debug>0 then print('mango send URL: '..full_url) end

            http.sendurl(full_url)
        end
    end

end

function get_url_base(url)
    local t={}
    local idx=1

    for s in string.gmatch(url,'(.-/)') do
        t[idx]=s
        idx=idx+1
    end

    return table.concat(t)
end

function mango_get_video_urls(mango_url)

    local urls={}

    local clip_m3u=plugin_download(mango_url)

    local idx=1

    if clip_m3u then
        for url in string.gmatch(clip_m3u,"(.-)\r?\n") do
            if url and url~='' and url:sub(1,1)~='#' then
                urls[idx]=url
                idx=idx+1
            end
        end
    end

    return urls

end

plugins['mango']={}
plugins.mango.name="Mango"
plugins.mango.desc=""
plugins.mango.sendurl=mango_sendurl
plugins.mango.updatefeed=mango_updatefeed
