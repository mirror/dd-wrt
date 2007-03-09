# Microsoft Developer Studio Project File - Name="eCosTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=eCosTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "eCosTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eCosTest.mak" CFG="eCosTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eCosTest - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "eCosTest - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eCosTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "eCosTest___Win32_Release"
# PROP BASE Intermediate_Dir "eCosTest___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "v:\eCosTest\release"
# PROP Intermediate_Dir "v:\eCosTest\release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "wsock32.lib" /I "..\common" /I "..\..\utils\common" /I "." /I "..\..\utils\win32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "eCosTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "v:\eCosTest\debug"
# PROP Intermediate_Dir "v:\eCosTest\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\common" /I "..\..\utils\common" /I "." /I "..\..\utils\win32" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
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

# Name "eCosTest - Win32 Release"
# Name "eCosTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Utils\common\Collections.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSerial.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosSocket.cpp

!IF  "$(CFG)" == "eCosTest - Win32 Release"

!ELSEIF  "$(CFG)" == "eCosTest - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosStd.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTest.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestDownloadFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestMonitorFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestPlatform.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestSerialFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosThreadUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosTrace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Properties.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ResetAttributes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Subprocess.cpp
# End Source File
# Begin Source File

SOURCE=..\common\TestResource.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Utils\common\Collections.h
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

SOURCE=..\common\eCosTest.h
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestDownloadFilter.h
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestMonitorFilter.h
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestPlatform.h
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestSerialFilter.h
# End Source File
# Begin Source File

SOURCE=..\common\eCosTestUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosThreadUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\eCosTrace.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Properties.h
# End Source File
# Begin Source File

SOURCE=..\common\ResetAttributes.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\Subprocess.h
# End Source File
# Begin Source File

SOURCE=..\common\TestResource.h
# End Source File
# Begin Source File

SOURCE=..\..\Utils\common\wcharunix.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# Begin Source File

SOURCE=..\..\Utils\ChangeLog
# End Source File
# End Target
# End Project
