# Microsoft Developer Studio Project File - Name="sf_smtp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sf_smtp - Win32 IPv6 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sf_smtp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sf_smtp.mak" CFG="sf_smtp - Win32 IPv6 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sf_smtp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_smtp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_smtp - Win32 IPv6 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_smtp - Win32 IPv6 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sf_smtp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SMTP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\libs" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../libs/Release/sfdynamic_preproc_libs.lib /nologo /dll /machine:I386 /libpath:"../../../src/win32/WIN32-Libraries"

!ELSEIF  "$(CFG)" == "sf_smtp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SMTP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\libs" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "SF_SNORT_PREPROC_DLL" /D "_DEBUG" /D "DEBUG" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../libs/Debug/sfdynamic_preproc_libs.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../../src/win32/WIN32-Libraries"

!ELSEIF  "$(CFG)" == "sf_smtp - Win32 IPv6 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sf_smtp___Win32_IPv6_Debug"
# PROP BASE Intermediate_Dir "sf_smtp___Win32_IPv6_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "IPv6_Debug"
# PROP Intermediate_Dir "IPv6_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SNORT_PREPROC_DLL" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\libs" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "SUP_IP6" /D "SF_SNORT_PREPROC_DLL" /D "_DEBUG" /D "DEBUG" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../../src/win32/WIN32-Libraries"
# ADD LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../libs/IPv6_Debug/sfdynamic_preproc_libs.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../../src/win32/WIN32-Libraries"

!ELSEIF  "$(CFG)" == "sf_smtp - Win32 IPv6 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sf_smtp___Win32_IPv6_Release"
# PROP BASE Intermediate_Dir "sf_smtp___Win32_IPv6_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "IPv6_Release"
# PROP Intermediate_Dir "IPv6_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "NDEBUG" /D "SF_SMTP_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SNORT_PREPROC_DLL" /D "HAVE_CONFIG_H" /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\libs" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "SUP_IP6" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /libpath:"../../../src/win32/WIN32-Libraries"
# ADD LINK32 pcre.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../libs/IPv6_Release/sfdynamic_preproc_libs.lib /nologo /dll /machine:I386 /libpath:"../../../src/win32/WIN32-Libraries"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sf_smtp - Win32 Release"
# Name "sf_smtp - Win32 Debug"
# Name "sf_smtp - Win32 IPv6 Debug"
# Name "sf_smtp - Win32 IPv6 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\include\mempool.c
# End Source File
# Begin Source File

SOURCE=..\include\sf_base64decode.c
# End Source File
# Begin Source File

SOURCE=..\include\sf_dynamic_preproc_lib.c
# End Source File
# Begin Source File

SOURCE=..\include\sf_sdlist.c
# End Source File
# Begin Source File

SOURCE=..\include\sfPolicyUserData.c
# End Source File
# Begin Source File

SOURCE=.\smtp_config.c
# End Source File
# Begin Source File

SOURCE=.\smtp_log.c
# End Source File
# Begin Source File

SOURCE=.\smtp_normalize.c
# End Source File
# Begin Source File

SOURCE=.\smtp_util.c
# End Source File
# Begin Source File

SOURCE=.\smtp_xlink2state.c
# End Source File
# Begin Source File

SOURCE=.\snort_smtp.c
# End Source File
# Begin Source File

SOURCE=.\spp_smtp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\mempool.h
# End Source File
# Begin Source File

SOURCE=..\include\sf_base64decode.h
# End Source File
# Begin Source File

SOURCE=.\sf_preproc_info.h
# End Source File
# Begin Source File

SOURCE=..\include\sf_sdlist.h
# End Source File
# Begin Source File

SOURCE=.\smtp_config.h
# End Source File
# Begin Source File

SOURCE=.\smtp_log.h
# End Source File
# Begin Source File

SOURCE=.\smtp_normalize.h
# End Source File
# Begin Source File

SOURCE=.\smtp_util.h
# End Source File
# Begin Source File

SOURCE=.\smtp_xlink2state.h
# End Source File
# Begin Source File

SOURCE=.\snort_smtp.h
# End Source File
# Begin Source File

SOURCE=.\spp_smtp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
