-- smorygo.philipp at gmail.com
-- Licensed under GNU GPL version 2 
-- https://www.gnu.org/licenses/gpl-2.0.html

-- Uses 'si' parameter of http://www.1tv.ru/sprojects/si=5685
-- config example
-- feeds = 
-- {
--     { "orttv", "5685", "Пусть говорят" },
-- }
function orttv_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url='http://phism.ru/1tv_proxy.php?si='..feed
    local feed_name='1tv_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_data=http.download(feed_url)

    if feed_data then
        local dfd=io.open(tmp_m3u_path,'w+')
        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=orttv\n')

            local pattern=string.format('<div class="img"><a href="/sprojects_edition/si=%s&amp;fi=[0-9]-"><img src="(.-)" width.-<h3><a href="(/sprojects_edition/si=%s&amp;fi=[0-9]-)">(.-)</a></h3>',feed,feed)
	    for logo,u,name in string.gmatch(feed_data,pattern) do
		local url=string.format('http://www.1tv.ru%s',u)
		url = url:gsub("&amp;", "&")
		url = url:gsub("si=", "si")
		url = url:gsub("&fi=", "/fi")
		dfd:write('#EXTINF:0 logo=',logo,',',name,'\n',url,'\n')
            end

            dfd:close()

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('1tv.ru feed \''..feed_name..'\' updated') end
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

function orttv_sendurl(orttv_url,range)
    local url=nil

    if plugin_sendurl_from_cache(orttv_url,range) then return end

    local clip_page=http.download(orttv_url)

    if clip_page then
	local mp4_pattern=string.format("'file': '(.+mp4)'")
        for file_name in string.gmatch(clip_page,mp4_pattern) do
		url=file_name
		break
	end
    end

    if url then
        if cfg.debug>0 then print('1tv.ru Real URL: '..url) end

        plugin_sendurl(orttv_url,url,range)
    else
        if cfg.debug>0 then print('1tv.ru clip is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

plugins['orttv']={}
plugins.orttv.name="1tv"
plugins.orttv.desc="'<b>si</b>' parameter from 1tv.ru url<br/>example: http://www.1tv.ru/sprojects/si=<b>5685</b>"
plugins.orttv.sendurl=orttv_sendurl
plugins.orttv.updatefeed=orttv_updatefeed

--orttv_updatefeed("5685")
