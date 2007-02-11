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
# PROP Output_Dir "../lib/release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /I "..\..\agent" /I "..\..\agent\mibgroup" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/release/netsnmphelpers.lib"

!ELSEIF  "$(CFG)" == "libhelpers - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../lib/debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /I "..\..\agent" /I "..\..\agent\mibgroup" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/debug/netsnmphelpers.lib"

!ENDIF 

# Begin Target

# Name "libhelpers - Win32 Release"
# Name "libhelpers - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\agent\helpers\all_helpers.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\baby_steps.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\bulk_to_next.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\cache_handler.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\debug_handler.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\instance.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\mode_end_call.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\multiplexer.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\null.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\old_api.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\read_only.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\row_merge.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\scalar.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\scalar_group.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\serialize.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\stash_cache.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\stash_to_next.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_array.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_container.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_data.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_tdata.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_dataset.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\table_iterator.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\helpers\watcher.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\all_helpers.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\bulk_to_next.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\debug_handler.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\instance.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\multiplexer.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\null.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\old_api.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\read_only.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\serialize.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table_array.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table_data.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table_tdata.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table_dataset.h"
# End Source File
# Begin Source File

SOURCE="..\..\include\net-snmp\agent\table_iterator.h"
# End Source File
# End Group
# End Target
# End Project
