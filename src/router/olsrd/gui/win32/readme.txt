Build the olsr.org Routing Daemon on Windows
============================================

Some remarks that may help you to build and run the olsr.org's
routing daemon on Windows. You need the following softs to 
compile:

- A Cygwin compiling environment. Download the Cygwin setup.exe
  from http://www.cygwin.com/ and install at least these packages:
  devel/bison, devel/flex, devel/gcc-mingw-g++ and devel/make

- You need a running copy of MSVC. Either MSVC6 or MSVC8 may be
  supported. You also need the Microsoft Platform SDK installed.
  MSVC8 should be easy, while MSVC6 needs an older version:
  http://www.microsoft.com/msdownload/platformsdk/sdkupdate/psdk-full.htm
  It's OK to install the "Core SDK" and ignore other stuff.

- Be sure to add the Microsoft SDK pathes to your MSVC-GUI-Pathes
  (Extras/Options/Pathes, Add Inlude and Lib at least)

- To build the setup, Nullsoft's installer is required. Download:
  http://nsis.sourceforge.net/

You need the Cygwin suite up and running. Normally, the users home
dir is not installed properly with this. Start the "bash" command
line and check by entering "cd". If an error message shows up you
can set a shortcut/link from c:\cygwin to c:\Docs+Settings, then go
to Windows-SysCtl/System/Advanced/Envrionment and explicitly set the
HOME=c:\Docs+Settings\[yourloginname]. Then try again to "cd".

Then enter the olsrd's directory (either bash or cmd.exe will do).
Start "make all libs". Should compile. Spits out olsrd.exe and some
dll's for the plugins. For the GUI, you also need the linking lib to
the olsrd_cfgparser.dll: "make gui/win32/Main/olsrd_cfgparser.lib"

Then enter your favorite MSVC flavour and hit [F7]. After some prayers,
you may be able to hack in the GUI changes you want...

// Sven-Ola in Dec-2007
