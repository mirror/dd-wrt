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
if(!isdigit(I,M1)){
return false;
}
d=parseInt(I.value,10);
if(!(d<=_4&&d>=_3)){
alert(M1+errmsg.err14+_3+" - "+_4+"].");
I.value=I.defaultValue;
}else{
I.value=d;
return true;
}
return false;
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
function valid_domain(I) {
	var m,m3;
	
	if (I.value == "") {
		alert(errmsg.err113);
		return true;
	}
	if (I.value.length==4) {
		for (i=0;i<4;i++) {
			m=parseInt(I.value.charAt(i), 16);
			if (isNaN(m)) break;
		}
		if (i!=4) {
			alert(errmsg.err113);
			I.value = I.defaultValue;
		}
	} else {
		alert(errmsg.err113);
		I.value = I.defaultValue;
	}
	return true;
}

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
result=isascii(I,M);
var bbb=I.value.replace(/^\s*/,"");
var ccc=bbb.replace(/\s*$/,"");
I.value=ccc;
if(_13&SPACE_NO){
result=check_space(I,M);
}
return result;
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
alert("\""+M+"\""+errmsg.err29);
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
var h=g.replace(re8,"\\");
var i=h.replace(re9,"'");
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
if(document.getElementsByName("refresh_button").length){
document.getElementsByName("refresh_button")[0].disabled=true;
}
_43=setTimeout(me.doUpdate,_44);
return true;
};
this.stop=function(){
clearTimeout(_43);
if(document.getElementsByName("refresh_button").length){
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
function invalidTextValue(_54){
var _55=new Array("<",">");
if(_54){
var i=0;
var _56="";
for(i=0;i<_55.length;i++){
re=new RegExp(_55[i]);
if(_54.match(re)){
if(_56.length){
_56=_56+", "+_55[i];
}else{
_56=_55[i];
}
}
}
if(_56.length){
return _56;
}
}
return false;
};
function getInputLabel(_57,_58){
var _59=document.getElementsByTagName(_57);
for(var i=0;i<_59.length;i++){
if(_59[i].name==_58&&_59[i].parentNode.children){
if(_59[i].parentNode.children[0].className=="label"){
return _59[i].parentNode.children[0].innerHTML.match(/[^>]+$/);
}
}
}
return _58;
};
function checkformelements(_5a){
var _5b=null;
var i=0;
for(i=0;i<_5a.elements.length;i++){
if(_5a.elements[i].className=="no-check"){
}else{
if(_5a.elements[i].type=="text"){
if(chars=invalidTextValue(_5a.elements[i].value)){
alert(errmsg.err112.replace("<invchars>",chars).replace("<fieldname>",getInputLabel("input",_5a.elements[i].name)));
_5a.elements[i].style.border="solid 2px #f00";
_5a.elements[i].focus();
return false;
}
}
}
}
return true;
};
function apply(_5c){
if(!checkformelements(_5c)){
return false;
}else{
_5c.submit();
for(i=0;i<_5c.elements.length;i++){
if(defined(_5c.elements[i].disabled)){
_5c.elements[i].disabled=true;
}
}
if(_5c.contents){
document.getElementById("contents").style.color="#999999";
}
}
};
function applytake(_5d){
if(!checkformelements(_5d)){
return false;
}else{
_5d.action.value="ApplyTake";
_5d.submit();
for(i=0;i<_5d.elements.length;i++){
if(defined(_5d.elements[i].disabled)){
_5d.elements[i].disabled=true;
}
}
if(_5d.contents){
document.getElementById("contents").style.color="#999999";
}
}
};
function applyupdate(_5e){
_5e.submit();
for(i=0;i<_5e.elements.length;i++){
if(defined(_5e.elements[i].disabled)){
_5e.elements[i].disabled=true;
}
}
if(_5e.contents){
document.getElementById("contents").style.color="#999999";
}
};
function getOUIFromMAC(mac){
var top=30;
var _5f=Math.floor(screen.availWidth*0.66)-10;
var _60=700;
var _61=400;
var tab=new Array();
tab=mac.split(mac.substr(2,1));
var win=window.open("https://oidsearch.s.dd-wrt.com/search/"+tab[0]+':'+tab[1]+':'+tab[2],"DDWRT_OUI_Search","top="+top+",left="+_5f+",width="+_60+",height="+_61+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function openBW(_62){
var top=30;
var _63=Math.floor(screen.availWidth*0.66)-10;
var _64=555;
var _65=275;
var win=window.open("graph_if.svg?"+_62,"Bandwidth","top="+top+",left="+_63+",width="+_64+",height="+_65+",resizable=yes,scrollbars=yes,statusbar=no");
addEvent(window,"unload",function(){
if(!win.closed){
win.close();
}
});
win.focus();
};
function setElementMask(id,_6b){
var _6c=document.getElementById(id);
if(!_6c){
return;
}
var val=_6c.value;
var _6d=_6c.maxlength;
var _6e=_6c.size;
var _6f=_6c.onblur;
var _70=_6c.parentNode;
var _71=_6c.nextSibling;
var _72=_6c.className;
var _73=document.createElement("input");
_73.setAttribute("value",val);
_73.setAttribute("name",id);
_73.setAttribute("id",id);
_73.setAttribute("maxlength",_6d);
_73.setAttribute("size",_6e);
_73.className=_72;
_73.setAttribute("rel",_6c.getAttribute("rel"));
_73.onblur=_6f;
if(_6b==true){
_73.setAttribute("type","text");
}else{
_73.setAttribute("type","password");
}
_70.removeChild(_6c);
_70.insertBefore(_73,_71);
_73.focus();
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
function show_layer_ext(obj,id,_74){
if(!obj){
return;
}
if(_74){
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
function toggle_layer_ext(obj,id1,id2,_75){
show_layer_ext(obj,id1,_75);
show_layer_ext(obj,id2,!_75);
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
function DisplayDiv(_76,evt,h,w,_77){
var _78=w;
var _79=h;
_77=_77.replace(/&lt;/gi,"<");
_77=_77.replace(/&gt;/gi,">");
if(document.all){
if(document.readyState=="complete"){
document.all.bulle.innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_77+"</td></tr></table>";
document.all.bulle.style.pixelLeft=event.clientX+document.body.scrollLeft+_78;
document.all.bulle.style.pixelTop=event.clientY+document.body.scrollTop+_79;
document.all.bulle.style.visibility="visible";
}
}else{
if(document.getElementById){
document.getElementById("bulle").innerHTML="<table class=\"bulle\" cellspacing=\"0\"><tr><td class=\"bulle\">"+_77+"</td></tr></table>";
document.getElementById("bulle").style.left=evt.pageX+_78+"px";
document.getElementById("bulle").style.top=evt.pageY+_79+"px";
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
function submitFooterButton(sub,res,reb,_7a,ref,clo){
if(sub){
document.write("<input title=\""+sbutton.savetitle+"\" class=\"button\" type=\"button\" name=\"save_button\" value=\""+sbutton.save+"\" onclick=\"to_submit(this.form);\" />");
}
if(sub){
document.write("<input title=\""+sbutton.applytitle+"\" class=\"button\" type=\"button\" name=\"apply_button\" value=\""+sbutton.apply+"\" onclick=\"to_apply(this.form);\" />");
}
if(res){
if(document.forms[0].elements["submit_button"].value){
var _7b="document.location=document.forms[0].elements['submit_button'].value+'.asp';";
}else{
var _7b="window.location.reload();";
}
document.write("<input title=\""+sbutton.canceltitle+"\" class=\"button\" type=\"button\" name=\"reset_button\" value=\""+sbutton.cancel+"\" onclick=\""+_7b+"\" />");
}
if(reb){
document.write("<input class=\"button\" type=\"button\" name=\"reboot_button\" value=\""+sbutton.reboot+"\" onclick=\"to_reboot(this.form);\" />");
}
if(_7a){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+_7a+"\" onclick=\"window.location.reload();\">");
}
if(ref){
document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\""+sbutton.refres+"\" onclick=\"window.location.reload();\" />");
}
if(clo){
document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\""+sbutton.clos+"\" onclick=\"self.close();\" />");
}
};
function SortableTable(_7c){
this.tbody=_7c.getElementsByTagName("tbody");
this.thead=_7c.getElementsByTagName("tbody");
this.getInnerText=function(el){
if (typeof el == "undefined") return null;
if (el == null) return null;
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
this.getParent=function(el,_7d){
if(el==null){
return null;
}else{
if(el.nodeType==1&&el.tagName.toLowerCase()==_7d.toLowerCase()){
return el;
}else{
return this.getParent(el.parentNode,_7d);
}
}
};
this.sort=function(_7e){
var _7f=_7e.cellIndex;
var itm=this.getInnerText(this.tbody[0].rows[1].cells[_7f]);
var _80=this.sortCaseInsensitive;
if(itm != null && itm.replace(/^\s+|\s+$/g,"").match(/^[\d]+$/)){
_80=this.sortNumeric;
}
this.sortColumnIndex=_7f;
var _81=new Array();
for(j=1;j<this.tbody[0].rows.length;j++){
_81[j-1]=this.tbody[0].rows[j];
}
_81.sort(_80);
if(_7e.getAttribute("sortdir")=="down"){
_81.reverse();
_7e.setAttribute("sortdir","up");
}else{
_7e.setAttribute("sortdir","down");
}
for(i=1;i<_81.length;i++){
this.tbody[0].appendChild(_81[i]);
}
};
this.sortCaseInsensitive=function(a,b){
aa=_82.getInnerText(a.cells[_82.sortColumnIndex]).toLowerCase();
bb=_82.getInnerText(b.cells[_82.sortColumnIndex]).toLowerCase();
if(aa==bb){
return 0;
}
if(aa<bb){
return -1;
}
return 1;
};
this.sortNumeric=function(a,b){
aa=parseFloat(_82.getInnerText(a.cells[_82.sortColumnIndex]));
if(isNaN(aa)){
aa=0;
}
bb=parseFloat(_82.getInnerText(b.cells[_82.sortColumnIndex]));
if(isNaN(bb)){
bb=0;
}
return aa-bb;
};
var _82=this;
var _83=this.thead;
if(!(this.tbody&&this.tbody[0].rows&&this.tbody[0].rows.length>0)){
return;
}
if(_83&&_83[0].rows&&_83[0].rows.length>0){
var _84=_83[0].rows[0];
}else{
return;
}
for(var i=0;i<_84.cells.length;i++){
_84.cells[i].sTable=this;
_84.cells[i].onclick=function(){
this.sTable.sort(this);
return false;
};
}
};
function addTableEntry(_85){
var _86=$(_85);
var _87=_86.childElements()[0];
var _88=_87.childElements();
var row=null;
for(i=0;i<_88.length;i++){
if(_88[i].id){
if(_88[i].id.substr(_88[i].id.length-9,9)=="_template"){
row=document.createElement("TR");
row.id=_85+"_row_"+(_88.length-i);
for(j=0;j<_88[i].childElements().length;j++){
var _89=_88[i].childElements()[j].cloneNode(true);
for(k=0;k<_89.childElements().length;k++){
if(_89.childElements()[k].name){
_89.childElements()[k].name=_89.childElements()[k].name+"_"+(_88.length-i);
}
if(_89.childElements()[k].id){
_89.childElements()[k].id=_89.childElements()[k].id+"_"+(_88.length-i);
}
}
row.appendChild(_89);
}
}
}
}
if(row!=null){
_87.appendChild(row);
if($(_85+"_count")){
$(_85+"_count").value++;
}
if($(_85+"_add")){
if($(_85+"_count_limit")){
if($(_85+"_count_limit").value<=_88.length-2){
$(_85+"_add").hide();
}
}
}
return _87.childElements()[_87.childElements().length-1];
}else{
return null;
}
};
function removeTableEntry(_8a,_8b){
if(_8b.name.indexOf("_del_")>=0){
var _8c=parseInt(_8b.name.substr(_8b.name.indexOf("_del_")+5,_8b.name.length-_8b.name.indexOf("_del_")-5));
if(_8c>0){
var _8d=$(_8a);
var _8e=_8d.childElements()[0];
var row=$(_8a+"_row_"+_8c);
_8e.removeChild(row);
var _8f=_8e.childElements();
var _90=_8a+"_row_";
for(i=0;i<_8f.length;i++){
if(_8f[i].id.substr(0,_90.length)==_90&&_8f[i].id.substr(_8f[i].id.length-9,9)!="_template"){
var _91=parseInt(_8f[i].id.substr(_90.length,_8f[i].id.length-_90.length));
if(_91>_8c){
_8f[i].id=_90+(_91-1);
for(j=0;j<_8f[i].childElements().length;j++){
var _92=_8f[i].childElements()[j];
for(k=0;k<_92.childElements().length;k++){
if(_92.childElements()[k].name){
var _93=_92.childElements()[k].name;
_93=_93.substr(0,_93.length-String(_91).length);
_92.childElements()[k].name=_93+String(_91-1);
}
if(_92.childElements()[k].id){
var id=_92.childElements()[k].id;
id=id.substr(0,id.length-String(_91).length);
_92.childElements()[k].id=id+String(_91-1);
}
}
}
}
}
}
if($(_8a+"_count")){
$(_8a+"_count").value--;
}
if($(_8a+"_add")){
if($(_8a+"_count_limit")){
if($(_8a+"_count_limit").value>_8f.length-4){
$(_8a+"_add").show();
}
}
}
}
}
};

