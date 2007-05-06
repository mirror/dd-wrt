var Window=Class.create();
Window.keepMultiModalWindow=false;
Window.hasEffectLib=String.prototype.parseColor!=null;
Window.resizeEffectDuration=0.4;
Window.prototype={initialize:function(){
var id;
var _2=0;
if(arguments.length>0){
if(typeof arguments[0]=="string"){
id=arguments[0];
_2=1;
}else{
id=arguments[0]?arguments[0].id:null;
}
}
if(!id){
id="window_"+new Date().getTime();
}
if($(id)){
alert("Window "+id+" is already registered in the DOM! Make sure you use setDestroyOnClose() or destroyOnClose: true in the constructor");
}
this.options=Object.extend({className:"dialog",minWidth:100,minHeight:20,resizable:true,closable:true,minimizable:true,maximizable:true,draggable:true,userData:null,showEffect:(Window.hasEffectLib?Effect.Appear:Element.show),hideEffect:(Window.hasEffectLib?Effect.Fade:Element.hide),showEffectOptions:{},hideEffectOptions:{},effectOptions:null,parent:document.body,title:"&nbsp;",url:null,onload:Prototype.emptyFunction,width:200,height:300,opacity:1,recenterAuto:true,wiredDrag:false,closeCallback:null,destroyOnClose:false,gridX:1,gridY:1},arguments[_2]||{});
if(typeof this.options.top=="undefined"&&typeof this.options.bottom=="undefined"){
this.options.top=this._round(Math.random()*500,this.options.gridY);
}
if(typeof this.options.left=="undefined"&&typeof this.options.right=="undefined"){
this.options.left=this._round(Math.random()*500,this.options.gridX);
}
if(this.options.effectOptions){
Object.extend(this.options.hideEffectOptions,this.options.effectOptions);
Object.extend(this.options.showEffectOptions,this.options.effectOptions);
if(this.options.showEffect==Element.Appear){
this.options.showEffectOptions.to=this.options.opacity;
}
}
if(Window.hasEffectLib){
if(this.options.showEffect==Effect.Appear){
this.options.showEffectOptions.to=this.options.opacity;
}
if(this.options.hideEffect==Effect.Fade){
this.options.hideEffectOptions.from=this.options.opacity;
}
}
if(this.options.hideEffect==Element.hide){
this.options.hideEffect=function(){
Element.hide(this.element);
if(this.options.destroyOnClose){
this.destroy();
}
}.bind(this);
}
if(this.options.parent!=document.body){
this.options.parent=$(this.options.parent);
}
this.element=this._createWindow(id);
this.eventMouseDown=this._initDrag.bindAsEventListener(this);
this.eventMouseUp=this._endDrag.bindAsEventListener(this);
this.eventMouseMove=this._updateDrag.bindAsEventListener(this);
this.eventOnLoad=this._getWindowBorderSize.bindAsEventListener(this);
this.eventMouseDownContent=this.toFront.bindAsEventListener(this);
this.eventResize=this._recenter.bindAsEventListener(this);
this.topbar=$(this.element.id+"_top");
this.bottombar=$(this.element.id+"_bottom");
this.content=$(this.element.id+"_content");
Event.observe(this.topbar,"mousedown",this.eventMouseDown);
Event.observe(this.bottombar,"mousedown",this.eventMouseDown);
Event.observe(this.content,"mousedown",this.eventMouseDownContent);
Event.observe(window,"load",this.eventOnLoad);
Event.observe(window,"resize",this.eventResize);
Event.observe(window,"scroll",this.eventResize);
if(this.options.draggable){
var _3=this;
[this.topbar,this.topbar.up().previous(),this.topbar.up().next()].each(function(_4){
_4.observe("mousedown",_3.eventMouseDown);
_4.addClassName("top_draggable");
});
[this.bottombar.up(),this.bottombar.up().previous(),this.bottombar.up().next()].each(function(_5){
_5.observe("mousedown",_3.eventMouseDown);
_5.addClassName("bottom_draggable");
});
}
if(this.options.resizable){
this.sizer=$(this.element.id+"_sizer");
Event.observe(this.sizer,"mousedown",this.eventMouseDown);
}
this.useLeft=null;
this.useTop=null;
if(typeof this.options.left!="undefined"){
this.element.setStyle({left:parseFloat(this.options.left)+"px"});
this.useLeft=true;
}else{
this.element.setStyle({right:parseFloat(this.options.right)+"px"});
this.useLeft=false;
}
if(typeof this.options.top!="undefined"){
this.element.setStyle({top:parseFloat(this.options.top)+"px"});
this.useTop=true;
}else{
this.element.setStyle({bottom:parseFloat(this.options.bottom)+"px"});
this.useTop=false;
}
this.storedLocation=null;
this.setOpacity(this.options.opacity);
if(this.options.zIndex){
this.setZIndex(this.options.zIndex);
}
if(this.options.destroyOnClose){
this.setDestroyOnClose(true);
}
this._getWindowBorderSize();
this.width=this.options.width;
this.height=this.options.height;
this.visible=false;
this.constraint=false;
this.constraintPad={top:0,left:0,bottom:0,right:0};
if(this.width&&this.height){
this.setSize(this.options.width,this.options.height);
}
this.setTitle(this.options.title);
Windows.register(this);
},destroy:function(){
this._notify("onDestroy");
Event.stopObserving(this.topbar,"mousedown",this.eventMouseDown);
Event.stopObserving(this.bottombar,"mousedown",this.eventMouseDown);
Event.stopObserving(this.content,"mousedown",this.eventMouseDownContent);
Event.stopObserving(window,"load",this.eventOnLoad);
Event.stopObserving(window,"resize",this.eventResize);
Event.stopObserving(window,"scroll",this.eventResize);
Event.stopObserving(this.content,"load",this.options.onload);
if(this._oldParent){
var _6=this.getContent();
var _7=null;
for(var i=0;i<_6.childNodes.length;i++){
_7=_6.childNodes[i];
if(_7.nodeType==1){
break;
}
_7=null;
}
if(_7){
this._oldParent.appendChild(_7);
}
this._oldParent=null;
}
if(this.sizer){
Event.stopObserving(this.sizer,"mousedown",this.eventMouseDown);
}
if(this.options.url){
this.content.src=null;
}
if(this.iefix){
Element.remove(this.iefix);
}
Element.remove(this.element);
Windows.unregister(this);
},setCloseCallback:function(_9){
this.options.closeCallback=_9;
},getContent:function(){
return this.content;
},setContent:function(id,_b,_c){
var _d=$(id);
if(null==_d){
throw "Unable to find element '"+id+"' in DOM";
}
this._oldParent=_d.parentNode;
var d=null;
var p=null;
if(_b){
d=Element.getDimensions(_d);
}
if(_c){
p=Position.cumulativeOffset(_d);
}
var _10=this.getContent();
this.setHTMLContent("");
_10=this.getContent();
_10.appendChild(_d);
_d.show();
if(_b){
this.setSize(d.width,d.height);
}
if(_c){
this.setLocation(p[1]-this.heightN,p[0]-this.widthW);
}
},setHTMLContent:function(_11){
if(this.options.url){
this.content.src=null;
this.options.url=null;
var _12="<div id=\""+this.getId()+"_content\" class=\""+this.options.className+"_content\"> </div>";
$(this.getId()+"_table_content").innerHTML=_12;
this.content=$(this.element.id+"_content");
}
this.getContent().innerHTML=_11;
},setAjaxContent:function(url,_14,_15,_16){
this.showFunction=_15?"showCenter":"show";
this.showModal=_16||false;
_14=_14||{};
this.setHTMLContent("");
this.onComplete=_14.onComplete;
if(!this._onCompleteHandler){
this._onCompleteHandler=this._setAjaxContent.bind(this);
}
_14.onComplete=this._onCompleteHandler;
new Ajax.Request(url,_14);
_14.onComplete=this.onComplete;
},_setAjaxContent:function(_17){
Element.update(this.getContent(),_17.responseText);
if(this.onComplete){
this.onComplete(_17);
}
this.onComplete=null;
this[this.showFunction](this.showModal);
},setURL:function(url){
if(this.options.url){
this.content.src=null;
}
this.options.url=url;
var _19="<iframe frameborder='0' name='"+this.getId()+"_content'  id='"+this.getId()+"_content' src='"+url+"' width='"+this.width+"' height='"+this.height+"'> </iframe>";
$(this.getId()+"_table_content").innerHTML=_19;
this.content=$(this.element.id+"_content");
},getURL:function(){
return this.options.url?this.options.url:null;
},refresh:function(){
if(this.options.url){
$(this.element.getAttribute("id")+"_content").src=this.options.url;
}
},setCookie:function(_1a,_1b,_1c,_1d,_1e){
_1a=_1a||this.element.id;
this.cookie=[_1a,_1b,_1c,_1d,_1e];
var _1f=WindowUtilities.getCookie(_1a);
if(_1f){
var _20=_1f.split(",");
var x=_20[0].split(":");
var y=_20[1].split(":");
var w=parseFloat(_20[2]),h=parseFloat(_20[3]);
var _24=_20[4];
var _25=_20[5];
this.setSize(w,h);
if(_24=="true"){
this.doMinimize=true;
}else{
if(_25=="true"){
this.doMaximize=true;
}
}
this.useLeft=x[0]=="l";
this.useTop=y[0]=="t";
this.element.setStyle(this.useLeft?{left:x[1]}:{right:x[1]});
this.element.setStyle(this.useTop?{top:y[1]}:{bottom:y[1]});
}
},getId:function(){
return this.element.id;
},setDestroyOnClose:function(){
this.options.destroyOnClose=true;
},setConstraint:function(_26,_27){
this.constraint=_26;
this.constraintPad=Object.extend(this.constraintPad,_27||{});
if(this.useTop&&this.useLeft){
this.setLocation(parseFloat(this.element.style.top),parseFloat(this.element.style.left));
}
},_initDrag:function(_28){
if(Event.element(_28)==this.sizer&&this.isMinimized()){
return;
}
if(Event.element(_28)!=this.sizer&&this.isMaximized()){
return;
}
if(window.ie&&this.heightN==0){
this._getWindowBorderSize();
}
this.pointer=[this._round(Event.pointerX(_28),this.options.gridX),this._round(Event.pointerY(_28),this.options.gridY)];
if(this.options.wiredDrag){
this.currentDrag=this._createWiredElement();
}else{
this.currentDrag=this.element;
}
if(Event.element(_28)==this.sizer){
this.doResize=true;
this.widthOrg=this.width;
this.heightOrg=this.height;
this.bottomOrg=parseFloat(this.element.getStyle("bottom"));
this.rightOrg=parseFloat(this.element.getStyle("right"));
this._notify("onStartResize");
}else{
this.doResize=false;
var _29=$(this.getId()+"_close");
if(_29&&Position.within(_29,this.pointer[0],this.pointer[1])){
this.currentDrag=null;
return;
}
this.toFront();
if(!this.options.draggable){
return;
}
this._notify("onStartMove");
}
Event.observe(document,"mouseup",this.eventMouseUp,false);
Event.observe(document,"mousemove",this.eventMouseMove,false);
WindowUtilities.disableScreen("__invisible__","__invisible__",this.overlayOpacity);
document.body.ondrag=function(){
return false;
};
document.body.onselectstart=function(){
return false;
};
this.currentDrag.show();
Event.stop(_28);
},_round:function(val,_2b){
return _2b==1?val:val=Math.floor(val/_2b)*_2b;
},_updateDrag:function(_2c){
var _2d=[this._round(Event.pointerX(_2c),this.options.gridX),this._round(Event.pointerY(_2c),this.options.gridY)];
var dx=_2d[0]-this.pointer[0];
var dy=_2d[1]-this.pointer[1];
if(this.doResize){
var w=this.widthOrg+dx;
var h=this.heightOrg+dy;
dx=this.width-this.widthOrg;
dy=this.height-this.heightOrg;
if(this.useLeft){
w=this._updateWidthConstraint(w);
}else{
this.currentDrag.setStyle({right:(this.rightOrg-dx)+"px"});
}
if(this.useTop){
h=this._updateHeightConstraint(h);
}else{
this.currentDrag.setStyle({bottom:(this.bottomOrg-dy)+"px"});
}
this.setSize(w,h);
this._notify("onResize");
}else{
this.pointer=_2d;
if(this.useLeft){
var _32=parseFloat(this.currentDrag.getStyle("left"))+dx;
var _33=this._updateLeftConstraint(_32);
this.pointer[0]+=_33-_32;
this.currentDrag.setStyle({left:_33+"px"});
}else{
this.currentDrag.setStyle({right:parseFloat(this.currentDrag.getStyle("right"))-dx+"px"});
}
if(this.useTop){
var top=parseFloat(this.currentDrag.getStyle("top"))+dy;
var _35=this._updateTopConstraint(top);
this.pointer[1]+=_35-top;
this.currentDrag.setStyle({top:_35+"px"});
}else{
this.currentDrag.setStyle({bottom:parseFloat(this.currentDrag.getStyle("bottom"))-dy+"px"});
}
this._notify("onMove");
}
if(this.iefix){
this._fixIEOverlapping();
}
this._removeStoreLocation();
Event.stop(_2c);
},_endDrag:function(_36){
WindowUtilities.enableScreen("__invisible__");
if(this.doResize){
this._notify("onEndResize");
}else{
this._notify("onEndMove");
}
Event.stopObserving(document,"mouseup",this.eventMouseUp,false);
Event.stopObserving(document,"mousemove",this.eventMouseMove,false);
Event.stop(_36);
this._hideWiredElement();
this._saveCookie();
document.body.ondrag=null;
document.body.onselectstart=null;
},_updateLeftConstraint:function(_37){
if(this.constraint&&this.useLeft&&this.useTop){
var _38=this.options.parent==document.body?WindowUtilities.getPageSize().windowWidth:this.options.parent.getDimensions().width;
if(_37<this.constraintPad.left){
_37=this.constraintPad.left;
}
if(_37+this.width+this.widthE+this.widthW>_38-this.constraintPad.right){
_37=_38-this.constraintPad.right-this.width-this.widthE-this.widthW;
}
}
return _37;
},_updateTopConstraint:function(top){
if(this.constraint&&this.useLeft&&this.useTop){
var _3a=this.options.parent==document.body?WindowUtilities.getPageSize().windowHeight:this.options.parent.getDimensions().height;
var h=this.height+this.heightN+this.heightS;
if(top<this.constraintPad.top){
top=this.constraintPad.top;
}
if(top+h>_3a-this.constraintPad.bottom){
top=_3a-this.constraintPad.bottom-h;
}
}
return top;
},_updateWidthConstraint:function(w){
if(this.constraint&&this.useLeft&&this.useTop){
var _3d=this.options.parent==document.body?WindowUtilities.getPageSize().windowWidth:this.options.parent.getDimensions().width;
var _3e=parseFloat(this.element.getStyle("left"));
if(_3e+w+this.widthE+this.widthW>_3d-this.constraintPad.right){
w=_3d-this.constraintPad.right-_3e-this.widthE-this.widthW;
}
}
return w;
},_updateHeightConstraint:function(h){
if(this.constraint&&this.useLeft&&this.useTop){
var _40=this.options.parent==document.body?WindowUtilities.getPageSize().windowHeight:this.options.parent.getDimensions().height;
var top=parseFloat(this.element.getStyle("top"));
if(top+h+this.heightN+this.heightS>_40-this.constraintPad.bottom){
h=_40-this.constraintPad.bottom-top-this.heightN-this.heightS;
}
}
return h;
},_createWindow:function(id){
var _43=this.options.className;
var win=document.createElement("div");
win.setAttribute("id",id);
win.className="dialog";
var _45;
if(this.options.url){
_45="<iframe frameborder=\"0\" name=\""+id+"_content\"  id=\""+id+"_content\" src=\""+this.options.url+"\"> </iframe>";
}else{
_45="<div id=\""+id+"_content\" class=\""+_43+"_content\"> </div>";
}
var _46=this.options.closable?"<div class='"+_43+"_close' id='"+id+"_close' onclick='Windows.close(\""+id+"\", event)'> </div>":"";
var _47=this.options.minimizable?"<div class='"+_43+"_minimize' id='"+id+"_minimize' onclick='Windows.minimize(\""+id+"\", event)'> </div>":"";
var _48=this.options.maximizable?"<div class='"+_43+"_maximize' id='"+id+"_maximize' onclick='Windows.maximize(\""+id+"\", event)'> </div>":"";
var _49=this.options.resizable?"class='"+_43+"_sizer' id='"+id+"_sizer'":"class='"+_43+"_se'";
var _4a="../themes/default/blank.gif";
win.innerHTML=_46+_47+_48+"      <table id='"+id+"_row1' class=\"top table_window\">        <tr>          <td class='"+_43+"_nw'></td>          <td class='"+_43+"_n'><div id='"+id+"_top' class='"+_43+"_title title_window'>"+this.options.title+"</div></td>          <td class='"+_43+"_ne'></td>        </tr>      </table>      <table id='"+id+"_row2' class=\"mid table_window\">        <tr>          <td class='"+_43+"_w'></td>            <td id='"+id+"_table_content' class='"+_43+"_content' valign='top'>"+_45+"</td>          <td class='"+_43+"_e'></td>        </tr>      </table>        <table id='"+id+"_row3' class=\"bot table_window\">        <tr>          <td class='"+_43+"_sw'></td>            <td class='"+_43+"_s'><div id='"+id+"_bottom' class='status_bar'><span style='float:left; width:1px; height:1px'></span></div></td>            <td "+_49+"></td>        </tr>      </table>    ";
Element.hide(win);
this.options.parent.insertBefore(win,this.options.parent.firstChild);
Event.observe($(id+"_content"),"load",this.options.onload);
return win;
},changeClassName:function(_4b){
var _4c=this.options.className;
var id=this.getId();
var win=this;
$A(["_close","_minimize","_maximize","_sizer","_content"]).each(function(_4f){
win._toggleClassName($(id+_4f),_4c+_4f,_4b+_4f);
});
$$("#"+id+" td").each(function(td){
td.className=td.className.sub(_4c,_4b);
});
this.options.className=_4b;
},_toggleClassName:function(_51,_52,_53){
if(_51){
_51.removeClassName(_52);
_51.addClassName(_53);
}
},setLocation:function(top,_55){
top=this._updateTopConstraint(top);
_55=this._updateLeftConstraint(_55);
var e=this.currentDrag||this.element;
e.setStyle({top:top+"px"});
e.setStyle({left:_55+"px"});
this.useLeft=true;
this.useTop=true;
},getLocation:function(){
var _57={};
if(this.useTop){
_57=Object.extend(_57,{top:this.element.getStyle("top")});
}else{
_57=Object.extend(_57,{bottom:this.element.getStyle("bottom")});
}
if(this.useLeft){
_57=Object.extend(_57,{left:this.element.getStyle("left")});
}else{
_57=Object.extend(_57,{right:this.element.getStyle("right")});
}
return _57;
},getSize:function(){
return {width:this.width,height:this.height};
},setSize:function(_58,_59,_5a){
_58=parseFloat(_58);
_59=parseFloat(_59);
if(!this.minimized&&_58<this.options.minWidth){
_58=this.options.minWidth;
}
if(!this.minimized&&_59<this.options.minHeight){
_59=this.options.minHeight;
}
if(this.options.maxHeight&&_59>this.options.maxHeight){
_59=this.options.maxHeight;
}
if(this.options.maxWidth&&_58>this.options.maxWidth){
_58=this.options.maxWidth;
}
if(this.useTop&&this.useLeft&&Window.hasEffectLib&&Effect.ResizeWindow&&_5a){
new Effect.ResizeWindow(this,null,null,_58,_59,{duration:Window.resizeEffectDuration});
}else{
this.width=_58;
this.height=_59;
var e=this.currentDrag?this.currentDrag:this.element;
e.setStyle({width:_58+this.widthW+this.widthE+"px"});
e.setStyle({height:_59+this.heightN+this.heightS+"px"});
if(!this.currentDrag||this.currentDrag==this.element){
var _5c=$(this.element.id+"_content");
_5c.setStyle({height:_59+"px"});
_5c.setStyle({width:_58+"px"});
}
}
},updateHeight:function(){
this.setSize(this.width,this.content.scrollHeight,true);
},updateWidth:function(){
this.setSize(this.content.scrollWidth,this.height,true);
},toFront:function(){
if(this.element.style.zIndex<Windows.maxZIndex){
this.setZIndex(Windows.maxZIndex+1);
}
this._notify("onFocus");
if(this.iefix){
this._fixIEOverlapping();
}
},show:function(_5d){
if(_5d){
if(typeof this.overlayOpacity=="undefined"){
var _5e=this;
setTimeout(function(){
_5e.show(_5d);
},10);
return;
}
Windows.addModalWindow(this);
this.modal=true;
this.setZIndex(Windows.maxZIndex+1);
Windows.unsetOverflow(this);
}else{
if(!this.element.style.zIndex){
this.setZIndex(Windows.maxZIndex++ +1);
}
}
if(this.oldStyle){
this.getContent().setStyle({overflow:this.oldStyle});
}
if(!this.width||!this.height){
var _5f=WindowUtilities._computeSize(this.content.innerHTML,this.content.id,this.width,this.height,0,this.options.className);
if(this.height){
this.width=_5f+5;
}else{
this.height=_5f+5;
}
}
this.setSize(this.width,this.height);
if(this.centered){
this._center(this.centerTop,this.centerLeft);
}
this._notify("onBeforeShow");
if(this.options.showEffect!=Element.show&&this.options.showEffectOptions){
this.options.showEffect(this.element,this.options.showEffectOptions);
}else{
this.options.showEffect(this.element);
}
this._checkIEOverlapping();
this.visible=true;
WindowUtilities.focusedWindow=this;
this._notify("onShow");
},showCenter:function(_60,top,_62){
this.centered=true;
this.centerTop=top;
this.centerLeft=_62;
this.show(_60);
},isVisible:function(){
return this.visible;
},_center:function(top,_64){
var _65=WindowUtilities.getWindowScroll();
var _66=WindowUtilities.getPageSize();
if(typeof top=="undefined"){
top=(_66.windowHeight-(this.height+this.heightN+this.heightS))/2;
}
top+=_65.top;
if(typeof _64=="undefined"){
_64=(_66.windowWidth-(this.width+this.widthW+this.widthE))/2;
}
_64+=_65.left;
this.setLocation(top,_64);
this.toFront();
},_recenter:function(_67){
if(this.centered){
var _68=WindowUtilities.getPageSize();
if(this.pageSize&&this.pageSize.windowWidth==_68.windowWidth&&this.pageSize.windowHeight==_68.windowHeight){
return;
}
this.pageSize=_68;
if($("overlay_modal")){
$("overlay_modal").setStyle({height:(_68.pageHeight+"px")});
}
if(this.options.recenterAuto){
this._center(this.centerTop,this.centerLeft);
}
}
},hide:function(){
this.visible=false;
if(this.modal){
Windows.removeModalWindow(this);
Windows.resetOverflow();
}
this.oldStyle=this.getContent().getStyle("overflow")||"auto";
this.getContent().setStyle({overflow:"hidden"});
this.options.hideEffect(this.element,this.options.hideEffectOptions);
if(this.iefix){
this.iefix.hide();
}
if(!this.doNotNotifyHide){
this._notify("onHide");
}
},close:function(){
if(this.visible){
if(this.options.closeCallback&&!this.options.closeCallback(this)){
return;
}
if(this.options.destroyOnClose){
var _69=this.destroy.bind(this);
if(this.options.hideEffectOptions.afterFinish){
var _6a=this.options.hideEffectOptions.afterFinish;
this.options.hideEffectOptions.afterFinish=function(){
_6a();
_69();
};
}else{
this.options.hideEffectOptions.afterFinish=function(){
_69();
};
}
}
Windows.updateFocusedWindow();
this.doNotNotifyHide=true;
this.hide();
this.doNotNotifyHide=false;
this._notify("onClose");
}
},minimize:function(){
if(this.resizing){
return;
}
var r2=$(this.getId()+"_row2");
if(!this.minimized){
this.minimized=true;
var dh=r2.getDimensions().height;
this.r2Height=dh;
var h=this.element.getHeight()-dh;
if(this.useLeft&&this.useTop&&Window.hasEffectLib&&Effect.ResizeWindow){
new Effect.ResizeWindow(this,null,null,null,this.height-dh,{duration:Window.resizeEffectDuration});
}else{
this.height-=dh;
this.element.setStyle({height:h+"px"});
r2.hide();
}
if(!this.useTop){
var _6e=parseFloat(this.element.getStyle("bottom"));
this.element.setStyle({bottom:(_6e+dh)+"px"});
}
}else{
this.minimized=false;
var dh=this.r2Height;
this.r2Height=null;
if(this.useLeft&&this.useTop&&Window.hasEffectLib&&Effect.ResizeWindow){
new Effect.ResizeWindow(this,null,null,null,this.height+dh,{duration:Window.resizeEffectDuration});
}else{
var h=this.element.getHeight()+dh;
this.height+=dh;
this.element.setStyle({height:h+"px"});
r2.show();
}
if(!this.useTop){
var _71=parseFloat(this.element.getStyle("bottom"));
this.element.setStyle({bottom:(_71-dh)+"px"});
}
this.toFront();
}
this._notify("onMinimize");
this._saveCookie();
},maximize:function(){
if(this.isMinimized()||this.resizing){
return;
}
if(window.ie&&this.heightN==0){
this._getWindowBorderSize();
}
if(this.storedLocation!=null){
this._restoreLocation();
if(this.iefix){
this.iefix.hide();
}
}else{
this._storeLocation();
Windows.unsetOverflow(this);
var _72=WindowUtilities.getWindowScroll();
var _73=WindowUtilities.getPageSize();
var _74=_72.left;
var top=_72.top;
if(this.options.parent!=document.body){
_72={top:0,left:0,bottom:0,right:0};
var dim=this.options.parent.getDimensions();
_73.windowWidth=dim.width;
_73.windowHeight=dim.height;
top=0;
_74=0;
}
if(this.constraint){
_73.windowWidth-=Math.max(0,this.constraintPad.left)+Math.max(0,this.constraintPad.right);
_73.windowHeight-=Math.max(0,this.constraintPad.top)+Math.max(0,this.constraintPad.bottom);
_74+=Math.max(0,this.constraintPad.left);
top+=Math.max(0,this.constraintPad.top);
}
var _77=_73.windowWidth-this.widthW-this.widthE;
var _78=_73.windowHeight-this.heightN-this.heightS;
if(this.useLeft&&this.useTop&&Window.hasEffectLib&&Effect.ResizeWindow){
new Effect.ResizeWindow(this,top,_74,_77,_78,{duration:Window.resizeEffectDuration});
}else{
this.setSize(_77,_78);
this.element.setStyle(this.useLeft?{left:_74}:{right:_74});
this.element.setStyle(this.useTop?{top:top}:{bottom:top});
}
this.toFront();
if(this.iefix){
this._fixIEOverlapping();
}
}
this._notify("onMaximize");
this._saveCookie();
},isMinimized:function(){
return this.minimized;
},isMaximized:function(){
return (this.storedLocation!=null);
},setOpacity:function(_79){
if(Element.setOpacity){
Element.setOpacity(this.element,_79);
}
},setZIndex:function(_7a){
this.element.setStyle({zIndex:_7a});
Windows.updateZindex(_7a,this);
},setTitle:function(_7b){
if(!_7b||_7b==""){
_7b="&nbsp;";
}
Element.update(this.element.id+"_top",_7b);
},setStatusBar:function(_7c){
var _7d=$(this.getId()+"_bottom");
if(typeof (_7c)=="object"){
if(this.bottombar.firstChild){
this.bottombar.replaceChild(_7c,this.bottombar.firstChild);
}else{
this.bottombar.appendChild(_7c);
}
}else{
this.bottombar.innerHTML=_7c;
}
},_checkIEOverlapping:function(){
if(!this.iefix&&(navigator.appVersion.indexOf("MSIE")>0)&&(navigator.userAgent.indexOf("Opera")<0)&&(this.element.getStyle("position")=="absolute")){
new Insertion.After(this.element.id,"<iframe id=\""+this.element.id+"_iefix\" "+"style=\"display:none;position:absolute;filter:progid:DXImageTransform.Microsoft.Alpha(opacity=0);\" "+"src=\"javascript:false;\" frameborder=\"0\" scrolling=\"no\"></iframe>");
this.iefix=$(this.element.id+"_iefix");
}
if(this.iefix){
setTimeout(this._fixIEOverlapping.bind(this),50);
}
},_fixIEOverlapping:function(){
Position.clone(this.element,this.iefix);
this.iefix.style.zIndex=this.element.style.zIndex-1;
this.iefix.show();
},_getWindowBorderSize:function(_7e){
var div=this._createHiddenDiv(this.options.className+"_n");
this.heightN=Element.getDimensions(div).height;
div.parentNode.removeChild(div);
var div=this._createHiddenDiv(this.options.className+"_s");
this.heightS=Element.getDimensions(div).height;
div.parentNode.removeChild(div);
var div=this._createHiddenDiv(this.options.className+"_e");
this.widthE=Element.getDimensions(div).width;
div.parentNode.removeChild(div);
var div=this._createHiddenDiv(this.options.className+"_w");
this.widthW=Element.getDimensions(div).width;
div.parentNode.removeChild(div);
var div=document.createElement("div");
div.className="overlay_"+this.options.className;
document.body.appendChild(div);
var _84=this;
setTimeout(function(){
_84.overlayOpacity=($(div).getStyle("opacity"));
div.parentNode.removeChild(div);
},10);
if(window.ie){
this.heightS=$(this.getId()+"_row3").getDimensions().height;
this.heightN=$(this.getId()+"_row1").getDimensions().height;
}
if(window.khtml&&!window.webkit){
this.setSize(this.width,this.height);
}
if(this.doMaximize){
this.maximize();
}
if(this.doMinimize){
this.minimize();
}
},_createHiddenDiv:function(_85){
var _86=document.body;
var win=document.createElement("div");
win.setAttribute("id",this.element.id+"_tmp");
win.className=_85;
win.style.display="none";
win.innerHTML="";
_86.insertBefore(win,_86.firstChild);
return win;
},_storeLocation:function(){
if(this.storedLocation==null){
this.storedLocation={useTop:this.useTop,useLeft:this.useLeft,top:this.element.getStyle("top"),bottom:this.element.getStyle("bottom"),left:this.element.getStyle("left"),right:this.element.getStyle("right"),width:this.width,height:this.height};
}
},_restoreLocation:function(){
if(this.storedLocation!=null){
this.useLeft=this.storedLocation.useLeft;
this.useTop=this.storedLocation.useTop;
if(this.useLeft&&this.useTop&&Window.hasEffectLib&&Effect.ResizeWindow){
new Effect.ResizeWindow(this,this.storedLocation.top,this.storedLocation.left,this.storedLocation.width,this.storedLocation.height,{duration:Window.resizeEffectDuration});
}else{
this.element.setStyle(this.useLeft?{left:this.storedLocation.left}:{right:this.storedLocation.right});
this.element.setStyle(this.useTop?{top:this.storedLocation.top}:{bottom:this.storedLocation.bottom});
this.setSize(this.storedLocation.width,this.storedLocation.height);
}
Windows.resetOverflow();
this._removeStoreLocation();
}
},_removeStoreLocation:function(){
this.storedLocation=null;
},_saveCookie:function(){
if(this.cookie){
var _88="";
if(this.useLeft){
_88+="l:"+(this.storedLocation?this.storedLocation.left:this.element.getStyle("left"));
}else{
_88+="r:"+(this.storedLocation?this.storedLocation.right:this.element.getStyle("right"));
}
if(this.useTop){
_88+=",t:"+(this.storedLocation?this.storedLocation.top:this.element.getStyle("top"));
}else{
_88+=",b:"+(this.storedLocation?this.storedLocation.bottom:this.element.getStyle("bottom"));
}
_88+=","+(this.storedLocation?this.storedLocation.width:this.width);
_88+=","+(this.storedLocation?this.storedLocation.height:this.height);
_88+=","+this.isMinimized();
_88+=","+this.isMaximized();
WindowUtilities.setCookie(_88,this.cookie);
}
},_createWiredElement:function(){
if(!this.wiredElement){
if(window.ie){
this._getWindowBorderSize();
}
var div=document.createElement("div");
div.className="wired_frame "+this.options.className+"_wired_frame";
div.style.position="absolute";
this.options.parent.insertBefore(div,this.options.parent.firstChild);
this.wiredElement=$(div);
}
if(this.useLeft){
this.wiredElement.setStyle({left:this.element.getStyle("left")});
}else{
this.wiredElement.setStyle({right:this.element.getStyle("right")});
}
if(this.useTop){
this.wiredElement.setStyle({top:this.element.getStyle("top")});
}else{
this.wiredElement.setStyle({bottom:this.element.getStyle("bottom")});
}
var dim=this.element.getDimensions();
this.wiredElement.setStyle({width:dim.width+"px",height:dim.height+"px"});
this.wiredElement.setStyle({zIndex:Windows.maxZIndex+30});
return this.wiredElement;
},_hideWiredElement:function(){
if(!this.wiredElement||!this.currentDrag){
return;
}
if(this.currentDrag==this.element){
this.currentDrag=null;
}else{
if(this.useLeft){
this.element.setStyle({left:this.currentDrag.getStyle("left")});
}else{
this.element.setStyle({right:this.currentDrag.getStyle("right")});
}
if(this.useTop){
this.element.setStyle({top:this.currentDrag.getStyle("top")});
}else{
this.element.setStyle({bottom:this.currentDrag.getStyle("bottom")});
}
this.currentDrag.hide();
this.currentDrag=null;
if(this.doResize){
this.setSize(this.width,this.height);
}
}
},_notify:function(_8b){
if(this.options[_8b]){
this.options[_8b](this);
}else{
Windows.notify(_8b,this);
}
}};
var Windows={windows:[],modalWindows:[],observers:[],focusedWindow:null,maxZIndex:0,overlayShowEffectOptions:{duration:0.5},overlayHideEffectOptions:{duration:0.5},addObserver:function(_8c){
this.removeObserver(_8c);
this.observers.push(_8c);
},removeObserver:function(_8d){
this.observers=this.observers.reject(function(o){
return o==_8d;
});
},notify:function(_8f,win){
this.observers.each(function(o){
if(o[_8f]){
o[_8f](_8f,win);
}
});
},getWindow:function(id){
return this.windows.detect(function(d){
return d.getId()==id;
});
},getFocusedWindow:function(){
return this.focusedWindow;
},updateFocusedWindow:function(){
this.focusedWindow=this.windows.length>=2?this.windows[this.windows.length-2]:null;
},register:function(win){
this.windows.push(win);
},addModalWindow:function(win){
if(this.modalWindows.length==0){
WindowUtilities.disableScreen(win.options.className,"overlay_modal",win.overlayOpacity,win.getId());
}else{
if(Window.keepMultiModalWindow){
$("overlay_modal").style.zIndex=Windows.maxZIndex+1;
Windows.maxZIndex+=1;
WindowUtilities._hideSelect(this.modalWindows.last().getId());
}else{
this.modalWindows.last().element.hide();
}
WindowUtilities._showSelect(win.getId());
}
this.modalWindows.push(win);
},removeModalWindow:function(win){
this.modalWindows.pop();
if(this.modalWindows.length==0){
WindowUtilities.enableScreen();
}else{
if(Window.keepMultiModalWindow){
this.modalWindows.last().toFront();
WindowUtilities._showSelect(this.modalWindows.last().getId());
}else{
this.modalWindows.last().element.show();
}
}
},register:function(win){
this.windows.push(win);
},unregister:function(win){
this.windows=this.windows.reject(function(d){
return d==win;
});
},closeAll:function(){
this.windows.each(function(w){
Windows.close(w.getId());
});
},closeAllModalWindows:function(){
WindowUtilities.enableScreen();
this.modalWindows.each(function(win){
if(win){
win.close();
}
});
},minimize:function(id,_9d){
var win=this.getWindow(id);
if(win&&win.visible){
win.minimize();
}
Event.stop(_9d);
},maximize:function(id,_a0){
var win=this.getWindow(id);
if(win&&win.visible){
win.maximize();
}
Event.stop(_a0);
},close:function(id,_a3){
var win=this.getWindow(id);
if(win){
win.close();
}
if(_a3){
Event.stop(_a3);
}
},unsetOverflow:function(_a5){
this.windows.each(function(d){
d.oldOverflow=d.getContent().getStyle("overflow")||"auto";
d.getContent().setStyle({overflow:"hidden"});
});
if(_a5&&_a5.oldOverflow){
_a5.getContent().setStyle({overflow:_a5.oldOverflow});
}
},resetOverflow:function(){
this.windows.each(function(d){
if(d.oldOverflow){
d.getContent().setStyle({overflow:d.oldOverflow});
}
});
},updateZindex:function(_a8,win){
if(_a8>this.maxZIndex){
this.maxZIndex=_a8;
}
this.focusedWindow=win;
}};
var Dialog={dialogId:null,onCompleteFunc:null,callFunc:null,parameters:null,confirm:function(_aa,_ab){
if(_aa&&typeof _aa!="string"){
Dialog._runAjaxRequest(_aa,_ab,Dialog.confirm);
return;
}
_aa=_aa||"";
_ab=_ab||{};
var _ac=_ab.okLabel?_ab.okLabel:"Ok";
var _ad=_ab.cancelLabel?_ab.cancelLabel:"Cancel";
_ab=Object.extend(_ab,_ab.windowParameters||{});
_ab.windowParameters=_ab.windowParameters||{};
_ab.className=_ab.className||"alert";
var _ae="class ='"+(_ab.buttonClass?_ab.buttonClass+" ":"")+" ok_button'";
var _af="class ='"+(_ab.buttonClass?_ab.buttonClass+" ":"")+" cancel_button'";
var _b0="      <div class='"+_ab.className+"_message'>"+_b0+"</div>        <div class='"+_ab.className+"_buttons'>          <input type='button' value='"+_ac+"' onclick='Dialog.okCallback()' "+_ae+"/>          <input type='button' value='"+_ad+"' onclick='Dialog.cancelCallback()' "+_af+"/>        </div>    ";
return this._openDialog(_b0,_ab);
},alert:function(_b1,_b2){
if(_b1&&typeof _b1!="string"){
Dialog._runAjaxRequest(_b1,_b2,Dialog.alert);
return;
}
_b1=_b1||"";
_b2=_b2||{};
var _b3=_b2.okLabel?_b2.okLabel:"Ok";
_b2=Object.extend(_b2,_b2.windowParameters||{});
_b2.windowParameters=_b2.windowParameters||{};
_b2.className=_b2.className||"alert";
var _b4="class ='"+(_b2.buttonClass?_b2.buttonClass+" ":"")+" ok_button'";
var _b5="      <div class='"+_b2.className+"_message'>"+_b5+"</div>        <div class='"+_b2.className+"_buttons'>          <input type='button' value='"+_b3+"' onclick='Dialog.okCallback()' "+_b4+"/>        </div>";
return this._openDialog(_b5,_b2);
},info:function(_b6,_b7){
if(_b6&&typeof _b6!="string"){
Dialog._runAjaxRequest(_b6,_b7,Dialog.info);
return;
}
_b6=_b6||"";
_b7=_b7||{};
_b7=Object.extend(_b7,_b7.windowParameters||{});
_b7.windowParameters=_b7.windowParameters||{};
_b7.className=_b7.className||"alert";
var _b8="<div id='modal_dialog_message' class='"+_b7.className+"_message'>"+_b8+"</div>";
if(_b7.showProgress){
_b8+="<div id='modal_dialog_progress' class='"+_b7.className+"_progress'>  </div>";
}
_b7.ok=null;
_b7.cancel=null;
return this._openDialog(_b8,_b7);
},setInfoMessage:function(_b9){
$("modal_dialog_message").update(_b9);
},closeInfo:function(){
Windows.close(this.dialogId);
},_openDialog:function(_ba,_bb){
var _bc=_bb.className;
if(!_bb.height&&!_bb.width){
_bb.width=WindowUtilities.getPageSize().pageWidth/2;
}
if(_bb.id){
this.dialogId=_bb.id;
}else{
var t=new Date();
this.dialogId="modal_dialog_"+t.getTime();
_bb.id=this.dialogId;
}
if(!_bb.height||!_bb.width){
var _be=WindowUtilities._computeSize(_ba,this.dialogId,_bb.width,_bb.height,5,_bc);
if(_bb.height){
_bb.width=_be+5;
}else{
_bb.height=_be+5;
}
}
_bb.resizable=_bb.resizable||false;
_bb.effectOptions=_bb.effectOptions;
_bb.minimizable=false;
_bb.maximizable=false;
_bb.draggable=false;
_bb.closable=false;
var win=new Window(_bb);
win.getContent().innerHTML=_ba;
win.showCenter(true,_bb.top,_bb.left);
win.setDestroyOnClose();
win.cancelCallback=_bb.onCancel||_bb.cancel;
win.okCallback=_bb.onOk||_bb.ok;
return win;
},_getAjaxContent:function(_c0){
Dialog.callFunc(_c0.responseText,Dialog.parameters);
},_runAjaxRequest:function(_c1,_c2,_c3){
if(_c1.options==null){
_c1.options={};
}
Dialog.onCompleteFunc=_c1.options.onComplete;
Dialog.parameters=_c2;
Dialog.callFunc=_c3;
_c1.options.onComplete=Dialog._getAjaxContent;
new Ajax.Request(_c1.url,_c1.options);
},okCallback:function(){
var win=Windows.focusedWindow;
if(!win.okCallback||win.okCallback(win)){
$$("#"+win.getId()+" input").each(function(_c5){
_c5.onclick=null;
});
win.close();
}
},cancelCallback:function(){
var win=Windows.focusedWindow;
$$("#"+win.getId()+" input").each(function(_c7){
_c7.onclick=null;
});
win.close();
if(win.cancelCallback){
win.cancelCallback(win);
}
}};
if(window.ActiveXObject){
window.ie=window[window.XMLHttpRequest?"ie7":"ie6"]=true;
}else{
if(document.childNodes&&!document.all&&!navigator.taintEnabled){
window.khtml=true;
}else{
if(document.getBoxObjectFor!=null){
window.gecko=true;
}
}
}
var array=navigator.userAgent.match(new RegExp(/AppleWebKit\/([\d\.\+]*)/));
window.webkit=array&&array.length==2?parseFloat(array[1])>=420:false;
var WindowUtilities={getWindowScroll:function(){
var w=window;
var T,L,W,H;
L=window.pageXOffset||document.documentElement.scrollLeft;
T=window.pageYOffset||document.documentElement.scrollTop;
if(window.ie){
W=Math.max(document.documentElement.offsetWidth,document.documentElement.scrollWidth);
}else{
if(window.khtml){
W=document.body.scrollWidth;
}else{
W=document.documentElement.scrollWidth;
}
}
if(window.ie){
H=Math.max(document.documentElement.offsetHeight,document.documentElement.scrollHeight);
}else{
if(window.khtml){
H=document.body.scrollHeight;
}else{
H=document.documentElement.scrollHeight;
}
}
return {top:T,left:L,width:W,height:H};
},getPageSize:function(){
var _ca,yScroll;
if(window.innerHeight&&window.scrollMaxY){
_ca=document.body.scrollWidth;
yScroll=window.innerHeight+window.scrollMaxY;
}else{
if(document.body.scrollHeight>document.body.offsetHeight){
_ca=document.body.scrollWidth;
yScroll=document.body.scrollHeight;
}else{
_ca=document.body.offsetWidth;
yScroll=document.body.offsetHeight;
}
}
var _cb,windowHeight;
if(self.innerHeight){
_cb=self.innerWidth;
windowHeight=self.innerHeight;
}else{
if(document.documentElement&&document.documentElement.clientHeight){
_cb=document.documentElement.clientWidth;
windowHeight=document.documentElement.clientHeight;
}else{
if(document.body){
_cb=document.body.clientWidth;
windowHeight=document.body.clientHeight;
}
}
}
var _cc,pageWidth;
if(yScroll<windowHeight){
_cc=windowHeight;
}else{
_cc=yScroll;
}
if(_ca<_cb){
pageWidth=_cb;
}else{
pageWidth=_ca;
}
return {pageWidth:pageWidth,pageHeight:_cc,windowWidth:_cb,windowHeight:windowHeight};
},disableScreen:function(_cd,_ce,_cf,_d0){
var _d1=this;
WindowUtilities.initLightbox(_ce,_cd,function(){
_d1._disableScreen(_cd,_ce,_cf,_d0);
});
},_disableScreen:function(_d2,_d3,_d4,_d5){
var _d6=document.body;
var _d7=$(_d3);
var _d8=WindowUtilities.getPageSize();
if(_d5&&window.ie){
WindowUtilities._hideSelect();
WindowUtilities._showSelect(_d5);
}
_d7.style.height=(_d8.pageHeight+"px");
_d7.style.display="none";
if(_d3=="overlay_modal"&&Window.hasEffectLib&&Windows.overlayShowEffectOptions){
_d7.overlayOpacity=_d4;
new Effect.Appear(_d7,Object.extend({from:0,to:_d4},Windows.overlayShowEffectOptions));
}else{
_d7.style.display="block";
}
},enableScreen:function(id){
id=id||"overlay_modal";
var _da=$(id);
if(_da){
if(id=="overlay_modal"&&Window.hasEffectLib&&Windows.overlayHideEffectOptions){
new Effect.Fade(_da,Object.extend({from:_da.overlayOpacity,to:0},Windows.overlayHideEffectOptions));
}else{
_da.style.display="none";
_da.parentNode.removeChild(_da);
}
if(id!="__invisible__"){
WindowUtilities._showSelect();
}
}
},_hideSelect:function(id){
if(window.ie){
id=id==null?"":"#"+id+" ";
$$(id+"select").each(function(_dc){
if(!WindowUtilities.isDefined(_dc.oldVisibility)){
_dc.oldVisibility=_dc.style.visibility?_dc.style.visibility:"visible";
_dc.style.visibility="hidden";
}
});
}
},_showSelect:function(id){
if(window.ie){
id=id==null?"":"#"+id+" ";
$$(id+"select").each(function(_de){
if(WindowUtilities.isDefined(_de.oldVisibility)){
try{
_de.style.visibility=_de.oldVisibility;
}
catch(e){
_de.style.visibility="visible";
}
_de.oldVisibility=null;
}else{
if(_de.style.visibility){
_de.style.visibility="visible";
}
}
});
}
},isDefined:function(_df){
return typeof (_df)!="undefined"&&_df!=null;
},initLightbox:function(id,_e1,_e2){
if($(id)){
Element.setStyle(id,{zIndex:Windows.maxZIndex+1});
Windows.maxZIndex++;
_e2();
}else{
var _e3=document.body;
var _e4=document.createElement("div");
_e4.setAttribute("id",id);
_e4.className="overlay_"+_e1;
_e4.style.display="none";
_e4.style.position="absolute";
_e4.style.top="0";
_e4.style.left="0";
_e4.style.zIndex=Windows.maxZIndex+1;
Windows.maxZIndex++;
_e4.style.width="100%";
_e3.insertBefore(_e4,_e3.firstChild);
if(window.khtml&&id=="overlay_modal"){
setTimeout(function(){
_e2();
},10);
}else{
_e2();
}
}
},setCookie:function(_e5,_e6){
document.cookie=_e6[0]+"="+escape(_e5)+((_e6[1])?"; expires="+_e6[1].toGMTString():"")+((_e6[2])?"; path="+_e6[2]:"")+((_e6[3])?"; domain="+_e6[3]:"")+((_e6[4])?"; secure":"");
},getCookie:function(_e7){
var dc=document.cookie;
var _e9=_e7+"=";
var _ea=dc.indexOf("; "+_e9);
if(_ea==-1){
_ea=dc.indexOf(_e9);
if(_ea!=0){
return null;
}
}else{
_ea+=2;
}
var end=document.cookie.indexOf(";",_ea);
if(end==-1){
end=dc.length;
}
return unescape(dc.substring(_ea+_e9.length,end));
},_computeSize:function(_ec,id,_ee,_ef,_f0,_f1){
var _f2=document.body;
var _f3=document.createElement("div");
_f3.setAttribute("id",id);
_f3.className=_f1+"_content";
if(_ef){
_f3.style.height=_ef+"px";
}else{
_f3.style.width=_ee+"px";
}
_f3.style.position="absolute";
_f3.style.top="0";
_f3.style.left="0";
_f3.style.display="none";
_f3.innerHTML=_ec;
_f2.insertBefore(_f3,_f2.firstChild);
var _f4;
if(_ef){
_f4=$(id).getDimensions().width+_f0;
}else{
_f4=$(id).getDimensions().height+_f0;
}
_f2.removeChild(_f3);
return _f4;
}};

