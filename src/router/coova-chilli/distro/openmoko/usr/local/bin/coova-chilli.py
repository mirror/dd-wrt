#!/usr/bin/python
"""
 CoovaChilli User Interface "coova-chilli-gui"
 Copyright (C) 2009  David Bird <david@coova.com>
 Copyright (C) 2009  Andre' "Source" Pedersen marco(at)cakebox(dot)net

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""


import pygtk
pygtk.require('2.0')

import os
import sys
import gtk
import time
import string
import socket
import fcntl
import struct
import array
import gobject
import ConfigParser
import subprocess

sys.path.append("/usr/local/lib/python") 

import CoovaChilliLib

class CoovaChilliGui(CoovaChilliLib.CoovaChilli):

    def get_ip( self, ifname, data = None ):
        s = socket.socket( socket.AF_INET, socket.SOCK_DGRAM )
        ip = socket.inet_ntoa( fcntl.ioctl( s.fileno(), 0x8915, struct.pack( '256s', ifname[:15] ) ) [20:24] )
        s.close()
        return ip

    def ifaceUpDown( self, chkIface, data = None ):
        max_possible = 128  # arbitrary. raise if needed.
        bytes = max_possible * 32
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        names = array.array('B', '\0' * bytes)
        outbytes = struct.unpack( 'iL', fcntl.ioctl( s.fileno(), 0x8912, struct.pack( 'iL' , bytes, names.buffer_info() [0] ) ) ) [0]
        s.close()
        namestr = names.tostring()
        iface_list = [namestr[i:i+32].split('\0', 1)[0] for i in range(0, outbytes, 32)]
        return iface_list.count(chkIface)

    def interfaceStatus ( self, data = None ):
        chilliUp = False

        if self.ifaceUpDown('tun0') == 1:
            self.btnStart.set_sensitive( False )
            self.btnStop.set_sensitive( True )

            try:
                configfile = open(self.Section, 'r')
                name = configfile.readline()
                configfile.close()
            except:
                name=''

            if name == '':
                name = "Hotspot"

            self.lblStat.set_markup( '<b><span color="green">'+name+' is UP</span></b>' )
            tip = self.get_ip( "tun0" )
            chilliTunIp = "<b>Chilli:</b> %s" % ( tip )
            self.lblChilli.set_markup( chilliTunIp )
            chilliUp = True

        elif self.ifaceUpDown('tun0') == 0:
            self.btnStart.set_sensitive( True )
            self.btnStop.set_sensitive( False )
            self.lblStat.set_markup( '<b><span color="red">Hotspot is DOWN</span></b>' )
            chilliTunIp = '<b>Chilli:</b> <span color="red">Down</span>'
            self.lblChilli.set_markup( chilliTunIp )

        if self.ifaceUpDown('eth0') == 1:
            tip = self.get_ip( "eth0" )
            wifiLbl = "<b>WiFi:</b> %s" % (tip)
            self.lblWiFi.set_markup( wifiLbl )

        elif chilliUp:
            wifiLbl = '<b>WiFi:</b> Hotspot'
            self.lblWiFi.set_markup( wifiLbl )

        elif self.ifaceUpDown('eth0') == 0:
            wifiLbl = '<b>WiFi:</b> <span color="red">Down</span>'
            self.lblWiFi.set_markup( wifiLbl )

        """ USB Check """
        if self.ifaceUpDown('usb0') == 1:
            tip = self.get_ip( "usb0" )
            usbLbl = "<b>USB:</b> %s" % (tip)
            self.lblUsb.set_markup( usbLbl )

        elif self.ifaceUpDown('usb0') == 0:
            usbLbl = '<b>USB:</b> <span color="red">Down</span>'
            self.lblUsb.set_markup( usbLbl )

        """ GPRS Check """
        if self.ifaceUpDown('ppp0') == 1:
            tip = self.get_ip( "ppp0" )
            gprsLbl = "<b>GPRS:</b> %s" % (tip)
            self.lblGprs.set_markup( gprsLbl )

        elif self.ifaceUpDown('ppp0') == 0:
            gprsLbl = '<b>GPRS:</b> <span color="red">Down</span>'
            self.lblGprs.set_markup( gprsLbl )

        return True

    def __init__( self, data=None ):

        CoovaChilliLib.CoovaChilli.__init__(self)

        self.winMain = gtk.Window()
        self.winMain.set_title('CoovaChilli')
        self.winMain.connect( 'delete-event', gtk.main_quit )
        
        self.vboxMain = gtk.VBox( homogeneous = True )
        self.hboxBtns = gtk.HBox( homogeneous = True )

        self.notebook = gtk.Notebook()
        self.notebook.set_tab_pos(gtk.POS_BOTTOM)

        self.lblMain = gtk.Label( "Main" )
        self.lblConfig = gtk.Label( "Config" )
        self.lblSessions = gtk.Label( "Sessions" )
        self.lblAbout = gtk.Label( "About" )
        
        self.lblStat = gtk.Label( "Initializing" )

        self.lblChilli = gtk.Label()
        self.lblChilli.set_markup( "<b>Chilli:</b> N/A" )

        self.lblWiFi = gtk.Label()
        self.lblWiFi.set_markup( "<b>WiFi:</b> N/A" )

        self.lblUsb = gtk.Label()
        self.lblUsb.set_markup( "<b>usb0:</b> N/A" )
        
        self.lblGprs = gtk.Label()
        self.lblGprs.set_markup( "<b>GPRS:</b> N/A" )

        self.vboxMain.pack_start( self.hboxBtns, padding = 5 )
        self.hboxBtns.pack_start( self.sectionBox, False )
        self.hboxBtns.pack_start( self.btnStart )
        self.hboxBtns.pack_start( self.btnStop )

        self.vboxMain.pack_start( self.lblStat )
        self.vboxMain.pack_start( self.lblChilli )
        self.vboxMain.pack_start( self.lblWiFi )
        self.vboxMain.pack_start( self.lblUsb )
        self.vboxMain.pack_start( self.lblGprs )

        self.lblAboutContent = gtk.Label ( )
        self.lblAboutContent.set_markup( self.About + "\n\nbased on ovpngui\nby |Marco| on FreeNode" )

        self.makeConfigTable( )

        gobject.timeout_add( 2000, self.interfaceStatus )

        gobject.timeout_add( 30000, self.chilliQuery )
                
        self.sectionBox.connect('changed', self.changeSection)
        self.sectionBox2.connect('changed', self.changeSection2)

        self._changeSection(self.section)

        self.btnStart.show()
        self.btnStop.show()

        self.lblStat.show()
        self.lblChilli.show()
        self.lblWiFi.show()
        self.lblUsb.show()
        self.lblGprs.show()
        
        self.hboxBtns.show()
        self.vboxMain.show()

        self.scrollConfig = gtk.ScrolledWindow()
        self.scrollConfig.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        self.scrollConfig.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC) 

        self.scrollConfig.add_with_viewport(self.configTable)
        self.scrollConfig.show()

        self.scrollSessions = gtk.ScrolledWindow()
        self.scrollSessions.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        self.scrollSessions.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC) 

        self.scrollSessions.add_with_viewport(self.vboxSessions)
        self.scrollSessions.show()

        self.winMain.show()

        self.notebook.show()
        self.lblAboutContent.show()
        self.configTable.show()
        
        self.winMain.add( self.notebook )
        self.notebook.append_page( self.vboxMain, self.lblMain )
        self.notebook.append_page( self.scrollSessions, self.lblSessions )
        self.notebook.append_page( self.scrollConfig, self.lblConfig )
        self.notebook.append_page( self.lblAboutContent, self.lblAbout )
        self.chilliQuery()

    def main( self ):
        gtk.main()


if __name__ == "__main__":
    COOVACHILLIGUI = CoovaChilliGui()
    COOVACHILLIGUI.main()
