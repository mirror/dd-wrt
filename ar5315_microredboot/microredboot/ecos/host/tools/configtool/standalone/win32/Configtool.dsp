# Microsoft Developer Studio Project File - Name="Configtool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Configtool - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Configtool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Configtool.mak" CFG="Configtool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Configtool - Win32 ANSI Release" (based on "Win32 (x86) Application")
!MESSAGE "Configtool - Win32 ANSI Debug" (based on "Win32 (x86) Application")
!MESSAGE "Configtool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Configtool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Configtool - Win32 ANSI Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\ConfigTool\ANSIRelease"
# PROP Intermediate_Dir "v:\ConfigTool\ANSIRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "V:\cdl\Release\include" /I "..\..\common\win32" /I "..\..\standalone\win32" /I "..\..\..\ecostest\common" /I "..\..\..\testtool\win32" /I "..\..\..\Utils\common" /I "..\..\common\common" /I "..\..\..\..\..\doc\htmlhelp" /I "..\..\..\pkgadmin\win32" /I "..\..\..\utils\win32" /D "NDEBUG" /D "MLTVIEW" /D "ECOS_CT" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D _WIN32_IE=0x0400 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "..\..\..\testtool\win32" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib wininet.lib cyginfra.lib tcl82.lib cdl.lib shlwapi.lib htmlhelp.lib /nologo /version:1.3 /subsystem:windows /incremental:yes /debug /machine:I386 /libpath:"V:\cdl\Release\lib"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "Configtool - Win32 ANSI Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\ConfigTool\ANSIDebug"
# PROP Intermediate_Dir "v:\ConfigTool\ANSIDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /I "V:\cdl\Debug\include" /I "..\..\common\win32" /I "..\..\standalone\win32" /I "..\..\..\ecostest\common" /I "..\..\..\testtool\win32" /I "..\..\..\Utils\common" /I "..\..\common\common" /I "..\..\..\..\..\doc\htmlhelp" /I "..\..\..\pkgadmin\win32" /I "..\..\..\utils\win32" /D "_DEBUG" /D "_THERMOMETER" /D "MLTVIEW" /D "ECOS_CT" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D _WIN32_IE=0x0400 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "..\..\..\testtool\win32" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib wininet.lib cyginfra.lib tcl82d.lib cdl.lib shlwapi.lib htmlhelp.lib /nologo /version:1.3 /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"V:\cdl\Debug\lib"
# SUBTRACT LINK32 /incremental:no /map /nodefaultlib

!ELSEIF  "$(CFG)" == "Configtool - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Configtool___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "Configtool___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\configtool\debug"
# PROP Intermediate_Dir "v:\configtool\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "V:\ide\Debug\cdl\include" /I "." /I "..\src-mlt" /I "..\..\..\libnotcdl" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest\win32" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest\win32\testtool" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_THERMOMETER" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /I "V:\cdl\Debug\include" /I "..\..\common\win32" /I "..\..\standalone\win32" /I "..\..\..\ecostest\common" /I "..\..\..\testtool\win32" /I "..\..\..\Utils\common" /I "..\..\common\common" /I "..\..\..\..\..\doc\htmlhelp" /I "..\..\..\pkgadmin\win32" /I "..\..\..\utils\win32" /D "_DEBUG" /D "_UNICODE" /D "ECOS_CT" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D _WIN32_IE=0x0400 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /i "..\..\..\testtool\win32" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib wininet.lib cyginfra.lib tcl81.lib /nologo /version:1.2 /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"V:\ide\Debug\cdl\lib"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 wsock32.lib wininet.lib cyginfra.lib tcl82d.lib cdl.lib shlwapi.lib htmlhelp.lib /nologo /version:1.3 /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"V:\cdl\Debug\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Configtool - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Configtool___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "Configtool___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\ConfigTool\release"
# PROP Intermediate_Dir "v:\ConfigTool\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /Zi /O2 /I "V:\ide\Release\cdl\include" /I "." /I "..\src-mlt" /I "..\..\..\libnotcdl" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest\win32" /I "..\..\..\..\..\..\..\..\devo\ide\src\tools\ecostest\win32\testtool" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "V:\cdl\Release\include" /I "..\..\common\win32" /I "..\..\standalone\win32" /I "..\..\..\ecostest\common" /I "..\..\..\testtool\win32" /I "..\..\..\Utils\common" /I "..\..\common\common" /I "..\..\..\..\..\doc\htmlhelp" /I "..\..\..\pkgadmin\win32" /I "..\..\..\utils\win32" /D "NDEBUG" /D "_UNICODE" /D "MLTVIEW" /D "ECOS_CT" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D _WIN32_IE=0x0400 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /i "..\..\..\testtool\win32" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib wininet.lib cyginfra.lib tcl81.lib /nologo /version:1.2 /subsystem:windows /map /debug /machine:I386 /libpath:"V:\ide\Release\cdl\lib"
# ADD LINK32 wsock32.lib wininet.lib cyginfra.lib tcl82.lib cdl.lib shlwapi.lib htmlhelp.lib /nologo /version:1.3 /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:yes /debug /machine:I386 /libpath:"V:\cdl\Release\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Configtool - Win32 ANSI Release"
# Name "Configtool - Win32 ANSI Debug"
# Name "Configtool - Win32 Debug"
# Name "Configtool - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\win32\AddRemoveDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BCMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\BinDirDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\common\build.cxx
# ADD CPP /W3
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\BuildOptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CdlPackagesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CdlTemplatesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\Cell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CellEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CellView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Collections.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\ComboEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ConfigItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Configtool.cpp
# End Source File
# Begin Source File

SOURCE=.\Configtool.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "..\..\common\win32" /i "common\win32" /i "..\..\..\pkgadmin\win32"
# End Source File
# Begin Source File

SOURCE=.\ConfigToolDoc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ConfigViewOptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ControlView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHCommon.cpp
# ADD CPP /I "..\..\configtool\standalone\win32"
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTCommon.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTCommonDoc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTCommonres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTOptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTPropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CTres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CTUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\DescView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\DoubleEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSerial.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSocket.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosStd.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestDownloadFilter.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestPlatform.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestSerialFilter.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestUtils.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosThreadUtils.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosTrace.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\ExecutionPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FailingRulesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\FileListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\FileName.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FindDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\common\flags.cxx
# ADD CPP /W3
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\IdleMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\IntegerEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\LocalPropertiesDialog.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\memmap.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\messagebox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\mltview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\win32\MultiLineEditDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\NewFolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\NotePage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\OutputEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\OutputPage.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdmin.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminLicenseDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminTclWaitDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PlatformDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PlatformsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Properties.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PropertiesDialog.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\PropertiesList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\PropertiesView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RegionGeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\RegKeyEx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\RemotePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\ResetAttributes.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RulesList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RulesView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\RunTestsSheet.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SectionGeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SectionRelocationPage.cpp
# End Source File
# Begin Source File

SOURCE=.\splash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SplitterWndEx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\StringEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Subprocess.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\SummaryPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\TestResource.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\TestTool.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\testtoolres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\Thermometer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\thinsplitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ttlistctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewOptions.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\win32\AddRemoveDialog.h
# End Source File
# Begin Source File

SOURCE=.\BCMenu.h
# End Source File
# Begin Source File

SOURCE=.\BinDirDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\common\build.hxx
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\BuildOptionsDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CdlPackagesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CdlTemplatesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\Cell.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CellEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\cellview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Collections.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\ComboEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ConfigItem.h
# End Source File
# Begin Source File

SOURCE=.\Configtool.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ConfigToolDoc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ConfigViewOptionsDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ControlView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CSHPropertySheet.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTCommonres.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTOptionsDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\CTPropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\CTres.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\CTUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\DescView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\DoubleEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\eCosPropertySheet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSerial.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosStd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestDownloadFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestPlatform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestSerialFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\eCosTestUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosThreadUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\eCosTrace.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\ExecutionPage.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FailingRulesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\FileListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\FileName.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FindDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\common\flags.hxx
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\IdleMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\IncludeSTL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\IntegerEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\listctrltoview.inl
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\LocalPropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\memmap.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\messagebox.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\mltview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\win32\MultiLineEditDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\NewFolderDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\notepage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\OutputEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\OutputPage.h
# End Source File
# Begin Source File

SOURCE=.\OutputView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminLicenseDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminres.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\PkgAdminTclWaitDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PlatformDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PlatformsDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\PropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\PropertiesList.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\PropertiesView.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RegionGeneralPage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\RegKeyEx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\RemotePropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\ResetAttributes.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RulesList.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\RulesView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\RunTestsSheet.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SectionGeneralPage.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SectionRelocationPage.h
# End Source File
# Begin Source File

SOURCE=.\SPLASH.H
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\SplitterWndEx.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\stdafx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\win32\StringEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\Subprocess.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\SummaryPage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\ecostest\common\TestResource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\testtoolres.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\Thermometer.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\thinsplitter.h
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\treectrltoview.inl
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\ttlistctrl.h
# End Source File
# Begin Source File

SOURCE=.\ViewOptions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Utils\common\wcharunix.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Configtool.ico
# End Source File
# Begin Source File

SOURCE=.\res\Configtool.rc2
# End Source File
# Begin Source File

SOURCE=.\res\ConfigtoolDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\cygnus.ico
# End Source File
# Begin Source File

SOURCE=.\res\CygnusIcon.ico
# End Source File
# Begin Source File

SOURCE=.\res\delu1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\downbutt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\downd1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\downu1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\help.bmp
# End Source File
# Begin Source File

SOURCE=.\res\html.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\res\Miscbar.bmp
# End Source File
# Begin Source File

SOURCE=..\..\common\win32\res\MLTbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\newd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PackageIcon.ico
# End Source File
# Begin Source File

SOURCE=..\..\..\pkgadmin\win32\res\PkgAdmin.ico
# End Source File
# Begin Source File

SOURCE=.\res\Splsh16.bmp
# End Source File
# Begin Source File

SOURCE=..\..\..\testtool\win32\res\TestTool.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\upbutton.bmp
# End Source File
# Begin Source File

SOURCE=.\res\upu1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\ChangeLog
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\doc\eCos.hhp

!IF  "$(CFG)" == "Configtool - Win32 ANSI Release"

# Begin Custom Build
InputDir=\CVSroot\devo\ecos\doc
OutDir=v:\ConfigTool\ANSIRelease
InputPath=..\..\..\..\..\doc\eCos.hhp

"$(OutDir)\Configtool.chm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)\html 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do copy ..\%%f 
	echo Generating contents and index... 
	sh ../tohhc.sh 
	echo.>link2.htm 
	hhc.exe eCos.hhp > hh.out 2>&1 
	del link2.htm 
	egrep -v "HHC5003.*http" hh.out  | egrep -v "gnupro\-ref" 
	copy eCos.chm $(OutDir)\Configtool.chm 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do del %%f 
	del eCos.chm 
	del hh.out 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Configtool - Win32 ANSI Debug"

# Begin Custom Build
InputDir=\CVSroot\devo\ecos\doc
OutDir=v:\ConfigTool\ANSIDebug
InputPath=..\..\..\..\..\doc\eCos.hhp

"$(OutDir)\Configtool.chm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)\html 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do copy ..\%%f 
	echo Generating contents and index... 
	sh ../tohhc.sh 
	echo.>link2.htm 
	hhc.exe eCos.hhp > hh.out 2>&1 
	del link2.htm 
	egrep -v "HHC5003.*http" hh.out  | egrep -v "gnupro\-ref" 
	copy eCos.chm $(OutDir)\Configtool.chm 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do del %%f 
	del eCos.chm 
	del hh.out 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Configtool - Win32 Debug"

# Begin Custom Build
InputDir=\CVSroot\devo\ecos\doc
OutDir=v:\configtool\debug
InputPath=..\..\..\..\..\doc\eCos.hhp

"$(OutDir)\Configtool.chm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)\html 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do copy ..\%%f 
	echo Generating contents and index... 
	sh ../tohhc.sh 
	echo.>link2.htm 
	hhc.exe eCos.hhp > hh.out 2>&1 
	del link2.htm 
	egrep -v "HHC5003.*http" hh.out  | egrep -v "gnupro\-ref" 
	copy eCos.chm $(OutDir)\Configtool.chm 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do del %%f 
	del eCos.chm 
	del hh.out 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Configtool - Win32 Release"

# Begin Custom Build
InputDir=\CVSroot\devo\ecos\doc
OutDir=v:\ConfigTool\release
InputPath=..\..\..\..\..\doc\eCos.hhp

"$(OutDir)\Configtool.chm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir)\html 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do copy ..\%%f 
	echo Generating contents and index... 
	sh ../tohhc.sh 
	echo.>link2.htm 
	hhc.exe eCos.hhp > hh.out 2>&1 
	del link2.htm 
	egrep -v "HHC5003.*http" hh.out  | egrep -v "gnupro\-ref" 
	copy eCos.chm $(OutDir)\Configtool.chm 
	for %%f in ( Link1.htm eCos.hhc eCos.hhk eCos.hhp ) do del %%f 
	del eCos.chm 
	del hh.out 
	
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
# Section Configtool : {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
# 	2:5:Class:CWebBrowser21
# 	2:10:HeaderFile:webbrowser3.h
# 	2:8:ImplFile:webbrowser3.cpp
# End Section
# Section Configtool : {8856F961-340A-11D0-A96B-00C04FD705A2}
# 	2:21:DefaultSinkHeaderFile:webbrowser3.h
# 	2:16:DefaultSinkClass:CWebBrowser21
# End Section
