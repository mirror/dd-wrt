# Microsoft Developer Studio Project File - Name="libsnmp_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libsnmp_dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libsnmp_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libsnmp_dll.mak" CFG="libsnmp_dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libsnmp_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libsnmp_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libsnmp_dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I ".." /I "..\..\snmplib" /I "..\..\include" /I "..\.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 wsock32.lib msvcrt.lib kernel32.lib user32.lib oldnames.lib advapi32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /def:".\libsnmp.def" /out:"../bin/libsnmp.dll"

!ELSEIF  "$(CFG)" == "libsnmp_dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0xffffffff /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib msvcrt.lib kernel32.lib user32.lib oldnames.lib advapi32.lib /nologo /subsystem:windows /dll /pdb:none /debug /machine:I386 /nodefaultlib /def:".\libsnmp.def" /out:"../bin/libsnmp_d.dll"

!ENDIF 

# Begin Target

# Name "libsnmp_dll - Win32 Release"
# Name "libsnmp_dll - Win32 Debug"
# Begin Source File

SOURCE=..\..\snmplib\asn1.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\callback.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\check_varbind.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\cmu_compat.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\container.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\container_binary_array.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\data_list.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\default_store.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\int64.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\keytools.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\lcd_time.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\mib.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\mt_support.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\oid_stash.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\parse.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\read_config.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\scapi.c
# End Source File
# Begin Source File

SOURCE="..\..\snmplib\snmp-tc.c"
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_alarm.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_api.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_auth.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_client.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_debug.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_enum.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_logging.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_parse_args.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_secmod.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_transport.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmp_version.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmpCallbackDomain.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmpTCPDomain.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmpUDPDomain.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmpusm.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\snmpv3.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\system.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\tools.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\ucd_compat.c
# End Source File
# Begin Source File

SOURCE=..\..\snmplib\vacm.c
# End Source File
# End Target
# End Project
