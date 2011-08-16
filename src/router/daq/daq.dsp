# Microsoft Developer Studio Project File - Name="daq" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=daq - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "daq.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "daq.mak" CFG="daq - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "daq - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "daq - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "daq - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "api" /I "sfbpf" /I "../src/win32/WIN32-Includes" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "DAQ_DLL" /D "HAVE_CONFIG_H" /D "BUILD_PCAP_MODULE" /D "STATIC_MODULE_LIST" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "daq - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "api" /I "sfbpf" /I "../src/win32/WIN32-Includes" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "DAQ_DLL" /D "HAVE_CONFIG_H" /D "BUILD_PCAP_MODULE" /D "STATIC_MODULE_LIST" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "daq - Win32 Release"
# Name "daq - Win32 Debug"
# Begin Group "api"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\api\daq.h
# End Source File
# Begin Source File

SOURCE=.\api\daq_api.h
# End Source File
# Begin Source File

SOURCE=.\api\daq_base.c
# End Source File
# Begin Source File

SOURCE=.\api\daq_common.h
# End Source File
# Begin Source File

SOURCE=.\api\daq_mod_ops.c
# End Source File
# End Group
# Begin Group "sfbpf"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sfbpf\arcnet.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\atmuni31.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\bittypes.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\ethertype.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\gencode.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\grammar.y

!IF  "$(CFG)" == "daq - Win32 Release"

# Begin Custom Build
InputPath=.\sfbpf\grammar.y
InputName=grammar

BuildCmds= \
	c:\cygwin\bin\bison -d -psfbpf_lval -osfbpf/$(InputName).c sfbpf/$(InputName).y \
	c:\cygwin\bin\mv sfbpf/$(InputName).h sfbpf/tokdefs.h \
	

"sfbpf/$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"sfbpf/tokdefs.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "daq - Win32 Debug"

# Begin Custom Build
InputPath=.\sfbpf\grammar.y
InputName=grammar

BuildCmds= \
	c:\cygwin\bin\bison -d -psfbpf_lval -osfbpf/$(InputName).c sfbpf/$(InputName).y \
	c:\cygwin\bin\mv sfbpf/$(InputName).h sfbpf/tokdefs.h \
	

"sfbpf/$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"sfbpf/tokdefs.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sfbpf\ieee80211.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\IP6_misc.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\ipnet.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\llc.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\namedb.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\nlpid.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\ppp.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\scanner.l

!IF  "$(CFG)" == "daq - Win32 Release"

# Begin Custom Build
InputPath=.\sfbpf\scanner.l
InputName=scanner

"sfbpf/$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\cygwin\bin\flex -i -Psfbpf_lval -osfbpf/$(InputName).c sfbpf/$(InputName).l

# End Custom Build

!ELSEIF  "$(CFG)" == "daq - Win32 Debug"

# Begin Custom Build
InputPath=.\sfbpf\scanner.l
InputName=scanner

"sfbpf/$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\cygwin\bin\flex -i -Psfbpf_lval -osfbpf/$(InputName).c sfbpf/$(InputName).l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\sfbpf\sf-redefines.h"
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sf_bpf_filter.c
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sf_bpf_printer.c
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sf_gencode.c
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sf_nametoaddr.c
# ADD CPP /I "../src/win32/WIN32-Includes/WinPCAP"
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sf_optimize.c
# End Source File
# Begin Source File

SOURCE=".\sfbpf\sfbpf-int.c"
# End Source File
# Begin Source File

SOURCE=".\sfbpf\sfbpf-int.h"
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sfbpf.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sfbpf_dlt.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sll.h
# End Source File
# Begin Source File

SOURCE=.\sfbpf\sunatmpos.h
# End Source File
# Begin Source File

SOURCE=".\sfbpf\win32-stdinc.h"
# End Source File
# End Group
# Begin Group "os-daq-modules"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\os-daq-modules\daq_pcap.c"
# ADD CPP /I "../src/win32/WIN32-Includes/WinPCAP"
# End Source File
# Begin Source File

SOURCE=".\os-daq-modules\daq_static_modules.c"
# End Source File
# Begin Source File

SOURCE=".\os-daq-modules\daq_static_modules.h"
# End Source File
# End Group
# End Target
# End Project
