eCos Cross-platform Configuration Tool
Copyright (c) Red Hat Inc., 2001-2002
======================================

Version 2
=========

Contents

  * Introduction
  * What's in this release?
  * Installing the Configuration Tool
  * Running the Configuration Tool
  * Frequently Asked Questions
  * Building the Configuration Tool under Linux
  * Building the Configuration Tool under Windows


Introduction
============

  Welcome to the eCos Configuration Tool, a graphical tool to
  help a user configure and build a custom version of the
  eCos operating system.

  This is a cross-platform version built using the wxWindows
  toolkit. The tool uses the GTK+ widget set on Linux, and the
  WIN32 API on Windows 9x, Windows NT and Windows 2000. It is
  similar to the MFC, Windows-only version but at present lacks
  a few of its features, such as the Memory Layout Tool and the
  ability to run tests from within the tool.

  Please note that this is alpha-level code. However, all feedback to
  the eCos team is appreciated, via ecos-discuss@sources.redhat.com or the
  bug reporting form at http://sources.redhat.com/ecos/problemreport.html.

  These are the instructions for running and building the eCos
  Configuration Tool for Linux and Windows.


What's in this release?
=======================

  This version allows you to edit, load, save and build eCos
  configurations much as the original Windows Configuration Tool.
  However the following features are missing with respect to the
  original tool:

  - Memory Layout Tool
  - Administration functionality (initial code present but untested)
  - Gauge indicating time left to build library and tests

  The following features are present in the new tool but not in the
  original tool:

  - Repository Information dialog (available from the Help menu)
  - Most dialogs are resizeable

Documentation works a little differently. Instead of using a
precompiled HTML Help file, this version compiles a documentation
index on the fly for use with its own internal HTML help viewer. The
cached indexes are placed in the .eCosDocs directory under the
user's home directory (Linux) or in the installed repository
(Windows). The internal viewer cannot cope with all of the eCos
and GNUPro documentation, so for these occasions please use an
external browser (see the Settings dialog).

The documentation for the Configuration Tool is supplied in its
install directory as HTML only, and is a modified version of the
eCos User's Guide. Invoke the tool help from the
"Help|Configuration Tool Help" menu item, or from the internal
help viewer's index, under "Linux Configuration Tool Guide".

Known bugs:

  - The documentation index only lists the packages in the
    configuration active when the documentation was indexed
    (normally when the repository is first seen by the
    Configuration Tool).

Version History
---------------

See CHANGES.txt.

Installing the Configuration Tool
=================================

  The Configuration Tool can be used with existing eCos
  installations and CVS source hierarchies. You can get the
  binaries from ftp://sources.redhat.com/pub/ecos/ct2/.

  See also mirror sites at:

        http://sources.redhat.com/ecos/mirror.html
        http://sources.redhat.com/mirrors.html

  Please try to use a mirror site close to you, as it will be
  faster.

  *** Under Linux:

  Download:
  ftp://sources.redhat.com/pub/ecos/ct2/configtool-2.0-i386.tar.gz
  (or similar name).

  Unarchive the tar file into a suitable directory and add the
  directory to your path. You do not have to install it as root.
  For example:

  % mkdir -p /opt/ecos/configtool/bin
  % cd /opt/ecos/configtool/bin
  % tar xvfz /cdrom/configtool-2.0.tar.gz
  % export PATH=/opt/ecos/configtool/bin:$PATH

  The following files will be extracted to the installation directory:

  configtool
  configtool.bin
  README.txt
  license.txt
  ecosplatforms.tar.gz
  manual/

  Optionally, untar the contents of ecosplatforms.tar.gz into your
  home directory, where it will create a directory called
  .eCosPlatforms. The information in these files isn't yet used
  by the Configuration Tool, but will keep the tool from
  generating some warnings.

  The executable is statically linked to the wxWindows library,
  but does require the GTK+ 1.2, GDK 1.2 and Tcl 8.x libraries
  to be installed.

  *** Under Windows:

  Download configtool-2.0-setup.exe (or similar name).

  Run the installer provided, preferably having installed eCos
  for Windows on your PC previously so the tool can pick up the
  relevant registry information added by the eCos installer.
  A new eCos Configuration Tool group will be added to your
  Start menu and a shortcut to the Configuration Tool will
  appear on your desktop.


Running the Configuration Tool
==============================

  Run the configtool executable and (on Linux) ignore any
  initial console messages, which may be suppressed by
  unarchiving ecosplatforms.tar.gz as per the installation
  instructions above.

  You can invoke the tool with zero, one, or two parameters. The
  two parameters can be the location of the repository and/or
  the location of a save file (extension .ecc). If no parameters
  are passed, the tool will look in the current directory for a
  save file and also (on Linux) in the /opt/ecos directory for a
  suitable repository. Failing that, the tool will use the last
  loaded repository or ask the user for a suitable location.

  For detailed information about the Configuration Tool, please
  refer to the HTML manual which may be invoked from the Help
  menu or by clicking on the "Linux Configuration Tool Guide" in
  the internal help system's contents. This is similar to but
  different from "The eCos Configuration Tool" section in the
  eCos User Guide, which refers to the original Windows version
  of the configuration tool (as opposed to the new
  cross-platform Linux and Windows version).


Frequently Asked Questions
==========================

Q:  On Linux, invoking HTML documentation for a configuration item
    doesn't seem to work.

A:  You need to have a .mailcap entry similar to the following:

    text/html; netscape -no-about-splash %s
    
    and in .mime.types:
    
    type=text/html \
    desc="HTML document" \
    exts="htm,html"

    Also, be aware that the browser can sometimes end up behind the
    configuration tool so it may have run even if you think it didn't.


Q:  On Linux, right-clicking in the configuration pane and choosing
    'What's This?' causes an information window to pop up and
    then quickly disappear.

A:  If the window manager options are set to raise a window when it gains
    the focus, the information window can get sent behind the
    application window when the popup menu loses focus. Either
    adjust your window manager settings, or use an alternate way
    of invoking help for the item: click on the arrow/question
    mark toolbar button, then on the item you are interested in.


Q:  The internal HTML help viewer doesn't display HTML correctly.

A:  The internal HTML help viewer cannot display some of the more complex
    HTML correctly, such as the GNUPro reference. We hope to
    provide the documentation in a suitable form in future, but
    for now, please use an external browser to view this
    documentation. You can use the Settings dialog, Viewers tab,
    to choose to view using an external browser.


Q:  Why does the Configuration Tool use wxWindows?

A:  wxWindows is an open source, mature multi-platform GUI
    toolkit for C++. It makes platform-independence relatively
    easy to achieve, whilst remaining compatibility with the
    look and feel of GTK+ and WIN32 on the respective platforms.
    wxWindows was chosen on its own merits but, by sheer
    coincidence, the author of the new Configuration Tool is
    also the original author of wxWindows. Using a
    platform-independent API will make it easier to port the
    Configuration Tool to other platforms if needed.


Q:  Where can I get more information about wxWindows?

A:  The wxWindows web site is at http://www.wxwindows.org.
    wxWindows distributions come with documentation in a
    variety of formats.


Q:  Does the new Configuration Tool make the old one obsolete?

A:  Not yet. The new tool doesn't yet support some features, such
    as the Memory Layout Tool. When these have been implemented,
    and the tool has been subject to the required quality assurance
    procedure, then we can retire the old tool.


Q:  How can I help improve the Configuration Tool?

A:  All help is very welcome: please see
    http://sources.redhat.com/ecos/faq.html for how to
    contribute.


Q:  Who do I contact when things go wrong?

A:  Please discuss problems on the ecos-discuss mailing list:
    see http://sources.redhat.com/ecos/intouch.html.


Q:  The compiler fails to compile dcclient.cpp. What do I do?

A:  It may be that you have a non-standard GTK+ 1.2.6 which has changes
    backported from 1.2.7. Use the makefile target 'wxgtkfix' to
    fix this after the error has happened (i.e. after configure
    has produced the setup.h and makefiles):

    % make -f $CONFIGTOOLDIR/Makefile WXDIR=$WXDIR ECOSDIR=$ECOSDIR LEVEL=release wxgtkfix

    or

    % make -f $CONFIGTOOLDIR/Makefile WXDIR=$WXDIR ECOSDIR=$ECOSDIR LEVEL=debug wxgtkfix

    Now use the 'wx ecc ct' targets (not 'full') to continue
    building.


Building the Configuration Tool under Linux
===========================================

This build system is subject to change.

You will need:

  o gcc 2.95.2 or later

  o GTK+ and glib 1.2.6 or above. Please remove any 1.3 development RPMs
    from your setup, using for example:

    rpm -e gtk+-gtkbeta-devel-1.3.1b-2

    You can get GTK+ and glib for Red Hat 6.2 and above from:

    ftp://ftp.gtk.org/pub/gtk/v1.2/binary/RPMS/RedHat-6.2/RPMS/i386/gtk+-1.2.8-1.i386.rpm
    ftp://ftp.gtk.org/pub/gtk/v1.2/binary/RPMS/RedHat-6.2/RPMS/i386/glib-1.2.8-1.i386.rpm

  o a suitable version of wxWindows for GTK+, available from
    from ftp://sources.redhat.com/pub/ecos/ct2/.

    Unarchive the file wxGTK-x.y.z.tgz into a suitable directory, e.g.
    
    % mkdir /home/julians/wxWindows
    % cd /home/julians/wxWindows
    % tar xvfz /tmp/wxGTK-x.y.z.tgz

    You may also wish to download and unarchive wxWindows
    documentation, which is supplied in zip form. Unarchive
    using e.g.:

    % cd /home/julians/wxWindows
    % unzip -a wxWindows-x.y.z-HTML.zip

  o an eCos source hierarchy. See http://sources.redhat.com/ecos/anoncvs.html
    for how to download this from the CVS repository. For example:

    cvs -d :pserver:anoncvs@sources.redhat.com:/cvs/ecos co ecos/host

  o other tools:

    - Tcl/Tk 8.2 or above. See http://www.tcl.tk/

Summary:

  There is no 'configure' step for the eCos Configuration Tool
  as a whole, although the host tool libraries and wxWindows
  have configure scripts which are invoked by the makefile.

  You work in a build directory of your choosing, and the
  makefile will create the following directories underneath it:
  
  ct-build-debug/		; Configtool debug build
  ecc-build-debug/		; eCos libraries debug build
  wxwin-build-debug/		; wxWindows debug build

  ct-build-release/		; Configtool release build
  ecc-build-release/		; eCos libraries release build
  wxwin-build-release/		; wxWindows release build

  You need to pass the wxWindows source directory (WXDIR) and
  eCos hierarchy directory (ECOSDIR) to the makefile, along with
  the build LEVEL (debug or release). Please see makect.sh in
  the source directory, which is a useful helper script for
  making it easier to invoke the makefile.

  You also supply a target to build, such as full (everything),
  ct (just the Configuration Tool), wx (wxWindows only) or ecc
  (eCos libraries only). It is important to supply the target as
  the _last_ command(s) on the command line.

  The makefile builds and uses wxWindows as a static library,
  which minimizes problems with shared libraries and still
  results in a reasonable size of executable (4.5 MB
  uncompressed, or under 2 MB when compressed with UPX).
  Switching off unnecessary wxWindows features may result in
  smaller executables.

  IMPORTANT NOTE: currently, there are inadequate dependencies
  in the makefiles, so please make sure you start with a clean
  directory before building (use the cleanct target if
  necessary).

Examples:

  1. This builds the eCos libraries, wxWindows, and the Configuration Tool,
     in debug mode.

     % export ECOSDIR=/home/julians/cvs/eCos # The dir above 'host'
     % export CONFIGTOOLDIR=$ECOSDIR/host/tools/configtool/standalone/wxwin
     % export WXDIR=/home/julians/wxWindows
     %
     % mkdir /tmp/ecos-build
     % cd /tmp/ecos-build
     % make -f $CONFIGTOOLDIR/Makefile WXDIR=$WXDIR ECOSDIR=$ECOSDIR LEVEL=debug full

  2. This builds just the Configuration Tool, say after a file was edited,
     in release mode.

     % export ECOSDIR=/home/julians/cvs/eCos
     % export CONFIGTOOLDIR=$ECOSDIR/host/tools/configtool/standalone/wxwin
     % export WXDIR=/home/julians/wxWindows
     %
     % cd /tmp/ecos-build
     % make -f $CONFIGTOOLDIR/Makefile WXDIR=$WXDIR ECOSDIR=$ECOSDIR LEVEL=release ct

  *** Troubleshooting

  Please see the FAQ for what to do if the wxWindows file
  dcclient.cpp fails to compile.

  For more recent versions of wxWindows, you have to pass --static to wx-config.
  If you get a lot of GTK+-related link errors, check the beginning of the makefile
  and make sure WXCONFIGFLAGS=--static.


Building the Configuration Tool under Windows
=============================================

You will need:

  o Microsoft Visual C++ 6.0 or later

  o a suitable version of wxWindows for MS Windows, available
    from ftp://sources.redhat.com/pub/ecos/ct2/. To install the
    sources in 'setup' form, just run the executable and follow
    the instructions. If you have downloaded the sources in zip
    format, you will need to unarchive the file wxMSW-x.y.z.zip
    into a suitable directory, e.g.
    
    > mkdir c:\wxWindows
    > c:
    > cd \wxWindows
    > unzip c:\temp\wxMSW-x.y.z.zip

    Alternatively, you can use WinZip or similar utility to
    unarchive the files.

    Documentation in Windows HTML Help format is supplied in
    the setup or zipped distribution and does not have to be
    downloaded separately.

  o an eCos source hierarchy. See http://sources.redhat.com/ecos/anoncvs.html
    for how to download this from the CVS repository. For example:

    cvs -d :pserver:anoncvs@sources.redhat.com:/cvs/ecos co ecos/host

  o other tools:

    - Cygwin (sometimes called GNUPro). See http://sources.redhat.com/cygwin/
    - Tcl/Tk 8.2 or above. See http://www.tcl.tk/

Summary:

  There are two main steps: building wxWindows, and building the Configuration Tool.
  The Configuration Tool project file also builds the required eCos libraries.
  Note that unlike compilation under Linux, the wxWindows objects and libraries end up in
  the wxWindows source tree.

  Before routinely building the Configuration Tool, you will need to set up
  Visual C++ with the correct paths (see below).

Steps:

  1. Execute in a DOS box:

     subst v: d:\tmp

     where d:\tmp is a suitable temporary directory where the Configuration Tool
     binaries and objects will end up.

  2. Set the TCLHOME environment variable to where Tcl is installed. On Windows 9x,
     edit autoexec.bat. On Windows NT or W2K, use the System control panel applet.

  3. Build wxWindows. To do this, run VC++ and open src/msvc.dsw in the wxWindows
     project hierarchy. Select Build | Batch Build and check
     wxvc - WIN32 Debug and wxvc - WIN32 Release. Click on Build.

  4. Open the eCos/ecc/host/tools/configtool/standalone/wxwin/ConfigtoolVC.dsw
     workspace. Choose Tools | Options and click on the Directories tab.
     Select Show directories for: Include files.

     You need to add paths such as these:

     c:\Program Files\Tcl\include
     d:\wxWindows-010212\include

     Select Show directories for: Library files. Add these paths, changing
     as necessary:

     c:\Program Files\Tcl\lib
     d:\wxWindows-010212\lib

  5. Click on Select Active Configuration... and select the one you wish to
     build, such as WIN32 Ansi Debug (note that Unicode configurations are
     not yet available). Choose Build | Rebuild All to build the project.
     The configtool.exe executable should end up in v:\Configtool\AnsiDebug.

When compiled, configtool.exe depends on the following DLLs found
in your system32 directory that should be supplied with the executable:

  TCL82.DLL (or other name if you used a different version of TCL)
  MSVCIRT.DLL
  MSVCP60.DLL
  MSVCRT.DLL

