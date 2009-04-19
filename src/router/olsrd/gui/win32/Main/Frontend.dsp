# Microsoft Developer Studio Project File - Name="Frontend" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Frontend - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Frontend.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Frontend.mak" CFG="Frontend - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Frontend - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Frontend - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Frontend - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W4 /GX /O2 /I "..\..\..\src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 olsrd_cfgparser.lib ws2_32.lib iphlpapi.lib /nologo /subsystem:windows /machine:I386 /out:"Release/Switch.exe"

!ELSEIF  "$(CFG)" == "Frontend - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "..\..\..\src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 olsrd_cfgparser.lib ws2_32.lib iphlpapi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/Switch.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Frontend - Win32 Release"
# Name "Frontend - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Frontend.cpp
# End Source File
# Begin Source File

SOURCE=.\Frontend.rc
# End Source File
# Begin Source File

SOURCE=.\FrontendDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HnaEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\MidEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\MprEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\MyDialog1.cpp
# End Source File
# Begin Source File

SOURCE=.\MyDialog2.cpp
# End Source File
# Begin Source File

SOURCE=.\MyDialog3.cpp
# End Source File
# Begin Source File

SOURCE=.\MyDialog4.cpp
# End Source File
# Begin Source File

SOURCE=.\MyEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\MyTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TrayIcon.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Frontend.h
# End Source File
# Begin Source File

SOURCE=.\FrontendDlg.h
# End Source File
# Begin Source File

SOURCE=.\HnaEntry.h
# End Source File
# Begin Source File

SOURCE=.\Ipc.h
# End Source File
# Begin Source File

SOURCE=.\MidEntry.h
# End Source File
# Begin Source File

SOURCE=.\MprEntry.h
# End Source File
# Begin Source File

SOURCE=.\MyDialog1.h
# End Source File
# Begin Source File

SOURCE=.\MyDialog2.h
# End Source File
# Begin Source File

SOURCE=.\MyDialog3.h
# End Source File
# Begin Source File

SOURCE=.\MyDialog4.h
# End Source File
# Begin Source File

SOURCE=.\MyEdit.h
# End Source File
# Begin Source File

SOURCE=.\MyTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\NodeEntry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\olsr_cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\olsr_types.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TrayIcon.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Frontend.ico
# End Source File
# Begin Source File

SOURCE=.\res\Frontend.rc2
# End Source File
# Begin Source File

SOURCE=.\res\Tray1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tray2.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\trustInfo.manifest
# End Source File
# End Target
# End Project
