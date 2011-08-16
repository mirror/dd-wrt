# Microsoft Developer Studio Project File - Name="sfdynamic_preproc_libs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sfdynamic_preproc_libs - Win32 IPv6 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sfdynamic_preproc_libs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sfdynamic_preproc_libs.mak" CFG="sfdynamic_preproc_libs - Win32 IPv6 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sfdynamic_preproc_libs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sfdynamic_preproc_libs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sfdynamic_preproc_libs - Win32 IPv6 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sfdynamic_preproc_libs - Win32 IPv6 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sfdynamic_preproc_libs - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "_LIB" /D "SF_SNORT_PREPROC_DLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ACTIVE_RESPONSE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sfdynamic_preproc_libs - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "_DEBUG" /D "DEBUG" /D "DYNAMIC_PLUGIN" /D "_LIB" /D "SF_SNORT_PREPROC_DLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ACTIVE_RESPONSE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sfdynamic_preproc_libs - Win32 IPv6 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sfdynamic_preproc_libs___Win32_IPv6_Debug"
# PROP BASE Intermediate_Dir "sfdynamic_preproc_libs___Win32_IPv6_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "IPv6_Debug"
# PROP Intermediate_Dir "IPv6_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "SUP_IP6" /D "_WINDOWS" /D "_USRDLL" /D "_DEBUG" /D "DEBUG" /D "DYNAMIC_PLUGIN" /D "_LIB" /D "SF_SNORT_PREPROC_DLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ACTIVE_RESPONSE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sfdynamic_preproc_libs - Win32 IPv6 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sfdynamic_preproc_libs___Win32_IPv6_Release"
# PROP BASE Intermediate_Dir "sfdynamic_preproc_libs___Win32_IPv6_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "IPv6_Release"
# PROP Intermediate_Dir "IPv6_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "SUP_IP6" /D "_WINDOWS" /D "_USRDLL" /D "_LIB" /D "SF_SNORT_PREPROC_DLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ACTIVE_RESPONSE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sfdynamic_preproc_libs - Win32 Release"
# Name "sfdynamic_preproc_libs - Win32 Debug"
# Name "sfdynamic_preproc_libs - Win32 IPv6 Debug"
# Name "sfdynamic_preproc_libs - Win32 IPv6 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sfparser.c
# End Source File
# Begin Source File

SOURCE=.\ssl.c
# End Source File
# Begin Source File

SOURCE="..\..\win32\WIN32-Code\strtok_r.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\sfcommon.h
# End Source File
# Begin Source File

SOURCE=.\ssl.h
# End Source File
# End Group
# End Target
# End Project
