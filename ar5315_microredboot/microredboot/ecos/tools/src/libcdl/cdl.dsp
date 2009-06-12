# Microsoft Developer Studio Project File - Name="cdl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=cdl - Win32 ANSI Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cdl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cdl.mak" CFG="cdl - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cdl - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "cdl - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE "cdl - Win32 ANSI Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE "cdl - Win32 ANSI Release" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "cdl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "V:\cdl\Release"
# PROP Intermediate_Dir "V:\cdl\Release\build"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "cdl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cdl___Win32_Debug"
# PROP BASE Intermediate_Dir "cdl___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "V:\cdl\Debug"
# PROP Intermediate_Dir "V:\cdl\Debug\build"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "cdl - Win32 ANSI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cdl___Win32_ANSI_Debug"
# PROP BASE Intermediate_Dir "cdl___Win32_ANSI_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\cdl\Debug"
# PROP Intermediate_Dir "v:\cdl\Debug\build"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "cdl - Win32 ANSI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cdl___Win32_ANSI_Release"
# PROP BASE Intermediate_Dir "cdl___Win32_ANSI_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "V:\cdl\Release"
# PROP Intermediate_Dir "V:\cdl\Release\build"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "cdl - Win32 Release"
# Name "cdl - Win32 Debug"
# Name "cdl - Win32 ANSI Debug"
# Name "cdl - Win32 ANSI Release"
# Begin Source File

SOURCE=.\base.cxx
# End Source File
# Begin Source File

SOURCE=.\build.cxx
# End Source File
# Begin Source File

SOURCE=.\cdl.hxx
# End Source File
# Begin Source File

SOURCE=.\cdlcore.hxx
# End Source File
# Begin Source File

SOURCE=.\cdlmisc.cxx
# End Source File
# Begin Source File

SOURCE=.\ChangeLog

!IF  "$(CFG)" == "cdl - Win32 Release"

# Begin Custom Build - Performing Custom Build Step for libCDL
IntDir=V:\cdl\Release\build
OutDir=V:\cdl\Release
InputPath=.\ChangeLog

BuildCmds= \
	if not exist $(IntDir)\tools\configtool\standalone\common\Makefile sh -c "ECOSHOST=`echo ""puts [ file attributes [ pwd ] -shortname ]"" | tclsh`/.. ; echo ""ECOSHOST=$ECOSHOST"" ; echo ""TCLHOME=$TCLHOME"" ; mkdir -p `cygpath -u ""$(IntDir)""` ; cd `cygpath -u ""$(IntDir)""` && CC=cl CXX=cl `cygpath -u ""$ECOSHOST""`/configure --prefix=`cygpath -u ""$(OutDir)""` --with-tcl=`cygpath -u ""$TCLHOME""` --with-tcl_version=82" \
	cd $(IntDir) \
	v: \
	make --unix install \
	

"$(OutDir)\lib\cdl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\lib\cyginfra.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "cdl - Win32 Debug"

# Begin Custom Build - Performing Custom Build Step for libCDL
IntDir=V:\cdl\Debug\build
OutDir=V:\cdl\Debug
InputPath=.\ChangeLog

BuildCmds= \
	if not exist $(IntDir)\tools\configtool\standalone\common\Makefile sh -c "ECOSHOST=`echo ""puts [ file attributes [ pwd ] -shortname ]"" | tclsh`/.. ; echo ""ECOSHOST=$ECOSHOST"" ; echo ""TCLHOME=$TCLHOME"" ; mkdir -p `cygpath -u ""$(IntDir)""` ; cd `cygpath -u ""$(IntDir)""` && CC=cl CXX=cl `cygpath -u ""$ECOSHOST""`/configure --prefix=`cygpath -u ""$(OutDir)""` --with-tcl=`cygpath -u ""$TCLHOME""` --with-tcl_version=82d --enable-debug --enable-maintainer-mode" \
	cd $(IntDir) \
	v: \
	make --unix install \
	

"$(OutDir)\lib\cdl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\lib\cyginfra.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "cdl - Win32 ANSI Debug"

# Begin Custom Build - Performing Custom Build Step for libCDL
IntDir=v:\cdl\Debug\build
OutDir=v:\cdl\Debug
InputPath=.\ChangeLog

BuildCmds= \
	if not exist $(IntDir)\tools\configtool\standalone\common\Makefile sh -c "ECOSHOST=`echo ""puts [ file attributes [ pwd ] -shortname ]"" | tclsh`/.. ; echo ""ECOSHOST=$ECOSHOST"" ; echo ""TCLHOME=$TCLHOME"" ; mkdir -p `cygpath -u ""$(IntDir)""` ; cd `cygpath -u ""$(IntDir)""` && CC=cl CXX=cl `cygpath -u ""$ECOSHOST""`/configure --prefix=`cygpath -u ""$(OutDir)""` --with-tcl=`cygpath -u ""$TCLHOME""` --with-tcl_version=82d --enable-debug --enable-maintainer-mode" \
	cd $(IntDir) \
	v: \
	make --unix install \
	

"$(OutDir)\lib\cdl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\lib\cyginfra.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "cdl - Win32 ANSI Release"

# Begin Custom Build - Performing Custom Build Step for libCDL
IntDir=V:\cdl\Release\build
OutDir=V:\cdl\Release
InputPath=.\ChangeLog

BuildCmds= \
	echo $(PATH) \
	if not exist $(IntDir)\tools\configtool\standalone\common\Makefile sh -c "ECOSHOST=`echo ""puts [ file attributes [ pwd ] -shortname ]"" | tclsh`/.. ; echo ""ECOSHOST=$ECOSHOST"" ; echo ""TCLHOME=$TCLHOME"" ; mkdir -p `cygpath -u ""$(IntDir)""` ; cd `cygpath -u ""$(IntDir)""` && CC=cl CXX=cl `cygpath -u ""$ECOSHOST""`/configure --prefix=`cygpath -u ""$(OutDir)""` --with-tcl=`cygpath -u ""$TCLHOME""` --with-tcl_version=82" \
	cd $(IntDir) \
	v: \
	make --unix install \
	

"$(OutDir)\lib\cdl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\lib\cyginfra.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\component.cxx
# End Source File
# Begin Source File

SOURCE=.\config.cxx
# End Source File
# Begin Source File

SOURCE=.\conflict.cxx
# End Source File
# Begin Source File

SOURCE=.\database.cxx
# End Source File
# Begin Source File

SOURCE=.\dialog.cxx
# End Source File
# Begin Source File

SOURCE=.\expr.cxx
# End Source File
# Begin Source File

SOURCE=.\interface.cxx
# End Source File
# Begin Source File

SOURCE=.\interp.cxx
# End Source File
# Begin Source File

SOURCE=.\option.cxx
# End Source File
# Begin Source File

SOURCE=.\package.cxx
# End Source File
# Begin Source File

SOURCE=.\parse.cxx
# End Source File
# Begin Source File

SOURCE=.\property.cxx
# End Source File
# Begin Source File

SOURCE=.\refer.cxx
# End Source File
# Begin Source File

SOURCE=.\transact.cxx
# End Source File
# Begin Source File

SOURCE=.\value.cxx
# End Source File
# Begin Source File

SOURCE=.\wizard.cxx
# End Source File
# End Target
# End Project
