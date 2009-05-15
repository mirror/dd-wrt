var op=(navigator.userAgent.indexOf("Opera")!=-1&&document.getElementById);
var ie4=(document.all);
var ns4=(document.layers);
var ns6=(!document.all&&document.getElementById);
var ZERO_NO=1;
var ZERO_OK=2;
var MASK_NO=4;
var MASK_OK=8;
var BCST_NO=16;
var BCST_OK=32;
var SPACE_NO=1;
var SPACE_OK=2;
function choose_enable(_1){
if(!_1){
return;
}
_1.disabled=false;
if(!ns4){
_1.style.backgroundColor="";
}
};
function choose_disable(_2){
if(!_2){
return;
}
_2.disabled=true;
if(!ns4){
_2.style.backgroundColor="#e0e0e0";
}
};
function check_action(I,N){
if(ns4){
if(N==0){
if(EN_DIS==1){
I.focus();
}else{
I.blur();
}
}else{
if(N==1){
if(EN_DIS1==1){
I.focus();
}else{
I.blur();
}
}else{
if(N==2){
if(EN_DIS2==1){
I.focus();
}else{
I.blur();
}
}else{
if(N==3){
if(EN_DIS3==1){
I.focus();
}else{
I.blur();
}
}
}
}
}
}
};
function check_action1(I,T,N){
if(ns4){
if(N==0){
if(EN_DIS==1){
I.focus();
}else{
I.value=I.defaultChecked;
}
}
if(N==1){
if(EN_DIS1==1){
I.focus();
}else{
I.value=I.defaultChecked;
}
}
}
};
function valid_range(I,_9,_a,M){
M1=unescape(M);
isdigit(I,M1);
d=parseInt(I.value,10);
if(!(d<=_a&&d>=_9)){
alert(M1+errmsg.err14+_9+" - "+_a+"].");
I.value=I.defaultValue;
}else{
I.value=d;
}
};
function valid_psk_length(I){
if(I.value==""){
return true;
}
if(I.value.length<8||I.value.length>64){
alert(errmsg.err39);
I.value=I.defaultValue;
}
return true;
};
function valid_macs_all(I){
if(I.value==""){
return true;
}
if(I.value.length==12){
valid_macs_12(I);
}else{
if(I.value.length==17){
valid_macs_17(I);
}else{
alert(errmsg.err5);
I.value=I.defaultValue;
}
}
return true;
};
function valid_macs_list(I){
if(I.value==""){
return true;
}
I.value=I.value.replace("\n"," ");
var _f=I.value.split(" ");
var ret=true;
var _11="";
while(_f.length>0){
var mac=new Object;
mac.value=_f.shift();
if(!valid_macs_17(mac)){
ret=false;
}else{
_11=_11+" "+mac.value;
}
}
while(_11.indexOf(" ")==0){
_11=_11.substr(1);
}
I.value=_11;
return ret;
};
function valid_mac(I,T){
var m1,m2=0;
if(I.value.length==1){
I.value="0"+I.value;
}
m1=parseInt(I.value.charAt(0),16);
m2=parseInt(I.value.charAt(1),16);
if(isNaN(m1)||isNaN(m2)){
alert(errmsg.err15);
I.value=I.defaultValue;
}
I.value=I.value.toUpperCase();
if(T==0){
if((m2&1)==1){
alert(errmsg.err16);
I.value=I.defaultValue;
}
}
};
function valid_macs_12(I){
var m,m3;
if(I.value==""){
return true;
}
if(I.value.length==12){
for(i=0;i<12;i++){
m=parseInt(I.value.charAt(i),16);
if(isNaN(m)){
break;
}
}
if(i!=12){
alert(errmsg.err17);
I.value=I.defaultValue;
}
}else{
alert(errmsg.err18);
I.value=I.defaultValue;
}
I.value=I.value.toUpperCase();
if(I.value=="FFFFFFFFFFFF"){
alert(errmsg.err19);
I.value=I.defaultValue;
}
m3=I.value.charAt(1);
if((m3&1)==1){
alert(errmsg.err16);
I.value=I.defaultValue;
}
return true;
};
function valid_email(_1a,man,db){
if(_1a==""&&man){
if(db){
alert("email address is mandatory");
}
return false;
}
if(_1a==""){
return true;
}
var _1d="/'\\ \";:?!()[]{}^|";
for(i=0;i<_1d.length;i++){
if(_1a.indexOf(_1d.charAt(i),0)>-1){
if(db){
alert("email address contains invalid characters");
}
return false;
}
}
for(i=0;i<_1a.length;i++){
if(_1a.charCodeAt(i)>127){
if(db){
alert("email address contains non ascii characters.");
}
return false;
}
}
var _1e=_1a.indexOf("@",0);
if(_1e==-1){
if(db){
alert("email address must contain an @");
}
return false;
}
if(_1e==0){
if(db){
alert("email address must not start with @");
}
return false;
}
if(_1a.indexOf("@",_1e+1)>-1){
if(db){
alert("email address must contain only one @");
}
return false;
}
if(_1a.indexOf(".",_1e)==-1){
if(db){
alert("email address must contain a period in the domain name");
}
return false;
}
if(_1a.indexOf("@.",0)!=-1){
if(db){
alert("period must not immediately follow @ in email address");
}
return false;
}
if(_1a.indexOf(".@",0)!=-1){
if(db){
alert("period must not immediately precede @ in email address");
}
return false;
}
if(_1a.indexOf("..",0)!=-1){
if(db){
alert("two periods must not be adjacent in email address");
}
return false;
}
var _1f=_1a.substring(_1a.lastIndexOf(".")+1);
if(_1f.length!=2&&_1f!="com"&&_1f!="net"&&_1f!="org"&&_1f!="edu"&&_1f!="int"&&_1f!="mil"&&_1f!="gov"&_1f!="arpa"&&_1f!="biz"&&_1f!="aero"&&_1f!="name"&&_1f!="coop"&&_1f!="info"&&_1f!="pro"&&_1f!="museum"){
if(db){
alert("invalid primary domain in email address");
}
return false;
}
return true;
};
function valid_macs_17(I){
oldmac=I.value;
var mac=ignoreSpaces(oldmac);
if(mac==""){
return true;
}
var m=mac.split(":");
if(m.length!=6){
alert(errmsg.err21);
I.value=I.defaultValue;
return false;
}
var idx=oldmac.indexOf(":");
if(idx!=-1){
var _24=oldmac.substring(0,oldmac.length).split(":");
for(var i=0;i<_24.length;i++){
nameVal=_24[i];
len=nameVal.length;
if(len<1||len>2){
alert(errmsg.err22);
I.value=I.defaultValue;
return false;
}
for(iln=0;iln<len;iln++){
ch=nameVal.charAt(iln).toLowerCase();
if(ch>="0"&&ch<="9"||ch>="a"&&ch<="f"){
}else{
alert(errmsg.err23+nameVal+errmsg.err24+oldmac+".");
I.value=I.defaultValue;
return false;
}
}
}
}
I.value=I.value.toUpperCase();
if(I.value=="FF:FF:FF:FF:FF:FF"){
alert(errmsg.err19);
I.value=I.defaultValue;
}
m3=I.value.charAt(1);
if((m3&1)==1){
alert(errmsg.err16);
I.value=I.defaultValue;
}
return true;
};
function ignoreSpaces(_26){
var _27="";
_26=""+_26;
splitstring=_26.split(" ");
for(i=0;i<splitstring.length;i++){
_27+=splitstring[i];
}
return _27;
};
function check_space(I,M1){
M=unescape(M1);
for(i=0;i<I.value.length;i++){
ch=I.value.charAt(i);
if(ch==" "){
alert(M+errmsg.err34);
I.value=I.defaultValue;
return false;
}
}
return true;
};
function valid_key(I,l){
var m;
if(I.value.length==l*2){
for(i=0;i<l*2;i++){
m=parseInt(I.value.charAt(i),16);
if(isNaN(m)){
break;
}
}
if(i!=l*2){
alert(errmsg.err25);
I.value=I.defaultValue;
}
}else{
alert(errmsg.err26);
I.value=I.defaultValue;
}
};
function valid_name(I,M,_2f){
isascii(I,M);
var bbb=I.value.replace(/^\s*/,"");
var ccc=bbb.replace(/\s*$/,"");
I.value=ccc;
if(_2f&SPACE_NO){
check_space(I,M);
}
};
function valid_mask(F,N,_34){
var _35=-1;
var _36=-1;
var m=new Array(4);
for(i=0;i<4;i++){
m[i]=eval(N+"_"+i).value;
}
if(m[0]=="0"&&m[1]=="0"&&m[2]=="0"&&m[3]=="0"){
if(_34&ZERO_NO){
alert(errmsg.err27);
return false;
}else{
if(_34&ZERO_OK){
return true;
}
}
}
if(m[0]=="255"&&m[1]=="255"&&m[2]=="255"&&m[3]=="255"){
if(_34&BCST_NO){
alert(errmsg.err27);
return false;
}else{
if(_34&BCST_OK){
return true;
}
}
}
for(i=3;i>=0;i--){
for(j=1;j<=8;j++){
if((m[i]%2)==0){
_35=(3-i)*8+j;
}else{
if(((m[i]%2)==1)&&_36==-1){
_36=(3-i)*8+j;
}
}
m[i]=Math.floor(m[i]/2);
}
}
if(_35>_36){
alert(errmsg.err27);
return false;
}
return true;
};
function isdigit(I,M){
if(I.value.charAt(0)=="-"){
i=1;
}else{
i=0;
}
for(i;i<I.value.length;i++){
ch=I.value.charAt(i);
if(ch<"0"||ch>"9"){
alert(M+errmsg.err28);
I.value=I.defaultValue;
return false;
}
}
return true;
};
function isascii(I,M){
for(i=0;i<I.value.length;i++){
ch=I.value.charAt(i);
if(ch<" "||ch>"~"){
alert(M+errmsg.err29);
I.value=I.defaultValue;
return false;
}
}
return true;
};
function isxdigit(I,M){
for(i=0;i<I.value.length;i++){
ch=I.value.charAt(i).toLowerCase();
if(ch>="0"&&ch<="9"||ch>="a"&&ch<="f"){
}else{
alert(M+errmsg.err30);
I.value=I.defaultValue;
return false;
}
}
return true;
};
function closeWin(_3e){
if(((_3e!=null)&&(_3e.close))||((_3e!=null)&&(_3e.closed==false))){
_3e.close();
}
};
function valid_ip(F,N,M1,_42){
var m=new Array(4);
M=unescape(M1);
for(i=0;i<4;i++){
m[i]=eval(N+"_"+i).value;
}
if(m[0]==127||m[0]==224){
alert(M+errmsg.err31);
return false;
}
if(m[0]=="0"&&m[1]=="0"&&m[2]=="0"&&m[3]=="0"){
if(_42&ZERO_NO){
alert(M+errmsg.err31);
return false;
}
}
if((m[0]!="0"||m[1]!="0"||m[2]!="0")&&m[3]=="0"){
if(_42&MASK_NO){
alert(M+errmsg.err31);
return false;
}
}
return true;
};
function valid_ip_str(I,M){
if(I.value==""||I.value==" "){
return true;
}
var m=new Array(4);
var _47=I.value.split(".");
for(i=0;i<4;i++){
m[i]=parseInt(_47[i],10);
if(isNaN(m[i])){
alert(M+errmsg.err31);
I.value=I.defaultValue;
return false;
}
}
if(m[0]==127||m[0]==224){
alert(M+errmsg.err31);
I.value=I.defaultValue;
return false;
}
if((m[0]>"255"||m[1]>"255"||m[2]>"255")&&m[3]>"255"){
alert(M+errmsg.err31);
I.value=I.defaultValue;
return false;
}
return true;
};
function valid_ip_gw(F,I,N,G){
var IP=new Array(4);
var NM=new Array(4);
var GW=new Array(4);
for(i=0;i<4;i++){
IP[i]=eval(I+"_"+i).value;
}
for(i=0;i<4;i++){
NM[i]=eval(N+"_"+i).value;
}
for(i=0;i<4;i++){
GW[i]=eval(G+"_"+i).value;
}
for(i=0;i<4;i++){
if((IP[i]&NM[i])!=(GW[i]&NM[i])){
alert(errmsg.err32);
return false;
}
}
if((IP[0]==GW[0])&&(IP[1]==GW[1])&&(IP[2]==GW[2])&&(IP[3]==GW[3])){
alert(errmsg.err33);
return false;
}
return true;
};
function fix_cr(F){
var re1=new RegExp("&#13;&#10;","gi");
var re2=new RegExp("&#13;","gi");
var re3=new RegExp("&#10;","gi");
var re4=new RegExp("&#38;","gi");
var re5=new RegExp("&#34;","gi");
var re6=new RegExp("&#62;","gi");
var re7=new RegExp("&#60;","gi");
var re8=new RegExp("&#92;","gi");
var re9=new RegExp("&#39;","gi");
var a=F.replace(re1,"\n");
var b=a.replace(re2,"\n");
var c=b.replace(re3,"\n");
var d=c.replace(re4,"&");
var e=d.replace(re5,"\"");
var f=e.replace(re6,">");
var g=f.replace(re7,"<");
var h=g.replace(re8,"'");
var i=h.replace(re9,"\\");
return i;
};
var w3c=(document.getElementById)?true:false;
var ie=(document.all)?true:false;
var N=-1;
function createBar(w,h,_64,_65,_66,_67){
if(ie||w3c){
var t="<div class=\"progressbar\" id=\"_xpbar"+(++N)+"\" style=\"width:"+w+"px; height:"+h+"px;\">";
t+="<span class=\"progressbarblock\" id=\"blocks"+N+"\" style=\"left:-"+(h*2+1)+"px;\">";
for(var i=0;i<_65;i++){
t+="<span class=\"progressbarblock\" style=\"left:-"+((h*i)+i)+"px; width:"+h+"px; height:"+h+"px; ";
t+=(ie)?"filter:alpha(opacity="+(100-i*(100/_65))+")":"-Moz-opacity:"+((100-i*(100/_65))/100);
t+="\"></span>";
}
t+="</span></div>";
document.write(t);
var bA=(ie)?document.all["blocks"+N]:document.getElementById("blocks"+N);
bA.bar=(ie)?document.all["_xpbar"+N]:document.getElementById("_xpbar"+N);
bA.blocks=_65;
bA.N=N;
bA.w=w;
bA.h=h;
bA.speed=_64;
bA.ctr=0;
bA.count=_66;
bA.action=_67;
bA.togglePause=togglePause;
bA.showBar=function(){
this.bar.style.visibility="visible";
};
bA.hideBar=function(){
this.bar.style.visibility="hidden";
};
bA.tid=setInterval("startBar("+N+")",_64);
return bA;
}
return false;
};
function startBar(bn){
var t=(ie)?document.all["blocks"+bn]:document.getElementById("blocks"+bn);
if(parseInt(t.style.left)+t.h+1-(t.blocks*t.h+t.blocks)>t.w){
t.style.left=-(t.h*2+1)+"px";
t.ctr++;
if(t.ctr>=t.count){
eval(t.action);
t.ctr=0;
}
}else{
t.style.left=(parseInt(t.style.left)+t.h+1)+"px";
}
};
function togglePause(){
if(this.tid==0){
this.tid=setInterval("startBar("+this.N+")",this.speed);
}else{
clearInterval(this.tid);
this.tid=0;
}
};
function change_style(id,_6e){
var _6f=document.getElementById(id);
_6f.className=_6e;
};
function Capture(obj){
document.write(obj);
};
function defined(val){
return (typeof val!="undefined");
};
function cleanTable(_72){
for(var i=_72.rows.length-1;i>0;i--){
_72.deleteRow(i);
}
};
function openHelpWindow(url){
var top=30;
var _76=Math.floor(screen.availWidth*0.66)-10;
var _77=Math.floor(screen.availWidth*0.33);
var _78=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("help/"+url,"DDWRT_Help","top="+top+",left="+_76+",width="+_77+",height="+_78+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
};
function openHelpWindowExt(url){
var top=30;
var _7c=Math.floor(screen.availWidth*0.66)-10;
var _7d=Math.floor(screen.availWidth*0.33);
var _7e=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("http://www.dd-wrt.com/help/english/"+url,"DDWRT_Help","top="+top+",left="+_7c+",width="+_7d+",height="+_7e+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
};
function openAboutWindow(){
var _80=750;
var _81=650;
var top=Math.floor((screen.availHeight-_81-10)/2);
var _83=Math.floor((screen.availWidth-_80)/2);
var win=window.open("About.htm","DDWRT_About","top="+top+",left="+_83+",width="+_80+",height="+_81+",resizable=no,scrollbars=no,statusbar=no");
win.focus();
};
function openWindow(url,_86,_87,_88){
if(!_88){
_88=url.replace(/\.asp/,"");
}
var top=Math.floor((screen.availHeight-_87-10)/2);
var _8a=Math.floor((screen.availWidth-_86)/2);
var win=window.open(url,"DDWRT_"+_88,"top="+top+",left="+_8a+",width="+_86+",height="+_87+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function setMeterBar(id,_8d,_8e){
if(isNaN(_8d)){
_8d=0;
}
_8d=Math.max(0,Math.min(100.0,Math.round(_8d)))+"%";
var _8f=(typeof id=="string"?document.getElementById(id):id);
if(_8f.firstChild){
_8f.firstChild.childNodes[0].style.width=_8d;
_8f.firstChild.childNodes[1].firstChild.data=_8d;
if(defined(_8e)){
_8f.lastChild.data=_8e;
}
}else{
_8f.innerHTML="<div class=\"meter\"><div class=\"bar\" style=\"width:"+_8d+";\"></div>"+"<div class=\"text\">"+_8d+"</div></div>"+(defined(_8e)?_8e:"");
}
};
function setElementContent(id,_91){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).innerHTML=_91;
};
function setElementVisible(id,_93){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).style.display=(_93?"":"none");
};
function setElementActive(_94,_95){
var _96=document.getElementsByName(_94);
if(!_96){
return;
}
for(var i=0;i<_96.length;i++){
_96[i].disabled=!_95;
}
};
function setElementsActive(_98,_99,_9a){
if(!document.forms[0].elements[_98]||!document.forms[0].elements[_99]){
return;
}
var go=false;
for(var i=0;i<document.forms[0].elements.length;i++){
var _9d=document.forms[0].elements[i].name;
if(!document.forms[0].elements[i].type||(_9d!=_98&&!go)){
continue;
}
go=true;
document.forms[0].elements[i].disabled=!_9a;
if(_9d==_99){
break;
}
}
};
function addEvent(_9e,_9f,_a0){
if(_9e.addEventListener){
_9e.addEventListener(_9f,_a0,false);
}else{
if(_9e.attachEvent){
_9e.attachEvent("on"+_9f,_a0);
}
}
};
function removeEvent(_a1,_a2,_a3){
if(_a1.removeEventListener){
_a1.removeEventListener(_a2,_a3,false);
}else{
if(_a1.detachEvent){
_a1.detachEvent("on"+_a2,_a3);
}
}
};
function StatusUpdate(_a4,_a5){
var _a6;
var _a7;
var url=_a4;
var _a9=_a5*1000;
var me=this;
var _ab=new Object();
var _ac=new Object();
this.start=function(){
if((!window.XMLHttpRequest&&!window.ActiveXObject)||_a9==0){
return false;
}
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=true;
}
_a7=setTimeout(me.doUpdate,_a9);
return true;
};
this.stop=function(){
clearTimeout(_a7);
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=false;
}
_a6=null;
};
this.onUpdate=function(id,_ae){
_ab[id]=_ae;
};
this.doUpdate=function(){
if(_a6&&_a6.readyState<4){
return;
}
if(window.XMLHttpRequest){
_a6=new XMLHttpRequest();
}
if(window.ActiveXObject){
_a6=new ActiveXObject("Microsoft.XMLHTTP");
}
_a6.open("GET",url,true);
_a6.onreadystatechange=function(){
if(_a6.readyState<4||_a6.status!=200){
return;
}
var _af=new Array();
var _b0=/\{(\w+)::([^\}]*)\}/g;
while(result=_b0.exec(_a6.responseText)){
var key=result[1];
var _b2=result[2];
if(defined(_ac[key])&&_ac[key]==_b2){
continue;
}
_ac[key]=_b2;
if(defined(_ab[key])){
_af.push(_ab[key]);
}
setElementContent(key,_b2);
}
for(var i=0;i<_af.length;i++){
(_af[i])(_ac);
}
_a7=setTimeout(me.doUpdate,_a9);
};
_a6.send("");
};
this.forceUpdate=function(){
this.stop();
this.doUpdate();
};
};
function StatusbarUpdate(){
var _b4;
var _b5;
var url="Statusinfo.live.asp";
var _b7=5000;
var me=this;
var _b9=new Object();
var _ba=new Object();
this.start=function(){
if(!window.XMLHttpRequest&&!window.ActiveXObject){
return false;
}
_b5=setTimeout(me.doUpdate,_b7);
return true;
};
this.stop=function(){
clearTimeout(_b5);
_b4=null;
};
this.onUpdate=function(id,_bc){
_b9[id]=_bc;
};
this.doUpdate=function(){
if(_b4&&_b4.readyState<4){
return;
}
if(window.XMLHttpRequest){
_b4=new XMLHttpRequest();
}
if(window.ActiveXObject){
_b4=new ActiveXObject("Microsoft.XMLHTTP");
}
_b4.open("GET",url,true);
_b4.onreadystatechange=function(){
if(_b4.readyState<4||_b4.status!=200){
return;
}
var _bd=new Array();
var _be=/\{(\w+)::([^\}]*)\}/g;
while(result=_be.exec(_b4.responseText)){
var key=result[1];
var _c0=result[2];
if(defined(_ba[key])&&_ba[key]==_c0){
continue;
}
_ba[key]=_c0;
if(defined(_b9[key])){
_bd.push(_b9[key]);
}
setElementContent(key,_c0);
}
for(var i=0;i<_bd.length;i++){
(_bd[i])(_ba);
}
_b5=setTimeout(me.doUpdate,_b7);
};
_b4.send("");
};
};
function apply(_c2){
_c2.submit();
for(i=0;i<_c2.elements.length;i++){
if(defined(_c2.elements[i].disabled)){
_c2.elements[i].disabled=true;
}
}
if(_c2.contents){
document.getElementById("contents").style.color="#999999";
}
};
function applytake(_c3){
_c3.action.value="ApplyTake";
_c3.submit();
for(i=0;i<_c3.elements.length;i++){
if(defined(_c3.elements[i].disabled)){
_c3.elements[i].disabled=true;
}
}
if(_c3.contents){
document.getElementById("contents").style.color="#999999";
}
};
function applyupdate(_c4){
_c4.submit();
for(i=0;i<_c4.elements.length;i++){
if(defined(_c4.elements[i].disabled)){
_c4.elements[i].disabled=true;
}
}
if(_c4.contents){
document.getElementById("contents").style.color="#999999";
}
};
function getOUIFromMAC(mac){
var top=30;
var _c7=Math.floor(screen.availWidth*0.66)-10;
var _c8=700;
var _c9=400;
var tab=new Array();
tab=mac.split(mac.substr(2,1));
var win=window.open("http://standards.ieee.org/cgi-bin/ouisearch?"+tab[0]+"-"+tab[1]+"-"+tab[2],"DDWRT_OUI_Search","top="+top+",left="+_c7+",width="+_c8+",height="+_c9+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function openBW(iface){
var top=30;
var left=Math.floor(screen.availWidth*.66)-10;
var width=555
var height=275
var win=window.open("graph_if.svg?"+iface,'Bandwidth','top='+top+',left='+left+',width='+width+',height='+height+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
}
function getTimeOut(clk,_cd,_ce){
var _cf=60;
var _d0=(_cf/5)-3;
var _d1=1.5;
if(clk==125||clk==240){
_d1=2.0;
}
if(_cd==1){
_d1=_d1*1.5;
}
if(_ce==1){
_d1=_d1*3;
}
if(_ce==2){
_d1=_d1*1.8;
}
this.wait_time=_d1*_cf*(125/clk);
this.scroll_count=this.wait_time/5-3;
};
function setElementMask(id,_d3){
var _d4=document.getElementById(id);
if(!_d4){
return;
}
var val=_d4.value;
var _d6=_d4.maxlength;
var _d7=_d4.size;
var _d8=_d4.onblur;
var _d9=_d4.parentNode;
var _da=_d4.nextSibling;
var _db=document.createElement("input");
_db.setAttribute("value",val);
_db.setAttribute("name",id);
_db.setAttribute("id",id);
_db.setAttribute("maxlength",_d6);
_db.setAttribute("size",_d7);
_db.setAttribute("onblur",_d8);
if(_d3==true){
_db.setAttribute("type","text");
}else{
_db.setAttribute("type","password");
}
_d9.removeChild(_d4);
_d9.insertBefore(_db,_da);
_db.focus();
};
var windo={getWindoSize:function(){
if(window.innerHeight){
return {height:window.innerHeight,width:window.innerWidth};
}else{
if(document.documentElement&&document.documentElement.clientHeight){
return {height:document.documentElement.clientHeight,width:document.documentElement.clientWidth};
}
}
return {Height:document.body.clientHeight,width:document.body.clientWidth};
}};
function show_layer_ext(obj,id,_de){
if(!obj){
return;
}
if(_de){
visibility_style="visible";
display_style="block";
}else{
visibility_style="hidden";
display_style="none";
}
if(ie4){
eval("document.all."+id+".style.visibility='"+visibility_style+"'");
eval("document.all."+id+".style.display='"+display_style+"'");
}
if(ns4){
eval("document."+id+".visibility='"+visibility_style+"'");
eval("document."+id+".display='"+display_style+"'");
}
if(ns6||op){
eval("document.getElementById('"+id+"').style.visibility='"+visibility_style+"'");
eval("document.getElementById('"+id+"').style.display='"+display_style+"'");
}
};
function toggle_layer_ext(obj,id1,id2,_e2){
show_layer_ext(obj,id1,_e2);
show_layer_ext(obj,id2,!_e2);
};
function lgout(){
document.forms[0].action.value="Logout";
document.forms[0].submit();
};
function comma(n){
n=""+n;
var p=n;
while((n=n.replace(/(\d+)(\d{3})/g,"$1,$2"))!=p){
p=n;
}
return n;
};
function scaleSize(num){
if(isNaN(num*=1)){
return "-";
}
if(num<=9999){
return ""+num;
}
var s=-1;
do{
num/=1024;
++s;
}while((num>9999)&&(s<2));
return comma(num.toFixed(2))+"<small> "+(["KB","MB","GB"])[s]+"</small>";
};
function DisplayDiv(_e7,evt,h,w,_eb){
var _ec=w;
var _ed=h;
_eb=_eb.replace(/&lt;/gi,"<");
_eb=_eb.replace(/&gt;/gi,">");
if(document.all){
if(document.readyState=="complete"){
document.all.bulle.innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_eb+"</td></tr></table>";
document.all.bulle.style.pixelLeft=event.clientX+document.body.scrollLeft+_ec;
document.all.bulle.style.pixelTop=event.clientY+document.body.scrollTop+_ed;
document.all.bulle.style.visibility="visible";
}
}else{
if(document.getElementById){
document.getElementById("bulle").innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_eb+"</td></tr></table>";
document.getElementById("bulle").style.left=evt.pageX+_ec+"px";
document.getElementById("bulle").style.top=evt.pageY+_ed+"px";
document.getElementById("bulle").style.visibility="visible";
}
}
};
function unDisplayDiv(){
if(document.all){
document.all.bulle.style.visibility="hidden";
}else{
if(document.layers){
document.bulle.visibility="hidden";
}else{
if(document.getElementById){
document.getElementById("bulle").style.visibility="hidden";
}
}
}
};
function submitFooterButton(sub,res,reb,_f1,ref,clo){
if(sub){
document.write("<input title=\""+sbutton.savetitle+"\" class=\"button\" type=\"button\" name=\"save_button\" value=\""+sbutton.save+"\" onclick=\"to_submit(this.form);\" />");
}
if(sub){
document.write("<input title=\""+sbutton.applytitle+"\" class=\"button\" type=\"button\" name=\"apply_button\" value=\""+sbutton.apply+"\" onclick=\"to_apply(this.form);\" />");
}
if(res){
document.write("<input title=\""+sbutton.canceltitle+"\" class=\"button\" type=\"button\" name=\"reset_button\" value=\""+sbutton.cancel+"\" onclick=\"window.location.reload();\" />");
}
if(reb){
document.write("<input class=\"button\" type=\"button\" name=\"reboot_button\" value=\""+sbutton.reboot+"\" onclick=\"to_reboot(this.form);\" />");
}
if(_f1){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+_f1+"\" onclick=\"window.location.reload();\">");
}
if(ref){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+sbutton.refres+"\" onclick=\"window.location.reload();\" />");
}
if(clo){
document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\""+sbutton.clos+"\" onclick=\"self.close();\" />");
}
};

function SortableTable (tableEl) {
 
	this.tbody = tableEl.getElementsByTagName('tbody');
	this.thead = tableEl.getElementsByTagName('tbody');
 
	this.getInnerText = function (el) {
		if (typeof(el.textContent) != 'undefined') return el.textContent;
		if (typeof(el.innerText) != 'undefined') return el.innerText;
		if (typeof(el.innerHTML) == 'string') return el.innerHTML.replace(/<[^<>]+>/g,'');
	}
 
	this.getParent = function (el, pTagName) {
		if (el == null) return null;
		else if (el.nodeType == 1 && el.tagName.toLowerCase() == pTagName.toLowerCase())
			return el;
		else
			return this.getParent(el.parentNode, pTagName);
	}
 
	this.sort = function (cell) {
 
	    var column = cell.cellIndex;
	    var itm = this.getInnerText(this.tbody[0].rows[1].cells[column]);
		var sortfn = this.sortCaseInsensitive;
 
		if (itm.replace(/^\s+|\s+$/g,"").match(/^[\d]+$/)) sortfn = this.sortNumeric;
 
		this.sortColumnIndex = column;
 
	    var newRows = new Array();
	    for (j = 1; j < this.tbody[0].rows.length; j++) {
			newRows[j - 1] = this.tbody[0].rows[j];
		}
 
		newRows.sort(sortfn);
 
		if (cell.getAttribute("sortdir") == 'down') {
			newRows.reverse();
			cell.setAttribute('sortdir','up');
		} else {
			cell.setAttribute('sortdir','down');
		}
 
		for (i = 1; i < newRows.length; i++) {
			this.tbody[0].appendChild(newRows[i]);
		}
 
	}
 
	this.sortCaseInsensitive = function(a,b) {
		aa = thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]).toLowerCase();
		bb = thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]).toLowerCase();
		if (aa==bb) return 0;
		if (aa<bb) return -1;
		return 1;
	}
 
	this.sortNumeric = function(a,b) {
		aa = parseFloat(thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]));
		if (isNaN(aa)) aa = 0;
		bb = parseFloat(thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]));
		if (isNaN(bb)) bb = 0;
		return aa-bb;
	}
 
	var thisObject = this;
	var sortSection = this.thead;
 
	if (!(this.tbody && this.tbody[0].rows && this.tbody[0].rows.length > 0)) return;
 
	if (sortSection && sortSection[0].rows && sortSection[0].rows.length > 0) {
		var sortRow = sortSection[0].rows[0];
	} else {
		return;
	}
 
	for (var i=0; i<sortRow.cells.length; i++) {
		sortRow.cells[i].sTable = this;
		sortRow.cells[i].onclick = function () {
			this.sTable.sort(this);
			return false;
		}
	}
 
}

