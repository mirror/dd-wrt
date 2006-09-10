op=(navigator.userAgent.indexOf("Opera")!=-1&&document.getElementById);
ie4=(document.all);
ns4=(document.layers);
ns6=(!document.all&&document.getElementById);
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
}
function choose_disable(_2){
if(!_2){
return;
}
_2.disabled=true;
if(!ns4){
_2.style.backgroundColor="#e0e0e0";
}
}
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
}
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
}
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
}
function valid_macs_all(I){
if(I.value==""){
return true;
}else{
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
}
}
function valid_macs_list(I){
if(I.value==""){
return true;
}
I.value=I.value.replace("\n"," ");
var _e=I.value.split(" ");
var _f=true;
var _10="";
while(_e.length>0){
var mac=new Object;
mac.value=_e.shift();
if(!valid_macs_17(mac)){
_f=false;
}else{
_10=_10+" "+mac.value;
}
}
while(_10.indexOf(" ")==0){
_10=_10.substr(1);
}
I.value=_10;
return _f;
}
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
}
function valid_macs_12(I){
var m,m3;
if(I.value==""){
return true;
}else{
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
}
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
var _1b=oldmac.substring(0,oldmac.length).split(":");
for(var i=0;i<_1b.length;i++){
nameVal=_1b[i];
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
}
function ignoreSpaces(_1d){
var _1e="";
_1d=""+_1d;
splitstring=_1d.split(" ");
for(i=0;i<splitstring.length;i++){
_1e+=splitstring[i];
}
return _1e;
}
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
}
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
}
function valid_name(I,M,_26){
isascii(I,M);
var bbb=I.value.replace(/^\s*/,"");
var ccc=bbb.replace(/\s*$/,"");
I.value=ccc;
if(_26&SPACE_NO){
check_space(I,M);
}
}
function valid_mask(F,N,_2b){
var _2c=-1;
var _2d=-1;
var m=new Array(4);
for(i=0;i<4;i++){
m[i]=eval(N+"_"+i).value;
}
if(m[0]=="0"&&m[1]=="0"&&m[2]=="0"&&m[3]=="0"){
if(_2b&ZERO_NO){
alert(errmsg.err27);
return false;
}else{
if(_2b&ZERO_OK){
return true;
}
}
}
if(m[0]=="255"&&m[1]=="255"&&m[2]=="255"&&m[3]=="255"){
if(_2b&BCST_NO){
alert(errmsg.err27);
return false;
}else{
if(_2b&BCST_OK){
return true;
}
}
}
for(i=3;i>=0;i--){
for(j=1;j<=8;j++){
if((m[i]%2)==0){
_2c=(3-i)*8+j;
}else{
if(((m[i]%2)==1)&&_2d==-1){
_2d=(3-i)*8+j;
}
}
m[i]=Math.floor(m[i]/2);
}
}
if(_2c>_2d){
alert(errmsg.err27);
return false;
}
return true;
}
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
}
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
}
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
}
function closeWin(_35){
if(((_35!=null)&&(_35.close))||((_35!=null)&&(_35.closed==false))){
_35.close();
}
}
function valid_ip(F,N,M1,_39){
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
if(_39&ZERO_NO){
alert(M+errmsg.err31);
return false;
}
}
if((m[0]!="0"||m[1]!="0"||m[2]!="0")&&m[3]=="0"){
if(_39&MASK_NO){
alert(M+errmsg.err31);
return false;
}
}
return true;
}
function valid_ip_str(I,M){
if(I.value==""||I.value==" "){
return true;
}
var m=new Array(4);
var _3e=I.value.split(".");
for(i=0;i<4;i++){
m[i]=parseInt(_3e[i],10);
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
}
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
}
function fix_cr(F){
var re1=new RegExp("&#13;&#10;","gi");
var re2=new RegExp("&#13;","gi");
var re3=new RegExp("&#10;","gi");
var re4=new RegExp("&#38;","gi");
var re5=new RegExp("&#34;","gi");
var re6=new RegExp("&#62;","gi");
var re7=new RegExp("&#60;","gi");
var a=F.replace(re1,"\n");
var b=a.replace(re2,"\n");
var c=b.replace(re3,"\n");
var d=c.replace(re4,"&");
var e=d.replace(re5,"\"");
var f=e.replace(re4,">");
var g=f.replace(re5,"<");
return g;
}
var w3c=(document.getElementById)?true:false;
var ie=(document.all)?true:false;
var N=-1;
function createBar(w,h,_57,_58,_59,_5a){
if(ie||w3c){
var t="<div class=\"progressbar\" id=\"_xpbar"+(++N)+"\" style=\"width:"+w+"px; height:"+h+"px;\">";
t+="<span class=\"progressbarblock\" id=\"blocks"+N+"\" style=\"left:-"+(h*2+1)+"px;\">";
for(i=0;i<_58;i++){
t+="<span class=\"progressbarblock\" style=\"left:-"+((h*i)+i)+"px; width:"+h+"px; height:"+h+"px; ";
t+=(ie)?"filter:alpha(opacity="+(100-i*(100/_58))+")":"-Moz-opacity:"+((100-i*(100/_58))/100);
t+="\"></span>";
}
t+="</span></div>";
document.write(t);
var bA=(ie)?document.all["blocks"+N]:document.getElementById("blocks"+N);
bA.bar=(ie)?document.all["_xpbar"+N]:document.getElementById("_xpbar"+N);
bA.blocks=_58;
bA.N=N;
bA.w=w;
bA.h=h;
bA.speed=_57;
bA.ctr=0;
bA.count=_59;
bA.action=_5a;
bA.togglePause=togglePause;
bA.showBar=function(){
this.bar.style.visibility="visible";
};
bA.hideBar=function(){
this.bar.style.visibility="hidden";
};
bA.tid=setInterval("startBar("+N+")",_57);
return bA;
}
}
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
}
function togglePause(){
if(this.tid==0){
this.tid=setInterval("startBar("+this.N+")",this.speed);
}else{
clearInterval(this.tid);
this.tid=0;
}
}
function change_style(id,_60){
identity=document.getElementById(id);
identity.className=_60;
}
function Capture(obj){
document.write(obj);
}
function defined(val){
return (typeof val!="undefined");
}
function cleanTable(_63){
for(var i=_63.rows.length-1;i>0;i--){
_63.deleteRow(i);
}
}
function openHelpWindow(url){
var top=30;
var _67=Math.floor(screen.availWidth*0.66)-10;
var _68=Math.floor(screen.availWidth*0.33);
var _69=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("help/"+url,"DDWRT_Help","top="+top+",left="+_67+",width="+_68+",height="+_69+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
}
function openHelpWindowExt(url){
var top=30;
var _6d=Math.floor(screen.availWidth*0.66)-10;
var _6e=Math.floor(screen.availWidth*0.33);
var _6f=Math.floor(screen.availHeight*0.9)-30;
var win=window.open("http://www.dd-wrt.com/help/english/"+url,"DDWRT_Help","top="+top+",left="+_6d+",width="+_6e+",height="+_6f+",resizable=yes,scrollbars=yes,statusbar=no");
win.focus();
}
function openAboutWindow(){
var _71=400;
var _72=600;
var top=Math.floor((screen.availHeight-_72-10)/2);
var _74=Math.floor((screen.availWidth-_71)/2);
var win=window.open("About.htm","DDWRT_About","top="+top+",left="+_74+",width="+_71+",height="+_72+",resizable=no,scrollbars=no,statusbar=no");
win.focus();
}
function openWindow(url,_77,_78){
var top=Math.floor((screen.availHeight-_78-10)/2);
var _7a=Math.floor((screen.availWidth-_77)/2);
var win=window.open(url,"DDWRT_"+url.replace(/\.asp/,""),"top="+top+",left="+_7a+",width="+_77+",height="+_78+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
}
function setMeterBar(id,_7d,_7e){
if(isNaN(_7d)){
_7d=0;
}
_7d=Math.max(0,Math.min(100,Math.round(_7d)))+"%";
var _7f=(typeof id=="string"?document.getElementById(id):id);
if(_7f.firstChild){
_7f.firstChild.childNodes[0].style.width=_7d;
_7f.firstChild.childNodes[1].firstChild.data=_7d;
if(defined(_7e)){
_7f.lastChild.data=_7e;
}
}else{
_7f.innerHTML="<div class=\"meter\"><div class=\"bar\" style=\"width:"+_7d+";\"></div>"+"<div class=\"text\">"+_7d+"</div></div>"+(defined(_7e)?_7e:"");
}
}
function setElementContent(id,_81){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).innerHTML=_81;
}
function setElementVisible(id,_83){
if(!document.getElementById(id)){
return;
}
document.getElementById(id).style.display=(_83?"":"none");
}
function setElementActive(_84,_85){
var _86=document.getElementsByName(_84);
if(!_86){
return;
}
for(var i=0;i<_86.length;i++){
_86[i].disabled=!_85;
}
}
function setElementsActive(_88,_89,_8a){
if(!document.forms[0].elements[_88]||!document.forms[0].elements[_89]){
return;
}
var go=false;
for(var i=0;i<document.forms[0].elements.length;i++){
var _8d=document.forms[0].elements[i].name;
if(!document.forms[0].elements[i].type||(_8d!=_88&&!go)){
continue;
}
go=true;
document.forms[0].elements[i].disabled=!_8a;
if(_8d==_89){
break;
}
}
}
function addEvent(_8e,_8f,_90){
if(_8e.addEventListener){
_8e.addEventListener(_8f,_90,false);
}else{
if(_8e.attachEvent){
_8e.attachEvent("on"+_8f,_90);
}
}
}
function removeEvent(_91,_92,_93){
if(_91.removeEventListener){
_91.removeEventListener(_92,_93,false);
}else{
if(_91.detachEvent){
_91.detachEvent("on"+_92,_93);
}
}
}
function StatusUpdate(_94,_95){
var _96;
var _97;
var url=_94;
var _99=_95*1000;
var me=this;
var _9b=new Object();
var _9c=new Object();
this.start=function(){
if((!window.XMLHttpRequest&&!window.ActiveXObject)||_99==0){
return false;
}
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=true;
}
_97=setTimeout(me.doUpdate,_99);
};
this.stop=function(){
clearTimeout(_97);
if(document.getElementsByName("refresh_button")){
document.getElementsByName("refresh_button")[0].disabled=false;
}
_96=null;
};
this.onUpdate=function(id,_9e){
_9b[id]=_9e;
};
this.doUpdate=function(){
if(_96&&_96.readyState<4){
return;
}
if(window.XMLHttpRequest){
_96=new XMLHttpRequest();
}
if(window.ActiveXObject){
_96=new ActiveXObject("Microsoft.XMLHTTP");
}
_96.open("GET",url,true);
_96.onreadystatechange=function(){
if(_96.readyState<4||_96.status!=200){
return;
}
var _9f=new Array();
var _a0=/\{(\w+)::([^\}]*)\}/g;
while(result=_a0.exec(_96.responseText)){
var key=result[1];
var _a2=result[2];
if(defined(_9c[key])&&_9c[key]==_a2){
continue;
}
_9c[key]=_a2;
if(defined(_9b[key])){
_9f.push(_9b[key]);
}
setElementContent(key,_a2);
}
for(var i=0;i<_9f.length;i++){
(_9f[i])(_9c);
}
_97=setTimeout(me.doUpdate,_99);
};
_96.send("");
};
this.forceUpdate=function(){
this.stop();
this.doUpdate();
};
}
function apply(_a4){
_a4.submit();
for(i=0;i<_a4.elements.length;i++){
if(defined(_a4.elements[i].disabled)){
_a4.elements[i].disabled=true;
}
}
if(_a4.contents){
document.getElementById("contents").style.color="#999999";
}
}
function getOUIFromMAC(mac){
var top=30;
var _a7=Math.floor(screen.availWidth*0.66)-10;
var _a8=700;
var _a9=400;
var tab=new Array();
tab=mac.split(mac.substr(2,1));
var win=window.open("http://standards.ieee.org/cgi-bin/ouisearch?"+tab[0]+"-"+tab[1]+"-"+tab[2],"DDWRT_OUI_Search","top="+top+",left="+_a7+",width="+_a8+",height="+_a9+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
}
function getTimeOut(clk,_ad,_ae){
var _af=60;
var _b0=(_af/5)-3;
var _b1=1;
if(_ad==1){
_b1=1.5;
}
if(_ae==1){
_b1=_b1*3;
}
if(_ae==2){
_b1=_b1*1.5;
}
this.wait_time=_b1*_af*(125/clk);
this.scroll_count=this.wait_time/5-3;
}
function setElementMask(id,_b3){
var _b4=document.getElementById(id);
if(!_b4){
return;
}
var val=_b4.value;
var _b6=_b4.maxlength;
var _b7=_b4.size;
var _b8=_b4.onblur;
var _b9=_b4.parentNode;
var _ba=_b4.nextSibling;
var _bb=document.createElement("input");
_bb.setAttribute("value",val);
_bb.setAttribute("name",id);
_bb.setAttribute("id",id);
_bb.setAttribute("maxlength",_b6);
_bb.setAttribute("size",_b7);
_bb.setAttribute("onblur",_b8);
if(_b3==true){
_bb.setAttribute("type","text");
}else{
_bb.setAttribute("type","password");
}
_b9.removeChild(_b4);
_b9.insertBefore(_bb,_ba);
_bb.focus();
}
function show_layer_ext(obj,id,_be){
if(obj){
if(_be){
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
}else{
return;
}
}

