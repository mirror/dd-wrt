# Microsoft Developer Studio Project File - Name="libhelpers" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libhelpers - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libhelpers.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libhelpers.mak" CFG="libhelpers - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libhelpers - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libhelpers - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libhelpers - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../lib"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /I "..\..\agent" /I "..\..\agent\mibgroup" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/netsnmphelpers.lib"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../lib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\..\agent" /I "..\..\agent\mibgroup" /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0xffffffff
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/netsnmphelpers_d.lib"

!ENDIF 

# Begin Target

# Name "libhelpers - Win32 Release"
# Name "libhelpers - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\helpers\all_helpers.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\bulk_to_next.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\debug_handler.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\instance.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\multiplexer.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\null.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\old_api.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\read_only.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\serialize.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_array.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_data.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_dataset.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_iterator.c

!IF  "$(CFG)" == "libhelpers - Win32 Release"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\agent\helpers\all_helpers.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\bulk_to_next.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\debug_handler.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\instance.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\multiplexer.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\null.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\old_api.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\read_only.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\serialize.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_array.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_data.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_dataset.h
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_iterator.h
# End Source File
# End Group
# End Target
# End Project
