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
function valid_range(I,_3,_4,M){
M1=unescape(M);
isdigit(I,M1);
d=parseInt(I.value,10);
if(!(d<=_4&&d>=_3)){
alert(M1+errmsg.err14+_3+" - "+_4+"].");
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
var _5=I.value.split(" ");
var _6=true;
var _7="";
while(_5.length>0){
var _8=new Object;
_8.value=_5.shift();
if(!valid_macs_17(_8)){
_6=false;
}else{
_7=_7+" "+_8.value;
}
}
while(_7.indexOf(" ")==0){
_7=_7.substr(1);
}
I.value=_7;
return _6;
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
function valid_email(_9,_a,db){
if(_9==""&&_a){
if(db){
alert("email address is mandatory");
}
return false;
}
if(_9==""){
return true;
}
var _b="/'\\ \";:?!()[]{}^|";
for(i=0;i<_b.length;i++){
if(_9.indexOf(_b.charAt(i),0)>-1){
if(db){
alert("email address contains invalid characters");
}
return false;
}
}
for(i=0;i<_9.length;i++){
if(_9.charCodeAt(i)>127){
if(db){
alert("email address contains non ascii characters.");
}
return false;
}
}
var _c=_9.indexOf("@",0);
if(_c==-1){
if(db){
alert("email address must contain an @");
}
return false;
}
if(_c==0){
if(db){
alert("email address must not start with @");
}
return false;
}
if(_9.indexOf("@",_c+1)>-1){
if(db){
alert("email address must contain only one @");
}
return false;
}
if(_9.indexOf(".",_c)==-1){
if(db){
alert("email address must contain a period in the domain name");
}
return false;
}
if(_9.indexOf("@.",0)!=-1){
if(db){
alert("period must not immediately follow @ in email address");
}
return false;
}
if(_9.indexOf(".@",0)!=-1){
if(db){
alert("period must not immediately precede @ in email address");
}
return false;
}
if(_9.indexOf("..",0)!=-1){
if(db){
alert("two periods must not be adjacent in email address");
}
return false;
}
var _d=_9.substring(_9.lastIndexOf(".")+1);
if(_d.length!=2&&_d!="com"&&_d!="net"&&_d!="org"&&_d!="edu"&&_d!="int"&&_d!="mil"&&_d!="gov"&_d!="arpa"&&_d!="biz"&&_d!="aero"&&_d!="name"&&_d!="coop"&&_d!="info"&&_d!="pro"&&_d!="museum"){
if(db){
alert("invalid primary domain in email address");
}
return false;
}
return true;
};
function valid_macs_17(I){
oldmac=I.value;
var _e=ignoreSpaces(oldmac);
if(_e==""){
return true;
}
var m=_e.split(":");
if(m.length!=6){
alert(errmsg.err21);
I.value=I.defaultValue;
return false;
}
var _f=oldmac.indexOf(":");
if(_f!=-1){
var _10=oldmac.substring(0,oldmac.length).split(":");
for(var i=0;i<_10.length;i++){
nameVal=_10[i];
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
function ignoreSpaces(_11){
var _12="";
_11=""+_11;
splitstring=_11.split(" ");
for(i=0;i<splitstring.length;i++){
_12+=splitstring[i];
}
return _12;
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
function valid_name(I,M,_13){
isascii(I,M);
var bbb=I.value.replace(/^\s*/,"");
var ccc=bbb.replace(/\s*$/,"");
I.value=ccc;
if(_13&SPACE_NO){
check_space(I,M);
}
};
function valid_mask(F,N,_14){
var _15=-1;
var _16=-1;
var m=new Array(4);
for(i=0;i<4;i++){
m[i]=eval(N+"_"+i).value;
}
if(m[0]=="0"&&m[1]=="0"&&m[2]=="0"&&m[3]=="0"){
if(_14&ZERO_NO){
alert(errmsg.err27);
return false;
}else{
if(_14&ZERO_OK){
return true;
}
}
}
if(m[0]=="255"&&m[1]=="255"&&m[2]=="255"&&m[3]=="255"){
if(_14&BCST_NO){
alert(errmsg.err27);
return false;
}else{
if(_14&BCST_OK){
return true;
}
}
}
for(i=3;i>=0;i--){
for(j=1;j<=8;j++){
if((m[i]%2)==0){
_15=(3-i)*8+j;
}else{
if(((m[i]%2)==1)&&_16==-1){
_16=(3-i)*8+j;
}
}
m[i]=Math.floor(m[i]/2);
}
}
if(_15>_16){
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
function closeWin(_17){
if(((_17!=null)&&(_17.close))||((_17!=null)&&(_17.closed==false))){
_17.close();
}
};
function valid_ip(F,N,M1,_18){
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
if(_18&ZERO_NO){
alert(M+errmsg.err31);
return false;
}
}
if((m[0]!="0"||m[1]!="0"||m[2]!="0")&&m[3]=="0"){
if(_18&MASK_NO){
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
var _19=I.value.split(".");
for(i=0;i<4;i++){
m[i]=parseInt(_19[i],10);
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
function createBar(w,h,_1a,_1b,_1c,_1d){
if(ie||w3c){
var t="<div class=\"progressbar\" id=\"_xpbar"+(++N)+"\" style=\"width:"+w+"px; height:"+h+"px;\">";
t+="<span class=\"progressbarblock\" id=\"blocks"+N+"\" style=\"left:-"+(h*2+1)+"px;\">";
for(var i=0;i<_1b;i++){
t+="<span class=\"progressbarblock\" style=\"left:-"+((h*i)+i)+"px; width:"+h+"px; height:"+h+"px; ";
t+=(ie)?"filter:alpha(opacity="+(100-i*(100/_1b))+")":"-Moz-opacity:"+((100-i*(100/_1b))/100);
t+="\"></span>";
}
t+="</span></div>";
document.write(t);
var bA=(ie)?document.all["blocks"+N]:document.getElementById("blocks"+N);
bA.bar=(ie)?document.all["_xpbar"+N]:document.getElementById("_xpbar"+N);
bA.blocks=_1b;
bA.N=N;
bA.w=w;
bA.h=h;
bA.speed=_1a;
bA.ctr=0;
bA.count=_1c;
bA.action=_1d;
bA.togglePause=togglePause;
bA.showBar=function(){
this.bar.style.visibility="visible";
};
bA.hideBar=function(){
this.bar.style.visibility="hidden";
};
bA.tid=setInterval("startBar("+N+")",_1a);
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
function change_style(id,_1e){
var _1f=document.getElementById(id);
_1f.className=_1e;
};
function Capture(obj){
document.write(obj);
};
function defined(val){
return (typeof val!="undefined");
};
function cleanTable(_20){
for(var i=_20.rows.length-1;i>0;i--){
_20.deleteRow(i);
}
};
function openHelpWindow(url){
var top=30;
var _21=Math.floor(screen.availWidth*0.66)-10;
var _22=Math.floor(screen.availWidth*0.33);
var _23=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("help/"+url,"DDWRT_Help","top="+top+",left="+_21+",width="+_22+",height="+_23+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
};
function openHelpWindowExt(url){
var top=30;
var _24=Math.floor(screen.availWidth*0.66)-10;
var _25=Math.floor(screen.availWidth*0.33);
var _26=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("http://www.dd-wrt.com/help/english/"+url,"DDWRT_Help","top="+top+",left="+_24+",width="+_25+",height="+_26+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
};
function openAboutWindow(){
var _27=750;
var _28=650;
var top=Math.floor((screen.availHeight-_28-10)/2);
var _29=Math.floor((screen.availWidth-_27)/2);
var win=window.open("About.htm","DDWRT_About","top="+top+",left="+_29+",width="+_27+",height="+_28+",resizable=no,scrollbars=no,statusbar=no");
win.focus();
};
function openWindow(url,_2a,_2b,_2c){
if(!_2c){
_2c=url.replace(/\.asp/,"");
}
var top=Math.floor((screen.availHeight-_2b-10)/2);
var _2d=Math.floor((screen.availWidth-_2a)/2);
var win=window.open(url,"DDWRT_"+_2c,"top="+top+",left="+_2d+",width="+_2a+",height="+_2b+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function setMeterBar(id,_2e,_2f){
if(isNaN(_2e)){
_2e=0;
}
_2e=Math.max(0,Math.min(100,Math.round(_2e)))+"%";
var _30=(typeof id=="string"?document.getElementById(id):id);
if(_30.firstChild){
_30.firstChild.childNodes[0].style.width=_2e;
_30.firstChild.childNodes[1].firstChild.data=_2e;
if(defined(_2f)){
_30.lastChild.data=_2f;
}
}else{
_30.innerHTML="<div class=\"meter\"><div class=\"bar\" style=\"width:"+_2e+";\"></div>"+"<div class=\"text\">"+_2e+"</div></div>"+(defined(_2f)?_2f:"");
}
};
function setElementContent(id,_31){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).innerHTML=_31;
};
function setElementVisible(id,_32){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).style.display=(_32?"":"none");
};
function setElementActive(_33,_34){
var _35=document.getElementsByName(_33);
if(!_35){
return;
}
for(var i=0;i<_35.length;i++){
_35[i].disabled=!_34;
}
};
function setElementsActive(_36,_37,_38){
if(!document.forms[0].elements[_36]||!document.forms[0].elements[_37]){
return;
}
var go=false;
for(var i=0;i<document.forms[0].elements.length;i++){
var _39=document.forms[0].elements[i].name;
if(!document.forms[0].elements[i].type||(_39!=_36&&!go)){
continue;
}
go=true;
document.forms[0].elements[i].disabled=!_38;
if(_39==_37){
break;
}
}
};
function addEvent(_3a,_3b,_3c){
if(_3a.addEventListener){
_3a.addEventListener(_3b,_3c,false);
}else{
if(_3a.attachEvent){
_3a.attachEvent("on"+_3b,_3c);
}
}
};
function removeEvent(_3d,_3e,_3f){
if(_3d.removeEventListener){
_3d.removeEventListener(_3e,_3f,false);
}else{
if(_3d.detachEvent){
_3d.detachEvent("on"+_3e,_3f);
}
}
};
function StatusUpdate(_40,_41){
var _42;
var _43;
var url=_40;
var _44=_41*1000;
var me=this;
var _45=new Object();
var _46=new Object();
this.start=function(){
if((!window.XMLHttpRequest&&!window.ActiveXObject)||_44==0){
return false;
}
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=true;
}
_43=setTimeout(me.doUpdate,_44);
return true;
};
this.stop=function(){
clearTimeout(_43);
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=false;
}
_42=null;
};
this.onUpdate=function(id,_47){
_45[id]=_47;
};
this.doUpdate=function(){
if(_42&&_42.readyState<4){
return;
}
if(window.XMLHttpRequest){
_42=new XMLHttpRequest();
}
if(window.ActiveXObject){
_42=new ActiveXObject("Microsoft.XMLHTTP");
}
_42.open("GET",url,true);
_42.onreadystatechange=function(){
if(_42.readyState<4||_42.status!=200){
return;
}
var _48=new Array();
var _49=/\{(\w+)::([^\}]*)\}/g;
while(result=_49.exec(_42.responseText)){
var key=result[1];
var _4a=result[2];
if(defined(_46[key])&&_46[key]==_4a){
continue;
}
_46[key]=_4a;
if(defined(_45[key])){
_48.push(_45[key]);
}
setElementContent(key,_4a);
}
for(var i=0;i<_48.length;i++){
(_48[i])(_46);
}
_43=setTimeout(me.doUpdate,_44);
};
_42.send("");
};
this.forceUpdate=function(){
this.stop();
this.doUpdate();
};
};
function StatusbarUpdate(){
var _4b;
var _4c;
var url="Statusinfo.live.asp";
var _4d=5000;
var me=this;
var _4e=new Object();
var _4f=new Object();
this.start=function(){
if(!window.XMLHttpRequest&&!window.ActiveXObject){
return false;
}
_4c=setTimeout(me.doUpdate,_4d);
return true;
};
this.stop=function(){
clearTimeout(_4c);
_4b=null;
};
this.onUpdate=function(id,_50){
_4e[id]=_50;
};
this.doUpdate=function(){
if(_4b&&_4b.readyState<4){
return;
}
if(window.XMLHttpRequest){
_4b=new XMLHttpRequest();
}
if(window.ActiveXObject){
_4b=new ActiveXObject("Microsoft.XMLHTTP");
}
_4b.open("GET",url,true);
_4b.onreadystatechange=function(){
if(_4b.readyState<4||_4b.status!=200){
return;
}
var _51=new Array();
var _52=/\{(\w+)::([^\}]*)\}/g;
while(result=_52.exec(_4b.responseText)){
var key=result[1];
var _53=result[2];
if(defined(_4f[key])&&_4f[key]==_53){
continue;
}
_4f[key]=_53;
if(defined(_4e[key])){
_51.push(_4e[key]);
}
setElementContent(key,_53);
}
for(var i=0;i<_51.length;i++){
(_51[i])(_4f);
}
_4c=setTimeout(me.doUpdate,_4d);
};
_4b.send("");
};
};
function apply(_54){
_54.submit();
for(i=0;i<_54.elements.length;i++){
if(defined(_54.elements[i].disabled)){
_54.elements[i].disabled=true;
}
}
if(_54.contents){
document.getElementById("contents").style.color="#999999";
}
};
function applytake(_55){
_55.action.value="ApplyTake";
_55.submit();
for(i=0;i<_55.elements.length;i++){
if(defined(_55.elements[i].disabled)){
_55.elements[i].disabled=true;
}
}
if(_55.contents){
document.getElementById("contents").style.color="#999999";
}
};
function applyupdate(_56){
_56.submit();
for(i=0;i<_56.elements.length;i++){
if(defined(_56.elements[i].disabled)){
_56.elements[i].disabled=true;
}
}
if(_56.contents){
document.getElementById("contents").style.color="#999999";
}
};
function getOUIFromMAC(mac){
var top=30;
var _57=Math.floor(screen.availWidth*0.66)-10;
var _58=700;
var _59=400;
var tab=new Array();
tab=mac.split(mac.substr(2,1));
var win=window.open("http://standards.ieee.org/cgi-bin/ouisearch?"+tab[0]+"-"+tab[1]+"-"+tab[2],"DDWRT_OUI_Search","top="+top+",left="+_57+",width="+_58+",height="+_59+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function openBW(_5a){
var top=30;
var _5b=Math.floor(screen.availWidth*0.66)-10;
var _5c=555;
var _5d=275;
var win=window.open("graph_if.svg?"+_5a,"Bandwidth","top="+top+",left="+_5b+",width="+_5c+",height="+_5d+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function getTimeOut(clk,_5e,_5f){
var _60=60;
var _61=(_60/5)-3;
var _62=2;
if(clk==125||clk==240){
_62=2.5;
}
if(_5e==1){
_62=_62*1.5;
}
if(_5f==1){
_62=_62*3;
}
if(_5f==2){
_62=_62*1.8;
}
this.wait_time=_62*_60*(125/clk);
this.scroll_count=this.wait_time/5-3;
};
function setElementMask(id,_63){
var _64=document.getElementById(id);
if(!_64){
return;
}
var val=_64.value;
var _65=_64.maxlength;
var _66=_64.size;
var _67=_64.onblur;
var _68=_64.parentNode;
var _69=_64.nextSibling;
var _6a=document.createElement("input");
_6a.setAttribute("value",val);
_6a.setAttribute("name",id);
_6a.setAttribute("id",id);
_6a.setAttribute("maxlength",_65);
_6a.setAttribute("size",_66);
_6a.setAttribute("onblur",_67);
if(_63==true){
_6a.setAttribute("type","text");
}else{
_6a.setAttribute("type","password");
}
_68.removeChild(_64);
_68.insertBefore(_6a,_69);
_6a.focus();
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
function show_layer_ext(obj,id,_6b){
if(!obj){
return;
}
if(_6b){
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
function toggle_layer_ext(obj,id1,id2,_6c){
show_layer_ext(obj,id1,_6c);
show_layer_ext(obj,id2,!_6c);
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
function DisplayDiv(_6d,evt,h,w,_6e){
var _6f=w;
var _70=h;
_6e=_6e.replace(/&lt;/gi,"<");
_6e=_6e.replace(/&gt;/gi,">");
if(document.all){
if(document.readyState=="complete"){
document.all.bulle.innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_6e+"</td></tr></table>";
document.all.bulle.style.pixelLeft=event.clientX+document.body.scrollLeft+_6f;
document.all.bulle.style.pixelTop=event.clientY+document.body.scrollTop+_70;
document.all.bulle.style.visibility="visible";
}
}else{
if(document.getElementById){
document.getElementById("bulle").innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_6e+"</td></tr></table>";
document.getElementById("bulle").style.left=evt.pageX+_6f+"px";
document.getElementById("bulle").style.top=evt.pageY+_70+"px";
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
function submitFooterButton(sub,res,reb,_71,ref,clo){
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
if(_71){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+_71+"\" onclick=\"window.location.reload();\">");
}
if(ref){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+sbutton.refres+"\" onclick=\"window.location.reload();\" />");
}
if(clo){
document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\""+sbutton.clos+"\" onclick=\"self.close();\" />");
}
};
function SortableTable(_72){
this.tbody=_72.getElementsByTagName("tbody");
this.thead=_72.getElementsByTagName("tbody");
this.getInnerText=function(el){
if(typeof (el.textContent)!="undefined"){
return el.textContent;
}
if(typeof (el.innerText)!="undefined"){
return el.innerText;
}
if(typeof (el.innerHTML)=="string"){
return el.innerHTML.replace(/<[^<>]+>/g,"");
}
};
this.getParent=function(el,_73){
if(el==null){
return null;
}else{
if(el.nodeType==1&&el.tagName.toLowerCase()==_73.toLowerCase()){
return el;
}else{
return this.getParent(el.parentNode,_73);
}
}
};
this.sort=function(_74){
var _75=_74.cellIndex;
var itm=this.getInnerText(this.tbody[0].rows[1].cells[_75]);
var _76=this.sortCaseInsensitive;
if(itm.replace(/^\s+|\s+$/g,"").match(/^[\d]+$/)){
_76=this.sortNumeric;
}
this.sortColumnIndex=_75;
var _77=new Array();
for(j=1;j<this.tbody[0].rows.length;j++){
_77[j-1]=this.tbody[0].rows[j];
}
_77.sort(_76);
if(_74.getAttribute("sortdir")=="down"){
_77.reverse();
_74.setAttribute("sortdir","up");
}else{
_74.setAttribute("sortdir","down");
}
for(i=1;i<_77.length;i++){
this.tbody[0].appendChild(_77[i]);
}
};
this.sortCaseInsensitive=function(a,b){
aa=_78.getInnerText(a.cells[_78.sortColumnIndex]).toLowerCase();
bb=_78.getInnerText(b.cells[_78.sortColumnIndex]).toLowerCase();
if(aa==bb){
return 0;
}
if(aa<bb){
return -1;
}
return 1;
};
this.sortNumeric=function(a,b){
aa=parseFloat(_78.getInnerText(a.cells[_78.sortColumnIndex]));
if(isNaN(aa)){
aa=0;
}
bb=parseFloat(_78.getInnerText(b.cells[_78.sortColumnIndex]));
if(isNaN(bb)){
bb=0;
}
return aa-bb;
};
var _78=this;
var _79=this.thead;
if(!(this.tbody&&this.tbody[0].rows&&this.tbody[0].rows.length>0)){
return;
}
if(_79&&_79[0].rows&&_79[0].rows.length>0){
var _7a=_79[0].rows[0];
}else{
return;
}
for(var i=0;i<_7a.cells.length;i++){
_7a.cells[i].sTable=this;
_7a.cells[i].onclick=function(){
this.sTable.sort(this);
return false;
};
}
};

