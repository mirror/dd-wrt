# Microsoft Developer Studio Project File - Name="ConfigtoolVC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ConfigtoolVC - Win32 ANSI Debug For Recent wxWin
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ConfigtoolVC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ConfigtoolVC.mak" CFG="ConfigtoolVC - Win32 ANSI Debug For Recent wxWin"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ConfigtoolVC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ConfigtoolVC - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ConfigtoolVC - Win32 ANSI Debug" (based on "Win32 (x86) Application")
!MESSAGE "ConfigtoolVC - Win32 ANSI Release" (based on "Win32 (x86) Application")
!MESSAGE "ConfigtoolVC - Win32 ANSI Debug For Recent wxWin" (based on "Win32 (x86) Application")
!MESSAGE "ConfigtoolVC - Win32 ANSI Release for Recent wxWin" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ConfigtoolVC - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\ConfigToolWX\Release"
# PROP Intermediate_Dir "v:\ConfigToolWX\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /Ob2 /I "v:\cdl\Release\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wx.lib wxmsw.lib xpm.lib png.lib zlib.lib jpeg.lib tiff.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib shlwapi.lib winmm.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /nodefaultlib:"msvcrtd.lib" /out:"v:/ConfigToolWX/Release/configtool.exe" /libpath:"v:\cdl\release\lib"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\ConfigToolWX\Debug"
# PROP Intermediate_Dir "v:\ConfigToolWX\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxd.lib wxmswd.lib xpmd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib winmm.lib shlwapi.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82d.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrt.lib" /out:"v:/ConfigToolWX/Debug/configtool.exe" /pdbtype:sept /libpath:"v:\cdl\debug\lib"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ConfigtoolVC___Win32_ANSI_Debug"
# PROP BASE Intermediate_Dir "ConfigtoolVC___Win32_ANSI_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\ConfigToolWX\ANSIDebug"
# PROP Intermediate_Dir "v:\ConfigToolWX\ANSIDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxd.lib xpmd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib winmm.lib shlwapi.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82d.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrt.lib" /out:"Debug/configtool.exe" /pdbtype:sept /libpath:"v:\cdl\debug\lib"
# ADD LINK32 wxd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib winmm.lib shlwapi.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82d.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrt.lib" /out:"v:/ConfigToolWX/ANSIDebug/configtool.exe" /pdbtype:sept /libpath:"v:\cdl\debug\lib"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ConfigtoolVC___Win32_ANSI_Release"
# PROP BASE Intermediate_Dir "ConfigtoolVC___Win32_ANSI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\ConfigToolWX\ANSIRelease"
# PROP Intermediate_Dir "v:\ConfigToolWX\ANSIRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /Ob2 /I "v:\cdl\Release\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /Ob2 /I "v:\cdl\Release\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wx.lib xpm.lib png.lib zlib.lib jpeg.lib tiff.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib shlwapi.lib winmm.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /nodefaultlib:"msvcrtd.lib" /out:"Release/configtool.exe" /libpath:"v:\cdl\release\lib"
# ADD LINK32 wx.lib png.lib zlib.lib jpeg.lib tiff.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib shlwapi.lib winmm.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /nodefaultlib:"msvcrtd.lib" /out:"v:/ConfigToolWX/ANSIRelease/configtool.exe" /libpath:"v:\cdl\release\lib"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Debug For Recent wxWin"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ConfigtoolVC___Win32_ANSI_Debug_For_Recent_wxWin"
# PROP BASE Intermediate_Dir "ConfigtoolVC___Win32_ANSI_Debug_For_Recent_wxWin"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\ConfigToolWX\ANSIDebug"
# PROP Intermediate_Dir "v:\ConfigToolWX\ANSIDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib winmm.lib shlwapi.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82d.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrt.lib" /out:"v:/ConfigToolWX/ANSIDebug/configtool.exe" /pdbtype:sept /libpath:"v:\cdl\debug\lib"
# ADD LINK32 wxmswd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib winmm.lib shlwapi.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82d.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /nodefaultlib:"msvcrt.lib" /out:"v:/ConfigToolWX/ANSIDebug/configtool.exe" /pdbtype:sept /libpath:"v:\cdl\debug\lib"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Release for Recent wxWin"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ConfigtoolVC___Win32_ANSI_Release_for_Recent_wxWin"
# PROP BASE Intermediate_Dir "ConfigtoolVC___Win32_ANSI_Release_for_Recent_wxWin"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\ConfigToolWX\ANSIRelease"
# PROP Intermediate_Dir "v:\ConfigToolWX\ANSIRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /Ob2 /I "v:\cdl\Release\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /Ob2 /I "v:\cdl\Release\include" /I "..\..\common\common" /I "..\..\..\Utils\common" /I "..\..\..\ecostest\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /Yu"ecpch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wx.lib png.lib zlib.lib jpeg.lib tiff.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib shlwapi.lib winmm.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /nodefaultlib:"msvcrtd.lib" /out:"v:/ConfigToolWX/ANSIRelease/configtool.exe" /libpath:"v:\cdl\release\lib"
# ADD LINK32 wxmsw.lib png.lib zlib.lib jpeg.lib tiff.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wininet.lib shlwapi.lib winmm.lib htmlhelp.lib cyginfra.lib cdl.lib tcl82.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /nodefaultlib:"msvcrtd.lib" /out:"v:/ConfigToolWX/ANSIRelease/configtool.exe" /libpath:"v:\cdl\release\lib"

!ENDIF 

# Begin Target

# Name "ConfigtoolVC - Win32 Release"
# Name "ConfigtoolVC - Win32 Debug"
# Name "ConfigtoolVC - Win32 ANSI Debug"
# Name "ConfigtoolVC - Win32 ANSI Release"
# Name "ConfigtoolVC - Win32 ANSI Debug For Recent wxWin"
# Name "ConfigtoolVC - Win32 ANSI Release for Recent wxWin"
# Begin Group "GUI files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\aboutdlg.h
# End Source File
# Begin Source File

SOURCE=.\admindlg.cpp
# End Source File
# Begin Source File

SOURCE=.\admindlg.h
# End Source File
# Begin Source File

SOURCE=.\appsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\appsettings.h
# End Source File
# Begin Source File

SOURCE=.\buildoptionsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\buildoptionsdlg.h
# End Source File
# Begin Source File

SOURCE=.\choosereposdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\choosereposdlg.h
# End Source File
# Begin Source File

SOURCE=.\configitem.cpp
# End Source File
# Begin Source File

SOURCE=.\configitem.h
# End Source File
# Begin Source File

SOURCE=.\configpropdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\configpropdlg.h
# End Source File
# Begin Source File

SOURCE=.\configtool.cpp
# ADD CPP /I "..\..\..\include" /Yc"ecpch.h"
# End Source File
# Begin Source File

SOURCE=.\configtool.h
# End Source File
# Begin Source File

SOURCE=.\configtool.rc
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x809
# End Source File
# Begin Source File

SOURCE=.\configtooldoc.cpp
# End Source File
# Begin Source File

SOURCE=.\configtooldoc.h
# End Source File
# Begin Source File

SOURCE=.\configtoolview.cpp
# End Source File
# Begin Source File

SOURCE=.\configtoolview.h
# End Source File
# Begin Source File

SOURCE=.\configtree.cpp
# End Source File
# Begin Source File

SOURCE=.\configtree.h
# End Source File
# Begin Source File

SOURCE=.\conflictsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\conflictsdlg.h
# End Source File
# Begin Source File

SOURCE=.\conflictwin.cpp
# End Source File
# Begin Source File

SOURCE=.\conflictwin.h
# End Source File
# Begin Source File

SOURCE=.\docsystem.cpp
# End Source File
# Begin Source File

SOURCE=.\docsystem.h
# End Source File
# Begin Source File

SOURCE=.\ecpch.h
# End Source File
# Begin Source File

SOURCE=.\ecscrolwin.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\ecscrolwin.h
# End Source File
# Begin Source File

SOURCE=.\ecutils.cpp
# End Source File
# Begin Source File

SOURCE=.\ecutils.h
# End Source File
# Begin Source File

SOURCE=.\filename.cpp
# End Source File
# Begin Source File

SOURCE=.\filename.h
# End Source File
# Begin Source File

SOURCE=.\finddlg.cpp
# End Source File
# Begin Source File

SOURCE=.\finddlg.h
# End Source File
# Begin Source File

SOURCE=.\folderdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\folderdlg.h
# End Source File
# Begin Source File

SOURCE=.\htmlparser.cpp
# End Source File
# Begin Source File

SOURCE=.\htmlparser.h
# End Source File
# Begin Source File

SOURCE=.\licensedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\licensedlg.h
# End Source File
# Begin Source File

SOURCE=.\mainwin.cpp

!IF  "$(CFG)" == "ConfigtoolVC - Win32 Release"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Debug"

# ADD BASE CPP /Yu
# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Debug For Recent wxWin"

# ADD BASE CPP /Yu
# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "ConfigtoolVC - Win32 ANSI Release for Recent wxWin"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mainwin.h
# End Source File
# Begin Source File

SOURCE=.\memmap.cpp
# End Source File
# Begin Source File

SOURCE=.\memmap.h
# End Source File
# Begin Source File

SOURCE=.\mltwin.cpp
# End Source File
# Begin Source File

SOURCE=.\mltwin.h
# End Source File
# Begin Source File

SOURCE=.\msgdlgex.cpp
# End Source File
# Begin Source File

SOURCE=.\msgdlgex.h
# End Source File
# Begin Source File

SOURCE=.\outputwin.cpp
# End Source File
# Begin Source File

SOURCE=.\outputwin.h
# End Source File
# Begin Source File

SOURCE=.\packagesdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\packagesdlg.h
# End Source File
# Begin Source File

SOURCE=.\platformeditordlg.cpp
# End Source File
# Begin Source File

SOURCE=.\platformeditordlg.h
# End Source File
# Begin Source File

SOURCE=.\platformsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\platformsdlg.h
# End Source File
# Begin Source File

SOURCE=.\propertywin.cpp
# End Source File
# Begin Source File

SOURCE=.\propertywin.h
# End Source File
# Begin Source File

SOURCE=.\reposdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\reposdlg.h
# End Source File
# Begin Source File

SOURCE=.\runtestsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\runtestsdlg.h
# End Source File
# Begin Source File

SOURCE=.\sectiondlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sectiondlg.h
# End Source File
# Begin Source File

SOURCE=.\settingsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\settingsdlg.h
# End Source File
# Begin Source File

SOURCE=.\shortdescrwin.cpp
# End Source File
# Begin Source File

SOURCE=.\shortdescrwin.h
# End Source File
# Begin Source File

SOURCE=.\solutionswin.cpp
# End Source File
# Begin Source File

SOURCE=.\solutionswin.h
# End Source File
# Begin Source File

SOURCE=.\splittree.cpp
# End Source File
# Begin Source File

SOURCE=.\splittree.h
# End Source File
# Begin Source File

SOURCE=.\templatesdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\templatesdlg.h
# End Source File
# End Group
# Begin Group "Common files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\common\build.cxx
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Collections.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Collections.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSerial.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSerial.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSocket.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosStd.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosStd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestDownloadFilter.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestDownloadFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestPlatform.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestPlatform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestSerialFilter.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestSerialFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestUtils.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosThreadUtils.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosThreadUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosTrace.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosTrace.h
# End Source File
# Begin Source File

SOURCE=..\..\common\common\flags.cxx
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Properties.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\ResetAttributes.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\ResetAttributes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Subprocess.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Subprocess.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\TestResource.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\TestResource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\wcharunix.h
# End Source File
# End Group
# Begin Group "Doc & Setup files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\about.htm
# End Source File
# Begin Source File

SOURCE=..\..\..\..\ChangeLog
# End Source File
# Begin Source File

SOURCE=.\CHANGES.txt
# End Source File
# Begin Source File

SOURCE=.\setup\innobott.txt
# End Source File
# Begin Source File

SOURCE=.\setup\innotop.txt
# End Source File
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\setup\makesetup.sh
# End Source File
# Begin Source File

SOURCE=.\setup\maketarball.sh
# End Source File
# Begin Source File

SOURCE=.\readme.html
# End Source File
# Begin Source File

SOURCE=.\README.txt
# End Source File
# Begin Source File

SOURCE=.\symbols.h
# End Source File
# Begin Source File

SOURCE=.\TODO.txt
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\ChangeLog
# End Source File
# End Target
# End Project
