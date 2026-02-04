#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Displays your USB devices in reasonable form.
#
# Copyright (c) 2009 Kurt Garloff <garloff@suse.de>
# Copyright (c) 2013,2018 Kurt Garloff <kurt@garloff.de>
#
# Usage: See usage()

# Py2 compat
from __future__ import print_function
import getopt
import os
import re
import sys

HUB_ICLASS = 0x09

# Global options
showint = False
showhubint = False
noemptyhub = False
nohub = False
showeps = False
showwakeup = False

prefix = "/sys/bus/usb/devices/"
usbids = [
	"/usr/share/hwdata/usb.ids",
	"/usr/share/misc/usb.ids",
	"/usr/share/usb.ids",
]
cols = ("", "", "", "", "", "")

norm	= "\033[0;0m"
bold	= "\033[0;1m"
red	= "\033[0;31m"
green	= "\033[0;32m"
amber	= "\033[0;33m"
blue	= "\033[0;34m"

usbvendors = {}
usbproducts = {}
usbclasses = {}

def colorize(num, text):
	return cols[num] + str(text) + cols[0]

def ishexdigit(str):
	"return True if all digits are valid hex digits"
	for dg in str:
		if not dg.isdigit() and not dg in 'abcdef':
			return False
	return True

def open_read_ign(fn):
	try:
		return open(fn, 'r', errors='ignore')
	except:
		return open(fn, 'r')

def myenum(*args):
	enums = dict(zip(args, range(len(args))))
	return type('MyEnum', (), enums)

def parse_usb_ids():
	"Parse /usr/share/{hwdata/,misc/}usb.ids and fill usbvendors, usbproducts, usbclasses"
	vid = 0
	did = 0
	modes = myenum('Vendor', 'Class', 'Misc')
	mode = modes.Vendor
	strg = ""
	cstrg = ""
	for unm in usbids:
		if os.path.exists(unm):
			break
	for ln in open_read_ign(unm).readlines():
		if ln[0] == '#':
			continue
		ln = ln.rstrip('\n')
		if len(ln) == 0:
			continue
		if ishexdigit(ln[0:4]):
			mode = modes.Vendor
			vid = int(ln[:4], 16)
			usbvendors[vid] = ln[6:]
			continue
		if ln[0] == '\t' and ishexdigit(ln[1:3]):
			# usb.ids has a device id of 01xy, sigh
			if ln[3:5] == "xy":
				did = int(ln[1:3], 16)*256
			else:
				did = int(ln[1:5], 16)
			# USB devices
			if mode == modes.Vendor:
				usbproducts[vid, did] = ln[7:]
				continue
			elif mode == modes.Class:
				nm = ln[5:]
				if nm != "Unused":
					strg = cstrg + ":" + nm
				else:
					strg = cstrg + ":"
				usbclasses[vid, did, -1] = strg
				continue
		if ln[0] == 'C':
			mode = modes.Class
			cid = int(ln[2:4], 16)
			cstrg = ln[6:]
			usbclasses[cid, -1, -1] = cstrg
			continue
		if mode == modes.Class and ln[0] == '\t' and ln[1] == '\t' and ishexdigit(ln[2:4]):
			prid = int(ln[2:4], 16)
			usbclasses[cid, did, prid] = ln[6:]
			continue
		mode = modes.Misc
	usbclasses[0xFF, 0xFF, 0xFF] = "Vendor Specific"

def find_usb_prod(vid, pid):
	"Return device name from USB Vendor:Product list"
	strg = ""
	vendor = usbvendors.get(vid)
	if vendor:
		strg = str(vendor)
	else:
		return ""
	product = usbproducts.get((vid, pid))
	if product:
		return strg + " " + str(product)
	return strg

def find_usb_class(cid, sid, pid):
	"Return USB protocol from usbclasses list"
	lnlst = len(usbclasses)
	cls = usbclasses.get((cid, sid, pid)) \
		or usbclasses.get((cid, sid, -1)) \
		or usbclasses.get((cid, -1, -1))
	if cls:
		return str(cls)
	return ""


devlst = [
	'host',			# usb-storage
	'video4linux/video',	# uvcvideo et al.
	'sound/card',		# snd-usb-audio
	'net/',			# cdc_ether, ...
	'input/input',		# usbhid
	'usb:hiddev',		# usb hid
	'bluetooth/hci',	# btusb
	'ttyUSB',		# btusb
	'tty/',			# cdc_acm
	'usb:lp',		# usblp
	#'usb/lp',		# usblp
	'usb/',			# hiddev, usblp
	#'usbhid',		# hidraw
]

def find_storage(hostno):
	"Return SCSI block dev names for host"
	res = ""
	for ent in os.listdir("/sys/class/scsi_device/"):
		(host, bus, tgt, lun) = ent.split(":")
		if host == hostno:
			try:
				for ent2 in os.listdir("/sys/class/scsi_device/%s/device/block" % ent):
					res += ent2 + " "
			except:
				pass
	return res

def add_drv(path, drvnm):
	res = ""
	try:
		for e2 in os.listdir(path+"/"+drvnm):
			if e2[0:len(drvnm)] == drvnm:
				res += e2 + " "
		try:
			if res:
				res += "(" + os.path.basename(os.readlink(path+"/driver")) + ") "
		except:
			pass
	except:
		pass
	return res

def find_dev(driver, usbname):
	"Return pseudo devname that's driven by driver"
	res = ""
	for nm in devlst:
		dirnm = prefix + usbname
		prep = ""
		idx = nm.find('/')
		if idx != -1:
			prep = nm[:idx+1]
			dirnm += "/" + nm[:idx]
			nm = nm[idx+1:]
		ln = len(nm)
		try:
			for ent in os.listdir(dirnm):
				if ent[:ln] == nm:
					res += prep+ent+" "
					if nm == "host":
						res += "(" + find_storage(ent[ln:])[:-1] + ")"
		except:
			pass
	if driver == "usbhid":
		rg = re.compile(r'[0-9A-F]{4}:[0-9A-F]{4}:[0-9A-F]{4}\.[0-9A-F]{4}')
		for ent in os.listdir(prefix + usbname):
			m = rg.match(ent)
			if m:
				res += add_drv(prefix+usbname+"/"+ent, "hidraw")
				add = add_drv(prefix+usbname+"/"+ent, "input")
				if add:
					res += add
				else:
					for ent2 in os.listdir(prefix+usbname+"/"+ent):
						m = rg.match(ent2)
						if m:
							res += add_drv(prefix+usbname+"/"+ent+"/"+ent2, "input")
	return res


class UsbObject:
	def read_attr(self, name):
		path = prefix + self.path + "/" + name
		return open(path).readline().strip()

	def read_link(self, name):
		path = prefix + self.path + "/" + name
		return os.path.basename(os.readlink(path))

class UsbEndpoint(UsbObject):
	"Container for USB endpoint info"
	def __init__(self, parent, fname, level):
		self.parent = parent
		self.level = level
		self.fname = fname
		self.path = ""
		self.epaddr = 0
		self.len = 0
		self.ival = ""
		self.type = ""
		self.attr = 0
		self.max = 0
		if self.fname:
			self.read(self.fname)

	def read(self, fname):
		self.fname = fname
		self.path = self.parent.path + "/" + fname
		self.epaddr = int(self.read_attr("bEndpointAddress"), 16)
		ival = int(self.read_attr("bInterval"), 16)
		if ival:
			self.ival = " (%s)" % self.read_attr("interval")
		self.len = int(self.read_attr("bLength"), 16)
		self.type = self.read_attr("type")
		self.attr = int(self.read_attr("bmAttributes"), 16)
		self.max = int(self.read_attr("wMaxPacketSize"), 16)

	def __repr__(self):
		return "<UsbEndpoint[%r]>" % self.fname

	def __str__(self):
		indent = "  " * self.level
		#name = "%s/ep_%02X" % (self.parent.fname, self.epaddr)
		name = ""
		body = "(EP) %02x: %s%s attr %02x len %02x max %03x" % \
			(self.epaddr, self.type, self.ival, self.attr, self.len, self.max)
		body = colorize(5, body)
		return "%-17s %s\n" % (indent + name, indent + body)


class UsbInterface(UsbObject):
	"Container for USB interface info"
	def __init__(self, parent, fname, level=1):
		self.parent = parent
		self.level = level
		self.fname = fname
		self.path = ""
		self.iclass = 0
		self.isclass = 0
		self.iproto = 0
		self.noep = 0
		self.driver = ""
		self.devname = ""
		self.protoname = ""
		self.eps = []
		if self.fname:
			self.read(self.fname)

	def read(self, fname):
		self.fname = fname
		self.path = self.parent.path + "/" + fname
		self.iclass = int(self.read_attr("bInterfaceClass"),16)
		self.isclass = int(self.read_attr("bInterfaceSubClass"),16)
		self.iproto = int(self.read_attr("bInterfaceProtocol"),16)
		self.noep = int(self.read_attr("bNumEndpoints"))
		try:
			self.driver = self.read_link("driver")
			self.devname = find_dev(self.driver, self.path)
		except:
			pass
		self.protoname = find_usb_class(self.iclass, self.isclass, self.iproto)
		if showeps:
			for dirent in os.listdir(prefix + self.path):
				if dirent.startswith("ep_"):
					ep = UsbEndpoint(self, dirent, self.level+1)
					self.eps.append(ep)

	def __repr__(self):
		return "<UsbInterface[%r]>" % self.fname

	def __str__(self):
		indent = "  " * self.level
		name = self.fname
		plural = (" " if self.noep == 1 else "s")
		body = "(IF) %02x:%02x:%02x %iEP%s (%s) %s %s" % \
			(self.iclass, self.isclass, self.iproto, self.noep, plural,
			 self.protoname, colorize(3, self.driver), colorize(4, self.devname))
		strg = "%-17s %s\n" % (indent + name, indent + body)
		if showeps and self.eps:
			for ep in self.eps:
				strg += str(ep)
		return strg

class UsbDevice(UsbObject):
	"Container for USB device info"
	def __init__(self, parent, fname, level=0):
		self.parent = parent
		self.level = level
		self.fname = fname
		self.path = ""
		self.iclass = 0
		self.isclass = 0
		self.iproto = 0
		self.vid = 0
		self.pid = 0
		self.name = ""
		self.usbver = ""
		self.speed = ""
		self.maxpower = ""
		self.wakeup = ""
		self.noports = 0
		self.nointerfaces = 0
		self.driver = ""
		self.devname = ""
		self.interfaces = []
		self.children = []
		if self.fname:
			self.read(self.fname)
			self.readchildren()

	def read(self, fname):
		self.fname = fname
		self.path = fname
		self.iclass = int(self.read_attr("bDeviceClass"), 16)
		self.isclass = int(self.read_attr("bDeviceSubClass"), 16)
		self.iproto = int(self.read_attr("bDeviceProtocol"), 16)
		self.vid = int(self.read_attr("idVendor"), 16)
		self.pid = int(self.read_attr("idProduct"), 16)
		try:
			self.name = self.read_attr("manufacturer") + " " \
				  + self.read_attr("product")
		except:
			pass
		if self.name:
			mch = re.match(r"Linux [^ ]* (.hci[_-]hcd) .HCI Host Controller", self.name)
			if mch:
				self.name = mch.group(1)
		if not self.name:
			self.name = find_usb_prod(self.vid, self.pid)
		# Some USB Card readers have a better name then Generic ...
		if self.name.startswith("Generic"):
			oldnm = self.name
			self.name = find_usb_prod(self.vid, self.pid)
			if not self.name:
				self.name = oldnm
		try:
			ser = self.read_attr("serial")
			# Some USB devs report "serial" as serial no. suppress
			if (ser and ser != "serial"):
				self.name += " " + ser
		except:
			pass
		self.usbver = self.read_attr("version")
		self.speed = self.read_attr("speed")
		self.maxpower = self.read_attr("bMaxPower")
		self.noports = int(self.read_attr("maxchild"))
		try:
			self.nointerfaces = int(self.read_attr("bNumInterfaces"))
		except:
			self.nointerfaces = 0
		try:
			self.driver = self.read_link("driver")
			self.devname = find_dev(self.driver, self.path)
		except:
			pass
		if showwakeup:
			try:
				self.wakeup = self.read_attr('power/wakeup')
			except:
				self.wakeup = "unsupported"

	def readchildren(self):
		if self.fname[0:3] == "usb":
			fname = self.fname[3:]
		else:
			fname = self.fname
		for dirent in os.listdir(prefix + self.fname):
			if not dirent[0:1].isdigit():
				continue
			if os.access(prefix + dirent + "/bInterfaceClass", os.R_OK):
				iface = UsbInterface(self, dirent, self.level+1)
				self.interfaces.append(iface)
			else:
				usbdev = UsbDevice(self, dirent, self.level+1)
				self.children.append(usbdev)
		usbsortkey = lambda obj: [int(x) for x in re.split(r"[-:.]", obj.fname)]
		self.interfaces.sort(key=usbsortkey)
		self.children.sort(key=usbsortkey)

	def __repr__(self):
		return "<UsbDevice[%r]>" % self.fname

	def __str__(self):
		is_hub = (self.iclass == HUB_ICLASS)
		if is_hub:
			if noemptyhub and len(self.children) == 0:
				return ""
		strg = ""
		if not (nohub and is_hub):
			indent = "  " * self.level
			name = self.fname
			plural = (" " if self.nointerfaces == 1 else "s")
			body = "%s %02x %iIF%s [USB %s, %5s Mbps, %5s%s] (%s)%s" % \
				(colorize(1, "%04x:%04x" % (self.vid, self.pid)),
				 self.iclass, self.nointerfaces, plural,
				 self.usbver.strip(), self.speed, self.maxpower,
				 ("" if self.wakeup == "" else (",  power wakeup: %s" % self.wakeup)),
				 colorize(2 if is_hub else 1, self.name),
				 colorize(2, " hub") if is_hub else "")
			strg = "%-17s %s\n" % (indent + name, indent + body)
			if not (is_hub and not showhubint):
				if showeps:
					ep = UsbEndpoint(self, "ep_00", self.level+1)
					strg += str(ep)
				if showint:	
					for iface in self.interfaces:
						strg += str(iface)
		for child in self.children:
			strg += str(child)
		return strg


def usage():
	"Displays usage information"
	print("Usage: lsusb.py [options]")
	print()
	print("Options:")
	print("  -h, --help            display this help")
	print("  -i, --interfaces      display interface information")
	print("  -I, --hub-interfaces  display interface information, even for hubs")
	print("  -u, --hide-empty-hubs suppress empty hubs")
	print("  -U, --hide-hubs       suppress all hubs")
	print("  -c, --color           use colors")
	print("  -C, --no-color        disable colors")
	print("  -e, --endpoints       display endpoint info")
	print("  -f FILE, --usbids-path FILE")
	print("                        override filename for /usr/share/{hwdata/,}usb.ids")
	print("  -w, --wakeup          display power wakeup setting")
	print()
	print("Use lsusb.py -ciu to get a nice overview of your USB devices.")

def read_usb():
	"Read toplevel USB entries and print"
	root_hubs = []
	for dirent in os.listdir(prefix):
		if not dirent[0:3] == "usb":
			continue
		usbdev = UsbDevice(None, dirent, 0)
		root_hubs.append(usbdev)
	root_hubs.sort(key=lambda x: int(x.fname[3:]))
	for usbdev in root_hubs:
		print(usbdev, end="")

def main(argv):
	"main entry point"
	global showint, showhubint, noemptyhub, nohub
	global cols, usbids, showeps, showwakeup
	usecols = None

	long_options = [
		"help",
		"interfaces",
		"hub-interfaces",
		"hide-empty-hubs",
		"hide-hubs",
		"color",
		"no-color",
		"usbids-path=",
		"endpoints",
		"wakeup",
	]

	try:
		(optlist, args) = getopt.gnu_getopt(argv[1:], "hiIuUwcCef:", long_options)
	except getopt.GetoptError as exc:
		print("Error:", exc, file=sys.stderr)
		sys.exit(2)
	for opt in optlist:
		if opt[0] in {"-h", "--help"}:
			usage()
			sys.exit(0)
		elif opt[0] in {"-i", "--interfaces"}:
			showint = True
		elif opt[0] in {"-I", "--hub-interfaces"}:
			showint = True
			showhubint = True
		elif opt[0] in {"-u", "--hide-empty-hubs"}:
			noemptyhub = True
		elif opt[0] in {"-U", "--hide-hubs"}:
			noemptyhub = True
			nohub = True
		elif opt[0] in {"-c", "--color"}:
			usecols = True
		elif opt[0] in {"-C", "--no-color"}:
			usecols = False
		elif opt[0] in {"-f", "--usbids-path"}:
			usbids = [opt[1]]
		elif opt[0] in {"-e", "--endpoints"}:
			showeps = True
		elif opt[0] in {"-w", "--wakeup"}:
			showwakeup = True
	if len(args) > 0:
		print("Error: excess args %s ..." % args[0], file=sys.stderr)
		sys.exit(2)

	if usecols is None:
		usecols = sys.stdout.isatty() and os.environ.get("TERM", "dumb") != "dumb"

	if usecols:
		cols = (norm, bold, red, green, amber, blue)

	if usbids[0]:
		try:
			parse_usb_ids()
		except:
			print(" WARNING: Failure to read usb.ids", file=sys.stderr)
			#print(sys.exc_info(), file=sys.stderr)
	read_usb()

# Entry point
if __name__ == "__main__":
	main(sys.argv)


