var Window=Class.create();
Window.keepMultiModalWindow=false;
Window.hasEffectLib=(typeof Effect!="undefined");
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
this.options=Object.extend({className:"dialog",blurClassName:null,minWidth:100,minHeight:20,resizable:true,closable:true,minimizable:true,maximizable:true,draggable:true,userData:null,showEffect:(Window.hasEffectLib?Effect.Appear:Element.show),hideEffect:(Window.hasEffectLib?Effect.Fade:Element.hide),showEffectOptions:{},hideEffectOptions:{},effectOptions:null,parent:document.body,title:"&nbsp;",url:null,onload:Prototype.emptyFunction,width:200,height:300,opacity:1,recenterAuto:true,wiredDrag:false,closeCallback:null,destroyOnClose:false,gridX:1,gridY:1},arguments[_2]||{});
if(this.options.blurClassName){
this.options.focusClassName=this.options.className;
}
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
this.element.win=this;
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
Event.observe(this.options.parent,"scroll",this.eventResize);
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
var _25=_20[4];
var _26=_20[5];
this.setSize(w,h);
if(_25=="true"){
this.doMinimize=true;
}else{
if(_26=="true"){
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
},setConstraint:function(_27,_28){
this.constraint=_27;
this.constraintPad=Object.extend(this.constraintPad,_28||{});
if(this.useTop&&this.useLeft){
this.setLocation(parseFloat(this.element.style.top),parseFloat(this.element.style.left));
}
},_initDrag:function(_29){
if(Event.element(_29)==this.sizer&&this.isMinimized()){
return;
}
if(Event.element(_29)!=this.sizer&&this.isMaximized()){
return;
}
if(Prototype.Browser.IE&&this.heightN==0){
this._getWindowBorderSize();
}
this.pointer=[this._round(Event.pointerX(_29),this.options.gridX),this._round(Event.pointerY(_29),this.options.gridY)];
if(this.options.wiredDrag){
this.currentDrag=this._createWiredElement();
}else{
this.currentDrag=this.element;
}
if(Event.element(_29)==this.sizer){
this.doResize=true;
this.widthOrg=this.width;
this.heightOrg=this.height;
this.bottomOrg=parseFloat(this.element.getStyle("bottom"));
this.rightOrg=parseFloat(this.element.getStyle("right"));
this._notify("onStartResize");
}else{
this.doResize=false;
var _2a=$(this.getId()+"_close");
if(_2a&&Position.within(_2a,this.pointer[0],this.pointer[1])){
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
Event.stop(_29);
},_round:function(val,_2c){
return _2c==1?val:val=Math.floor(val/_2c)*_2c;
},_updateDrag:function(_2d){
var _2e=[this._round(Event.pointerX(_2d),this.options.gridX),this._round(Event.pointerY(_2d),this.options.gridY)];
var dx=_2e[0]-this.pointer[0];
var dy=_2e[1]-this.pointer[1];
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
this.pointer=_2e;
if(this.useLeft){
var _33=parseFloat(this.currentDrag.getStyle("left"))+dx;
var _34=this._updateLeftConstraint(_33);
this.pointer[0]+=_34-_33;
this.currentDrag.setStyle({left:_34+"px"});
}else{
this.currentDrag.setStyle({right:parseFloat(this.currentDrag.getStyle("right"))-dx+"px"});
}
if(this.useTop){
var top=parseFloat(this.currentDrag.getStyle("top"))+dy;
var _36=this._updateTopConstraint(top);
this.pointer[1]+=_36-top;
this.currentDrag.setStyle({top:_36+"px"});
}else{
this.currentDrag.setStyle({bottom:parseFloat(this.currentDrag.getStyle("bottom"))-dy+"px"});
}
this._notify("onMove");
}
if(this.iefix){
this._fixIEOverlapping();
}
this._removeStoreLocation();
Event.stop(_2d);
},_endDrag:function(_37){
WindowUtilities.enableScreen("__invisible__");
if(this.doResize){
this._notify("onEndResize");
}else{
this._notify("onEndMove");
}
Event.stopObserving(document,"mouseup",this.eventMouseUp,false);
Event.stopObserving(document,"mousemove",this.eventMouseMove,false);
Event.stop(_37);
this._hideWiredElement();
this._saveCookie();
document.body.ondrag=null;
document.body.onselectstart=null;
},_updateLeftConstraint:function(_38){
if(this.constraint&&this.useLeft&&this.useTop){
var _39=this.options.parent==document.body?WindowUtilities.getPageSize().windowWidth:this.options.parent.getDimensions().width;
if(_38<this.constraintPad.left){
_38=this.constraintPad.left;
}
if(_38+this.width+this.widthE+this.widthW>_39-this.constraintPad.right){
_38=_39-this.constraintPad.right-this.width-this.widthE-this.widthW;
}
}
return _38;
},_updateTopConstraint:function(top){
if(this.constraint&&this.useLeft&&this.useTop){
var _3b=this.options.parent==document.body?WindowUtilities.getPageSize().windowHeight:this.options.parent.getDimensions().height;
var h=this.height+this.heightN+this.heightS;
if(top<this.constraintPad.top){
top=this.constraintPad.top;
}
if(top+h>_3b-this.constraintPad.bottom){
top=_3b-this.constraintPad.bottom-h;
}
}
return top;
},_updateWidthConstraint:function(w){
if(this.constraint&&this.useLeft&&this.useTop){
var _3e=this.options.parent==document.body?WindowUtilities.getPageSize().windowWidth:this.options.parent.getDimensions().width;
var _3f=parseFloat(this.element.getStyle("left"));
if(_3f+w+this.widthE+this.widthW>_3e-this.constraintPad.right){
w=_3e-this.constraintPad.right-_3f-this.widthE-this.widthW;
}
}
return w;
},_updateHeightConstraint:function(h){
if(this.constraint&&this.useLeft&&this.useTop){
var _41=this.options.parent==document.body?WindowUtilities.getPageSize().windowHeight:this.options.parent.getDimensions().height;
var top=parseFloat(this.element.getStyle("top"));
if(top+h+this.heightN+this.heightS>_41-this.constraintPad.bottom){
h=_41-this.constraintPad.bottom-top-this.heightN-this.heightS;
}
}
return h;
},_createWindow:function(id){
var _44=this.options.className;
var win=document.createElement("div");
win.setAttribute("id",id);
win.className="dialog";
var _46;
if(this.options.url){
_46="<iframe frameborder=\"0\" name=\""+id+"_content\"  id=\""+id+"_content\" src=\""+this.options.url+"\"> </iframe>";
}else{
_46="<div id=\""+id+"_content\" class=\""+_44+"_content\"> </div>";
}
var _47=this.options.closable?"<div class='"+_44+"_close' id='"+id+"_close' onclick='Windows.close(\""+id+"\", event)'> </div>":"";
var _48=this.options.minimizable?"<div class='"+_44+"_minimize' id='"+id+"_minimize' onclick='Windows.minimize(\""+id+"\", event)'> </div>":"";
var _49=this.options.maximizable?"<div class='"+_44+"_maximize' id='"+id+"_maximize' onclick='Windows.maximize(\""+id+"\", event)'> </div>":"";
var _4a=this.options.resizable?"class='"+_44+"_sizer' id='"+id+"_sizer'":"class='"+_44+"_se'";
var _4b="../themes/default/blank.gif";
win.innerHTML=_47+_48+_49+"      <table id='"+id+"_row1' class=\"top table_window\">        <tr>          <td class='"+_44+"_nw'></td>          <td class='"+_44+"_n'><div id='"+id+"_top' class='"+_44+"_title title_window'>"+this.options.title+"</div></td>          <td class='"+_44+"_ne'></td>        </tr>      </table>      <table id='"+id+"_row2' class=\"mid table_window\">        <tr>          <td class='"+_44+"_w'></td>            <td id='"+id+"_table_content' class='"+_44+"_content' valign='top'>"+_46+"</td>          <td class='"+_44+"_e'></td>        </tr>      </table>        <table id='"+id+"_row3' class=\"bot table_window\">        <tr>          <td class='"+_44+"_sw'></td>            <td class='"+_44+"_s'><div id='"+id+"_bottom' class='status_bar'><span style='float:left; width:1px; height:1px'></span></div></td>            <td "+_4a+"></td>        </tr>      </table>    ";
Element.hide(win);
this.options.parent.insertBefore(win,this.options.parent.firstChild);
Event.observe($(id+"_content"),"load",this.options.onload);
return win;
},changeClassName:function(_4c){
var _4d=this.options.className;
var id=this.getId();
$A(["_close","_minimize","_maximize","_sizer","_content"]).each(function(_4f){
this._toggleClassName($(id+_4f),_4d+_4f,_4c+_4f);
}.bind(this));
this._toggleClassName($(id+"_top"),_4d+"_title",_4c+"_title");
$$("#"+id+" td").each(function(td){
td.className=td.className.sub(_4d,_4c);
});
this.options.className=_4c;
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
if(this.iefix){
this._fixIEOverlapping();
}
},getBounds:function(_5d){
if(!this.width||!this.height||!this.visible){
this.computeBounds();
}
var w=this.width;
var h=this.height;
if(!_5d){
w+=this.widthW+this.widthE;
h+=this.heightN+this.heightS;
}
var _60=Object.extend(this.getLocation(),{width:w+"px",height:h+"px"});
return _60;
},computeBounds:function(){
if(!this.width||!this.height){
var _61=WindowUtilities._computeSize(this.content.innerHTML,this.content.id,this.width,this.height,0,this.options.className);
if(this.height){
this.width=_61+5;
}else{
this.height=_61+5;
}
}
this.setSize(this.width,this.height);
if(this.centered){
this._center(this.centerTop,this.centerLeft);
}
},show:function(_62){
this.visible=true;
if(_62){
if(typeof this.overlayOpacity=="undefined"){
var _63=this;
setTimeout(function(){
_63.show(_62);
},10);
return;
}
Windows.addModalWindow(this);
this.modal=true;
this.setZIndex(Windows.maxZIndex+1);
Windows.unsetOverflow(this);
}else{
if(!this.element.style.zIndex){
this.setZIndex(Windows.maxZIndex+1);
}
}
if(this.oldStyle){
this.getContent().setStyle({overflow:this.oldStyle});
}
this.computeBounds();
this._notify("onBeforeShow");
if(this.options.showEffect!=Element.show&&this.options.showEffectOptions){
this.options.showEffect(this.element,this.options.showEffectOptions);
}else{
this.options.showEffect(this.element);
}
this._checkIEOverlapping();
WindowUtilities.focusedWindow=this;
this._notify("onShow");
},showCenter:function(_64,top,_66){
this.centered=true;
this.centerTop=top;
this.centerLeft=_66;
this.show(_64);
},isVisible:function(){
return this.visible;
},_center:function(top,_68){
var _69=WindowUtilities.getWindowScroll(this.options.parent);
var _6a=WindowUtilities.getPageSize(this.options.parent);
if(typeof top=="undefined"){
top=(_6a.windowHeight-(this.height+this.heightN+this.heightS))/2;
}
top+=_69.top;
if(typeof _68=="undefined"){
_68=(_6a.windowWidth-(this.width+this.widthW+this.widthE))/2;
}
_68+=_69.left;
this.setLocation(top,_68);
this.toFront();
},_recenter:function(_6b){
if(this.centered){
var _6c=WindowUtilities.getPageSize(this.options.parent);
var _6d=WindowUtilities.getWindowScroll(this.options.parent);
if(this.pageSize&&this.pageSize.windowWidth==_6c.windowWidth&&this.pageSize.windowHeight==_6c.windowHeight&&this.windowScroll.left==_6d.left&&this.windowScroll.top==_6d.top){
return;
}
this.pageSize=_6c;
this.windowScroll=_6d;
if($("overlay_modal")){
$("overlay_modal").setStyle({height:(_6c.pageHeight+"px")});
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
var _6e=this.destroy.bind(this);
if(this.options.hideEffectOptions.afterFinish){
var _6f=this.options.hideEffectOptions.afterFinish;
this.options.hideEffectOptions.afterFinish=function(){
_6f();
_6e();
};
}else{
this.options.hideEffectOptions.afterFinish=function(){
_6e();
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
var _73=parseFloat(this.element.getStyle("bottom"));
this.element.setStyle({bottom:(_73+dh)+"px"});
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
var _73=parseFloat(this.element.getStyle("bottom"));
this.element.setStyle({bottom:(_73-dh)+"px"});
}
this.toFront();
}
this._notify("onMinimize");
this._saveCookie();
},maximize:function(){
if(this.isMinimized()||this.resizing){
return;
}
if(Prototype.Browser.IE&&this.heightN==0){
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
var _74=WindowUtilities.getWindowScroll(this.options.parent);
var _75=WindowUtilities.getPageSize(this.options.parent);
var _76=_74.left;
var top=_74.top;
if(this.options.parent!=document.body){
_74={top:0,left:0,bottom:0,right:0};
var dim=this.options.parent.getDimensions();
_75.windowWidth=dim.width;
_75.windowHeight=dim.height;
top=0;
_76=0;
}
if(this.constraint){
_75.windowWidth-=Math.max(0,this.constraintPad.left)+Math.max(0,this.constraintPad.right);
_75.windowHeight-=Math.max(0,this.constraintPad.top)+Math.max(0,this.constraintPad.bottom);
_76+=Math.max(0,this.constraintPad.left);
top+=Math.max(0,this.constraintPad.top);
}
var _79=_75.windowWidth-this.widthW-this.widthE;
var _7a=_75.windowHeight-this.heightN-this.heightS;
if(this.useLeft&&this.useTop&&Window.hasEffectLib&&Effect.ResizeWindow){
new Effect.ResizeWindow(this,top,_76,_79,_7a,{duration:Window.resizeEffectDuration});
}else{
this.setSize(_79,_7a);
this.element.setStyle(this.useLeft?{left:_76}:{right:_76});
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
},setOpacity:function(_7b){
if(Element.setOpacity){
Element.setOpacity(this.element,_7b);
}
},setZIndex:function(_7c){
this.element.setStyle({zIndex:_7c});
Windows.updateZindex(_7c,this);
},setTitle:function(_7d){
if(!_7d||_7d==""){
_7d="&nbsp;";
}
Element.update(this.element.id+"_top",_7d);
},getTitle:function(){
return $(this.element.id+"_top").innerHTML;
},setStatusBar:function(_7e){
var _7f=$(this.getId()+"_bottom");
if(typeof (_7e)=="object"){
if(this.bottombar.firstChild){
this.bottombar.replaceChild(_7e,this.bottombar.firstChild);
}else{
this.bottombar.appendChild(_7e);
}
}else{
this.bottombar.innerHTML=_7e;
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
},_getWindowBorderSize:function(_80){
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
var _82=this;
setTimeout(function(){
_82.overlayOpacity=($(div).getStyle("opacity"));
div.parentNode.removeChild(div);
},10);
if(Prototype.Browser.IE){
this.heightS=$(this.getId()+"_row3").getDimensions().height;
this.heightN=$(this.getId()+"_row1").getDimensions().height;
}
if(Prototype.Browser.WebKit&&Prototype.Browser.WebKitVersion<420){
this.setSize(this.width,this.height);
}
if(this.doMaximize){
this.maximize();
}
if(this.doMinimize){
this.minimize();
}
},_createHiddenDiv:function(_83){
var _84=document.body;
var win=document.createElement("div");
win.setAttribute("id",this.element.id+"_tmp");
win.className=_83;
win.style.display="none";
win.innerHTML="";
_84.insertBefore(win,_84.firstChild);
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
var _86="";
if(this.useLeft){
_86+="l:"+(this.storedLocation?this.storedLocation.left:this.element.getStyle("left"));
}else{
_86+="r:"+(this.storedLocation?this.storedLocation.right:this.element.getStyle("right"));
}
if(this.useTop){
_86+=",t:"+(this.storedLocation?this.storedLocation.top:this.element.getStyle("top"));
}else{
_86+=",b:"+(this.storedLocation?this.storedLocation.bottom:this.element.getStyle("bottom"));
}
_86+=","+(this.storedLocation?this.storedLocation.width:this.width);
_86+=","+(this.storedLocation?this.storedLocation.height:this.height);
_86+=","+this.isMinimized();
_86+=","+this.isMaximized();
WindowUtilities.setCookie(_86,this.cookie);
}
},_createWiredElement:function(){
if(!this.wiredElement){
if(Prototype.Browser.IE){
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
},_notify:function(_89){
if(this.options[_89]){
this.options[_89](this);
}else{
Windows.notify(_89,this);
}
}};
var Windows={windows:[],modalWindows:[],observers:[],focusedWindow:null,maxZIndex:0,overlayShowEffectOptions:{duration:0.5},overlayHideEffectOptions:{duration:0.5},addObserver:function(_8a){
this.removeObserver(_8a);
this.observers.push(_8a);
},removeObserver:function(_8b){
this.observers=this.observers.reject(function(o){
return o==_8b;
});
},notify:function(_8d,win){
this.observers.each(function(o){
if(o[_8d]){
o[_8d](_8d,win);
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
WindowUtilities.disableScreen(win.options.className,"overlay_modal",win.overlayOpacity,win.getId(),win.options.parent);
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
},minimize:function(id,_9b){
var win=this.getWindow(id);
if(win&&win.visible){
win.minimize();
}
Event.stop(_9b);
},maximize:function(id,_9e){
var win=this.getWindow(id);
if(win&&win.visible){
win.maximize();
}
Event.stop(_9e);
},close:function(id,_a1){
var win=this.getWindow(id);
if(win){
win.close();
}
if(_a1){
Event.stop(_a1);
}
},blur:function(id){
var win=this.getWindow(id);
if(!win){
return;
}
if(win.options.blurClassName){
win.changeClassName(win.options.blurClassName);
}
if(this.focusedWindow==win){
this.focusedWindow=null;
}
win._notify("onBlur");
},focus:function(id){
var win=this.getWindow(id);
if(!win){
return;
}
if(this.focusedWindow){
this.blur(this.focusedWindow.getId());
}
if(win.options.focusClassName){
win.changeClassName(win.options.focusClassName);
}
this.focusedWindow=win;
win._notify("onFocus");
},unsetOverflow:function(_a7){
this.windows.each(function(d){
d.oldOverflow=d.getContent().getStyle("overflow")||"auto";
d.getContent().setStyle({overflow:"hidden"});
});
if(_a7&&_a7.oldOverflow){
_a7.getContent().setStyle({overflow:_a7.oldOverflow});
}
},resetOverflow:function(){
this.windows.each(function(d){
if(d.oldOverflow){
d.getContent().setStyle({overflow:d.oldOverflow});
}
});
},updateZindex:function(_aa,win){
if(_aa>this.maxZIndex){
this.maxZIndex=_aa;
if(this.focusedWindow){
this.blur(this.focusedWindow.getId());
}
}
this.focusedWindow=win;
if(this.focusedWindow){
this.focus(this.focusedWindow.getId());
}
}};
var Dialog={dialogId:null,onCompleteFunc:null,callFunc:null,parameters:null,confirm:function(_ac,_ad){
if(_ac&&typeof _ac!="string"){
Dialog._runAjaxRequest(_ac,_ad,Dialog.confirm);
return;
}
_ac=_ac||"";
_ad=_ad||{};
var _ae=_ad.okLabel?_ad.okLabel:"Ok";
var _af=_ad.cancelLabel?_ad.cancelLabel:"Cancel";
_ad=Object.extend(_ad,_ad.windowParameters||{});
_ad.windowParameters=_ad.windowParameters||{};
_ad.className=_ad.className||"alert";
var _b0="class ='"+(_ad.buttonClass?_ad.buttonClass+" ":"")+" ok_button'";
var _b1="class ='"+(_ad.buttonClass?_ad.buttonClass+" ":"")+" cancel_button'";
var _ac="      <div class='"+_ad.className+"_message'>"+_ac+"</div>        <div class='"+_ad.className+"_buttons'>          <input type='button' value='"+_ae+"' onclick='Dialog.okCallback()' "+_b0+"/>          <input type='button' value='"+_af+"' onclick='Dialog.cancelCallback()' "+_b1+"/>        </div>    ";
return this._openDialog(_ac,_ad);
},alert:function(_b2,_b3){
if(_b2&&typeof _b2!="string"){
Dialog._runAjaxRequest(_b2,_b3,Dialog.alert);
return;
}
_b2=_b2||"";
_b3=_b3||{};
var _b4=_b3.okLabel?_b3.okLabel:"Ok";
_b3=Object.extend(_b3,_b3.windowParameters||{});
_b3.windowParameters=_b3.windowParameters||{};
_b3.className=_b3.className||"alert";
var _b5="class ='"+(_b3.buttonClass?_b3.buttonClass+" ":"")+" ok_button'";
var _b2="      <div class='"+_b3.className+"_message'>"+_b2+"</div>        <div class='"+_b3.className+"_buttons'>          <input type='button' value='"+_b4+"' onclick='Dialog.okCallback()' "+_b5+"/>        </div>";
return this._openDialog(_b2,_b3);
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
var _b6="<div id='modal_dialog_message' class='"+_b7.className+"_message'>"+_b6+"</div>";
if(_b7.showProgress){
_b6+="<div id='modal_dialog_progress' class='"+_b7.className+"_progress'>  </div>";
}
_b7.ok=null;
_b7.cancel=null;
return this._openDialog(_b6,_b7);
},setInfoMessage:function(_b8){
$("modal_dialog_message").update(_b8);
},closeInfo:function(){
Windows.close(this.dialogId);
},_openDialog:function(_b9,_ba){
var _bb=_ba.className;
if(!_ba.height&&!_ba.width){
_ba.width=WindowUtilities.getPageSize(_ba.options.parent||document.body).pageWidth/2;
}
if(_ba.id){
this.dialogId=_ba.id;
}else{
var t=new Date();
this.dialogId="modal_dialog_"+t.getTime();
_ba.id=this.dialogId;
}
if(!_ba.height||!_ba.width){
var _bd=WindowUtilities._computeSize(_b9,this.dialogId,_ba.width,_ba.height,5,_bb);
if(_ba.height){
_ba.width=_bd+5;
}else{
_ba.height=_bd+5;
}
}
_ba.effectOptions=_ba.effectOptions;
_ba.resizable=_ba.resizable||false;
_ba.minimizable=_ba.minimizable||false;
_ba.maximizable=_ba.maximizable||false;
_ba.draggable=_ba.draggable||false;
_ba.closable=_ba.closable||false;
var win=new Window(_ba);
win.getContent().innerHTML=_b9;
win.showCenter(true,_ba.top,_ba.left);
win.setDestroyOnClose();
win.cancelCallback=_ba.onCancel||_ba.cancel;
win.okCallback=_ba.onOk||_ba.ok;
return win;
},_getAjaxContent:function(_bf){
Dialog.callFunc(_bf.responseText,Dialog.parameters);
},_runAjaxRequest:function(_c0,_c1,_c2){
if(_c0.options==null){
_c0.options={};
}
Dialog.onCompleteFunc=_c0.options.onComplete;
Dialog.parameters=_c1;
Dialog.callFunc=_c2;
_c0.options.onComplete=Dialog._getAjaxContent;
new Ajax.Request(_c0.url,_c0.options);
},okCallback:function(){
var win=Windows.focusedWindow;
if(!win.okCallback||win.okCallback(win)){
$$("#"+win.getId()+" input").each(function(_c4){
_c4.onclick=null;
});
win.close();
}
},cancelCallback:function(){
var win=Windows.focusedWindow;
$$("#"+win.getId()+" input").each(function(_c6){
_c6.onclick=null;
});
win.close();
if(win.cancelCallback){
win.cancelCallback(win);
}
}};
if(Prototype.Browser.WebKit){
var array=navigator.userAgent.match(new RegExp(/AppleWebKit\/([\d\.\+]*)/));
Prototype.Browser.WebKitVersion=parseFloat(array[1]);
}
var WindowUtilities={getWindowScroll:function(_c7){
var T,L,W,H;
_c7=_c7||document.body;
if(_c7!=document.body){
T=_c7.scrollTop;
L=_c7.scrollLeft;
W=_c7.scrollWidth;
H=_c7.scrollHeight;
}else{
var w=window;
with(w.document){
if(w.document.documentElement&&documentElement.scrollTop){
T=documentElement.scrollTop;
L=documentElement.scrollLeft;
}else{
if(w.document.body){
T=body.scrollTop;
L=body.scrollLeft;
}
}
if(w.innerWidth){
W=w.innerWidth;
H=w.innerHeight;
}else{
if(w.document.documentElement&&documentElement.clientWidth){
W=documentElement.clientWidth;
H=documentElement.clientHeight;
}else{
W=body.offsetWidth;
H=body.offsetHeight;
}
}
}
}
return {top:T,left:L,width:W,height:H};
},getPageSize:function(_cd){
_cd=_cd||document.body;
var _ce,_cf;
var _d0,_d1;
if(_cd!=document.body){
_ce=_cd.getWidth();
_cf=_cd.getHeight();
_d1=_cd.scrollWidth;
_d0=_cd.scrollHeight;
}else{
var _d2,_d3;
if(window.innerHeight&&window.scrollMaxY){
_d2=document.body.scrollWidth;
_d3=window.innerHeight+window.scrollMaxY;
}else{
if(document.body.scrollHeight>document.body.offsetHeight){
_d2=document.body.scrollWidth;
_d3=document.body.scrollHeight;
}else{
_d2=document.body.offsetWidth;
_d3=document.body.offsetHeight;
}
}
if(self.innerHeight){
_ce=self.innerWidth;
_cf=self.innerHeight;
}else{
if(document.documentElement&&document.documentElement.clientHeight){
_ce=document.documentElement.clientWidth;
_cf=document.documentElement.clientHeight;
}else{
if(document.body){
_ce=document.body.clientWidth;
_cf=document.body.clientHeight;
}
}
}
if(_d3<_cf){
_d0=_cf;
}else{
_d0=_d3;
}
if(_d2<_ce){
_d1=_ce;
}else{
_d1=_d2;
}
}
return {pageWidth:_d1,pageHeight:_d0,windowWidth:_ce,windowHeight:_cf};
},disableScreen:function(_d4,_d5,_d6,_d7,_d8){
WindowUtilities.initLightbox(_d5,_d4,function(){
this._disableScreen(_d4,_d5,_d6,_d7);
}.bind(this),_d8||document.body);
},_disableScreen:function(_d9,_da,_db,_dc){
var _dd=$(_da);
var _de=WindowUtilities.getPageSize(_dd.parentNode);
if(_dc&&Prototype.Browser.IE){
WindowUtilities._hideSelect();
WindowUtilities._showSelect(_dc);
}
_dd.style.height=(_de.pageHeight+"px");
_dd.style.display="none";
if(_da=="overlay_modal"&&Window.hasEffectLib&&Windows.overlayShowEffectOptions){
_dd.overlayOpacity=_db;
new Effect.Appear(_dd,Object.extend({from:0,to:_db},Windows.overlayShowEffectOptions));
}else{
_dd.style.display="block";
}
},enableScreen:function(id){
id=id||"overlay_modal";
var _e0=$(id);
if(_e0){
if(id=="overlay_modal"&&Window.hasEffectLib&&Windows.overlayHideEffectOptions){
new Effect.Fade(_e0,Object.extend({from:_e0.overlayOpacity,to:0},Windows.overlayHideEffectOptions));
}else{
_e0.style.display="none";
_e0.parentNode.removeChild(_e0);
}
if(id!="__invisible__"){
WindowUtilities._showSelect();
}
}
},_hideSelect:function(id){
if(Prototype.Browser.IE){
id=id==null?"":"#"+id+" ";
$$(id+"select").each(function(_e2){
if(!WindowUtilities.isDefined(_e2.oldVisibility)){
_e2.oldVisibility=_e2.style.visibility?_e2.style.visibility:"visible";
_e2.style.visibility="hidden";
}
});
}
},_showSelect:function(id){
if(Prototype.Browser.IE){
id=id==null?"":"#"+id+" ";
$$(id+"select").each(function(_e4){
if(WindowUtilities.isDefined(_e4.oldVisibility)){
try{
_e4.style.visibility=_e4.oldVisibility;
}
catch(e){
_e4.style.visibility="visible";
}
_e4.oldVisibility=null;
}else{
if(_e4.style.visibility){
_e4.style.visibility="visible";
}
}
});
}
},isDefined:function(_e5){
return typeof (_e5)!="undefined"&&_e5!=null;
},initLightbox:function(id,_e7,_e8,_e9){
if($(id)){
Element.setStyle(id,{zIndex:Windows.maxZIndex+1});
Windows.maxZIndex++;
_e8();
}else{
var _ea=document.createElement("div");
_ea.setAttribute("id",id);
_ea.className="overlay_"+_e7;
_ea.style.display="none";
_ea.style.position="absolute";
_ea.style.top="0";
_ea.style.left="0";
_ea.style.zIndex=Windows.maxZIndex+1;
Windows.maxZIndex++;
_ea.style.width="100%";
_e9.insertBefore(_ea,_e9.firstChild);
if(Prototype.Browser.WebKit&&id=="overlay_modal"){
setTimeout(function(){
_e8();
},10);
}else{
_e8();
}
}
},setCookie:function(_eb,_ec){
document.cookie=_ec[0]+"="+escape(_eb)+((_ec[1])?"; expires="+_ec[1].toGMTString():"")+((_ec[2])?"; path="+_ec[2]:"")+((_ec[3])?"; domain="+_ec[3]:"")+((_ec[4])?"; secure":"");
},getCookie:function(_ed){
var dc=document.cookie;
var _ef=_ed+"=";
var _f0=dc.indexOf("; "+_ef);
if(_f0==-1){
_f0=dc.indexOf(_ef);
if(_f0!=0){
return null;
}
}else{
_f0+=2;
}
var end=document.cookie.indexOf(";",_f0);
if(end==-1){
end=dc.length;
}
return unescape(dc.substring(_f0+_ef.length,end));
},_computeSize:function(_f2,id,_f4,_f5,_f6,_f7){
var _f8=document.body;
var _f9=document.createElement("div");
_f9.setAttribute("id",id);
_f9.className=_f7+"_content";
if(_f5){
_f9.style.height=_f5+"px";
}else{
_f9.style.width=_f4+"px";
}
_f9.style.position="absolute";
_f9.style.top="0";
_f9.style.left="0";
_f9.style.display="none";
_f9.innerHTML=_f2;
_f8.insertBefore(_f9,_f8.firstChild);
var _fa;
if(_f5){
_fa=$(_f9).getDimensions().width+_f6;
}else{
_fa=$(_f9).getDimensions().height+_f6;
}
_f8.removeChild(_f9);
return _fa;
}};

