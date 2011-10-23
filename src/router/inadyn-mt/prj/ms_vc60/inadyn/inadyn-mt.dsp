# Microsoft Developer Studio Project File - Name="inadyn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=inadyn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "inadyn-mt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "inadyn-mt.mak" CFG="inadyn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "inadyn - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "inadyn - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "inadyn"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "inadyn - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /D "NDEBUG" /D "_CONSOLE" /D "EXCLUDE_CONFIG_H" /D "USE_SNDFILE" /D "USE_THREADS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib rasapi32.lib libcmt.lib sensapi.lib winmm.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"kernel32.lib" /nodefaultlib:"user32.lib" /nodefaultlib:"gdi32.lib" /nodefaultlib:"winspool.lib" /nodefaultlib:"comdlg32.lib" /nodefaultlib:"advapi32.lib" /nodefaultlib:"shell32.lib" /nodefaultlib:"ole32.lib" /nodefaultlib:"oleaut32.lib" /nodefaultlib:"uuid.lib" /nodefaultlib:"odbc32.lib" /nodefaultlib:"odbccp32.lib" /nodefaultlib:"Ws2_32.lib" /nodefaultlib:"rasapi32.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"sensapi.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "inadyn - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /Gm /GX /ZI /Od /I "..\..\..\src\include\\" /D "_DEBUG" /D "_CONSOLE" /D "EXCLUDE_CONFIG_H" /D "USE_SNDFILE" /D "USE_THREADS" /FR /YX /FD /GZ
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib rasapi32.lib libcmt.lib sensapi.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "inadyn - Win32 Release"
# Name "inadyn - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\base64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\base64utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dblhash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dblhash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\debug_if.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\debug_service.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dyndns.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dyndns.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\errorcode.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\errorcode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\event_trap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\event_trap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\get_cmd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\get_cmd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\http_client.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\http_client.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\inadyn_cmd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\lang.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\lang.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\numbers.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\numbers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\os.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\os.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\os_psos.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\os_unix.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\os_windows.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\path.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\path.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\psos_net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\safe_mem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\safe_mem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\service.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\service.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\service_main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\service_main.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tcp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tcp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads_wrapper.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads_wrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\unicode_util.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\unicode_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\wave_util.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\wave_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\waveout.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\waveout.h
# End Source File
# End Group
# End Target
# End Project
