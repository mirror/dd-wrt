# Microsoft Developer Studio Project File - Name="TestTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TestTool - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TestTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestTool.mak" CFG="TestTool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestTool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TestTool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TestTool - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\eCosTest\Release"
# PROP Intermediate_Dir "v:\eCosTest\TestTool\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "..\TestSheet" /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /I "..\..\Utils\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib htmlhelp.lib shlwapi.lib /nologo /subsystem:windows /machine:I386 /swaprun:net
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TestTool - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\eCosTest\Debug"
# PROP Intermediate_Dir "v:\eCosTest\TestTool\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /I "..\..\Utils\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib htmlhelp.lib shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /swaprun:net
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "TestTool - Win32 Release"
# Name "TestTool - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Utils\common\Collections.cpp
# End Source File
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

SOURCE=..\..\Utils\win32\CSHPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSerial.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosStd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestDownloadFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestPlatform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestSerialFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosThreadUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\ExecutionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\FileListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\FileName.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalPropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RemotePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\ResetAttributes.cpp
# End Source File
# Begin Source File

SOURCE=.\RunTestsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RunTestsSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Subprocess.cpp
# End Source File
# Begin Source File

SOURCE=.\SummaryPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\TestResource.cpp
# End Source File
# Begin Source File

SOURCE=.\TestTool.cpp

!IF  "$(CFG)" == "TestTool - Win32 Release"

# ADD CPP /W4 /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "TestTool - Win32 Debug"

# ADD CPP /W4 /I "C:\cvs\devo\ide\src\tools\eCosTest" /I "..\.."
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestTool.rc
# End Source File
# Begin Source File

SOURCE=.\testtoolres.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Utils\common\Collections.h
# End Source File
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

SOURCE=..\..\Utils\win32\CSHPropertySheet.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\eCosPropertySheet.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSerial.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosStd.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTest.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestDownloadFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestSerialFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestStd.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\eCosTestUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosTrace.h
# End Source File
# Begin Source File

SOURCE=.\ExecutionPage.h
# End Source File
# Begin Source File

SOURCE=.\FileListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\win32\FileName.h
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\LocalPropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\NewFolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\OutputEdit.h
# End Source File
# Begin Source File

SOURCE=.\OutputPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Properties.h
# End Source File
# Begin Source File

SOURCE=.\PropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\RemotePropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\ResetAttributes.h
# End Source File
# Begin Source File

SOURCE=.\RunTestsDlg.h
# End Source File
# Begin Source File

SOURCE=.\RunTestsPropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\RunTestsSheet.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Subprocess.h
# End Source File
# Begin Source File

SOURCE=.\SummaryPage.h
# End Source File
# Begin Source File

SOURCE=..\..\ecostest\common\TestResource.h
# End Source File
# Begin Source File

SOURCE=.\TestTool.h
# End Source File
# Begin Source File

SOURCE=.\TestToolRes.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\TestTool.ico
# End Source File
# Begin Source File

SOURCE=.\res\TestTool.rc2
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter "cnt;rtf"
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# End Target
# End Project
