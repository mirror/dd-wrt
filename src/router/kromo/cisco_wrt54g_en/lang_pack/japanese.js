// Not working in IE & Opera ?
//************ Include the current language file after english.js ************//
//  var Head = document.getElementsByTagName('head')[0];
//  var head_inc = document.createElement('script');
//  head_inc.setAttribute('type','text/javascript');
//  head_inc.setAttribute('src',"lang_pack/language.js");
//  Head.appendChild(head_inc);

//////////////////////////////////////////////////////////////////////////////////////////////
//      English reference translation file - DD-WRT V23 SP1 by Botho 17/05/2006             //
//////////////////////////////////////////////////////////////////////////////////////////////


// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="EUC-JP";

var share = new Object();
share.firmware="ファームウェア";
share.time="時間";
share.interipaddr="インターネットIPアドレス";
share.more="もっと...";
share.help="ヘルプ";
share.enable="有効にする";
share.enabled="有効";
share.disable="無効にする";
share.disabled="無効";
share.usrname="ユーザー名";
share.passwd="パスワード";
share.hostname="ホスト名";
share.domainname="ドメイン名";
share.wandomainname="WANドメイン名";
share.landomainname="LANドメイン名";
share.statu="ステータス";
share.start="スタート";
share.end="エンド";
share.proto="プロトコル";
share.ip="IPアドレス";
share.mac="MACアドレス";
share.none="なし";
share.none2="no";
share.both="両方";
share.del="削除";
share.remove="除去";
share.descr="説明";
share.from="From";
share.to="To";
share.about="About";
share.everyday="毎日";
share.sun="日曜日";
share.sun_s="日";
share.sun_s1="S";
share.mon="月曜日";
share.mon_s="月";
share.mon_s1="M";
share.tue="火曜日";
share.tue_s="火";
share.tue_s1="T";
share.wed="水曜日";
share.wed_s="水";
share.wed_s1="W";
share.thu="木曜日";
share.thu_s="木";
share.thu_s1="T";
share.fri="金曜日";
share.fri_s="金";
share.fri_s1="F";
share.sat="土曜日";
share.sat_s="土";
share.sat_s1="S";
share.expires="有効期限";
share.yes="イエス";
share.no="ノー";
share.allow="許可";
share.deny="拒否";
share.range="レンジ";
share.use="Use";
share.mins="分.";
share.secs="秒.";
share.routername="ルーター名";
share.manual="マニュアル";
share.port="ポート";
share.ssid="SSID";
share.channel="チャンネル";
share.rssi="Rssi";
share.signal="シグナル";
share.noise="ノイズ";
share.beacon="ビーコン";
share.openn="開く";
share.dtim="dtim";
share.rates="Rate";
share.low="低";
share.medium="中";
share.high="高";
share.option="オプション";
share.rule="ルール";
share.lan="LAN";
share.point2point="Point to Point";
share.nat="NAT";
share.subnet="サブネットマスク";
share.unmask="Unmask";
share.deflt="ディフォルト";  //don't use share.default !!!
share.all="全";
share.auto="自動";
share.right="右";
share.left="左";
share.share_key="Shared Key";
share.inter="間隔 (秒)";
share.srv="サービス名";
share.port_range="ポート範囲";
share.priority="プライオリティ";
share.gateway="ゲートウェイ";
share.intrface="インターフェース";  //don't use share.interface, Mozilla problem!!!
share.router="ルーター";
share.static_lease="Static Leases";
share.srvip="サーバーIP";
share.localdns="ローカルDNS";
share.minutes="minutes";
share.oui="OUI検索";
share.sttic="Static";
share.connecting="接続中";
share.connect="接続";
share.connected="接続しました";
share.disconnect="切断";
share.disconnected="切断しました";
share.info="インフォーメーション";
share.state="状態";
share.mode="モード";
share.encrypt="暗号化";
share.key="キー";
share.wireless="ワイアレス";
share.dhcp="DHCP";
share.styl="スタイル";
share.err="エラー";
share.errs="エラー";
share.meters="メーター";



var sbutton = new Object();
sbutton.save="設定の保存";
sbutton.saving="保存しました";
sbutton.cmd="実行中";
sbutton.cancel="変更のキャンセル";
sbutton.refres="リフレッシュ";
sbutton.clos="閉じる";
sbutton.del="削除";
sbutton.continu="続行";
sbutton.add="登録";
sbutton.remove="除去";
sbutton.modify="変更";
sbutton.deleted="削除されました";
sbutton.delall="全削除";
sbutton.autorefresh="自動更新はOnです";
sbutton.backup="バックアップ";
sbutton.restore="復元";
sbutton.cptotext="編集";
sbutton.runcmd="コマンドの実行";
sbutton.startup="Save Startup";
sbutton.firewall="ファイアウォール保存";
sbutton.wol="起動";
sbutton.add_wol="ホスト追加";
sbutton.manual_wol="マニュアル起動";
sbutton.summary="まとめ";
sbutton.filterIP="PCリストの編集";
sbutton.filterMac="MACフィルターリストの編集";
sbutton.filterSer="サービスの追加と編集";
sbutton.reboot="ロボットルーター";
sbutton.help="   ヘルプ  ";
sbutton.wl_client_mac="ワイアレスMACクライアントリスト";
sbutton.update_filter="フィルターリストの更新";
sbutton.join="Join";
sbutton.log_in="Incoming Log";
sbutton.log_out="Outgoing Log";
sbutton.apply="適用";
sbutton.edit_srv="サービスの追加と編集";
sbutton.routingtab="ルーティングテーブルの表示";
sbutton.wanmac="現PCのMACアドレス";
sbutton.dhcprel="DHCPリリース";
sbutton.dhcpren="DHCP更新";
sbutton.survey="サイトの調査";
sbutton.upgrading="アップグレード中";
sbutton.upgrade="アップグレード";
sbutton.preview="プリビュー";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//
var errmsg = new Object();
errmsg.err0="ユーザー名を入力されていません。";
errmsg.err1="ルター名が入力されていません。";
errmsg.err2="範囲外です。スタートIPアドレス、あるいはユーザー番号を修正してください。";
errmsg.err3="最低一日以上選択する必要があります。"
errmsg.err4="終了時間は開始時間より遅い必要があります。";
errmsg.err5="MACアドレス長さが異なります。";
errmsg.err6="パスワードを入力してください。";
errmsg.err7="ホスト名を入力してください。";
errmsg.err8="IPアドレスかドメイン名を入力してください。";
errmsg.err9="不正なDMZのIPアドレスです。";
errmsg.err10="確認パスワードが異なっています。もう一度パスワードを入力してください。";
errmsg.err11="パスワードに空白が入っています。";
errmsg.err12="実行するコマンドを入力してください";
errmsg.err13="更新に失敗しました。";
errmsg.err45="HTTPSによる接続はできません。HTTPモードで接続してください";
errmsg.err46="HTTPSによる接続はできません。";


//common.js error messages
errmsg.err14=" 値が範囲外です [";
errmsg.err15="WAN側のMACアドレスが範囲外です [00 - ff].";
errmsg.err16="MACの2文字目は偶数です : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="MACアドレスに問題があります。";
errmsg.err18="MACアドレス長に問題があります。";
errmsg.err19="MACアドレスにはブロードキャストアドレスは使えません。"
errmsg.err20="MACアドレスを(xx:xx:xx:xx:xx:xx)のフォーマットで入力してください。";
errmsg.err21="MACアドレスフォーマットに問題があります。";
errmsg.err22="WAN側のMACアドレスに問題があります。";
errmsg.err23="HEX値ではありません";
errmsg.err24=" found in MAC address ";
errmsg.err25="キー値に問題があります。";
errmsg.err26="キー長さに問題があります。";
errmsg.err27="不正なサブネットマスクです。";
errmsg.err28=" 不正な文字です。[ 0 - 9 ]で入力してください。";
errmsg.err29=" 不正なASCIIコードです。";
errmsg.err30=" 不正な16進数です。";
errmsg.err31=" 不正な値です。";
errmsg.err32="IPアドレスとゲートウェイが違うサブネットマスクにあります。";
errmsg.err33="IPアドレスとゲートウェイは同じ値にはできません。";
errmsg.err34="スペース（空白）を含むことはできません。";

//Wol.asp error messages
errmsg.err35="実行するにはMACアドレスを登録してください。";
errmsg.err36="実行するにはネットワークブロードキャストアドレスを登録してください。";
errmsg.err37="実行するにはUDPポートを登録してください。";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="共有キーを入力してください！";
errmsg.err39="不正なキーです。キーは8～63文字のASCII、あるいは64桁の16進数です。";
errmsg.err40="をYou have to enter a key for Key ";
errmsg.err41="不正キー長 ";
errmsg.err43="Rekey interval";

//config.asp error messages
errmsg.err42="復元する設定ファイルを選んでください。";

//WL_ActiveTable.asp error messages
errmsg.err44="The total checks exceed 128 counts.";

//Site_Survey.asp error messages
errmsg.err47="不正なSSIDです。";

//Wireless_WDS.asp error messages
errmsg.err48="WDSは現在のルーター設定と互換性がありません。以下の点についてチェックしてください。\n　＊　ワイアレスモードがAPに設定されている。\n　＊　WPA2がWDSでサポートされている。\n　＊　ワイアレスネットワークのB-OnlyモードはWDSでサポートされていない。";

//Wireless_radauth.asp error messages
errmsg.err49="RadiusはAPモードのみです。";

//Wireless_Basic.asp error messages
errmsg.err50="SSIDを入力する必要があります。";

// Management.asp error messages
errmsg.err51="ルーターのパスワードはデフォルト値のままです。セキュリティ保護のため、リモート管理を使用する前にパスワードを変更する必要があります。ＯＫボタンを押すとパスワードの変更ができます。キャンセルボタンを押すとリモート管理は仕様不可のままになります。";
errmsg.err52="確認用のパスワードが異なります";

// Port_Services.asp error messages
errmsg.err53="全ての操作が終わった後、Applyボタンを押して設定を保存します。";
errmsg.err54="サービス名を入力してください。";
errmsg.err55="サービス名はすでに存在します。";

// QoS.asp error messages
errmsg.err56="ポート値は次の範囲で指定してください[0 - 65535]。";

// Routing.asp error messages
errmsg.err57="エントリを削除しますか？";

// Status_Lan.asp error messages
errmsg.err58="クリックしてリースを解除してください";

//Status_Wireless.asp error messages
errmsg.err59="利用できません！ワイアレスネットワークを確認してください。";

//Upgrade.asp error messages
errmsg.err60="アップグレードするファイルを選択してください。";
errmsg.err61="不正なイメージファイルです。";

//Services.asp error messages
errmsg.err62="は静的リースに設定されています。";

// *******************************************  COMMON MENU ENTRIES  *******************************************//
var bmenu= new Object();
bmenu.setup="設定";
bmenu.setupbasic="基本設定";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MACアドレスクローン";
bmenu.setuprouting="アドバンスルーティング";
bmenu.setupvlan="VLANs";

bmenu.wireless="ワイアレス";
bmenu.wirelessBasic="基本設定";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="ワイアレスセキュリティ";
bmenu.wirelessMac="MACフィルター";
bmenu.wirelessAdvanced="アドバンス設定";
bmenu.wirelessWds="WDS";

bmenu.security="セキュリティ";
bmenu.firwall="ファイアウォール";
bmenu.vpn="VPN";

bmenu.accrestriction="アクセス制限";
bmenu.webaccess="インターネットアクセス";


bmenu.applications="アプリケーション &amp; ゲーム";
bmenu.applicationsprforwarding="ポートレンジ転送";
bmenu.applicationspforwarding="ポート転送";
bmenu.applicationsptriggering="ポートトリガーリング";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Overview";
bmenu.sipathphone="Phonebook";
bmenu.sipathstatus="Status";

bmenu.admin="管理者";
bmenu.adminManagement="管理";
bmenu.adminHotspot="ホットスポット";
bmenu.adminServices="サービス";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Log";
bmenu.adminDiag="コマンド";
bmenu.adminWol="WOL";
bmenu.adminFactory="初期設定";
bmenu.adminUpgrade="ファームウェアアップグレード";
bmenu.adminBackup="バックアップ";


bmenu.statu="ステータス";
bmenu.statuRouter="ルーター";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Sputnikエージェント";
bmenu.statuWLAN="ワイアレス";
bmenu.statuSysInfo="Sys-Info";


// ******************************************* Alive.asp *******************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="スケジュールリブート";
alive.sevr1="スケジュールリブート";
alive.hour="At a set Time";
alive.legend2="WDS/Connection Watchdog";
alive.sevr2="Watchdogを有効にする";
alive.IP="IPアドレス";
alive.legend3="プロクシー/接続Watchdog";
alive.sevr3="プロクシーWatchdogを有効にする";
alive.IP2="プロクシーIPアドレス";
alive.port="プロクシーポート";

//help container
var halive = new Object();
halive.right2="ルーターをリブートする時間を選んでください。管理タブでCronが設定されている必要があります。";
halive.right4="IPは<em>空白</em>で区切りで最大で３つまでです。IPフォーマットは xxx.xxx.xxx.xxx です";



// ******************************************* config.asp *******************************************//

var config = new Object();
config.titl=" - バックアップ & 復元";
config.h2="バックアップ機器構成";
config.legend="バックアップ設定";
config.mess1="\"" + sbutton.backup + "\"ボタンをクリックして、機器構成バックアップファイルを自分のコンピューターにダウンロードしてください。";
config.h22="復元構成";
config.legend2="復元設定";
config.mess2="復元するファイルを選択してください";
config.mess3="注意W A R N I N G";
config.mess4="本機器と同じファームウェアとモデルのバックアップファイルのみをアップロードしてください。<br />　本設定画面で作成されていないファイルをアップロードしないでください！";

//help container
var hconfig = new Object();
hconfig.right2="工場出荷の初期値にルーターの設定を戻す必要がある場合、現在の設定をバックアップしてください。<br /><br /><em>バックアップ</em>ボタンをクリックして現在の設定をバックアップします。";
hconfig.right4="<em>ブラウズ...</em>ボタンを押すとPCに保存されている設定ファイル一覧を表示します。<br /><br /><em>" + sbutton.restore + "</em>ボタンをクリックすると、全ての機器設定が設定ファイルに保存された値に上書きされます。";



// ******************************************* DDNS.asp *******************************************//

var ddns = new Object();
ddns.titl=" - ダイナミックDNS"
ddns.h2="ダイナミック ドメイン名システム(DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNSサービス";
ddns.emailaddr="E-mailアドレス";
ddns.typ="タイプ";
ddns.dynamic="ダイナミック";
ddns.custom="カスタム";
ddns.wildcard="ワイルドカード";
ddns.statu="DDNSステータス";
ddns.system="DNSシステム";
ddns.options="追加DDNSオプション";

var ddnsm = new Object();
ddnsm.dyn_strange="サーバーからの応答に問題があります。正しいサーバーに接続しているか確認してください。";
ddnsm.dyn_good="DDNSのアップデートに成功しました";
ddnsm.dyn_noupdate="現在、アップデートは必要ありません";
ddnsm.dyn_nohost="ホスト名がありません";
ddnsm.dyn_notfqdn="ホスト名に問題が間違っています";
ddnsm.dyn_yours="Host is not under your control";
ddnsm.dyn_abuse="Host has been blocked for abuse";
ddnsm.dyn_nochg="最後のアップデートからIPアドレスは変化ありません";
ddnsm.dyn_badauth="認証に失敗しました (username あるいは passwords)";
ddnsm.dyn_badsys="不正なシステムパラメーター";
ddnsm.dyn_badagent="このユーザーエージェントはブロックされました";
ddnsm.dyn_numhost="多すぎるか少なすぎるホストが見つかりました";
ddnsm.dyn_dnserr="DNS内部エラー";
ddnsm.dyn_911="予想外のエラー911";
ddnsm.dyn_999="予想外のエラー999";
ddnsm.dyn_donator="寄付者のみ利用できる機能です。寄付をお願いします。";
ddnsm.dyn_uncode="予想外の戻りコード";

ddnsm.tzo_good="操作完了";
ddnsm.tzo_noupdate="現在、アップグレードは必要ありません";
ddnsm.tzo_notfqdn="不正なドメイン名";
ddnsm.tzo_notmail="不正なEmail";
ddnsm.tzo_notact="不正な操作";
ddnsm.tzo_notkey="不正なKey";
ddnsm.tzo_notip="不正なIPアドレス";
ddnsm.tzo_dupfqdn="同じドメイン名";
ddnsm.tzo_fqdncre="ドメイン名はすでに作成されています";
ddnsm.tzo_expired="アカウントは期限切れです";
ddnsm.tzo_error="予想外のサーバーエラー";

ddnsm.zone_701="このアカウントはゾーンがセットアップされていません";
ddnsm.zone_702="アップデート失敗";
ddnsm.zone_703="<em>zones</em> あるいは <em>host</em> のどちらかが必要です";
ddnsm.zone_704="ゾーンは有効な<em>dotted</em>インターネット名でなければいけません";
ddnsm.zone_705="ゾーンは空ではいけません";
ddnsm.zone_707="同じホスト･IPに同一のアップデートです。クライアントの設定を修正してください";
ddnsm.zone_201="レコードのアップデートは必要ありません";
ddnsm.zone_badauth="認証に失敗しました (username あるいは passwords)";
ddnsm.zone_good="ZoneEditのアップデートが完了しました";
ddnsm.zone_strange="不明なサーバー応答です。正しいサーバーに接続しているか確認してください。";

ddnsm.all_closed="DDNSサーバーは現在、閉鎖されています";
ddnsm.all_resolving="ドメイン名を解決しています";
ddnsm.all_errresolv="ドメイン名の解決に失敗しました";
ddnsm.all_connecting="サーバーに接続しています";
ddnsm.all_connectfail="サーバーへの接続に失敗しました";
ddnsm.all_disabled="DDNS機能は使用不可になっています";
ddnsm.all_noip="インターネット接続できません";

//help container
var hddns = new Object();
hddns.right2="DDNSを使うとIPアドレスの代わりにドメイン名でネットワークにアクセスできます。IPアドレスの変更に対応して、IPアドレスとドメイン情報を動的にアップデートします。DynDNS、freedns.afraid.org、ZoneEdit.com、No-IP.com、Customなどのサービスと契約してください。";



// ******************************************* Diagnostics.asp *******************************************//

var diag = new Object();
diag.titl=" - 診断";
diag.h2="診断";
diag.legend="コマンドシェル";
diag.cmd="コマンド";
diag.startup="スタートアップ";
diag.firewall="ファイアウォール";

//help container
var hdiag = new Object();
hdiag.right2="ウェブ画面からコマンドを発行することができます。テキストエリアにコマンドを打ち込み、<em>" + sbutton.runcmd + "</em>を押してください。";



// ******************************************* DMZ.asp *******************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Demilitarized Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="DMZを使用する";
dmz.host="DMZホストIPアドレス";


//help container
var hdmz = new Object();
hdmz.right2="このオプションを使用すると、選択したホストがインターネットにさらされます。全てのポートがインターネットからアクセスできるようになります。";



// ******************************************* Factory_Defaults.asp *******************************************//

var factdef = new Object();
factdef.titl=" - 初期設定";
factdef.h2="初期設定";
factdef.legend="ルーター設定のリセット";
factdef.restore="初期設定を復元する";

factdef.mess1="注意！ＯＫをクリックすると機器の設定は初期設定に戻り、今までの設定は全て消去されます。";

//help container
var hfactdef = new Object();
hfactdef.right1="全ての設定を初期設定に戻します。今までの設定は消去されます。";



// ******************************************* FilterIP%AC.asp *******************************************//

var filterIP = new Object();
filterIP.titl=" - PCリスト";
filterIP.h2="PCリスト";
filterIP.h3="PCのMACアドレスを xx:xx:xx:xx:xx:xx のフォーマットで入力してください";
filterIP.h32="PCのIPアドレスを入力してください";
filterIP.h33="PCのIPレンジを入力してください";
filterIP.ip_range="IPレンジ";



// ******************************************* Filter.asp *******************************************//

var filter = new Object();
filter.titl=" - アクセス制限";
filter.h2="インターネットアクセス";
filter.legend="アクセスポリシー";
filter.restore="初期設定の復元";
filter.pol="ポリシー";
filter.polname="ポリシー名";
filter.pcs="PCs";
filter.polallow="選択した日時のインターネットアクセス。";
filter.legend2="日";
filter.time="時";
filter.h24="24時間";
filter.legend3="ブロックされたサービス";
filter.catchall="全てのP2Pプロトコルをキャッチする";
filter.legend4="URLアドレスによるウェブサイトブロック";
filter.legend5="キーワードによるによるウェブサイトブロック";

filter.mess1="ポリシーを削除しますか？";
filter.mess2="最低1日は選択してください。";
filter.mess3="スタート時間は終了時間以前を指定してください。";

//help container
var hfilter = new Object();
hfilter.right2="最大10のアクセスポリシーを定義できます。<em>" + sbutton.del + "</em>をクリックしてポリシーを削除するか、<em>" + sbutton.summary + "</em>をクリックしてポリシーの一覧を表示してください。";
hfilter.right4="ポリシーを有効あるいは無効にする。";
hfilter.right6="ポリシーに名前をつけることができます。";
hfilter.right8="ポリシーを有効にする曜日を選択してください。";
hfilter.right10="ポリシーを有効にする時刻を入力してください";
hfilter.right12="いくつかのサービスへのアクセスをブロックすることができます。<em>" + sbutton.filterSer + "</em>をクリックして設定を変更してください";
hfilter.right14="URLで指定したウェブサイトへのアクセスをブロックできます。";
hfilter.right16="指定したキーワードを含むウェブサイトへのアクセスをブロックできます。";



// ******************************************* FilterSummary.asp *******************************************//

var filterSum = new Object();
filterSum.titl=" - アクセス制限サマリー";
filterSum.h2="インターネット ポリシー サマリー";
filterSum.polnum="No.";
filterSum.polday="時刻";



// ******************************************* Firewall.asp *******************************************//

var firewall = new Object();
firewall.titl=" - ファイアウォール";
firewall.h2="セキュリティ";
firewall.legend="ファイアウォールによる保護";
firewall.firewall="SPIファイアウォール";
firewall.legend2="追加フィルター";
firewall.proxy="プロクシーをフィルター";
firewall.cookies="クッキーをフィルター";
firewall.applet="Javaアプレットをフィルター";
firewall.activex="ActiveXをフィルター";
firewall.legend3="WANリクエストブロック";
firewall.ping="匿名のインターネットアクセスをブロック";
firewall.muticast="マルチキャストをフィルター";
filter.nat="インターネットNATリダイレクションをフィルター";
filter.port113="IDENT (Port 113)をフィルター";

//help container
var hfirewall = new Object();
hfirewall.right2="SPIファイアウォールを有効･無効にする";



// ******************************************* Forward.asp *******************************************//

var prforward = new Object();
prforward.titl=" - Port Range Forwarding";
prforward.h2="Port Range Forward";
prforward.legend="転送";
prforward.app="アプリケーション";

//help container
var hprforward = new Object();
hprforward.right2="アプリケーションによってはポートを開放する必要があります。例としてサーバーやオンラインゲームなどがあります。インターネットからあるポートへの要求が来た場合、ルーターは指定されたコンピューターへデータをルートします。セキュリティのため使用しているポートへポートフォワーディングをすることができます。終了後は<em>" + share.enable +"</em>のチェックを外してください。";



// ******************************************* ForwardSpec.asp *******************************************//

var pforward = new Object();
pforward.titl=" - ポートフォワーディング";
pforward.h2="ポート転送";
pforward.legend="転送";
pforward.app="アプリケーション";
pforward.from="受付ポート";
pforward.to="出力ポート";

//help container
var hpforward = new Object();
hpforward.right2="アプリケーションによってはポートを開放する必要があります。例としてサーバーやオンラインゲームなどがあります。インターネットからあるポートへの要求が来た場合、ルーターは指定されたコンピューターへデータをルートします。セキュリティのため使用しているポートへポートフォワーディングをすることができます。終了後は<em>" + share.enable +"</em>のチェックを外してください。";



// ******************************************* Hotspot.asp *******************************************//

var hotspot = new Object();
hotspot.titl=" - ホットスポット";
hotspot.h2="ホットスポット Portal";
hotspot.legend="チリスポット";
hotspot.nowifibridge="WifiとLANブリッジを切り離す";
hotspot.hotspot="チリスポット";
hotspot.pserver="Primary Radius Server IP/DNS";
hotspot.bserver="Backup Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="URLリダイレクト";
hotspot.dhcp="DHCPインターフェース";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM Secret";
hotspot.uamdns="UAM Any DNS";
hotspot.allowuam="UAMを許可";
hotspot.macauth="MACauth";
hotspot.option="追加チリスポットオプション";
hotspot.fon_chilli="チリスポット Local ユーザー管理";
hotspot.fon_user="ユーザー一覧";
hotspot.http_legend="HTTPリダイレクト";
hotspot.http_srv="HTTPリダイレクト";
hotspot.http_ip="HTTP Destination IP";
hotspot.http_port="HTTP Destination Port";
hotspot.http_net="HTTP Source Network";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Gateway Name";
hotspot.nocat_home="ホームページ";
hotspot.nocat_allowweb="許可されたウェブホスト";
hotspot.nocat_docroot="Document Root";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="除外Ports";
hotspot.nocat_timeout="Login Timeout";
hotspot.nocat_verbose="饒舌度";
hotspot.nocat_route="Route Only";
hotspot.smtp_legend="SMTPリダイレクト";
hotspot.smtp_srv="SMTPリダイレクト";
hotspot.smtp_ip="SMTP Destination IP";
hotspot.smtp_net="SMTP Source Network";
hotspot.shat_legend="Zero IP 設定";
hotspot.shat_srv="Zero IP 設定";
hotspot.shat_srv2="Zero IP 設定は有効です";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik エージェント";
hotspot.sputnik_id="Sputnik サーバー ID";
hotspot.sputnik_instant="Use Sputnik 簡単設定";
hotspot.sputnik_express="Use SputnikNet Express";
hotspot.sputnik_about="Sputnikについて";



// ******************************************* Info.htm *******************************************//

var info = new Object();
info.titl=" - 情報";
info.h2="システム情報";
info.wlanmac="ワイアレスMAC";
info.srv="サービス";
info.ap="アクセスポイント";



// ******************************************* index_heartbeat.asp *******************************************//

var idx_h = new Object();
idx_h.srv="Heart Beat Server";
idx_h.con_strgy="接続戦術";
idx_h.max_idle="要求に応じた接続: アイドル時間を最大";
idx_h.alive="Keep Alive: Redial Period";



// ******************************************* index_l2tp.asp *******************************************//

var idx_l = new Object();
idx_l.srv="L2TPサーバー";



// ******************************************* index_pppoe.asp *******************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="RP PPPoEを用いる";



// ******************************************* index_pptp.asp *******************************************//

var idx_pptp = new Object();
idx_pptp.srv="Use DHCP";
idx_pptp.wan_ip="インターネットIPアドレス";
idx_pptp.gateway="ゲートウェイ (PPTPサーバー)";
idx_pptp.encrypt="PPTP暗号化";



// ******************************************* index_static.asp *******************************************//

var idx_static = new Object();
idx_static.dns="スタティックDNS";



// ******************************************* index.asp *******************************************//

var idx = new Object();
idx.titl=" - 設定";
idx.h2="インターネット設定";
idx.h22="ワイアレス設定";
idx.legend="インターネット接続タイプ";
idx.conn_type="接続タイプ";
idx.stp="STP";
idx.stp_mess="(COMCAST ISPのため無効にする)";
idx.optional="オプション設定(ISPにより必要)";
idx.mtu="MTU";
idx.h23="ネットワーク設定";
idx.routerip="ルーターIP";
idx.lanip="ローカルIPアドレス";
idx.legend2="WANポート";
idx.wantoswitch="スイッチするWANポートを指定";
idx.legend3="時刻設定";
idx.timeset="タイムゾーン/夏時間(DST)";
idx.localtime="ローカル時間を用いる";
idx.static_ip="スタティックIP";
idx.dhcp="自動設定 - DHCP";
idx.dhcp_legend="ネットワーク アドレス サーバー設定 (DHCP)";
idx.dhcp_type="DHCPタイプ";
idx.dhcp_srv="DHCPサーバー";
idx.dhcp_fwd="DHCP Forwarder";
idx.dhcp_start="IPアドレスを開始";
idx.dhcp_end="IPアドレスを終了";        //used in Status_Lan.asp
idx.dhcp_maxusers="最大DHCPユーザー数";
idx.dhcp_lease="クライアントリース時間";
idx.dhcp_dnsmasq="DHCPにDNSMasqを使う";
idx.dns_dnsmasq="DHCPにDNSMasqを使う";
idx.auth_dnsmasq="DHCP-Authoritative";



//help container
var hidx = new Object();
hidx.right2="この設定は、ケーブル会社でもっとも広く使われています。";
hidx.right4="Enter the host name provided by your ISPから指定されたホスト名を入力";
hidx.right6="Enter the domain name provided by your ISPから指定されたドメイン名を入力";
hidx.right8="ルーターのアドレスです";
hidx.right10="Tルーターのサブネットマスク(subnet mask)です";
hidx.right12="ルーターにIPアドレスの管理を許可する";
hidx.right14="開始の際に使いたいアドレス";
hidx.right16="ルーターが指定するアドレスを制限することができます。";
hidx.right18="タイムゾーンと夏時間（DST）を指定してください。ルーターがローカル時間とUTC時間も使えます。";



// ******************************************* Join.asp *******************************************//

var join = new Object();

//sshd.webservices
join.titl=" - ジョイン";
join.mess1="次のネットワークとクライアントとしてジョインに成功しました: ";



// ******************************************* Log_incoming.asp *******************************************//

var log_in = new Object();
log_in.titl=" - Incomingログテーブル";
log_in.h2="Incomingログテーブル";
log_in.th_ip="Source IP";
log_in.th_port="Destination Port Number";



// ******************************************* Log_outgoing.asp *******************************************//

var log_out = new Object();
log_out.titl=" - Outgoingログテーブル";
log_out.h2="Outgoingログテーブル";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destination URL/IP";
log_out.th_port="サービス/ポート番号";



// ******************************************* Log.asp *******************************************//

var log = new Object();
log.titl=" - ログ";
log.h2="ログ管理";
log.legend="ログ";
log.lvl="ログレベル";
log.drop="ドロップ";
log.reject="拒否";
log.accept="アクセプト";



// ******************************************* Management.asp *******************************************//

var management = new Object();
management.titl=" - 管理者";
management.h2="ルーター管理";

management.psswd_legend="ルーターパスワード";
management.psswd_user="ルーターユーザー名";
management.psswd_pass="ルーターパスワード";
management.pass_conf="確認のため再入力";

management.remote_legend="リモートアクセス";
management.remote_gui="ウェブ管理画面";
management.remote_https="HTTPSを使用";
management.remote_guiport="ウェブ管理画面ポート";
management.remote_ssh="SSH管理";
management.remote_sshport="SSHリモートポート";

management.web_legend="ウェブアクセス";
management.web_refresh="自動リフレッシュ(秒)";
management.web_sysinfo="Info Siteを有効にする";
management.web_sysinfopass="Info Site パスワード保護";
management.web_sysinfomasq="Info Site MACマスカレード";

management.boot_legend="Boot Wait";
management.boot_srv="Boot Wait";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="NTPクライアント";
management.ntp_srv="NTP";

management.rst_legend="リセットボタン";
management.rst_srv="リセットボタン";

management.routing_legend="ルーティング";
management.routing_srv="ルーティング";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="SecureOnパスワード";
management.wol_mac="MACアドレス<br/>( フォーマット: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="IPv6サポート";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvdを有効にする";
management.ipv6_radconf="Radvd設定";

management.jffs_legend="JFFS2サポート";
management.jffs_srv="JFFS2";
management.jffs_clean="Clean JFFS2";

management.lang_legend="言語選択";
management.lang_srv="言語";
management.lang_bulgarian="ブルガリア語";
management.lang_chinese_traditional="中国語（繁体字）";
management.lang_chinese_simplified="中国語（簡体字）";
management.lang_croatian="クロアチア語";
management.lang_czech="チェコ語";
management.lang_dutch="オランダ語";
management.lang_portuguese_braz="ポルトガル語 (ブラジル)";
management.lang_english="英語";
management.lang_french="仏語";
management.lang_german="独語";
management.lang_italian="伊太利亜後";
management.lang_brazilian="ブラジル語";
management.lang_slovenian="スロベニア後";
management.lang_spanish="スペイン語";
management.lang_swedish="スウェーデン語"; // management.lang_japanese="日本語";

management.net_legend="IPフィルター設定 (P2P用の設定)";
management.net_port="最大Port";
management.net_tcptimeout="TCP Timeout (秒)";
management.net_udptimeout="UDP Timeout (秒)";

management.clock_legend="オーバークロック";
management.clock_frq="周波数";
management.clock_support="サポート外です";

management.mmc_legend="MMC/SDカードサポート";
management.mmc_srv="MMCデバイス";

management.samba_legend="Samba FS Automount";
management.samba_srv="SMBファイルシステム";
management.samba_share="Share";
management.samba_stscript="Startscript";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIPポート";
management.SIPatH_domain="SIPドメイン";

management.gui_style="ルーターGUIスタイル";



//help container
var hmanagement = new Object();
hmanagement.right1="Auto-Refresh:";
hmanagement.right2="ウェブGUIの自動更新時間を調整します。0は無効にします。";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

var portserv = new Object();
portserv.titl=" - ポートサービス";
portserv.h2="ポートサービス";



// ******************************************* QoS.asp *******************************************//

var qos = new Object();
qos.titl=" - Quality of Service";
qos.h2="Quality Of Service (QoS)";
qos.legend="QoS設定";
qos.srv="QoS開始";
qos.type="パケット スケジューラー";
qos.uplink="Uplink (kbps)";
qos.dnlink="Downlink (kbps)";
qos.gaming="ゲーム用に最適化";
qos.legend2="サービス プライオリティ";
qos.prio_x="免除";
qos.prio_p="プレミアム";
qos.prio_e="エクスプレス";
qos.prio_s="スタンダード";
qos.prio_b="バルク";
qos.legend3="ネットマスク プライオリティ";
qos.ipmask="IP/Mask";
qos.maxrate_b="最大Kbits";
qos.maxrate_o="最大Rate";
qos.legend4="MAC プライオリティ";
qos.legend5="Ethernetポート プライオリティ";
qos.legend6="ディフォルト回線容量レベル";
qos.bandwith="回線容量（Kbits）";

//help container
var hqos = new Object();
hqos.right1="Uplink:";
hqos.right2="アップロード制限の80%-95% (max)に設定してください";
hqos.right3="Downlink:";
hqos.right4="トータルダウンロード制限の80%-100% (max)に設定してください";
hqos.right6="通信帯域を消費しているアプリケーションについてデータレートを制御できます。";
hqos.right8="IPアドレスあるいはIPアドレスの範囲について通信のプライオリティを設定できます。";
hqos.right10="デバイス名とMACアドレスを指定して、ネットワーク上にあるデバイスからの通信のプライオリティを設定できます。";
hqos.right12="デバイスが接続している物理的LANポートを元に、データレートを制御できます。LANポートごとに１から４のプライオリティを指定できます。";



// ******************************************* RouteTable.asp *******************************************//

var routetbl = new Object();
routetbl.titl=" - ルーティング テーブル";
routetbl.h2="ルーティング テーブル 入力リスト";
routetbl.th1="Destination LAN IP";



// ******************************************* Routing.asp *******************************************//

var route = new Object();
route.titl=" - ルーティング";
route.h2="Advanced ルーティング";
route.mod="動作モード";
route.bgp_legend="BGP設定";
route.bgp_ip="Neighbor IP";
route.bgp_as="Neighbor AS#";
route.rip2_mod="RIP2ルーター";
route.ospf_mod="OSPFルーター";
route.gateway_legend="動的ルーター";
route.static_legend="静的ルーター";
route.static_setno="set numberを選択";
route.static_name="ルート名";
route.static_ip="Destination LAN IP";

//help container
var hroute = new Object();
hroute.right2="ルーターを用いてインターネット接続を干すとしている場合は、<em>ゲートウェイ</em>モードを選択してください。他のルーターがネットワーク上にある場合、<em>ルーター</em>モードを選択してください。";
hroute.right4="これはユニークなルート番号です。最大20までのルートにつき設定できます。";
hroute.right6="このルートにつける名前を入力してください。";
hroute.right8="これはスタティックルートを割り当てるリモートホストです。";
hroute.right10="ホストとネットワークポーションを決定します。";


// ******************************************* Site_Survey.asp *******************************************//

var survey = new Object();
survey.titl=" - サイト調査";
survey.h2="Neighbor&#39;s Wireless Networks";
survey.thjoin="サイトのジョイン";



// ******************************************* Services.asp *******************************************//

var service = new Object();

service.titl=" - サービス";
service.h2="サービス管理";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Kaid開始";
service.kaid_mac="コンソールMacs: (「;」で終了すること)";

//DHCPd
service.dhcp_legend="DHCPクライアント";
service.dhcp_vendor="ベンダークラスの設定";
service.dhcp_legend2="DHCPサーバー";
service.dhcp_srv="DHCP Daemon";
service.dhcp_jffs2="クライアントリースDBにJFFS2を使う";
service.dhcp_domain="使用されているドメイン";
service.dhcp_landomain="LANドメイン";
service.dhcp_option="Additional DHCPdオプション";
service.dnsmasq_legend="DNSMasq";
service.dnsmasq_srv="DNSMasq";
service.dnsmasq_loc="ローカルDNS";
service.dnsmasq_opt="追加DNSオプション";

//pptp.webservices
service.pptp_legend="PPTP";
service.pptp_srv="PPTPサーバー";
service.pptp_client="クライアントIP(s)";
service.pptp_chap="CHAP-Secrets";

//syslog.webservices
service.syslog_legend="システムログ";
service.syslog_srv="Syslogd";
service.syslog_ip="リモート サーバー";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//pptpd_client.webservices
service.pptpd_legend="PPTPクライアント";
service.pptpd_option="PPTPクライアント オプション";
service.pptpd_ipdns="サーバーIP or DNS名";
service.pptpd_subnet="リモートSubnet";
service.pptpd_subnetmask="リモートSubnetマスク";
service.pptpd_encry="MPPE暗号化";
service.pptpd_mtu="MTU";
service.pptpd_mru="MRU";
service.pptpd_nat="NAT";

//rflow.webservices
service.rflow_legend="RFlow / MACupd";
service.rflow_srv1="RFlow";
service.rflow_srv2="MACupd";

//pppoe-relay.webservices
service.pppoe_legend="PPPOE Relay";
service.pppoe_srv="Relay";

//snmp.webservices
service.snmp_legend="SNMP";
service.snmp_srv="SNMP";
service.snmp_loc="Location";
service.snmp_contact="Contact";
service.snmp_name="Name";
service.snmp_read="RO Community";
service.snmp_write="RW Community";

//openvpn.webservices
service.vpn_legend="OpenVPNクライアント";
service.vpn_srv="OpenVPNの開始";
service.vpn_ipname="サーバーIP/名";
service.vpn_mtu="TUN MTU設定";
service.vpn_mru="TUN MTU Extra";
service.vpn_mss="TCP MSS";
service.vpn_compress="LZO圧縮を使う";
service.vpn_tunnel="Tunnelプロトコル";
service.vpn_srvcert="パブリック サーバー 証明書";
service.vpn_clicert="パブリック クライアント 証明書";
service.vpn_certtype="nsCertType";
service.vpn_clikey="プライベート クライアント キー";

//sshd.webservices
service.ssh_legend="セキュアシェル";
service.ssh_srv="SSHd";
service.ssh_password="パスワードログイン";
service.ssh_key="Authorized Keys";



// ******************************************* Sipath.asp + cgi *******************************************//

var sipath = new Object();
sipath.titl=" - SiPath 概要";
sipath.phone_titl=" - フォーンブック";
sipath.status_titl=" - ステータス";



// ******************************************* Status_Lan.asp *******************************************//

var status_lan = new Object();
status_lan.titl=" - LANステータス";
status_lan.h2="ローカル ネットワーク";
status_lan.legend="LANステータス";
status_lan.h22="Dynamic Host Configuration Protocol";
status_lan.legend2="DHCPステータス";
status_lan.legend3="DHCPクライアント";

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="ローカルEthernetネットワークにおけるルーターのMACアドレスです。";
hstatus_lan.right4="ローカルEthernetネットワークにおけるルーターのIPアドレスです。";
hstatus_lan.right6="ルーターがSubnetマスクを使用している場合、ここに表示されます。";
hstatus_lan.right8="ルーターをDHCPサーバーとして使用している場合、ここに表示されます。";
hstatus_lan.right10="MACアドレスをクリックすると、ネットワークインターフェースのOrganizationally Unique Identifier (IEEE標準のOUIデータベース検索)が分かります。";



// ******************************************* Status_Router.asp *******************************************//

var status_router = new Object();
status_router.titl=" - ルーター ステータス";
status_router.h2="ルーター情報";
status_router.legend="システム";
status_router.sys_model="ルーターモデル";
status_router.sys_firmver="ファームウェア バージョン";
status_router.sys_time="現在時刻";
status_router.sys_up="Uptime";
status_router.sys_load="平均ロード";
status_router.legend2="CPU";
status_router.cpu="CPUモデル";
status_router.clock="CPUクロック";
status_router.legend3="メモリ";
status_router.mem_tot="Total Available";
status_router.mem_free="フリー";
status_router.mem_used="使用";
status_router.mem_buf="バッファー";
status_router.mem_cached="キャッシュ";
status_router.mem_active="有効";
status_router.mem_inactive="無効";
status_router.legend4="ネットワーク";
status_router.net_maxports="IPフィルター最大ポート";
status_router.net_conntrack="有効なIP接続";
status_router.h22="インターネット";
status_router.legend5="設定タイプ";
status_router.www_login="ログインタイプ";
status_router.www_loginstatus="ログイン ステータス";

//help container
var hstatus_router = new Object();
hstatus_router.right2="ルーターの名前です。タブの<i>セットアップ</i>タブから設定できます。";
hstatus_router.right4="ISPから見えるルーターのMACアドレスです。";
hstatus_router.right6="現在のルーターのファームウェアです";
hstatus_router.right8="This is time received from the ntp server set on the <em>" + bmenu.admin + " | " + bmenu.adminManagement + "</em> tab.";
hstatus_router.right10="ルーターが動作して空の経過時間です。";
hstatus_router.right12="直近1分、5分、15分におけるシステムのロードを3つの数字で表します。";
hstatus_router.right14="お使いのISPを使ってインターネット接続に必要な情報です。設定タブから入力された情報です。現在の通信を<em>接続</em>あるいは<em>切断</em>できます。";



// ******************************************* Status_SputnikAPD.asp *******************************************//

var status_sputnik = new Object();
status_sputnik.titl=" - Sputnikエージェント ステータス";
status_sputnik.h2="Sputnik&reg; エージェント&trade;";
status_sputnik.manage="Managed By";
status_sputnik.license="SCCライセンス番号";

//help container
var hstatus_sputnik = new Object();
hstatus_sputnik.right1="Sputnikエージェント ステータス";
hstatus_sputnik.right2="Sputnikエージェントプロセスのステータスを表示しています。";
hstatus_sputnik.right4="アクセスポイントが接続しているSputnikコントロールセンター";
hstatus_sputnik.right6="現在のエージェント ステータス";
hstatus_sputnik.right8="Sputnikコントロールセンターのライセンス番号";



// ******************************************* Status_Wireless.asp *******************************************//

var status_wireless = new Object();
status_wireless.titl=" - ワイアレス ステータス";
status_wireless.h2="ワイアレス";
status_wireless.legend="ワイアレス ステータス";
status_wireless.net="ネットワーク";
status_wireless.pptp="PPTPステータス";
status_wireless.legend2="ワイアレス パケット情報";
status_wireless.rx="Received (RX)";
status_wireless.tx="Transmitted (TX)";
status_wireless.h22="ワイアレス ノード";
status_wireless.legend3="クライアント";
status_wireless.signal_qual="シグナル品質";
status_wireless.wds="WDSノード";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="ローカルワイアレスネットワークにおけるルーターのMACアドレスです。";
hstatus_wireless.right4="ワイアレスタブで指定された、ネットワークで使用するワイアレスモードを表示します（混在、Gのみ、Bのみ、無効）";



// ******************************************* Triggering.asp *******************************************//

var trforward = new Object();
trforward.titl=" - ポートTriggering";
trforward.h2="ポートTriggering";
trforward.legend="Forwards";
trforward.trrange="Triggeredポートレンジ";
trforward.fwdrange="Forwardedポートレンジ";
trforward.app="アプリケーション";

//help container
var htrforward = new Object();
htrforward.right2="triggerとなるアプリケーション名を入力";
htrforward.right4="各アプリケーションごとに、Triggered番号の範囲を一覧します。インターネットアプリケーションのドキュメントから必要とされるポート番号を確認してください。";
htrforward.right6="各アプリケーションごとに、Forwarded番号の範囲を一覧します。インターネットアプリケーションのドキュメントから必要とされるポート番号を確認してください。";
htrforward.right8="Triggered and Forwarded範囲の始まりのポート番号を入力してください。";
htrforward.right10="Triggered and Forwarded範囲の終わりのポート番号を入力してください。 ";



// ******************************************* Upgrade.asp *******************************************//

var upgrad = new Object();
upgrad.titl=" - ファームウェア アップグレード";
upgrad.h2="ファームウェア管理";
upgrad.legend="ファームウェア アップグレード";
upgrad.info1="After flashing, reset to";
upgrad.resetOff="リセット不可";
upgrad.resetOn="初期設定";
upgrad.file="アップグレードするファイルを指定してください";
upgrad.warning="W A R N I N G";
upgrad.mess1="ファームウェアのアップグレードには数分かかる場合があります。<br />その間、リセットしたり電源を切らないでください！";

//help container
var hupgrad = new Object();
hupgrad.right2="<em>Browse...</em>ボタンをクリックするとアップロードするファームウェアファイルを選択できます。<br /><br /><em>Upgrade</em>ボタンを押すとアップグレードが始まります。アップグレード処理は中断しないでください。";



// ******************************************* UPnP.asp *******************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="転送";
upnp.legend2="UPnP設定";
upnp.serv="UPnPサービス";
upnp.clear="開始時のポート転送をクリア";
upnp.url="Send presentation URL";
upnp.msg1="クリックして入力を削除";
upnp.msg2="全ての入力を削除しますか？";


//help container
var hupnp = new Object();
hupnp.right2="ゴミ箱をクリックすると各入力を削除できます。";
hupnp.right4="自動でアプリケーションがポート転送を設定することを許可します。";



// ******************************************* VPN.asp *******************************************//

var vpn = new Object();
vpn.titl=" - VPN";
vpn.h2="Virtual Private Network (VPN)";
vpn.legend="VPNパススルー";
vpn.ipsec="IPSecパススルー";
vpn.pptp="PPTPパススルー";
vpn.l2tp="L2TPパススルー";

//help container
var hvpn = new Object();
hvpn.right1="VPNを使ってネットワークデバイスとの通信を許可するため、IPSec、PPTP、あるいはL2TPパススルーを有効にできます。";


// ******************************************* Vlan.asp *******************************************//

var vlan = new Object();
vlan.titl=" - Virtual LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Assigned To<br />ブリッジ";
vlan.tagged="Tagged";
vlan.negociate="Auto-Negotiate";
vlan.aggregation="Link Aggregation<br>on Ports 3 & 4";
vlan.trunk="Trunk";


// ******************************************* WEP.asp *******************************************//

var wep = new Object();
wep.defkey="デフォルトTransmit Key";
wep.passphrase="パスフレーズ";



// ******************************************* WOL.asp *******************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="ホスト一覧";
wol.legend2="WOLアドレス";
wol.legend3="出力";
wol.legend4="マニュアルWOL";
wol.enable="WOLを有効にする?";
wol.mac="MACアドレス";
wol.broadcast="ネットブロードキャスト";
wol.udp="UDPポート";
wol.msg1="クリックするとWOLホストを削除します";

//help container
var hwol = new Object();
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router).";
hwol.right4="MAC Addresses are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB)";
hwol.right6="IP Address is typically the broadcast address for the local network, but could be a remote address if the target host is not connected to the router's local network."



// ******************************************* WanMAC.asp *******************************************//

var wanmac = new Object();
wanmac.titl=" - MACアドレスクローン";
wanmac.h2="MACアドレスクローン";
wanmac.legend="MACクローン";
wanmac.wan="クローンWAN MAC";
wanmac.wlan="クローン ワイアレスMAC";

//help container
var hwanmac = new Object();
hwanmac.right2="Some ISPs will require you to register your MAC address. If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

var wpa = new Object();
wpa.titl=" - ワイアレス セキュリティ";
wpa.h2="ワイアレス セキュリティ";
wpa.secmode="セキュリティ モード";
wpa.legend="ワイアレス 暗号化";
wpa.auth_mode="ネットワーク認証";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval (in seconds)";
wpa.radius_ipaddr="RADIUS サーバーアドレス";
wpa.radius_port="RADIUS サーバーポート";
wpa.radius_key="RADIUS Key";
wpa.algorithms="WPAアルゴリズム";
wpa.shared_key="WPA Shared Key";

//help container
var hwpa = new Object();
hwpa.right2="You may choose from Disable, WEP, WPA Pre-Shared Key, WPA RADIUS, or RADIUS. All devices on your network must use the same security mode.";



// ******************************************* WL_FilterTable.asp *******************************************//

var wl_filter = new Object();
wl_filter.titl=" - MACアドレス フィルター一覧";
wl_filter.h2="MACアドレス フィルター一覧";
wl_filter.h3="Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";



// ******************************************* WL_ActiveTable.asp *******************************************//

var wl_active = new Object();
wl_active.titl=" - ワイアレス Active クライアントMAC一覧";
wl_active.h2="ワイアレス クライアントMAC一覧";
wl_active.h3="MACフィルターを有効にする";
wl_active.active="Active PC";
wl_active.inactive="Inactive PC";



// ******************************************* Wireless_WDS.asp *******************************************//

var wds = new Object();
wds.titl=" - WDS";
wds.h2="Wireless Distribution System";
wds.legend="WDS設定";
wds.label="Lazy WDS";
wds.label2="WDSサブネット";
wds.wl_mac="ワイアレスMAC";
wds.lazy_default="初期値: 無効";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="サブネット";
wds.legend2="追加オプション";



// ******************************************* Wireless_radauth.asp *******************************************//

var radius = new Object();
radius.titl=" - Radius";
radius.h2="Remote Authentication Dial-In User Service";
radius.legend="Radius";
radius.label="MAC Radius クライアント";
radius.label2="MAC フォーマット";
radius.label3="Radius サーバーIP";
radius.label4="Radius サーバーポート";
radius.label5="認証のない最大ユーザー数";
radius.label6="パスワードフォーマット";
radius.label7="RADIUS Shared Secret";
radius.label8="サーバーが利用できない場合Radiusを無効にする";



// ******************************************* Wireless_MAC.asp *******************************************//

var wl_mac = new Object();
wl_mac.titl=" - MAC フィルター";
wl_mac.h2="ワイアレス MAC フィルター";
wl_mac.legend="MAC フィルター";
wl_mac.label="フィルターを使用する";
wl_mac.label2="フィルター モード";
wl_mac.deny="一覧のPCのワイアレスネットワークへの接続を許可しない。";
wl_mac.allow="一覧のPCのワイアレスネットワークへの接続を許可する。";



// ******************************************* Wireless_Basic.asp *******************************************//

var wl_basic = new Object();
wl_basic.titl=" - ワイアレス";
wl_basic.h2="ワイアレス";
wl_basic.legend="基本設定";
wl_basic.label="ワイアレス モード";
wl_basic.label2="ワイアレス ネットワーク モード";
wl_basic.label3="ワイアレス ネットワーク名 (SSID)";
wl_basic.label4="ワイアレス チャンネル";
wl_basic.label5="ワイアレス SSID ブロードキャスト";
wl_basic.label6="Sensitivity Range (ACK Timing)";
wl_basic.ap="AP";
wl_basic.client="クライアント";
wl_basic.repeater="Repeater";
wl_basic.clientBridge="クライアント ブリッジ";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="混在";
wl_basic.b="B-Only";
wl_basic.a="A-Only";
wl_basic.g="G-Only";

//help container
var hwl_basic = new Object();
hwl_basic.right2="ワイアレスGクライアントを排除したい場合は、<em>B-Only</em>モードを選択してください。ワイアレスによるアクセスを無効にしたい場合は<em>Disable</em>を選択してください";
hwl_basic.right3="Sensitivity Range: ";
hwl_basic.right4="ackタイミングを調整します。0でackタイミングを完全に無効にします。";



// ******************************************* Wireless_Advanced.asp *******************************************//

var wl_adv = new Object();
wl_adv.titl=" - アドバンスト ワイアレス 設定";
wl_adv.h2="アドバンスト ワイアレス 設定";
wl_adv.legend="アドバンスト 設定";
wl_adv.legend2="ワイアレス マルチメディア サポート設定";
wl_adv.label="認証タイプ";
wl_adv.label2="Basic Rate";
wl_adv.label3="Transmission Rate";
wl_adv.label4="CTS Protection Mode";
wl_adv.label5="Frame Burst";
wl_adv.label6="Beacon Interval";
wl_adv.label7="DTIM Interval";
wl_adv.label8="Fragmentation Threshold";
wl_adv.label9="RTS Threshold";
wl_adv.label10="Max Associated Clients";
wl_adv.label11="AP Isolation";
wl_adv.label12="TX Antenna";
wl_adv.label13="RX Antenna";
wl_adv.label14="Preamble";
wl_adv.reference="Noise Reference";
wl_adv.label15="Xmit Power";
wl_adv.label16="Afterburner";
wl_adv.label17="ワイアレス GUIアクセス";
wl_adv.label18="WMM サポート";
wl_adv.label19="No-Acknowledgement";
wl_adv.table1="EDCA AP Parameters (AP to Client)";
wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="Admin Forced";
wl_adv.row1="Background";
wl_adv.row2="Best Effort";
wl_adv.row3="Video";
wl_adv.row4="Voice";
wl_adv.table2="EDCA STA Parameters (Client to AP)";
wl_adv.lng="Long";                  //************* don't use .long ! *************
wl_adv.shrt="Short";                //************* don't use .short ! **************

//help container
var hwl_adv = new Object();
hwl_adv.right2="AutoあるいはShared Keyを選べます。Shared Key認証のほうがセキュリティが高くなりますが、ネットワーク上の全てのデバイスがSheared Key認証をサポートしている必要があります。";



// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

var fail = new Object();
fail.mess1="不正な値です。もう一度入力してください。";
fail.mess2="アップグレード失敗。";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="設定を保存しました。";
success.restore="設定を復元しました。<br/>リブートを開始します。少しお待ちください...";
success.upgrade="アップグレードしました。<br/>リブートを開始します。少しお待ちください...";
success.success_noreboot="設定に問題ありません。";
success.success_reboot=success.success_noreboot + "<br />リブートを開始します。少しお待ちください...";

success.alert_reset="全ての設定を初期値に復元しました。<br /><br />";
success.alert1="接続する前に次のチェックを行ってください：";
success.alert2="ルーターのIPアドレスを変更した場合、ネットワーク上のクライアントのアドレスをリリースあるいは更新する必要があります。";
success.alert3="WLANで接続している場合、ネットワークをジョインして、<em>続ける</em>を押してください。";

// *****************************************************        OLD PAGES       ************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

var dhcp = new Object();
dhcp.titl=" - DHCP Active IP Table";
dhcp.h2="DHCP Active IP Table";
dhcp.server="DHCP Server IP Address :";
dhcp.tclient="Client Host Name";

var donate = new Object();
donate.mb="You may also donate through the Moneybookers account mb@dd-wrt.com";
