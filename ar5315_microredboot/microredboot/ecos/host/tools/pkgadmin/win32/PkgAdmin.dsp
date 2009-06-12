# Microsoft Developer Studio Project File - Name="PkgAdmin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PkgAdmin - Win32 ANSI Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PkgAdmin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PkgAdmin.mak" CFG="PkgAdmin - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PkgAdmin - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PkgAdmin - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PkgAdmin - Win32 ANSI Debug" (based on "Win32 (x86) Application")
!MESSAGE "PkgAdmin - Win32 ANSI Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PkgAdmin - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "V:\PkgAdmin\Release"
# PROP Intermediate_Dir "V:\PkgAdmin\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "v:\cdl\Release\include" /I "..\..\utils\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 tcl82.lib htmlhelp.lib shlwapi.lib /nologo /version:1.3 /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PkgAdmin - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "V:\PkgAdmin\Debug"
# PROP Intermediate_Dir "V:\PkgAdmin\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\utils\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 tcl82d.lib htmlhelp.lib shlwapi.lib /nologo /version:1.3 /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PkgAdmin - Win32 ANSI Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PkgAdmin___Win32_ANSI_Debug"
# PROP BASE Intermediate_Dir "PkgAdmin___Win32_ANSI_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "V:\PkgAdmin\ANSI_Debug"
# PROP Intermediate_Dir "V:\PkgAdmin\ANSI_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "v:\ide\Debug\cdl\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "v:\cdl\Debug\include" /I "..\..\utils\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 tcl81.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 tcl82d.lib htmlhelp.lib shlwapi.lib /nologo /version:1.3 /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PkgAdmin - Win32 ANSI Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PkgAdmin___Win32_ANSI_Release"
# PROP BASE Intermediate_Dir "PkgAdmin___Win32_ANSI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "V:\PkgAdmin\ANSI_Release"
# PROP Intermediate_Dir "V:\PkgAdmin\ANSI_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "v:\ide\Release\cdl\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "v:\cdl\Release\include" /I "..\..\utils\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 tcl81.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 tcl82.lib htmlhelp.lib shlwapi.lib /nologo /version:1.3 /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "PkgAdmin - Win32 Release"
# Name "PkgAdmin - Win32 Debug"
# Name "PkgAdmin - Win32 ANSI Debug"
# Name "PkgAdmin - Win32 ANSI Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Utils\win32\CSHCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\CSHDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\CSHPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\FileName.cpp
# End Source File
# Begin Source File

SOURCE=.\PkgAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\PkgAdmin.rc
# End Source File
# Begin Source File

SOURCE=.\PkgAdminDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PkgAdminLicenseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PkgAdminres.cpp
# End Source File
# Begin Source File

SOURCE=.\PkgAdminTclWaitDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\RegKeyEx.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Utils\win32\CSHCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\CSHDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\CSHPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\FileName.h
# End Source File
# Begin Source File

SOURCE=.\PkgAdmin.h
# End Source File
# Begin Source File

SOURCE=.\PkgAdminDlg.h
# End Source File
# Begin Source File

SOURCE=.\PkgAdminLicenseDlg.h
# End Source File
# Begin Source File

SOURCE=.\PkgAdminres.h
# End Source File
# Begin Source File

SOURCE=.\PkgAdminTclWaitDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\RegKeyEx.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\PkgAdmin.ico
# End Source File
# Begin Source File

SOURCE=.\res\PkgAdmin.rc2
# End Source File
# Begin Source File

SOURCE=.\res\pkgadmin1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# End Target
# End Project
