// Opera
var op = (navigator.userAgent.indexOf("Opera") != -1 && document.getElementById)

// Internet Explorer e.g. IE4+
var ie4 = (document.all)

// Netscape version 4 :
var ns4 = (document.layers)

// Mozila e.g. Netscape 6 upwards
var ns6 = (!document.all && document.getElementById)

// 0.0.0.0
var ZERO_NO = 1;	// 0x0000 0001
var ZERO_OK = 2;	// 0x0000 0010
// x.x.x.0
var MASK_NO = 4;	// 0x0000 0100
var MASK_OK = 8;	// 0x0000 1000
// 255.255.255.255
var BCST_NO = 16;	// 0x0001 0000
var BCST_OK = 32;	// 0x0010 0000

var SPACE_NO = 1;
var SPACE_OK = 2;
var CHECK = 4;

function addClass(element, classToAdd) {
    var currentClassValue = element.className;
      
    if (currentClassValue.indexOf(classToAdd) == -1) {
        if ((currentClassValue == null) || (currentClassValue === "")) {
            element.className = classToAdd;
        } else {
            element.className += " " + classToAdd;
        }
    }
}
 
function removeClass(element, classToRemove) {
    var currentClassValue = element.className;
 
    if (currentClassValue == classToRemove) {
        element.className = "";
        return;
    }
 
    var classValues = currentClassValue.split(" ");
    var filteredList = [];
 
    for (var i = 0 ; i < classValues.length; i++) {
        if (classToRemove != classValues[i]) {
            filteredList.push(classValues[i]);
        }
    }
 
    element.className = filteredList.join(" ");
}

function choose_enable(en_object) {
	if(!en_object)	return;
	en_object.disabled = false;					// netscape 4.x can not work, but 6.x can work

	if(!ns4)
		// better via css class
		removeClass(en_object, "off");
		// en_object.style.backgroundColor = "";	// netscape 4.x have error
}

function choose_disable(dis_object) {
	if(!dis_object)	return;
	dis_object.disabled = true;

	if(!ns4)
		// better via css class
		addClass(dis_object, "off");
		// dis_object.style.backgroundColor = "#e0e0e0";
}

function check_action(I,N) {
	if(ns4){									//ie.  will not need and will have question in "select"
		if(N == 0){
			if(EN_DIS == 1)
				I.focus();
			else
				I.blur();
		} else if(N == 1){
			if(EN_DIS1 == 1)
				I.focus();
			else
				I.blur();
		} else if(N == 2){
			if(EN_DIS2 == 1)
				I.focus();
			else
				I.blur();
		} else if(N == 3){
			if(EN_DIS3 == 1)
				I.focus();
			else
				I.blur();
		}
	}
}

function check_action1(I,T,N) {
	if(ns4){									//ie.  will not need and will have question in "select"
		if(N == 0){
			if(EN_DIS == 1)
				I.focus();
			else
				I.value = I.defaultChecked;
		}
		if(N == 1){
			if(EN_DIS1 == 1)
				I.focus();
			else
				I.value = I.defaultChecked;
		}
	}
}
function valid_range(I,start,end,M) {
	M1 = unescape(M);
	if(!isdigit(I,M1))
		return false;

	d = parseInt(I.value, 10);
	if ( !(d<=end && d>=start) ) {
		alert(M1 + errmsg.err14 + start + " - " + end +"].");
		I.value = I.defaultValue;
	} else {
		I.value = d;	// strip 0
		return true;
	}
	return false;
}

function valid_psk_length(I) {
	if(I.value == "")
		return true;
	
	if (I.value.length < 8 || I.value.length > 64) {
		alert(errmsg.err39);
		I.value = I.defaultValue;
	}
	return true;
}

function valid_macs_all(I) {
	if(I.value == "")
		return true;
	
	if (I.value.length == 12) {
		valid_macs_12(I);
	} else if (I.value.length == 17) {
		valid_macs_17(I);
	} else {
		alert(errmsg.err5);
		I.value = I.defaultValue;
	}
	return true;
}

function valid_macs_list(I) {
	if(I.value == "") return true;
	I.value = I.value.replace("\n", " ");
	var macs = I.value.split(" ");
	var ret = true;
	var good_macs = "";

	while (macs.length > 0) {
		var mac = new Object;
		mac.value = macs.shift();
		if(!valid_macs_17(mac)) {
			ret = false;
		} else {
			good_macs = good_macs + " " + mac.value;
		}
	}

	while (good_macs.indexOf(" ") == 0) {
		good_macs = good_macs.substr(1);
	}

	I.value = good_macs;

	return ret;
}

function valid_mac(I,T) {
	var m1,m2=0;

	if(I.value.length == 1)
		I.value = "0" + I.value;

	m1 =parseInt(I.value.charAt(0), 16);
	m2 =parseInt(I.value.charAt(1), 16);
	if( isNaN(m1) || isNaN(m2) ) {
		alert(errmsg.err15);
		I.value = I.defaultValue;
	}
	I.value = I.value.toUpperCase();
	if(T == 0) {
		if((m2 & 1) == 1){
			alert(errmsg.err16);
			I.value = I.defaultValue;
		}
	}
}

function valid_macs_12(I) {
	var m,m3;
	
	if (I.value == "")
		return true;
		
	if (I.value.length==12) {
		for (i=0;i<12;i++) {
			m=parseInt(I.value.charAt(i), 16);
			if (isNaN(m)) break;
		}
		if (i!=12) {
			alert(errmsg.err17);
			I.value = I.defaultValue;
		}
	} else {
		alert(errmsg.err18);
		I.value = I.defaultValue;
	}
	
	I.value = I.value.toUpperCase();
	if (I.value == "FFFFFFFFFFFF") {
		alert(errmsg.err19);
		I.value = I.defaultValue;
	}
	m3 = I.value.charAt(1);
	if ((m3 & 1) == 1) {
		alert(errmsg.err16);
		I.value = I.defaultValue;
	}
	return true;
}

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

function valid_email(addr,man,db) {
if (addr == '' && man) {
   if (db) alert('email address is mandatory');
   return false;
}
if (addr == '') return true;
var invalidChars = '\/\'\\ ";:?!()[]\{\}^|';
for (i=0; i<invalidChars.length; i++) {
   if (addr.indexOf(invalidChars.charAt(i),0) > -1) {
      if (db) alert('email address contains invalid characters');
      return false;
   }
}
for (i=0; i<addr.length; i++) {
   if (addr.charCodeAt(i)>127) {
      if (db) alert("email address contains non ascii characters.");
      return false;
   }
}

var atPos = addr.indexOf('@',0);
if (atPos == -1) {
   if (db) alert('email address must contain an @');
   return false;
}
if (atPos == 0) {
   if (db) alert('email address must not start with @');
   return false;
}
if (addr.indexOf('@', atPos + 1) > - 1) {
   if (db) alert('email address must contain only one @');
   return false;
}
if (addr.indexOf('.', atPos) == -1) {
   if (db) alert('email address must contain a period in the domain name');
   return false;
}
if (addr.indexOf('@.',0) != -1) {
   if (db) alert('period must not immediately follow @ in email address');
   return false;
}
if (addr.indexOf('.@',0) != -1){
   if (db) alert('period must not immediately precede @ in email address');
   return false;
}
if (addr.indexOf('..',0) != -1) {
   if (db) alert('two periods must not be adjacent in email address');
   return false;
}
var suffix = addr.substring(addr.lastIndexOf('.')+1);
if (suffix.length != 2 && suffix != 'com' && suffix != 'net' && suffix != 'org' && suffix != 'edu' && suffix != 'int' && suffix != 'mil' && suffix != 'gov' & suffix != 'arpa' && suffix != 'biz' && suffix != 'aero' && suffix != 'name' && suffix != 'coop' && suffix != 'info' && suffix != 'pro' && suffix != 'museum') {
   if (db) alert('invalid primary domain in email address');
   return false;
}
return true;
}


function valid_macs_17(I)
{
	oldmac = I.value;
	var mac = ignoreSpaces(oldmac);
	if (mac == "") {
		return true;
	}
	var m = mac.split(":");
	if (m.length != 6) {
		alert(errmsg.err21);
		I.value = I.defaultValue;
		return false;
	}
	var idx = oldmac.indexOf(':');
	if (idx != -1) {
		var pairs = oldmac.substring(0, oldmac.length).split(':');
		for (var i=0; i<pairs.length; i++) {
			nameVal = pairs[i];
			len = nameVal.length;
			if (len < 1 || len > 2) {
				alert(errmsg.err22);
				I.value = I.defaultValue;
				return false;
			}
			for(iln = 0; iln < len; iln++) {
				ch = nameVal.charAt(iln).toLowerCase();
				if (ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'f') {
					// nothing to do here
				} else {
					alert (errmsg.err23 + nameVal + errmsg.err24 + oldmac + ".");
					I.value = I.defaultValue;
					return false;
				}
			}
		}
	}
	I.value = I.value.toUpperCase();
	if(I.value == "FF:FF:FF:FF:FF:FF"){
		alert(errmsg.err19);
		I.value = I.defaultValue;
	}
	m3 = I.value.charAt(1);
	if((m3 & 1) == 1){
		alert(errmsg.err16);
		I.value = I.defaultValue;
	}
	return true;
}

function ignoreSpaces(string) {
	var temp = "";

	string = '' + string;
	splitstring = string.split(" ");
	for(i = 0; i < splitstring.length; i++) {
		temp += splitstring[i];
	}

	return temp;
}

function check_char(I,M1,c) {
	var i;
	M = unescape(M1);
	for(i=0 ; i<I.value.length; i++){
		ch = I.value.charAt(i);
		if(ch == c) {
			alert(M+" "+errmsg.err103 + " \""+c+"\"");
			I.value = I.defaultValue;
			return false;
		}
	}

	return true;

}
function check_space(I,M1){
	M = unescape(M1);
	for(i=0 ; i<I.value.length; i++){
		ch = I.value.charAt(i);
		if(ch == ' ') {
			alert(M+errmsg.err34);
			I.value = I.defaultValue;
			return false;
		}
	}

	return true;
}

function valid_key(I,l){
	var m;
	if(I.value.length==l*2)	{
		for(i=0;i<l*2;i++) {
			m=parseInt(I.value.charAt(i), 16);
			if( isNaN(m) )
				break;
		}
		if( i!=l*2 ){
			alert(errmsg.err25);
			I.value = I.defaultValue;
		}
	} else{
		alert(errmsg.err26);
		I.value = I.defaultValue;
	}
}

function valid_name(I,M,flag) {
	var invalid = ":";
	var len = invalid.length;
	var i;
	result = isascii(I,M);
	if(flag & CHECK){
	for (i=0;i<len;i++) {
		if (!check_char(I,M, invalid.charAt(i))) {
		    return false;
		}
	
	}
	}
	if(flag & SPACE_NO){
		result = check_space(I,M);
	}
	return result;
}

function valid_mask(F,N,flag){
	var match0 = -1;
	var match1 = -1;
	var m = new Array(4);

	for(i=0;i<4;i++)
		m[i] = eval(N+"_"+i).value;

	if(m[0] == "0" && m[1] == "0" && m[2] == "0" && m[3] == "0"){
		if(flag & ZERO_NO){
			alert(errmsg.err27);
			return false;
		} else if(flag & ZERO_OK){
			return true;
		}
	}

	if(m[0] == "255" && m[1] == "255" && m[2] == "255" && m[3] == "255"){
		if(flag & BCST_NO){
			alert(errmsg.err27);
			return false;
		} else if(flag & BCST_OK){
			return true;
		}
	}

	for(i=3;i>=0;i--){
		for(j=1;j<=8;j++){
			if((m[i] % 2) == 0)   match0 = (3-i)*8 + j;
			else if(((m[i] % 2) == 1) && match1 == -1)   match1 = (3-i)*8 + j;
			m[i] = Math.floor(m[i] / 2);
		}
	}
	if(match0 > match1){
		alert(errmsg.err27);
		return false;
	}

	return true;
}

function isdigit(I,M) {
	if (I.value.charAt(0) == "-") {i = 1}
	else {i = 0};
	for(i ; i<I.value.length; i++){

		ch = I.value.charAt(i);
		if(ch < '0' || ch > '9') {
			alert(M+errmsg.err28);
			I.value = I.defaultValue;
			return false;
		}
	}

	return true;
}

function isascii(I,M) {
	for(i=0 ; i<I.value.length; i++){
		ch = I.value.charAt(i);
		if(ch < ' ' || ch > '~'){
			alert('"'+M+'"'+errmsg.err29);
			I.value = I.defaultValue;
			return false;
		}
	}

	return true;
}

function isxdigit(I,M) {
	for(i=0 ; i<I.value.length; i++){
		ch = I.value.charAt(i).toLowerCase();
		if(ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'f') {

		} else {
			alert(M+errmsg.err30);
			I.value = I.defaultValue;
			return false;
		}
	}

	return true;
}

function closeWin(var_win){
	if ( ((var_win != null) && (var_win.close)) || ((var_win != null) && (var_win.closed==false)) )
		var_win.close();
}

function valid_ip(F,N,M1,flag){
	var m = new Array(4);
	M = unescape(M1);

	for(i=0;i<4;i++)
		m[i] = eval(N+"_"+i).value

	if(m[0] == 127 || m[0] == 224){
		alert(M+errmsg.err31);
		return false;
	}

	if(m[0] == "0" && m[1] == "0" && m[2] == "0" && m[3] == "0"){
		if(flag & ZERO_NO){
			alert(M+errmsg.err31);
			return false;
		}
	}

	if((m[0] != "0" || m[1] != "0" || m[2] != "0") && m[3] == "0"){
		if(flag & MASK_NO){
			alert(M+errmsg.err31);
			return false;
		}
	}

	return true;
}

function valid_ip_str(I, M){
	if(I.value == "" || I.value == " ") return true;

	var m = new Array(4);
	var ip_str = I.value.split(".");

	for(i=0;i<4;i++) {
		m[i] = parseInt(ip_str[i], 10);
		if( isNaN(m[i]) ) {
			alert(M+errmsg.err31);
			I.value = I.defaultValue;
			return false;
		}
	}

	if(m[0] == 127 || m[0] == 224){
		alert(M+errmsg.err31);
		I.value = I.defaultValue;
		return false;
	}

	if((m[0] > "255" || m[1] > "255" || m[2] > "255") && m[3] > "255"){
		alert(M+errmsg.err31);
		I.value = I.defaultValue;
		return false;
	}

	return true;
}

function valid_ip_gw(F,I,N,G) {
	var IP = new Array(4);
	var NM = new Array(4);
	var GW = new Array(4);

	for(i=0;i<4;i++)
		IP[i] = eval(I+"_"+i).value
	for(i=0;i<4;i++)
		NM[i] = eval(N+"_"+i).value
	for(i=0;i<4;i++)
		GW[i] = eval(G+"_"+i).value

	for(i=0;i<4;i++){
		if((IP[i] & NM[i]) != (GW[i] & NM[i])){
			alert(errmsg.err32);
			return false;
		}
	}
	if((IP[0] == GW[0]) && (IP[1] == GW[1]) && (IP[2] == GW[2]) && (IP[3] == GW[3])){
		alert(errmsg.err33);
		return false;
	}

	return true;
}

function fix_cr(F) {
	var re1 = new RegExp( '&#13;&#10;', 'gi' );
	var re2 = new RegExp( '&#13;', 'gi' );
	var re3 = new RegExp( '&#10;', 'gi' );
	var re4 = new RegExp( '&#38;', 'gi' );
	var re5 = new RegExp( '&#34;', 'gi' );
	var re6 = new RegExp( '&#62;', 'gi' );
	var re7 = new RegExp( '&#60;', 'gi' );
	var re8 = new RegExp( '&#92;', 'gi' );
	var re9 = new RegExp( '&#39;', 'gi' );
	var re10 = new RegExp( '&#9;', 'gi' );
	var a = F.replace(re1, '\n');
	var b = a.replace(re2, '\n');
	var c = b.replace(re3, '\n');
	var d = c.replace(re4, '&');
	var e = d.replace(re5, '"');
	var f = e.replace(re6, '>');
	var g = f.replace(re7, '<');
	var h = g.replace(re8, '\\');
	var i = h.replace(re9, '\'');
	var j = i.replace(re10,'');
return j;
}

// 03/03/06 : Botho - New progress bar implementation
//
// BEGIN xp_progressbar
// Copyright 2004 Brian Gosselin of ScriptAsylum.com
//
// v1.0 - Initial release
// v1.1 - Added ability to pause the scrolling action (requires you to assign
//        the bar to a unique arbitrary variable).
//      - Added ability to specify an action to perform after a x amount of
//      - bar scrolls. This requires two added arguments.
// v1.2 - Added ability to hide/show each bar (requires you to assign the bar
//        to a unique arbitrary variable).
// v.13 - Botho 03/03/06 : incorporate DD-WRT styles

// var xyz = createBar(
// total_width,
// total_height,
// background_color,       Botho 03/03/06 : incorporate DD-WRT styles
// border_width,           Botho 03/03/06 : incorporate DD-WRT styles
// border_color,           Botho 03/03/06 : incorporate DD-WRT styles
// block_color,            Botho 03/03/06 : incorporate DD-WRT styles
// scroll_speed,
// block_count,
// scroll_count,
// action_to_perform_after_scrolled_n_times
// )

var w3c=(document.getElementById)?true:false;
var ie=(document.all)?true:false;
var N=-1;

function createBar(w,h,speed,blocks,count,action) {
	if (ie||w3c) {	
		var t='<div class="progressbar" id="_xpbar'+(++N)+'" style="width:'+w+'px; height:'+h+'px;">';
		t+='<span class="progressbarblock" id="blocks'+N+'" style="left:-'+(h*2+1)+'px;">';
		for (var i=0;i<blocks;i++) {
			t+='<span class="progressbarblock" style="left:-'+((h*i)+i)+'px; width:'+h+'px; height:'+h+'px; ';
			t+=(ie)?'filter:alpha(opacity='+(100-i*(100/blocks))+')':'-Moz-opacity:'+((100-i*(100/blocks))/100);
			t+='"></span>';
		}
		t+='</span></div>';
		document.write(t);
		var bA=(ie) ? document.all['blocks'+N] : document.getElementById('blocks'+N);
		bA.bar=(ie) ? document.all['_xpbar'+N] : document.getElementById('_xpbar'+N);
		bA.blocks=blocks;
		bA.N=N;
		bA.w=w;
		bA.h=h;
		bA.speed=speed;
		bA.ctr=0;
		bA.count=count;
		bA.action=action;
		bA.togglePause=togglePause;
		bA.showBar=function() {
			this.bar.style.visibility="visible";
		}
		bA.hideBar=function() {
			this.bar.style.visibility="hidden";
		}
		bA.tid=setInterval('startBar('+N+')',speed);
		return bA;
	}
	return false;
}

function startBar(bn) {
	var t=(ie)?document.all['blocks'+bn]:document.getElementById('blocks'+bn);
	if (parseInt(t.style.left)+t.h+1-(t.blocks*t.h+t.blocks)>t.w) {
		t.style.left=-(t.h*2+1)+'px';
		t.ctr++;
		if (t.ctr>=t.count) {
			eval(t.action);
			t.ctr=0;
		}
	} else {
		t.style.left=(parseInt(t.style.left)+t.h+1)+'px';
	}
}

function togglePause() {
	if (this.tid==0) {
		this.tid=setInterval('startBar('+this.N+')',this.speed);
	} else {
		clearInterval(this.tid);
		this.tid=0;
	}
}

// END xp_progressbar

// 03/03/06 : Botho - Change style of the element in param
function change_style(id, newClass) {
   var identity=document.getElementById(id);
   identity.className=newClass;
}

// 14/03/06 : Botho - New way for translation. (Linksys one)
function Capture(obj)
{
	document.write(obj);
}


function defined(val) {
	return (typeof val != "undefined");
}

function cleanTable(table) {
	for(var i = table.rows.length - 1; i > 0; i--) table.deleteRow(i);
}

// Opens the help window at the right side of the screen.
function openHelpWindow(url) {
	var top = 30;
	var left = Math.floor(screen.availWidth * .66) - 10;
	var width = Math.floor(screen.availWidth * .33);
	var height = Math.floor(screen.availHeight * .9) - 30;
	var win = window.open("help/" + url, 'DDWRT_Help', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	win.focus();
}

function openHelpWindowExt(url) {
	var top = 30;
	var left = Math.floor(screen.availWidth * .66) - 10;
	var width = Math.floor(screen.availWidth * .33);
	var height = Math.floor(screen.availHeight * .9) - 30;
	var win = window.open("http://www.dd-wrt.com/help/english/" + url, 'DDWRT_Help', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	win.focus();
}

function openAboutWindow() {
	var width = 750;
	var height = 650;
	var top = Math.floor((screen.availHeight - height - 10) / 2);
	var left = Math.floor((screen.availWidth - width) / 2);
	var win = window.open("About.htm", 'DDWRT_About', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=no,scrollbars=no,statusbar=no");
	win.focus();
	/* no time to finish that now :-(
	Dialog.alert({url: "About.htm", options: {method: 'get'}},  
               {className: "ddwrt", width:500, height:600, okLabel: sbutton.clos, top:0,
                showEffect: Effect.ModalSlideDown, hideEffect: Effect.ModalSlideUp, showEffectOptions: {duration: 1.5, delay: 0.3}, hideEffectOptions: {duration: 0.3}});
    */
}

// Opens a new window in the center of the screen and closes it, if the parent window is unloaded
function openWindow(url, width, height, title) {
	if (!title) title=url.replace(/\.asp/, "");
	var top = Math.floor((screen.availHeight - height - 10) / 2);
	var left = Math.floor((screen.availWidth - width) / 2);
	var win = window.open(url, 'DDWRT_' + title, 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	addEvent(window, "unload", function() { if(!win.closed) win.close(); });
	win.focus();
}


// Renders a nice meter with the percentage value
function setMeterBar(id, fraq, text) {
	if(isNaN(fraq)) fraq = 0;
	fraq = Math.max(0, Math.min(100, Math.round(fraq))) + "%";
	var node = (typeof id == "string" ? document.getElementById(id) : id);
	if(node.firstChild) {
		node.firstChild.childNodes[0].style.width = fraq;
		node.firstChild.childNodes[1].firstChild.data = fraq;
		if(defined(text)) node.lastChild.data = text;
	} else {
		node.innerHTML = '<div class="meter"><div class="bar" style="width:' + fraq + ';"></div>'
			+ '<div class="text">' + fraq + '</div></div>' + (defined(text) ? text : "");
	}
}

// Sets the content inside the tag given by its id
function setElementContent(id, content) {
	if(!document.getElementById(id)) return;
	document.getElementById(id).innerHTML = content;
}

// Shows or hides element given by its id
function setElementVisible(id, state) {
	if(!document.getElementById(id)) return;
	document.getElementById(id).style.display = (state ? "" : "none");
}

// Disables or enables a form element given by its name
// (Might replace choose_enable and choose_disable in future)
function setElementActive(name, state) {
	var elements = document.getElementsByName(name);
	if(!elements) return;
	for(var i = 0; i < elements.length; i++) { elements[i].disabled = !state; }
}

// Disables or enables several elements given by name of the first and the last element
function setElementsActive(firstName, lastName, state) {
	if(!document.forms[0].elements[firstName] || !document.forms[0].elements[lastName]) return;
	var go = false;
	for(var i = 0; i < document.forms[0].elements.length; i++) {
		var currentName = document.forms[0].elements[i].name;
		if(!document.forms[0].elements[i].type || (currentName != firstName && !go)) continue;
		go = true;
		document.forms[0].elements[i].disabled = !state;
		if(currentName == lastName) break;
	}
}

// Adds an eventlistner to object
function addEvent(object, type, func) {
	if(object.addEventListener)
		object.addEventListener(type, func, false);
	else if (object.attachEvent)
		object.attachEvent("on" + type, func);
}

// Removes an eventlistner from object
function removeEvent(object, type, func) {
	if(object.removeEventListener)
		object.removeEventListener(type, func, false);
	else if (object.detachEvent)
		object.detachEvent("on" + type, func);
}

// Class for requesting updates periodically using ajax
function StatusUpdate(_url, _frequency) {
	var request;
	var timer;
	var url = _url;
	var frequency = _frequency * 1000;
	var me = this;
	var callbacks = new Object();
	var updates = new Object();

	this.start = function() {
		if((!window.XMLHttpRequest && !window.ActiveXObject) || frequency == 0) return false;
		if(document.getElementsByName("refresh_button").length) {
			document.getElementsByName("refresh_button")[0].disabled = true;
		}
		timer = setTimeout(me.doUpdate, frequency);
		return true;
	}

	this.stop = function() {
		clearTimeout(timer);
		if(document.getElementsByName("refresh_button").length)
			document.getElementsByName("refresh_button")[0].disabled = false;
		request = null;
	}

	this.onUpdate = function(id, func) {
		callbacks[id] = func;
	}

	this.doUpdate = function() {
		if(request && request.readyState < 4) return;
		if(window.XMLHttpRequest) request = new XMLHttpRequest();
		if(window.ActiveXObject) request = new ActiveXObject("Microsoft.XMLHTTP");
		request.open("GET", url, true);
		request.onreadystatechange = function() {
			if(request.readyState < 4 || request.status != 200) return;
			var activeCallbacks = new Array();
			var regex = /\{(\w+)::([^\}]*)\}/g;
			while(result = regex.exec(request.responseText)) {
				var key = result[1];
				var value = result[2];
				if(defined(updates[key]) && updates[key] == value) continue;
				updates[key] = value;
				if(defined(callbacks[key])) activeCallbacks.push(callbacks[key]);
				setElementContent(key, value);
			}
			for(var i = 0; i < activeCallbacks.length; i++) { (activeCallbacks[i])(updates); }
			timer = setTimeout(me.doUpdate, frequency);
		}
		request.send("");
	}

	this.forceUpdate = function() {
		this.stop();
		this.doUpdate();
	}
}

//Statusbar live
function StatusbarUpdate() {
	var request;
	var timer;
	var url = "Statusinfo.live.asp";
	var frequency = 5000;
	var me = this;
	var callbacks = new Object();
	var updates = new Object();

	this.start = function() {
		if(!window.XMLHttpRequest && !window.ActiveXObject) return false;
		timer = setTimeout(me.doUpdate, frequency);
		return true;
	}

	this.stop = function() {
		clearTimeout(timer);
		request = null;
	}

	this.onUpdate = function(id, func) {
		callbacks[id] = func;
	}

	this.doUpdate = function() {
		if(request && request.readyState < 4) return;
		if(window.XMLHttpRequest) request = new XMLHttpRequest();
		if(window.ActiveXObject) request = new ActiveXObject("Microsoft.XMLHTTP");
		request.open("GET", url, true);
		request.onreadystatechange = function() {
			if(request.readyState < 4 || request.status != 200) return;
			var activeCallbacks = new Array();
			var regex = /\{(\w+)::([^\}]*)\}/g;
			while(result = regex.exec(request.responseText)) {
				var key = result[1];
				var value = result[2];
				if(defined(updates[key]) && updates[key] == value) continue;
				updates[key] = value;
				if(defined(callbacks[key])) activeCallbacks.push(callbacks[key]);
				setElementContent(key, value);
			}
			for(var i = 0; i < activeCallbacks.length; i++) { (activeCallbacks[i])(updates); }
			timer = setTimeout(me.doUpdate, frequency);
		}
		request.send("");

	}

}

function invalidTextValue( value ) {
	var needle = new Array(
		'<',
		'>'
		);
	
	if( value ) {
		var i = 0;
		var chars = '';
		
		for(i = 0; i < needle.length; i++) {
			re = new RegExp( needle[i] );
			if( value.match( re ) ) {
				if( chars.length ) {
					chars = chars + ', ' + needle[i];
				} else {
					chars = needle[i];
				}
			}	
		}
		if(chars.length) {
			return chars;
		}
	}
	return false;
}

function getInputLabel( type, name ) {
	var elements = document.getElementsByTagName( type );
	for(var i = 0; i < elements.length; i++) {
		if( elements[i].name == name && elements[i].parentNode.children ) {
			if( elements[i].parentNode.children[0].className == "label" ) {
				return elements[i].parentNode.children[0].innerHTML.match(/[^>]+$/);
			}
		}
	}
	return name;
}

function checkformelements( form ) {
	var errors = null;
	var i = 0;
	for( i = 0; i < form.elements.length; i++ ) {
		if( form.elements[i].className == "no-check" ) {
			// don't check for html characters in input fields	
		} else if( form.elements[i].type == 'text' ) {
			if( chars = invalidTextValue(form.elements[i].value ) ) {
				//alert('Invalid input characters "' + chars + '" in field "' + getInputLabel( 'input', form.elements[i].name ) + '"');
				alert(errmsg.err112.replace('<invchars>', chars).replace('<fieldname>', getInputLabel( 'input', form.elements[i].name )));
				form.elements[i].style.border = "solid 2px #f00";
				form.elements[i].focus();
				return false;
			}
		}
	}
	return true;
}

// Gray all form when submitting
function apply(form, text, delay) {
	if( !checkformelements( form )) {
		return false;
	} else {
		form.submit();
		for (i = 0; i < form.elements.length; i++) {
			if(defined(form.elements[i].disabled)) 
				form.elements[i].disabled = true;
		}
		if (form.contents) document.getElementById("contents").style.color = '#999999';
	}
}

function applytake(form, text,seconds, delay) {
	if( !checkformelements( form )) {
                return false;
        } else {
		form.action.value = "ApplyTake";
		if(!text)
			text=errmsg.err100;
			
		Dialog.info(text, {windowParameters: {className:"ddwrt", width:250, height:100}, showProgress:true});
		setTimeout(function(){form.submit();}, 2000);
		
		if(delay) {
			setTimeout(function(){infoTimeout(text,seconds, delay);}, 1000);
    		}
	}
}

function applyupdate(form, text,seconds, delay) {
    if(!text)
    	text=errmsg.err100;

    Dialog.info(text, {windowParameters: {className:"ddwrt", width:250, height:100}, showProgress:true});
    setTimeout(function(){form.submit();}, 2000);

    if(delay) {
    	setTimeout(function(){infoTimeout(text,seconds, delay);}, 1000);
    }
}

function infoTimeout(text,seconds, delay) {
	delay--;
	if (delay > 0) {
		Dialog.setInfoMessage(text+" "+delay+seconds+" ...");
		setTimeout(function(){infoTimeout(text,seconds, delay);}, 1000);
	}
	else {
		Dialog.setInfoMessage(text);
	}
}

// Class for sending a request using ajax
// This is work in progress, plz do not use yet.
/* function Request(_url) {
	var url = _url;
	var values = new Array();
	var callback = new Array();

	this.addAllElements = function() {
		for(var i = 0; i < document.forms[0].elements.length; i++)
			this.addElement(document.forms[0].elements[i].name);
	}

	this.addElement = function(name) {
		if(!document.forms[0].elements[name]) return;
		this.addValue(name, document.forms[0].elements[name].value);
	}

	this.addValue = function(name, value) {
		if(name.match(/[^\w]/)) return;
		values.push(name + "=" + encodeURIComponent(value));
	}

	this.onRequest = function(func) {
		callback.push(func);
	}

	this.send = function() {
		// some function to disable page
		if(window.XMLHttpRequest) request = new XMLHttpRequest();
		if(window.ActiveXObject) request = new ActiveXObject("Microsoft.XMLHTTP");
		request.open("POST", url, false);
		request.send(values.join("&"));
		if(request.readyState < 4 || request.status != 200) alert("Error while processing this request: " + values.join("&"));
		var updates = new Object();
		while(result = /\{(\w+)::([^\}]*)\}/g.exec(request.responseText)) {
			updates[result[1]] = result[2];
		}
		for(var i = 0; i < callback.length; i++) { (callback[i])(updates); }
		// enable page
	}

} */


// Botho 06/04/2006 : Function to resolve OUI names
function getOUIFromMAC(mac) {

	var top = 30;
	var left = Math.floor(screen.availWidth * .66) - 10;
	var width = 700
	var height = 400
	var tab = new Array();

	tab = mac.split(mac.substr(2,1));

	var win = window.open("https://oidsearch.s.dd-wrt.com/search/" + tab[0] + ':' + tab[1] + ':' + tab[2], 'DDWRT_OUI_Search', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	addEvent(window, "unload", function() { if(!win.closed) win.close(); });
	win.focus();
}

function openBW(iface) {
	var top = 30;
	var left = Math.floor(screen.availWidth * .66) - 10;
	var width = 555
	var height = 275
	var win = window.open("graph_if.svg?" + iface, 'Bandwidth', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	addEvent(window, "unload", function() { if(!win.closed) win.close(); });
	win.focus();
}

/* Added by Botho 06.May.06 */
// Unmask or Mask an input type (for password or key)
function setElementMask(id, state) {

	var OldInput = document.getElementById(id);
	if(!OldInput) return;

	var val = OldInput.value;
	var val_maxlength = OldInput.maxlength;
	var val_size = OldInput.size;
	var val_onblur = OldInput.onblur;
	var parent = OldInput.parentNode;
	var sibling = OldInput.nextSibling;
	var className = OldInput.className;
	var newInput = document.createElement('input');
	newInput.setAttribute('value', val);
	newInput.setAttribute('name', id);
	newInput.setAttribute('id', id);
	newInput.setAttribute('maxlength', val_maxlength);
	newInput.setAttribute('size', val_size);
	newInput.className = className;
	newInput.setAttribute('rel', OldInput.getAttribute('rel'));
	//newInput.setAttribute('onblur', val_onblur);
	newInput.onblur = val_onblur;
	
	if (state == true)
		newInput.setAttribute('type', 'text');
	else
		newInput.setAttribute('type', 'password');

	parent.removeChild(OldInput);
	parent.insertBefore(newInput, sibling);
	newInput.focus();

}

var windo = {
	getWindoSize: function() {
		if (window.innerHeight)
			return {height: window.innerHeight, width: window.innerWidth};
		else if (document.documentElement && document.documentElement.clientHeight)
			return {height: document.documentElement.clientHeight, width: document.documentElement.clientWidth};

		return {Height: document.body.clientHeight,	width: document.body.clientWidth};
	}

};

function show_layer_ext(obj, id, state)
{
	if(!obj) return;
	if(state){
		visibility_style='visible';
		display_style='block';
	} else {
		visibility_style='hidden';
		display_style='none';
	}
	// Show/hide the layer
	if(ie4)
	{
		eval("document.all." + id + ".style.visibility='" + visibility_style + "'");
		eval("document.all." + id + ".style.display='" + display_style + "'");
	}
	if(ns4)
	{
		eval("document." + id + ".visibility='" + visibility_style + "'");
		eval("document." + id + ".display='" + display_style + "'");
	}
	if(ns6 || op)
	{
		eval("document.getElementById('" + id + "').style.visibility='" + visibility_style + "'");
		eval("document.getElementById('" + id + "').style.display='" + display_style + "'");
	}
}

function toggle_layer_ext(obj, id1, id2, state)
{
	show_layer_ext(obj, id1, state)
	show_layer_ext(obj, id2, !state)
}


function lgout() {
	document.forms[0].action.value = "Logout";
	document.forms[0].submit();
}

function comma(n)
{
	n = '' + n;
	var p = n;
	while ((n = n.replace(/(\d+)(\d{3})/g, '$1,$2')) != p) p = n;
	return n;
}

function scaleSize(num)
{
	if (isNaN(num *= 1)) return '-';
	if (num <= 9999) return '' + num;
	var s = -1;
	do {
		num /= 1000;
		++s;
	} while ((num > 9999) && (s < 2));
	return comma(num.toFixed(2)) + '<small> ' + (['kB', 'MB', 'GB'])[s] + '</small>';
}

function DisplayDiv(current,evt,h,w,text)
{
	var height = h;
	var width = w;

	text = text.replace(/&lt;/gi,'<');
	text = text.replace(/&gt;/gi,'>');

	if(document.all) {
		if(document.readyState == 'complete') {
			document.all.bulle.innerHTML = '<table class="bulle" cellspacing="0"><tr><td class="bulle">' + text + '</td></tr></table>';
			document.all.bulle.style.pixelLeft = event.clientX + document.body.scrollLeft + width;
			document.all.bulle.style.pixelTop = event.clientY + document.body.scrollTop + height;
			document.all.bulle.style.visibility = 'visible';
		}
	}
	else if(document.getElementById) {
		document.getElementById('bulle').innerHTML = '<table class="bulle" cellspacing="0"><tr><td class="bulle">' + text + '</td></tr></table>';
		document.getElementById('bulle').style.left = evt.pageX + width + 'px';
		document.getElementById('bulle').style.top = evt.pageY + height + 'px';
		document.getElementById('bulle').style.visibility = 'visible';
	}
}

function unDisplayDiv()
{
	if(document.all) {
		document.all.bulle.style.visibility = 'hidden';
	}
	else if(document.layers) {
		document.bulle.visibility = 'hidden';
	}
	else if(document.getElementById) {
		document.getElementById('bulle').style.visibility = 'hidden';
	}
}

function submitFooterButton(sub, res, reb, autoref, ref, clo) {
	if(sub)
		document.write("<input title=\"" + sbutton.savetitle + "\" class=\"button\" type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form);\" />");
	if(sub)
		document.write("<input title=\"" + sbutton.applytitle + "\" class=\"button\" type=\"button\" name=\"apply_button\" value=\"" + sbutton.apply + "\" onclick=\"to_apply(this.form);\" />");
	if(res) {
		if(document.forms[0].elements['submit_button'].value) {
			var button_action = "document.location=document.forms[0].elements['submit_button'].value+'.asp';";
		} else {
			var button_action = "window.location.reload();";
		}
		document.write("<input title=\"" + sbutton.canceltitle + "\" class=\"button\" type=\"button\" name=\"reset_button\" value=\"" + sbutton.cancel + "\" onclick=\"" + button_action + "\" />");
	}
	if(reb)
		document.write("<input class=\"button\" type=\"button\" name=\"reboot_button\" value=\"" + sbutton.reboot + "\" onclick=\"to_reboot(this.form);\" />");
	if(autoref)
		document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\"" + autoref + "\" onclick=\"window.location.reload();\">");
	if(ref)
		document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload();\" />");
	if(clo)
		document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\"" + sbutton.clos + "\" onclick=\"self.close();\" />");
}

/**
*
*  Sortable HTML table
*  http://www.webtoolkit.info/
*
**/
function SortableTable (tableEl) {
 
	this.tbody = tableEl.getElementsByTagName('tbody');
	this.thead = tableEl.getElementsByTagName('tbody');
 
	this.getInnerText = function (el) {
		if (typeof el == "undefined") return null;
		if (el == null) return null;
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
	    if (itm != null && itm.replace(/^\s+|\s+$/g,"").match(/^[\d]+$/)) sortfn = this.sortNumeric;
 
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
		if (thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]) == null) 
		    return 0;
		if (thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]) == null) 
		    return 0;
		aa = thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]).toLowerCase();
		bb = thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]).toLowerCase();
		if (aa==bb) return 0;
		if (aa<bb) return -1;
		return 1;
	}
 
	this.sortNumeric = function(a,b) {
		if (thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]) == null)
		    return 0;
		if (thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]) == null)
		    return 0;
		aa = parseFloat(thisObject.getInnerText(a.cells[thisObject.sortColumnIndex]));
		if (isNaN(aa)) aa = 0;
		bb = parseFloat(thisObject.getInnerText(b.cells[thisObject.sortColumnIndex]));
		if (isNaN(bb)) bb = 0;
		return aa-bb;
	}
 
	// define variables
	var thisObject = this;
	var sortSection = this.thead;
 
	// constructor actions
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

function addTableEntry( tableId ) {
	
	var table = $(tableId);
	var section = table.childElements()[0];
	var rows = section.childElements();
	var row = null;
	
	for (i = 0; i < rows.length; i++) {
		if(rows[i].id) {
			if(rows[i].id.substr( rows[i].id.length - 9, 9 ) == '_template' ) {
				// create copy
				row = document.createElement('TR');
				row.id = tableId + '_row_' + (rows.length - i);
				for(j = 0; j < rows[i].childElements().length; j++) {
					var cell = rows[i].childElements()[j].cloneNode(true);
					// rename fields
					for(k = 0; k < cell.childElements().length; k++) {
						if(cell.childElements()[k].name) {
							cell.childElements()[k].name = cell.childElements()[k].name + '_' + (rows.length - i);
						}
						if(cell.childElements()[k].id) {
							cell.childElements()[k].id = cell.childElements()[k].id + '_' + (rows.length - i);
						}
					}
					row.appendChild(cell);
				}
			}
		}
	}

	if(row != null) {
		section.appendChild(row);
		
		// adjust share counter
		if($(tableId + '_count')) {
			$(tableId + '_count').value++;
		}

		// remove add button
		if($(tableId + '_add')) {
			if($(tableId + '_count_limit')) {
				if($(tableId + '_count_limit').value <= rows.length - 2 ) {
					$(tableId + '_add').hide();
				}
			}
		}
		
		return section.childElements()[section.childElements().length - 1];
	} else {
		return null;
	}
}

function removeTableEntry( tableId, button ) {
	
	if(button.name.indexOf('_del_') >= 0) {
		var rowNumber = parseInt(button.name.substr(button.name.indexOf('_del_') + 5, button.name.length - button.name.indexOf('_del_') - 5));
		
		if(rowNumber > 0) {
			var table = $(tableId);
			var section = table.childElements()[0];
			var row = $(tableId + '_row_' + rowNumber);
			section.removeChild( row );
			
			// reorder remaining rows
			var rows = section.childElements();
			var sublabel = tableId + '_row_';
			for (i = 0; i < rows.length; i++) {
				if( rows[i].id.substr( 0, sublabel.length ) == sublabel && rows[i].id.substr(rows[i].id.length - 9, 9) != '_template') {
					var index = parseInt(rows[i].id.substr( sublabel.length, rows[i].id.length - sublabel.length));
					if(index > rowNumber) {
						rows[i].id = sublabel + (index - 1);
						for(j = 0; j < rows[i].childElements().length; j++) {
							var cell = rows[i].childElements()[j];
							// rename fields
							for(k = 0; k < cell.childElements().length; k++) {
								if(cell.childElements()[k].name) {
									var label = cell.childElements()[k].name;
									label = label.substr(0, label.length - String(index).length);
									cell.childElements()[k].name = label + String(index - 1);
								}

								if(cell.childElements()[k].id) {
									var id = cell.childElements()[k].id;
									id = id.substr(0, id.length - String(index).length);
									cell.childElements()[k].id = id + String(index - 1);
								}
							}
						}
					}
				}
			}
			
			// adjust share counter
			if($(tableId + '_count')) {
				$(tableId + '_count').value--;
			}

			// show add button
			if($(tableId + '_add')) {
				if($(tableId + '_count_limit')) {
					if($(tableId + '_count_limit').value > rows.length - 4 ) {
						$(tableId + '_add').show();
					}
				}
			}
		}
	}
}
