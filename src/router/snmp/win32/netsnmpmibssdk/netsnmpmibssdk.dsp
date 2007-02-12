# Microsoft Developer Studio Project File - Name="netsnmpmibssdk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=netsnmpmibssdk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "netsnmpmibssdk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "netsnmpmibssdk.mak" CFG="netsnmpmibssdk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "netsnmpmibssdk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "netsnmpmibssdk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I ".." /I "..\..\agent" /I "..\..\snmplib" /I "..\..\include" /I "..\..\agent\mibgroup" /I "..\.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\..\agent" /I "..\..\agent\mibgroup" /I "." /I ".." /I "..\..\snmplib" /I "..\.." /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0xffffffff
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/netsnmpmibssdk_d.lib"

!ENDIF 

# Begin Target

# Name "netsnmpmibssdk - Win32 Release"
# Name "netsnmpmibssdk - Win32 Debug"
# Begin Group "mibII"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\snmp_mib.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\sysORTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\system_mib.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\vacm_context.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\vacm_vars.c
# End Source File
# End Group
# Begin Group "examples"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\examples\example.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\examples\ucdDemoPublic.c
# End Source File
# End Group
# Begin Group "snmpv3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\snmpv3\snmpEngine.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\snmpv3\snmpMPDStats.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\snmpv3\usmStats.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\snmpv3\usmUser.c
# End Source File
# End Group
# Begin Group "notification"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\notification\snmpNotifyFilterProfileTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\notification\snmpNotifyFilterTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\notification\snmpNotifyTable.c
# End Source File
# End Group
# Begin Group "target"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\target\snmpTargetAddrEntry.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\target\snmpTargetParamsEntry.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\target\target.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\target\target_counters.c
# End Source File
# End Group
# Begin Group "agentx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\agentx_config.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\client.c

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\master.c

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\master_admin.c

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\protocol.c

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agentx\subagent.c

!IF  "$(CFG)" == "netsnmpmibssdk - Win32 Release"

!ELSEIF  "$(CFG)" == "netsnmpmibssdk - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\agent\nsModuleTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\agent\nsTransactionTable.c
# End Source File
# End Group
# Begin Group "utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\utilities\override.c
# End Source File
# End Group
# Begin Group "mibIIsdk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\at.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\icmp.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\interfaces.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\ip.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\ipAddr.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\route_write.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\tcp.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\tcpTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\udp.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\udpTable.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\mibII\var_route.c
# End Source File
# End Group
# Begin Group "ucd-snmp"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\disk.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\dlmod.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\errormib.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\extensible.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\file.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\loadave.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\pass.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\pass_persist.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\proc.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\proxy.c"
# End Source File
# Begin Source File

SOURCE="..\..\agent\mibgroup\ucd-snmp\versioninfo.c"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\agent\mibgroup\header_complex.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mib_modules.c
# End Source File
# Begin Source File

SOURCE=..\..\agent\mibgroup\util_funcs.c
# End Source File
# End Target
# End Project
