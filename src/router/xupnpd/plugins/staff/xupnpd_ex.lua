-- Copyright (C) 2012 
--- Uses PARAMETER of [url]http://www.ex.ua/view/PARAMETER?r=23775[/url] on [url]http://www.ex.ua/ru/video[/url]
-- <guid isPermaLink="false">PARAMETER</guid> on [url]http://www.ex.ua/rss/23775[/url]
--
-- 23775   - ВИДЕО (раздел)
-- 70538   - НАШЕ  (подраздел)
-- v=1,0   - показывать списком, новое вначале
-- p=1     - страница 2
-- per=100 - 100 позиций на странице
-- [url]http://www.ex.ua/view/70538?r=23775&v=1,0&per=100&p=1[/url]
--
-- config example
-- feeds = 
-- {
--     { "ex", "2", "Зарубежное" },
--     { "ex", "70538", "Наше" },
-- }
-- pages num from 0
cfg.ex_max_pages=10
cfg.debug=1

function ex_updatefeed(feed,friendly_name)
local rc=false
local feed_url='http://www.ex.ua/view/'..feed..'?r=23775&v=1,0&per=100'
local feed_name='ex_'..string.gsub(feed,'/','_')
local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'
local page=0
local scroll=true
if feed_url then
  local dfd=io.open(tmp_m3u_path,'w+')
    if dfd then
      dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" plugin=ex\n')
     --dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" plugin=generic dlna_extras=mp4_avc_sd_ac3\n')
     --dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=ex\n')
local page=0
    while(page<cfg.ex_max_pages) do
    local url=feed_url..'&p='..page
    if cfg.debug>0 then print('EX.UA try url '..url) end
    local feed_data
    feed_data=http.download(url)
    if feed_data then
      for urn1,urn2,logo,name in string.gmatch(feed_data,'<tr><td><a href=\'/view/(.-)\?r=(.-)\'><img src=\'(.-)\' width=.-alt=\'(.-)\'>') do
          local chapter=1
          local url_spec='http://www.ex.ua/playlist/'..urn1..'.m3u'
          local feed_data_spec=http.download(url_spec)
          if feed_data_spec then
            for urs in string.gmatch(feed_data_spec,'http://www.ex.ua/get/(%d*)') do
          --dfd:write('#EXTINF:0 logo=',logo,' ,',name,' (chapter-',chapter,')','\n','http://www.ex.ua/get/',urs,'\n')
            dfd:write('#EXTINF:0 ,',name,' (chapter-',chapter,')','\n','http://www.ex.ua/get/',urs,'\n')
              if cfg.debug>0 then print('EX.UA find urs '..urs..' name '..name) end
           chapter=chapter+1
                  end
          end
          feed_data_spec=nil                                                                                                                                                                                                                                                                                                             end
          feed_data=nil
       end
          page=page+1
      end
dfd:close()
  if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
     if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
       if cfg.debug>0 then print('EX.UA feed \''..feed_name..'\' updated') end
         rc=true
       end
       else
       util.unlink(tmp_m3u_path)
       end
     end
  end
return rc
end

function ex_sendurl(ex_url,range)
local rc,location
location=ex_url
  for i=1,5,1 do
    rc,location=http.sendurl(location,1,range)
    if not location then
      break
    else
    if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
    end
  end
end

plugins['ex']={}
plugins.ex.sendurl=ex_sendurl
plugins.ex.updatefeed=ex_updatefeed
