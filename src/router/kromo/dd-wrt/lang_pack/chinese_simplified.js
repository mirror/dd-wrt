//////////////////////////////////////////////////////////////////////////////////////////////////
//		Chinese (Simplified) reference translation file - DD-WRT V23 SP1 by JnJn 25/05/2006		//
//////////////////////////////////////////////////////////////////////////////////////////////////

// ******************************************* COMMON SHARE LABEL *******************************************//

lang_charset.set="UTF-8";

share.firmware="固件";
share.time="时间";
share.interipaddr="Internet IP地址";
share.more="更多...";
share.help="帮助";
share.enable="启用";
share.enabled="已启用";
share.disable="禁用";
share.disabled="已禁用";
share.usrname="用户名";
share.passwd="密码";
share.hostname="主机名";
share.domainname="域名";
share.wandomainname="WAN Domain Name";
share.landomainname="LAN Domain Name";
share.statu="状态";
share.start="开始";
share.end="结束";
share.proto="协议";
share.ip="IP地址";
share.mac="MAC地址";
share.none="无";
share.both="两者都有";
share.del="删除";
share.remove="移除";
share.descr="描述";
share.from="起始于";
share.to="终止于";
share.about="关于";
share.everyday="每天";
share.sun="星期天";
share.sun_s="周日";
share.sun_s1="日";
share.mon="星期一";
share.mon_s="周一";
share.mon_s1="一";
share.tue="星期二";
share.tue_s="周二";
share.tue_s1="二";
share.wed="星期三";
share.wed_s="周三";
share.wed_s1="三";
share.thu="星期四";
share.thu_s="周四";
share.thu_s1="四";
share.fri="星期五";
share.fri_s="周五";
share.fri_s1="五";
share.sat="星期六";
share.sat_s="周六";
share.sat_s1="六";
share.expires="过期";
share.yes="是";
share.no="否";
share.filter="Filter";
share.deny="拒绝";
share.range="范围";
share.use="使用";
share.mins="分";
share.secs="秒";
share.routername="路由器名称";
share.manual="手动";
share.port="端口";
share.ssid="SSID";
share.channel="频道";
share.rssi="Rssi";
share.signal="信号";
share.noise="噪声";
share.beacon="信标";
share.openn="打开";
share.dtim="dtim";
share.rates="速率";
share.low="低";
share.medium="中";
share.high="高";
share.option="选项";
share.rule="规则";
share.lan="LAN（局域网络）";
share.point2point="点对点";
share.nat="NAT（网络地址转换）";
share.subnet="子网掩码";
share.unmask="非掩模";
share.deflt="默认";  //don't use share.default !!!
share.all="全部";
share.auto="自动";
share.right="右";
share.left="左";
share.share_key="Shared Key（共享秘钥）";
share.inter="间歇（以秒为单位）";
share.srv="服务名称";
share.port_range="端口范围";
share.priority="优先级";
share.gateway="网关";
share.intrface="接口";  //don't use share.interface, Mozilla problem!!!
share.pintrface="Physical Interface";
share.vintrface="Virtual Interfaces";
share.router="路由器";
share.static_lease="静态租用";
share.srvip="服务器IP地址";
share.localdns="本地DNS（域名服务器）";
share.minutes="分钟";
share.oui="OUI搜索";
share.sttic="静态";
share.connecting="正在连接";
share.connect="连接";
share.connected="已连接";
share.disconnect="断开连接";
share.disconnected="已断开连接";
share.info="信息";
share.state="状态";
share.mode="模式";
share.encrypt="加密";
share.key="秘钥";
share.wireless="无线";
share.dhcp="DHCP";
share.styl="风格";
share.meters="meters";
share.full="Full (20 Mhz)";
share.half="Half (10 Mhz)";
share.quarter="Quarter (5 Mhz)";
share.seealso="See also";
share.never="never";
share.unknown="unknown";
share.expired="expired";
share.standard="Standard（标准）";


sbutton.save="保存设置";
sbutton.saving="已保存"; // is this saving or saved? if saving, use "正在保存"
sbutton.cmd="正在执行";
sbutton.cancel="取消改动";
sbutton.refres="刷新";
sbutton.clos="关闭";
sbutton.del="删除";
sbutton.continu="继续";
sbutton.add="添加";
sbutton.remove="移除";
sbutton.modify="修改";
sbutton.deleted="已删除";
sbutton.delall="删除全部";
sbutton.autorefresh="自动刷新状态[打开]";
sbutton.backup="备份";
sbutton.restore="恢复";
sbutton.cptotext="复制到文字区";
sbutton.runcmd="运行命令";
sbutton.startup="保存启动命令";
sbutton.firewall="保存防火墙命令";
sbutton.wol="唤醒";
sbutton.add_wol="添加主机";
sbutton.manual_wol="手动唤醒";
sbutton.summary="综述";
sbutton.filterIP="编辑PC列表";
sbutton.filterMac="编辑MAC过滤器列表";
sbutton.filterSer="添加/编辑服务";
sbutton.reboot="重新启动路由器";
sbutton.help="   帮助  ";
sbutton.wl_client_mac="无线客户端MAC列表";
sbutton.update_filter="更新过滤器列表";
sbutton.join="加入";
sbutton.log_in="连入日志";
sbutton.log_out="连出日志";
sbutton.apply="应用";
sbutton.edit_srv="添加/编辑服务";
sbutton.routingtab="显示路由表";
sbutton.wanmac="获取当前PC的MAC地址";
sbutton.dhcprel="DHCP发放";
sbutton.dhcpren="DHCP更新";
sbutton.survey="Site Survey（选址）";
sbutton.upgrading="正在升级";
sbutton.upgrade="升级";
sbutton.preview="预览";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//

errmsg.err0="您必须输入一个用户名。";
errmsg.err1="您必须输入一个路由器名称。";
errmsg.err2="超出范围，请调整起始IP地址或用户的个数。";
errmsg.err3="您必须至少选择一个日期。"
errmsg.err4="结束时间必须比开始时间大。";
errmsg.err5="MAC地址长度不正确。";
errmsg.err6="您必须输入一个密码。";
errmsg.err7="您必须输入一个主机名。";
errmsg.err8="您必须输入一个IP地址或者域名。";
errmsg.err9="非法DMZ IP地址。";
errmsg.err10="确认密码与所输入密码不相符。请重新输入密码。";
errmsg.err11="在密码中不允许使用空格";
errmsg.err12="你必须输入一个要运行的命令。";
errmsg.err13="升级失败。";
errmsg.err45="HTTPS模式下不可用！请使用HTTP模式连接。";
errmsg.err46="HTTPS模式下不可用";


//common.js error messages
errmsg.err14=" 值超出范围 [";
errmsg.err15="WAM口MAC地址超出范围 [00 - ff].";
errmsg.err16="MAC地址的第二个字符必须是偶数 : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="MAC地址不正确。";
errmsg.err18="MAC地址长度不正确。";
errmsg.err19="MAC地址不能是广播MAC地址。"
errmsg.err20="使用形如(xx:xx:xx:xx:xx:xx)的格式输入MAC地址。";
errmsg.err21="无效MAC地址格式。";
errmsg.err22="WAN口MAC地址不正确。";
errmsg.err23="无效16进制数值 ";
errmsg.err24=" 在如下MAC地址中出现 ";
errmsg.err25="秘钥值不正确。";
errmsg.err26="秘钥长度不正确。";
errmsg.err27="非法子网掩码。";
errmsg.err28=" 含有非法字符, 字符必须是 [ 0 - 9 ]。";
errmsg.err29=" 含有非法ASCII码。";
errmsg.err30=" 含有非法16进制数字。";
errmsg.err31=" 数值非法。";
errmsg.err32="IP地址和网关不在同一子网中。";
errmsg.err33="IP地址与网关不能相同。";
errmsg.err34=" 中不允许含有空格。";

//Wol.asp error messages
errmsg.err35="你必须输入一个MAC地址来执行操作。";
errmsg.err36="你必须输入一个网络广播地址来执行操作。";
errmsg.err37="你必须输入一个UDP端口来执行操作。";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="请输入一个Shared Key（共享秘钥）！";
errmsg.err39="无效秘钥，必须是长度为8到63个ASCII字符或者64个十六进制数字";
errmsg.err40="你必须为如下秘钥输入一个关键字 ";
errmsg.err41="关键字长度无效 ";
errmsg.err43="重新获取秘钥间隔";

//config.asp error messages
errmsg.err42="请选择一个用来恢复的配置文件。";

//WL_ActiveTable.asp error messages
errmsg.err44="总检查次数超过128次。";

//Site_Survey.asp error messages
errmsg.err47="无效SSID。";   //changed 20060525

//Wireless_WDS.asp error messages
errmsg.err48="WDS与当前路由器的配置不兼容。请检查以下几点 :\n * 无线模式必须被设置为AP（接入点） \n * WDS下不支持WPA2 \n * WDS下不支持无线网络使用 仅B 模式";

//Wireless_radauth.asp error messages
errmsg.err49="Radius仅在AP（接入点）模式下可用。";

//Wireless_Basic.asp error messages
errmsg.err50="你必须输入一个SSID。";

// Management.asp error messages
errmsg.err51="本路由器当前配置的密码为默认密码。 \
			出于安全的考虑，您必须远程管理特性被开启前更改密码。 \
			点击确定按钮更改您的密码。点击取消按钮保持远程管理特性关闭。";
errmsg.err52="密码复核不相符。";

// Port_Services.asp error messages
errmsg.err53="当所有操作完成之后，点击[应用]按钮保存设置。";
errmsg.err54="你必须输入一个服务名称。";
errmsg.err55="此服务名称已存在。";

// QoS.asp error messages
errmsg.err56="端口值超出范围 [0 - 65535]";

// Routing.asp error messages
errmsg.err57="删除此条目？";

// Status_Lan.asp error messages
errmsg.err58="点击删除";

//Status_Wireless.asp error messages
errmsg.err59="不可用！请启用无线网络。";

//Upgrade.asp error messages
errmsg.err60="请选择升级文件。";
errmsg.err61="不正确的镜像文件。";

// *******************************************  COMMON MENU ENTRIES  *******************************************//

bmenu.setup="设置";
bmenu.setupbasic="基本设置";
bmenu.setupddns="DDNS（动态DNS）";
bmenu.setupmacclone="MAC地址克隆";
bmenu.setuprouting="高级路由";
bmenu.setupvlan="VLANs";
bmenu.setupeop="EoIP Tunnel";
bmenu.networking="Networking";

bmenu.wireless="无线";
bmenu.wirelessBasic="基本设置";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="无线安全";
bmenu.wirelessMac="MAC过滤";
bmenu.wirelessAdvanced="高级设置";
bmenu.wirelessWds="WDS";

bmenu.security="安全";
bmenu.firwall="防火墙";
bmenu.vpn="VPN穿透";

bmenu.accrestriction="访问限制";
bmenu.webaccess="Internet访问";


bmenu.applications="应用程序 &amp; 游戏";
bmenu.applicationsprforwarding="端口段转发（映射）";
bmenu.applicationspforwarding="端口转发（映射）";
bmenu.applicationsptriggering="端口激发";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="总览";
bmenu.sipathphone="电话本";
bmenu.sipathstatus="状态";

bmenu.admin="管理";
bmenu.adminManagement="普通管理";
bmenu.adminHotspot="热点";
bmenu.adminServices="服务";
bmenu.adminAlive="保持活动";
bmenu.adminLog="日志";
bmenu.adminDiag="诊断";
bmenu.adminWol="WOL（网络唤醒）";
bmenu.adminFactory="出厂默认设置";
bmenu.adminUpgrade="固件升级";
bmenu.adminBackup="备份";


bmenu.statu="状态";
bmenu.statuRouter="路由器";
bmenu.statuInet="WAN";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Sputnik代理";
bmenu.statuWLAN="无线";
bmenu.statuVPN="OpenVPN";
bmenu.statuSysInfo="系统信息";


// ******************************************* Alive.asp *******************************************//

alive.titl="保持活动";
alive.h2="保持活动";
alive.legend="计划重新启动";
alive.sevr1="计划重新启动";
alive.hour="在设置的时间";
alive.legend2="WDS/连接看门狗";
alive.sevr2="开启看门狗";
alive.IP="IP地址";
alive.legend3="代理服务器/连接看门口";
alive.sevr3="开启代理服务器看守程序";
alive.IP2="代理服务器IP地址";
alive.port="代理服务器端口";

//help container

halive.right2="选择何时重新启动路由器。在普通管理标签中的Cron选项必须被开启。";
halive.right4="最多允许3个IP地址，之间使用<em>空格</em> 分割。<BR/>IP地址格式为：xxx.xxx.xxx.xxx。";



// ******************************************* config.asp *******************************************//

config.titl="备份与恢复";
config.h2="备份配置";
config.legend="备份设置";
config.mess1="点击 \"" + sbutton.backup + "\" 按钮将配置备份文件下载到您的个人电脑。";
config.h22="恢复配置";
config.legend2="恢复设置";
config.mess2="请选择一个用来恢复的文件";
config.mess3=" [ 警 ] [ 告 ] ";
config.mess4="请仅上传使用此（版本）固件并且从相同型号路由器上备份的文件。<br />请勿上传任何不是使用本界面创建的文件！";

//help container

hconfig.right2="您可以备份您当前配置以备您将来需要将路由器复位到出厂设置后使用。<br /><br />点击 <em>备份</em> 按钮备份您的当前设置。";
hconfig.right4="点击 <em>浏览...</em> 按钮浏览到一个当前存储在您个人电脑上的配置文件。<br /><br />点击 <em>" + sbutton.restore + "</em> 按钮使用配置文件覆盖当前配置。";



// ******************************************* DDNS.asp *******************************************//

ddns.titl="动态DNS"
ddns.h2="动态域名系统(DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS服务";
ddns.emailaddr="E-mail地址";
ddns.typ="类型";
ddns.dynamic="动态";
// ddns.static="静态"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.custom="自定义";
ddns.wildcard="统配符";
ddns.statu="DDNS状态";


ddnsm.all_closed="DDNS服务器当前处于关闭状态";
ddnsm.all_resolving="正在解析域名";
ddnsm.all_errresolv="域名解析失败";
ddnsm.all_connecting="正在连接到服务器";
ddnsm.all_connectfail="连接到服务器失败";
ddnsm.all_disabled="DDNS功能被禁用";
ddnsm.all_noip="没有Internet连接";

//help container

hddns.right2="DDNS允许您使用域名代替IP地址来访问您的网络。 \
	这个服务动态的管理IP地址的变动，更新您的域名信息。 \
	您必须通过DynDNS.org，TZO.com或ZoneEdit.com进行注册。";



// ******************************************* Diagnostics.asp *******************************************//

diag.titl="诊断";
diag.h2="诊断";
diag.legend="命令外壳";
diag.cmd="命令";
diag.startup="启动";
diag.firewall="防火墙";

//help container

hdiag.right2="您可以通过Web界面运行命令行。将您的命令填入文本区域并且点击 <em>" + sbutton.runcmd + "</em> 按钮进行提交。";



// ******************************************* DMZ.asp *******************************************//


dmz.titl="DMZ";
dmz.h2="非军事区 (DMZ)";
dmz.legend="DMZ";
dmz.serv="使用DMZ";
dmz.host="DMZ主机IP地址";


//help container

hdmz.right2="启用这个选项将使特定的主机被暴露给Internet。所有的端口都可以被从Internet访问。";



// ******************************************* Factory_Defaults.asp *******************************************//

factdef.titl="出厂默认设置";
factdef.h2="出厂默认设置";
factdef.legend="复位路由器设置";
factdef.restore="恢复出厂默认设置";

factdef.mess1="警告！如果您点击确定，此设备将会被复位到出厂默认设置，所有以往的设置都会被清除。";

//help container

hfactdef.right1="此操作将会将所有设置复位回出厂默认设置。您所做的所有设置都将被清除。";



// ******************************************* FilterIP%AC.asp *******************************************//


filterIP.titl="PC列表";
filterIP.h2="PC列表";
filterIP.h3="以如下格式输入PC的MAC地址：xx:xx:xx:xx:xx:xx";
filterIP.h32="输入PC的IP地址";
filterIP.h33="输入PC的IP范围";
filterIP.ip_range="IP范围";



// ******************************************* Filter.asp *******************************************//

filter.titl="访问限制";
filter.h2="Internet访问";
filter.legend="访问策略";
filter.pol="策略";
filter.polname="策略名称";
filter.pcs="PC";
filter.polallow="在选定的日期和时间允许Internet访问。";
filter.legend2="日期";
filter.time="时间";
filter.h24="24小时";
filter.legend3="被封禁的服务";
filter.catchall="捕获所有P2P协议";
filter.legend4="通过URL地址封锁Web站点";
filter.legend5="通过关键字封锁Web站点";

filter.mess1="删除此策略？";
filter.mess2="您必须至少选择一个日期。";
filter.mess3="结束时间必须大于起始时间。";

//help container

hfilter.right2="您可以最多定义10条访问策略。点击 <em>" + sbutton.del + "</em> 按钮删除一条策略，或者点击 <em>" + sbutton.summary + "</em> 按钮察看策略综述。";
hfilter.right4="启用或禁用一条策略。";
hfilter.right6="您可以为您的策略指定一个名称。";
hfilter.right8="请选择您希望您的策略被应用的日期。";
hfilter.right10="请输入您希望您的策略被应用的时间。";
hfilter.right12="您可以选择封禁特定的服务。点击 <em>" + sbutton.filterSer + "</em> 按钮更改这些设置。";
hfilter.right14="您可以通过输入特定Web站点的URL来封禁对其访问。";
hfilter.right16="您可以通过输入包含在特定Web站点页面中的关键字来封禁对其访问。";;



// ******************************************* FilterSummary.asp *******************************************//

filterSum.titl="访问限制综览";
filterSum.h2="Internet策略综览";
filterSum.polnum="序号";
filterSum.polday="（每天的）时间";



// ******************************************* Firewall.asp *******************************************//


firewall.titl="防火墙";
firewall.h2="安全";
firewall.legend="防火墙保护";
firewall.firewall="SPI防火墙";
firewall.legend2="附加的过滤器";
firewall.proxy="过滤代理";
firewall.cookies="过滤Cookies";
firewall.applet="过滤Java Applets（Java小应用程序）";
firewall.activex="过滤ActiveX";
firewall.legend3="封禁来自WAN口的请求";
firewall.ping="封禁来自Internet的匿名请求";
firewall.muticast="过滤多播";
filter.nat="过滤Internet NAT重定向";
filter.port113="过滤IDENT（113号端口）";

//help container

hfirewall.right2="启用或禁用SPI防火墙。";



// ******************************************* Forward.asp *******************************************//

prforward.titl="端口段转发（映射）";
prforward.h2="端口段转发（映射）";
prforward.legend="转发（映射）";
prforward.app="应用程序";

//help container

hprforward.right2="某些应用程序可能需要打开特定端口才能正常工作。 \
	举例来说，这些应用程序包括服务器软件和一些在线游戏。 \
	当从Internet接收到一个针对特定端口的请求时，路由器将会把这些数据路由到您指定的计算机。 \
	处于安全的考虑，您可能想只对您需要使用的端口设置端口转发（映射）， \
	同时，在您完成之后，取消对 <em>" + share.enable +"</em> 复选框的选定。";



// ******************************************* ForwardSpec.asp *******************************************//

pforward.titl="端口转发（映射）";
pforward.h2="端口转发（映射）";
pforward.legend="转发（映射）";
pforward.app="应用程序";
pforward.from="源端口";
pforward.to="目的端口";

//help container

hpforward.right2="某些应用程序可能需要打开特定端口才能正常工作。 \
	举例来说，这些应用程序包括服务器软件和一些在线游戏。 \
	当从Internet接收到一个针对特定端口的请求时，路由器将会把这些数据路由到您指定的计算机。 \
	处于安全的考虑，您可能想只对您需要使用的端口设置端口转发（映射）， \
	同时，在您完成之后，取消对 <em>开启</em> 复选框的选定。";



// ******************************************* Hotspot.asp *******************************************//

hotspot.titl="Hotspot";
hotspot.h2="Hotspot门户";
hotspot.legend="Chillispot";
hotspot.hotspot="Chillispot";
hotspot.nowifibridge="Separate Wifi from the LAN Bridge";
hotspot.pserver="主Radius服务器IP/DNS";
hotspot.bserver="备份RadiusIP/DNS";
hotspot.dns="DNS IP";
hotspot.url="重定向URL";
hotspot.dhcp="DHCP接口";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM秘密";
hotspot.uamdns="UAM任意DNS";
hotspot.allowuam="UAM被允许";
hotspot.macauth="MACauth";
hotspot.option="附加的Chillispot选项";
hotspot.fon_chilli="Chillispot本地用户管理";
hotspot.fon_user="用户列表";
hotspot.http_legend="HTTP重定向";
hotspot.http_srv="HTTP重定向";
hotspot.http_ip="HTTP目标IP";
hotspot.http_port="HTTP目标端口";
hotspot.http_net="HTTP源网络";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="网关名称";
hotspot.nocat_home="主页";
hotspot.nocat_allowweb="被许可的Web主机列表";
hotspot.nocat_docroot="文档根（目录）";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="排除的端口";
hotspot.nocat_timeout="登录超时";
hotspot.nocat_verbose="细致程度";
hotspot.nocat_route="仅路由";
hotspot.nocat_MAClist="MAC White List";
hotspot.smtp_legend="SMTP重定向";
hotspot.smtp_srv="SMTP重定向";
hotspot.smtp_ip="SMTP目标IP";
hotspot.smtp_net="SMTP源网络";
hotspot.shat_legend="零IP配置";
hotspot.shat_srv="零IP配置";
hotspot.shat_srv2="零IP配置已开启";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik代理";
hotspot.sputnik_id="Sputnik服务器ID";
hotspot.sputnik_instant="使用Sputnik即时设置";
hotspot.sputnik_express="使用SputnikNet Express";
hotspot.sputnik_about="关于Sputnik";



// ******************************************* Info.htm *******************************************//

info.titl="信息";
info.h2="系统信息";
info.wlanmac="无线MAC";
info.srv="服务";



// ******************************************* index_heartbeat.asp *******************************************//

idx_h.srv="心跳服务器";
idx_h.con_strgy="连接策略";
idx_h.max_idle="按需连接：最大空闲时间";
idx_h.alive="保持活动：重拨间期";



// ******************************************* index_l2tp.asp *******************************************//


idx_l.srv="L2TP服务器";



// ******************************************* index_pppoe.asp *******************************************//


idx_pppoe.use_rp="使用RP PPPoE";



// ******************************************* index_pptp.asp *******************************************//

idx_pptp.srv="使用DHCP";
idx_pptp.wan_ip="Internet IP地址";
idx_pptp.gateway="网关（PPTP服务）";
idx_pptp.encrypt="PPTP加密";



// ******************************************* index_static.asp *******************************************//

idx_static.dns="静态DNS";



// ******************************************* index.asp *******************************************//

idx.titl="设置";
idx.h2="Internet设置";
idx.h22="无线设置";
idx.legend="Internet连接类型";
idx.conn_type="连接类型";
idx.stp="STP";
idx.stp_mess="(对COMCAST ISP禁用)";
idx.optional="可选设置（某些ISP需要这些设置）";
idx.mtu="MTU";
idx.h23="网络设置";
idx.routerip="路由器IP";
idx.lanip="本地IP地址";
idx.legend2="WAN端口";
idx.wantoswitch="将WAN口指定给交换机";
idx.legend3="时间设置";
idx.timeset="时区";
idx.dsttime="夏令时 (DST)";
idx.static_ip="静态IP";
idx.dhcp="自动配置 - DHCP";
idx.dhcp_legend="网络地址服务器设置 (DHCP)";
idx.dhcp_type="DHCP类型";
idx.dhcp_srv="DHCP服务器";
idx.dhcp_fwd="DHCP转发器";
idx.dhcp_start="起始IP地址";
idx.dhcp_end="终止IP地址";		//used in Status_Lan.asp
idx.dhcp_maxusers="最大DHCP用户数";
idx.dhcp_lease="客户端租用时间";
idx.dhcp_dnsmasq="为DHCP使用DNSMasq";
idx.dns_dnsmasq="为DNS使用DNSMasq";
idx.auth_dnsmasq="以DHCP为准";
idx.summt_opt1="none";
idx.summt_opt2="first Sun Apr - last Sun Oct";
idx.summt_opt3="last Sun Mar - last Sun Oct";
idx.summt_opt4="last Sun Oct - last Sun Mar";
idx.summt_opt5="2nd Sun Mar - first Sun Nov";
idx.ntp_client="NTP客户端";



//help container

hidx.right2="这些设置最经常被有线电视操作人员使用。";
hidx.right4="请输入您的ISP提供的主机名。";
hidx.right6="请输入您的ISP提供的域名。";
hidx.right8="这是路由器的地址。";
hidx.right10="这是路由器的子网掩码。";
hidx.right12="允许路由器管理您的IP地址。";
hidx.right14="您希望起始的地址。";
hidx.right16="您可以限制您的路由器提交的地址个数。";
hidx.right18="请选择您所在的时区和夏令时（DST）期间。本路由器可以使用本地时间或者UTC时间。";



// ******************************************* Join.asp *******************************************//


//sshd.webservices
join.titl="加入";
join.mess1="已经以客户端身份成功地加入了下列网络：";



// ******************************************* Log_incoming.asp *******************************************//

log_in.titl="连入日志表";
log_in.h2="连入日志表";
log_in.th_ip="源IP";
log_in.th_port="目的端口号";



// ******************************************* Log_outgoing.asp *******************************************//

log_out.titl="连出日志表";
log_out.h2="连出日志表";
log_out.th_lanip="LAN IP";
log_out.th_wanip="目的URL/IP";
log_out.th_port="服务/端口号";



// ******************************************* Log.asp *******************************************//

log.titl="日志";
log.h2="日志管理";
log.legend="日志";
log.lvl="日志等级";
log.drop="丢弃的";
log.reject="拒绝的";
log.accept="接受的";



// ******************************************* Management.asp *******************************************//

management.titl="管理";
management.h2="路由器管理";

management.psswd_legend="路由器密码";
management.psswd_user="路由器用户名";
management.psswd_pass="路由器密码";
management.pass_conf="重新输入从而确认";

management.remote_legend="远程管理";
management.remote_gui="Web图形用户界面管理";
management.remote_https="使用HTTPS";
management.remote_guiport="Web图形用户界面端口";
management.remote_ssh="SSH管理";
management.remote_sshport="SSH端口";

management.web_legend="Web访问";
management.web_refresh="自动刷新（以秒为单位）";
management.web_sysinfo="启用信息站点";
management.web_sysinfopass="信息站点密码保护";
management.web_sysinfomasq="信息站点MAC伪装";

management.boot_legend="Boot Wait";
management.boot_srv="Boot Wait";

management.cron_legend="Cron";
management.cron_srvd="Cron";
management.cron_jobs="Additional Cron Jobs";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.rst_legend="复位按钮";
management.rst_srv="复位按钮";

management.routing_legend="路由";
management.routing_srv="路由";

management.ipv6_legend="IPv6支持";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd已开启";
management.ipv6_radconf="Radvd配置";

management.jffs_legend="JFFS2支持";
management.jffs_srv="JFFS2";
management.jffs_clean="清除JFFS2";

management.lang_legend="语言选择";
management.lang_srv="语言";
management.lang_bulgarian="bulgarian（保加利亚语）";
management.lang_chinese_traditional="tradchinese（繁体中文）"; 
management.lang_chinese_simplified="simpchinese（简体中文，Simplified Chinese）";
management.lang_croatian="croatian（克罗地亚）";
management.lang_czech="czech（捷克）";
management.lang_dutch="dutch（荷兰语）";
management.lang_portuguese_braz="brazilian（巴西语）";
management.lang_english="english（英语）";
management.lang_french="french（法语）";
management.lang_german="german（德语）";
management.lang_italian="italian（意大利语）";
management.lang_brazilian="brazilian（巴西语）";
management.lang_slovenian="slovenian（斯洛文尼亚语）";
management.lang_spanish="spanish（西班牙语）";
management.lang_swedish="swedish（瑞典语）";
management.lang_polish="polish";
management.lang_japanese="japanese";

management.net_legend="IP过滤设置（为P2P调整这些设置）";
management.net_port="最大端口数";
management.net_tcptimeout="TCP超时（以秒为单位）";
management.net_udptimeout="UDP超时（以秒为单位）";

management.clock_legend="超频";
management.clock_frq="频率";
management.clock_support="不支持";

management.mmc_legend="MMC/SD卡支持";
management.mmc_srv="MMC设备";

management.samba_legend="Samba FS自动装载";
management.samba_srv="SMB文件系统";
management.samba_share="共享";
management.samba_stscript="启动脚本";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP端口";
management.SIPatH_domain="SIP域";

management.gui_style="路由器图形用户界面风格";



//help container

hmanagement.right1="自动刷新：";
hmanagement.right2="调整Web图形用户界面自动刷新间隙。输入0完全关闭这个特性。";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

portserv.titl="端口服务";
portserv.h2="端口服务";



// ******************************************* QoS.asp *******************************************//

qos.titl="服务质量（QoS）";
qos.h2="服务质量（QoS）";
qos.legend="QoS设置";
qos.srv="开启QoS";
qos.type="数据包计划器";
qos.uplink="上行 (kbps)";
qos.dnlink="下行 (kbps)";
qos.gaming="为游戏优化";
qos.legend2="服务优先级";
qos.prio_x="Exempt（豁免）";
qos.prio_p="Premium（促进）";
qos.prio_e="Express（快速）";
qos.prio_b="Bulk（压底）";
qos.legend3="Netmask优先级";
qos.ipmask="IP/掩模";
qos.maxrate_b="最大Kbits";
qos.maxrate_o="最大数据率";
qos.legend4="MAC优先级";
qos.legend5="以太网端口优先级";
qos.legend6="默认带宽等级";
qos.bandwidth="以Kbits表示的带宽";

//help container

hqos.right1="上行：";
hqos.right2="将此设置为您的总上行限制的80%-95%（最大）。";
hqos.right3="下行：";
hqos.right4="将此设置为您的总下行限制的80%-100%（最大）。";
hqos.right6="您可以注意消耗带宽的应用程序，从而控制您的数据传输速率。";
hqos.right8="您可以为一个给定的IP地址或者IP段指定优先级。";
hqos.right10="您可以通过为一个设备指定设备名，制定优先级并输入MAC地址， \
			从而为该设备的所有流量指定优先级。";
hqos.right12="您可以根据您的设备连接到哪一个无力的LAN口来控制它的数据传输速率。 \
			您可以根据设备连接到从1到4号LAN端口从而制定优先级。";



// ******************************************* RouteTable.asp *******************************************//

routetbl.titl="路由表";
routetbl.h2="路由表项目列表";
routetbl.th1="目的LAN IP";



// ******************************************* Routing.asp *******************************************//

route.titl="路由操作";
route.h2="高级路由操作";
route.mod="工作模式";
route.bgp_legend="BGP设置";
route.bgp_own_as="BGP Own AS#";
route.bgp_ip="邻近IP";
route.bgp_as="邻近AS#";
route.rip2_mod="RIP2路由器";
route.ospf_mod="OSPF路由器";
route.gateway_legend="动态路由操作";
route.static_legend="动态路由操作";
route.static_setno="选择设置号";
route.static_name="路由名称";
route.static_ip="目的LAN IP";

//help container

hroute.right2="如果本路由器正主控您的Internet连接，选择 <em>网关</em> 模式。如果您的网络中存在另外一个路由器，选择 <em>路由</em> 模式。";
hroute.right4="这是唯一的路由号，您可以设置最多20条路由。";
hroute.right6="设置您希望指定给这条路由的名称。";
hroute.right8="这是您希望设定静态路由的远程主机。";
hroute.right10="决定主机和网络分配。";


// ******************************************* Site_Survey.asp *******************************************//


survey.titl="Site Survey（选址）";
survey.h2="邻近的无线网络";
survey.thjoin="加入站点";



// ******************************************* Services.asp *******************************************//

service.titl="服务";
service.h2="服务管理";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="开启Kaid";
service.kaid_locdevnum="Number of Local Devices";

//DHCPd
service.dhcp_legend="DHCP客户端";
service.dhcp_vendor="设置Vendorclass";
service.dhcp_reqip="Request IP";
service.dhcp_legend2="DHCP服务器";
service.dhcp_srv="DHCP Daemon";
service.dhcp_jffs2="使用JFFS2作为客户端租约数据库";
service.dhcp_domain="已使用的域";
service.dhcp_landomain="LAN域";
service.dhcp_option="附加的DHCPd选项";
service.dnsmasq_legend="DNSMasq";
service.dnsmasq_srv="DNSMasq";
service.dnsmasq_loc="本地DNS";
service.dnsmasq_opt="附加的DNSMasq选项";

//pptp.webservices
service.pptp_legend="PPTP";
service.pptp_srv="PPTP服务器";
service.pptp_client="客户端IP";
service.pptp_chap="CHAP-Secrets";

//syslog.webservices
service.syslog_legend="系统日志";
service.syslog_srv="系统日志";
service.syslog_ip="远程服务器";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//pptpd_client.webservices
service.pptpd_legend="PPTP客户端";
service.pptpd_option="PPTP客户端选项";
service.pptpd_ipdns="服务器IP或DNS名称";
service.pptpd_subnet="远程子网";
service.pptpd_subnetmask="远程子网掩码";
service.pptpd_encry="MPPE加密";
service.pptpd_mtu="MTU";
service.pptpd_mru="MRU";
service.pptpd_nat="NAT";

//rflow.webservices
service.rflow_legend="RFlow / MACupd";
service.rflow_srv1="RFlow";
service.rflow_srv2="MACupd";

//pppoe-relay.webservices
service.pppoe_legend="PPPOE中继";
service.pppoe_srv="中继";

//snmp.webservices
service.snmp_legend="SNMP";
service.snmp_srv="SNMP";
service.snmp_loc="位置";
service.snmp_contact="联系";
service.snmp_name="名称";
service.snmp_read="RO群社";
service.snmp_write="RW群社";

//openvpn.webservices
service.vpn_legend="OpenVPN客户端";
service.vpn_srv="开启OpenVPN";
service.vpn_ipname="服务器IP/名称";
service.vpn_mtu="TUN MTU设置";
service.vpn_mru="TUN MTU额外附加";
service.vpn_mss="TCP MSS";
service.vpn_compress="使用LZO压缩";
service.vpn_tunnel="隧道协议";
service.vpn_srvcert="公共服务器端证书";
service.vpn_clicert="公共客户端证书";
service.vpn_certtype="nsCertType（ns证书类型）";
service.vpn_clikey="客户端私钥";

//sshd.webservices
service.ssh_legend="Secure Shell（安全外壳）";
service.ssh_srv="SSHd";
service.ssh_password="密码登录";
service.ssh_key="授权秘钥";



// ******************************************* Sipath.asp + cgi *******************************************//

sipath.titl="SiPath总览";
sipath.phone_titl="电话本";
sipath.status_titl="状态";



// ******************************************* Status_Lan.asp *******************************************//

status_lan.titl="LAN状态";
status_lan.h2="本地网络";
status_lan.legend="LAN状态";
status_lan.h22="动态主机配置协议";
status_lan.legend2="DHCP状态";
status_lan.legend3="DHCP客户端";
status_lan.legend4="Active Clients";

//help container

hstatus_lan.right2="这是您的本地以太网络中看到的路由器的MAC地址。";
hstatus_lan.right4="这是您的本地以太网络中看到的路由器的IP地址。";
hstatus_lan.right6="当路由器使用一个子网掩码的时候，它被显示在这里。";
hstatus_lan.right8="如果您将您的路由器当作DHCP服务器使用，那将被显示在这里。";
hstatus_lan.right10="点击任意MAC地址，您将能获取该网络接口的“唯一组织识别码”（IEEE标准OUI数据库搜索）。";



// ******************************************* Status_Router.asp *******************************************//

status_router.titl="路由器状态";
status_router.h2="路由器信息";
status_router.legend="系统";
status_router.sys_model="路由器型号";
status_router.sys_firmver="固件版本";
status_router.sys_time="当前时间";
status_router.sys_up="运行时间";
status_router.sys_load="平均负载";
status_router.legend2="CPU";
status_router.cpu="CPU型号";
status_router.clock="CPU时钟";
status_router.legend3="内存";
status_router.mem_tot="所有可用";
status_router.mem_free="空闲";
status_router.mem_used="已使用";
status_router.mem_buf="缓冲";
status_router.mem_cached="被缓存的";
status_router.mem_active="活动";
status_router.mem_inactive="不活动";
status_router.legend4="网络";
status_router.net_maxports="IP过滤器最大端口数";
status_router.net_conntrack="活动的IP连接数";
status_router.notavail="Not available";

//help container

hstatus_router.right2="这是您在 <i>设置</i> 标签中所指定的路由器的名称。";
hstatus_router.right4="这是您的ISP所看到的路由器的MAC地址。";
hstatus_router.right6="这是路由器当前的固件。";
hstatus_router.right8="这是您在设置标签中设置得到时间。";
hstatus_router.right10="这个数值表示路由器\"启动\"并且运行的时间。";
hstatus_router.right12="这里给出的三个数字表示系统在过去1、5、15分钟时间内的系统负载。";

// ** Status_Internet.asp **//

status_inet.titl="WAN Status";
status_inet.h11="WAN";
status_inet.conft="配置类型";
status_inet.www_loginstatus="登录状态";
status_inet.wanuptime="Connection Uptime";
status_inet.leasetime="Remaining Lease Time";
status_inet.traff="Traffic";
status_inet.traff_tot="Total Traffic";
status_inet.traff_mon="Traffic by Month";
status_inet.traffin="Incoming";
status_inet.traffout="Outgoing";
status_inet.previous="Previous Month";
status_inet.next="Next Month";

//help container
hstatus_inet.right2="这里显示您的ISP所需要将您连接到Internet的信息。 \
				这些信息被输入在设置标签中。通过按这里的按钮，您可以 <em>连接</em> 或者 <em>断开</em> 您的连接。";
hstatus_inet.right4="This shows your router's Internet traffic.";

// ******************************************* Status_SputnikAPD.asp *******************************************//

status_sputnik.titl="Sputnik代理状态";
status_sputnik.h2="Sputnik&reg; Agent&trade;";
status_sputnik.manage="管理器";
status_sputnik.license="SCC授权编号";

//help container

hstatus_sputnik.right1="Sputnik代理状态";
hstatus_sputnik.right2="此屏幕显示Sputnik代理操作的状态。";
hstatus_sputnik.right4="本访问点所连接到的Sputnik控制中心。";
hstatus_sputnik.right6="当前代理状态。";
hstatus_sputnik.right8="您的Sputnik控制中心的授权数。";



// ******************************************* Status_Wireless.asp *******************************************//

status_wireless.titl="无线状态";
status_wireless.h2="无线";
status_wireless.legend="无线状态";
status_wireless.net="网络";
status_wireless.pptp="PPTP状态";
status_wireless.legend2="数据包信息";
status_wireless.rx="已接收的 (RX)";
status_wireless.tx="已传送的 (TX)";
status_wireless.h22="无线节点";
status_wireless.legend3="无线客户端";
status_wireless.signal_qual="信号品质";
status_wireless.wds="WDS节点";

//help container

hstatus_wireless.right2="这是您在您的本地无线网络中看到的路由器的MAC地址。";
hstatus_wireless.right4="正如您在在无线标签里所设置的，这里将显示网络所使用的无线模式（混合，仅G，仅B或者禁用）。";



// ******************************************* Status_OpenVPN.asp *******************************************//

status_openvpn.titl="OpenVPN Status";


// ******************************************* Triggering.asp *******************************************//

trforward.titl="端口激发器";
trforward.h2="端口激发器";
trforward.legend="转发（映射）";
trforward.trrange="被激发的端口范围";
trforward.fwdrange="被转发（映射）的端口范围";
trforward.app="应用程序。";

//help container

htrforward.right2="输入激发器的应用名称。";
htrforward.right4="为每一个应用程序列出被激发的端口范围。请参考Internet应用程序文档来查找所需要的端口号。";
htrforward.right6="为每一个应用程序列出被转发（映射）的端口范围。请参考Internet应用程序文档来查找所需要的端口号。";
htrforward.right8="输入被激发和被转发（映射）端口范围的起始端口号。";
htrforward.right10="输入被激发和被转发（映射）端口范围的终止端口号。";



// ******************************************* Upgrade.asp *******************************************//

upgrad.titl="固件升级";
upgrad.h2="固件管理";
upgrad.legend="固件升级";
upgrad.info1="在刷新之后，复位到";
upgrad.resetOff="不复位";
upgrad.resetOn="默认设置";
upgrad.file="请选择一个用来升级的文件";
upgrad.warning=" [警] [告] ";
upgrad.mess1="升级固件可能需要花费数分钟。<br />请不要关闭电源或者按复位按钮！";

//help container

hupgrad.right2="点击 <em>浏览...</em> 按钮选择将要上载到路由器的固件文件。<br /><br /> \
			点击 <em>升级</em> 按钮开始升级过程。升级绝对不可以被打断。";



// ******************************************* UPnP.asp *******************************************//

upnp.titl="UPnP";
upnp.h2="通用即插即用（UPnP）";
upnp.legend="转发（映射）";
upnp.legend2="UPnP配置";
upnp.serv="UPnP服务";
upnp.clear="在启动时清除端口转发（映射）";
upnp.url="发送陈述URL";
upnp.msg1="点击删除租用";
upnp.msg2="删除所有项目？";


//help container

hupnp.right2="点击垃圾桶删除单一条目。";
hupnp.right4="允许应用程序自动地设置端口转发（映射）";



// ******************************************* VPN.asp *******************************************//

vpn.titl="VPN穿透";
vpn.h2="虚拟专用网络（VPN）";
vpn.legend="VPN穿透";
vpn.ipsec="IPSec穿透";
vpn.pptp="PPTP穿透";
vpn.l2tp="L2TP穿透";

//help container

hvpn.right1="您可以选择启用IPSec，PPTP 和/或 L2TP 穿透，从而允许您的网络设备通过VPN进行通讯。";


// ******************************************* Vlan.asp *******************************************//

vlan.titl="虚拟LAN";
vlan.h2="虚拟本地网络（VLAN）";
vlan.legend="VLAN";
vlan.bridge="被指定到<br />网桥";
vlan.tagged="已标记的";
vlan.negociate="自动协商";
vlan.aggregation="<br>3号与4号端口链路聚合";
vlan.trunk="主干";


// ******************************************* WEP.asp *******************************************//

wep.defkey="默认传输秘钥";
wep.passphrase="口令";



// ******************************************* WOL.asp *******************************************//


wol.titl="WOL";
wol.h2="网络唤醒";
wol.legend="可用主机";
wol.legend2="WOL地址";
wol.legend3="输出";
wol.legend4="手工WOL";
wol.enable="启用WOL？";
wol.mac="MAC地址";
wol.broadcast="网络广播";
wol.udp="UDP端口";
wol.msg1="点击移除WOL主机";
wol.h22="Automatic Wake-On-LAN";
wol.legend5="网络唤醒";
wol.srv="WOL";
wol.pass="SecureOn密码";

//help container

hwol.right2="本页面允许您 <em>唤醒</em> 在您本地网络上的主机（例如，本地的连接到您的路由器上的）。";
hwol.right4="MAC地址使用如下格式输入 xx:xx:xx:xx:xx:xx （例如 01:23:45:67:89:AB）";
hwol.right6="IP地址通常是本地网络的广播地址，但是如果目标主机并为连接到路由器的本地网络，则应该设置为远程地址。"



// ******************************************* WanMAC.asp *******************************************//

wanmac.titl="MAC地址克隆";
wanmac.h2="MAC地址克隆";
wanmac.legend="MAC克隆";
wanmac.wan="克隆WAN口MAC";
wanmac.wlan="克隆无线MAC";

//help container

hwanmac.right2="某些ISP可能要求您注册您的MAC地址。 \
			如果您不想重新注册您的MAC地址，您可以将路由器的MAC地址克隆为您注册在您的ISP处的MAC地址。";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

wpa.titl="无线安全";
wpa.h2="无线安全";
wpa.legend="无线加密";
wpa.auth_mode="网络鉴权";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA组重新获取秘钥间隔";
wpa.rekey="秘钥更新间歇（以秒为单位）";
wpa.radius_ipaddr="RADIUS服务器地址";
wpa.radius_port="RADIUS服务器端口";
wpa.radius_key="RADIUS秘钥";
wpa.algorithms="WPA算法";
wpa.shared_key="WPA共享秘钥";
wpa.rekeyInt="重新获取秘钥间隔";

//help container

hwpa.right1="安全模式：";
hwpa.right2="您可以从禁用、WEP、WPA预共享秘钥、WPA RADIUS或者RADIUS中选取一种。在您的网络上的所有设备必须使用相同的安全模式。";



// ******************************************* WL_FilterTable.asp *******************************************//

wl_filter.titl="MAC地址过滤器列表";
wl_filter.h2="MAC地址过滤器列表";
wl_filter.h3="使用如下格式输入MAC地址&nbsp;：&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";



// ******************************************* WL_ActiveTable.asp *******************************************//

wl_active.titl="活动的无线客户端MAC列表";
wl_active.h2="无线客户端MAC列表";
wl_active.h3="启用MAC过滤器";
wl_active.active="活动的PC";
wl_active.inactive="不活动的PC";



// ******************************************* Wireless_WDS.asp *******************************************//

wds.titl="WDS";
wds.h2="无线分布系统";
wds.legend="WDS设置";
wds.label="Lazy WDS";
wds.label2="WDS子网";
wds.wl_mac="无线MAC";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="子网";
wds.legend2="额外的选项";



// ******************************************* Wireless_radauth.asp *******************************************//

radius.titl="Radius";
radius.h2="远程授权拨入用户服务（Radius）";
radius.legend="Radius";
radius.label="MAC Radius客户端";
radius.label2="MAC格式";
radius.label3="Radius服务器IP";
radius.label4="Radius服务器端口";
radius.label5="最大非授权用户数";
radius.label6="密码格式";
radius.label7="RADIUS Shared Secret";
radius.label8="如果服务器不可用则越过Radius";



// ******************************************* Wireless_MAC.asp *******************************************//

wl_mac.titl="MAC过滤器";
wl_mac.h2="无线MAC过滤器";
wl_mac.legend="MAC过滤器";
wl_mac.label="使用过滤器";
wl_mac.label2="过滤器模式";
wl_mac.deny="阻止所列PC机访问无线网络";
wl_mac.allow="只允许所列PC机访问无线网络";

// ******************************************* Wireless_Advanced.asp *******************************************//

wl_adv.titl="高级无线设置";
wl_adv.h2="高级无线设置";
wl_adv.legend="高级设置";
wl_adv.legend2="无线多媒体支持设置";
wl_adv.label="鉴权类型";
wl_adv.label2="基本速率";
wl_adv.label3="传输速率";
wl_adv.label4="CTS保护模式";
wl_adv.label5="帧突发";
wl_adv.label6="信标间隙";
wl_adv.label7="DTIM间隙";
wl_adv.label8="分片阈值";
wl_adv.label9="RTS阈值";
wl_adv.label10="最大关联的客户端数";
wl_adv.label11="AP独立";
wl_adv.label12="传送天线";
wl_adv.label13="接受天线";
wl_adv.label14="Preamble";
wl_adv.reference="噪声参照";
wl_adv.label16="Afterburner";
wl_adv.label17="无线图形用户界面访问";
wl_adv.label18="WMM支持";
wl_adv.label19="无知晓";
wl_adv.table1="EDCA AP参数（AP到客户端）";
wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="管理员强制";
wl_adv.row1="背景";
wl_adv.row2="尽力服务";
wl_adv.row3="视频";
wl_adv.row4="语音";
wl_adv.table2="EDCA STA参数（客户端到AP）";
wl_adv.lng="长"; 					//************* don't use .long ! *************
wl_adv.shrt="短"; 				//************* don't use .short ! **************

//help container

hwl_adv.right2="您可以从自动或者共享秘钥中选择一个。共享秘钥授权更加安全，但是所有在您网络中的设备必须支持共享秘钥授权。";


// ******************************************* Wireless_Basic.asp *******************************************//

wl_basic.titl="无线";
wl_basic.h2="无线";
wl_basic.legend="基本设置";
wl_basic.label="无线模式";
wl_basic.label2="无线网络模式";
wl_basic.label3="无线网络名（SSID）";
wl_basic.label4="无线频道";
wl_basic.label5="无线SSID广播";
wl_basic.label6="感受范围（ACK时序）";
wl_basic.ap="访问点（AP）";
wl_basic.client="客户端";
wl_basic.clientBridge="客户端网桥";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="混合";
wl_basic.b="仅B";
wl_basic.g="仅G";
wl_basic.legend2="Radio Time Restrictions";
wl_basic.radio="Radio";
wl_basic.radiotimer="Radio Scheduling";
wl_basic.radio_on="Radio is On";
wl_basic.radio_off="Radio is Off";

//help container

hwl_basic.right2="如果您想剔除无线-G客户端，选择 <em>仅B</em> 模式。如果您想禁用无线访问，选择 <em>禁用</em>。";
hwl_basic.right3="感受范围：";
hwl_basic.right4="调整ack时序。输入0将完全禁用ack时序。";


// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

fail.mess1="您所输入的数值是无效的。请再试一次。";
fail.mess2="升级失败。";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

success.saved="设置已保存。";
success.restore="设置已恢复。<br/>设备正在重新启动。请稍候……";
success.upgrade="升级成功。<br/>设备正在重新启动。请稍候……";
success.success_noreboot="设置成功。";
success.success_reboot=success.success_noreboot + "<br />设备正在重新启动。请稍候……";

success.alert_reset="所有配置已经被恢复到它们的默认值。<br /><br />";
success.alert1="请在再次连接前检查以下内容：";
success.alert2="如果您更改了您的路由器的IP地址，请注意您必须release并且renew您网络上客户端的地址。";
success.alert3="如果您是通过WLAN连接的，请加入网络并且点击 <em>继续</em>.";

// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

dhcp.titl="DHCP活动IP列表";
dhcp.h2="DHCP活动IP列表";
dhcp.server="DHCP服务器IP地址：";
dhcp.tclient="客户端主机名";


donate.mb="您也可以通过Moneybookers帐号mb@dd-wrt.com进行捐赠";
