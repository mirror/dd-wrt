-- ******************************
-- Attention!!! Adult only      *
-- 18+                          *
-- ******************************

cfg.user_age=0
cfg.youporn_max_pages=3

youporn_category=
{
    ['top_rated']='/top_rated/', ['most_viewed']='/most_viewed/', ['amateur']='/category/1/amateur/', ['anal']='/category/2/anal/',
    ['asian']='/category/3/asian/', ['bbw']='/category/4/bbw/', ['big_butt']='/category/6/big-butt/', ['big_tits']='/category/7/big-tits/',
    ['bisexual']='/category/5/bisexual/', ['blonde']='/category/51/blonde/', ['blowjob']='/category/9/blowjob/',
    ['brunette']='/category/52/brunette/', ['coed']='/category/10/coed/', ['compilation']='/category/11/compilation/',
    ['couples']='/category/12/couples/', ['creampie']='/category/13/creampie/', ['cumshots']='/category/37/cumshots/',
    ['cunnilingus']='/category/15/cunnilingus/', ['dp']='/category/16/dp/', ['ebony']='/category/8/ebony/',
    ['european']='/category/48/european/', ['facial']='/category/17/facial/', ['fantasy']='/category/42/fantasy/',
    ['fetish']='/category/18/fetish/', ['fingering']='/category/62/fingering/', ['funny']='/category/19/funny/',
    ['gay']='/category/20/gay/', ['german']='/category/58/german/', ['gonzo']='/category/50/gonzo/',
    ['group_sex']='/category/21/group-sex/', ['hairy']='/category/46/hairy/', ['handjob']='/category/22/handjob/',
    ['hentai']='/category/23/hentai/', ['instructional']='/category/24/instructional/', ['interracial']='/category/25/interracial/',
    ['interview']='/category/41/interview/', ['kissing']='/category/40/kissing/', ['latina']='/category/49/latina/',
    ['lesbian']='/category/26/lesbian/', ['milf']='/category/29/milf/', ['masturbate']='/category/55/masturbate/',
    ['mature']='/category/28/mature/', ['pov']='/category/36/pov/', ['panties']='/category/56/panties/',
    ['pantyhose']='/category/57/pantyhose/', ['public']='/category/30/public/', ['redhead']='/category/53/redhead/',
    ['rimming']='/category/43/rimming/', ['romantic']='/category/61/romantic/', ['shaved']='/category/54/shaved/',
    ['shemale']='/category/31/shemale/', ['solo_male']='/category/60/solo-male/', ['solo_girl']='/category/27/solo-girl/',
    ['squirting']='/category/39/squirting/', ['strt_sex']='/category/47/strt-sex/', ['swallow']='/category/59/swallow/',
    ['teen']='/category/32/teen/', ['threesome']='/category/38/threesome/', ['vintage']='/category/33/vintage/',
    ['voyeur']='/category/34/voyeur/', ['webcam']='/category/35/webcam/', ['3d']='/category/63/3d/'
}

function youporn_updatefeed(feed,friendly_name)
    local rc=false

    local ff=youporn_category[feed]

    if not ff then return false end

    local feed_name='youporn_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'
    local feed_url='http://www.youporn.com'..ff..'?'

    local dfd=io.open(tmp_m3u_path,'w+')

    if dfd then
        dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=youporn\n')

        local page=1

        while(page<=cfg.youporn_max_pages) do
            local url=feed_url..'&page='..page

            if cfg.debug>0 then print('YouPorn try url '..url) end

            local feed_data=http.download(url)

            if feed_data then
                local n=0

                for urn,logo,name in string.gmatch(feed_data,'.-<a href="(/watch/.-)">%s*<img src="(.-)" alt="(.-)".-') do
                    local m=string.find(urn,'?',1,true)
                    if m then urn=urn:sub(1,m-1) end

                    dfd:write('#EXTINF:0 logo=',logo,' ,',name,'\n','http://www.youporn.com',urn,'\n')
                    n=n+1
                end                 

                if n<1 then page=cfg.youporn_max_pages end

                feed_data=nil
            end

            page=page+1
        end

        dfd:close()

        if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
            if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                if cfg.debug>0 then print('YouPorn feed \''..feed_name..'\' updated') end
                rc=true
            end
        else
            util.unlink(tmp_m3u_path)
        end
    end

    return rc
end

function youporn_sendurl(youporn_url,range)

    local url=nil

    if plugin_sendurl_from_cache(youporn_url,range) then return end

    local clip_page=plugin_download(youporn_url)
    if clip_page then
        url=string.match(clip_page,'.+<a href="(http://.-)">MP4 %- For Windows 7, Mac and iPad</a>.+')
        clip_page=nil

        if url then url=string.gsub(url,'&amp;','&') end

    else
        if cfg.debug>0 then print('Clip is not found') end
    end

    if url then
        if cfg.debug>0 then print('Real URL: '..url) end

        plugin_sendurl(youporn_url,url,range)
    else
        if cfg.debug>0 then print('Real URL is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

plugins['youporn']={}
plugins.youporn.disabled=true
plugins.youporn.name="YouPorn"
plugins.youporn.sendurl=youporn_sendurl
plugins.youporn.updatefeed=youporn_updatefeed

function youporn_desc()
    local t={}
    for i,j in pairs(youporn_category) do
        t[table.maxn(t)+1]=i
    end
    return table.concat(t,',')
end

plugins.youporn.desc=youporn_desc()
if cfg.user_age<18 then plugins.youporn.disabled=true end
