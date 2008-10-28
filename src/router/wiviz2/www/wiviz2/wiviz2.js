var demomode = false;
var demourl = "capture/f_capture_";
var demomax = 38;
var demoindex = 1;

var IEfix = window.ActiveXObject ? true : false;

var prefs = new Object();
prefs.rotate = true;
prefs.grid = true;
prefs.animation = true;
prefs.flyin = true;
prefs.activetab = 'scanoptions';
prefs.blend = true;
prefs.scale = true;
prefs.clientass = true;
prefs.clientdiss = true;
prefs.apenc = true;
prefs.apunenc = true;

var hexX = new Array(
	0, 2, -1, -1, 1, -2, 1
	);
var hexY = new Array(
	0, 0, 1, -1, 1, 0, -1
	);

//window.onerror = function() { return true; }

var now = (new Date()).getTime();
var max_age = 30;
var camera = new Object();
var tickMoveStep = false;
var view_mtx, model_mtx, mtx = new Array(16);
var items = new Array();
var screenH, screenW, mainarea;
var logodiv;
var lastWvzUpdate = now - 4500;
var lastInfoUpdate = now;
var wvzFrame;
var wvzLoc = "Wiviz.live.asp";
var hosts;
var root;
var globalRedraw = false;
var centeredObj;
var colors = new Array("red", "orange", "yellow", "green", "blue");
var numcolors = colors.length;
var globalrefs = new Object();
var centerLocked = false;
var selfrouterdiv;

function resetCamera() {
    camera.azimuth = 0;
	camera.elevation = -0.4;
	camera.zoom = 1;
	camera.panX = 0;
	camera.panY = 0;
	camera.panZ = 0;
}
function resetCameraWithFlair() {
	movePropertyTo("camera.panX", 0, "",
		 0.6, 0.01, true, false);
	movePropertyTo("camera.panY", 0, "",
		 0.6, 0.01, true, false);
	movePropertyTo("camera.panZ", 0, "",
		 0.6, 0.01, true, false);
	movePropertyTo("camera.elevation", -0.4, "",
		 0.6, 0.01, true, false);
	movePropertyTo("camera.zoom", 1, "",
		 0.6, 0.01, true, false);
	closeExpando(document.getElementById('details'));
	centerLocked = false;
}
function cameraPanX(amt) {
	movePropertyTo("camera.panX", parseFloat(camera.panX) + amt, "",
		 0.6, 0.01, true, false);
}
function cameraPanY(amt) {
	movePropertyTo("camera.panY", parseFloat(camera.panY) + amt, "",
		 0.6, 0.01, true, false);
}
//movePropertyTo(property, target, post, lambda, delta, needsRedraw) {
function cameraElevation(amt) {
	movePropertyTo("camera.elevation", parseFloat(camera.elevation) + amt, "",
		 0.6, 0.01, true, false);
}
function cameraZoom(factor) {
	movePropertyTo("camera.zoom", parseFloat(camera.zoom) * factor, "",
		 0.6, 0.01, true, false);
}
function centerObj(what) {
	if (prefs.flyin) {
		movePropertyTo("camera.zoom", 3, "", 0.6, 0.01, true, false);
		while (camera.azimuth > 2 * Math.PI) camera.azimuth -= 2 * Math.PI;
		while (camera.azimuth < 0) camera.azimuth += 2 * Math.PI;
		var targazi = Math.atan2(what.backref.x, what.backref.y);
		while (targazi < camera.azimuth) targazi += 2 * Math.PI;
		movePropertyTo("camera.azimuth", targazi,
			"", 0.6, 0.01, true, false);
	}
	centeredObj = what;
	centerLocked = true;
	updateCenteredInfo();
	closeExpando(document.getElementById('details'));
	openExpando(document.getElementById('details'));
}

function updatePrefs(what) {
	var cke = "";
	if (what) {
	    if (what.tagName == 'INPUT' && what.getAttribute("type") == 'checkbox') {
	        eval("prefs." + what.id + " = " + what.checked);
	    }
	    if (what.tagName == 'DIV' && what.className == 'slidingheader') {
			prefs.activetab = what.id;
		}
	}
	else {
	    //Only happens on startup, read cookies!
	    var matches = document.cookie.match(/[a-z]+=[a-z]+/gi);
	    for (var i = 0; matches && i < matches.length; i++) {
	        var keyma = matches[i].split("=");
            if (keyma[1] == 'true' || keyma[1] == 'false') {
        		prefs[keyma[0]] = (keyma[1] == 'true') ? true : false;
            }
            else {
        		prefs[keyma[0]] = keyma[1];
            }
	    }
	    var el = document.getElementById(prefs.activetab);
	    if (el) {
	        openExpando(el);
	    }
	}

	for (var i in prefs) {
	    var el = document.getElementById(i);
	    if (el) {
	    	el.checked = eval("prefs." + i);
	    }
		document.cookie = i + "=" + eval("prefs." + i) + "; expires=01-Jan-9999 00:00:00 GMT";
	}
	globalRedraw = true;
}

function findParentOfClass(el, parentClass) {
	var p = el;
	while (p.parentNode && p.className != parentClass) {
	    p = p.parentNode;
	}
	if (p && p.className == parentClass) return p;
	return null;
}
function findChildOfClass(el, childClass) {
	var a = el.childNodes;
	var i, p;
	for (i = 0; i < a.length; i++) {
		p = a[i];
		if (p.className == childClass) return p;
		p = findChildOfClass(p, childClass);
		if (p) return p;
	}
	return null;
}
var shrinkingExpando, growingExpando;
function toggleExpando(e) {
	Expando(e, false, false);
	}
function openExpando(e) {
	Expando(e, true, false);
}
function closeExpando(e) {
	Expando(e, false, true);
}
function Expando(e, onlyOpen, onlyClose) {
	updatePrefs(e);
	mn = findParentOfClass(e, "rightmenu");
	ex = findParentOfClass(e, "expando");
	bds = findChildOfClass(mn, "slidingbodyshow");
	bdh = findChildOfClass(ex, "slidingbody");
	var newsx;
	if (onlyOpen && !bdh) return;
	if (growingExpando || bds) {
	    if (shrinkingExpando != growingExpando)
			newsx = growingExpando;
		else
		    newsx = null;
		if (bds) newsx = bds;
		if (newsx) {
			shrinkingExpando = newsx;
		    shrinkingExpando.className = 'slidingbody';
		  shrinkingExpando.style.height = shrinkingExpando.scrollHeight + "px";
			movePropertyTo("shrinkingExpando.style.height",
			        1, "px", 0.5, 1, false, true);
		}
	}
	if (bdh && !onlyClose) {
	    growingExpando = bdh;
	    bdh.className = 'slidingbodyshow';
	   bdh.style.height = "0px";
	    movePropertyTo("growingExpando.style.height",
	        bdh.scrollHeight, "px", 0.5, 1, false, true);
	}
}
function get_coords(d) {
  var x = 0;
  var y = 0;
  var p = d;
  while (p.offsetParent) {
      x += p.offsetLeft;
      y += p.offsetTop;
    p = p.offsetParent;
  }
  var n = new Object();
  n.x = x;
  n.y = y;
  return n;
}
var movings = new Array();
function movePropertyTo(property, target, post, lambda, delta, needsRedraw, integral) {
  if (!prefs.animation || Math.abs(parseFloat(eval(property)) - target) < delta) {
    eval(property + " = " + target);
    globalRedraw = globalRedraw || needsRedraw;
    return;
  }
  var m = new Object();
  var mnew = true;
  for (var i = 0; i < movings.length; i++) {
    if (movings[i].property == property) {
        if (movings[i].on) {
	        if (movings[i].lambda <= lambda) {
			  m = movings[i];
			  mnew = false;
	        }
	        else return;
        }
        else {
            m = movings[i];
            mnew = false;
        }
    }
  }
  m.property = property;
  m.lambda = lambda;
  m.delta = delta;
  m.target = target;
  m.orig = parseFloat(eval(property));
  m.dist = Math.abs(m.orig - m.target);
  m.post = post;
  m.needsRedraw = needsRedraw;
  m.integral = integral;
  m.on = true;
  if (mnew) movings[movings.length] = m;
  tickMoveStep = true;
}
function moveStep() {
  var i, m, o, c = 0, t;
  for (i = 0; i < movings.length; i++) {
    m = movings[i];
    if (m.on == false) continue;
    c++;
    o = parseFloat(eval(m.property));
    if (!o) o = 0;
    t = (o - m.orig) / (m.target - m.orig);
    if (t < 0.1) m.lambda = 0.9;
    else if (t > 0.6) m.lambda = 0.7;
    else {
      if (t > 0.5) t = 0.5 - (t - 0.5);
      m.lambda = 0.9 - (0.3 * t * 2);
      }
	outVal = o * m.lambda + m.target * (1 - m.lambda)
	if (m.integral) outVal = Math.round(outVal);
	//document.getElementById('debugger').innerHTML = m.property + " = " + outVal;
	if (isNaN(outVal)) {
	    movings[i].on = false;
	    eval(m.property + " = " + m.target + (m.post ? (" + '" + m.post + "'") : "") + ";");
	    continue;
	}
    try {
  	  eval(m.property + " = '" + outVal + "';");
    }
	catch (e) {}
    if (Math.abs(o - parseFloat(eval(m.property))) < m.delta) {
      	eval(m.property + " = " + m.target + (m.post ? (" + '" + m.post + "'") : "") + ";");
        m.on = false;
      }
	globalRedraw = globalRedraw || m.needsRedraw;
    }
  if (c) {
  	tickMoveStep = true;
  }
  else {
    tickMoveStep = false;
  	movings = new Array();
  }
  }
function redraw() {
	var item;
	var x, y, z, w, i;
	var num_invisible = 0;
	if (centerLocked && centeredObj) {
		movePropertyTo("camera.panX", -centeredObj.backref.x, "",
	    0.6, 0.01, true, false);
		movePropertyTo("camera.panY", -centeredObj.backref.y, "",
		    0.6, 0.01, true, false);
		movePropertyTo("camera.panZ", -centeredObj.backref.z, "",
		    0.6, 0.01, true, false);
	}
	calc_camera_matrix();
	globalRedraw = false;
	for (i = 0; i < items.length; i++) {
	    item = items[i];
	    if (!item.visible) {
	        num_invisible++;
	        if (item.div) item.div.style.display = 'none';
			continue;
	    }
	    if (!item.div) {
			item.div = document.createElement('div');
			item.div.className = 'div3d';
			item.div.innerHTML = "<img class='sizeimage' src='images/wiviz/ssid.png'>";
			mainarea.appendChild(item.div);
			item.div.backref = item;
	    }
	    if (item.div.className == 'gridpoint' && !prefs.grid) {
	        item.div.style.display = 'none';
	        continue;
	    }
	    x = item.x * mtx[0] + item.y * mtx[1] + item.z * mtx[2] + mtx[3];
	    y = item.x * mtx[4] + item.y * mtx[5] + item.z * mtx[6] + mtx[7];
	    z = item.x * mtx[8] + item.y * mtx[9] + item.z * mtx[10] + mtx[11];
		w = item.x * mtx[12] + item.y * mtx[13] + item.z * mtx[14] + mtx[15];
		x /= w;
		y /= w;
//		z /= w;
		var mg = findChildOfClass(item.div, "sizeimage");
		var gl = findChildOfClass(item.div, "glow");
		if (mg) {
			if (prefs.scale) {
			    w = Math.round(z * camera.zoom + 15);
			    if (!isNaN(w) && w > 0) {
			    	mg.style.height = w * (item.scale ? item.scale : 1) + "px";
			    	mg.style.width = w * (item.scale ? item.scale : 1) + "px";
				//	document.getElementById('debugger').innerHTML = w;
			    }
			}
			else {
			mg.style.height = "50px";
			mg.style.width = "50px";
			}
		}
		if (gl && mg) {
		    if (prefs.blend) {
		    	gl.style.height = mg.style.height;
		    	gl.style.width = mg.style.width;
		    	gl.style.display = 'inline';
		    }
		    else {
		        gl.style.display = 'none';
		    }
		}
		if (!isNaN(x)) item.div.style.left = x - (item.div.clientWidth / 2) + "px";
		if (!isNaN(y)) item.div.style.top = y - item.div.clientHeight / 2 + "px";
		item.div.style.display = 'block';
		if (z < 1) z = 1;
		if (z > 100) z = 100;
		if (prefs.blend) {
		    var o = 1;
		    if (item.fadein > 0) {
				item.fadein -= 0.05;
				o = 1 - item.fadein;
				globalRedraw = true;
		    }
			o -= z / 100.0 / 2;
		    if (z > 40 && z <= 50) {
		        o -= (z - 40) / 10.0;
		    }
		    var age = 0;
			if (item.hostinfo) {
			    age = item.hostinfo.age + (now - item.hostinfo.agestart) / 1000;
			}
			if (age > max_age - 3) {
			    o -= (age - max_age + 3) / 3.0;
			}
			if (age >= max_age) item.visible = false;
			if (item.visible == true && age > max_age - 3) globalRedraw = true;
		    if (o <= 0) o = 0;
		    if (o > 1) o = 1;
		    if (IEfix)
		        item.div.style.filter = "progid:DXImageTransform.Microsoft.Alpha(opacity="
		            + Math.round(o * 100) + ")";
			else item.div.style.MozOpacity = o;
		}
		else {
		    if (IEfix)
		        item.div.style.filter = "progid:DXImageTransform.Microsoft.Alpha(opacity="
		            + "100)";
			else item.div.style.MozOpacity = "1.0";
		}
		if (z > 50) {
	        item.div.style.display = 'none';
		}
		if (!isNaN(z)) item.div.style.zIndex = Math.round(z) + 10;
//		mg = findChildOfClass(item.div, "title3d");
//		if (mg) mg.innerHTML = Math.round(z);
	}
	if (num_invisible > 5) {
		var nitems = new Array();
	    for (i = 0; i < items.length; i++) {
	        item = items[i];
	        if (item.visible) {
	            nitems[nitems.length] = item;
	        }
	        else {
				if (item.div && item.div.parentNode) {
				    item.div.parentNode.removeChild(item.div);
				}
	        }
	    }
	    items = nitems;
	}
}
function loadBasicMatrix() {
	model_mtx = new Array(1, 0, 0, 0,
	                      0, 1, 0, 0,
	                      0, 0, 1, 0,
					      0, 0, 0, 1);
	resetCamera();
	calc_camera_matrix();
}
function calc_camera_matrix() {
	//Basic 2d Ortho matrix
	view_mtx = new Array(screenH / 2, 0, 0, screenW / 2,
	               		 0, screenH / 2, 0, screenH / 2,
	                   	 0, 0, -30, 30,
					     0, 0, 0, 1);
	//Perspective matrix
	view_mtx = mult_mtx(view_mtx, new Array(
						  1, 0, 0, 0,
	                      0, 1, 0, 0,
	                      0, 0, 1, 0,
					      0, 0, 0.6, 1 ));
	//Rotation matrices
	//Elevation
	view_mtx = mult_mtx(view_mtx, rotate_mtx(-1,0,0,camera.elevation));
	//Azimuth
	view_mtx = mult_mtx(view_mtx, rotate_mtx(0,1,0,camera.azimuth));
	//Zoom
	view_mtx = mult_mtx(view_mtx, scale_mtx(camera.zoom));
	//Pan X/Y/Z
	view_mtx = mult_mtx(view_mtx, new Array(
	            1, 0, 0, camera.panX,
				0, 1, 0, camera.panY,
				0, 0, 1, camera.panZ,
				0, 0, 0, 1));
	mtx = mult_mtx(view_mtx, model_mtx);
}
function rotate_mtx(x, y, z, amt) {
	var s = Math.sin(amt);
	var c = Math.cos(amt);
	var omc = 1 - c;
	return new Array(
	    x*x*omc+c, x*y*omc-z*s,x*z*omc+y*s, 0,
	    y*x*omc+z*s, y*y*omc+c, y*z*omc-x*s, 0,
	    x*z*omc-y*s, y*z*omc+x*s,z*z*omc+c, 0,
	    0, 0, 0, 1);
}
function postrotate(x, y, z, amt) {
	model_mtx = mult_mtx(rotate_mtx(x, y, z, amt), model_mtx);
	mtx = mult_mtx(view_mtx, model_mtx);
}
function rotate(x, y, z, amt) {
	model_mtx = mult_mtx(model_mtx, rotate_mtx(x, y, z, amt));
	mtx = mult_mtx(view_mtx, model_mtx);
}
function scale_mtx(factor) {
	return new Array(factor, 0, 0, 0,
	                      0, factor, 0, 0,
	                      0, 0, factor, 0,
					      0, 0, 0, 1);
}
function scale(factor) {
	model_mtx = mult_mtx(scale_mtx(factor), model_mtx);
	mtx = mult_mtx(view_mtx, model_mtx);
}
function translate (x, y, z) {
	model_mtx[3] += x;
	model_mtx[7] += y;
	model_mtx[11] += z;
}
function init_3d() {
	items = new Array();
	var item;
	var x, y, z;
	var grid_range = 0.75;
	var grid_step = 0.75;
	mainarea = document.getElementById('mainarea');
	mainarea.innerHTML = "";
	for (x = -grid_range; x <= grid_range; x += grid_step) {
		for (y = -grid_range; y <= grid_range; y += grid_step) {
			for (z = -grid_range; z <= grid_range; z += grid_step) {
			    item = new Object();
			    item.x = x;
			    item.y = y;
			    item.z = z;
			    item.visible = true;
				item.div = document.createElement('div');
				item.div.className = 'gridpoint';
				mainarea.appendChild(item.div);
				items[items.length] = item;
			}
		}
	}
	screenH = mainarea.clientHeight;
	screenW = mainarea.clientWidth;
	loadBasicMatrix();
	globalRedraw = true;
	setTimeout('tick()', 10);
	updatePrefs();
	logodiv = document.getElementById('logo');
	logodiv.style.left = "30%";
	logodiv.style.top = "30%";
	movePropertyTo("logodiv.style.left", 0, "%",
		 0.6, 0.01, true, true);
	movePropertyTo("logodiv.style.top", 0, "%",
		 0.6, 0.01, true, true);
}
function mult_mtx(mtx1, mtx2) {
	mtxo = new Array(16);
	var r, c;
	for (r = 0; r < 4; r++) {
	    for (c = 0; c < 4; c++) {
	        mtxo[r*4 + c] =
	            mtx1[r*4] * mtx2[c]
			  + mtx1[r*4+1] * mtx2[c + 4]
			  + mtx1[r*4+2] * mtx2[c + 8]
			  + mtx1[r*4+3] * mtx2[c + 12];
	    }
	}
	return mtxo;
}
function tick() {
	now = (new Date()).getTime();
//	document.getElementById('debugger').innerHTML = "now = " + now;
	if (prefs.rotate) {
		movePropertyTo("camera.azimuth", parseFloat(camera.azimuth) + 0.02, "",
			 0.1, 0.02, true, false);
	}
	if (tickMoveStep) moveStep();
	if (!wvzFrame) wvzFrame = document.getElementById('wivizGetFrame');
	if (wvzFrame && now - lastWvzUpdate > 5000) {
		var loc = wvzFrame.contentWindow.location;
		if (demomode) {
		    wvzLoc = demourl + demoindex + ".html";
		    demoindex++;
		    if (demoindex > demomax) demoindex = 1;
		}
		if (loc.href == wvzLoc) 
		    loc.reload(true);
		else
		    loc.replace(wvzLoc);
		lastWvzUpdate = (new Date()).getTime();
	}
	if (now - lastInfoUpdate > 1000) {
		updateCenteredInfo();
		lastInfoUpdate = now;
	}
	if (globalRedraw) redraw();
	setTimeout('tick()', 30);
}
function wiviz_callback(hosts, cfgobj) {
	var i, h, d, e, el, a;
	var root = new Object();
	var elem, path;
	//For each node
	for (i = 0; i < hosts.length; i++) {
	    h = hosts[i];
	    if (!h ||!h.mac) continue;
	    h.agestart = now;
	    d = document.getElementById(h.mac);
		if (d && d.backref){
		    e = d.backref;
		    e.hostinfo = h;
		    if (h.age > max_age) {
		        e.div.parentNode.removeChild(e.div);
		    }
		}
		else {
		    if (h.age > max_age) continue;
		    e = new Object();
		    e.hostinfo = h;
		    e.x = 0;//Math.random() - 0.5;
		    e.y = 0;//Math.random() - 0.5;
		    e.z = 0;//Math.random() - 0.5;
		    e.visible = false;
		    items[items.length] = e;
		    e.div = document.createElement('div');
		    e.div.className = 'div3d';
		    e.div.style.display = 'none';
		    e.div.id = h.mac;
		    e.div.backref = e;
			e.div.setAttribute("onClick", "centerObj(this)");
			mainarea.appendChild(e.div);
			el = document.createElement('img');
			el.className = 'glow';
			el.src = "images/wiviz/" + colors[Math.floor(Math.random() * numcolors)] + 'glow.png';
			if (IEfix) {
				var ne = document.createElement('a');
				ne.className = 'glow';
				ne.style.width = "50px";
				ne.style.height = "50px";
			    ne.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader"
				 + "(src=\'" + el.src + "\', sizingMethod='scale')";
				ne.setAttribute("href", "javascript:centerObj(document.getElementById('"
					+ e.hostinfo.mac + "'))");
				e.div.appendChild(ne);
			}
			else {
				e.div.appendChild(el);
			}
			el = document.createElement('img');
			el.className = 'sizeimage';
			el.src = "images/wiviz/" + h.type + (h.encrypted == 'yes' ? "-enc" : "") + ".png";
			if (IEfix) {
				var ne = document.createElement('a');
				ne.style.width = "50px";
				ne.style.height = "50px";
				ne.className = 'sizeimage';
				ne.setAttribute("href", "javascript:centerObj(document.getElementById('"
					+ e.hostinfo.mac + "'))");
			    ne.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader"
				 + "(src=\'" + el.src + "\', sizingMethod='scale')";
				e.div.appendChild(ne);
			}
			else {
				e.div.appendChild(el);
			}
			e.div.appendChild(document.createElement('br'));
			el = document.createElement('span');
			el.innerHTML = h.title;
			el.className = 'title3d';
			e.div.appendChild(el);
		}
		if (e.visible == false && prefs.blend) {
		    e.fadein = 1;
		}
		e.visible = true;
		h = e.hostinfo;
		h.title = false;
	    if (!h.title) h.title = h.ssid;
	    if (!h.title) h.title = h.hostname;
	    if (!h.title) h.title = h.mac;
		el = findChildOfClass(e.div, 'title3d');
		if (el) {
		    el.innerHTML = h.title;
		}
		elem = false;
		if (h.self) selfrouterdiv = e.div;
		if (h.type == 'sta') {
		    if (h.sta_state == 'assoc') {
				a = document.getElementById(h.sta_bssid);
				if (a) a = a.backref;
				if (a) a = a.hostinfo;
				if (a) a = a.ssid;
				elem = root;
				if (a) {
					elem = walk(elem, a);
					elem.place = true;
				}
				elem = walk(elem, h.sta_bssid);
				elem.place = true;
				elem = walk(elem, h.mac);
				elem.place = true;
		    }
		    else {
		        elem = walk(root, "Unassociated Clients");
		        elem.yOffset = 0.5;
		        elem = walk(elem, h.mac);
		        elem.place = true;
		    }
		}
		if (h.type == 'ap' || h.type == 'adhoc') {
		    elem = walk(root, h.ssid);
		    elem.place = true;
		    elem = walk(elem, h.mac);
		}
		if (elem) {
		    elem.backref = e;
		    e.elemref = elem;
		}
	}
	root.x = root.z = 0;
	root.y = -0.5;
	placeItems(root);
	lockCoords(root);
	updateCenteredInfo();
	globalRedraw = true;
}
function walk (elem, target) {
	if (!elem.children) elem.children = new Array();
	if (!elem.children[target]) elem.children[target] = new Object();
	elem.children[target].parent = elem;
	return elem.children[target];
}
function placeItems(root) {
	var i, child, n, spacing = 0.05, rootY, childct = 0;
	if (root.children) {
	    for (i in root.children) {
	        child = root.children[i];
	        placeItems(child);
	        if (child.bound > spacing)
	            spacing = child.bound;
			childct++;
	    }
	    if (childct > 1) root.bound = spacing * 2.5;
		else root.bound = spacing;
	    n = 0;
	    if ((!root.place && childct <= 1) || !root.parent) rootY = 0;
		else rootY = 0.3;
		if (root.yOffset) rootY += root.yOffset;
		var usurped;
	    for (i in root.children) {
	        child = root.children[i];
	        if (n == 0 && root.children['Unassociated Clients']) {
	            usurped = child;
	            child = root.children['Unassociated Clients'];
	        }
	        if (i == 'Unassociated Clients') {
	            child = usurped;
	        }
	        /*child.relX = (Math.random() - 0.5) * root.bound;
	        child.relY = 0.5 + (Math.random() - 0.5) * root.bound;
	        child.relZ = (Math.random() - 0.5) * root.bound;*/
			child.relX = hexX[n] * root.bound;
			child.relZ = hexY[n] * root.bound * 1.732;
			child.relY = rootY;
			n++;
			if (n > 6) {
				n = 0;
				rootY += root.bound;
			}
	    }
	}
	else {
	    root.bound = spacing;
	}
}
function lockCoords(root) {
	if (root.children) {
	    for (i in root.children) {
	        child = root.children[i];
	        child.x = root.x + child.relX;
	        child.y = root.y + child.relY;
	        child.z = root.z + child.relZ;
			if (child.backref) {
			    globalrefs[child.backref.div.id] = child.backref;
				movePropertyTo("globalrefs['" + child.backref.div.id + "'].x",
					 child.x, "",
					 0.6, 0.01, true, false);
				movePropertyTo("globalrefs['" + child.backref.div.id + "'].y",
					 child.y, "",
					 0.6, 0.01, true, false);
				movePropertyTo("globalrefs['" + child.backref.div.id + "'].z",
					 child.z, "",
					 0.6, 0.01, true, false);
			}
			else {
			}
			lockCoords(child);
	    }
	}
}
function updateCenteredInfo() {
	if (!centeredObj) return;
	var e = centeredObj.backref;
	if (!e) return;
	var h = e.hostinfo;
	if (!h) return;
	var sp = document.getElementById('detail_type');
	if (sp) {
	    sp.innerHTML = "Type Unknown";
	    if (h.type == 'ap') sp.innerHTML = "Access Point";
	    if (h.type == 'ssid') sp.innerHTML = "SSID Collection";
	    if (h.type == 'adhoc') sp.innerHTML = "Ad-hoc entity";
	    if (h.type == 'sta') sp.innerHTML = "Client";
	}
	sp = document.getElementById('detail_info');
	if (sp) {
	    var s = "";
	    s += "MAC: " + h.mac + "<br>";
	    h.realage = Math.floor((h.age + (now - h.agestart) / 1000) );
	    s += "Signal: " + h.rssi + " dBm<br>";
	    s += "Seen " + h.realage + " seconds ago<br>";
	    if (h.type == 'sta') {
		    s += "State: ";
		    if (h.sta_state == 'assoc') {
				s += "Associated<br>Network: ";
				var a = document.getElementById(h.sta_bssid);
				if (a) a = a.backref;
				if (a) a = a.hostinfo;
				if (a) a = a.ssid;
				if (a) {
				    //a = "<a class='objlink' href='javascript:centerObj(null)'>" + a + "</a>";
				}
				if (!a) a = "<i>Unknown</i>";
				s += a + "<br>AP: ";
				a = h.sta_bssid;
				if (a) {
				    a = "<a class='objlink' href='javascript:centerObj(document.getElementById(\""
						+ a + "\"));'>" + a + "</a>";
				}
				else {
				    a = "<i>Unknown</i>";
				}
				s += a + "<br>";
		    }
		    if (h.sta_state == 'unassoc') {
				s += "Scanning<br>Last asked for: " + h.sta_lastssid + "<br>";
		    }
	    }
	    if (h.type == 'ap' || h.type == 'adhoc') {
	        s += "SSID: " + h.ssid + "<br>";
	        s += "Channel: " + h.channel + "<br>";
	        s += "Encryption: ";
			if (h.encrypted == 'yes') {
				if (h.enctype == 'wpa') s += "WPA";
				if (h.enctype == 'wpa2') s += "WPA2";
				if (h.enctype == 'wpa wpa2') s += "WPA/WPA2";
				if (h.enctype == 'wep') s += "WEP";
				if (h.enctype == 'unknown') s += "Unknown type";
			}
			else if (h.encrypted == 'no') {
				s += "None";
			}
			else {
				s += "Unknown";
			}
	        s += "<center>Clients</center>";
	        var clientct = 0;
			a = e.elemref;
			if (a) a = a.children;
			if (a) {
			    for (var u in a) {
					if (!u) continue;
			        s+= "<a href='javascript:centerObj(document.getElementById(\""
						+ u + "\"))'>" + u + "</a><br>";
					clientct++;
			    }
			}
	        if (clientct == 0) s += "<i>No clients</i>";
	    }
	    sp.innerHTML = s;
	    document.getElementById('detail_network_actions').style.display
	        = ((h.type == 'ap' || h.type == 'adhoc') && !h.self) ? "inline" : "none";
	    document.getElementById('detail_ap_actions').style.display
	        = (h.type == 'ap' && !h.self) ? "inline" : "none";
	    document.getElementById('detail_sta_actions').style.display
	        = (h.type == 'sta') ? "inline" : "none";
		a = e.elemref;
		if (a) a = a.parent;
		if (a) a = a.backref;
		if (a) a = a.hostinfo;
		if (a) a = a.self;
	    document.getElementById('detail_sta_assoc_actions').style.display
	        = (a) ? "inline" : "none";
	    document.getElementById('detail_local_actions').style.display
	        = (h.self) ? "inline" : "none";
	}
}
function ap_join() {
	if (!centeredObj) return;
	var e = centeredObj.backref;
	if (!e) return;
	var h = e.hostinfo;
	if (!h) return;
	var f = document.getElementById('radioform');
	if (!f) return;
	f.mode.value = 'sta';
	var sir = h.ssid.match(/[1-9][0-9]+/g);
	var ssid = "";
	for (var i = 0; i < sir.length; i++) {
	    ssid = ssid + String.fromCharCode(parseInt(sir[i]));
	}
	f.ssid.value = ssid;
	f.channel.value = h.channel;
	if (h.encrypted == 'no') f.encryption.value = 'none';
	else if (h.enctype == 'wep')
	    f.encryption.value = 'wep';
	else
		f.encryption.value = 'noch';
	ap_join_box();
}
function ap_join_box() {
	document.getElementById('radioform').mode.value = 'sta';
	ap_setup();
}
function ap_wds() {
	alert("Unimplemented");
}
function ap_copy() {
	ap_join();
	document.getElementById('radioform').mode.value = 'ap';
}
function sta_spy() {
	alert("Unimplemented");
}
function sta_unblock() {
	alert("Unimplemented");
}
function sta_static() {
	alert("Unimplemented");
}
function ap_setup() {
	openExpando(document.getElementById('configuration'));
}
function commit() {
	alert("Unimplemented");
}
setTimeout("init_3d()", 100);
window.onresize = init_3d;

