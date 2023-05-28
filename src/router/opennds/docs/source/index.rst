.. openNDS documentation master file, created by
   sphinx-quickstart on Thu Nov 10 13:53:25 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the documentation for openNDS
========================================

openNDS (open Network Demarcation Service) is a high performance, small footprint, Captive Portal. It provides a border control gateway between a public local area network and the Internet.

It supports all ranges between small stand alone venues through to large mesh networks with multiple portal entry points.

Both the client driven Captive Portal Detection (CPD) method and gateway driven Captive Portal Identification method (CPI - RFC 8910 and RFC 8908) are supported.

In its default configuration, openNDS offers a dynamically generated and adaptive splash page sequence. Internet access is granted by a click to continue button, accepting Terms of Service. A simple option enables input forms for user login.

The package incorporates the FAS API allowing many flexible customisation options. The creation of sophisticated third party authentication applications is fully supported.

Internet hosted **https portals** can be implemented with no security errors, to inspire maximum user confidence.

It is a fork of the NoDogSplash project that in turn was derived originally from the codebase of the Wifi Guard Dog project.

openNDS is released under the GNU General Public License.

* openNDS: https://github.com/openNDS/openNDS
* Original Homepage *down*: http://kokoro.ucsd.edu/nodogsplash
* Archive: https://web.archive.org/web/20140210131130/http://kokoro.ucsd.edu/nodogsplash
* Wifidog *down*: http://dev.wifidog.org/
* Archive: https://web.archive.org/web/20220518120459/http://dev.wifidog.org/wiki/About
* NoDogSplash: https://github.com/nodogsplash/nodogsplash
* GNU GPL: http://www.gnu.org/copyleft/gpl.html

The following describes what openNDS does, how to get it and run it, and
how to customize its behavior for your application.

Contents:

.. toctree::
   :maxdepth: 2

   overview
   changelog
   howitworks
   install
   splash
   customize
   config
   customparams
   fas
   themespec
   preauth
   binauth
   walledgarden
   libraries
   ndsctl
   traffic
   clientstatus
   faq
   compile
   debug
   todo

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`

