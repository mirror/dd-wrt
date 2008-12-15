Effect.ResizeWindow=Class.create();
Object.extend(Object.extend(Effect.ResizeWindow.prototype,Effect.Base.prototype),{initialize:function(_1,_2,_3,_4,_5){
this.window=_1;
this.window.resizing=true;
var _6=_1.getSize();
this.initWidth=parseFloat(_6.width);
this.initHeight=parseFloat(_6.height);
var _7=_1.getLocation();
this.initTop=parseFloat(_7.top);
this.initLeft=parseFloat(_7.left);
this.width=_4!=null?parseFloat(_4):this.initWidth;
this.height=_5!=null?parseFloat(_5):this.initHeight;
this.top=_2!=null?parseFloat(_2):this.initTop;
this.left=_3!=null?parseFloat(_3):this.initLeft;
this.dx=this.left-this.initLeft;
this.dy=this.top-this.initTop;
this.dw=this.width-this.initWidth;
this.dh=this.height-this.initHeight;
this.r2=$(this.window.getId()+"_row2");
this.content=$(this.window.getId()+"_content");
this.contentOverflow=this.content.getStyle("overflow")||"auto";
this.content.setStyle({overflow:"hidden"});
if(this.window.options.wiredDrag){
this.window.currentDrag=_1._createWiredElement();
this.window.currentDrag.show();
this.window.element.hide();
}
this.start(arguments[5]);
},update:function(_8){
var _9=Math.floor(this.initWidth+this.dw*_8);
var _a=Math.floor(this.initHeight+this.dh*_8);
var _b=Math.floor(this.initTop+this.dy*_8);
var _c=Math.floor(this.initLeft+this.dx*_8);
if(window.ie){
if(Math.floor(_a)==0){
this.r2.hide();
}else{
if(Math.floor(_a)>1){
this.r2.show();
}
}
}
this.r2.setStyle({height:_a});
this.window.setSize(_9,_a);
this.window.setLocation(_b,_c);
},finish:function(_d){
if(this.window.options.wiredDrag){
this.window._hideWiredElement();
this.window.element.show();
}
this.window.setSize(this.width,this.height);
this.window.setLocation(this.top,this.left);
this.r2.setStyle({height:null});
this.content.setStyle({overflow:this.contentOverflow});
this.window.resizing=false;
}});
Effect.ModalSlideDown=function(_e){
var _f=WindowUtilities.getWindowScroll();
var _10=_e.getStyle("height");
_e.setStyle({top:-(parseFloat(_10)-_f.top)+"px"});
_e.show();
return new Effect.Move(_e,Object.extend({x:0,y:parseFloat(_10)},arguments[1]||{}));
};
Effect.ModalSlideUp=function(_11){
var _12=_11.getStyle("height");
return new Effect.Move(_11,Object.extend({x:0,y:-parseFloat(_12)},arguments[1]||{}));
};
PopupEffect=Class.create();
PopupEffect.prototype={initialize:function(_13){
this.html=$(_13);
this.options=Object.extend({className:"popup_effect",duration:0.4},arguments[1]||{});
},show:function(_14,_15){
var _16=Position.cumulativeOffset(this.html);
var _17=this.html.getDimensions();
var _18=_14.win.getBounds();
this.window=_14.win;
if(!this.div){
this.div=document.createElement("div");
this.div.className=this.options.className;
this.div.style.height=_17.height+"px";
this.div.style.width=_17.width+"px";
this.div.style.top=_16[1]+"px";
this.div.style.left=_16[0]+"px";
this.div.style.position="absolute";
document.body.appendChild(this.div);
}
if(this.options.fromOpacity){
this.div.setStyle({opacity:this.options.fromOpacity});
}
this.div.show();
var _19="top:"+_18.top+";left:"+_18.left+";width:"+_18.width+";height:"+_18.height;
if(this.options.toOpacity){
_19+=";opacity:"+this.options.toOpacity;
}
new Effect.Morph(this.div,{style:_19,duration:this.options.duration,afterFinish:this._showWindow.bind(this)});
},hide:function(_1a,_1b){
var _1c=Position.cumulativeOffset(this.html);
var _1d=this.html.getDimensions();
this.window.visible=true;
var _1e=this.window.getBounds();
this.window.visible=false;
this.window.element.hide();
this.div.style.height=_1e.height;
this.div.style.width=_1e.width;
this.div.style.top=_1e.top;
this.div.style.left=_1e.left;
if(this.options.toOpacity){
this.div.setStyle({opacity:this.options.toOpacity});
}
this.div.show();
var _1f="top:"+_1c[1]+"px;left:"+_1c[0]+"px;width:"+_1d.width+"px;height:"+_1d.height+"px";
if(this.options.fromOpacity){
_1f+=";opacity:"+this.options.fromOpacity;
}
new Effect.Morph(this.div,{style:_1f,duration:this.options.duration,afterFinish:this._hideDiv.bind(this)});
},_showWindow:function(){
this.div.hide();
this.window.element.show();
},_hideDiv:function(){
this.div.hide();
}};

